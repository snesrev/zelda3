#include "overlord.h"
#include "sprite.h"
#include "misc.h"
#include "sprite_main.h"

uint16 Overlord_GetX(int k) { return (overlord_x_lo[k] | overlord_x_hi[k] << 8); }
uint16 Overlord_GetY(int k) { return (overlord_y_lo[k] | overlord_y_hi[k] << 8); }
static HandlerFuncK *const kOverlordFuncs[26] = {
  &Overlord01_PositionTarget,
  &Overlord02_FullRoomCannons,
  &Overlord03_VerticalCannon,
  &Overlord_StalfosFactory,
  &Overlord05_FallingStalfos,
  &Overlord06_BadSwitchSnake,
  &Overlord07_MovingFloor,
  &Overlord08_BlobSpawner,
  &Overlord09_WallmasterSpawner,
  &Overlord0A_FallingSquare,
  &Overlord0A_FallingSquare,
  &Overlord0A_FallingSquare,
  &Overlord0A_FallingSquare,
  &Overlord0A_FallingSquare,
  &Overlord0A_FallingSquare,
  &Overlord10_PirogusuSpawner_left,
  &Overlord10_PirogusuSpawner_left,
  &Overlord10_PirogusuSpawner_left,
  &Overlord10_PirogusuSpawner_left,
  &Overlord14_TileRoom,
  &Overlord15_WizzrobeSpawner,
  &Overlord16_ZoroSpawner,
  &Overlord17_PotTrap,
  &Overlord18_InvisibleStalfos,
  &Overlord19_ArmosCoordinator_bounce,
  &Overlord06_BadSwitchSnake,
};
static inline uint8 ArmosMult(uint16 a, uint8 b);
static inline int8 ArmosSin(uint16 a, uint8 b);
void Overlord_StalfosFactory(int k) {
  // unused
  assert(0);
}

void Overlord_SetX(int k, uint16 v) {
  overlord_x_lo[k] = v;
  overlord_x_hi[k] = v >> 8;
}

void Overlord_SetY(int k, uint16 v) {
  overlord_y_lo[k] = v;
  overlord_y_hi[k] = v >> 8;
}

static inline uint8 ArmosMult(uint16 a, uint8 b) {
  if (a >= 256)
    return b;
  int p = a * b;
  return (p >> 8) + (p >> 7 & 1);
}

static inline int8 ArmosSin(uint16 a, uint8 b) {
  uint8 t = ArmosMult(kSinusLookupTable[a & 0xff], b);
  return (a & 0x100) ? -t : t;
}

void Overlord_SpawnBoulder() {  // 89b714
  if (player_is_indoors || !byte_7E0FFD || (submodule_index | flag_unk1) || ++byte_7E0FFE & 63)
    return;

  if (sign8((BG2VOFS_copy2 >> 8) - (sprcoll_y_base >> 8) - 2))
    return;

  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(0, 0xc2, &info);
  if (j >= 0) {
    Sprite_SetX(j, BG2HOFS_copy2 + (GetRandomNumber() & 127) + 64);
    Sprite_SetY(j, BG2VOFS_copy2 - 0x30);
    sprite_floor[j] = 0;
    sprite_D[j] = 0;
    sprite_z[j] = 0;
  }
}

void Overlord_Main() {  // 89b773
  Overlord_ExecuteAll();
  Overlord_SpawnBoulder();
}

void Overlord_ExecuteAll() {  // 89b77e
  if (submodule_index | flag_unk1)
    return;
  for (int i = 7; i >= 0; i--) {
    if (overlord_type[i])
      Overlord_ExecuteSingle(i);
  }
}

void Overlord_ExecuteSingle(int k) {  // 89b793
  int j = overlord_type[k];
  Overlord_CheckIfActive(k);
  kOverlordFuncs[j - 1](k);
}

