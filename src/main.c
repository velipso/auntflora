//
// auntflora - Port of Aunt Flora's Mansion to Game Boy Advance
// by Sean Connelly (@velipso), https://sean.cm
// Project Home: https://github.com/velipso/auntflora
// SPDX-License-Identifier: 0BSD
//

#include "sys.h"
#include <stdint.h>
#include <stdlib.h>
#include "ani.h"
#include "anidata.h"

u16 pcolor SECTION_EWRAM = RGB15(31, 31, 31);
static inline void pset(u32 x, u32 y) {
  sys_pset_1f(x, y, pcolor);
}

static void print_hex(u32 x, u32 y, u32 ch) {
  x *= 4;
  y *= 6;
  switch (ch) {
    case 0:
      pset(x + 0, y + 0);  pset(x + 1, y + 0);  pset(x + 2, y + 0);
      pset(x + 0, y + 1);/*pset(x + 1, y + 1);*/pset(x + 2, y + 1);
      pset(x + 0, y + 2);/*pset(x + 1, y + 2);*/pset(x + 2, y + 2);
      pset(x + 0, y + 3);/*pset(x + 1, y + 3);*/pset(x + 2, y + 3);
      pset(x + 0, y + 4);  pset(x + 1, y + 4);  pset(x + 2, y + 4);
      break;
    case 1:
      pset(x + 0, y + 0);  pset(x + 1, y + 0);/*pset(x + 2, y + 0);*/
    /*pset(x + 0, y + 1);*/pset(x + 1, y + 1);/*pset(x + 2, y + 1);*/
    /*pset(x + 0, y + 2);*/pset(x + 1, y + 2);/*pset(x + 2, y + 2);*/
    /*pset(x + 0, y + 3);*/pset(x + 1, y + 3);/*pset(x + 2, y + 3);*/
      pset(x + 0, y + 4);  pset(x + 1, y + 4);  pset(x + 2, y + 4);
      break;
    case 2:
      pset(x + 0, y + 0);  pset(x + 1, y + 0);  pset(x + 2, y + 0);
    /*pset(x + 0, y + 1);  pset(x + 1, y + 1);*/pset(x + 2, y + 1);
      pset(x + 0, y + 2);  pset(x + 1, y + 2);  pset(x + 2, y + 2);
      pset(x + 0, y + 3);/*pset(x + 1, y + 3);  pset(x + 2, y + 3);*/
      pset(x + 0, y + 4);  pset(x + 1, y + 4);  pset(x + 2, y + 4);
      break;
    case 3:
      pset(x + 0, y + 0);  pset(x + 1, y + 0);  pset(x + 2, y + 0);
    /*pset(x + 0, y + 1);  pset(x + 1, y + 1);*/pset(x + 2, y + 1);
      pset(x + 0, y + 2);  pset(x + 1, y + 2);  pset(x + 2, y + 2);
    /*pset(x + 0, y + 3);  pset(x + 1, y + 3);*/pset(x + 2, y + 3);
      pset(x + 0, y + 4);  pset(x + 1, y + 4);  pset(x + 2, y + 4);
      break;
    case 4:
      pset(x + 0, y + 0);/*pset(x + 1, y + 0);*/pset(x + 2, y + 0);
      pset(x + 0, y + 1);/*pset(x + 1, y + 1);*/pset(x + 2, y + 1);
      pset(x + 0, y + 2);  pset(x + 1, y + 2);  pset(x + 2, y + 2);
    /*pset(x + 0, y + 3);  pset(x + 1, y + 3);*/pset(x + 2, y + 3);
    /*pset(x + 0, y + 4);  pset(x + 1, y + 4);*/pset(x + 2, y + 4);
      break;
    case 5:
      pset(x + 0, y + 0);  pset(x + 1, y + 0);  pset(x + 2, y + 0);
      pset(x + 0, y + 1);/*pset(x + 1, y + 1);  pset(x + 2, y + 1);*/
      pset(x + 0, y + 2);  pset(x + 1, y + 2);  pset(x + 2, y + 2);
    /*pset(x + 0, y + 3);  pset(x + 1, y + 3);*/pset(x + 2, y + 3);
      pset(x + 0, y + 4);  pset(x + 1, y + 4);  pset(x + 2, y + 4);
      break;
    case 6:
      pset(x + 0, y + 0);  pset(x + 1, y + 0);  pset(x + 2, y + 0);
      pset(x + 0, y + 1);/*pset(x + 1, y + 1);  pset(x + 2, y + 1);*/
      pset(x + 0, y + 2);  pset(x + 1, y + 2);  pset(x + 2, y + 2);
      pset(x + 0, y + 3);/*pset(x + 1, y + 3);*/pset(x + 2, y + 3);
      pset(x + 0, y + 4);  pset(x + 1, y + 4);  pset(x + 2, y + 4);
      break;
    case 7:
      pset(x + 0, y + 0);  pset(x + 1, y + 0);  pset(x + 2, y + 0);
    /*pset(x + 0, y + 1);  pset(x + 1, y + 1);*/pset(x + 2, y + 1);
    /*pset(x + 0, y + 2);  pset(x + 1, y + 2);*/pset(x + 2, y + 2);
    /*pset(x + 0, y + 3);  pset(x + 1, y + 3);*/pset(x + 2, y + 3);
    /*pset(x + 0, y + 4);  pset(x + 1, y + 4);*/pset(x + 2, y + 4);
      break;
    case 8:
      pset(x + 0, y + 0);  pset(x + 1, y + 0);  pset(x + 2, y + 0);
      pset(x + 0, y + 1);/*pset(x + 1, y + 1);*/pset(x + 2, y + 1);
      pset(x + 0, y + 2);  pset(x + 1, y + 2);  pset(x + 2, y + 2);
      pset(x + 0, y + 3);/*pset(x + 1, y + 3);*/pset(x + 2, y + 3);
      pset(x + 0, y + 4);  pset(x + 1, y + 4);  pset(x + 2, y + 4);
      break;
    case 9:
      pset(x + 0, y + 0);  pset(x + 1, y + 0);  pset(x + 2, y + 0);
      pset(x + 0, y + 1);/*pset(x + 1, y + 1);*/pset(x + 2, y + 1);
      pset(x + 0, y + 2);  pset(x + 1, y + 2);  pset(x + 2, y + 2);
    /*pset(x + 0, y + 3);  pset(x + 1, y + 3);*/pset(x + 2, y + 3);
      pset(x + 0, y + 4);  pset(x + 1, y + 4);  pset(x + 2, y + 4);
      break;
    case 10:
    /*pset(x + 0, y + 0);*/pset(x + 1, y + 0);/*pset(x + 2, y + 0);*/
      pset(x + 0, y + 1);/*pset(x + 1, y + 1);*/pset(x + 2, y + 1);
      pset(x + 0, y + 2);  pset(x + 1, y + 2);  pset(x + 2, y + 2);
      pset(x + 0, y + 3);/*pset(x + 1, y + 3);*/pset(x + 2, y + 3);
      pset(x + 0, y + 4);/*pset(x + 1, y + 4);*/pset(x + 2, y + 4);
      break;
    case 11:
      pset(x + 0, y + 0);  pset(x + 1, y + 0);/*pset(x + 2, y + 0);*/
      pset(x + 0, y + 1);/*pset(x + 1, y + 1);*/pset(x + 2, y + 1);
      pset(x + 0, y + 2);  pset(x + 1, y + 2);/*pset(x + 2, y + 2);*/
      pset(x + 0, y + 3);/*pset(x + 1, y + 3);*/pset(x + 2, y + 3);
      pset(x + 0, y + 4);  pset(x + 1, y + 4);/*pset(x + 2, y + 4);*/
      break;
    case 12:
    /*pset(x + 0, y + 0);*/pset(x + 1, y + 0);  pset(x + 2, y + 0);
      pset(x + 0, y + 1);/*pset(x + 1, y + 1);  pset(x + 2, y + 1);*/
      pset(x + 0, y + 2);/*pset(x + 1, y + 2);  pset(x + 2, y + 2);*/
      pset(x + 0, y + 3);/*pset(x + 1, y + 3);  pset(x + 2, y + 3);*/
    /*pset(x + 0, y + 4);*/pset(x + 1, y + 4);  pset(x + 2, y + 4);
      break;
    case 13:
      pset(x + 0, y + 0);  pset(x + 1, y + 0);/*pset(x + 2, y + 0);*/
      pset(x + 0, y + 1);/*pset(x + 1, y + 1);*/pset(x + 2, y + 1);
      pset(x + 0, y + 2);/*pset(x + 1, y + 2);*/pset(x + 2, y + 2);
      pset(x + 0, y + 3);/*pset(x + 1, y + 3);*/pset(x + 2, y + 3);
      pset(x + 0, y + 4);  pset(x + 1, y + 4);/*pset(x + 2, y + 4);*/
      break;
    case 14:
      pset(x + 0, y + 0);  pset(x + 1, y + 0);  pset(x + 2, y + 0);
      pset(x + 0, y + 1);/*pset(x + 1, y + 1);  pset(x + 2, y + 1);*/
      pset(x + 0, y + 2);  pset(x + 1, y + 2);  pset(x + 2, y + 2);
      pset(x + 0, y + 3);/*pset(x + 1, y + 3);  pset(x + 2, y + 3);*/
      pset(x + 0, y + 4);  pset(x + 1, y + 4);  pset(x + 2, y + 4);
      break;
    case 15:
      pset(x + 0, y + 0);  pset(x + 1, y + 0);  pset(x + 2, y + 0);
      pset(x + 0, y + 1);/*pset(x + 1, y + 1);  pset(x + 2, y + 1);*/
      pset(x + 0, y + 2);  pset(x + 1, y + 2);  pset(x + 2, y + 2);
      pset(x + 0, y + 3);/*pset(x + 1, y + 3);  pset(x + 2, y + 3);*/
      pset(x + 0, y + 4);/*pset(x + 1, y + 4);  pset(x + 2, y + 4);*/
      break;
    default:
      pset(x + 0, y + 0);  pset(x + 1, y + 0);  pset(x + 2, y + 0);
      pset(x + 0, y + 1);  pset(x + 1, y + 1);  pset(x + 2, y + 1);
      pset(x + 0, y + 2);  pset(x + 1, y + 2);  pset(x + 2, y + 2);
      pset(x + 0, y + 3);  pset(x + 1, y + 3);  pset(x + 2, y + 3);
      pset(x + 0, y + 4);  pset(x + 1, y + 4);  pset(x + 2, y + 4);
      break;
  }
}

