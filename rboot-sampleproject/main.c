//////////////////////////////////////////////////
// rBoot sample project.
// Copyright 2015 Richard A Burton
// richardaburton@gmail.com
// See license.txt for license terms.
//////////////////////////////////////////////////

#include <c_types.h>
#include <osapi.h>
#include <user_interface.h>
#include <time.h>
#include <mem.h>

#include "main.h"
#include "user_config.h"
#include "rboot-ota.h"
#include "uart.h"

static os_timer_t network_timer;

void ICACHE_FLASH_ATTR user_rf_pre_init(){
}

void ICACHE_FLASH_ATTR network_wait_for_ip() {

	struct ip_info ipconfig;
	os_timer_disarm(&network_timer);
	wifi_get_ip_info(STATION_IF, &ipconfig);
	if (wifi_station_get_connect_status() == STATION_GOT_IP && ipconfig.ip.addr != 0) {
		char page_buffer[40];
		os_sprintf(page_buffer,"ip: %d.%d.%d.%d\r\n",IP2STR(&ipconfig.ip));
		uart0_send(page_buffer);
	} else {
		char page_buffer[40];
		os_sprintf(page_buffer,"network retry, status: %d\r\n",wifi_station_get_connect_status());
		if(wifi_station_get_connect_status() == 3) wifi_station_connect();
		uart0_send(page_buffer);
		os_timer_setfn(&network_timer, (os_timer_func_t *)network_wait_for_ip, NULL);
		os_timer_arm(&network_timer, 2000, 0);
	}
}

void ICACHE_FLASH_ATTR wifi_config_station() {

	struct station_config stationConf;

	wifi_set_opmode(0x1);
	stationConf.bssid_set = 0;
	os_strcpy(&stationConf.ssid, SSID, os_strlen(SSID));
	os_strcpy(&stationConf.password, PASS, os_strlen(PASS));
	wifi_station_set_config(&stationConf);
	uart0_send("wifi connecting...\r\n");
	wifi_station_connect();
	os_timer_disarm(&network_timer);
	os_timer_setfn(&network_timer, (os_timer_func_t *)network_wait_for_ip, NULL);
	os_timer_arm(&network_timer, 2000, 0);
}

void ICACHE_FLASH_ATTR ShowIP() {
	struct ip_info ipconfig;
	char msg[50];
	wifi_get_ip_info(STATION_IF, &ipconfig);
	if (wifi_station_get_connect_status() == STATION_GOT_IP && ipconfig.ip.addr != 0) {
		os_sprintf(msg, "ip: %d.%d.%d.%d, mask: %d.%d.%d.%d, gw: %d.%d.%d.%d\r\n",
			IP2STR(&ipconfig.ip), IP2STR(&ipconfig.netmask), IP2STR(&ipconfig.gw));
	} else {
		os_sprintf(msg, "network status: %d\r\n", wifi_station_get_connect_status());
	}
	uart0_send(msg);
}

void ICACHE_FLASH_ATTR Switch() {
	char msg[50];
	uint8 before, after;
	before = rboot_get_current_rom();
	if (before == 0) after = 1; else after = 0;
	os_sprintf(msg, "Swapping from rom %d to rom %d.\r\n", before, after);
	uart0_send(msg);
	rboot_set_current_rom(after);
	uart0_send("Restarting...\r\n\r\n");
	system_restart();
}

static void ICACHE_FLASH_ATTR OtaUpdate_CallBack(void *arg, bool result) {
	
	char msg[40];
	rboot_ota *ota = (rboot_ota*)arg;
	
	if(result == true) {
		// success, reboot
		os_sprintf(msg, "Firmware updated, rebooting to rom %d...\r\n", ota->rom_slot);
		uart0_send(msg);
		rboot_set_current_rom(ota->rom_slot);
		system_restart();
	} else {
		// fail, cleanup
		uart0_send("Firmware update failed!\r\n");
		os_free(ota->request);
		os_free(ota);
	}
}

#define HTTP_HEADER "Connection: keep-alive\r\n\
Cache-Control: no-cache\r\n\
User-Agent: rBoot-Sample/1.0\r\n\
Accept: */*\r\n\r\n"
/**/

static void ICACHE_FLASH_ATTR OtaUpdate() {
	
	uint8 slot;
	rboot_ota *ota;
	
	// create the update structure
	ota = (rboot_ota*)os_zalloc(sizeof(rboot_ota));
	os_memcpy(ota->ip, ota_ip, 4);
	ota->port = 80;
	ota->callback = (ota_callback)OtaUpdate_CallBack;
	ota->request = (uint8 *)os_zalloc(512);
	
	// select rom slot to flash
	slot = rboot_get_current_rom();
	if (slot == 0) slot = 1; else slot = 0;
	ota->rom_slot = slot;
	
	// actual http request
	os_sprintf((char*)ota->request,
		"GET /%s HTTP/1.1\r\nHost: "IPSTR"\r\n" HTTP_HEADER,
		(slot == 0 ? "rom0.bin" : "rom1.bin"),
		IP2STR(ota->ip));
	
	// start the upgrade process
	if (rboot_ota_start(ota)) {
		uart0_send("Updating...\r\n");
	} else {
		uart0_send("Updating failed!\r\n\r\n");
		os_free(ota->request);
		os_free(ota);
	}
	
}

void ICACHE_FLASH_ATTR ProcessCommand(char* str) {
	if (!strcmp(str, "help")) {
		uart0_send("available commands\r\n");
		uart0_send("  help - display this message\r\n");
		uart0_send("  ip - show current ip address\r\n");
		uart0_send("  connect - connect to wifi\r\n");
		uart0_send("  restart - restart the esp8266\r\n");
		uart0_send("  switch - switch to the other rom and reboot\r\n");
		uart0_send("  ota - perform ota update, switch rom and reboot\r\n");
		uart0_send("\r\n");
	} else if (!strcmp(str, "connect")) {
		wifi_config_station();
	} else if (!strcmp(str, "restart")) {
		uart0_send("Restarting...\r\n\r\n");
		system_restart();
	} else if (!strcmp(str, "switch")) {
		Switch();
	} else if (!strcmp(str, "ota")) {
		OtaUpdate();
	} else if (!strcmp(str, "ip")) {
		ShowIP();
	}
}

void ICACHE_FLASH_ATTR user_init(void) {

	char msg[50];

	uart_init(BIT_RATE_115200,BIT_RATE_115200);
	uart0_send("\r\n\r\nrBoot Sample Project\r\n");
	os_sprintf(msg, "\r\nCurrently running rom %d.\r\n", rboot_get_current_rom());
	uart0_send(msg);

	uart0_send("type \"help\" and press <enter> for help...\r\n");

}
