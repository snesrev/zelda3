// net_udp.c — UDP transport for online input-lockstep (see net_udp.h).
//
// Wire protocol v2: every datagram begins with a 1-byte type (NETPKT_*).
// INPUT packets carry an ack ("next frame of YOURS I need") plus a window of
// InputFrames starting at the peer's last ack — so any loss, reorder, or a
// peer that joins seconds late is healed by retransmission from exactly the
// point the peer is missing. HELLO does the version/input-delay handshake
// (carrying a got_yours flag so a lost reply can't deadlock the handshake);
// SYNC exchanges periodic state checksums for desync detection; BYE is a
// clean disconnect. The lockstep driver still only send()s/recv()s
// InputFrames — the control packets are handled here and surface as status
// fields the frontend reads. The per-tick poll() hook retransmits/keeps alive
// even while input capture is held (full ring / stalled sim), which is
// exactly when healing is needed.
//
// Hardening: UDP is a raw, unauthenticated, anyone-can-send-you-bytes socket, so
// the receive path is strict about what it accepts. Every datagram is bounds-
// checked against its declared type (Udp_PacketWellFormed) BEFORE any field is
// read, oversized reads are clamped, the host's peer slot can only be claimed
// by a version-matching HELLO (a stray scan datagram can't hijack or
// BYE-kill the session), and once we know our peer's address we drop anything
// that doesn't come from it (Udp_AddrMatches). (Full anti-spoofing of a forged
// source addr needs crypto and is out of scope; this stops everything short
// of that.)
// System/socket headers must come BEFORE any game header: types.h defines a
// function-like DWORD(x) macro that mangles `typedef DWORD (*...)` lines in
// TCC's winnt.h (pulled in by winsock2.h). main.c follows the same order.
#ifdef ZELDA3_MULTIPLAYER
#if defined(_WIN32)
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #pragma comment(lib, "ws2_32")   // TCC honors this; otherwise add -lws2_32
  typedef int socklen_t;
  #define CLOSESOCK closesocket
  static int g_wsa_started = 0;
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <netdb.h>
  #include <unistd.h>
  #include <fcntl.h>
  #include <errno.h>
  #define CLOSESOCK close
  #define INVALID_SOCKET (-1)
#endif
#endif

#include "net_udp.h"

#ifdef ZELDA3_MULTIPLAYER
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "player_state.h"   // INPUT_RING_SIZE (tx-history sufficiency check)

// Udp_Send records every captured frame into the tx history; the sufficiency
// argument (see net_udp.h) needs history >= both peers' input rings combined.
// Enforce it so a future ring resize can't silently enable resend-window
// overwrite (wrong frames retransmitted -> guaranteed desync).
#if UDP_TX_HISTORY < 2 * INPUT_RING_SIZE
#error UDP_TX_HISTORY must be at least 2 * INPUT_RING_SIZE
#endif

// INPUT packet: [type][count][ack:4 LE][count * frame]
#define UDP_INPUT_HDR    6
#define UDP_INPUT_MAX    (UDP_INPUT_HDR + UDP_MAX_BURST * INPUT_FRAME_WIRE_SIZE)
// SRAM packet: [type][chunk_idx][total][512-byte chunk]
#define UDP_SRAM_HDR     3
#define UDP_SRAM_PKT     (UDP_SRAM_HDR + UDP_SRAM_CHUNK)
// Receive buffer must fit the largest packet type or recvfrom() truncates.
#define UDP_MAX_PACKET   (UDP_SRAM_PKT > UDP_INPUT_MAX ? UDP_SRAM_PKT : UDP_INPUT_MAX)

static void Udp_SetNonBlocking(int sock) {
#if defined(_WIN32)
  u_long nb = 1; ioctlsocket(sock, FIONBIO, &nb);
#else
  int fl = fcntl(sock, F_GETFL, 0); fcntl(sock, F_SETFL, fl | O_NONBLOCK);
#endif
}

static bool Udp_PlatformInit(void) {
#if defined(_WIN32)
  if (!g_wsa_started) { WSADATA w; if (WSAStartup(MAKEWORD(2, 2), &w) != 0) return false; g_wsa_started = 1; }
#endif
  return true;
}

