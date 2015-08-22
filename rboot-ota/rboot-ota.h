#ifndef __RBOOT_OTA_H__
#define __RBOOT_OTA_H__

//////////////////////////////////////////////////
// API for OTA and rBoot config, for ESP8266.
// Copyright 2015 Richard A Burton
// richardaburton@gmail.com
// See license.txt for license terms.
// OTA code based on SDK sample from Espressif.
//////////////////////////////////////////////////

#include "rboot.h"

// timeout for the initial connect (in ms)
#define OTA_CONNECT_TIMEOUT  10000

// timeout for each ota download recv
#define OTA_DOWNLOAD_TIMEOUT 10000

#define UPGRADE_FLAG_IDLE		0x00
#define UPGRADE_FLAG_START		0x01
#define UPGRADE_FLAG_FINISH		0x02

#define FLASH_BY_ADDR 0xff

typedef void (*ota_callback)(void* server, bool result);

typedef struct {
	uint8 ip[4];      // ota server ip
	uint16 port;      // ota server port
	uint8 *request;   // http request header
	uint8 rom_slot;   // rom slot to update, or FLASH_BY_ADDR
	uint32 rom_addr;  // address to flash when rom_slot==FLASH_BY_ADDR (otherwise ignored)
	ota_callback callback;  // user callback when completed
} rboot_ota;

bool rboot_ota_start(rboot_ota *ota);
rboot_config rboot_get_config();
bool rboot_set_config(rboot_config *conf);
uint8 rboot_get_current_rom();
bool rboot_set_current_rom(uint8 rom);

#endif
