# keg20 sensor bridge firmware - TI MSP430-GCC
#   make                       build for MSP430FR2100 (896 B code)
#   make MCU=msp430fr2111      pin compatible, 3.75 KB FRAM (FR2000: 384 B, too small)
#   make clean

MCU     ?= msp430fr2100
GCCDIR  ?= /usr/lib/gcc/msp430
SUPPORT ?= $(GCCDIR)/include
PREFIX  := $(GCCDIR)/bin/msp430-elf-

CC      := $(PREFIX)gcc
OBJCOPY := $(PREFIX)objcopy
OBJDUMP := $(PREFIX)objdump
SIZE    := $(PREFIX)size

BUILD   := build
TARGET  := $(BUILD)/sensor

CFLAGS  := -mmcu=$(MCU) -I$(SUPPORT) -Os -std=gnu11 -Wall -Wextra \
           -ffunction-sections -fdata-sections
LDFLAGS := -L$(SUPPORT) -Wl,--gc-sections -Wl,-Map=$(TARGET).map

all: $(TARGET).elf $(TARGET).hex $(TARGET).bin $(TARGET).lst
	@$(SIZE) $(TARGET).elf

$(BUILD):
	mkdir -p $@

$(TARGET).elf: main.c Makefile | $(BUILD)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ main.c

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex $< $@

# FRAM image incl. vectors (0xFC00 - 0xFFFF on FR2100)
$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary --gap-fill 0xFF $< $@

$(TARGET).lst: $(TARGET).elf
	$(OBJDUMP) -d -S $< > $@

clean:
	rm -rf $(BUILD)

.PHONY: all clean