// Raw send to the peer (no-op for a host that hasn't learned the client yet).
// In relay mode peer_addr is the RELAY's address and every datagram is prefixed
// with the 4-byte room id; the relay strips it and forwards the inner payload to
// the other peer in the room (so the receiver still sees a clean NETPKT_*).
static void Udp_RawSendTo(UdpTransport *ut, const uint8 *buf, int len) {
  if (ut->is_host && !ut->peer_known) return;   // direct mode: no client addr yet
  if (len < 0) return;
  if (ut->relay) {
    // Hole-punch path selection: while TRYING, every packet goes BOTH via the
    // relay (framed) and straight at the peer's public endpoint (plain) — the
    // duplicate direct sends are the punches that open both NATs, and the
    // receiver's protocol is idempotent so duplicates are harmless. Once
    // DIRECT, send only direct (plus a periodic framed ping so the relay's
    // room stays warm for an instant fallback if the direct path dies).
    if (ut->punch_state != UDP_PUNCH_DIRECT ||
        ++ut->relay_keepwarm >= UDP_RELAY_KEEPWARM) {
      ut->relay_keepwarm = 0;
      uint8 framed[4 + UDP_MAX_PACKET];
      if (len > (int)sizeof(framed) - 4) return;
      framed[0] = (uint8)ut->relay_room;
      framed[1] = (uint8)(ut->relay_room >> 8);
      framed[2] = (uint8)(ut->relay_room >> 16);
      framed[3] = (uint8)(ut->relay_room >> 24);
      memcpy(framed + 4, buf, (size_t)len);
      sendto(ut->sock, (const char *)framed, len + 4, 0,
             (struct sockaddr *)ut->peer_addr, ut->peer_addr_len);
    }
    if (ut->punch_state != UDP_PUNCH_NONE)
      sendto(ut->sock, (const char *)buf, len, 0,
             (struct sockaddr *)ut->direct_addr, ut->direct_addr_len);
  } else {
    sendto(ut->sock, (const char *)buf, len, 0,
           (struct sockaddr *)ut->peer_addr, ut->peer_addr_len);
  }
}

static void Udp_SendHello(UdpTransport *ut) {
  uint8 p[4] = { NETPKT_HELLO, (uint8)NET_PROTO_VERSION, (uint8)ut->announced_delay,
                 (uint8)(ut->handshaked ? 1 : 0) };
  Udp_RawSendTo(ut, p, 4);
}

// Transmit one INPUT packet: our ack + the window of local frames the peer is
// still missing ([peer_ack .. tx_next-1], capped at UDP_MAX_BURST). count may
// be 0 — the packet then just carries the ack (keepalive + lets the peer's
// window slide). This is THE transmit path; it always restarts at peer_ack,
// so any gap on the peer's side is healed by the next transmission.
static void Udp_TransmitWindow(UdpTransport *ut) {
  uint32 start = ut->peer_ack;
  // Frames older than the history ring can't be resent (structurally unacked
  // never exceeds the ring; clamp defensively so we never read overwritten slots).
  if (ut->tx_next - start > UDP_TX_HISTORY)
    start = ut->tx_next - UDP_TX_HISTORY;
  uint32 avail = ut->tx_next - start;
  int count = avail > UDP_MAX_BURST ? UDP_MAX_BURST : (int)avail;

  uint8 pkt[UDP_INPUT_MAX];
  pkt[0] = NETPKT_INPUT;
  pkt[1] = (uint8)count;
  pkt[2] = (uint8)(ut->tx_ack);
  pkt[3] = (uint8)(ut->tx_ack >> 8);
  pkt[4] = (uint8)(ut->tx_ack >> 16);
  pkt[5] = (uint8)(ut->tx_ack >> 24);
  for (int i = 0; i < count; i++)
    InputFrame_Serialize(&ut->hist[(start + i) & (UDP_TX_HISTORY - 1)],
                         &pkt[UDP_INPUT_HDR + i * INPUT_FRAME_WIRE_SIZE]);
  Udp_RawSendTo(ut, pkt, UDP_INPUT_HDR + count * INPUT_FRAME_WIRE_SIZE);
}

// ---- NetTransport vtable ---------------------------------------------------
static bool Udp_Send(NetTransport *t, const InputFrame *f) {
  UdpTransport *ut = (UdpTransport *)t->impl;
  if (!ut->valid) return false;

  // Keep handshaking until the peer's HELLO arrives.
  if (!ut->handshaked) Udp_SendHello(ut);

  // Record into the history ring (frames arrive here in order from the driver).
  ut->hist[f->frame_number & (UDP_TX_HISTORY - 1)] = *f;
  ut->tx_next = f->frame_number + 1;

  Udp_TransmitWindow(ut);
  ut->sent_since_poll = 1;
  return true;
}

