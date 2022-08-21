# Zelda3
A reimplementation of Zelda 3.

## About

This is a reverse engineered clone of Zelda 3 - A Link to the Past.

It's around 70-80kLOC of C99 code, and reimplements all parts of the original game. The game is playable from start to end.

You need a copy of the ROM to extract game resources (levels, images). Then once that's done, the ROM is no longer needed.

It uses the PPU and DSP implementation from [LakeSnes](https://github.com/elzo-d/LakeSnes).
Additionally, it can be configured to also run the original machine code side by side. Then the RAM state is compared after each frame, to verify that the C++ implementation is correct.

I got much assistance from spannierism's Zelda 3 JP disassembly and the other ones that documented loads of function names and variables.

## Dependencies

- the `libsdl2-dev` library (ubuntu: `apt install libsdl2-dev`, macOS: `brew install sdl2`)
- a `tables/zelda3.sfc` ROM file (for asset extraction step only)
- The `pillow` and `pyyaml` python dependencies used by assets extractor

```
cd tables; pip install pillow pyyaml
```

### Windows
Build the .sln file with Visual Studio

### Linux

```sh
make
```

You can set `make` variables to use custom compilation flags or custom compiler:

```sh
make # will use default compiler
CC=gcc make
CC=clang make
CFLAGS=-O2 make
CFLAGS=-DSDL_DISABLE_IMMINTRIN_H CC=tcc make
CFLAGS=-DSDL_DISABLE_IMMINTRIN_H CC=chibicc make # also need SDL.h tweaking
CFLAGS=-m32 LDFLAGS=-m32 make # compile and link as x86 executable
```

## Usage and controls

The game supports snapshots. The joypad input history is also saved in the snapshot. It's thus possible to replay a playthrough in turbo mode to verify that the game behaves correctly.

The game is run with `./zelda3` and takes an optional path to the ROM-file, which will verify for each frame that the C++ code matches the original behavior.

| Button | Key         |
| ------ | ----------- |
| Up     | Up arrow    |
| Down   | Down arrow  |
| Left   | Left arrow  |
| Right  | Right arrow |
| Start  | Enter       |
| Select | Right shift |
| A      | X           |
| B      | Z           |
| X      | S           |
| Y      | A           |
| L      | D           |
| R      | C           |


Additionally, the following commands are available:

| Key | Action                |
| --- | --------------------- |
| W   | Fill health/magic     |
| E   | Hard reset            |
| P   | Pause                 |
| T   | Toggle replay turbo   |
| K   | Clear all input history from current snapshot  |
| F1-F10 | Load snapshot      |
| Shift+F1-F10 | Save snapshot |
| Ctrl+F1-F10 | Replay the snapshot |

Additionally, there are a bunch of included playthrough snapshots that play all dungeons of the game. You access them with the digit keys. If you want to replay the stage in turbo mode, press Ctrl+Digit (eg Ctrl-5).


## License

This project is licensed under the MIT license. See 'LICENSE.txt' for details.
