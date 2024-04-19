//
// auntflora - Port of Aunt Flora's Mansion to Game Boy Advance
// by Sean Connelly (@velipso), https://sean.cm
// Project Home: https://github.com/velipso/auntflora
// SPDX-License-Identifier: 0BSD
//

#pragma once
#include <stdint.h>
#include <stddef.h>
#include "gba/reg.h"

typedef uint8_t  byte;
typedef uint8_t  u8;
typedef uint16_t ushort;
typedef uint16_t u16;
typedef uint32_t uint;
typedef uint32_t u32;
typedef int8_t   sbyte;
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;

// access binary files generated via Makefile's objbinary
#define BINFILE(n) \
  extern const u8 _binary_ ## n ## _start[]; \
  extern const u8 _binary_ ## n ## _size[]
#define BINADDR(n)  ((void *)&_binary_ ## n ## _start)
#define BINSIZE(n)  ((u32)&_binary_ ## n ## _size)

extern void gvmain();

#define SYS_SCREEN_MODE_4T     0  // 4 text
#define SYS_SCREEN_MODE_2T1S   1  // 2 text, 1 scaling, arbitrary scaling
#define SYS_SCREEN_MODE_2S     2  // 2 scaling, arbitrary scaling
#define SYS_SCREEN_MODE_2S6X6  3  // 2 scaling, targeting 6x6 tiles
#define SYS_SCREEN_MODE_2S5X5  4  // 2 scaling, targeting 5x5 tiles
#define SYS_SCREEN_MODE_1F     5  // 1 full-color, arbitrary scaling
#define SYS_SCREEN_MODE_2F     6  // 2 full-color, arbitrary scaling
#define SYS_SCREEN_MODE_2I     7  // 2 indexed (256 color)

#define SYS_BGT_SIZE_256X256    0
#define SYS_BGT_SIZE_512X256    1
#define SYS_BGT_SIZE_256X512    2
#define SYS_BGT_SIZE_512X512    3
#define SYS_BGS_SIZE_128X128    0
#define SYS_BGS_SIZE_256X256    1
#define SYS_BGS_SIZE_512X512    2
#define SYS_BGS_SIZE_1024X1024  3

#define SYS_INPUT_A   (1 << 0)
#define SYS_INPUT_B   (1 << 1)
#define SYS_INPUT_SE  (1 << 2)
#define SYS_INPUT_ST  (1 << 3)
#define SYS_INPUT_R   (1 << 4)
#define SYS_INPUT_L   (1 << 5)
#define SYS_INPUT_U   (1 << 6)
#define SYS_INPUT_D   (1 << 7)
#define SYS_INPUT_ZR  (1 << 8)
#define SYS_INPUT_ZL  (1 << 9)

void sys_init();
void sys_set_sprite_enable(i32 enable);
void sys_set_screen_enable(i32 enable);
void sys_set_screen_mode(i32 mode);
void sys_set_vblank(void (*irq_vblank_handler)());
void sys_nextframe();

#define RGB15(r, g, b)  (((r) & 0x1f) | (((g) & 0x1f) << 5) | (((b) & 0x1f) << 10))

#if defined(SYS_GBA)

extern void memcpy32(void *dest, const void *src, u32 bytecount);
extern void memcpy16(void *dest, const void *src, u32 bytecount);
extern void memcpy8(void *dest, const void *src, u32 bytecount);
extern void memset32(void *dest, u32 data, u32 bytecount);
extern void memset16(void *dest, u32 data, u32 bytecount);
extern void memset8(void *dest, u32 data, u32 bytecount);

#define SECTION_EWRAM      __attribute__((section(".ewram")))
#define SECTION_IWRAM_ARM  __attribute__((section(".iwram"), target("arm")))

#define sys_pset_1f(x, y, c)  ((u16 *)0x06000000)[(x) + (y) * 240] = (c)

