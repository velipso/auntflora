//
// auntflora - Port of Aunt Flora's Mansion to Game Boy Advance
// by Sean Connelly (@velipso), https://sean.fun
// Project Home: https://github.com/velipso/auntflora
// SPDX-License-Identifier: 0BSD
//

#include "gfx.h"
#include "reg.h"

void gfx_init() {
  REG_DISPCNT = 0x0080; // turn off screen by default
}

void gfx_setmode(enum gfx_mode mode) {
  switch (mode) {
    case GFX_MODE_4T: // 4 text
      REG_DISPCNT = (REG_DISPCNT & 0xfff8) | 0x0000;
      REG_BG0HOFS = 0;
      REG_BG0VOFS = 0;
      REG_BG1HOFS = 0;
      REG_BG1VOFS = 0;
      REG_BG2HOFS = 0;
      REG_BG2VOFS = 0;
      REG_BG3HOFS = 0;
      REG_BG3VOFS = 0;
      break;
    case GFX_MODE_2T1S: // 2 text, 1 scaling, arbitrary scaling
      REG_DISPCNT = (REG_DISPCNT & 0xfff8) | 0x0001;
      REG_BG0HOFS = 0;
      REG_BG0VOFS = 0;
      REG_BG1HOFS = 0;
      REG_BG1VOFS = 0;
      REG_BG2X = 0;
      REG_BG2Y = 0;
      REG_BG2PA = 0x0100;
      REG_BG2PB = 0;
      REG_BG2PC = 0;
      REG_BG2PD = 0x0100;
      break;
    case GFX_MODE_2S: // 2 scaling, arbitrary scaling
      REG_DISPCNT = (REG_DISPCNT & 0xfff8) | 0x0002;
      REG_BG2X = 0;
      REG_BG2Y = 0;
      REG_BG2PA = 0x0100;
      REG_BG2PB = 0;
      REG_BG2PC = 0;
      REG_BG2PD = 0x0100;
      REG_BG3X = 0;
      REG_BG3Y = 0;
      REG_BG3PA = 0x0100;
      REG_BG3PB = 0;
      REG_BG3PC = 0;
      REG_BG3PD = 0x0100;
      break;
    case GFX_MODE_2S6X6: // 2 scaling, targeting 6x6 tiles
      REG_DISPCNT = (REG_DISPCNT & 0xfff8) | 0x0002;
      REG_BG2X = 0;
      REG_BG2Y = 0;
      REG_BG2PA = 0x0156; // 8/6 * 0x100
      REG_BG2PB = 0;
      REG_BG2PC = 0;
      REG_BG2PD = 0x0156; // 8/6 * 0x100
      REG_BG3X = 0;
      REG_BG3Y = 0;
      REG_BG3PA = 0x0156; // 8/6 * 0x100
      REG_BG3PB = 0;
      REG_BG3PC = 0;
      REG_BG3PD = 0x0156; // 8/6 * 0x100
      break;
    case GFX_MODE_2S5X5: // 2 scaling, targeting 5x5 tiles
      REG_DISPCNT = (REG_DISPCNT & 0xfff8) | 0x0002;
      REG_BG2X = 0;
      REG_BG2Y = 0;
      REG_BG2PA = 0x019a; // 8/5 * 0x100
      REG_BG2PB = 0;
      REG_BG2PC = 0;
      REG_BG2PD = 0x019a; // 8/5 * 0x100
      REG_BG3X = 0;
      REG_BG3Y = 0;
      REG_BG3PA = 0x019a; // 8/5 * 0x100
      REG_BG3PB = 0;
      REG_BG3PC = 0;
      REG_BG3PD = 0x019a; // 8/5 * 0x100
      break;
    case GFX_MODE_1F: // 1 full-color, arbitrary scaling
      REG_DISPCNT = (REG_DISPCNT & 0xfff8) | 0x0003;
      REG_BG2X = 0;
      REG_BG2Y = 0;
      REG_BG2PA = 0x0100;
      REG_BG2PB = 0;
      REG_BG2PC = 0;
      REG_BG2PD = 0x0100;
      break;
    case GFX_MODE_2F: // 2 full-color, arbitrary scaling
      REG_DISPCNT = (REG_DISPCNT & 0xfff8) | 0x0005;
      REG_BG2X = 0;
      REG_BG2Y = 0;
      REG_BG2PA = 0x0100;
      REG_BG2PB = 0;
      REG_BG2PC = 0;
      REG_BG2PD = 0x0100;
      break;
    case GFX_MODE_2I: // 2 indexed (256 color)
      REG_DISPCNT = (REG_DISPCNT & 0xfff8) | 0x0004;
      REG_BG2X = 0;
      REG_BG2Y = 0;
      REG_BG2PA = 0x0100;
      REG_BG2PB = 0;
      REG_BG2PC = 0;
      REG_BG2PD = 0x0100;
      break;
  }
}

void gfx_showscreen(bool show) {
  if (show) {
    REG_DISPCNT &= 0xff7f;
  } else {
    REG_DISPCNT |= 0x0080;
  }
}

void gfx_showobj(bool show) {
  if (show) {
    REG_DISPCNT |= 0x1000;
  } else {
    REG_DISPCNT &= 0xefff;
  }
}

void gfx_showbg0(bool show) {
  if (show) {
    REG_DISPCNT |= 0x0100;
  } else {
    REG_DISPCNT &= 0xfeff;
  }
}

void gfx_showbg1(bool show) {
  if (show) {
    REG_DISPCNT |= 0x0200;
  } else {
    REG_DISPCNT &= 0xfdff;
  }
}

void gfx_showbg2(bool show) {
  if (show) {
    REG_DISPCNT |= 0x0400;
  } else {
    REG_DISPCNT &= 0xfbff;
  }
}

void gfx_showbg3(bool show) {
  if (show) {
    REG_DISPCNT |= 0x0800;
  } else {
    REG_DISPCNT &= 0xf7ff;
  }
}

