//
// auntflora - Port of Aunt Flora's Mansion to Game Boy Advance
// by Sean Connelly (@velipso), https://sean.cm
// Project Home: https://github.com/velipso/auntflora
// SPDX-License-Identifier: 0BSD
//

#include "song.h"
#include "snd.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "stb_ds.h"

static int song_parsenote(const char *str) {
  if (str[0] == '-' && str[1] == '-' && str[2] == '-') // continue command
    return 0;
  if (
    (str[0] == 'o' || str[0] == 'O') &&
    (str[1] == 'f' || str[1] == 'F') &&
    (str[2] == 'f' || str[2] == 'F')
  ) // OFF command
    return 1;
  if (str[0] == '=' && str[1] == '=' && str[2] == '=') // OFF command, alternate syntax
    return 1;
  if (
    (str[0] == 'e' || str[0] == 'E') &&
    (str[1] == 'n' || str[1] == 'N') &&
    (str[2] == 'd' || str[2] == 'D')
  ) // END command
    return 2;
  if (str[0] == '0' && str[1] == '0' && str[2] == '0') // stop command
    return 7;
  // base note
  int note = 0;
  if (str[0] == 'c' || str[0] == 'C') {
    note = 8;
  } else if (str[0] == 'd' || str[0] == 'D') {
    note = 10;
  } else if (str[0] == 'e' || str[0] == 'E') {
    note = 12;
  } else if (str[0] == 'f' || str[0] == 'F') {
    note = 13;
  } else if (str[0] == 'g' || str[0] == 'G') {
    note = 15;
  } else if (str[0] == 'a' || str[0] == 'A') {
    note = 17;
  } else if (str[0] == 'b' || str[0] == 'B') {
    note = 19;
  } else {
    return -1;
  }
  // sharp/flat
  if (str[1] == '-') {
    // do nothing
  } else if (str[1] == 'b' || str[1] == 'B') {
    note--;
  } else if (str[1] == '#') {
    note++;
  } else {
    return -1;
  }
  // octave
  if (str[2] >= '0' && str[2] <= '9') {
    note += (str[2] - '0') * 12;
    if (note < 0x08 || note > 0x7f)
      return -2; // note out of range
  }
  return note;
}

static int song_parseeffect(const char *str) {
  if (str[0] == '-' && str[1] == '-' && str[2] == '-') // continue command
    return 0x00;
  if (
    (str[0] == 'e' || str[0] == 'E') &&
    (str[1] == 'n' || str[1] == 'N') &&
    (str[2] == 'd' || str[2] == 'D')
  ) // END command
    return 0x01;
  if (
    (str[0] == 'o' || str[0] == 'O') &&
    (str[1] == 'f' || str[1] == 'F') &&
    (str[2] == 'f' || str[2] == 'F')
  ) // OFF command
    return 0x51;
  // parse tempo
  if (
    (str[0] >= '0' && str[0] <= '9') &&
    (str[1] >= '0' && str[1] <= '9') &&
    (str[2] >= '0' && str[2] <= '9')
  ) {
    double tempo = (str[0] - '0') * 100 + (str[1] - '0') * 10 + (str[2] - '0');
    if (tempo < 45 || tempo > 202)
      return -2; // tempo out of range
    int best_i = -1;
    double best_tempo = 0;
    double sample_rate = 32768.0;
    for (int i = 0; i < 64; i++) {
      double ideal_tempo = (i + 18.0) * 10.0 / 4.0;
      double start_value = round(sample_rate * 60.0 * 256.0 / (608.0 * 16.0 * ideal_tempo));
      double actual_tempo = sample_rate * 60.0 * 256.0 / (608.0 * 16.0 * start_value);
      if (best_i < 0 || fabs(tempo - actual_tempo) < fabs(tempo - best_tempo)) {
        best_tempo = actual_tempo;
        best_i = i;
      }
    }
    return 0xc0 | best_i;
  }
  // parse bend
  if (
    (str[0] == 'b' || str[0] == 'B') &&
    (str[1] >= '0' && str[1] <= '9') &&
    (str[2] >= '0' && str[2] <= '9')
  ) {
    int bend = (str[1] - '0') * 10 + (str[2] - '0');
    if (bend == 0)
      return 0x0002;
    if (bend > 64)
      return -3; // bend out of range
    return 0x80 | (bend - 1);
  }
  // parse delay
  if (
    (str[0] == 'd' || str[0] == 'D') &&
    (str[1] >= '0' && str[1] <= '9') &&
    (str[2] >= '0' && str[2] <= '9')
  ) {
    int delay = (str[1] - '0') * 10 + (str[2] - '0');
    if (delay > 60)
      return -4; // delay out of range
    return 0x00 | delay + 3;
  }
  // parse volume
  if (
    (str[0] == 'v' || str[0] == 'V') &&
    (str[1] >= '0' && str[1] <= '9') &&
    (str[2] >= '0' && str[2] <= '9')
  ) {
    int volume = (str[1] - '0') * 10 + (str[2] - '0');
    if (volume > 16)
      return -5; // volume out of range
    return 0x40 | volume;
  }
  return -1; // unrecognized
}

static void song_printseq(const struct song_sequence_st *seq) {
  for (int i = 0; i < arrlen(seq->seq) + 1; i++) {
    if (i == seq->exit)
      printf("}");
    if (i >= arrlen(seq->seq))
      break;
    printf(" ");
    if (i == seq->loop)
      printf("{");
    printf("%d", seq->seq[i]);
  }
}

void song_print(const struct song_st *song) {
  printf("song %p:\n", song);
  printf("  channel_count = %d\n", song->channel_count);
  printf("  instruments[%d]:\n", (int)arrlen(song->instruments));
  for (int i = 0; i < arrlen(song->instruments); i++) {
    struct song_instrument_st *inst = song->instruments[i];
    printf("  %2d: wave = %d\n", i, inst->wave);
    printf("      phase = %d\n", inst->phase);
    printf("      volume =");
    song_printseq(&inst->volume);
    printf("\n");
    printf("      pitch =");
    song_printseq(&inst->pitch);
    printf("\n");
  }
  printf("  samples[%d]:\n", (int)arrlen(song->samples));
  for (int i = 0; i < arrlen(song->samples); i++) {
    struct song_sample_st *samp = song->samples[i];
    printf("  %2d: index = %d\n", i, samp->index);
    printf("      volume = %d\n", samp->volume);
  }
  printf("  patterns[%d]\n", (int)arrlen(song->patterns));
  for (int i = 0; i < arrlen(song->patterns); i++) {
    struct song_patternline_st **lines = song->patterns[i]->lines;
    printf("  %2d: lines[%d]:\n", i, (int)arrlen(lines));
    for (int j = 0; j < arrlen(lines); j++) {
      struct song_patternline_st *line = lines[j];
      printf("    ");
      for (int k = 0; k < song->channel_count; k++) {
        printf("  %04X", line->commands[k]);
      }
      printf("  wait %d\n", line->wait);
    }
  }
  printf("  sequences[%d]:\n", (int)arrlen(song->sequences));
  for (int i = 0; i < arrlen(song->sequences); i++) {
    struct song_sequence_st *seq = song->sequences[i];
    printf("  %2d: seq =", i);
    song_printseq(seq);
    printf("\n");
  }
}

void song_free(struct song_st *song) {
  for (int i = 0; i < arrlen(song->instruments); i++) {
    struct song_instrument_st *inst = song->instruments[i];
    arrfree(inst->volume.seq);
    arrfree(inst->pitch.seq);
    free(inst);
  }
  arrfree(song->instruments);
  for (int i = 0; i < arrlen(song->samples); i++) {
    struct song_sample_st *samp = song->samples[i];
    free(samp);
  }
  arrfree(song->samples);
  for (int i = 0; i < arrlen(song->patterns); i++) {
    struct song_pattern_st *pat = song->patterns[i];
    for (int j = 0; j < arrlen(pat->lines); j++)
      free(pat->lines[j]);
    arrfree(pat->lines);
    free(pat);
  }
  arrfree(song->patterns);
  for (int i = 0; i < arrlen(song->sequences); i++) {
    struct song_sequence_st *seq = song->sequences[i];
    arrfree(seq->seq);
    free(seq);
  }
  arrfree(song->sequences);
  free(song);
}

