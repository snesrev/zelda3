// net_udp.h — UDP transport implementing NetTransport for online lockstep.
//
// Lockstep can't proceed without the remote frame, so delivery must be
// loss-proof without TCP. Protocol v2 makes the input channel ack-based:
// every INPUT packet carries the sender's "next remote frame I need" (an ack
// of the peer's stream), and each peer retransmits its own input from the
// peer's last ack forward (a sliding window over a 512-frame history ring).
// That makes delivery self-healing under arbitrary burst loss, lets a client
// that connects late still receive frame 0+, and — via the per-tick poll()
// hook — keeps retransmitting even when local input capture is held (full
// ring / stalled sim), which is exactly when healing is needed most.
// POSIX sockets (Linux/macOS — no extra link lib) and Winsock (Windows —
// requires linking ws2_32; see NET_ONLINE.md). If sockets aren't available
// the init functions return false and the caller stays offline.
#pragma once
#include "net_transport.h"

#ifdef ZELDA3_MULTIPLAYER

// Sender-side input history. Bound: local capture stalls once the local input
// ring (256) is full, and the peer can run at most its own ring (256) plus the
// input delay (<=10) further, so unacked frames never exceed ~512-and-change…
// actually never exceed 511 (see Udp_RecordFrame); 512 slots is exactly enough.
#define UDP_TX_HISTORY       512
#define UDP_MAX_BURST        32          // max InputFrames per INPUT packet
#define UDP_RECV_QUEUE       256         // decomposed inbound InputFrame FIFO
#define NET_PROTO_VERSION    2           // bump on any wire-format change
#define UDP_SYNC_RING        16          // recent local checksums kept for desync compare
#define UDP_TIMEOUT_POLLS    600         // ~10s at 60fps of no packets -> peer lost

// Datagram type byte (first byte of every packet).
#define NETPKT_INPUT    1  // [1][count][ack:4 LE][count*8 InputFrame]  (v2: +ack)
#define NETPKT_HELLO    2  // [2][proto_version][input_delay][got_yours] (v2: +got_yours)
#define NETPKT_SYNC     3  // [3][frame:4 LE][checksum:4 LE]   periodic desync check
#define NETPKT_BYE      4  // [4]                              clean disconnect
#define NETPKT_SRAM_REQ 5  // [5]  client asks the host for its save data (SRAM)
#define NETPKT_SRAM     6  // [6][chunk_idx][total_chunks][512-byte chunk]

// Save-data sync: both peers' sims must load saves from IDENTICAL SRAM, but
// each machine boots with its own saves/sram.dat. At connect the host streams
// its 8KB SRAM to the client (16 chunks, client REQs until complete — loss-
// proof by idempotent re-request); the lockstep holds both sims at frame 0
// until the transfer lands (see NetTransport.ready). All later in-game SRAM
// mutations are deterministic sim code, so one initial sync keeps the whole
// save lifecycle identical on both peers.
#define UDP_SRAM_SIZE    8192
#define UDP_SRAM_CHUNK   512
#define UDP_SRAM_CHUNKS  (UDP_SRAM_SIZE / UDP_SRAM_CHUNK)   // 16
#define UDP_SRAM_REQ_INTERVAL 30   // client re-requests every ~0.5s until done

typedef struct UdpTransport {
  NetTransport iface;
  int sock;                              // socket fd (-1 if not open)
  int valid;                             // initialized ok
  int is_host;
  int peer_known;                        // host learns client addr from its HELLO
  unsigned char peer_addr[28];           // sockaddr_storage-sized blob (opaque here)
  int peer_addr_len;

  // --- connection status (read by the frontend for on-screen UX) ---
  int announced_delay;                   // our input_delay, sent in HELLO
  int handshaked;                        // got a valid peer HELLO
  int version_mismatch;                  // peer HELLO had an incompatible proto version
  int peer_input_delay;                  // input_delay the peer reported
  int peer_lost;                         // peer sent BYE, or no packets for UDP_TIMEOUT_POLLS
  int idle_polls;                        // ticks with no peer traffic -> timeout counter
  int desynced;                          // a remote checksum disagreed with ours
  uint32 desync_frame;                   // frame where the desync was detected

  // outbound input history ring, indexed by frame_number & (UDP_TX_HISTORY-1).
  // Valid range is [peer_ack .. tx_next-1]; the transmit window resends from
  // peer_ack so any gap the peer reports is healed.
  InputFrame hist[UDP_TX_HISTORY];
  uint32 tx_next;                        // next local frame number to record (== frames recorded)
  uint32 peer_ack;                       // next frame of OURS the peer still needs (monotonic)
  uint32 tx_ack;                         // next frame of THEIRS we need (advertised in our packets)
  int sent_since_poll;                   // send() ran since the last poll() (skip duplicate resend)

  // inbound decomposed frame FIFO
  InputFrame rxq[UDP_RECV_QUEUE];
  int rx_head, rx_tail;

  // our recent per-frame checksums, to compare against peer SYNC packets
  uint32 sync_frame[UDP_SYNC_RING];
  uint32 sync_crc[UDP_SYNC_RING];

  // --- save-data (SRAM) sync at connect ---
  const uint8 *sram_src;     // host: the live 8KB SRAM to serve (set by frontend)
  uint8  sram_buf[UDP_SRAM_SIZE];  // client: chunks land here
  uint32 sram_have_mask;     // client: bitmask of received chunks (16 bits)
  int    sram_ok;            // host: always 1; client: 1 once all chunks landed
  int    sram_applied;       // frontend flag: client copied sram_buf into the game
  int    sram_req_timer;     // client: poll countdown between SRAM_REQs
} UdpTransport;

// Host: bind `port` and wait for a client's HELLO (learns its address; only a
// version-matching HELLO can claim the peer slot — stray datagrams can't).
// Client: target host at ip:port and send to it immediately.
// Both return true on success; the NetTransport vtable (ut->iface) is then ready
// to hand to Multiplayer_LockstepInit. `input_delay` is announced in the HELLO.
bool Udp_InitHost(UdpTransport *ut, unsigned short port, int input_delay);
bool Udp_InitClient(UdpTransport *ut, const char *host_ip, unsigned short port, int input_delay);

// Send a clean-disconnect notice to the peer (call on quit).
void Udp_SendBye(UdpTransport *ut);

#ifdef ZELDA3_HEADLESS_TEST
// Test-only: send a raw datagram to 127.0.0.1:port from a throwaway socket.
int Udp_TestRawSendLocal(unsigned short port, const uint8 *buf, int len);
#endif

#endif  // ZELDA3_MULTIPLAYER
