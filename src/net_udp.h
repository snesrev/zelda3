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
#define NET_PROTO_VERSION    1           // bump on any wire-format change
#define UDP_SYNC_RING        16          // recent local checksums kept for desync compare
#define UDP_TIMEOUT_POLLS    600         // ~10s at 60fps of no packets -> peer lost

// Datagram type byte (first byte of every packet).
#define NETPKT_INPUT 1   // [1][count][count*8 InputFrame]
#define NETPKT_HELLO 2   // [2][proto_version][input_delay]
#define NETPKT_SYNC  3   // [3][frame:4 LE][checksum:4 LE]   periodic desync check
#define NETPKT_BYE   4   // [4]                              clean disconnect

typedef struct UdpTransport {
  NetTransport iface;
  int sock;                              // socket fd (-1 if not open)
  int valid;                             // initialized ok
  int is_host;
  int peer_known;                        // host learns client addr from 1st packet
  unsigned char peer_addr[28];           // sockaddr_storage-sized blob (opaque here)
  int peer_addr_len;

  // --- connection status (read by the frontend for on-screen UX) ---
  int announced_delay;                   // our input_delay, sent in HELLO
  int handshaked;                        // got a valid peer HELLO
  int version_mismatch;                  // peer HELLO had an incompatible proto version
  int peer_input_delay;                  // input_delay the peer reported
  int peer_lost;                         // peer sent BYE, or no packets for UDP_TIMEOUT_POLLS
  int idle_polls;                        // recvs with nothing -> timeout counter
  int desynced;                          // a remote checksum disagreed with ours
  uint32 desync_frame;                   // frame where the desync was detected

  // outbound redundancy ring (last N frames we sent)
  InputFrame hist[UDP_REDUNDANT_FRAMES];
  int hist_count;

  // inbound decomposed frame FIFO
  InputFrame rxq[UDP_RECV_QUEUE];
  int rx_head, rx_tail;

  // our recent per-frame checksums, to compare against peer SYNC packets
  uint32 sync_frame[UDP_SYNC_RING];
  uint32 sync_crc[UDP_SYNC_RING];
} UdpTransport;

// Host: bind `port` and wait for a client's first packet (learns its address).
// Client: target host at ip:port and send to it immediately.
// Both return true on success; the NetTransport vtable (ut->iface) is then ready
// to hand to Multiplayer_LockstepInit. `input_delay` is announced in the HELLO.
bool Udp_InitHost(UdpTransport *ut, unsigned short port, int input_delay);
bool Udp_InitClient(UdpTransport *ut, const char *host_ip, unsigned short port, int input_delay);

// Send a clean-disconnect notice to the peer (call on quit).
void Udp_SendBye(UdpTransport *ut);

#endif  // ZELDA3_MULTIPLAYER
