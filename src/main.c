//
// auntflora - Port of Aunt Flora's Mansion to Game Boy Advance
// by Sean Connelly (@velipso), https://sean.cm
// Project Home: https://github.com/velipso/auntflora
// SPDX-License-Identifier: 0BSD
//

#define SYS_GBA
#include "sys.h"
#include <stdint.h>

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
BINFILE(world_bin);

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
  u32 tx = t & 0xf;
  u32 ty = t >> 4;
  u32 tk = (tx * 2) + (ty * 16 * 2);
  map0[k + 0] = tk;
  map0[k + 1] = tk + 1;
  map0[k + 64] = tk + 32;
  map0[k + 65] = tk + 33;
}

static void settile1(u32 x, u32 y, u32 t) {
  u32 k = (x * 2) + (y * 64 * 2);
  u32 tx = t & 0xf;
  u32 ty = t >> 4;
  u32 tk = (tx * 2) + (ty * 16 * 2);
  map1[k + 0] = tk;
  map1[k + 1] = tk + 1;
  map1[k + 64] = tk + 32;
  map1[k + 65] = tk + 33;
}

static void SECTION_IWRAM_ARM irq_vblank_6x6() {
  sys_copy_map(28, 0, map0, 64 * 64);
  sys_copy_map(30, 0, map1, 64 * 64);
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
  sys_copy_tiles(0, BINADDR(tiles_hd_bin), BINSIZE(tiles_hd_bin));
  sys_copy_tiles(1, BINADDR(font_hd_bin), BINSIZE(font_hd_bin));

  for (int y = 0; y < 32; y++) {
    for (int x = 0; x < 32; x++) {
      settile0(x, y, 4);
    }
  }
  settile0(12, 7, 0);
  settile0(12, 8, 0);
  sys_set_bgs2_scroll(0x0156 * 30, 0x0156 * 16);
  sys_nextframe();
  sys_set_screen_enable(1);

  while (1) {
    sys_nextframe();
  }
}
