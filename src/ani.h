//
// auntflora - Port of Aunt Flora's Mansion to Game Boy Advance
// by Sean Connelly (@velipso), https://sean.cm
// Project Home: https://github.com/velipso/auntflora
// SPDX-License-Identifier: 0BSD
//

#pragma once
#include "sys.h"

typedef struct {
  const u16 *pc;
  u8 waitcount;
  u8 loopcount;
  i16 gravity; // Q8.8
  struct { // Q16.0
    i16 x;
    i16 y;
  } origin;
  struct { // Q8.8
    i16 x;
    i16 y;
    i16 dx;
    i16 dy;
  } offset;
} sprite_st;

extern u16 g_oam[0x200];
extern sprite_st g_sprites[128];

void ani_flushxy(u32 i);
void ani_step(u32 i);
