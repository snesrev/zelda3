# Online Multiplayer — Input-Lockstep Foundation

This documents the **online netcode foundation** that has been built and verified,
and exactly what remains to make two separate machines play together.

## Model: deterministic input-lockstep

The game simulation is 100% deterministic — given identical inputs each frame,
two machines produce byte-identical state (verified: the headless harness CRCs
`g_ram` **and** the per-player `PlayerState` structs and gets identical results
run-to-run across many input scenarios). So online does **not** send game state;
it sends only the tiny 8-byte `InputFrame` per player per frame, and both peers
run the same `ZeldaRunFrame(p1, p2)`.

```
   local input ─┐                        ┌─ local input
                ▼                        ▼
        ┌───────────────┐        ┌───────────────┐
        │  Peer A (host)│        │ Peer B (client)│
        │ ring[0]=local │◄─wire─►│ ring[1]=local  │
        │ ring[1]=remote│        │ ring[0]=remote │
        └──────┬────────┘        └──────┬─────────┘
               ▼ both have frame N?      ▼
        ZeldaRunFrame(p1,p2)      ZeldaRunFrame(p1,p2)   ← identical → in sync
```

`input_delay_frames` (2–4 online, 0 for loopback/LAN) keeps the sim that many
frames behind input capture so the remote frame has time to arrive.

## What's built and verified (this foundation)

| Piece | File | Status |
|---|---|---|
| `InputFrame` + 8-byte wire (de)serialize | `net_types.h` | pre-existing |
| Input ring buffer + `InputsReady`/`ConsumeInputs` | `player_state.c` | wired + frame-accurate |
| **Transport abstraction** (`NetTransport`) | `net_transport.h` | the seam for real UDP |
| **Loopback transport** (in-process, lossless, real wire format) | `net_transport.c` | done |
| **Lockstep driver** `Multiplayer_LockstepInit` / `Multiplayer_LockstepTick` | `zelda_rtl.c` | done |
| Dual-player loop runs for HOST/CLIENT (not just LOCAL) | `zelda_rtl.c` | fixed |
| Desync checksum covers `g_ram` + `g_players` + ancilla owner | `player_state.c` | done |
| Headless verification (`ZELDA3_TEST_NET`) | `main.c` | done |

**Verified** (headless harness):
- **Pipeline-neutral:** driving the sim through the full lockstep pipeline
  (ring → transport serialize/deserialize → consume) at `input_delay=0` produces
  a **byte-identical** `g_ram`+`g_players` CRC to the direct path for the same
  inputs.
- **Deterministic with delay:** `input_delay=2` and `=4` are identical
  run-to-run — the input buffer never corrupts state.

Because two online peers both run the lockstep driver from the identical input
pair, this is the proof they will stay in sync; the periodic checksum
(`Multiplayer_ComputeChecksum`) detects any divergence.

### Try it
```
./build.sh harness
./zelda3_harness                       # direct path, prints final CRC
ZELDA3_TEST_NET=1 ./zelda3_harness     # same sim via the lockstep pipeline → same CRC
ZELDA3_TEST_NET=1 ZELDA3_NET_DELAY=2 ./zelda3_harness   # buffered, still deterministic
```

## What remains for real two-machine online

Only the **transport + connection** — the lockstep core above is done and the
driver is transport-agnostic, so this is "implement one interface":

1. **UDP transport** implementing `NetTransport` (send/recv `InputFrame`): a
   `UdpTransport` next to the loopback one. Raw sockets behind `#ifdef _WIN32`
   (Winsock) / POSIX, or add SDL_net. Resend-last-input on packet loss (inputs
   are tiny and idempotent per frame).
2. **Connection setup:** host opens a port; client connects by `IP:port`
   (a simple connect screen / `zelda3.ini` entry). Exchange a handshake with the
   agreed `input_delay`, RNG/seed sanity, and feature flags.
3. **Real-time loop wiring** in `main.c`: in HOST/CLIENT mode call
   `Multiplayer_LockstepTick(local_joypad)` once per display frame instead of the
   direct `ZeldaRunFrame`; if it returns 0 (waiting on the peer) render the last
   frame and try again next tick (stall, don't advance).
4. **Failure handling:** timeout → pause/"waiting for player"; `INPUT_FLAG_DISCONNECT`
   to end cleanly; on a checksum mismatch, surface a desync error.

None of these touch the simulation — they only feed it inputs. The determinism
that makes them work is already proven.

## Why lockstep (not rollback)

Lockstep matches this engine's design, is simple, and is ideal for co-op PvE.
Rollback (GGPO-style) feels better under high latency but needs full per-frame
state snapshot/restore (only stubbed here) and prediction/re-simulation — a much
larger, riskier change. Start with lockstep; rollback can layer on later using
the same deterministic core + the `GameState_Snapshot/Restore` stubs.