void Overlord19_ArmosCoordinator_bounce(int k) {  // 89b7dc
  static const uint8 kArmosCoordinator_BackWallX[6] = { 49, 77, 105, 131, 159, 187 };

  if (overlord_gen2[k])
    overlord_gen2[k]--;
  switch (overlord_gen1[k]) {
  case 0:  // wait for knight activation
    if (sprite_A[0]) {
      overlord_x_lo[k] = 120;
      overlord_floor[k] = 255;
      overlord_x_lo[2] = 64;
      overlord_x_lo[0] = 192;
      overlord_x_lo[1] = 1;
      ArmosCoordinator_RotateKnights(k);
    }
    break;
  case 1:  // wait knight under coercion
    if (ArmosCoordinator_CheckKnights()) {
      overlord_gen1[k]++;
      overlord_gen2[k] = 0xff;
    }
    break;
  case 2:  // timed rotate then transition
  case 4:
    ArmosCoordinator_RotateKnights(k);
    break;
  case 3:  // radial contraction
    if (--overlord_x_lo[2] == 32) {
      overlord_gen1[k]++;
      overlord_gen2[k] = 64;
    }
    ArmosCoordinator_Rotate(k);
    break;
  case 5:  // radial dilation
    if (++overlord_x_lo[2] == 64) {
      overlord_gen1[k]++;
      overlord_gen2[k] = 64;
    }
    ArmosCoordinator_Rotate(k);
    break;
  case 6:  // order knights to back wall
    if (overlord_gen2[k])
      return;
    ArmosCoordinator_DisableCoercion(k);
    for (int j = 5; j >= 0; j--) {
      overlord_x_hi[j] = kArmosCoordinator_BackWallX[j];
      overlord_gen2[j] = 48;
    }
    overlord_gen1[k]++;
    overlord_gen2[k] = 255;
    break;
  case 7:  // cascade knights to front wall
    if (overlord_gen2[k])
      return;
    for (int j = 5; j >= 0; j--) {
      if (++overlord_gen2[j] == 192) {
        overlord_gen1[k] = 1;
        overlord_floor[k] = -overlord_floor[k];
        ArmosCoordinator_DisableCoercion(k);
        ArmosCoordinator_Rotate(k);
        return;
      }
    }
    break;
  }
}

void Overlord18_InvisibleStalfos(int k) {  // 89b7f5
  static const int8 kRedStalfosTrap_X[4] = { 0, 0, -48, 48 };
  static const int8 kRedStalfosTrap_Y[4] = { -40, 56, 8, 8 };
  static const uint8 kRedStalfosTrap_Delay[4] = { 0x30, 0x50, 0x70, 0x90 };

  uint16 x = (overlord_x_lo[k] | overlord_x_hi[k] << 8);
  uint16 y = (overlord_y_lo[k] | overlord_y_hi[k] << 8);
  if ((uint16)(x - link_x_coord + 24) >= 48 || (uint16)(y - link_y_coord + 24) >= 48)
    return;
  overlord_type[k] = 0;
  tmp_counter = 3;
  do {
    SpriteSpawnInfo info;
    int j = Sprite_SpawnDynamicallyEx(k, 0xa7, &info, 12);
    if (j < 0)
      return;
    Sprite_SetX(j, link_x_coord + kRedStalfosTrap_X[tmp_counter]);
    Sprite_SetY(j, link_y_coord + kRedStalfosTrap_Y[tmp_counter]);
    sprite_delay_main[j] = kRedStalfosTrap_Delay[tmp_counter];
    sprite_floor[j] = overlord_floor[k];
    sprite_E[j] = 1;
    sprite_flags2[j] = 3;
    sprite_D[j] = 2;
  } while (!sign8(--tmp_counter));
}

void Overlord17_PotTrap(int k) {  // 89b884
  uint16 x = (overlord_x_lo[k] | overlord_x_hi[k] << 8);
  uint16 y = (overlord_y_lo[k] | overlord_y_hi[k] << 8);
  if ((uint16)(x - link_x_coord + 32) < 64 &&
      (uint16)(y - link_y_coord + 32) < 64) {
    overlord_type[k] = 0;
    byte_7E0B9E++;
  }
}

