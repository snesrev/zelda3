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

#endif  // ZELDA3_MULTIPLAYER