static void print_num(u32 x, u32 y, u32 n) {
  print_hex(x + 0, y, (n >> 28) & 0xf);
  print_hex(x + 1, y, (n >> 24) & 0xf);
  print_hex(x + 2, y, (n >> 20) & 0xf);
  print_hex(x + 3, y, (n >> 16) & 0xf);
  print_hex(x + 4, y, (n >> 12) & 0xf);
  print_hex(x + 5, y, (n >>  8) & 0xf);
  print_hex(x + 6, y, (n >>  4) & 0xf);
  print_hex(x + 7, y, (n >>  0) & 0xf);
}

static void print_clr(u32 x, u32 y) {
  print_hex(x + 0, y, 0xff);
  print_hex(x + 1, y, 0xff);
  print_hex(x + 2, y, 0xff);
  print_hex(x + 3, y, 0xff);
  print_hex(x + 4, y, 0xff);
  print_hex(x + 5, y, 0xff);
  print_hex(x + 6, y, 0xff);
  print_hex(x + 7, y, 0xff);
}

static u32 test_i = 0;
static void SECTION_IWRAM_ARM irq_vblank_test() {
  test_i++;
}

BINFILE(palette_bin);
BINFILE(font_hd_bin);
BINFILE(tiles_hd_bin);
BINFILE(sprites_hd_bin);
BINFILE(worldbg_bin);
BINFILE(worldlogic_bin);
BINFILE(markers_bin);