void Overlord16_ZoroSpawner(int k) {  // 89b8d1
  static const int8 kOverlordZoroFactory_X[8] = { -4, -2, 0, 2, 4, 6, 8, 12 };
  overlord_gen2[k]--;
  uint16 x = Overlord_GetX(k) + 8;
  uint16 y = Overlord_GetY(k) + 8;
  if (GetTileAttribute(overlord_floor[k], &x, y) != 0x82)
    return;
  if (overlord_gen2[k] >= 0x18 || (overlord_gen2[k] & 3) != 0)
    return;
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamicallyEx(k, 0x9c, &info, 12);
  if (j >= 0) {
    Sprite_SetX(j, info.r5_overlord_x + kOverlordZoroFactory_X[GetRandomNumber() & 7] + 8);
    sprite_y_lo[j] = info.r7_overlord_y + 8;
    sprite_y_hi[j] = info.r7_overlord_y >> 8;
    sprite_floor[j] = overlord_floor[k];
    sprite_flags4[j] = 1;
    sprite_E[j] = 1;
    sprite_ignore_projectile[j] = 1;
    sprite_y_vel[j] = 16;
    sprite_flags2[j] = 32;
    sprite_oam_flags[j] = 13;
    sprite_subtype2[j] = GetRandomNumber();
    sprite_delay_main[j] = 48;
    sprite_bump_damage[j] = 3;
  }
}

void Overlord15_WizzrobeSpawner(int k) {  // 89b986
  static const int8 kOverlordWizzrobe_X[4] = { 48, -48, 0, 0 };
  static const int8 kOverlordWizzrobe_Y[4] = { 16, 16, 64, -32 };
  static const uint8 kOverlordWizzrobe_Delay[4] = { 0, 16, 32, 48 };
  if (overlord_gen2[k] != 128) {
    if (frame_counter & 1)
      overlord_gen2[k]--;
    return;
  }
  overlord_gen2[k] = 127;
  for (int i = 3; i >= 0; i--) {
    SpriteSpawnInfo info;
    int j = Sprite_SpawnDynamicallyEx(k, 0x9b, &info, 12);
    if (j >= 0) {
      Sprite_SetX(j, link_x_coord + kOverlordWizzrobe_X[i]);
      Sprite_SetY(j, link_y_coord + kOverlordWizzrobe_Y[i]);
      sprite_delay_main[j] = kOverlordWizzrobe_Delay[i];
      sprite_floor[j] = overlord_floor[k];
      sprite_B[j] = 1;
    }
  }
  tmp_counter = 0xff;
}

void Overlord14_TileRoom(int k) {  // 89b9e8
  uint16 x = (overlord_x_lo[k] | overlord_x_hi[k] << 8) - BG2HOFS_copy2;
  uint16 y = (overlord_y_lo[k] | overlord_y_hi[k] << 8) - BG2VOFS_copy2;
  if (x & 0xff00 || y & 0xff00)
    return;
  if (--overlord_gen2[k] != 0x80)
    return;
  int j = TileRoom_SpawnTile(k);
  if (j < 0) {
    overlord_gen2[k] = 0x81;
    return;
  }
  if (++overlord_gen1[k] != 22)
    overlord_gen2[k] = 0xE0;
  else
    overlord_type[k] = 0;
}