// Per-tick upkeep (lockstep driver calls this once per display tick, even when
// input capture is held): advertise our ack, retransmit the window if send()
// didn't already this tick, keep the handshake alive, drive the save-data
// sync, and count the liveness timeout. This is what makes a stalled session
// (full ring, paused peer, burst loss, late join) self-heal instead of
// hanging forever.
static void Udp_Poll(NetTransport *t, uint32 ack_frame) {
  UdpTransport *ut = (UdpTransport *)t->impl;
  if (!ut->valid) return;
  ut->tx_ack = ack_frame;
  if (!ut->sent_since_poll) {
    if (!ut->handshaked) Udp_SendHello(ut);
    Udp_TransmitWindow(ut);
  }
  ut->sent_since_poll = 0;
  // Client: request the host's save data until every chunk has landed
  // (idempotent re-request heals any chunk loss).
  if (!ut->is_host && ut->handshaked && !ut->sram_ok) {
    if (--ut->sram_req_timer <= 0) {
      uint8 p[1] = { NETPKT_SRAM_REQ };
      Udp_RawSendTo(ut, p, 1);
      ut->sram_req_timer = UDP_SRAM_REQ_INTERVAL;
    }
  }
  // Hole-punch: if the confirmed direct path goes quiet, fall back to dual-path
  // punching (relay + direct) — the session continues over the relay while the
  // direct sends keep trying to re-open the NAT mapping.
  if (ut->punch_state == UDP_PUNCH_DIRECT &&
      ++ut->direct_idle > UDP_PUNCH_DIRECT_IDLE) {
    ut->punch_state = UDP_PUNCH_TRYING;
    ut->punch_try_polls = 0;
    printf("[net] direct path quiet - falling back to relay (still punching)\n");
  }
  // If punching never lands (e.g. a symmetric NAT randomizes the port per
  // destination), stop the extra direct sends; the session simply stays on the
  // relay, and the relay's periodic PEERINFO re-triggers an occasional retry.
  if (ut->punch_state == UDP_PUNCH_TRYING &&
      ++ut->punch_try_polls > UDP_TIMEOUT_POLLS) {
    ut->punch_state = UDP_PUNCH_NONE;
    ut->punch_try_polls = 0;
  }
  // Liveness: one tick with no peer traffic; recv() resets this on arrival.
  // Counted only once a session exists — a host (or early-started client)
  // still waiting for its peer to appear is "connecting", not "disconnected"
  // (an ungated count used to flag peer_lost after ~10s of sitting on the
  // title screen waiting for the first join).
  if (ut->handshaked && ++ut->idle_polls > UDP_TIMEOUT_POLLS) ut->peer_lost = 1;
}

// Session readiness for the lockstep driver: input capture starts only when
// the handshake is done and (for the client) the host's save data is in.
// Both peers therefore begin the simulation together at frame 0.
static int Udp_Ready(NetTransport *t) {
  UdpTransport *ut = (UdpTransport *)t->impl;
  return ut->valid && ut->handshaked && ut->sram_ok;
}

static bool Udp_RxqEmpty(UdpTransport *ut) { return ut->rx_head == ut->rx_tail; }

// Reject anything whose declared type/length is impossible BEFORE we read fields.
// This is the single gate every inbound datagram passes; downstream code may then
// trust the lengths. Returns false for unknown types, short packets, an INPUT
// frame count past the burst cap, or an INPUT packet too small for its count.
static bool Udp_PacketWellFormed(const uint8 *pkt, int n) {
  if (n < 1) return false;
  switch (pkt[0]) {
    case NETPKT_INPUT:
      if (n < UDP_INPUT_HDR) return false;
      if (pkt[1] > UDP_MAX_BURST) return false;
      return UDP_INPUT_HDR + (int)pkt[1] * INPUT_FRAME_WIRE_SIZE <= n;
    case NETPKT_HELLO: return n >= 4;
    case NETPKT_SYNC:  return n >= 9;
    case NETPKT_BYE:   return true;
    case NETPKT_SRAM_REQ: return true;
    case NETPKT_SRAM:
      return n >= UDP_SRAM_HDR + UDP_SRAM_CHUNK &&
             pkt[1] < UDP_SRAM_CHUNKS && pkt[2] == UDP_SRAM_CHUNKS;
    case NETPKT_PEERINFO: return n >= 7;
    default:           return false;
  }
}

