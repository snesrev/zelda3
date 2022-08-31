# Zelda3
A reimplementation of Zelda 3.

## About

This is a reverse engineered clone of Zelda 3 - A Link to the Past.

It's around 70-80kLOC of C/C++ code, and reimplements all parts of the original game. The game is playable from start to end.

You need a copy of the ROM to extract game resources (levels, images). Then once that's done, the ROM is no longer needed.

It uses the PPU and DSP implementation from LakeSnes. Additionally, it can be configured to also run the original machine code side by side. Then the RAM state is compared after each frame, to verify that the C++ implementation is correct.

I got much assistance from spannierism's Zelda 3 JP disassembly and the other ones that documented loads of function names and variables.

## Compiling

Put the ROM in tables/zelda3.sfc. The ROM needs to be the US ROM with SHA256 hash `66871d66be19ad2c34c927d6b14cd8eb6fc3181965b6e517cb361f7316009cfb`.

`cd tables`

Install python dependencies: `pip install pillow` and `pip install pyyaml`

Run `python extract_resources.py` to extract resources from the ROM into a more human readable format.

Run `python compile_resources.py` to produce .h files that gets included by the C++ code.

### Windows
Build the .sln file with Visual Studio

### Linux/macOS
#### Dependencies
Linux: `apt install libsdl2-dev`

macOS: `brew install sdl2`

#### Building
Make sure you are in the root directory.

```
clang++ `sdl2-config --cflags` -O2 -ozelda3 *.cpp snes/*.cpp `sdl2-config --libs`
```
or
`make -j$(nproc)`


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
| Keypad +/- | Resize window         |
| Alt+Enter | Toggle Fullscreen     |
| Shift+F1-F10 | Save snapshot |
| Ctrl+F1-F10 | Replay the snapshot |

Additionally, there are a bunch of included playthrough snapshots that play all dungeons of the game. You access them with the digit keys. If you want to replay the stage in turbo mode, press Ctrl+Digit (eg Ctrl-5).


## License

This project is licensed under the MIT license. See 'LICENSE.txt' for details.
