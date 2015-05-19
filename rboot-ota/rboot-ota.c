//////////////////////////////////////////////////
// API for OTA and rBoot config, for ESP8266.
// OTA code based on SDK sample from Espressif.
// richardaburton@gmail.com
//////////////////////////////////////////////////

#include <c_types.h>
#include <user_interface.h>
#include <espconn.h>
#include <mem.h>
#include <osapi.h>

#include "rboot-ota.h"

// structure to hold our internal update state
typedef struct {
	uint32 start_addr;
	uint32 start_sector;
	uint32 max_sector_count;
	uint32 last_sector_erased;
	uint8 extra_count;
	uint8 extra_bytes[4];
	rboot_ota *ota;
	uint32 totallength;
	uint32 sumlength;
	struct espconn *conn;
} upgrade_param;

static upgrade_param *upgrade;
static os_timer_t ota_timer;

// get the rboot config
rboot_config ICACHE_FLASH_ATTR rboot_get_config() {
	rboot_config conf;
	spi_flash_read(BOOT_CONFIG_SECTOR * SECTOR_SIZE, (uint32*)&conf, sizeof(rboot_config));
	return conf;
}

// write the rboot config
// preserves contents of rest of sector, so rest
// of sector can be used to store user data
bool ICACHE_FLASH_ATTR rboot_set_config(rboot_config *conf) {
	uint8 *buffer;
	buffer = (uint8*)os_malloc(SECTOR_SIZE);
	if (!buffer) {
		uart0_send("no ram!\r\n");
		return false;
	}
	spi_flash_read(BOOT_CONFIG_SECTOR * SECTOR_SIZE, (uint32*)buffer, SECTOR_SIZE);
	memcpy(buffer, conf, sizeof(rboot_config));
	spi_flash_erase_sector(BOOT_CONFIG_SECTOR);
	spi_flash_write(BOOT_CONFIG_SECTOR * SECTOR_SIZE, (uint32*)buffer, SECTOR_SIZE);
	os_free(buffer);
	return true;
}

// get current boot rom
uint8 ICACHE_FLASH_ATTR rboot_get_current_rom() {
	rboot_config conf;
	conf = rboot_get_config();
	return conf.current_rom;
}

// set current boot rom
bool ICACHE_FLASH_ATTR rboot_set_current_rom(uint8 rom) {
	rboot_config conf;
	conf = rboot_get_config();
	if (rom >= conf.count) return false;
	conf.current_rom = rom;
	return rboot_set_config(&conf);
}

// function to do the actual writing to flash
static bool ICACHE_FLASH_ATTR write_flash(uint8 *data, uint16 len) {
	
	bool ret = false;
	uint8 *buffer;
	
	if (data == NULL || len == 0) {
		return true;
	}
	
	// get a buffer
	buffer = (uint8 *)os_zalloc(len + upgrade->extra_count);

	// copy in any remaining bytes from last chunk
	os_memcpy(buffer, upgrade->extra_bytes, upgrade->extra_count);
	// copy in new data
	os_memcpy(buffer + upgrade->extra_count, data, len);

	// calculate length, must be multiple of 4
	// save any remaining bytes for next go
	len += upgrade->extra_count;
	upgrade->extra_count = len % 4;
	len -= upgrade->extra_count;
	os_memcpy(upgrade->extra_bytes, buffer + len, upgrade->extra_count);

	// check data will fit
	if (upgrade->start_addr + len < (upgrade->start_sector + upgrade->max_sector_count) * SECTOR_SIZE) {

		if (len > SECTOR_SIZE) {
			// here we should erase current (if not already done), next
			// and possibly later sectors too, but doesn't look like we
			// actually ever get more than 4k at a time though
		} else {
			// check if sector the write finishes in has been erased yet,
			// this is fine as long as data len < sector size
			if (upgrade->last_sector_erased != (upgrade->start_addr + len) / SECTOR_SIZE) {
				upgrade->last_sector_erased = (upgrade->start_addr + len) / SECTOR_SIZE;
				spi_flash_erase_sector(upgrade->last_sector_erased);
			}
		}

		// write current chunk
		if (spi_flash_write(upgrade->start_addr, (uint32 *)buffer, len) == SPI_FLASH_RESULT_OK) {
			ret = true;
			upgrade->start_addr += len;
		}
	}

	os_free(buffer);
	return ret;
}

