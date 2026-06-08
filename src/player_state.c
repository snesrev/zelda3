// player_state.c — Per-player state management for two-player mod
#include "player_state.h"
#include <string.h>
#include <assert.h>

// The RAM array (defined elsewhere)
extern uint8 g_ram[0x20000];

// Global player instances
PlayerState g_players[MAX_PLAYERS];

// Active player pointer — all link_ macros resolve through this
PlayerState *cur_player = &g_players[0];

void PlayerState_SetCurrent(int player_index) {
  assert(player_index >= 0 && player_index < MAX_PLAYERS);
  cur_player = &g_players[player_index];
}

// ============================================================================
// Sync helpers: bridge between PlayerState struct and the flat g_ram array.
//
// During Phase 1, many subsystems still read/write g_ram directly (sprites,
// dungeon logic, etc.). These functions keep them in sync.
//
// The goal is to eventually eliminate these once all subsystems use cur_player.
// ============================================================================

// Macro to define a sync pair for a uint8 field
#define SYNC_U8(field, offset) \
  do { \
    if (to_ram) g_ram[(offset)] = cur_player->field; \
    else cur_player->field = g_ram[(offset)]; \
  } while(0)

// Macro to define a sync pair for a uint16 field
#define SYNC_U16(field, offset) \
  do { \
    if (to_ram) { \
      g_ram[(offset)]   = (uint8)(cur_player->field & 0xFF); \
      g_ram[(offset)+1] = (uint8)(cur_player->field >> 8); \
    } else { \
      cur_player->field = g_ram[(offset)] | (g_ram[(offset)+1] << 8); \
    } \
  } while(0)

