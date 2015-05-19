TARGET_ARCH	= WINDOWS
CFLAGS		= -Os -O2
SDK_LIBDIR	=
SDK_LIBS	=
SDK_INCLUDES	=

CC		:= gcc
LD		:= gcc

MODULES		:= infohelper elf binimage argparse


-include local/Makefile.local.$(TARGET_ARCH)
SRC_DIR		:= $(MODULES)
BUILD_DIR	:= $(addprefix build/,$(MODULES))

SRC		:= $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.c))
OBJ		:= $(patsubst %.c,build/%.o,$(SRC))
INCLUDES	:= $(addprefix -I,$(SRC_DIR))

all: esptool2

esptool2.o: esptool2.c esptool2.h esptool2_elf.h elf.h
	$(CC) $(CFLAGS) -c $< -o $@

esptool2_elf.o: esptool2_elf.c esptool2.h esptool2_elf.h elf.h
	$(CC) $(CFLAGS) -c $< -o $@

esptool2: esptool2.o esptool2_elf.o
	$(LD) -o $@ $^

clean:
	@rm -f *.o
	@rm -f esptool2
