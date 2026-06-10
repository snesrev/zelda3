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
their own Link. Launch order doesn't matter — the host can idle on the title
screen until the client connects; the game holds at the start until both peers
are linked and the client has received the host's save data, then both run.
The host's save files are the session's saves (the client's own local saves are
neither used nor touched). The host must be reachable on the UDP port (LAN, or
port-forward for WAN). **Windows note:** the TCC build needs `-lws2_32` added
to link Winsock (POSIX/macOS need no extra lib).

### Test the foundation headlessly
```
./zelda3_harness                       # direct path, prints final CRC
ZELDA3_TEST_NET=1 ./zelda3_harness     # same sim via the lockstep pipeline → same CRC
ZELDA3_TEST_NET=1 ZELDA3_NET_DELAY=2 ./zelda3_harness   # buffered, still deterministic
ZELDA3_TEST_UDP=1 ./zelda3_harness     # UDP host<->client round-trip over localhost
```

## Robustness & status (implemented — protocol v2)

The UDP protocol prefixes each datagram with a type byte and carries more
than just input:
- **Ack-based input delivery (`INPUT`, v2)** — every INPUT packet carries the
  sender's "next frame of yours I still need" (an ack). Each peer retransmits
  its own input from the peer's last ack forward (a sliding window over a
  512-frame history ring, ≤32 frames per packet), and the per-tick transport
  `poll()` keeps retransmitting **even while input capture is held** (full
  ring / stalled sim) — exactly when healing is needed. This makes delivery
  self-healing under arbitrary burst loss and lets a client that connects
  late still receive everything from frame 0. (v1 resent only the last 8
  frames and only while capturing: >8 lost packets, a paused-peer stall, or a
  client joining >130ms after host launch hung the session forever.)
- **Session start gate (`ready`)** — the lockstep driver holds input capture
  until the transport reports the session established (handshake done +
  save data synced), so BOTH sims begin together at frame 0. The host can sit
  on the title screen for minutes before the client connects.
- **Save-data sync (`SRAM_REQ`/`SRAM`)** — both sims must load saves from
  identical SRAM, but each machine boots with its own `saves/sram.dat`. At
  connect the host streams its 8KB SRAM to the client (16×512B chunks; the
  client re-requests until complete, so chunk loss is harmless). All later
  in-game SRAM mutations are deterministic sim code, so one initial sync
  keeps the whole save lifecycle identical on both peers. The client does
  NOT persist the host's SRAM to disk (your local saves are never
  overwritten by joining a friend's game).
- **Handshake (`HELLO`)** — peers exchange protocol version + `input_delay`; a
  version mismatch is flagged (and shown) instead of silently desyncing. v2
  HELLOs carry a `got_yours` flag and a peer answers any HELLO whose flag is
  0 — a lost reply can no longer deadlock the handshake, and replies
  terminate without a storm.
- **Desync detection (`SYNC`)** — the lockstep driver hands the transport a full
  state checksum every second; peers exchange them and flag a divergence with
  the frame number. (This is the determinism invariant, now checked live.)
- **Disconnect (`BYE`)** — sent on quit; the peer also flags `peer_lost` after
  ~10s of silence (timeout counted per tick in `poll()`, so it fires even
  while capture is held).
- **On-screen status** — the window title shows `connecting…`,
  `receiving save data…`, `waiting for player…` (the lockstep stalled awaiting
  the peer), `player disconnected`, `DESYNC DETECTED`, or `INCOMPATIBLE
  VERSION`; session milestones are also printed to the console once each.
- **Packet hardening** — UDP accepts bytes from anyone, so the receive path is
  strict: every datagram is type/length-validated before any field is read
  (`Udp_PacketWellFormed`), over-length reads are clamped, the host's peer
  slot can only be claimed by a **version-matching HELLO** (a stray scan
  datagram can no longer hijack the slot or BYE-kill the session before it
  starts), and once the peer's address is known, datagrams from any other
  source are dropped (`Udp_AddrMatches`). (Defeating a *forged* source
  address needs crypto and is out of scope.)