struct song_st *song_new(const char *input, const char *names_file) {
  struct song_st *song = calloc(sizeof(struct song_st), 1);
  char **sample_waves = NULL;
  const char **instrument_waves = NULL;
  const char **instrument_names = NULL;
  const char **sample_names = NULL;
  struct pattern_name_st {
    const char *name;
    int part;
  };
  struct pattern_name_st **pattern_names = NULL;
  uint64_t sequence_idx[4] = {0};
  int sequence_length = 0;
  arrpush(instrument_waves, "rnd");
  arrpush(instrument_waves, "sq1");
  arrpush(instrument_waves, "sq2");
  arrpush(instrument_waves, "sq3");
  arrpush(instrument_waves, "sq4");
  arrpush(instrument_waves, "sq5");
  arrpush(instrument_waves, "sq6");
  arrpush(instrument_waves, "sq7");
  arrpush(instrument_waves, "sq8");
  arrpush(instrument_waves, "tri");
  arrpush(instrument_waves, "saw");
  arrpush(instrument_waves, "sin");
  arrpush(instrument_waves, "ds1");
  arrpush(instrument_waves, "ds2");

  #define TOK_IDENT    1
  #define TOK_SPECIAL  2
  #define TOK_NUMBER   3
  #define TOK_STR      4

  struct token_st {
    int type;
    union {
      char *ident;
      char special;
      int number;
      char *str;
    } u;
    int line;
    int chr;
  };
  struct token_st **tokens = NULL;

  { // parse sample_waves from names file
    FILE *fp = fopen(names_file, "r");
    if (!fp) {
      fprintf(stderr, "\nFailed to read: %s\n", names_file);
      goto cleanup_fail;
    }
    char name[101];
    int i = 0;
    while (!feof(fp)) {
      int ch = fgetc(fp);
      if (ch == EOF || ch == '\r' || ch == '\n') {
        name[i++] = 0;
        if (i > 1) {
          for (int j = 0; j < arrlen(sample_waves); j++) {
            if (strcmp(name, sample_waves[j]) == 0) {
              fprintf(stderr, "\n%s: Duplicate sample wave: \"%s\"\n", names_file, name);
              goto cleanup_fail;
            }
          }
          char *n = malloc(sizeof(char) * i);
          memcpy(n, name, sizeof(char) * i);
          if (arrlen(sample_waves) < 0x1000) {
            arrpush(sample_waves, n);
          } else {
            fprintf(stderr, "\n%s: Too many sample names\n", names_file);
            goto cleanup_fail;
          }
          i = 0;
        }
        if (ch == EOF)
          break;
      } else if (i < 100) {
        name[i++] = ch;
      } else {
        name[i++] = 0;
        fclose(fp);
        fprintf(stderr, "\n%s: Name \"%s...\" too long\n", names_file, name);
        goto cleanup_fail;
      }
    }
    fclose(fp);
  }

