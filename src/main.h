//
// auntflora - Port of Aunt Flora's Mansion to Game Boy Advance
// by Sean Connelly (@velipso), https://sean.cm
// Project Home: https://github.com/velipso/auntflora
// SPDX-License-Identifier: 0BSD
//

#pragma once
#include "sys.h"

extern u8 g_map0[64 * 64];
extern u8 g_map1[64 * 64];
extern int g_inputdown;
extern int g_inputhit;
extern struct viewport_st {
  int wx;
  int wy;
} g_viewport;
extern struct world_st {
  u16 *data;
  int width;
  int height;
} g_world;
extern struct markers_st {
  int x;
  int y;
} g_markers[3];
extern int g_playerdir;
extern int g_maskworld;

BINFILE(palette_bin);
BINFILE(font_hd_bin);
BINFILE(tiles_hd_bin);
BINFILE(sprites_hd_bin);
BINFILE(worldbg_bin);
BINFILE(worldlogic_bin);
BINFILE(markers_bin);

void move_screen_to_player();
void copy_world_offset();
void set_player_ani_dir(int dir);
void nextframe();
void gvmain();

static inline u16 world_at(int x, int y) {
  return g_world.data[x + y * g_world.width];
}

static inline void delay_push() {
  for (int i = 0; i < 10; i++)
    nextframe();
}