static void PlayerState_Sync(bool to_ram) {
  // Movement & Position
  SYNC_U16(y_coord,                    0x20);
  SYNC_U16(x_coord,                    0x22);
  SYNC_U16(z_coord,                    0x24);
  SYNC_U8 (direction_last,             0x26);
  SYNC_U8 (actual_vel_y,               0x27);
  SYNC_U8 (actual_vel_x,               0x28);
  SYNC_U8 (actual_vel_z,               0x29);
  SYNC_U8 (subpixel_y,                 0x2A);
  SYNC_U8 (subpixel_x,                 0x2B);
  SYNC_U8 (subpixel_z,                 0x2C);
  SYNC_U8 (counter_var1,               0x2D);
  SYNC_U8 (animation_steps,            0x2E);
  SYNC_U8 (direction_facing,           0x2F);
  SYNC_U8 (y_vel,                       0x30);
  SYNC_U8 (x_vel,                       0x31);
  SYNC_U16(y_coord_original,            0x32);
  SYNC_U8 (delay_timer_spin_attack,     0x3D);
  SYNC_U8 (y_coord_safe_return_lo,      0x3E);
  SYNC_U8 (x_coord_safe_return_lo,      0x3F);
  SYNC_U8 (y_coord_safe_return_hi,      0x40);
  SYNC_U8 (x_coord_safe_return_hi,      0x41);
  SYNC_U8 (direction_mask_a,            0x42);
  SYNC_U8 (direction_mask_b,            0x43);
  SYNC_U8 (incapacitated_timer,         0x46);
  SYNC_U8 (visibility_status,           0x4B);
  SYNC_U8 (auxiliary_state,             0x4D);
  SYNC_U8 (cant_change_direction,       0x50);
  SYNC_U8 (cape_mode,                   0x55);
  SYNC_U8 (is_bunny,                    0x56);
  SYNC_U8 (speed_modifier,              0x57);
  SYNC_U8 (this_controls_sprite_oam,    0x5A);
  SYNC_U8 (player_handler_state,        0x5D);
  SYNC_U8 (speed_setting,               0x5E);
  SYNC_U8 (last_direction_moved_towards, 0x66);
  SYNC_U8 (direction,                   0x67);
  SYNC_U8 (y_page_movement_delta,       0x68);
  SYNC_U8 (x_page_movement_delta,       0x69);
  SYNC_U8 (num_orthogonal_directions,   0x6A);
  SYNC_U8 (moving_against_diag_tile,    0x6B);
  SYNC_U8 (spin_attack_step_counter,    0x79);

  // Quadrant & Layer
  SYNC_U8 (quadrant_x,                  0xA9);
  SYNC_U8 (quadrant_y,                  0xAA);
  SYNC_U8 (is_on_lower_level,           0xEE);

  // DMA & Graphics
  SYNC_U16(dma_graphics_index,          0x100);
  SYNC_U16(dma_var1,                    0x102);
  SYNC_U16(dma_var2,                    0x104);
  SYNC_U8 (dma_var3,                    0x107);
  SYNC_U8 (dma_var4,                    0x108);
  SYNC_U8 (dma_var5,                    0x109);
  SYNC_U8 (tile_below,                  0x114);
  SYNC_U8 (tile_below_PADDING,          0x115);

  // Debug
  SYNC_U8 (debug_value_1,               0x20B);

  // Combat & State
  SYNC_U8 (recoilmode_timer,            0x2C6);
  SYNC_U8 (actual_vel_z_copy,           0x2C7);
  SYNC_U8 (receiveitem_index,           0x2D8);
  SYNC_U8 (receiveitem_var1,            0x2D9);
  SYNC_U8 (pose_for_item,               0x2DA);
  SYNC_U8 (triggered_by_whirlpool_sprite, 0x2DB);
  SYNC_U16(x_coord_copy,                0x2DC);
  SYNC_U16(y_coord_copy,                0x2DE);
  SYNC_U8 (is_bunny_mirror,             0x2E0);
  SYNC_U8 (is_transforming,             0x2E1);
  SYNC_U8 (bunny_transform_timer,       0x2E2);
  SYNC_U8 (sword_delay_timer,           0x2E3);
  SYNC_U8 (dash_ctr,                    0x2F1);
  SYNC_U8 (want_make_noise_when_dashed,  0x2F8);
  SYNC_U8 (is_near_moveable_statue,      0x2FA);

  // Item & Action State
  SYNC_U8 (item_in_hand,                0x301);
  SYNC_U8 (state_bits,                  0x308);
  SYNC_U8 (picking_throw_state,         0x309);
  SYNC_U8 (var30d,                      0x30D);
  SYNC_U8 (var30e,                      0x30E);
  SYNC_U8 (spin_offsets,                0x31E);
  SYNC_U8 (direction_facing_mirror,     0x323);
  SYNC_U8 (maybe_swim_faster,           0x32A);
  SYNC_U8 (some_direction_bits,         0x340);
  SYNC_U8 (is_in_deep_water,            0x345);
  SYNC_U16(palette_bits_of_oam,         0x346);
  SYNC_U8 (flag_moving,                 0x34A);
  SYNC_U8 (swim_hard_stroke,            0x34F);
  SYNC_U8 (debug_value_2,               0x350);

  // Physics & Timers
  SYNC_U8 (electrocute_on_touch,         0x360);
  SYNC_U8 (actual_vel_z_mirror,          0x362);
  SYNC_U8 (actual_vel_z_copy_mirror,     0x363);
  SYNC_U16(z_coord_mirror,              0x364);
  SYNC_U8 (timer_push_get_tired,         0x371);
  SYNC_U8 (is_running,                   0x372);
  SYNC_U8 (give_damage,                  0x373);
  SYNC_U8 (countdown_for_dash,           0x374);
  SYNC_U8 (timer_jump_ledge,             0x375);
  SYNC_U8 (grabbing_wall,                0x376);
  SYNC_U8 (unk_master_sword,             0x377);
  SYNC_U8 (position_mode,                0x37A);
  SYNC_U8 (disable_sprite_damage,        0x37B);
  SYNC_U8 (pose_during_opening,          0x37D);

  // Hookshot & Special
  SYNC_U8 (something_with_hookshot,      0x3E9);
  SYNC_U8 (force_hold_sword_up,          0x3EF);
  SYNC_U8 (on_conveyor_belt,             0x3F3);
  SYNC_U16(timer_tempbunny,              0x3F5);
  SYNC_U8 (need_for_poof_for_transform,  0x3F7);
  SYNC_U8 (need_for_pullforrupees_sprite, 0x3F8);

  // Mirror & Misc
  SYNC_U8 (is_on_lower_level_mirror,     0x476);
  SYNC_U8 (lowlife_countdown_timer_beep, 0x4CA);
  SYNC_U8 (prevent_from_moving,          0xB7B);

  // Previous Coords
  SYNC_U16(x_coord_prev,                0xFC2);
  SYNC_U16(y_coord_prev,                0xFC4);

  // Cached Coords
  SYNC_U16(y_coord_spexit,              0xC108);
  SYNC_U16(x_coord_spexit,              0xC10A);
  SYNC_U16(y_coord_exit,                0xC148);
  SYNC_U16(x_coord_exit,                0xC14A);
  SYNC_U16(y_coord_cached,              0xC184);
  SYNC_U16(x_coord_cached,              0xC186);
  SYNC_U8 (quadrant_x_cached,           0xC19E);
  SYNC_U8 (quadrant_y_cached,           0xC19F);
  SYNC_U8 (direction_facing_cached,     0xC1A6);
  SYNC_U8 (is_on_lower_level_cached,    0xC1A7);
  SYNC_U8 (is_on_lower_level_mirror_cached, 0xC1A8);

  // Additional per-player Link state (co-op review fixes)
  SYNC_U8 (player_handler_timer,         0x300);
  SYNC_U8 (state_for_spin_attack,        0x31C);
  SYNC_U8 (step_counter_for_spin_attack, 0x31D);
  SYNC_U8 (countdown_for_blink,          0x31F);
  SYNC_U8 (player_near_pit_state,        0x5B);
  SYNC_U8 (player_on_somaria_platform,   0x2F5);
  SYNC_U8 (flag_is_link_immobilized,     0x2E4);
  SYNC_U16(kind_of_in_room_staircase,    0x44A);
  SYNC_U8 (about_to_jump_off_ledge,      0x47A);
  SYNC_U8 (flag_unk1,                    0xFC1);

  // Additional per-player Link state (co-op review round 2 — also leaked via g_ram)
  SYNC_U8 (draw_water_ripples_or_grass,  0x351);
  SYNC_U8 (some_animation_timer,         0x30B);
  SYNC_U8 (some_animation_timer_steps,   0x30A);
  SYNC_U8 (swimming_countdown,           0x2CB);
  SYNC_U8 (byte_7E02CC,                  0x2CC);
  SYNC_U8 (fallhole_var1,                0x302);
  SYNC_U8 (fallhole_var2,                0x2CA);
  SYNC_U8 (byte_7E02C9,                  0x2C9);
  SYNC_U8 (related_to_hookshot,          0x37E);
  SYNC_U8 (is_standing_in_doorway,       0x6C);
  SYNC_U8 (byte_7E0324,                  0x324);
  SYNC_U8 (byte_7E005C,                  0x5C);
  SYNC_U8 (byte_7E0322,                  0x322);
  SYNC_U8 (gravestone_push_timeout,      0x61);
  SYNC_U8 (byte_7E02C2,                  0x2C2);
  SYNC_U8 (byte_7E02C5,                  0x2C5);
  SYNC_U8 (primary_water_grass_timer,    0x356);
  SYNC_U8 (secondary_water_grass_timer,  0x355);

  // Inventory
  SYNC_U8 (item_bow,                     0xF340);
  SYNC_U8 (item_boomerang,               0xF341);
  SYNC_U8 (item_hookshot,                0xF342);
  SYNC_U8 (item_bombs,                   0xF343);
  SYNC_U8 (item_mushroom,                0xF344);
  SYNC_U8 (item_fire_rod,                0xF345);
  SYNC_U8 (item_ice_rod,                 0xF346);
  SYNC_U8 (item_bombos_medallion,        0xF347);
  SYNC_U8 (item_ether_medallion,         0xF348);
  SYNC_U8 (item_quake_medallion,         0xF349);
  SYNC_U8 (item_torch,                   0xF34A);
  SYNC_U8 (item_hammer,                  0xF34B);
  SYNC_U8 (item_flute,                   0xF34C);
  SYNC_U8 (item_bug_net,                 0xF34D);
  SYNC_U8 (item_book_of_mudora,          0xF34E);
  SYNC_U8 (item_bottle_index,            0xF34F);
  SYNC_U8 (item_cane_somaria,            0xF350);
  SYNC_U8 (item_cane_byrna,              0xF351);
  SYNC_U8 (item_cape,                    0xF352);
  SYNC_U8 (item_mirror,                  0xF353);
  SYNC_U8 (item_gloves,                  0xF354);
  SYNC_U8 (item_boots,                   0xF355);
  SYNC_U8 (item_flippers,                0xF356);
  SYNC_U8 (item_moon_pearl,              0xF357);
  SYNC_U8 (sword_type,                   0xF359);
  SYNC_U8 (shield_type,                  0xF35A);
  SYNC_U8 (armor,                        0xF35B);
  // bottle_info array
  if (to_ram) {
    for (int i = 0; i < NUM_BOTTLES; i++)
      g_ram[0xF35C + i] = cur_player->bottle_info[i];
  } else {
    for (int i = 0; i < NUM_BOTTLES; i++)
      cur_player->bottle_info[i] = g_ram[0xF35C + i];
  }
  SYNC_U16(rupees_goal,                  0xF360);
  SYNC_U16(rupees_actual,                0xF362);
  SYNC_U16(compass,                      0xF364);
  SYNC_U16(bigkey,                       0xF366);
  SYNC_U16(dungeon_map,                  0xF368);
  SYNC_U8 (rupees_in_pond,               0xF36A);
  SYNC_U8 (heart_pieces,                 0xF36B);
  SYNC_U8 (health_capacity,              0xF36C);
  SYNC_U8 (health_current,               0xF36D);
  SYNC_U8 (magic_power,                  0xF36E);
  SYNC_U8 (num_keys,                     0xF36F);
  SYNC_U8 (bomb_upgrades,                0xF370);
  SYNC_U8 (arrow_upgrades,               0xF371);
  SYNC_U8 (hearts_filler,                0xF372);
  SYNC_U8 (magic_filler,                 0xF373);
  SYNC_U8 (which_pendants,               0xF374);
  SYNC_U8 (bomb_filler,                  0xF375);
  SYNC_U8 (arrow_filler,                 0xF376);
  SYNC_U8 (num_arrows,                   0xF377);
  SYNC_U8 (ability_flags,                0xF379);
  SYNC_U8 (has_crystals,                 0xF37A);
  SYNC_U8 (magic_consumption,            0xF37B);
  // keys_earned_per_dungeon array
  if (to_ram) {
    for (int i = 0; i < NUM_DUNGEON_KEY_SLOTS; i++)
      g_ram[0xF37C + i] = cur_player->keys_earned_per_dungeon[i];
  } else {
    for (int i = 0; i < NUM_DUNGEON_KEY_SLOTS; i++)
      cur_player->keys_earned_per_dungeon[i] = g_ram[0xF37C + i];
  }

  // Per-player non-link_ globals
  SYNC_U8 (button_mask_b_y,              0x3A);
  SYNC_U8 (bitfield_for_a_button,        0x3B);
  SYNC_U8 (button_b_frames,              0x3C);
  SYNC_U8 (oam_y_offset,                 0x44);
  SYNC_U8 (oam_x_offset,                 0x45);
  SYNC_U8 (set_when_damaging_enemies,    0x47);
  SYNC_U8 (bitmask_of_dragstate,         0x48);
  SYNC_U16(force_move_any_direction,     0x49);
  SYNC_U8 (cape_decrement_counter,       0x4C);
  SYNC_U8 (index_of_dashing_sfx,         0x4F);

  // Input
  SYNC_U8 (joypad1H_last,                0xF0);
  SYNC_U8 (joypad1L_last,                0xF2);
  SYNC_U8 (filtered_joypad_H,            0xF4);
  SYNC_U8 (filtered_joypad_L,            0xF6);
  SYNC_U8 (joypad1H_last2,               0xF8);
  SYNC_U8 (joypad1L_last2,               0xFA);
}