  { // lex main song file into tokens
    FILE *fp = fopen(input, "r");
    if (!fp) {
      fprintf(stderr, "\nFailed to read: %s\n", input);
      goto cleanup_fail;
    }

    int state = 0;
    int number = 0;
    int line = 1;
    int chr = 1;
    struct token_st *tok = NULL;
    char ident[101];
    int ident_i = 0;
    int new_pattern = 0;
    int new_pattern_line = 0;
    int new_pattern_chr = 0;
    while (!feof(fp)) {
      int ch = fgetc(fp);
      if (ch == EOF)
        break;
      switch (state) {
        case 0: // start
          if (ch == ' ' || ch == '\t' || ch == '\r') {
            // ignore
          } else if (ch == '#') {
            state = 1;
          } else if (ch >= '0' && ch <= '9') {
            tok = malloc(sizeof(struct token_st));
            tok->type = TOK_NUMBER;
            tok->line = line;
            tok->chr = chr;
            number = ch - '0';
            state = 2;
          } else if (
            (ch >= 'a' && ch <= 'z') ||
            (ch >= 'A' && ch <= 'Z') ||
            ch == '_'
          ) {
            tok = malloc(sizeof(struct token_st));
            tok->type = TOK_IDENT;
            tok->line = line;
            tok->chr = chr;
            tok->u.ident = NULL;
            ident_i = 0;
            ident[ident_i++] = ch;
            state = 3;
          } else if (ch == ':' || ch == '{' || ch == '}' || ch == '-') {
            if (new_pattern == 3 && ch == ':')
              new_pattern = 4;
            else
              new_pattern = 0;
            tok = malloc(sizeof(struct token_st));
            tok->type = TOK_SPECIAL;
            tok->line = line;
            tok->chr = chr;
            tok->u.special = ch;
            arrpush(tokens, tok);
            tok = NULL;
          } else if (ch == '\n') {
            if (new_pattern == 3 || new_pattern == 5)
              state = 4;
            new_pattern = 0;
          } else {
            fprintf(stderr, "\n%s:%d:%d: Unexpected character '%c'\n", input, line, chr, ch);
            goto cleanup_fail;
          }
          break;
        case 1: // end of line comment
          if (ch == '\n')
            state = 0;
          break;
        case 2: // number
          if (ch >= '0' && ch <= '9') {
            number = (number * 10) + (ch - '0');
          } else {
            if (new_pattern == 4)
              new_pattern = 5;
            else
              new_pattern = 0;
            tok->u.number = number;
            arrpush(tokens, tok);
            tok = NULL;
            ungetc(ch, fp);
            ch = -1;
            state = 0;
          }
          break;
        case 3: // ident
          if (
            (ch >= '0' && ch <= '9') ||
            (ch >= 'a' && ch <= 'z') ||
            (ch >= 'A' && ch <= 'Z') ||
            ch == '_'
          ) {
            if (ident_i < 100) {
              ident[ident_i++] = ch;
            } else {
              ident[ident_i++] = 0;
              free(tok);
              fprintf(stderr, "\n%s:%d:%d: Identifier \"%s...\" too long\n", input, line, chr,
                ident);
              goto cleanup_fail;
            }
          } else {
            ident[ident_i++] = 0;
            if (strcmp(ident, "new") == 0) {
              new_pattern = 1;
              new_pattern_line = line;
              new_pattern_chr = chr;
            } else if (new_pattern == 1 && strcmp(ident, "pattern") == 0)
              new_pattern = 2;
            else if (new_pattern == 2)
              new_pattern = 3;
            else
              new_pattern = 0;
            tok->u.ident = malloc(sizeof(char) * ident_i);
            memcpy(tok->u.ident, ident, sizeof(char) * ident_i);
            arrpush(tokens, tok);
            ident_i = 0;
            tok = NULL;
            ungetc(ch, fp);
            ch = -1;
            state = 0;
          }
          break;
        case 4: // reading pattern
          if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n') {
            // ignore
          } else if (
            (ch >= '0' && ch <= '9') ||
            (ch >= 'a' && ch <= 'z') ||
            (ch >= 'A' && ch <= 'Z')
          ) {
            // start of timestamp
            tok = malloc(sizeof(struct token_st));
            tok->type = TOK_NUMBER;
            tok->line = line;
            tok->chr = chr;
            number = 16 * (ch - (
              (ch >= '0' && ch <= '9')
              ? '0'
              : (ch >= 'a' && ch <= 'z')
              ? ('a' - 10)
              : ('A' - 10)
            ));
            // if 'e', it could be start of 'end' instead of a timestamp
            state = ch == 'e' ? 7 : 6;
          } else if (ch == '#') {
            state = 5;
          } else {
            fprintf(stderr, "\n%s:%d:%d: Unexpected character '%c'\n", input, line, chr, ch);
            goto cleanup_fail;
          }
          break;
        case 5: // end of line comment inside pattern
          if (ch == '\n')
            state = 4;
          break;
        case 6: // timestamp
          if (
            (ch >= '0' && ch <= '9') ||
            (ch >= 'a' && ch <= 'f') ||
            (ch >= 'A' && ch <= 'F')
          ) {
            number += ch - (
              (ch >= '0' && ch <= '9')
              ? '0'
              : (ch >= 'a' && ch <= 'f')
              ? ('a' - 10)
              : ('A' - 10)
            );
            tok->u.number = number;
            arrpush(tokens, tok);
            tok = NULL;
            state = 10;
          } else {
            free(tok);
            fprintf(stderr, "\n%s:%d:%d: Unexpected character '%c'\n", input, line, chr, ch);
            goto cleanup_fail;
          }
          break;
        case 7: // start of 'end' or timestamp
          if (ch == 'n') {
            // oops, it's the start of 'end'
            tok->type = TOK_IDENT;
            state = 8;
          } else {
            ungetc(ch, fp);
            ch = -1;
            state = 6;
          }
          break;
        case 8: // 'en'
          if (ch == 'd') {
            state = 9;
          } else {
            free(tok);
            fprintf(stderr, "\n%s:%d:%d: Unexpected character '%c'\n", input, line, chr, ch);
            goto cleanup_fail;
          }
          break;
        case 9: // 'end'
          if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n') {
            // got 'end'
            tok->u.ident = malloc(4);
            strcpy(tok->u.ident, "end");
            arrpush(tokens, tok);
            tok = NULL;
            state = 0;
          } else {
            free(tok);
            fprintf(stderr, "\n%s:%d:%d: Unexpected character '%c'\n", input, line, chr, ch);
            goto cleanup_fail;
          }
          break;
        case 10: // expecting channel data
          if (ch == ' ' || ch == '\t' || ch == '\r') {
            // ignore
          } else if (ch == '#') {
            // comment
            state = 5;
          } else if (ch == '\n') {
            // end of channel data
            state = 4;
          } else if (
            (ch >= '0' && ch <= '9') ||
            (ch >= 'a' && ch <= 'z') ||
            (ch >= 'A' && ch <= 'Z') ||
            ch == '_' ||
            ch == '-'
          ) {
            ident_i = 0;
            ident[ident_i++] = ch;
            tok = malloc(sizeof(struct token_st));
            tok->type = TOK_STR;
            tok->line = line;
            tok->chr = chr;
            state = 11;
          } else {
            fprintf(stderr, "\n%s:%d:%d: Unexpected character '%c'\n", input, line, chr, ch);
            goto cleanup_fail;
          }
          break;
        case 11: // inside channel data
          if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n') {
            ident[ident_i++] = 0;
            tok->u.str = malloc(sizeof(char) * ident_i);
            memcpy(tok->u.str, ident, sizeof(char) * ident_i);
            arrpush(tokens, tok);
            ident_i = 0;
            tok = NULL;
            state = ch == '\n' ? 4 : 10;
          } else if (
            (ch >= '0' && ch <= '9') ||
            (ch >= 'a' && ch <= 'z') ||
            (ch >= 'A' && ch <= 'Z') ||
            ch == '_' ||
            ch == '-' ||
            ch == ':'
          ) {
            if (ident_i < 7)
              ident[ident_i++] = ch;
            else {
              ident[ident_i++] = 0;
              free(tok);
              fprintf(stderr, "\n%s:%d:%d: Channel data \"%s...\" too long\n", input, line, chr,
                ident);
              goto cleanup_fail;
            }
          } else {
            free(tok);
            fprintf(stderr, "\n%s:%d:%d: Unexpected character '%c'\n", input, line, chr, ch);
            goto cleanup_fail;
          }
          break;
      }
      if (ch == '\n') {
        line++;
        chr = 1;
      } else if (ch >= 0) {
        chr++;
      }
    }
    switch (state) {
      case 0: // start
      case 1: // end of line comment
        break;
      case 2: // number
        tok->u.number = number;
        arrpush(tokens, tok);
        tok = NULL;
        break;
      case 3: // ident
        ident[ident_i++] = 0;
        tok->u.ident = malloc(sizeof(char) * ident_i);
        memcpy(tok->u.ident, ident, sizeof(char) * ident_i);
        arrpush(tokens, tok);
        tok = NULL;
        break;
      case 4: // pattern
      case 5: // end of line comment inside pattern
      case 6: // timestamp
      case 7: // start of 'end' or timestamp
      case 8: // 'en'
      case 9: // 'end'
      case 10: // expecting channel data
      case 11: // inside channel data
        fprintf(stderr, "\n%s:%d:%d: Missing 'end' of pattern\n", input, new_pattern_line,
          new_pattern_chr);
        free(tok);
        goto cleanup_fail;
    }
    free(tok);
    fclose(fp);
  }

  { // validate `new (instrument|sample|sequence|pattern) ... end`
    int state = 0;
    int new_line = 0;
    int new_chr = 0;
    for (int i = 0; i < arrlen(tokens); i++) {
      struct token_st *tok = tokens[i];
      switch (state) {
        case 0:
          if (tok->type == TOK_IDENT && strcmp(tok->u.ident, "new") == 0) {
            new_line = tok->line;
            new_chr = tok->chr;
            state = 1;
          } else {
            fprintf(stderr, "\n%s:%d:%d: Unknown statement\n", input, tok->line, tok->chr);
            goto cleanup_fail;
          }
          break;
        case 1: // reading type
          if (tok->type == TOK_IDENT && strcmp(tok->u.ident, "instrument") == 0)
            state = 2;
          else if (tok->type == TOK_IDENT && strcmp(tok->u.ident, "sample") == 0)
            state = 3;
          else if (tok->type == TOK_IDENT && strcmp(tok->u.ident, "sequence") == 0)
            state = 4;
          else if (tok->type == TOK_IDENT && strcmp(tok->u.ident, "pattern") == 0)
            state = 5;
          else {
            fprintf(stderr, "\n%s:%d:%d: Unknown statement\n", input, tok->line, tok->chr);
            goto cleanup_fail;
          }
          break;
        case 2: // instrument
          if (tok->type == TOK_IDENT) {
            if (strlen(tok->u.ident) != 3) {
              fprintf(stderr, "\n%s:%d:%d: Invalid instrument name \"%s\" - must be 3 characters\n",
                input, tok->line, tok->chr, tok->u.ident);
              goto cleanup_fail;
            }
            if (song_parseeffect(tok->u.ident) != -1) {
              fprintf(stderr, "\n%s:%d:%d: Invalid instrument name \"%s\" - must not be an "
                "effect\n", input, tok->line, tok->chr, tok->u.ident);
              goto cleanup_fail;
            }
            for (int j = 0; j < arrlen(instrument_names); j++) {
              if (strcmp(instrument_names[j], tok->u.ident) == 0) {
                fprintf(stderr, "\n%s:%d:%d: Cannot redefine name \"%s\"\n", input, tok->line,
                  tok->chr, tok->u.ident);
                goto cleanup_fail;
              }
            }
            arrpush(instrument_names, tok->u.ident);
            state = 6;
          } else {
            fprintf(stderr, "\n%s:%d:%d: Expecting instrument name\n", input, tok->line, tok->chr);
            goto cleanup_fail;
          }
          break;
        case 3: // sample
          if (tok->type == TOK_IDENT) {
            if (strlen(tok->u.ident) != 3) {
              fprintf(stderr, "\n%s:%d:%d: Invalid sample name \"%s\" - must be 3 characters\n",
                input, tok->line, tok->chr, tok->u.ident);
              goto cleanup_fail;
            }
            if (song_parsenote(tok->u.ident) != -1) {
              fprintf(stderr, "\n%s:%d:%d: Invalid sample name \"%s\" - must not be a note\n",
                input, tok->line, tok->chr, tok->u.ident);
              goto cleanup_fail;
            }
            for (int j = 0; j < arrlen(sample_names); j++) {
              if (strcmp(sample_names[j], tok->u.ident) == 0) {
                fprintf(stderr, "\n%s:%d:%d: Cannot redefine name \"%s\"\n", input, tok->line,
                  tok->chr, tok->u.ident);
                goto cleanup_fail;
              }
            }
            arrpush(sample_names, tok->u.ident);
            state = 6;
          } else {
            fprintf(stderr, "\n%s:%d:%d: Expecting sample name\n", input, tok->line, tok->chr);
            goto cleanup_fail;
          }
          break;
        case 4: // sequence
          if (tok->type == TOK_NUMBER) {
            if (tok->u.number < 0 || tok->u.number > 254) {
              fprintf(stderr, "\n%s:%d:%d: Invalid sequence number %d - must be 0-254\n",
                input, tok->line, tok->chr, tok->u.number);
              goto cleanup_fail;
            }
            int n = tok->u.number >> 6;
            uint64_t b = UINT64_C(1) << (tok->u.number & 0x3f);
            if (sequence_idx[n] & b) {
              fprintf(stderr, "\n%s:%d:%d: Cannot redefine sequence number %d\n",
                input, tok->line, tok->chr, tok->u.number);
              goto cleanup_fail;
            }
            sequence_idx[n] |= b;
            state = 6;
          } else {
            fprintf(stderr, "\n%s:%d:%d: Expecting sequence number\n", input, tok->line, tok->chr);
            goto cleanup_fail;
          }
          break;
        case 5: // pattern
          if (tok->type == TOK_IDENT) {
            int part = -1;
            if (
              i < arrlen(tokens) - 1 &&
              tokens[i + 1]->type == TOK_SPECIAL &&
              tokens[i + 1]->u.special == ':'
            ) {
              // parse part
              if (
                i < arrlen(tokens) - 2 &&
                tokens[i + 2]->type == TOK_NUMBER
              ) {
                part = tokens[i + 2]->u.number;
              } else {
                fprintf(stderr, "\n%s:%d:%d: Invalid pattern name \"%s\" - expecting number after "
                  "':'\n", input, tok->line, tok->chr, tok->u.ident);
                goto cleanup_fail;
              }
            }
            for (int j = 0; j < arrlen(pattern_names); j++) {
              struct pattern_name_st *pat = pattern_names[j];
              if (strcmp(pat->name, tok->u.ident) == 0 && pat->part == part) {
                if (part >= 0) {
                  fprintf(stderr, "\n%s:%d:%d: Cannot redefine pattern \"%s:%d\"\n",
                    input, tok->line, tok->chr, tok->u.ident, part);
                } else {
                  fprintf(stderr, "\n%s:%d:%d: Cannot redefine pattern \"%s\"\n",
                    input, tok->line, tok->chr, tok->u.ident);
                }
                goto cleanup_fail;
              }
            }
            struct pattern_name_st *pat = malloc(sizeof(struct pattern_name_st));
            pat->name = tok->u.ident;
            pat->part = part;
            arrpush(pattern_names, pat);
            state = 6;
          } else {
            fprintf(stderr, "\n%s:%d:%d: Expecting pattern name\n", input, tok->line, tok->chr);
            goto cleanup_fail;
          }
          break;
        case 6: // waiting for 'end'
          if (tok->type == TOK_IDENT && strcmp(tok->u.ident, "end") == 0)
            state = 0;
          break;
      }
    }
    switch (state) {
      case 0:
        break;
      case 1: // reading type
      case 2: // instrument
      case 3: // sample
      case 4: // sequence
      case 5: // pattern
      case 6: // waiting for 'end'
        fprintf(stderr, "\n%s:%d:%d: Missing 'end'\n", input, new_line, new_chr);
        goto cleanup_fail;
    }
  }

  if (arrlen(instrument_names) > 46) {
    fprintf(stderr, "\nCannot have more than 46 instruments\n");
    goto cleanup_fail;
  }

  if (arrlen(sample_names) > 128) {
    fprintf(stderr, "\nCannot have more than 128 samples\n");
    goto cleanup_fail;
  }

  { // validate sequences are 0..n
    if (!(sequence_idx[0] & 1)) {
      fprintf(stderr, "\nMissing sequence 0\n");
      goto cleanup_fail;
    }
    int last_seq = 0;
    for (int seq = 1; seq < 256; seq++) {
      int n = seq >> 6;
      uint64_t b = UINT64_C(1) << (seq & 0x3f);
      if (sequence_idx[n] & b) {
        if (seq != last_seq + 1) {
          if (seq != last_seq + 2) {
            fprintf(stderr, "\nSequences missing: %d-%d\n", last_seq + 1, seq - 1);
          } else {
            fprintf(stderr, "\nSequence missing: %d\n", last_seq + 1);
          }
          goto cleanup_fail;
        }
        last_seq = seq;
      }
    }
    sequence_length = last_seq + 1;
    if (sequence_length > 255) {
      fprintf(stderr, "\nCannot have more than 255 sequences\n");
      goto cleanup_fail;
    }
  }

  if (arrlen(pattern_names) > 65535) {
    fprintf(stderr, "\nCannot have more than 65535 patterns\n");
    goto cleanup_fail;
  }

  { // parse samples
    int state = 0;
    int new_line = 0;
    int new_chr = 0;
    int sample_index = -1;
    int sample_volume = -1;
    for (int i = 0; i < arrlen(tokens); i++) {
      struct token_st *tok = tokens[i];
      switch (state) {
        case 0:
          if (tok->type == TOK_IDENT && strcmp(tok->u.ident, "new") == 0) {
            new_line = tok->line;
            new_chr = tok->chr;
            state = 1;
          }
          break;
        case 1: // reading type
          if (tok->type == TOK_IDENT && strcmp(tok->u.ident, "sample") == 0) {
            sample_index = -1;
            sample_volume = -1;
            state = 3;
          } else
            state = 2;
          break;
        case 2: // waiting for 'end'
          if (tok->type == TOK_IDENT && strcmp(tok->u.ident, "end") == 0)
            state = 0;
          break;
        case 3: // sample
          if (tok->type == TOK_IDENT) {
            // find this sample
            int found = -1;
            for (int j = 0; j < arrlen(sample_names); j++) {
              if (strcmp(sample_names[j], tok->u.ident) == 0) {
                found = j;
                break;
              }
            }
            if (found < 0) {
              fprintf(stderr, "UNREACHABLE: Sample name missing\n");
              exit(1);
            }
            state = 4;
          }
          break;
        case 4: // search for key:value pair
          if (tok->type == TOK_IDENT && strcmp(tok->u.ident, "end") == 0) {
            if (sample_index < 0) {
              fprintf(stderr, "\n%s:%d:%d: Missing 'wave' in sample\n", input, new_line, new_chr);
              goto cleanup_fail;
            }
            if (sample_volume < 0)
              sample_volume = 16;
            struct song_sample_st *samp = calloc(sizeof(struct song_sample_st), 1);
            samp->index = sample_index;
            samp->volume = sample_volume;
            arrpush(song->samples, samp);
            state = 0;
          } else if (tok->type == TOK_IDENT && strcmp(tok->u.ident, "wave") == 0) {
            state = 5;
          } else if (tok->type == TOK_IDENT && strcmp(tok->u.ident, "volume") == 0) {
            state = 6;
          } else {
            fprintf(stderr, "\n%s:%d:%d: Unknown sample statement\n", input, tok->line, tok->chr);
            goto cleanup_fail;
          }
          break;
        case 5: // reading 'wave'
        case 6: // reading 'volume'
          if (tok->type == TOK_SPECIAL && tok->u.special == ':') {
            state += 2;
          } else {
            fprintf(stderr, "\n%s:%d:%d: Expecting ':'\n", input, tok->line, tok->chr);
            goto cleanup_fail;
          }
          break;
        case 7: // wave value
          if (tok->type == TOK_IDENT) {
            if (sample_index >= 0) {
              fprintf(stderr, "\n%s:%d:%d: Cannot set sample wave more than once\n", input,
                tok->line, tok->chr);
              goto cleanup_fail;
            }
            for (int j = 0; j < arrlen(sample_waves); j++) {
              if (strcmp(sample_waves[j], tok->u.ident) == 0) {
                sample_index = j;
                break;
              }
            }
            if (sample_index < 0) {
              fprintf(stderr, "\n%s:%d:%d: Unknown sample wave: %s\n", input,
                tok->line, tok->chr, tok->u.ident);
              goto cleanup_fail;
            }
            state = 4;
          } else {
            fprintf(stderr, "\n%s:%d:%d: Expecting wave name\n", input, tok->line, tok->chr);
            goto cleanup_fail;
          }
          break;
        case 8: // volume value
          if (tok->type == TOK_NUMBER) {
            if (tok->u.number < 1 || tok->u.number > 16) {
              fprintf(stderr, "\n%s:%d:%d: Volume must be 1-16\n", input, tok->line, tok->chr);
              goto cleanup_fail;
            }
            if (sample_volume >= 0) {
              fprintf(stderr, "\n%s:%d:%d: Cannot set sample volume more than once\n", input,
                tok->line, tok->chr);
              goto cleanup_fail;
            }
            sample_volume = tok->u.number;
            state = 4;
          } else {
            fprintf(stderr, "\n%s:%d:%d: Expecting volume number\n", input, tok->line, tok->chr);
            goto cleanup_fail;
          }
          break;
      }
    }
  }

  { // parse instruments
    int state = 0;
    int new_line = 0;
    int new_chr = 0;
    int inst_wave = -1;
    int inst_phase = -1;
    bool negative = false;
    struct song_sequence_st inst_volume = {0};
    struct song_sequence_st inst_pitch = {0};
    for (int i = 0; i < arrlen(tokens); i++) {
      struct token_st *tok = tokens[i];
      switch (state) {
        case 0:
          if (tok->type == TOK_IDENT && strcmp(tok->u.ident, "new") == 0) {
            new_line = tok->line;
            new_chr = tok->chr;
            state = 1;
          }
          break;
        case 1: // reading type
          if (tok->type == TOK_IDENT && strcmp(tok->u.ident, "instrument") == 0) {
            inst_wave = -1;
            inst_phase = -1;
            inst_volume.seq = NULL;
            inst_volume.loop = -1;
            inst_volume.exit = -1;
            inst_pitch.seq = NULL;
            inst_pitch.loop = -1;
            inst_pitch.exit = -1;
            negative = false;
            state = 3;
          } else
            state = 2;
          break;
        case 2: // waiting for 'end'
          if (tok->type == TOK_IDENT && strcmp(tok->u.ident, "end") == 0)
            state = 0;
          break;
        case 3: // instrument
          if (tok->type == TOK_IDENT) {
            // find this instrument
            int found = -1;
            for (int j = 0; j < arrlen(instrument_names); j++) {
              if (strcmp(instrument_names[j], tok->u.ident) == 0) {
                found = j;
                break;
              }
            }
            if (found < 0) {
              fprintf(stderr, "UNREACHABLE: Instrument name missing\n");
              exit(1);
            }
            state = 4;
          }
          break;
        case 4: // search for key:value pair
          if (tok->type == TOK_IDENT && strcmp(tok->u.ident, "end") == 0) {
            if (inst_wave < 0) {
              fprintf(stderr, "\n%s:%d:%d: Missing 'wave' in instrument\n", input, new_line,
                new_chr);
              goto cleanup_instfail;
            }
            if (inst_phase < 0)
              inst_phase = 0; // default to reset
            if (inst_volume.seq == NULL) {
              arrpush(inst_volume.seq, 16);
              arrpush(inst_volume.seq, 0);
              inst_volume.loop = 0;
              inst_volume.exit = 1;
            }
            if (inst_pitch.seq == NULL) {
              arrpush(inst_pitch.seq, 0);
              inst_pitch.loop = 0;
              inst_pitch.exit = 1;
            }
            if (arrlen(inst_volume.seq) >= 255) {
              fprintf(stderr, "\n%s:%d:%d: Volume envelope too long (max 255 entries)\n", input,
                new_line, new_chr);
              goto cleanup_instfail;
            }
            if (arrlen(inst_pitch.seq) >= 255) {
              fprintf(stderr, "\n%s:%d:%d: Pitch envelope too long (max 255 entries)\n", input,
                new_line, new_chr);
              goto cleanup_instfail;
            }
            struct song_instrument_st *inst = calloc(sizeof(struct song_instrument_st), 1);
            inst->wave = inst_wave;
            inst->phase = inst_phase;
            inst->volume = inst_volume;
            inst_volume.seq = NULL;
            inst->pitch = inst_pitch;
            inst_pitch.seq = NULL;
            arrpush(song->instruments, inst);
            state = 0;
          } else if (tok->type == TOK_IDENT && strcmp(tok->u.ident, "wave") == 0) {
            state = 5;
          } else if (tok->type == TOK_IDENT && strcmp(tok->u.ident, "phase") == 0) {
            state = 6;
          } else if (tok->type == TOK_IDENT && strcmp(tok->u.ident, "volume") == 0) {
            negative = false;
            state = 7;
          } else if (tok->type == TOK_IDENT && strcmp(tok->u.ident, "pitch") == 0) {
            negative = false;
            state = 8;
          } else {
            fprintf(stderr, "\n%s:%d:%d: Unknown sample statement\n", input, tok->line, tok->chr);
            goto cleanup_instfail;
          }
          break;
        case 5: // reading 'wave'
        case 6: // reading 'phase'
        case 7: // reading 'volume'
        case 8: // reading 'pitch'
          if (tok->type == TOK_SPECIAL && tok->u.special == ':') {
            state += 4;
          } else {
            fprintf(stderr, "\n%s:%d:%d: Expecting ':'\n", input, tok->line, tok->chr);
            goto cleanup_instfail;
          }
          break;
        case 9: // wave value
          if (tok->type == TOK_IDENT) {
            if (inst_wave >= 0) {
              fprintf(stderr, "\n%s:%d:%d: Cannot set instrument wave more than once\n", input,
                tok->line, tok->chr);
              goto cleanup_instfail;
            }
            for (int j = 0; j < arrlen(instrument_waves); j++) {
              if (strcmp(instrument_waves[j], tok->u.ident) == 0) {
                inst_wave = j;
                break;
              }
            }
            if (inst_wave < 0) {
              fprintf(stderr, "\n%s:%d:%d: Unknown instrument wave: %s\n", input,
                tok->line, tok->chr, tok->u.ident);
              goto cleanup_instfail;
            }
            state = 4;
          } else {
            fprintf(stderr, "\n%s:%d:%d: Expecting wave name\n", input, tok->line, tok->chr);
            goto cleanup_instfail;
          }
          break;
        case 10: // phase value
          {
            int reset = tok->type == TOK_IDENT && strcmp(tok->u.ident, "reset") == 0;
            int conti = tok->type == TOK_IDENT && strcmp(tok->u.ident, "continue") == 0;
            if (reset || conti) {
              if (inst_phase >= 0) {
                fprintf(stderr, "\n%s:%d:%d: Cannot set instrument phase more than once\n", input,
                  tok->line, tok->chr);
                goto cleanup_instfail;
              }
              inst_phase = reset ? 0 : 1;
              state = 4;
            } else {
              fprintf(stderr, "\n%s:%d:%d: Expecting phase to be 'reset' or 'continue'\n", input,
                tok->line, tok->chr);
              goto cleanup_instfail;
            }
          }
          break;
        case 11: // volume value
        case 12: // pitch value
          {
            struct song_sequence_st *seq;
            int min, max;
            if (state == 11) { // volume
              seq = &inst_volume;
              min = 0;
              max = 16;
            } else { // pitch
              seq = &inst_pitch;
              min = -128;
              max = 127;
            }
            if (negative && tok->type != TOK_NUMBER) {
              fprintf(stderr, "\n%s:%d:%d: Malformed negative number\n", input, tok->line,
                tok->chr);
              goto cleanup_instfail;
            }
            if (tok->type == TOK_NUMBER) {
              int n = (negative ? -1 : 1) * tok->u.number;
              if (n < min || n > max) {
                fprintf(stderr, "\n%s:%d:%d: Number %d outside range [%d, %d]\n", input, tok->line,
                  tok->chr, n, min, max);
                goto cleanup_instfail;
              }
              arrpush(seq->seq, n);
              negative = false;
            } else if (tok->type == TOK_SPECIAL && tok->u.special == '-') {
              negative = true;
            } else if (tok->type == TOK_SPECIAL && tok->u.special == '{') {
              if (seq->loop >= 0) {
                fprintf(stderr, "\n%s:%d:%d: Cannot set start loop more than once\n", input,
                  tok->line, tok->chr);
                goto cleanup_instfail;
              }
              seq->loop = arrlen(seq->seq);
            } else if (tok->type == TOK_SPECIAL && tok->u.special == '}') {
              if (seq->loop < 0) {
                fprintf(stderr, "\n%s:%d:%d: Missing matching '{'\n", input, tok->line,
                  tok->chr);
                goto cleanup_instfail;
              }
              if (seq->exit >= 0) {
                fprintf(stderr, "\n%s:%d:%d: Cannot set end loop more than once\n", input,
                  tok->line, tok->chr);
                goto cleanup_instfail;
              }
              seq->exit = arrlen(seq->seq);
            } else {
              if (seq->loop >= 0 && seq->exit < 0) {
                fprintf(stderr, "\n%s:%d:%d: Missing matching '}'\n", input, tok->line,
                  tok->chr);
                goto cleanup_instfail;
              }
              if (arrlen(seq->seq) <= 0) {
                // push 16 for volume env, 0 for pitch env
                arrpush(seq->seq, state == 11 ? 16 : 0);
              }
              if (seq->loop < 0) {
                seq->loop = arrlen(seq->seq) - 1;
                seq->exit = arrlen(seq->seq);
              }
              if (seq->seq[arrlen(seq->seq) - 1] != 0) {
                arrpush(seq->seq, 0);
              }
              i--;
              state = 4;
            }
          }
          break;
      }
    }
    goto skip_instfail;
cleanup_instfail:
    arrfree(inst_volume.seq);
    arrfree(inst_pitch.seq);
    goto cleanup_fail;
skip_instfail:;
  }

  { // parse patterns
    int state = 0;
    int new_line = 0;
    int new_chr = 0;
    int last_time = 0;
    int pat_channels = -1;
    struct song_patternline_st **lines = NULL;
    for (int i = 0; i < arrlen(tokens); i++) {
      struct token_st *tok = tokens[i];
      switch (state) {
        case 0:
          if (tok->type == TOK_IDENT && strcmp(tok->u.ident, "new") == 0) {
            new_line = tok->line;
            new_chr = tok->chr;
            state = 1;
          }
          break;
        case 1: // reading type
          if (tok->type == TOK_IDENT && strcmp(tok->u.ident, "pattern") == 0) {
            state = 3;
          } else
            state = 2;
          break;
        case 2: // waiting for 'end'
          if (tok->type == TOK_IDENT && strcmp(tok->u.ident, "end") == 0)
            state = 0;
          break;
        case 3: // pattern
          {
            int part = -1;
            if (
              tokens[i + 1]->type == TOK_SPECIAL &&
              tokens[i + 1]->u.special == ':'
            ) {
              // parse part
              part = tokens[i + 2]->u.number;
              i += 2;
            }
            // find this pattern
            int found = -1;
            for (int j = 0; j < arrlen(pattern_names); j++) {
              if (
                strcmp(pattern_names[j]->name, tok->u.ident) == 0 &&
                pattern_names[j]->part == part
              ) {
                found = j;
                break;
              }
            }
            if (found < 0) {
              fprintf(stderr, "UNREACHABLE: Pattern name/part missing\n");
              exit(1);
            }
            last_time = 0;
            pat_channels = -1;
            lines = NULL;
            state = 4;
          }
          break;
        case 4: // parse pattern lines
          if (tok->type == TOK_IDENT && strcmp(tok->u.ident, "end") == 0) {
            if (arrlen(lines) <= 0) {
              fprintf(stderr, "\n%s:%d:%d: Empty pattern\n", input, new_line, new_chr);
              goto cleanup_patfail;
            }
            struct song_pattern_st *pat = malloc(sizeof(struct song_pattern_st));
            pat->lines = lines;
            lines = NULL;
            arrpush(song->patterns, pat);
            state = 0;
          } else if (tok->type == TOK_NUMBER) {
            // got timestamp, calculate wait
            int wait = tok->u.number - last_time;
            last_time = tok->u.number;
            if (wait < 0) {
              fprintf(stderr, "\n%s:%d:%d: Timestamps must not decrease\n", input, tok->line,
                tok->chr);
              goto cleanup_patfail;
            }
            if (arrlen(lines) <= 0) {
              if (wait != 0) {
                fprintf(stderr, "\n%s:%d:%d: First row must start on time 00\n", input, tok->line,
                  tok->chr);
                goto cleanup_patfail;
              }
            } else {
              // see if we can remove the last line
              if (arrlen(lines) > 1) {
                struct song_patternline_st *last_line = lines[arrlen(lines) - 1];
                bool empty = true;
                for (int k = 0; k < 16 && empty; k++)
                  empty = last_line->commands[k] == 0;
                if (empty) {
                  free(last_line);
                  arrdel(lines, arrlen(lines) - 1);
                }
              }
              lines[arrlen(lines) - 1]->wait += wait;
            }

            struct song_patternline_st *line = calloc(sizeof(struct song_patternline_st), 1);

            // parse line commands
            i++;
            int cur_channels = 0;
            for (int channel = 0; channel <= 16; channel++, i++) {
              tok = tokens[i];
              if (tok->type != TOK_STR)
                break;
              if (channel >= 16) {
                free(line);
                fprintf(stderr, "\n%s:%d:%d: Cannot exceed 16 channels\n", input, tok->line,
                  tok->chr);
                goto cleanup_patfail;
              }
              // parse str into cmd
              cur_channels = channel + 1;
              const char *str = tok->u.str;
              uint16_t cmd = 0;
              if (strcmp(str, "---:---") != 0) {
                int note = song_parsenote(str);
                if (note == -1) { // not found... maybe it's a sample?
                  for (int j = 0; j < arrlen(sample_names); j++) {
                    if (
                      sample_names[j][0] == str[0] &&
                      sample_names[j][1] == str[1] &&
                      sample_names[j][2] == str[2]
                    ) {
                      note = 128 + j;
                      break;
                    }
                  }
                  if (note < 0) {
                    free(line);
                    fprintf(stderr, "\n%s:%d:%d: Invalid note: %s\n", input, tok->line, tok->chr,
                      str);
                    goto cleanup_patfail;
                  }
                } else if (note == -2) { // note out of range
                  free(line);
                  fprintf(stderr, "\n%s:%d:%d: Note out of range: %s\n", input, tok->line,
                    tok->chr, str);
                  goto cleanup_patfail;
                }
                int eff = song_parseeffect(&str[4]);
                if (eff == -1) { // not found... maybe it's an instrument?
                  for (int j = 0; j < arrlen(instrument_names); j++) {
                    if (
                      instrument_names[j][0] == str[4] &&
                      instrument_names[j][1] == str[5] &&
                      instrument_names[j][2] == str[6]
                    ) {
                      // 0-16 reserved for volume, 17 reserved for instrument 0, so offset
                      // the instrument command by 18
                      eff = 0x40 + 18 + j;
                      break;
                    }
                  }
                  if (eff < 0) {
                    free(line);
                    fprintf(stderr, "\n%s:%d:%d: Invalid effect: %s\n", input, tok->line, tok->chr,
                      str);
                    goto cleanup_patfail;
                  }
                } else if (eff == -2) { // tempo out of range
                  free(line);
                  fprintf(stderr, "\n%s:%d:%d: Tempo out of range (045-202): %s\n", input,
                    tok->line, tok->chr, str);
                  goto cleanup_patfail;
                } else if (eff == -3) { // bend out of range
                  free(line);
                  fprintf(stderr, "\n%s:%d:%d: Bend out of range (B00-B64): %s\n", input, tok->line,
                    tok->chr, str);
                  goto cleanup_patfail;
                } else if (eff == -4) { // delay out of range
                  free(line);
                  fprintf(stderr, "\n%s:%d:%d: Delay out of range (D00-D60): %s\n", input,
                    tok->line, tok->chr, str);
                  goto cleanup_patfail;
                } else if (eff == -5) { // volume out of range
                  free(line);
                  fprintf(stderr, "\n%s:%d:%d: Volume out of range (V00-V16): %s\n", input,
                    tok->line, tok->chr, str);
                  goto cleanup_patfail;
                }
                if (eff < 0 || eff > 255 || note < 0 || note > 255) {
                  fprintf(stderr, "UNREACHABLE: Invalid note/effect\n");
                  exit(1);
                }
                cmd = (eff << 8) | note;
              }

              // TODO: verify cmd is not a bend effect + PCM note
              line->commands[channel] = cmd;
            }

            // verify channel count
            if (cur_channels == 0) {
              free(line);
              fprintf(stderr, "\n%s:%d:%d: No channel information\n", input, tok->line, tok->chr);
              goto cleanup_patfail;
            }
            if (pat_channels < 0) {
              pat_channels = cur_channels;
              if (song->channel_count < pat_channels)
                song->channel_count = pat_channels;
            } else if (pat_channels != cur_channels) {
              free(line);
              fprintf(stderr, "\n%s:%d:%d: Inconsistent channel count\n", input, tok->line,
                tok->chr);
              goto cleanup_patfail;
            }

            arrpush(lines, line);
            i--;
          } else {
            fprintf(stderr, "\n%s:%d:%d: Malformed pattern line\n", input, tok->line, tok->chr);
            goto cleanup_patfail;
          }
          break;
      }
    }
    goto skip_patfail;
cleanup_patfail:
    for (int i = 0; i < arrlen(lines); i++)
      free(lines[i]);
    arrfree(lines);
    goto cleanup_fail;
skip_patfail:;
  }

  { // parse sequences in order
    struct song_sequence_st *seq = NULL;
    for (int seq_i = 0; seq_i < sequence_length; seq_i++) {
      int state = 0;
      for (int i = 0; i < arrlen(tokens); i++) {
        struct token_st *tok = tokens[i];
        switch (state) {
          case 0:
            if (tok->type == TOK_IDENT && strcmp(tok->u.ident, "new") == 0) {
              state = 1;
            }
            break;
          case 1: // reading type
            if (
              tok->type == TOK_IDENT &&
              strcmp(tok->u.ident, "sequence") == 0 &&
              tokens[i + 1]->type == TOK_NUMBER &&
              tokens[i + 1]->u.number == seq_i
            ) {
              // found *this* sequence
              seq = malloc(sizeof(struct song_sequence_st));
              seq->seq = NULL;
              seq->loop = -1;
              seq->exit = -1;
              i++;
              state = 3;
            } else
              state = 2;
            break;
          case 2: // waiting for 'end'
            if (tok->type == TOK_IDENT && strcmp(tok->u.ident, "end") == 0)
              state = 0;
            break;
          case 3: // sequence
            if (tok->type == TOK_IDENT && strcmp(tok->u.ident, "end") == 0) {
              if (seq->loop >= 0 && seq->exit < 0) {
                fprintf(stderr, "\n%s:%d:%d: Missing matching '}'\n", input, tok->line,
                  tok->chr);
                goto cleanup_seqfail;
              }
              if (seq->loop < 0) {
                seq->loop = 0;
                seq->exit = arrlen(seq->seq);
              }
              arrpush(song->sequences, seq);
              seq = NULL;
              goto parse_next_sequence;
            } else if (tok->type == TOK_IDENT) {
              int start_part = -1;
              int end_part = -1;
              if (
                tokens[i + 1]->type == TOK_SPECIAL &&
                tokens[i + 1]->u.special == ':' &&
                tokens[i + 2]->type == TOK_NUMBER
              ) {
                start_part = tokens[i + 2]->u.number;
                i += 2;
                if (
                  tokens[i + 1]->type == TOK_SPECIAL &&
                  tokens[i + 1]->u.special == '-' &&
                  tokens[i + 2]->type == TOK_NUMBER
                ) {
                  end_part = tokens[i + 2]->u.number;
                  i += 2;
                }
              }
              int part = start_part;
              while (true) {
                // find this pattern
                int found = -1;
                for (int j = 0; j < arrlen(pattern_names); j++) {
                  if (
                    strcmp(pattern_names[j]->name, tok->u.ident) == 0 &&
                    pattern_names[j]->part == part
                  ) {
                    found = j;
                    break;
                  }
                }
                if (found < 0) {
                  if (part < 0) {
                    fprintf(stderr, "\n%s:%d:%d: Unknown pattern \"%s\"\n", input, tok->line,
                      tok->chr, tok->u.ident);
                  } else {
                    fprintf(stderr, "\n%s:%d:%d: Unknown pattern \"%s:%d\"\n", input, tok->line,
                      tok->chr, tok->u.ident, part);
                  }
                  goto cleanup_seqfail;
                }
                arrpush(seq->seq, found);

                // move towards end_part
                if (end_part < 0 || part == end_part)
                  break;
                else if (end_part < part)
                  part--;
                else
                  part++;
              }
            } else if (tok->type == TOK_SPECIAL && tok->u.special == '{') {
              if (seq->loop >= 0) {
                fprintf(stderr, "\n%s:%d:%d: Cannot set start loop more than once\n", input,
                  tok->line, tok->chr);
                goto cleanup_seqfail;
              }
              seq->loop = arrlen(seq->seq);
            } else if (tok->type == TOK_SPECIAL && tok->u.special == '}') {
              if (seq->loop < 0) {
                fprintf(stderr, "\n%s:%d:%d: Missing matching '{'\n", input, tok->line,
                  tok->chr);
                goto cleanup_seqfail;
              }
              if (seq->exit >= 0) {
                fprintf(stderr, "\n%s:%d:%d: Cannot set exit loop more than once\n", input,
                  tok->line, tok->chr);
                goto cleanup_seqfail;
              }
              seq->exit = arrlen(seq->seq);
            } else {
              fprintf(stderr, "\n%s:%d:%d: Unknown sequence statement\n", input, tok->line,
                tok->chr);
              goto cleanup_seqfail;
            }
            break;
        }
      }
parse_next_sequence:;
    }
    goto skip_seqfail;
cleanup_seqfail:
    if (seq) {
      arrfree(seq->seq);
      free(seq);
    }
    goto cleanup_fail;
skip_seqfail:;
  }

  // ALL DONE!

