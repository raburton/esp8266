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

ESPTOOL2      ?= $(abspath esptool2/esptool2)
SDK_BASE      ?= /opt/esp-open-sdk/sdk
# XTENSA_BINDIR needs trailing slash or can be blank if already in $PATH
XTENSA_BINDIR ?= /opt/esp-open-sdk/xtensa-lx106-elf/bin/

export

.PHONY: all esptool2 rboot rboot-sampleproject clean

all: esptool2 rboot rboot-sampleproject

esptool2:
	@echo "Building esptool2 firmware tool"
	@$(MAKE) -C esptool2

rboot:
	@echo "Building rBoot boot loader"
	@$(MAKE) -C rboot

rboot-sampleproject/rboot.h: rboot/rboot.h
	@echo "Copying $< to $@"
	@cp $< $@

rboot-sampleproject/rboot-api.h: rboot/appcode/rboot-api.h
	@echo "Copying $< to $@"
	@cp $< $@

rboot-sampleproject/rboot-api.c: rboot/appcode/rboot-api.c
	@echo "Copying $< to $@"
	@cp $< $@

rboot-sampleproject: rboot-sampleproject/rboot.h rboot-sampleproject/rboot-api.h rboot-sampleproject/rboot-api.c
	@echo "Building rBoot sample project"
	@$(MAKE) -C rboot-sampleproject

clean:
	@$(MAKE) -C esptool2 clean
	@$(MAKE) -C rboot clean
	@$(MAKE) -C rboot-sampleproject clean
