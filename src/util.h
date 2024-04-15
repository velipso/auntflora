//
// auntflora - Port of Aunt Flora's Mansion to Game Boy Advance
// by Sean Connelly (@velipso), https://sean.cm
// Project Home: https://github.com/velipso/auntflora
// SPDX-License-Identifier: 0BSD
//

#pragma once
#include "sys.h"

static inline void advance_pt(int *x, int *y, int dir, int amt) {
  switch (dir) {
    case 0: *y = *y - amt; return; // UP
    case 1: *x = *x + amt; return; // RIGHT
    case 2: *y = *y + amt; return; // DOWN
    case 3: *x = *x - amt; return; // LEFT
  }
}
