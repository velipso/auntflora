//
// auntflora - Port of Aunt Flora's Mansion to Game Boy Advance
// by Sean Connelly (@velipso), https://sean.cm
// Project Home: https://github.com/velipso/auntflora
// SPDX-License-Identifier: 0BSD
//

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "snd_ds1.h"
#include "snd_ds2.h"
#include "song.h"
#include "tinydir.h"
#include "stb_ds.h"

#pragma pack(push, 4)  // align to 4 byte boundaries
#include "../sys/gba/gba.h"
#pragma pack(pop)

#define DUTY_COUNT        8
#define MAX_PHASE_BITS    11
#define MAX_PHASE         (1 << MAX_PHASE_BITS)
#define PITCH_DIV_BITS    4 /* how many freqs between notes (2^n) */
#define MAX_PHASE_Q_BITS  (MAX_PHASE_BITS + 5)
#define MAX_RND_SAMPLE    (1 << 15)
#define PI                3.14159265358979323846

// align files to 4 bytes... required to keep linker in alignment (???)
static void fclose4(FILE *fp) {
  long pos = ftell(fp);
  int pad = 4 - (pos % 4);
  for (int i = 0; i < pad; i++)
    fputc(0, fp);
  fclose(fp);
}

static void snd_print_usage() {
  printf(
    "Usage:\n"
    "  snd <command> [options...]\n"
    "\n"
    "Commands:\n"
    "\n"
    "  tables <osc.bin> <tempo.bin> <slice.bin> <dphase.bin> <bend.bin>\n"
    "    Generate the lookup tables\n"
    "      osc.bin    - Raw oscillator sample data\n"
    "      tempo.bin  - Cursor step per tempo\n"
    "      slice.bin  - Oscillator lookup per note\n"
    "      dphase.bin - Delta phase per pitch\n"
    "      bend.bin   - How much to bend each frame to hit target length\n"
    "\n"
    "  wav <wav/directory> <waves.bin> <offsets.bin> <sizes.bin> <names.txt>\n"
    "    Consolidate wav files and output metadata\n"
    "      waves.bin   - Raw 16-bit sample data\n"
    "      offsets.bin - Location where each wave starts\n"
    "      sizes.bin   - Number of samples per wave\n"
    "      names.txt   - Name per wave, used in `makesong` command\n"
    "\n"
    "  gbaoffsets\n"
    "    Autogenerate offsets of structs for assembly\n"
    "\n"
    "  makesong <input.txt> <names.txt> <output.bin>\n"
    "    Compile a song into a binary file for embedding\n"
  );
}

// copied from: https://github.com/velipso/whisky
static uint32_t whisky1(uint32_t i0){
  uint32_t z0 = (i0 * 1831267127) ^ i0;
  uint32_t z1 = (z0 * 3915839201) ^ (z0 >> 20);
  uint32_t z2 = (z1 * 1561867961) ^ (z1 >> 24);
  return z2;
}

static uint32_t whisky1alt(uint32_t i0){
  uint32_t z0 = (i0 * 1850913749) ^ i0;
  uint32_t z1 = (z0 *  499907189) ^ (z0 >> 14);
  uint32_t z2 = (z1 *  237314647) ^ (z1 >> 24);
  return z2;
}

static double whiskyd(uint32_t a, uint32_t b){
  uint64_t m = a;
  m = (m << 20) | (b >> 12);
  union { uint64_t i; double d; } u = { .i = (UINT64_C(0x3FF) << 52) | m };
  return u.d - 1.0;
}

static double whisky1d(uint32_t i0){
  return whiskyd(whisky1(i0), whisky1alt(i0));
}

