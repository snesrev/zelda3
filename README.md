# Zelda3
A reimplementation of Zelda 3.

## About

This is a reverse engineered clone of Zelda 3 - A Link to the Past.

It's around 70-80kLOC of C code, and reimplements all parts of the original game. The game is playable from start to end.

You need a copy of the ROM to extract game resources (levels, images). Then once that's done, the ROM is no longer needed.

It uses the PPU and DSP implementation from [LakeSnes](https://github.com/elzo-d/LakeSnes).
Additionally, it can be configured to also run the original machine code side by side. Then the RAM state is compared after each frame, to verify that the C implementation is correct.

I got much assistance from spannierism's Zelda 3 JP disassembly and the other ones that documented loads of function names and variables.

## Dependencies

- the `libsdl2-dev` library
  - Windows: automatically installed with NuGet
  - Ubuntu: `apt install libsdl2-dev`
  - macOS: `brew install sdl2`
- a `tables/zelda3.sfc` US ROM file (for asset extraction step only)
  - SHA256 : `66871d66be19ad2c34c927d6b14cd8eb6fc3181965b6e517cb361f7316009cfb`. 
- The `pillow` and `pyyaml` python dependencies used by the assets extractor.
  - `pip install pillow pyyaml`

## Compiling

Look at the wiki at https://github.com/snesrev/zelda3/wiki for more help.

### Windows
First extract and compile resources.

`cd tables`

Run `python3 extract_resources.py` to extract resources from the ROM into a more human readable format.

Run `python3 compile_resources.py` to produce .h files that get included by the C code.

Then build the .sln file with Visual Studio.

### Linux/macOS

```sh
make
```
<details>
<summary>
Advanced make usage ...
</summary>

```sh
make -j$(nproc) # run on all core
make clean all  # clear gen+obj and rebuild
CC=clang make   # specify compiler
```
</details>

## Usage and controls

The game supports snapshots. The joypad input history is also saved in the snapshot. It's thus possible to replay a playthrough in turbo mode to verify that the game behaves correctly.

The game is run with `./zelda3` and takes an optional path to the ROM-file, which will verify for each frame that the C code matches the original behavior.

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

The keys can be reconfigured in zelda3.ini

Additionally, the following commands are available:

| Key | Action                |
| --- | --------------------- |
| W   | Fill health/magic     |
| Shift+W   | Fill rupees/bombs/arrows     |
| E   | Hard reset            |
| P   | Pause                 |
| T   | Toggle replay turbo   |
| O   | Set dungeon key to 1  |
| K   | Clear all input history from the joypad log  |
| L   | Stop replaying a shapshot  |
| R   | Toggle between fast and slow renderer |
| F   | Display renderer performance |
| F1-F10 | Load snapshot      |
| Alt+Enter | Toggle Fullscreen     |
| Shift+F1-F10 | Save snapshot |
| Ctrl+F1-F10 | Replay the snapshot |
| 1-9 | run a dungeons playthrough snapshots |
| Ctrl+1-9 | run a dungeons playthrough in turbo mode |


## License

This project is licensed under the MIT license. See 'LICENSE.txt' for details.