static inline void sys_set_bg_config(
  i32 bgn,       // 0-3
  i32 priority,  // 0 (front) - 3 (back)
  i32 tilestart, // 0-3
  i32 mosaic,    // 0 (disable) - 1 (enable)
  i32 color256,  // 0 (disable) - 1 (enable)
  i32 mapstart,  // 0-31
  i32 wrap,      // 0 (disable) - 1 (enable)
  i32 size       // SYS_BGT_SIZE_* or SYS_BGS_SIZE_*
) {
  volatile u16 *cnt = &REG_BG0CNT + bgn;
  *cnt =
    ((priority  &  3) <<  0) |
    ((tilestart &  3) <<  2) |
    ((mosaic    &  1) <<  6) |
    ((color256  &  1) <<  7) |
    ((mapstart  & 31) <<  8) |
    ((wrap      &  1) << 13) |
    ((size      &  3) << 14);
}

static inline void sys_copy_tiles(
  u32 tilestart, // matching sys_set_bg_config
  u32 offset,
  const void *src,
  u32 size       // bytes
) {
  memcpy32(((void *)0x06000000) + tilestart * 0x4000 + offset, src, size);
}

static inline void sys_copy_map(
  u32 mapstart, // matching sys_set_bg_config
  u32 offset,
  const void *src,
  u32 size      // bytes
) {
  memcpy32(((void *)0x06000000) + mapstart * 0x800 + offset, src, size);
}

static inline void sys_copy_bgpal(
  u32 start, // entry to start at, 0-254
  const void *src,
  u32 size   // bytes
) {
  memcpy32(((void *)0x05000000) + start * 2, src, size);
}

static inline void sys_copy_spritepal(
  u32 start, // entry to start at, 0-254
  const void *src,
  u32 size   // bytes
) {
  memcpy32(((void *)0x05000200) + start * 2, src, size);
}

static inline void sys_set_bgs2_scroll(i32 x, i32 y) {
  REG_BG2X = x;
  REG_BG2Y = y;
}

static inline void sys_set_bgs3_scroll(i32 x, i32 y) {
  REG_BG3X = x;
  REG_BG3Y = y;
}

static inline u16 sys_input() {
  return REG_KEYINPUT;
}

static inline void sys_copy_oam(u16 *oam) {
  memcpy32((void *)0x07000000, oam, 0x400);
}

#endif // SYS_GBA

#if defined(SYS_SDL)

#define SECTION_EWRAM
#define SECTION_IWRAM_ARM

void memcpy32(u8 *dest, const u8 *src, u32 bytecount);
void memcpy16(u8 *dest, const u8 *src, u32 bytecount);
void memcpy8(u8 *dest, const u8 *src, u32 bytecount);
void memset32(u8 *dest, u32 data, u32 bytecount);
void memset16(u8 *dest, u32 data, u32 bytecount);
void memset8(u8 *dest, u32 data, u32 bytecount);

void sys_pset_1f(i32 x, i32 y, u16 color);
void sys_set_bg_config(
  i32 bgn,       // 0-3
  i32 priority,  // 0 (front) - 3 (back)
  i32 tilestart, // 0-3
  i32 mosaic,    // 0 (disable) - 1 (enable)
  i32 color256,  // 0 (disable) - 1 (enable)
  i32 mapstart,  // 0-31
  i32 wrap,      // 0 (disable) - 1 (enable)
  i32 size       // SYS_BGT_SIZE_* or SYS_BGS_SIZE_*
);
void sys_copy_tiles(
  u32 tilestart, // matching sys_set_bg_config
  u32 offset,
  const void *src,
  u32 size       // bytes
);
void sys_copy_map(
  u32 mapstart,  // matching sys_set_bg_config
  u32 offset,
  const void *src,
  u32 size       // bytes
);
void sys_copy_bgpal(
  u32 start, // entry to start at, 0-254
  const void *src,
  u32 size   // bytes
);
void sys_copy_spritepal(
  u32 start, // entry to start at, 0-254
  const void *src,
  u32 size   // bytes
);
void sys_set_bgs2_scroll(i32 x, i32 y);
void sys_set_bgs3_scroll(i32 x, i32 y);
u16 sys_input();
void sys_copy_oam(u16 *oam);

#endif // SYS_SDL
