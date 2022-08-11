#include "tile_detect.h"
#include "zelda_rtl.h"
#include "ancilla.h"
#include "variables.h"
#include "overworld.h"

static const uint8 kDetectTiles_tab0[] = { 8, 24, 0, 15 };
static const uint8 kDetectTiles_tab1[] = { 0, 0, 8, 8 };
static const uint8 kDetectTiles_tab2[] = { 8, 8, 16, 16 };
static const uint8 kDetectTiles_tab3[] = { 15, 15, 23, 23 };
static const int8 kDetectTiles_tab4[] = { 7, 24, -1, 16 };
static const uint8 kDetectTiles_tab5[] = { 0, 0, 8, 8 };
static const uint8 kDetectTiles_tab6[] = { 15, 15, 23, 23 };
uint8 Overworld_GetTileAttributeAtLocation(uint16 x, uint16 y) {  // 80882e
  uint16 t;

  t = ((y - overworld_offset_base_y) & overworld_offset_mask_y) * 8;
  t |= ((x - overworld_offset_base_x) & overworld_offset_mask_x);
  t = overworld_tileattr[t >> 1] * 4;
  t |= (y & 8) >> 2;
  t |= (x & 1);
  t = GetMap16toMap8Table()[t];

  uint8 rv = GetMap8toTileAttr()[t & 0x1ff];
  if (rv >= 0x10 && rv < 0x1C) {
    rv |= (t >> 14) & 1;
  }
  return rv;
}

void TileDetect_Movement_Y(uint16 direction) {  // 87cdcb
  assert(direction < 4);
  TileDetect_ResetState();
  tiledetect_pit_tile = 0;

  tiledetect_which_y_pos[0] = (link_y_coord + kDetectTiles_tab0[direction]);
  uint16 y = tiledetect_which_y_pos[0] & tilemap_location_calc_mask;
  uint16 x0 = ((link_x_coord + kDetectTiles_tab1[direction]) & tilemap_location_calc_mask) >> 3;
  uint16 x1 = ((link_x_coord + kDetectTiles_tab2[direction]) & tilemap_location_calc_mask) >> 3;
  uint16 x2 = ((link_x_coord + kDetectTiles_tab3[direction]) & tilemap_location_calc_mask) >> 3;
  scratch_1 = x2;
  TileDetection_Execute(x0, y, 1);
  TileDetection_Execute(x1, y, 2);
  TileDetection_Execute(x2, y, 4);
}

void TileDetect_Movement_X(uint16 direction) {  // 87ce2a
  assert(direction < 4);
  TileDetect_ResetState();
  tiledetect_pit_tile = 0;

  uint16 x = ((link_x_coord + kDetectTiles_tab0[direction]) & tilemap_location_calc_mask) >> 3;
  uint16 y0 = ((link_y_coord + kDetectTiles_tab1[direction]) & tilemap_location_calc_mask);
  tiledetect_which_y_pos[0] = link_y_coord + kDetectTiles_tab2[direction];
  uint16 y1 = tiledetect_which_y_pos[0] & tilemap_location_calc_mask;
  tiledetect_which_y_pos[1] = link_y_coord + kDetectTiles_tab3[direction];
  uint16 y2 = tiledetect_which_y_pos[1] & tilemap_location_calc_mask;
  TileDetection_Execute(x, y0, 1);
  TileDetection_Execute(x, y1, 2);
  TileDetection_Execute(x, y2, 4);
}

void TileDetect_Movement_VerticalSlopes(uint16_t direction) {  // 87ce85
  assert(direction < 4);
  TileDetect_ResetState();
  tiledetect_pit_tile = 0;

  uint16 y = (link_y_coord + kDetectTiles_tab4[direction]) & tilemap_location_calc_mask;
  uint16 x0 = ((link_x_coord + kDetectTiles_tab5[direction]) & tilemap_location_calc_mask) >> 3;
  uint16 x1 = ((link_x_coord + kDetectTiles_tab6[direction]) & tilemap_location_calc_mask) >> 3;
  TileDetection_Execute(x0, y, 1);
  TileDetection_Execute(x1, y, 2);
}

