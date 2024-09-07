Aunt Flora's Mansion for Game Boy Advance
=========================================

Port of [_Aunt Flora's Mansion_](https://w.itch.io/aunt-floras-mansion) to Game Boy Advance.

_Aunt Flora's Mansion_ is a cute [PuzzleScript](https://www.puzzlescript.net/) game made by
[anna anthropy](https://w.itch.io/).  We thought it would be fun to port it to the GBA (with
permission), so more people are exposed to the game, and as a hackable project for others.

Features:

1. Puzzle, graphics, and sound effects from original game
2. Undo and checkpoint saving
3. New HD graphics and expanded viewport
4. Music track (Hungarian Dance no. 5)
5. Permissible license to encourage hacking

My best score for total steps is 1213.

[Download the GBA ROM here](https://github.com/velipso/auntflora/releases/download/v0.8/auntflora.gba).

You will need an emulator to play the game, we suggest [mGBA](https://mgba.io).

![Screenshot](screenshot.png)

Build Instructions
------------------

This project used [gba-bootstrap](https://github.com/AntonioND/gba-bootstrap) as a starting point.

You will need to install the ARM GCC via:

```bash
# Mac OSX
brew install gcc-arm-embedded

# Linux
sudo apt install gcc-arm-none-eabi

# Windows
# Download from:
#   https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads
```

Then, run `make`:

```bash
make
```

And the ROM will be output to `tgt/auntflora.gba`.

Hacking Instructions
--------------------

The map is edited with [Tiled](https://www.mapeditor.org/).

There are two maps:

1. `data/world_sd.json` - standard definition (SD)
2. `data/world_hd.json` - high definition (HD)

In order to edit the logic, you must edit both maps, so they are in sync with each other.

The tilemaps have layers:

1. `screens` - ignored; marks the screen boundaries for convenience
2. `markers` - special positions on the map that trigger actions (text popups, end of game, etc)
3. `objs` - objects that are layered above the base layer; these can potentially move
4. `base` - base layer, this is static and never changes

During the build process, the SD map is considered authoritative, and the HD map will be compared
against it to try and detect mistakes.

If you are changing the tilemap completely, then you should know there is some hardcoded behavior,
based on the original map:

1. `xform/xform.c`, function `ignore_warnings`, which ignores warnings on the HD end screen, which
   was changed a lot from the SD end screen
2. `src/main.c`, function `move_player`, has hardcoded logic to expand the size of the Parlor and
   Kitchen markers

The tilesets are:

1. `data/tiles_sd.png` - standard definition (SD)
2. `data/tiles_hd.png` - high definition (HD)

Notice that the SD tiles are 10x10, but with pixels doubled to match the original game's resolution
of 5x5.  The HD tiles are 12x12.

If you are changing the meaning of a tile, then you need to update a few locations:

1. `xform/xform.c`, constants `is_solid_sd` and `is_solid_hd`, which are used to flag solid tiles,
   and show warnings
2. `src/cellinfo.c`, constant `g_cellinfo`, which contains flags of how to interpret each tile

Note that if a tile is solid (via `is_solid_hd`), then it should have the high bit set in
`g_cellinfo` (`0x8000`).

Lastly, if you're making a new game, you probably want to change the metadata of the GBA ROM file:

1. `sys/gba/crt0.s`, game title, currently set to `AUNTFLORA\0\0\0`
2. `sys/gba/crt0.s`, game code, currently set to `CAFE77`

See [GBATEK](https://problemkaputt.de/gbatek.htm#gbacartridgeheader) for more information on these
values.
