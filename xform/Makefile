#
# auntflora - Port of Aunt Flora's Mansion to Game Boy Advance
# by Sean Connelly (@velipso), https://sean.cm
# Project Home: https://github.com/velipso/auntflora
# SPDX-License-Identifier: 0BSD
#

NAME      := xform
SRC       := .
TGT       := ../tgt/xform
CC        := gcc
MKDIR     := mkdir
RM        := rm -rf
CFLAGS    := -Wall -O3
SOURCES_C := $(wildcard $(SRC)/*.c $(SRC)/**/*.c)
LDFLAGS   := -lm
OBJS      := $(patsubst $(SRC)/%.c,$(TGT)/%.c.o,$(SOURCES_C))
DEPS      := $(OBJS:.o=.d)

$(TGT)/%.c.o : $(SRC)/%.c
	$(MKDIR) -p $(@D)
	$(CC) $(CFLAGS) -MMD -MP -c -o $@ $<

.PHONY: all clean

all: $(TGT)/$(NAME)

$(TGT)/$(NAME): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

clean:
	$(RM) $(TGT)/$(NAME)

-include $(DEPS)
