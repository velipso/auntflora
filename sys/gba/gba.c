//
// auntflora - Port of Aunt Flora's Mansion to Game Boy Advance
// by Sean Connelly (@velipso), https://sean.cm
// Project Home: https://github.com/velipso/auntflora
// SPDX-License-Identifier: 0BSD
//

#include "gba.h"
#include "reg.h"

extern void sys__irq_init();
extern void (*sys__irq_vblank)();
extern void (*sys__irq_timer1)();
extern void sys__snd_timer1_handler();

struct snd_st g_snd;

static u16 SECTION_EWRAM g_screen_sprite_enable;
static u16 SECTION_EWRAM g_screen_mode;
static u16 SECTION_EWRAM g_screen_enable;
static void (*g_vblank)();

static void _sys_set_screen_mode();
static void _sys_snd_init();

void sys_init() {
  sys__irq_init();
  _sys_snd_init();
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

static void _sys_wrap_vblank() {
  // allow re-entrant IRQs so timer1 for snd is handled
  REG_IME = 1;
  if (g_vblank)
    g_vblank();
}

void sys_set_vblank(void (*irq_vblank_handler)()) {
  g_vblank = irq_vblank_handler;
}

void sys_nextframe() {
  __asm__("swi #5" ::: "r0", "r1", "r2", "r3", "r12", "lr", "memory", "cc");
}

static void _sys_snd_init() {
  REG_IME = 0;
  // enable vblank handler
  REG_DISPSTAT |= 0x0008;
  REG_IE |= 1;
  g_vblank = NULL;
  sys__irq_vblank = _sys_wrap_vblank;
  // enable timer1 handler
  sys__irq_timer1 = sys__snd_timer1_handler;
  // clear struct
  memset32(&g_snd, 0, sizeof(g_snd));
  // enable timer1
  REG_IE |= 0x10;
  // setup timers
  REG_TM0CNT = 0;
  REG_TM1CNT = 0;
  // setup timer0 - drives the sample rate
  // cpuHz      = 2^24
  // sampleRate = 2^15
  // timer0Wait = cpuHz / sampleRate = 2^9 = 0x200
  // timer0Res  = 1 cycle
  // timer0Wait / timer0Res = 0x200
  REG_TM0D = 0x10000 - 0x200;
  // setup timer1 - drives the DMA buffer cycling
  // bufferSize = 0x260
  // timer1Wait = bufferSize * timer0Wait = 311296 cycles
  // timer1Res  = 64 cycles
  // timer1Wait / timer1Res = 0x1300
  REG_TM1D = 0x10000 - 0x1300;
  REG_IME = 1;
  // turn sound chip on
  REG_SOUNDCNT_X = 0x0080;
  // set sound to use FIFO A
  REG_SOUNDCNT_H = 0x0b0f;
  // set DMA1 destination to FIFO A
  REG_DMA1DAD = &REG_FIFO_A;
  // point DMA1 to buffer1
  REG_DMA1SAD = (u32)g_snd.buffer1;
  // enable DMA1
  REG_DMA1CNT_H = 0xb640;
  // save alt buffers for next render
  g_snd.buffer_addr[0] = g_snd.buffer1;
  g_snd.buffer_addr[1] = g_snd.buffer2;
  g_snd.buffer_addr[2] = g_snd.buffer3;
  g_snd.next_buffer_index = 4;
  // start timer0
  REG_TM0CNT = 0x0080;
  // start timer1
  REG_TM1CNT = 0x00c1;
}
