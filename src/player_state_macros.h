// player_state_macros.h — Macro redirects for two-player mod
//
// When ZELDA3_MULTIPLAYER is defined, all link_ macros resolve through
// cur_player-> instead of g_ram+offset. This means the entire codebase
// works with per-player state without changing a single function.
//
// Include this AFTER variables.h in any file that uses link_ variables.
// In Phase 1, we add it to the bottom of variables.h itself.

#pragma once

#ifdef ZELDA3_MULTIPLAYER

// First, undefine all macros that conflict with PlayerState field names.
// We must do this BEFORE including player_state.h, because that header
// declares struct fields whose names collide with these macros.

// Undefine all original link_ macros and redirect through cur_player
#undef link_y_coord
#undef link_x_coord
#undef link_z_coord
#undef link_direction_last
#undef link_actual_vel_y
#undef link_actual_vel_x
#undef link_actual_vel_z
#undef link_subpixel_y
#undef link_subpixel_x
#undef link_subpixel_z
#undef link_counter_var1
#undef link_animation_steps
#undef link_direction_facing
#undef link_y_vel
#undef link_x_vel
#undef link_y_coord_original
#undef link_delay_timer_spin_attack
#undef link_y_coord_safe_return_lo
#undef link_x_coord_safe_return_lo
#undef link_y_coord_safe_return_hi
#undef link_x_coord_safe_return_hi
#undef link_direction_mask_a
#undef link_direction_mask_b
#undef link_incapacitated_timer
#undef link_visibility_status
#undef link_auxiliary_state
#undef link_cant_change_direction
#undef link_cape_mode
#undef link_is_bunny
#undef link_speed_modifier
#undef link_this_controls_sprite_oam
#undef link_player_handler_state
#undef link_speed_setting
#undef link_last_direction_moved_towards
#undef link_direction
#undef link_y_page_movement_delta
#undef link_x_page_movement_delta
#undef link_num_orthogonal_directions
#undef link_moving_against_diag_tile
#undef link_spin_attack_step_counter
#undef link_quadrant_x
#undef link_quadrant_y
#undef link_is_on_lower_level
#undef link_dma_graphics_index
#undef link_dma_var1
#undef link_dma_var2
#undef link_dma_var3
#undef link_dma_var4
#undef link_dma_var5
#undef link_tile_below
#undef link_tile_below_PADDING
#undef link_debug_value_1
#undef link_recoilmode_timer
#undef link_actual_vel_z_copy
#undef link_receiveitem_index
#undef link_receiveitem_var1
#undef link_pose_for_item
#undef link_triggered_by_whirlpool_sprite
#undef link_x_coord_copy
#undef link_y_coord_copy
#undef link_is_bunny_mirror
#undef link_is_transforming
#undef link_bunny_transform_timer
#undef link_sword_delay_timer
#undef link_dash_ctr
#undef link_want_make_noise_when_dashed
#undef link_is_near_moveable_statue
#undef link_item_in_hand
#undef link_state_bits
#undef link_picking_throw_state
#undef link_var30d
#undef link_var30e
#undef link_spin_offsets
#undef link_direction_facing_mirror
#undef link_maybe_swim_faster
#undef link_some_direction_bits
#undef link_is_in_deep_water
#undef link_palette_bits_of_oam
#undef link_flag_moving
#undef link_swim_hard_stroke
#undef link_debug_value_2
#undef link_electrocute_on_touch
#undef link_actual_vel_z_mirror
#undef link_actual_vel_z_copy_mirror
#undef link_z_coord_mirror
#undef link_timer_push_get_tired
#undef link_is_running
#undef link_give_damage
#undef link_countdown_for_dash
#undef link_timer_jump_ledge
#undef link_grabbing_wall
#undef link_unk_master_sword
#undef link_position_mode
#undef link_disable_sprite_damage
#undef link_pose_during_opening
#undef link_something_with_hookshot
#undef link_force_hold_sword_up
#undef link_on_conveyor_belt
#undef link_timer_tempbunny
#undef link_need_for_poof_for_transform
#undef link_need_for_pullforrupees_sprite
#undef link_is_on_lower_level_mirror
#undef link_lowlife_countdown_timer_beep
#undef link_prevent_from_moving
#undef link_x_coord_prev
#undef link_y_coord_prev
#undef link_y_coord_spexit
#undef link_x_coord_spexit
#undef link_y_coord_exit
#undef link_x_coord_exit
#undef link_y_coord_cached
#undef link_x_coord_cached
#undef link_quadrant_x_cached
#undef link_quadrant_y_cached
#undef link_direction_facing_cached
#undef link_is_on_lower_level_cached
#undef link_is_on_lower_level_mirror_cached
#undef link_item_bow
#undef link_item_boomerang
#undef link_item_hookshot
#undef link_item_bombs
#undef link_item_mushroom
#undef link_item_fire_rod
#undef link_item_ice_rod
#undef link_item_bombos_medallion
#undef link_item_ether_medallion
#undef link_item_quake_medallion
#undef link_item_torch
#undef link_item_hammer
#undef link_item_flute
#undef link_item_bug_net
#undef link_item_book_of_mudora
#undef link_item_bottle_index
#undef link_item_cane_somaria
#undef link_item_cane_byrna
#undef link_item_cape
#undef link_item_mirror
#undef link_item_gloves
#undef link_item_boots
#undef link_item_flippers
#undef link_item_moon_pearl
#undef link_sword_type
#undef link_shield_type
#undef link_armor
#undef link_bottle_info
#undef link_rupees_goal
#undef link_rupees_actual
#undef link_compass
#undef link_bigkey
#undef link_dungeon_map
#undef link_rupees_in_pond
#undef link_heart_pieces
#undef link_health_capacity
#undef link_health_current
#undef link_magic_power
#undef link_num_keys
#undef link_bomb_upgrades
#undef link_arrow_upgrades
#undef link_hearts_filler
#undef link_magic_filler
#undef link_which_pendants
#undef link_bomb_filler
#undef link_arrow_filler
#undef link_num_arrows
#undef link_ability_flags
#undef link_has_crystals
#undef link_magic_consumption
#undef link_keys_earned_per_dungeon

