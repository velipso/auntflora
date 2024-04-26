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
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#include "json.h"
#include "snd.h"

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;

static void print_usage() {
  printf(
    "Usage:\n"
    "  xform <command> [options...]\n"
    "\n"
    "Commands:\n"
    "\n"
    "  snd <more...>\n"
    "    Interact with the sound subsystem\n"
    "\n"
    "  fix <input.gba>\n"
    "    Patches the ROM header and pads to power of 2\n"
    "\n"
    "  palette256 <output.bin> [input1.png input2.png ...]\n"
    "    Create a 256 color palette from the input files\n"
    "\n"
    "  expand6x6to8x8 <input.png> <palette.bin> <output.bin>\n"
    "    Outputs 8x8 tiles from 6x6 source image\n"
    "\n"
    "  copy8x8 <input.png> <palette.bin> <output.bin>\n"
    "    Outputs 8x8 tiles from 8x8 source image\n"
    "\n"
    "  world <input.json> <bg.bin> <logic.bin> <sprite.bin>\n"
    "    Process Tiled world data\n"
  );
}

// align files to 4 bytes... required to keep linker in alignment (???)
static void fclose4(FILE *fp) {
  long pos = ftell(fp);
  int pad = 4 - (pos % 4);
  for (int i = 0; i < pad; i++)
    fputc(0, fp);
  fclose(fp);
}

static int fix_rom(const char *file) {
  printf("Fixing: %s\n", file);
  FILE *fp = fopen(file, "r+b");

  { // fix the header CRC
    int8_t header[0xc0];
    if (fread(&header, sizeof(header), 1, fp) != 1) {
      fclose(fp);
      fprintf(stderr, "\nFailed to read header: %s\n", file);
      return 1;
    }
    int8_t crc = 0;
    for (int n = 0xa0; n < 0xbd; n++) {
      crc += header[n];
    }
    header[0xbd] = -(0x19 + crc);
    fseek(fp, 0, SEEK_SET);
    fwrite(&header, sizeof(header), 1, fp);
  }

  { // pad with 0xff to the next power of 2
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);

    // round up to next power of 2
    long p = size;
    p--;
    p |= p >> 1;
    p |= p >> 2;
    p |= p >> 4;
    p |= p >> 8;
    p |= p >> 16;
    p++;

    while (size < p) {
      fputc(0xff, fp);
      size++;
    }
  }

  fclose(fp);
  printf("ROM fixed!\n");
  return 0;
}

static u16 *readpal(const char *file, int *entries) {
  FILE *fp = fopen(file, "rb");
  if (fp == NULL) {
    fprintf(stderr, "\nFailed to read palette: %s\n", file);
    exit(1);
  }
  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  u8 *data = malloc(size);
  *entries = size / 2;
  fread(data, size, 1, fp);
  fclose(fp);
  return (u16 *)data;
}

static void single6x6to8x8(u8 *dst, const u8 *src, int srcbytes) {
  const int map[] = {
     0, 1, 2, 2, 3, 4, 5, 5,
     6, 7, 8, 8, 9,10,11,11,
    12,13,14,14,15,16,17,17,
    12,13,14,14,15,16,17,17,
    18,19,20,20,21,22,23,23,
    24,25,26,26,27,28,29,29,
    30,31,32,32,33,34,35,35,
    30,31,32,32,33,34,35,35,
  };
  int tiles = srcbytes / 36;
  for (int t = 0; t < tiles; t++) {
    for (int i = 0; i < 64; i++) {
      dst[i] = src[map[i]];
    }
    src += 36;
    dst += 64;
  }
}

static int findpal(u32 color, const u16 *palette, int maxpal) {
  int r = (color >> 0) & 0xff;
  int g = (color >> 8) & 0xff;
  int b = (color >> 16) & 0xff;
  int a = (color >> 24) & 0xff;
  if (a != 0xff) {
    return 0;
  }
  u16 rgb = (r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10);
  for (int i = 1; i < maxpal; i++) {
    if (palette[i] == rgb) {
      return i;
    }
  }
  return -1;
}