void PlayerState_SyncToRam(void) {
  PlayerState_Sync(true);
}

void PlayerState_SyncFromRam(void) {
  PlayerState_Sync(false);
}

void PlayerState_Init(int player_index) {
  assert(player_index >= 0 && player_index < MAX_PLAYERS);
  PlayerState *ps = &g_players[player_index];

  if (player_index == 0) {
    // P1: pull initial state from g_ram (the normal game init path populates g_ram)
    memset(ps, 0, sizeof(PlayerState));
    PlayerState *saved = cur_player;
    cur_player = ps;
    PlayerState_SyncFromRam();
    cur_player = saved;
    ps->player_index = 0;
    ps->is_active = 1;
    ps->palette_index = 0;  // green tunic
  } else {
    // P2 co-op: clone P1's FULL progression (items, sword, shield, abilities,
    // dungeon access) so the second player has matching capabilities, per the
    // design ("P2 spawns from P1"). Then override spawn + per-player fields.
    // Health/magic become independent pools from here on.
    PlayerState *p1 = &g_players[0];
    *ps = *p1;
    ps->player_index = 1;
    ps->is_active = 1;
    ps->is_dead = 0;
    ps->is_ghost = 0;
    ps->respawn_timer = 0;
    ps->palette_index = 1;                       // purple/red tunic
    ps->health_current = p1->health_capacity;    // full health on join
    ps->direction_facing = 0x02;                 // facing down
    ps->x_coord = p1->x_coord + 16;              // spawn beside P1
    ps->y_coord = p1->y_coord;
    // Clear transient movement/combat state inherited from the clone.
    ps->x_vel = ps->y_vel = 0;
    ps->incapacitated_timer = 0;
    ps->recoilmode_timer = 0;
  }
}

