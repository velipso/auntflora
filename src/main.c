//
// auntflora - Port of Aunt Flora's Mansion to Game Boy Advance
// by Sean Connelly (@velipso), https://sean.fun
// Project Home: https://github.com/velipso/auntflora
// SPDX-License-Identifier: 0BSD
//

#include "main.h"
#include "pushers.h"
#include <stdlib.h>
#include "ani.h"
#include "anidata.h"
#include "util.h"
#include "cellinfo.h"
#include "sfx.h"
#include "undo.h"

u8 g_map0[64 * 64] = {0};
u8 g_map1[64 * 64] = {0};
int g_inputdown = 0;
int g_inputhit = 0;
struct viewport_st g_viewport = {0};
struct world_st g_world = {0};
struct markers_st g_markers[MAX_MARKERS] = {0};
int g_seen_marker[MAX_MARKERS] = {0};
int g_playerdir = 0;
int g_options = 0;
int g_total_steps = 0;
static int g_load_palette = 0;
static int g_song_volume = 6;
static int g_sfx_volume = 16;
static int g_show_counter = 0;
static const u8 zero[64] = {0};
static bool g_noclip = false;

static void SECTION_IWRAM_ARM irq_vblank_title() {
  sys_copy_oam(g_oam);
  int inp = sys_input() ^ 0x3ff;
  g_inputhit = ~g_inputdown & inp;
  g_inputdown = inp;
}

static void SECTION_IWRAM_ARM irq_vblank_game() {
  if (g_load_palette == 1) { // load black
    for (int i = 0; i < 8; i++) {
      sys_copy_bgpal(32 * i, zero, sizeof(zero));
      sys_copy_spritepal(32 * i, zero, sizeof(zero));
    }
    g_load_palette = 0;
  } else if (g_load_palette == 2) { // load colors
    sys_copy_bgpal(0, BINADDR(palette_bin), BINSIZE(palette_bin));
    sys_copy_spritepal(0, BINADDR(palette_bin), BINSIZE(palette_bin));
    g_load_palette = 0;
  }
  sys_copy_map(28, 0, g_map0, 64 * 64);
  sys_copy_map(30, 0, g_map1, 64 * 64);
  sys_copy_oam(g_oam);
  int inp = sys_input() ^ 0x3ff;
  g_inputhit = ~g_inputdown & inp;
  g_inputdown = inp;
}

static void settile0(u32 x, u32 y, u32 t) {
  u32 k = (x * 2) + (y * 64 * 2);
  if (t == 0) {
    g_map0[k + 0] = 0;
    g_map0[k + 1] = 0;
    g_map0[k + 64] = 0;
    g_map0[k + 65] = 0;
  } else {
    u32 tx = t & 0xf;
    u32 ty = t >> 4;
    u32 tk = (tx * 2) + (ty * 32 * 2);
    g_map0[k + 0] = tk;
    g_map0[k + 1] = tk + 1;
    g_map0[k + 64] = tk + 32;
    g_map0[k + 65] = tk + 33;
  }
}

static inline bool opt_hide_borders() {
  return (g_options & OPT_HIDE_BORDERS) != 0;
}

static inline bool opt_snap_scroll() {
  return (g_options & OPT_SNAP_SCROLL) != 0;
}

static inline bool opt_standard_def() {
  return (g_options & OPT_TILESET_MASK) >= 2;
}

static void snap_player();
static void render_total_steps();
static void set_options(int opt, bool load_colors) {
  g_load_palette = 1; // load black
  nextframe();

  g_options = opt;
  gfx_showbg2(true);
  gfx_showbg3(true);
  gfx_showobj(true);
  gfx_setmode(
    opt_standard_def()
      ? GFX_MODE_2S5X5
      : GFX_MODE_2S6X6
  );
  sys_set_bg_config(
    2, // background #
    0, // priority
    0, // tile start
    0, // mosaic
    1, // 256 colors
    28, // map start
    0, // wrap
    SYS_BGS_SIZE_512X512
  );
  sys_set_bg_config(
    3, // background #
    0, // priority
    0, // tile start
    0, // mosaic
    1, // 256 colors
    30, // map start
    0, // wrap
    SYS_BGS_SIZE_512X512
  );
  if (opt_standard_def()) {
    sys_copy_tiles(0, 0, BINADDR(tiles_sd_bin), BINSIZE(tiles_sd_bin));
    sys_copy_tiles(4, 0, BINADDR(sprites_sd_bin), BINSIZE(sprites_sd_bin));
    sys_copy_tiles(1, 0, BINADDR(font_sd_bin), BINSIZE(font_sd_bin));
    sys_set_bgs2_scroll(0x019a * 5, 0x019a * 0);
    sys_set_bgs3_scroll(0x019a * 5, 0x019a * 0);
  } else {
    sys_copy_tiles(0, 0, BINADDR(tiles_hd_bin), BINSIZE(tiles_hd_bin));
    sys_copy_tiles(4, 0, BINADDR(sprites_hd_bin), BINSIZE(sprites_hd_bin));
    sys_copy_tiles(1, 0, BINADDR(font_hd_bin), BINSIZE(font_hd_bin));
    sys_set_bgs2_scroll(0x0156 * 30 - 12, 0x0156 * 16);
    sys_set_bgs3_scroll(0x0156 * 30 - 12, 0x0156 * 16);
  }
  // ensure that tile 0 is fully transparent
  sys_copy_tiles(0, 0, zero, sizeof(zero));

  // redraw everything
  render_total_steps();
  snap_player();
  copy_world_offset();

  if (load_colors) {
    g_load_palette = 2; // load colors
    nextframe();
  }
}