static void snd_generate_oscillator(
  double *real,
  double *imag,
  int lowpass,    // 0-9
  double duty,    // 0-1
  double square,  // 0-1
  double sine,    // 0-1
  double saw,     // 0-1
  double triangle // 0-1
) {
  for (int i = 0; i < 1024; i++) {
    real[i] = 0;
    imag[i] = 0;
  }
  if (duty < 0.5)
    duty = 1.0 - duty;
  // v1: https://www.desmos.com/calculator/jztbyinfbz  (equal area square wave)
  // v2: https://www.desmos.com/calculator/adyrewgqco  (max amplitude square wave)

  // use the lowpass variable to cap the number of harmonics
  // this is the standard way to implement a lowpass filter with additive synthesis
  int max = 1024.0 * pow(2.0, -1.0 * lowpass);
  for (int n = 1; n < max; n++) {
    double n_pi = n * PI;
    double pi_factor = 2.0 / n_pi;
    double b_sine = n == 1 ? 1.0 : 0;
    double a_square = pi_factor * sin(duty * 2.0 * n_pi);
    double sinterm = sin(duty * n_pi);
    double b_square = pi_factor * 2.0 * sinterm * sinterm;
    double b_saw = pi_factor * ((n % 2) == 1 ? 1.0 : -1.0);
    double b_triangle = ((n % 2) == 1)
      ? 2.0 * (pi_factor * pi_factor) * (
        (((n - 1) / 2) % 2) == 1 ? -1.0 : 1.0
      )
      : 0;
    real[n] = square * a_square;
    imag[n] = square * b_square +
      sine * b_sine +
      saw * b_saw +
      triangle * b_triangle;
  }
  // DC offset for square -- if left in, there is a lot of low frequency content in square waves
  // real[0] = (2 * duty - 1) * square
}

static double *snd_push_realimag(
  double *list,
  const double *real,
  const double *imag,
  int length
) {
  for (int x = 0; x < MAX_PHASE; x++) {
    double v = 0;
    double xp = x * PI * 2.0 / MAX_PHASE;
    for (int i = 0; i < length; i++) {
      v +=
        (imag ? imag[i] : 0) * sin(xp * i) +
        (real ? real[i] : 0) * cos(xp * i);
    }
    arrpush(list, v);
  }
  return list;
}

static double *snd_add_wave(
  double *waves,
  const int *lowpass_table,
  int lowpass_table_size,
  double duty,
  double square,
  double sine,
  double saw,
  double triangle
) {
  double real[1024];
  double imag[1024];
  for (int i = 0; i < lowpass_table_size; i++) {
    int lowpass = lowpass_table[i];
    printf(" %d", lowpass);
    snd_generate_oscillator(
      real,
      imag,
      lowpass,
      duty,
      square,
      sine,
      saw,
      triangle
    );
    waves = snd_push_realimag(waves, real, imag, 1024);
  }
  printf("\n");
  return waves;
}