void TileDetect_Movement_HorizontalSlopes(uint16_t direction) {  // 87cec9
  assert(direction < 4);
  TileDetect_ResetState();
  tiledetect_pit_tile = 0;

  uint16 x = ((link_x_coord + kDetectTiles_tab4[direction]) & tilemap_location_calc_mask) >> 3;
  uint16 y0 = ((link_y_coord + kDetectTiles_tab5[direction]) & tilemap_location_calc_mask);
  uint16 y1 = ((link_y_coord + kDetectTiles_tab6[direction]) & tilemap_location_calc_mask);
  TileDetection_Execute(x, y0, 1);
  TileDetection_Execute(x, y1, 2);
}

void Player_TileDetectNearby() {  // 87cf12
  TileDetect_ResetState();
  tiledetect_pit_tile = 0;

  uint16 x0 = ((link_x_coord + kDetectTiles_tab1[0]) & tilemap_location_calc_mask) >> 3;
  uint16 x1 = ((link_x_coord + kDetectTiles_tab3[0]) & tilemap_location_calc_mask) >> 3;

  uint16 y0 = ((link_y_coord + kDetectTiles_tab1[2]) & tilemap_location_calc_mask);
  uint16 y1 = ((link_y_coord + kDetectTiles_tab3[2]) & tilemap_location_calc_mask);

  scratch_1 = y0;

  TileDetection_Execute(x0, y0, 8);
  TileDetection_Execute(x0, y1, 2);
  TileDetection_Execute(x1, y0, 4);
  TileDetection_Execute(x1, y1, 1);
}

void Hookshot_CheckTileCollision(int k) {  // 87d576
  uint8 bak0 = BYTE(dungeon_room_index);
  uint8 bak1 = link_is_on_lower_level;

  if (ancilla_arr1[k]) {
    if (!BYTE(kind_of_in_room_staircase))
      BYTE(dungeon_room_index) += 0x10;
    link_is_on_lower_level ^= 1;
  }

  uint16 x = Ancilla_GetX(k), y = Ancilla_GetY(k);
  int dir = ancilla_dir[k];

  tiledetect_pit_tile = 0;
  TileDetect_ResetState();
  if (dung_hdr_collision == 2) {
    link_is_on_lower_level = 1;
    Hookshot_CheckSingleLayerTileCollision(x + BG1HOFS_copy2 - BG2HOFS_copy2, y + BG1VOFS_copy2 - BG2VOFS_copy2, dir);
    link_is_on_lower_level = 0;
  }
  Hookshot_CheckSingleLayerTileCollision(x, y, dir);

  link_is_on_lower_level = bak1;
  BYTE(dungeon_room_index) = bak0;
}

void Hookshot_CheckSingleLayerTileCollision(uint16 x, uint16 y, int dir) {  // 87d607
  static const uint8 kHookShot_CheckColl_X[8] = { 0, 15, 0, 15, 0, 0, 8, 8 };
  static const uint8 kHookShot_CheckColl_Y[8] = { 0, 0, 7, 7, 0, 15, 0, 15 };
  uint16 y0 = (y + kHookShot_CheckColl_Y[dir * 2 + 0]) & tilemap_location_calc_mask;
  uint16 y1 = (y + kHookShot_CheckColl_Y[dir * 2 + 1]) & tilemap_location_calc_mask;
  uint16 x0 = ((x + kHookShot_CheckColl_X[dir * 2 + 0]) & tilemap_location_calc_mask) >> 3;
  uint16 x1 = ((x + kHookShot_CheckColl_X[dir * 2 + 1]) & tilemap_location_calc_mask) >> 3;
  TileDetection_Execute(x0, y0, 1);
  TileDetection_Execute(x1, y1, 2);
}

