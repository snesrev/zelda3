#include "player.h"
#include "zelda_rtl.h"
#include "variables.h"
#include "tile_detect.h"
#include "ancilla.h"
#include "sprite.h"
#include "load_gfx.h"
#include "hud.h"
#include "overworld.h"
#include "tagalong.h"
#include "dungeon.h"
#include "misc.h"
#include "player_oam.h"
#include "sprite_main.h"

static bool g_ApplyLinksMovementToCamera_called;

static const uint8 kSpinAttackDelays[] = { 1, 0, 0, 0, 0, 3, 0, 0, 1, 0, 3, 3, 3, 3, 4, 4, 1, 5 };
static const uint8 kFireBeamSounds[] = { 1, 2, 3, 4, 0, 9, 18, 27 };
static const int8 kTagalongArr1[] = { -1, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static const int8 kTagalongArr2[] = { -1, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static const uint8 kLinkSpinGraphicsByDir[] = {
  10, 11, 10, 6, 7, 8, 9, 2, 3, 4, 5, 10, 0, 1, 0, 2, 3, 4, 5, 6, 7, 8, 9, 0, 12, 13, 12, 4, 5, 6, 7, 8, 9, 2, 3, 12, 14, 15, 14, 8, 9, 2, 3, 4, 5, 6, 7, 14
};
static const uint8 kLinkSpinDelays[] = { 1, 5, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5 };
static const uint8 kGrabWallDirs[] = { 4, 8, 1, 2 };
static const uint8 kGrabWall_AnimSteps[] = { 0, 1, 2, 3, 1, 2, 3 };
static const uint8 kGrabWall_AnimTimer[] = { 0, 5, 5, 12, 5, 5, 12 };
static const uint8 kCapeDepletionTimers[] = { 4, 8, 8 };
static const int8 kAvoidJudder1[] = { 0, 1, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7, 6, 5, 4, 3, 2, 1, 0, 0, 1, 2, 3, 4, 5, 6, 7 };
static const int8 kLink_DoMoveXCoord_Outdoors_Helper2_y[2] = { -8, 8 };
static const int8 kLink_DoMoveXCoord_Outdoors_Helper2_y2[2] = { -16, 16 };
static const uint8 kLink_DoMoveXCoord_Outdoors_Helper2_velz[8] = { 32, 32, 32, 40, 48, 56, 64, 72 };
static const uint8 kLink_DoMoveXCoord_Outdoors_Helper2_velx[8] = { 16, 28, 28, 28, 28, 28, 28, 28 };
static const uint8 kLink_Lift_tab[9] = { 0x54, 0x52, 0x50, 0xFF, 0x51, 0x53, 0x55, 0x56, 0x57 };
static const uint8 kLink_Move_Helper6_tab0[] = { 8, 8, 23, 23, 8, 23, 8, 23 };
static const uint8 kLink_Move_Helper6_tab1[] = { 0, 15, 0, 15, 0, 0, 15, 15 };
static const uint8 kLink_Move_Helper6_tab2[] = { 23, 23, 8, 8, 8, 23, 8, 23 };
static const uint8 kLink_Move_Helper6_tab3[] = { 0, 15, 0, 15, 15, 15, 0, 0 };
const uint8 kSwimmingTab1[4] = { 2, 0, 1, 0 };
const uint8 kSwimmingTab2[2] = { 32, 8 };
static PlayerHandlerFunc *const kPlayerHandlers[31] = {
  &LinkState_Default,
  &LinkState_Pits,
  &LinkState_Recoil,
  &LinkState_SpinAttack,
  &PlayerHandler_04_Swimming,
  &LinkState_OnIce,
  &LinkState_Recoil,
  &LinkState_Zapped,
  &LinkState_UsingEther,
  &LinkState_UsingBombos,
  &LinkState_UsingQuake,
  &LinkHop_HoppingSouthOW,
  &LinkState_HoppingHorizontallyOW,
  &LinkState_HoppingDiagonallyUpOW,
  &LinkState_HoppingDiagonallyDownOW,
  &LinkState_0F,
  &LinkState_0F,
  &LinkState_Dashing,
  &LinkState_ExitingDash,
  &LinkState_Hookshotting,
  &LinkState_CrossingWorlds,
  &PlayerHandler_15_HoldItem,
  &LinkState_Sleeping,
  &PlayerHandler_17_Bunny,
  &LinkState_HoldingBigRock,
  &LinkState_ReceivingEther,
  &LinkState_ReceivingBombos,
  &LinkState_ReadingDesertTablet,
  &LinkState_TemporaryBunny,
  &LinkState_TreePull,
  &LinkState_SpinAttack,
};
// forwards
static const uint8 kLinkItem_MagicCosts[] = { 16, 8, 4, 32, 16, 8, 8, 4, 2, 8, 4, 2, 8, 4, 2, 16, 8, 4, 4, 2, 2, 8, 4, 2, 16, 8, 4 };
static const uint8 kBombosAnimDelays[] = { 5, 5, 5, 5, 5, 5, 5, 5, 3, 3, 3, 3, 3, 7, 1, 1, 1, 1, 1, 13 };
static const uint8 kBombosAnimStates[] = { 0, 1, 2, 3, 0, 1, 2, 3, 8, 9, 10, 11, 12, 10, 8, 13, 14, 15, 16, 17 };
static const uint8 kEtherAnimDelays[] = { 5, 5, 5, 5, 5, 5, 5, 5, 7, 7, 3, 3 };
static const uint8 kEtherAnimDelaysNoFlash[] = { 5, 5, 5, 5, 5, 5, 5, 5, 7, 7, 24, 24 };
static const uint8 kEtherAnimStates[] = { 0, 1, 2, 3, 0, 1, 2, 3, 4, 5, 6, 7 };
static const uint8 kQuakeAnimDelays[] = { 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 19 };
static const uint8 kQuakeAnimStates[] = { 0, 1, 2, 3, 0, 1, 2, 3, 18, 19, 20, 22 };
static inline uint8 BitSum4(uint8 t);
static inline uint8 BitSum4(uint8 t) {
  return (t & 1) + ((t >> 1) & 1) + ((t >> 2) & 1) + ((t >> 3) & 1);
}

void Dungeon_HandleLayerChange() {  // 81ff05
  link_is_on_lower_level_mirror = 1;
  if (kind_of_in_room_staircase == 0)
    BYTE(dungeon_room_index) += 16;
  if (kind_of_in_room_staircase != 2)
    link_is_on_lower_level = 1;
  about_to_jump_off_ledge = 0;
  SetAndSaveVisitedQuadrantFlags();
}

void CacheCameraProperties() {  // 81ff28
  BG2HOFS_copy2_cached = BG2HOFS_copy2;
  BG2VOFS_copy2_cached = BG2VOFS_copy2;
  link_y_coord_cached = link_y_coord;
  link_x_coord_cached = link_x_coord;
  room_scroll_vars_y_vofs1_cached = room_bounds_y.a0;
  room_scroll_vars_y_vofs2_cached = room_bounds_y.a1;
  room_scroll_vars_x_vofs1_cached = room_bounds_x.a0;
  room_scroll_vars_x_vofs2_cached = room_bounds_x.a1;
  up_down_scroll_target_cached = up_down_scroll_target;
  up_down_scroll_target_end_cached = up_down_scroll_target_end;
  left_right_scroll_target_cached = left_right_scroll_target;
  left_right_scroll_target_end_cached = left_right_scroll_target_end;
  camera_y_coord_scroll_low_cached = camera_y_coord_scroll_low;
  camera_x_coord_scroll_low_cached = camera_x_coord_scroll_low;
  quadrant_fullsize_x_cached = quadrant_fullsize_x;
  quadrant_fullsize_y_cached = quadrant_fullsize_y;
  link_quadrant_x_cached = link_quadrant_x;
  link_quadrant_y_cached = link_quadrant_y;
  link_direction_facing_cached = link_direction_facing;
  link_is_on_lower_level_cached = link_is_on_lower_level;
  link_is_on_lower_level_mirror_cached = link_is_on_lower_level_mirror;
  is_standing_in_doorway_cahed = is_standing_in_doorway;
  dung_cur_floor_cached = dung_cur_floor;
}

void CheckAbilityToSwim() {  // 81ffb6
  if (!link_is_bunny_mirror && link_item_flippers)
    return;
  if (link_item_moon_pearl)
    link_is_bunny_mirror = 0;
  link_visibility_status = 0xc;
  submodule_index = player_is_indoors ? 20 : 42;
}

void Link_Main() {  // 878000
//  RunEmulatedFunc(0x878000, 0, 0, 0, true, true, -2, 0);
//  return;



  link_x_coord_prev = link_x_coord;
  link_y_coord_prev = link_y_coord;
  flag_unk1 = 0;
  if (!flag_is_link_immobilized)
    Link_ControlHandler();
  HandleSomariaAndGraves();
}

void Link_ControlHandler() {  // 87807f
  if (link_give_damage) {
    if (link_cape_mode) {
      link_give_damage = 0;
      link_auxiliary_state = 0;
      link_incapacitated_timer = 0;
    } else {
      if (!link_disable_sprite_damage) {
        uint8 dmg = link_give_damage;
        link_give_damage = 0;
        if (ancilla_type[0] == 5 && player_handler_timer == 0 && link_delay_timer_spin_attack) {
          ancilla_type[0] = 0;
          flag_for_boomerang_in_place = 0;
        }
        if (countdown_for_blink == 0)
          countdown_for_blink = 58;
        Ancilla_Sfx2_Near(38);
        number_of_times_hurt_by_sprites++;
        uint8 new_dmg = link_health_current - dmg;
        if (new_dmg == 0 || new_dmg >= 0xa8) {
          mapbak_TM = TM_copy;
          mapbak_TS = TS_copy;
          saved_module_for_menu = main_module_index;
          main_module_index = 18;
          submodule_index = 1;
          countdown_for_blink = 0;
          link_hearts_filler = 0;
          new_dmg = 0;
        }
        link_health_current = new_dmg;
      }
    }
  }
  if (link_player_handler_state)
    Player_CheckHandleCapeStuff();
  kPlayerHandlers[link_player_handler_state]();
}

void LinkState_Default() {  // 878109
  CacheCameraPropertiesIfOutdoors();
  if (Link_HandleBunnyTransformation()) {
    if (link_player_handler_state == 23)
      PlayerHandler_17_Bunny();
    return;
  }
  fallhole_var2 = 0;
  if (link_auxiliary_state)
    HandleLink_From1D();
  else
    PlayerHandler_00_Ground_3();
}

void HandleLink_From1D() {  // 878130
  link_item_in_hand = 0;
  link_position_mode = 0;
  link_debug_value_1 = 0;
  link_debug_value_2 = 0;
  link_var30d = 0;
  link_var30e = 0;
  some_animation_timer_steps = 0;
  bitfield_for_a_button = 0;
  button_mask_b_y &= ~0x40;
  link_state_bits = 0;
  link_picking_throw_state = 0;
  link_grabbing_wall = 0;
  bitmask_of_dragstate = 0;
  Link_ResetSwimmingState();
  link_cant_change_direction &= ~1;
  link_z_coord &= 0xff;
  if (link_electrocute_on_touch != 0) {
    if (link_cape_mode)
      Link_ForceUnequipCape_quietly();
    Link_ResetSwordAndItemUsage();
    link_disable_sprite_damage = 1;
    player_handler_timer = 0;
    link_delay_timer_spin_attack = 2;
    link_animation_steps = 0;
    link_direction &= ~0xf;
    Ancilla_Sfx3_Near(43);
    link_player_handler_state = 7;
    LinkState_Zapped();
  } else {
    link_moving_against_diag_tile = 0;
    link_player_handler_state = 2;
    LinkState_Recoil();
  }
}

void PlayerHandler_00_Ground_3() {  // 8781a0
  g_ApplyLinksMovementToCamera_called = false;

  link_z_coord = 0xffff;
  link_actual_vel_z = 0xff;
  link_recoilmode_timer = 0;

  if (!Link_HandleToss()) {
    Link_HandleAPress();
    if ((link_state_bits | link_grabbing_wall) == 0 && link_unk_master_sword == 0 && link_player_handler_state != kPlayerState_StartDash) {
      Link_HandleYItem();
      // Ensure we're not handling potions. Things further
      // down don't assume this and change the module indexes randomly.
      // This also fixes a bug where bombos, ether, quake get aborted if you use spin attack at the same time.
      if ((enhanced_features0 & kFeatures0_MiscBugFixes) && (
          main_module_index == 14 && submodule_index != 2 ||
          link_player_handler_state == kPlayerState_Bombos ||
          link_player_handler_state == kPlayerState_Ether ||
          link_player_handler_state == kPlayerState_Quake))
        goto getout_clear_vel;
      if (sram_progress_indicator != 0) {
        Link_HandleSwordCooldown();
        if (link_player_handler_state == 3)
          goto getout_clear_vel;
      }
    }
  }

  Link_HandleCape_passive_LiftCheck();
  if (link_incapacitated_timer) {
    link_moving_against_diag_tile = 0;
    link_var30d = 0;
    link_var30e = 0;
    some_animation_timer_steps = 0;
    bitfield_for_a_button = 0;
    link_picking_throw_state = 0;
    link_state_bits = 0;
    link_grabbing_wall = 0;
    if (!(button_mask_b_y & 0x80))
      link_cant_change_direction &= ~1;
    Link_HandleRecoilAndTimer(false);
    return;
  }

  if (link_unk_master_sword) {
    link_direction = 0;
  } else if (!link_is_transforming && (link_grabbing_wall & ~2) == 0 && (link_state_bits & 0x7f) == 0 &&
             ((link_state_bits & 0x80) == 0 || (link_picking_throw_state & 1) == 0) && !link_item_in_hand && !link_position_mode &&
             (button_b_frames >= 9 || (button_mask_b_y & 0x20) != 0 || (button_mask_b_y & 0x80) == 0)) {
    // if_4

    if (link_flag_moving) {
      swimcoll_var9[0] = swimcoll_var9[1] = 0x180;
      Link_HandleSwimMovements();
      return;
    }
    ResetAllAcceleration();
    uint8 dir;

    if ((dir = (force_move_any_direction & 0xf)) == 0) {
      if (link_grabbing_wall & 2)
        goto endif_3;
      if ((dir = (joypad1H_last & kJoypadH_AnyDir)) == 0) {
        link_x_vel = 0;
        link_y_vel = 0;
        link_direction = 0;
        link_direction_last = 0;
        link_animation_steps = 0;
        bitmask_of_dragstate &= ~0xf;
        link_timer_push_get_tired = 32;
        link_timer_jump_ledge = 19;
        goto endif_3;
      }
    }
    link_direction = dir;
    if (dir != link_direction_last) {
      link_direction_last = dir;
      link_subpixel_x = link_subpixel_y = 0;
      link_moving_against_diag_tile = 0;
      bitmask_of_dragstate = 0;
      link_timer_push_get_tired = 32;
      link_timer_jump_ledge = 19;
    }
  }
  // endif_3
endif_3:
  Link_HandleDiagonalCollision();
  Link_HandleVelocity();
  Link_HandleCardinalCollision();
  Link_HandleMovingAnimation_FullLongEntry();
  if (link_unk_master_sword) getout_clear_vel: {
    link_y_vel = link_x_vel = 0;
  }

  fallhole_var1 = 0;

  // HandleIndoorCameraAndDoors must not be called twice in the same frame,
  // this might mess up camera positioning. For example when using spin attack
  // in between bumpers.
  if (g_ApplyLinksMovementToCamera_called && (enhanced_features0 & kFeatures0_MiscBugFixes))
    return;

  HandleIndoorCameraAndDoors();
}

bool Link_HandleBunnyTransformation() {  // 8782da
  if (!link_timer_tempbunny)
    return false;

  if (!link_need_for_poof_for_transform) {
    if (link_player_handler_state == kPlayerState_PermaBunny || link_player_handler_state == kPlayerState_TempBunny) {
      link_timer_tempbunny = 0;
      return false;
    }
    if (link_picking_throw_state & 2)
      link_state_bits = 0;
    uint8 bak = link_state_bits & 0x80;
    Link_ResetProperties_A();
    link_state_bits = bak;

    for (int i = 4; i >= 0; i--) {
      if (ancilla_type[i] == 0x30 || ancilla_type[i] == 0x31)
        ancilla_type[i] = 0;
    }
    Link_CancelDash();
    AncillaAdd_CapePoof(0x23, 4);
    Ancilla_Sfx2_Near(0x14);
    link_bunny_transform_timer = 20;
    link_disable_sprite_damage = 1;
    link_need_for_poof_for_transform = 1;
    link_visibility_status = 12;
  }
  if (sign8(--link_bunny_transform_timer)) {
    link_player_handler_state = kPlayerState_TempBunny;
    link_is_bunny_mirror = 1;
    link_is_bunny = 1;
    LoadGearPalettes_bunny();
    link_visibility_status = 0;
    link_disable_sprite_damage = 0;
    link_need_for_poof_for_transform = 0;
  }
  return true;
}

void LinkState_TemporaryBunny() {  // 878365
  if (!link_timer_tempbunny) {
    AncillaAdd_CapePoof(0x23, 4);
    Ancilla_Sfx2_Near(0x15);
    link_bunny_transform_timer = 32;
    link_player_handler_state = 0;
    Link_ResetProperties_C();
    link_need_for_poof_for_transform = 0;
    link_is_bunny = 0;
    link_is_bunny_mirror = 0;
    LoadActualGearPalettes();
    link_need_for_poof_for_transform = 0;
    LinkState_Default();
  } else {
    link_timer_tempbunny--;
    PlayerHandler_17_Bunny();
  }
}

void PlayerHandler_17_Bunny() {  // 8783a1
  CacheCameraPropertiesIfOutdoors();
  fallhole_var2 = 0;
  if (!link_is_in_deep_water) {
    if (link_auxiliary_state == 0) {
      Link_TempBunny_Func2();
      return;
    }
    if (link_item_moon_pearl)
      link_is_bunny_mirror = 0;
  }
  LinkState_Bunny_recache();
}

void LinkState_Bunny_recache() {  // 8783c7
  link_need_for_poof_for_transform = 0;
  link_timer_tempbunny = 0;
  if (link_item_moon_pearl) {
    link_is_bunny = 0;
    link_auxiliary_state = 0;
  }
  link_animation_steps = 0;
  link_is_transforming = 0;
  link_cant_change_direction = 0;
  Link_ResetSwimmingState();
  link_player_handler_state = kPlayerState_RecoilWall;
  if (link_item_moon_pearl) {
    link_player_handler_state = kPlayerState_Ground;
    LoadActualGearPalettes();
  }
}

void Link_TempBunny_Func2() {  // 8783fa
  if (link_incapacitated_timer != 0) {
    Link_HandleRecoilAndTimer(false);
    return;
  }
  link_z_coord = 0xffff;
  link_actual_vel_z = 0xff;
  link_recoilmode_timer = 0;
  if (link_flag_moving) {
    swimcoll_var9[0] = swimcoll_var9[1] = 0x180;
    Link_HandleSwimMovements();
    return;
  }

  ResetAllAcceleration();
  Link_HandleYItem();
  uint8 dir;
  if (!(dir = force_move_any_direction & 0xf) && !(dir = joypad1H_last & kJoypadH_AnyDir)) {
    link_x_vel = link_y_vel = 0;
    link_direction = link_direction_last = 0;
    link_animation_steps = 0;
    bitmask_of_dragstate &= ~9;
    link_timer_push_get_tired = 32;
    link_timer_jump_ledge = 19;
  } else {
    link_direction = dir;
    if (dir != link_direction_last) {
      link_direction_last = dir;
      link_subpixel_x = 0;
      link_subpixel_y = 0;
      link_moving_against_diag_tile = 0;
      bitmask_of_dragstate = 0;
      link_timer_push_get_tired = 32;
      link_timer_jump_ledge = 19;
    }
  }
  Link_HandleDiagonalCollision();
  Link_HandleVelocity();
  Link_HandleCardinalCollision();
  Link_HandleMovingAnimation_FullLongEntry();
  fallhole_var1 = 0;
  HandleIndoorCameraAndDoors();
}

void LinkState_HoldingBigRock() {  // 878481
  if (link_auxiliary_state) {
    link_item_in_hand = 0;
    link_position_mode = 0;
    link_debug_value_1 = 0;
    link_debug_value_2 = 0;
    link_var30d = 0;
    link_var30e = 0;
    some_animation_timer_steps = 0;
    bitfield_for_a_button = 0;
    link_state_bits = 0;
    link_picking_throw_state = 0;
    link_grabbing_wall = 0;
    bitmask_of_dragstate = 0;
    link_cant_change_direction &= ~1;
    link_z_coord &= ~0xff;
    if (link_electrocute_on_touch) {
      Link_ResetSwordAndItemUsage();
      link_disable_sprite_damage = 1;
      player_handler_timer = 0;
      link_delay_timer_spin_attack = 2;
      link_animation_steps = 0;
      link_direction &= ~0xf;
      Ancilla_Sfx3_Near(43);
      link_player_handler_state = kPlayerState_Electrocution;
      LinkState_Zapped();
    } else {
      link_player_handler_state = kPlayerState_RecoilWall;
      LinkState_Recoil();
    }
    return;
  }

  link_z_coord = 0xffff;
  link_actual_vel_z = 0xff;
  link_recoilmode_timer = 0;
  if (link_incapacitated_timer) {
    link_var30d = 0;
    link_var30e = 0;
    some_animation_timer_steps = 0;
    bitfield_for_a_button = 0;
    link_state_bits = 0;
    link_picking_throw_state = 0;
    link_grabbing_wall = 0;
    if (!(button_mask_b_y & 0x80))
      link_cant_change_direction &= ~1;
    Link_HandleRecoilAndTimer(false);
    return;
  }

  Link_HandleAPress();
  if (!(joypad1H_last & kJoypadH_AnyDir)) {
    link_y_vel = 0;
    link_x_vel = 0;
    link_direction = 0;
    link_direction_last = 0;
    link_animation_steps = 0;
    bitmask_of_dragstate &= ~9;
    link_timer_push_get_tired = 32;
    link_timer_jump_ledge = 19;
  } else {
    link_direction = joypad1H_last & kJoypadH_AnyDir;
    if (link_direction != link_direction_last) {
      link_direction_last = link_direction;
      link_subpixel_x = 0;
      link_subpixel_y = 0;
      link_moving_against_diag_tile = 0;
      bitmask_of_dragstate = 0;
      link_timer_push_get_tired = 32;
      link_timer_jump_ledge = 19;
    }
  }
  Link_HandleMovingAnimation_FullLongEntry();
  fallhole_var1 = 0;
  HandleIndoorCameraAndDoors();
}

void EtherTablet_StartCutscene() {  // 87855a
  button_b_frames = 0xc0;
  link_delay_timer_spin_attack = 0;
  link_player_handler_state = kPlayerState_ReceivingEther;
  link_disable_sprite_damage = 1;
  flag_block_link_menu = 1;
}

void LinkState_ReceivingEther() {  // 878570
  link_auxiliary_state = 0;
  link_incapacitated_timer = 0;
  link_give_damage = 0;
  int i = --WORD(button_b_frames);
  if (sign16(i)) {
    button_b_frames = 0;
    link_delay_timer_spin_attack = 0;
  } else if (i == 0xbf) {
    link_force_hold_sword_up = 1;
  } else if (i == 160) {
    uint16 x = link_x_coord, y = link_y_coord;
    link_x_coord = 0x6b0;
    link_y_coord = 0x37;
    AncillaAdd_EtherSpell(0x18, 0);
    link_x_coord = x, link_y_coord = y;
  } else if (i == 0) {
    AncillaAdd_FallingPrize(0x29, 0, 4);
    flag_is_link_immobilized = 1;
    flag_block_link_menu = 0;
  }
}

void BombosTablet_StartCutscene() {  // 8785e5
  button_b_frames = 0xe0;
  link_delay_timer_spin_attack = 0;
  link_player_handler_state = kPlayerState_ReceivingBombos;
  link_disable_sprite_damage = 1;
  flag_custom_spell_anim_active = 1;
}

void LinkState_ReceivingBombos() {  // 8785fb
  link_auxiliary_state = 0;
  link_incapacitated_timer = 0;
  link_give_damage = 0;
  int i = --WORD(button_b_frames);
  if (sign16(i)) {
    button_b_frames = 0;
    link_delay_timer_spin_attack = 0;
  } else if (i == 223) {
    link_force_hold_sword_up = 1;
  } else if (i == 160) {
    uint16 x = link_x_coord, y = link_y_coord;
    link_x_coord = 0x378;
    link_y_coord = 0xeb0;
    AncillaAdd_BombosSpell(0x19, 0);
    link_x_coord = x, link_y_coord = y;
  } else if (i == 0) {
    AncillaAdd_FallingPrize(0x29, 5, 4);
    flag_is_link_immobilized = 1;
  }
}

void LinkState_ReadingDesertTablet() {  // 87867b
  if (!--button_b_frames) {
    link_player_handler_state = kPlayerState_Ground;
    Link_PerformDesertPrayer();
  }
}

void HandleSomariaAndGraves() {  // 878689
  if (!player_is_indoors && link_something_with_hookshot) {
    int i = 4;
    do {
      if (ancilla_type[i] == 0x24)
        Gravestone_Move(i);
    } while (--i >= 0);
  }
  int i = 4;
  do {
    if (ancilla_type[i] == 0x2C) {
      SomariaBlock_HandlePlayerInteraction(i);
      return;
    }
  } while (--i >= 0);
}

void LinkState_Recoil() {  // 8786b5
  link_y_coord_safe_return_lo = link_y_coord;
  link_y_coord_safe_return_hi = link_y_coord >> 8;
  link_x_coord_safe_return_lo = link_x_coord;
  link_x_coord_safe_return_hi = link_x_coord >> 8;
  Link_HandleChangeInZVelocity();
  link_cant_change_direction = 0;
  draw_water_ripples_or_grass = 0;
  if (!sign8(link_z_coord) || !sign8(link_actual_vel_z)) {
    Link_HandleRecoilAndTimer(false);
    return;
  }
  TileDetect_MainHandler(5);
  if (tiledetect_deepwater & 1) {
    link_player_handler_state = kPlayerState_Swimming;
    Link_SetToDeepWater();
    Link_ResetSwordAndItemUsage();
    AncillaAdd_Splash(21, 0);
    Link_HandleRecoilAndTimer(true);
  } else {
    if (++link_recoilmode_timer != 4) {
      uint8 t = link_actual_vel_z_copy, s = link_recoilmode_timer;
      do {
        t >>= 1;
      } while (!--s); // wtf?
      link_actual_vel_z = t;
    } else {
      link_recoilmode_timer = 3;
    }
    Link_HandleRecoilAndTimer(false);
  }
}

void Link_HandleRecoilAndTimer(bool jump_into_middle) {  // 878711
  if (jump_into_middle)
    goto lbl_jump_into_middle;

  link_x_page_movement_delta = 0;
  link_y_page_movement_delta = 0;
  link_num_orthogonal_directions = 0;
  Link_HandleRecoiling();  // not
  if (--link_incapacitated_timer == 0) {
    link_incapacitated_timer = 1;
    int8 z;
    z = link_z_coord & 0xfe;
    if (z <= 0 && (int8)link_actual_vel_z < 0) {
      if (link_auxiliary_state != 0) {
        link_disable_sprite_damage = 0;
        scratch_0 = link_player_handler_state;
        if (link_player_handler_state != 6) {
          button_b_frames = 0;
          button_mask_b_y = 0;
          link_delay_timer_spin_attack = 0;
          link_spin_attack_step_counter = 0;
        }
        Link_SplashUponLanding();
        if (!link_is_bunny_mirror || !link_is_in_deep_water) {
          if (link_want_make_noise_when_dashed) {
            link_want_make_noise_when_dashed = 0;
            Ancilla_Sfx2_Near(33);
          } else if (scratch_0 != 2 && link_player_handler_state != 4) {
            Ancilla_Sfx2_Near(33);
          }
          if (link_player_handler_state == 4) {
            Link_ForceUnequipCape_quietly();
            if (player_is_indoors && scratch_0 != 2 && link_item_flippers) {
              link_is_on_lower_level = 1;
            }
            AncillaAdd_Splash(21, 0);
          }
          TileDetect_MainHandler(0);
          if (tiledetect_thick_grass & 1)
            Ancilla_Sfx2_Near(26);
          if (tiledetect_shallow_water & 1 && sound_effect_1 != 36)
            Ancilla_Sfx2_Near(28);

          if (tiledetect_deepwater & 1) {
            link_player_handler_state = kPlayerState_Swimming;
            Link_SetToDeepWater();
            Link_ResetSwordAndItemUsage();
            AncillaAdd_Splash(21, 0);
          }

          // OMG something jumps to here...
lbl_jump_into_middle:
          if (link_is_on_lower_level == 2)
            link_is_on_lower_level = 0;
          if (about_to_jump_off_ledge)
            Dungeon_HandleLayerChange();
        }
        link_z_coord = 0;
        link_auxiliary_state = 0;
        link_speed_setting = 0;
        link_cant_change_direction = 0;
        link_item_in_hand = 0;
        link_position_mode = 0;
        player_handler_timer = 0;
        link_disable_sprite_damage = 0;
        link_electrocute_on_touch = 0;
        link_actual_vel_x = 0;
        link_actual_vel_y = 0;
      }
      link_animation_steps = 0;
      link_incapacitated_timer = 0;
    }
  }

  if (link_player_handler_state != 5 && link_incapacitated_timer >= 33) {
    if (!sign8(--byte_7E02C5))
      goto timer_running;
    byte_7E02C5 = link_incapacitated_timer >> 4;
  }

  Flag67WithDirections();
  if (link_player_handler_state != 6) {
    Link_HandleDiagonalCollision();  // not
    if ((link_direction & 3) == 0)
      link_actual_vel_x = 0;
    if ((link_direction & 0xc) == 0)
      link_actual_vel_y = 0;
  }
  Link_MovePosition(); // not
timer_running:
  if (link_player_handler_state != 6) {
    Link_HandleCardinalCollision(); // not
    fallhole_var1 = 0;
  }
  HandleIndoorCameraAndDoors();
  if (BYTE(link_z_coord) == 0 || BYTE(link_z_coord) >= 0xe0) {
    Player_TileDetectNearby();
    if ((tiledetect_pit_tile & 0xf) == 0xf) {
      link_player_handler_state = kPlayerState_FallingIntoHole;
      link_speed_setting = 4;
    }
  }
  HIBYTE(link_z_coord) = 0;
}

void LinkState_OnIce() {  // 878872
  assert(0);
}

void Link_HandleChangeInZVelocity() {  // 878926
  Player_ChangeZ(link_player_handler_state == kPlayerState_TurtleRock ? 1 : 2);
}

void Player_ChangeZ(uint8 zd) {  // 878932
  if (sign8(link_actual_vel_z)) {
    if (!(uint8)link_z_coord)
      return;
    if (sign8(link_z_coord)) {
      link_z_coord = 0xffff;
      link_actual_vel_z = 0xff;
      return;
    }
  }
  link_actual_vel_z -= zd;
}

void LinkHop_HoppingSouthOW() {  // 87894e
  link_last_direction_moved_towards = 1;
  link_cant_change_direction = 0;
  link_actual_vel_x = 0;
  link_actual_vel_y = 0;
  draw_water_ripples_or_grass = 0;
  if (!link_incapacitated_timer && !link_actual_vel_z_mirror) {
    Ancilla_Sfx2_Near(32);
    LinkHop_FindTileToLandOnSouth();
    if (!player_is_indoors)
      link_is_on_lower_level = 2;
  }
  link_actual_vel_z = link_actual_vel_z_mirror;
  link_actual_vel_z_copy = link_actual_vel_z_copy_mirror;
  link_z_coord = link_z_coord_mirror;
  link_actual_vel_z -= 2;
  Link_MovePosition();
  if (sign8(link_actual_vel_z)) {
    if (link_actual_vel_z < 0xa0)
      link_actual_vel_z = 0xa0;
    if (link_z_coord >= 0xfff0) {
      link_z_coord = 0;
      Link_SplashUponLanding();
      // This is the place that caused the water walking bug after bonk, 
      // player_near_pit_state was not reset.
      if (player_near_pit_state)
        link_player_handler_state = kPlayerState_FallingIntoHole;
      if (link_player_handler_state != kPlayerState_Swimming &&
          link_player_handler_state != kPlayerState_FallingIntoHole && !link_is_in_deep_water)
        Ancilla_Sfx2_Near(33);
      link_disable_sprite_damage = 0;
      allow_scroll_z = 0;
      link_auxiliary_state = 0;
      link_actual_vel_z = 0xff;
      link_z_coord = 0xffff;
      link_incapacitated_timer = 0;
      if (!player_is_indoors)
        link_is_on_lower_level = 0;
    } else {
      link_y_vel = link_z_coord_mirror - link_z_coord;
    }
  } else {
    link_y_vel = link_z_coord_mirror - link_z_coord;
  }
  link_actual_vel_z_mirror = link_actual_vel_z;
  link_actual_vel_z_copy_mirror = link_actual_vel_z_copy;
  link_z_coord_mirror = link_z_coord;
}

void LinkState_HandlingJump() {  // 878a05
  link_actual_vel_z = link_actual_vel_z_mirror;
  link_actual_vel_z_copy = link_actual_vel_z_copy_mirror;
  BYTE(link_z_coord) = link_z_coord_mirror;
  link_actual_vel_z -= 2;
  Link_MovePosition();
  if (sign8(link_actual_vel_z)) {
    if (link_actual_vel_z < 0xa0)
      link_actual_vel_z = 0xa0;
    if ((uint8)link_z_coord >= 0xf0) {
      link_z_coord = 0;
      if (link_player_handler_state == kPlayerState_FallOfLeftRightLedge || link_player_handler_state == kPlayerState_JumpOffLedgeDiag) {
        TileDetect_MainHandler(0);
        if (tiledetect_deepwater & 1) {
          link_player_handler_state = kPlayerState_Swimming;
          Link_SetToDeepWater();
          Link_ResetSwordAndItemUsage();
          AncillaAdd_Splash(21, 0);
        } else if (tiledetect_pit_tile & 1) {
          byte_7E005C = 9;
          link_this_controls_sprite_oam = 0;
          player_near_pit_state = 1;
          link_player_handler_state = kPlayerState_FallingIntoHole;
          goto after_pit;
        }
      }
      Link_SplashUponLanding();
      if (link_player_handler_state != kPlayerState_Swimming && !link_is_in_deep_water)
        Ancilla_Sfx2_Near(33);
after_pit:
      if (link_player_handler_state != kPlayerState_Swimming || !link_is_bunny_mirror)
        link_disable_sprite_damage = 0;

      allow_scroll_z = 0;
      link_auxiliary_state = 0;
      link_actual_vel_z = 0xff;
      link_z_coord = 0xffff;
      link_incapacitated_timer = 0;
      if (!player_is_indoors)
        link_is_on_lower_level = 0;
    } else {
      link_y_vel = link_z_coord_mirror - link_z_coord;
    }
  } else {
    link_y_vel = link_z_coord_mirror - link_z_coord;
  }
  link_actual_vel_z_mirror = link_actual_vel_z;
  link_actual_vel_z_copy_mirror = link_actual_vel_z_copy;
  BYTE(link_z_coord_mirror) = link_z_coord;
}

void LinkHop_FindTileToLandOnSouth() {  // 878ad1
  link_y_coord_original = link_y_coord;
  link_y_vel = link_y_coord - link_y_coord_safe_return_lo;
  for (;;) {
    link_y_coord += kLink_DoMoveXCoord_Outdoors_Helper2_y[link_last_direction_moved_towards];
    TileDetect_Movement_Y(link_last_direction_moved_towards);
    uint8 k = tiledetect_normal_tiles | tiledetect_pit_tile | tiledetect_destruction_aftermath | tiledetect_thick_grass | tiledetect_deepwater;
    if ((k & 7) == 7)
      break;
  }
  if (tiledetect_deepwater & 7) {
    link_is_in_deep_water = 1;
    if (link_auxiliary_state != 4)
      link_auxiliary_state = 2;
    link_some_direction_bits = link_direction_last;
    Link_ResetSwimmingState();
    link_grabbing_wall = 0;
    link_speed_setting = 0;
  }
  if (tiledetect_pit_tile & 7) {
    byte_7E005C = 9;
    link_this_controls_sprite_oam = 0;
    player_near_pit_state = 1;
  }
  link_y_coord += kLink_DoMoveXCoord_Outdoors_Helper2_y2[link_last_direction_moved_towards];
  link_y_coord_safe_return_lo = link_y_coord;
  link_y_coord_safe_return_hi = link_y_coord >> 8;
  link_incapacitated_timer = 1;
  uint8 z = link_z_coord;
  if (z >= 0xf0)
    z = 0;
  link_z_coord = link_z_coord_mirror = link_y_coord - link_y_coord_original + z;
}

// used on right ledges
void LinkState_HoppingHorizontallyOW() {  // 878b74
  link_direction = sign8(link_actual_vel_x) ? 6 : 5;
  link_cant_change_direction = 0;
  link_actual_vel_y = 0;
  draw_water_ripples_or_grass = 0;
  LinkState_HandlingJump();
}

void Link_HoppingHorizontally_FindTile_Y() {  // 878b9b
  link_y_coord_original = link_y_coord;
  link_y_vel = link_y_coord - link_y_coord_safe_return_lo;

  link_y_coord += kLink_DoMoveXCoord_Outdoors_Helper2_y[link_last_direction_moved_towards];
  TileDetect_Movement_Y(link_last_direction_moved_towards);

  uint8 tt = tiledetect_normal_tiles | tiledetect_destruction_aftermath | tiledetect_thick_grass |
    tiledetect_deepwater;

  if ((tt & 7) != 7) {
    link_y_coord = link_y_coord_original;
    link_incapacitated_timer = 1;

    int8 velx = link_actual_vel_x, org_velx = velx;
    if (velx < 0) velx = -velx;
    velx >>= 4;
    link_actual_vel_z_mirror = link_actual_vel_z_copy_mirror = kLink_DoMoveXCoord_Outdoors_Helper2_velz[velx];

    uint8 xt = kLink_DoMoveXCoord_Outdoors_Helper2_velx[velx];
    if (org_velx < 0) xt = -xt;
    link_actual_vel_x = xt;
  } else {  // else_1
    link_y_coord += kLink_DoMoveXCoord_Outdoors_Helper2_y2[link_last_direction_moved_towards];
    link_y_coord_safe_return_lo = link_y_coord;
    link_y_coord_safe_return_hi = link_y_coord >> 8;
    link_incapacitated_timer = 1;
    uint8 z = link_z_coord;
    if (z == 255) z = 0;
    link_z_coord_mirror = link_z_coord = link_y_coord - link_y_coord_original + z;
  }  // endif_1

  if (tiledetect_deepwater & 7) {
    link_auxiliary_state = 2;
    Link_SetToDeepWater();
  }
}

void Link_SetToDeepWater() {  // 878c44
  link_is_in_deep_water = 1;
  link_some_direction_bits = link_direction_last;
  Link_ResetSwimmingState();
  link_grabbing_wall = 0;
  link_speed_setting = 0;
}

void LinkState_0F() {  // 878c69
  assert(0);
}

uint8 Link_HoppingHorizontally_FindTile_X(uint8 o) {  // 878d2b
  assert(o == 0 || o == 2);
  link_y_coord_original = link_x_coord;
  int i = 7;
  static const int8 kLink_DoMoveXCoord_Outdoors_Helper1_tab1[2] = { -8, 8 };
  static const int8 kLink_DoMoveXCoord_Outdoors_Helper1_tab2[2] = { -32, 32 };
  static const int8 kLink_DoMoveXCoord_Outdoors_Helper1_tab3[2] = { -16, 16 };
  static const uint8 kLink_DoMoveXCoord_Outdoors_Helper1_velx[24] = { 20, 20, 20, 24, 24, 24, 24, 28, 28, 36, 36, 36, 36, 36, 36, 38, 38, 38, 38, 38, 38, 38, 40, 40 };
  static const uint8 kLink_DoMoveXCoord_Outdoors_Helper1_velz[24] = { 20, 20, 20, 20, 20, 20, 20, 24, 24, 32, 32, 32, 36, 36, 36, 38, 38, 38, 38, 38, 38, 38, 40, 40 };
  do {
    link_x_coord += kLink_DoMoveXCoord_Outdoors_Helper1_tab1[o >> 1];
    TileDetect_Movement_X(link_last_direction_moved_towards);

    uint8 tt = tiledetect_normal_tiles | tiledetect_destruction_aftermath | tiledetect_thick_grass |
      tiledetect_deepwater | tiledetect_pit_tile;

    if ((tt & 7) == 7) {
      if ((tiledetect_deepwater & 7) == 7) {
        link_is_in_deep_water = 1;
        link_auxiliary_state = 2;
        link_some_direction_bits = link_direction_last;
        swimming_countdown = 0;
        link_speed_setting = 0;
        link_grabbing_wall = 0;
        ResetAllAcceleration();
      }
      goto finish;
    }
  } while (--i >= 0);

  link_x_coord = link_y_coord_original + kLink_DoMoveXCoord_Outdoors_Helper1_tab2[o >> 1];
finish:
  link_x_coord += kLink_DoMoveXCoord_Outdoors_Helper1_tab3[o >> 1];
  int16 xt = link_y_coord_original - link_x_coord;
  if (xt < 0) xt = -xt;
  xt >>= 3;
  uint8 velx = kLink_DoMoveXCoord_Outdoors_Helper1_velx[xt];
  if (o != 2) velx = -velx;
  link_actual_vel_x = velx;
  link_actual_vel_z_mirror = link_actual_vel_z_copy_mirror = kLink_DoMoveXCoord_Outdoors_Helper1_velz[xt];

  return i;
}

// used on diag ledges
void LinkState_HoppingDiagonallyUpOW() {  // 878dc6
  draw_water_ripples_or_grass = 0;
  Player_ChangeZ(2);
  Link_MovePosition();
  if (sign8(link_z_coord)) {
    Link_SplashUponLanding();
    if (link_player_handler_state != kPlayerState_Swimming && !link_is_in_deep_water)
      Ancilla_Sfx2_Near(33);
    link_disable_sprite_damage = 0;
    link_auxiliary_state = 0;
    link_actual_vel_z = 0xff;
    link_z_coord = 0xffff;
    link_incapacitated_timer = 0;
    link_cant_change_direction = 0;
  }
}

void LinkState_HoppingDiagonallyDownOW() {  // 878e15
  uint8 dir = sign8(link_actual_vel_x) ? 2 : 3;
  link_last_direction_moved_towards = dir;
  link_cant_change_direction = 0;
  link_actual_vel_y = 0;
  draw_water_ripples_or_grass = 0;
  if (!link_incapacitated_timer && !link_actual_vel_z_mirror) {
    link_last_direction_moved_towards = 1;
    uint16 old_x = link_x_coord;
    Ancilla_Sfx2_Near(32);
    LinkHop_FindLandingSpotDiagonallyDown();
    link_x_coord = old_x;

    static const uint8 kLedgeVelX[] = { 
      4, 4, 4, 10, 10, 10, 11, 18,
      18, 18, 20, 20, 20, 20, 22, 22,
      26, 26, 26, 26, 28, 28, 28, 28
    };

    int t = (uint16)(link_y_coord - link_y_coord_original);
    // Fix out of bounds read
    int8 velx = kLedgeVelX[IntMin(t >> 3, 23)];
    link_actual_vel_x = (dir != 2) ? velx : -velx;
    if (!player_is_indoors)
      link_is_on_lower_level = 2;
  }
  LinkState_HandlingJump();
}

void LinkHop_FindLandingSpotDiagonallyDown() {  // 878e7b
  static const int8 kLink_Ledge_Func1_dx[2] = { -8, 8 };
  static const int8 kLink_Ledge_Func1_dy[2] = { -9, 9 };
  static const uint8 kLink_Ledge_Func1_bits[2] = { 6, 3 };
  static const int8 kLink_Ledge_Func1_dy2[2] = { -24, 24 };
  link_y_coord_original = link_y_coord;
  link_y_vel = link_y_coord - link_y_coord_safe_return_lo;

  uint8 scratch;
  for (;;) {
    int o = sign8(link_actual_vel_x) ? 0 : 1;

    link_x_coord += kLink_Ledge_Func1_dx[o];
    link_y_coord += kLink_Ledge_Func1_dy[link_last_direction_moved_towards];
    TileDetect_Movement_Y(link_last_direction_moved_towards);
    scratch = kLink_Ledge_Func1_bits[o];
    uint8 k = tiledetect_normal_tiles | tiledetect_destruction_aftermath | tiledetect_thick_grass | tiledetect_deepwater;
    if ((k & scratch) == scratch)
      break;
  }

  if (tiledetect_deepwater & scratch) {
    link_is_in_deep_water = 1;
    link_auxiliary_state = 2;
    link_some_direction_bits = link_direction_last;
    Link_ResetSwimmingState();
    link_speed_setting = 0;
    link_grabbing_wall = 0;
  }

  link_y_coord += kLink_Ledge_Func1_dy2[link_last_direction_moved_towards];
  link_y_coord_safe_return_lo = link_y_coord;
  link_y_coord_safe_return_hi = link_y_coord >> 8;
  link_incapacitated_timer = 1;
  link_z_coord_mirror = link_y_coord - link_y_coord_original + (uint8)link_z_coord;
  link_z_coord = link_z_coord_mirror;
}

void Link_SplashUponLanding() {  // 878f1d
  if (link_is_bunny_mirror) {
    if (link_is_in_deep_water) {
      AncillaAdd_Splash(21, 0);
      LinkState_Bunny_recache();
      return;
    }
    link_player_handler_state = (link_item_moon_pearl) ? kPlayerState_TempBunny : kPlayerState_PermaBunny;
  } else if (link_is_in_deep_water) {
    if (link_player_handler_state != kPlayerState_RecoilOther)
      AncillaAdd_Splash(21, 0);
    Link_ForceUnequipCape_quietly();
    link_player_handler_state = kPlayerState_Swimming;
  } else {
    link_player_handler_state = kPlayerState_Ground;
  }
}

void LinkState_Dashing() {  // 878f86
  CacheCameraPropertiesIfOutdoors();
  if (Link_HandleBunnyTransformation()) {
    if (link_player_handler_state == 23)
      PlayerHandler_17_Bunny();
    return;
  }
  if (!link_is_running) {
    link_disable_sprite_damage = 0;
    link_countdown_for_dash = 0;
    link_speed_setting = 0;
    link_player_handler_state = kPlayerState_Ground;
    link_cant_change_direction = 0;
    return;
  }

  if (button_mask_b_y & 0x80) {
    if (button_b_frames >= 9)
      button_b_frames = 9;
  }
  fallhole_var2 = 0;

  if (link_auxiliary_state) {
    link_disable_sprite_damage = 0;
    link_countdown_for_dash = 0;
    link_speed_setting = 0;
    link_cant_change_direction = 0;
    link_is_running = 0;
    bitmask_of_dragstate = 0;
    if (link_electrocute_on_touch) {
      if (link_cape_mode)
        Link_ForceUnequipCape_quietly();
      Link_ResetSwordAndItemUsage();
      link_disable_sprite_damage = 1;
      player_handler_timer = 0;
      link_delay_timer_spin_attack = 2;
      link_animation_steps = 0;
      link_direction &= ~0xf;
      Ancilla_Sfx3_Near(43);
      link_player_handler_state = kPlayerState_Electrocution;
      LinkState_Zapped();
    } else {
      link_player_handler_state = kPlayerState_RecoilWall;
      LinkState_Recoil();
    }
    return;
  }
  static const uint8 kDashTab1[] = { 7, 15, 15 };
  static const uint8 kDashTab2[] = { 8, 4, 2, 1 };
  uint8 a = link_countdown_for_dash;
  if (a == 0)
    a = index_of_dashing_sfx--;
  if (!(kDashTab1[link_countdown_for_dash >> 4] & a))
    Ancilla_Sfx2_Near(35);
  if (sign8(--link_countdown_for_dash)) {
    link_countdown_for_dash = 0;
    if (follower_indicator == kTagalongArr1[follower_indicator])
      follower_indicator = kTagalongArr2[follower_indicator];
  } else {
    index_of_dashing_sfx = 0;
    if (!(joypad1L_last & kJoypadL_A)) {
      link_animation_steps = 0;
      link_countdown_for_dash = 0;
      link_speed_setting = 0;
      link_player_handler_state = kPlayerState_Ground;
      link_is_running = 0;
      if (!(button_mask_b_y & 0x80))
        link_cant_change_direction = 0;
      return;
    }
    AncillaAdd_DashDust_charging(30, 0);
    link_x_vel = link_y_vel = 0;
    link_dash_ctr = 64;
    link_speed_setting = 16;
    uint8 dir;
    if (button_mask_b_y & 0x80 || is_standing_in_doorway || (dir = joypad1H_last & kJoypadH_AnyDir) == 0)
      dir = kDashTab2[link_direction_facing >> 1];
    link_some_direction_bits = link_direction = link_direction_last = dir;
    link_moving_against_diag_tile = 0;
    Link_HandleMovingAnimation_FullLongEntry();
    uint16 org_x = link_x_coord, org_y = link_y_coord;
    link_y_coord_safe_return_lo = link_y_coord;
    link_y_coord_safe_return_hi = link_y_coord >> 8;
    link_x_coord_safe_return_lo = link_x_coord;
    link_x_coord_safe_return_hi = link_x_coord >> 8;
    Link_HandleMovingFloor();
    Link_ApplyConveyor();
    if (player_on_somaria_platform)
      Link_HandleVelocityAndSandDrag(org_x, org_y);
    link_y_vel = link_y_coord - link_y_coord_safe_return_lo;
    link_x_vel = link_x_coord - link_x_coord_safe_return_lo;
    Link_HandleCardinalCollision();
    HandleIndoorCameraAndDoors();
    return;
  }

  if (link_animation_steps >= 6)
    link_animation_steps = 0;

  link_dash_ctr--;
  if (link_dash_ctr < 32)
    link_dash_ctr = 32;

  AncillaAdd_DashDust(30, 0);
  link_spin_attack_step_counter = 0;

  if ((uint8)(link_sword_type + 1) & 0xfe)
    TileDetect_MainHandler(7);

  if (sram_progress_indicator) {
    button_mask_b_y |= 0x80;
    button_b_frames = 9;
  }

  link_incapacitated_timer = 0;

  bool want_stop_dash = false;

  if (enhanced_features0 & kFeatures0_TurnWhileDashing) {
    if (!(joypad1L_last & kJoypadL_A)) {
      link_countdown_for_dash = 0x11;
      want_stop_dash = true;
    } else {
      static const uint8 kDashCtrlsToDir[16] = { 0, 1, 2, 0, 4, 4, 4, 0, 8, 8, 8, 0, 0, 0, 0, 0 };
      uint8 t = kDashCtrlsToDir[joypad1H_last & kJoypadH_AnyDir];
      if (t != 0 && t != link_direction_last) {
        link_direction = link_direction_last = t;
        link_some_direction_bits = t;
        Link_HandleMovingAnimation_FullLongEntry();
      }
    }
  } else {
    want_stop_dash = (joypad1H_last & kJoypadH_AnyDir) && (joypad1H_last & kJoypadH_AnyDir) != kDashTab2[link_direction_facing >> 1];
  }

  if (want_stop_dash) {
    link_player_handler_state = kPlayerState_StopDash;
    button_mask_b_y &= ~0x80;
    button_b_frames = 0;
    link_delay_timer_spin_attack = 0;
    LinkState_ExitingDash();
    return;
  }

  if (link_speed_setting == 0 && (enhanced_features0 & kFeatures0_TurnWhileDashing))
    link_speed_setting = 16;

  uint8 dir = force_move_any_direction & 0xf;
  if (dir == 0)
    dir = kDashTab2[link_direction_facing >> 1];
  link_direction = link_direction_last = dir;
  Link_HandleDiagonalCollision();
  Link_HandleVelocity();
  Link_HandleCardinalCollision();
  Link_HandleMovingAnimation_FullLongEntry();
  fallhole_var1 = 0;
  HandleIndoorCameraAndDoors();
}

void LinkState_ExitingDash() {  // 87915e
  CacheCameraPropertiesIfOutdoors();
  if (joypad1H_last & kJoypadH_AnyDir || link_countdown_for_dash >= 16) {
    link_countdown_for_dash = 0;
    link_speed_setting = 0;
    link_player_handler_state = kPlayerState_Ground;
    link_is_running = 0;
    swimcoll_var5[0] &= 0xff00;
    if (button_b_frames < 9)
      link_cant_change_direction = 0;
  } else {
    link_countdown_for_dash++;
  }
  Link_HandleMovingAnimation_FullLongEntry();
}

void Link_CancelDash() {  // 879195
  if (link_is_running) {
    int i = 4;
    do {
      if (ancilla_type[i] == 0x1e)
        ancilla_type[i] = 0;
    } while (--i >= 0);
    link_countdown_for_dash = 0;
    link_speed_setting = 0;
    link_is_running = 0;
    link_cant_change_direction = 0;
    swimcoll_var5[0] = 0;
  }
}

void RepelDash() {  // 8791f1
  if (link_is_running && link_dash_ctr != 64) {
    Link_ResetSwimmingState();
    AncillaAdd_DashTremor(29, 1);
    Prepare_ApplyRumbleToSprites();
    if ((sound_effect_2 & 0x3f) != 27 && (sound_effect_2 & 0x3f) != 50)
      Ancilla_Sfx3_Near(3);
    LinkApplyTileRebound();
  }
}

void LinkApplyTileRebound() {  // 879222
  static const int8 kDashTab6Y[] = { 24, -24, 0, 0 };
  static const int8 kDashTab6X[] = { 0, 0, 24, -24 };
  static const int8 kDashTabSw11Y[] = { 1, 0, 0, 0 };
  static const int8 kDashTabSw11X[] = { 0, 0, 1, 0 };
  static const uint16 kDashTabSw7Y[] = { 384, 384, 0, 0, 256, 256, 0, 0 };
  static const uint16 kDashTabSw7X[] = { 0, 0, 384, 384, 0, 0, 256, 256 };
  static const uint8 kDashTabDir[] = { 8,4,2,1 };

  link_actual_vel_y = kDashTab6Y[link_last_direction_moved_towards];
  link_actual_vel_x = kDashTab6X[link_last_direction_moved_towards];
  link_incapacitated_timer = 24;
  link_actual_vel_z = link_actual_vel_z_copy = 36;
  if (link_flag_moving) {
    link_some_direction_bits = link_direction = kDashTabDir[link_last_direction_moved_towards];
    swimcoll_var11[0] = kDashTabSw11Y[link_last_direction_moved_towards];
    swimcoll_var11[1] = kDashTabSw11X[link_last_direction_moved_towards];

    int i = (link_flag_moving - 1) * 4 + link_last_direction_moved_towards;
    swimcoll_var7[0] = kDashTabSw7Y[i];
    swimcoll_var7[1] = kDashTabSw7X[i];
  }
  link_auxiliary_state = 1;
  link_want_make_noise_when_dashed = 1;
  BYTE(scratch_1) = 0;
  link_electrocute_on_touch = 0;
  link_speed_setting = 0;
  link_cant_change_direction = 0;
  link_moving_against_diag_tile = 0;
  if (link_last_direction_moved_towards & 2)
    link_y_vel = 0;
  else
    link_x_vel = 0;
}

void Sprite_RepelDash() {  // 879291
  link_last_direction_moved_towards = link_direction_facing >> 1;
  RepelDash();
}

void Flag67WithDirections() {  // 8792a0
  link_direction = 0;
  if (link_actual_vel_y)
    link_direction |= sign8(link_actual_vel_y) ? 8 : 4;
  if (link_actual_vel_x)
    link_direction |= sign8(link_actual_vel_x) ? 2 : 1;
}

void LinkState_Pits() {  // 8792d3
  link_direction = 0;
  if (fallhole_var1 && ++fallhole_var2 == 0x20) {
    fallhole_var2 = 31;
  } else {
    if (!link_is_running)
      goto aux_state;
    // If you use a turbo controller to perfectly spam the dash button,
    // the check for Link being in a hole is endlessly skipped and you
    // can levitate across chasms.
    // Fix this by ensuring that the dash button is held down before proceeding to the dash state.
    if (link_countdown_for_dash &&
        (!(enhanced_features0 & kFeatures0_MiscBugFixes) || (joypad1L_last & kJoypadL_A))) {
      LinkState_Dashing();
      return;
    }
    if (joypad1H_last & kJoypadH_AnyDir && !(joypad1H_last & kJoypadH_AnyDir & link_direction)) {
      Link_CancelDash();
aux_state:
      if (link_auxiliary_state != 1)
        link_direction = joypad1H_last & kJoypadH_AnyDir;
    }
  }
  TileDetect_MainHandler(4);
  if (!(tiledetect_pit_tile & 1)) {
    // Reset player_near_pit_state if we're no longer near a hole.
    // This fixes a bug where you could walk on water
    if (enhanced_features0 & kFeatures0_MiscBugFixes)
      player_near_pit_state = 0;

    if (link_is_running) {
      LinkState_Dashing();
      return;
    }
    link_speed_setting = 0;
    Link_CancelDash();
    if (!(button_mask_b_y & 0x80))
      link_cant_change_direction &= ~1;
    player_near_pit_state = 0;
    link_player_handler_state = !link_is_bunny_mirror ? kPlayerState_Ground :
      link_item_moon_pearl ? kPlayerState_TempBunny : kPlayerState_PermaBunny;
    if (link_player_handler_state == kPlayerState_PermaBunny)
      PlayerHandler_17_Bunny();
    else if (link_player_handler_state == kPlayerState_TempBunny)
      LinkState_TemporaryBunny();
    else
      LinkState_Default();
    return;
  }

  Player_TileDetectNearby();
  link_speed_setting = 4;
  if (!(tiledetect_pit_tile & 0xf)) {
    player_near_pit_state = 0;
    link_speed_setting = 0;
    link_player_handler_state = !link_is_bunny_mirror ? kPlayerState_Ground :
      link_item_moon_pearl ? kPlayerState_TempBunny : kPlayerState_PermaBunny;
    Link_CancelDash();
    if (!(button_mask_b_y & 0x80))
      link_cant_change_direction &= ~1;
    return;
  }

  static const uint8 kFallHolePitDirs[] = { 12, 3, 10, 5 };
  static const uint8 kFallHoleDirs[] = { 5, 6, 9, 10, 4, 8, 1, 2 };
  static const uint8 kFallHoleDirs2[] = { 10, 9, 6, 5, 8, 4, 2, 1 };

  if ((tiledetect_pit_tile & 0xf) != 0xf) {
    int i = 3;
    do {
      if ((tiledetect_pit_tile & 0xf) == kFallHolePitDirs[i]) {
        i += 4;
        goto endif_1;
      }
    } while (--i >= 0);

    i = 3;
    uint8 pit_tile;
    pit_tile= tiledetect_pit_tile;
    while (!(pit_tile & 1)) {
      i -= 1;
      pit_tile >>= 1;
    }
    assert(i >= 0);
endif_1:
    byte_7E02C9 = i;
    if (link_direction & kFallHoleDirs[i]) {
      link_direction_last = link_direction;
      link_speed_setting = 6;
      Link_HandleMovingAnimation_FullLongEntry();
    } else {
      uint8 old_dir = link_direction;
      link_direction |= kFallHoleDirs2[byte_7E02C9];
      if (old_dir)
        Link_HandleMovingAnimation_FullLongEntry();
    }
    Link_HandleDiagonalCollision();
    Link_HandleVelocity();
    Link_HandleCardinalCollision();
    ApplyLinksMovementToCamera();
    return;
  }

  // Initiate fall down
  if (player_near_pit_state != 2) {
    if (link_item_moon_pearl) {
      link_need_for_poof_for_transform = 0;
      link_is_bunny = 0;
      link_is_bunny_mirror = 0;
      link_timer_tempbunny = 0;
    }
    link_direction = 0;
    player_near_pit_state = 2;
    link_disable_sprite_damage = 1;
    button_mask_b_y = 0;
    button_b_frames = 0;
    link_item_in_hand = 0;
    link_position_mode = 0;
    link_incapacitated_timer = 0;
    link_auxiliary_state = 0;
    Ancilla_Sfx3_Near(31);
  }

  link_cant_change_direction = 0;
  link_incapacitated_timer = 0;
  link_z_coord = 0;
  link_actual_vel_z = 0;
  link_auxiliary_state = 0;
  link_give_damage = 0;
  link_is_transforming = 0;
  Link_ForceUnequipCape_quietly();
  link_disable_sprite_damage++;
  if (!sign8(--byte_7E005C))
    return;
  uint8 x = ++link_this_controls_sprite_oam;
  byte_7E005C = 9;
  if (follower_indicator != 13 && x == 1)
    tagalong_var5 = x;

  if (x == 6) {
    Link_CancelDash();
    submodule_index = 7;
    link_this_controls_sprite_oam = 6;
    player_near_pit_state = 3;
    link_visibility_status = 12;
    link_speed_modifier = 16;
    uint16 y = (uint8)(link_y_coord - BG2VOFS_copy2);
    link_state_bits = 0;
    link_picking_throw_state = 0;
    link_grabbing_wall = 0;
    some_animation_timer = 0;
    if (player_is_indoors) {
      BYTE(dungeon_room_index_prev) = dungeon_room_index;
      Dungeon_FlagRoomData_Quadrants();
      if (Dungeon_IsPitThatHurtsPlayer()) {
        DungeonPitDoDamage();
        return;
      }
    }
    BYTE(dungeon_room_index_prev) = dungeon_room_index;
    BYTE(dungeon_room_index) = dung_hdr_travel_destinations[0];
    tiledetect_which_y_pos[0] = link_y_coord;
    link_y_coord = link_y_coord - y - 0x10;
    if (player_is_indoors) {
      HandleLayerOfDestination();
    } else {
      if ((uint8)overworld_screen_index != 5) {
        Overworld_GetPitDestination();
        main_module_index = 17;
        submodule_index = 0;
        subsubmodule_index = 0;
      } else {
        TakeDamageFromPit();
      }
    }
  }
}

void HandleLayerOfDestination() {  // 8794f1
  link_is_on_lower_level_mirror = (dung_hdr_hole_teleporter_plane >= 1);
  link_is_on_lower_level = (dung_hdr_hole_teleporter_plane >= 2);
}

void DungeonPitDoDamage() {  // 879502
  submodule_index = 20;
  link_health_current -= 8;
  if (link_health_current >= 0xa8)
    link_health_current = 0;
}

void HandleDungeonLandingFromPit() {  // 879520
  LinkOam_Main();
  link_x_coord_prev = link_x_coord;
  link_y_coord_prev = link_y_coord;
  if (submodule_index == 7)
    link_visibility_status = 0;
  if (!(frame_counter & 3) && ++link_this_controls_sprite_oam == 10)
    link_this_controls_sprite_oam = 6;
  link_direction = 4;
  Link_HandleVelocity();
  if (sign16(link_y_coord) && !sign16(tiledetect_which_y_pos[0])) {
    if (!sign16(-link_y_coord + tiledetect_which_y_pos[0]))
      return;
  } else {
    if (tiledetect_which_y_pos[0] >= link_y_coord)
      return;
  }
  //exploration glitch could also be armed without quitting
  //by jumping off a dungeon ledge into an access pit
  if (enhanced_features0 & kFeatures0_MiscBugFixes) {
    about_to_jump_off_ledge = 0;
  }
  link_y_coord = tiledetect_which_y_pos[0];
  link_animation_steps = 0;
  link_speed_modifier = 0;
  link_this_controls_sprite_oam = 0;
  player_near_pit_state = 0;
  link_speed_setting = 0;
  subsubmodule_index = 0;
  submodule_index = 0;
  link_disable_sprite_damage = 0;
  if (follower_indicator != 0 && follower_indicator != 3) {
    tagalong_var5 = 0;
    if (follower_indicator == 13) {
      follower_indicator = 0;
      super_bomb_indicator_unk2 = 0;
      super_bomb_indicator_unk1 = 0;
      follower_dropped = 0;
    } else {
      Follower_Initialize();
    }
  }
  TileDetect_MainHandler(0);
  if (tiledetect_shallow_water & 1)
    Ancilla_Sfx2_Near(0x24);
  Player_TileDetectNearby();
  if ((sound_effect_1 & 0x3f) != 0x24)
    Ancilla_Sfx2_Near(0x21);

  if (dung_hdr_collision_2 == 2 && (tiledetect_water_staircase & 0xf))
    byte_7E0322 = 3;
  if ((tiledetect_deepwater & 0xf) == 0xf) {
    link_is_in_deep_water = 1;
    link_some_direction_bits = link_direction_last;
    Link_ResetSwimmingState();
    link_is_on_lower_level = 1;
    AncillaAdd_Splash(0x15, 1);
    link_player_handler_state = kPlayerState_Swimming;
    Link_ForceUnequipCape_quietly();
    link_state_bits = 0;
    link_picking_throw_state = 0;
    link_grabbing_wall = 0;
    link_speed_setting = 0;
  } else {
    link_player_handler_state = (tiledetect_pit_tile & 0xf) ? kPlayerState_FallingIntoHole : kPlayerState_Ground;
  }
}

void PlayerHandler_04_Swimming() {  // 87963b
  if (link_auxiliary_state) {
    link_player_handler_state = kPlayerState_RecoilWall;
    link_z_coord &= 0xff;
    ResetAllAcceleration();
    link_maybe_swim_faster = 0;
    link_swim_hard_stroke = 0;
    link_cant_change_direction &= ~1;
    LinkState_Recoil();
    return;
  }

  button_mask_b_y = 0;
  button_b_frames = 0;
  link_delay_timer_spin_attack = 0;
  link_spin_attack_step_counter = 0;
  link_state_bits = 0;
  link_picking_throw_state = 0;
  if (!link_item_flippers)
    return;

  if (!(swimcoll_var7[0] | swimcoll_var7[1])) {
    if ((uint8)swimcoll_var5[0] != 2 && (uint8)swimcoll_var5[1] != 2)
      ResetAllAcceleration();
    link_animation_steps &= 1;
    if (++link_counter_var1 >= 16) {
      link_counter_var1 = 0;
      byte_7E02CC = 0;
      link_animation_steps = (link_animation_steps & 1) ^ 1;
    }
  } else {
    if (++link_counter_var1 >= 8) {
      link_counter_var1 = 0;
      link_animation_steps = (link_animation_steps + 1) & 3;
      byte_7E02CC = kSwimmingTab1[link_animation_steps];
    }
  }

  if (!link_swim_hard_stroke) {
    uint8 t;
    if (!(swimcoll_var7[0] | swimcoll_var7[1]) || (t = ((filtered_joypad_L & kJoypadL_A) | filtered_joypad_H) & 0xc0) == 0) {
      Link_HandleSwimMovements();
      return;
    }
    link_swim_hard_stroke = t;
    Ancilla_Sfx2_Near(37);
    link_maybe_swim_faster = 1;
    swimming_countdown = 7;
    Link_HandleSwimAccels();
  }
  if (sign8(--swimming_countdown)) {
    swimming_countdown = 7;
    if (++link_maybe_swim_faster == 5) {
      link_maybe_swim_faster = 0;
      link_swim_hard_stroke &= ~0xC0;
    }
  }

  Link_HandleSwimMovements();
}

void Link_HandleSwimMovements() {  // 879715
  uint8 t;

  if (!(t = force_move_any_direction & 0xf) && !(t = joypad1H_last & kJoypadH_AnyDir)) {
    link_y_vel = link_x_vel = 0;
    Link_FlagMaxAccels();
    if (link_flag_moving) {
      if (link_is_running) {
        t = link_some_direction_bits;
      } else {
        if (!(swimcoll_var7[0] | swimcoll_var7[1])) {
          bitmask_of_dragstate = 0;
          Link_ResetSwimmingState();
        }
        goto out;
      }
    } else {
      if (link_player_handler_state != kPlayerState_Swimming)
        link_animation_steps = 0;
      goto out;
    }
  }

  if (t != link_some_direction_bits) {
    link_some_direction_bits = t;
    link_subpixel_x = link_subpixel_y = 0;
    link_moving_against_diag_tile = 0;
    bitmask_of_dragstate = 0;
  }
  Link_SetIceMaxAccel();
  Link_SetMomentum();
  Link_SetTheMaxAccel();
out:
  Link_HandleDiagonalCollision();
  Link_HandleVelocity();
  Link_HandleCardinalCollision();
  Link_HandleMovingAnimation_FullLongEntry();
  fallhole_var1 = 0;
  HandleIndoorCameraAndDoors();
}

void Link_FlagMaxAccels() {  // 879785
  if (!link_flag_moving)
    return;
  for (int i = 1; i >= 0; i--) {
    if (swimcoll_var7[i]) {
      swimcoll_var9[i] = swimcoll_var7[i];
      swimcoll_var5[i] = 1;
    }
  }
}

void Link_SetIceMaxAccel() {  // 8797a6
  if (!link_flag_moving)
    return;
  swimcoll_var9[0] = 0x180;
  swimcoll_var9[1] = 0x180;
}

void Link_SetMomentum() {  // 8797c7
  uint8 joy = joypad1H_last & kJoypadH_AnyDir;
  uint8 mask = 12, bit = 8;
  for (int i = 0; i < 2; i++, mask >>= 2, bit >>= 2) {
    if (joy & mask) {
      swimcoll_var3[i] = link_flag_moving ? kSwimmingTab2[link_flag_moving - 1] : 32;
      if (((link_some_direction_bits | link_direction) & mask) == mask) {
        swimcoll_var5[i] = 2;
      } else {
        swimcoll_var11[i] = (joy & bit) ? 0 : 1;
        swimcoll_var5[i] = 0;
      }
      if (!swimcoll_var9[i])
        swimcoll_var9[i] = 240;
    }
  }
}

void Link_ResetSwimmingState() {  // 87983a
  swimming_countdown = 0;
  link_swim_hard_stroke = 0;
  link_maybe_swim_faster = 0;
  ResetAllAcceleration();
}

void Link_ResetStateAfterDamagingPit() {  // 87984b
  Link_ResetSwimmingState();
  link_player_handler_state = link_is_bunny && !link_item_moon_pearl ?
    kPlayerState_PermaBunny : kPlayerState_Ground;
  link_direction_last = link_some_direction_bits;
  link_is_in_deep_water = 0;
  link_disable_sprite_damage = 0;
  link_this_controls_sprite_oam = 0;
  player_near_pit_state = 0;
}

void ResetAllAcceleration() {  // 879873
  swimcoll_var1[0] = 0;
  swimcoll_var1[1] = 0;
  swimcoll_var3[0] = 0;
  swimcoll_var3[1] = 0;
  swimcoll_var5[0] = 0;
  swimcoll_var5[1] = 0;
  swimcoll_var7[0] = 0;
  swimcoll_var7[1] = 0;
  swimcoll_var9[0] = 0;
  swimcoll_var9[1] = 0;
}

void Link_HandleSwimAccels() {  // 8798a8
  static const  uint16 kSwimmingTab3[] = { 128, 160, 192, 224, 256, 288, 320, 352, 384 };
  uint8 mask = 12;
  for (int i = 0; i < 2; i++, mask >>= 2) {
    if (joypad1H_last & mask) {
      if (swimcoll_var7[i] && swimcoll_var9[i] >= 384) {
        uint16 t;
        for (int j = 0; j < 9 && (t = kSwimmingTab3[j]) < swimcoll_var7[i]; j++) {}
        swimcoll_var9[i] = t;
      } else {
        uint16 t = swimcoll_var9[i];
        if (t) {
          t += 160;
          if (t >= 384)
            t = 384;
          swimcoll_var9[i] = t;
        } else {
          swimcoll_var7[i] = 1;
          swimcoll_var9[i] = 240;
        }
      }
    }
  }
}

void Link_SetTheMaxAccel() {  // 879903
  if (link_flag_moving || link_swim_hard_stroke)
    return;
  uint8 mask = 12;
  for (int i = 0; i < 2; i++, mask >>= 2) {
    if ((joypad1H_last & mask) && swimcoll_var5[i] != 2) {
      if (swimcoll_var1[i] || swimcoll_var7[i] >= 240 && swimcoll_var7[i] >= swimcoll_var9[i]) {
        swimcoll_var5[i] = 0;
        if (swimcoll_var7[i] >= 240) {
          swimcoll_var1[i] = 1;
          swimcoll_var5[i] = 1;
        } else {
          swimcoll_var9[i] = 240;
          swimcoll_var1[i] = 0;
        }
      }
    } else {
      swimcoll_var9[i] = 240;
      swimcoll_var1[i] = 0;
    }
  }
}

void LinkState_Zapped() {  // 87996c
  CacheCameraPropertiesIfOutdoors();
  LinkZap_HandleMosaic();
  if (!sign8(--link_delay_timer_spin_attack))
    return;
  link_delay_timer_spin_attack = 2;
  player_handler_timer++;
  if (player_handler_timer & 1)
    Palette_ElectroThemedGear();
  else
    LoadActualGearPalettes();
  if (player_handler_timer == 8) {
    player_handler_timer = 0;
    link_player_handler_state = kPlayerState_Ground;
    link_disable_sprite_damage = 0;
    link_electrocute_on_touch = 0;
    link_auxiliary_state = 0;
    Player_SetCustomMosaicLevel(0);
  }
}

void PlayerHandler_15_HoldItem() {  // 8799ac
  // empty by design
}

void Link_ReceiveItem(uint8 item, int chest_position) {  // 8799ad
  if (link_auxiliary_state) {
    link_auxiliary_state = 0;
    link_incapacitated_timer = 0;
    countdown_for_blink = 0;
    link_state_bits = 0;
  }
  link_receiveitem_index = item;
  if (item == 0x3e)
    Ancilla_Sfx3_Near(0x2e);
  link_receiveitem_var1 = 0x60;
  if (item_receipt_method == 0 || item_receipt_method == 3) {
    link_state_bits = 0;
    button_mask_b_y = 0;
    bitfield_for_a_button = 0;
    button_b_frames = 0;
    link_speed_setting = 0;
    link_cant_change_direction = 0;
    link_item_in_hand = 0;
    link_position_mode = 0;
    player_handler_timer = 0;
    link_player_handler_state = kPlayerState_HoldUpItem;
    link_pose_for_item = 1;
    link_disable_sprite_damage = 1;
    if (item == 0x20)
      link_pose_for_item = 2;
  }
  AncillaAdd_ItemReceipt(0x22, 4, chest_position);
  if (item != 0x20 && item != 0x37 && item != 0x38 && item != 0x39)
    Hud_RefreshIcon();
  Link_CancelDash();
}

void Link_TuckIntoBed() {  // 879a2c
  link_y_coord = 0x215a;
  link_x_coord = 0x940;
  link_player_handler_state = kPlayerState_AsleepInBed;
  player_sleep_in_bed_state = 0;
  link_pose_during_opening = 0;
  link_countdown_for_dash = 3;
  AncillaAdd_Blanket(0x20);
}

void LinkState_Sleeping() {  // 879a5a
  switch (player_sleep_in_bed_state) {
  case 0:
    if (!(frame_counter & 0x1f))
      AncillaAdd_Snoring(0x21, 1);
    break;
  case 1:
    if (submodule_index == 0 && sign8(--link_countdown_for_dash)) {
      link_countdown_for_dash = 0;
      if (((filtered_joypad_H & 0xe0) | (filtered_joypad_H << 4) | filtered_joypad_L) & 0xf0) {
        link_pose_during_opening++;
        link_direction_facing = 6;
        player_sleep_in_bed_state++;
        link_countdown_for_dash = 4;
      }
    }
    break;
  case 2:
    if (sign8(--link_countdown_for_dash)) {
      link_actual_vel_y = 4;
      link_actual_vel_x = 21;
      link_actual_vel_z = 24;
      link_actual_vel_z_copy = 24;
      link_incapacitated_timer = 16;
      link_auxiliary_state = 2;
      link_player_handler_state = kPlayerState_RecoilOther;
    }
    break;
  }
}

void Link_HandleSwordCooldown() {  // 879ac2
  if (!sign8(--link_sword_delay_timer))
    return;

  link_sword_delay_timer = 0;
  if (link_item_in_hand | link_position_mode)
    return;

  if (button_b_frames < 9) {
    if (!link_is_running)
      Link_CheckForSwordSwing();
  } else {
    HandleSwordControls();
  }

}

void Link_HandleYItem() {  // 879b0e
  if (button_b_frames && button_b_frames < 9)
    return;

  uint8 item = current_item_y;

  if (link_is_bunny_mirror && (item != 11 && item != 20))
    return;

  if (is_archer_or_shovel_game && !link_is_bunny_mirror) {
    if (is_archer_or_shovel_game == 2)
      LinkItem_Bow();
    else
      LinkItem_Shovel();
    return;
  }

  uint8 old_down = joypad1H_last, old_pressed = filtered_joypad_H, old_bottle = link_item_bottle_index;
  if ((link_item_in_hand | link_position_mode) == 0 && !(old_down & kJoypadH_Y)) {
    // Is any special key held down?
    int btn_index = GetCurrentItemButtonIndex();
    if (btn_index != 0) {
      uint8 *cur_item_ptr = GetCurrentItemButtonPtr(btn_index);
      if (*cur_item_ptr) {
        if (*cur_item_ptr >= kHudItem_Bottle1)
          link_item_bottle_index = *cur_item_ptr - kHudItem_Bottle1 + 1;
        item = Hud_LookupInventoryItem(*cur_item_ptr);
        // Pretend it's actually Y that's down
        joypad1H_last = old_down | kJoypadH_Y;
        static const uint8 kButtonIndexKeys[4] = { 0, kJoypadL_X, kJoypadL_L, kJoypadL_R };
        filtered_joypad_H = old_pressed | ((filtered_joypad_L & kButtonIndexKeys[btn_index]) ? kJoypadH_Y : 0);
      }
    }
  }

  if (item != current_item_active) {
    if (current_item_active == 8 && (link_item_flute & 2))
      button_mask_b_y &= ~0x40;
    if (current_item_active == 19 && link_cape_mode)
      Link_ForceUnequipCape();
  }


  if ((link_item_in_hand | link_position_mode) == 0)
    current_item_active = item;

  if (current_item_active == 5 || current_item_active == 6)
    eq_selected_rod = current_item_active - 5 + 1;

  switch (current_item_active) {
  case 0:
    break;
  case 1: LinkItem_Bombs(); break;
  case 2: LinkItem_Boomerang(); break;
  case 3: LinkItem_Bow(); break;
  case 4: LinkItem_Hammer(); break;
  case 5: LinkItem_Rod(); break;
  case 6: LinkItem_Rod(); break;
  case 7: LinkItem_Net(); break;
  case 8: LinkItem_ShovelAndFlute(); break;
  case 9: LinkItem_Lamp(); break;
  case 10: LinkItem_Powder(); break;
  case 11: LinkItem_Bottle(); break;
  case 12: LinkItem_Book(); break;
  case 13: LinkItem_CaneOfByrna(); break;
  case 14: LinkItem_Hookshot(); break;
  case 15: LinkItem_Bombos(); break;
  case 16: LinkItem_Ether(); break;
  case 17: LinkItem_Quake(); break;
  case 18: LinkItem_CaneOfSomaria(); break;
  case 19: LinkItem_Cape(); break;
  case 20: LinkItem_Mirror(); break;
  case 21: LinkItem_Shovel(); break;
  default:
    assert(0);
  }

  joypad1H_last = old_down;
  filtered_joypad_H = old_pressed;
  link_item_bottle_index = old_bottle;
}

void Link_HandleAPress() {  // 879baa
  flag_is_sprite_to_pick_up_cached = 0;
  if (link_item_in_hand || (link_position_mode & 0x1f) || byte_7E0379)
    return;

  if (button_b_frames < 9 && (button_mask_b_y & 0x80))
    return;

  uint8 action = tile_action_index;

  if ((link_state_bits | link_grabbing_wall) == 0) {
    if (!Link_CheckNewAPress()) {
      bitfield_for_a_button = 0;
      return;
    }

    if (link_need_for_pullforrupees_sprite && !link_direction_facing) {
      action = 7;
    } else if (link_is_near_moveable_statue) {
      action = 6;
    } else {
      if (!flag_is_ancilla_to_pick_up) {
        if (!flag_is_sprite_to_pick_up) {
          action = Link_HandleLiftables();
          goto attempt_action;
        }
        flag_is_sprite_to_pick_up_cached = flag_is_sprite_to_pick_up;
      }

      if (button_b_frames)
        Link_ResetSwordAndItemUsage();

      if (link_item_in_hand | link_position_mode) {
        link_item_in_hand = 0;
        link_position_mode = 0;
        Link_ResetBoomerangYStuff();
        flag_for_boomerang_in_place = 0;
        if (ancilla_type[0] == 5)
          ancilla_type[0] = 0;
      }
      action = 1;
    }
    static const uint8 kAbilityBitmasks[] = { 0xE0, 0x40, 4, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0 };
attempt_action:
    if (!(kAbilityBitmasks[action] & link_ability_flags)) {
      bitfield_for_a_button = 0;
      return;
    }

    tile_action_index = action;
    Link_APress_PerformBasic(action * 2);
  }

  // actionInProgress
  unused_2 = tile_action_index;
  switch (tile_action_index) {
  case 1: Link_APress_LiftCarryThrow(); break;
  case 3: Link_APress_PullObject(); break;
  case 6: Link_APress_StatueDrag(); break;
  }
}

void Link_APress_PerformBasic(uint8 action_x2) {  // 879c5f
  switch (action_x2 >> 1) {
  case 0: Link_PerformDesertPrayer(); break;
  case 1: Link_PerformThrow(); break;
  case 2: Link_PerformDash(); break;
  case 3: Link_PerformGrab(); break;
  case 4: Link_PerformRead(); break;
  case 5: Link_PerformOpenChest(); break;
  case 6: Link_PerformStatueDrag(); break;
  case 7: Link_PerformRupeePull(); break;
  default:
    assert(0);
  }
}

void HandleSwordSfxAndBeam() {  // 879c66
  link_direction &= ~0xf;
  button_b_frames = 0;
  link_spin_attack_step_counter = 0;

  uint8 health = link_health_capacity - 4;
  if (health < link_health_current && ((link_sword_type + 1) & 0xfe) && link_sword_type >= 2) {
    int i = 4;
    while (ancilla_type[i] != 0x31) {
      if (--i < 0) {
        AddSwordBeam(0);
        break;
      }
    }
  }
  uint8 sword = link_sword_type - 1;
  if (sword != 0xfe && sword != 0xff)
    sound_effect_1 = kFireBeamSounds[sword] | Link_CalculateSfxPan();
  link_delay_timer_spin_attack = 1;
}

void Link_CheckForSwordSwing() {  // 879cd9
  if (bitfield_for_a_button & 0x10)
    return;

  if (!(button_mask_b_y & 0x80)) {
    if (!(filtered_joypad_H & 0x80))
      return;
    if (is_standing_in_doorway) {
      TileDetect_SwordSwingDeepInDoor(is_standing_in_doorway);
      if ((R14 & 0x30) == 0x30)
        return;
    }
    button_mask_b_y |= 0x80;
    HandleSwordSfxAndBeam();
    link_cant_change_direction |= 1;
    link_animation_steps = 0;
  }

  if (!(joypad1H_last & kJoypadH_B))
    button_mask_b_y |= 1;
  HaltLinkWhenUsingItems();
  link_direction &= ~0xf;
  if (sign8(--link_delay_timer_spin_attack)) {
    if (++button_b_frames >= 9) {
      HandleSwordControls();
      return;
    }
    link_delay_timer_spin_attack = kSpinAttackDelays[button_b_frames];
    if (button_b_frames == 5) {
      if (link_sword_type != 0 && link_sword_type != 1 && link_sword_type != 0xff)
        AncillaAdd_SwordSwingSparkle(0x26, 4);
      if (link_sword_type != 0 && link_sword_type != 0xff)
        TileDetect_MainHandler(link_sword_type == 1 ? 1 : 6);
    } else if (button_b_frames >= 4 && (button_mask_b_y & 1) && (joypad1H_last & kJoypadH_B)) {
      button_mask_b_y &= ~1;
      HandleSwordSfxAndBeam();
      return;
    }
  }
  CalculateSwordHitBox();
}

void HandleSwordControls() {  // 879d72
  if (joypad1H_last & kJoypadH_B) {
    Player_Sword_SpinAttackJerks_HoldDown();
  } else {
    if (link_spin_attack_step_counter < 48) {
      Link_ResetSwordAndItemUsage();
    } else {
      Link_ResetSwordAndItemUsage();
      link_spin_attack_step_counter = 0;
      Link_ActivateSpinAttack();
    }
  }
}

void Link_ResetSwordAndItemUsage() {  // 879d84
  link_speed_setting = 0;
  bitmask_of_dragstate &= ~9;
  link_delay_timer_spin_attack = 0;
  button_b_frames = 0;
  button_mask_b_y &= ~0x81;
  link_cant_change_direction &= ~1;
}

void Player_Sword_SpinAttackJerks_HoldDown() {  // 879d9f
  if ((bitmask_of_dragstate & 0x80) || (bitmask_of_dragstate & 9) == 0) {
    if (set_when_damaging_enemies == 0) {
      button_b_frames = 9;
      link_cant_change_direction |= 1;
      link_delay_timer_spin_attack = 0;
      if (link_speed_setting != 4 && link_speed_setting != 16) {
        link_speed_setting = 12;
        if (!((uint8)(link_sword_type + 1) & ~1))
          return;
        int i = 4;
        do {
          if (ancilla_type[i] == 0x30 || ancilla_type[i] == 0x31)
            return;
        } while (--i >= 0);

        if (link_spin_attack_step_counter >= 6 && (frame_counter & 3) == 0)
          AncillaSpawn_SwordChargeSparkle();

        if (link_spin_attack_step_counter < 64 && ++link_spin_attack_step_counter == 48) {
          Ancilla_Sfx2_Near(55);
          AncillaAdd_ChargedSpinAttackSparkle();
        }
      } else {
        CalculateSwordHitBox();
      }
      return;
    } else if (set_when_damaging_enemies == 1) {
      Link_ResetSwordAndItemUsage();
      return;
    }
  }
  // endif_2
  if (button_b_frames == 9) {
    button_b_frames = 10;
    link_delay_timer_spin_attack = kSpinAttackDelays[button_b_frames];
  }

  if (sign8(--link_delay_timer_spin_attack)) {
    uint8 frames = button_b_frames + 1;
    if (frames == 13) {
      if ((uint8)(link_sword_type + 1) & ~1 && (bitmask_of_dragstate & 9)) {
        AncillaAdd_WallTapSpark(27, 1);
        Ancilla_Sfx2_Near((bitmask_of_dragstate & 8) ? 6 : 5);
        TileDetect_MainHandler(1);
      }
      frames = 10;
    }
    button_b_frames = frames;
    link_delay_timer_spin_attack = kSpinAttackDelays[button_b_frames];
  }
  CalculateSwordHitBox();
}

void LinkItem_Rod() {  // 879eef
  static const uint8 kRodAnimDelays[] = { 3, 3, 5 };
  if (!(button_mask_b_y & 0x40)) {
    if (is_standing_in_doorway || !CheckYButtonPress())
      return;
    if (!LinkCheckMagicCost(0))
      goto out;
    link_debug_value_2 = 1;
    if (eq_selected_rod == 1)
      AncillaAdd_FireRodShot(2, 1);
    else
      AncillaAdd_IceRodShot(11, 1);
    link_delay_timer_spin_attack = kRodAnimDelays[0];
    link_animation_steps = 0;
    player_handler_timer = 0;
    link_item_in_hand = 1;
  }
  HaltLinkWhenUsingItems();
  link_direction &= ~0xf;
  if (!sign8(--link_delay_timer_spin_attack))
    return;
  player_handler_timer++;

  link_delay_timer_spin_attack = kRodAnimDelays[player_handler_timer];
  if (player_handler_timer != 3)
    return;
  link_debug_value_2 = 0;
  link_speed_setting = 0;
  player_handler_timer = 0;
  link_delay_timer_spin_attack = 0;
  link_item_in_hand &= ~1;
out:
  button_mask_b_y &= ~0x40;
}

void LinkItem_Hammer() {  // 879f7b
  static const uint8 kHammerAnimDelays[] = { 3, 3, 16 };
  if (link_item_in_hand & 0x10)
    return;
  if (!(button_mask_b_y & 0x40)) {
    if (is_standing_in_doorway || !(filtered_joypad_H & kJoypadH_Y))
      return;
    button_mask_b_y |= 0x40;
    link_delay_timer_spin_attack = kHammerAnimDelays[0];
    link_cant_change_direction |= 1;
    link_animation_steps = 0;
    player_handler_timer = 0;
    link_item_in_hand = 2;
  }

  HaltLinkWhenUsingItems();
  link_direction &= ~0xf;
  if (!sign8(--link_delay_timer_spin_attack))
    return;
  player_handler_timer++;

  link_delay_timer_spin_attack = kHammerAnimDelays[player_handler_timer];
  if (player_handler_timer == 1) {
    TileDetect_MainHandler(3);
    Ancilla_AddHitStars(22, 0);
    if (sound_effect_1 == 0) {
      Ancilla_Sfx2_Near(16);
      SpawnHammerWaterSplash();
    }
  } else if (player_handler_timer == 3) {
    player_handler_timer = 0;
    link_delay_timer_spin_attack = 0;
    button_mask_b_y &= ~0x40;
    link_cant_change_direction &= ~1;
    link_item_in_hand &= ~2;
  }
}

void LinkItem_Bow() {  // 87a006
  static const uint8 kBowDelays[] = { 3, 3, 8 };

  if (!(button_mask_b_y & 0x40)) {
    if (is_standing_in_doorway || !CheckYButtonPress())
      return;
    link_cant_change_direction |= 1;
    link_delay_timer_spin_attack = kBowDelays[0];
    link_animation_steps = 0;
    player_handler_timer = 0;
    link_item_in_hand = 16;
  }
  HaltLinkWhenUsingItems();
  link_direction &= ~0xf;
  if (!sign8(--link_delay_timer_spin_attack))
    return;
  player_handler_timer++;

  link_delay_timer_spin_attack = kBowDelays[player_handler_timer];
  if (player_handler_timer != 3)
    return;

  int obj = AncillaAdd_Arrow(9, link_direction_facing, 2, link_x_coord, link_y_coord);
  if (obj >= 0) {
    if (archery_game_arrows_left) {
      archery_game_arrows_left--;
      link_num_arrows += 2;
    }
    if (!archery_game_out_of_arrows && link_num_arrows) {
      if (--link_num_arrows == 0)
        Hud_RefreshIcon();
    } else {
      ancilla_type[obj] = 0;
      Ancilla_Sfx2_Near(60);
    }
  }

  player_handler_timer = 0;
  link_delay_timer_spin_attack = 0;
  button_mask_b_y &= ~0x40;
  link_cant_change_direction &= ~1;
  link_item_in_hand &= ~0x10;
  if (button_b_frames >= 9)
    button_b_frames = 9;
}

void LinkItem_Boomerang() {  // 87a0bb
  if (!(button_mask_b_y & 0x40)) {
    if (is_standing_in_doorway || !CheckYButtonPress() || flag_for_boomerang_in_place)
      return;
    link_animation_steps = 0;
    link_item_in_hand = 0x80;
    player_handler_timer = 0;
    link_delay_timer_spin_attack = 7;

    int s0 = AncillaAdd_Boomerang(5, 0);

    if (button_b_frames >= 9) {
      Link_ResetBoomerangYStuff();
      return;
    }

    if (!s0) {
      link_direction_last = joypad1H_last & kJoypadH_AnyDir;
    } else {
      link_cant_change_direction |= 1;
    }
  } else {
    link_cant_change_direction |= 1;
  }

  if (link_item_in_hand) {
    HaltLinkWhenUsingItems();
    link_direction &= ~0xf;
    if (!sign8(--link_delay_timer_spin_attack))
      return;
    link_delay_timer_spin_attack = 5;
    if (++player_handler_timer != 2)
      return;
  }
  Link_ResetBoomerangYStuff();
}

void Link_ResetBoomerangYStuff() {  // 87a11f
  link_item_in_hand = 0;
  player_handler_timer = 0;
  link_delay_timer_spin_attack = 0;
  button_mask_b_y &= ~0x40;
  if (!(button_mask_b_y & 0x80))
    link_cant_change_direction &= ~1;
}

void LinkItem_Bombs() {  // 87a138
  if (is_standing_in_doorway || follower_indicator == 13 || !CheckYButtonPress())
    return;
  button_mask_b_y &= ~0x40;
  AncillaAdd_Bomb(7, enhanced_features0 & kFeatures0_MoreActiveBombs ? 3 : 1);
  link_item_in_hand = 0;
}

void LinkItem_Bottle() {  // 87a15b
  if (!CheckYButtonPress())
    return;
  button_mask_b_y &= ~0x40;
  int btidx = link_item_bottle_index - 1;
  uint8 b = link_bottle_info[btidx];
  if (b == 0)
    return;
  if (b < 3) {
fail:
    Ancilla_Sfx2_Near(60);
  } else if (b == 3) {  // red potion
    if (link_health_capacity == link_health_current)
      goto fail;
    link_bottle_info[btidx] = 2;
    link_item_in_hand = 0;
    submodule_index = 4;
    saved_module_for_menu = main_module_index;
    main_module_index = 14;
    animate_heart_refill_countdown = 7;
    Hud_Rebuild();
  } else if (b == 4) { // green potion
    if (link_magic_power == 128)
      goto fail;
    link_bottle_info[btidx] = 2;
    link_item_in_hand = 0;
    submodule_index = 8;
    saved_module_for_menu = main_module_index;
    main_module_index = 14;
    animate_heart_refill_countdown = 7;
    Hud_Rebuild();
  } else if (b == 5) { // blue potion
    if (link_health_capacity == link_health_current && link_magic_power == 128)
      goto fail;
    link_bottle_info[btidx] = 2;
    link_item_in_hand = 0;
    submodule_index = 9;
    saved_module_for_menu = main_module_index;
    main_module_index = 14;
    animate_heart_refill_countdown = 7;
    Hud_Rebuild();
  } else if (b == 6) { // fairy
    link_item_in_hand = 0;
    if (ReleaseFairy() < 0)
      goto fail;
    link_bottle_info[btidx] = 2;
    Hud_Rebuild();
  } else if (b == 7 || b == 8) {  // bad/good bee
    if (!ReleaseBeeFromBottle(btidx))
      goto fail;
    link_bottle_info[btidx] = 2;
    Hud_Rebuild();
  }
}

void LinkItem_Lamp() {  // 87a24d
  if (is_standing_in_doorway || !CheckYButtonPress())
    return;
  if (link_item_torch && LinkCheckMagicCost(6)) {
    AncillaAdd_MagicPowder(0x1a, 0);
    Dungeon_LightTorch();
    AncillaAdd_LampFlame(0x2f, 2);
  }
  link_item_in_hand = 0;
  button_mask_b_y = 0;
  button_b_frames = 0;
  link_cant_change_direction = 0;
  if (button_b_frames == 9)
    link_speed_setting = 0;
}

void LinkItem_Powder() {  // 87a293
  static const uint8 kMushroomTimer[] = { 2, 1, 1, 3, 2, 2, 2, 2, 6, 0 };

  if (!(button_mask_b_y & 0x40)) {
    if (is_standing_in_doorway || !CheckYButtonPress())
      return;
    if (link_item_mushroom != 2) {
      Ancilla_Sfx2_Near(60);
      goto out;
    }
    if (!LinkCheckMagicCost(2))
      goto out;
    link_delay_timer_spin_attack = kMushroomTimer[0];
    player_handler_timer = 0;
    link_animation_steps = 0;
    link_direction &= ~0xf;
    link_item_in_hand = 0x40;
  }
  link_x_vel = link_y_vel = 0;
  link_direction = 0;
  link_subpixel_x = link_subpixel_y = 0;
  link_moving_against_diag_tile = 0;
  if (!sign8(--link_delay_timer_spin_attack))
    return;
  player_handler_timer++;
  link_delay_timer_spin_attack = kMushroomTimer[player_handler_timer];
  if (player_handler_timer == 4)
    AncillaAdd_MagicPowder(26, 0);
  if (player_handler_timer != 9)
    return;
  if (submodule_index == 0)
    TileDetect_MainHandler(1);
out:
  link_item_in_hand = 0;
  player_handler_timer = 0;
  button_mask_b_y &= ~0x40;
}

void LinkItem_ShovelAndFlute() {  // 87a313
  if (link_item_flute == 1)
    LinkItem_Shovel();
  else if (link_item_flute != 0)
    LinkItem_Flute();
}

void LinkItem_Shovel() {  // 87a32c
  static const uint8 kShovelAnimDelay[] = { 7, 18, 16, 7, 18, 16 };
  static const uint8 kShovelAnimDelay2[] = { 0, 1, 2, 0, 1, 2 };
  if (!(button_mask_b_y & 0x40)) {
    if (is_standing_in_doorway || !CheckYButtonPress())
      return;

    link_delay_timer_spin_attack = kShovelAnimDelay[0];
    link_var30d = 0;
    player_handler_timer = 0;
    link_position_mode = 1;
    link_cant_change_direction |= 1;
    link_animation_steps = 0;
  }
  HaltLinkWhenUsingItems();
  link_direction &= ~0xf;
  if (!sign8(--link_delay_timer_spin_attack))
    return;
  link_var30d++;
  link_delay_timer_spin_attack = kShovelAnimDelay[link_var30d];
  player_handler_timer = kShovelAnimDelay2[link_var30d];

  if (player_handler_timer == 1) {
    TileDetect_MainHandler(2);
    if (BYTE(word_7E04B2)) {
      Ancilla_Sfx3_Near(27);
      AncillaAdd_DugUpFlute(54, 0);
    }

    if (!((tiledetect_thick_grass | tiledetect_destruction_aftermath) & 1)) {
      Ancilla_AddHitStars(22, 0); // hit stars
      Ancilla_Sfx2_Near(5);
    } else {
      AncillaAdd_ShovelDirt(23, 0); // shovel dirt
      if (is_archer_or_shovel_game)
        DiggingGameGuy_AttemptPrizeSpawn();
      Ancilla_Sfx2_Near(18);
    }
  }

  if (link_var30d == 3) {
    link_var30d = 0;
    player_handler_timer = 0;
    button_mask_b_y &= 0x80;
    link_position_mode = 0;
    link_cant_change_direction &= ~1;
  }
}

void LinkItem_Flute() {  // 87a3db
  if (button_mask_b_y & 0x40) {
    if (--flute_countdown)
      return;
    button_mask_b_y &= ~0x40;
  }
  if (!CheckYButtonPress())
    return;
  flute_countdown = 128;
  Ancilla_Sfx2_Near(19);
  if (player_is_indoors || overworld_screen_index & 0x40 || main_module_index == 11)
    return;
  int i = 4;
  do {
    if (ancilla_type[i] == 0x27)
      return;
  } while (--i >= 0);
  if (link_item_flute == 2) {
    if (overworld_screen_index == 0x18 && link_y_coord >= 0x760 && link_y_coord < 0x7e0 && link_x_coord >= 0x1cf && link_x_coord < 0x230) {
      submodule_index = 45;
      AncillaAdd_ExplodingWeatherVane(55, 0);
    }
  } else {
    AncillaAdd_Duck_take_off(39, 4);
    link_need_for_pullforrupees_sprite = 0;
  }
}

void LinkItem_Book() {  // 87a471
  if (button_mask_b_y & 0x40 || is_standing_in_doorway || !CheckYButtonPress())
    return;
  button_mask_b_y &= ~0x40;
  if (byte_7E02ED) {
    Link_PerformDesertPrayer();
  } else {
    Ancilla_Sfx2_Near(60);
  }
}

void LinkItem_Ether() {  // 87a494
  if (!CheckYButtonPress())
    return;
  button_mask_b_y &= ~0x40;

  if (is_standing_in_doorway || flag_block_link_menu || dung_savegame_state_bits & 0x8000 || !((uint8)(link_sword_type + 1) & ~1) ||
      follower_dropped && follower_indicator == 13) {
    Ancilla_Sfx2_Near(60);
    return;
  }

  if (ancilla_type[0] | ancilla_type[1] | ancilla_type[2])
    return;

  if (!LinkCheckMagicCost(1))
    return;
  link_player_handler_state = kPlayerState_Ether;
  link_cant_change_direction |= 1;
  link_delay_timer_spin_attack = kEtherAnimDelays[0];
  state_for_spin_attack = 0;
  step_counter_for_spin_attack = 0;
  byte_7E0324 = 0;
  Ancilla_Sfx3_Near(35);
}

void LinkState_UsingEther() {  // 87a50f
  flag_unk1++;
  if (!sign8(--link_delay_timer_spin_attack))
    return;

  step_counter_for_spin_attack++;
  if (step_counter_for_spin_attack == 4) {
    Ancilla_Sfx3_Near(35);
  } else if (step_counter_for_spin_attack == 9) {
    Ancilla_Sfx2_Near(44);
  } else if (step_counter_for_spin_attack == 12) {
    step_counter_for_spin_attack = 10;
  }
  const uint8 *table = (enhanced_features0 & kFeatures0_DimFlashes) ? kEtherAnimDelaysNoFlash : kEtherAnimDelays;
  link_delay_timer_spin_attack = table[step_counter_for_spin_attack];
  state_for_spin_attack = kEtherAnimStates[step_counter_for_spin_attack];
  if (!byte_7E0324 && step_counter_for_spin_attack == 10) {
    byte_7E0324 = 1;
    AncillaAdd_EtherSpell(24, 0);
    link_auxiliary_state = 0;
    link_incapacitated_timer = 0;
  }
}

void LinkItem_Bombos() {  // 87a569
  if (!CheckYButtonPress())
    return;
  button_mask_b_y &= ~0x40;

  if (is_standing_in_doorway || flag_block_link_menu || dung_savegame_state_bits & 0x8000 || !((uint8)(link_sword_type + 1) & ~1) ||
      follower_dropped && follower_indicator == 13) {
    Ancilla_Sfx2_Near(60);
    return;
  }

  if (ancilla_type[0] | ancilla_type[1] | ancilla_type[2])
    return;

  if (!LinkCheckMagicCost(1))
    return;
  link_player_handler_state = kPlayerState_Bombos;
  link_cant_change_direction |= 1;
  link_delay_timer_spin_attack = kBombosAnimDelays[0];
  state_for_spin_attack = kBombosAnimStates[0];
  step_counter_for_spin_attack = 0;
  byte_7E0324 = 0;
  Ancilla_Sfx3_Near(35);
}

void LinkState_UsingBombos() {  // 87a5f7
  flag_unk1++;
  if (!sign8(--link_delay_timer_spin_attack))
    return;

  step_counter_for_spin_attack++;
  if (step_counter_for_spin_attack == 4) {
    Ancilla_Sfx3_Near(35);
  } else if (step_counter_for_spin_attack == 10) {
    Ancilla_Sfx2_Near(44);
  } else if (step_counter_for_spin_attack == 20) {
    step_counter_for_spin_attack = 19;
  }
  link_delay_timer_spin_attack = kBombosAnimDelays[step_counter_for_spin_attack];
  state_for_spin_attack = kBombosAnimStates[step_counter_for_spin_attack];
  if (!byte_7E0324 && step_counter_for_spin_attack == 19) {
    byte_7E0324 = 1;
    AncillaAdd_BombosSpell(25, 0);
    link_auxiliary_state = 0;
    link_incapacitated_timer = 0;
  }
}

void LinkItem_Quake() {  // 87a64b
  if (!CheckYButtonPress())
    return;
  button_mask_b_y &= ~0x40;

  if (is_standing_in_doorway || flag_block_link_menu || dung_savegame_state_bits & 0x8000 || !((uint8)(link_sword_type + 1) & ~1) ||
      follower_dropped && follower_indicator == 13) {
    Ancilla_Sfx2_Near(60);
    return;
  }

  if (ancilla_type[0] | ancilla_type[1] | ancilla_type[2])
    return;

  if (!LinkCheckMagicCost(1))
    return;
  link_player_handler_state = kPlayerState_Quake;
  link_cant_change_direction |= 1;
  link_delay_timer_spin_attack = kQuakeAnimDelays[0];
  state_for_spin_attack = kQuakeAnimStates[0];
  step_counter_for_spin_attack = 0;
  byte_7E0324 = 0;
  link_actual_vel_z_mirror = 40;
  link_actual_vel_z_copy_mirror = 40;
  BYTE(link_z_coord_mirror) = 0;
  Ancilla_Sfx3_Near(35);
}

void LinkState_UsingQuake() {  // 87a6d6
  flag_unk1++;
  link_actual_vel_x = link_actual_vel_y = 0;

  if (step_counter_for_spin_attack == 10) {
    link_actual_vel_z = link_actual_vel_z_mirror;
    link_actual_vel_z_copy = link_actual_vel_z_copy_mirror;
    BYTE(link_z_coord) = link_z_coord_mirror;
    link_auxiliary_state = 2;
    Player_ChangeZ(2);
    Link_MovePosition();
    link_actual_vel_z_mirror = link_actual_vel_z;
    link_actual_vel_z_copy_mirror = link_actual_vel_z_copy;
    BYTE(link_z_coord_mirror) = link_z_coord;
    if (!sign8(link_z_coord)) {
      state_for_spin_attack = sign8(link_actual_vel_z) ? 21 : 20;
      return;
    }
  } else {
    if (!sign8(--link_delay_timer_spin_attack))
      return;
  }

  step_counter_for_spin_attack++;
  if (step_counter_for_spin_attack == 4) {
    Ancilla_Sfx3_Near(35);
  } else if (step_counter_for_spin_attack == 10) {
    Ancilla_Sfx2_Near(44);
  } else if (step_counter_for_spin_attack == 11) {
    Ancilla_Sfx2_Near(12);
  } else if (step_counter_for_spin_attack == 12) {
    step_counter_for_spin_attack = 11;
  }
  link_delay_timer_spin_attack = kQuakeAnimDelays[step_counter_for_spin_attack];
  state_for_spin_attack = kQuakeAnimStates[step_counter_for_spin_attack];
  if (!byte_7E0324 && step_counter_for_spin_attack == 11) {
    byte_7E0324 = 1;
    AncillaAdd_QuakeSpell(28, 0);
    link_auxiliary_state = 0;
    link_incapacitated_timer = 0;
  }
}

void Link_ActivateSpinAttack() {  // 87a77a
  AncillaAdd_SpinAttackInitSpark(42, 0, 0);
  Link_AnimateVictorySpin();
}

void Link_AnimateVictorySpin() {  // 87a783
  link_player_handler_state = 3;
  link_spin_offsets = (link_direction_facing >> 1) * 12;
  link_delay_timer_spin_attack = 3;
  state_for_spin_attack = kLinkSpinGraphicsByDir[link_spin_offsets];
  step_counter_for_spin_attack = 0;
  button_b_frames = 144;
  link_cant_change_direction |= 1;
  button_mask_b_y = 0x80;
  LinkState_SpinAttack();
}

void LinkState_SpinAttack() {  // 87a804
  CacheCameraPropertiesIfOutdoors();

  if (link_auxiliary_state) {
    int i = 4;
    do {
      if (ancilla_type[i] == 0x2a || ancilla_type[i] == 0x2b)
        ancilla_type[i] = 0;
    } while (--i >= 0);
    link_z_coord &= 0xff;
    link_cant_change_direction &= ~1;
    link_delay_timer_spin_attack = 0;
    button_b_frames = 0;
    button_mask_b_y = 0;
    bitfield_for_a_button = 0;
    state_for_spin_attack = 0;
    step_counter_for_spin_attack = 0;
    link_speed_setting = 0;
    if (link_electrocute_on_touch) {
      if (link_cape_mode)
        Link_ForceUnequipCape_quietly();
      Link_ResetSwordAndItemUsage();
      link_disable_sprite_damage = 1;
      player_handler_timer = 0;
      link_delay_timer_spin_attack = 2;
      link_animation_steps = 0;
      link_direction &= ~0xf;
      Ancilla_Sfx3_Near(43);
      link_player_handler_state = kPlayerState_Electrocution;
      LinkState_Zapped();
    } else {
      link_player_handler_state = kPlayerState_RecoilWall;
      LinkState_Recoil();
    }
    return;
  }

  if (link_incapacitated_timer) {
    Link_HandleRecoilAndTimer(false);
  } else {
    link_direction = 0;
    Link_HandleVelocity();
    Link_HandleCardinalCollision();
    link_player_handler_state = kPlayerState_SpinAttacking;
    fallhole_var1 = 0;
    HandleIndoorCameraAndDoors();
  }

  if (!sign8(--link_delay_timer_spin_attack))
    return;

  step_counter_for_spin_attack++;

  if (step_counter_for_spin_attack == 2)
    Ancilla_Sfx3_Near(35);

  if (step_counter_for_spin_attack == 12) {
    link_cant_change_direction &= ~1;
    link_delay_timer_spin_attack = 0;
    button_b_frames = 0;
    state_for_spin_attack = 0;
    step_counter_for_spin_attack = 0;
    if (link_player_handler_state != kPlayerState_SpinAttackMotion) {
      button_mask_b_y = (button_b_frames) ? (joypad1H_last & kJoypadH_B) : 0; // wtf, it's zero,
    }
    link_player_handler_state = kPlayerState_Ground;
  } else {
    state_for_spin_attack = kLinkSpinGraphicsByDir[step_counter_for_spin_attack + link_spin_offsets];
    link_delay_timer_spin_attack = kLinkSpinDelays[step_counter_for_spin_attack];
    TileDetect_MainHandler(8);
  }
}

void LinkItem_Mirror() {  // 87a91a
  if (!(button_mask_b_y & 0x40)) {
    if (!CheckYButtonPress())
      return;

    if (follower_indicator == 10) {
      dialogue_message_index = 289;
      Main_ShowTextMessage();
      return;
    }
  }
  button_mask_b_y &= ~0x40;

  if (is_standing_in_doorway || 
      !cheatWalkThroughWalls && !(enhanced_features0 & kFeatures0_MirrorToDarkworld) && 
      !player_is_indoors && !(overworld_screen_index & 0x40)) {
    Ancilla_Sfx2_Near(60);
    return;
  }

  DoSwordInteractionWithTiles_Mirror();
}

void DoSwordInteractionWithTiles_Mirror() {  // 87a95c
  if (player_is_indoors) {
    if (flag_block_link_menu)
      return;
    Mirror_SaveRoomData();
    if (sound_effect_1 != 60) {
      index_of_changable_dungeon_objs[0] = 0;
      index_of_changable_dungeon_objs[1] = 0;
    }
  } else if (main_module_index != 11) {
    last_light_vs_dark_world = overworld_screen_index & 0x40;
    if (last_light_vs_dark_world) {
      bird_travel_y_lo[15] = link_y_coord;
      bird_travel_y_hi[15] = link_y_coord >> 8;
      bird_travel_x_lo[15] = link_x_coord;
      bird_travel_x_hi[15] = link_x_coord >> 8;
    }
    submodule_index = 35;
    link_need_for_pullforrupees_sprite = 0;
    link_triggered_by_whirlpool_sprite = 1;
    subsubmodule_index = 0;
    link_actual_vel_x = link_actual_vel_y = 0;
    link_player_handler_state = kPlayerState_Mirror;
  }
}

void LinkState_CrossingWorlds() {  // 87a9b1
  uint8 t;

  Link_ResetProperties_B();
  TileCheckForMirrorBonk();

  if ((overworld_screen_index & 0x40) != last_light_vs_dark_world && ((t = R12 | R14) & 0xc) != 0 && BitSum4(t) >= 2)
    goto do_mirror;

  if (BitSum4(tiledetect_deepwater) >= 2) {
    if (link_item_flippers) {
      link_is_in_deep_water = 1;
      link_some_direction_bits = link_direction_last;
      Link_ResetSwimmingState();
      link_player_handler_state = kPlayerState_Swimming;
      Link_ForceUnequipCape_quietly();
      link_speed_setting = 0;
      return;
    }
    if ((overworld_screen_index & 0x40) != last_light_vs_dark_world) {
do_mirror:
      submodule_index = 44;
      link_need_for_pullforrupees_sprite = 0;
      link_triggered_by_whirlpool_sprite = 1;
      subsubmodule_index = 0;
      link_actual_vel_x = link_actual_vel_y = 0;
      link_player_handler_state = kPlayerState_Mirror;
      return;
    }
    CheckAbilityToSwim();
  }

  if (link_is_in_deep_water) {
    link_is_in_deep_water = 0;
    link_direction_last = link_some_direction_bits;
  }

  link_countdown_for_dash = 0;
  link_is_running = 0;
  link_speed_setting = 0;
  button_mask_b_y = 0;
  button_b_frames = 0;
  link_cant_change_direction = 0;
  swimcoll_var5[0] &= ~0xff;
  link_actual_vel_y = 0;

  if ((overworld_screen_index & 0x40) != last_light_vs_dark_world)
    num_memorized_tiles = 0;

  link_player_handler_state = (link_item_moon_pearl || !(overworld_screen_index & 0x40)) ? kPlayerState_Ground : kPlayerState_PermaBunny;
}

void Link_PerformDesertPrayer() {  // 87aa6c
  submodule_index = 5;
  saved_module_for_menu = main_module_index;
  main_module_index = 14;
  flag_unk1 = 1;
  some_animation_timer = 22;
  some_animation_timer_steps = 0;
  link_state_bits = 2;
  link_cant_change_direction |= 1;
  link_animation_steps = 0;
  link_direction &= ~0xf;
  sound_effect_ambient = 17;
  music_control = 242;
}

void HandleFollowersAfterMirroring() {  // 87aaa2
  TileDetect_MainHandler(0);
  link_animation_steps = 0;
  if (follower_indicator == 12 || follower_indicator == 13) {
    if (follower_indicator == 13) {
      super_bomb_indicator_unk2 = 0xfe;
      super_bomb_indicator_unk1 = 0;
    }
    if (follower_dropped) {
      follower_dropped = 0;
      follower_indicator = 0;
    }
  } else if (follower_indicator == 9 || follower_indicator == 10) {
    follower_indicator = 0;
  } else if (follower_indicator == 7 || follower_indicator == 8) {
    follower_indicator ^= (7 ^ 8);
    LoadFollowerGraphics();
    AncillaAdd_DwarfPoof(0x40, 4);
  }

  if (!link_item_moon_pearl) {
    AncillaAdd_BunnyPoof(0x23, 4);
    Link_ForceUnequipCape_quietly();
    link_bunny_transform_timer = 0;
  } else if (link_cape_mode) {
    Link_ForceUnequipCape();
    link_bunny_transform_timer = 0;
  }
}

void LinkItem_Hookshot() {  // 87ab25
  if (button_mask_b_y & 0x40 || is_standing_in_doorway || bitmask_of_dragstate & 2 || !CheckYButtonPress())
    return;

  ResetAllAcceleration();
  player_handler_timer = 0;
  link_cant_change_direction |= 1;
  link_delay_timer_spin_attack = 7;
  link_animation_steps = 0;
  link_direction &= ~0xf;
  link_position_mode = 4;
  link_player_handler_state = kPlayerState_Hookshot;
  link_disable_sprite_damage = 1;
  AncillaAdd_Hookshot(31, 3);
}

void LinkState_Hookshotting() {  // 87ab7c
  static const int8 kHookshotArrA[4] = { -8, -16, 0, 0 };
  static const int8 kHookshotArrB[4] = { 0, 0, 4, -12 };
  static const int8 kHookshotArrC[4] = { -64, 64, 0, 0 };
  static const int8 kHookshotArrD[4] = { 0, 0, -64, 64 };

  link_give_damage = 0;
  link_auxiliary_state = 0;
  link_incapacitated_timer = 0;
  int i = 4;
  while (ancilla_type[i] != 0x1f) {
    if (--i < 0) {
      if (!sign8(--link_delay_timer_spin_attack))
        return;
      player_handler_timer = 0;
      link_disable_sprite_damage = 0;
      button_mask_b_y &= ~0x40;
      link_cant_change_direction &= ~1;
      link_position_mode &= ~4;
      link_player_handler_state = kPlayerState_Ground;
      if (button_b_frames >= 9)
        button_b_frames = 9;
      return;
    }
  }

  if (sign8(--link_delay_timer_spin_attack))
    link_delay_timer_spin_attack = 0;

  if (!related_to_hookshot) {
    link_y_coord_safe_return_lo = link_y_coord;
    link_x_coord_safe_return_lo = link_x_coord;
    link_y_vel = link_x_vel = 0;
    Link_HandleCardinalCollision();
    return;
  }

  player_on_somaria_platform = 0;

  uint8 hei = hookshot_effect_index;
  if (sign8(--ancilla_item_to_link[hei])) {
    ancilla_item_to_link[hei] = 0;
  } else {
    uint16 x = ancilla_x_lo[hei] | (ancilla_x_hi[hei] << 8);
    uint16 y = ancilla_y_lo[hei] | (ancilla_y_hi[hei] << 8);
    int8 r4 = kHookshotArrA[ancilla_dir[hei]];
    int8 r6 = kHookshotArrB[ancilla_dir[hei]];
    link_actual_vel_x = link_actual_vel_y = 0;
    int8 r8 = kHookshotArrC[ancilla_dir[hei]];
    int8 r10 = kHookshotArrD[ancilla_dir[hei]];

    uint16 yd = (int16)(y + r4 - link_y_coord);
    if ((int16)yd < 0)
      yd = -yd;
    if (yd >= 2)
      link_actual_vel_y = r8;

    uint16 xd = (int16)(x + r6 - link_x_coord);
    if ((int16)xd < 0)
      xd = -xd;
    if (xd >= 2)
      link_actual_vel_x = r10;

    if (link_actual_vel_x | link_actual_vel_y)
      goto loc_87AD49;
  }

  ancilla_type[hei] = 0;
  tagalong_var7 = tagalong_var1;
  link_player_handler_state = kPlayerState_Ground;
  player_handler_timer = 0;
  link_delay_timer_spin_attack = 0;
  related_to_hookshot = 0;
  button_mask_b_y &= ~0x40;
  link_cant_change_direction &= ~1;
  link_position_mode &= ~4;
  link_disable_sprite_damage = 0;

  if (ancilla_arr1[hei]) {
    link_is_on_lower_level_mirror ^= 1;
    dung_cur_floor--;
    if (kind_of_in_room_staircase == 0) {
      BYTE(dungeon_room_index2) = dungeon_room_index;
      BYTE(dungeon_room_index) += 0x10;
    }
    if (kind_of_in_room_staircase != 2) {
      link_is_on_lower_level ^= 1;
    }
    Dungeon_FlagRoomData_Quadrants();
  }
  Player_TileDetectNearby();
  if (tiledetect_deepwater & 0xf && !link_is_in_deep_water) {
    link_is_in_deep_water = 1;
    link_some_direction_bits = link_direction_last;
    Link_ResetSwimmingState();
    AncillaAdd_Splash(21, 0);
    link_player_handler_state = kPlayerState_Swimming;
    Link_ForceUnequipCape_quietly();
    link_state_bits = 0;
    link_picking_throw_state = 0;
    link_grabbing_wall = 0;
    link_speed_setting = 0;
    if (player_is_indoors)
      link_is_on_lower_level = 1;
    if (button_b_frames >= 9)
      button_b_frames = 9;
  } else if (tiledetect_pit_tile & 0xf) {
    byte_7E005C = 9;
    link_this_controls_sprite_oam = 0;
    player_near_pit_state = 1;
    link_player_handler_state = kPlayerState_FallingIntoHole;
    if (button_b_frames >= 9)
      button_b_frames = 9;
  } else {
    link_y_coord_safe_return_lo = link_y_coord;
    link_y_coord_safe_return_hi = link_y_coord >> 8;
    link_x_coord_safe_return_lo = link_x_coord;
    link_x_coord_safe_return_hi = link_x_coord >> 8;
    Link_HandleCardinalCollision();
    HandleIndoorCameraAndDoors();
  }
  return;
loc_87AD49:
  Link_MovePosition();
  TileDetect_MainHandler(5);
  if (player_is_indoors) {
    uint8 x = tiledetect_vertical_ledge >> 4 | tiledetect_vertical_ledge | detection_of_ledge_tiles_horiz_uphoriz;
    if (x & 1 && sign8(--hookshot_var1)) {
      hookshot_var1 = 3;
      related_to_hookshot ^= 2;
    }
  }
  draw_water_ripples_or_grass = 0;
  if (!(related_to_hookshot & 2)) {
    if (tiledetect_thick_grass & 1) {
      draw_water_ripples_or_grass = 2;
      if (!Link_PermissionForSloshSounds())
        Ancilla_Sfx2_Near(26);
    } else if ((tiledetect_shallow_water | tiledetect_deepwater) & 1) {
      draw_water_ripples_or_grass++;
      Ancilla_Sfx2_Near((uint8)overworld_screen_index == 0x70 ? 27 : 28);
    }
  }

  HandleIndoorCameraAndDoors();
}

void LinkItem_Cape() {  // 87adc1
  if (!link_cape_mode) {
    if (!sign8(--link_bunny_transform_timer)) {
      link_direction &= ~0xf;
      HaltLinkWhenUsingItems();
      return;
    }
    link_bunny_transform_timer = 0;
    if (is_standing_in_doorway || !CheckYButtonPress())
      return;
    button_mask_b_y &= ~0x40;
    if (!link_magic_power) {
      Ancilla_Sfx2_Near(60);
      dialogue_message_index = 123;
      Main_ShowTextMessage();
      return;
    }
    player_handler_timer = 0;
    link_cape_mode = 1;
    cape_decrement_counter = kCapeDepletionTimers[link_magic_consumption];
    link_bunny_transform_timer = 20;
    AncillaAdd_CapePoof(35, 4);
    Ancilla_Sfx2_Near(20);
  } else {
    link_disable_sprite_damage = 1;
    HaltLinkWhenUsingItems();
    link_direction &= ~0xf;
    if (!--cape_decrement_counter) {
      cape_decrement_counter = kCapeDepletionTimers[link_magic_consumption];
      // Avoid magic underflow if an anti-fairy consumes magic.
      if (link_magic_power == 0 && (enhanced_features0 & kFeatures0_MiscBugFixes) ||
          !--link_magic_power) {
        Link_ForceUnequipCape();
        return;
      }
    }
    if (sign8(--link_bunny_transform_timer)) {
      link_bunny_transform_timer = 0;
      if (filtered_joypad_H & kJoypadH_Y)
        Link_ForceUnequipCape();
    }
  }
}

void Link_ForceUnequipCape() {  // 87ae47
  AncillaAdd_CapePoof(35, 4);
  Ancilla_Sfx2_Near(21);
  Link_ForceUnequipCape_quietly();
}

void Link_ForceUnequipCape_quietly() {  // 87ae54
  link_bunny_transform_timer = 32;
  link_disable_sprite_damage = 0;
  link_cape_mode = 0;
  link_electrocute_on_touch = 0;
}

void HaltLinkWhenUsingItems() {  // 87ae65
  if (dung_hdr_collision_2 == 2 && (byte_7E0322 & 3) == 3) {
    link_y_vel = 0;
    link_x_vel = 0;
    link_direction = 0;
    link_subpixel_y = 0;
    link_subpixel_x = 0;
    link_moving_against_diag_tile = 0;
  }
  if (player_on_somaria_platform)
    link_direction = 0;
}

void Link_HandleCape_passive_LiftCheck() {  // 87ae88
  //bugfix: grabbing or pulling while wearing cape didn't drain magic
  if (link_state_bits & 0x80 || (enhanced_features0 & kFeatures0_MiscBugFixes && link_grabbing_wall))
    Player_CheckHandleCapeStuff();
}

void Player_CheckHandleCapeStuff() {  // 87ae8f
  if (link_cape_mode && current_item_active == 19) {
    if (current_item_active == current_item_y) {
      if (--cape_decrement_counter)
        return;
      cape_decrement_counter = kCapeDepletionTimers[link_magic_consumption];
      if (!link_magic_power || --link_magic_power)
        return;
    }
    Link_ForceUnequipCape();
  }
}

void LinkItem_CaneOfSomaria() {  // 87aec0
  static const uint8 kRodAnimDelays[] = { 3, 3, 5 };
  if (!(button_mask_b_y & 0x40)) {
    if (player_on_somaria_platform || is_standing_in_doorway || !CheckYButtonPress())
      return;
    int i = 4;
    bool did_charge_magic = false;

    while (ancilla_type[i] != 0x2c) {
      if (--i < 0) {
        if (!LinkCheckMagicCost(4)) {
          // If you use the Cane of Somaria with an empty magic meter,
          // then quickly switch to the mushroom or magic powder after
          // the "no magic" prompt, you will automatically sprinkle magic powder
          // despite pressing no button and having no magic.
          if (enhanced_features0 & kFeatures0_MiscBugFixes)
            goto out;
          return;
        }
        did_charge_magic = true;
        break;
      }
    }
    link_debug_value_2 = 1;
    if (AncillaAdd_SomariaBlock(0x2c, 1) < 0) {
      // If you use the Cane of Somaria while two bombs and the boomerang are active,
      // magic will be refunded instead of used.
      if (did_charge_magic || !(enhanced_features0 & kFeatures0_MiscBugFixes))
        Refund_Magic(4);
    }
    link_delay_timer_spin_attack = kRodAnimDelays[0];
    link_animation_steps = 0;
    player_handler_timer = 0;
    link_item_in_hand = 0;
    link_position_mode |= 8;
  }

  HaltLinkWhenUsingItems();
  link_direction &= ~0xf;
  if (!sign8(--link_delay_timer_spin_attack))
    return;
  player_handler_timer++;

  link_delay_timer_spin_attack = kRodAnimDelays[player_handler_timer];
  if (player_handler_timer != 3)
    return;
  link_speed_setting = 0;
  player_handler_timer = 0;
  link_delay_timer_spin_attack = 0;
  link_debug_value_2 = 0;
  link_position_mode &= ~8;
out:
  button_mask_b_y &= ~0x40;
}

void LinkItem_CaneOfByrna() {  // 87af3e
  static const uint8 kByrnaDelays[] = { 19, 7, 13, 32 };
  if (SearchForByrnaSpark())
    return;
  if (!(button_mask_b_y & 0x40)) {
    if (is_standing_in_doorway || !CheckYButtonPress())
      return;
    if (!LinkCheckMagicCost(8))
      goto out;
    AncillaAdd_CaneOfByrnaInitSpark(48, 0);
    link_spin_attack_step_counter = 0;
    link_delay_timer_spin_attack = kByrnaDelays[0];
    link_var30d = 0;
    player_handler_timer = 0;
    link_position_mode = 8;
    link_cant_change_direction |= 1;
    link_animation_steps = 0;
  }
  HaltLinkWhenUsingItems();
  link_direction &= ~0xf;
  if (!sign8(--link_delay_timer_spin_attack))
    return;
  player_handler_timer++;
  link_delay_timer_spin_attack = kByrnaDelays[player_handler_timer];
  if (player_handler_timer == 1) {
    Ancilla_Sfx3_Near(42);
  } else if (player_handler_timer == 3) {
out:
    link_var30d = 0;
    player_handler_timer = 0;
    button_mask_b_y &= 0x80;
    link_position_mode = 0;
    link_cant_change_direction &= ~1;
  }
}

bool SearchForByrnaSpark() {  // 87afb5
  if (link_position_mode & 8)
    return false;
  int i = 4;
  do {
    if (ancilla_type[i] == 0x31)
      return true;
  } while (--i >= 0);
  return false;
}

void LinkItem_Net() {  // 87aff8
  static const uint8 kBugNetTimers[] = { 11, 6, 7, 8, 1, 2, 3, 4, 5, 6, 1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 9, 4, 5, 6, 7, 8, 1, 2, 3, 4, 10, 8, 1, 2, 3, 4, 5, 6, 7, 8 };
  if (!(button_mask_b_y & 0x40)) {
    if (is_standing_in_doorway || !CheckYButtonPress())
      return;

    player_handler_timer = kBugNetTimers[(link_direction_facing >> 1) * 10];
    link_delay_timer_spin_attack = 3;
    link_var30d = 0;
    link_position_mode = 16;
    link_cant_change_direction |= 1;
    link_animation_steps = 0;
    Ancilla_Sfx2_Near(50);
  }

  HaltLinkWhenUsingItems();
  link_direction &= ~0xf;
  if (!sign8(--link_delay_timer_spin_attack))
    return;

  link_var30d++;
  link_delay_timer_spin_attack = 3;
  player_handler_timer = kBugNetTimers[(link_direction_facing >> 1) * 10 + link_var30d];

  if (link_var30d == 10) {
    link_var30d = 0;
    player_handler_timer = 0;
    button_mask_b_y &= 0x80;
    link_position_mode = 0;
    link_cant_change_direction &= ~1;
    player_oam_x_offset = 0x80;
    player_oam_y_offset = 0x80;
  }
}

bool CheckYButtonPress() {  // 87b073
  if (button_mask_b_y & 0x40 || link_incapacitated_timer || !(filtered_joypad_H & kJoypadH_Y))
    return false;
  button_mask_b_y |= 0x40;
  return true;
}

bool LinkCheckMagicCost(uint8 x) {  // 87b0ab
  uint8 cost = kLinkItem_MagicCosts[x * 3 + link_magic_consumption];
  uint8 a = link_magic_power;
  if (a && (a -= cost) < 0x80) {
    link_magic_power = a;
    return true;
  }
  if (x != 3) {
    Ancilla_Sfx2_Near(60);
    dialogue_message_index = 123;
    Main_ShowTextMessage();
  }
  return false;
}

void Refund_Magic(uint8 x) {  // 87b0e9
  uint8 cost = kLinkItem_MagicCosts[x * 3 + link_magic_consumption];

  int new_magic = link_magic_power + cost;
  // Ensure magic can't overflow (for example the cane of somaria bug)
  if (enhanced_features0 & kFeatures0_MiscBugFixes && new_magic >= 128)
    new_magic = 128;
  link_magic_power = new_magic;
}

void Link_ItemReset_FromOverworldThings() {  // 87b107
  some_animation_timer_steps = 0;
  bitfield_for_a_button = 0;
  link_state_bits = 0;
  link_picking_throw_state = 0;
  link_grabbing_wall = 0;
  link_cant_change_direction &= ~1;
}

void Link_PerformThrow() {  // 87b11c

  if (!(flag_is_sprite_to_pick_up | flag_is_ancilla_to_pick_up)) {
    Link_ResetSwordAndItemUsage();
    bitfield_for_a_button = 0;
    int i = 15;
    while (sprite_state[i] != 0) {
      if (--i < 0)
        return;
    }

    if (interacting_with_liftable_tile_x1 == 5 || interacting_with_liftable_tile_x1 == 6) {
      player_handler_timer = 1;
    } else {
      Point16U pt;
      uint8 attr = player_is_indoors ? Dungeon_LiftAndReplaceLiftable(&pt) : Overworld_HandleLiftableTiles(&pt);

      i = 8;
      while (kLink_Lift_tab[i] != attr) {
        if (--i < 0)
          return;
      }

      flag_is_sprite_to_pick_up = 1;
      Sprite_SpawnThrowableTerrain(i, pt.x, pt.y);
      filtered_joypad_L &= ~kJoypadL_A;
      player_handler_timer = 0;
    }
  } else {
    player_handler_timer = 0;
  }

  button_mask_b_y = 0;
  some_animation_timer = 6;
  link_picking_throw_state = 1;
  link_state_bits = 0x80;
  some_animation_timer_steps = 0;
  link_speed_setting = 12;
  link_animation_steps = 0;
  link_direction &= 0xf0;
  link_cant_change_direction |= 1;
}

void Link_APress_LiftCarryThrow() {  // 87b1ca
  if (!link_state_bits)
    return;

  // throwing?
  if ((link_picking_throw_state & 2) && some_animation_timer >= 5)
    some_animation_timer = 5;

  // picking up?
  if (link_picking_throw_state)
    HaltLinkWhenUsingItems();

  if (link_picking_throw_state & 1) {
    link_animation_steps = 0;
    link_counter_var1 = 0;
    link_direction &= ~0xf;
  }

  if (--some_animation_timer)
    return;

  if (link_picking_throw_state & 2) {
    link_state_bits = 0;
    bitmask_of_dragstate = 0;
    link_speed_setting = 0;
    if (link_player_handler_state == 24)
      link_player_handler_state = 0;
  } else {
    static const uint8 kLiftTab0[10] = { 8, 24, 8, 24, 8, 32, 6, 8, 13, 13 };
    static const uint8 kLiftTab1[10] = { 0, 1, 0, 1, 0, 1, 0, 1, 2, 3 };
    static const uint8 kLiftTab2[29] = { 6, 7, 7, 5, 10, 0, 23, 0, 18, 0, 18, 0, 8, 0, 8, 0, 254, 255, 17, 0, 
        0x54, 0x52, 0x50, 0xFF, 0x51, 0x53, 0x55, 0x56, 0x57 };

    if (player_handler_timer != 0) {
      if (player_handler_timer + 1 != 9) {
        player_handler_timer++;
        some_animation_timer = kLiftTab0[player_handler_timer];
        some_animation_timer_steps = kLiftTab1[player_handler_timer];
        if (player_handler_timer == 6) {
          BYTE(dung_secrets_unk1) = 0;
          Point16U pt;
          uint8 what = (player_is_indoors) ? Dungeon_LiftAndReplaceLiftable(&pt) : Overworld_HandleLiftableTiles(&pt);
          link_player_handler_state = 24;
          flag_is_sprite_to_pick_up = 1;
          Sprite_SpawnThrowableTerrain((what & 0xf) + 1, pt.x, pt.y);
          filtered_joypad_L &= ~kJoypadL_A;
        }
        return;
      }
    } else {
      // fix OOB read triggered when lifting for too long
      if (some_animation_timer_steps >= sizeof(kLiftTab2) - 1)
        return;
      some_animation_timer = kLiftTab2[++some_animation_timer_steps];
      assert(some_animation_timer_steps < arraysize(kLiftTab2));
      if (some_animation_timer_steps != 3)
        return;
    }
  }

  // stop animation
  link_picking_throw_state = 0;
  link_cant_change_direction &= ~1;
}

void Link_PerformDash() {  // 87b281
  if (player_on_somaria_platform)
    return;
  if (flag_is_sprite_to_pick_up | flag_is_ancilla_to_pick_up)
    return;
  if (link_state_bits & 0x80)
    return;
  bitfield_for_a_button = 0;
  link_countdown_for_dash = 29;
  link_dash_ctr = 64;
  link_player_handler_state = kPlayerState_StartDash;
  link_is_running = 1;
  button_mask_b_y &= 0x80;
  link_state_bits = 0;
  link_item_in_hand = 0;
  bitmask_of_dragstate = 0;
  link_moving_against_diag_tile = 0;

  if (follower_indicator == kTagalongArr1[follower_indicator]) {
    printf("Warning: Write to CART!\n");
    link_speed_setting = 0;
    timer_tagalong_reacquire = 64;
  }
}

void Link_PerformGrab() {  // 87b2ee
  if ((button_mask_b_y & 0x80) && button_b_frames >= 9)
    return;

  link_grabbing_wall = 1;
  link_cant_change_direction |= 1;
  link_animation_steps = 0;
  some_animation_timer_steps = 0;
  some_animation_timer = 0;
  link_var30d = 0;
}

void Link_APress_PullObject() {  // 87b322
  link_direction &= ~0xf;

  if (!(kGrabWallDirs[link_direction_facing >> 1] & joypad1H_last)) {
    link_var30d = 0;
    goto set;
  } else if (sign8(--some_animation_timer)) {
    link_var30d = (link_var30d + 1 == 7) ? 1 : link_var30d + 1;
set:
    some_animation_timer_steps = kGrabWall_AnimSteps[link_var30d];
    some_animation_timer = kGrabWall_AnimTimer[link_var30d];
  }

  if (!(joypad1L_last & kJoypadL_A)) {
    link_var30d = 0;
    some_animation_timer_steps = 0;
    link_grabbing_wall = 0;
    bitfield_for_a_button = 0;
    link_cant_change_direction &= ~1;
  }
}

void Link_PerformStatueDrag() {  // 87b371
  link_grabbing_wall = 2;
  link_cant_change_direction |= 1;
  link_animation_steps = 0;
  some_animation_timer_steps = 0;
  some_animation_timer = kGrabWall_AnimTimer[0];
  link_var30d = 0;
}

void Link_APress_StatueDrag() {  // 87b389
  link_speed_setting = 20;
  int j;
  if (!(j = joypad1H_last & kGrabWallDirs[link_direction_facing >> 1])) {
    link_direction = 0;
    link_x_vel = link_y_vel = 0;
    link_animation_steps = 0;
    link_var30d = 0;
  } else {
    link_direction = j;
    if (!sign8(--some_animation_timer))
      goto skip_set;
    link_var30d = (link_var30d + 1 == 7) ? 1 : link_var30d + 1;
  }
  some_animation_timer_steps = kGrabWall_AnimSteps[link_var30d];
  some_animation_timer = kGrabWall_AnimTimer[link_var30d];
skip_set:
  if (!(joypad1L_last & kJoypadL_A)) {
    link_speed_setting = 0;
    link_is_near_moveable_statue = 0;
    link_var30d = 0;
    some_animation_timer_steps = 0;
    link_grabbing_wall = 0;
    bitfield_for_a_button = 0;
    link_cant_change_direction &= ~1;
  }
}

void Link_PerformRupeePull() {  // 87b3e5
  if (link_direction_facing != 0)
    return;
  Link_ResetProperties_A();
  link_grabbing_wall = 2;
  link_cant_change_direction |= 2;

  link_animation_steps = 0;
  some_animation_timer_steps = 0;
  some_animation_timer = kGrabWall_AnimTimer[0];
  link_var30d = 0;
  link_player_handler_state = kPlayerState_PullForRupees;
  link_actual_vel_y = 0;
  link_actual_vel_x = 0;
  button_mask_b_y = 0;
}

void LinkState_TreePull() {  // 87b416
  CacheCameraPropertiesIfOutdoors();
  if (link_auxiliary_state) {
    HandleLink_From1D();
    return;
  }

  if (link_grabbing_wall) {
    if (!button_mask_b_y) {
      if (!(joypad1L_last & kJoypadL_A)) {
        link_grabbing_wall = 0;
        link_var30d = 0;
        some_animation_timer = 2;
        some_animation_timer_steps = 0;
        link_cant_change_direction = 0;
        link_player_handler_state = 0;
        LinkState_Default();
        return;
      }
      if (!(joypad1H_last & kJoypadH_Down))
        goto out;
      button_mask_b_y = 4;
      Ancilla_Sfx2_Near(0x22);
    }

    if (!sign8(--some_animation_timer))
      goto out;
    int j = ++link_var30d;
    some_animation_timer_steps = kGrabWall_AnimSteps[j];
    some_animation_timer = kGrabWall_AnimTimer[j];
    if (j != 7)
      goto out;

    link_grabbing_wall = 0;
    link_var30d = 0;
    some_animation_timer = 2;
    some_animation_timer_steps = 0;
    link_state_bits = 1;
    link_picking_throw_state = 0;
  }

  if (bitmask_of_dragstate & 9) {
reset_to_normal:
    link_direction_facing = 0;
    link_state_bits = 0;
    link_cant_change_direction = 0;
    link_player_handler_state = kPlayerState_Ground;
    return;
  }
  if (link_var30d == 9) {
    if (!(filtered_joypad_H & kJoypadH_AnyDir))
      goto out2;
    link_player_handler_state = kPlayerState_Ground;
    LinkState_Default();
    return;
  }
  AncillaAdd_DashDust_charging(0x1e, 0);
  if (sign8(--some_animation_timer)) {
    static const uint8 kGrabWall_AnimSteps2[10] = { 0, 1, 2, 3, 4, 0, 1, 2, 3, 0x20 };  // oob read
    int j = ++link_var30d;
    some_animation_timer_steps = kGrabWall_AnimSteps2[j];
    some_animation_timer = 2;
    link_actual_vel_y = 48;
    if (j == 9)
      goto reset_to_normal;
  }
  Flag67WithDirections();
  if (!(link_direction & 3))
    link_actual_vel_x = 0;
  if (!(link_direction & 0xc))
    link_actual_vel_y = 0;
out:
  Link_MovePosition();
out2:
  Link_HandleCardinalCollision();
  HandleIndoorCameraAndDoors();
}

void Link_PerformRead() {  // 87b4f2
  if (player_is_indoors) {
    dialogue_message_index = Dungeon_GetTeleMsg(dungeon_room_index);
  } else {
    dialogue_message_index = (sram_progress_indicator < 2) ? 0x3A : Overworld_GetSignText(overworld_screen_index);
  }
  Main_ShowTextMessage();
  bitfield_for_a_button = 0;
}

void Link_PerformOpenChest() {  // 87b574
  static const uint8 kReceiveItemAlternates[] = { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 68, 255, 255, 255, 255, 255, 53, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 70, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 };
  if (link_direction_facing || item_receipt_method || link_auxiliary_state)
    return;
  bitfield_for_a_button = 0;
  int chest_position = -1;
  uint8 item = OpenChestForItem(index_of_interacting_tile, &chest_position);
  if (sign8(item)) {
    item_receipt_method = 0;
    return;
  }
  assert(chest_position != -1);
  item_receipt_method = 1;
  uint8 alt = kReceiveItemAlternates[item];
  if (alt != 0xff) {
    uint16 ram_addr = kMemoryLocationToGiveItemTo[item];
    if (g_ram[ram_addr])
      item = alt;
  }

  Link_ReceiveItem(item, chest_position);
}

bool Link_CheckNewAPress() {  // 87b5c0
  if (bitfield_for_a_button & 0x80 || link_incapacitated_timer || !(filtered_joypad_L & kJoypadL_A))
    return false;
  bitfield_for_a_button |= 0x80;
  return true;
}

bool Link_HandleToss() {  // 87b5d6
  if (!(bitfield_for_a_button & 0x80) || !(filtered_joypad_L & kJoypadL_A) || (link_picking_throw_state & 1))
    return false;
  link_var30d = 0;
  link_var30e = 0;
  some_animation_timer_steps = 0;
  bitfield_for_a_button = 0;
  link_cant_change_direction &= ~1;
  // debug stuff here
  return true;
}

void Link_HandleDiagonalCollision() {  // 87b64f
  if (CheckIfRoomNeedsDoubleLayerCheck()) {
    Player_LimitDirections_Inner();
    CreateVelocityFromMovingBackground();
  }
  link_direction &= 0xf;
  Player_LimitDirections_Inner();
}

void Player_LimitDirections_Inner() {  // 87b660
  link_direction_mask_a = 0xf;
  link_direction_mask_b = 0xf;
  link_num_orthogonal_directions = 0;

  static const uint8 kMasks[4] = { 7, 0xB, 0xD, 0xE };

  if (link_direction & 0xC) {
    link_num_orthogonal_directions++;

    link_last_direction_moved_towards = link_direction & 8 ? 0 : 1;
    TileDetect_Movement_VerticalSlopes(link_last_direction_moved_towards);

    if ((R14 & 0x30) && !(tiledetect_var1 & 2) && !(((R14 & 0x30) >> 4) & link_direction) && (link_direction & 3)) {
      link_direction_mask_a = kMasks[(link_direction & 2) ? 2 : 3];
    } else {
      if (dung_hdr_collision == 0) {
        if (link_auxiliary_state != 0 && (R12 & 3))
          goto set_thingy;
      }

      if (R14 & 3) {
        link_moving_against_diag_tile = 0;
        if (link_flag_moving && (bitfield_spike_cactus_tiles & 3) == 0 && (link_direction & 3)) {
          swimcoll_var1[0] = 0;
          swimcoll_var5[0] = 0;
          swimcoll_var7[0] = 0;
          swimcoll_var9[0] = 0;
        }
set_thingy:
        fallhole_var1 = 1;
        link_direction_mask_a = kMasks[link_last_direction_moved_towards];
      }
    }

    if (link_direction & 3) {
      link_num_orthogonal_directions++;

      link_last_direction_moved_towards = link_direction & 2 ? 2 : 3;
      TileDetect_Movement_HorizontalSlopes(link_last_direction_moved_towards);

      if ((R14 & 0x30) && (tiledetect_var1 & 2) && !(((R14 & 0x30) >> 2) & link_direction) && (link_direction & 0xC)) {
        link_direction_mask_b = kMasks[(link_direction & 8) ? 0 : 1];
      } else {
        if (dung_hdr_collision == 0) {
          if (link_auxiliary_state != 0 && (R12 & 3))
            goto set_thingy_b;
        }

        if (R14 & 3) {
          link_moving_against_diag_tile = 0;
          if (link_flag_moving && (bitfield_spike_cactus_tiles & 3) == 0 && (link_direction & 0xC)) {
            swimcoll_var1[1] = 0;
            swimcoll_var5[1] = 0;
            swimcoll_var7[1] = 0;
            swimcoll_var9[1] = 0;
          }
set_thingy_b:
          fallhole_var1 = 1;
          link_direction_mask_b = kMasks[link_last_direction_moved_towards];
        }
      }

      link_direction &= link_direction_mask_a & link_direction_mask_b;
    }
  }

  // ending
  if ((link_direction & 0xf) && (link_moving_against_diag_tile & 0xf))
    link_direction = link_moving_against_diag_tile & 0xf;

  if (link_num_orthogonal_directions == 2) {
    link_num_orthogonal_directions = (link_direction_facing & 4) ? 2 : 1;
  } else {
    link_num_orthogonal_directions = 0;
  }
}

void Link_HandleCardinalCollision() {  // 87b7c7
  tiledetect_diag_state = 0;
  tiledetect_diagonal_tile = 0;

  if (((link_moving_against_diag_tile & 0x30) != 0 || (Link_HandleDiagonalKickback(), moving_against_diag_deadlocked == 0)) &&
      CheckIfRoomNeedsDoubleLayerCheck()) {

    if (dung_hdr_collision < 2 || dung_hdr_collision == 3)
      goto yx;
    tile_coll_flag = 2;
    Player_TileDetectNearby();
    byte_7E0316 = R14;
    if (byte_7E0316 == 0)
      goto yx;
    link_y_vel += dung_floor_y_vel;
    link_x_vel += dung_floor_x_vel;

    uint8 a;
    a = R14;
    if (a == 12 || a == 3)
      goto yx;
    if (a == 10 || a == 5)
      goto xy;
    if ((a & 0xc) == 0 && (a & 3) == 0)
      goto yx;

    if (link_y_vel)
      goto xy;
    if (!link_x_vel)
      goto yx;

    if (sign8(dung_floor_y_vel)) {
yx:   RunSlopeCollisionChecks_VerticalFirst();
    } else {
xy:   RunSlopeCollisionChecks_HorizontalFirst();
    }
    CreateVelocityFromMovingBackground();
  } // endif_1

  if (dung_hdr_collision == 2) {
    Player_TileDetectNearby();
    if ((R14 | byte_7E0316) == 0xf) {
      if (!countdown_for_blink)
        countdown_for_blink = 58;
      if (link_direction == 0) {
        if (BYTE(dung_floor_y_vel))
          link_y_vel = -link_y_vel;
        if (BYTE(dung_floor_x_vel))
          link_x_vel = -link_x_vel;
      }
    }
    tile_coll_flag = 1;
    RunSlopeCollisionChecks_VerticalFirst();
  } else if (dung_hdr_collision == 3) {
    tile_coll_flag = 1;
    RunSlopeCollisionChecks_HorizontalFirst();
  } else if (dung_hdr_collision == 4 || (link_x_vel | link_y_vel) != 0) {
    tile_coll_flag = 1;
    RunSlopeCollisionChecks_VerticalFirst();
  } else {
    uint8 st = link_player_handler_state;
    if (st != 19 && st != 8 && st != 9 && st != 10 && st != 3) {
      Player_TileDetectNearby();
      if (tiledetect_pit_tile & 0xf) {
        link_player_handler_state = kPlayerState_FallingIntoHole;
        if (!link_is_running)
          link_speed_setting = 4;
      }
    }
  }

  TileDetect_MainHandler(0);
  if (link_num_orthogonal_directions != 0)
    link_moving_against_diag_tile = 0;

  if (link_player_handler_state != 11) {
    link_y_vel = link_y_coord - link_y_coord_safe_return_lo;
    if (link_y_vel)
      link_direction = (link_direction & 3) | (sign8(link_y_vel) ? 8 : 4);
  }

  link_x_vel = link_x_coord - link_x_coord_safe_return_lo;
  if (link_x_vel)
    link_direction = (link_direction & 0xC) | (sign8(link_x_vel) ? 2 : 1);

  if (!player_is_indoors || dung_hdr_collision != 4 || link_player_handler_state != kPlayerState_Swimming)
    return;

  if (dung_floor_y_vel && (uint8)(link_y_vel - dung_floor_y_vel) == 0)
    link_direction &= sign8(dung_floor_y_vel) ? ~8 : ~4;

  if (dung_floor_x_vel && (uint8)(link_x_vel - dung_floor_x_vel) == 0)
    link_direction &= sign8(dung_floor_x_vel) ? ~2 : ~1;
}

void RunSlopeCollisionChecks_VerticalFirst() {  // 87b956
  if (!(link_moving_against_diag_tile & 0x20))
    StartMovementCollisionChecks_Y();
  if (!(link_moving_against_diag_tile & 0x10))
    StartMovementCollisionChecks_X();
}

void RunSlopeCollisionChecks_HorizontalFirst() {  // 87b969
  if (!(link_moving_against_diag_tile & 0x10))
    StartMovementCollisionChecks_X();
  if (!(link_moving_against_diag_tile & 0x20))
    StartMovementCollisionChecks_Y();
}

bool CheckIfRoomNeedsDoubleLayerCheck() {  // 87b97c
  if (dung_hdr_collision == 0 || dung_hdr_collision == 4)
    return false;

  if (dung_hdr_collision >= 2) {
    link_y_coord += BG1VOFS_copy2 - BG2VOFS_copy2;
    related_to_moving_floor_y = link_y_coord;
    link_x_coord += BG1HOFS_copy2 - BG2HOFS_copy2;
    related_to_moving_floor_x = link_x_coord;
  }
  link_is_on_lower_level = 1;
  return true;
}

void CreateVelocityFromMovingBackground() {  // 87b9b3
  if (dung_hdr_collision != 1) {
    uint16 x = link_x_coord - related_to_moving_floor_x;
    uint16 y = link_y_coord - related_to_moving_floor_y;
    link_y_coord += BG2VOFS_copy2 - BG1VOFS_copy2;
    link_x_coord += BG2HOFS_copy2 - BG1HOFS_copy2;
    if (link_direction) {
      link_x_vel += x;
      link_y_vel += y;
    }
  }
  link_is_on_lower_level = 0;
}

void StartMovementCollisionChecks_Y() {  // 87ba0a
  if (!link_y_vel)
    return;

  if (is_standing_in_doorway == 1)
    link_last_direction_moved_towards = (uint8)link_y_coord < 0x80 ? 0 : 1;
  else
    link_last_direction_moved_towards = sign8(link_y_vel) ? 0 : 1;
  TileDetect_Movement_Y(link_last_direction_moved_towards);
  if (player_is_indoors)
    StartMovementCollisionChecks_Y_HandleIndoors();
  else
    StartMovementCollisionChecks_Y_HandleOutdoors();
}

void StartMovementCollisionChecks_Y_HandleIndoors() {  // 87ba35
  if (sign8(link_state_bits) || link_incapacitated_timer != 0) {
    R14 |= R14 >> 4;
  } else {
    if (is_standing_in_doorway == 2) {
      if (link_num_orthogonal_directions == 0) {
        if (dung_hdr_collision != 3 || link_is_on_lower_level == 0) {
          Link_AddInVelocityY();
          ChangeAxisOfPerpendicularDoorMovement_Y();
          return;
        }
        goto label_3;
      } else if (tiledetect_var1) {
        Link_AddInVelocityY();
        goto endif_1b;
      }
    } // else_3
    if (R14 & 0x70) {
      if ((R14 >> 8) & 7) {
        force_move_any_direction = (sign8(link_y_vel)) ? 8 : 4;
      } // endif_6

      is_standing_in_doorway = 1;
      link_on_conveyor_belt = 0;
      if ((R14 & 0x70) != 0x70) {
        if (R14 & 5) { // if_7
          link_moving_against_diag_tile = 0;
          Link_AddInVelocityYFalling();
          CalculateSnapScratch_Y();
          is_standing_in_doorway = 0;

          if (R14 & 0x20 && (R14 & 1) == 0 && (link_x_coord & 7) == 1)
            link_x_coord &= ~7;
          goto else_7;
        }
        if (R14 & 0x20)
          goto else_7;
      } else { // else_7
else_7:
        if (!(tile_coll_flag & 2))
          link_cant_change_direction &= ~2;
        return;
      }
    }
  } // endif_1

  if (!(tile_coll_flag & 2)) {
    is_standing_in_doorway = 0;
  }

endif_1b:
  if (!(tile_coll_flag & 2)) {
    link_cant_change_direction &= ~2;
    room_transitioning_flags = 0;
    force_move_any_direction = 0;
  } // label_3

label_3:

  if ((R14 & 7) == 0 && (R12 & 5) != 0) {
    link_on_conveyor_belt = 0;
    FlagMovingIntoSlopes_Y();
    if ((link_moving_against_diag_tile & 0xf) != 0)
      return;
  } // endif_9

  link_moving_against_diag_tile = 0;
  if (tiledetect_key_lock_gravestones & 0x20) {
    uint16 bak = R14;
    int dummy;
    OpenChestForItem(tiledetect_tile_type, &dummy);
    tiledetect_tile_type = 0;
    R14 = bak;
  }
  if (!link_is_on_lower_level) {
    if (tiledetect_water_staircase & 7) {
      byte_7E0322 |= 1;
    } else if ((bitfield_spike_cactus_tiles & 7) == 0 && (R14 & 2) == 0) { // else_11
      byte_7E0322 &= ~1;
    } // endif_11
  } else { // else_10
    if ((tiledetect_moving_floor_tiles & 7) != 0) {
      byte_7E0322 |= 2;
    } else {
      byte_7E0322 &= ~2;
    }
  } // endif_11

  if (tiledetect_misc_tiles & 0x2200) {
    uint16 dy = tiledetect_misc_tiles & 0x2000 ? 8 : 0;

    static const uint8 kLink_DoMoveXCoord_Indoors_dx[] = { 8, 8, 0, 15 };
    static const uint8 kLink_DoMoveXCoord_Indoors_dy[] = { 8, 24, 16, 16 };

    link_rupees_goal += 5;
    uint16 y = link_y_coord + kLink_DoMoveXCoord_Indoors_dy[link_last_direction_moved_towards] - dy;
    uint16 x = link_x_coord + kLink_DoMoveXCoord_Indoors_dx[link_last_direction_moved_towards];

    Dungeon_DeleteRupeeTile(x, y);
    Ancilla_Sfx3_Near(10);
  }  // endif_12_norupee

  if (tiledetect_var4 & 0x22) {
    link_on_conveyor_belt = tiledetect_var4 & 0x20 ? 2 : 1;
  } else if (tiledetect_var4 & 0x2200) {
    link_on_conveyor_belt = tiledetect_var4 & 0x2000 ? 4 : 3;
  } else {
    if (!(bitfield_spike_cactus_tiles & 7) && !(R14 & 2))
      link_on_conveyor_belt = 0;
  } // endif_15

  if ((tiledetect_vertical_ledge & 7) == 7 && RunLedgeHopTimer()) {
    Link_CancelDash();
    about_to_jump_off_ledge++;
    link_disable_sprite_damage = 1;
    link_auxiliary_state = 2;
    Ancilla_Sfx2_Near(0x20);

    goto endif_19;
  } else if ((tiledetect_deepwater & 7) == 7 && link_is_in_deep_water == 0) {
    // if_20
    Link_CancelDash();
    if (TS_copy == 0) {
      Dungeon_HandleLayerChange();
    } else {
      link_is_in_deep_water = 1;
      link_some_direction_bits = link_direction_last;
      link_state_bits = 0;
      link_picking_throw_state = 0;
      link_grabbing_wall = 0;
      link_speed_setting = 0;
      Link_ResetSwimmingState();
      Ancilla_Sfx2_Near(0x20);
    }
endif_19:
    link_disable_sprite_damage = 1;
    Link_HopInOrOutOfWater_Y();
  } else {
    // else_20
    if ((tiledetect_normal_tiles & 2) && link_is_in_deep_water != 0) {
      if (link_auxiliary_state != 0) {
        R14 = 7;
      } else {
        Link_CancelDash();
        link_direction_last = link_some_direction_bits;
        link_is_in_deep_water = 0;
        if (AncillaAdd_Splash(0x15, 0)) {
          link_is_in_deep_water = 1;
          R14 = 7;
        } else {
          link_disable_sprite_damage = 1;
          Link_HopInOrOutOfWater_Y();
        }
      }
    }
  } // endif_21

  if ((tiledetect_stair_tile & 7) == 7) {
    if (link_incapacitated_timer) {
      R14 &= ~0xff;
      R14 |= tiledetect_stair_tile & 7;
      HandlePushingBonkingSnaps_Y();
      return;
    }
    if (tiledetect_inroom_staircase & 0x77) {
      submodule_index = tiledetect_inroom_staircase & 0x70 ? 16 : 8;
      main_module_index = 7;
      Link_CancelDash();
    } else if (enhanced_features0 & kFeatures0_TurnWhileDashing) {
      // avoid weirdness in stairs
      Link_CancelDash();
    }

    if ((link_last_direction_moved_towards & 2) == 0) {
      link_speed_setting = 2;
      link_speed_modifier = 1;
      return;
    }
  }

  if (link_speed_setting == 2)
    link_speed_setting = link_is_running ? 16 : 0;

  if (link_speed_modifier == 1)
    link_speed_modifier = 2;

  if (tiledetect_pit_tile & 5 && (R14 & 2) == 0) {
    if (link_player_handler_state == 5 || link_player_handler_state == 2)
      return;
    byte_7E005C = 9;
    link_this_controls_sprite_oam = 0;
    player_near_pit_state = 1;
    link_player_handler_state = kPlayerState_FallingIntoHole;
    return;
  } // endif_23

  link_this_controls_sprite_oam = 0;

  if (bitfield_spike_cactus_tiles & 7) {
    if ((link_incapacitated_timer | countdown_for_blink | link_cape_mode) == 0) {
      if (((link_last_direction_moved_towards == 0) ? (link_y_coord & 4) == 0 : ((link_y_coord & 4) != 0)) && (countdown_for_blink == 0)) {
        link_give_damage = 8;
        Link_CancelDash();
        Link_ForceUnequipCape_quietly();
        LinkApplyTileRebound();
        return;
      }
    } else {
      R14 &= ~0xFF;
      R14 |= bitfield_spike_cactus_tiles & 7;
    } // endif_24
  } // endif_24
  if (dung_hdr_collision == 0 || dung_hdr_collision == 4 || !link_is_on_lower_level) {
    if (tiledetect_var2 && link_num_orthogonal_directions == 0) {
      byte_7E02C2 = tiledetect_var2;
      if (!sign8(--gravestone_push_timeout))
        goto endif_26;
      uint16 bits = tiledetect_var2;
      int i = 15;
      do {
        if (bits & 0x8000) {
          uint8 idx = FindFreeMovingBlockSlot(i);
          if (idx == 0xff)
            continue;
          R14 = idx;
          if (InitializePushBlock(idx, i * 2))
            continue;
          Sprite_Dungeon_DrawSinglePushBlock(idx * 2);
          R14 = 4;  // Unwanted side effect
          pushedblock_facing[idx] = link_last_direction_moved_towards * 2;
          push_block_direction = link_last_direction_moved_towards * 2;
          pushedblocks_target[idx] = (pushedblocks_y_lo[idx] - (link_last_direction_moved_towards == 1)) & 0xf;
        }
      } while (bits <<= 1, --i >= 0);
    }
    // endif_27
    gravestone_push_timeout = 21;
  }
  // endif_26
endif_26:
  HandlePushingBonkingSnaps_Y();
}

void HandlePushingBonkingSnaps_Y() {  // 87bdb1
  if (R14 & 7) {
    if (link_player_handler_state == kPlayerState_Swimming) {
      if ((uint8)dung_floor_y_vel == 0)
        ResetAllAcceleration();

      if (link_num_orthogonal_directions != 0) {
        Link_AddInVelocityYFalling();
        goto label_a;
      }
    }  // endif_2

    if (R14 & 2 || (R14 & 5) == 5) {
      uint16 bak = R14;
      Link_BonkAndSmash();
      RepelDash();
      R14 = bak;
    }

    fallhole_var1 = 1;

    if ((R14 & 2) == 2) {
      Link_AddInVelocityYFalling();
    } else {
      if (link_num_orthogonal_directions == 1)
        goto returnb;
      Link_AddInVelocityYFalling();
      if (link_num_orthogonal_directions == 2)
        goto returnb;
    } // endif_4

label_a:

    if ((R14 & 5) == 5) {
      Link_BonkAndSmash();
      RepelDash();
    } else if (R14 & 4) {
      uint8 tt = sign8(link_y_vel) ? link_y_vel : -link_y_vel;
      uint8 r0 = sign8(tt) ? 0xff : 1;
      if ((R14 & 2) == 0) {
        if (link_x_coord & 7) {
          link_x_coord += (int8)r0;
          HandleNudging(r0);
          return;
        }
        Link_BonkAndSmash();
        RepelDash();
      }
    } else { // else_7
      uint8 tt = sign8(link_y_vel) ? -link_y_vel : link_y_vel;
      uint8 r0 = sign8(tt) ? 0xff : 1;
      if ((R14 & 2) == 0) {
        if (link_x_coord & 7) {
          link_x_coord += (int8)r0;
          HandleNudging(r0);
          return;
        }
        Link_BonkAndSmash();
        RepelDash();
      }
    }
    // endif_10
    if (link_last_direction_moved_towards * 2 == link_direction_facing) {
      bitmask_of_dragstate |= (tile_coll_flag & 1) << 1;
      if (button_b_frames == 0 && !sign8(--link_timer_push_get_tired))
        return;

      bitmask_of_dragstate |= (tiledetect_misc_tiles & 0x20) ? tile_coll_flag << 3 : tile_coll_flag;
    }
  } else {// else_1
    if (link_is_on_lower_level)
      return;
    bitmask_of_dragstate &= ~9;
  } // endif_1

returnb:
  link_timer_push_get_tired = 32;
  bitmask_of_dragstate &= ~2;
}

void StartMovementCollisionChecks_Y_HandleOutdoors() {  // 87beaf
  if (link_speed_setting == 2)
    link_speed_setting = link_is_running ? 16 : 0;

  if ((tiledetect_pit_tile & 5) != 0 && (R14 & 2) == 0) {
    if (link_player_handler_state != 5 && link_player_handler_state != 2) {
      // start fall into hole
      byte_7E005C = 9;
      link_this_controls_sprite_oam = 0;
      player_near_pit_state = 1;
      link_player_handler_state = kPlayerState_FallingIntoHole;
    }
    return;
  }

  if (tiledetect_read_something & 2) {
    interacting_with_liftable_tile_x1 = interacting_with_liftable_tile_x2 >> 1;
  } else {
    interacting_with_liftable_tile_x1 = 0;
  }  // endif_2

  if ((tiledetect_deepwater & 2) && !link_is_in_deep_water && !link_auxiliary_state) {
    Link_ResetSwordAndItemUsage();
    Link_CancelDash();
    link_is_in_deep_water = 1;
    link_some_direction_bits = link_direction_last;
    link_grabbing_wall = 0;
    link_speed_setting = 0;
    Link_ResetSwimmingState();
    if ((draw_water_ripples_or_grass == 1) && (Link_ForceUnequipCape_quietly(), link_item_flippers != 0)) {
      if (!link_is_bunny_mirror)
        link_player_handler_state = kPlayerState_Swimming;
    } else {
      Ancilla_Sfx2_Near(0x20);
      link_y_coord = (link_y_coord_safe_return_hi << 8) | link_y_coord_safe_return_lo;
      link_x_coord = (link_x_coord_safe_return_hi << 8) | link_x_coord_safe_return_lo;
      link_disable_sprite_damage = 1;
      Link_HopInOrOutOfWater_Y();
    }
  }  // endif_afterSwimCheck

  if (link_is_in_deep_water) {
    if (tiledetect_vertical_ledge & 7) {
      R14 = tiledetect_vertical_ledge & 7;
      HandlePushingBonkingSnaps_Y();
      return;
    }
    if ((tiledetect_stair_tile & 7) == 7 || (tiledetect_normal_tiles & 7) == 7) {
      Link_CancelDash();
      link_is_in_deep_water = 0;
      if (link_auxiliary_state == 0) {
        link_direction_last = link_some_direction_bits;
        link_disable_sprite_damage = 1;
        AncillaAdd_Splash(0x15, 0);
        Link_HopInOrOutOfWater_Y();
        return;
      }
    }
  }

  if (detection_of_ledge_tiles_horiz_uphoriz & 2 || detection_of_unknown_tile_types & 0x22) {
    R14 = 7;
    HandlePushingBonkingSnaps_Y();
    return;
  }

  if (tiledetect_vertical_ledge & 0x70 && RunLedgeHopTimer()) {
    Link_CancelDash();
    link_disable_sprite_damage = 1;
    allow_scroll_z = 1;
    link_player_handler_state = 11;
    link_incapacitated_timer = 0;
    link_z_coord_mirror = -1;
    bitmask_of_dragstate = 0;
    link_speed_setting = 0;
    link_actual_vel_z_copy_mirror = link_actual_vel_z_mirror = link_is_in_deep_water ? 14 : 20;
    link_auxiliary_state = link_is_in_deep_water ? 4 : 2;
    return;
  }

  if (tiledetect_vertical_ledge & 7 && RunLedgeHopTimer()) {
    Ancilla_Sfx2_Near(0x20);
    link_disable_sprite_damage = 1;
    Link_CancelDash();
    bitmask_of_dragstate = 0;
    link_speed_setting = 0;
    Link_FindValidLandingTile_North();
    return;
  }

  if (!link_is_in_deep_water) {
    if (tiledetect_ledges_down_leftright & 7 && !(tiledetect_vertical_ledge & 0x77)) {
      uint8 xand = index_of_interacting_tile == 0x2f ? 4 : 1;
      if ((tiledetect_ledges_down_leftright & xand) && RunLedgeHopTimer()) {
        Link_CancelDash();
        link_actual_vel_x = tiledetect_ledges_down_leftright & 4 ? 16 : -16;
        link_disable_sprite_damage = 1;
        bitmask_of_dragstate = 0;
        link_speed_setting = 0;
        allow_scroll_z = 1;
        link_auxiliary_state = 2;
        link_actual_vel_z_copy_mirror = link_actual_vel_z_mirror = 20;
        link_z_coord_mirror |= 0xff;
        link_incapacitated_timer = 0;
        link_player_handler_state = 14;
        return;
      }
    } // endif_6

    if (detection_of_ledge_tiles_horiz_uphoriz & 0x70 && !(tiledetect_vertical_ledge & 0x77) && RunLedgeHopTimer()) {
      Link_CancelDash();
      Ancilla_Sfx2_Near(0x20);
      link_last_direction_moved_towards = detection_of_ledge_tiles_horiz_uphoriz & 0x40 ? 3 : 2;
      link_disable_sprite_damage = 1;
      bitmask_of_dragstate = 0;
      link_speed_setting = 0;
      Link_FindValidLandingTile_DiagonalNorth();
      return;
    }
  } // endif_7

  if ((tiledetect_stair_tile & 7) == 7) {
    if (link_incapacitated_timer != 0) {
      R14 = tiledetect_stair_tile & 7;
      HandlePushingBonkingSnaps_Y();
      return;
    } else if (!(link_last_direction_moved_towards & 2)) {
      link_speed_setting = 2;
      link_speed_modifier = 1;
      return;
    }
  }  // endif_8

  if (link_speed_setting == 2)
    link_speed_setting = link_is_running ? 16 : 0;

  if (link_speed_modifier == 1)
    link_speed_modifier = 2;

  if ((R14 & 7) == 0 && (R12 & 5) != 0) {
    FlagMovingIntoSlopes_Y();
    if ((link_moving_against_diag_tile & 0xf) != 0)
      return;
  }  // endif_11

  link_moving_against_diag_tile = 0;
  if (tiledetect_key_lock_gravestones & 2 && link_last_direction_moved_towards == 0) {
    if (link_is_running || sign8(--gravestone_push_timeout)) {
      uint16 bak = R14;
      AncillaAdd_GraveStone(0x24, 4);
      R14 = bak;
      gravestone_push_timeout = 52;
    }
  } else {
    gravestone_push_timeout = 52;
  } // endif_12

  if ((bitfield_spike_cactus_tiles & 7) != 0) {
    if ((link_incapacitated_timer | countdown_for_blink | link_cape_mode) == 0) {
      if (link_last_direction_moved_towards == 0 ? ((link_y_coord & 4) == 0) : ((link_y_coord & 4) != 0)) {
        link_give_damage = 8;
        Link_CancelDash();
        Link_ForceUnequipCape_quietly();
        LinkApplyTileRebound();
        return;
      }
    } else {
      R14 = bitfield_spike_cactus_tiles & 7;
    }
  }  // endif_13
  HandlePushingBonkingSnaps_Y();
}

bool RunLedgeHopTimer() { // carry  // 87c16d
  bool rv = false;
  if (link_auxiliary_state != 1) {
    if (!link_is_running) {
      if (sign8(--link_timer_jump_ledge)) {
        link_timer_jump_ledge = 19;
        return true;
      }
    } else {
      rv = true;
    }
  }
  link_y_coord = link_y_coord_prev;
  link_x_coord = link_x_coord_prev;
  link_subpixel_y = link_subpixel_x = 0;
  return rv;
}

void Link_BonkAndSmash() {  // 87c1a1
  if (!link_is_running || (link_dash_ctr == 64) || !(bitmask_for_dashable_tiles & 0x70))
    return;
  for (int i = 0; i < 2; i++) {
    Point16U pt;
    int j = Overworld_SmashRockPile(i != 0, &pt);
    if (j >= 0) {
      int k = FindInByteArray(kLink_Lift_tab, (uint8)j, 9);
      if (k >= 0) {
        if (k == 2 || k == 4)
          Ancilla_Sfx3_Near(0x32);
        Sprite_SpawnImmediatelySmashedTerrain(k, pt.x, pt.y);
      }
    }
  }
}

void Link_AddInVelocityYFalling() {  // 87c1e4
  link_y_coord -= (tiledetect_which_y_pos[0] & 7) - (sign8(link_y_vel) ? 8 : 0);
}

// Adjust X coord to fit through door
void CalculateSnapScratch_Y() {  // 87c1ff
  uint8 yv = link_y_vel;
  if (R14 & 4) {
    if (!sign8(yv)) yv = -yv;
  } else {
    if (sign8(yv)) yv = -yv;
  }
  link_x_coord += !sign8(yv) ? 1 : -1;
}

void ChangeAxisOfPerpendicularDoorMovement_Y() {  // 87c23d
  link_cant_change_direction |= 2;
  uint8 t = (R14 | (R14 >> 4)) & 0xf;
  if (!(t & 7)) {
    is_standing_in_doorway = 0;
    return;
  }
  int8 vel;
  uint8 dir;

  if ((uint8)link_x_coord >= 0x80) {
    uint8 t = link_y_vel;
    if (!sign8(t)) t = -t;
    vel = sign8(t) ? -1 : 1;
    dir = 4;
  } else {
    uint8 t = link_y_vel;
    if (sign8(t)) t = -t;
    vel = sign8(t) ? -1 : 1;
    dir = 6;
  }
  if (!(link_cant_change_direction & 1))
    link_direction_facing = dir;
  link_x_coord += vel;
}

void Link_AddInVelocityY() {  // 87c29f
  link_y_coord -= (int8)link_y_vel;
}

void Link_HopInOrOutOfWater_Y() {  // 87c2c3
  static const uint8 kRecoilVelY[] = { 24, 16, 16 };
  static const uint8 kRecoilVelZ[] = { 36, 24, 24 };

  uint8 ts = !player_is_indoors ? 2 :
    about_to_jump_off_ledge ? 0 : TS_copy;

  int8 vel = kRecoilVelY[ts];
  if (!link_last_direction_moved_towards)
    vel = -vel;

  link_actual_vel_y = vel;
  link_actual_vel_x = 0;
  link_actual_vel_z_copy = link_actual_vel_z = kRecoilVelZ[ts];
  link_z_coord = 0;
  link_incapacitated_timer = 16;
  if (link_auxiliary_state != 2) {
    link_auxiliary_state = 1;
    link_electrocute_on_touch = 0;
  }
  link_player_handler_state = 6;
}

void Link_FindValidLandingTile_North() {  // 87c36c
  uint16 y_coord_bak = link_y_coord;
  link_y_coord_original = link_y_coord;

  for (;;) {
    link_y_coord -= 16;
    TileDetect_Movement_Y(link_last_direction_moved_towards);
    uint8 k = tiledetect_normal_tiles | tiledetect_destruction_aftermath | tiledetect_thick_grass | tiledetect_deepwater;
    if ((k & 7) == 7)
      break;
  }

  if (tiledetect_deepwater & 7) {
    link_auxiliary_state = 1;
    link_electrocute_on_touch = 0;
    link_is_in_deep_water = 1;
    link_some_direction_bits = link_direction_last;
    Link_ResetSwimmingState();
    link_grabbing_wall = 0;
    link_speed_setting = 0;
  }

  link_y_coord -= 16;
  link_y_coord_original -= link_y_coord;
  link_y_coord = y_coord_bak;

  uint8 o = (uint8)link_y_coord_original >> 3;

  static const uint8 kLink_MoveY_RecoilOther_dy[32] = { 16, 16, 20, 20, 24, 24, 28, 28, 32, 32, 36, 36, 40, 40, 44, 44, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48 };
  static const uint8 kLink_MoveY_RecoilOther_dz[32] = { 24, 24, 24, 24, 28, 28, 28, 28, 32, 32, 32, 32, 36, 36, 36, 36, 40, 40, 40, 40, 44, 44, 44, 44, 48, 48, 48, 48, 52, 52, 52, 52 };
  static const uint8 kLink_MoveY_RecoilOther_timer[32] = { 16, 16, 20, 20, 24, 24, 28, 28, 32, 32, 36, 36, 40, 40, 44, 44, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48 };

  int8 dy = kLink_MoveY_RecoilOther_dy[o];
  link_actual_vel_y = (link_last_direction_moved_towards != 0) ? dy : -dy;
  link_actual_vel_x = 0;
  link_actual_vel_z_copy = link_actual_vel_z = kLink_MoveY_RecoilOther_dz[o];
  link_z_coord = 0;
  link_incapacitated_timer = kLink_MoveY_RecoilOther_timer[o];
  link_auxiliary_state = 2;
  link_electrocute_on_touch = 0;
  link_player_handler_state = 6;
}

void Link_FindValidLandingTile_DiagonalNorth() {  // 87c46d
  uint8 b0 = link_y_coord_safe_return_lo;
  uint16 b1 = link_x_coord;
  uint8 dir = link_last_direction_moved_towards;

  link_actual_vel_x = (link_last_direction_moved_towards != 2 ? 1 : -1);
  link_last_direction_moved_towards = 0;
  LinkHop_FindLandingSpotDiagonallyDown();

  link_x_coord = b1;
  link_y_coord_safe_return_lo = b0;

  uint16 o = (uint16)(link_y_coord_original - link_y_coord) >> 3;
  link_y_coord = link_y_coord_original;

  static const uint8 kLink_JumpOffLedgeUpDown_dx[32] = { 8, 8, 8, 8, 16, 16, 16, 16, 24, 24, 24, 24, 16, 16, 16, 16, 8, 20, 20, 20, 24, 24, 24, 24, 28, 28, 28, 28, 32, 32, 32, 32 };
  static const uint8 kLink_JumpOffLedgeUpDown_dy[32] = { 8, 8, 8, 8, 16, 16, 20, 20, 24, 24, 24, 24, 32, 32, 32, 32, 8, 20, 20, 20, 24, 24, 24, 24, 28, 28, 28, 28, 32, 32, 32, 32 };
  static const uint8 kLink_JumpOffLedgeUpDown_dz[32] = { 32, 32, 32, 32, 32, 32, 32, 32, 36, 36, 36, 36, 40, 40, 40, 40, 32, 40, 40, 40, 44, 44, 44, 44, 48, 48, 48, 48, 52, 52, 52, 52 };

  link_actual_vel_y = -kLink_JumpOffLedgeUpDown_dy[o];
  uint8 dx = kLink_JumpOffLedgeUpDown_dx[o];
  link_actual_vel_x = (dir != 2) ? dx : -dx;
  link_actual_vel_z_copy = link_actual_vel_z = kLink_JumpOffLedgeUpDown_dz[o];
  link_z_coord = 0;
  link_z_coord_mirror &= ~0xff;
  link_auxiliary_state = 2;
  link_electrocute_on_touch = 0;
  link_player_handler_state = 13;
}

void StartMovementCollisionChecks_X() {  // 87c4d4
  if (!link_x_vel)
    return;

  if (is_standing_in_doorway == 2)
    link_last_direction_moved_towards = (uint8)link_x_coord < 0x80 ? 2 : 3;
  else
    link_last_direction_moved_towards = sign8(link_x_vel) ? 2 : 3;
  TileDetect_Movement_X(link_last_direction_moved_towards);
  if (player_is_indoors)
    StartMovementCollisionChecks_X_HandleIndoors();
  else
    StartMovementCollisionChecks_X_HandleOutdoors();
}

void StartMovementCollisionChecks_X_HandleIndoors() {  // 87c4ff
  if (sign8(link_state_bits) || link_incapacitated_timer != 0) {
    R14 |= R14 >> 4;
  } else {
    if (link_num_orthogonal_directions == 0)
      link_speed_modifier = 0;
    if (is_standing_in_doorway == 1 && link_num_orthogonal_directions == 0) {
      if (dung_hdr_collision != 3 || link_is_on_lower_level == 0) {
        SnapOnX();
        int8 spd = ChangeAxisOfPerpendicularDoorMovement_X();
        HandleNudgingInADoor(spd);
        return;
      }
      goto label_3;
    } // else_3

    if (R14 & 0x70) {
      if ((R14 >> 8) & 7) {
        force_move_any_direction = (sign8(link_x_vel)) ? 2 : 1;
      } // endif_6

      is_standing_in_doorway = 2;
      link_on_conveyor_belt = 0;
      if ((R14 & 0x70) != 0x70) {
        if (R14 & 7) { // if_7
          link_moving_against_diag_tile = 0;
          is_standing_in_doorway = 0;
          SnapOnX();
          CalculateSnapScratch_X();
          return;
        }
        if (R14 & 0x70)
          goto else_7;
      } else { // else_7
else_7:
        if (!(tile_coll_flag & 2))
          link_cant_change_direction &= ~2;
        return;
      }
    }
  } // endif_1

  if (!(tile_coll_flag & 2)) {
    link_cant_change_direction &= ~2;
    is_standing_in_doorway = 0;
    room_transitioning_flags = 0;
    force_move_any_direction = 0;
  }  // label_3

label_3:

  if ((R14 & 2) == 0 && (R12 & 5) != 0) {
    link_on_conveyor_belt = 0;
    FlagMovingIntoSlopes_X();
    if ((link_moving_against_diag_tile & 0xf) != 0)
      return;
  } // endif_9

  link_moving_against_diag_tile = 0;
  if (!link_is_on_lower_level) {
    if (tiledetect_water_staircase & 7) {
      byte_7E0322 |= 1;
    } else if ((bitfield_spike_cactus_tiles & 7) == 0 && (R14 & 2) == 0) { // else_11
      byte_7E0322 &= ~1;
    } // endif_11
  } else { // else_10
    if ((tiledetect_moving_floor_tiles & 7) != 0) {
      byte_7E0322 |= 2;
    } else {
      byte_7E0322 &= ~2;
    }
  } // endif_11

  if (tiledetect_misc_tiles & 0x2200) {
    uint16 dy = tiledetect_misc_tiles & 0x2000 ? 8 : 0;

    static const uint8 kLink_DoMoveXCoord_Indoors_dx[] = { 8, 8, 0, 15 };
    static const uint8 kLink_DoMoveXCoord_Indoors_dy[] = { 8, 24, 16, 16 };

    link_rupees_goal += 5;
    uint16 y = link_y_coord + kLink_DoMoveXCoord_Indoors_dy[link_last_direction_moved_towards] - dy;
    uint16 x = link_x_coord + kLink_DoMoveXCoord_Indoors_dx[link_last_direction_moved_towards];

    Dungeon_DeleteRupeeTile(x, y);
    Ancilla_Sfx3_Near(10);
  }  // endif_12_norupee

  if (tiledetect_var4 & 0x22) {
    link_on_conveyor_belt = tiledetect_var4 & 0x20 ? 2 : 1;
  } else if (tiledetect_var4 & 0x2200) {
    link_on_conveyor_belt = tiledetect_var4 & 0x2000 ? 4 : 3;
  } else {
    if (!(bitfield_spike_cactus_tiles & 7) && !(R14 & 2))
      link_on_conveyor_belt = 0;
  } // endif_15

  if ((detection_of_ledge_tiles_horiz_uphoriz & 7) == 7 && RunLedgeHopTimer()) {
    Link_CancelDash();
    about_to_jump_off_ledge++;
    link_auxiliary_state = 2;
    goto endif_19;
  } else if ((tiledetect_deepwater & 7) == 7 && link_is_in_deep_water == 0 && link_player_handler_state != 6) {
    // if_20
    link_y_coord = link_y_coord_safe_return_lo | link_y_coord_safe_return_hi << 8;
    link_x_coord = link_x_coord_safe_return_lo | link_x_coord_safe_return_hi << 8;
    Link_CancelDash();
    if (TS_copy == 0) {
      Dungeon_HandleLayerChange();
    } else {
      link_is_in_deep_water = 1;
      link_some_direction_bits = link_direction_last;
      link_state_bits = 0;
      link_picking_throw_state = 0;
      link_grabbing_wall = 0;
      link_speed_setting = 0;
      Link_ResetSwimmingState();
    }
endif_19:
    link_disable_sprite_damage = 1;
    Link_HopInOrOutOfWater_X();
    Ancilla_Sfx2_Near(0x20);
  } else {
    // else_20
    if ((tiledetect_normal_tiles & 7) == 7 && link_is_in_deep_water != 0) {
      if (link_auxiliary_state != 0) {
        R14 = 7;
      } else {
        Link_CancelDash();
        if (link_auxiliary_state == 0) {
          link_direction_last = link_some_direction_bits;
          link_is_in_deep_water = 0;
          AncillaAdd_Splash(0x15, 0);
          link_disable_sprite_damage = 1;
          Link_HopInOrOutOfWater_X();
        }
      }
    }
  } // endif_21

  if (tiledetect_pit_tile & 5 && (R14 & 2) == 0) {
    if (link_player_handler_state == 5 || link_player_handler_state == 2)
      return;
    byte_7E005C = 9;
    link_this_controls_sprite_oam = 0;
    player_near_pit_state = 1;
    link_player_handler_state = kPlayerState_FallingIntoHole;
    return;
  } // endif_23

  player_near_pit_state = 0;

  if (bitfield_spike_cactus_tiles & 7) {
    if ((link_incapacitated_timer | countdown_for_blink | link_cape_mode) == 0) {
      if (((link_last_direction_moved_towards == 2) ? (link_x_coord & 4) == 0 : ((link_x_coord & 4) != 0)) && (countdown_for_blink == 0)) {
        link_give_damage = 8;
        Link_CancelDash();
        Link_ForceUnequipCape_quietly();
        LinkApplyTileRebound();
        return;
      }
    } else {
      R14 &= ~0xFF;
      R14 |= bitfield_spike_cactus_tiles & 7;
    } // endif_24
  } // endif_24
  if (dung_hdr_collision == 0 || dung_hdr_collision == 4 || !link_is_on_lower_level) {
    if (tiledetect_var2 && link_num_orthogonal_directions == 0) {
      byte_7E02C2 = tiledetect_var2;
      if (!sign8(--gravestone_push_timeout))
        goto endif_26;
      uint16 bits = tiledetect_var2;
      int i = 15;
      do {
        if (bits & 0x8000) {
          uint8 idx = FindFreeMovingBlockSlot(i);
          if (idx == 0xff)
            continue;
          R14 = idx;  // This seems like it's overwriting the tiledetector's stuff
          if (InitializePushBlock(idx, i * 2))
            continue;
          Sprite_Dungeon_DrawSinglePushBlock(idx * 2);
          R14 = 4;
          pushedblock_facing[idx] = link_last_direction_moved_towards * 2;
          push_block_direction = link_last_direction_moved_towards * 2;
          pushedblocks_target[idx] = (pushedblocks_x_lo[idx] - (link_last_direction_moved_towards != 2)) & 0xf;
        }
      } while (bits <<= 1, --i >= 0);
    }
    // endif_27
    gravestone_push_timeout = 21;
  }
  // endif_26
endif_26:
  if (link_num_orthogonal_directions == 0) {
    link_speed_modifier = 0;
    if (link_speed_setting == 2)
      link_speed_setting = 0;
  }
  HandlePushingBonkingSnaps_X();
}

void HandlePushingBonkingSnaps_X() {  // 87c7fc
  if (R14 & 7) {
    if (link_player_handler_state == kPlayerState_Swimming && (uint8)dung_floor_x_vel == 0)
      ResetAllAcceleration();

    if (R14 & 2) {
      uint16 bak = R14;
      Link_BonkAndSmash();
      RepelDash();
      R14 = bak;
    }

    fallhole_var1 = 1;

    if ((R14 & 7) == 7) {
      SnapOnX();
    } else {
      if (link_num_orthogonal_directions == 2)
        goto returnb;
      SnapOnX();
      if (link_num_orthogonal_directions == 1)
        goto returnb;
    } // endif_4

    if ((R14 & 5) == 5) {
      Link_BonkAndSmash();
      RepelDash();
    } else if (R14 & 4) {
      uint8 tt = sign8(link_x_vel) ? link_x_vel : -link_x_vel;
      uint8 r0 = sign8(tt) ? 0xff : 1;
      if ((R14 & 2) == 0) {
        if (link_y_coord & 7) {
          link_y_coord += (int8)r0;
          HandleNudging(r0);
          return;
        }
        Link_BonkAndSmash();
        RepelDash();
      }
    } else { // else_7
      uint8 tt = sign8(link_x_vel) ? -link_x_vel : link_x_vel;
      uint8 r0 = sign8(tt) ? 0xff : 1;
      if ((R14 & 2) == 0) {
        if (link_y_coord & 7) {
          link_y_coord += (int8)r0;
          HandleNudging(r0);
          return;
        }
        Link_BonkAndSmash();
        RepelDash();
      }
    }
    // endif_10
    if (link_last_direction_moved_towards * 2 == link_direction_facing) {
      bitmask_of_dragstate |= (tile_coll_flag & 1) << 1;
      if (button_b_frames == 0 && !sign8(--link_timer_push_get_tired))
        return;

      bitmask_of_dragstate |= (tiledetect_misc_tiles & 0x20) ? tile_coll_flag << 3 : tile_coll_flag;
    }
  } else {// else_1
    if (link_is_on_lower_level)
      return;
    bitmask_of_dragstate &= ~9;
  } // endif_1

returnb:
  link_timer_push_get_tired = 32;
  bitmask_of_dragstate &= ~2;
}

void StartMovementCollisionChecks_X_HandleOutdoors() {  // 87c8e9
  if (link_num_orthogonal_directions == 0) {
    link_speed_modifier = 0;
    if (link_speed_setting == 2)
      link_speed_setting = 0;
  }

  if ((tiledetect_pit_tile & 5) != 0 && (R14 & 2) == 0) {
    if (link_player_handler_state != 5 && link_player_handler_state != 2) {
      // start fall into hole
      byte_7E005C = 9;
      link_this_controls_sprite_oam = 0;
      player_near_pit_state = 1;
      link_player_handler_state = kPlayerState_FallingIntoHole;
    }
    return;
  }

  if (tiledetect_read_something & 2) {
    interacting_with_liftable_tile_x1b = interacting_with_liftable_tile_x2 >> 1;
  } else {
    interacting_with_liftable_tile_x1b = 0;
  }  // endif_2

  if ((tiledetect_deepwater & 4) && !link_is_in_deep_water && !link_auxiliary_state) {
    Link_CancelDash();
    Link_ResetSwordAndItemUsage();
    link_is_in_deep_water = 1;
    link_some_direction_bits = link_direction_last;
    Link_ResetSwimmingState();
    link_grabbing_wall = 0;
    link_speed_setting = 0;
    if ((draw_water_ripples_or_grass == 1) && (Link_ForceUnequipCape_quietly(), link_item_flippers != 0)) {
      if (!link_is_bunny_mirror)
        link_player_handler_state = kPlayerState_Swimming;
    } else {
      link_y_coord = (link_y_coord_safe_return_hi << 8) | link_y_coord_safe_return_lo;
      link_x_coord = (link_x_coord_safe_return_hi << 8) | link_x_coord_safe_return_lo;
      link_disable_sprite_damage = 1;
      Link_HopInOrOutOfWater_X();
      Ancilla_Sfx2_Near(0x20);
    }
  }  // endif_afterSwimCheck

  if (link_is_in_deep_water ? ((detection_of_ledge_tiles_horiz_uphoriz & 7) == 7) : (tiledetect_vertical_ledge & 0x42)) {
    // not implemented, jumps to another routine
    R14 = 7;
    HandlePushingBonkingSnaps_X();
    return;
  } // endif_3

  if ((tiledetect_normal_tiles & 7) == 7 && link_is_in_deep_water) {
    Link_CancelDash();
    if (!link_auxiliary_state) {
      link_direction_last = link_some_direction_bits;
      link_is_in_deep_water = 0;
      AncillaAdd_Splash(0x15, 0);
      link_disable_sprite_damage = 1;
      Link_HopInOrOutOfWater_X();
      return;
    }
  }  // endif_4

  if ((detection_of_ledge_tiles_horiz_uphoriz & 7) != 0 && RunLedgeHopTimer()) {
    Ancilla_Sfx2_Near(0x20);
    link_actual_vel_x = (link_last_direction_moved_towards & 1) ? 0x10 : -0x10;
    Link_CancelDash();
    link_auxiliary_state = 2;
    link_actual_vel_z_mirror = link_actual_vel_z_copy_mirror = 20;
    link_z_coord_mirror |= 0xff;
    link_player_handler_state = kPlayerState_FallOfLeftRightLedge;
    link_disable_sprite_damage = 1;
    allow_scroll_z = 1;
    bitmask_of_dragstate = 0;
    link_speed_setting = 0;
    if (!player_is_indoors)
      link_is_on_lower_level = 2;

    uint16 xbak = link_x_coord;
    uint8 rv = Link_HoppingHorizontally_FindTile_X((link_last_direction_moved_towards & ~2) * 2);
    link_last_direction_moved_towards = 1;
    if (rv != 0xff) {
      Link_HoppingHorizontally_FindTile_Y();
    } else {
      LinkHop_FindTileToLandOnSouth();
    }
    link_x_coord = xbak;
    return;
  }  // endif_5

  if ((detection_of_unknown_tile_types & 0x77) != 0 && RunLedgeHopTimer()) {
    uint8 sfx = Ancilla_Sfx2_Near(0x20);
    link_player_handler_state = (sfx & 7) == 0 ? 16 : 15;
    link_actual_vel_x = (link_last_direction_moved_towards & 1) ? 0x10 : -0x10;
    Link_CancelDash();
    link_auxiliary_state = 2;
    link_actual_vel_z_mirror = link_actual_vel_z_copy_mirror = 20;
    link_z_coord_mirror |= 0xff;
    link_incapacitated_timer = 0;
    link_disable_sprite_damage = 1;
    allow_scroll_z = 1;
    bitmask_of_dragstate = 0;
    link_speed_setting = 0;
    return;
  }  // endif_6

  if ((detection_of_ledge_tiles_horiz_uphoriz & 0x70) != 0 &&
      (detection_of_ledge_tiles_horiz_uphoriz & 0x7) == 0 &&
      (detection_of_unknown_tile_types & 0x77) == 0 &&
      link_player_handler_state != 13 && RunLedgeHopTimer()) {
    Ancilla_Sfx2_Near(0x20);
    Link_CancelDash();
    link_disable_sprite_damage = 1;
    bitmask_of_dragstate = 0;
    link_speed_setting = 0;
    Link_FindValidLandingTile_DiagonalNorth();
    return;
  }  // endif_7

  if ((tiledetect_ledges_down_leftright & 7) != 0 && (detection_of_ledge_tiles_horiz_uphoriz & 7) == 0 &&
      (detection_of_unknown_tile_types & 0x77) == 0 && RunLedgeHopTimer()) {
    link_actual_vel_x = (link_last_direction_moved_towards & 1) ? 0x10 : -0x10;
    Link_CancelDash();
    link_auxiliary_state = 2;
    link_actual_vel_z_mirror = link_actual_vel_z_copy_mirror = 20;
    link_z_coord_mirror |= 0xff;
    link_player_handler_state = 14;
    link_incapacitated_timer = 0;
    link_disable_sprite_damage = 1;
    allow_scroll_z = 1;
    bitmask_of_dragstate = 0;
    link_speed_setting = 0;
    return;
  } // endif_8

  // If force facing down (hold B button), while turboing on the Run key, we'll never
  // reach FlagMovingIntoSlopes_X causing a Dash Buffering glitch.
  // Fix by always calling it, not sure why you wouldn't always want to call it.
  if ((R14 & 2) == 0 && (R12 & 5) != 0) {
    bool skip_check = link_is_running && !(link_direction_facing & 4);
    if (!skip_check || (enhanced_features0 & kFeatures0_MiscBugFixes)) {
      FlagMovingIntoSlopes_X();
      if ((link_moving_against_diag_tile & 0xf) != 0)
        return;
    }  // endif_9
  }

  link_moving_against_diag_tile = 0;

  if ((bitfield_spike_cactus_tiles & 7) != 0) {
    if ((link_incapacitated_timer | countdown_for_blink | link_cape_mode) == 0) {
      if (link_last_direction_moved_towards == 2 ? ((link_x_coord & 4) == 0) : ((link_x_coord & 4) != 0)) {
        link_give_damage = 8;
        Link_CancelDash();
        LinkApplyTileRebound();
        return;
      }
    } else {
      R14 = bitfield_spike_cactus_tiles & 7;
    }
  }  // endif_10
  HandlePushingBonkingSnaps_X();
}

void SnapOnX() {  // 87cb84
  link_x_coord -= (link_x_coord & 7) - (sign8(link_x_vel) ? 8 : 0);
}

void CalculateSnapScratch_X() {  // 87cb9f
  if (R14 & 4) {
    int8 x = link_x_vel;
    if (x >= 0) x = -x; // wtf
    link_y_coord += x < 0 ? -1 : 1;
  } else {
    int8 x = link_x_vel;
    if (x < 0) x = -x;
    link_y_coord += x < 0 ? -1 : 1;
  }
}

int8 ChangeAxisOfPerpendicularDoorMovement_X() {  // 87cbdd
  link_cant_change_direction |= 2;
  uint8 r0 = (R14 | (R14 >> 4)) & 0xf;
  if ((r0 & 7) == 0) {
    is_standing_in_doorway = 0;
    return r0; // wtf?
  }

  int8 x_vel = link_x_vel;
  uint8 dir;
  if ((uint8)link_y_coord >= 0x80) {
    if (x_vel >= 0) x_vel = -x_vel;
    dir = 0;
  } else {
    if (x_vel < 0) x_vel = -x_vel;
    dir = 2;
  }
  if (!(link_cant_change_direction & 1))
    link_direction_facing = dir;
  link_y_coord += x_vel;
  return x_vel;
}

void Link_HopInOrOutOfWater_X() {  // 87cc3c
  static const uint8 kRecoilVelX[] = { 28, 24, 16 };
  static const uint8 kRecoilVelZ[] = { 32, 24, 24 };

  uint8 ts = !player_is_indoors ? 2 :
    about_to_jump_off_ledge ? 0 : TS_copy;

  int8 vel = kRecoilVelX[ts];
  if (!(link_last_direction_moved_towards & 1))
    vel = -vel;
  link_actual_vel_x = vel;
  link_actual_vel_y = 0;
  link_actual_vel_z_copy = link_actual_vel_z = kRecoilVelZ[ts];
  link_incapacitated_timer = 16;
  if (link_auxiliary_state != 2) {
    link_auxiliary_state = 1;
    link_electrocute_on_touch = 0;
  }
  link_player_handler_state = 6;
}

void Link_HandleDiagonalKickback() {  // 87ccab
  if (link_x_vel && link_y_vel) {
    link_y_coord_copy = link_y_coord;
    link_x_coord_copy = link_x_coord;

    TileDetect_Movement_X(sign8(link_x_vel) ? 2 : 3);
    if ((R12 & 5) == 0)
      goto noHorizOrNoVertical;
    FlagMovingIntoSlopes_X();
    if (!(link_moving_against_diag_tile & 0xf))
      goto noHorizOrNoVertical;

    int8 xd = link_x_coord - link_x_coord_copy;
    link_x_coord = link_x_coord_copy;
    link_x_vel = xd;

    TileDetect_Movement_Y(sign8(link_y_vel) ? 0 : 1);
    if ((R12 & 5) == 0)
      goto noHorizOrNoVertical;
    FlagMovingIntoSlopes_Y();
    if (!(link_moving_against_diag_tile & 0xf))
      goto noHorizOrNoVertical;

    moving_against_diag_deadlocked = link_moving_against_diag_tile;

    int8 yd = link_y_coord - link_y_coord_copy;
    link_y_vel = yd;

    static const int8 x0[] = { 0, 1, 1, 1, 2, 2, 2, 3, 3, 3 };
    static const int8 x1[] = { 0, -1, -1, -1, -2, -2, -2, -3, -3, -3 };
    link_x_coord += sign8(link_x_vel) ? x1[-(int8)link_x_vel] : x0[link_x_vel];

    static const int8 y0[10] = { 0, 0, 0, 1, 1, 1, 2, 2, 2, 3 };
    // Bug in zelda, might read index 15
    static const int8 y1[16] = { 0, 1, 1, 2, 2, 2, 3, 3, 3, 3, 0xa5, 0x30, 0xf0, 0x04, 0xa5, 0x31 };
    link_y_coord += sign8(link_y_vel) ? y1[-(int8)link_y_vel] : y0[link_y_vel];
  } else {
noHorizOrNoVertical:
    moving_against_diag_deadlocked = 0;
  }
  link_moving_against_diag_tile = 0;
}

void TileDetect_MainHandler(uint8 item) {  // 87d077
  tiledetect_pit_tile = 0;
  TileDetect_ResetState();
  uint16 o;

  if (item == 8) {
    int16 a = state_for_spin_attack - 2;
    if (a < 0 || a >= 8)
      return;
    static const uint8 kDoSwordInteractionWithTiles_o[] = { 10, 6, 14, 2, 12, 4, 8, 0 };
    o = kDoSwordInteractionWithTiles_o[a] + 0x40;
  } else {
    o = item * 8 + link_direction_facing;
  }
  o >>= 1;

  static const int8 kDoSwordInteractionWithTiles_x[] = { 8, 8, 8, 8, 6, 8, -1, 22, 19, 19, 0, 19, 6, 8, -1, 22, 8, 8, 8, 8, 8, 8, 0, 15, 6, 8, -10, 29, 6, 8, -6, 22, 6, 8, -4, 22, -4, 22, -4, 22 };
  static const int8 kDoSwordInteractionWithTiles_y[] = { 20, 20, 20, 20, 4, 28, 16, 16, 22, 22, 22, 22, 4, 24, 16, 16, 16, 16, 16, 16, 20, 20, 23, 23, -4, 36, 16, 16, 4, 28, 16, 16, 4, 28, 16, 16, 4, 4, 28, 28 };
  uint16 x = ((link_x_coord + kDoSwordInteractionWithTiles_x[o]) & tilemap_location_calc_mask) >> 3;
  uint16 y = ((link_y_coord + kDoSwordInteractionWithTiles_y[o]) & tilemap_location_calc_mask);

  if (item == 1 || item == 2 || item == 3 || item == 6 || item == 7 || item == 8) {
    TileBehavior_HandleItemAndExecute(x, y);
    return;
  }

  TileDetection_Execute(x, y, 1);

  if (item == 5)
    return;

  if (tiledetect_thick_grass & 0x10) {
    uint8 tx = (link_x_coord + 0) & 0xf;
    uint8 ty = (link_y_coord + 8) & 0xf;

    if ((ty < 4 || ty >= 11) && (tx < 4 || tx >= 12) && countdown_for_blink == 0 && link_auxiliary_state == 0) {
      if (player_is_indoors) {
        Dungeon_FlagRoomData_Quadrants();
        Ancilla_Sfx2_Near(0x33);
        link_speed_setting = 0;
        submodule_index = 21;
        BYTE(dungeon_room_index_prev) = dungeon_room_index;
        BYTE(dungeon_room_index) = dung_hdr_travel_destinations[0];
        HandleLayerOfDestination();
      } else if (!link_triggered_by_whirlpool_sprite) {
        DoSwordInteractionWithTiles_Mirror();
      }
    }
  } else {  // else_3
    link_triggered_by_whirlpool_sprite = 0;
    if (tiledetect_thick_grass & 1) {
      draw_water_ripples_or_grass = 2;
      if (!Link_PermissionForSloshSounds() && link_auxiliary_state == 0)
        Ancilla_Sfx2_Near(26);
      return;
    }

    if (tiledetect_shallow_water & 1) {
      draw_water_ripples_or_grass = 1;

      if (!player_is_indoors && link_is_in_deep_water && !link_is_bunny_mirror) {
        if (link_item_flippers) {
          link_is_in_deep_water = 0;
          link_direction_last = link_some_direction_bits;
          link_player_handler_state = 0;
        }
      } else if (!Link_PermissionForSloshSounds()) {
        if ((uint8)overworld_screen_index == 0x70) {
          Ancilla_Sfx2_Near(27);
        } else if (link_auxiliary_state == 0) {
          Ancilla_Sfx2_Near(28);
        }
      }
      return;
    }

    if (!player_is_indoors && !link_is_in_deep_water && (tiledetect_deepwater & 1)) {
      draw_water_ripples_or_grass = 1;
      if (!Link_PermissionForSloshSounds()) {
        if ((uint8)overworld_screen_index == 0x70) {
          Ancilla_Sfx2_Near(27);
        } else if (link_auxiliary_state == 0) {
          Ancilla_Sfx2_Near(28);
        }
      }
      return;
    }
  }
  // else_6
  draw_water_ripples_or_grass = 0;

  if (tiledetect_spike_floor_and_tile_triggers & 1) {
    byte_7E02ED = 1;
    return;
  }

  byte_7E02ED = 0;

  if (tiledetect_spike_floor_and_tile_triggers & 0x10) {
    link_give_damage = 0;
    if (!link_cape_mode && !SearchForByrnaSpark() && !countdown_for_blink) {
      link_need_for_poof_for_transform = 0;
      link_timer_tempbunny = 0;
      if (link_item_moon_pearl) {
        link_is_bunny = 0;
        link_is_bunny_mirror = 0;
      }
      link_give_damage = 8;
      Link_CancelDash();
      return;
    }
  }

  if (tiledetect_icy_floor & 0x11) {
    if (link_flag_moving) {
      if (link_num_orthogonal_directions)
        link_direction_last = link_some_direction_bits;
    } else { // else_11
      if (link_direction & 0xC)
        swimcoll_var7[0] = 0x180;
      if (link_direction & 3)
        swimcoll_var7[0] = 0x180;

      link_flag_moving = (tiledetect_icy_floor & 1) ? 1 : 2;
      link_some_direction_bits = link_direction_last;
      Link_ResetSwimmingState();
    }
  } else {
    if (link_player_handler_state != 4) {
      if (link_flag_moving)
        link_direction_last = link_some_direction_bits;
      Link_ResetSwimmingState();
    }
    link_flag_moving = 0;
  }

  if ((bitfield_spike_cactus_tiles & 0x10) && countdown_for_blink == 0)
    countdown_for_blink = 58;
}

bool Link_PermissionForSloshSounds() {  // 87d2c6
  if (!(link_direction & 0xf))
    return true;
  if (link_player_handler_state != 17) {
    return (frame_counter & 0xf) != 0;
  } else {
    return (frame_counter & 0x7) != 0;
  }
}

bool PushBlock_AttemptToPushTheBlock(uint8 what, uint16 x, uint16 y) {  // 87d304
  static const int8 kChangableDungeonObj_Func1B_y0[4] = { -4, 20, 4, 4 };
  static const int8 kChangableDungeonObj_Func1B_y1[4] = { -4, 20, 12, 12 };
  static const int8 kChangableDungeonObj_Func1B_x0[4] = { 4, 4, -4, 20 };
  static const int8 kChangableDungeonObj_Func1B_x1[4] = { 12, 12, -4, 20 };
  static const uint8 T[256] = {
    0, 1, 2, 3, 2, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1,
    0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1,
    0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1

  };

  uint8 idx = what * 4 + link_last_direction_moved_towards, xt;
  uint16 new_x, new_y;

  new_x = ((x + kChangableDungeonObj_Func1B_x0[idx]) & tilemap_location_calc_mask) >> 3;
  new_y = ((y + kChangableDungeonObj_Func1B_y0[idx]) & tilemap_location_calc_mask);
  xt = PushBlock_GetTargetTileFlag(new_x, new_y);
  if (T[xt] != 0 && xt != 9)
    return true;

  new_x = ((x + kChangableDungeonObj_Func1B_x1[idx]) & tilemap_location_calc_mask) >> 3;
  new_y = ((y + kChangableDungeonObj_Func1B_y1[idx]) & tilemap_location_calc_mask);
  xt = PushBlock_GetTargetTileFlag(new_x, new_y);
  if (T[xt] != 0 && xt != 9)
    return true;

  return false;
}

uint8 Link_HandleLiftables() {  // 87d383
  static const uint8 kGetBestActionToPerformOnTile_a[7] = { 0, 1, 0, 0, 2, 1, 2 };
  static const uint8 kGetBestActionToPerformOnTile_b[7] = { 2, 3, 1, 4, 0, 5, 6 };

  tiledetect_pit_tile = 0;
  TileDetect_ResetState();

  uint16 y0 = (link_y_coord + kGetBestActionToPerformOnTile_y[link_direction_facing >> 1]) & tilemap_location_calc_mask;
  uint16 y1 = (link_y_coord + 20) & tilemap_location_calc_mask;

  uint16 x0 = ((link_x_coord + kGetBestActionToPerformOnTile_x[link_direction_facing >> 1]) & tilemap_location_calc_mask) >> 3;
  uint16 x1 = ((link_x_coord + 8) & tilemap_location_calc_mask) >> 3;

  TileDetection_Execute(x0, y0, 1);
  TileDetection_Execute(x1, y1, 2);

  uint8 action = ((R14 | tiledetect_vertical_ledge) & 1) ? 3 : 2;

  if (player_is_indoors) {
    uint8 a = Dungeon_CheckForAndIDLiftableTile();
    if (a != 0xff) {
      interacting_with_liftable_tile_x1 = kGetBestActionToPerformOnTile_b[a & 0xf];
    } else {
      if ((tiledetect_read_something & 1) && link_direction_facing == 0 && interacting_with_liftable_tile_x2 == 0)
        action = 4;
      goto getout;
    }
  } else {
    if (!(tiledetect_read_something & 1))
      goto getout;
    if (link_direction_facing == 0 && interacting_with_liftable_tile_x2 == 0) {
      action = 4;
      goto getout;
    }
    interacting_with_liftable_tile_x1 = interacting_with_liftable_tile_x2 >> 1;
  }
  if (kGetBestActionToPerformOnTile_a[interacting_with_liftable_tile_x1] - link_item_gloves <= 0)
    action = 1;
getout:
  if (tiledetect_chest & 1)
    action = 5;
  return action;
}

void HandleNudging(int8 arg_r0) {  // 87d485
  uint8 p, o;

  if ((link_last_direction_moved_towards & 2) == 0) {
    p = (link_last_direction_moved_towards & 1) ? 4 : 0;
    o = (R14 & 4) ? 0 : 2;
  } else {
    p = (link_last_direction_moved_towards & 1) ? 12 : 8;
    o = (R14 & 4) ? 0 : 2;
  }
  o = (o + p) >> 1;

  tiledetect_pit_tile = 0;
  TileDetect_ResetState();

  uint16 y0 = (link_y_coord + kLink_Move_Helper6_tab0[o]) & tilemap_location_calc_mask;
  uint16 x0 = ((link_x_coord + kLink_Move_Helper6_tab1[o]) & tilemap_location_calc_mask) >> 3;

  uint16 y1 = (link_y_coord + kLink_Move_Helper6_tab2[o]) & tilemap_location_calc_mask;
  uint16 x1 = ((link_x_coord + kLink_Move_Helper6_tab3[o]) & tilemap_location_calc_mask) >> 3;

  TileDetection_Execute(x0, y0, 1);
  TileDetection_Execute(x1, y1, 2);

  if ((R14 | detection_of_ledge_tiles_horiz_uphoriz) & 3 ||
      (tiledetect_vertical_ledge | detection_of_unknown_tile_types) & 0x33) {

    if (link_last_direction_moved_towards & 2)
      link_y_coord -= arg_r0;
    else
      link_x_coord -= arg_r0;
  }
}

void TileBehavior_HandleItemAndExecute(uint16 x, uint16 y) {  // 87dc4a
  uint8 tile = HandleItemTileAction_Overworld(x, y);
  TileDetect_ExecuteInner(tile, 0, 1, false);
}

uint8 PushBlock_GetTargetTileFlag(uint16 x, uint16 y) {  // 87e026
  return dung_bg2_attr_table[(y & ~7) * 8 + (x & 0x3f) + (link_is_on_lower_level ? 0x1000 : 0)];
}

void FlagMovingIntoSlopes_Y() {  // 87e076
  int8 y = (tiledetect_which_y_pos[0] & 7);
  uint8 o = (tiledetect_diag_state * 4) + ((link_x_coord - ((R12 & 4) != 0)) & 7);

  if (tiledetect_diagonal_tile & 5) {
    int8 ym = tiledetect_which_y_pos[0] & 7;

    if (enhanced_features0 & kFeatures0_MiscBugFixes) {
      if (tiledetect_diag_state & 2) {
        ym = -ym;
      } else {
        ym = kAvoidJudder1[o] - (8 - ym);
      }
    } else {
      // This code is bad because it could cause the player
      // to move up to 15 pixels, causing an array out bounds read.
      // Not sure how it works, but changed it to look more like the X version.
      if (!(tiledetect_diag_state & 2)) {
        ym = 8 - ym; // 0 to 8
      } else {
        ym += 8; // 8 to 15
      }
      // -15 to 7
      ym = kAvoidJudder1[o] - ym;
    }

    if (link_y_vel == 0)
      return;
    if (sign8(link_y_vel))
      ym = -ym;
    y = ym;
  } else {  // else_1
    y = kAvoidJudder1[o] - y;
  } // endif_1

  if (sign8(link_y_vel)) {
    if (y <= 0)
      return;
    link_y_coord += y;
    link_moving_against_diag_tile = 8;
  } else {
    if (y >= 0)
      return;
    link_y_coord += y;
    link_moving_against_diag_tile = 4;
  }
  link_moving_against_diag_tile |= (R12 & 4) ? 0x10 + 2 : 0x10 + 1;
}

void FlagMovingIntoSlopes_X() {  // 87e112
  int8 x = (link_x_coord - (tiledetect_diag_state == 6)) & 7;
  uint8 o = (tiledetect_diag_state * 4) + (tiledetect_which_y_pos[(R12 & 4) ? 1 : 0] & 7);

  if (tiledetect_diagonal_tile & 5) {
    int8 xm = link_x_coord & 7;

    if (tiledetect_diag_state != 4 && tiledetect_diag_state != 6) {
      xm = -xm;
    } else {
      xm = kAvoidJudder1[o] - (8 - xm);
    } // endif_5
    if (link_x_vel == 0)
      return;
    if (sign8(link_x_vel))
      xm = -xm;
    x = xm;
  } else {  // else_1
    x = kAvoidJudder1[o] - x;
  } //  endif_1

  if (sign8(link_x_vel)) {
    if (x <= 0)
      return;
    link_x_coord += x;
    link_moving_against_diag_tile = 2;
  } else {
    if (x >= 0)
      return;
    link_x_coord += x;
    link_moving_against_diag_tile = 1;
  }
  link_moving_against_diag_tile |= (tiledetect_diag_state & 2) ? 0x20 + 8 : 0x20 + 4;
}

void Link_HandleRecoiling() {  // 87e1be
  link_direction = 0;
  if (link_actual_vel_y) {
    link_direction_last = (link_direction |= sign8(link_actual_vel_y) ? 8 : 4);
    Player_HandleIncapacitated_Inner2();
  }
  if (link_actual_vel_x)
    link_direction_last = (link_direction |= sign8(link_actual_vel_x) ? 2 : 1);
  Player_HandleIncapacitated_Inner2();
}

void Player_HandleIncapacitated_Inner2() {  // 87e1d7
  if ((link_moving_against_diag_tile & 0xc) && (link_moving_against_diag_tile & 3) && link_player_handler_state == 2) {
    link_actual_vel_x = -link_actual_vel_x;
    link_actual_vel_y = -link_actual_vel_y;
  }
  if (is_standing_in_doorway == 1) {
    link_direction_last &= 0xc;
    link_direction &= 0xc;
    link_actual_vel_x = 0;
  } else if (is_standing_in_doorway == 2) {
    link_direction_last &= 3;
    link_direction &= 3;
    link_actual_vel_y = 0;
  }
}

void Link_HandleVelocity() {  // 87e245
  if (submodule_index == 2 && main_module_index == 14 || link_prevent_from_moving) {
    link_y_coord_safe_return_lo = link_y_coord;
    link_y_coord_safe_return_hi = link_y_coord >> 8;
    link_x_coord_safe_return_lo = link_x_coord;
    link_x_coord_safe_return_hi = link_x_coord >> 8;
    Link_HandleVelocityAndSandDrag(link_x_coord, link_y_coord);
    return;
  }

  if (link_player_handler_state == kPlayerState_Swimming) {
    HandleSwimStrokeAndSubpixels();
    return;
  }
  uint8 r0;

  if (link_flag_moving) {
    if (!link_is_running) {
      HandleSwimStrokeAndSubpixels();
      return;
    }
    r0 = 24;
  } else {
    if (link_is_running) {
      link_speed_modifier = 0;
      assert(link_dash_ctr >= 32);
    }

    if ((byte_7E0316 | byte_7E0317) == 0xf)
      return;

    r0 = link_speed_setting;
    if (draw_water_ripples_or_grass) {
      r0 = (link_speed_setting == 16) ? 22 :
        (link_speed_setting == 12) ? 14 : 12;
    }
  }  // endif_4

  link_actual_vel_x = link_actual_vel_y = 0;
  link_y_page_movement_delta = link_x_page_movement_delta = 0;

  r0 += ((link_direction & 0xC) != 0 && (link_direction & 3) != 0);

  if (player_near_pit_state) {
    if (player_near_pit_state == 3)
      link_speed_modifier = (link_speed_modifier < 48) ? link_speed_modifier + 8 : 32;
  } else {
    if (link_speed_modifier) {
      r0 = (submodule_index == 8 || submodule_index == 16) ? 10 : 2;
      if (link_speed_modifier != 1) {
        if (link_speed_modifier < 16) {
          link_speed_modifier += 1;
          r0 = 26; // kSpeedMod[26] is 0
        } else {
          link_speed_modifier = 0;
          link_speed_setting = 0;
        }
      }
    }
  } // endif_7

  static const uint8 kSpeedMod[27] = { 24, 16, 10, 24, 16, 8, 8, 4, 12, 16, 9, 25, 20, 13, 16, 8, 64, 42, 16, 8, 4, 2, 48, 24, 32, 21, 0 };

  uint8 vel = link_speed_modifier + kSpeedMod[r0];
  if (link_direction & 3)
    link_actual_vel_x = (link_direction & 2) ? -vel : vel;
  if (link_direction & 0xC)
    link_actual_vel_y = (link_direction & 8) ? -vel : vel;

  link_actual_vel_z = 0xff;
  link_z_coord = 0xffff;
  link_subpixel_z = 0;
  Link_MovePosition();
}

void Link_MovePosition() {  // 87e370
  uint16 x = link_x_coord, y = link_y_coord;
  link_y_coord_safe_return_lo = link_y_coord;
  link_y_coord_safe_return_hi = link_y_coord >> 8;
  link_x_coord_safe_return_lo = link_x_coord;
  link_x_coord_safe_return_hi = link_x_coord >> 8;

  if (link_player_handler_state != 10 && player_on_somaria_platform == 2) {
    Link_HandleVelocityAndSandDrag(x, y);
    return;
  }

  uint32 tmp;
  tmp = link_subpixel_x + (int8)link_actual_vel_x * 16 + link_x_coord * 256;
  link_subpixel_x = (uint8)tmp, link_x_coord = (tmp >> 8);
  tmp = link_subpixel_y + (int8)link_actual_vel_y * 16 + link_y_coord * 256;
  link_subpixel_y = (uint8)tmp, link_y_coord = (tmp >> 8);
  if (link_auxiliary_state) {
    tmp = link_subpixel_z + (int8)link_actual_vel_z * 16 + link_z_coord * 256;
    link_subpixel_z = (uint8)tmp, link_z_coord = (tmp >> 8);
  }

  Link_HandleMovingFloor();
  Link_ApplyConveyor();
  Link_HandleVelocityAndSandDrag(x, y);
}

void Link_HandleVelocityAndSandDrag(uint16 x, uint16 y) {  // 87e3e0
  link_y_coord += drag_player_y;
  link_x_coord += drag_player_x;
  link_y_vel = link_y_coord - y;
  link_x_vel = link_x_coord - x;
}

void HandleSwimStrokeAndSubpixels() {  // 87e42a
  link_actual_vel_x = link_actual_vel_y = 0;

  static const int8 kSwimmingTab4[] = { 8, -12, -8, -16, 4, -6, -12, -6, 10, -16, -12, -6 };
  static const uint8 kSwimmingTab5[] = { ~0xc & 0xff, ~3 & 0xff };
  static const uint8 kSwimmingTab6[] = { 8, 4, 2, 1 };
  uint16 S[2];
  for (int i = 1; i >= 0; i--) {
    if ((int16)--swimcoll_var3[i] < 0) {
      swimcoll_var3[i] = 0;
      swimcoll_var5[i] = 1;
    }
    uint16 t = swimcoll_var5[i];
    if (link_flag_moving)
      t += link_flag_moving * 4;

    uint16 sum = swimcoll_var7[i] + kSwimmingTab4[t];
    if ((int16)sum <= 0) {
      link_direction &= kSwimmingTab5[i];
      link_direction_last = link_direction;
      //link_actual_vel_y = link_y_page_movement_delta; // WTF bug?!
      if (swimcoll_var5[i] == 2) {
        swimcoll_var5[i] = 0;
        swimcoll_var9[i] = 240;
        swimcoll_var7[i] = 2;
      } else {
        swimcoll_var5[i] = 0;
        swimcoll_var9[i] = 0;
        swimcoll_var7[i] = 0;
      }
    } else {
      link_direction |= kSwimmingTab6[swimcoll_var11[i] + i * 2];
      if (sum >= swimcoll_var9[i])
        sum = swimcoll_var9[i];
      swimcoll_var7[i] = sum;
    }
    S[i] = swimcoll_var7[i];
    if (link_num_orthogonal_directions | link_moving_against_diag_tile)
      S[i] -= S[i] >> 2;
    if (!swimcoll_var11[i])
      S[i] = -S[i];
  }

  Player_SomethingWithVelocity_TiredOrSwim(S[1], S[0]);

}

void Player_SomethingWithVelocity_TiredOrSwim(uint16 xvel, uint16 yvel) {  // 87e4d3
  uint16 org_x = link_x_coord, org_y = link_y_coord;
  link_y_coord_safe_return_lo = link_y_coord;
  link_y_coord_safe_return_hi = link_y_coord >> 8;
  link_x_coord_safe_return_lo = link_x_coord;
  link_x_coord_safe_return_hi = link_x_coord >> 8;

  uint8 u;

  uint32 tmp;
  tmp = link_subpixel_x + (int16)xvel + link_x_coord * 256;
  link_subpixel_x = (uint8)tmp, link_x_coord = (tmp >> 8);

  u = xvel >> 8;
  link_actual_vel_x = ((sign8(u) ? -u : u) << 4) | ((uint8)xvel >> 4);

  tmp = link_subpixel_y + (int16)yvel + link_y_coord * 256;
  link_subpixel_y = (uint8)tmp, link_y_coord = (tmp >> 8);
  u = yvel >> 8;
  link_actual_vel_y = ((sign8(u) ? -u : u) << 4) | ((uint8)yvel >> 4);

  if (dung_hdr_collision == 4)
    Link_ApplyMovingFloorVelocity();
  link_x_page_movement_delta = 0;
  link_y_page_movement_delta = 0;
  Link_HandleVelocityAndSandDrag(org_x, org_y);
}

void Link_HandleMovingFloor() {  // 87e595
  if (!dung_hdr_collision)
    return;
  if (BYTE(link_z_coord) != 0 && BYTE(link_z_coord) != 255)
    return;
  if (((byte_7E0322) & 3) != 3)
    return;
  if (link_player_handler_state == 19) // hookshot
    return;

  if (dung_floor_y_vel)
    link_direction |= sign8(dung_floor_y_vel) ? 8 : 4;

  if (dung_floor_x_vel)
    link_direction |= sign8(dung_floor_x_vel) ? 2 : 1;

  Link_ApplyMovingFloorVelocity();
}

void Link_ApplyMovingFloorVelocity() {  // 87e5cd
  link_num_orthogonal_directions = 0;
  link_y_coord += dung_floor_y_vel;
  link_x_coord += dung_floor_x_vel;
}

void Link_ApplyConveyor() {  // 87e5f0
  static const uint8 kMovePosDirFlag[4] = { 8, 4, 2, 1 };
  static const int8 kMovingBeltY[4] = { -8, 8, 0, 0 };
  static const int8 kMovingBeltX[4] = { 0, 0, -8, 8 };

  if (!link_on_conveyor_belt)
    return;
  if (BYTE(link_z_coord) != 0 && BYTE(link_z_coord) != 0xff)
    return;
  if (link_grabbing_wall & 1 || link_player_handler_state == kPlayerState_Hookshot || link_auxiliary_state)
    return;

  int j = link_on_conveyor_belt - 1;
  if (link_is_running && link_dash_ctr == 32 && (link_direction & kMovePosDirFlag[j]))
    return;

  link_num_orthogonal_directions = 0;
  link_direction |= kMovePosDirFlag[j];

  uint32 t = link_y_coord << 8 | dung_some_subpixel[0];
  t += kMovingBeltY[j] << 4;
  dung_some_subpixel[0] = t;
  link_y_coord = t >> 8;

  t = link_x_coord << 8 | dung_some_subpixel[1];
  t += kMovingBeltX[j] << 4;
  dung_some_subpixel[1] = t;
  link_x_coord = t >> 8;
}

void Link_HandleMovingAnimation_FullLongEntry() {  // 87e6a6
  if (link_player_handler_state == 4) {
    Link_HandleMovingAnimationSwimming();
    return;
  }

  static const uint8 kTab[4] = { 8, 4, 2, 1 };

  uint8 r0 = link_direction_last;
  uint8 y;
  if (r0 == 0)
    return;
  if (link_flag_moving)
    r0 = link_some_direction_bits;
  if (link_cant_change_direction)
    goto bail;
  if (link_num_orthogonal_directions == 0)
    goto not_diag;
  if (is_standing_in_doorway) {
    y = (is_standing_in_doorway * 2) & ~3;
  } else {
    if (r0 & kTab[link_direction_facing >> 1])
      goto bail;
not_diag:
    y = (r0 & 0xc) ? 0 : 4;
  }

  if (y != 4) {
    y += (r0 & 4) ? 2 : 0;
  } else {
    y += (r0 & 1) ? 2 : 0;
  }
  link_direction_facing = y;
bail:
  Link_HandleMovingAnimation_StartWithDash();
}

void Link_HandleMovingAnimation_StartWithDash() {  // 87e704
  if (link_is_running) {
    Link_HandleMovingAnimation_Dash();
    return;
  }

  uint8 x = link_direction_facing >> 1;
  if (link_speed_setting == 6) {
    x += 4;
  } else if (link_flag_moving) {
    if (!(joypad1H_last & kJoypadH_AnyDir)) {
      link_animation_steps = 0;
      return;
    }
    x += 4;
  }

  static const uint8 tab2[16] = { 4, 4, 4, 4, 1, 1, 1, 1, 2, 2, 2, 2, 8, 8, 8, 8 };
  static const uint8 tab3[24] = { 1, 2, 3, 2, 2, 2, 3, 2, 1, 1, 2, 1, 1, 1, 2, 1, 2, 2, 3, 2, 2, 2, 3, 2 };

//bugfix: tempbunny animation steps are wrong due to missing check
  if (link_player_handler_state == 23 || (enhanced_features0 & kFeatures0_MiscBugFixes && link_player_handler_state == 28)) {  // bunny states
    if (link_animation_steps < 4 && player_on_somaria_platform != 2) {
      if (++link_counter_var1 >= tab2[x]) {
        link_counter_var1 = 0;
        if (++link_animation_steps == 4)
          link_animation_steps = 0;
      }
    } else {
      link_animation_steps = 0;
    }
    return;
  }

  if (submodule_index == 18 || submodule_index == 19) {
    x = 12;
  } else if (submodule_index != kPlayerState_JumpOffLedgeDiag && (link_state_bits & 0x80) == 0) {
    if (bitmask_of_dragstate & 0x8d) {
      x = 12;
    } else if (!draw_water_ripples_or_grass && !button_b_frames) {
      // else_6
      x = link_animation_steps;
      if (link_speed_setting == 6)
        x += 8;
      if (link_flag_moving)
        x += 8;
      if (player_on_somaria_platform == 2)
        return;
      if (++link_counter_var1 >= tab3[x]) {
        link_counter_var1 = 0;
        if (++link_animation_steps == 9)
          link_animation_steps = 1;
      }
      return;
    }
  }
  // endif_4

  if (link_animation_steps < 6 && player_on_somaria_platform != 2) {
    if (++link_counter_var1 >= tab2[x]) {
      link_counter_var1 = 0;
      if (++link_animation_steps == 6)
        link_animation_steps = 0;
    }
  } else {
    link_animation_steps = 0;
  }
}

void Link_HandleMovingAnimationSwimming() {  // 87e7fa
  static const uint8 kTab[4] = { 8, 4, 2, 1 };
  if (!link_some_direction_bits || link_cant_change_direction)
    return;
  uint8 y;

  if (link_num_orthogonal_directions) {
    if (is_standing_in_doorway) {
      y = (is_standing_in_doorway * 2) & ~3;
    } else {
      if (link_some_direction_bits & kTab[link_direction_facing >> 1])
        return;
      y = link_some_direction_bits & 0xC ? 0 : 4;
    }
  } else {
    y = link_some_direction_bits & 0xC ? 0 : 4;
  }
  if (y != 4) {
    y += (link_some_direction_bits & 4) ? 2 : 0;
  } else {
    y += (link_some_direction_bits & 1) ? 2 : 0;
  }
  link_direction_facing = y;
}

void Link_HandleMovingAnimation_Dash() {  // 87e88f
  static const uint8 kDashTab3[] = { 48, 36, 24, 16, 12, 8, 4 };
  static const uint8 kDashTab4[] = { 3, 3, 5, 3, 3, 3, 5, 3, 2, 2, 4, 2, 2, 2, 4, 2, 2, 2, 3, 2, 2, 2, 3, 2, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  static const uint8 kDashTab5[] = { 1, 2, 2, 2, 2, 2, 2 };

  uint8 t = 6;
  while (link_countdown_for_dash >= kDashTab3[t] && t)
    t--;

  if (button_b_frames < 9 && !draw_water_ripples_or_grass) {
    if (++link_counter_var1 >= kDashTab4[t * 8]) {
      link_counter_var1 = 0;
      link_animation_steps++;
      if (link_animation_steps == 9)
        link_animation_steps = 1;
    }
  } else {
    if (++link_counter_var1 >= kDashTab5[t]) {
      link_counter_var1 = 0;
      link_animation_steps++;
      if (link_animation_steps >= 6)
        link_animation_steps = 0;
    }
  }
}

void HandleIndoorCameraAndDoors() {  // 87e8f0
  if (player_is_indoors) {
    if (is_standing_in_doorway)
      HandleDoorTransitions();
    else
      ApplyLinksMovementToCamera();
  }
}

void HandleDoorTransitions() {  // 87e901
  uint16 t;

  link_x_page_movement_delta = 0;
  link_y_page_movement_delta = 0;

  // Using a potion might have changed us into a different module, and the routines
  // below just increment the submodule value, causing all kinds of havoc.
  // There's an added return to catch the same behavior a bit up, but this one catches more cases,
  // at the expense of link already having done his movement, so by returning here we might
  // miss handling the door causing other kinds of issues.
  if ((enhanced_features0 & kFeatures0_MiscBugFixes) && !(main_module_index == 7 && submodule_index == 0))
    return;

  if (link_direction_last & 0xC && is_standing_in_doorway == 1) {
    if (link_direction_last & 4) {
      if (((t = link_y_coord + 28) & 0xfc) == 0)
        link_y_page_movement_delta = (t >> 8) - link_y_coord_safe_return_hi;
    } else {
      t = link_y_coord - 18;
      link_y_page_movement_delta = (t >> 8) - link_y_coord_safe_return_hi;
    }
  }

  if (link_direction_last & 3 && is_standing_in_doorway == 2) {
    if (link_direction_last & 1) {
      if (((t = link_x_coord + 21) & 0xfc) == 0)
        link_x_page_movement_delta = (t >> 8) - link_x_coord_safe_return_hi;
    } else {
      t = link_x_coord - 8;
      link_x_page_movement_delta = (t >> 8) - link_x_coord_safe_return_hi;
    }
  }

  if (link_x_page_movement_delta) {
    some_animation_timer = 0;
    link_state_bits = 0;
    link_picking_throw_state = 0;
    link_grabbing_wall = 0;
    if (sign8(link_x_page_movement_delta))
      Dung_StartInterRoomTrans_Left_Plus();
    else
      HandleEdgeTransitionMovementEast_RightBy8();
  } else if (link_y_page_movement_delta) {
    some_animation_timer = 0;
    link_state_bits = 0;
    link_picking_throw_state = 0;
    link_grabbing_wall = 0;
    if (sign8(link_y_page_movement_delta))
      Dungeon_StartInterRoomTrans_Up();
    else
      HandleEdgeTransitionMovementSouth_DownBy16();
  }
}

void ApplyLinksMovementToCamera() {  // 87e9d3
  // Sometimes, when using spin attack, this routine will end up getting
  // called twice in the same frame, which messes up things.
  g_ApplyLinksMovementToCamera_called = true;

  link_y_page_movement_delta = (link_y_coord >> 8) - link_y_coord_safe_return_hi;
  link_x_page_movement_delta = (link_x_coord >> 8) - link_x_coord_safe_return_hi;

  if (link_x_page_movement_delta) {
    if (sign8(link_x_page_movement_delta))
      AdjustQuadrantAndCamera_left();
    else
      AdjustQuadrantAndCamera_right();
  }

  if (link_y_page_movement_delta) {
    if (sign8(link_y_page_movement_delta))
      AdjustQuadrantAndCamera_up();
    else
      AdjustQuadrantAndCamera_down();
  }
}

uint8 FindFreeMovingBlockSlot(uint8 x) {  // 87ed2c
  if (index_of_changable_dungeon_objs[1] == 0) {
    index_of_changable_dungeon_objs[1] = x + 1;
    return 1;
  }
  if (index_of_changable_dungeon_objs[0] == 0) {
    index_of_changable_dungeon_objs[0] = x + 1;
    return 0;
  }
  return 0xff;
}

bool InitializePushBlock(uint8 r14, uint8 idx) {  // 87ed3f
  uint16 pos = dung_object_tilemap_pos[idx >> 1];
  uint16 x = (pos & 0x007e) << 2;
  uint16 y = (pos & 0x1f80) >> 4;

  x += (dung_loade_bgoffs_h_copy & 0xff00);
  y += (dung_loade_bgoffs_v_copy & 0xff00);

  pushedblocks_x_lo[r14] = (uint8)x;
  pushedblocks_x_hi[r14] = (uint8)(x >> 8);
  pushedblocks_y_lo[r14] = (uint8)y;
  pushedblocks_y_hi[r14] = (uint8)(y >> 8);
  pushedblocks_target[r14] = 0;
  pushedblocks_subpixel[r14] = 0;

  if (dung_hdr_tag[0] != 38 && dung_replacement_tile_state[idx >> 1] == 0) {
    if (!PushBlock_AttemptToPushTheBlock(0, x, y)) {
      Ancilla_Sfx2_Near(0x22);
      dung_replacement_tile_state[idx >> 1] = 1;
      return false;
    }
  }

  index_of_changable_dungeon_objs[r14] = 0;
  return true;
}

void Sprite_Dungeon_DrawSinglePushBlock(int j) {  // 87f0d9
  static const uint8 kPushedBlock_Tab1[9] = { 0, 1, 2, 3, 4, 0, 0, 0, 0 };
  static const uint8 kPushedblock_Char[4] = { 0xc, 0xc, 0xc, 0xff };
  j >>= 1;
  Oam_AllocateFromRegionB(4);
  OamEnt *oam = GetOamCurPtr();
  int y = (uint8)pushedblocks_y_lo[j] | (uint8)pushedblocks_y_hi[j] << 8;
  int x = (uint8)pushedblocks_x_lo[j] | (uint8)pushedblocks_x_hi[j] << 8;
  y -= BG2VOFS_copy2 + 1;
  x -= BG2HOFS_copy2;
  uint8 ch = kPushedblock_Char[kPushedBlock_Tab1[pushedblocks_some_index]];
  if (ch != 0xff)
    SetOamPlain(oam, x, y, ch, 0x20, 2);
}

void Link_Initialize() {  // 87f13c
  link_direction_facing = 2;
  link_direction_last = 0;
  link_item_in_hand = 0;
  link_position_mode = 0;
  link_debug_value_1 = 0;
  link_debug_value_2 = 0;
  link_var30d = 0;
  link_var30e = 0;
  some_animation_timer_steps = 0;
  link_is_transforming = 0;
  bitfield_for_a_button = 0;
  button_mask_b_y &= ~0x40;
  link_state_bits = 0;
  link_picking_throw_state = 0;
  link_grabbing_wall = 0;
  Link_ResetSwimmingState();
  link_cant_change_direction &= ~1;
  link_z_coord &= 0xff;
  link_auxiliary_state = 0;
  link_incapacitated_timer = 0;
  countdown_for_blink = 0;
  link_electrocute_on_touch = 0;
  link_pose_for_item = 0;
  link_cape_mode = 0;
  Link_ForceUnequipCape_quietly();
  Link_ResetSwordAndItemUsage();
  link_disable_sprite_damage = 0;
  player_handler_timer = 0;
  link_direction &= ~0xf;
  player_on_somaria_platform = 0;
  link_spin_attack_step_counter = 0;

  if (enhanced_features0 & kFeatures0_MiscBugFixes) {
    // If you quit while jumping from a ledge and get hit on a platform you can go under solid layers
    about_to_jump_off_ledge = 0;
 
    // If you use the mirror near a moveable statue you can pull thin air and glitch the camera
    link_is_near_moveable_statue = 0;

    // If you use the mirror on a conveyor belt you will retain momentum and clip into the entrance wall
    link_on_conveyor_belt = 0;
    
    // bugfix: you use the mirror on ice, you retain momentum
    link_flag_moving = 0;

    // If you quit in the middle of red armos knight stomp the lumberjack tree will fall on its own
    bg1_y_offset = bg1_x_offset = 0;
      //bugfix: if you die in a dungeon as a permabunny and continue, you revert back to link
      if (!link_item_moon_pearl && savegame_is_darkworld) {
        link_player_handler_state = kPlayerState_PermaBunny;
        link_is_bunny = 1;
        link_is_bunny_mirror = 1;
        LoadGearPalettes_bunny();
      }
  }
}

void Link_ResetProperties_A() {  // 87f1a3
  link_direction_last = 0;
  link_direction = 0;
  link_flag_moving = 0;
  Link_ResetSwimmingState();
  link_is_transforming = 0;
  countdown_for_blink = 0;
  ancilla_arr24[0] = 0;
  link_is_bunny = 0;
  link_is_bunny_mirror = 0;
  BYTE(link_timer_tempbunny) = 0;
  link_need_for_poof_for_transform = 0;
  is_archer_or_shovel_game = 0;
  link_need_for_pullforrupees_sprite = 0;
  BYTE(bit9_of_xcoord) = 0;
  link_something_with_hookshot = 0;
  link_give_damage = 0;
  link_spin_offsets = 0;
  tagalong_event_flags = 0;
  link_want_make_noise_when_dashed = 0;
  BYTE(tiledetect_tile_type) = 0;
  item_receipt_method = 0;
  link_triggered_by_whirlpool_sprite = 0;
  Link_ResetProperties_B();
}

void Link_ResetProperties_B() {  // 87f1e6
  player_on_somaria_platform = 0;
  link_spin_attack_step_counter = 0;
  fallhole_var1 = 0;
  flag_is_sprite_to_pick_up_cached = 0;
  bitmask_of_dragstate = 0;
  link_this_controls_sprite_oam = 0;
  player_near_pit_state = 0;
  Link_ResetProperties_C();
}

void Link_ResetProperties_C() {  // 87f1fa
  if (enhanced_features0 & kFeatures0_MiscBugFixes) {
    // Fix save menu lockout when dying after medallion cast (#126)
    flag_custom_spell_anim_active = 0;
  }

  tile_action_index = 0;
  state_for_spin_attack = 0;
  step_counter_for_spin_attack = 0;
  tile_coll_flag = 0;
  link_force_hold_sword_up = 0;
  link_sword_delay_timer = 0;
  tiledetect_misc_tiles = 0;
  link_item_in_hand = 0;
  link_position_mode = 0;
  link_debug_value_1 = 0;
  link_debug_value_2 = 0;
  link_var30d = 0;
  link_var30e = 0;
  some_animation_timer_steps = 0;
  bitfield_for_a_button = 0;
  button_mask_b_y = 0;
  button_b_frames = 0;
  link_state_bits = 0;
  link_picking_throw_state = 0;
  link_grabbing_wall = 0;
  link_cant_change_direction = 0;
  link_auxiliary_state = 0;
  link_incapacitated_timer = 0;
  link_electrocute_on_touch = 0;
  link_pose_for_item = 0;
  link_cape_mode = 0;
  Link_ResetSwordAndItemUsage();
  link_disable_sprite_damage = 0;
  player_handler_timer = 0;
  related_to_hookshot = 0;
  flag_is_ancilla_to_pick_up = 0;
  flag_is_sprite_to_pick_up = 0;
  link_need_for_pullforrupees_sprite = 0;
  link_is_near_moveable_statue = 0;
}

bool Link_CheckForEdgeScreenTransition() {  // 87f439
  uint8 st = link_player_handler_state;
  if (st == 3 || st == 8 || st == 9 || st == 10 || !link_incapacitated_timer)
    return false;
  link_actual_vel_x = link_actual_vel_y = 0;
  link_recoilmode_timer = 3;
  link_x_coord = link_x_coord_prev;
  link_y_coord = link_y_coord_prev;
  return true;
}

void CacheCameraPropertiesIfOutdoors() {  // 87f514
  if (!player_is_indoors)
    CacheCameraProperties();
}

void SomariaBlock_HandlePlayerInteraction(int k) {  // 88e7e6
  cur_object_index = k;
  if (ancilla_G[k])
    return;

  if (!ancilla_H[k]) {
    if (link_auxiliary_state || (link_state_bits & 1) || ancilla_z[k] != 0 && ancilla_z[k] != 0xff || ancilla_K[k] || ancilla_L[k])
      return;
    if (!(joypad1H_last & kJoypadH_AnyDir)) {
      ancilla_arr3[k] = 0;
      bitmask_of_dragstate = 0;
      ancilla_A[k] = 255;
      if (!link_is_running) {
        link_speed_setting = 0;
        return;
      }
    } else if ((joypad1H_last & kJoypadH_AnyDir) == ancilla_arr3[k]) {
      if (link_speed_setting == 18)
        bitmask_of_dragstate |= 0x81;
    } else {
      ancilla_arr3[k] = (joypad1H_last & kJoypadH_AnyDir);
      link_speed_setting = 0;
    }

    CheckPlayerCollOut coll_out;
    if (!Ancilla_CheckLinkCollision(k, 4, &coll_out) || ancilla_floor[k] != link_is_on_lower_level)
      return;

    if (!link_is_running || link_dash_ctr == 64) {
      uint8 t;
      ancilla_x_vel[k] = 0;
      ancilla_y_vel[k] = 0;
      ancilla_arr3[k] = t = joypad1H_last & kJoypadH_AnyDir;
      if (t & 3) {
        ancilla_x_vel[k] = t & 1 ? 16 : -16;
        ancilla_dir[k] = t & 1 ? 3 : 2;
      } else {
        ancilla_y_vel[k] = t & 8 ? -16 : 16;
        ancilla_dir[k] = t & 8 ? 0 : 1;
      }
      if (link_actual_vel_y == 0 || link_actual_vel_x == 0) {
        if (!Ancilla_CheckTileCollision_Class2(k)) {
          Ancilla_MoveY(k);
          Ancilla_MoveX(k);
          if (!(link_state_bits & 0x80) && !(++ancilla_A[k] & 7))
            Ancilla_Sfx2_Pan(k, 0x22);
        }
        bitmask_of_dragstate = 0x81;
        link_speed_setting = 0x12;
      }
      Sprite_NullifyHookshotDrag();
      return;
    }
    static const int8 kSomarianBlock_Yvel[4] = { -40, 40, 0, 0 };
    static const int8 kSomarianBlock_Xvel[4] = { 0, 0, -40, 40 };
    if (flag_is_ancilla_to_pick_up == k + 1)
      flag_is_ancilla_to_pick_up = 0;
    Link_CancelDash();
    Ancilla_Sfx3_Pan(k, 0x32);
    int j = link_direction_facing >> 1;
    ancilla_dir[k] = j;
    ancilla_y_vel[k] = kSomarianBlock_Yvel[j];
    ancilla_x_vel[k] = kSomarianBlock_Xvel[j];
    ancilla_z_vel[k] = 48;
    ancilla_H[k] = 1;
    ancilla_z[k] = 0;
  }

  ancilla_z_vel[k] -= 2;
  Ancilla_MoveY(k);
  Ancilla_MoveX(k);
  Ancilla_MoveZ(k);
  if (ancilla_z[k] && ancilla_z[k] < 252)
    return;

  Ancilla_Sfx2_Pan(k, 0x21);
  ancilla_z[k] = 0;
  int j = ancilla_H[k]++;
  if (j == 3) {
    ancilla_arr4[k] = 0;
    ancilla_H[k] = 0;
  } else {
    static const int8 kSomarianBlock_Zvel[4] = { 48, 24, 16, 8 };
    ancilla_z_vel[k] = kSomarianBlock_Zvel[j - 1];
    ancilla_y_vel[k] = (int8)ancilla_y_vel[k] / 2;
    ancilla_x_vel[k] = (int8)ancilla_x_vel[k] / 2;
  }
}

void Gravestone_Move(int k) {  // 88ed89
  if (submodule_index)
    return;
  ancilla_y_vel[k] = -8;
  Ancilla_MoveY(k);

  Gravestone_ActAsBarrier(k);
  uint16 y_target = ancilla_B[k] << 8 | ancilla_A[k];
  uint16 y_cur = Ancilla_GetY(k);

  if (y_cur >= y_target)
    return;

  ancilla_type[k] = 0;
  link_something_with_hookshot = 0;
  bitmask_of_dragstate &= ~4;
  BYTE(scratch_0) = ((uint8 *)door_debris_y)[k];
  HIBYTE(scratch_0) = ((uint8 *)door_debris_x)[k];
  big_rock_starting_address = scratch_0;

  door_open_closed_counter = big_rock_starting_address == 0x532 ? 0x48 :
    big_rock_starting_address == 0x488 ? 0x60 : 0x40;
  Overworld_DoMapUpdate32x32_B();
}

void Gravestone_ActAsBarrier(int k) {  // 88ee57
  uint16 x = Ancilla_GetX(k);
  uint16 y = Ancilla_GetY(k);
  uint16 r4 = y + 0x18;
  uint16 r6 = x + 0x20;
  uint16 lx = link_x_coord + 8;
  uint16 ly = link_y_coord + 8;
  if (ly >= y && ly < r4 &&
      lx >= x && lx < r6) {
    uint16 r10 = abs16(ly - r4);
    link_y_coord += r10;
    link_y_vel += r10;
    bitmask_of_dragstate |= 4;
  }
  if (link_direction_facing)
    link_direction_facing &= ~4;
}

void AncillaAdd_DugUpFlute(uint8 a, uint8 y) {  // 898c73
  int k = Ancilla_AddAncilla(a, y);
  if (k < 0)
    return;
  ancilla_step[k] = 0;
  ancilla_z[k] = 0;
  ancilla_z_vel[k] = 24;
  ancilla_x_vel[k] = link_direction_facing == 4 ? -8 : 8;
  DecodeAnimatedSpriteTile_variable(12);
  Ancilla_SetXY(k, 0x490, 0xa8a);
}

void AncillaAdd_CaneOfByrnaInitSpark(uint8 a, uint8 y) {  // 898ee0
  for (int k = 4; k >= 0; k--) {
    if (ancilla_type[k] == 0x31)
      ancilla_type[k] = 0;
  }
  int k = Ancilla_AddAncilla(a, y);
  if (k >= 0) {
    ancilla_item_to_link[k] = 0;
    ancilla_aux_timer[k] = 9;
    link_disable_sprite_damage = 1;
    ancilla_arr3[k] = 2;
  }
}

void AncillaAdd_ShovelDirt(uint8 a, uint8 y) {  // 898f5b
  int k = Ancilla_AddAncilla(a, y);
  if (k >= 0) {
    ancilla_item_to_link[k] = 0;
    ancilla_timer[k] = 20;
    Ancilla_SetXY(k, link_x_coord, link_y_coord);
  }
}

void AncillaAdd_Hookshot(uint8 a, uint8 y) {  // 899b10
  static const int8 kHookshot_Yvel[4] = { -64, 64, 0, 0 };
  static const int8 kHookshot_Xvel[4] = { 0, 0, -64, 64 };
  static const int8 kHookshot_Yd[4] = { 4, 20, 8, 8 };
  static const int8 kHookshot_Xd[4] = { 0, 0, -4, 11 };

  int k = Ancilla_AddAncilla(a, y);
  if (k >= 0) {
    ancilla_aux_timer[k] = 3;
    ancilla_item_to_link[k] = 0;
    ancilla_step[k] = 0;
    ancilla_L[k] = 0;
    related_to_hookshot = 0;
    hookshot_effect_index = k;
    ancilla_K[k] = 0;
    ancilla_G[k] = 255;
    ancilla_arr1[k] = 0;
    ancilla_timer[k] = 0;
    int j = link_direction_facing >> 1;
    ancilla_dir[k] = j;
    ancilla_x_vel[k] = kHookshot_Xvel[j];
    ancilla_y_vel[k] = kHookshot_Yvel[j];
    Ancilla_SetXY(k, link_x_coord + kHookshot_Xd[j], link_y_coord + kHookshot_Yd[j]);
  }
}

void ResetSomeThingsAfterDeath(uint8 a) {  // 8bffbf
  link_is_in_deep_water = 0;
  link_speed_setting = a;
  link_on_conveyor_belt = 0;
  byte_7E0322 = 0;
  flag_is_link_immobilized = 0;
  palette_swap_flag = 0;
  player_unk1 = 0;
  link_give_damage = 0;
  link_actual_vel_y = 0;
  link_actual_vel_x = 0;
  link_actual_vel_z = 0;
  BYTE(link_z_coord) = 0;
  draw_water_ripples_or_grass = 0;
  byte_7E0316 = 0;
  countdown_for_blink = 0;
  link_player_handler_state = 0;
  link_visibility_status = 0;
  Ancilla_TerminateSelectInteractives(0);
  Link_ResetProperties_A();
}

void SpawnHammerWaterSplash() {  // 9aff3c
  static const int8 kItem_Hammer_SpawnWater_X[4] = { 0, 12, -8, 24 };
  static const int8 kItem_Hammer_SpawnWater_Y[4] = { 8, 32, 24, 24 };
  if (submodule_index | flag_is_link_immobilized | flag_unk1)
    return;
  int i = link_direction_facing >> 1;
  uint16 x = link_x_coord + kItem_Hammer_SpawnWater_X[i];
  uint16 y = link_y_coord + kItem_Hammer_SpawnWater_Y[i];
  uint8 tiletype;
  if (player_is_indoors) {
    int t = (link_is_on_lower_level >= 1) ? 0x1000 : 0;
    t += (x & 0x1f8) >> 3;
    t += (y & 0x1f8) << 3;
    tiletype = dung_bg2_attr_table[t];
  } else {
    tiletype = Overworld_ReadTileAttribute(x >> 3, y);
  }

  if (tiletype == 8 || tiletype == 9) {
    int j = Sprite_SpawnSmallSplash(0);
    if (j >= 0) {
      Sprite_SetX(j, x - 8);
      Sprite_SetY(j, y - 16);
      sprite_floor[j] = link_is_on_lower_level;
      sprite_z[j] = 0;
    }
  }
}

void DiggingGameGuy_AttemptPrizeSpawn() {  // 9dfd5c
  static const int8 kDiggingGameGuy_Xvel[2] = { -16, 16 };
  static const int8 kDiggingGameGuy_X[2] = { 0, 19 };
  static const uint8 kDiggingGameGuy_Items[4] = { 0xdb, 0xda, 0xd9, 0xdf };

  beamos_x_hi[1]++;
  if (link_y_coord >= 0xb18)
    return;
  int j = GetRandomNumber() & 7;
  uint8 item_to_spawn;
  switch (j) {
  case 0: case 1: case 2: case 3:
    item_to_spawn = kDiggingGameGuy_Items[j];
    break;
  case 4:
    if (beamos_x_hi[1] < 25 || beamos_x_hi[0] || GetRandomNumber() & 3)
      return;
    item_to_spawn = beamos_x_hi[0] = 0xeb;
    break;
  default:
    return;
  }
  SpriteSpawnInfo info;
  j = Sprite_SpawnDynamically(4, item_to_spawn, &info); // zelda bug: 4 wtf...
  if (j >= 0) {
    int i = link_direction_facing != 4;
    sprite_x_vel[j] = kDiggingGameGuy_Xvel[i];
    sprite_y_vel[j] = 0;
    sprite_z_vel[j] = 24;
    sprite_stunned[j] = 255;
    sprite_delay_aux4[j] = 48;
    Sprite_SetX(j, (link_x_coord + kDiggingGameGuy_X[i]) & ~0xf);
    Sprite_SetY(j, (link_y_coord + 22) & ~0xf);
    sprite_floor[j] = 0;
    SpriteSfx_QueueSfx3WithPan(j, 0x30);
  }
}

