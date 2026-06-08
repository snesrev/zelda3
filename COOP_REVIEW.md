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
