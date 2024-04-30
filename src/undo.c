//
// auntflora - Port of Aunt Flora's Mansion to Game Boy Advance
// by Sean Connelly (@velipso), https://sean.cm
// Project Home: https://github.com/velipso/auntflora
// SPDX-License-Identifier: 0BSD
//

#include "undo.h"
#include "main.h"
#include "util.h"
#include "cellinfo.h"

int g_dirty = 0;
static struct {
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
} g_undo SECTION_EWRAM = {0};
static int g_checkpoint = -1;

void undo_init() {
  g_undo.head = 1;
  g_undo.tail = 0;
  g_undo.entries[0] = 0x80000000;
}

static void undo_push(u32 entry) {
  g_undo.entries[g_undo.head] = entry;
  g_undo.head++;
  g_undo.head &= UNDO_SIZE - 1;
  if (g_undo.head == g_undo.tail) {
    g_undo.tail++;
    g_undo.tail &= UNDO_SIZE - 1;
    if (g_undo.tail == g_checkpoint)
      g_checkpoint = -1; // out of memory, lost checkpoint :-(
  }
}

void undo_finish() {
  int prev = g_undo.head;
  if (prev == 0)
    prev = UNDO_SIZE - 1;
  else
    prev--;
  g_undo.entries[prev] |= 0x80000000;
}

bool undo_fire() {
  if (g_undo.head == g_undo.tail) {
    // can't rewind further, out of undo data!
    return false;
  }
  // rewind head until finalized entry is found
  int final = (g_undo.head - 1) & (UNDO_SIZE - 1);
  int count = 0;
  while (1) {
    if (g_undo.entries[final] & 0x80000000) {
      count++;
      if (count == 2)
        break;
    }
    if (final == g_undo.tail) {
      // can't rewind further, out of undo data!
      return false;
    }
    final--;
    final &= UNDO_SIZE - 1;
  }
  final = (final + 1) & (UNDO_SIZE - 1);
  int here = g_undo.head;
  while (here != final) {
    if (here == g_checkpoint)
      g_checkpoint = -1; // remove checkpoint if undo prior to it
    here--;
    here &= UNDO_SIZE - 1;

    u32 entry = g_undo.entries[here];
    if (entry == 0x80000000) // initial entry
      continue;
    int x = (entry >> 12) & 0x1ff;
    int y = (entry >> 21) & 0x1ff;
    int payload = entry & 0xfff;
    if (entry & 0x40000000) {
      // player write
      int olddir = (payload >> 8) & 3;
      int newdir = (payload >> 10) & 3;
      int msg = payload & 0xff;
      advance_pt(&x, &y, newdir, -1);
      if (is_forward(world_at(x, y), newdir)) {
        // undo past a forwarder
        advance_pt(&x, &y, newdir, -1);
      }
      set_player_ani_dir(olddir);
      if (
        g_markers[0].x != x ||
        g_markers[0].y != y
      ) {
        g_markers[0].x = x;
        g_markers[0].y = y;
        // cut the player some slack and don't count undo's towards total steps
        g_total_steps--;
      }
      if (msg >= 0 && msg < MAX_MARKERS)
        g_seen_marker[msg] = 0;
    } else {
      // logic write
      int oldlogic = payload & 0x3f;
      int k = x + y * g_world.width;
      g_world.data[k] = oldlogic;
    }
  }
  g_undo.head = here;
  return true;
}

void checkpoint_save() {
  g_checkpoint = g_undo.head;
}

bool checkpoint_restore() {
  if (g_checkpoint < 0)
    return false;
  while (g_undo.head != g_checkpoint)
    undo_fire();
  return true;
}

void write_logic(int x, int y, int data) {
  g_dirty = 1;
  int k = x + y * g_world.width;
  int old = g_world.data[k];
  g_world.data[k] = data;
  undo_push((y << 21) | (x << 12) | (data << 6) | old);
}

void write_player(int x, int y, int dir, int message) {
  g_dirty = 1;
  int old = g_playerdir;
  if (
    g_markers[0].x != x ||
    g_markers[0].y != y
  ) {
    g_markers[0].x = x;
    g_markers[0].y = y;
    g_total_steps++;
  }
  if (message >= 0 && message < MAX_MARKERS) {
    if (g_seen_marker[message])
      message = -1;
    else
      g_seen_marker[message] = 1;
  }
  set_player_ani_dir(dir);
  undo_push(0x40000000 | (y << 21) | (x << 12) | (dir << 10) | (old << 8) | (message & 0xff));
}
