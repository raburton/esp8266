#
# Makefile for all buildable projects, inc. esptool2 and rBoot
# https://github.com/raburton/esp8266
#
# This parent Makefile configures environment variables in one place, using
# sub-make to pass settings to each component. You can also build components
# individually, but be sure to modify paths in each respective Makefile to
# match your environment.

# Rules:
# `all`	 compile esptool2 and copy it to the project bin dir, compile rboot,
#				 compile rboot-sampleproject (which automatically copies required
#				 headers and source files from rBoot)
# `clean` run clean on esptool2, rboot, and rboot-sampleproject with inherited
#				 configuration

ESPTOOL2      ?= $(abspath esptool2/esptool2.exe)
SDK_BASE      ?= C:/esp_iot_sdk_v1.1.2
XTENSA_BINDIR ?= C:/xtensa-lx106-elf/bin

export

.PHONY: all esptool2 rboot rboot-sampleproject clean

all: esptool2 rboot rboot-sampleproject

esptool2:
	@echo "Building esptool2 firmware tool"
	@cd esptool2 && $(MAKE)

rboot:
	@echo "Building rBoot boot loader"
	@cd rboot && $(MAKE)

rboot-sampleproject/rboot.h: rboot/rboot.h
	@cp $< $@

rboot-sampleproject/rboot-ota.h: rboot-ota/rboot-ota.h
	@cp $< $@

rboot-sampleproject/rboot-ota.c: rboot-ota/rboot-ota.c
	@cp $< $@

rboot-sampleproject: rboot-sampleproject/rboot.h rboot-sampleproject/rboot-ota.h rboot-sampleproject/rboot-ota.c
	@echo "Building rBoot sample project"
	@cd rboot-sampleproject && $(MAKE)

clean:
	@cd esptool2 && $(MAKE) clean
	@cd rboot && $(MAKE) clean
	@cd rboot-sampleproject && $(MAKE) clean
