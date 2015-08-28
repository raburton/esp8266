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
    http://www.esp8266.com/viewtopic.php?f=35&t=4288&p=26914#p26914
 1) Edit Sming/appinit/user_main.cpp and comment out "spiffs_mount();"
 2) Copy rboot.h (the version you used to build rBoot) & rboot-ota.h to the
    include directory.
 3) Copy rboot-bigflash.c & rboot-smingota.c to app directory.
 4) Set ESP_HOME & SMING_HOME, as environment variables or edit the Makefile, as
    you would for general Sming compiling.
 5) Set ESPTOOL2 (env var or in the Makefile) to point to the esptool2 binary.
 6) Edit the OTA ip address (ota_ip) at the top of application.cpp to point to
    your OTA webserver.
 7) Set WIFI_SSID & WIFI_PWD environment variable with your wifi details.
 8) make
 9) Create spiffs filesystem (currently a manual step, tested at length 0x30000).
10) Flash rom and spiffs (see below).
11) Put rom0.bin in the root of your webserver for OTA.

Flashing
--------
Use esptool.py to flash rBoot, rom & spiffs:
  esptool.py --port <port> write_flash -fs 32m 0x00000 rboot.bin 0x02000 rom0.bin
    0x50000 spiffs.rom

You can also flash rom0.bin to 0x202000, but booting and using OTA is quicker! At
the moment OTA for spiffs is't implemented so you'll need to manually flash the
spiffs for the second rom to 0x0x250000, or an empty one will be created and
formatted when the second rom first boots.

Disabling big flash
-------------------
This assumes you understand the concepts explained in the rBoot readme about
memory mapping and setting linker script address. This is not covered here, just
how to use this sample without bigflash support.

- Copy rom0.ld to rom1.ld.
- Adjust the rom offsets and length as appropriate in each ld file.
- Edit the "all" target in the Makefile, comment out the default version and
  uncomment the alternate version. This will cause two roms to be built.
- Uncomment the indicated lines in OtaUpdate (in application.cpp) so the http
  request gets the correct rom.
- After building copy all the rom*.bin files to the root of your web server.

If you want more than two roms you must be an advanced user and should be able
to work out what to copy and edit to acheive this!

Credits
-------
piperpilot, gschmott on esp8266.com made this sample possible.
