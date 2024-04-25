//
// auntflora - Port of Aunt Flora's Mansion to Game Boy Advance
// by Sean Connelly (@velipso), https://sean.cm
// Project Home: https://github.com/velipso/auntflora
// SPDX-License-Identifier: 0BSD
//

#pragma once
#include "../sys.h"

#define SND_MAX_CHANNELS  6
#define SND_BUFFER_SIZE   608

struct snd_song_st {
  int magic;
  u8 version;
  u8 channel_count;
  u8 reserved;
  u8 samps_length;
  u8 insts_length;
  u8 seqs_length;
  u16 pats_length;
  int inst_table_offset;
  int seq_table_offset;
  int pat_table_offset;
  u16 samp_table[1]; // variable length array
};

struct snd_songinst_st {
  u16 wave;
  char volume_env_attack;
  char volume_env_sustain;
  char volume_env_length;
  char pitch_env_attack;
  char pitch_env_sustain;
  char pitch_env_length;
  u16 volume_env_offset;
  u16 pitch_env_offset;
};

struct snd_songseq_st {
  u16 pat_length;
  u16 loop_index;
  u16 exit;
  u16 patterns[1]; // variable length array
};

struct snd_channel_st {
  int state; // 0 off, 1 note released, 2 note on
  int delay;
  int delayed_note_on_left;
  int delayed_note_on_note;
  int delayed_note_off_left;
  int delayed_bend_left;
  int delayed_bend_duration;
  int delayed_bend_note;
  int chan_volume;
  int env_volume_index;
  int base_pitch;
  int target_pitch;
  int bend_counter;
  int bend_counter_max;
  int env_pitch_index;
  int phase;
  #if defined(__GBA__)
  int *inst_base;
  #else
  int inst_base; // force pointers to take up 32-bits when using xform on 64-bit machines
  #endif
};

struct snd_synth_st {
  #if defined(__GBA__)
  void *song_base;
  #else
  int song_base;
  #endif
  int sequence;
  int tempo_index;
  int tick_start;
  int tick_left;
  int seq_index;
  #if defined(__GBA__)
  void *pat;
  #else
  int pat;
  #endif
  int volume; // 0-16
  struct snd_channel_st channel[SND_MAX_CHANNELS];
};

struct snd_st {
  struct snd_synth_st synth;

  // buffer_addr[0] points to the DMA source
  // buffer_addr[1] is the DMA source on deck
  // buffer_addr[2] is after that
  #if defined(__GBA__)
  void *buffer_addr[3];
  #else
  int buffer_addr[3];
  #endif
  int next_buffer_index;
  char buffer1[SND_BUFFER_SIZE];
  char buffer2[SND_BUFFER_SIZE];
  char buffer3[SND_BUFFER_SIZE];
  char buffer_temp[SND_BUFFER_SIZE * 2]; // u16
  int master_volume; // 0-16
};

extern struct snd_st g_snd;
