//
// auntflora - Port of Aunt Flora's Mansion to Game Boy Advance
// by Sean Connelly (@velipso), https://sean.cm
// Project Home: https://github.com/velipso/auntflora
// SPDX-License-Identifier: 0BSD
//

#pragma once
#include "sys.h"

extern struct { int x; int y; int dir; } g_pushers[32];
extern int g_pushers_size;

static inline void pushers_reset() {
  g_pushers_size = 0;
}

void pushers_find(int x, int y, int dir);
void pushers_find_around_player(int movedir);
void pushers_apply();