// initialise the internal update state structure
static bool ICACHE_FLASH_ATTR rboot_ota_init(rboot_ota *ota) {

	rboot_config bootconf;

	upgrade = (upgrade_param*)os_zalloc(sizeof(upgrade_param));
	if (!upgrade) {
		uart0_send("no ram!\r\n");
		return false;
	}
	
	// store user update options
	upgrade->ota = ota;
	
	// get details of rom slot to update
	bootconf = rboot_get_config();
	if ((ota->rom_slot > bootconf.count) || (bootconf.roms[ota->rom_slot] % 4)) {
		uart0_send("Bad rom slot.\r\n");
		os_free(upgrade);
		return false;
	}
	upgrade->start_addr = bootconf.roms[ota->rom_slot];
	upgrade->start_sector = bootconf.roms[ota->rom_slot] / SECTOR_SIZE;
	upgrade->max_sector_count = 200; //todo fix
	
	// create connection
	upgrade->conn = (struct espconn *)os_zalloc(sizeof(struct espconn));
	if (!upgrade->conn) {
		uart0_send("no ram!\r\n");
		os_free(upgrade);
		return false;
	}
	upgrade->conn->proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));
	if (!upgrade->conn->proto.tcp) {
		os_free(upgrade->conn);
		upgrade->conn = 0;
		uart0_send("no ram!\r\n");
		os_free(upgrade);
		return false;
	}
	
	// set update flag
	system_upgrade_flag_set(UPGRADE_FLAG_START);
	
	return true;
}

// clean up at the end of the update
// will call the user call back to indicate completion
static void ICACHE_FLASH_ATTR rboot_ota_deinit() {
	
	rboot_ota *ota;
	struct espconn *conn;

	os_timer_disarm(&ota_timer);
	
	// save only remaining bits of interest from upgrade struct
	// then we can clean it up early, so disconnect callback
	// can distinguish between us calling it after update finished
	// or being called earlier in the update process
	ota = upgrade->ota;
	conn = upgrade->conn;
	
	// clean up
	os_free(upgrade);
	upgrade = 0;
	
	// if connected, disconnect and clean up connection
	if (conn) espconn_disconnect(conn);
	
	// check for completion
	if (system_upgrade_flag_check() == UPGRADE_FLAG_FINISH) {
		ota->result = true;
	} else {
		system_upgrade_flag_set(UPGRADE_FLAG_IDLE);
		ota->result = false;
	}
	
	// call user call back
	if (ota->callback) {
		ota->callback(ota);
	}
	
}

// called when connection receives data (hopefully the rom)
static void ICACHE_FLASH_ATTR upgrade_recvcb(void *arg, char *pusrdata, unsigned short length) {
	
	char *ptrData, *ptrLen, *ptr;
	
	// first reply?
	if (upgrade->totallength == 0) {
		//	valid http response?
		if ((ptrLen = os_strstr(pusrdata, "Content-Length: ")) && (ptrData = os_strstr(ptrLen, "\r\n\r\n"))) {
			// end of header/start of data
			ptrData += 4;
			// length of data after header in this chunk
			length -= (ptrData - pusrdata);
			// running total of download length
			upgrade->totallength += length;
			// process current chunk
			write_flash((uint8*)ptrData, length);
			// work out total download size
			ptrLen += 16;
			ptr = (char *)os_strstr(ptrLen, "\r\n");
			*ptr = '\0'; // destructive
			upgrade->sumlength = atoi(ptrLen);
		} else {
			// fail
			// not a valid http header
		}
	} else {
		// not the first chunk, process it
		upgrade->totallength += length;
		write_flash((uint8*)pusrdata, length);
	}

	// check if we are finished
	if (upgrade->totallength == upgrade->sumlength) {
		uart0_send("upgrade file download finished.\r\n");
		system_upgrade_flag_set(UPGRADE_FLAG_FINISH);
		// clean up and call user callback
		rboot_ota_deinit();
	} else if (upgrade->conn->state != ESPCONN_READ) {
		// fail, but how do we get here? premature end of stream?
		rboot_ota_deinit();
	}
}