static int snd_tables_osc(const char *output) {
  FILE *fp = fopen(output, "wb");
  if (!fp) {
    fprintf(stderr, "\nFailed to open for writing: %s\n", output);
    return 1;
  }
  printf("Generating: %s\n", output);

  double *waves = NULL;

  printf("Generating: rnd\n");
  for (int i = 1; i <= MAX_RND_SAMPLE; i++) {
    double v = whisky1d(i) * 2.0 - 1.0;
    arrpush(waves, v);
  }

  for (int duty = 0; duty < DUTY_COUNT; duty++) {
    printf("Generating: sq%d\n ", duty + 1);
    // generate multiple waves with different lowpass thresholds applied... by applying a lowpass
    // filter for higher pitched waves, we reduce artifacts... remember that higher pitched waves
    // will zip through the sample data more quickly, so we don't want high frequency data in the
    // sample itself
    int lowpass_table[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    waves = snd_add_wave(
      waves,
      lowpass_table,
      sizeof(lowpass_table) / sizeof(int),
      (duty + 1.0) / (2.0 * DUTY_COUNT),
      1, 0, 0, 0
    );
  }

  {
    printf("Generating: tri\n ");
    int lowpass_table[] = {0, 4, 6, 7, 8, 9};
    waves = snd_add_wave(
      waves,
      lowpass_table,
      sizeof(lowpass_table) / sizeof(int),
      0, 0, 0, 0, 1
    );
  }

  {
    printf("Generating: saw\n ");
    int lowpass_table[] = {0, 4, 5, 6, 7, 8, 9};
    waves = snd_add_wave(
      waves,
      lowpass_table,
      sizeof(lowpass_table) / sizeof(int),
      0, 0, 0, 1, 0
    );
  }

  {
    printf("Generating: sin\n ");
    int lowpass_table[] = {0};
    waves = snd_add_wave(
      waves,
      lowpass_table,
      sizeof(lowpass_table) / sizeof(int),
      0, 0, 1, 0, 0
    );
  }

  {
    printf("Generating: ds1\n ");
    printf(" 4"); waves = snd_push_realimag(waves, NULL, snd_ds1_imag, 64);
    printf(" 5"); waves = snd_push_realimag(waves, NULL, snd_ds1_imag, 32);
    printf(" 6"); waves = snd_push_realimag(waves, NULL, snd_ds1_imag, 16);
    printf(" 7"); waves = snd_push_realimag(waves, NULL, snd_ds1_imag, 8);
    printf(" 8"); waves = snd_push_realimag(waves, NULL, snd_ds1_imag, 4);
    printf(" 9"); waves = snd_push_realimag(waves, NULL, snd_ds1_imag, 2);
    printf("\n");
  }

  {
    printf("Generating: ds2\n ");
    printf(" 4"); waves = snd_push_realimag(waves, NULL, snd_ds2_imag, 64);
    printf(" 5"); waves = snd_push_realimag(waves, NULL, snd_ds2_imag, 32);
    printf(" 6"); waves = snd_push_realimag(waves, NULL, snd_ds2_imag, 16);
    printf(" 7"); waves = snd_push_realimag(waves, NULL, snd_ds2_imag, 8);
    printf(" 8"); waves = snd_push_realimag(waves, NULL, snd_ds2_imag, 4);
    printf(" 9"); waves = snd_push_realimag(waves, NULL, snd_ds2_imag, 2);
    printf("\n");
  }

  uint16_t *data = malloc(sizeof(uint16_t) * arrlen(waves));
  for (int i = 0; i < arrlen(waves); i++) {
    double v = waves[i];
    if (v < -1)
      v = -1;
    if (v > 1)
      v = 1;
    v = v * (v < 0 ? 16384 : 16383);
    if (v < 0)
      v += 65536;
    if (v < 0)
      v = 0;
    if (v >= 65535)
      v = 65535;
    data[i] = v;
  }
  fwrite(data, sizeof(uint16_t), arrlen(waves), fp);
  free(data);
  arrfree(waves);
  fclose4(fp);

  return 0;
}

static int snd_tables_tempo(const char *output) {
  FILE *fp = fopen(output, "wb");
  if (!fp) {
    fprintf(stderr, "\nFailed to open for writing: %s\n", output);
    return 1;
  }
  printf("Generating: %s\n", output);

  uint16_t tempo[64];
  double sample_rate = 32768.0;
  for (int i = 0; i < 64; i++) {
    double ideal_tempo = (i + 18.0) * 10.0 / 4.0;
    double start_value = round(sample_rate * 60.0 * 256.0 / (608.0 * 16.0 * ideal_tempo));
    //double actual_tempo = sample_rate * 60.0 * 256.0 / (608.0 * 16.0 * start_value);
    //printf("Tempo %d: target %g BPM, actual %g BPM\n", i, ideal_tempo, actual_tempo);
    tempo[i] = (uint16_t)start_value;
  }
  fwrite(tempo, sizeof(uint16_t), 64, fp);
  fclose4(fp);

  return 0;
}

static void snd_tables_slice_push(FILE *fp, int offsets[128], int *next_offset) {
  int max_slice = 0;
  for (int i = 0; i < 128; i++) {
    uint16_t v = *next_offset + offsets[i];
    if (offsets[i] > max_slice)
      max_slice = offsets[i];
    fwrite(&v, sizeof(uint16_t), 1, fp);
  }
  *next_offset = *next_offset + max_slice + 1;
}

static int snd_tables_slice(const char *output) {
  FILE *fp = fopen(output, "wb");
  if (!fp) {
    fprintf(stderr, "\nFailed to open for writing: %s\n", output);
    return 1;
  }
  printf("Generating: %s\n", output);

  int next_offset = 16; // account for rnd

  // sqX
  for (int i = 0; i < DUTY_COUNT; i++) {
    // if using sqX instrument and playing note Y, then use lowpass table Z
    // these were found by ear to reduce ugly artifacts, nothing special
    int offsets[128] = {
      0, 0, 0, 0, 0, 0, 0, 0, // lower notes, no lowpass filter on sample data
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
      3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
      4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
      5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
      7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
      8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
      9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, // higher notes, aggressive lowpass filter on sample data
    };
    snd_tables_slice_push(fp, offsets, &next_offset);
  }

  { // tri
    int offsets[128] = {
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
      3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
      4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
      5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    };
    snd_tables_slice_push(fp, offsets, &next_offset);
  }

  { // saw
    int offsets[128] = {
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
      3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
      4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
      5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    };
    snd_tables_slice_push(fp, offsets, &next_offset);
  }

  { // sin
    int offsets[128] = {
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };
    snd_tables_slice_push(fp, offsets, &next_offset);
  }

  { // ds1
    int offsets[128] = {
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
      3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
      4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
      5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    };
    snd_tables_slice_push(fp, offsets, &next_offset);
  }

  { // ds2
    int offsets[128] = {
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
      3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
      4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
      5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    };
    snd_tables_slice_push(fp, offsets, &next_offset);
  }

  fclose4(fp);

  return 0;
}

static int snd_tables_dphase(const char *output) {
  FILE *fp = fopen(output, "wb");
  if (!fp) {
    fprintf(stderr, "\nFailed to open for writing: %s\n", output);
    return 1;
  }
  printf("Generating: %s\n", output);

  double sample_rate = 32768.0;
  double max_phase_q = 1 << MAX_PHASE_Q_BITS;
  double pitch_div = 1 << PITCH_DIV_BITS;

  // for each unique pitch (128 notes * pitch division)
  for (int i = 0; i < 0x80 * pitch_div; i++) {
    // calculate the frequency for this pitch
    double n = i;
    double ideal_freq = 440.0 * pow(2.0, ((n / pitch_div) - 65.0) / 12.0);
    // and the phase delta needed to produce that pitch
    double dphase = round(ideal_freq * max_phase_q / sample_rate);
    //double actual_freq = dphase * sample_rate / max_phase_q;
    //printf("Pitch %d: target %g Hz, actual %g Hz\n", i, ideal_freq, actual_freq);
    if (dphase >= 65536.0) {
      fprintf(stderr, "dphase overflow\n");
      fclose(fp);
      return 1;
    }
    uint16_t d = (uint16_t)dphase;
    fwrite(&d, sizeof(uint16_t), 1, fp);
  }
  fclose4(fp);

  return 0;
}

static int snd_tables_bend(const char *output) {
  FILE *fp = fopen(output, "wb");
  if (!fp) {
    fprintf(stderr, "\nFailed to open for writing: %s\n", output);
    return 1;
  }
  printf("Generating: %s\n", output);

  double sample_rate = 32768.0;
  double pitch_div = 1 << PITCH_DIV_BITS;

  // for each tempo...
  for (int tempo = 0; tempo < 64; tempo++) {
    double ideal_tempo = (tempo + 18.0) * 10.0 / 4.0;
    double start_value = round(sample_rate * 60.0 * 256.0 / (608.0 * 16.0 * ideal_tempo));
    //double actual_tempo = sample_rate * 60.0 * 256.0 / (608.0 * 16.0 * start_value);
    double frames_per_16th_note = start_value / 256.0;

    // if we want to bend to a note dpitch away in a 16th note's length...
    for (int dpitch_abs = 1; dpitch_abs <= 128; dpitch_abs++) {
      // calculate the bend counter max in Q0.16
      double bend_counter_max = floor(65536.0 * frames_per_16th_note / (dpitch_abs * pitch_div));
      if (bend_counter_max <= 0 || bend_counter_max >= 65536.0) {
        fprintf(stderr, "bend overflow\n");
        return 1;
      }
      uint16_t d = (uint16_t)bend_counter_max;
      fwrite(&d, sizeof(uint16_t), 1, fp);
    }
  }
  fclose4(fp);

  return 0;
}

static int snd_tables(
  const char *osc,
  const char *tempo,
  const char *slice,
  const char *dphase,
  const char *bend
) {
  int ret;
  ret = snd_tables_osc(osc);
  if (ret != 0)
    return ret;
  ret = snd_tables_tempo(tempo);
  if (ret != 0)
    return ret;
  ret = snd_tables_slice(slice);
  if (ret != 0)
    return ret;
  ret = snd_tables_dphase(dphase);
  if (ret != 0)
    return ret;
  ret = snd_tables_bend(bend);
  if (ret != 0)
    return ret;
  return 0;
}

static inline int next_u8(FILE *fp) {
  return fgetc(fp);
}

static inline int next_u16(FILE *fp) {
  int a = next_u8(fp);
  if (a == EOF)
    return EOF;
  int b = next_u8(fp);
  if (b == EOF)
    return EOF;
  return (b << 8) + a;
}

static inline int next_i16(FILE *fp) {
  int a = next_u16(fp);
  if (a == EOF)
    return EOF;
  return a >= (1 << 15) ? a - (1 << 16) : a;
}

static inline uint32_t next_u32(FILE *fp) {
  uint32_t a = next_u8(fp);
  if (a == EOF)
    return EOF;
  uint32_t b = next_u8(fp);
  if (b == EOF)
    return EOF;
  uint32_t c = next_u8(fp);
  if (c == EOF)
    return EOF;
  uint32_t d = next_u8(fp);
  if (d == EOF)
    return EOF;
  return (d * (1 << 24)) + (c * (1 << 16)) + (b * (1 << 8)) + a;
}

static int snd_wav(
  const char *dir,
  const char *waves_out,
  const char *offsets_out,
  const char *sizes_out,
  const char *names_out
) {
  int dirlen = strlen(dir);
  tinydir_dir tdir;
  tinydir_open_sorted(&tdir, dir);

  FILE *waves_fp = fopen(waves_out, "wb");
  if (!waves_fp) {
    fprintf(stderr, "\nFailed to write to: %s\n", waves_out);
    return 1;
  }
  FILE *offsets_fp = fopen(offsets_out, "wb");
  if (!offsets_fp) {
    fclose(waves_fp);
    fprintf(stderr, "\nFailed to write to: %s\n", offsets_out);
    return 1;
  }
  FILE *sizes_fp = fopen(sizes_out, "wb");
  if (!sizes_fp) {
    fclose(offsets_fp);
    fclose(waves_fp);
    fprintf(stderr, "\nFailed to write to: %s\n", sizes_out);
    return 1;
  }
  FILE *names_fp = fopen(names_out, "wb");
  if (!names_fp) {
    fclose(sizes_fp);
    fclose(offsets_fp);
    fclose(waves_fp);
    fprintf(stderr, "\nFailed to write to: %s\n", names_out);
    return 1;
  }

  uint32_t next_offset = 0;
  int total_wavs = 0;

  for (int i = 0; i < tdir.n_files; i++) {
    tinydir_file file;
    tinydir_readfile_n(&tdir, &file, i);
    if (file.is_dir) // skip directories
      continue;
    int flen = strlen(file.name);
    if (!(
      flen >= 4 &&
      file.name[flen - 4] == '.' &&
      file.name[flen - 3] == 'w' &&
      file.name[flen - 2] == 'a' &&
      file.name[flen - 1] == 'v'
    )) // skip non-wav files
      continue;

    // skip ".wav" :-P
    if (flen < 5) {
      fprintf(stderr, "WARNING: Skipping: %s\n  Filename too short\n", file.name);
      continue;
    }
    if (flen > 104) {
      fprintf(stderr, "WARNING: Skipping: %s\n  Filename too long\n", file.name);
      continue;
    }

    // validate filename
    for (int n = 0; n < flen - 4; n++) {
      char ch = file.name[n];
      if (!(
        (ch >= 'a' && ch <= 'z') ||
        (ch >= 'A' && ch <= 'Z') ||
        ch == '_' ||
        (n > 0 && ch >= '0' && ch <= '9')
      )) {
        fprintf(
          stderr,
          "WARNING: Skipping due to bad character '%c': %s\n"
          "  Filename must be in format: [a-zA-Z_][a-zA-Z_0-9]*\n",
          ch, file.name
        );
        goto skip;
      }
    }

    // process wav file
    char *full = malloc(sizeof(char) * (dirlen + flen + 3));
    snprintf(full, dirlen + flen + 2, "%s/%s", dir, file.name);
    FILE *fp = fopen(full, "rb");
    if (!fp) {
      fprintf(stderr, "WARNING: Failed to read: %s\n", full);
      free(full);
      continue;
    }
    printf("Processing: %s\n", full);
    free(full);

    // read the wav file
    if (next_u32(fp) != 0x46464952) // 'RIFF'
      goto invalid_wav_file;
    next_u32(fp); // size
    if (next_u32(fp) != 0x45564157) // 'WAVE'
      goto invalid_wav_file;

    while (1) {
      if (feof(fp))
        goto invalid_wav_file;
      uint32_t type = next_u32(fp);
      if (type == 0x20746d66) { // 'fmt '
        uint32_t size = next_u32(fp);
        int fmt = next_u16(fp);
        int channels = next_u16(fp);
        uint32_t rate = next_u32(fp);
        uint32_t rate2 = next_u32(fp);
        next_u16(fp); // align
        int bits = next_u16(fp);
        if (
          size != 16 ||
          fmt != 1 ||
          channels != 1 ||
          rate != 32768 ||
          rate2 != 65536 ||
          bits != 16
        ) {
          fprintf(stderr,
            "WARNING: Incompatible wav file:\n"
            "  Required 32768 sample rate, mono, 16-bits per sample\n"
          );
          goto invalid_wav_file;
        }
      } else if (type == 0x61746164) { // 'data'
        break;
      }
    }

    // all waves must be in steps of 608 samples
    int orig_size = next_u32(fp) >> 1;
    uint32_t size = (orig_size % 608) != 0 ? orig_size + 608 - (orig_size % 608) : orig_size;

    // output everything
    if (total_wavs < 0x1000) {
      printf("%s:\n  next_offset %08X\n  size %08X\n", file.name, next_offset, size);
      fprintf(names_fp, "%.*s\n", flen - 4, file.name);
      fwrite(&next_offset, sizeof(uint32_t), 1, offsets_fp);
      fwrite(&size, sizeof(uint32_t), 1, sizes_fp);
      for (int i = 0; i < size; i++) {
        uint16_t v = i < orig_size ? next_i16(fp) : 0;
        fwrite(&v, sizeof(uint16_t), 1, waves_fp);
      }
      next_offset += size * 2;
      total_wavs++;
      fclose(fp);
    } else {
      fprintf(stderr, "WARNING: Too many wav files!\n");
      fclose(fp);
      break;
    }
    continue;
invalid_wav_file:
    fclose(fp);
    fprintf(stderr, "WARNING: Invalid wav file\n");
skip:;
  }

  fclose4(waves_fp);
  fclose4(offsets_fp);
  fclose4(sizes_fp);
  fclose4(names_fp);

  printf("Total wav files: %d\n", total_wavs);
  tinydir_close(&tdir);
  return 0;
}

static void print_offset(const char* name, size_t offset) {
  printf(".set %s, %zu\n", name, offset);
}

#define print_sooff(name, field) \
  print_offset("SND_SONG_" name, offsetof(struct snd_song_st, field))
#define print_sioff(name, field) \
  print_offset("SND_SONGINST_" name, offsetof(struct snd_songinst_st, field))
#define print_ssoff(name, field) \
  print_offset("SND_SONGSEQ_" name, offsetof(struct snd_songseq_st, field))
#define print_choff(name, field) \
  print_offset("SND_CHANNEL_" name, offsetof(struct snd_channel_st, field))
#define print_syoff(name, field) \
  print_offset("SND_SYNTH_" name, offsetof(struct snd_synth_st, field))
#define print_sfoff(name, field) \
  print_offset("SND_SFX_" name, offsetof(struct snd_sfx_st, field))
#define print_snoff(name, field) \
  print_offset("SND_" name, offsetof(struct snd_st, field))

static int snd_makesong(const char *input, const char *names_file, const char *output) {
  struct song_st *song = song_new(input, names_file);
  if (!song)
    return 1;
  FILE *fp = fopen(output, "wb");
  if (!fp) {
    song_free(song);
    fprintf(stderr, "\nFailed to open: %s\n", output);
    return 1;
  }
  song_serialize(song, (song_serialize_write)fwrite, fp);
  song_free(song);
  fclose4(fp);
  return 0;
}

int snd_main(int argc, const char **argv) {
  if (argc <= 0) {
    snd_print_usage();
    return 0;
  }

  if (strcmp(argv[0], "tables") == 0) {
    if (argc != 6) {
      snd_print_usage();
      fprintf(
        stderr,
        "\nExpecting: tables <osc.bin> <tempo.bin> <slice.bin> <dphase.bin> <bend.bin>\n"
      );
      return 1;
    }
    return snd_tables(argv[1], argv[2], argv[3], argv[4], argv[5]);
  } else if (strcmp(argv[0], "wav") == 0) {
    if (argc != 6) {
      snd_print_usage();
      fprintf(
        stderr,
        "\nExpecting: wav <directory/wav> <waves.bin> <offsets.bin> <sizes.bin> <names.txt>\n"
      );
      return 1;
    }
    return snd_wav(argv[1], argv[2], argv[3], argv[4], argv[5]);
  } else if (strcmp(argv[0], "makesong") == 0) {
    if (argc != 4) {
      snd_print_usage();
      fprintf(stderr, "\nExpecting: makesong <input.txt> <names.txt> <output.bin>\n");
      return 1;
    }
    return snd_makesong(argv[1], argv[2], argv[3]);
  } else if (strcmp(argv[0], "gbaoffsets") == 0) {
    print_sooff("MAGIC"                  , magic                         );
    print_sooff("VERSION"                , version                       );
    print_sooff("CHANNEL_COUNT"          , channel_count                 );
    print_sooff("SAMPS_LENGTH"           , samps_length                  );
    print_sooff("INSTS_LENGTH"           , insts_length                  );
    print_sooff("SEQS_LENGTH"            , seqs_length                   );
    print_sooff("PATS_LENGTH"            , pats_length                   );
    print_sooff("INST_TABLE_OFFSET"      , inst_table_offset             );
    print_sooff("SEQ_TABLE_OFFSET"       , seq_table_offset              );
    print_sooff("PAT_TABLE_OFFSET"       , pat_table_offset              );
    print_sooff("SAMP_TABLE"             , samp_table                    );
    print_offset("SIZEOF_SND_SONG_ST"    , sizeof(struct snd_song_st)    );
    print_sioff("WAVE"                   , wave                          );
    print_sioff("VOLUME_ENV_ATTACK"      , volume_env_attack             );
    print_sioff("VOLUME_ENV_SUSTAIN"     , volume_env_sustain            );
    print_sioff("VOLUME_ENV_LENGTH"      , volume_env_length             );
    print_sioff("PITCH_ENV_ATTACK"       , pitch_env_attack              );
    print_sioff("PITCH_ENV_SUSTAIN"      , pitch_env_sustain             );
    print_sioff("PITCH_ENV_LENGTH"       , pitch_env_length              );
    print_sioff("VOLUME_ENV_OFFSET"      , volume_env_offset             );
    print_sioff("PITCH_ENV_OFFSET"       , pitch_env_offset              );
    print_offset("SIZEOF_SND_SONGINST_ST", sizeof(struct snd_songinst_st));
    print_ssoff("PAT_LENGTH"             , pat_length                    );
    print_ssoff("LOOP_INDEX"             , loop_index                    );
    print_ssoff("EXIT"                   , exit                          );
    print_ssoff("PATTERNS"               , patterns                      );
    print_offset("SIZEOF_SND_SONGSEQ_ST" , sizeof(struct snd_songseq_st) );
    print_choff("STATE"                  , state                         );
    print_choff("DELAY"                  , delay                         );
    print_choff("DELAYED_NOTE_ON_LEFT"   , delayed_note_on_left          );
    print_choff("DELAYED_NOTE_ON_NOTE"   , delayed_note_on_note          );
    print_choff("DELAYED_NOTE_OFF_LEFT"  , delayed_note_off_left         );
    print_choff("DELAYED_BEND_LEFT"      , delayed_bend_left             );
    print_choff("DELAYED_BEND_DURATION"  , delayed_bend_duration         );
    print_choff("DELAYED_BEND_NOTE"      , delayed_bend_note             );
    print_choff("CHAN_VOLUME"            , chan_volume                   );
    print_choff("ENV_VOLUME_INDEX"       , env_volume_index              );
    print_choff("BASE_PITCH"             , base_pitch                    );
    print_choff("TARGET_PITCH"           , target_pitch                  );
    print_choff("BEND_COUNTER"           , bend_counter                  );
    print_choff("BEND_COUNTER_MAX"       , bend_counter_max              );
    print_choff("ENV_PITCH_INDEX"        , env_pitch_index               );
    print_choff("PHASE"                  , phase                         );
    print_choff("INST_BASE"              , inst_base                     );
    print_offset("SIZEOF_SND_CHANNEL_ST" , sizeof(struct snd_channel_st) );
    print_syoff("SONG_BASE"              , song_base                     );
    print_syoff("SEQUENCE"               , sequence                      );
    print_syoff("TEMPO_INDEX"            , tempo_index                   );
    print_syoff("TICK_START"             , tick_start                    );
    print_syoff("TICK_LEFT"              , tick_left                     );
    print_syoff("SEQ_INDEX"              , seq_index                     );
    print_syoff("PAT"                    , pat                           );
    print_syoff("VOLUME"                 , volume                        );
    print_syoff("CHANNEL"                , channel                       );
    print_offset("SIZEOF_SND_SYNTH_ST"   , sizeof(struct snd_synth_st)   );
    print_sfoff("WAV_BASE"               , wav_base                      );
    print_sfoff("SAMPLES_LEFT"           , samples_left                  );
    print_sfoff("PRIORITY"               , priority                      );
    print_offset("SIZEOF_SND_SFX_ST"     , sizeof(struct snd_sfx_st)     );
    print_snoff("SYNTH"                  , synth                         );
    print_snoff("SFX"                    , sfx                           );
    print_snoff("BUFFER_ADDR"            , buffer_addr                   );
    print_snoff("NEXT_BUFFER_INDEX"      , next_buffer_index             );
    print_snoff("BUFFER1"                , buffer1                       );
    print_snoff("BUFFER2"                , buffer2                       );
    print_snoff("BUFFER3"                , buffer3                       );
    print_snoff("BUFFER_TEMP"            , buffer_temp                   );
    print_snoff("MASTER_VOLUME"          , master_volume                 );
    print_snoff("SFX_VOLUME"             , sfx_volume                    );
    print_offset("SIZEOF_SND_ST"         , sizeof(struct snd_st)         );
    return 0;
  } else {
    snd_print_usage();
    fprintf(stderr, "\nUnknown command: %s\n", argv[0]);
    return 1;
  }

  return 0;
}
