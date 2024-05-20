//
// auntflora - Port of Aunt Flora's Mansion to Game Boy Advance
// by Sean Connelly (@velipso), https://sean.cm
// Project Home: https://github.com/velipso/auntflora
// SPDX-License-Identifier: 0BSD
//

#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

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
#define BINADDR(n)  ((const void *)&_binary_ ## n ## _start)
#define BINSIZE(n)  ((u32)&_binary_ ## n ## _size)

enum gfx_mode {
  GFX_MODE_4T,    // 4 text
  GFX_MODE_2T1S,  // 2 text, 1 scaling, arbitrary scaling
  GFX_MODE_2S,    // 2 scaling, arbitrary scaling
  GFX_MODE_2S6X6, // 2 scaling, targeting 6x6 tiles
  GFX_MODE_2S5X5, // 2 scaling, targeting 5x5 tiles
  GFX_MODE_1F,    // 1 full-color, arbitrary scaling
  GFX_MODE_2F,    // 2 full-color, arbitrary scaling
  GFX_MODE_2I     // 2 indexed (256 color)
};