void HandleNudgingInADoor(int8 speed) {  // 87d667
  uint8 y;

  if (link_last_direction_moved_towards & 2) {
    y = (uint8)link_y_coord < 0x80 ? 1 : 0;
  } else {
    y = (uint8)link_x_coord < 0x80 ? 3 : 2;
  }
  tiledetect_pit_tile = 0;
  TileDetect_ResetState();

  static const int8 kDetectTiles_7_Y[] = { 8, 23, 16, 16 };
  static const int8 kDetectTiles_7_X[] = { 8, 8, 0, 15 };

  uint16 x0 = ((link_x_coord + kDetectTiles_7_X[y]) & tilemap_location_calc_mask) >> 3;
  uint16 y0 = ((link_y_coord + kDetectTiles_7_Y[y]) & tilemap_location_calc_mask);

  TileDetection_Execute(x0, y0, 1);

  if (((R14 | detection_of_ledge_tiles_horiz_uphoriz) & 3) == 0) {
    if (((tiledetect_vertical_ledge | detection_of_unknown_tile_types) & 0x33) == 0)
      return;
  }

  if (link_last_direction_moved_towards & 2)
    link_y_coord -= speed;
  else
    link_x_coord -= speed;
}

void TileCheckForMirrorBonk() {  // 87d6f4
  tiledetect_pit_tile = 0;
  TileDetect_ResetState();

  uint16 x0 = ((link_x_coord + 2) & tilemap_location_calc_mask) >> 3;
  uint16 x1 = ((link_x_coord + 13) & tilemap_location_calc_mask) >> 3;

  uint16 y0 = ((link_y_coord + 10) & tilemap_location_calc_mask);
  uint16 y1 = ((link_y_coord + 21) & tilemap_location_calc_mask);

  scratch_1 = y0;

  TileDetection_Execute(x0, y0, 8);
  TileDetection_Execute(x0, y1, 2);
  TileDetection_Execute(x1, y0, 4);
  TileDetection_Execute(x1, y1, 1);

}

// Used when holding sword in doorway
void TileDetect_SwordSwingDeepInDoor(uint8 dw) {  // 87d73e
  tiledetect_pit_tile = 0;
  TileDetect_ResetState();

  static const int8 kDoorwayDetectX[] = { 8, 8, -1, 16 };
  static const int8 kDoorwayDetectY[] = { -1, 24, 16, 16 };
  int o = (dw - 1) * 2;
  uint16 x0 = ((link_x_coord + kDoorwayDetectX[o + 0]) & tilemap_location_calc_mask) >> 3;
  uint16 x1 = ((link_x_coord + kDoorwayDetectX[o + 1]) & tilemap_location_calc_mask) >> 3;

  uint16 y0 = ((link_y_coord + kDoorwayDetectY[o + 0]) & tilemap_location_calc_mask);
  uint16 y1 = ((link_y_coord + kDoorwayDetectY[o + 1]) & tilemap_location_calc_mask);

  TileDetection_Execute(x0, y0, 1);
  TileDetection_Execute(x1, y1, 2);
}

void TileDetect_ResetState() {  // 87d798
  R12 = 0;
  R14 = 0;
  tiledetect_diagonal_tile = 0;
  tiledetect_stair_tile = 0;
  tiledetect_pit_tile = 0;
  tiledetect_inroom_staircase = 0;
  tiledetect_var2 = 0;
  tiledetect_var1 = 0;
  tiledetect_moving_floor_tiles = 0;
  tiledetect_deepwater = 0;
  tiledetect_normal_tiles = 0;
  tiledetect_icy_floor = 0;
  tiledetect_water_staircase = 0;
  tiledetect_thick_grass = 0;
  tiledetect_shallow_water = 0;
  tiledetect_destruction_aftermath = 0;
  tiledetect_read_something = 0;
  tiledetect_vertical_ledge = 0;
  detection_of_ledge_tiles_horiz_uphoriz = 0;
  tiledetect_ledges_down_leftright = 0;
  detection_of_unknown_tile_types = 0;
  tiledetect_chest = 0;
  tiledetect_key_lock_gravestones = 0;
  bitfield_spike_cactus_tiles = 0;
  tiledetect_spike_floor_and_tile_triggers = 0;
  bitmask_for_dashable_tiles = 0;
  tiledetect_misc_tiles = 0;
  tiledetect_var4 = 0;
}

