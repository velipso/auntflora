//
// auntflora - Port of Aunt Flora's Mansion to Game Boy Advance
// by Sean Connelly (@velipso), https://sean.cm
// Project Home: https://github.com/velipso/auntflora
// SPDX-License-Identifier: 0BSD
//

#define SYS_GBA
#include "sys.h"
#include "gba.h"

extern void sys__irq_init();
extern void (*sys__irq_vblank)();

static u16 SECTION_EWRAM g_screen_sprite_enable;
static u16 SECTION_EWRAM g_screen_mode;
static u16 SECTION_EWRAM g_screen_enable;

static void _sys_set_screen_mode();

void sys_init() {
  sys__irq_init();
  g_screen_sprite_enable = 0x1000;
  g_screen_enable = 0x0080;
  g_screen_mode = SYS_SCREEN_MODE_4T;
  _sys_set_screen_mode();
}

void sys_set_sprite_enable(i32 enable) {
  g_screen_sprite_enable = enable ? 0x1000 : 0;
  _sys_set_screen_mode();
}

void sys_set_screen_enable(i32 enable) {
  g_screen_enable = enable ? 0 : 0x0080;
  _sys_set_screen_mode();
}

void sys_set_screen_mode(i32 mode) {
  g_screen_mode = mode;
  _sys_set_screen_mode();
}

static void _sys_set_screen_mode() {
  switch (g_screen_mode) {
    case SYS_SCREEN_MODE_4T: // 4 text
      REG_DISPCNT =
        g_screen_enable |
        g_screen_sprite_enable |
        0x0000 | // mode 0
        0x0100 | // enable BG0
        0x0200 | // enable BG1
        0x0400 | // enable BG2
        0x0800;  // enable BG3
      break;
    case SYS_SCREEN_MODE_2T1S: // 2 text, 1 scaling, arbitrary scaling
      REG_DISPCNT =
        g_screen_enable |
        g_screen_sprite_enable |
        0x0001 | // mode 1
        0x0100 | // enable BG0
        0x0200 | // enable BG1
        0x0400 | // enable BG2
        0x1000;  // enable OAM
      break;
    case SYS_SCREEN_MODE_2S: // 2 scaling, arbitrary scaling
      REG_DISPCNT =
        g_screen_enable |
        g_screen_sprite_enable |
        0x0002 | // mode 2
        0x0400 | // enable BG2
        0x0800;  // enable BG3
      break;
    case SYS_SCREEN_MODE_2S6X6: // 2 scaling, targeting 6x6 tiles
      REG_DISPCNT =
        g_screen_enable |
        g_screen_sprite_enable |
        0x0002 | // mode 2
        0x0400 | // enable BG2
        0x0800;  // enable BG3

      // set scaling registers
      REG_BG2PA = 0x0156; // 8/6 * 0x100
      REG_BG2PB = 0;
      REG_BG2PC = 0;
      REG_BG2PD = 0x0156; // 8/6 * 0x100
      REG_BG3PA = 0x0156; // 8/6 * 0x100
      REG_BG3PB = 0;
      REG_BG3PC = 0;
      REG_BG3PD = 0x0156; // 8/6 * 0x100
      break;
    case SYS_SCREEN_MODE_2S5X5: // 2 scaling, targeting 5x5 tiles
      REG_DISPCNT =
        g_screen_enable |
        g_screen_sprite_enable |
        0x0002 | // mode 2
        0x0400 | // enable BG2
        0x0800;  // enable BG3

      // set scaling registers
      REG_BG2PA = 0x019a; // 8/5 * 0x100
      REG_BG2PB = 0;
      REG_BG2PC = 0;
      REG_BG2PD = 0x019a; // 8/5 * 0x100
      REG_BG3PA = 0x019a; // 8/5 * 0x100
      REG_BG3PB = 0;
      REG_BG3PC = 0;
      REG_BG3PD = 0x019a; // 8/5 * 0x100
      break;
    case SYS_SCREEN_MODE_1F: // 1 full-color, arbitrary scaling
      REG_DISPCNT =
        g_screen_enable |
        g_screen_sprite_enable |
        0x0003 | // mode 3
        0x0400;  // enable BG2
      break;
    case SYS_SCREEN_MODE_2F: // 2 full-color, arbitrary scaling
      REG_DISPCNT =
        g_screen_enable |
        g_screen_sprite_enable |
        0x0005 | // mode 5
        0x0400;  // enable BG2
      break;
    case SYS_SCREEN_MODE_2I: // 2 indexed (256 color)
      REG_DISPCNT =
        g_screen_enable |
        g_screen_sprite_enable |
        0x0004 | // mode 4
        0x0400;  // enable BG2
      break;
  }
}

void sys_set_vblank(void (*irq_vblank_handler)()) {
  REG_IME = 0;
  if (irq_vblank_handler) {
    REG_DISPSTAT |= 0x0008;
    REG_IE |= 1;
    sys__irq_vblank = irq_vblank_handler;
  } else {
    REG_DISPSTAT &= 0xfff7;
    REG_IE &= 0xfffe;
    sys__irq_vblank = NULL;
  }
  REG_IME = 1;
}

void sys_nextframe() {
  __asm__("swi #5" ::: "r0", "r1", "r2", "r3", "r12", "lr", "memory", "cc");
}