void set_player_ani_dir(int dir) {
  g_playerdir = dir;
  switch (dir) {
    case 0: g_sprites[1].pc = ani_player_u; break;
    case 1: g_sprites[1].pc = ani_player_r; break;
    case 2: g_sprites[1].pc = ani_player_d; break;
    case 3: g_sprites[1].pc = ani_player_l; break;
  }
}

void move_player_ani_dir(int dir) {
  if (opt_standard_def()) {
    set_player_ani_dir(dir);
    return;
  }
  static int which = 0;
  g_playerdir = dir;
  switch (dir) {
    case 0:
      g_sprites[1].pc = (which & 1) ? ani_player_move_u1 : ani_player_move_u2;
      which ^= 1;
      break;
    case 1:
      g_sprites[1].pc = (which & 2) ? ani_player_move_r1 : ani_player_move_r2;
      which ^= 2;
      break;
    case 2:
      g_sprites[1].pc = (which & 4) ? ani_player_move_d1 : ani_player_move_d2;
      which ^= 4;
      break;
    case 3:
      g_sprites[1].pc = (which & 8) ? ani_player_move_l1 : ani_player_move_l2;
      which ^= 8;
      break;
  }
}

static struct viewport_st find_player_level() {
  int wx = 0;
  int wy = 0;
  int x = g_markers[0].x - 21;
  int y = g_markers[0].y - 14;
  while (x >= wx) wx += 17;
  while (y >= wy) wy += 12;
  return (struct viewport_st){
    .wx = wx,
    .wy = wy
  };
}

static void snap_player() {
  if (g_show_counter) {
    g_sprites[0].origin.x = 240 - 64;
    g_sprites[0].origin.y = 160 - 11;
  } else {
    g_sprites[0].origin.x = 240;
    g_sprites[0].origin.y = 160;
  }
  if (opt_standard_def()) {
    g_sprites[1].origin.x = (g_markers[0].x - g_viewport.wx) * 10 - 8;
    g_sprites[1].origin.y = (g_markers[0].y - g_viewport.wy) * 10 - 3;
    g_sprites[2].origin.x = (g_markers[1].x - g_viewport.wx) * 10 - 8;
    g_sprites[2].origin.y = (g_markers[1].y - g_viewport.wy) * 10 - 3;
    g_sprites[3].origin.x = (g_markers[2].x - g_viewport.wx) * 10 - 8;
    g_sprites[3].origin.y = (g_markers[2].y - g_viewport.wy) * 10 - 3;
  } else {
    g_sprites[1].origin.x = (g_markers[0].x - g_viewport.wx) * 12 - 32;
    g_sprites[1].origin.y = (g_markers[0].y - g_viewport.wy) * 12 - 18;
    g_sprites[2].origin.x = (g_markers[1].x - g_viewport.wx) * 12 - 32;
    g_sprites[2].origin.y = (g_markers[1].y - g_viewport.wy) * 12 - 18;
    g_sprites[3].origin.x = (g_markers[2].x - g_viewport.wx) * 12 - 32;
    g_sprites[3].origin.y = (g_markers[2].y - g_viewport.wy) * 12 - 18;
  }
}

void copy_world_offset() {
  for (int y = 0; y < 16; y++) {
    int sy = g_viewport.wy + y;
    for (int x = 0; x < 25; x++) {
      int sx = g_viewport.wx + x;
      int ww = 0;
      if (sx >= 0 && sx < g_world.width && sy >= 0 && sy < g_world.height) {
        ww = world_at(sx, sy);
      }
      if (opt_hide_borders() && (x < 4 || x >= 21 || y < 2 || y >= 14))
        ww = 0;
      settile0(x, y, ww & 0xff);
    }
  }

  const u8 *bg = opt_standard_def() ? BINADDR(worldbg_sd_bin) : BINADDR(worldbg_hd_bin);
  for (int y = 0; y < 32; y++) {
    memcpy8(
      &g_map1[y * 64],
      &bg[g_viewport.wx * 2 + (g_viewport.wy * 2 + y) * g_world.width * 2],
      49
    );
  }
  if (opt_hide_borders()) {
    // clear border
    for (int y = 0; y < 4; y++) {
      memset8(&g_map1[y * 64], 0, 49);
    }
    for (int y = 4; y < 28; y++) {
      for (int i = 0; i < 8; i++) {
        g_map1[y * 64 + i] = 0;
        g_map1[y * 64 + 42 + i] = 0;
      }
    }
    for (int y = 28; y < 32; y++) {
      memset8(&g_map1[y * 64], 0, 49);
    }
  }
}

