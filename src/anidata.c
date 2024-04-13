//
// auntflora - Port of Aunt Flora's Mansion to Game Boy Advance
// by Sean Connelly (@velipso), https://sean.cm
// Project Home: https://github.com/velipso/auntflora
// SPDX-License-Identifier: 0BSD
//

#include "anidata.h"
#include <stdlib.h>

#define U2(x) \
  __builtin_choose_expr(__builtin_constant_p(x) && \
    (x) >= 0 && (x) <= 3, (x), \
  exit(1))
#define U4(x) \
  __builtin_choose_expr(__builtin_constant_p(x) && \
    (x) >= 0 && (x) <= 15, (x), \
  exit(1))
#define U8(x) \
  __builtin_choose_expr(__builtin_constant_p(x) && \
    (x) >= 0 && (x) <= 255, (x), \
  exit(1))
#define S8(x) \
  __builtin_choose_expr(__builtin_constant_p(x) && \
    (x) >= -128 && (x) <= 127, (x) < 0 ? (x) + 0x100 : (x), \
  exit(1))
#define U10(x) \
  __builtin_choose_expr(__builtin_constant_p(x) && \
    (x) >= 0 && (x) <= 1023, (x), \
  exit(1))
#define U12(x) \
  __builtin_choose_expr(__builtin_constant_p(x) && \
    (x) >= 0 && (x) <= 4095, (x), \
  exit(1))
#define S12(x) \
  __builtin_choose_expr(__builtin_constant_p(x) && \
    (x) >= -2048 && (x) <= 2047, (x) < 0 ? (x) + 0x1000 : (x), \
  exit(1))
#define F48(x)  S12(((int)((x) * 256)))
#define F84(x)  S12(((int)((x) * 16)))

#define RESET()        0x0000
#define STOP()         0x0001
#define DESTROY()      0x0002
#define SIZE(v)        (0x0010 | U4(v))
#define SIZE_8x8()     SIZE(0)
#define SIZE_16x16()   SIZE(1)
#define SIZE_32x32()   SIZE(2)
#define SIZE_64x64()   SIZE(3)
#define SIZE_16x8()    SIZE(4)
#define SIZE_32x8()    SIZE(5)
#define SIZE_32x16()   SIZE(6)
#define SIZE_64x32()   SIZE(7)
#define SIZE_8x16()    SIZE(8)
#define SIZE_8x32()    SIZE(9)
#define SIZE_16x32()   SIZE(10)
#define SIZE_32x64()   SIZE(11)
#define PRIORITY(v)    (0x0020 | U4(v))
#define PALETTE(v)     (0x0030 | U4(v))
#define FLIP(v)        (0x0040 | U2(v))
#define BLEND(v)       (0x0050 | U2(v))
#define WAIT(v)        (0x0100 | U8(v))
#define LOOP(v)        (0x0200 | U8(v))
#define JUMP(v)        (0x0300 | S8(v))
#define JUMPIFLOOP(v)  (0x0400 | S8(v))
#define ADDRNDX(v)     (0x0500 | S8(v))
#define ADDRNDY(v)     (0x0600 | S8(v))
#define ADDRNDDX(v)    (0x0700 | S8(v))
#define ADDRNDDY(v)    (0x0800 | S8(v))
#define SPAWN(v)       (0x0900 | U8(v))
#define ADDTILE(v)     (0x0a00 | S8(v))
#define TILE(v)        (0x1000 | U10(v))
#define TILEXY(x, y)   TILE((((x) >> 3) + (((y) >> 3) << 4)) << 1)
#define GRAVITY(v)     (0x4000 | F48(v))
#define X(v)           (0x5000 | F4(v))
#define Y(v)           (0x6000 | F4(v))
#define ADDX(v)        (0x7000 | F4(v))
#define ADDY(v)        (0x8000 | F4(v))
#define DX(v)          (0x9000 | F4(v))
#define DY(v)          (0xa000 | F4(v))
#define ADDDX(v)       (0xb000 | F4(v))
#define ADDDY(v)       (0xc000 | F4(v))

u16 ani_player_u[] = {
  RESET(),
  SIZE_16x16(),
  TILEXY(0, 16),
  STOP()
};

u16 ani_player_r[] = {
  RESET(),
  SIZE_16x16(),
  TILEXY(0, 32),
  STOP()
};

u16 ani_player_d[] = {
  RESET(),
  SIZE_16x16(),
  TILEXY(0, 48),
  STOP()
};

u16 ani_player_l[] = {
  RESET(),
  SIZE_16x16(),
  TILEXY(0, 64),
  STOP()
};
