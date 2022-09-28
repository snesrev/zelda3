# Zelda3
A reimplementation of Zelda 3. 

Our discord server is: https://discord.gg/AJJbJAzNNJ

## About

This is a reverse engineered clone of Zelda 3 - A Link to the Past.

It's around 70-80kLOC of C code, and reimplements all parts of the original game. The game is playable from start to end.

You need a copy of the ROM to extract game resources (levels, images). Then once that's done, the ROM is no longer needed.

It uses the PPU and DSP implementation from [LakeSnes](https://github.com/elzo-d/LakeSnes), but with lots of speed optimizations.
Additionally, it can be configured to also run the original machine code side by side. Then the RAM state is compared after each frame, to verify that the C implementation is correct.

I got much assistance from spannierism's Zelda 3 JP disassembly and the other ones that documented loads of function names and variables.

## Additional features

Some features have been added that are not supported by the original game.

Secondary item slot on button X (Hold X in inventory to select).

Displays max rupees, bombs and arrows with yellow or orange color.

Extends throwing bombs to four instead of two.

Support for MSU audio tracks.

Support for enhanced aspect ratios of 16:9 or 16:10.

Switching current item with L/R keys.

Reordering of inventory by pressing Y+Arrows.

Higher quality map screen.

Disable low health beep.

Pick up items and destroy pots with Sword.

## Dependencies

- the `libsdl2-dev` library
  - Windows: automatically installed with NuGet
  - Ubuntu: `apt install libsdl2-dev`
  - macOS: `brew install sdl2`
- a `tables/zelda3.sfc` US ROM file (for asset extraction step only)
  - SHA256 : `66871d66be19ad2c34c927d6b14cd8eb6fc3181965b6e517cb361f7316009cfb`. 
- The `pillow` and `pyyaml` python dependencies used by the assets extractor.
  - `python -m pip install -r requirements.txt`

## Compiling

Look at the wiki at https://github.com/snesrev/zelda3/wiki for more help.

### Windows
First extract and compile resources. 

`cd tables`

Run `python3 extract_resources.py` to extract resources from the ROM into a more human readable format.

Run `python3 compile_resources.py` to create a file called `zelda3_assets.dat` that gets loaded by the executable.

In case you're planning to move the .exe to a different folder, please include `zelda3_assets.dat`.

Then build the .sln file with Visual Studio.

### Building with TCC instead of Visual Studio on Windows

You can also build and run it through TCC in case you don't have Visual Studio. Note that TCC builds without optimizations so the game will run considerably slower.


<details>
<summary>
Read about how to use TCC here...
</summary>
Extract the assets as detailed above.

Unzip [TCC for win64](http://download.savannah.gnu.org/releases/tinycc/tcc-0.9.27-win64-bin.zip) into `third_party/tcc`.

Unzip [SDL2](https://github.com/libsdl-org/SDL/releases/download/release-2.24.0/SDL2-devel-2.24.0-VC.zip) into `third_party/SDL2-2.24.0`.

Start `run_with_tcc.bat`. It will perform some basic error checks and then start the game.

</details>


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

In case you're planning to move the executable to a different location, please include the file `tables/zelda3_assets.dat`.

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
| L      | C           |
| R      | V           |

The keys can be reconfigured in zelda3.ini

Additionally, the following commands are available:

| Key | Action                |
| --- | --------------------- |
| Tab | Turbo mode |
| W   | Fill health/magic     |
| Shift+W   | Fill rupees/bombs/arrows     |
| Ctrl+E | Reset            |
| P   | Pause (with dim)                |
| Shift+P   | Pause (without dim)                |
| Ctrl+Up   | Increase window size                |
| Ctrl+Down   | Decrease window size                |
| T   | Toggle replay turbo mode  |
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
