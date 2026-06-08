// mp_dual.h — apply a position-based, per-player effect to BOTH co-op players.
//
// Many enemy/hazard handlers apply contact damage inline in their AI by testing
// the CURRENT player's coords (link_x_coord/link_y_coord) and writing the
// current player's hurt vars. During the sprite-AI pass cur_player is always
// Player 1, so those inline checks never see Player 2 — which made P2 immune to
// a whole class of bosses and hazards.
//
// MP_ALSO_FOR_P2(stmt) re-runs `stmt` with cur_player switched to Player 2 (only
// when P2 exists, is active, and is alive — ghosts are intangible), then
// restores the current player. This mirrors the shipped bomb pattern in
// ancilla.c (Bomb_CheckSpriteAndPlayerDamage). `stmt` must be a self-contained,
// RNG-free, per-current-player check; re-running it is therefore deterministic.
// In a vanilla build MP_ALSO_FOR_P2 expands to nothing, so the original
// single-player path is byte-for-byte unchanged.
#ifndef ZELDA3_MP_DUAL_H_
#define ZELDA3_MP_DUAL_H_

#ifdef ZELDA3_MULTIPLAYER
#include "player_state.h"

#define MP_ALSO_FOR_P2(stmt)                                             \
  do {                                                                   \
    if (g_mp_p2_enabled && g_players[1].is_active && !g_players[1].is_dead) { \
      PlayerState *mp_saved_player_ = cur_player;                        \
      PlayerState_SetCurrent(1);                                         \
      stmt;                                                              \
      cur_player = mp_saved_player_;                                     \
    }                                                                    \
  } while (0)

#else
#define MP_ALSO_FOR_P2(stmt) do {} while (0)
#endif

#endif  // ZELDA3_MP_DUAL_H_
