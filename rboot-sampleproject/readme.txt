This is a sample project to show how to build roms for use with rBoot and how to
perform an OTA update.

To compile
----------
1) You will need to symlink or copy rboot.h, rboot-api.h, rboot-api.c,
   rboot-ota.h, rboot-ota.c in to this directory.
2) You will also need a compiled copy of esptool2.
3) Edit the Makefile to set the paths to the SDK and esptool2.
4) Set WIFI_SSID & WIFI_PWD as env vars or in the makefile.
5) Set OTA server IP in main.h
6) If you haven't already compiled rBoot you'll need to do that too.
7) Flash, as below.
8) Connect a terminal and type 'help'.

All the above are available from GitHub: https://github.com/raburton/esp8266

Once built simply flash with something like this:
  esptool.py --port COM2 write_flash -fs 8m 0x00000 rboot.bin 0x02000 rom0.bin 0x82000 rom1.bin 0xfc000 blank4.bin

Tested with SDK v1.3 on an ESP12 (if using a board with less than 1mb of flash
a change to the second linker script will be required).
