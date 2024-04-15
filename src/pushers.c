//
// auntflora - Port of Aunt Flora's Mansion to Game Boy Advance
// by Sean Connelly (@velipso), https://sean.cm
// Project Home: https://github.com/velipso/auntflora
// SPDX-License-Identifier: 0BSD
//

#include "sys.h"
#include "main.h"
#include "util.h"
#include "cellinfo.h"
#include "snd.h"
#include "undo.h"

#define PUSHERS_SIZE  32
struct {
  int x;
  int y;
  int dir;
} g_pushers[PUSHERS_SIZE];
int g_pushers_size;

#if 0
#include "ani.h"
#include "anidata.h"
static void blink_tile(int x, int y) {
  for (int blink = 0; blink < 4; blink++) {
    g_sprites[3].pc = (blink & 1) ? NULL : ani_cat;
    g_sprites[3].origin.x = (x - g_viewport.wx) * 12 - 32;
    g_sprites[3].origin.y = (y - g_viewport.wy) * 12 - 18;
    nextframe();nextframe();
  }
}
#else
#define blink_tile(x, y)
#endif

#if 0
#include "ani.h"
#include "anidata.h"
static void pushers_mark() {
  for (int i = 0; i < 128 - 3; i++) {
    if (i < g_pushers_size) {
      g_sprites[i + 3].pc = ani_cat;
      g_sprites[i + 3].origin.x = (g_pushers[i].x - g_viewport.wx) * 12 - 32;
      g_sprites[i + 3].origin.y = (g_pushers[i].y - g_viewport.wy) * 12 - 18;
    } else {
      g_sprites[i + 3].pc = NULL;
    }
  }
  nextframe();
}
#else
#define pushers_mark()
#endif

void pushers_find(int x, int y, int dir) {
  int dir2 = (dir + 2) & 3; // rotate by 180
  while (1) {
    advance_pt(&x, &y, dir, 1);
    u16 nc = world_at(x, y);
    blink_tile(x, y);
    if (is_pusher(nc, dir2)) {
      for (int i = 0; i < g_pushers_size; i++) {
        if (g_pushers[i].x == x && g_pushers[i].y == y) {
          // already found
          return;
        }
      }
      if (g_pushers_size < PUSHERS_SIZE) {
        g_pushers[g_pushers_size].x = x;
        g_pushers[g_pushers_size].y = y;
        g_pushers[g_pushers_size].dir = dir2;
        g_pushers_size++;
      }
      return;
    } else if (!is_pushable_by_block(nc, dir2)) {
      return;
    }
  }
}

void pushers_find_around_player(int movedir) {
  int x = g_markers[0].x;
  int y = g_markers[0].y;
  for (int d = 0; d < 4; d++) {
    if (d == movedir)
      continue;
    pushers_find(x, y, d);
  }
}

void pushers_apply() {
  while (1) {
    pushers_mark();
    int dirty = 0;
    int gs = g_pushers_size; // cache because we can add more as we go
    for (int i = 0; i < gs; i++) {
      int nx = g_pushers[i].x;
      int ny = g_pushers[i].y;
      int dir = g_pushers[i].dir;
      int px = nx;
      int py = ny;
      while (1) {
        advance_pt(&px, &py, dir, 1);
        if (g_markers[0].x == px && g_markers[0].y == py) {
          // can't push player
          break;
        }
        u16 nc = world_at(px, py);
        if (is_empty_for_block(nc)) {
          if (!dirty) {
            move_screen_to_player();
            copy_world_offset();
            delay_push();
            snd_push();
            dirty = 1;
          }
          int hx, hy;
          while (px != nx || py != ny) {
            hx = px;
            hy = py;
            advance_pt(&px, &py, dir, -1);
            write_logic(hx, hy, world_at(px, py));
          }
          write_logic(nx, ny, 0);
          advance_pt(&g_pushers[i].x, &g_pushers[i].y, dir, 1);
          // find pushers perpendicular to this one
          pushers_find(nx, ny, (dir + 1) & 3);
          pushers_find(nx, ny, (dir + 3) & 3);
          break;
        } else if (is_pushable_by_block(nc, dir)) {
          // pusher could still push
          continue;
        } else {
          // pusher is blocked
          break;
        }
      }
    }
    if (!dirty)
      break;
  }
}
