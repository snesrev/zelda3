// player_state.h — Per-player state for two-player mod
//
// Design constraint: This struct must remain trivially serializable for
// future online multiplayer. Rules:
//   - No pointers (arrays are inline)
//   - No padding-sensitive layouts (use explicit sized types)
//   - All fields are plain integer types
//   - Struct can be memcpy'd, checksummed, or sent over the wire
//
// Phase 1: Extract all link_ globals into a struct, redirect macros through cur_player
#pragma once
#include "types.h"
#include "net_types.h"

// Number of bottle slots and dungeon key slots
#define NUM_BOTTLES 4
#define NUM_DUNGEON_KEY_SLOTS 16

// NOTE (future cross-platform online): this struct mixes uint8/uint16 fields, so
// the compiler inserts implicit padding bytes. Multiplayer_ComputeChecksum CRCs
// sizeof(g_players) INCLUDING that padding, and P2 is cloned from P1 with a
// struct copy — both deterministic within ONE binary, but two peers built with
// different compilers/ABIs could pad differently and disagree. If online ever
// runs across heterogeneous builds, checksum field-by-field (skipping padding)
// or pack this struct. Single-binary lockstep today is unaffected.
typedef struct PlayerState {
  // === Movement & Position (g_ram 0x20–0x69) ===
  uint16 y_coord;                     // 0x20
  uint16 x_coord;                     // 0x22
  uint16 z_coord;                     // 0x24
  uint8  direction_last;              // 0x26
  uint8  actual_vel_y;                // 0x27
  uint8  actual_vel_x;                // 0x28
  uint8  actual_vel_z;                // 0x29
  uint8  subpixel_y;                  // 0x2A
  uint8  subpixel_x;                  // 0x2B
  uint8  subpixel_z;                  // 0x2C
  uint8  counter_var1;                // 0x2D
  uint8  animation_steps;             // 0x2E
  uint8  direction_facing;            // 0x2F
  uint8  y_vel;                       // 0x30
  uint8  x_vel;                       // 0x31
  uint16 y_coord_original;            // 0x32
  uint8  delay_timer_spin_attack;     // 0x3D
  uint8  y_coord_safe_return_lo;      // 0x3E
  uint8  x_coord_safe_return_lo;      // 0x3F
  uint8  y_coord_safe_return_hi;      // 0x40
  uint8  x_coord_safe_return_hi;      // 0x41
  uint8  direction_mask_a;            // 0x42
  uint8  direction_mask_b;            // 0x43
  uint8  incapacitated_timer;         // 0x46
  uint8  visibility_status;           // 0x4B
  uint8  auxiliary_state;             // 0x4D
  uint8  cant_change_direction;       // 0x50
  uint8  cape_mode;                   // 0x55
  uint8  is_bunny;                    // 0x56
  uint8  speed_modifier;              // 0x57
  uint8  this_controls_sprite_oam;    // 0x5A
  uint8  player_handler_state;        // 0x5D
  uint8  speed_setting;               // 0x5E
  uint8  last_direction_moved_towards; // 0x66
  uint8  direction;                   // 0x67
  uint8  y_page_movement_delta;       // 0x68
  uint8  x_page_movement_delta;       // 0x69
  uint8  num_orthogonal_directions;   // 0x6A
  uint8  moving_against_diag_tile;    // 0x6B
  uint8  spin_attack_step_counter;    // 0x79

  // === Quadrant & Layer (0xA9–0xEE) ===
  uint8  quadrant_x;                  // 0xA9
  uint8  quadrant_y;                  // 0xAA
  uint8  is_on_lower_level;           // 0xEE

  // === DMA & Graphics (0x100–0x115) ===
  uint16 dma_graphics_index;          // 0x100
  uint16 dma_var1;                    // 0x102
  uint16 dma_var2;                    // 0x104
  uint8  dma_var3;                    // 0x107
  uint8  dma_var4;                    // 0x108
  uint8  dma_var5;                    // 0x109
  uint8  tile_below;                  // 0x114
  uint8  tile_below_PADDING;          // 0x115

  // === Debug (0x20B) ===
  uint8  debug_value_1;               // 0x20B

  // === Combat & State (0x2C6–0x2FA) ===
  uint8  recoilmode_timer;            // 0x2C6
  uint8  actual_vel_z_copy;           // 0x2C7
  uint8  receiveitem_index;           // 0x2D8
  uint8  receiveitem_var1;            // 0x2D9
  uint8  pose_for_item;               // 0x2DA
  uint8  triggered_by_whirlpool_sprite; // 0x2DB
  uint16 x_coord_copy;               // 0x2DC
  uint16 y_coord_copy;               // 0x2DE
  uint8  is_bunny_mirror;            // 0x2E0
  uint8  is_transforming;            // 0x2E1
  uint8  bunny_transform_timer;      // 0x2E2
  uint8  sword_delay_timer;          // 0x2E3
  uint8  dash_ctr;                   // 0x2F1
  uint8  want_make_noise_when_dashed; // 0x2F8
  uint8  is_near_moveable_statue;    // 0x2FA

  // === Item & Action State (0x301–0x34F) ===
  uint8  item_in_hand;               // 0x301
  uint8  state_bits;                  // 0x308
  uint8  picking_throw_state;        // 0x309
  uint8  var30d;                     // 0x30D
  uint8  var30e;                     // 0x30E
  uint8  spin_offsets;               // 0x31E
  uint8  direction_facing_mirror;    // 0x323
  uint8  maybe_swim_faster;          // 0x32A
  uint8  some_direction_bits;        // 0x340
  uint8  is_in_deep_water;           // 0x345
  uint16 palette_bits_of_oam;        // 0x346
  uint8  flag_moving;                // 0x34A
  uint8  swim_hard_stroke;           // 0x34F
  uint8  debug_value_2;              // 0x350

  // === Physics & Timers (0x360–0x37D) ===
  uint8  electrocute_on_touch;       // 0x360
  uint8  actual_vel_z_mirror;        // 0x362
  uint8  actual_vel_z_copy_mirror;   // 0x363
  uint16 z_coord_mirror;             // 0x364
  uint8  timer_push_get_tired;       // 0x371
  uint8  is_running;                 // 0x372
  uint8  give_damage;                // 0x373
  uint8  countdown_for_dash;         // 0x374
  uint8  timer_jump_ledge;           // 0x375
  uint8  grabbing_wall;              // 0x376
  uint8  unk_master_sword;           // 0x377
  uint8  position_mode;              // 0x37A
  uint8  disable_sprite_damage;      // 0x37B
  uint8  pose_during_opening;        // 0x37D

  // === Hookshot & Special (0x3E9–0x3F8) ===
  uint8  something_with_hookshot;    // 0x3E9
  uint8  force_hold_sword_up;        // 0x3EF
  uint8  on_conveyor_belt;           // 0x3F3
  uint16 timer_tempbunny;            // 0x3F5
  uint8  need_for_poof_for_transform; // 0x3F7
  uint8  need_for_pullforrupees_sprite; // 0x3F8

  // === Mirror & Misc (0x476–0xB7B) ===
  uint8  is_on_lower_level_mirror;   // 0x476
  uint8  lowlife_countdown_timer_beep; // 0x4CA
  uint8  prevent_from_moving;        // 0xB7B

  // === Previous Coords (0xFC2–0xFC4) ===
  uint16 x_coord_prev;              // 0xFC2
  uint16 y_coord_prev;              // 0xFC4

  // === Cached Coords (0xC108–0xC1A8) ===
  uint16 y_coord_spexit;            // 0xC108
  uint16 x_coord_spexit;            // 0xC10A
  uint16 y_coord_exit;              // 0xC148
  uint16 x_coord_exit;              // 0xC14A
  uint16 y_coord_cached;            // 0xC184
  uint16 x_coord_cached;            // 0xC186
  uint8  quadrant_x_cached;         // 0xC19E
  uint8  quadrant_y_cached;         // 0xC19F
  uint8  direction_facing_cached;   // 0xC1A6
  uint8  is_on_lower_level_cached;  // 0xC1A7
  uint8  is_on_lower_level_mirror_cached; // 0xC1A8

  // === Additional per-player Link state (co-op review fixes) ===
  // Link_Main writes these every frame as part of his OWN state, but they were
  // missing here, so the two players shared/corrupted them via g_ram.
  uint8  player_handler_timer;            // 0x300  action/animation timer
  uint8  state_for_spin_attack;           // 0x31C  spin/medallion anim gfx
  uint8  step_counter_for_spin_attack;    // 0x31D  spin anim step counter
  uint8  countdown_for_blink;             // 0x31F  post-hit invulnerability flash
  uint8  player_near_pit_state;           // 0x5B   pit/hole fall state machine
  uint8  player_on_somaria_platform;      // 0x2F5  standing on a Somaria platform
  uint8  flag_is_link_immobilized;        // 0x2E4  control lock
  uint16 kind_of_in_room_staircase;       // 0x44A  dungeon staircase/layer change
  uint8  about_to_jump_off_ledge;         // 0x47A  ledge-jump state
  uint8  flag_unk1;                       // 0xFC1  per-frame scratch flag

  // Additional per-player Link state (co-op review round 2 — also leaked via g_ram)
  uint8  draw_water_ripples_or_grass;     // 0x351  ripple/grass + sword/shadow OAM variant
  uint8  some_animation_timer;            // 0x30B  lift/throw/grab/dash anim timer
  uint8  some_animation_timer_steps;      // 0x30A  lift/throw/grab/dash anim step
  uint8  swimming_countdown;              // 0x2CB  swim-stroke pacing
  uint8  byte_7E02CC;                     // 0x2CC  swim-stroke / swim anim
  uint8  fallhole_var1;                   // 0x302  pit/hole fall state
  uint8  fallhole_var2;                   // 0x2CA  pit/hole fall state
  uint8  byte_7E02C9;                     // 0x2C9  fall direction index
  uint8  related_to_hookshot;             // 0x37E  hookshot grab/pull state
  uint8  is_standing_in_doorway;          // 0x6C   doorway state (gates sword/items)
  uint8  byte_7E0324;                     // 0x324  medallion-cast one-shot guard
  uint8  byte_7E005C;                     // 0x5C   animation-cadence countdown
  uint8  byte_7E0322;                     // 0x322  water-staircase / moving-floor bits
  uint8  gravestone_push_timeout;         // 0x61   block/gravestone push timer
  uint8  byte_7E02C2;                     // 0x2C2  block/gravestone push companion
  uint8  byte_7E02C5;                     // 0x2C5  recoil/knockback anim sub-timer
  uint8  primary_water_grass_timer;       // 0x356  foot ripple/grass anim phase
  uint8  secondary_water_grass_timer;     // 0x355  foot ripple/grass anim phase

  // === Inventory (0xF340–0xF37B) — shared in Phase 1, split later ===
  uint8  item_bow;                   // 0xF340
  uint8  item_boomerang;             // 0xF341
  uint8  item_hookshot;              // 0xF342
  uint8  item_bombs;                 // 0xF343
  uint8  item_mushroom;              // 0xF344
  uint8  item_fire_rod;              // 0xF345
  uint8  item_ice_rod;               // 0xF346
  uint8  item_bombos_medallion;      // 0xF347
  uint8  item_ether_medallion;       // 0xF348
  uint8  item_quake_medallion;       // 0xF349
  uint8  item_torch;                 // 0xF34A
  uint8  item_hammer;                // 0xF34B
  uint8  item_flute;                 // 0xF34C
  uint8  item_bug_net;               // 0xF34D
  uint8  item_book_of_mudora;        // 0xF34E
  uint8  item_bottle_index;          // 0xF34F
  uint8  item_cane_somaria;          // 0xF350
  uint8  item_cane_byrna;            // 0xF351
  uint8  item_cape;                  // 0xF352
  uint8  item_mirror;                // 0xF353
  uint8  item_gloves;                // 0xF354
  uint8  item_boots;                 // 0xF355
  uint8  item_flippers;              // 0xF356
  uint8  item_moon_pearl;            // 0xF357
  uint8  sword_type;                 // 0xF359
  uint8  shield_type;                // 0xF35A
  uint8  armor;                      // 0xF35B
  uint8  bottle_info[NUM_BOTTLES];   // 0xF35C–0xF35F
  uint16 rupees_goal;                // 0xF360
  uint16 rupees_actual;              // 0xF362
  uint16 compass;                    // 0xF364
  uint16 bigkey;                     // 0xF366
  uint16 dungeon_map;                // 0xF368
  uint8  rupees_in_pond;             // 0xF36A
  uint8  heart_pieces;               // 0xF36B
  uint8  health_capacity;            // 0xF36C
  uint8  health_current;             // 0xF36D
  uint8  magic_power;                // 0xF36E
  uint8  num_keys;                   // 0xF36F
  uint8  bomb_upgrades;              // 0xF370
  uint8  arrow_upgrades;             // 0xF371
  uint8  hearts_filler;              // 0xF372
  uint8  magic_filler;               // 0xF373
  uint8  which_pendants;             // 0xF374
  uint8  bomb_filler;                // 0xF375
  uint8  arrow_filler;               // 0xF376
  uint8  num_arrows;                 // 0xF377
  uint8  ability_flags;              // 0xF379
  uint8  has_crystals;               // 0xF37A
  uint8  magic_consumption;          // 0xF37B
  uint8  keys_earned_per_dungeon[NUM_DUNGEON_KEY_SLOTS]; // 0xF37C+

  // === Per-player non-link_ globals ===
  uint8  button_mask_b_y;            // 0x3A
  uint8  bitfield_for_a_button;      // 0x3B
  uint8  button_b_frames;            // 0x3C
  uint8  oam_y_offset;               // 0x44  (player_oam_y_offset)
  uint8  oam_x_offset;               // 0x45  (player_oam_x_offset)
  uint8  set_when_damaging_enemies;  // 0x47
  uint8  bitmask_of_dragstate;       // 0x48
  uint16 force_move_any_direction;   // 0x49
  uint8  cape_decrement_counter;     // 0x4C
  uint8  index_of_dashing_sfx;       // 0x4F

  // === Per-player input state ===
  uint8  joypad1H_last;              // 0xF0
  uint8  joypad1L_last;              // 0xF2
  uint8  filtered_joypad_H;          // 0xF4
  uint8  filtered_joypad_L;          // 0xF6
  uint8  joypad1H_last2;             // 0xF8
  uint8  joypad1L_last2;             // 0xFA

  // === Co-Op Multiplayer State (new, not from SNES RAM) ===
  uint8  player_index;               // 0 = P1, 1 = P2
  uint8  is_active;                  // Is this player slot in use?
  uint8  is_dead;                    // In ghost/respawn state?
  uint8  respawn_timer;              // Frames until respawn (0 = alive)
  uint8  is_ghost;                   // Translucent, no collision, waiting to respawn
  uint8  palette_index;              // 0 = green (P1), 1 = purple/red (P2)
} PlayerState;