#if 0
  // print everything out (debug)
  for (int i = 0; i < arrlen(instrument_waves); i++) {
    printf("instrument_waves[%d] = \"%s\"\n", i, instrument_waves[i]);
  }
  for (int i = 0; i < arrlen(sample_waves); i++) {
    printf("sample_waves[%d] = \"%s\"\n", i, sample_waves[i]);
  }
  for (int i = 0; i < arrlen(tokens); i++) {
    struct token_st *tok = tokens[i];
    switch (tok->type) {
      case TOK_IDENT:
        printf("[%d] %d:%d ident \"%s\"\n", i, tok->line, tok->chr, tok->u.ident);
        break;
      case TOK_SPECIAL:
        printf("[%d] %d:%d special '%c'\n", i, tok->line, tok->chr, tok->u.special);
        break;
      case TOK_NUMBER:
        printf("[%d] %d:%d number %d\n", i, tok->line, tok->chr, tok->u.number);
        break;
      case TOK_STR:
        printf("[%d] %d:%d str \"%s\"\n", i, tok->line, tok->chr, tok->u.str);
        break;
    }
  }
  for (int i = 0; i < arrlen(instrument_names); i++) {
    printf("instrument_names[%d] = \"%s\"\n", i, instrument_names[i]);
  }
  for (int i = 0; i < arrlen(sample_names); i++) {
    printf("sample_names[%d] = \"%s\"\n", i, sample_names[i]);
  }
  for (int i = 0; i < arrlen(pattern_names); i++) {
    printf("pattern_names[%d] = \"%s\" part %d\n", i, pattern_names[i]->name,
      pattern_names[i]->part);
  }
  printf("sequences:");
  for (int i = 0; i < 4; i++) {
    for (int b = 0; b < 64; b++) {
      if (sequence_idx[i] & (UINT64_C(1) << b)) {
        printf(" %d", i * 64 + b);
      }
    }
  }
  printf("\n");
  song_print(song);
