# Co-Op Code Review — Two-Player Capabilities

Focused review of whether **Player 2 is a complete second player**. The
single-player engine is unchanged and works; this looks only at the co-op paths.

**Architecture verdict:** sound. The dual-player loop (`ZeldaRunGameLoop_Multiplayer`
in `zelda_rtl.c`), the `cur_player` macro redirect (`player_state_macros.h`), and
the struct↔g_ram sync (`player_state.c`) are the right design. P2 movement,
collision, melee combat both ways, screen transitions, death/respawn, and the HUD
all work. The issues below are about completeness of P2's capabilities.

---

## ✅ FIXED — Per-player state leaks (commit `4446aff`)

**Severity: HIGH (general desync).** 10 variables that `Link_Main` writes every
frame as Link's *own* state were not in `PlayerState`, so both players shared them
through `g_ram` and corrupted each other:

`player_handler_timer` (0x300, ~45 write sites), `countdown_for_blink` (0x31F,
post-hit i-frames), `state_for_spin_attack`/`step_counter_for_spin_attack`
(0x31C/0x31D), `player_near_pit_state` (0x5B), `player_on_somaria_platform`
(0x2F5), `flag_is_link_immobilized` (0x2E4), `kind_of_in_room_staircase` (0x44A),
`about_to_jump_off_ledge` (0x47A), `flag_unk1` (0xFC1).

Fixed by adding all 10 as per-player fields (struct + sync + macro redirect).
Verified: builds clean, deterministic, self-test passes.

---

## ✅ FIXED — P2 can pick up items / hearts / drops (commits `b5c2675`, `35625d0`)

> Resolution: in P2's update loop, active prize sprites (types 0xD8–0xE6) are run
> through the engine's own `Sprite_CheckAbsorptionByPlayer` with `cur_player == P2`,
> so P2 collects what it overlaps (P1's earlier pass + despawn-on-collect prevents
> double-collect). `Multiplayer_RefillP2` then converts P2's collected heart/magic
> fillers into P2 health/magic (the engine's `Hud_RefillLogic` only runs for P1).
> Verified: P2 heals from collected hearts. (Shared-item team-pooling — P2's
> rupees/keys going to P1's totals — is the remaining nuance; per-player health/
> magic, the main need, works.)

**(original finding below)**
**Severity: HIGH for "real second player".** Item/rupee/heart/key collection is
detected inside **sprite AI**, which runs only in P1's full `Module_MainRouting()`
(`Sprite_Main` → per-sprite handlers; the touch test uses `link_x/y` = P1). P2's
reduced update never runs sprite AI, so **P2 walking over a drop collects nothing**
— most visibly, P2 can't grab hearts to heal (health is per-player).

- Evidence: `sprite.c` `Sprite_ReturnIfLiftedPermissive` (~2615) and
  `Link_ReceiveItem` calls in `sprite_main.c` are reached only via `Sprite_Main`
  (`zelda_rtl.c` P1 routing). P2 loop runs only `Link_Main` + the damage checks.
- Mitigation today: the leash keeps players close, so P1 usually grabs shared
  drops (rupees/keys/items go to the shared inventory). The real loss is **P2
  healing from hearts** and the feel of P2 collecting.
- **Recommended fix:** after P2's `Link_Main`, run a P2-targeted collection pass —
  loop active "collectible" sprites and run the same proximity/collect check with
  `cur_player == P2` (mirror the structure of the existing P2 damage loop). Care:
  don't double-collect P1's pickups (gate on which player actually overlaps).

---

## ✅ FIXED — P2's link-relative projectiles follow P2 (commit `43359c5`)

> Resolution: each ancilla slot is tagged with the firing player in
> `Ancilla_AllocInit` (the universal slot allocator, so every spawn records the
> current `cur_player`), and `Ancilla_ExecuteOne` runs the link-relative types
> (boomerang 0x05, hookshot 0x1F, Byrna 0x30/0x31) with `cur_player` set to that
> owner, restoring afterward. Other ancillae pass through the same save/restore as
> a no-op. Verified: determinism holds with ancillae active, no crash, vanilla +
> coop build clean. (P2 *can* use Y-items — `Link_HandleYItem` runs in P2's
> `Link_Main` path — so this is meaningful; exact in-hand behavior of the 3 items
> is best confirmed in interactive play.)

**(original finding below)**
## 🔴 OPEN — P2's link-relative projectiles track P1 (boomerang, hookshot, Cane of Byrna)

**Severity: HIGH for those 3 items.** Ancillae are updated once per frame inside
P1's `Module_MainRouting` (`Ancilla_ExecuteAll`, `ancilla.c:661`) with
`cur_player == P1`. They spawn correctly at P2's position (created during P2's
`Link_Main`), but every subsequent frame they read `link_*` = **P1**:

- **Boomerang** (`ancilla.c:1282`) returns to P1, not the thrower.
- **Hookshot** (`player.c:3123/3129`) pulls P1, not the user.
- **Cane of Byrna** sparks (`ancilla.c:4520-4521`) orbit P1, not the user.

Confirmed *fine*: sword melee (handled in `Link_Main`, runs for P2), the sword
**beam** (spawn-time position), **arrows**, **bombs** (position-based), **fire/ice
rod** shots (position+velocity), picked-up-bomb carry (latches current carrier).

- **Why it's not a quick fix:** ancilla creation is **decentralized** (~150 direct
  `ancilla_type[k]=` sites, no single allocator), so tagging an owner per slot is
  a scattered change.
- **Recommended fix:** add `g_ancilla_owner[16]`; set it = current player index at
  the boomerang/hookshot/byrna spawn sites (only ~3 types need it); in
  `Ancilla_ExecuteOne`, for those types do `PlayerState_SetCurrent(owner)` around
  the call and restore P1 after. Reset the owner slot when an ancilla frees.
  (A fuller version tags *all* ancillae via a shared spawn helper.)

---

## 🟡 OPEN / by-design — P2 cannot independently trigger door/screen transitions

**Severity: LOW (shared-screen design).** Transitions are driven by P1
(`is_standing_in_doorway` etc. are set in P1's routing). P2 **follows** P1 through
any transition (handled — P2 freezes during the scroll and warps beside P1 on
completion). P2 walking into a door that P1 isn't near does nothing. This is
acceptable for one shared screen; true independent transitions need the
split-screen path (stub left in `zelda_rtl.c`). No action needed unless
split-screen is desired.

---

## 🟢 Verified working for P2

Movement & tile collision; pushing/lifting/reading (tile actions run in
`Link_Main`); pits/holes; swimming; **melee sword damage to enemies** and
**taking damage from enemies** (the real `Sprite_CheckDamageTo/FromLink` loop —
the earlier crude stub was replaced); shared camera (leash); screen-transition
follow; downed/ghost + respawn + revive-on-touch; shared inventory (P2 inherits
P1's gear); per-player health/magic; two-row HUD; corrected P2 keyboard +
controller input.

**Note (benign):** the P2 damage loop runs `Sprite_CheckDamageTo/FromLink` a
second time per frame; per-sprite hit cooldowns (`sprite_hit_timer`) prevent
double-processing, so this is safe but slightly redundant.

---

## Priority for further work
1. ~~P2 pickups~~ — **done** (`b5c2675`).
2. ~~Ancilla owner for boomerang/hookshot/Byrna~~ — **done** (`43359c5`).
3. ~~Shared-item team pooling~~ — **done** (`c5e2beb`): `Multiplayer_ShareInventory`
   mirrors the inventory block both ways each frame, so items/rupees/keys/upgrades
   are one shared pool while health/magic stay per-player. Verified: P2 inherits
   P1's rupees/keys/bow; P2 health stays independent; deterministic.
4. (Optional) independent transitions / split-screen.

---

## Round 2 — adversarial hardening (PR #3)

A second full review (multiple focused audits) found and fixed:

- **18 more per-player state leaks** — Link-personal `g_ram` vars `Link_Main`/the
  OAM renderer write each frame that weren't in `PlayerState` (ripple/grass & foot
  OAM variant, lift/throw/grab/dash anim timing, swim-stroke cadence, pit-fall
  state, hookshot pull, doorway gating, medallion-cast guard, block-push and
  knockback-recoil timers). P2-disabled CRC is byte-identical to before.
- **Death/respawn** — double-KO race (zombie revive during game-over), ghost now
  frozen (can't corrupt shared module state), revive health rounded to a whole
  heart, fillers cleared on death, ghost invulnerability gated on `is_dead`.
- **CRITICAL save-load inventory wipe** — `SyncToRam` ran from boot with a zeroed
  P1 struct and overwrote the just-loaded inventory before P1 was seeded; now
  `SyncToRam` is gated on `g_p1_seeded` and P1 is seeded across the load+gameplay
  module span (5–11).
- **Controller disconnect** — `SDL_CONTROLLERDEVICEREMOVED` handler (no stuck
  inputs / mis-routed reconnect).
- **Shared heart-container capacity** — `health_capacity` now shared so containers
  raise both players' max HP; current health/magic stay per-player.
- **Desync checksum** — now folds `g_players[]` + the ancilla owner table in, so
  P2 desyncs are detectable and the headless determinism checks cover P2.

### Still open (foundation / cosmetic, non-blocking)
- P2 magic meter on HUD (cosmetic; needs visual iteration).
- Replay/record logs P1 input only; input-ring-buffer abstraction not yet wired
  into the live path (future online-lockstep foundation).
- A few bosses read `link_x/y` directly and still target P1 (most target nearest
  via the centralized helpers).

---

## Round 3 — combat / ancilla / collision audit

Verified working for P2: sword, sword beam, spin attack, arrows, fire/ice rod,
hammer all damage enemies (the damage path reads only per-player state); no
double-damage / double-drop (detection sets pending damage, recoil applies it
once, `Sprite_GiveDamage` takes max not sum); no friendly fire via
sword/arrow/beam (players aren't sprites); P2 tile collision, block pushing,
chest opening and dungeon-pot lifting are independent and correct; ancilla owner
tagging covers all four swapped link-relative types.

**Fixed:**
- **Bombs damage both players** (was: blast only hit P1 — FF against P1 and P2
  bomb-immune). Split `Bomb_CheckPlayerDamage` out and run it for both players;
  sprite damage still runs once.

**Deferred to interactive testing (risky to do blind; documented):**
- **P2 lifting *sprite-based* objects** (overworld bushes/rocks/pots): the
  detect→latch→execute→carry chain is split across P1's sprite-AI phase and the
  player handler and mutates shared sprite state; enabling it for P2 safely needs
  real liftable-sprite testing (a blind change risks breaking P1's core
  lift/carry). P2 can already lift *tile-based* dungeon pots. Plan: make the
  lift/carry flags per-player and run a P2 detect+execute pass.
- **Cane of Somaria block carry** mis-targets P1 for P2 (type 0x2C not in the
  ancilla owner-swap list; also touches shared carry flags). Advanced item;
  needs the same per-player carry-flag work.

**By design:** P2's Y-item is P1's selected item (Phase 5.7: "item selection is
P1 only"); could add an independent P2 item-cycle later.

**Low / cosmetic:** 1-frame homing lag on P2's returning boomerang / Byrna spark
(ancillae update in P1's phase, before P2 moves); these are non-blocking.

## Review round — boss damage, P2 world-flow, soft collision

A fresh 3-front adversarial review (determinism/`cur_player`, combat/sprites,
world-flow) after the online netcode landed. Fixes shipped:

- **Bosses & hazards damage BOTH players** (was: P2 immune to a whole class of
  inline-AI contact damage). Agahnim's barrier, King Helmasaur body/tail/fireball,
  Trinexx trail/shell, Blind bump, indoor boulder, Guruguru/firebar now hit P2 via
  the bomb pattern + new `MP_ALSO_FOR_P2()` (`src/mp_dual.h`). RNG-free, P1 path
  byte-identical, vanilla unchanged.
- **P2 hazards no longer hurt P1.** P2 drowning/pit/fall used to set the shared
  module/submodule, whose recovery ran against P1 (bounced P1, stranded P2). P2's
  `Link_Main` is now bracketed: the shared transition state is reverted and the
  hazard is resolved locally for P2 (ghost-if-lethal, else pull beside P1).
- **P2 input** gets the same opposing-direction (up+down / left+right) filter as P1.
- **Stale transient state** (handler/aux/z/deep-water/incap) now cleared whenever
  P2 is relocated (transition warp, hazard recovery, ghost revive) — unified into
  `Multiplayer_PlaceP2Beside()`.
- **Camera-leash** low-side clamp no longer underflows `uint16` near the map origin.
- **Soft player collision (Phase 3):** P2 is nudged 1px/frame out of P1's body so
  the Links don't fully stack (P1 stays the camera anchor); integer/deterministic.

**Verified clean (no action):** `cur_player` save/restore discipline, no
float/rand/time in the sim, sync coverage, ancilla ownership, friendly-fire
impossibility, game-over only on double-KO, transition-warp coverage,
shared-inventory vs per-player health/magic, pause/cutscene lock.

**Documented cautions (future, not bugs):** StateRecorder rewind is P1-only (co-op
sessions can't rewind — P2 input isn't recorded); the rollback snapshot stub must
also capture the ancilla/sprite owner tables; `PlayerState` implicit padding is
fine within one binary but would need field-wise checksums for cross-ABI online.

**Headless limits:** boss fights, deep water/pits, and the leash boundary aren't
reachable by the start-area harness, so those exact paths aren't exercised in CI;
changes are `#ifdef`-guarded and determinism stays byte-identical + lockstep-neutral.

## Review round 2 — frontend/online + special subsystems

A second adversarial pass (`main.c` input/controller/online wiring; tagalongs,
mirror/warps, menu, swimming, HUD) after round-1's fixes landed.

**Fixed:**
- **Online input-ring overflow on a stalled/paused peer (CRITICAL desync).**
  `InputRing_Push` had no full-check; a paused peer (stops ticking, sends no
  input and no BYE) froze the sim while local capture kept pushing, wrapping the
  256-slot ring and clobbering unconsumed input -> desync on resume. Push now
  refuses to overwrite when full and the lockstep driver holds local capture.
  Headless `ringcap` test added.
- **Online autosave desync (MEDIUM).** The launch autosave-load ran for online
  peers, seeding host/client from possibly-different saves -> frame-1 desync.
  Skipped for HOST/CLIENT.
- **P2 dungeon-hole room corruption (CRITICAL).** A P2 pit fall reached the
  fall's room-change writes (`dungeon_room_index` / overworld pit transition)
  that the hazard-revert can't undo. P2 now bails at the room-change moment (the
  `submodule_index` it sets trips the revert, snapping P2 beside P1); only P1
  leads room-changing falls. Covers indoor travel/damaging pits and outdoor pits.

**Documented (verified non-desync; deferred — cosmetic/minor or unverifiable
headlessly, where a blind change risks a regression):**
- **In-room staircases:** P2 stepping on one only sets `submodule`/`main_module`
  (both cleaned up by the revert -> no corruption); P2 is snapped beside P1
  rather than changing dungeon layer independently. Non-corrupting.
- **HUD: P2 hearts overlap P1's 2nd heart row** once a player exceeds 10 hearts
  (both use cols 20-29; P2 draws over P1's row 2). Purely cosmetic. A clean
  relocation needs a verified free, on-screen HUD row; the reference save has 3
  hearts so neither the overlap nor a fix is observable headlessly — deferred
  rather than risk an off-screen/flickering move (the dungeon floor indicator
  occupies the only obvious lower-row slot, intermittently).
- **Both players swimming at once** share the swim-stroke scratch
  (`swimcoll_var*`, g_ram 0x326-0x33F, not per-player) -> glitchy strokes.
  Deterministic (identical on both online peers, so NOT a desync); a lone
  swimmer is fine. Per-player split deferred (irregular scratch layout,
  unverifiable headlessly).
- **P2 firing the Magic Mirror / a warp item:** absorbed by the hazard-revert
  (P2 snapped to P1, no magic cost, shared writes overwritten by P1). Benign.
- **Shared camera anchors on a downed P1** for the ~4s revive window, leashing a
  living P2 near P1's body (revive-on-touch always reachable). By design.
- **Controller hotplug edge cases:** unplugging P1's pad compacts the table and
  promotes P2's pad to P1; a 5th controller leaks its handle; a duplicate
  DEVICEADDED double-tracks a pad. Real but niche (need physical replug / 5+
  pads) and unverifiable in the SDL-less harness.

**Verified clean:** controller instance-id vs device-index handling, keyboard
P1/P2 mapping, online local/remote wiring (host=P1 / client=P2, in-order
consume, input sampled 1:1 per sim frame), tagalongs (P1-only by spec), medallion
spells, menu/pause freeze, P2 OAM floor priority, magic-HUD tilemap bounds,
ghost-revive gating, BYE-on-quit.

## Review round 3 — dungeon/puzzle/item mechanics with two players

Audited the least-covered area. Dungeon room-tag handlers and switch/lever
sprite-AI run only in P1's module pass (P2 runs `Link_Main` only), so they read
P1's position/buttons exclusively. **None are desyncs** (deterministic on both
peers) or hard soft-locks (P1 can operate everything; the leash keeps P2 close,
so dungeons stay completable). The gap is the co-op affordance — "P2 holds the
switch / pulls the lever."

**Fixed (additive, P1 path byte-identical — verified CRC unchanged):**
- **Pressure plates, blue/orange star-tile floor switches, and shutter switches**
  now trigger from EITHER player. `RoomTag_MaybeCheckShutters` /
  `RoomTag_CheckForPressedSwitch` check P1 first (unchanged), then re-run the
  identical tile check as P2. P2 can hold a plate to keep a door open, flip a
  star-tile floor, or trip a shutter.

**Documented / deferred** (unverifiable headlessly; fixes touch shared switch-AI
state where a blind change could break a co-op puzzle — need in-dungeon
playtesting):
- **Pull levers / crystal-pull switches** (water flood/drain, pull-switch walls)
  live in sprite AI gated on P1's contact+buttons — P2 can't pull them.
- **Push/depress water switches & A-press interaction sprites** detect only P1.
- **Boots-dash breakable walls** check P1's dash state only (bomb-opened walls
  already work for both).
- **Locked / big-key doors** open only from P1's facing tile (keys are SHARED,
  only P1 decrements — verified NO double-consume; P2 just can't open a lock).

**Verified CLEAN:** torch lighting works for P2 (fire keys off the tile
attribute, not position; lit count is shared room state); chests & absorbable
pickups are single-award, routed to the correct player then pooled (no
double-award / double-key-consume); crystal switches toggle from P2's hits;
block-pushing works for P2. No new desync vectors found.

## Review round 4 — economy / NPC / item-receipt interactions

Audited shops, paying minigames, NPC dialogue, fountains, and the item-receipt
machinery.

**Fixed (CRITICAL + the High that shares its root cause):**
- **P2 collecting a hold-up item (e.g. a dropped big key) soft-locked P2
  permanently.** `Link_ReceiveItem` puts the receiving player in the empty
  `HoldUpItem` pose + sets `disable_sprite_damage`, and that pose is ONLY ended
  by the item-receipt ancilla (type 0x22). But `Ancilla_ExecuteOne` redirected
  `cur_player` to the ancilla owner for only 0x05/0x1F/0x2C/0x30/0x31 — not
  0x22 — so a P2 receipt ran as P1, reset P1, and left P2 frozen forever (the
  ghost-revive couldn't fire because `disable_sprite_damage` stayed set). Also
  the heal/heart-container went to P1. Fix: add 0x22 to the owner-redirect so the
  receipt runs as the collector, AND mirror any heart-container `health_capacity`
  delta from P2 onto P1 so the per-frame inventory sync doesn't wipe the shared
  max-HP gain. Purely additive — P1/vanilla path byte-identical (CRC unchanged).

**Documented / deferred:**
- **Milestone (big-chest) item ancilla (0x29)** reads Link's collision to decide
  who collects and is more complex; it is NOT a P2 soft-lock (a P2-opened big
  chest is simply collected by P1 — the item is shared, so still obtained), so it
  is left as a minor affordance gap pending in-dungeon playtesting.

**Verified CLEAN (no double-charge / double-award / desync):** shops & all paying
minigames check affordability before deducting and run once in P1's pass against
the SHARED rupee pool (no double-charge, no underflow; P2 can't buy — the
documented A-press-interaction-P1-only limitation); prize/rupee/heart absorption
is single-collect (`sprite_state=0` immediately) and credited once then pooled;
the rupee count-up animation runs once and is mirrored (no divergence); text
boxes freeze BOTH players (the `submodule_index!=0` gate re-evaluates the same
frame a box opens); archery/digging awards fire once into the shared pool.

These were verified by code inspection; the start-area harness can't reach
dungeon big-key drops / shops to exercise them at runtime.

## Review round 5 — save / continue / game-over / intro / map flows

Audited the last-uncovered flows.

**Fixed (CRITICAL):**
- **Real double-KO → "Continue" (or bottle-fairy revival) left BOTH players as
  permanent frozen 0-HP ghosts (soft-lock).** The co-op downed flags are cleared
  only by `Multiplayer_OnSaveLoaded` (via `CopySaveToWRAM`), but the in-game
  continue choices and the fairy-revival path return to gameplay WITHOUT routing
  through it. So both players stayed `is_dead`, and `UpdateDeathRespawn`'s
  "both down -> stay down" rule kept re-zeroing their HP and freezing them every
  frame, with no revive path. Fix (additive): in the per-frame co-op bookkeeping,
  if both players are dead but the engine is back in a normal gameplay module
  (7/9, not the game-over module), call `Multiplayer_OnSaveLoaded()` so the next
  `InitIfNeeded` re-seeds P1 (clearing its downed state) and re-clones P2 — both
  come back. (A fatal double-KO switches to the game-over module the same frame,
  so both-dead can't legitimately persist in 7/9; the trigger only fires
  post-continue.) P1/vanilla path byte-identical (CRC unchanged).

**Verified CLEAN:** save / Save-and-Quit / autosave write only the shared
inventory block to SRAM (no P2 bytes, no P1 corruption; state consistent at save
time); file-select load re-seeds via `CopySaveToWRAM`→`OnSaveLoaded` (P2 respawns
fresh, the old inventory-wipe path stays sound); the scripted intro spawns P2 and
holds it through P1's control-locks (no soft-lock); map screens freeze P2 and
resume coherently (struct stays authoritative); mirror/whirlpool/bird/endgame are
module changes caught by `WarpP2OnTransition`; no flow leaves P2 permanently
inactive except the (now-fixed) game-over→continue case.

Code-verified; the game-over→continue revival is not reproducible in the
start-area headless harness (its death test only exercises a single-KO ghost).

## Review round 6 — progression / follower / dark-world / dungeon-camera

Audited progression-gating events, escort/follower quests, dark-world/bunny, and
deeper item-grab + dungeon-movement interactions. Root pattern (same as rounds
1/3/5): P2's `Link_Main` writing SHARED g_ram dungeon state the hazard-revert
doesn't undo.

**Fixed (HIGH):**
- **P2 crossing an intra-room quadrant boundary INDOORS corrupted the shared
  camera/scroll + saved quadrant flags.** `ApplyLinksMovementToCamera` was the
  unguarded sibling of the already-P2-guarded `HandleDoorTransitions`
  (`HandleIndoorCameraAndDoors` calls one or the other). When P2 crossed a
  quadrant boundary its pass rewrote shared `room_bounds`/`composite_of_layout_
  and_quadrant`/BG scroll and OR'd `dung_quadrants_visited` + `save_dung_info`
  from P2's position — shoving P1's camera and polluting the room's save flags.
  Fix (additive, mirrors the door guard): early-return for P2. P2 stays on screen
  via the leash and uses P1's (correct, leashed) shared quadrant context for its
  own collision, so it never needs to drive the camera. P1 path byte-identical.

**Documented / deferred (unverifiable headlessly; intricate per-player vs shared
split where a blind guard could strand P2 on the wrong layer/room):**
- **P2 hitting a layer-change / in-room-staircase / hookshot-onto-staircase tile**
  rewrites the SHARED `dungeon_room_index` (`+= 16` / `+= 0x10` / `= travel_dest`)
  in `Dungeon_HandleLayerChange` (player.c:89) and player.c:3204-3211 / 5388-5396.
  Conditional (only on those specific tiles) and the leash keeps P2 near P1, so
  Medium; the correct fix must keep P2's per-player `is_on_lower_level` while
  skipping the shared room-id/save writes — needs in-dungeon playtesting.

**LOW affordance gaps (P1-driven by design; co-op still completable — P1 performs
them, P2 can't trigger but also can't corrupt/double-fire):** Master Sword pull,
King Zora flippers, Zelda-rescue escort start, whirlpool/bird travel — all gated
on P1 proximity/buttons in P1's sprite/event pass.

**Verified CLEAN:** dark-world/bunny transform (all per-player fields; shared
moon pearl consistent); follower/escort delivery (followers follow P1, so P1 is
always the one at the destination; P2 can't break it); hookshot pull (owner-
tagged, pulls P2 to P2's anchor); magic powder / cape / bombable floors work for
P2; chests open + single-award for P2; boss/maiden/Agahnim rewards single-fire
into shared inventory. No new desync vectors.

Code-verified; the dungeon-camera and layer-change paths need a real dungeon
(not the start-area harness) to exercise at runtime.

## Review round 7 — systematic sweep: P2 writes to shared dungeon state

A full enumeration of the recurring class (P2's `Link_Main` call-tree writing
SHARED g_ram state the hazard-revert can't undo), tracing every `kPlayerHandlers`
entry + movement/collision + item/A-press path.

**CLASS CLOSED for camera/quadrant/dungeon-save:** the only P2-reachable entries
to the camera/quadrant/visited-flag/room-bounds writes are `HandleDoorTransitions`
and `ApplyLinksMovementToCamera`, both now P2-guarded — verified end-to-end. No
other path reaches `SetAndSaveVisitedQuadrantFlags`/`Dungeon_AdjustQuadrant`/
room-bounds for P2.

**Fixed (additive, matches design):**
- **P2 casting the Magic Mirror.** `LinkItem_Mirror` wrote shared world-warp
  state (`last_light_vs_dark_world`, `bird_travel_*`, `Mirror_SaveRoomData`,
  `index_of_changable_dungeon_objs`) from P2's pass — the warp submodule is
  reverted but those shared writes are not. Mirror is P1-led (Phase 5.5), so P2
  now early-returns from `LinkItem_Mirror`; it follows P1's warp via
  `WarpP2OnTransition`. Purely additive; P1 byte-identical (CRC unchanged).

**Confirmed WORKING-AS-INTENDED (the shared write is the CORRECT shared event —
NOT bugs; guarding them would regress shipped features):** P2 opening chests
(sets the shared chest-opened bit once, item to shared pool, tile redrawn — no
double-collect, verified round 4); P2 lifting/throwing pots/bushes (PR #10, the
shared tile-replacement is the legitimate lift); P2 pushing dungeon blocks
(round 3; single-pusher; the simultaneous-two-block edge stays the documented
minor); P2 collecting indoor floor-rupee tiles (single-collect, tile consumed
for the team). P2 reading signs is neutralized by the revert (benign no-op).

**Still DEFERRED (intricate per-player/shared split or ancilla-class; need
in-dungeon playtesting):** layer-change / in-room-staircase / hookshot-staircase
`dungeon_room_index` writes; hookshot-pulled gravestone overworld reveal (ancilla
class). Transient scratch (`allow_scroll_z`, `room_transitioning_flags`, etc.)
is the same low-risk category as the accepted `flag_unk1` — at most a 1-frame
cosmetic scroll perturbation; left as-is.

Static trace (reachability + write targets verified from source); the dungeon
paths aren't reachable by the start-area harness.

## Review round 8 — full-stack review: online transport, frontend commands, savestates

A fresh end-to-end review of the whole co-op surface (diff vs vanilla master),
focusing on the layers earlier rounds touched least: the UDP transport's
protocol-level behavior over time, the frontend's out-of-band commands, and
the savestate/reset lifecycle. The engine-side diffs (sprite/ancilla/player/
dungeon/HUD/OAM and the mp_dual duplication blocks) were re-audited and came
back clean: cur_player save/restore discipline, owner-table lifecycle,
RNG-free duplicated damage blocks, and the vanilla-neutral prototype fixes
all hold.

**Fixed (CRITICAL — online was effectively unshippable):**
- **The documented online command line never launched.** Vanilla treats the
  first positional arg as a ROM path for the verification emulator, so
  `zelda3_coop --host 7777` died at startup with "Failed to read file"
  (`LoadRom("--host")`). The co-op flags and their values are now skipped
  when looking for a ROM arg (multiplayer build only).
- **Input delivery could hang the session permanently.** v1 resent only the
  last 8 frames, and only while capturing: a client joining >~130ms after
  host launch could never receive frame 0; an 8-packet loss burst was
  unrecoverable; a paused/stalled peer stopped retransmission entirely (and
  the 10s liveness timeout stopped counting with it). Protocol v2 makes the
  input channel ack-based — packets carry "next frame I need", the sender
  resends from that ack (512-frame history, ≤32/packet), and a per-tick
  transport `poll()` retransmits/keeps alive independent of capture. A new
  `ready()` gate holds BOTH sims at frame 0 until the session is established,
  so launch order no longer matters.
- **Different save files on the two machines = guaranteed desync.** Each
  machine loaded its own `saves/sram.dat` the moment a file was selected. The
  host now streams its 8KB SRAM to the client at connect (chunked,
  re-requested until complete, gated by `ready()`); all later SRAM mutation
  is deterministic sim code, so the one sync covers the session. The client
  never persists the host's SRAM (a guest's local saves can't be clobbered).
- **Savestate load / reset corrupted co-op state (local AND online).**
  `LoadSnesState` and `ZeldaReset` restored/wiped g_ram but left the
  still-"seeded" P1 struct authoritative, so the next frame SyncToRam stomped
  the loaded state (load silently undone, P2 kept stale state). Both now
  route through `Multiplayer_OnSaveLoaded()` (re-seed P1 from the restored
  RAM, respawn P2) — same proven path as file-select loads. New headless
  `ZELDA3_TEST_SAVESTATE` regression test: save, walk 57px, load, verify
  position snaps back (drift ≤2px).
- **Online out-of-band desync hatches closed.** Savestate-load, replay,
  reset, and RAM-patching cheat hotkeys are ignored while online (each was an
  instant-desync button on one machine); state SAVING stays allowed.

**Fixed (HIGH/MEDIUM):**
- **Handshake deadlock:** the single HELLO reply could be lost, leaving one
  peer "connecting…" forever (it stopped answering once handshaked). v2
  HELLOs carry a `got_yours` flag; any HELLO with the flag clear is answered,
  and flag-set HELLOs terminate the exchange (no reply storm).
- **Host peer-slot hijack:** the host locked onto the first well-formed
  datagram from ANYONE — a single stray BYE byte from a port scan could claim
  the slot (and immediately flag the peer lost), shutting out the real
  client. The slot now locks only on a version-matching HELLO.
- **Controller double-tracking:** SDL fires CONTROLLERDEVICEADDED for pads
  already present at startup on top of the manual open loop, so every initial
  pad occupied two slots — with two pads, the second one never mapped to
  Player 2 (both pads drove P1). Pads are now de-duped by joystick instance
  id; a pad beyond the table is closed instead of leaked.
- **Turbo online** raced input capture a full ring (256 frames) ahead of the
  lockstep sim, permanently pinning ~4s of input latency. Turbo is ignored
  online; in local co-op it's unchanged.
- **Unbounded catch-up burst:** after a long stall healed, the driver simmed
  the whole backlog (up to 256 frames) inside one display tick — a multi-
  second UI freeze with garbled audio. Capped at 8 frames/tick.

**Verified (no change needed):** lockstep driver framing/dedup ordering (rings
are strictly sequential, Peek/Pop heads agree); `Multiplayer_ProcessP2Input`
exactly mirrors nmi.c's bit-reversal + opposing-direction filter; the macro
redirect layer's undef/define sets are complete and consistent; checksum
covers g_ram + both PlayerState structs + owner tables; in-game SRAM writes
are all deterministic sim code (the initial-disk-read divergence was the only
gap); exit-time autosave and state-saves online are read-only-safe.

**Test coverage added this round** (all PASS, plus the full prior battery; the
sim CRC for every pre-existing scenario is byte-identical to before —
`0f79cbba` for the standard run): UDP late-join/burst-loss recovery, HELLO-
only peer lock, SRAM sync + ready() gating, savestate re-seed, and a real
two-process `--host`/`--connect` localhost session (client joining 6s late:
handshake, save transfer, lockstep established, no desync).

**Still deferred, unchanged (documented gameplay affordance gaps, not
correctness):** P2 pulling levers / opening locked doors / A-press NPC
interactions (P1 performs them; leash keeps players together); both-players-
swimming stroke scratch sharing (deterministic, cosmetic; `swimcoll_*` is
also touched from dungeon.c, so a blind per-player split risks breaking
swimming for everyone); HUD heart-row overlap past 10 hearts (cosmetic);
sprite-based liftables P1-only; P2 item selection follows P1 (by design).
