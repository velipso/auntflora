//
// auntflora - Port of Aunt Flora's Mansion to Game Boy Advance
// by Sean Connelly (@velipso), https://sean.fun
// Project Home: https://github.com/velipso/auntflora
// SPDX-License-Identifier: 0BSD
//

#pragma once
#include "common.h"

void gfx_init();
void gfx_setmode(enum gfx_mode mode);
void gfx_showscreen(bool show);
void gfx_showobj(bool show);
void gfx_showbg0(bool show);
void gfx_showbg1(bool show);
void gfx_showbg2(bool show);
void gfx_showbg3(bool show);

static inline void gfx_pset_1f(int x, int y, u16 c) {
  ((u16 *)0x06000000)[(x) + (y) * 240] = (c);
}

// sets two colors at once
static inline void gfx_pset2_obj(int x, int y, u16 c) {
  int tx = x >> 3;
  int ty = y >> 3;
  int sx = x & 7;
  int sy = y & 7;
  u16 *obj = (u16 *)0x06010000;
  obj[((tx + ty * 16) * 64 + sx + sy * 8) >> 1] = c;
}
