//
// auntflora - Port of Aunt Flora's Mansion to Game Boy Advance
// by Sean Connelly (@velipso), https://sean.cm
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
