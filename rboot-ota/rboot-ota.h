#ifndef __RBOOT_OTA_H__
#define __RBOOTOTA_H__

//////////////////////////////////////////////////
// API for OTA and rBoot config, for ESP8266.
// OTA code based on SDK sample from Espressif.
// richardaburton@gmail.com
//////////////////////////////////////////////////

#include "rboot.h"

#define UPGRADE_FLAG_IDLE		0x00
#define UPGRADE_FLAG_START		0x01
#define UPGRADE_FLAG_FINISH		0x02

typedef void (*ota_callback)(void* server);

typedef struct {
	uint8 ip[4];
	uint16 port;
	uint8 *request;
	uint8 rom_slot;
	ota_callback callback;
	bool result;
} rboot_ota;

bool rboot_ota_start(rboot_ota *ota);
rboot_config rboot_get_config();
bool rboot_set_config(rboot_config *conf);
uint8 rboot_get_current_rom();
bool rboot_set_current_rom(uint8 rom);

#endif