// IPv4 sockaddr equality (compare family/port/addr explicitly — memcmp would
// also compare sockaddr padding, which isn't guaranteed zeroed).
static bool Udp_SockaddrEq(const unsigned char *stored, int stored_len,
                           const unsigned char *src, socklen_t srclen) {
  if (stored_len < (int)sizeof(struct sockaddr_in) ||
      srclen < (socklen_t)sizeof(struct sockaddr_in)) return false;
  const struct sockaddr_in *a = (const struct sockaddr_in *)stored;
  const struct sockaddr_in *b = (const struct sockaddr_in *)src;
  return a->sin_family == b->sin_family &&
         a->sin_port   == b->sin_port &&
         a->sin_addr.s_addr == b->sin_addr.s_addr;
}

// Does a received datagram's source address match the peer we've locked onto
// (the relay's address in relay mode)?
static bool Udp_AddrMatches(const UdpTransport *ut, const unsigned char *src, socklen_t srclen) {
  return Udp_SockaddrEq(ut->peer_addr, ut->peer_addr_len, src, srclen);
}

// Process one received datagram by type.
static void Udp_HandlePacket(UdpTransport *ut, const uint8 *pkt, int n) {
  if (n < 1) return;
  switch (pkt[0]) {
    case NETPKT_INPUT: {
      if (n < UDP_INPUT_HDR) return;
      int count = pkt[1];
      if (count < 0 || count > UDP_MAX_BURST) return;
      if (UDP_INPUT_HDR + count * INPUT_FRAME_WIRE_SIZE > n) return;   // truncated
      // The peer's ack of OUR frames: slides our retransmit window forward.
      // Monotonic guard — a reordered old packet must not move it backwards.
      uint32 ack = pkt[2] | (pkt[3] << 8) | (pkt[4] << 16) | ((uint32)pkt[5] << 24);
      if ((int32)(ack - ut->peer_ack) > 0 && (int32)(ack - ut->tx_next) <= 0)
        ut->peer_ack = ack;
      for (int i = 0; i < count; i++) {
        int nh = (ut->rx_tail + 1) % UDP_RECV_QUEUE;
        if (nh == ut->rx_head) break;                            // FIFO full
        InputFrame_Deserialize(&pkt[UDP_INPUT_HDR + i * INPUT_FRAME_WIRE_SIZE],
                               &ut->rxq[ut->rx_tail]);
        ut->rx_tail = nh;
      }
      break;
    }
    case NETPKT_HELLO: {
      if (n < 4) return;
      if (pkt[1] != NET_PROTO_VERSION) { ut->version_mismatch = 1; return; }
      ut->peer_input_delay = pkt[2];
      ut->handshaked = 1;
      // got_yours==0 means the peer hasn't received any HELLO of ours yet —
      // answer it (every time, so a lost reply is healed by the peer's next
      // proactive HELLO). got_yours==1 means it has: nothing more to say, and
      // not replying is what terminates the exchange (no reply storm).
      if (pkt[3] == 0)
        Udp_SendHello(ut);
      break;
    }
    case NETPKT_SYNC: {
      if (n < 9) return;
      uint32 frame = pkt[1] | (pkt[2] << 8) | (pkt[3] << 16) | ((uint32)pkt[4] << 24);
      uint32 crc   = pkt[5] | (pkt[6] << 8) | (pkt[7] << 16) | ((uint32)pkt[8] << 24);
      for (int i = 0; i < UDP_SYNC_RING; i++) {
        if (ut->sync_frame[i] == frame && (ut->sync_frame[i] || ut->sync_crc[i])) {
          if (ut->sync_crc[i] != crc) { ut->desynced = 1; ut->desync_frame = frame; }
          break;
        }
      }
      break;
    }
    case NETPKT_BYE:
      ut->peer_lost = 1;
      break;
    case NETPKT_SRAM_REQ:
      // Host only: serve the full save image (idempotent — the client re-asks
      // until every chunk has landed, so per-chunk loss needs no bookkeeping).
      if (ut->is_host && ut->sram_src) {
        uint8 p[UDP_SRAM_PKT];
        p[0] = NETPKT_SRAM;
        p[2] = (uint8)UDP_SRAM_CHUNKS;
        for (int i = 0; i < UDP_SRAM_CHUNKS; i++) {
          p[1] = (uint8)i;
          memcpy(&p[UDP_SRAM_HDR], ut->sram_src + i * UDP_SRAM_CHUNK, UDP_SRAM_CHUNK);
          Udp_RawSendTo(ut, p, UDP_SRAM_PKT);
        }
      }
      break;
    case NETPKT_SRAM:
      // Client only (length/index already validated by Udp_PacketWellFormed;
      // re-checked here so this handler stays memory-safe even if it ever
      // gains another caller or the gate changes).
      if (!ut->is_host && !ut->sram_ok) {
        int idx = pkt[1];
        if (idx >= UDP_SRAM_CHUNKS) break;
        memcpy(&ut->sram_buf[idx * UDP_SRAM_CHUNK], &pkt[UDP_SRAM_HDR], UDP_SRAM_CHUNK);
        ut->sram_have_mask |= 1u << idx;
        if (ut->sram_have_mask == (1u << UDP_SRAM_CHUNKS) - 1)
          ut->sram_ok = 1;   // frontend applies sram_buf, then sets sram_applied
      }
      break;
    default: break;
  }
}