static void load_world() {
  free(g_world.data);
  const u16 *world = BINADDR(worldlogic_bin);
  g_world.width = world[0];
  g_world.height = world[1];
  int size = g_world.width * g_world.height * 2;
  g_world.data = malloc(size);
  memcpy32(g_world.data, &world[2], size);

  // load markers
  const u16 *markers = BINADDR(markers_bin);
  int i = 0;
  while (1) {
    int x = markers[i * 2];
    int y = markers[i * 2 + 1];
    if (x == 0xffff)
      break;
    if (i < MAX_MARKERS) {
      g_markers[i].x = x;
      g_markers[i].y = y;
    }
    i++;
  }
  for (; i < MAX_MARKERS; i++) {
    g_markers[i].x = -1;
    g_markers[i].y = -1;
  }

  // initialize undo
  undo_init();
}

void nextframe() {
  for (int i = 0; i < 128; i++)
    ani_step(i);
  sys_nextframe();
}

void move_screen_to_player() {
  // move screen based on player location
  struct viewport_st vp2 = find_player_level();
  int vdx = vp2.wx > g_viewport.wx ? 1 : vp2.wx < g_viewport.wx ? -1 : 0;
  int vdy = vp2.wy > g_viewport.wy ? 1 : vp2.wy < g_viewport.wy ? -1 : 0;
  if (vdx != 0 || vdy != 0) {
    if (opt_snap_scroll()) {
      // snap to new location
      g_viewport = vp2;
    } else {
      // scroll to new location
      while (vp2.wx != g_viewport.wx || vp2.wy != g_viewport.wy) {
        g_viewport.wx += vdx;
        g_viewport.wy += vdy;
        copy_world_offset();
        snap_player();
        nextframe();
      }
    }
  }
  snap_player();
}

static void roll_credits();
static void title_screen() {
  if (sys_mGBA()) {
    sys_set_vblank(irq_vblank_title);
    nextframe();
    gfx_setmode(GFX_MODE_2I);
    gfx_showbg2(true);
    gfx_showobj(true);
    sys_copy_tiles(0, 0, BINADDR(keyboard_bin), BINSIZE(keyboard_bin));
    sys_copy_bgpal(0, BINADDR(keyboard_palette_bin), BINSIZE(keyboard_palette_bin));
    gfx_showscreen(true);
    while (g_inputdown) nextframe();
    while (!g_inputdown) nextframe();
  }

  snd_set_master_volume(16);
  snd_set_song_volume(g_song_volume);
  snd_set_sfx_volume(g_sfx_volume);
  snd_load_song(BINADDR(song1_gvsong), 1);
restart_title_screen:
  sys_set_vblank(irq_vblank_title);
  nextframe();

  gfx_setmode(GFX_MODE_2I);
  gfx_showbg2(true);
  gfx_showobj(true);
  sys_copy_tiles(0, 0, BINADDR(title_bin), BINSIZE(title_bin));
  sys_copy_tiles(4, 0, BINADDR(sprites_hd_bin), BINSIZE(sprites_hd_bin));
  sys_copy_bgpal(0, BINADDR(title_palette_bin), BINSIZE(title_palette_bin));
  sys_copy_spritepal(0, BINADDR(palette_bin), BINSIZE(palette_bin));
  gfx_showscreen(true);

  // wait for no keys
  while (g_inputdown) nextframe();

  // wait two second
  for (int i = 0; i < 120; i++) {
    nextframe();
    if (
      (g_inputhit & SYS_INPUT_A) ||
      (g_inputhit & SYS_INPUT_ST)
    )
      break;
  }

  g_sprites[0].pc = ani_start1;
  g_sprites[0].origin.x = 56;
  g_sprites[0].origin.y = 128;
  g_sprites[1].pc = ani_start2;
  g_sprites[1].origin.x = 120;
  g_sprites[1].origin.y = 128;

  int menu = 0;

  // wait for keypress
  while (1) {
    nextframe();
    if (
      (g_inputhit & SYS_INPUT_U) ||
      (g_inputhit & SYS_INPUT_R) ||
      (g_inputhit & SYS_INPUT_D) ||
      (g_inputhit & SYS_INPUT_L) ||
      (g_inputhit & SYS_INPUT_SE)
    ) {
      menu = 1 - menu;
      if (menu) {
        g_sprites[0].pc = ani_credits1;
        g_sprites[1].pc = ani_credits2;
      } else {
        g_sprites[0].pc = ani_start1;
        g_sprites[1].pc = ani_start2;
      }
      sfx_click();
    } else if (
      (g_inputhit & SYS_INPUT_A) ||
      (g_inputhit & SYS_INPUT_ST)
    ) {
      sfx_click();

      if (menu == 0) {
        // fade out music
        for (int i = g_song_volume; i >= 0; i--) {
          snd_set_song_volume(i);
          nextframe();
        }
      }

      sys_copy_tiles(1, 0, BINADDR(font_hd_bin), BINSIZE(font_hd_bin));
      g_sprites[0].pc = NULL;
      g_sprites[1].pc = NULL;

      if (menu == 0)
        break;
      else {
        roll_credits();
        goto restart_title_screen;
      }
    }
  }
}