// ============================================================================
// Co-op death / respawn
// (guarded: references g_mp_p2_enabled, which only exists in the co-op build)
// ============================================================================
#ifdef ZELDA3_MULTIPLAYER
#define MP_RESPAWN_FRAMES 240   // ~4 seconds downed before reviving (fits uint8 timer)

// Called at the engine's game-over points (a player just reached 0 health).
// cur_player is the player that's going down. Returns true to SUPPRESS the
// game-over (the player is put into a downed ghost state) when the other player
// is still up; returns false only on a true double-KO so the engine can run the
// normal game-over.
bool Multiplayer_PreventGameOver(void) {
  // Only meaningful when both players exist and co-op is on.
  if (!g_mp_p2_enabled || !g_players[0].is_active || !g_players[1].is_active)
    return false;

  PlayerState *dying = cur_player;
  PlayerState *other = (cur_player == &g_players[0]) ? &g_players[1] : &g_players[0];

  // Both players down at once -> let the real game-over happen.
  if (other->is_dead)
    return false;

  // Put the current player into the downed/ghost state (idempotent).
  if (!dying->is_dead) {
    dying->is_dead = 1;
    dying->is_ghost = 1;
    dying->respawn_timer = MP_RESPAWN_FRAMES;
  }
  dying->health_current = 0;
  dying->disable_sprite_damage = 1;   // ghost: invulnerable while downed
  dying->visibility_status = 0;       // visible (LinkOam_Main flashes the ghost)
  return true;
}

