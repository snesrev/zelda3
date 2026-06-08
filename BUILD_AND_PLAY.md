# Build & Play — Two-Player Co-Op

This fork adds **local two-player co-op** to the C port of *Zelda: A Link to the Past*.
Two Links share one screen: green **Player 1**, red **Player 2**.

> All co-op code is guarded by `-DZELDA3_MULTIPLAYER=1`. Building **without** that
> define produces the unmodified single-player game.

---

## 1. What you need

The repository contains **source only**. To build and run you must also provide,
in the project root:

| File / folder | What it is | How to get it |
|---|---|---|
| `zelda3.sfc` | US v1.0 ROM | Supply your own (never committed — copyright) |
| `zelda3_assets.dat` | Extracted game assets | Generate it (see below) |
| `third_party\tcc\` | Tiny C Compiler | From your existing zelda3 setup |
| `third_party\SDL2-2.26.3\` | SDL2 dev libs | From your existing zelda3 setup |
| `SDL2.dll` | SDL2 runtime (next to the .exe) | From `third_party\SDL2-2.26.3\lib\x64\` |

Generate the assets once (needs Python 3 + `pip install Pillow PyYAML`):

```
python assets\restool.py -r zelda3.sfc --extract-from-rom
```

---

## 2. Build (Windows / TCC)

**Co-op build:**

```
third_party\tcc\tcc.exe -ozelda3_coop.exe -DCOMPILER_TCC=1 -DSTBI_NO_SIMD=1 -DHAVE_STDINT_H=1 -D_HAVE_STDINT_H=1 -DSYSTEM_VOLUME_MIXER_AVAILABLE=0 -DZELDA3_MULTIPLAYER=1 -Ithird_party\SDL2-2.26.3/include -Lthird_party\SDL2-2.26.3/lib/x64 -lSDL2 -I. src/*.c snes/*.c third_party/gl_core/gl_core_3_1.c third_party/opus-1.3.1-stripped/opus_decoder_amalgam.c
```

**Vanilla build** (same, minus `-DZELDA3_MULTIPLAYER=1`, output `zelda3.exe`):

```
third_party\tcc\tcc.exe -ozelda3.exe -DCOMPILER_TCC=1 -DSTBI_NO_SIMD=1 -DHAVE_STDINT_H=1 -D_HAVE_STDINT_H=1 -DSYSTEM_VOLUME_MIXER_AVAILABLE=0 -Ithird_party\SDL2-2.26.3/include -Lthird_party\SDL2-2.26.3/lib/x64 -lSDL2 -I. src/*.c snes/*.c third_party/gl_core/gl_core_3_1.c third_party/opus-1.3.1-stripped/opus_decoder_amalgam.c
```

Then run `zelda3_coop.exe` (make sure `SDL2.dll`, `zelda3.sfc`, and
`zelda3_assets.dat` are in the same folder).

> **Linux/gcc** is also supported via `build.sh` (`./build.sh coop`) — used for the
> automated headless tests; see `src/main.c` (`ZELDA3_HEADLESS_TEST`).

---

## 3. Starting co-op

There's nothing to toggle. **Load a save or start a new game**, and as soon as
Player 1 is in normal gameplay (the overworld or a dungeon), **Player 2 spawns
right next to Player 1** (red tunic) and is fully controllable. Both players'
hearts show in the HUD — P1's row on top, P2's row directly below.

---

## 4. Controls

Both players can share one keyboard, or each can use an Xbox controller.

### Controllers (recommended)
Plug in two controllers. The **first** connected = **Player 1**, the **second** =
**Player 2**. Both use the game's standard pad mapping (face buttons = A/B/X/Y,
D-pad **or** left stick to move, shoulders = L/R, Start/Back = Start/Select).
Mappings can be customized in `zelda3.ini`.

### Keyboard

| Action | Player 1 | Player 2 *(NumLock ON)* |
|---|---|---|
| Move | **Arrow keys** | **Numpad 8 / 2 / 4 / 6** (Up/Down/Left/Right) |
| **B** — sword | **Z** | **Numpad 5** |
| **A** — lift / run / talk | **X** | **Numpad 0** |
| **Y** — use equipped item | **A** | **Numpad 7** |
| **X** — map | **S** | **Numpad 9** |
| **L** | **C** | **Numpad −** |
| **R** | **V** | **Numpad +** |
| **Start** | **Enter** | **Numpad 3** |
| **Select** | **Right Shift** | **Numpad 1** |

> P2's keys are deliberately on the numpad so they don't collide with P1's arrows.
> NumLock must be **on** for the numpad keys to register.

### Useful P1 hotkeys (single set, affects the whole game)
- **Save state:** `Shift`+`F1`…`F10` — **Load state:** `F1`…`F10`
- **Pause:** `P` — **Fullscreen:** `Alt`+`Enter` — **Reset:** `Ctrl`+`R` — **Turbo:** hold `Tab`

---

## 5. How co-op plays

- **Shared screen / camera** centers on Player 1; Player 2 is kept within the
  viewport (leashed) so both stay on screen.
- **Combat both ways:** enemies can damage either player, and either player's
  sword/items damage enemies. Most enemies and bosses **chase whichever player is
  closer**. **No friendly fire** with swords/arrows/beams. Bombs explode on
  whoever is in range (either player) — but there's no extra cross-damage.
- **Screen transitions:** when P1 goes through a door, dungeon entrance, or screen
  edge, P2 comes along and re-appears beside P1 in the new room/area.
- **Going down (not game over):** if one player's hearts hit 0 they drop into a
  **flashing "downed" ghost** — invulnerable, can't act normally — instead of
  ending the game. They **revive** automatically after a few seconds beside the
  living partner, **or instantly if the other player walks over to them**
  (revive-on-touch). It's only **Game Over if *both* players are down at once.**
- **Shared progression:** Player 2 starts with the same items/equipment as Player 1,
  and pickups/upgrades are pooled (incl. heart containers, which raise **both**
  players' max hearts). **Current health and magic are independent** per player.

---

## 6. Known limitations (current state)

- P2's hearts are the same color as P1's (distinguished by being the second HUD
  row); if P1 has **more than 10 hearts**, P1's second heart row overlaps P2's.
- P2's magic meter is a compact green gauge in the gap just left of the heart
  rows (P1's is the tall gauge on the far left); both are independent.
- **P2 can lift *dungeon* pots, but not yet *overworld* sprite objects** (bushes,
  rocks, throwable pots). The team still gets those items via P1 (shared
  inventory). This needs a per-player lift/carry pass and is best done with
  interactive testing.
- The **Cane of Somaria** block, when created by P2, currently latches to P1 for
  carrying.
- A few bosses that read the player position directly still target P1 (most
  enemies/bosses target the nearest player).
- **Item selection (the Y-item) is Player 1's** — P2 uses whatever item P1 has
  equipped.
- The "downed" ghost is a simple flash (the engine has no true translucency).
- Co-op shares one screen (no split-screen). A hook for future independent-room /
  split-screen transitions is stubbed in `zelda_rtl.c`.
- Inventory/menu and reference-save hotkeys are Player 1 only.

---

## 7. Where the co-op code lives

| File | Role |
|---|---|
| `src/player_state.h/.c` | Per-player state struct, sync, death/respawn, init |
| `src/player_state_macros.h` | Redirects `link_*` globals through `cur_player` |
| `src/zelda_rtl.c` | Dual-player game loop, shared camera, transitions |
| `src/main.c` | Dual input (keyboard + 2 controllers), headless test harness |
| `src/player.c`, `overworld.c` | Guarded co-op death interception |
| `src/player_oam.c` | P2 sprite (palette swap, ghost flash) |
| `src/hud.c` | P2 heart row |
| `src/sprite.c` | Two-way damage (driven from the loop) |

Every change is guarded by `#ifdef ZELDA3_MULTIPLAYER`, so the vanilla build is
byte-for-byte unaffected.