static void card_screen_setup() {
  sys_set_vblank(irq_vblank_game);
  g_load_palette = 1; // load black
  nextframe();
  gfx_setmode(GFX_MODE_2S6X6);
  sys_set_bg_config(
    2, // background #
    0, // priority
    1, // tile start
    0, // mosaic
    1, // 256 colors
    28, // map start
    0, // wrap
    SYS_BGS_SIZE_512X512
  );
  gfx_showbg2(true);
  gfx_showbg3(false);
  gfx_showobj(true);
  sys_set_bgs2_scroll(0x0156 * -6, 0x0156 * 4);
}

static void card_screen_print(const char *text) {
  // print message...
  const char *chars =
    " !\"'<>,-.0123456"
    "789?ABCDFGHIJKLM"
    "NOPRSTWabcdefghi"
    "jklmnoprstuvwxyz";
  int x = 0;
  int y = 0;
  for (int i = 0; text[i]; i++) {
    if (text[i] == '\n') {
      x = 0;
      y++;
    } else {
      // print text[i] at (x, y)
      for (int j = 0; chars[j]; j++) {
        if (text[i] == chars[j]) {
          // print character index j at (x, y)
          int jx = j % 16;
          int jy = j / 16;
          int jk = jx * 2 + jy * 2 * 32;
          int k = x * 2 + y * 2 * 64;
          g_map0[k +  0] = jk +  0;
          g_map0[k +  1] = jk +  1;
          g_map0[k + 64] = jk + 32;
          g_map0[k + 65] = jk + 33;
          x++;
          break;
        }
      }
    }
  }

  g_load_palette = 2; // load colors
}

static void card_screen(const char *message) {
  card_screen_setup();
  card_screen_print(message);
  // wait for keypress
  while (g_inputdown) nextframe();
  // prevent spamming
  for (int i = 0; i < 20 && !g_noclip; i++) nextframe();
  while (!(
    (g_inputdown & SYS_INPUT_A) ||
    (g_inputdown & SYS_INPUT_ST)
  )) nextframe();
  while (g_inputdown) nextframe();
  sfx_click();
}

static void card_screen_from_marker(int marker);
static void restore_game_state();
static void move_player(int x, int y, int dir) {
  // check for messages or end
  int hit_marker = -1;
  for (int m = 3; m < MAX_MARKERS; m++) {
    if (g_markers[m].x == x && g_markers[m].y == y) {
      hit_marker = m - 3;
      break;
    }
    if (m == 12 && g_markers[m].x == x && g_markers[m].y == y + 1) {
      // annoyingly, the Parlor marker is two blocks high :-( so special hack here
      // to handle it
      hit_marker = 9;
      break;
    }
    if (m == 4 && g_markers[m].x == x && (g_markers[m].y == y + 1 || g_markers[m].y == y + 2)) {
      // annoyingly, the Kitchen marker is THREE blocks high :-( so special hack here
      // to handle it
      hit_marker = 1;
      break;
    }
  }
  if (hit_marker >= 0 && g_seen_marker[hit_marker])
    hit_marker = -1;
  write_player(x, y, dir, hit_marker);
  render_total_steps();
  if (hit_marker >= 0) {
    card_screen_from_marker(hit_marker);
    restore_game_state();
  }
}

static void restore_game_state() {
  sys_set_vblank(irq_vblank_game);
  g_viewport = find_player_level();
  set_options(g_options, true);
  sys_nextframe();
}

