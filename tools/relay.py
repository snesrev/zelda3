#!/usr/bin/env python3
"""
zelda3-coop WAN relay — a tiny, dependency-free UDP forwarder.

Run this on any host both players can reach (a $5 VPS, a port-forwarded home
PC, a friend's box). It pairs the two peers of a session by a "room" id and
forwards their packets, so NEITHER player needs a port-forward or VPN — both
just send OUTBOUND to this relay (which NAT allows), and the relay relays.

    python3 relay.py            # listen on UDP 0.0.0.0:7777
    python3 relay.py 9999       # custom port

In the game (host side):  zelda3_coop --host --relay <this-host>:7777
It prints a JOIN CODE; the other player runs:  zelda3_coop --join <code>
(The code bakes in this relay's IP + port + a random room, so the joiner needs
nothing else.)

Wire protocol (matches src/net_udp.c): every datagram from a peer is
    [room: 4 bytes little-endian][payload...]
The relay learns each peer's address from the packets it sends, keeps the two
most recent distinct addresses per room, and forwards each inner `payload` to
the OTHER peer in that room. It is otherwise completely transparent to the
game's handshake / lockstep / save-sync protocol.

No game state ever touches the relay — only opaque input/control bytes pass
through — and rooms idle out after a few minutes, so a long-running relay needs
no maintenance. There is no auth/encryption (out of scope); run it for friends.
"""
import socket
import struct
import sys
import time

ROOM_IDLE_SECONDS = 300          # forget a room after this much silence
MAX_DATAGRAM = 2048              # generously larger than any game packet


def main():
    port = 7777
    if len(sys.argv) > 1:
        try:
            port = int(sys.argv[1])
        except ValueError:
            print("usage: relay.py [port]", file=sys.stderr)
            return 2
    if not (0 < port <= 65535):
        print("port must be 1..65535", file=sys.stderr)
        return 2

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.bind(("0.0.0.0", port))
    sock.settimeout(30.0)
    print(f"[relay] listening on UDP 0.0.0.0:{port} — Ctrl-C to stop")

    # room id -> { addr: last_seen_monotonic } (keeps the 2 most-recent addrs)
    rooms = {}
    last_sweep = time.monotonic()

    while True:
        try:
            data, addr = sock.recvfrom(MAX_DATAGRAM)
        except socket.timeout:
            data = None
        now = time.monotonic()

        if data is not None and len(data) >= 4:
            room = struct.unpack_from("<I", data, 0)[0]
            payload = data[4:]
            peers = rooms.setdefault(room, {})
            peers[addr] = now
            # Keep only the two most-recently-seen addresses for this room.
            if len(peers) > 2:
                for stale in sorted(peers, key=peers.get)[:-2]:
                    del peers[stale]
            # Forward the inner payload to the OTHER peer(s) in the room.
            for other in peers:
                if other != addr:
                    try:
                        sock.sendto(payload, other)
                    except OSError:
                        pass

        # Periodically drop idle rooms so memory stays bounded.
        if now - last_sweep > 30.0:
            last_sweep = now
            for room in list(rooms):
                peers = rooms[room]
                for a in [a for a, t in peers.items() if now - t > ROOM_IDLE_SECONDS]:
                    del peers[a]
                if not peers:
                    del rooms[room]


if __name__ == "__main__":
    try:
        sys.exit(main() or 0)
    except KeyboardInterrupt:
        print("\n[relay] stopped")
