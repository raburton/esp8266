This is a sample project to show how to build roms for use with rBoot and how to
perform an OTA update.

To compile
----------
1) You will need to copy rboot.h rboot-ota.h & rboot-ota.c in to this directory.
2) You will also need a compiled copy of esptool2.
3) Edit the Makefile to set the paths to the SDK and esptool2.
4) Put your SSID, password and OTA server IP in main.h
4) If you haven't already compiled rBoot you'll need to do that too.

Tested with SDK v1.1.1_15_06_05.

All the above are available from GitHub: https://github.com/raburton/esp8266

Once built simply flash with something like this:
	esptool.py --port COM7 write_flash -fs 8m 0x00000 rboot.bin 0x02000 rom0.bin 0x82000 rom1.bin 0xfc000 blank4.bin