static void render_total_steps() {
  static int white_pix = -1;
  static int black_pix = -1;

  if (white_pix < 0) {
    const u16 *pal = BINADDR(palette_bin);
    int size = BINSIZE(palette_bin) / 2;
    for (int i = 1; i < size; i++) {
      if (pal[i] == 0)
        black_pix = i;
      else if (pal[i] == 0x7fff)
        white_pix = i;
    }
  }

  const u8 pics[] = {
    0,1,1,1,2,0,
    1,1,2,1,1,2,
    1,1,2,1,1,2,
    1,1,2,1,1,2,
    1,1,2,1,1,2,
    0,1,1,1,2,2,
    0,0,2,2,2,0,
    0,0,0,0,0,0,

    0,0,1,1,2,0,
    0,1,1,1,2,0,
    0,0,1,1,2,0,
    0,0,1,1,2,0,
    0,0,1,1,2,0,
    0,1,1,1,1,2,
    0,0,2,2,2,2,
    0,0,0,0,0,0,

    0,1,1,1,2,0,
    1,1,2,1,1,2,
    0,2,2,1,1,2,
    0,0,1,1,2,0,
    0,1,1,2,2,0,
    1,1,1,1,1,2,
    0,2,2,2,2,2,
    0,0,0,0,0,0,

    1,1,1,1,2,0,
    0,2,2,1,1,2,
    0,0,1,1,2,2,
    0,0,0,1,1,2,
    0,0,0,1,1,2,
    1,1,1,1,2,2,
    0,2,2,2,2,0,
    0,0,0,0,0,0,

    1,1,2,1,1,2,
    1,1,2,1,1,2,
    1,1,1,1,1,2,
    0,2,2,1,1,2,
    0,0,0,1,1,2,
    0,0,0,1,1,2,
    0,0,0,0,2,2,
    0,0,0,0,0,0,

    1,1,1,1,1,2,
    1,1,2,2,2,2,
    1,1,1,1,2,0,
    0,2,2,1,1,2,
    0,0,0,1,1,2,
    1,1,1,1,2,2,
    0,2,2,2,2,0,
    0,0,0,0,0,0,

    0,1,1,1,1,2,
    1,1,2,2,2,2,
    1,1,1,1,2,0,
    1,1,2,1,1,2,
    1,1,2,1,1,2,
    0,1,1,1,2,2,
    0,0,2,2,2,0,
    0,0,0,0,0,0,

    1,1,1,1,1,2,
    0,2,2,1,1,2,
    0,0,0,1,1,2,
    0,0,1,1,2,2,
    0,0,1,1,2,0,
    0,0,1,1,2,0,
    0,0,0,2,2,0,
    0,0,0,0,0,0,

    0,1,1,1,2,0,
    1,1,2,1,1,2,
    0,1,1,1,2,2,
    1,1,2,1,1,2,
    1,1,2,1,1,2,
    0,1,1,1,2,2,
    0,0,2,2,2,0,
    0,0,0,0,0,0,

    0,1,1,1,1,2,
    1,1,2,1,1,2,
    0,1,1,1,1,2,
    0,0,2,1,1,2,
    0,0,0,1,1,2,
    0,0,0,1,1,2,
    0,0,0,0,2,2,
    0,0,0,0,0,0,
  };
  int digits[] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
  int i = 9;
  int n = g_total_steps;
  if (n == 0)
    digits[i] = 0;
  else {
    do {
      int d = n / 10;
      int r = n - d * 10;
      n = d;
      if (i >= 0)
        digits[i] = r;
      i--;
    } while (n > 0);
  }
  for (int dig = 0; dig < 10; dig++) {
    const u8 *pic = digits[dig] < 0 ? NULL : &pics[digits[dig] * 48];
    for (int py = 0; py < 8; py++) {
      for (int px = 0; px < 6; px += 2) {
        int c = 0;
        if (pic) {
          switch (pic[px + py * 6]) {
            case 1: c = white_pix; break;
            case 2: c = black_pix; break;
          }
          switch (pic[px + py * 6 + 1]) {
            case 1: c |= white_pix << 8; break;
            case 2: c |= black_pix << 8; break;
          }
        }
        gfx_pset2_obj(dig * 6 + px, 160 + py, c);
      }
    }
  }
}

static void card_options();
static bool fire_undo_next_frame = false;
static void play_game() {
  load_world();

  g_sprites[0].pc = ani_counter;
  g_sprites[1].pc = ani_player_u;
  g_sprites[2].pc = ani_aunt;
  g_sprites[3].pc = ani_cat;

  restore_game_state();
  gfx_showbg2(true);
  gfx_showbg3(true);
  gfx_showobj(true);
  gfx_showscreen(true);

  snd_set_master_volume(16);
  snd_set_song_volume(g_song_volume);
  snd_set_sfx_volume(g_sfx_volume);
  snd_load_song(BINADDR(song1_gvsong), 0);

  int dir = -1;
  int last_dir = -1;
  int repeat_timer = -1;
  bool last_undo = false;
  while (1) {
    nextframe();

    if (
      (g_inputhit & SYS_INPUT_ZL) ||
      (g_inputhit & SYS_INPUT_ZR)
    ) {
      g_show_counter = 1 - g_show_counter;
      render_total_steps();
      snap_player();
    }

    if (
      fire_undo_next_frame ||
      (g_inputdown & SYS_INPUT_B)
    ) {
      bool hit = false;
      if (!fire_undo_next_frame && last_undo && repeat_timer > 0) {
        repeat_timer--;
        if (repeat_timer == 0)
          hit = true;
      } else
        hit = true;
      last_undo = true;
      if (hit) {
        repeat_timer = 11;
        fire_undo_next_frame = false;
        if (undo_fire()) {
          render_total_steps();
          g_viewport = find_player_level();
          snap_player();
        } else
          sfx_bump(); // failed to undo
      }
    } else
      last_undo = false;

    if (g_inputhit & SYS_INPUT_A) {
      // can we checkpoint here?
      if (is_checkpoint(world_at(g_markers[0].x, g_markers[0].y))) {
        sfx_checkpoint();
        checkpoint_save();
      }
    } else if (g_inputhit & SYS_INPUT_SE) {
      if (checkpoint_restore()) {
        sfx_forward();
        render_total_steps();
        g_viewport = find_player_level();
        snap_player();
      } else
        sfx_bump(); // failed to restore checkpoint
    } else if (g_inputhit & SYS_INPUT_ST) {
      card_options();
      restore_game_state();
    } else {
      g_dirty = 0;
      dir = -1;
      if (g_inputdown & SYS_INPUT_U) dir = 0;
      if (g_inputdown & SYS_INPUT_R) dir = 1;
      if (g_inputdown & SYS_INPUT_D) dir = 2;
      if (g_inputdown & SYS_INPUT_L) dir = 3;

      // fire repeat inputs
      bool hit = false;
      if (dir >= 0 && dir < 4) {
        if (last_dir != dir)
          hit = true;
        else if (repeat_timer > 0) {
          repeat_timer--;
          if (repeat_timer == 0)
            hit = true;
        }
      }
      last_dir = dir;

      if (hit) {
        repeat_timer = 11;
        int nx = g_markers[0].x;
        int ny = g_markers[0].y;
        advance_pt(&nx, &ny, dir, 1);
        u32 cell = world_at(nx, ny);
        if (g_noclip || !is_solid_static(cell)) {
          pushers_reset();
          if (g_noclip || is_empty_for_player(cell)) {
            if (!g_noclip) {
              int bg = worldbg_at(nx, ny) & 0x1f;
              if (bg >= 6 && bg <= 9)
                sfx_splash();
              else
                sfx_walk();
            }
            pushers_find_around_player(dir);
            move_player(nx, ny, dir);
          } else if (is_pushable_by_player(cell, dir)) {
            int px = nx;
            int py = ny;
            while (1) {
              u16 nc = world_at(px, py);
              if (is_empty_for_block(nc)) {
                int hx, hy;
                while (px != nx || py != ny) {
                  hx = px;
                  hy = py;
                  advance_pt(&px, &py, dir, -1);
                  write_logic(hx, hy, world_at(px, py));
                }
                write_logic(nx, ny, 0);
                sfx_push();
                pushers_find_around_player(dir);
                move_player(nx, ny, dir);
                break;
              } else if (is_pushable_by_block(nc, dir)) {
                advance_pt(&px, &py, dir, 1);
                // find pushers in perpendicular directions
                pushers_find(px, py, (dir + 1) & 3);
                pushers_find(px, py, (dir + 3) & 3);
                continue;
              } else {
                break;
              }
            }
          } else if (is_forward(cell, dir)) {
            advance_pt(&nx, &ny, dir, 1);
            if (is_empty_for_player(world_at(nx, ny))) {
              sfx_forward();
              pushers_find_around_player(dir);
              move_player(nx, ny, dir);
            }
          }

          pushers_apply();
        }

        if (g_dirty) {
          undo_finish();
          move_screen_to_player();
        } else {
          sfx_bump();
          set_player_ani_dir(dir);
        }
      }
    }

    copy_world_offset();
  }
}