// Revive a downed player: clear the ghost state and give it some health back.
static void Multiplayer_RevivePlayer(PlayerState *ps) {
  ps->health_current = ps->health_capacity >> 1;   // revive with half hearts
  if (ps->health_current < 8) ps->health_current = 8;
  ps->is_dead = 0;
  ps->is_ghost = 0;
  ps->disable_sprite_damage = 0;
  ps->visibility_status = 0;
  ps->incapacitated_timer = 0;
  ps->x_vel = ps->y_vel = 0;
  ps->flag_moving = 0;
}

// Per-frame: keep downed players in the ghost state, let the living partner
// revive a downed player by reaching them (revive-on-touch), and otherwise
// auto-revive beside the partner once the respawn timer elapses.
void Multiplayer_UpdateDeathRespawn(void) {
  if (!g_mp_p2_enabled) return;
  for (int i = 0; i < 2; i++) {
    PlayerState *ps = &g_players[i];
    PlayerState *other = &g_players[i ^ 1];
    if (!ps->is_active || !ps->is_dead)
      continue;

    // Maintain the downed/ghost state each frame (invulnerable, 0 HP, visible
    // base state — the flashing is done at render time in LinkOam_Main).
    ps->disable_sprite_damage = 1;
    ps->health_current = 0;
    ps->visibility_status = 0;

    // Both players down -> stay down; the killing blow already let the real
    // game-over trigger.
    if (other->is_dead)
      continue;

    // Revive-on-touch: the living partner walking onto the downed ghost revives
    // it on the spot (classic co-op rescue).
    int dx = (int)ps->x_coord - (int)other->x_coord;
    int dy = (int)ps->y_coord - (int)other->y_coord;
    if (dx * dx + dy * dy <= 24 * 24) {
      Multiplayer_RevivePlayer(ps);
      continue;
    }

    // Otherwise count down and auto-revive beside the partner when it elapses.
    if (ps->respawn_timer > 0) {
      ps->respawn_timer--;
      continue;
    }
    ps->x_coord = other->x_coord + 16;
    ps->y_coord = other->y_coord;
    ps->is_on_lower_level = other->is_on_lower_level;
    ps->quadrant_x = other->quadrant_x;
    ps->quadrant_y = other->quadrant_y;
    Multiplayer_RevivePlayer(ps);
  }
}

