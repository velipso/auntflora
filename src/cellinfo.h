//
// auntflora - Port of Aunt Flora's Mansion to Game Boy Advance
// by Sean Connelly (@velipso), https://sean.fun
// Project Home: https://github.com/velipso/auntflora
// SPDX-License-Identifier: 0BSD
//

#pragma once
#include "sys.h"

extern const u16 g_cellinfo[];

static inline bool is_solid_static(int cell) {
  // cells are marked as statically solid if they're walls, trees, fountains, etc...
  // things that never move
  return (cell & 0x8000) != 0;
}

static inline bool is_solid(int cell) {
  return is_solid_static(cell) || (g_cellinfo[cell & 0x1f] & 0x8000);
}

static inline bool is_forward(int cell, int dir) {
  return g_cellinfo[cell] == (0x0100 | dir);
}

static inline bool is_checkpoint(int cell) {
  return g_cellinfo[cell] == 0x0200;
}

static inline bool is_pusher(int cell, int dir) {
  return g_cellinfo[cell] == (0x0300 | dir);
}

static inline bool is_openbox(int cell) {
  return g_cellinfo[cell] == 0x0400;
}

static inline bool is_closedbox(int cell) {
  return g_cellinfo[cell] == 0x0500;
}

static inline bool is_pushable(int cell, int dir) {
  dir = 1 << dir;
  return (g_cellinfo[cell] & (0xfff0 | dir)) == (0x0600 | dir);
}

static inline bool is_empty(int cell) {
  return g_cellinfo[cell] == 0;
}

static inline bool is_empty_for_player(int cell) {
  return is_empty(cell) || is_openbox(cell) || is_checkpoint(cell);
}

static inline bool is_empty_for_block(int cell) {
  return is_empty(cell);
}

static inline bool is_pushable_by_player(int cell, int dir) {
  return is_pushable(cell, dir);
}

static inline bool is_pushable_by_block(int cell, int dir) {
  return is_pushable(cell, dir) || is_openbox(cell) || is_closedbox(cell);
}

