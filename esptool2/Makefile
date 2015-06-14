CFLAGS = -Os -O2
CC = gcc
LD = gcc

ESPTOOL2	   ?= $(abspath ../bin/esptool2)
INSTALL_BINDIR	= $(basename $(ESPTOOL2))

.PHONY: all clean install

all: esptool2

esptool2.o: esptool2.c esptool2.h esptool2_elf.h elf.h
	$(CC) $(CFLAGS) -c $< -o $@

esptool2_elf.o: esptool2_elf.c esptool2.h esptool2_elf.h elf.h
	$(CC) $(CFLAGS) -c $< -o $@

esptool2: esptool2.o esptool2_elf.o
	$(LD) -o $@ $^

install: esptool2
	@cp esptool2 $(INSTALL_BINDIR)

clean:
	@rm -f *.o
	@rm -f esptool2
	@rm -f $(INSTALL_BINDIR)
