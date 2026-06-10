# Online Playtest Guide — first two-machine session

A practical checklist for playing (and validating) online co-op across two real
machines. The netcode is verified over localhost; **this playtest is the first
run on real hardware/networks** — follow the capture steps at the bottom so any
problem found is reportable and fixable.

## 0. Requirements (do these first)

- **Identical build on both machines.** Lockstep assumes a byte-identical
  simulation: same source revision, same compiler/platform build if possible,
  and the same `zelda3_assets.dat`. A mismatched build is flagged
  (`INCOMPATIBLE VERSION`) only when the *network protocol* differs — a same-
  protocol but different-sim build instead shows up later as `DESYNC DETECTED`.
- **Saves:** only the **host's** save files matter. At connect the host streams
  its SRAM to the client; the client's local saves are neither used nor
  modified. Whoever's progress you want to play, make them the host.
- **Windows note:** the TCC build needs `-lws2_32` appended to the build
  command to link Winsock (POSIX/macOS need nothing extra).

## 1. Connect — pick one of three paths

### a) Same LAN (easiest, no setup)
```
host:    zelda3_coop --host 7777
client:  zelda3_coop --connect <host's LAN IP> 7777
```
Find the host's LAN IP with `ipconfig` (Windows) / `ip addr` (Linux) /
`ifconfig` (macOS).

### b) Internet via relay + JOIN CODE (recommended for WAN — no router config)
A small relay forwards between the two players, so **neither** needs a
port-forward or VPN (both just send outbound, which NAT allows). Someone runs
the bundled relay on any host both players can reach — a $5 VPS, a
port-forwarded PC, a friend's box:
```
relay box:  python3 tools/relay.py 7777      # leave it running; no deps
```
Then the host points at that relay and gets a **join code** to share; the other
player needs only the code:
```
host:    zelda3_coop --host --relay <relay-host>:7777
         ->  prints:  share this JOIN CODE with Player 2:  FW00-00CR-B023-YGPE
client:  zelda3_coop --join FW00-00CR-B023-YGPE
```
The code bakes in the relay's IP + port + a random room, so no other arguments
are needed on the joining side. (The relay only passes opaque input/control
bytes — no game state — and forgets idle rooms.)

### c) Internet via VPN overlay (no relay host needed)
Install **Tailscale / ZeroTier / WireGuard / Hamachi** on both PCs, then connect
exactly like LAN using the host's VPN address:
```
client:  zelda3_coop --connect <host's VPN IP> 7777
```

### d) Internet via port-forward (direct, lowest latency)
On the **host's** router, forward **UDP 7777** to the host PC's LAN IP. Client
connects to the host's **public** IP. Only the host needs the forward; the
client just needs outbound UDP.

> NAT hole-punching / a hosted default relay / lobby matchmaking are still
> future niceties; (b) covers WAN today with one self-run relay.

## 2. Latency buffer (`--net-delay`)

Felt input lag ≈ link round-trip time + `net-delay` frames (1 frame ≈ 16.7 ms).
Both sides should pass the **same** value:

| Link | Suggested `--net-delay` |
|---|---|
| LAN / same city | 2 (default) |
| Typical internet (20–60 ms RTT) | 3–4 |
| Long-haul / jittery | 5–6 |

If movement stutters with `waiting for player…` flickering in the title, raise
it by one and relaunch. Smooth-but-laggy means you can lower it.

## 3. Launch order & session start

Order doesn't matter. The host can sit on the title screen indefinitely; both
sims hold at frame 0 until the handshake completes **and** the client has the
host's save data, then both run together. Console prints
`session established - simulation running in lockstep` on each side when live.

## 4. Status indicators (window title)

| Title shows | Meaning / action |
|---|---|
| `online: connecting to peer...` | No handshake yet. Check IP/port, firewall, UDP forward/VPN. |
| `receiving save data…` | SRAM transfer in progress (sub-second normally). Stuck = heavy loss; check link. |
| `online: waiting for player...` | Sim stalled awaiting the peer's input — transient blips are normal; constant = raise `--net-delay` or check loss. |
| `online: player disconnected` | Peer quit (BYE) or ~10 s of silence. Restart both to reconnect (no mid-session rejoin yet). |
| `online: DESYNC DETECTED (states diverged)` | **The bug we're hunting.** Sims diverged; see §6. |
| `online: INCOMPATIBLE VERSION - cannot play` | Different protocol versions — rebuild both from the same revision. |

Milestones are also printed once each to the console — keep the terminals
visible during the playtest.

## 5. What to exercise (in rough order)

1. **Lobby basics:** connect, walk both Links around, verify each player
   controls their own Link with their local pad/keys.
2. **Late join:** start the host, wait 30+ s, then connect the client.
3. **Combat:** both players fight the same enemies; both take hits; bombs hurt
   anyone in range; no friendly fire from sword/arrows.
4. **Co-op verbs:** P2 lifts pots/bushes, opens chests, stands on pressure
   plates, collects drops; revive a downed partner by touch.
5. **Transitions:** doors, stairs, overworld edges, a dungeon entrance — P2
   should snap beside P1 after each.
6. **Death:** let one player die (ghost + revive), then both (game over →
   Continue → both alive again).
7. **Stress:** pause one side for ~10 s and unpause (both should resume in
   sync); play through at least one full dungeon.
8. **Quit:** close one side — the other should show `player disconnected`
   within seconds.

## 6. If `DESYNC DETECTED` fires — capture this

The title/console line includes the **frame number** where checksums first
diverged. Please record:

1. Both consoles' full output (host and client) — especially the
   `[net] DESYNC at frame N` line from each side.
2. What both players were doing in the seconds before (room, enemies, items
   used, anything unusual — e.g. "P2 fell in a pit as P1 opened the menu").
3. Exact build revision (`git rev-parse HEAD`) on both machines, OS of each,
   and the `--net-delay` used.
4. If reproducible: the shortest sequence that re-triggers it.

A desync is *detected*, not auto-healed (recovery/resync is documented future
work) — restart both sides to continue playing.

## 7. Known limitations going in

- No NAT punch-through / relay / join codes (use VPN or port-forward).
- No mid-session reconnect; a drop means relaunch both.
- Desyncs warn (with frame number) but don't auto-resync.
- Savestate-load / replay / reset / cheat hotkeys are intentionally ignored
  while online (each would desync); saving a state still works and is
  checksum-pure (it cannot trip a false desync).
- A state saved DURING an online session captures the session — on the client
  that includes the host's save data. The quit-time autosave is skipped while
  online for exactly that reason (so a later offline launch can't accidentally
  carry the host's progression into your own saves).
- No wire encryption/authentication — play with people you know, don't
  leave the port exposed long-term.