// disconnect callback, clean up the connection
// we also call this ourselves
static void ICACHE_FLASH_ATTR upgrade_disconcb(void *arg) {
	// use passed ptr, as upgrade struct may have gone by now
	struct espconn *conn = (struct espconn*)arg;
	
	os_timer_disarm(&ota_timer);
	if (conn) {
		if (conn->proto.tcp) os_free(conn->proto.tcp);
		os_free(conn);
	}
	
	// is upgrade struct still around?
	// if so disconnect was from remote end, or we called
	// ourselves to cleanup a failed connection attempt
	if (upgrade) {
		// mark connection as gone
		upgrade->conn = 0;
		// end the update process
		rboot_ota_deinit();
	}
}

// successfully connected to update server, send the request
static void ICACHE_FLASH_ATTR upgrade_connect_cb(void *arg) {
	
	// disable the timeout
	os_timer_disarm(&ota_timer);

	// register connection callbacks
	espconn_regist_disconcb(upgrade->conn, upgrade_disconcb);
	espconn_regist_recvcb(upgrade->conn, upgrade_recvcb);

	// send the http request, with timeout
	os_timer_setfn(&ota_timer, (os_timer_func_t *)rboot_ota_deinit, 0);
	os_timer_arm(&ota_timer, 10000, 0);
	espconn_sent(upgrade->conn, upgrade->ota->request, os_strlen((char*)upgrade->ota->request));
}

// connection attempt timed out
static void ICACHE_FLASH_ATTR connect_timeout_cb() {
	uart0_send("Connect timeout.\r\n");
	// not connected so don't call disconnect on the connection
	// but call our own disconnect callback to do the cleanup
	upgrade_disconcb(upgrade->conn);
}

// call back for lost connection
static void ICACHE_FLASH_ATTR upgrade_recon_cb(void *arg, sint8 errType) {
	uart0_send("Connection error.\r\n");
	// not connected so don't call disconnect on the connection
	// but call our own disconnect callback to do the cleanup
	upgrade_disconcb(upgrade->conn);
}

// start the ota process, with user supplied options
bool ICACHE_FLASH_ATTR rboot_ota_start(rboot_ota *ota) {
	
	// check not already updating
	if (system_upgrade_flag_check() == UPGRADE_FLAG_START) {
		return false;
	}
	
	// check parameters
	if (!ota || !ota->request) {
		uart0_send("invalid parameters\r\n");
		return false;
	}
	
	// set up update structure
	if (!rboot_ota_init(ota)) {
		return false;
	}
	
	// set up connection
	upgrade->conn->type = ESPCONN_TCP;
	upgrade->conn->state = ESPCONN_NONE;
	upgrade->conn->proto.tcp->local_port = espconn_port();
	upgrade->conn->proto.tcp->remote_port = ota->port;
	*(uint32*)upgrade->conn->proto.tcp->remote_ip = *(uint32*)ota->ip;
	// set connection call backs
	espconn_regist_connectcb(upgrade->conn, upgrade_connect_cb);
	espconn_regist_reconcb(upgrade->conn, upgrade_recon_cb);

	// try to connect
	espconn_connect(upgrade->conn);

	// set connection timeout timer
	os_timer_disarm(&ota_timer);
	os_timer_setfn(&ota_timer, (os_timer_func_t *)connect_timeout_cb, 0);
	os_timer_arm(&ota_timer, 10000, 0);

	return true;
}