static bool Udp_Recv(NetTransport *t, InputFrame *out) {
  UdpTransport *ut = (UdpTransport *)t->impl;
  if (!ut->valid) return false;

  // Drain all pending datagrams (processing control packets) until we either
  // have a decomposed InputFrame to return or the socket is empty.
  while (Udp_RxqEmpty(ut)) {
    uint8 pkt[UDP_MAX_PACKET];
    unsigned char src[28];
    socklen_t srclen = sizeof(src);
    int n = recvfrom(ut->sock, (char *)pkt, sizeof(pkt), 0,
                     (struct sockaddr *)src, &srclen);
    if (n <= 0) return false;                  // nothing pending (non-blocking)
    if (n > (int)sizeof(pkt)) n = (int)sizeof(pkt);   // clamp (paranoia: no over-read)
    if (!Udp_PacketWellFormed(pkt, n)) continue;      // drop garbage outright

    if (ut->is_host && !ut->peer_known) {
      // The host's peer slot is claimed ONLY by a version-matching HELLO (the
      // client HELLOs until handshaked, so this is the natural first contact).
      // A stray INPUT/SYNC/BYE — port scans, a stale earlier session — can't
      // hijack the slot or make the session look dead before it starts.
      if (pkt[0] != NETPKT_HELLO || pkt[1] != NET_PROTO_VERSION) {
        // A HELLO with the wrong version is surfaced, not silently dropped —
        // otherwise two mismatched builds both sit on "connecting..." forever
        // with no hint why. Self-healing: a later version-matching HELLO (the
        // real client) clears the flag below, so stray noise can't wedge it.
        if (pkt[0] == NETPKT_HELLO)
          ut->version_mismatch = 1;
        continue;
      }
      if (srclen > (socklen_t)sizeof(ut->peer_addr)) srclen = sizeof(ut->peer_addr);
      memcpy(ut->peer_addr, src, srclen);
      ut->peer_addr_len = (int)srclen;
      ut->peer_known = 1;
      ut->version_mismatch = 0;   // a valid peer bound; clear any stray-noise flag
    } else if (Udp_AddrMatches(ut, src, srclen)) {
      // From the locked peer (direct mode) or from the relay (relay mode).
      if (ut->relay && pkt[0] == NETPKT_PEERINFO) {
        // Rendezvous: the relay told us the OTHER peer's public endpoint as it
        // sees it. Record it and start (or refresh) hole-punching — our normal
        // outbound packets will additionally be fired straight at that endpoint
        // (see Udp_RawSendTo); the peer does the same, and the simultaneous
        // outbound traffic opens both NATs. Only honored from the relay's own
        // address; it is relay CONTROL traffic, so it doesn't count as peer
        // liveness. (A peer that can't be punched — e.g. symmetric NAT — just
        // keeps playing via the relay; see the TRYING timeout in Udp_Poll.)
        struct sockaddr_in da;
        memset(&da, 0, sizeof(da));
        da.sin_family = AF_INET;
        memcpy(&da.sin_addr, &pkt[1], 4);
        da.sin_port = htons((uint16)((pkt[5] << 8) | pkt[6]));
        memcpy(ut->direct_addr, &da, sizeof(da));
        ut->direct_addr_len = (int)sizeof(da);
        if (ut->punch_state == UDP_PUNCH_NONE) {
          ut->punch_state = UDP_PUNCH_TRYING;
          ut->punch_try_polls = 0;
          if (!ut->punch_try_printed) {
            ut->punch_try_printed = 1;
            printf("[net] peer endpoint received - attempting direct P2P (hole-punch)\n");
          }
        }
        continue;
      }
    } else if (ut->relay && ut->punch_state != UDP_PUNCH_NONE &&
               Udp_SockaddrEq(ut->direct_addr, ut->direct_addr_len, src, srclen)) {
      // Punched-through traffic arriving straight from the peer: the direct
      // path works. Switch to it (lower latency; the relay stays warm via the
      // periodic framed ping and we fall back automatically if this goes quiet).
      ut->direct_idle = 0;
      if (ut->punch_state != UDP_PUNCH_DIRECT) {
        ut->punch_state = UDP_PUNCH_DIRECT;
        printf("[net] direct P2P established - bypassing the relay\n");
      }
    } else {
      continue;                                 // not from our peer — ignore (anti-spoof)
    }

    // Only genuine peer traffic counts toward liveness / clears a lost flag.
    ut->idle_polls = 0;
    if (ut->peer_lost && pkt[0] != NETPKT_BYE) ut->peer_lost = 0;  // traffic resumed
    Udp_HandlePacket(ut, pkt, n);
  }

  *out = ut->rxq[ut->rx_head];
  ut->rx_head = (ut->rx_head + 1) % UDP_RECV_QUEUE;
  return true;
}