int TileRoom_SpawnTile(int k) {  // 89ba56
  static const uint8 kSpawnFlyingTile_X[22] = {
    0x70, 0x80, 0x60, 0x90, 0x90, 0x60, 0x70, 0x80, 0x80, 0x70, 0x50, 0xa0, 0xa0, 0x50, 0x50, 0xa0,
    0xa0, 0x50, 0x70, 0x80, 0x80, 0x70,
  };
  static const uint8 kSpawnFlyingTile_Y[22] = {
    0x80, 0x80, 0x70, 0x90, 0x70, 0x90, 0x60, 0xa0, 0x60, 0xa0, 0x60, 0xb0, 0x60, 0xb0, 0x80, 0x90,
    0x80, 0x90, 0x70, 0x90, 0x70, 0x90,
  };
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0x94, &info);
  if (j < 0)
    return j;
  sprite_E[j] = 1;
  int i = overlord_gen1[k];
  sprite_x_lo[j] = kSpawnFlyingTile_X[i];
  sprite_y_lo[j] = kSpawnFlyingTile_Y[i] - 8;
  sprite_y_hi[j] = overlord_y_hi[k];
  sprite_x_hi[j] = overlord_x_hi[k];
  sprite_floor[j] = overlord_floor[k];
  sprite_health[j] = 4;
  sprite_flags5[j] = 0;
  sprite_health[j] = 0;
  sprite_defl_bits[j] = 8;
  sprite_flags2[j] = 4;
  sprite_oam_flags[j] = 1;
  sprite_bump_damage[j] = 4;
  return j;
}

void Overlord10_PirogusuSpawner_left(int k) {  // 89baac
  static const uint8 kOverlordPirogusu_A[4] = { 2, 3, 0, 1 };

  tmp_counter = overlord_type[k] - 16;
  if (overlord_gen2[k] != 128) {
    overlord_gen2[k]--;
    return;
  }
  overlord_gen2[k] = (GetRandomNumber() & 31) + 96;
  int n = 0;
  for (int i = 0; i != 16; i++) {
    if (sprite_state[i] != 0 && sprite_type[i] == 0x10)
      n++;
  }
  if (n >= 5)
    return;
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamicallyEx(k, 0x94, &info, 12);
  if (j >= 0) {
    Sprite_SetX(j, info.r5_overlord_x);
    Sprite_SetY(j, info.r7_overlord_y);
    sprite_floor[j] = overlord_floor[k];
    sprite_delay_main[j] = 32;
    sprite_D[j] = tmp_counter;
    sprite_A[j] = kOverlordPirogusu_A[tmp_counter];
  }
}

void Overlord0A_FallingSquare(int k) {  // 89bbb2
  static const uint8 kCrumbleTilePathData[108 + 1] = {
    2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 3, 3, 3,
    3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 3, 1, 3, 0, 3, 1,
    3, 0, 3, 1, 3, 0, 3, 1, 3, 0, 3, 1, 3, 0, 3, 1,
    3, 0, 3, 1, 3, 0, 3, 1, 3, 0, 3, 1, 3, 0, 3, 1,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 0xff
  };
  static const uint8 kCrumbleTilePathOffs[7] = {
    0, 25, 66, 77, 87, 98, 108
  };
  static const int8 kCrumbleTilePath_X[4] = { 16, -16, 0, 0 };
  static const int8 kCrumbleTilePath_Y[4] = { 0, 0, 16, -16 };
  if (overlord_gen2[k]) {
    if (overlord_gen3[k]) {
      overlord_gen2[k]--;
      return;
    }
    uint16 x = Overlord_GetX(k) - BG2HOFS_copy2;
    uint16 y = Overlord_GetY(k) - BG2VOFS_copy2;
    if (!(x & 0xff00 || y & 0xff00))
      overlord_gen3[k]++;
    return;
  }

  overlord_gen2[k] = 16;
  SpawnFallingTile(k);
  int j = overlord_type[k] - 10;
  int i = overlord_gen1[k]++;
  if (i == kCrumbleTilePathOffs[j + 1] - kCrumbleTilePathOffs[j]) {
    overlord_type[k] = 0;
  }
  int t = kCrumbleTilePathData[kCrumbleTilePathOffs[j] + i];
  if (t == 0xff) {
    Overlord_SetX(k, Overlord_GetX(k) + 0xc1a);
    Overlord_SetY(k, Overlord_GetY(k) + 0xbb66);
  } else {
    Overlord_SetX(k, Overlord_GetX(k) + kCrumbleTilePath_X[t]);
    Overlord_SetY(k, Overlord_GetY(k) + kCrumbleTilePath_Y[t]);
  }
}

