// net_transport.h — Transport abstraction for online input-lockstep.
//
// The lockstep driver (zelda_rtl.c) is transport-agnostic: it exchanges
// InputFrames through this interface. Local co-op never uses it. Real online
// drops a UDP implementation in here; everything above it is unchanged.
//
// All methods are NON-BLOCKING. send() queues/transmits one InputFrame; recv()
// returns the next available remote InputFrame if one has arrived. Frames travel
// in the 8-byte wire format (InputFrame_Serialize), so anything tested through
// the loopback transport exercises the exact bytes a socket would carry.
#pragma once
#include "net_types.h"

#ifdef ZELDA3_MULTIPLAYER

typedef struct NetTransport {
  bool (*send)(struct NetTransport *t, const InputFrame *f);
  bool (*recv)(struct NetTransport *t, InputFrame *out);  // non-blocking
  void (*close)(struct NetTransport *t);
  // Optional (may be NULL, e.g. loopback): exchange a periodic state checksum so
  // the transport can flag a desync. Called by the lockstep driver every so many
  // frames with the local sim's checksum for that frame.
  void (*send_sync)(struct NetTransport *t, uint32 frame, uint32 checksum);
  // Optional (may be NULL, e.g. loopback): per-tick upkeep, called once per
  // display tick BEFORE send/recv with `ack_frame` = the next remote frame the
  // lockstep driver still needs. A lossy transport uses this to advertise the
  // ack, retransmit un-acked local input (decoupled from input capture, which
  // stalls exactly when healing is needed), keep the handshake alive, and run
  // its liveness timeout.
  void (*poll)(struct NetTransport *t, uint32 ack_frame);
  // Optional (may be NULL = always ready): is the session established enough to
  // start capturing/sending local input? While false the lockstep driver holds
  // capture, so BOTH sims sit at frame 0 and begin together once the transport
  // is ready (UDP: handshake complete + the host's save data received). This is
  // what makes "host launches, client connects a minute later" start cleanly.
  int (*ready)(struct NetTransport *t);
  void *impl;
} NetTransport;

// In-process loopback transport — for testing the lockstep pipeline with no real
// network (lossless, zero-latency), and as the reference impl a UDP transport
// must behave like. Backed by a byte FIFO of serialized frames, so it exercises
// the real wire format.
#define LOOPBACK_FIFO_BYTES (INPUT_FRAME_WIRE_SIZE * 1024)
typedef struct LoopbackFifo {
  uint8 buf[LOOPBACK_FIFO_BYTES];
  uint32 head, tail;  // byte indices (monotonic, masked on use)
} LoopbackFifo;

typedef struct LoopbackTransport {
  NetTransport iface;
  LoopbackFifo *out;  // frames this endpoint sends go here
  LoopbackFifo *in;   // frames this endpoint receives come from here
  LoopbackFifo fifo_a, fifo_b;  // storage (used when self-paired)
} LoopbackTransport;

// Initialize a standalone loopback endpoint: send() writes to its own out-FIFO,
// recv() reads from its in-FIFO. Use Loopback_Inject() to feed it "remote"
// frames (e.g. a scripted opponent in tests).
void Loopback_Init(LoopbackTransport *lt);

// Inject a frame into the endpoint's in-FIFO as if the remote peer sent it.
void Loopback_Inject(LoopbackTransport *lt, const InputFrame *f);

// Cross-connect two endpoints so a->send is b->recv and vice versa (two peers
// in one process). Storage lives in `a`.
void Loopback_CreatePair(LoopbackTransport *a, LoopbackTransport *b);

// ============================================================================
// Lockstep driver (defined in zelda_rtl.c).
//
// Drives the deterministic simulation from local input + a transport carrying
// the remote player's input. Both peers run the identical ZeldaRunFrame(p1,p2)
// from the identical input pair, so they stay in sync; only the 8-byte
// InputFrames cross the wire. `input_delay` keeps the sim that many frames
// behind input capture, giving the network slack to deliver the remote frame.
// ============================================================================
void Multiplayer_LockstepInit(NetTransport *t, int local_player_index, int input_delay);

// Call once per display tick with the local player's joypad. Sends the local
// input, ingests any remote input, and advances the sim for every frame that's
// now ready. Returns the number of simulation frames advanced this tick (0 if
// stalled waiting on the remote peer).
int Multiplayer_LockstepTick(uint16 local_joypad);

// Transport upkeep WITHOUT input capture or sim advance, for display ticks
// where the frontend skips the lockstep entirely (the pause screen): keeps
// acks/retransmission/keepalives and the liveness timeout flowing, so a
// machine paused for >10s reads as "waiting for player" on its peer instead
// of falsely tripping "player disconnected". Safe any time; no-op offline.
void Multiplayer_NetIdle(void);

#endif  // ZELDA3_MULTIPLAYER