static int expand6x6to8x8(const char *input, const char *palette, const char *output) {
  int maxpal;
  u16 *pal = readpal(palette, &maxpal);
  FILE *fp = fopen(input, "rb");
  if (fp == NULL) {
    fprintf(stderr, "\nFailed to read: %s\n", input);
    return 1;
  }
  int width, height;
  u32 *data = (u32 *)stbi_load_from_file(fp, &width, &height, NULL, 4);
  fclose(fp);

  fp = fopen(output, "wb");
  if (fp == NULL) {
    fprintf(stderr, "\nFailed to write: %s\n", output);
    return 1;
  }

  int tcx = width / 6;
  int tcy = height / 6;
  for (int ty = 0; ty < tcy; ty++) {
    for (int tx = 0; tx < tcx; tx++) {
      u8 src[36] = {0};

      // load 6x6 tile into src
      for (int py = 0; py < 6; py++) {
        int y = ty * 6 + py;
        for (int px = 0; px < 6; px++) {
          int x = tx * 6 + px;
          int k = x + y * width;
          int c = findpal(data[k], pal, maxpal);
          if (c < 0) {
            fprintf(stderr, "\nCannot find color at (%d, %d) in %s\n", x, y, input);
          }
          src[px + py * 6] = c;
        }
      }

      // expand
      u8 dst[64];
      single6x6to8x8(dst, src, sizeof(src));

      // write results
      fwrite(dst, sizeof(dst), 1, fp);
    }
  }

  fclose4(fp);
  stbi_image_free(data);
  free(pal);
  return 0;
}

static int copy8x8(const char *input, const char *palette, const char *output) {
  int maxpal;
  u16 *pal = readpal(palette, &maxpal);
  FILE *fp = fopen(input, "rb");
  if (fp == NULL) {
    fprintf(stderr, "\nFailed to read: %s\n", input);
    return 1;
  }
  int width, height;
  u32 *data = (u32 *)stbi_load_from_file(fp, &width, &height, NULL, 4);
  fclose(fp);

  fp = fopen(output, "wb");
  if (fp == NULL) {
    fprintf(stderr, "\nFailed to write: %s\n", output);
    return 1;
  }

  int tcx = width / 8;
  int tcy = height / 8;
  for (int ty = 0; ty < tcy; ty++) {
    for (int tx = 0; tx < tcx; tx++) {
      u8 src[64] = {0};

      // load 8x8 tile into src
      for (int py = 0; py < 8; py++) {
        int y = ty * 8 + py;
        for (int px = 0; px < 8; px++) {
          int x = tx * 8 + px;
          int k = x + y * width;
          int c = findpal(data[k], pal, maxpal);
          if (c < 0) {
            fprintf(stderr, "\nCannot find color at (%d, %d) in %s\n", x, y, input);
          }
          src[px + py * 8] = c;
        }
      }

      // write results
      fwrite(src, sizeof(src), 1, fp);
    }
  }

  fclose4(fp);
  stbi_image_free(data);
  free(pal);
  return 0;
}

static int addpalette(const char *input, u16 *palette, int *nextpal, int maxpal) {
  FILE *fp = fopen(input, "rb");
  int width, height;
  u32 *data = (u32 *)stbi_load_from_file(fp, &width, &height, NULL, 4);
  if (data == NULL) {
    fprintf(stderr, "\nFailed to read: %s\n", input);
    return 1;
  }
  for (int i = 0; i < width * height; i++) {
    u32 color = data[i];
    int r = (color >> 0) & 0xff;
    int g = (color >> 8) & 0xff;
    int b = (color >> 16) & 0xff;
    int a = (color >> 24) & 0xff;
    if (a != 0xff) {
      continue;
    }
    u16 rgb = (r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10);
    bool found = false;
    for (int j = 1; j < *nextpal && !found; j++) {
      found = palette[j] == rgb;
    }
    if (!found) {
      if (*nextpal >= maxpal) {
        fprintf(stderr, "\nOut of palette space!\n");
        return 1;
      }
      palette[*nextpal] = rgb;
      *nextpal = *nextpal + 1;
    }
  }
  stbi_image_free(data);
  return 0;
}

