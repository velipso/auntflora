//
// auntflora - Port of Aunt Flora's Mansion to Game Boy Advance
// by Sean Connelly (@velipso), https://sean.cm
// Project Home: https://github.com/velipso/auntflora
// SPDX-License-Identifier: 0BSD
//

#pragma once

#include <stdint.h>
#include <stddef.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;

extern void gvmain();

#define SYS_SCREEN_MODE_4T     0  // 4 text
#define SYS_SCREEN_MODE_2T1S   1  // 2 text, 1 scaling, arbitrary scaling
#define SYS_SCREEN_MODE_2S     2  // 2 scaling, arbitrary scaling
#define SYS_SCREEN_MODE_2S6X6  3  // 2 scaling, targeting 6x6 tiles
#define SYS_SCREEN_MODE_2S5X5  4  // 2 scaling, targeting 5x5 tiles
#define SYS_SCREEN_MODE_1F     5  // 1 full-color, arbitrary scaling
#define SYS_SCREEN_MODE_2F     6  // 2 full-color, arbitrary scaling
#define SYS_SCREEN_MODE_2I     7  // 2 indexed (256 color)

void sys_init();
void sys_set_sprite_enable(i32 enable);
void sys_set_screen_enable(i32 enable);
void sys_set_screen_mode(i32 mode);
void sys_set_vblank(void (*irq_vblank_handler)());
void sys_nextframe();

static inline u16 RGB15(u16 r, u16 g, u16 b) {
  return (r & 0x1F) | ((g & 0x1F) << 5) | ((b & 0x1F) << 10);
}

#if defined(SYS_GBA)

#define SECTION_EWRAM      __attribute__((section(".ewram")))
#define SECTION_IWRAM_ARM  __attribute__((section(".iwram"), target("arm")))

#define sys_pset_1f(x, y, c)  ((uint16_t *)0x06000000)[(x) + (y) * 240] = (c)

#endif // SYS_GBA

#if defined(SYS_SDL)

#define SECTION_EWRAM
#define SECTION_IWRAM_ARM

void sys_pset_1f(i32 x, i32 y, u16 color);

#endif // SYS_SDL