static void Udp_SendSyncFn(NetTransport *t, uint32 frame, uint32 checksum) {
  UdpTransport *ut = (UdpTransport *)t->impl;
  if (!ut->valid) return;
  // Remember our checksum for this frame so an incoming peer SYNC can be compared.
  int slot = (int)(frame % UDP_SYNC_RING);
  ut->sync_frame[slot] = frame;
  ut->sync_crc[slot] = checksum;
  uint8 p[9] = { NETPKT_SYNC,
                 (uint8)frame, (uint8)(frame >> 8), (uint8)(frame >> 16), (uint8)(frame >> 24),
                 (uint8)checksum, (uint8)(checksum >> 8), (uint8)(checksum >> 16), (uint8)(checksum >> 24) };
  Udp_RawSendTo(ut, p, 9);
}

void Udp_SendBye(UdpTransport *ut) {
  if (!ut || !ut->valid) return;
  uint8 p[1] = { NETPKT_BYE };
  Udp_RawSendTo(ut, p, 1);
}

void Udp_GetPeerIPv4(const UdpTransport *ut, uint8 *out) {
  const struct sockaddr_in *pa = (const struct sockaddr_in *)ut->peer_addr;
  memcpy(out, &pa->sin_addr, 4);
}

static void Udp_Close(NetTransport *t) {
  UdpTransport *ut = (UdpTransport *)t->impl;
  if (ut->sock != INVALID_SOCKET) { CLOSESOCK(ut->sock); ut->sock = INVALID_SOCKET; }
  ut->valid = 0;
}

static void Udp_Wire(UdpTransport *ut) {
  ut->iface.send = Udp_Send;
  ut->iface.recv = Udp_Recv;
  ut->iface.close = Udp_Close;
  ut->iface.send_sync = Udp_SendSyncFn;
  ut->iface.poll = Udp_Poll;
  ut->iface.ready = Udp_Ready;
  ut->iface.impl = ut;
}

// ---- init ------------------------------------------------------------------
static bool Udp_Common(UdpTransport *ut, int input_delay) {
  memset(ut, 0, sizeof(*ut));
  ut->sock = INVALID_SOCKET;
  ut->announced_delay = input_delay < 0 ? 0 : input_delay;
  if (!Udp_PlatformInit()) return false;
  ut->sock = (int)socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (ut->sock == INVALID_SOCKET) return false;
  Udp_SetNonBlocking(ut->sock);
  Udp_Wire(ut);
  return true;
}

bool Udp_InitHost(UdpTransport *ut, unsigned short port, int input_delay) {
  if (!Udp_Common(ut, input_delay)) return false;
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(port);
  if (bind(ut->sock, (struct sockaddr *)&addr, sizeof(addr)) != 0) { Udp_Close(&ut->iface); return false; }
  ut->is_host = 1;
  ut->sram_ok = 1;   // the host's own SRAM is the authoritative save data
  ut->valid = 1;
  printf("[net] hosting on UDP port %u, waiting for a client...\n", port);
  return true;
}

bool Udp_InitClient(UdpTransport *ut, const char *host_ip, unsigned short port, int input_delay) {
  if (!Udp_Common(ut, input_delay)) return false;
  struct sockaddr_in *pa = (struct sockaddr_in *)ut->peer_addr;
  memset(pa, 0, sizeof(*pa));
  pa->sin_family = AF_INET;
  pa->sin_port = htons(port);
  if (inet_pton(AF_INET, host_ip, &pa->sin_addr) != 1) { Udp_Close(&ut->iface); return false; }
  ut->peer_addr_len = sizeof(struct sockaddr_in);
  ut->peer_known = 1;
  ut->is_host = 0;
  ut->valid = 1;
  printf("[net] connecting to host %s:%u\n", host_ip, port);
  return true;
}