void gvmain_test() {
  sys_init();
  sys_set_vblank(irq_vblank_test);
  sys_set_screen_mode(SYS_SCREEN_MODE_1F);
  sys_set_screen_enable(1);

  pcolor = RGB15(31, 31, 31);
  print_num(0, 1, BINSIZE(palette_bin));

  while (1) {
    pcolor = 0;
    print_clr(0, 0);
    pcolor = RGB15(31, 31, 31);
    print_num(0, 0, test_i);
    for (u32 j = 0; j < 60; j++) {
      sys_nextframe();
    }
  }
}

static u8 map0[64 * 64] = {0};
static u8 map1[64 * 64] = {0};

static void settile0(u32 x, u32 y, u32 t) {
  u32 k = (x * 2) + (y * 64 * 2);
  if (t == 0) {
    map0[k + 0] = 0;
    map0[k + 1] = 0;
    map0[k + 64] = 0;
    map0[k + 65] = 0;
  } else {
    u32 tx = t & 0xf;
    u32 ty = t >> 4;
    u32 tk = (tx * 2) + (ty * 32 * 2);
    map0[k + 0] = tk;
    map0[k + 1] = tk + 1;
    map0[k + 64] = tk + 32;
    map0[k + 65] = tk + 33;
  }
}

static u16 g_inputdown = 0;
static u16 g_inputhit = 0;
static void SECTION_IWRAM_ARM irq_vblank_6x6() {
  sys_copy_map(28, 0, map0, 64 * 64);
  sys_copy_map(30, 0, map1, 64 * 64);
  sys_copy_oam(g_oam);
  int inp = ~sys_input();
  g_inputhit = ~g_inputdown & inp;
  g_inputdown = inp;
}

