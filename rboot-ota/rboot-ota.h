#ifndef __RBOOT_OTA_H__
#define __RBOOT_OTA_H__

//////////////////////////////////////////////////
// rBoot OTA sample code for ESP8266.
// Copyright 2015 Richard A Burton
// richardaburton@gmail.com
// See license.txt for license terms.
// OTA code based on SDK sample from Espressif.
//////////////////////////////////////////////////

#include "rboot-api.h"

#ifdef __cplusplus
extern "C" {
#endif

// timeout for the initial connect and each recv (in ms)
#define OTA_NETWORK_TIMEOUT  10000

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

#ifdef __cplusplus
}
#endif

#endif
