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
#include "music_ds1.h"
#include "music_ds2.h"
#include "tinydir.h"

#define DUTY_COUNT      8
#define MAX_PHASE_BITS  11
#define MAX_PHASE       (1 << MAX_PHASE_BITS)
#define MAX_RND_SAMPLE  (1 << 15)
#define PI              3.14159265358979323846

static void music_print_usage() {
  printf(
    "Usage:\n"
    "  music <command> [options...]\n"
    "\n"
    "Commands:\n"
    "\n"
    "  tables <output.bin>\n"
    "    Generate the wave tables\n"
    "\n"
    "  wav <directory/wav> <waves.bin> <offsets.bin> <names.txt>\n"
    "    Consolidate wav files and output metadata\n"
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

typedef struct {
  double *data;
  int size;
  int count;
} list_double;

static void list_double_push(list_double *list, double v) {
  if (list->size + 1 > list->count) {
    int new_count = list->count <= 0 ? MAX_PHASE : list->count * 2;
    list->data = realloc(list->data, new_count * sizeof(double));
    list->count = new_count;
  }
  list->data[list->size] = v;
  list->size++;
}

static void music_generate_oscillator(
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

static void music_push_realimag(
  list_double *list,
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
    list_double_push(list, v);
  }
}

static void music_add_wave(
  list_double *waves,
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
    music_generate_oscillator(
      real,
      imag,
      lowpass,
      duty,
      square,
      sine,
      saw,
      triangle
    );
    music_push_realimag(waves, real, imag, 1024);
  }
  printf("\n");
}

static int music_tables(const char *output) {
  FILE *fp = fopen(output, "wb");
  if (!fp) {
    fprintf(stderr, "\nFailed to open for writing: %s\n", output);
    return 1;
  }

  list_double waves = {0};

  printf("Generating: rnd\n");
  for (int i = 1; i <= MAX_RND_SAMPLE; i++) {
    list_double_push(&waves, whisky1d(i) * 2.0 - 1.0);
  }

  for (int duty = 0; duty < DUTY_COUNT; duty++) {
    printf("Generating: sq%d\n ", duty + 1);
    int lowpass_table[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    music_add_wave(
      &waves,
      lowpass_table,
      sizeof(lowpass_table) / sizeof(int),
      (duty + 1.0) / (2.0 * DUTY_COUNT),
      1, 0, 0, 0
    );
  }

  {
    printf("Generating: tri\n ");
    int lowpass_table[] = {0, 4, 6, 7, 8, 9};
    music_add_wave(
      &waves,
      lowpass_table,
      sizeof(lowpass_table) / sizeof(int),
      0, 0, 0, 0, 1
    );
  }

  {
    printf("Generating: saw\n ");
    int lowpass_table[] = {0, 4, 5, 6, 7, 8, 9};
    music_add_wave(
      &waves,
      lowpass_table,
      sizeof(lowpass_table) / sizeof(int),
      0, 0, 0, 1, 0
    );
  }

  {
    printf("Generating: sin\n ");
    int lowpass_table[] = {0};
    music_add_wave(
      &waves,
      lowpass_table,
      sizeof(lowpass_table) / sizeof(int),
      0, 0, 1, 0, 0
    );
  }

  {
    printf("Generating: ds1\n ");
    printf(" 4"); music_push_realimag(&waves, NULL, music_ds1_imag, 64);
    printf(" 5"); music_push_realimag(&waves, NULL, music_ds1_imag, 32);
    printf(" 6"); music_push_realimag(&waves, NULL, music_ds1_imag, 16);
    printf(" 7"); music_push_realimag(&waves, NULL, music_ds1_imag, 8);
    printf(" 8"); music_push_realimag(&waves, NULL, music_ds1_imag, 4);
    printf(" 9"); music_push_realimag(&waves, NULL, music_ds1_imag, 2);
    printf("\n");
  }

  {
    printf("Generating: ds2\n ");
    printf(" 4"); music_push_realimag(&waves, NULL, music_ds2_imag, 64);
    printf(" 5"); music_push_realimag(&waves, NULL, music_ds2_imag, 32);
    printf(" 6"); music_push_realimag(&waves, NULL, music_ds2_imag, 16);
    printf(" 7"); music_push_realimag(&waves, NULL, music_ds2_imag, 8);
    printf(" 8"); music_push_realimag(&waves, NULL, music_ds2_imag, 4);
    printf(" 9"); music_push_realimag(&waves, NULL, music_ds2_imag, 2);
    printf("\n");
  }

  printf("Writing to: %s\n", output);
  uint16_t *data = malloc(sizeof(uint16_t) * waves.size);
  for (int i = 0; i < waves.size; i++) {
    double v = waves.data[i];
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
  fwrite(data, sizeof(uint16_t), waves.size, fp);
  free(data);
  free(waves.data);
  fclose(fp);

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

static int music_wav(
  const char *dir,
  const char *waves_out,
  const char *offsets_out,
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
  FILE *names_fp = fopen(names_out, "w");
  if (!names_fp) {
    fclose(offsets_fp);
    fclose(waves_fp);
    fprintf(stderr, "\nFailed to write to: %s\n", names_out);
    return 1;
  }

  uint32_t next_offset = 0;

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

    // validate filename
    for (int n = 0; n < flen - 4; n++) {
      char ch = file.name[n];
      if (!(
        (ch >= 'a' && ch <= 'z') ||
        (ch >= 'A' && ch <= 'Z') ||
        (n > 0 && ch >= '0' && ch <= '9') ||
        (n > 0 && ch == '_')
      )) {
        fprintf(
          stderr,
          "WARNING: Skipping due to bad character '%c': %s\n"
          "  Filename must be in format: [a-zA-Z][a-zA-Z0-9_]*\n",
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
    int size = (orig_size % 608) != 0 ? orig_size + 608 - (orig_size % 608) : orig_size;

    // output everything
    fprintf(names_fp, "%.*s\n", flen - 4, file.name);
    fwrite(&next_offset, sizeof(uint32_t), 1, offsets_fp);
    for (int i = 0; i < size; i++) {
      uint16_t v = i < orig_size ? next_i16(fp) : 0;
      fwrite(&v, sizeof(uint16_t), 1, waves_fp);
    }
    next_offset += size * 2;

    fclose(fp);
    continue;
invalid_wav_file:
    fclose(fp);
    fprintf(stderr, "WARNING: Invalid wav file\n");
skip:;
  }

  tinydir_close(&tdir);
  return 0;
}

int music_main(int argc, const char **argv) {
  if (argc <= 0) {
    music_print_usage();
    return 0;
  }

  if (strcmp(argv[0], "tables") == 0) {
    if (argc != 2) {
      music_print_usage();
      fprintf(stderr, "\nExpecting: tables <output.bin>\n");
      return 1;
    }
    return music_tables(argv[1]);
  } else if (strcmp(argv[0], "wav") == 0) {
    if (argc != 5) {
      music_print_usage();
      fprintf(stderr, "\nExpecting: wav <directory/wav> <waves.bin> <offsets.bin> <names.txt>\n");
      return 1;
    }
    return music_wav(argv[1], argv[2], argv[3], argv[4]);
  } else {
    music_print_usage();
    fprintf(stderr, "\nUnknown command: %s\n", argv[0]);
    return 1;
  }

  return 0;
}