static int *json_tiled_layer(json_value *tiled, const char *layer_name, int *data_count) {
  json_value *layers = json_objectkey(tiled, "layers");
  if (layers->kind != JSON_ARRAY) {
    fprintf(stderr, "\nExpecting \"layers\" to be an array\n");
    exit(1);
  }
  for (int i = 0; i < layers->u.array.count; i++) {
    json_value *layer = layers->u.array.values[i];
    if (strcmp(json_string(json_objectkey(layer, "name")), layer_name) == 0) {
      json_value *data = json_objectkey(layer, "data");
      if (data->kind != JSON_ARRAY) {
        fprintf(stderr, "\nExpecting \"data\" to be an array\n");
        exit(1);
      }
      int *result = malloc(sizeof(int) * data->u.array.count);
      if (data_count)
        *data_count = data->u.array.count;
      for (int j = 0; j < data->u.array.count; j++) {
        result[j] = json_number(data->u.array.values[j]);
      }
      return result;
    }
  }
  fprintf(stderr, "\nMissing layer \"%s\"\n", layer_name);
  exit(1);
}

static int process_world(json_value *jv, FILE *fpbg, FILE *fplogic, FILE *fpsprite) {
  // marks whether a background tile is solid or not
  const int is_solid[] = {
    // 0 = not solid
    // 1 = solid
    // 2 = shouldn't be on base layer
    0, 1, 1, 0, 1, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1,
    0, 1, 0, 0, 0, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1,
    0, 1, 2, 2, 1, 1, 1, 1, 2, 2, 1, 1, 1, 1, 1, 1,
    0, 2, 2, 2, 1, 1, 1, 1, 2, 2, 1, 1, 1, 1, 1, 1,
  };
  int width = json_number(json_objectkey(jv, "width"));
  int height = json_number(json_objectkey(jv, "height"));
  int *base = json_tiled_layer(jv, "base", NULL);
  int *objs = json_tiled_layer(jv, "objs", NULL);
  int *markers = json_tiled_layer(jv, "markers", NULL);
  int w = width / 2;
  int h = height / 2;
  int warnings = 0;

  int bg_size = width * height;
  uint8_t *bg = malloc(sizeof(uint8_t) * bg_size);
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      int b = base[x + y * width];
      if (b <= 0) {
        warnings++;
        fprintf(stderr, "WARNING: base tile is transparent at (%d, %d)\n", x, y);
      } else {
        b--;
      }
      int bx = (b % 32) / 2;
      int by = (b / 32) / 2;
      if (is_solid[bx + by * 16] == 2) {
        warnings++;
        fprintf(stderr,
          "WARNING: base tile is using graphics reserved for objects at (%d, %d)\n",
          x, y
        );
      }
      bg[x + y * width] = b;
    }
  }
  fwrite(bg, sizeof(uint8_t), bg_size, fpbg);

  int logic_size = w * h + 2;
  uint16_t *logic_res = malloc(sizeof(uint16_t) * logic_size);
  logic_res[0] = w;
  logic_res[1] = h;
  uint16_t *logic = &logic_res[2];
  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      int o1 = objs[(x * 2 + 0) + (y * 2 + 0) * width];
      int o2 = objs[(x * 2 + 1) + (y * 2 + 0) * width];
      int o3 = objs[(x * 2 + 0) + (y * 2 + 1) * width];
      int o4 = objs[(x * 2 + 1) + (y * 2 + 1) * width];
      if (o1 != 0 && (o2 != o1 + 1 || o3 != o1 + 32 || o4 != o1 + 33)) {
        warnings++;
        fprintf(stderr, "WARNING: Bad objs tile data at (%d, %d)\n", x * 2, y * 2);
      }
      int b = base[(x * 2 + 0) + (y * 2 + 0) * width];
      int bx = ((b - 1) % 32) / 2;
      int by = ((b - 1) / 32) / 2;
      b = bx + by * 16;
      int ox = o1 == 0 ? 0 : ((o1 - 1) % 32) / 2;
      int oy = o1 == 0 ? 0 : ((o1 - 1) / 32) / 2;
      int o = ox + oy * 16;
      if (is_solid[b])
        o |= 0x8000;
      logic[x + y * w] = o;
    }
  }
  fwrite(logic_res, sizeof(uint16_t), logic_size, fplogic);

  uint16_t *sprite = NULL;
  int sprite_count = 0;
  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      int m1 = markers[(x * 2 + 0) + (y * 2 + 0) * width];
      int m2 = markers[(x * 2 + 1) + (y * 2 + 0) * width];
      int m3 = markers[(x * 2 + 0) + (y * 2 + 1) * width];
      int m4 = markers[(x * 2 + 1) + (y * 2 + 1) * width];
      if (m1 != 0 && (m2 != m1 + 1 || m3 != m1 + 32 || m4 != m1 + 33)) {
        warnings++;
        fprintf(stderr, "WARNING: Bad marker tile data at (%d, %d)\n", x * 2, y * 2);
      }
      if (m1 != 0) {
        int mx = ((m1 - 257) % 32) / 2;
        int my = ((m1 - 257) / 32) / 2;
        int m = mx + my * 16;
        if (sprite_count < (m + 1) * 2) {
          sprite = realloc(sprite, sizeof(uint16_t) * (m + 1) * 2);
          for (int i = sprite_count; i < (m + 1) * 2; i++)
            sprite[i] = 0xfffe;
          sprite_count = (m + 1) * 2;
        }
        printf("marker %2d at (%2d, %2d)\n", m, x, y);
        sprite[m * 2 + 0] = x;
        sprite[m * 2 + 1] = y;
      }
    }
  }
  for (int i = 0; i < sprite_count; i += 2) {
    if (sprite[i] == 0xfffe) {
      warnings++;
      fprintf(stderr, "WARNING: Missing sprite %d location\n", i / 2);
    }
  }
  sprite_count += 2;
  sprite = realloc(sprite, sizeof(uint16_t) * sprite_count);
  sprite[sprite_count - 2] = 0xffff;
  sprite[sprite_count - 1] = 0xffff;
  fwrite(sprite, sizeof(uint16_t), sprite_count, fpsprite);

  free(markers);
  free(objs);
  free(base);
  if (warnings > 0) {
    fprintf(stderr, "\n#\n# WARNINGS: %d\n#\n", warnings);
  }
  return 0;
}