static void roll_credits() {
  card_screen(
    "                    \n"
    "                    \n"
    "                    \n"
    "                    \n"
    "  created by        \n"
    "     anna anthropy  \n"
    "                    \n"
    "                    \n"
    "                    \n"
    "                    \n"
    "                    \n"
    "   A to continue    \n"
    "                    \n"
    "                    \n"
  );
  card_screen(
    "                    \n"
    "                    \n"
    "                    \n"
    "                    \n"
    "  with help from    \n"
    "     Alan Hazelden, \n"
    "     Jonah Ostroff, \n"
    "       and          \n"
    "     Jamie Perconti \n"
    "                    \n"
    "                    \n"
    "   A to continue    \n"
    "                    \n"
    "                    \n"
  );
  card_screen(
    "                    \n"
    "                    \n"
    "                    \n"
    "  playtested by     \n"
    "     Jen Ada,       \n"
    "     John H.,       \n"
    "     Chris Harris,  \n"
    "       and          \n"
    "     Kelsey Higham  \n"
    "                    \n"
    "                    \n"
    "   A to continue    \n"
    "                    \n"
    "                    \n"
  );
  card_screen(
    "                    \n"
    "                    \n"
    "  Game Boy Advance  \n"
    "  port by           \n"
    "     Casey Dean     \n"
    "       and          \n"
    "     Sean Connelly  \n"
    "       from         \n"
    "     Pocket Pulp    \n"
    "     www.pulp.biz   \n"
    "                    \n"
    "   A to continue    \n"
    "                    \n"
    "                    \n"
  );
}

