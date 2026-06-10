# Portable Co-op Netcode Kit

A self-contained snapshot of the **game-agnostic online layer** built for
`zelda3-coop`, packaged so it can be lifted into a new project (first target:
an Ocarina of Time co-op). Everything here was shipped, adversarially reviewed
across multiple rounds, and verified by headless tests plus live multi-process
sessions — read `COOP_REVIEW.md` for the full bug catalogue and why each
invariant exists.

`KICKOFF_PROMPT.md` is the ready-to-paste brief for starting the next project.

These are **frozen copies**; the canonical sources live in `src/` and `tools/`
of this repo. Re-snapshot with `./refresh.sh` after changing them.

## Inventory

| File | Role | Portability |
|---|---|---|
| `net_types.h` | `InputFrame` (8-byte wire format + serialize/deserialize), `SyncChecksum`, `MultiplayerMode`/`MultiplayerConfig`, CRC32 decl | Agnostic (config enum is mildly zelda-flavored; harmless) |
| `net_transport.h` | **The seam.** `NetTransport` vtable (`send`/`recv`/`poll`/`ready`/`send_sync`/`close`) + loopback transport + lockstep-driver declarations | Agnostic |
| `net_transport.c` | In-process loopback transport (lossless, real wire format) — the reference a real transport must behave like, and the headless-test workhorse | Agnostic |
| `net_udp.h/.c` | UDP transport, **protocol v2**: ack-based input delivery (512-frame retransmit window), HELLO handshake (version + input-delay + loss-proof `got_yours`), periodic state-checksum exchange (desync detection), BYE + liveness timeout, strict packet validation, save streaming at connect, **relay mode + join codes + automatic NAT hole-punching**, status fields for frontend UX, test helpers | Agnostic with small seams (below) |
| `relay.py` | Dependency-free public UDP relay: pairs two peers per room, forwards opaque payloads, emits the hole-punch rendezvous (`PEERINFO`), idles rooms out | Fully agnostic — deploy as-is on any VPS |
| `NET_ONLINE.md` | Architecture & protocol documentation (lockstep model, v2 protocol, relay/punch design, verification record) | Read first |
| `ONLINE_PLAYTEST.md` | The human playtest script (connection paths, `--net-delay` tuning, status meanings, what to capture on desync) | Template for the new game's guide |
| `COOP_REVIEW.md` | Nine review rounds of findings: every bug class hit, the fix, and what's deferred — the "lessons learned" ledger | Read before designing |

## Seams to satisfy in a new project

The transport compiles against a handful of small externals. Bring or stub:

1. **`types.h`** — `uint8/uint16/uint32/int32/uint64`, `bool`. Any project has an
   equivalent; adjust the includes.
2. **`ZELDA3_MULTIPLAYER` guard** — rename to your project's flag (e.g.
   `OOT_COOP`). Keep the discipline: the base single-player build must compile
   byte-identical with the flag off.
3. **`net_udp.c` includes `player_state.h`** solely for the
   `UDP_TX_HISTORY >= 2 * INPUT_RING_SIZE` compile-time check. Either bring an
   input-ring with that constant or replace the check with your own bound.
4. **Save streaming sizes** — `UDP_SRAM_SIZE/CHUNK/CHUNKS` are 8KB/512/16 for
   zelda3's SRAM. OoT's `SaveContext` is a different size: change the constants
   (the chunk protocol validates `total_chunks`, so bump `NET_PROTO_VERSION`
   when you do).
5. **The lockstep driver is NOT in this kit** (it's lockstep-specific): in
   zelda3 it lives in `src/zelda_rtl.c` (`Multiplayer_LockstepInit`,
   `Multiplayer_LockstepTick`, `Multiplayer_NetIdle`) on top of the input rings
   and `Multiplayer_ComputeChecksum` in `src/player_state.c`. Copy those only
   if the new game proves bit-exact deterministic (see the kickoff prompt's
   Phase 0). For host-authoritative state-sync, reuse this transport but
   replace `send_sync` desync-CRC usage with state broadcast.
6. **Frontend integration points** (reference: zelda3 `src/main.c`):
   CLI flags `--host/--connect/--relay/--join/--net-delay` and the `--online`
   interactive menu; the window-title status block (join code while waiting,
   `direct P2P`/`via relay`, `DESYNC DETECTED`, disconnect); `Udp_SendBye` on
   quit; `Multiplayer_NetIdle()` from the pause loop; autosave-load AND
   autosave-save both gated off while online.
7. **Headless tests** — port the pattern: env-gated scenarios
   (`*_TEST_UDP/RELAY/PUNCH`) that stand up transports in-process (loopback,
   localhost UDP, the in-kit C test relay) and assert the protocol. These caught
   bugs no windowed test would have.

## Invariants the hard way (violating any of these was a real shipped bug)

- Saving state must be **read-only w.r.t. checksummed memory** (scratch writes
  during save caused a permanently-latched false DESYNC).
- The liveness/disconnect timeout counts **only after the handshake** (else
  waiting for a friend reads as "player disconnected").
- **Bound every ring buffer** and refuse to overwrite unconsumed input — a
  paused/stalled peer must stall capture, not corrupt it.
- Keep transport upkeep (`poll` + recv drain) running **while paused**.
- Surface protocol-version mismatch on **both** roles; make it self-healing.
- The client must **never persist the host's save** over its own (gate
  quit-autosave online; document that online savestates embed the host save).
- Validate every datagram's type/length **before reading fields**; lock the
  peer slot only on a version-matching HELLO; drop wrong-source traffic; honor
  `PEERINFO` only from the relay's address.
- Cap post-stall catch-up (frames simulated per display tick) or a healed stall
  freezes the UI for seconds.
- Path flips (relay <-> punched direct P2P) are safe **only because** the
  protocol is connectionless and idempotent — keep it that way.

## License

zelda3-coop derives from `snesrev/zelda3` (MIT). These copies carry the same
license; keep attribution when lifting them.
