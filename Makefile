#
# auntflora - Port of Aunt Flora's Mansion to Game Boy Advance
# by Sean Connelly (@velipso), https://sean.cm
# Project Home: https://github.com/velipso/auntflora
# SPDX-License-Identifier: 0BSD
#

NAME := auntflora

PREFIX   := arm-none-eabi-
CC       := $(PREFIX)gcc
LD       := $(PREFIX)ld
OBJDUMP  := $(PREFIX)objdump
OBJCOPY  := $(PREFIX)objcopy
MKDIR    := mkdir
RM       := rm -rf
SYS      := sys
SRC      := src
DATA     := data
SND      := $(DATA)/snd
TGT      := tgt
TGT_DATA := $(TGT)/$(DATA)
TGT_SND  := $(TGT)/$(SND)

ELF  := $(TGT)/$(NAME).elf
DUMP := $(TGT)/$(NAME).dump
ROM  := $(TGT)/$(NAME).gba
MAP  := $(TGT)/$(NAME).map

XFORM := $(TGT)/xform/xform

SOURCES_S := $(wildcard $(SRC)/*.s $(SRC)/**/*.s $(SYS)/*.s $(SYS)/gba/*.s $(SYS)/gba/**/*.s)
SOURCES_C := $(wildcard $(SRC)/*.c $(SRC)/**/*.c $(SYS)/*.c $(SYS)/gba/*.c $(SYS)/gba/**/*.c)
SOURCES_WAV := $(wildcard $(SND)/*.wav)

DEFINES := -DSYS_GBA
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
	$(TGT_DATA)/palette.o \
	$(TGT_DATA)/font_hd.o \
	$(TGT_DATA)/tiles_hd.o \
	$(TGT_DATA)/tiles_sd.o \
	$(TGT_DATA)/sprites_hd.o \
	$(TGT_DATA)/sprites_sd.o \
	$(TGT_DATA)/worldbg.o \
	$(TGT_DATA)/worldlogic.o \
	$(TGT_DATA)/markers.o \
	$(TGT_DATA)/song1.o \
	$(TGT_SND)/snd_osc.o \
	$(TGT_SND)/snd_tempo.o \
	$(TGT_SND)/snd_slice.o \
	$(TGT_SND)/snd_dphase.o \
	$(TGT_SND)/snd_bend.o \
	$(TGT_SND)/snd_wavs.o \
	$(TGT_SND)/snd_offsets.o \
	$(TGT_SND)/snd_sizes.o \
	$(TGT_SND)/snd_names.o

DEPS := $(OBJS:.o=.d)

# verifies binary files are divisible by 4 -- apparently the linker script just ignores alignment
# for binary blobs!??
verifyfilealign = \
	filesize=$$(wc -c $(1) | awk '{print $$1}'); \
	if [ $$((filesize % 4)) -ne 0 ]; then \
		echo "Error: File size of $(1) is not divisible by 4"; \
		exit 1; \
	fi

# converts a binary file to an object file in the ROM
#   input.bin -> input.o
#     extern const u8 _binary_input_bin_start[];
#     extern const u8 _binary_input_bin_end[];
#     extern const u8 _binary_input_bin_size[];
objbinary = $(call verifyfilealign,$1) ;\
	$(OBJCOPY) -I binary -O elf32-littlearm -B arm \
	--rename-section .data=.rodata,alloc,load,readonly,data,contents \
	$1 $2

$(TGT)/%.s.o: %.s
	$(MKDIR) -p $(@D)
	$(CC) $(ASFLAGS) -I$(dir $<) -MMD -MP -c -o $@ $<

$(TGT)/%.c.o: %.c
	$(MKDIR) -p $(@D)
	$(CC) $(CFLAGS) -MMD -MP -c -o $@ $<

.PHONY: all clean dump

all: $(ROM)

$(TGT_DATA)/palette.bin: \
	$(DATA)/font_hd.png \
	$(DATA)/tiles_sd.png \
	$(DATA)/tiles_hd.png \
	$(DATA)/sprites_hd.png \
	$(DATA)/sprites_sd.png \
	$(XFORM)
	$(MKDIR) -p $(@D)
	$(XFORM) palette256 $(TGT_DATA)/palette.bin \
		$(DATA)/font_hd.png $(DATA)/tiles_hd.png $(DATA)/tiles_sd.png \
		$(DATA)/sprites_hd.png $(DATA)/sprites_sd.png

$(TGT_DATA)/palette.o: $(TGT_DATA)/palette.bin
	cd $(TGT_DATA) && $(call objbinary,palette.bin,palette.o)

$(TGT_DATA)/font_hd.o: $(DATA)/font_hd.png $(TGT_DATA)/palette.bin $(XFORM)
	$(MKDIR) -p $(@D)
	$(XFORM) expand6x6to8x8 $(DATA)/font_hd.png $(TGT_DATA)/palette.bin $(TGT_DATA)/font_hd.bin
	cd $(TGT_DATA) && $(call objbinary,font_hd.bin,font_hd.o)

$(TGT_DATA)/tiles_hd.o: $(DATA)/tiles_hd.png $(TGT_DATA)/palette.bin $(XFORM)
	$(MKDIR) -p $(@D)
	$(XFORM) expand6x6to8x8 $(DATA)/tiles_hd.png $(TGT_DATA)/palette.bin $(TGT_DATA)/tiles_hd.bin
	cd $(TGT_DATA) && $(call objbinary,tiles_hd.bin,tiles_hd.o)

$(TGT_DATA)/tiles_sd.o: $(DATA)/tiles_sd.png $(TGT_DATA)/palette.bin $(XFORM)
	$(MKDIR) -p $(@D)
	$(XFORM) expand5x5to8x8 $(DATA)/tiles_sd.png $(TGT_DATA)/palette.bin $(TGT_DATA)/tiles_sd.bin
	cd $(TGT_DATA) && $(call objbinary,tiles_sd.bin,tiles_sd.o)

$(TGT_DATA)/sprites_hd.o: $(DATA)/sprites_hd.png $(TGT_DATA)/palette.bin $(XFORM)
	$(MKDIR) -p $(@D)
	$(XFORM) copy8x8 $(DATA)/sprites_hd.png $(TGT_DATA)/palette.bin $(TGT_DATA)/sprites_hd.bin
	cd $(TGT_DATA) && $(call objbinary,sprites_hd.bin,sprites_hd.o)

$(TGT_DATA)/sprites_sd.o: $(DATA)/sprites_sd.png $(TGT_DATA)/palette.bin $(XFORM)
	$(MKDIR) -p $(@D)
	$(XFORM) copy8x8 $(DATA)/sprites_sd.png $(TGT_DATA)/palette.bin $(TGT_DATA)/sprites_sd.bin
	cd $(TGT_DATA) && $(call objbinary,sprites_sd.bin,sprites_sd.o)

$(TGT_DATA)/worldbg.o \
$(TGT_DATA)/worldlogic.o \
$(TGT_DATA)/markers.o: $(DATA)/world.json $(XFORM)
	$(MKDIR) -p $(@D)
	$(XFORM) world $(DATA)/world.json \
		$(TGT_DATA)/worldbg.bin $(TGT_DATA)/worldlogic.bin $(TGT_DATA)/markers.bin
	cd $(TGT_DATA) && $(call objbinary,worldbg.bin,worldbg.o)
	cd $(TGT_DATA) && $(call objbinary,worldlogic.bin,worldlogic.o)
	cd $(TGT_DATA) && $(call objbinary,markers.bin,markers.o)

$(TGT_SND)/snd_osc.o \
$(TGT_SND)/snd_tempo.o \
$(TGT_SND)/snd_slice.o \
$(TGT_SND)/snd_dphase.o \
$(TGT_SND)/snd_bend.o: $(XFORM)
	$(MKDIR) -p $(@D)
	$(XFORM) snd tables \
		$(TGT_SND)/snd_osc.bin \
		$(TGT_SND)/snd_tempo.bin \
		$(TGT_SND)/snd_slice.bin \
		$(TGT_SND)/snd_dphase.bin \
		$(TGT_SND)/snd_bend.bin
	cd $(TGT_SND) && $(call objbinary,snd_osc.bin,snd_osc.o)
	cd $(TGT_SND) && $(call objbinary,snd_tempo.bin,snd_tempo.o)
	cd $(TGT_SND) && $(call objbinary,snd_slice.bin,snd_slice.o)
	cd $(TGT_SND) && $(call objbinary,snd_dphase.bin,snd_dphase.o)
	cd $(TGT_SND) && $(call objbinary,snd_bend.bin,snd_bend.o)

$(TGT_SND)/snd_wavs.o \
$(TGT_SND)/snd_offsets.o \
$(TGT_SND)/snd_sizes.o \
$(TGT_SND)/snd_names.o \
$(TGT_SND)/snd_names.txt: $(SOURCES_WAV) $(XFORM)
	$(MKDIR) -p $(@D)
	$(XFORM) snd wav $(SND) \
		$(TGT_SND)/snd_wavs.bin \
		$(TGT_SND)/snd_offsets.bin \
		$(TGT_SND)/snd_sizes.bin \
		$(TGT_SND)/snd_names.txt
	cd $(TGT_SND) && $(call objbinary,snd_wavs.bin,snd_wavs.o)
	cd $(TGT_SND) && $(call objbinary,snd_offsets.bin,snd_offsets.o)
	cd $(TGT_SND) && $(call objbinary,snd_sizes.bin,snd_sizes.o)
	cd $(TGT_SND) && $(call objbinary,snd_names.txt,snd_names.o)

$(TGT_DATA)/song1.o: $(DATA)/song1.txt $(TGT_SND)/snd_names.txt $(XFORM)
	$(MKDIR) -p $(@D)
	$(XFORM) snd makesong $(DATA)/song1.txt $(TGT_SND)/snd_names.txt $(TGT_DATA)/song1.gvsong
	cd $(TGT_DATA) && $(call objbinary,song1.gvsong,song1.o)

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