// Global pointer to current player being processed.
// All link_ macros dereference through this pointer.
extern PlayerState *cur_player;

// Both player states
extern PlayerState g_players[MAX_PLAYERS];

// Runtime multiplayer configuration
extern MultiplayerConfig g_mp_config;

// Master co-op toggle (defined in zelda_rtl.c). When false, P2 is never
// spawned/updated and the game behaves as ordinary single-player.
extern bool g_mp_p2_enabled;

// ============================================================================
// P2 sprite graphics. The NMI streams P1's current pose graphics into the
// fixed sprite chars Link's OAM entries reference (body 0x00-0x04/0x10-0x14,
// sword 0x05-0x06/0x15-0x16, shield 0x07-0x08/0x17-0x18), so a second Link
// drawn with those chars would mirror P1's animation ("walks in place").
// P2 therefore gets its own char region — the otherwise unused chars
// 0x26-0x2f/0x36-0x3f, skipping 0x28/0x38 which the alternate Link shadow
// uses: the NMI uploads P2's own pose graphics there (see NMI_DoUpdates) and
// P2's OAM char numbers are remapped into the region after its LinkOam_Main.
// ============================================================================

// P2's DMA source addresses, captured after P2's LinkOam_Main each frame:
// [0..5] body/head rows (kLinkGraphics offsets), [6..9] sword/shield rows
// (g_ram offsets). Filled by Multiplayer_ComputeP2DmaAddrs (misc.c).
extern uint16 g_p2_dma_addrs[10];