#endif

  goto cleanup;
cleanup_fail:
  song_free(song);
  song = NULL;
cleanup:
  for (int i = 0; i < arrlen(pattern_names); i++)
    free(pattern_names[i]);
  arrfree(pattern_names);
  arrfree(instrument_names);
  arrfree(sample_names);
  for (int i = 0; i < arrlen(tokens); i++) {
    struct token_st *tok = tokens[i];
    if (tok->type == TOK_IDENT)
      free(tok->u.ident);
    else if (tok->type == TOK_STR)
      free(tok->u.str);
    free(tokens[i]);
  }
  arrfree(tokens);
  for (int i = 0; i < arrlen(sample_waves); i++)
    free(sample_waves[i]);
  arrfree(sample_waves);
  arrfree(instrument_waves);
  #undef TOK_IDENT
  #undef TOK_SPECIAL
  #undef TOK_NUMBER
  #undef TOK_STR
  return song;
}

void song_serialize(struct song_st *song, song_serialize_write f_write, void *stream) {
  uint8_t *out = NULL;
  #define U8(n) \
    do { uint8_t t = (n); arrpush(out, t); } while (false)
  #define U16(n) \
    do { uint16_t t = (n); arrpush(out, t & 0xff); arrpush(out, (t >> 8) & 0xff); } while (false)
  #define U32(n) \
    do { uint32_t t = (n); arrpush(out, t & 0xff); arrpush(out, (t >> 8) & 0xff); \
      arrpush(out, (t >> 16) & 0xff); arrpush(out, (t >> 24) & 0xff); } while (false)
  #define ALIGN(n) \
    do { while ((arrlen(out) % n) != 0) { arrpush(out, 0); } } while (false)
  #define SET16(i, n) \
    do { int k = (i); uint16_t t = (n); \
      out[k++] = t & 0xff; out[k] = (t >> 8) & 0xff; } while (false)
  #define SET32(i, n) \
    do { int k = (i); uint32_t t = (n); \
      out[k++] = t & 0xff; out[k++] = (t >> 8) & 0xff; \
      out[k++] = (t >> 16) & 0xff; out[k] = (t >> 24) & 0xff; } while (false)

  // header
  U32(0x737667fb);               // magic 'gvs'
  U8(0x80);                      // version
  U8(song->channel_count);       // channel count
  U8(0);                         // reserved
  U8(arrlen(song->samples));     // samples length
  U8(arrlen(song->instruments)); // instruments length
  U8(arrlen(song->sequences));   // sequences length
  U16(arrlen(song->patterns));   // patterns length
  int inst_table_offset = arrlen(out);
  U32(0);
  int seq_table_offset = arrlen(out);
  U32(0);
  int pat_table_offset = arrlen(out);
  U32(0);

  // samples
  for (int i = 0; i < arrlen(song->samples); i++) {
    struct song_sample_st *samp = song->samples[i];
    U16(((samp->volume - 1) << 12) | samp->index);
  }
  ALIGN(4);

  // instrument table
  SET32(inst_table_offset, arrlen(out));
  int inst_offset = arrlen(out);
  for (int i = 0; i < arrlen(song->instruments); i++) {
    U32(0);
  }

  // sequence table
  SET32(seq_table_offset, arrlen(out));
  int seq_offset = arrlen(out);
  for (int i = 0; i < arrlen(song->sequences); i++) {
    U32(0);
  }

  // pattern table
  SET32(pat_table_offset, arrlen(out));
  int pat_offset = arrlen(out);
  for (int i = 0; i < arrlen(song->patterns); i++) {
    U32(0);
  }

  // instruments
  for (int i = 0; i < arrlen(song->instruments); i++) {
    int inst_label = arrlen(out);
    SET32(inst_offset + i * 4, inst_label);
    struct song_instrument_st *inst = song->instruments[i];
    U16((inst->phase << 15) | inst->wave); // phase + wave
    U8(inst->volume.loop);                 // volume attack
    U8(inst->volume.exit);                 // volume sustain
    U8(arrlen(inst->volume.seq));          // volume length
    U8(inst->pitch.loop);                  // pitch attack
    U8(inst->pitch.exit);                  // pitch sustain
    U8(arrlen(inst->pitch.seq));           // pitch length
    int volume_offset = arrlen(out);       // volume env offset
    U16(0);
    int pitch_offset = arrlen(out);        // pitch env offset
    U16(0);
    SET16(volume_offset, arrlen(out) - inst_label);
    for (int j = 0; j < arrlen(inst->volume.seq); j++) {
      U8(inst->volume.seq[j]);
    }
    ALIGN(2);
    SET16(pitch_offset, arrlen(out) - inst_label);
    for (int j = 0; j < arrlen(inst->pitch.seq); j++) {
      U8(inst->pitch.seq[j]);
    }
    ALIGN(2);
  }

  // sequences
  for (int i = 0; i < arrlen(song->sequences); i++) {
    SET32(seq_offset + i * 4, arrlen(out));
    struct song_sequence_st *seq = song->sequences[i];
    U16(arrlen(seq->seq));
    U16(seq->loop);
    U16(seq->exit);
    for (int j = 0; j < arrlen(seq->seq); j++) {
      U16(seq->seq[j]);
    }
  }

  // patterns
  for (int i = 0; i < arrlen(song->patterns); i++) {
    SET32(pat_offset + i * 4, arrlen(out));
    struct song_pattern_st *pat = song->patterns[i];
    for (int j = 0; j < arrlen(pat->lines); j++) {
      struct song_patternline_st *line = pat->lines[j];
      for (int k = 0; k < song->channel_count; k++) {
        U16(line->commands[k]);
      }
      U16(line->wait);
    }
  }

  f_write(out, sizeof(char), arrlen(out), stream);
  arrfree(out);
  #undef U8
  #undef U16
  #undef U32
  #undef ALIGN
  #undef SET16
  #undef SET32
}
