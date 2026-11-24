# BSL entry bench test, MSP430FR2311 LaunchPad
#   make            build/v1.hex (slow blink), build/v2.hex (fast blink)
#                   build/ow.hex (version 3, one wire half duplex, blink in between)

MCU     ?= msp430fr2311
GCCDIR  ?= /usr/lib/gcc/msp430
SUPPORT ?= $(GCCDIR)/include
PREFIX  := $(GCCDIR)/bin/msp430-elf-

CC      := $(PREFIX)gcc
OBJCOPY := $(PREFIX)objcopy
OBJDUMP := $(PREFIX)objdump
SIZE    := $(PREFIX)size

CFLAGS  := -mmcu=$(MCU) -I$(SUPPORT) -Os -std=gnu11 -Wall -Wextra \
           -ffunction-sections -fdata-sections
LDFLAGS := -L$(SUPPORT) -Wl,--gc-sections

all: build/v1.hex build/v2.hex build/ow.hex
	@$(SIZE) build/v1.elf build/v2.elf build/ow.elf

build:
	mkdir -p $@

build/v1.elf: DEFS := -DFW_VERSION=1 -DBLINK=50
build/v2.elf: DEFS := -DFW_VERSION=2 -DBLINK=10
build/ow.elf: DEFS := -DFW_VERSION=3 -DBLINK=25 -DONEWIRE=1

build/%.elf: main.c Makefile | build
	$(CC) $(CFLAGS) $(DEFS) $(LDFLAGS) -o $@ main.c
	$(OBJDUMP) -d -S $@ > build/$*.lst

build/%.hex: build/%.elf
	$(OBJCOPY) -O ihex $< $@

clean:
	rm -rf build

.PRECIOUS: build/%.elf
.PHONY: all clean