void SpawnFallingTile(int k) {  // 89bc31
  int j = GarnishAlloc();
  if (j >= 0) {
    garnish_type[j] = 3;
    garnish_x_hi[j] = overlord_x_hi[k];
    garnish_x_lo[j] = overlord_x_lo[k];
    sound_effect_1 = CalculateSfxPan_Arbitrary(garnish_x_lo[j]) | 0x1f;
    int y = Overlord_GetY(k) + 16;
    garnish_y_lo[j] = y;
    garnish_y_hi[j] = y >> 8;
    garnish_countdown[j] = 31;
    garnish_active = 31;
  }
}

void Overlord09_WallmasterSpawner(int k) {  // 89bc7b
  if (overlord_gen2[k] != 128) {
    if (!(frame_counter & 1))
      overlord_gen2[k]--;
    return;
  }
  overlord_gen2[k] = 127;
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamicallyEx(k, 0x90, &info, 12);
  if (j < 0)
    return;
  Sprite_SetX(j, link_x_coord);
  Sprite_SetY(j, link_y_coord);
  sprite_z[j] = 208;
  SpriteSfx_QueueSfx2WithPan(j, 0x20);
  sprite_floor[j] = link_is_on_lower_level;
}

void Overlord08_BlobSpawner(int k) {  // 89bcc3
  if (overlord_gen2[k]) {
    overlord_gen2[k]--;
    return;
  }
  overlord_gen2[k] = 0xa0;
  int n = 0;
  for (int i = 0; i != 16; i++) {
    if (sprite_state[i] != 0 && sprite_type[i] == 0x8f)
      n++;
  }
  if (n >= 5)
    return;

  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamicallyEx(k, 0x8f, &info, 12);
  if (j >= 0) {
    static const int8 kOverlordZol_X[4] = { 0, 0, -48, 48 };
    static const int8 kOverlordZol_Y[4] = { -40, 56, 8, 8 };
    int i = link_direction_facing >> 1;
    Sprite_SetX(j, link_x_coord + kOverlordZol_X[i]);
    Sprite_SetY(j, link_y_coord + kOverlordZol_Y[i]);
    sprite_z[j] = 192;
    sprite_floor[j] = link_is_on_lower_level;
    sprite_ai_state[j] = 2;
    sprite_E[j] = 2;
    sprite_C[j] = 2;
    sprite_head_dir[j] = GetRandomNumber() & 31 | 16;
  }
}

void Overlord07_MovingFloor(int k) {  // 89bd3f
  if (sprite_state[0] == 4) {
    overlord_type[k] = 0;
    BYTE(dung_floor_move_flags) = 1;
    return;
  }
  if (!overlord_gen1[k]) {
    if (++overlord_gen2[k] == 32) {
      overlord_gen2[k] = 0;
      BYTE(dung_floor_move_flags) = (GetRandomNumber() & (overlord_x_lo[k] ? 3 : 1)) * 2;
      overlord_gen2[k] = (GetRandomNumber() & 127) + 128;
      overlord_gen1[k]++;
    } else {
      BYTE(dung_floor_move_flags) = 1;
    }
  } else {
    if (!--overlord_gen2[k])
      overlord_gen1[k] = 0;
  }
}

void Sprite_Overlord_PlayFallingSfx(int k) {  // 89bdfd
  SpriteSfx_QueueSfx2WithPan(k, 0x20);
}

