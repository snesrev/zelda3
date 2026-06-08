// net_udp.c — UDP transport for online input-lockstep (see net_udp.h).
//
// Wire protocol: every datagram begins with a 1-byte type (NETPKT_*). INPUT
// packets carry the last few InputFrames for loss tolerance; HELLO does the
// version/input-delay handshake; SYNC exchanges periodic state checksums for
// desync detection; BYE is a clean disconnect. The lockstep driver still only
// send()s/recv()s InputFrames — the control packets are handled here and surface
// as status fields the frontend reads.
//
// Hardening: UDP is a raw, unauthenticated, anyone-can-send-you-bytes socket, so
// the receive path is strict about what it accepts. Every datagram is bounds-
// checked against its declared type (Udp_PacketWellFormed) BEFORE any field is
// read, oversized reads are clamped, and once we know our peer's address we drop
// anything that doesn't come from it (Udp_AddrMatches) — stray scans, wrong-port
// traffic, and trivially spoofed packets can't inject input, flip status flags,
// or keep a dead link looking alive. (Full anti-spoofing of a forged source addr
// needs crypto and is out of scope; this stops everything short of that.)
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

#define UDP_MAX_PACKET (1 + UDP_REDUNDANT_FRAMES * INPUT_FRAME_WIRE_SIZE)

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
  uint8 p[3] = { NETPKT_HELLO, (uint8)NET_PROTO_VERSION, (uint8)ut->announced_delay };
  Udp_RawSendTo(ut, p, 3);
}

// ---- NetTransport vtable ---------------------------------------------------
static bool Udp_Send(NetTransport *t, const InputFrame *f) {
  UdpTransport *ut = (UdpTransport *)t->impl;
  if (!ut->valid) return false;

  // Keep handshaking until the peer's HELLO arrives.
  if (!ut->handshaked) Udp_SendHello(ut);

  // Append to the redundancy history (keep the last UDP_REDUNDANT_FRAMES).
  if (ut->hist_count < UDP_REDUNDANT_FRAMES) {
    ut->hist[ut->hist_count++] = *f;
  } else {
    memmove(&ut->hist[0], &ut->hist[1], (UDP_REDUNDANT_FRAMES - 1) * sizeof(InputFrame));
    ut->hist[UDP_REDUNDANT_FRAMES - 1] = *f;
  }

  uint8 pkt[UDP_MAX_PACKET];
  pkt[0] = NETPKT_INPUT;
  pkt[1] = (uint8)ut->hist_count;
  for (int i = 0; i < ut->hist_count; i++)
    InputFrame_Serialize(&ut->hist[i], &pkt[2 + i * INPUT_FRAME_WIRE_SIZE]);
  Udp_RawSendTo(ut, pkt, 2 + ut->hist_count * INPUT_FRAME_WIRE_SIZE);

  // Once-per-frame liveness: count up; recv() resets this when a packet arrives.
  if (++ut->idle_polls > UDP_TIMEOUT_POLLS) ut->peer_lost = 1;
  return true;
}

static bool Udp_RxqEmpty(UdpTransport *ut) { return ut->rx_head == ut->rx_tail; }

// Reject anything whose declared type/length is impossible BEFORE we read fields.
// This is the single gate every inbound datagram passes; downstream code may then
// trust the lengths. Returns false for unknown types, short packets, an INPUT
// frame count past the redundancy cap, or an INPUT packet too small for its count.
static bool Udp_PacketWellFormed(const uint8 *pkt, int n) {
  if (n < 1) return false;
  switch (pkt[0]) {
    case NETPKT_INPUT:
      if (n < 2) return false;
      if (pkt[1] > UDP_REDUNDANT_FRAMES) return false;
      return 2 + (int)pkt[1] * INPUT_FRAME_WIRE_SIZE <= n;
    case NETPKT_HELLO: return n >= 3;
    case NETPKT_SYNC:  return n >= 9;
    case NETPKT_BYE:   return true;
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
      if (n < 2) return;
      int count = pkt[1];
      if (count < 0 || count > UDP_REDUNDANT_FRAMES) return;
      if (2 + count * INPUT_FRAME_WIRE_SIZE > n) return;          // truncated
      for (int i = 0; i < count; i++) {
        int nh = (ut->rx_tail + 1) % UDP_RECV_QUEUE;
        if (nh == ut->rx_head) break;                            // FIFO full
        InputFrame_Deserialize(&pkt[2 + i * INPUT_FRAME_WIRE_SIZE], &ut->rxq[ut->rx_tail]);
        ut->rx_tail = nh;
      }
      break;
    }
    case NETPKT_HELLO: {
      if (n < 3) return;
      if (pkt[1] != NET_PROTO_VERSION) { ut->version_mismatch = 1; return; }
      ut->peer_input_delay = pkt[2];
      if (!ut->handshaked) {     // reply once so the peer also receives a HELLO
        ut->handshaked = 1;
        Udp_SendHello(ut);
      }
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

    if (ut->is_host && !ut->peer_known) {       // host locks onto the first valid sender
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