// Resolve a dotted-quad or hostname to an IPv4 address (numeric fast-path, then
// DNS). Returns true on success.
static bool Udp_ResolveIPv4(const char *host, struct in_addr *out) {
  if (inet_pton(AF_INET, host, out) == 1) return true;
  struct addrinfo hints, *res = NULL;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  if (getaddrinfo(host, NULL, &hints, &res) != 0 || !res) return false;
  *out = ((struct sockaddr_in *)res->ai_addr)->sin_addr;
  freeaddrinfo(res);
  return true;
}

bool Udp_InitRelay(UdpTransport *ut, const char *relay_host, unsigned short relay_port,
                   uint32 room, int is_host, int input_delay) {
  if (!Udp_Common(ut, input_delay)) return false;
  // No bind: both peers are OUTBOUND sockets to the relay (that's what makes it
  // NAT-friendly). peer_addr holds the RELAY's address; all sends are framed
  // with the room id (see Udp_RawSendTo) and the relay forwards between peers.
  struct sockaddr_in *pa = (struct sockaddr_in *)ut->peer_addr;
  memset(pa, 0, sizeof(*pa));
  pa->sin_family = AF_INET;
  pa->sin_port = htons(relay_port);
  if (!Udp_ResolveIPv4(relay_host, &pa->sin_addr)) { Udp_Close(&ut->iface); return false; }
  ut->peer_addr_len = sizeof(struct sockaddr_in);
  ut->peer_known = 1;          // we always talk to the relay; nothing to "learn"
  ut->is_host = is_host ? 1 : 0;
  ut->relay = 1;
  ut->relay_room = room;
  ut->sram_ok = is_host ? 1 : 0;   // host's own SRAM is authoritative
  ut->valid = 1;
  printf("[net] relay %s:%u room %08X — you are %s\n", relay_host, relay_port, room,
         is_host ? "the HOST (Player 1)" : "joining (Player 2)");
  return true;
}

// ---- join code (Crockford base32 of [ip:4][port:2 BE][room:4 LE]) ----------
static const char kJoinB32[] = "0123456789ABCDEFGHJKMNPQRSTVWXYZ";  // no I L O U

void Net_MakeJoinCode(const uint8 ip[4], uint16 port, uint32 room, char *out_code) {
  uint8 b[10];
  b[0] = ip[0]; b[1] = ip[1]; b[2] = ip[2]; b[3] = ip[3];
  b[4] = (uint8)(port >> 8); b[5] = (uint8)port;
  b[6] = (uint8)room; b[7] = (uint8)(room >> 8);
  b[8] = (uint8)(room >> 16); b[9] = (uint8)(room >> 24);
  int o = 0;
  for (int i = 0; i < 16; i++) {            // 80 bits -> 16 groups of 5, MSB-first
    if (i && (i % 4) == 0) out_code[o++] = '-';
    int bit = i * 5, byteidx = bit >> 3, off = bit & 7;
    int v = (b[byteidx] << 8) | (byteidx + 1 < 10 ? b[byteidx + 1] : 0);
    out_code[o++] = kJoinB32[(v >> (11 - off)) & 0x1F];
  }
  out_code[o] = 0;
}

static int JoinB32Val(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  c = (char)toupper((unsigned char)c);
  if (c == 'I' || c == 'L') return 1;       // forgiving: look-alikes
  if (c == 'O') return 0;
  for (int i = 10; i < 32; i++) if (kJoinB32[i] == c) return i;
  return -1;
}

bool Net_ParseJoinCode(const char *code, uint8 *ip_out, uint16 *port_out, uint32 *room_out) {
  uint8 b[10] = {0};
  int nbits = 0;
  for (const char *s = code; *s; s++) {
    if (*s == '-' || *s == ' ') continue;
    int v = JoinB32Val(*s);
    if (v < 0 || nbits + 5 > 80) return false;
    for (int k = 4; k >= 0; k--) {
      int pos = nbits + (4 - k);
      if ((v >> k) & 1) b[pos >> 3] |= (uint8)(0x80 >> (pos & 7));
    }
    nbits += 5;
  }
  if (nbits != 80) return false;
  ip_out[0] = b[0]; ip_out[1] = b[1]; ip_out[2] = b[2]; ip_out[3] = b[3];
  *port_out = (uint16)(((uint16)b[4] << 8) | b[5]);
  *room_out = (uint32)b[6] | ((uint32)b[7] << 8) | ((uint32)b[8] << 16) | ((uint32)b[9] << 24);
  return true;
}

