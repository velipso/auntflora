//
// auntflora - Port of Aunt Flora's Mansion to Game Boy Advance
// by Sean Connelly (@velipso), https://sean.cm
// Project Home: https://github.com/velipso/auntflora
// SPDX-License-Identifier: 0BSD
//

#pragma once
#include "sys.h"

#define UNDO_SIZE  (1 << 13)

extern struct undo_st {
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
} g_undo;
extern int g_dirty;
extern int g_checkpoint;

void undo_finish();
void undo_fire();
void checkpoint_save();
bool checkpoint_restore();
void write_logic(int x, int y, int data);
void write_player(int x, int y, int dir, int message);