// Additional per-player Link state (co-op review fixes — were leaking via g_ram)
#undef player_handler_timer
#undef state_for_spin_attack
#undef step_counter_for_spin_attack
#undef countdown_for_blink
#undef player_near_pit_state
#undef player_on_somaria_platform
#undef flag_is_link_immobilized
#undef kind_of_in_room_staircase
#undef about_to_jump_off_ledge
#undef flag_unk1

// Also redirect per-player non-link_ globals
#undef button_mask_b_y
#undef bitfield_for_a_button
#undef button_b_frames
#undef player_oam_y_offset
#undef player_oam_x_offset
#undef set_when_damaging_enemies
#undef bitmask_of_dragstate
#undef force_move_any_direction
#undef cape_decrement_counter
#undef index_of_dashing_sfx
#undef joypad1H_last
#undef joypad1L_last
#undef filtered_joypad_H
#undef filtered_joypad_L
#undef joypad1H_last2
#undef joypad1L_last2

// NOW it's safe to include player_state.h — all conflicting macros are gone
#include "player_state.h"

// ============================================================================
// Redirected macros — all resolve through cur_player->
// ============================================================================

// Movement & Position
#define link_y_coord                     (cur_player->y_coord)
#define link_x_coord                     (cur_player->x_coord)
#define link_z_coord                     (cur_player->z_coord)
#define link_direction_last              (cur_player->direction_last)
#define link_actual_vel_y                (cur_player->actual_vel_y)
#define link_actual_vel_x                (cur_player->actual_vel_x)
#define link_actual_vel_z                (cur_player->actual_vel_z)
#define link_subpixel_y                  (cur_player->subpixel_y)
#define link_subpixel_x                  (cur_player->subpixel_x)
#define link_subpixel_z                  (cur_player->subpixel_z)
#define link_counter_var1                (cur_player->counter_var1)
#define link_animation_steps             (cur_player->animation_steps)
#define link_direction_facing            (cur_player->direction_facing)
#define link_y_vel                       (cur_player->y_vel)
#define link_x_vel                       (cur_player->x_vel)
#define link_y_coord_original            (cur_player->y_coord_original)
#define link_delay_timer_spin_attack     (cur_player->delay_timer_spin_attack)
#define link_y_coord_safe_return_lo      (cur_player->y_coord_safe_return_lo)
#define link_x_coord_safe_return_lo      (cur_player->x_coord_safe_return_lo)
#define link_y_coord_safe_return_hi      (cur_player->y_coord_safe_return_hi)
#define link_x_coord_safe_return_hi      (cur_player->x_coord_safe_return_hi)
#define link_direction_mask_a            (cur_player->direction_mask_a)
#define link_direction_mask_b            (cur_player->direction_mask_b)
#define link_incapacitated_timer         (cur_player->incapacitated_timer)
#define link_visibility_status           (cur_player->visibility_status)
#define link_auxiliary_state             (cur_player->auxiliary_state)
#define link_cant_change_direction       (cur_player->cant_change_direction)
#define link_cape_mode                   (cur_player->cape_mode)
#define link_is_bunny                    (cur_player->is_bunny)
#define link_speed_modifier              (cur_player->speed_modifier)
#define link_this_controls_sprite_oam    (cur_player->this_controls_sprite_oam)
#define link_player_handler_state        (cur_player->player_handler_state)
#define link_speed_setting               (cur_player->speed_setting)
#define link_last_direction_moved_towards (cur_player->last_direction_moved_towards)
#define link_direction                   (cur_player->direction)
#define link_y_page_movement_delta       (cur_player->y_page_movement_delta)
#define link_x_page_movement_delta       (cur_player->x_page_movement_delta)
#define link_num_orthogonal_directions   (cur_player->num_orthogonal_directions)
#define link_moving_against_diag_tile    (cur_player->moving_against_diag_tile)
#define link_spin_attack_step_counter    (cur_player->spin_attack_step_counter)

