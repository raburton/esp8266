#
# Makefile for rboot - https://github.com/raburton/esp8266
#
# This parent Makefile configures environment variables in one place, using
# sub-make to pass settings to each component. You can also build components
# individulally, but be sure to modify paths in each respective Makefile to
# match your environment.

# Rules:
# `all`	 compile esptool2 and copy it to the project bin dir, compile rboot,
#				 compile rboot-sampleproject (which automatically copies required
#				 headers and source files from rboot)
# `clean` run clean on esptool2, rboot, and rboot-sampleproject with inherited
#				 configuration

BUILD_DIR = build
FIRMW_DIR = firmware

ESPTOOL2			= $(abspath bin/esptool2)

RBOOT_SRCDIR		= $(abspath rboot)
RBOOT_OTA_SRCDIR	= $(abspath rboot-ota)

XTENSA_BINDIR		= $(abspath ../esp-open-sdk/xtensa-lx106-elf/bin)

SDK_BASE			= $(abspath ../esp-open-sdk/sdk)
SDK_LIBDIR		   := $(addprefix $(SDK_BASE)/,lib)
SDK_INCDIR	 	   := $(addprefix -I$(SDK_BASE)/,include)

export

.PHONY: all esptool2 rboot rboot-sampleproject clean

all: esptool2 rboot rboot-sampleproject

esptool2:
	@cd esptool2 && $(MAKE) install

rboot:
	@cd rboot && $(MAKE)

rboot-sampleproject:
	@cd rboot-sampleproject && $(MAKE)

clean:
	@cd esptool2 && $(MAKE) clean
	@cd rboot && $(MAKE) clean
	@cd rboot-sampleproject && $(MAKE) clean
