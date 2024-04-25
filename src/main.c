//
// auntflora - Port of Aunt Flora's Mansion to Game Boy Advance
// by Sean Connelly (@velipso), https://sean.cm
// Project Home: https://github.com/velipso/auntflora
// SPDX-License-Identifier: 0BSD
//

#include "main.h"
#include "pushers.h"
#include <stdlib.h>
#include "ani.h"
#include "anidata.h"
#include "util.h"
#include "cellinfo.h"
#include "sfx.h"
#include "undo.h"

u8 g_map0[64 * 64] = {0};
u8 g_map1[64 * 64] = {0};
int g_inputdown = 0;
int g_inputhit = 0;
struct viewport_st g_viewport = {0};
struct world_st g_world = {0};
struct markers_st g_markers[3] = {0};
int g_playerdir = 3;
int g_maskworld = 0;

static void SECTION_IWRAM_ARM irq_vblank_6x6() {
  sys_copy_map(28, 0, g_map0, 64 * 64);
  sys_copy_map(30, 0, g_map1, 64 * 64);
  sys_copy_oam(g_oam);
  int inp = ~sys_input();
  g_inputhit = ~g_inputdown & inp;
  g_inputdown = inp;
}

static void settile0(u32 x, u32 y, u32 t) {
  u32 k = (x * 2) + (y * 64 * 2);
  if (t == 0) {
    g_map0[k + 0] = 0;
    g_map0[k + 1] = 0;
    g_map0[k + 64] = 0;
    g_map0[k + 65] = 0;
  } else {
    u32 tx = t & 0xf;
    u32 ty = t >> 4;
    u32 tk = (tx * 2) + (ty * 32 * 2);
    g_map0[k + 0] = tk;
    g_map0[k + 1] = tk + 1;
    g_map0[k + 64] = tk + 32;
    g_map0[k + 65] = tk + 33;
  }
}

void set_player_ani_dir(int dir) {
  g_playerdir = dir;
  switch (dir) {
    case 0: g_sprites[0].pc = ani_player_u; break;
    case 1: g_sprites[0].pc = ani_player_r; break;
    case 2: g_sprites[0].pc = ani_player_d; break;
    case 3: g_sprites[0].pc = ani_player_l; break;
  }
}

static struct viewport_st find_player_level() {
  int wx = 0;
  int wy = 0;
  int x = g_markers[0].x - 21;
  int y = g_markers[0].y - 14;
  while (x >= wx) wx += 17;
  while (y >= wy) wy += 12;
  return (struct viewport_st){
    .wx = wx,
    .wy = wy
  };
}

static void snap_player() {
  g_sprites[0].origin.x = (g_markers[0].x - g_viewport.wx) * 12 - 32;
  g_sprites[0].origin.y = (g_markers[0].y - g_viewport.wy) * 12 - 18;
  g_sprites[1].origin.x = (g_markers[1].x - g_viewport.wx) * 12 - 32;
  g_sprites[1].origin.y = (g_markers[1].y - g_viewport.wy) * 12 - 18;
  g_sprites[2].origin.x = (g_markers[2].x - g_viewport.wx) * 12 - 32;
  g_sprites[2].origin.y = (g_markers[2].y - g_viewport.wy) * 12 - 18;
}

void copy_world_offset() {
  for (int y = 0; y < 16; y++) {
    int sy = g_viewport.wy + y;
    for (int x = 0; x < 25; x++) {
      int sx = g_viewport.wx + x;
      int ww = 0;
      if (sx >= 0 && sx < g_world.width && sy >= 0 && sy < g_world.height) {
        ww = world_at(sx, sy);
      }
      if (g_maskworld && (x < 4 || x >= 21 || y < 2 || y >= 14))
        ww = 0;
      settile0(x, y, ww & 0xff);
    }
  }

  const u8 *bg = BINADDR(worldbg_bin);
  for (int y = 1; y < 30; y++) {
    memcpy8(
      &g_map1[y * 64],
      &bg[g_viewport.wx * 2 + (g_viewport.wy * 2 + y) * g_world.width * 2],
      45
    );
  }
  if (g_maskworld) {
    // clear border
    for (int y = 1; y < 4; y++) {
      memset8(&g_map1[y * 64], 0, 45);
    }
    for (int y = 4; y < 28; y++) {
      for (int i = 0; i < 8; i++) {
        g_map1[y * 64 + i] = 0;
        g_map1[y * 64 + 42 + i] = 0;
      }
    }
    for (int y = 28; y < 30; y++) {
      memset8(&g_map1[y * 64], 0, 45);
    }
  }
}

static void load_world() {
  free(g_world.data);
  const u16 *world = BINADDR(worldlogic_bin);
  g_world.width = world[0];
  g_world.height = world[1];
  int size = g_world.width * g_world.height * 2;
  g_world.data = malloc(size);
  memcpy32(g_world.data, &world[2], size);

  // load markers
  const u16 *markers = BINADDR(markers_bin);
  int i = 0;
  while (1) {
    int x = markers[i * 2];
    int y = markers[i * 2 + 1];
    if (x == 0xffff)
      break;
    g_markers[i].x = x;
    g_markers[i].y = y;
    i++;
  }

  // initialize undo
  g_undo.head = 1;
  g_undo.tail = 0;
  g_undo.entries[0] = 0x80000000;
}