// True when P2 was drawn this frame and the NMI should upload its graphics.
extern bool g_p2_draw_active;

// Fills g_p2_dma_addrs from cur_player's dma variables. Must be called with
// cur_player == P2, right after P2's LinkOam_Main. Defined in misc.c next to
// the DMA source tables.
void Multiplayer_ComputeP2DmaAddrs(void);

// Co-op death/respawn.
// Multiplayer_PreventGameOver() is called at the engine's game-over points (when
// a player reaches 0 health). If co-op is active and the OTHER player is still
// up, it puts the current player (cur_player) into a downed "ghost" state and
// returns true so the caller skips the game-over; only a simultaneous double-KO
// returns false (real game over). Multiplayer_UpdateDeathRespawn() runs once per
// frame to tick each downed player's respawn timer and revive it beside the
// living partner.
bool Multiplayer_PreventGameOver(void);
void Multiplayer_UpdateDeathRespawn(void);

// Called when a save is (re)loaded into WRAM (file-select load or post-death
// Continue) so P1 is re-seeded from the loaded state and P2 is re-spawned to
// match. Defined in zelda_rtl.c.
void Multiplayer_OnSaveLoaded(void);

// Copy the SHARED inventory (items, equipment, rupees, keys, bombs, arrows,
// abilities, progression) from one player to the other, preserving the
// destination's per-player health/magic. Keeps both players' item pools in sync.
void Multiplayer_ShareInventory(PlayerState *from, PlayerState *to);

