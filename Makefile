#
# auntflora - Port of Aunt Flora's Mansion to Game Boy Advance
# by Sean Connelly (@velipso), https://sean.cm
# Project Home: https://github.com/velipso/auntflora
# SPDX-License-Identifier: 0BSD
#

NAME := auntflora

PREFIX  := arm-none-eabi-
CC      := $(PREFIX)gcc
LD      := $(PREFIX)ld
OBJDUMP := $(PREFIX)objdump
OBJCOPY := $(PREFIX)objcopy
MKDIR   := mkdir
RM      := rm -rf
SYS     := sys
SRC     := src
DATA    := data
TGT     := tgt

ELF  := $(TGT)/$(NAME).elf
DUMP := $(TGT)/$(NAME).dump
ROM  := $(TGT)/$(NAME).gba
MAP  := $(TGT)/$(NAME).map

XFORM := $(TGT)/xform/xform

SOURCES_S := $(wildcard $(SRC)/*.s $(SRC)/**/*.s $(SYS)/*.s $(SYS)/gba/*.s $(SYS)/gba/**/*.s)
SOURCES_C := $(wildcard $(SRC)/*.c $(SRC)/**/*.c $(SYS)/*.c $(SYS)/gba/*.c $(SYS)/gba/**/*.c)

DEFINES :=
DEFINES += -D__GBA__

LIBS     := -lc
INCLUDES := $(SYS)

ARCH := -mcpu=arm7tdmi -mtune=arm7tdmi

WARNFLAGS := -Wall

INCLUDEFLAGS := $(foreach path,$(INCLUDES),-I$(path))

ASFLAGS :=
ASFLAGS += \
	-x assembler-with-cpp $(DEFINES) $(ARCH) \
	-mthumb -mthumb-interwork $(INCLUDEFLAGS) \
	-ffunction-sections -fdata-sections

CFLAGS += \
	-std=gnu11 $(WARNFLAGS) $(DEFINES) $(ARCH) \
	-mthumb -mthumb-interwork $(INCLUDEFLAGS) -O3 \
	-ffunction-sections -fdata-sections

LDFLAGS := \
	-mthumb -mthumb-interwork \
	-Wl,-Map,$(MAP) -Wl,--gc-sections \
	-specs=nano.specs -T $(SYS)/gba/link.ld \
	-Wl,--start-group $(LIBS) -Wl,--end-group

OBJS := \
	$(patsubst %.s,$(TGT)/%.s.o,$(SOURCES_S)) \
	$(patsubst %.c,$(TGT)/%.c.o,$(SOURCES_C)) \
	$(TGT)/data/palette.o \
	$(TGT)/data/font_hd.o \
	$(TGT)/data/tiles_hd.o

DEPS := $(OBJS:.o=.d)

# converts a binary file to an object file in the ROM
#   input.bin -> input.o
#     extern const u8 _binary_input_bin_start[];
#     extern const u8 _binary_input_bin_end[];
#     extern const u8 _binary_input_bin_size[];
objbinary = $(OBJCOPY) -I binary -O elf32-littlearm -B arm \
	--rename-section .data=.rodata,alloc,load,readonly,data,contents \
	$1 $2

$(TGT)/%.s.o : %.s
	$(MKDIR) -p $(@D)
	$(CC) $(ASFLAGS) -MMD -MP -c -o $@ $<

$(TGT)/%.c.o : %.c
	$(MKDIR) -p $(@D)
	$(CC) $(CFLAGS) -MMD -MP -c -o $@ $<

.PHONY: all clean dump

all: $(ROM)

$(TGT)/data/palette.bin: $(DATA)/font_hd.png $(DATA)/tiles_hd.png $(XFORM)
	$(MKDIR) -p $(@D)
	$(XFORM) palette256 $(TGT)/data/palette.bin $(DATA)/font_hd.png $(DATA)/tiles_hd.png

$(TGT)/data/palette.o: $(TGT)/data/palette.bin
	cd $(TGT)/data && $(call objbinary,palette.bin,palette.o)

$(TGT)/data/font_hd.o: $(DATA)/font_hd.png $(TGT)/data/palette.bin $(XFORM)
	$(MKDIR) -p $(@D)
	$(XFORM) expand6x6to8x8 $(DATA)/font_hd.png $(TGT)/data/palette.bin $(TGT)/data/font_hd.bin
	cd $(TGT)/data && $(call objbinary,font_hd.bin,font_hd.o)

$(TGT)/data/tiles_hd.o: $(DATA)/tiles_hd.png $(TGT)/data/palette.bin $(XFORM)
	$(MKDIR) -p $(@D)
	$(XFORM) expand6x6to8x8 $(DATA)/tiles_hd.png $(TGT)/data/palette.bin $(TGT)/data/tiles_hd.bin
	cd $(TGT)/data && $(call objbinary,tiles_hd.bin,tiles_hd.o)

$(XFORM):
	cd xform && make

$(ELF): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

$(ROM): $(ELF) $(XFORM)
	$(OBJCOPY) -O binary $< $@
	$(XFORM) fix $@

$(DUMP): $(ELF)
	$(OBJDUMP) -h -C -S $< > $@

dump: $(DUMP)

clean:
	$(RM) $(TGT)

-include $(DEPS)
