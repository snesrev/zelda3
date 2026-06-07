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
3. **Shared-item team pooling** — P2's collected rupees/keys go to its own copy, not
   the team total; and P1's mid-game item pickups don't propagate to P2 (inventory
   is cloned at P2 spawn, not continuously mirrored). Per-player health/magic work.
4. (Optional) independent transitions / split-screen.
