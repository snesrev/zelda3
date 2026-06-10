<!--
  Ready-to-paste kickoff brief for the Ocarina of Time co-op project.
  Usage: start a new session/project in the OoT repo (or an empty repo that
  will host it) and paste everything below the line as the opening prompt /
  CLAUDE.md. Bring tools/netcode-kit/ from zelda3-coop along (or keep that
  repo readable) so the referenced code and docs are at hand.
-->

---

# Project: Two-Player Co-Op for Ocarina of Time (local + online)

## Mission
Add **two-player co-op** to Ocarina of Time, architected for **online play over the
internet**, mirroring what was achieved for "A Link to the Past" in the
`zelda3-coop` project: local co-op, then online with relay + shareable join codes
+ automatic NAT hole-punching. Deliver a working, playable co-op build.

You are an autonomous engineer. Work in small, verified, CI-green increments.
Do NOT break the base game's single-player build, ever.

## Reuse this proven networking stack (it is game-agnostic — copy it)
The `zelda3-coop` repo contains a complete, hardened online layer that was
deliberately built as a transport seam. A self-contained snapshot of all of it —
code, reference relay, docs, and porting notes — lives in
**`tools/netcode-kit/`** of that repo (start with its `README.md`). PORT IT,
don't reinvent it:
- `net_transport.h` — `NetTransport` vtable (send/recv/poll/ready/send_sync)
  + an in-process loopback transport for headless testing.
- `net_udp.{c,h}` — UDP transport, protocol v2: ack-based input delivery with
  a retransmit window over a history ring, HELLO version handshake, periodic
  state-checksum exchange (desync detection), clean BYE + liveness timeout,
  strict packet validation (well-formed gate, source-address lock, length clamp),
  8KB save (SRAM) streaming at connect, **WAN relay + Crockford-base32 join
  codes**, and **automatic NAT hole-punching** (relay rendezvous → direct P2P,
  with automatic relay fallback).
- `relay.py` — dependency-free reference UDP relay (room pairing, forward,
  NAT-punch rendezvous). Runs on any VPS/PC.
- `--online` interactive launcher menu + window-title status (`direct P2P` /
  `via relay` / join code / `DESYNC` / `connecting`).
- The headless test harness pattern (`ZELDA3_TEST_*` env-gated scenarios that
  drive the sim with scripted inputs and compare a state CRC).
Read that repo's `NET_ONLINE.md`, `ONLINE_PLAYTEST.md`, and `COOP_REVIEW.md` —
they are the design record and the bug catalogue.

The UDP transport, relay, join codes, and hole-punching are reusable WHATEVER
netcode model you choose. Only the "exchange inputs + CRC the whole RAM" part is
lockstep-specific.

## CRITICAL architectural reality — OoT is NOT zelda3 (read twice)
zelda3 co-op worked via three things that DO NOT EXIST in OoT:
  (1) a flat `uint8 g_ram[128KB]` with every variable a `#define` into it →
      enabling a "redirect link_* macros to cur_player->field" trick and a
      whole-RAM CRC for determinism;
  (2) integer-only 2D simulation (no floats in game logic);
  (3) a single-threaded fixed 60Hz tick with a faithful integer RNG.
OoT (3D, float-heavy 3D math, an actor/object system with pointers, a heap/arena
allocator, real structs, threads in the PC port, and PC ports that add
high-FPS interpolation) has NONE of these. The macro-redirect/flat-RAM/whole-RAM-
CRC approach is impossible here, and **bit-exact input-lockstep across machines
is a hard, possibly infeasible problem** (float determinism across
compilers/CPUs, RNG, actor allocation/iteration order, threading, frame pacing).

### Phase 0 (MANDATORY, do this BEFORE committing to any netcode model):
1. **Pick the base.** Recommended: **Ship of Harkinian (SoH)** — the actively
   maintained cross-platform PC port of the `zeldaret/oot` decompilation (C/C++,
   libultraship, extracts assets from a legit ROM into an OTR archive, has
   modding hooks e.g. GameInteractor). Verify the current repo, license, and
   build instructions yourself. The raw `zeldaret/oot` decomp is the reference
   for game logic but targets N64 hardware and is a poor base for PC netcode.
2. **Study prior art.** OoT co-op already exists — e.g. "2 Ship 2 Harkinian"
   (SoH co-op fork) and older OoT-Online efforts. Read how they handle two
   players, scenes, save state, and netcode. Strongly consider building ON an
   existing co-op fork rather than from scratch; decide and justify.
3. **Run a determinism spike.** Get the base building headless, then run two
   instances (or one instance twice) from the same save with the same scripted
   input stream and hash the game state each frame. Measure whether it stays
   identical: same-binary first, then across OS/CPU if you can. REPORT the
   result. This decides everything below.
4. **Choose the netcode model and write it up** before implementing:
   - **(A) Input-lockstep** (like zelda3): only if the spike shows you can make
     the sim deterministic (likely requires: same binary, disable fast-math/FMA,
     fixed timestep, DISABLE SoH interpolation/high-FPS during play, single-
     threaded sim, controlled RNG, no pointer-value/time/uninitialised reads in
     sim). Lowest bandwidth, but brittle in 3D float code.
   - **(B) Host-authoritative state-sync** (what most 3D co-op mods use, and the
     realistic default if the spike diverges): host simulates and is the source
     of truth; clients send inputs, host broadcasts actor/state snapshots,
     clients interpolate; tolerant of non-determinism. Reuse the same UDP
     transport/relay/punch, but replace the desync-CRC with state broadcast +
     interest management + reconciliation.
   Default to (B) unless the spike convincingly proves (A) is achievable.