void Overlord05_FallingStalfos(int k) {  // 89be0f
  static const uint8 kStalfosTrap_Trigger[8] = { 255, 224, 192, 160, 128, 96, 64, 32 };

  uint16 x = (overlord_x_lo[k] | overlord_x_hi[k] << 8) - BG2HOFS_copy2;
  uint16 y = (overlord_y_lo[k] | overlord_y_hi[k] << 8) - BG2VOFS_copy2;
  if (x & 0xff00 || y & 0xff00)
    return;
  if (overlord_gen1[k] == 0) {
    if (byte_7E0B9E)
      overlord_gen1[k]++;
    return;
  }
  if (overlord_gen1[k]++ == kStalfosTrap_Trigger[k]) {
    overlord_type[k] = 0;
    SpriteSpawnInfo info;
    int j = Sprite_SpawnDynamicallyEx(k, 0x85, &info, 12);
    if (j < 0)
      return;
    Sprite_SetX(j, info.r5_overlord_x);
    Sprite_SetY(j, info.r7_overlord_y);
    sprite_z[j] = 224;
    sprite_floor[j] = overlord_floor[k];
    sprite_D[j] = 0; // zelda bug: unitialized
    Sprite_Overlord_PlayFallingSfx(j);
  }
}

void Overlord06_BadSwitchSnake(int k) {  // 89be75
  static const uint8 kSnakeTrapOverlord_Tab1[8] = { 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80, 0x90 };

  uint8 a = overlord_gen1[k];
  if (a == 0) {
    if (activate_bomb_trap_overlord != 0)
      overlord_gen1[k] = 1;
    return;
  }
  overlord_gen1[k] = a + 1;

  if (a != kSnakeTrapOverlord_Tab1[k])
    return;

  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0x6e, &info);
  if (j < 0)
    return;
  Sprite_SetX(j, info.r5_overlord_x);
  Sprite_SetY(j, info.r7_overlord_y);

  sprite_z[j] = 192;
  sprite_E[j] = 192;

  sprite_flags3[j] |= 0x10;
  sprite_floor[j] = overlord_floor[k];
  SpriteSfx_QueueSfx2WithPan(j, 0x20);
  uint8 type = overlord_type[k];
  overlord_type[k] = 0;
  if (type == 26) {
    sprite_type[j] = 74;
    Sprite_TransmuteToBomb(j);
    sprite_delay_aux1[j] = 112;
  }
}

void Overlord02_FullRoomCannons(int k) {  // 89bf09
  static const uint8 kAllDirectionMetalBallFactory_Idx[16] = { 2, 2, 2, 2, 1, 1, 1, 1, 3, 3, 3, 3, 0, 0, 0, 0 };
  static const uint8 kAllDirectionMetalBallFactory_X[16] = { 64, 96, 144, 176, 240, 240, 240, 240, 176, 144, 96, 64, 0, 0, 0, 0 };
  static const uint8 kAllDirectionMetalBallFactory_Y[16] = { 16, 16, 16, 16, 64, 96, 160, 192, 240, 240, 240, 240, 192, 160, 96, 64 };
  uint16 x = (overlord_x_lo[k] | overlord_x_hi[k] << 8) - BG2HOFS_copy2;
  uint16 y = (overlord_y_lo[k] | overlord_y_hi[k] << 8) - BG2VOFS_copy2;
  if ((x | y) & 0xff00 || frame_counter & 0xf)
    return;

  byte_7E0FB6 = 0;
  int j = GetRandomNumber() & 15;
  tmp_counter = kAllDirectionMetalBallFactory_Idx[j];
  overlord_x_lo[k] = kAllDirectionMetalBallFactory_X[j];
  overlord_x_hi[k] = byte_7E0FB0;
  overlord_y_lo[k] = kAllDirectionMetalBallFactory_Y[j];
  overlord_y_hi[k] = byte_7E0FB1 + 1;
  Overlord_SpawnCannonBall(k, 0);
}

void Overlord03_VerticalCannon(int k) {  // 89bf5b
  uint16 x = (overlord_x_lo[k] | overlord_x_hi[k] << 8) - BG2HOFS_copy2;
  if (x & 0xff00) {
    overlord_gen2[k] = 255;
    return;
  }
  if (!(frame_counter & 1) && overlord_gen2[k])
    overlord_gen2[k]--;
  tmp_counter = 2;
  byte_7E0FB6 = 0;
  if (!sign8(--overlord_gen1[k]))
    return;
  overlord_gen1[k] = 56;
  int xd;
  if (!overlord_gen2[k]) {
    overlord_gen2[k] = 160;
    byte_7E0FB6 = 160;
    xd = 8;
  } else {
    xd = (GetRandomNumber() & 2) * 8;
  }
  Overlord_SpawnCannonBall(k, xd);
}