// Initialize player state (copies from g_ram for P1, sets defaults for P2)
void PlayerState_Init(int player_index);

// Sync cur_player fields back to g_ram (needed for systems that still read g_ram directly)
void PlayerState_SyncToRam(void);

// Sync g_ram into cur_player (needed after systems that write to g_ram directly)
void PlayerState_SyncFromRam(void);

// Set the active player for processing
void PlayerState_SetCurrent(int player_index);

// ============================================================================
// Input ring buffer — stores InputFrames for both local and online use.
//
// For local: frames are consumed immediately (delay = 0).
// For online lockstep: frames are buffered by input_delay_frames. The game
// only advances when both players' inputs are available for that frame.
// For replay: frames are read from a file into this buffer.
// ============================================================================
#define INPUT_RING_SIZE 256  // Power of 2, ~4 seconds at 60fps

typedef struct InputRingBuffer {
  InputFrame frames[INPUT_RING_SIZE];
  uint32 write_pos;
  uint32 read_pos;
} InputRingBuffer;

// One ring buffer per player
extern InputRingBuffer g_input_rings[MAX_PLAYERS];

bool InputRing_Push(int player_index, const InputFrame *frame);
bool InputRing_Pop(int player_index, InputFrame *out);
bool InputRing_Peek(int player_index, uint32 frame_number, InputFrame *out);
uint32 InputRing_Count(int player_index);

