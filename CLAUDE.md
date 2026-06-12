# CLAUDE.md — Zelda 3 Two-Player Co-Op Mod

## Project Overview

This is a C reimplementation of "The Legend of Zelda: A Link to the Past" (SNES).
The original repo is https://github.com/snesrev/zelda3.git (MIT licensed).
We are modifying it to support **local two-player co-op**, architected so that
**online multiplayer via deterministic input-lockstep** can be added later.

The vanilla game compiles and runs. A Phase 1 foundation has been partially
completed (see below). Your job is to finish all phases and deliver a working
two-player co-op game.

---

## Build Instructions

This project builds on Windows with TCC (Tiny C Compiler). The build command is:

```
third_party\tcc\tcc.exe -ozelda3_coop.exe -DCOMPILER_TCC=1 -DSTBI_NO_SIMD=1 -DHAVE_STDINT_H=1 -D_HAVE_STDINT_H=1 -DSYSTEM_VOLUME_MIXER_AVAILABLE=0 -DZELDA3_MULTIPLAYER=1 -Ithird_party\SDL2-2.26.3/include -Lthird_party\SDL2-2.26.3/lib/x64 -lSDL2 -I. src/*.c snes/*.c third_party/gl_core/gl_core_3_1.c third_party/opus-1.3.1-stripped/opus_decoder_amalgam.c
```

The vanilla (unmodified) game builds with:
```
third_party\tcc\tcc.exe -ozelda3.exe -DCOMPILER_TCC=1 -DSTBI_NO_SIMD=1 -DHAVE_STDINT_H=1 -D_HAVE_STDINT_H=1 -DSYSTEM_VOLUME_MIXER_AVAILABLE=0 -Ithird_party\SDL2-2.26.3/include -Lthird_party\SDL2-2.26.3/lib/x64 -lSDL2 -I. src/*.c snes/*.c third_party/gl_core/gl_core_3_1.c third_party/opus-1.3.1-stripped/opus_decoder_amalgam.c
```

The only difference is `-DZELDA3_MULTIPLAYER=1`. All multiplayer code MUST be
guarded by `#ifdef ZELDA3_MULTIPLAYER` so the vanilla build always still works.

**CRITICAL: After every change, compile BOTH builds and verify neither has errors.**
The vanilla build must never break.

TCC compiles `src/*.c` as a glob, so any new .c files in src/ are automatically
included.

Asset extraction has already been done — `zelda3_assets.dat` exists.

---

## Codebase Architecture (READ THIS CAREFULLY)

### How the original game works

The game faithfully mirrors SNES RAM layout. There is a single flat byte array:
```c
uint8 g_ram[131072];  // 128KB, declared in zelda_rtl.c
```

All game variables are `#define` macros that dereference into this array:
```c
#define link_x_coord (*(uint16*)(g_ram+0x22))
#define link_y_coord (*(uint16*)(g_ram+0x20))
// ... 168 total link_ variables, plus input, sprites, dungeon state, etc.
```

Player state, sprite state, dungeon state, overworld state — everything is global
macros into g_ram. There is no Player struct, no object system, no encapsulation.

### Key files and their roles

| File | Lines | Purpose |
|------|-------|---------|
| `src/variables.h` | 1529 | ALL game variable #defines (link_, sprite_, dungeon_, etc.) |
| `src/player.c` | 6664 | Player movement, state machine, physics |
| `src/player_oam.c` | 1752 | Player sprite rendering (OAM = Object Attribute Memory) |
| `src/sprite.c` | 4200 | Sprite system (enemies, NPCs, objects) |
| `src/sprite_main.c` | 22000+ | Individual sprite AI behaviors |
| `src/main.c` | 882 | SDL setup, input handling, game loop |
| `src/zelda_rtl.c` | 889 | Runtime layer, frame execution, g_ram declaration |
| `src/hud.c` | 1600 | Heads-up display (hearts, magic, items) |
| `src/dungeon.c` | 8400 | Dungeon room loading, transitions, logic |
| `src/overworld.c` | 3800 | Overworld scrolling, transitions |
| `src/tile_detect.c` | 600 | Tile collision detection |
| `src/nmi.c` | ~400 | NMI interrupt handler (input reading) |
| `src/ancilla.c` | ~7000 | Ancillae (sword beams, arrows, bombs, boomerangs) |
| `src/tagalong.c` | ~700 | Follower NPCs (Zelda, old man, etc.) |
| `src/types.h` | ~100 | Type definitions (uint8, uint16, bool, etc.) |

### How input flows

