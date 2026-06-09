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
#include "net_udp.h"

#ifdef ZELDA3_MULTIPLAYER
#include <string.h>
#include <stdio.h>

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
  #include <unistd.h>
  #include <fcntl.h>
  #include <errno.h>
  #define CLOSESOCK close
  #define INVALID_SOCKET (-1)
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
static void Udp_RawSendTo(UdpTransport *ut, const uint8 *buf, int len) {
  if (ut->is_host && !ut->peer_known) return;
  sendto(ut->sock, (const char *)buf, len, 0,
         (struct sockaddr *)ut->peer_addr, ut->peer_addr_len);
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
  // Liveness: one tick with no peer traffic; recv() resets this on arrival.
  if (++ut->idle_polls > UDP_TIMEOUT_POLLS) ut->peer_lost = 1;
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
    default:           return false;
  }
}

// Does a received datagram's source address match the peer we've locked onto?
// We only ever speak IPv4 here, so compare family/port/addr explicitly (memcmp
// would also compare sockaddr padding, which isn't guaranteed zeroed).
static bool Udp_AddrMatches(const UdpTransport *ut, const unsigned char *src, socklen_t srclen) {
  if (srclen < (socklen_t)sizeof(struct sockaddr_in)) return false;
  const struct sockaddr_in *a = (const struct sockaddr_in *)ut->peer_addr;
  const struct sockaddr_in *b = (const struct sockaddr_in *)src;
  return a->sin_family == b->sin_family &&
         a->sin_port   == b->sin_port &&
         a->sin_addr.s_addr == b->sin_addr.s_addr;
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
      // Client only (already length/index-validated by Udp_PacketWellFormed).
      if (!ut->is_host && !ut->sram_ok) {
        int idx = pkt[1];
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
      if (pkt[0] != NETPKT_HELLO || pkt[1] != NET_PROTO_VERSION) continue;
      if (srclen > (socklen_t)sizeof(ut->peer_addr)) srclen = sizeof(ut->peer_addr);
      memcpy(ut->peer_addr, src, srclen);
      ut->peer_addr_len = (int)srclen;
      ut->peer_known = 1;
    } else if (!Udp_AddrMatches(ut, src, srclen)) {
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
#endif

#endif  // ZELDA3_MULTIPLAYER