// ============================================================================
// Frame management — coordinates the game loop with multiplayer
// ============================================================================

// Current simulation frame number (monotonically increasing)
extern uint32 g_sim_frame;

// Returns true if both players' inputs are available for g_sim_frame
bool Multiplayer_InputsReady(void);

// Get the combined joypad inputs for this frame (call after InputsReady)
FrameInputPair Multiplayer_ConsumeInputs(void);

// Reset the lockstep input rings + frame counter (call when starting a session)
void Multiplayer_ResetLockstep(void);

// Compute a checksum of g_ram for desync detection
SyncChecksum Multiplayer_ComputeChecksum(void);

// OAM offset for P2 rendering (used by player_oam.c)
#define P2_OAM_OFFSET 0x40

// ============================================================================
// Phase 6: Online Multiplayer Architecture Stubs
// These are interface stubs for future online play. Do NOT implement networking.
// ============================================================================

// State snapshot for rollback netcode (future).
// Would memcpy g_ram + PlayerState for both players.
void GameState_Snapshot(void *out_buffer, uint32 *out_size);
void GameState_Restore(const void *buffer, uint32 size);

// Replay recording (future).
// Recording = writing InputFrames to a file each frame.
// Playback = reading InputFrames from a file into InputRingBuffer.
void Replay_RecordFrame(uint32 frame_number, const InputFrame *p1, const InputFrame *p2);
void Replay_PlaybackFrame(uint32 frame_number, InputFrame *out_p1, InputFrame *out_p2);

// Determinism verification (debug mode).
// Runs two game instances with identical inputs and compares g_ram.
// Call every N frames (e.g., 60) to detect desync bugs.
void Debug_VerifyDeterminism(void);