void nextframe() {
  for (i32 i = 0; i < 128; i++)
    ani_step(i);
  sys_nextframe();
}

void move_screen_to_player() {
  // move screen based on player location
  struct viewport_st vp2 = find_player_level();
  int vdx = vp2.wx > g_viewport.wx ? 1 : vp2.wx < g_viewport.wx ? -1 : 0;
  int vdy = vp2.wy > g_viewport.wy ? 1 : vp2.wy < g_viewport.wy ? -1 : 0;
  if (vdx != 0 || vdy != 0) {
    if (g_maskworld) {
      // snap to new location
      g_viewport = vp2;
    } else {
      // scroll to new location
      while (vp2.wx != g_viewport.wx || vp2.wy != g_viewport.wy) {
        g_viewport.wx += vdx;
        g_viewport.wy += vdy;
        copy_world_offset();
        snap_player();
        nextframe();
      }
    }
  }
  snap_player();
}

void gvmain() {
  sys_init();
  sys_set_vblank(irq_vblank_6x6);
  sys_set_screen_mode(SYS_SCREEN_MODE_2S6X6);
  sys_set_bg_config(
    2, // background #
    0, // priority
    0, // tile start
    0, // mosaic
    1, // 256 colors
    28, // map start
    0, // wrap
    SYS_BGS_SIZE_512X512
  );
  sys_set_bg_config(
    3, // background #
    0, // priority
    0, // tile start
    0, // mosaic
    1, // 256 colors
    30, // map start
    0, // wrap
    SYS_BGS_SIZE_512X512
  );
  sys_copy_bgpal(0, BINADDR(palette_bin), BINSIZE(palette_bin));
  sys_copy_spritepal(0, BINADDR(palette_bin), BINSIZE(palette_bin));
  sys_copy_tiles(0, 0, BINADDR(tiles_hd_bin), BINSIZE(tiles_hd_bin));
  sys_copy_tiles(4, 0, BINADDR(sprites_hd_bin), BINSIZE(sprites_hd_bin));
  sys_copy_tiles(1, 0, BINADDR(font_hd_bin), BINSIZE(font_hd_bin));

  // ensure that tile 0 is fully transparent
  const u8 zero[64] = {0};
  sys_copy_tiles(0, 0, zero, sizeof(zero));

  sys_set_bgs2_scroll(0x0156 * 30 - 12, 0x0156 * 16);
  sys_set_bgs3_scroll(0x0156 * 30 - 12, 0x0156 * 16);
  sys_nextframe();
  sys_set_screen_enable(1);

  load_world();

  g_sprites[0].pc = ani_player_l;
  g_sprites[1].pc = ani_aunt;
  g_sprites[2].pc = ani_cat;

  g_viewport = find_player_level();
  snap_player();

  snd_set_master_volume(16);
  snd_set_song_volume(16);
  snd_load_song(BINADDR(song1_gvsong), 0);

  while (1) {
    nextframe();

    if (g_inputhit & SYS_INPUT_B) {
      // undo
      undo_fire();
      g_viewport = find_player_level();
      snap_player();
    } else {
      int dir = -1;
      g_dirty = 0;
      if (g_inputhit & SYS_INPUT_SE) g_maskworld = 1 - g_maskworld;
      if (g_inputhit & SYS_INPUT_U){ dir = 0; }
      if (g_inputhit & SYS_INPUT_R){ dir = 1; }
      if (g_inputhit & SYS_INPUT_D){ dir = 2; }
      if (g_inputhit & SYS_INPUT_L){ dir = 3; }
      if (dir >= 0 && dir < 4) {
        int nx = g_markers[0].x;
        int ny = g_markers[0].y;
        advance_pt(&nx, &ny, dir, 1);
        u32 cell = world_at(nx, ny);
        if (!is_solid_static(cell)) {
          pushers_reset();
          if (is_empty_for_player(cell)) {
            sfx_walk();
            pushers_find_around_player(dir);
            write_player(nx, ny, dir, -1);
          } else if (is_pushable_by_player(cell, dir)) {
            int px = nx;
            int py = ny;
            while (1) {
              u16 nc = world_at(px, py);
              if (is_empty_for_block(nc)) {
                int hx, hy;
                while (px != nx || py != ny) {
                  hx = px;
                  hy = py;
                  advance_pt(&px, &py, dir, -1);
                  write_logic(hx, hy, world_at(px, py));
                }
                write_logic(nx, ny, 0);
                sfx_push();
                pushers_find_around_player(dir);
                write_player(nx, ny, dir, -1);
                break;
              } else if (is_pushable_by_block(nc, dir)) {
                advance_pt(&px, &py, dir, 1);
                // find pushers in perpendicular directions
                pushers_find(px, py, (dir + 1) & 3);
                pushers_find(px, py, (dir + 3) & 3);
                continue;
              } else {
                break;
              }
            }
          } else if (is_forward(cell, dir)) {
            advance_pt(&nx, &ny, dir, 1);
            if (is_empty_for_player(world_at(nx, ny))) {
              sfx_forward();
              pushers_find_around_player(dir);
              write_player(nx, ny, dir, -1);
            }
          }

          pushers_apply();
        }

        if (g_dirty) {
          undo_finish();
          move_screen_to_player();
        } else {
          sfx_bump();
          set_player_ani_dir(dir);
        }
      }
    }

    copy_world_offset();
  }
}
