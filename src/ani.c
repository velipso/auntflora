//
// auntflora - Port of Aunt Flora's Mansion to Game Boy Advance
// by Sean Connelly (@velipso), https://sean.fun
// Project Home: https://github.com/velipso/auntflora
// SPDX-License-Identifier: 0BSD
//

#include "ani.h"

u16 g_oam[0x200] = {0};
sprite_st g_sprites[128] = {0};

void ani_flushxy(u32 i) {
  sprite_st *s = &g_sprites[i];
  u16 *oam = &g_oam[i * 4];
  i32 y = s->origin.y + (s->offset.y >> 8);
  if (y < -64 || y > 160)
    y = 160;
  oam[0] = (y & 0x00ff) | (oam[0] & 0xff00);
  i32 x = s->origin.x + (s->offset.x >> 8);
  if (x < -64 || x > 240)
    x = 240;
  oam[1] = (x & 0x01ff) | (oam[1] & 0xfe00);
}

void ani_step(u32 i) {
  sprite_st *s = &g_sprites[i];
  u16 *oam = &g_oam[i * 4];
  if (s->pc == NULL) {
    oam[0] = 160;
    oam[1] = 240;
    oam[2] = 0;
    return;
  }
  s->offset.x += s->offset.dx;
  s->offset.y += s->offset.dy;
  s->offset.dy += s->gravity;

  while (1) {
    i32 command = *s->pc;
    i32 param = command & 0x0fff;
    command >>= 12;
    switch (command) {
      case 0x0: { // subcommand
        command = param >> 8;
        param &= 0xff;
        switch (command) {
          case 0x0: { // subcommand
            command = param >> 4;
            param &= 0xf;
            switch (command) {
              case 0x0: { // subcommand
                switch (param) {
                  case 0x0: // reset
                    oam[0] = 0x2000 | 160; // 256 color
                    oam[1] = 240;
                    oam[2] = 0;
                    s->waitcount = 0;
                    s->loopcount = 0;
                    s->gravity = 0;
                    s->offset.x = 0;
                    s->offset.y = 0;
                    s->offset.dx = 0;
                    s->offset.dy = 0;
                    break;
                  case 0x1: // stop
                    ani_flushxy(i);
                    return;
                  case 0x2: // destroy
                    oam[0] = 160;
                    oam[1] = 240;
                    oam[2] = 0;
                    s->pc = NULL;
                    return;
                  case 0x3: // reserved
                  case 0x4: // reserved
                  case 0x5: // reserved
                  case 0x6: // reserved
                  case 0x7: // reserved
                  case 0x8: // reserved
                  case 0x9: // reserved
                  case 0xa: // reserved
                  case 0xb: // reserved
                  case 0xc: // reserved
                  case 0xd: // reserved
                  case 0xe: // reserved
                  case 0xf: // reserved
                    break;
                }
              } break;
              case 0x1: // setSize
                oam[0] = (oam[0] & 0x3fff) | ((param >> 2) << 14);
                oam[1] = (oam[1] & 0x3fff) | ((param & 0x3) << 14);
                break;
              case 0x2: // setPriority
                oam[2] = (oam[2] & 0xf3ff) | (param << 10);
                break;
              case 0x3: // setPalette
                oam[2] = (oam[2] & 0x0fff) | (param << 12);
                break;
              case 0x4: // setFlip
                oam[1] = (oam[1] & 0xcfff) | (param << 12);
                break;
              case 0x5: // setBlend
                oam[0] = (oam[0] & 0xf3ff) | (param << 10);
                break;
              case 0x6: // reserved
              case 0x7: // reserved
              case 0x8: // reserved
              case 0x9: // reserved
              case 0xa: // reserved
              case 0xb: // reserved
              case 0xc: // reserved
              case 0xd: // reserved
              case 0xe: // reserved
              case 0xf: // reserved
                break;
            }
          } break;
          case 0x1: // wait
            if (s->waitcount < param) {
              s->waitcount++;
              ani_flushxy(i);
              return;
            }
            s->waitcount = 0;
            break;
          case 0x2: // setLoop
            s->loopcount = param;
            break;
          case 0x3: // jump
            if (param >= 128)
              param -= 256;
            s->pc += param;
            continue;
          case 0x4: // jumpIfLooping
            if (s->loopcount > 0) {
              s->loopcount--;
              if (param >= 128)
                param -= 256;
              s->pc += param;
              continue;
            }
            // done looping
            break;
          case 0x5: // addRandomOffsetX
            // TODO: this
            break;
          case 0x6: // addRandomOffsetY
            // TODO: this
            break;
          case 0x7: // addRandomOffsetDX
            // TODO: this
            break;
          case 0x8: // addRandomOffsetDY
            // TODO: this
            break;
          case 0x9: // spawn
            // TODO: this
            break;
          case 0xa: // addTile
            if (param >= 128)
              param -= 256;
            oam[2] = (oam[2] & 0xfc00) | ((oam[2] + param) & 0x03ff);
            break;
          case 0xb: // reserved
          case 0xc: // reserved
          case 0xd: // reserved
          case 0xe: // reserved
          case 0xf: // reserved
            break;
        }
      } break;
      case 0x1: // setTile
        oam[2] = (oam[2] & 0xfc00) | param;
        break;
      case 0x2: // rotatePalette
        // TODO: this
        break;
      case 0x3: // playSound
        // TODO: this
        break;
      case 0x4: // setGravity
        if (param >= 2048)
          param -= 4096;
        s->gravity = param;
        break;
      case 0x5: // setOffsetX
        if (param >= 2048)
          param -= 4096;
        s->offset.x = param << 4;
        break;
      case 0x6: // setOffsetY
        if (param >= 2048)
          param -= 4096;
        s->offset.y = param << 4;
        break;
      case 0x7: // addOffsetX
        if (param >= 2048)
          param -= 4096;
        s->offset.x += param << 4;
        break;
      case 0x8: // addOffsetY
        if (param >= 2048)
          param -= 4096;
        s->offset.y += param << 4;
        break;
      case 0x9: // setOffsetDX
        if (param >= 2048)
          param -= 4096;
        s->offset.dx = param << 4;
        break;
      case 0xa: // setOffsetDY
        if (param >= 2048)
          param -= 4096;
        s->offset.dy = param << 4;
        break;
      case 0xb: // addOffsetDX
        if (param >= 2048)
          param -= 4096;
        s->offset.dx += param << 4;
        break;
      case 0xc: // addOffsetDY
        if (param >= 2048)
          param -= 4096;
        s->offset.dy += param << 4;
        break;
      case 0xd: // jumpIfRandom
        // TODO: this
        break;
      case 0xe: // reserved
      case 0xf: // reserved
        break;
    }

    // advance PC
    s->pc++;
  }
}
