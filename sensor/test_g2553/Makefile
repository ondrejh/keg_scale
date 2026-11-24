# BSL entry bench test, MSP430G2553
#   make            build/v1.hex (slow blink) and build/v2.hex (fast blink)

MCU     ?= msp430g2553
GCCDIR  ?= /usr/lib/gcc/msp430
SUPPORT ?= $(GCCDIR)/include
PREFIX  := $(GCCDIR)/bin/msp430-elf-

CC      := $(PREFIX)gcc
OBJCOPY := $(PREFIX)objcopy
OBJDUMP := $(PREFIX)objdump
SIZE    := $(PREFIX)size

CFLAGS  := -mmcu=$(MCU) -I$(SUPPORT) -Os -std=gnu11 -Wall -Wextra \
           -ffunction-sections -fdata-sections
LDFLAGS := -L$(SUPPORT) -Wl,--gc-sections -Wl,-u,bslskey

all: build/v1.hex build/v2.hex
	@$(SIZE) build/v1.elf build/v2.elf

build:
	mkdir -p $@

build/v1.elf: DEFS := -DFW_VERSION=1 -DBLINK=50
build/v2.elf: DEFS := -DFW_VERSION=2 -DBLINK=10

build/%.elf: main.c Makefile | build
	$(CC) $(CFLAGS) $(DEFS) $(LDFLAGS) -o $@ main.c
	$(OBJDUMP) -d -S $@ > build/$*.lst

build/%.hex: build/%.elf
	$(OBJCOPY) -O ihex $< $@

clean:
	rm -rf build

.PRECIOUS: build/%.elf
.PHONY: all clean
