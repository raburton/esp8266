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

.PHONY: all esptool2 rboot rboot-sampleproject clean

all: esptool2 rboot rboot-sampleproject

esptool2:
	@echo "Building esptool2 firmware tool"
	@$(MAKE) -Cesptool2

rboot:
	@echo "Building rBoot boot loader"
	@$(MAKE) -Crboot

rboot-sampleproject:
	@echo "Building rBoot sample project"
	@$(MAKE) -Crboot-sampleproject

clean:
	@$(MAKE) -Cesptool2 clean
	@$(MAKE) -Crboot clean
	@$(MAKE) -Crboot-sampleproject clean