void TileDetection_Execute(uint16 x, uint16 y, uint16 bits) {  // 87d9d8
  uint8 tile;
  uint16 offs = 0;
  if (player_is_indoors) {
    force_move_any_direction = force_move_any_direction & 0xff;
    offs = (y & ~7) * 8 + (x & 63) + (link_is_on_lower_level ? 0x1000 : 0);
    tile = dung_bg2_attr_table[offs];
    if (cheatWalkThroughWalls)
      tile = 0;
    link_tile_below = tile;
  } else {
    tile = Overworld_GetTileAttributeAtLocation(x, y);
  }
  TileDetect_ExecuteInner(tile, offs, bits, player_is_indoors);
}

void TileDetect_ExecuteInner(uint8 tile, uint16 offs, uint16 bits, bool is_indoors) {  // 87dc2e
  static const uint8 word_87DC55[] = { 4, 0, 6, 2 };
  if (cheatWalkThroughWalls)
    tile = 0;

  switch (tile) {
  case 0x00: case 0x05: case 0x06: case 0x07: case 0x14: case 0x15: case 0x16: case 0x17: case 0x21: case 0x23: case 0x24: case 0x25: case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x41: case 0x45: case 0x47: case 0x49: case 0x5e: case 0x5f: case 0x61: case 0x62: case 0x64: case 0x65: case 0x66: case 0xa6: case 0xa7: case 0xbe: case 0xbf: case 0xd0: case 0xd1: case 0xd2: case 0xd3: case 0xd4: case 0xd5: case 0xd6: case 0xd7: case 0xd8: case 0xd9: case 0xda: case 0xdb: case 0xdc: case 0xdd: case 0xde: case 0xdf: case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5: case 0xe6: case 0xe7: case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee: case 0xef:  // TileBehavior_NothingOW
    if (!is_indoors)
      tiledetect_normal_tiles |= bits;
    break;
  case 0x01: case 0x02: case 0x03:  // TileBehavior_StandardCollision
  case 0x26: case 0x43:
    R14 |= bits;
    break;
  case 0x6c: case 0x6d: case 0x6e: case 0x6f:
    if (is_indoors)
      R14 |= bits;
    else
      tiledetect_normal_tiles |= bits;
    break;
  case 0x04:
    if (is_indoors) {
      R14 |= bits;
    } else {
      tiledetect_thick_grass |= bits;
    }
    break;
  case 0x0b:
    if (is_indoors) {
      R14 |= bits;
    } else {
      index_of_interacting_tile = tile;
      tiledetect_deepwater |= bits << 4;
    }
    break;
  case 0x08:  // TileBehavior_DeepWater
    tiledetect_deepwater |= bits;
    break;
  case 0x09:  // TileBehavior_ShallowWater
    tiledetect_shallow_water |= bits;
    break;
  case 0x0a:  // TileBehavior_ShortWaterLadder
    tiledetect_normal_tiles |= bits;
    break;
  case 0x0c:  // TileBehavior_OverlayMask_0C
    tiledetect_moving_floor_tiles |= bits;
    break;
  case 0x0d:  // TileBehavior_SpikeFloor
    if (!flag_block_link_menu && !(dung_savegame_state_bits & 0x8000))
      tiledetect_spike_floor_and_tile_triggers |= bits << 4;
    break;
  case 0x0e:  // TileBehavior_GanonIce
    tiledetect_icy_floor |= bits;
    break;
  case 0x0f:  // TileBehavior_PalaceIce
    tiledetect_icy_floor |= bits << 4;
    break;
  case 0x10: case 0x11: case 0x12: case 0x13:  // TileBehavior_Slope
    R12 |= bits;
    tiledetect_diag_state = word_87DC55[tile & 3];
    break;
  case 0x18: case 0x19: case 0x1a: case 0x1b:  // TileBehavior_SlopeOuter
    tiledetect_diagonal_tile |= bits;
    R12 |= bits;
    tiledetect_diag_state = word_87DC55[tile & 3];
    break;
  case 0x1c:  // TileBehavior_OverlayMask_1C
    tiledetect_water_staircase |= bits;
    break;
  case 0x1d:  // TileBehavior_NorthSingleLayerStairs
    index_of_interacting_tile = tile;
    tiledetect_inroom_staircase |= bits;
    tiledetect_stair_tile |= bits;
    break;
  case 0x1e: case 0x1f:  // TileBehavior_NorthSwapLayerStairs
    index_of_interacting_tile = tile;
    tiledetect_inroom_staircase |= bits;
    tiledetect_stair_tile |= bits;
    break;
  case 0x20: case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7: case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd:  // TileBehavior_Pit
    if (!player_on_somaria_platform)
      tiledetect_pit_tile |= bits;
    break;
  case 0x22: case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:  // TileHandlerIndoor_22
    tiledetect_stair_tile |= bits;
    break;
  case 0x27:  // TileBehavior_Hookshottables
    R14 |= bits;
    tiledetect_misc_tiles |= bits;
    break;
  case 0x28:  // TileBehavior_Ledge_North
    index_of_interacting_tile = tile;
    tiledetect_vertical_ledge |= bits;
    break;
  case 0x29:  // TileBehavior_Ledge_South
    index_of_interacting_tile = tile;
    tiledetect_vertical_ledge |= bits << 4;
    break;
  case 0x2a: case 0x2b:  // TileBehavior_Ledge_EastWest
    index_of_interacting_tile = tile;
    detection_of_ledge_tiles_horiz_uphoriz |= bits;
    break;
  case 0x2c: case 0x2e:  // TileBehavior_Ledge_NorthDiagonal
    index_of_interacting_tile = tile;
    detection_of_ledge_tiles_horiz_uphoriz |= bits << 4;
    break;
  case 0x2d: case 0x2f:  // TileBehavior_Ledge_SouthDiagonal
    index_of_interacting_tile = tile;
    tiledetect_ledges_down_leftright |= bits;
    break;
  case 0x3d: case 0x3e: case 0x3f:  // TileHandlerIndoor_3E
    index_of_interacting_tile = tile;
    tiledetect_inroom_staircase |= bits << 4;
    tiledetect_stair_tile |= bits;
    break;
  case 0x40:  // TileBehavior_ThickGrass
    tiledetect_thick_grass |= bits;
    break;
  case 0x44:  // TileBehavior_Spike
    if (!flag_block_link_menu && !(dung_savegame_state_bits & 0x8000))
      bitfield_spike_cactus_tiles |= bits;
    else
      R14 |= bits;
    break;
  case 0x46:  // TileBehavior_HylianPlaque
    tiledetect_spike_floor_and_tile_triggers |= bits;
    R14 |= bits;
    break;
  case 0x48: case 0x4a:  // TileBehavior_DiggableGround
    tiledetect_destruction_aftermath |= bits;
    tiledetect_normal_tiles |= bits;
    break;
  case 0x4b:  // TileBehavior_Warp
    tiledetect_thick_grass |= bits << 4;
    break;
  case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: { // TileBehavior_Liftable
    static const uint8 kTile50data[] = { 0x54, 0x52, 0x50, 0x51, 0x53, 0x55, 0x56 };
    for (int i = 6; i >= 0; i--) {
      if (kTile50data[i] == tile) {
        if (tile == 0x50 || tile == 0x51)
          bitmask_for_dashable_tiles |= bits << 4;
        tiledetect_read_something |= bits;
        interacting_with_liftable_tile_x2 = i * 2;
        R14 |= bits;
        tiledetect_misc_tiles |= bits;
        break;
      }
    }
    break;
  }
  case 0x57:  // TileBehavior_BonkRocks
    R14 |= bits;
    bitmask_for_dashable_tiles |= bits << 4;
    break;
  case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d:  // TileBehavior_Chest
    tiledetect_misc_tiles |= bits;
    index_of_interacting_tile = tile;
    if (dung_chest_locations[tile - 0x58] >= 0x8000) {
      R14 |= bits;
      tiledetect_key_lock_gravestones |= bits << 4;
      if (bits & 2)
        tiledetect_tile_type = tile;
    } else {
      tiledetect_chest |= bits;  // small key lock
      R14 |= bits;
    }
    break;
  case 0x60:  // TileBehavior_RupeeTile
    if (is_indoors) {
      if (dung_bg2_attr_table[offs + 64] == 0x60) {
        tiledetect_misc_tiles |= bits << 8;
      } else {
        tiledetect_misc_tiles |= bits << 12;
      }
    } else {
      tiledetect_normal_tiles |= bits;
    }
    break;
  case 0x63:  // TileBehavior_MinigameChest
    tiledetect_misc_tiles |= bits;
    index_of_interacting_tile = tile;
    tiledetect_chest |= bits;  // small key lock
    R14 |= bits;
    break;
  case 0x67:  // TileBehavior_CrystalPeg_Up
    R14 |= bits;
    tiledetect_misc_tiles |= bits;
    bitfield_spike_cactus_tiles |= bits << 4;
    break;
  case 0x68:  // TileBehavior_Conveyor_Upwards
    tiledetect_var4 |= bits;
    break;
  case 0x69:  // TileBehavior_Conveyor_Downwards
    tiledetect_var4 |= bits << 4;
    break;
  case 0x6a:  // TileBehavior_Conveyor_Leftwards
    tiledetect_var4 |= bits << 8;
    break;
  case 0x6b:  // TileBehavior_Conveyor_Rightwards
    tiledetect_var4 |= bits << 12;
    break;
  case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77: case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:  // TileBehavior_ManipulablyReplaced
    if (bits & 2)
      tiledetect_var2 |= 1 << (tile & 0xf);
    R14 |= bits;
    tiledetect_misc_tiles |= bits;
    break;
  case 0x80: case 0x81: case 0x84: case 0x85: case 0x86: case 0x87: case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d:  // TileHandlerIndoor_80
    R14 |= bits << 4;
    tiledetect_var1 = 2 * (tile & 1);
    break;
  case 0x82: case 0x83:  // TileHandlerIndoor_82
    R14 |= (bits << 4) | (bits << 8);
    tiledetect_var1 = 2 * (tile & 1);
    break;
  case 0x8e: case 0x8f:  // TileBehavior_Entrance
    R14 |= bits << 4;
    bitmask_for_dashable_tiles |= bits;
    tiledetect_var1 = 0;
    break;
  case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:  // TileBehavior_LayerToggleShutterDoor
    room_transitioning_flags = 1;
    R14 |= (bits << 4) | (bits << 8);
    tiledetect_var1 = 2 * (tile & 1);
    break;
  case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f: case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:  // TileBehavior_LayerAndDungeonToggleShutterDoor
    room_transitioning_flags = 3;
    R14 |= (bits << 4) | (bits << 8);
    tiledetect_var1 = 2 * (tile & 1);
    break;
  case 0xa0: case 0xa1: case 0xa4: case 0xa5:  // TileBehavior_DungeonToggleManualDoor
    room_transitioning_flags = 2;
    R14 |= bits << 4;
    tiledetect_var1 = 2 * (tile & 1);
    break;
  case 0xa2: case 0xa3:  // TileBehavior_DungeonToggleShutterDoor
    room_transitioning_flags = 2;
    R14 |= (bits << 4) | (bits << 8);
    tiledetect_var1 = 2 * (tile & 1);
    break;
  case 0xc0: case 0xc1: case 0xc2: case 0xc3: case 0xc4: case 0xc5: case 0xc6: case 0xc7: case 0xc8: case 0xc9: case 0xca: case 0xcb: case 0xcc: case 0xcd: case 0xce: case 0xcf:  // TileBehavior_LightableTorch
    R14 |= bits;
    tiledetect_misc_tiles |= bits;
    break;
  case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7: case 0xf8: case 0xf9: case 0xfa: case 0xfb: case 0xfc: case 0xfd: case 0xfe: case 0xff:  // TileBehavior_FlaggableDoor
    R14 |= bits;
    tiledetect_misc_tiles |= bits << 4;
    break;

  case 0x42:  // TileBehavior_GraveStone
    if (!is_indoors) {
      tiledetect_key_lock_gravestones |= bits;
      R14 |= bits;
    }
    break;
  case 0x4c: case 0x4d:  // TileBehavior_UnusedCornerType
    if (!is_indoors) {
      index_of_interacting_tile = tile;
      detection_of_unknown_tile_types |= bits;
    }
    break;
  case 0x4e: case 0x4f:  // TileBehavior_EasternRuinsCorner
    if (!is_indoors) {
      index_of_interacting_tile = tile;
      detection_of_unknown_tile_types |= bits << 4;
    }
    break;
  default:
    assert(0);
  }
}

