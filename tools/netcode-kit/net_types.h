// net_types.h — Core types for multiplayer (local + future online)
//
// Design principle: The game simulation is deterministic. Given identical
// inputs on the same frame, two machines produce identical state. Therefore:
//   - Network exchange unit = InputFrame (tiny, ~4 bytes + header)
//   - Sync verification = periodic CRC32 of g_ram
//   - No game state is ever sent over the wire (only inputs)
//
// This file is included regardless of local vs online mode.
#pragma once
#include "types.h"

#define MAX_PLAYERS 2

// ============================================================================
// InputFrame — The atomic unit of player input per frame.
//
// For local co-op: both InputFrames come from the local SDL event loop.
// For online: your InputFrame is local, opponent's arrives over the network.
// For replay: InputFrames are read from a file.
//
// This struct must be trivially serializable (no pointers, fixed size).
// ============================================================================
typedef struct InputFrame {
  uint32 frame_number;        // Monotonic frame counter (for ordering & sync)
  uint16 joypad;              // 16-bit SNES joypad state (buttons + dpad)
  uint8  player_index;        // 0 or 1
  uint8  flags;               // Reserved (disconnect signal, etc.)
} InputFrame;

// InputFrame flag bits
#define INPUT_FLAG_NONE         0x00
#define INPUT_FLAG_DISCONNECT   0x01  // Player has disconnected
#define INPUT_FLAG_PAUSE_REQ    0x02  // Player requesting pause
#define INPUT_FLAG_DESYNC_ACK   0x04  // Acknowledging a desync was detected

// ============================================================================
// FrameInputPair — Both players' inputs for a single frame.
// The game loop consumes one of these per tick.
// ============================================================================
typedef struct FrameInputPair {
  uint32 frame_number;
  uint16 joypad[MAX_PLAYERS];
} FrameInputPair;

// ============================================================================
// SyncChecksum — Periodic integrity check.
// Both machines compute CRC32 of g_ram and compare.
// Mismatch = desync = bug in our determinism guarantee.
// Sent every N frames (e.g., every 60 = once per second).
// ============================================================================
typedef struct SyncChecksum {
  uint32 frame_number;
  uint32 checksum;            // CRC32 of g_ram[0..0x1FFFF]
} SyncChecksum;

#define SYNC_CHECKSUM_INTERVAL 60  // Frames between checksums

// ============================================================================
// MultiplayerMode — How multiplayer is operating
// ============================================================================
typedef enum {
  MP_MODE_SINGLE = 0,   // Original single-player (no multiplayer code active)
  MP_MODE_LOCAL,         // Local co-op (two inputs, one machine)
  MP_MODE_HOST,          // Online: this machine is the host
  MP_MODE_CLIENT,        // Online: this machine is the client
  MP_MODE_REPLAY,        // Playing back recorded InputFrames
} MultiplayerMode;

// ============================================================================
// MultiplayerConfig — Runtime configuration
// ============================================================================
typedef struct MultiplayerConfig {
  MultiplayerMode mode;
  uint8  num_players;          // 1 or 2
  uint8  local_player_index;   // Which player are we? (0 for host, 1 for client)
  uint8  input_delay_frames;   // Lockstep delay buffer (2-4 for online, 0 for local)

  // Online-specific (unused in local mode)
  // char   host_address[256];
  // uint16 port;
} MultiplayerConfig;

// ============================================================================
// Serialization helpers — InputFrame is the only thing sent over the wire.
// Fixed-size, packed, platform-independent byte order.
// ============================================================================

// InputFrame wire format: 8 bytes, little-endian
// [0-3] frame_number (uint32 LE)
// [4-5] joypad       (uint16 LE)
// [6]   player_index
// [7]   flags
#define INPUT_FRAME_WIRE_SIZE 8

static inline void InputFrame_Serialize(const InputFrame *f, uint8 *buf) {
  buf[0] = (uint8)(f->frame_number);
  buf[1] = (uint8)(f->frame_number >> 8);
  buf[2] = (uint8)(f->frame_number >> 16);
  buf[3] = (uint8)(f->frame_number >> 24);
  buf[4] = (uint8)(f->joypad);
  buf[5] = (uint8)(f->joypad >> 8);
  buf[6] = f->player_index;
  buf[7] = f->flags;
}

static inline void InputFrame_Deserialize(const uint8 *buf, InputFrame *f) {
  f->frame_number = buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);
  f->joypad = buf[4] | (buf[5] << 8);
  f->player_index = buf[6];
  f->flags = buf[7];
}

// ============================================================================
// CRC32 — For desync detection. Lightweight, no library dependency.
// Compute over g_ram to verify both machines agree on game state.
// ============================================================================
uint32 ComputeCRC32(const uint8 *data, size_t length);