static struct {
  u16 *data;
  u32 width;
  u32 height;
} g_world = {0};
static struct {
  u32 x;
  u32 y;
} g_markers[3] = {0};

struct find_player_level_st { int wx; int wy; };
static struct find_player_level_st find_player_level() {
  int wx = 0;
  int wy = 0;
  int x = g_markers[0].x - 21;
  int y = g_markers[0].y - 14;
  while (x >= wx) wx += 17;
  while (y >= wy) wy += 12;
  return (struct find_player_level_st){
    .wx = wx,
    .wy = wy
  };
}

static void snap_player(int wx, int wy) {
  g_sprites[0].origin.x = (g_markers[0].x - wx) * 12 - 32;
  g_sprites[0].origin.y = (g_markers[0].y - wy) * 12 - 18;
  g_sprites[1].origin.x = (g_markers[1].x - wx) * 12 - 32;
  g_sprites[1].origin.y = (g_markers[1].y - wy) * 12 - 18;
  g_sprites[2].origin.x = (g_markers[2].x - wx) * 12 - 32;
  g_sprites[2].origin.y = (g_markers[2].y - wy) * 12 - 18;
}

static int g_maskworld = 0;
static void copy_world_offset(int wx, int wy) {
  for (int y = 0; y < 16; y++) {
    int sy = wy + y;
    for (int x = 0; x < 25; x++) {
      int sx = wx + x;
      int ww = 0;
      if (sx >= 0 && sx < g_world.width && sy >= 0 && sy < g_world.height) {
        ww = g_world.data[sx + sy * g_world.width];
      }
      if (g_maskworld && (x < 4 || x >= 21 || y < 2 || y >= 14))
        ww = 0;
      settile0(x, y, ww & 0xff);
    }
  }

  const u8 *bg = BINADDR(worldbg_bin);
  for (int y = 1; y < 30; y++) {
    memcpy8(&map1[y * 64], &bg[wx * 2 + (wy * 2 + y) * g_world.width * 2], 45);
  }
  if (g_maskworld) {
    // clear border
    for (int y = 1; y < 4; y++) {
      memset8(&map1[y * 64], 0, 45);
    }
    for (int y = 4; y < 28; y++) {
      for (int i = 0; i < 8; i++) {
        map1[y * 64 + i] = 0;
        map1[y * 64 + 42 + i] = 0;
      }
    }
    for (int y = 28; y < 30; y++) {
      memset8(&map1[y * 64], 0, 45);
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
}

static void nextframe() {
  for (i32 i = 0; i < 128; i++)
    ani_step(i);
  sys_nextframe();
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

  struct find_player_level_st fp = find_player_level();
  snap_player(fp.wx, fp.wy);
  while (1) {
    nextframe();

    int dx = 0, dy = 0;
    if (g_inputhit & SYS_INPUT_SE) g_maskworld = 1 - g_maskworld;
    if (g_inputhit & SYS_INPUT_U){ dy--; g_sprites[0].pc = ani_player_u; }
    if (g_inputhit & SYS_INPUT_R){ dx++; g_sprites[0].pc = ani_player_r; }
    if (g_inputhit & SYS_INPUT_D){ dy++; g_sprites[0].pc = ani_player_d; }
    if (g_inputhit & SYS_INPUT_L){ dx--; g_sprites[0].pc = ani_player_l; }
    if (dx != 0 || dy != 0) {
      g_markers[0].x += dx;
      g_markers[0].y += dy;
      struct find_player_level_st fp2 = find_player_level();
      int fdx = fp2.wx > fp.wx ? 1 : fp2.wx < fp.wx ? -1 : 0;
      int fdy = fp2.wy > fp.wy ? 1 : fp2.wy < fp.wy ? -1 : 0;
      if (fdx != 0 || fdy != 0) {
        if (g_maskworld) {
          // snap to new location
          fp = fp2;
        } else {
          // scroll to new location
          while (fp2.wx != fp.wx || fp2.wy != fp.wy) {
            fp.wx += fdx;
            fp.wy += fdy;
            copy_world_offset(fp.wx, fp.wy);
            snap_player(fp.wx, fp.wy);
            nextframe();
          }
        }
      }
      snap_player(fp.wx, fp.wy);
    }

    copy_world_offset(fp.wx, fp.wy);
  }
}
