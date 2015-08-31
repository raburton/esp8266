rBoot Sming sample
------------------

** work in progress **

This sample integrates rBoot and Sming, for the many people who have been asking
for it. It demonstrates dual rom booting, big flash support, OTA updates and
dual spiffs filesystems. You must enable big flash support in rBoot and use on
an ESP12 (or similar device with 4mb flash). When using rBoot big flash support
with multiple 1mb slots only one rom image needs to be created. If you don't
want to use big flash support at all see the separate instructions below.

Building
--------
 0) Patch and compile sming, patch available here:
    https://patch-diff.githubusercontent.com/raw/anakod/Sming/pull/244.diff
 1) Copy or symlink rboot.h (the version you used to build rBoot) & rboot-ota.h
    to the include directory.
 2) Copy or symlink rboot-bigflash.c & rboot-smingota.c to the app directory.
 3) Set ESP_HOME & SMING_HOME, as environment variables or edit the Makefile, as
    you would for general Sming compiling.
 4) Set ESPTOOL2 (env var or in the Makefile) to point to the esptool2 binary.
 5) Edit the OTA ip address (ota_ip) at the top of application.cpp to point to
    your OTA webserver.
 6) Set WIFI_SSID & WIFI_PWD environment variable with your wifi details.
 7) make
 8) Flash rom and spiffs (see below).
 9) Put rom0.bin in the root of your webserver for OTA.

Flashing
--------
Use esptool.py to flash rBoot, rom & spiffs:
 esptool.py --port <port> write_flash -fs 32m 0x00000 rboot.bin 0x02000 rom0.bin
   0x100000 spiffs.rom

You can also flash rom0.bin to 0x202000, but booting and using OTA is quicker!
At the moment OTA for spiffs isn't implemented so you'll need to manually flash
the spiffs for the second rom to 0x300000 (or an empty one will be created and
formatted when the second rom first boots).

Notes
-----
spiffs_mount_manual(address, length) must be called from init. The address must
be 0x40200000 + physical flash address. It does not use memory mapped flash so
the reason for this strange addressing is not clear.

Important compiler flags used:
DISABLE_SPIFFS - doesn't disable spiffs, but prevents automounting at the wrong
  location (in Sming/appinit/user_main.cpp). Instead we call spiffs_mount_manual
  from init.
RBOOT_BUILD_SMING - ensures big flash support function is correcly marked to
  remain in iram (plus potentially other sming specific code in future).

Disabling big flash
-------------------
If you want to use, for example, two 512k roms in the first 1mb block of flash
(old style) then follow these instructions two produce two separately linked
roms. If you are flashing a single rom to multiple 1mb flash blocks (using big
flash) you only need one linked rom that can be used on each.

This assumes you understand the concepts explained in the rBoot readme about
memory mapping and setting linker script address. This is not covered here, just
how to use this sample without bigflash support.

- Do not copy/link rboot-bigflash.c in to your app directory.
- Copy rom0.ld to rom1.ld.
- Adjust the rom offsets and length as appropriate in each ld file.
- Uncomment 'TWO_ROMS ?= 1' in the makefile (or set as an environment variable).
- After building copy all the rom*.bin files to the root of your web server.

If you want more than two roms you must be an advanced user and should be able
to work out what to copy and edit to acheive this!

Credits
-------
Assistance from piperpilot & gschmott on esp8266.com made this sample possible.
