# Phase 5 — Polish & Edge Cases (Co-Op)

Audit of the seven Phase 5 items from `CLAUDE.md`. Each is marked **fixed**
(code change this pass), **already works** (the architecture already handles it),
or **by spec** (current behavior is what the design calls for). All changes are
guarded by `#ifdef ZELDA3_MULTIPLAYER`; the vanilla build is byte-for-byte
unchanged and both `zelda3` / `zelda3_coop` build clean with `-O2 -Werror`.
Determinism verified: the headless harness produces an identical `g_ram` CRC
across repeated runs (required for future online lockstep).

---

## 1. Cutscenes / scripted control lock — ✅ FIXED

**Problem.** `flag_is_link_immobilized` (RAM 0x2E4) is the engine's authoritative
"Link can't move this frame" gate — `Link_Main` skips `Link_ControlHandler` when
it is set (`player.c`). It is now a *per-player* field. P1's scripted sequences
that stay inside normal gameplay (the item-get overhead pose, Ether/Bombos/Quake
spell animations, tree pull, scripted dungeon events) set only **P1's** copy, so
P2 would keep walking and acting through the cutscene.

**Fix (`zelda_rtl.c`, `ZeldaRunGameLoop_Multiplayer`).** While `cur_player` is
still P1, capture `flag_is_link_immobilized` into `p1_immobilized`; in the P2
branch, feed P2 **zero input** (`p2_eff_input = p1_immobilized ? 0 : p2_input`)
so it can't walk or act while P1 is locked. P2's own item-gets still freeze P2
via its own per-player flag. Module-level cutscenes, the menu/map and text boxes
run in module 14 (or submodule != 0) and already freeze P2 via the
`(module==7||9) && submodule==0` gate, so this only needed to cover the in-module
case.

## 2. Boss fights / enemy targeting nearest player — ✅ FIXED

**Problem.** Sprite AI reads the player position through `link_x/y` = P1 during
the sprite pass, so every enemy and boss pathed/aimed at **P1 only**; P2 was
never targeted.

**Fix (`sprite.c`).** Enemy pathing and facing funnel through two leaf helpers,
`Sprite_IsRightOfLink` / `Sprite_IsBelowLink` (everything else —
`Sprite_ProjectSpeedTowardsLink`, `Sprite_ApplySpeedTowardsLink`,
`Sprite_DirectionToFaceLink` — is built on them). Both now resolve the target to
**whichever player is nearer the sprite** (`Sprite_NearestIsP2` + Manhattan
distance), falling back to P1 when P2 is inactive or downed.

This is **pathing/aim only**. Damage is hit-tested per player on a separate path
(the P2 loop re-runs `Sprite_CheckDamageTo/FromLink`, which use
`Link_SetupHitBox` + `CheckIfHitBoxesOverlap`, **not** these helpers), so
retargeting can never mis-apply damage. A blanket `cur_player` swap around whole
AI handlers was rejected as unsafe: some handlers *write* `link_` fields (pull
switches, transport/mirror sprites) and others apply damage inside the handler,
both of which a global swap would corrupt. Redirecting only the leaf
read-helpers is safe and contained.

*Coverage note:* enemies that read `link_x/y` directly (rather than via the
helpers) still target P1. That is a deliberately conservative, zero-risk
boundary; the major chasers and bosses use the centralized helpers.

## 3. Pits / holes — ✅ ALREADY WORKS

`LinkState_Pits` (`player.c`) is driven entirely by `TileDetect_MainHandler` and
`link_*` state, all per-player via `cur_player`. Each player detects and falls
into pits independently. No change needed.

## 4. Swimming — ✅ ALREADY WORKS

`PlayerHandler_04_Swimming` / `Link_HandleSwimMovements` (`player.c`) read
per-player input, the per-player flippers item, and per-player swim state. Each
player swims independently. No change needed.

## 5. Mirror warp (and other warps) — ✅ ALREADY WORKS

- **Magic Mirror** switches `main_module_index` to 15 (`Module15_MirrorWarpFromAga`)
  and back. The existing `Multiplayer_WarpP2OnTransition` detector fires on any
  `main_module_index` change, so P2 is re-placed beside P1 after the warp.
- **Whirlpool** stays in module 9 but drives `submodule_index` to 0x2E and back
  to 0; the detector's "submodule returned to 0 from non-zero in module 7/9"
  branch catches it.
- **Dungeon ↔ overworld** transitions change the module and are caught the same
  way. No change needed.

## 6. Tagalongs (followers) — ✅ BY SPEC

`Follower_Main` (`tagalong.c`) follows `link_x/y` = P1. The design (`CLAUDE.md`
Phase 5.6: "Followers (Zelda, old man) follow P1 only") calls for exactly this,
so the current behavior is correct. No change needed.

## 7. Menu / inventory / pause — ✅ BY SPEC / ALREADY WORKS

Opening the inventory, world map, or a text box switches to module 14, which
fails the P2 gate, so **P2 is frozen for the whole party** while the menu is up —
pause pauses for both. Item selection is P1-only, which the spec explicitly
allows ("Item selection is P1 only … or add P2 item selection"). No change
needed for the baseline.

---

## Verification

| Build | Result |
|---|---|
| `zelda3` (vanilla, CI target) | clean, unchanged |
| `zelda3_coop` (`-DZELDA3_MULTIPLAYER=1`) | clean |
| `zelda3_harness` | clean |
| Determinism (CRC across repeated runs) | identical (co-op and P2-disabled) |
| Co-op smoke (360 scripted frames) | both players alive, independent positions |

The co-op CRC changes versus the pre-Phase-5 baseline because enemy AI now
factors in P2's position — the intended behavior change — while remaining
perfectly reproducible run-to-run.