void Overlord_SpawnCannonBall(int k, int xd) {  // 89bfaf
  static const int8 kOverlordSpawnBall_Xvel[4] = { 24, -24, 0, 0 };
  static const int8 kOverlordSpawnBall_Yvel[4] = { 0, 0, 24, -24 };
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0x50, &info);
  if (j < 0)
    return;

  Sprite_SetX(j, info.r5_overlord_x + xd);
  Sprite_SetY(j, info.r7_overlord_y - 1);

  sprite_x_vel[j] = kOverlordSpawnBall_Xvel[tmp_counter];
  sprite_y_vel[j] = kOverlordSpawnBall_Yvel[tmp_counter];
  sprite_floor[j] = overlord_floor[k];
  if (byte_7E0FB6) {
    sprite_ai_state[j] = byte_7E0FB6;
    sprite_y_lo[j] = sprite_y_lo[j] + 8;
    sprite_flags2[j] = 3;
    sprite_flags4[j] = 9;
  }
  sprite_delay_aux2[j] = 64;
  SpriteSfx_QueueSfx3WithPan(j, 0x7);
}

void Overlord01_PositionTarget(int k) {  // 89c01e
  byte_7E0FDE = k;
}

void Overlord_CheckIfActive(int k) {  // 89c08d
  static const int16 kOverlordInRangeOffs[2] = { 0x130, -0x40 };
  if (player_is_indoors)
    return;
  int j = frame_counter & 1;
  uint16 x = BG2HOFS_copy2 + kOverlordInRangeOffs[j] - (overlord_x_lo[k] | overlord_x_hi[k] << 8);
  uint16 y = BG2VOFS_copy2 + kOverlordInRangeOffs[j] - (overlord_y_lo[k] | overlord_y_hi[k] << 8);
  if ((x >> 15) != j || (y >> 15) != j) {
    overlord_type[k] = 0;
    uint16 blk = overlord_offset_sprite_pos[k];
    if (blk != 0xffff) {
      uint8 loadedmask = (0x80 >> (blk & 7));
      overworld_sprite_was_loaded[blk >> 3] &= ~loadedmask;
    }
  }
}

void ArmosCoordinator_RotateKnights(int k) {  // 9deccc
  if (!overlord_gen2[k])
    overlord_gen1[k]++;
  ArmosCoordinator_Rotate(k);
}

void ArmosCoordinator_Rotate(int k) {  // 9decd4
  static const uint16 kArmosCoordinator_Tab0[6] = { 0, 425, 340, 255, 170, 85 };

  WORD(overlord_x_lo[0]) += (int8)overlord_floor[k];
  for (int i = 0; i != 6; i++) {
    int t0 = WORD(overlord_x_lo[0]) + kArmosCoordinator_Tab0[i];
    uint8 size = overlord_x_lo[2];
    int tx = (overlord_x_lo[k] | overlord_x_hi[k] << 8) + ArmosSin(t0, size);
    overlord_x_hi[i] = tx;
    overlord_y_hi[i] = tx >> 8;
    int ty = (overlord_y_lo[k] | overlord_y_hi[k] << 8) + ArmosSin(t0 + 0x80, size);
    overlord_gen2[i] = ty;
    overlord_floor[i] = ty >> 8;
  }
  tmp_counter = 6;
}

bool ArmosCoordinator_CheckKnights() {  // 9dedb8
  for (int j = 5; j >= 0; j--) {
    if (sprite_state[j] && sprite_ai_state[j] == 0)
      return false;
  }
  return true;
}

void ArmosCoordinator_DisableCoercion(int k) {  // 9dedcb
  for (int j = 5; j >= 0; j--)
    sprite_ai_state[j] = 0;
}