// Quadrant & Layer
#define link_quadrant_x                  (cur_player->quadrant_x)
#define link_quadrant_y                  (cur_player->quadrant_y)
#define link_is_on_lower_level           (cur_player->is_on_lower_level)

// DMA & Graphics
#define link_dma_graphics_index          (cur_player->dma_graphics_index)
#define link_dma_var1                    (cur_player->dma_var1)
#define link_dma_var2                    (cur_player->dma_var2)
#define link_dma_var3                    (cur_player->dma_var3)
#define link_dma_var4                    (cur_player->dma_var4)
#define link_dma_var5                    (cur_player->dma_var5)
#define link_tile_below                  (cur_player->tile_below)
#define link_tile_below_PADDING          (cur_player->tile_below_PADDING)

// Debug
#define link_debug_value_1               (cur_player->debug_value_1)

// Combat & State
#define link_recoilmode_timer            (cur_player->recoilmode_timer)
#define link_actual_vel_z_copy           (cur_player->actual_vel_z_copy)
#define link_receiveitem_index           (cur_player->receiveitem_index)
#define link_receiveitem_var1            (cur_player->receiveitem_var1)
#define link_pose_for_item               (cur_player->pose_for_item)
#define link_triggered_by_whirlpool_sprite (cur_player->triggered_by_whirlpool_sprite)
#define link_x_coord_copy                (cur_player->x_coord_copy)
#define link_y_coord_copy                (cur_player->y_coord_copy)
#define link_is_bunny_mirror             (cur_player->is_bunny_mirror)
#define link_is_transforming             (cur_player->is_transforming)
#define link_bunny_transform_timer       (cur_player->bunny_transform_timer)
#define link_sword_delay_timer           (cur_player->sword_delay_timer)
#define link_dash_ctr                    (cur_player->dash_ctr)
#define link_want_make_noise_when_dashed  (cur_player->want_make_noise_when_dashed)
#define link_is_near_moveable_statue     (cur_player->is_near_moveable_statue)

// Item & Action State
#define link_item_in_hand                (cur_player->item_in_hand)
#define link_state_bits                  (cur_player->state_bits)
#define link_picking_throw_state         (cur_player->picking_throw_state)
#define link_var30d                      (cur_player->var30d)
#define link_var30e                      (cur_player->var30e)
#define link_spin_offsets                (cur_player->spin_offsets)
#define link_direction_facing_mirror     (cur_player->direction_facing_mirror)
#define link_maybe_swim_faster           (cur_player->maybe_swim_faster)
#define link_some_direction_bits         (cur_player->some_direction_bits)
#define link_is_in_deep_water            (cur_player->is_in_deep_water)
#define link_palette_bits_of_oam         (cur_player->palette_bits_of_oam)
#define link_flag_moving                 (cur_player->flag_moving)
#define link_swim_hard_stroke            (cur_player->swim_hard_stroke)
#define link_debug_value_2               (cur_player->debug_value_2)

