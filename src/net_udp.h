// net_udp.h — UDP transport implementing NetTransport for online lockstep.
//
// One small UDP datagram per frame, each carrying the last few InputFrames for
// loss tolerance (lockstep can't proceed without the remote frame, so we resend
// recent inputs rather than ACK). The receiver de-dups by frame number in the
// lockstep driver. POSIX sockets (Linux/macOS — no extra link lib) and Winsock
// (Windows — requires linking ws2_32; see NET_ONLINE.md). If sockets aren't
// available the init functions return false and the caller stays offline.
#pragma once
#include "net_transport.h"

#ifdef ZELDA3_MULTIPLAYER

#define UDP_REDUNDANT_FRAMES 8           // resend this many recent inputs per packet
#define UDP_RECV_QUEUE       256         // decomposed inbound InputFrame FIFO

typedef struct UdpTransport {
  NetTransport iface;
  int sock;                              // socket fd (-1 if not open)
  int valid;                             // initialized ok
  int is_host;
  int peer_known;                        // host learns client addr from 1st packet
  unsigned char peer_addr[28];           // sockaddr_storage-sized blob (opaque here)
  int peer_addr_len;

  // outbound redundancy ring (last N frames we sent)
  InputFrame hist[UDP_REDUNDANT_FRAMES];
  int hist_count;

  // inbound decomposed frame FIFO
  InputFrame rxq[UDP_RECV_QUEUE];
  int rx_head, rx_tail;
} UdpTransport;

// Host: bind `port` and wait for a client's first packet (learns its address).
// Client: target host at ip:port and send to it immediately.
// Both return true on success; the NetTransport vtable (ut->iface) is then ready
// to hand to Multiplayer_LockstepInit.
bool Udp_InitHost(UdpTransport *ut, unsigned short port);
bool Udp_InitClient(UdpTransport *ut, const char *host_ip, unsigned short port);

#endif  // ZELDA3_MULTIPLAYER
