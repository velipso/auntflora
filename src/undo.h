//
// auntflora - Port of Aunt Flora's Mansion to Game Boy Advance
// by Sean Connelly (@velipso), https://sean.fun
// Project Home: https://github.com/velipso/auntflora
// SPDX-License-Identifier: 0BSD
//

#pragma once
#include "sys.h"

#define UNDO_SIZE  (1 << 13)

extern int g_dirty;

void undo_init();
void undo_finish();
bool undo_fire();
void checkpoint_save();
bool checkpoint_restore();
void write_logic(int x, int y, int data);
void write_player(int x, int y, int dir, int message);