1. `main.c`: SDL events -> `g_input1_state` and `g_gamepad_buttons`
2. `main.c`: Combined into `inputs`, passed to `ZeldaRunFrame(inputs)`
3. `zelda_rtl.c`: `ZeldaRunFrame()` -> `ZeldaRunFrameInternal(input, run_what)`
4. `nmi.c`: `Interrupt_NMI(input)` writes to `joypad1H_last`, `filtered_joypad_H`, etc.
5. `player.c`: `Link_Main()` reads joypad globals to move Link

### How the frame loop works

```
ZeldaRunFrame(inputs)
  -> ZeldaRunFrameInternal(inputs, run_what)
    -> ZeldaRunGameLoop()     // one full game tick
    -> Interrupt_NMI(input)   // processes input, triggers DMA
```

### Critical property: DETERMINISM

The game simulation is 100% deterministic:
- RNG is SNES-faithful: `GetRandomNumber()` in misc.c uses frame_counter + state byte
- Zero floating point in game logic (only in SDL display layer in main.c)
- No system calls, threads, or rand() in simulation
- g_ram is 128KB — small enough to CRC32 every frame for desync detection

This means online multiplayer can use INPUT-ONLY LOCKSTEP: both machines run
identical simulation from identical inputs. Only InputFrame (8 bytes/player/frame)
needs to be exchanged over the network. This is why the architecture must preserve
determinism — NO floating point, NO system rand(), NO time-dependent logic in gameplay.

---

## What Has Been Done (Phase 1 — Partial)

### New files added to src/:

1. **`net_types.h`** — Network-ready type definitions:
   - `InputFrame` (8 bytes: frame_number, joypad, player_index, flags)
   - `FrameInputPair` (both players' inputs for one frame)
   - `SyncChecksum` (CRC32 of g_ram for desync detection)
   - `MultiplayerMode` enum (SINGLE, LOCAL, HOST, CLIENT, REPLAY)
   - `MultiplayerConfig` struct
   - Serialization/deserialization helpers for InputFrame
   - `ComputeCRC32()` declaration

2. **`player_state.h`** — PlayerState struct:
   - All 168 link_ variables as struct fields
   - Per-player input state (joypad globals)
   - Per-player non-link_ globals (button_mask_b_y, oam offsets, etc.)
   - Co-op fields (player_index, is_active, is_dead, respawn_timer, palette_index)
   - InputRingBuffer for future online play
   - Frame management API (Multiplayer_InputsReady, Multiplayer_ConsumeInputs, etc.)

3. **`player_state.c`** — Implementation:
   - g_players[2] array and cur_player pointer
   - PlayerState_SyncToRam() and PlayerState_SyncFromRam() using SYNC_U8/SYNC_U16 macros
   - PlayerState_Init() for both P1 (from g_ram) and P2 (defaults)
   - InputRingBuffer operations (Push, Pop, Peek, Count)
   - CRC32 implementation
   - Multiplayer_ComputeChecksum()
   - MultiplayerConfig with defaults

4. **`player_state_macros.h`** — Macro redirect layer:
   - Guarded by `#ifdef ZELDA3_MULTIPLAYER`
   - First `#undef`s all 168 link_ macros + input/button macros
   - Then `#include "player_state.h"` (AFTER undefs to avoid name collisions)
   - Then `#define`s them all to `cur_player->field_name`
   - This makes ALL existing code transparently use per-player state

### Modification to existing files:

- **`src/variables.h`**: Added `#include "player_state_macros.h"` just before the
  final `#endif // ZELDA3_VARIABLES_H_`

### Current status:

The game compiles and runs with `-DZELDA3_MULTIPLAYER=1`. It plays identically
to vanilla because cur_player always points to g_players[0] and the sync
functions bridge to g_ram. But only ONE player exists — no second player yet.

---

## What Needs To Be Done

### Phase 2: Dual Input and Second Player On Screen

**Goal: Two Links visible and independently controllable.**

1. **Dual input handling in main.c:**
   - Detect two Xbox controllers via SDL2 (SDL_GameController API)
   - Also support keyboard split: P1 = WASD/keys, P2 = arrow keys/numpad (configurable)
   - Route controller 0 -> player 0 inputs, controller 1 -> player 1 inputs
   - Store per-player raw input in InputRingBuffer or directly in player state
   - Feed inputs through existing Interrupt_NMI path but per-player

2. **Dual player update loop in zelda_rtl.c:**
   - After game loop processes P1, swap cur_player to P2 and run Link_Main() again
   - IMPORTANT: Must call PlayerState_SyncToRam() before P1 update and
     PlayerState_SyncFromRam() after, then swap and repeat for P2
   - The frame loop becomes:
     ```
     PlayerState_SetCurrent(0);
     PlayerState_SyncToRam();
     // run normal game loop (handles P1 via cur_player macros)
     PlayerState_SyncFromRam();
     
     PlayerState_SetCurrent(1);
     PlayerState_SyncToRam();
     Link_Main();  // P2 update only — world state already updated
     PlayerState_SyncFromRam();
     ```
   - This is the trickiest part — the normal game loop does MORE than just
     Link_Main(). It also updates sprites, scrolling, etc. P2 only needs
     Link_Main() + collision checks, NOT a full world re-tick.

3. **Render P2 sprite via player_oam.c:**
   - P2 needs OAM slots for their sprite
   - Palette swap: P2 uses a different palette (purple/red tunic)
   - Both players' sprites must be drawn each frame
   - Link's OAM rendering is in player_oam.c — need to call it for both players

4. **Shared camera (in overworld.c and dungeon.c):**
   - Camera centers between both players' positions
   - If players are close: center = midpoint
   - If players are far: center biased toward P1, P2 gets rubber-banded
   - Define a max leash distance (e.g., 200 pixels) — P2 is blocked at boundary

### Phase 3: Collision and Sprite Interaction

**Goal: Enemies see and interact with both players.**

1. **Sprite damage checks:**
   - In sprite.c, functions like `Sprite_CheckDamageToLink()` only check one player
   - Must iterate over both players OR check against whichever is closest
   - Sprites need to pick a target (nearest player, or random, or aggro-based)

2. **Ancilla (sword, arrows, boomerang, etc.):**
   - ancilla.c creates projectiles tied to the player who used them
   - Each player needs independent ancilla slots or a player_index tag on each ancilla
   - P2's sword swing must not use P1's position for hit detection

3. **Player-to-player interaction:**
   - NO friendly fire (co-op philosophy)
   - Players should not overlap (soft collision push-apart)
   - Players CAN push each other (optional, fun mechanic)

4. **Tile detection:**
   - tile_detect.c already uses link_ macros, so it works through cur_player
   - Verify it works correctly when called for P2

### Phase 4: HUD, Transitions, and Game State

**Goal: Full co-op game flow from start to finish.**

1. **HUD for P2:**
   - P1 HUD stays at top of screen (existing)
   - P2 HUD: compact health/magic display at bottom of screen
   - Or: P2 hearts shown below P1 hearts
   - Modify hud.c to draw for both players

2. **Screen transitions:**
   - When P1 goes through a door or walks off the overworld edge, the game
     transitions to a new area (unloads old, loads new)
   - P2 must be teleported to P1's position on transition
   - In dungeon.c: door transitions must warp P2
   - In overworld.c: scroll transitions must keep P2 in bounds
   - **Architecture for future split-screen:** Add a hook point where, instead of
     teleporting P2, we COULD load a second room. Do not implement it, just leave
     a clear comment and function stub like `// FUTURE: Multiplayer_HandleIndependentTransition()`

3. **Death and respawn:**
   - When a player dies, they enter ghost state (translucent, no collision)
   - Respawn timer (e.g., 5 seconds / 300 frames)
   - Respawn at the other player's position
   - Game over ONLY when both players are dead simultaneously
   - Modify the existing death handling in player.c

4. **Save/load:**
   - For now, P2 state is ephemeral (not saved)
   - P1 save/load works normally
   - P2 spawns fresh when co-op is activated

5. **Shared inventory:**
   - Both players share the same item pool (swords, items, keys, etc.)
   - If P1 picks up the hookshot, P2 also has it
   - Inventory lives in P1's PlayerState; P2 reads from P1 for items
   - Health and magic are independent per-player

### Phase 5: Polish and Edge Cases

1. **Cutscenes:** Many cutscenes lock player control. Both players must be locked.
2. **Boss fights:** Bosses target one player — should alternate or target nearest.
3. **Pits/holes:** Each player falls independently.
4. **Swimming:** Each player swims independently (already per-player via macros).
5. **Mirror warp:** If P1 uses mirror, P2 teleports with them.
6. **Tagalongs:** Followers (Zelda, old man) follow P1 only.
7. **Menu/inventory screen:** Pause should pause for both. Item selection is P1 only
   (or add P2 item selection on a different button).

### Phase 6: Online Multiplayer Foundation (Architecture Only)

**Do NOT implement networking. Just ensure the architecture supports it.**

1. **Verify determinism:**
   - Add a debug mode that runs two game instances in the same process with
     identical inputs and compares g_ram after each frame. Any divergence = bug.
   - Call Multiplayer_ComputeChecksum() every 60 frames in debug builds.

2. **Input abstraction:**
   - All input must flow through InputRingBuffer, never directly from SDL
   - Local mode: SDL -> InputRingBuffer -> game (delay = 0)
   - Future online: SDL -> InputRingBuffer locally, network -> InputRingBuffer for remote
   - The game loop calls Multiplayer_InputsReady() and Multiplayer_ConsumeInputs()

3. **State snapshot stubs:**
   - Add `GameState_Snapshot()` and `GameState_Restore()` function stubs
   - These would memcpy g_ram for rollback netcode (future)
   - Do not implement, just create the interface

4. **Replay recording stubs:**
   - Add `Replay_RecordFrame()` and `Replay_PlaybackFrame()` stubs
   - Recording = writing InputFrames to a file
   - Playback = reading InputFrames from a file into InputRingBuffer
   - Do not implement, just create the interface

---

## Design Principles (MUST FOLLOW)

1. **Co-op, not competitive.** No friendly fire. Players help each other.

2. **Vanilla-compatible.** The game must ALWAYS compile and run correctly without
   `-DZELDA3_MULTIPLAYER`. Guard ALL multiplayer code with `#ifdef ZELDA3_MULTIPLAYER`.

3. **Determinism is sacred.** No floating point in game logic. No rand(). No
   time-dependent behavior. The game must produce identical output from identical
   inputs, frame-for-frame. This is required for future online play.

4. **PlayerState must be serializable.** No pointers in PlayerState. No variable-length
   data. The struct must be safely memcpy-able and checksummable.

5. **Minimize diff to original code.** Use the macro redirect pattern to avoid
   rewriting existing functions. When you must modify existing files, make surgical
   changes guarded by `#ifdef ZELDA3_MULTIPLAYER`. Keep the original code path intact.

6. **Camera stays shared.** Both players share one screen. The camera centers between
   them. If they get too far apart, the trailing player is blocked or rubber-banded.
   Leave architectural hooks for future split-screen but do not implement it.

7. **P2 spawns from P1.** When co-op activates, P2 spawns next to P1 with matching
   progression (same items, same dungeon access) but independent health/magic.

8. **Xbox controller support is required.** SDL2 GameController API. Two controllers
   must work simultaneously. Keyboard fallback for testing (P1 = existing keys,
   P2 = configurable second set).

---

## How To Work

1. **Always compile after changes.** Run both build commands. Fix all errors before moving on.
2. **Work in phases.** Complete Phase 2 before starting Phase 3, etc.
3. **Test after each sub-feature.** After adding dual input, test it. After adding P2 rendering, test it. Do not batch everything.
4. **When stuck on a complex system** (like sprite interaction with two players), read the existing code thoroughly before modifying. grep for all callers of the function you are changing.
5. **Commit frequently** with descriptive messages like "Phase 2: dual Xbox controller input" or "Phase 3: sprite damage checks both players".

---

## File Map of New/Modified Files

```
src/
  net_types.h              — NEW: Network types, InputFrame, checksums
  player_state.h           — NEW: PlayerState struct, ring buffer, frame sync API  
  player_state.c           — NEW: Implementation of all the above
  player_state_macros.h    — NEW: #undef/#define redirect layer
  variables.h              — MODIFIED: added #include "player_state_macros.h" at bottom
  main.c                   — TO MODIFY: dual input, controller routing
  zelda_rtl.c              — TO MODIFY: dual player update loop
  player.c                 — TO MODIFY: death/respawn logic
  player_oam.c             — TO MODIFY: render both players
  sprite.c                 — TO MODIFY: damage checks vs both players
  sprite_main.c            — TO MODIFY: AI targeting
  hud.c                    — TO MODIFY: P2 health display
  overworld.c              — TO MODIFY: shared camera, transition warping
  dungeon.c                — TO MODIFY: shared camera, transition warping
  ancilla.c                — TO MODIFY: per-player projectiles
  nmi.c                    — TO MODIFY: per-player input processing
MULTIPLAYER_ROADMAP.md     — Project documentation
CLAUDE.md                  — This file
```

---

## Quick Reference: The Macro Redirect Pattern

This is the core architectural trick. Understanding it is essential.

**Before (vanilla):**
```c
// variables.h
#define link_x_coord (*(uint16*)(g_ram+0x22))

// player.c — reads/writes directly to g_ram
link_x_coord += link_x_vel;
```

**After (multiplayer):**
```c
// player_state_macros.h (included at bottom of variables.h)
#ifdef ZELDA3_MULTIPLAYER
  #undef link_x_coord
  // ... after all undefs ...
  #include "player_state.h"   // safe now, no macro conflicts
  #define link_x_coord (cur_player->x_coord)
#endif

// player.c — SAME CODE, but now goes through cur_player pointer
link_x_coord += link_x_vel;  // this line is UNCHANGED
```

When we set `cur_player = &g_players[1]`, ALL existing code automatically
operates on P2's state. This is why the macro redirect was Phase 1 — it
makes everything else possible without rewriting the game.

The SyncToRam/SyncFromRam functions bridge between the struct and g_ram
for subsystems that still access g_ram directly (sprites, dungeon logic, etc.).
As more systems are verified to work through macros, the sync calls can be
reduced.
