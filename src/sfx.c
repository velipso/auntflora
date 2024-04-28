//
// auntflora - Port of Aunt Flora's Mansion to Game Boy Advance
// by Sean Connelly (@velipso), https://sean.cm
// Project Home: https://github.com/velipso/auntflora
// SPDX-License-Identifier: 0BSD
//

#include "sfx.h"

void sfx_walk() {
  // nothing
}

void sfx_push() {
  static int sfx = -2;
  if (sfx == -2)
    sfx = snd_find_wav("push");
  snd_play_wav(sfx, 10, 10);
}

void sfx_forward() {
  static int sfx = -2;
  if (sfx == -2)
    sfx = snd_find_wav("teleport");
  snd_play_wav(sfx, 16, 10);
}

void sfx_bump() {
  static int sfx = -2;
  if (sfx == -2)
    sfx = snd_find_wav("bump");
  snd_play_wav(sfx, 16, 10);
}

void sfx_splash() {
  static int sfx = -2;
  if (sfx == -2)
    sfx = snd_find_wav("splash");
  snd_play_wav(sfx, 16, 10);
}

void sfx_click() {
  static int sfx = -2;
  if (sfx == -2)
    sfx = snd_find_wav("click");
  snd_play_wav(sfx, 8, 10);
}

void sfx_end() {
  static int sfx = -2;
  if (sfx == -2)
    sfx = snd_find_wav("end");
  snd_play_wav(sfx, 16, 10);
}

void sfx_checkpoint() {
  static int sfx = -2;
  if (sfx == -2)
    sfx = snd_find_wav("checkpoint");
  snd_play_wav(sfx, 16, 10);
}
