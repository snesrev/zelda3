#!/usr/bin/env bash
# Re-snapshot the portable netcode kit from the live zelda3-coop tree.
# Run from the repo root (or anywhere — it cd's itself). The kit is a frozen
# copy meant to be lifted into a NEW project; the canonical, evolving sources
# stay in src/ and tools/. Re-run this after changing any of them.
set -euo pipefail
cd "$(dirname "$0")/../.."
cp src/net_types.h src/net_transport.h src/net_transport.c \
   src/net_udp.h src/net_udp.c tools/netcode-kit/
cp tools/relay.py tools/netcode-kit/relay.py
cp NET_ONLINE.md ONLINE_PLAYTEST.md COOP_REVIEW.md tools/netcode-kit/
echo "netcode-kit refreshed from $(git rev-parse --short HEAD)"
