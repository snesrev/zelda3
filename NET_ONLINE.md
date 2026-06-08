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
| **UDP transport** (real sockets, loss-tolerant redundancy + de-dup) | `net_udp.c` | done |
| **Lockstep driver** `Multiplayer_LockstepInit` / `Multiplayer_LockstepTick` | `zelda_rtl.c` | done |
| Dual-player loop runs for HOST/CLIENT (not just LOCAL) | `zelda_rtl.c` | done |
| **Real-time loop wiring** + `--host`/`--connect`/`--net-delay` CLI | `main.c` | done |
| Desync checksum covers `g_ram` + `g_players` + ancilla owner | `player_state.c` | done |
| Headless verification (`ZELDA3_TEST_NET`, `ZELDA3_TEST_UDP`) | `main.c` | done |

**Verified** (headless harness):
- **Pipeline-neutral:** driving the sim through the full lockstep pipeline
  (ring → transport serialize/deserialize → consume) at `input_delay=0` produces
  a **byte-identical** `g_ram`+`g_players` CRC to the direct path for the same
  inputs.
- **Deterministic with delay:** `input_delay=2` and `=4` are identical
  run-to-run — the input buffer never corrupts state.
- **UDP wire works:** a host + client on localhost round-trip InputFrames over
  real sockets (incl. redundant-resend de-dup and host learning the client's
  address) — `ZELDA3_TEST_UDP=1 ./zelda3_harness` → PASS.

Because two online peers both run the lockstep driver from the identical input
pair, this is the proof they will stay in sync; the periodic checksum
(`Multiplayer_ComputeChecksum`) detects any divergence.

### Play online
```
# Machine A (host, becomes Player 1):
zelda3_coop --host 7777            (or any port; default 7777)
# Machine B (client, becomes Player 2):
zelda3_coop --connect <A's IP> 7777
# Optional latency buffer (default 2): --net-delay 3
```
Each player uses their normal controls (arrows + ZXASCV / a controller) to drive
their own Link. The host must be reachable on the UDP port (LAN, or port-forward
for WAN). **Windows note:** the TCC build needs `-lws2_32` added to link Winsock
(POSIX/macOS need no extra lib).

### Test the foundation headlessly
```
./zelda3_harness                       # direct path, prints final CRC
ZELDA3_TEST_NET=1 ./zelda3_harness     # same sim via the lockstep pipeline → same CRC
ZELDA3_TEST_NET=1 ZELDA3_NET_DELAY=2 ./zelda3_harness   # buffered, still deterministic
ZELDA3_TEST_UDP=1 ./zelda3_harness     # UDP host<->client round-trip over localhost
```

## Robustness & status (implemented)

The UDP protocol now prefixes each datagram with a type byte and carries more
than just input:
- **Handshake (`HELLO`)** — peers exchange protocol version + `input_delay`; a
  version mismatch is flagged (and shown) instead of silently desyncing. A
  received HELLO is answered once so both sides complete the handshake.
- **Desync detection (`SYNC`)** — the lockstep driver hands the transport a full
  state checksum every second; peers exchange them and flag a divergence with
  the frame number. (This is the determinism invariant, now checked live.)
- **Disconnect (`BYE`)** — sent on quit; the peer also flags `peer_lost` after
  ~10s of silence (timeout).
- **On-screen status** — the window title shows `connecting…`,
  `waiting for player…` (the lockstep stalled awaiting the peer),
  `player disconnected`, `DESYNC DETECTED`, or `INCOMPATIBLE VERSION`.

Verified headlessly over localhost (`ZELDA3_TEST_UDP=1 ./zelda3_harness`):
input round-trip, **handshake**, **desync detection**, and **clean disconnect**
all PASS.

## What remains (real-machine validation + polish)

1. **Two-real-machine playtest** over LAN/WAN (the sandbox is single-instance, so
   only localhost is exercisable here).
2. **NAT/WAN convenience:** port-forward today; a relay/hole-punch or a "code"
   matchmaking layer would make WAN connect-by-default.
3. **Recovery polish:** the desync flag currently warns; auto-resync (state
   transfer) and a graceful "pause + reconnect" on timeout are future niceties.

None of these touch the simulation — they only feed it inputs.

## Why lockstep (not rollback)

Lockstep matches this engine's design, is simple, and is ideal for co-op PvE.
Rollback (GGPO-style) feels better under high latency but needs full per-frame
state snapshot/restore (only stubbed here) and prediction/re-simulation — a much
larger, riskier change. Start with lockstep; rollback can layer on later using
the same deterministic core + the `GameState_Snapshot/Restore` stubs.