#ifdef ZELDA3_HEADLESS_TEST
// Test-only: fire a raw datagram at 127.0.0.1:port from a throwaway socket (so it
// arrives from an ephemeral source port that differs from any established peer).
// Lets the harness verify the receive path drops malformed and wrong-source
// packets. Returns sendto()'s result (>=0 ok), or -1 if the socket couldn't open.
int Udp_TestRawSendLocal(unsigned short port, const uint8 *buf, int len) {
  if (!Udp_PlatformInit()) return -1;
  int s = (int)socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (s == INVALID_SOCKET) return -1;
  struct sockaddr_in a;
  memset(&a, 0, sizeof(a));
  a.sin_family = AF_INET;
  a.sin_port = htons(port);
  inet_pton(AF_INET, "127.0.0.1", &a.sin_addr);
  int r = (int)sendto(s, (const char *)buf, len, 0, (struct sockaddr *)&a, sizeof(a));
  CLOSESOCK(s);
  return r;
}

// Test-only in-process relay: the exact forwarder tools/relay.py implements.
// Opens a loopback UDP socket, pairs the first two source addresses that send
// for the room, and forwards each datagram's inner payload (after the 4-byte
// room prefix) to the OTHER paired peer. Single room (enough for the harness).
static struct { unsigned char addr[28]; int len; } g_test_relay_peers[2];
static int g_test_relay_npeers;

int Udp_TestRelayOpen(unsigned short port) {
  if (!Udp_PlatformInit()) return -1;
  int s = (int)socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (s == INVALID_SOCKET) return -1;
  Udp_SetNonBlocking(s);
  struct sockaddr_in a;
  memset(&a, 0, sizeof(a));
  a.sin_family = AF_INET;
  a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  a.sin_port = htons(port);
  if (bind(s, (struct sockaddr *)&a, sizeof(a)) != 0) { CLOSESOCK(s); return -1; }
  g_test_relay_npeers = 0;
  return s;
}

void Udp_TestRelayPump(int s) {
  uint8 pkt[4 + UDP_MAX_PACKET];
  for (;;) {
    unsigned char src[28];
    socklen_t sl = sizeof(src);
    int n = recvfrom(s, (char *)pkt, sizeof(pkt), 0, (struct sockaddr *)src, &sl);
    if (n <= 0) break;
    if (n < 4) continue;                         // need at least the room prefix
    int idx = -1;
    for (int i = 0; i < g_test_relay_npeers; i++)
      if (g_test_relay_peers[i].len == (int)sl && memcmp(g_test_relay_peers[i].addr, src, sl) == 0) { idx = i; break; }
    if (idx < 0 && g_test_relay_npeers < 2) {
      idx = g_test_relay_npeers++;
      memcpy(g_test_relay_peers[idx].addr, src, sl);
      g_test_relay_peers[idx].len = (int)sl;
    }
    for (int i = 0; i < g_test_relay_npeers; i++)   // forward inner payload to the other peer
      if (i != idx)
        sendto(s, (const char *)(pkt + 4), n - 4, 0,
               (struct sockaddr *)g_test_relay_peers[i].addr, g_test_relay_peers[i].len);
  }
  // Rendezvous (mirrors tools/relay.py): once both peers are known, periodically
  // tell each the OTHER's observed endpoint so they can hole-punch.
  static int peerinfo_ctr;
  if (g_test_relay_npeers == 2 && ++peerinfo_ctr >= 25) {
    peerinfo_ctr = 0;
    for (int i = 0; i < 2; i++) {
      const struct sockaddr_in *other =
          (const struct sockaddr_in *)g_test_relay_peers[i ^ 1].addr;
      uint8 p[7];
      p[0] = NETPKT_PEERINFO;
      memcpy(&p[1], &other->sin_addr, 4);
      uint16 hp = ntohs(other->sin_port);
      p[5] = (uint8)(hp >> 8);
      p[6] = (uint8)hp;
      sendto(s, (const char *)p, 7, 0,
             (struct sockaddr *)g_test_relay_peers[i].addr, g_test_relay_peers[i].len);
    }
  }
}

void Udp_TestRelayClose(int s) { if (s != INVALID_SOCKET) CLOSESOCK(s); }

// Test-only: sabotage this transport's punched DIRECT send path by pointing
// direct_addr at a dead port — its direct sends silently vanish, so the PEER
// stops seeing direct traffic and must fall back to the relay. (Receiving on
// this side is unaffected.)
void Udp_TestBreakDirectPath(UdpTransport *ut) {
  struct sockaddr_in *da = (struct sockaddr_in *)ut->direct_addr;
  da->sin_port = htons(1);
}
#endif

#endif  // ZELDA3_MULTIPLAYER
