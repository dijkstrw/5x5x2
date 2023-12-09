BINARY = 5x5x2
OBJS = 5x5x2.o automouse.o clock.o command.o debug.o elog.o		\
       extrakey.o flash.o keyboard.o keymap.o layer.o led.o light.o	\
       macro.o matrix.o mouse.o map_ascii.o palette.o rgbease.o		\
       rgbpixel.o rgbmap.o ring.o rotary.o serial.o usb.o

OROCHI_VERSION   = $(shell git describe --tags --always)

DEVICE           = stm32f103c8t6
CPPFLAGS        += -MD
CFLAGS           = -DOROCHI_VERSION='"$(OROCHI_VERSION)"' -g -mfix-cortex-m3-ldrd
LDFLAGS         += -static -nostartfiles
LDLIBS          += -Wl,--start-group -lc -lgcc -lnosys -Wl,--end-group
OPENCM3_DIR      = libopencm3

STLINK_PORT      = 4242
BMP_HOST         = blackmagic.lan
BMP_PORT         = 2022

include $(OPENCM3_DIR)/mk/genlink-config.mk
LDSCRIPT         = stm32f103c8t6.ld
include $(OPENCM3_DIR)/mk/gcc-config.mk

GDB              = arm-none-eabi-gdb-py

all: $(BINARY).elf $(BINARY).bin

.PHONY: all clean

clean:
	$(Q)$(RM) -rf $(BINARY).elf $(BINARY).bin $(BINARY).list $(BINARY).map *.o *.d generated.*

size: $(BINARY).elf
	$(Q)./checksize $(LDSCRIPT) $(BINARY).elf

flash: $(BINARY).bin
	st-flash write $(BINARY).bin 0x8000000

.gdb_config:
	echo > .gdb_config "file $(BINARY).elf\ntarget extended-remote $(BMP_HOST):$(BMP_PORT)\nmonitor version\nmonitor swdp_scan\nattach 1\nbreak main\nset mem inaccessible-by-default off\nsource gdb-regview/gdb-regview.py\nregview load gdb-regview/defs/STM32F10X_CL.xml\n"

debug: .gdb_config $(BINARY).elf
	$(GDB) --command=.gdb_config

include $(OPENCM3_DIR)/mk/genlink-rules.mk
include $(OPENCM3_DIR)/mk/gcc-rules.mk