- **Out-of-band commands disabled** — loading savestates/replays, reset, and
  RAM-patching cheats are ignored while online (any of them would desync the
  peers instantly); saving a state stays allowed (read-only). Turbo is also
  ignored online — the sim can't outrun the peer, so turbo would only race
  input capture a full ring ahead and pin ~4s of input latency.
- **Catch-up cap** — after a stall heals, the sim fast-forwards at most 8
  frames per display tick (a worst-case 256-frame backlog clears in under a
  second) instead of bursting seconds of simulation in one frozen tick.

Verified headlessly (`ZELDA3_TEST_UDP=1 ./zelda3_harness`): input round-trip,
**handshake**, **desync detection**, **clean disconnect**, **packet hardening**
(wrong-source frame + malformed datagrams dropped), **ring capacity**,
**late-join/burst-loss recovery** (40 frames delivered through the sliding ack
window), **HELLO-only peer lock**, and **SRAM sync** (8KB transferred, ready()
gates until complete) all PASS.

Verified end-to-end (two real processes over localhost UDP, full `--host` /
`--connect` CLI path): the host ran alone for 6 seconds, the client joined
late, handshook, received the host's save data, and both sims established
lockstep ("session established - simulation running in lockstep" on both)
with no desync.

## What remains (real-machine validation + polish)

1. **Two-real-machine playtest** over LAN/WAN (the sandbox is single-machine,
   so only localhost is exercisable here). **See `ONLINE_PLAYTEST.md`** for the
   step-by-step session guide: connection paths (LAN / relay+join-code / VPN /
   port-forward), `--net-delay` tuning, status-indicator meanings, a test
   checklist, and exactly what to capture if a desync fires.
2. **Recovery polish:** the desync flag currently warns; auto-resync (state
   transfer) and a graceful "pause + reconnect" on timeout are future niceties.
3. **WAN convenience, further:** NAT hole-punching (direct P2P, lower latency
   than relay) and a hosted default relay / lobby "browse games" layer would
   remove the one remaining manual step (someone running `tools/relay.py`).

None of these touch the simulation — they only feed it inputs.

## WAN relay + join codes (implemented)

For internet play without a port-forward or VPN, both peers send OUTBOUND to a
small relay (NAT-friendly) that pairs them by a room id and forwards their
datagrams. The relay is **transparent** to everything above — it only strips a
4-byte room prefix and forwards the inner payload, so the handshake, lockstep,
SRAM sync and desync detection all run end-to-end unchanged.

- **Reference relay:** `tools/relay.py` — a dependency-free UDP forwarder
  (`python3 tools/relay.py [port]`) you run on any reachable host. It keeps the
  two most-recent addresses per room and idles rooms out; no game state ever
  touches it.
- **Join code:** the host (`--host --relay <host:port>`) prints a 16-char
  Crockford-base32 code (shown `XXXX-XXXX-XXXX-XXXX`) encoding the relay's IPv4
  + port + a random room; the friend runs `--join <code>` and needs nothing
  else. Parsing ignores case, dashes and spaces.
- **Game side:** `Udp_InitRelay()` (no bind; peer_addr = the relay) and the
  room-prefix framing in `Udp_RawSendTo`; `Net_MakeJoinCode`/`Net_ParseJoinCode`
  for the code. All in `src/net_udp.c`.
- **Verified headlessly** (`ZELDA3_TEST_RELAY=1 ./zelda3_harness`): join-code
  encode/decode round-trip (+ case/dash tolerance, rejects malformed), and a
  full session — handshake, input both ways, and 8KB SRAM sync — through an
  in-process relay forwarder (the same algorithm as `relay.py`). Also exercised
  live: `relay.py` + two real `--host`/`--join` processes reach
  "session established" with the save transferred.

## Why lockstep (not rollback)

Lockstep matches this engine's design, is simple, and is ideal for co-op PvE.
Rollback (GGPO-style) feels better under high latency but needs full per-frame
state snapshot/restore (only stubbed here) and prediction/re-simulation — a much
larger, riskier change. Start with lockstep; rollback can layer on later using
the same deterministic core + the `GameState_Snapshot/Restore` stubs.