// Physics & Timers
#define link_electrocute_on_touch        (cur_player->electrocute_on_touch)
#define link_actual_vel_z_mirror         (cur_player->actual_vel_z_mirror)
#define link_actual_vel_z_copy_mirror    (cur_player->actual_vel_z_copy_mirror)
#define link_z_coord_mirror              (cur_player->z_coord_mirror)
#define link_timer_push_get_tired        (cur_player->timer_push_get_tired)
#define link_is_running                  (cur_player->is_running)
#define link_give_damage                 (cur_player->give_damage)
#define link_countdown_for_dash          (cur_player->countdown_for_dash)
#define link_timer_jump_ledge            (cur_player->timer_jump_ledge)
#define link_grabbing_wall               (cur_player->grabbing_wall)
#define link_unk_master_sword            (cur_player->unk_master_sword)
#define link_position_mode               (cur_player->position_mode)
#define link_disable_sprite_damage       (cur_player->disable_sprite_damage)
#define link_pose_during_opening         (cur_player->pose_during_opening)

// Hookshot & Special
#define link_something_with_hookshot     (cur_player->something_with_hookshot)
#define link_force_hold_sword_up         (cur_player->force_hold_sword_up)
#define link_on_conveyor_belt            (cur_player->on_conveyor_belt)
#define link_timer_tempbunny             (cur_player->timer_tempbunny)
#define link_need_for_poof_for_transform (cur_player->need_for_poof_for_transform)
#define link_need_for_pullforrupees_sprite (cur_player->need_for_pullforrupees_sprite)

// Mirror & Misc
#define link_is_on_lower_level_mirror    (cur_player->is_on_lower_level_mirror)
#define link_lowlife_countdown_timer_beep (cur_player->lowlife_countdown_timer_beep)
#define link_prevent_from_moving         (cur_player->prevent_from_moving)

// Previous Coords
#define link_x_coord_prev                (cur_player->x_coord_prev)
#define link_y_coord_prev                (cur_player->y_coord_prev)

// Cached Coords
#define link_y_coord_spexit              (cur_player->y_coord_spexit)
#define link_x_coord_spexit              (cur_player->x_coord_spexit)
#define link_y_coord_exit                (cur_player->y_coord_exit)
#define link_x_coord_exit                (cur_player->x_coord_exit)
#define link_y_coord_cached              (cur_player->y_coord_cached)
#define link_x_coord_cached              (cur_player->x_coord_cached)
#define link_quadrant_x_cached           (cur_player->quadrant_x_cached)
#define link_quadrant_y_cached           (cur_player->quadrant_y_cached)
#define link_direction_facing_cached     (cur_player->direction_facing_cached)
#define link_is_on_lower_level_cached    (cur_player->is_on_lower_level_cached)
#define link_is_on_lower_level_mirror_cached (cur_player->is_on_lower_level_mirror_cached)