## OoT-specific design you must solve (independent of netcode model)
- **A real second player**, not a macro alias: a second `Player` actor (and its
  camera, Z-target, sword/shield/items, C-button items). Study how the engine
  assumes exactly one Player (GET_PLAYER, gPlayState->actorCtx, the player actor
  category) and what breaks with two.
- **Cameras / view**: 3D can't "leash to one screen" like 2D. Decide split-screen
  (two viewports/cameras — heavier render changes in Fast3D) vs. shared-camera
  vs. networked-separate-views. This is a major rendering decision.
- **Scenes & rooms**: scene/room load + transitions with two players (same room
  required? warp P2 to P1 on transition, like zelda3 did? or independent rooms
  for a future split path?). Cutscenes lock control — lock both.
- **Save context / progression**: OoT's `SaveContext` is one global save. Shared
  inventory/quest progress (like zelda3) but per-player health/magic/position.
  Sync the save at connect (we already stream SRAM; OoT's save is bigger/
  different — adapt). Beware: lots of game logic reads/writes `gSaveContext`
  directly — the "two players writing shared state" bug class will be everywhere.
- **Z-targeting, actor interaction, enemy aggro** per player; enemies must see/
  damage both; no friendly fire (co-op philosophy).
- **Items/ammo/rupees shared; hearts/magic per-player; death + revive.**
- **SoH specifics**: the OTR/resource system, GameInteractor/mod hooks, the
  CVar/config system, controller handling (it already supports multiple pads),
  and DISABLING interpolation/high-FPS for the sim if you go lockstep.

## Process lessons from zelda3-coop (NON-NEGOTIABLE — these are why it worked)
1. **Guard ALL co-op code behind a build flag** (e.g. `-DOOT_COOP=1`) so the base
   single-player build is byte-for-byte unaffected and always compiles. Verify
   BOTH builds after every change.
2. **Headless deterministic test harness first.** Build an env-gated, no-window
   mode that loads a save, drives scripted inputs, and hashes state — this is how
   you verify co-op/determinism without a human at a screen. Add a regression
   assertion for every fix. (In zelda3 this caught real bugs CI otherwise couldn't.)
3. **Adversarial review in rounds.** After features land, run multiple focused
   read-only review passes (determinism, combat, world-flow, save/transitions,
   the netcode itself — and a review OF the netcode review). They keep finding
   real bugs; budget for ~8+ rounds.
4. **Ship small, frequent, CI-green PRs** (draft → CI green → merge). Never batch.
5. **The dominant co-op bug class**: "player 2's update writes shared global state
   the host/other pass doesn't expect, and nothing reverts it." In OoT this is
   `gSaveContext`, scene flags, actor lists, the RNG. Prefer **additive guards**
   (only-run-for-the-owner) that leave the single-player path identical.
6. **Online false-signal pitfalls we already hit and fixed — pre-empt them:**
   saving state must not mutate checksummed sim memory (false desync); don't run
   the liveness/disconnect timeout before the handshake (false "disconnected");
   bound input ring buffers (a stalled/paused peer must not overflow them); keep
   transport upkeep running while paused; surface version mismatch; the client
   must not persist the host's save over its own.
7. **Determinism is sacred (model A) / the host is the single source of truth
   (model B).** Pick one and never violate it.
8. **Sync starting state at connect** (identical save) and gate the sim start
   until both peers are ready.
9. **Co-op design**: no friendly fire; shared progression; revive a downed
   partner; both players locked during cutscenes/menus.
10. **Environment resilience**: builds may run in ephemeral containers — script
    dependency install + asset extraction from the ROM, and commit/push often
    (a clean tree is the only thing that survives a reset).

## Phasing (adapt after Phase 0's decision)
- P0: base build + prior-art study + determinism spike + netcode decision (write-up).
- P1: port the networking stack (transport/relay/join-code/punch) and a headless
  loopback test green, decoupled from game logic.
- P2: a controllable second Player actor on screen (local), with its own input.
- P3: cameras/view for two players; scene/room/transition handling.
- P4: shared save/progression + per-player health/death/revive; combat vs both.
- P5: wire the chosen netcode to the game (lockstep tick OR host state-sync);
  save-sync at connect; desync/repair handling.
- P6: online over relay + join code + hole-punch; status UX; playtest guide.
- P7: adversarial review rounds + polish.

## Definition of done
- Single-player base build unchanged and always green.
- Local two-player co-op playable start-to-finish.
- Online co-op over the internet via a join code (relay + automatic hole-punch),
  with desync/disconnect handling and on-screen status.
- Determinism (model A) or host-authority (model B) preserved and tested.
- Docs: an architecture decision record (lockstep vs state-sync + why), a
  NET/ONLINE doc, a playtest guide, and a running review log.

## First actions
1. Stand up the SoH base build; confirm it runs and extracts assets from a ROM.
2. Read the `zelda3-coop` netcode + its three markdown docs (all included in
   `tools/netcode-kit/`).
3. Survey existing OoT co-op mods; write a 1-page comparison + recommendation.
4. Run the determinism spike; report numbers.
5. Produce the netcode-model decision record. THEN start P1.
Do not skip Phase 0. Picking the wrong netcode model is the only mistake that
can't be incrementally fixed.
