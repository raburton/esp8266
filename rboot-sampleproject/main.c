//////////////////////////////////////////////////
// rBoot sample project.
// richardaburton@gmail.com
//////////////////////////////////////////////////

#include <c_types.h>
#include <osapi.h>
#include <user_interface.h>
#include <time.h>

#include "user_config.h"
#include "rboot-ota.h"
#include "uart.h"

static os_timer_t network_timer;

void ICACHE_FLASH_ATTR network_wait_for_ip() {

	struct ip_info ipconfig;
	os_timer_disarm(&network_timer);
	wifi_get_ip_info(STATION_IF, &ipconfig);
	if (wifi_station_get_connect_status() == STATION_GOT_IP && ipconfig.ip.addr != 0) {
		char page_buffer[40];
		os_sprintf(page_buffer,"myIP: %d.%d.%d.%d\r\n",IP2STR(&ipconfig.ip));
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

void ICACHE_FLASH_ATTR network_init() {
	uart0_send("init network 1\r\n");
	os_timer_disarm(&network_timer);
	os_timer_setfn(&network_timer, (os_timer_func_t *)network_wait_for_ip, NULL);
	os_timer_arm(&network_timer, 2000, 0);
	uart0_send("init network 2\r\n");
}

void ICACHE_FLASH_ATTR wifi_config_station() {
	// Wifi configuration
	//char ssid[9] = "ssid";
	//char password[13] = "password";

	struct station_config stationConf;

	//Set station mode
	wifi_set_opmode(0x1);

	//Set ap settings
	stationConf.bssid_set = 0;
	os_strcpy(&stationConf.ssid, SSID, os_strlen(SSID));
	os_strcpy(&stationConf.password, PASS, os_strlen(PASS));
	//os_memcpy(&stationConf.ssid, ssid, 9);
	//os_memcpy(&stationConf.password, password, 13);

	wifi_station_set_config(&stationConf);
	uart0_send("init wifi\r\n");

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

void ICACHE_FLASH_ATTR ProcessCommand(char* str) {
	if (!strcmp(str, "help")) {
		uart0_send("available commands\r\n");
		uart0_send("  help - display this message\r\n");
		uart0_send("  ip - show current ip address\r\n");
		uart0_send("  connect - connect to wifi\r\n");
		uart0_send("  restart - restart the esp8266\r\n");
		uart0_send("  switch - switch to the other rom and reboot\r\n");
		//uart0_send("  ota - perform ota update, switch rom and reboot\r\n");
		uart0_send("\r\n");
	} else if (!strcmp(str, "connect")) {
		wifi_config_station();
	} else if (!strcmp(str, "restart")) {
		uart0_send("Restarting...\r\n\r\n");
		system_restart();
	} else if (!strcmp(str, "switch")) {
		Switch();
	} else if (!strcmp(str, "ota")) {
		//otaUpdate();
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
