#
# auntflora - Port of Aunt Flora's Mansion to Gameboy Advance
# by Sean Connelly (@velipso), https://sean.cm
# Project Home: https://github.com/velipso/auntflora
# SPDX-License-Identifier: 0BSD
#

NAME := auntflora

PREFIX  := arm-none-eabi-
CC      := $(PREFIX)gcc
OBJDUMP := $(PREFIX)objdump
OBJCOPY := $(PREFIX)objcopy
MKDIR   := mkdir
RM      := rm -rf
SRC     := src
TGT     := tgt

ELF  := $(TGT)/$(NAME).elf
DUMP := $(TGT)/$(NAME).dump
ROM  := $(TGT)/$(NAME).gba
MAP  := $(TGT)/$(NAME).map

XFORM := $(TGT)/xform/xform

SOURCES_S := $(wildcard $(SRC)/*.s $(SRC)/**/*.s)
SOURCES_C := $(wildcard $(SRC)/*.c $(SRC)/**/*.c)

DEFINES :=
DEFINES += -D__GBA__

ARCH := -mcpu=arm7tdmi -mtune=arm7tdmi

WARNFLAGS := -Wall

ASFLAGS :=
ASFLAGS += -x assembler-with-cpp $(DEFINES) $(ARCH) \
		   -mthumb -mthumb-interwork \
		   -ffunction-sections -fdata-sections

CFLAGS += -std=gnu11 $(WARNFLAGS) $(DEFINES) $(ARCH) \
		   -mthumb -mthumb-interwork -O3 \
		   -ffunction-sections -fdata-sections

LDFLAGS := -mthumb -mthumb-interwork \
		   -Wl,-Map,$(MAP) -Wl,--gc-sections \
		   -specs=nano.specs -T $(SRC)/sys/gba_cart.ld \
		   -Wl,--start-group -Wl,--end-group

OBJS := \
	$(patsubst $(SRC)/%.s,$(TGT)/%.s.o,$(SOURCES_S)) \
	$(patsubst $(SRC)/%.c,$(TGT)/%.c.o,$(SOURCES_C))

DEPS := $(OBJS:.o=.d)

$(TGT)/%.s.o : $(SRC)/%.s
	$(MKDIR) -p $(@D)
	$(CC) $(ASFLAGS) -MMD -MP -c -o $@ $<

$(TGT)/%.c.o : $(SRC)/%.c
	$(MKDIR) -p $(@D)
	$(CC) $(CFLAGS) -MMD -MP -c -o $@ $<

.PHONY: all clean dump

all: $(ROM)

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