// Share the team inventory between players. The inventory fields are contiguous
// in PlayerState (item_bow .. keys_earned_per_dungeon), so copy that whole block
// and then restore the destination's PER-PLAYER health/magic (which live inside
// that same address range). This makes one player's pickups/purchases/upgrades
// appear for both, while health and magic remain independent.
void Multiplayer_ShareInventory(PlayerState *from, PlayerState *to) {
  uint8 hcap = to->health_capacity, hcur = to->health_current;
  uint8 mpow = to->magic_power, hfill = to->hearts_filler, mfill = to->magic_filler;
  size_t n = (size_t)((char *)&to->keys_earned_per_dungeon[NUM_DUNGEON_KEY_SLOTS]
                      - (char *)&to->item_bow);
  memcpy(&to->item_bow, &from->item_bow, n);
  to->health_capacity = hcap;
  to->health_current  = hcur;
  to->magic_power     = mpow;
  to->hearts_filler   = hfill;
  to->magic_filler    = mfill;
}
#endif  // ZELDA3_MULTIPLAYER

// ============================================================================
// Multiplayer config
// ============================================================================
MultiplayerConfig g_mp_config = {
  .mode = MP_MODE_SINGLE,
  .num_players = 1,
  .local_player_index = 0,
  .input_delay_frames = 0,
};

// ============================================================================
// Input ring buffer
// ============================================================================
InputRingBuffer g_input_rings[MAX_PLAYERS];

void InputRing_Push(int player_index, const InputFrame *frame) {
  assert(player_index >= 0 && player_index < MAX_PLAYERS);
  InputRingBuffer *rb = &g_input_rings[player_index];
  rb->frames[rb->write_pos & (INPUT_RING_SIZE - 1)] = *frame;
  rb->write_pos++;
}

bool InputRing_Pop(int player_index, InputFrame *out) {
  assert(player_index >= 0 && player_index < MAX_PLAYERS);
  InputRingBuffer *rb = &g_input_rings[player_index];
  if (rb->read_pos == rb->write_pos)
    return false;
  *out = rb->frames[rb->read_pos & (INPUT_RING_SIZE - 1)];
  rb->read_pos++;
  return true;
}

bool InputRing_Peek(int player_index, uint32 frame_number, InputFrame *out) {
  assert(player_index >= 0 && player_index < MAX_PLAYERS);
  InputRingBuffer *rb = &g_input_rings[player_index];
  // Scan from read_pos forward looking for matching frame
  for (uint32 i = rb->read_pos; i != rb->write_pos; i++) {
    InputFrame *f = &rb->frames[i & (INPUT_RING_SIZE - 1)];
    if (f->frame_number == frame_number) {
      *out = *f;
      return true;
    }
  }
  return false;
}

uint32 InputRing_Count(int player_index) {
  assert(player_index >= 0 && player_index < MAX_PLAYERS);
  InputRingBuffer *rb = &g_input_rings[player_index];
  return rb->write_pos - rb->read_pos;
}

// ============================================================================
// Frame management
// ============================================================================
uint32 g_sim_frame = 0;

