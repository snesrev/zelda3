// net_transport.c — Loopback transport implementation (see net_transport.h).
#include "net_transport.h"

#ifdef ZELDA3_MULTIPLAYER
#include <string.h>

// --- byte FIFO of serialized InputFrames -----------------------------------
static bool Fifo_Push(LoopbackFifo *f, const InputFrame *frame) {
  if (f->tail - f->head + INPUT_FRAME_WIRE_SIZE > LOOPBACK_FIFO_BYTES)
    return false;  // full (frame would be dropped — like a saturated socket)
  uint8 wire[INPUT_FRAME_WIRE_SIZE];
  InputFrame_Serialize(frame, wire);
  for (int i = 0; i < INPUT_FRAME_WIRE_SIZE; i++)
    f->buf[(f->tail + i) % LOOPBACK_FIFO_BYTES] = wire[i];
  f->tail += INPUT_FRAME_WIRE_SIZE;
  return true;
}

static bool Fifo_Pop(LoopbackFifo *f, InputFrame *out) {
  if (f->tail - f->head < INPUT_FRAME_WIRE_SIZE)
    return false;  // empty
  uint8 wire[INPUT_FRAME_WIRE_SIZE];
  for (int i = 0; i < INPUT_FRAME_WIRE_SIZE; i++)
    wire[i] = f->buf[(f->head + i) % LOOPBACK_FIFO_BYTES];
  InputFrame_Deserialize(wire, out);
  f->head += INPUT_FRAME_WIRE_SIZE;
  return true;
}

// --- NetTransport vtable ----------------------------------------------------
static bool Loopback_Send(NetTransport *t, const InputFrame *f) {
  LoopbackTransport *lt = (LoopbackTransport *)t->impl;
  return Fifo_Push(lt->out, f);
}

static bool Loopback_Recv(NetTransport *t, InputFrame *out) {
  LoopbackTransport *lt = (LoopbackTransport *)t->impl;
  return Fifo_Pop(lt->in, out);
}

static void Loopback_Close(NetTransport *t) { (void)t; }

static void Loopback_Wire(LoopbackTransport *lt) {
  lt->iface.send = Loopback_Send;
  lt->iface.recv = Loopback_Recv;
  lt->iface.close = Loopback_Close;
  lt->iface.impl = lt;
}

void Loopback_Init(LoopbackTransport *lt) {
  memset(lt, 0, sizeof(*lt));
  lt->out = &lt->fifo_a;
  lt->in = &lt->fifo_b;
  Loopback_Wire(lt);
}

void Loopback_Inject(LoopbackTransport *lt, const InputFrame *f) {
  Fifo_Push(lt->in, f);
}

void Loopback_CreatePair(LoopbackTransport *a, LoopbackTransport *b) {
  memset(a, 0, sizeof(*a));
  memset(b, 0, sizeof(*b));
  // a sends into fifo_a (which b receives); b sends into fifo_b (which a receives).
  a->out = &a->fifo_a;  a->in = &a->fifo_b;
  b->out = &a->fifo_b;  b->in = &a->fifo_a;
  Loopback_Wire(a);
  Loopback_Wire(b);
}

#endif  // ZELDA3_MULTIPLAYER
