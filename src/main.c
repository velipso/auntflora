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

static inline void advance_pt(int *x, int *y, int dir, int amt) {
  switch (dir) {
    case 0: *y = *y - amt; break;
    case 1: *x = *x + amt; break;
    case 2: *y = *y + amt; break;
    case 3: *x = *x - amt; break;
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
static u32 g_playerdir = 3;

static u16 world_at(int x, int y) {
  return g_world.data[x + y * g_world.width];
}

static const u16 g_cellinfo[] = {
  0x0000, 0x8000, 0x8000, 0x0000, 0x8000, 0x0200, 0x8000, 0x8000,
  0x8000, 0x8000, 0x0100, 0x0100, 0x0300, 0x0300, 0x0300, 0x0300,
  0x0000, 0x8000, 0x0000, 0x0000, 0x0000, 0x0200, 0x0000, 0x0000,
  0x8000, 0x8000, 0x0101, 0x0101, 0x0301, 0x0301, 0x0301, 0x0301,
  0x0000, 0x8000, 0x0400, 0x0500, 0x8000, 0x8000, 0x8000, 0x8000,
  0x8000, 0x8000, 0x0102, 0x0102, 0x0302, 0x0302, 0x0302, 0x0302,
  0x0000, 0x060f, 0x060a, 0x0605, 0x8000, 0x8000, 0x8000, 0x8000,
  0x8000, 0x8000, 0x0103, 0x0103, 0x0303, 0x0303, 0x0303, 0x0303,
};

static inline int is_solid_static(u32 cell) {
  return cell & 0x8000;
}

static inline int is_solid(u32 cell) {
  return is_solid_static(cell) || (g_cellinfo[cell & 0x1f] & 0x8000);
}

static inline int is_forward(u32 cell, u32 dir) {
  return g_cellinfo[cell] == (0x0100 | dir);
}

static inline int is_checkpoint(u32 cell) {
  return g_cellinfo[cell] == 0x0200;
}

static inline int is_pusher(u32 cell, u32 dir) {
  return g_cellinfo[cell] == (0x0300 | dir);
}

static inline int is_openbox(u32 cell) {
  return g_cellinfo[cell] == 0x0400;
}

static inline int is_closedbox(u32 cell) {
  return g_cellinfo[cell] == 0x0500;
}

static inline int is_pushable(u32 cell, u32 dir) {
  dir = 1 << dir;
  return (g_cellinfo[cell] & (0xfff0 | dir)) == (0x0600 | dir);
}

static inline int is_empty(u32 cell) {
  return g_cellinfo[cell] == 0;
}

static inline int is_empty_for_player(u32 cell) {
  return is_empty(cell) || is_openbox(cell) || is_checkpoint(cell);
}

static inline int is_empty_for_block(u32 cell) {
  return is_empty(cell);
}

static inline int is_pushable_by_player(u32 cell, u32 dir) {
  return is_pushable(cell, dir);
}

static inline int is_pushable_by_block(u32 cell, u32 dir) {
  return is_pushable(cell, dir) || is_openbox(cell) || is_closedbox(cell);
}

#define UNDO_SIZE  (1 << 13)
static struct {
  // F T YYYYYYYYY XXXXXXXXX PPPP PPPP PPPP
  // | | \_______/ \_______/ \____________/
  // | |     |         |            |
  // | |     |         |            +------- payload (12 bits)
  // | |     |         +-------------------- X position (0-511)
  // | |     +------------------------------ Y position (0-511)
  // | +------------------------------------ type, 0 = logic write, 1 = player write
  // +-------------------------------------- final entry?
  // logic write:
  //   - 6 bits new data
  //   - 6 bits old data
  // player write:
  //   - 2 bits new direction
  //   - 2 bits old direction
  //   - 8 bits message flag (0-254 message activated, 255 no message)
  u32 entries[UNDO_SIZE];
  u16 head;
  u16 tail;
} g_undo SECTION_EWRAM = {0};

static void undo_push(u32 entry) {
  g_undo.entries[g_undo.head] = entry;
  g_undo.head++;
  g_undo.head &= UNDO_SIZE - 1;
  if (g_undo.head == g_undo.tail) {
    g_undo.tail++;
    g_undo.tail &= UNDO_SIZE - 1;
  }
}

static void undo_finish() {
  u32 prev = g_undo.head;
  if (prev == 0)
    prev = UNDO_SIZE - 1;
  else
    prev--;
  g_undo.entries[prev] |= 0x80000000;
}

static void set_player_ani_dir(u32 dir);
static void undo_fire() {
  if (g_undo.head == g_undo.tail) {
    // can't rewind further, out of undo data!
    return;
  }
  // rewind head until finalized entry is found
  u32 final = (g_undo.head - 1) & (UNDO_SIZE - 1);
  int count = 0;
  while (1) {
    if (g_undo.entries[final] & 0x80000000) {
      count++;
      if (count == 2)
        break;
    }
    if (final == g_undo.tail) {
      // can't rewind further, out of undo data!
      return;
    }
    final--;
    final &= UNDO_SIZE - 1;
  }
  final = (final + 1) & (UNDO_SIZE - 1);
  u32 here = g_undo.head;
  while (here != final) {
    here--;
    here &= UNDO_SIZE - 1;

    u32 entry = g_undo.entries[here];
    if (entry == 0x80000000) // initial entry
      continue;
    int x = (entry >> 12) & 0x1ff;
    int y = (entry >> 21) & 0x1ff;
    u32 payload = entry & 0xfff;
    if (entry & 0x40000000) {
      // player write
      u32 olddir = (payload >> 8) & 3;
      u32 newdir = (payload >> 10) & 3;
      u32 msg = payload & 0xff;
      advance_pt(&x, &y, newdir, -1);
      if (is_forward(world_at(x, y), newdir)) {
        // undo past a forwarder
        advance_pt(&x, &y, newdir, -1);
      }
      set_player_ani_dir(olddir);
      g_markers[0].x = x;
      g_markers[0].y = y;
      // TODO: do something with msg
    } else {
      // logic write
      u32 oldlogic = payload & 0x3f;
      u32 k = x + y * g_world.width;
      g_world.data[k] = oldlogic;
    }
  }
  g_undo.head = here;
}

static u32 g_dirty = 0;
static void write_logic(u32 x, u32 y, u32 data) {
  g_dirty = 1;
  u32 k = x + y * g_world.width;
  u32 old = g_world.data[k];
  g_world.data[k] = data;
  undo_push((y << 21) | (x << 12) | (data << 6) | old);
}

static void write_player(u32 x, u32 y, u32 dir, i32 message) {
  g_dirty = 1;
  u32 old = g_playerdir;
  g_markers[0].x = x;
  g_markers[0].y = y;
  set_player_ani_dir(dir);
  undo_push(0x40000000 | (y << 21) | (x << 12) | (dir << 10) | (old << 8) | (message & 0xff));
}

static void set_player_ani_dir(u32 dir) {
  g_playerdir = dir;
  switch (dir) {
    case 0: g_sprites[0].pc = ani_player_u; break;
    case 1: g_sprites[0].pc = ani_player_r; break;
    case 2: g_sprites[0].pc = ani_player_d; break;
    case 3: g_sprites[0].pc = ani_player_l; break;
  }
}

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
        ww = world_at(sx, sy);
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

  // initialize undo
  g_undo.head = 1;
  g_undo.tail = 0;
  g_undo.entries[0] = 0x80000000;
}

static void snd_walk() {
  // TODO: this
}

static void snd_push() {
  // TODO: this
}

static void snd_forward() {
  // TODO: this
}

static void snd_bump() {
  // TODO: this
}

static void nextframe() {
  for (i32 i = 0; i < 128; i++)
    ani_step(i);
  sys_nextframe();
}

static inline void delay_push() {
  for (int i = 0; i < 20; i++)
    nextframe();
}

static void move_screen_to_player(struct find_player_level_st *fp) {
  // move screen based on player location
  struct find_player_level_st fp2 = find_player_level();
  int fdx = fp2.wx > fp->wx ? 1 : fp2.wx < fp->wx ? -1 : 0;
  int fdy = fp2.wy > fp->wy ? 1 : fp2.wy < fp->wy ? -1 : 0;
  if (fdx != 0 || fdy != 0) {
    if (g_maskworld) {
      // snap to new location
      *fp = fp2;
    } else {
      // scroll to new location
      while (fp2.wx != fp->wx || fp2.wy != fp->wy) {
        fp->wx += fdx;
        fp->wy += fdy;
        copy_world_offset(fp->wx, fp->wy);
        snap_player(fp->wx, fp->wy);
        nextframe();
      }
    }
  }
  snap_player(fp->wx, fp->wy);
}

static struct { int x; int y; int dir; } g_pushers[32];
static int g_pushers_size = 0;

static inline void reset_pushers() {
  g_pushers_size = 0;
}

static void blink_tile(struct find_player_level_st *fp, int x, int y) {
#if 0
  for (int blink = 0; blink < 10; blink++) {
    g_sprites[3].pc = (blink & 1) ? NULL : ani_cat;
    g_sprites[3].origin.x = (x - fp->wx) * 12 - 32;
    g_sprites[3].origin.y = (y - fp->wy) * 12 - 18;
    nextframe();nextframe();
  }
#endif
}

static void find_pushers(struct find_player_level_st *fp, int x, int y, int dir) {
  int dir2 = (dir + 2) & 3; // rotate by 180
  while (1) {
    u16 nc = world_at(x, y);
    blink_tile(fp, x, y);
    if (is_pusher(nc, dir2)) {
      g_pushers[g_pushers_size].x = x;
      g_pushers[g_pushers_size].y = y;
      g_pushers[g_pushers_size].dir = dir2;
      g_pushers_size++;
      return;
    } else if (!is_pushable_by_block(nc, dir2)) {
      return;
    }
    advance_pt(&x, &y, dir, 1);
  }
}

static void apply_pushers(struct find_player_level_st *fp) {
  while (1) {
#if 0
    for (int i = 0; i < 128 - 3; i++) {
      if (i < g_pushers_size) {
        g_sprites[i + 3].pc = ani_cat;
        g_sprites[i + 3].origin.x = (g_pushers[i].x - fp->wx) * 12 - 32;
        g_sprites[i + 3].origin.y = (g_pushers[i].y - fp->wy) * 12 - 18;
      } else {
        g_sprites[i + 3].pc = NULL;
      }
    }
    nextframe();
#endif

    int dirty = 0;
    for (int i = 0; i < g_pushers_size; i++) {
      int nx = g_pushers[i].x;
      int ny = g_pushers[i].y;
      int dir = g_pushers[i].dir;
      int px = nx;
      int py = ny;
      while (1) {
        advance_pt(&px, &py, dir, 1);
        u16 nc = world_at(px, py);
        if (is_empty_for_block(nc)) {
          if (!dirty) {
            move_screen_to_player(fp);
            copy_world_offset(fp->wx, fp->wy);
            delay_push();
            snd_push();
            dirty = 1;
          }
          int hx, hy;
          while (px != nx || py != ny) {
            hx = px;
            hy = py;
            advance_pt(&px, &py, dir, -1);
            write_logic(hx, hy, world_at(px, py));
          }
          write_logic(nx, ny, 0);
          advance_pt(&g_pushers[i].x, &g_pushers[i].y, dir, 1);
          break;
        } else if (is_pushable_by_block(nc, dir)) {
          // pusher could still push
          continue;
        } else {
          // pusher is blocked
          break;
        }
      }
    }
    if (!dirty)
      break;
  }
}

static void find_pushers_around_player(struct find_player_level_st *fp, int movedir) {
  for (int d = 0; d < 4; d++) {
    if (d == movedir)
      continue;
    int x = g_markers[0].x;
    int y = g_markers[0].y;
    advance_pt(&x, &y, d, 1);
    find_pushers(fp, x, y, d);
  }
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

    if (g_inputhit & SYS_INPUT_B) {
      // undo
      undo_fire();
      fp = find_player_level();
      snap_player(fp.wx, fp.wy);
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
          reset_pushers();
          if (is_empty_for_player(cell)) {
            snd_walk();
            find_pushers_around_player(&fp, dir);
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
                snd_push();
                find_pushers_around_player(&fp, dir);
                write_player(nx, ny, dir, -1);
                break;
              } else if (is_pushable_by_block(nc, dir)) {
                advance_pt(&px, &py, dir, 1);
                // find pushers in perpendicular directions
                int dir1 = (dir + 1) & 3;
                int dir2 = (dir + 3) & 3;
                int fx = px;
                int fy = py;
                advance_pt(&fx, &fy, dir1, 1);
                if (is_pushable_by_block(nc, dir2))
                  find_pushers(&fp, fx, fy, dir1);
                advance_pt(&fx, &fy, dir2, 2);
                if (is_pushable_by_block(nc, dir1))
                  find_pushers(&fp, fx, fy, dir2);
                continue;
              } else {
                break;
              }
            }
          } else if (is_forward(cell, dir)) {
            advance_pt(&nx, &ny, dir, 1);
            if (is_empty_for_player(world_at(nx, ny))) {
              snd_forward();
              find_pushers_around_player(&fp, dir);
              write_player(nx, ny, dir, -1);
            }
          }

          apply_pushers(&fp);
        }

        if (g_dirty) {
          undo_finish();
          move_screen_to_player(&fp);
        } else {
          snd_bump();
          set_player_ani_dir(dir);
        }
      }
    }

    copy_world_offset(fp.wx, fp.wy);
  }
}
