BUILD_DIR = build
FIRMW_DIR = firmware

LIBS    = c gcc hal phy net80211 lwip wpa main pp
CFLAGS  = -Os -g -O2 -Wpointer-arith -Wundef -Werror -Wl,-EL -fno-inline-functions -nostdlib -mlongcalls -mtext-section-literals  -D__ets__ -DICACHE_FLASH
LDFLAGS = -nostdlib -Wl,--no-check-sections -u call_user_start -Wl,-static

XTENSA_TOOLS_BINDIR ?= $(abspath ../../esp-open-sdk/xtensa-lx106-elf/bin)
SDK_BASE	?= $(abspath ../../esp-open-sdk/sdk)
SDK_LIBDIR = lib
SDK_INCDIR = include

FW_TOOL       = $(abspath ../bin/esptool2)
FW_SECTS      = .text .data .rodata
FW_USER_ARGS  = -quiet -bin -boot2

RBOOT_SRCDIR			= $(abspath ../rboot)
RBOOT_OTA_SRCDIR	= $(abspath ../rboot-ota)

CC		:= $(addprefix $(XTENSA_TOOLS_BINDIR)/,xtensa-lx106-elf-gcc)
LD		:= $(addprefix $(XTENSA_TOOLS_BINDIR)/,xtensa-lx106-elf-gcc)

SDK_LIBDIR	:= $(addprefix $(SDK_BASE)/,$(SDK_LIBDIR))
SDK_INCDIR	:= $(addprefix -I$(SDK_BASE)/,$(SDK_INCDIR))

SRC		:= $(wildcard *.c)
OBJ		:= $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRC))
LIBS		:= $(addprefix -l,$(LIBS))

.SECONDARY:
.PHONY: all extra clean

C_FILES = $(wildcard *.c)
O_FILES = $(patsubst %.c,$(BUILD_DIR)/%.o,$(C_FILES))

all: extra $(BUILD_DIR) $(FIRMW_DIR) $(FIRMW_DIR)/rom0.bin $(FIRMW_DIR)/rom1.bin

extra:
	@cp $(addprefix $(RBOOT_SRCDIR)/,rboot.h) .
	@cp $(addprefix $(RBOOT_OTA_SRCDIR)/,rboot-ota.*) .

$(BUILD_DIR)/%.o: %.c %.h
	@echo "CC $<"
	@$(CC) -I. $(SDK_INCDIR) $(CFLAGS) -o $@ -c $<

$(BUILD_DIR)/%.elf: $(O_FILES)
	@echo "LD $(notdir $@)"
	@$(LD) -L$(SDK_LIBDIR) -T$(notdir $(basename $@)).ld $(LDFLAGS) -Wl,--start-group $(LIBS) $^ -Wl,--end-group -o $@

$(FIRMW_DIR)/%.bin: $(BUILD_DIR)/%.elf
	@echo "FW $(notdir $@)"
	@$(FW_TOOL) $(FW_USER_ARGS) $^ $@ $(FW_SECTS)

$(BUILD_DIR):
	@mkdir -p $@

$(FIRMW_DIR):
	@mkdir -p $@

clean:
	@rm -rf $(BUILD_DIR)
	@rm -rf $(FIRMW_DIR)
