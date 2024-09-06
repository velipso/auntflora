//
// auntflora - Port of Aunt Flora's Mansion to Game Boy Advance
// by Sean Connelly (@velipso), https://sean.fun
// Project Home: https://github.com/velipso/auntflora
// SPDX-License-Identifier: 0BSD
//

#include "gba.h"
#include "reg.h"

extern void sys__irq_init();
extern void (*sys__irq_vblank)();
extern void (*sys__irq_timer1)();
extern void sys__snd_timer1_handler();
extern void sys__snd_frame();

struct snd_st g_snd;

static void (*g_vblank)();

static void _sys_snd_init();

BINFILE(snd_tempo_bin);
BINFILE(snd_wavs_bin);
BINFILE(snd_offsets_bin);
BINFILE(snd_sizes_bin);
BINFILE(snd_names_txt);

void sys_init() {
  sys__irq_init();
  _sys_snd_init();
}

bool sys_mGBA() {
  volatile u16 *reg = (u16 *)0x4fff780;
  *reg = 0xc0de;
  return *reg == 0x1dea;
}

static void _sys_wrap_vblank() {
  // allow re-entrant IRQs so timer1 for snd is handled
  REG_IME = 1;
  if (g_vblank)
    g_vblank();
  sys__snd_frame();
}

void sys_set_vblank(void (*irq_vblank_handler)()) {
  g_vblank = irq_vblank_handler;
}

void sys_nextframe() {
  __asm__("swi #5" ::: "r0", "r1", "r2", "r3", "r12", "lr", "memory", "cc");
}

static void _sys_snd_init() {
  REG_IME = 0;
  // enable vblank handler
  REG_DISPSTAT |= 0x0008;
  REG_IE |= 1;
  g_vblank = NULL;
  sys__irq_vblank = _sys_wrap_vblank;
  // enable timer1 handler
  sys__irq_timer1 = sys__snd_timer1_handler;
  // clear struct
  memset32(&g_snd, 0, sizeof(g_snd));
  // enable timer1
  REG_IE |= 0x10;
  // setup timers
  REG_TM0CNT = 0;
  REG_TM1CNT = 0;
  // setup timer0 - drives the sample rate
  // cpuHz      = 2^24
  // sampleRate = 2^15
  // timer0Wait = cpuHz / sampleRate = 2^9 = 0x200
  // timer0Res  = 1 cycle
  // timer0Wait / timer0Res = 0x200
  REG_TM0D = 0x10000 - 0x200;
  // setup timer1 - drives the DMA buffer cycling
  // bufferSize = 0x260
  // timer1Wait = bufferSize * timer0Wait = 311296 cycles
  // timer1Res  = 64 cycles
  // timer1Wait / timer1Res = 0x1300
  REG_TM1D = 0x10000 - 0x1300;
  REG_IME = 1;
  // turn sound chip on
  REG_SOUNDCNT_X = 0x0080;
  // set sound to use FIFO A
  REG_SOUNDCNT_H = 0x0b0f;
  // set DMA1 destination to FIFO A
  REG_DMA1DAD = (u32)&REG_FIFO_A;
  // point DMA1 to buffer1
  REG_DMA1SAD = (u32)g_snd.buffer1;
  // enable DMA1
  REG_DMA1CNT_H = 0xb640;
  // save alt buffers for next render
  g_snd.buffer_addr[0] = g_snd.buffer1;
  g_snd.buffer_addr[1] = g_snd.buffer2;
  g_snd.buffer_addr[2] = g_snd.buffer3;
  g_snd.next_buffer_index = 4;
  // start timer0
  REG_TM0CNT = 0x0080;
  // start timer1
  REG_TM1CNT = 0x00c1;
}

void snd_load_song(const void *song_base, int sequence) {
  int volume = g_snd.synth.volume;
  memset32(&g_snd.synth, 0, sizeof(struct snd_synth_st));
  g_snd.synth.volume = volume;
  g_snd.synth.song_base = song_base;
  g_snd.synth.sequence = sequence;
  // set pattern pointer to sequence's first pattern
  const struct snd_song_st *song = song_base;
  int seq_offset = *((int *)(song_base + song->seq_table_offset + sequence * 4));
  const struct snd_songseq_st *songseq = song_base + seq_offset;
  u16 patterns = songseq->patterns[0];
  u32 pat_offset = *((u32 *)(song_base + song->pat_table_offset + patterns * 4));
  g_snd.synth.pat = song_base + pat_offset;

  // set tempo to index 0
  const u16 *tempo_table = BINADDR(snd_tempo_bin);
  g_snd.synth.tick_start = tempo_table[0];
  g_snd.synth.tick_left = 0;

  // initialize channel volume
  for (int i = 0; i < SND_MAX_CHANNELS; i++)
    g_snd.synth.channel[i].chan_volume = i < song->channel_count ? 8 : 0;
}

void snd_set_master_volume(int v) {
  g_snd.master_volume = v < 0 ? 0 : v > 16 ? 16 : v;
}

void snd_set_song_volume(int v) {
  g_snd.synth.volume = v < 0 ? 0 : v > 16 ? 16 : v;
}

void snd_set_sfx_volume(int v) {
  g_snd.sfx_volume = v < 0 ? 0 : v > 16 ? 16 : v;
}

int snd_find_wav(const char *name) {
  const char *start = BINADDR(snd_names_txt);
  int size = BINSIZE(snd_names_txt);
  int state = 0;
  int index = 0;
  int m = 0;
  int matching = 0;
  for (int i = 0; i < size; i++) {
    char ch = start[i];
    if (ch == 0)
      return -1;
    switch (state) {
      case 0:
        if (ch != '\n') {
          matching = ch == name[0];
          m = 1;
          state = 1;
        }
        break;
      case 1:
        if (ch == '\n') {
          if (matching && name[m] == 0)
            return index;
          index++;
          state = 0;
        } else if (matching) {
          matching = ch == name[m++];
        }
        break;
    }
  }
  return -1;
}

bool snd_play_wav(int wav_index, int volume, int priority) {
  if (wav_index < 0)
    return false;
  // look for either an empty slot or the lowest priority slot
  int slot;
  int best_slot = 0;
  int best_priority = g_snd.sfx[0].priority;
  for (slot = 0; slot < SND_MAX_SFX; slot++) {
    if (!g_snd.sfx[slot].wav_base)
      goto place_in_slot;
    if (g_snd.sfx[slot].priority < best_priority) {
      best_priority = g_snd.sfx[slot].priority;
      best_slot = slot;
    }
  }
  if (priority <= best_priority)
    return false;
  slot = best_slot;
place_in_slot:
  const u8 *wavs = BINADDR(snd_wavs_bin);
  const u32 *offsets = BINADDR(snd_offsets_bin);
  const u32 *sizes = BINADDR(snd_sizes_bin);
  g_snd.sfx[slot].wav_base = &wavs[offsets[wav_index]];
  g_snd.sfx[slot].wav_volume = volume;
  g_snd.sfx[slot].samples_left = sizes[wav_index];
  g_snd.sfx[slot].priority = priority;
  return true;
}