// Inventory
#define link_item_bow                    (cur_player->item_bow)
#define link_item_boomerang              (cur_player->item_boomerang)
#define link_item_hookshot               (cur_player->item_hookshot)
#define link_item_bombs                  (cur_player->item_bombs)
#define link_item_mushroom               (cur_player->item_mushroom)
#define link_item_fire_rod               (cur_player->item_fire_rod)
#define link_item_ice_rod                (cur_player->item_ice_rod)
#define link_item_bombos_medallion       (cur_player->item_bombos_medallion)
#define link_item_ether_medallion        (cur_player->item_ether_medallion)
#define link_item_quake_medallion        (cur_player->item_quake_medallion)
#define link_item_torch                  (cur_player->item_torch)
#define link_item_hammer                 (cur_player->item_hammer)
#define link_item_flute                  (cur_player->item_flute)
#define link_item_bug_net                (cur_player->item_bug_net)
#define link_item_book_of_mudora         (cur_player->item_book_of_mudora)
#define link_item_bottle_index           (cur_player->item_bottle_index)
#define link_item_cane_somaria           (cur_player->item_cane_somaria)
#define link_item_cane_byrna             (cur_player->item_cane_byrna)
#define link_item_cape                   (cur_player->item_cape)
#define link_item_mirror                 (cur_player->item_mirror)
#define link_item_gloves                 (cur_player->item_gloves)
#define link_item_boots                  (cur_player->item_boots)
#define link_item_flippers               (cur_player->item_flippers)
#define link_item_moon_pearl             (cur_player->item_moon_pearl)
#define link_sword_type                  (cur_player->sword_type)
#define link_shield_type                 (cur_player->shield_type)
#define link_armor                       (cur_player->armor)
#define link_bottle_info                 (cur_player->bottle_info)
#define link_rupees_goal                 (cur_player->rupees_goal)
#define link_rupees_actual               (cur_player->rupees_actual)
#define link_compass                     (cur_player->compass)
#define link_bigkey                      (cur_player->bigkey)
#define link_dungeon_map                 (cur_player->dungeon_map)
#define link_rupees_in_pond              (cur_player->rupees_in_pond)
#define link_heart_pieces                (cur_player->heart_pieces)
#define link_health_capacity             (cur_player->health_capacity)
#define link_health_current              (cur_player->health_current)
#define link_magic_power                 (cur_player->magic_power)
#define link_num_keys                    (cur_player->num_keys)
#define link_bomb_upgrades               (cur_player->bomb_upgrades)
#define link_arrow_upgrades              (cur_player->arrow_upgrades)
#define link_hearts_filler               (cur_player->hearts_filler)
#define link_magic_filler                (cur_player->magic_filler)
#define link_which_pendants              (cur_player->which_pendants)
#define link_bomb_filler                 (cur_player->bomb_filler)
#define link_arrow_filler                (cur_player->arrow_filler)
#define link_num_arrows                  (cur_player->num_arrows)
#define link_ability_flags               (cur_player->ability_flags)
#define link_has_crystals                (cur_player->has_crystals)
#define link_magic_consumption           (cur_player->magic_consumption)
#define link_keys_earned_per_dungeon     (cur_player->keys_earned_per_dungeon)

// Additional per-player Link state (co-op review fixes)
#define player_handler_timer             (cur_player->player_handler_timer)
#define state_for_spin_attack            (cur_player->state_for_spin_attack)
#define step_counter_for_spin_attack     (cur_player->step_counter_for_spin_attack)
#define countdown_for_blink              (cur_player->countdown_for_blink)
#define player_near_pit_state            (cur_player->player_near_pit_state)
#define player_on_somaria_platform       (cur_player->player_on_somaria_platform)
#define flag_is_link_immobilized         (cur_player->flag_is_link_immobilized)
#define kind_of_in_room_staircase        (cur_player->kind_of_in_room_staircase)
#define about_to_jump_off_ledge          (cur_player->about_to_jump_off_ledge)
#define flag_unk1                        (cur_player->flag_unk1)

// Per-player non-link_ globals
#define button_mask_b_y                  (cur_player->button_mask_b_y)
#define bitfield_for_a_button            (cur_player->bitfield_for_a_button)
#define button_b_frames                  (cur_player->button_b_frames)
#define player_oam_y_offset              (cur_player->oam_y_offset)
#define player_oam_x_offset              (cur_player->oam_x_offset)
#define set_when_damaging_enemies        (cur_player->set_when_damaging_enemies)
#define bitmask_of_dragstate             (cur_player->bitmask_of_dragstate)
#define force_move_any_direction         (cur_player->force_move_any_direction)
#define cape_decrement_counter           (cur_player->cape_decrement_counter)
#define index_of_dashing_sfx             (cur_player->index_of_dashing_sfx)

// Per-player input
#define joypad1H_last                    (cur_player->joypad1H_last)
#define joypad1L_last                    (cur_player->joypad1L_last)
#define filtered_joypad_H               (cur_player->filtered_joypad_H)
#define filtered_joypad_L               (cur_player->filtered_joypad_L)
#define joypad1H_last2                   (cur_player->joypad1H_last2)
#define joypad1L_last2                   (cur_player->joypad1L_last2)

#endif // ZELDA3_MULTIPLAYER