int main(int argc, const char **argv) {
  if (argc < 2) {
    print_usage();
    return 0;
  }

  if (strcmp(argv[1], "fix") == 0) {
    if (argc != 3) {
      print_usage();
      fprintf(stderr, "\nExpecting: fix <input.gba>\n");
      return 1;
    }
    return fix_rom(argv[2]);
  } else if (strcmp(argv[1], "palette256") == 0) {
    if (argc < 3) {
      print_usage();
      fprintf(stderr, "\nExpecting: palette256 <output.bin> [input.png...]\n");
      return 1;
    }
    u16 palette[256] = {0};
    int nextpal = 1;
    for (int i = 3; i < argc; i++) {
      int res = addpalette(argv[i], palette, &nextpal, 256);
      if (res != 0) {
        return res;
      }
    }
    printf("Palette size: %d colors\n", nextpal);
    FILE *fp = fopen(argv[2], "wb");
    if (fp == NULL) {
      fprintf(stderr, "\nFailed to write: %s\n", argv[2]);
      return 1;
    }
    fwrite(palette, sizeof(palette), 1, fp);
    fclose4(fp);
    return 0;
  } else if (strcmp(argv[1], "expand6x6to8x8") == 0) {
    if (argc != 5) {
      print_usage();
      fprintf(stderr, "\nExpecting expand6x6to8x8 <input.png> <palette.bin> <output.bin>\n");
      return 1;
    }
    return expand6x6to8x8(argv[2], argv[3], argv[4]);
  } else if (strcmp(argv[1], "copy8x8") == 0) {
    if (argc != 5) {
      print_usage();
      fprintf(stderr, "\nExpecting copy8x8 <input.png> <palette.bin> <output.bin>\n");
      return 1;
    }
    return copy8x8(argv[2], argv[3], argv[4]);
  } else if (strcmp(argv[1], "world") == 0) {
    if (argc != 6) {
      print_usage();
      fprintf(stderr, "\nExpecting world <input.json> <bg.bin> <logic.bin> <sprite.bin>\n");
      return 1;
    }
    json_value *jv = json_parsefile(argv[2]);
    FILE *bg = fopen(argv[3], "wb");
    FILE *logic = fopen(argv[4], "wb");
    FILE *sprite = fopen(argv[5], "wb");
    int res = process_world(jv, bg, logic, sprite);
    fclose4(sprite);
    fclose4(logic);
    fclose4(bg);
    json_free(jv);
    return res;
  } else if (strcmp(argv[1], "snd") == 0) {
    return snd_main(argc - 2, &argv[2]);
  } else {
    print_usage();
    fprintf(stderr, "\nUnknown command: %s\n", argv[1]);
    return 1;
  }

  return 0;
}