static void card_screen_from_marker(int marker) {
  char message[300];
  const char *template =
    "                    \n"
    "                    \n"
    "                    \n"
    "                    \n"
    "                    \n"
    "                    \n"
    "                    \n"
    "                    \n"
    "                    \n"
    "                    \n"
    "                    \n"
    "   A to continue    \n"
    "                    \n"
    "                    \n";
  int i;
  for (i = 0; template[i]; i++)
    message[i] = template[i];
  message[i] = 0;
  #define M(r, msg)                   \
    do {                              \
      const char *str = msg;          \
      for (i = 0; str[i]; i++)        \
        message[r * 21 + i] = str[i]; \
    } while (0)
  switch (marker) {
    case  0: M(5, "   The Main Hall    \n"); break;
    case  1: M(5, "    The Kitchen     \n"); break;
    case  2: M(5, " The Back Stairway  \n"); break;
    case  3: M(5, "   The Back Porch   \n"); break;
    case  4: M(5, "   The Side Gate    \n"); break;
    case  5: M(5, "  The Storage Room  \n"); break;
    case  6: M(5, "   The Colonnade    \n");
             M(6, "      Ballroom      \n"); break;
    case  7: M(5, "     The Annex      \n"); break;
    case  8: M(5, "     The Cellar     \n"); break;
    case  9: M(5, "     The Parlor     \n"); break;
    case 10: M(5, "    The Terrace     \n"); break;
    case 11: M(5, "     The Study      \n"); break;
    case 12: M(5, "    The Library     \n"); break;
    case 13: M(5, "  The Dining Room   \n"); break;
    case 14: M(5, "     The Secret     \n");
             M(6, "      Passage       \n"); break;
    case 15: M(5, "  The Wine Cellar   \n"); break;
    case 16: M(5, "  The Conservatory  \n"); break;
    case 17: M(5, "     The Attic      \n"); break;
    case 18: // beat game!
      // hide sprites
      for (int i = 0; i < 4; i++) {
        g_sprites[i].origin.x = 240;
        g_sprites[i].origin.y = 160;
        ani_step(i);
      }
      for (int i = 16; i >= 0; i--)
        snd_set_master_volume(i);
      snd_load_song(BINADDR(song1_gvsong), 2); // silence
      snd_set_master_volume(16);
      sfx_end();
      card_screen(
        "                    \n"
        "                    \n"
        "                    \n"
        "                    \n"
        "\"Oh hello, Sweet-   \n"
        " heart. Won't you   \n"
        " join your Auntie   \n"
        " for a cup of tea?\" \n"
        "                    \n"
        "                    \n"
        "                    \n"
        "   A to continue    \n"
        "                    \n"
        "                    \n"
      );
      card_screen(
        "                    \n"
        "                    \n"
        "                    \n"
        "\"Auntie, I don't    \n"
        " know how you can   \n"
        " live in this huge  \n"
        " place all by your- \n"
        " self.\"             \n"
        "                    \n"
        "                    \n"
        "                    \n"
        "   A to continue    \n"
        "                    \n"
        "                    \n"
      );
      card_screen(
        "                    \n"
        "                    \n"
        "                    \n"
        "\"I'm not alone, I   \n"
        " have Catsup here.  \n"
        " Now drink your tea \n"
        " before it gets     \n"
        " cold.\"             \n"
        "                    \n"
        "                    \n"
        "                    \n"
        "   A to continue    \n"
        "                    \n"
        "                    \n"
      );
      card_screen(
        "                    \n"
        "                    \n"
        "                    \n"
        "                    \n"
        "  \"Okay, Auntie.    \n"
        "   I missed you.\"   \n"
        "                    \n"
        "                    \n"
        "                    \n"
        "                    \n"
        "                    \n"
        "   A to continue    \n"
        "                    \n"
        "                    \n"
      );
      snd_load_song(BINADDR(song1_gvsong), 0);
      roll_credits();
      M( 3, "   Thank you for    ");
      M( 4, "      playing!      ");
      M( 6, "    Total Steps     ");
      { // output total steps
        int i = 8 * 21 + 11;
        int n = g_total_steps;
        while (n > 0) {
          int d = n / 10;
          int r = n - d * 10;
          n = d;
          message[i] = '0' + r;
          i--;
        }
      }
      fire_undo_next_frame = true; // undo to immediately before beating game
      break;
    default:
      return;
  }
  #undef M
  // hide sprites
  for (int i = 0; i < 4; i++) {
    g_sprites[i].origin.x = 240;
    g_sprites[i].origin.y = 160;
    ani_step(i);
  }

  card_screen(message);
}

// map 0-10 -> 0-16
static const u16 volume_map_fwd[]  = {0, 1, 2, 3, 5, 6, 8, 10, 12, 14, 16};
// map 0-16 -> 0-10
static const u16 volume_map_back[] = {0, 1, 2, 3, 3, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10};

static void card_options_volume(char *msg, int volume) {
  volume = volume_map_back[volume];
  if (volume == 0) {
    msg[0] = 'O';
    msg[1] = 'f';
    msg[2] = 'f';
  } else if (volume == 10) {
    msg[0] = 'M';
    msg[1] = 'a';
    msg[2] = 'x';
  } else { // 1-9
    msg[0] = ' ';
    msg[1] = '0' + volume;
    msg[2] = ' ';
  }
}

