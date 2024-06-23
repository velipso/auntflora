//
// auntflora - Port of Aunt Flora's Mansion to Game Boy Advance
// by Sean Connelly (@velipso), https://sean.fun
// Project Home: https://github.com/velipso/auntflora
// SPDX-License-Identifier: 0BSD
//

#include <stdlib.h>
#include <stdint.h>

struct song_sequence_st {
  int *seq;
  int loop;
  int exit;
};

struct song_instrument_st {
  int wave;
  int phase; // 0 = reset, 1 = continue
  struct song_sequence_st volume;
  struct song_sequence_st pitch;
};

struct song_sample_st {
  int index;
  int volume;
};

struct song_patternline_st {
  uint16_t commands[16];
  int wait;
};

struct song_pattern_st {
  struct song_patternline_st **lines;
};

struct song_st {
  int channel_count;
  struct song_instrument_st **instruments;
  struct song_sample_st **samples;
  struct song_pattern_st **patterns;
  struct song_sequence_st **sequences;
};

typedef size_t (*song_serialize_write)(const void *ptr, size_t size, size_t nitems, void *stream);

void song_free(struct song_st *song);
struct song_st *song_new(const char *input, const char *names_file);
void song_print(const struct song_st *song);
void song_serialize(
  struct song_st *song,
  song_serialize_write f_write,
  void *stream
);
