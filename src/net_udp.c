// net_udp.c — UDP transport for online input-lockstep (see net_udp.h).
#include "net_udp.h"

#ifdef ZELDA3_MULTIPLAYER
#include <string.h>
#include <stdio.h>

#if defined(_WIN32)
  #include <winsock2.h>
  #include <ws2tcpip.h>
  // Link ws2_32 (TCC: pragma works; otherwise add -lws2_32 to the build).
  #pragma comment(lib, "ws2_32")
  typedef int socklen_t;
  #define CLOSESOCK closesocket
  #define SOCK_WOULDBLOCK (WSAGetLastError() == WSAEWOULDBLOCK)
  static int g_wsa_started = 0;
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <unistd.h>
  #include <fcntl.h>
  #include <errno.h>
  #define CLOSESOCK close
  #define SOCK_WOULDBLOCK (errno == EWOULDBLOCK || errno == EAGAIN)
  #define INVALID_SOCKET (-1)
#endif

// One datagram: [uint8 count][count * 8-byte InputFrame], oldest frame first.
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

// ---- NetTransport vtable ---------------------------------------------------
static bool Udp_Send(NetTransport *t, const InputFrame *f) {
  UdpTransport *ut = (UdpTransport *)t->impl;
  if (!ut->valid) return false;

  // Append to the redundancy history (keep the last UDP_REDUNDANT_FRAMES).
  if (ut->hist_count < UDP_REDUNDANT_FRAMES) {
    ut->hist[ut->hist_count++] = *f;
  } else {
    memmove(&ut->hist[0], &ut->hist[1], (UDP_REDUNDANT_FRAMES - 1) * sizeof(InputFrame));
    ut->hist[UDP_REDUNDANT_FRAMES - 1] = *f;
  }

  // The host can't transmit until it has learned the client's address.
  if (ut->is_host && !ut->peer_known) return true;

  uint8 pkt[UDP_MAX_PACKET];
  pkt[0] = (uint8)ut->hist_count;
  for (int i = 0; i < ut->hist_count; i++)
    InputFrame_Serialize(&ut->hist[i], &pkt[1 + i * INPUT_FRAME_WIRE_SIZE]);
  int len = 1 + ut->hist_count * INPUT_FRAME_WIRE_SIZE;
  sendto(ut->sock, (const char *)pkt, len, 0,
         (struct sockaddr *)ut->peer_addr, ut->peer_addr_len);
  return true;
}

static bool Udp_RxqEmpty(UdpTransport *ut) { return ut->rx_head == ut->rx_tail; }

static bool Udp_Recv(NetTransport *t, InputFrame *out) {
  UdpTransport *ut = (UdpTransport *)t->impl;
  if (!ut->valid) return false;

  // If the decomposed FIFO is empty, pull one datagram (non-blocking).
  if (Udp_RxqEmpty(ut)) {
    uint8 pkt[UDP_MAX_PACKET];
    unsigned char src[28];
    socklen_t srclen = sizeof(src);
    int n = recvfrom(ut->sock, (char *)pkt, sizeof(pkt), 0,
                     (struct sockaddr *)src, &srclen);
    if (n <= 0) return false;          // nothing pending (or error) — non-blocking
    // Host learns / locks onto the client's address from its first packet.
    if (ut->is_host && !ut->peer_known) {
      memcpy(ut->peer_addr, src, srclen);
      ut->peer_addr_len = (int)srclen;
      ut->peer_known = 1;
    }
    int count = pkt[0];
    if (count < 0 || count > UDP_REDUNDANT_FRAMES) return false;
    if (1 + count * INPUT_FRAME_WIRE_SIZE > n) return false;  // truncated
    for (int i = 0; i < count; i++) {
      int nh = (ut->rx_tail + 1) % UDP_RECV_QUEUE;
      if (nh == ut->rx_head) break;    // FIFO full — drop extras (recovered later)
      InputFrame_Deserialize(&pkt[1 + i * INPUT_FRAME_WIRE_SIZE], &ut->rxq[ut->rx_tail]);
      ut->rx_tail = nh;
    }
    if (Udp_RxqEmpty(ut)) return false;
  }

  *out = ut->rxq[ut->rx_head];
  ut->rx_head = (ut->rx_head + 1) % UDP_RECV_QUEUE;
  return true;
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
  ut->iface.impl = ut;
}

// ---- init ------------------------------------------------------------------
static bool Udp_Common(UdpTransport *ut) {
  memset(ut, 0, sizeof(*ut));
  ut->sock = INVALID_SOCKET;
  if (!Udp_PlatformInit()) return false;
  ut->sock = (int)socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (ut->sock == INVALID_SOCKET) return false;
  Udp_SetNonBlocking(ut->sock);
  Udp_Wire(ut);
  return true;
}

bool Udp_InitHost(UdpTransport *ut, unsigned short port) {
  if (!Udp_Common(ut)) return false;
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

bool Udp_InitClient(UdpTransport *ut, const char *host_ip, unsigned short port) {
  if (!Udp_Common(ut)) return false;
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

#endif  // ZELDA3_MULTIPLAYER