static void card_options() {
  card_screen_setup();

  char message[300];
  const char *template =
    "                    \n"
    " Graphics           \n"
    "                    \n"
    " Border             \n"
    "                    \n"
    " Scrolling          \n"
    "                    \n"
    " Music              \n"
    "                    \n"
    " Sound Fx           \n"
    "                    \n"
    "                    \n"
    "  Start to return   \n"
    "                    \n";

  int menu = 0;
  int refresh = 1;
  while (1) {
    if (refresh) {
      if (refresh == 1)
        sfx_click();
      refresh = 0;
      // hide sprites
      for (int i = 0; i < 4; i++) {
        g_sprites[i].origin.x = 240;
        g_sprites[i].origin.y = 160;
        ani_step(i);
      }
      int i;
      for (i = 0; template[i]; i++)
        message[i] = template[i];
      message[i] = 0;
      message[42 * menu + 33] = '<';
      message[42 * menu + 39] = '>';
      switch (g_options & OPT_TILESET_MASK) {
        case 0:
        case 1:
          message[35] = 'M';
          message[36] = 'a';
          message[37] = 'x';
          break;
        case 2:
        case 3:
          message[35] = 'S';
          message[36] = 't';
          message[37] = 'd';
          break;
      }
      if (opt_hide_borders()) {
        message[77] = 'O';
        message[78] = 'n';
        message[79] = ' ';
      } else {
        message[77] = 'O';
        message[78] = 'f';
        message[79] = 'f';
      }
      if (opt_snap_scroll()) {
        message[119] = 'O';
        message[120] = 'f';
        message[121] = 'f';
      } else {
        message[119] = 'O';
        message[120] = 'n';
        message[121] = ' ';
      }
      card_options_volume(&message[161], g_song_volume);
      card_options_volume(&message[203], g_sfx_volume);
      card_screen_print(message);
    }
    nextframe();
    if (g_inputhit & SYS_INPUT_U) {
      if (menu > 0) {
        menu--;
        refresh = 1;
      }
    } else if (g_inputhit & SYS_INPUT_D) {
      if (menu < 4) {
        menu++;
        refresh = 1;
      }
    } else if (g_inputhit & SYS_INPUT_ST) {
      sfx_click();
      return;
    } else if (menu == 0) { // Graphics
      int d = 0;
      if (
        (g_inputhit & SYS_INPUT_L) ||
        (g_inputhit & SYS_INPUT_R)
      ) {
        d = 2;
      }
      if (d > 0) {
        int next =
          (g_options & ~OPT_TILESET_MASK) |
          (((g_options & OPT_TILESET_MASK) + d) & OPT_TILESET_MASK);
        set_options(next, false);
        card_screen_setup();
        refresh = 1;
      }
    } else if (menu == 1) { // Border
      if (
        (g_inputhit & SYS_INPUT_L) ||
        (g_inputhit & SYS_INPUT_R)
      ) {
        set_options(g_options ^ OPT_HIDE_BORDERS, false);
        card_screen_setup();
        refresh = 1;
      }
    } else if (menu == 2) { // Scrolling
      if (
        (g_inputhit & SYS_INPUT_L) ||
        (g_inputhit & SYS_INPUT_R)
      ) {
        set_options(g_options ^ OPT_SNAP_SCROLL, false);
        card_screen_setup();
        refresh = 1;
      }
    } else if (menu == 3) { // Music
      if (g_inputhit & SYS_INPUT_L) {
        if (g_song_volume > 0) {
          g_song_volume = volume_map_fwd[volume_map_back[g_song_volume] - 1];
          snd_set_song_volume(g_song_volume);
          refresh = 1;
        } else
          sfx_bump();
      }
      if (g_inputhit & SYS_INPUT_R) {
        if (g_song_volume < 16) {
          g_song_volume = volume_map_fwd[volume_map_back[g_song_volume] + 1];
          snd_set_song_volume(g_song_volume);
          refresh = 1;
        } else
          sfx_bump();
      }
    } else if (menu == 4) { // Sound Fx
      if (g_inputhit & SYS_INPUT_L) {
        if (g_sfx_volume > 0) {
          g_sfx_volume = volume_map_fwd[volume_map_back[g_sfx_volume] - 1];
          snd_set_sfx_volume(g_sfx_volume);
          sfx_push();
          refresh = 2; // don't play sfx_click
        } else
          sfx_bump();
      }
      if (g_inputhit & SYS_INPUT_R) {
        if (g_sfx_volume < 16) {
          g_sfx_volume = volume_map_fwd[volume_map_back[g_sfx_volume] + 1];
          snd_set_sfx_volume(g_sfx_volume);
          sfx_push();
          refresh = 2; // don't play sfx_click
        } else
          sfx_bump();
      }
    }
  }
}

void gvmain() {
  sys_init();
  title_screen();
  card_screen(
    "                    \n"
    "                    \n"
    "                    \n"
    "    Aunt Flora's    \n"
    "      Mansion       \n"
    "                    \n"
    "  by anna anthropy  \n"
    "                    \n"
    "                    \n"
    "                    \n"
    " D-pad to move      \n"
    " A to action        \n"
    " B to undo          \n"
    "                    \n"
  );
  card_screen(
    "                    \n"
    "                    \n"
    "                    \n"
    " A letter from      \n"
    " Great-Aunt Flora!  \n"
    "                    \n"
    " She wants me to    \n"
    " join her for tea.  \n"
    "                    \n"
    "                    \n"
    "                    \n"
    "   A to continue    \n"
    "                    \n"
    "                    \n"
  );
  card_screen(
    "                    \n"
    "                    \n"
    "  Auntie Flora's    \n"
    "  mansion is full   \n"
    "  of so much junk,  \n"
    "  though!           \n"
    "                    \n"
    "  How does she      \n"
    "  manage it all at  \n"
    "  her age?          \n"
    "                    \n"
    "   A to continue    \n"
    "                    \n"
    "                    \n"
  );
  card_screen(
    "                    \n"
    "                    \n"
    "                    \n"
    "                    \n"
    "                    \n"
    "    Aunt Flora's    \n"
    "      Mansion       \n"
    "                    \n"
    "                    \n"
    "                    \n"
    "                    \n"
    "   A to continue    \n"
    "                    \n"
    "                    \n"
  );
  card_screen(
    "                    \n"
    "Step into a Heart   \n"
    "and press A to save \n"
    "your game!          \n"
    "                    \n"
    "Press Select to     \n"
    "return to your last \n"
    "save!               \n"
    "                    \n"
    "It's possible, but  \n"
    "hopefully not easy, \n"
    "to get stuck - so   \n"
    "be careful saving!  \n"
    "                    \n"
  );
  play_game();
}
