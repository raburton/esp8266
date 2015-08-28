//////////////////////////////////////////////////
// rBoot sming sample project.
// Copyright 2015 Richard A Burton
// richardaburton@gmail.com
// See license.txt for license terms.
//////////////////////////////////////////////////

#include <user_config.h>
#include <SmingCore/SmingCore.h>

#include <rboot-ota.h>

static const uint8 ota_ip[] = {192,168,7,5};

#define HTTP_HEADER "Connection: keep-alive\r\n\
Cache-Control: no-cache\r\n\
User-Agent: rBoot-SmingSample/1.0\r\n\
Accept: */*\r\n\r\n"
/**/

static void OtaUpdate_CallBack(void *arg, bool result) {
	
	char msg[40];
	rboot_ota *ota = (rboot_ota*)arg;
	
	if(result == true) {
		// success, reboot
		os_sprintf(msg, "Firmware updated, rebooting to rom %d...\r\n", ota->rom_slot);
		Serial.println(msg);
		rboot_set_current_rom(ota->rom_slot);
		System.restart();
	} else {
		// fail, cleanup
		Serial.println("Firmware update failed!\r\n");
		os_free(ota->request);
		os_free(ota);
	}
}

static void OtaUpdate() {
	
	uint8 slot;
	rboot_ota *ota;
	
	// create the update structure
	ota = (rboot_ota*)os_zalloc(sizeof(rboot_ota));
	os_memcpy(ota->ip, ota_ip, 4);
	ota->port = 80;
	ota->callback = (ota_callback)OtaUpdate_CallBack;
	ota->request = (uint8 *)os_zalloc(512);
	
	// select rom slot to flash
	//uncomment these lines to enable multiple different roms
	// (only needed if not using big flash support with 1mb slots)
	slot = rboot_get_current_rom();
	if (slot == 0) slot = 1; else slot = 0;
	ota->rom_slot = slot;
	
	// actual http request
	os_sprintf((char*)ota->request, 
		"GET /%s HTTP/1.1\r\nHost: " IPSTR "\r\n" HTTP_HEADER,
		// comment out next line if not using big flash
		"rom0.bin",
		// and uncomment this one:
		//(slot == 0 ? "rom0.bin" : "rom1.bin"),
		IP2STR(ota->ip));
	
	// start the upgrade process
	if (rboot_ota_start(ota)) {
		Serial.println("Updating...\r\n");
	} else {
		Serial.println("Updating failed!\r\n\r\n");
		os_free(ota->request);
		os_free(ota);
	}
	
}

void Switch() {
	uint8 before, after;
	before = rboot_get_current_rom();
	if (before == 0) after = 1; else after = 0;
	Serial.printf("Swapping from rom %d to rom %d.\r\n", before, after);
	rboot_set_current_rom(after);
	Serial.printf("Restarting...\r\n\r\n");
	System.restart();
}

void ShowInfo() {
    Serial.printf("\r\nSDK: v%s\r\n", system_get_sdk_version());
    Serial.printf("Free Heap: %d\r\n", system_get_free_heap_size());
    Serial.printf("CPU Frequency: %d MHz\r\n", system_get_cpu_freq());
    Serial.printf("System Chip ID: 0x%x\r\n", system_get_chip_id());
    Serial.printf("SPI Flash ID: 0x%x\r\n", spi_flash_get_id());
    //Serial.printf("SPI Flash Size: %d\r\n", (1 << ((spi_flash_get_id() >> 16) & 0xff)));
}

void serialCallBack(Stream& stream, char arrivedChar, unsigned short availableCharsCount) {
	

	if (arrivedChar == '\n') {
		char str[availableCharsCount];
		for (int i = 0; i < availableCharsCount; i++) {
			str[i] = stream.read();
			if (str[i] == '\r' || str[i] == '\n') {
				str[i] = '\0';
			}
		}
		
		if (!strcmp(str, "connect")) {
			// connect to wifi
			WifiStation.config(WIFI_SSID, WIFI_PWD);
			WifiStation.enable(true);
		} else if (!strcmp(str, "ip")) {
			Serial.printf("ip: %s mac: %s\r\n", WifiStation.getIP().toString().c_str(), WifiStation.getMAC().c_str());
		} else if (!strcmp(str, "ota")) {
			OtaUpdate();
		} else if (!strcmp(str, "switch")) {
			Switch();
		} else if (!strcmp(str, "restart")) {
			System.restart();
		} else if (!strcmp(str, "ls")) {
			Vector<String> files = fileList();
			Serial.printf("filecount %d\r\n", files.count());
			for (unsigned int i = 0; i < files.count(); i++) {
				Serial.println(files[i]);
			}
		} else if (!strcmp(str, "cat")) {
			Vector<String> files = fileList();
			Serial.printf("dumping file %s:\r\n", files[0].c_str());
			if (files.count() > 0) {
				Serial.println(fileGetContent(files[0]));
			}
		} else if (!strcmp(str, "info")) {
			ShowInfo();
		} else if (!strcmp(str, "help")) {
			Serial.println();
			Serial.println("available commands:");
			Serial.println("  help - display this message");
			Serial.println("  ip - show current ip address");
			Serial.println("  connect - connect to wifi");
			Serial.println("  restart - restart the esp8266");
			Serial.println("  switch - switch to the other rom and reboot");
			Serial.println("  ota - perform ota update, switch rom and reboot");
			Serial.println("  info - show esp8266 info");
			Serial.println("  ls - list files in spiffs");
			Serial.println("  cat - show first file in spiffs");
			Serial.println();
		} else {
			Serial.println("unknown command");
		}
	}
}

void init() {
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true);

	int slot = rboot_get_current_rom();
	if (slot == 0) {
		spiffs_mount_manual(0x40250000, 0x30000);
	} else {
		spiffs_mount_manual(0x40450000, 0x30000);
	}
	
	// disable access point
	WifiAccessPoint.enable(false);
	
	Serial.println("Type 'help' and press enter for instructions.");
	Serial.println();
	
	Serial.setCallback(serialCallBack);
	
}