bool Multiplayer_InputsReady(void) {
  if (g_mp_config.mode == MP_MODE_SINGLE)
    return true;

  uint32 target_frame = g_sim_frame;

  // For online with delay buffer, we process frames that are
  // input_delay_frames behind what we're collecting
  if (g_mp_config.mode == MP_MODE_HOST || g_mp_config.mode == MP_MODE_CLIENT)
    target_frame = g_sim_frame;  // Already accounted for in input collection

  for (int i = 0; i < g_mp_config.num_players; i++) {
    InputFrame tmp;
    if (!InputRing_Peek(i, target_frame, &tmp))
      return false;
  }
  return true;
}

FrameInputPair Multiplayer_ConsumeInputs(void) {
  FrameInputPair pair;
  pair.frame_number = g_sim_frame;
  pair.joypad[0] = 0;
  pair.joypad[1] = 0;

  for (int i = 0; i < g_mp_config.num_players; i++) {
    InputFrame frame;
    if (InputRing_Pop(i, &frame)) {
      pair.joypad[i] = frame.joypad;
    }
  }
  return pair;
}

// ============================================================================
// CRC32 — table-driven, no dependency
// Used for desync detection: both machines checksum g_ram and compare.
// ============================================================================
static uint32 crc32_table[256];
static bool crc32_table_init = false;

static void CRC32_InitTable(void) {
  for (uint32 i = 0; i < 256; i++) {
    uint32 c = i;
    for (int j = 0; j < 8; j++)
      c = (c & 1) ? (0xEDB88320 ^ (c >> 1)) : (c >> 1);
    crc32_table[i] = c;
  }
  crc32_table_init = true;
}

uint32 ComputeCRC32(const uint8 *data, size_t length) {
  if (!crc32_table_init)
    CRC32_InitTable();
  uint32 crc = 0xFFFFFFFF;
  for (size_t i = 0; i < length; i++)
    crc = crc32_table[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);
  return crc ^ 0xFFFFFFFF;
}

SyncChecksum Multiplayer_ComputeChecksum(void) {
  SyncChecksum sc;
  sc.frame_number = g_sim_frame;
  sc.checksum = ComputeCRC32(g_ram, 0x20000);
  return sc;
}

// ============================================================================
// Phase 6: Online Multiplayer Architecture Stubs
// These are placeholder implementations for future online multiplayer.
// ============================================================================

void GameState_Snapshot(void *out_buffer, uint32 *out_size) {
  // STUB: Would memcpy g_ram (128KB) + both PlayerState structs
  // into out_buffer for rollback netcode.
  // *out_size = sizeof(g_ram) + sizeof(g_players);
  (void)out_buffer;
  if (out_size)
    *out_size = 0;
}

void GameState_Restore(const void *buffer, uint32 size) {
  // STUB: Would memcpy from buffer back into g_ram + g_players
  // to restore a previous frame for rollback.
  (void)buffer;
  (void)size;
}

void Replay_RecordFrame(uint32 frame_number, const InputFrame *p1, const InputFrame *p2) {
  // STUB: Would append both InputFrames to a replay file.
  // Format: [frame_number][p1 InputFrame][p2 InputFrame]
  (void)frame_number;
  (void)p1;
  (void)p2;
}

void Replay_PlaybackFrame(uint32 frame_number, InputFrame *out_p1, InputFrame *out_p2) {
  // STUB: Would read InputFrames from replay file for the given frame
  // and populate the output structs.
  (void)frame_number;
  if (out_p1) memset(out_p1, 0, sizeof(*out_p1));
  if (out_p2) memset(out_p2, 0, sizeof(*out_p2));
}

void Debug_VerifyDeterminism(void) {
  // STUB: In debug builds, would run two parallel game instances with
  // identical inputs and compare g_ram after each frame. Any divergence
  // indicates a determinism bug that would break online multiplayer.
  //
  // Implementation outline:
  //   1. Save current g_ram to buffer A
  //   2. Run frame with inputs
  //   3. Save result to buffer B
  //   4. Restore buffer A, run same frame with same inputs
  //   5. Compare result with buffer B
  //   6. Assert they match; if not, log divergent addresses
}
