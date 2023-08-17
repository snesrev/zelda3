#include "sprite_main.h"
#include "sprite.h"
#include "tagalong.h"
#include "ancilla.h"
#include "overworld.h"
#include "load_gfx.h"
#include "hud.h"
#include "dungeon.h"
#include "player.h"
#include "misc.h"

#define byte_7FFE01 (*(uint8*)(g_ram+0x1FE01))
static const int8 kSpriteKeese_Tab2[16] = {0, 8, 11, 14, 16, 14, 11, 8, 0, -8, -11, -14, -16, -14, -11, -8};
static const int8 kSpriteKeese_Tab3[16] = {-16, -14, -11, -8, 0, 8, 11, 14, 16, 14, 11, 8, 0, -9, -11, -14};
static const int8 kZazak_Yvel[4] = {0, 0, 16, -16};
static const int8 kFluteBoyAnimal_Xvel[4] = {16, -16, 0, 0};
static const int8 kDesertBarrier_Xv[4] = {16, -16, 0, 0};
static const int8 kDesertBarrier_Yv[4] = {0, 0, 16, -16};
static const uint8 kCrystalSwitchPal[2] = {2, 4};
static const uint8 kZazak_Dir2[8] = {2, 3, 2, 3, 0, 1, 0, 1};
#define moldorm_x_lo ((uint8*)(g_ram+0x1FC00))
#define moldorm_x_hi ((uint8*)(g_ram+0x1FC80))
#define moldorm_y_lo ((uint8*)(g_ram+0x1FD00))
#define moldorm_y_hi ((uint8*)(g_ram+0x1FD80))
static const int8 kBadPullDownSwitch_X[5] = {-4, 12, 0, -4, 4};
static const int8 kBadPullDownSwitch_Y[5] = {-3, -3, 0, 5, 5};
static const uint8 kBadPullDownSwitch_Char[5] = {0xd2, 0xd2, 0xc4, 0xe4, 0xe4};
static const uint8 kBadPullDownSwitch_Flags[5] = {0x40, 0, 0, 0x40, 0};
static const uint8 kBadPullDownSwitch_Big[5] = {0, 0, 2, 2, 2};
static const uint8 kBadPullSwitch_Tab5[6] = {0, 1, 2, 3, 4, 5};
static const uint8 kBadPullSwitch_Tab4[12] = {0, 0, 1, 1, 2, 2, 3, 3, 4, 5, 5, 5};
static const uint8 kThief_Gfx[12] = {11, 8, 2, 5, 9, 6, 0, 3, 10, 7, 1, 4};
#define word_7FFE00 (*(uint16*)(g_ram+0x1FE00))
#define word_7FFE02 (*(uint16*)(g_ram+0x1FE02))
#define word_7FFE04 (*(uint16*)(g_ram+0x1FE04))
#define word_7FFE06 (*(uint16*)(g_ram+0x1FE06))
static const int16 kTutorialSoldier_X[20] = {
  4, 0, -6, -6, 2, 0, 0, -7, -7, -7, 0, 0, 0xf, 0xf, 0xf, 6,
  0xe, -4, 4, 0,
};
static const int16 kTutorialSoldier_Y[20] = {
  0, -10, -4, 12, 12, 0, -9, -11, -3, 5, 0, -9, -11, -3, 5, -11,
  5, 0, 0, -9,
};
static const uint8 kTutorialSoldier_Char[20] = {
  0x46, 0x40, 0, 0x28, 0x29, 0x4e, 0x42, 0x39, 0x2a, 0x3a, 0x4e, 0x42, 0x39, 0x2a, 0x3a, 0x26,
  0x38, 0x64, 0x64, 0x44,
};
static const uint8 kTutorialSoldier_Flags[20] = {
  0x40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x40, 0x40, 0x40, 0x40, 0x40, 0,
  0x40, 0, 0x40, 0,
};
static const uint8 kTutorialSoldier_Big[20] = {
  2, 2, 2, 0, 0, 2, 2, 0, 0, 0, 2, 2, 0, 0, 0, 2,
  0, 2, 2, 2,
};
static const uint8 kSprite_TutorialEntities_Tab[4] = {2, 1, 0, 3};
static const uint8 kSoldier_DirectionLockSettings[4] = {3, 2, 0, 1};
static const uint8 kWishPond_X[8] = {0, 4, 8, 12, 16, 20, 24, 0};
static const uint8 kWishPond_Y[8] = {0, 8, 16, 24, 32, 40, 4, 36};
const uint8 kWishPond2_OamFlags[76] = {
  5, 0xff, 5, 5, 5, 5, 5, 1, 2, 1, 1, 1, 2, 2, 2, 4,
  4,    4, 1, 1, 2, 1, 1, 1, 2, 1, 2, 1, 4, 4, 2, 1,
  6,    1, 2, 1, 2, 2, 1, 2, 2, 4, 1, 1, 4, 2, 1, 4,
  2,    2, 4, 4, 4, 2, 1, 4, 1, 2, 2, 1, 2, 2, 1, 1,
  4,    4, 1, 2, 2, 4, 4, 4, 2, 5, 2, 1,
};
static const uint8 kWishPondItemOffs[32] = {
  0, 4, 6, 7, 8, 10, 11, 12, 13, 14, 15, 16, 17, 20, 21, 22, 22, 23, 24, 25, 28, 30, 31, 32, 33, 33, 37, 40, 42, 42, 42, 42
};
static const uint8 kWishPondItemData[50] = {
  0x3a, 0x3a, 0x3b, 0x3b, 0x0c, 0x2a, 0x0a, 0x27, 0x29, 0x0d, 0x07, 0x08, 0x0f, 0x10, 0x11, 0x12,
  0x09, 0x13, 0x14, 0x4a, 0x21, 0x1d, 0x15, 0x18, 0x19, 0x31, 0x1a, 0x1a, 0x1b, 0x1c, 0x4b, 0x1e,
  0x1f, 0x49, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x22, 0x23, 0x29, 0x16, 0x2b, 0x2c, 0x2d, 0x3d, 0x3c, 0x48
};
static const uint8 kUncle_LeaveHouse_Delay[2] = {64, 224};
static const uint8 kUncle_LeaveHouse_Dir[2] = {2, 1};
static const int8 kUncle_LeaveHouse_Xvel[4] = {0, 0, -12, 12};
static const int8 kUncle_LeaveHouse_Yvel[4] = {-12, 12, 0, 0};
static const DrawMultipleData kPriest_Dmd[20] = {
  { 0, -8, 0x0e20, 2},
  { 0,  0, 0x0e26, 2},
  { 0, -8, 0x0e20, 2},
  { 0,  0, 0x4e26, 2},
  { 0, -8, 0x0e0e, 2},
  { 0,  0, 0x0e24, 2},
  { 0, -8, 0x0e0e, 2},
  { 0,  0, 0x0e24, 2},
  { 0, -8, 0x0e22, 2},
  { 0,  0, 0x0e28, 2},
  { 0, -8, 0x0e22, 2},
  { 0,  0, 0x0e2a, 2},
  { 0, -8, 0x4e22, 2},
  { 0,  0, 0x4e28, 2},
  { 0, -8, 0x4e22, 2},
  { 0,  0, 0x4e2a, 2},
  {-7,  1, 0x0e0a, 2},
  { 3,  3, 0x0e0c, 2},
  {-7,  1, 0x0e0a, 2},
  { 3,  3, 0x0e0c, 2},
};
static const DrawMultipleData kSageMantle_Dmd[4] = {
  {0, 0, 0x162c, 2},
  {16, 0, 0x562c, 2},
  {0, 16, 0x062e, 2},
  {16, 16, 0x462e, 2},
};
static const uint8 kCrystalMaiden_Dma[16] = {0x20, 0xc0, 0x20, 0xc0, 0, 0xa0, 0, 0xa0, 0x40, 0x80, 0x40, 0x60, 0x40, 0x80, 0x40, 0x60};
static const DrawMultipleData kCrystalMaiden_SpriteData[16] = {
  {1, -7, 0x0120, 2},
  {1,  3, 0x0122, 2},
  {1, -7, 0x0120, 2},
  {1,  3, 0x4122, 2},
  {1, -7, 0x0120, 2},
  {1,  3, 0x0122, 2},
  {1, -7, 0x0120, 2},
  {1,  3, 0x4122, 2},
  {1, -7, 0x0120, 2},
  {1,  3, 0x0122, 2},
  {1, -7, 0x0120, 2},
  {1,  3, 0x0122, 2},
  {1, -7, 0x4120, 2},
  {1,  3, 0x4122, 2},
  {1, -7, 0x4120, 2},
  {1,  3, 0x4122, 2},
};
static const int8 kZelda_Xvel[4] = {0, 0, -9, 9};
static const int8 kZelda_Yvel[4] = {-9, 9, 0, 0};
static const int8 kHeartRefill_AccelX[2] = {1, -1};
static const int8 kHeartRefill_VelTarget[2] = {10, -10};
static const DrawMultipleData kFakeSword_Dmd[2] = {
  {4, 0, 0x00f4, 0},
  {4, 8, 0x00f5, 0},
};
static const uint8 kThrowableScenery_Char[12] = {0x42, 0x44, 0x46, 0, 0x46, 0x44, 0x42, 0x44, 0x44, 0, 0x46, 0x44};
const uint8 kThrowableScenery_Flags[9] = { 0xc, 0xc, 0xc, 0, 0, 0, 0xb0, 0x08, 0xb4 };
static const int16 kThrowableScenery_DrawLarge_X[4] = {-8, 8, -8, 8};
static const int16 kThrowableScenery_DrawLarge_Y[4] = {-14, -14, 2, 2};
static const uint8 kThrowableScenery_DrawLarge_Flags[4] = {0, 0x40, 0x80, 0xc0};
static const int16 kThrowableScenery_DrawLarge_X2[3] = {-6, 0, 6};
static const uint8 kThrowableScenery_DrawLarge_OamFlags[2] = {0xc, 0};
static const int8 kScatterDebris_X[4] = {-8, 8, -8, 8};
static const int8 kScatterDebris_Y[4] = {-8, -8, 8, 8};
static const uint8 kMovableMantle_X[6] = {0, 0x10, 0x20, 0, 0x10, 0x20};
static const uint8 kMovableMantle_Y[6] = {0, 0, 0, 0x10, 0x10, 0x10};
static const uint8 kMovableMantle_Char[6] = {0xc, 0xe, 0xc, 0x2c, 0x2e, 0x2c};
static const uint8 kMovableMantle_Flags[6] = {0x31, 0x31, 0x71, 0x31, 0x31, 0x71};
static const uint8 kSprite_SimplifiedTileAttr[256] = {
  0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 3, 3, 3,
  0, 0, 0, 0, 0, 0, 1, 1, 4, 4, 4, 4, 4, 4, 4, 4,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 4,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};
static const uint8 kSoldier_Gfx[4] = {8, 0, 12, 5};
static const uint8 kSoldier_Delay[4] = {0x60, 0xc0, 0xff, 0x40};
static const uint8 kSoldier_Draw1_Char[4] = {0x42, 0x42, 0x40, 0x44};
static const uint8 kSoldier_Draw1_Flags[4] = {0x40, 0, 0, 0};
static const int8 kSoldier_Draw1_Yd[26] = {
  7, 8, 7, 8, 8, 7, 8, 7, 8, 7, 8, 8, 7, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
};
static const int8 kSoldier_Draw2_Xd[104] = {
  -4, 4, 10, 10, -4, 4, 10, 10, -4, 4, 10, 10, -4, 4, 10, 10,
  -4, -4, 0, 0, -4, -4, 0, 0, -3, -3, 0, 0, -3, -3, -4, 4,
  -3, -3, -4, 4, -3, -3, -4, 4, -3, -3, -4, 4, 12, 12, 0, 0,
  12, 12, 0, 0, 11, 11, 0, 0, -4, 4, 0, 0, -4, 4, 0, 0,
  -4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  -4, 4, 0, 0, -4, 4, 0, 0, -4, 4, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
};
static const int8 kSoldier_Draw2_Yd[104] = {
  0, 0, 2, 10, 0, 0, 2, 10, 0, 0, 1, 9, 0, 0, 2, 10,
  -2, 6, 1, 1, -2, 6, 2, 2, -2, 6, 1, 1, -5, 3, 0, 0,
  -4, 4, 0, 0, -4, 4, 0, 0, -5, 3, 0, 0, -2, 6, 1, 1,
  -2, 6, 2, 2, -2, 6, 1, 1, 0, 0, 8, 8, 0, 0, 8, 8,
  0, 0, 8, 8, 0, 0, 8, 8, 0, 0, 8, 8, 0, 0, 8, 8,
  0, 0, 8, 8, 0, 0, 8, 8, 0, 0, 8, 8, 0, 0, 8, 8,
  0, 0, 8, 8, 0, 0, 8, 8,
};
static const uint8 kSoldier_Draw2_Char[104] = {
  0x48, 0x49, 0x6d, 0x7d, 0x49, 0x48, 0x6d, 0x7d, 0x46, 0x46, 0x6d, 0x7d, 0x4b, 0x46, 0x6d, 0x7d,
  0x4d, 0x5d, 0x4e, 0x4e, 0x4d, 0x5d, 0x60, 0x60, 0x4d, 0x5d, 0x62, 0x62, 0x6d, 0x7d, 0x64, 0x64,
  0x6d, 0x7d, 0x66, 0x67, 0x6d, 0x7d, 0x67, 0x66, 0x6d, 0x7d, 0x64, 0x69, 0x4d, 0x5d, 0x4e, 0x4e,
  0x4d, 0x5d, 0x60, 0x60, 0x4d, 0x5d, 0x62, 0x62, 2, 3, 0x20, 0x20, 2, 0xc, 0x20, 0x20,
  2, 0xc, 0x20, 0x20, 8, 8, 0x20, 0x20, 0xe, 0xe, 0x20, 0x20, 0xe, 0xe, 0x20, 0x20,
  5, 6, 0x20, 0x20, 0x22, 6, 0x20, 0x20, 0x22, 6, 0x20, 0x20, 8, 8, 0x20, 0x20,
  0xe, 0xe, 0x20, 0x20, 0xe, 0xe, 0x20, 0x20,
};
static const uint8 kSoldier_Draw2_Flags[104] = {
  0, 0, 0, 0, 0x40, 0x40, 0, 0, 0, 0x40, 0, 0, 0, 0x40, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x40,
  0, 0, 0, 0, 0, 0, 0x40, 0x40, 0, 0, 0, 0x40, 0x40, 0x40, 0x40, 0x40,
  0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x40, 0x40, 0x40, 0x40,
  0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
};
static const uint8 kSoldier_Draw2_Big[104] = {
  2, 2, 0, 0, 2, 2, 0, 0, 2, 2, 0, 0, 2, 2, 0, 0,
  0, 0, 2, 2, 0, 0, 2, 2, 0, 0, 2, 2, 0, 0, 2, 2,
  0, 0, 2, 2, 0, 0, 2, 2, 0, 0, 2, 2, 0, 0, 2, 2,
  0, 0, 2, 2, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2,
};
static const uint8 kSoldier_Draw2_OamIdx[4] = {12, 12, 12, 4};
static const int8 kSoldier_Draw3_Xd[28] = {
  -3, -3, -4, -4, -4, -4, -4, -4, -11, -3, -11, -3, -16, -8, 12, 12,
  12, 12, 12, 12, 12, 12, 21, 13, 21, 13, 24, 16,
};
static const int8 kSoldier_Draw3_Yd[28] = {
  11, 19, 11, 19, 10, 18, 14, 22, 8, 8, 8, 8, 6, 6, -10, -2,
  -9, -1, -9, -1, -16, -8, 8, 8, 8, 8, 6, 6,
};
static const uint8 kSoldier_Draw3_Char[28] = {
  0x7b, 0x6b, 0x7b, 0x6b, 0x7b, 0x6b, 0x7b, 0x6b, 0x6c, 0x7c, 0x6c, 0x7c, 0x6c, 0x7c, 0x6b, 0x7b,
  0x6b, 0x7b, 0x6b, 0x7b, 0x6b, 0x7b, 0x6c, 0x7c, 0x6c, 0x7c, 0x6c, 0x7c,
};
static const uint8 kSoldier_Draw3_Flags[28] = {
  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
};
static const uint8 kSoldier_Draw3_OamIdx[4] = {4, 4, 4, 20};
static const int8 kSoldier_Xvel[4] = {8, -8, 0, 0};
static const int8 kSoldier_Yvel[4] = {0, 0, 8, -8};
static const uint8 kSoldier_Gfx2[32] = {
  11, 12, 13, 12, 4, 5, 6, 5, 0, 1, 2, 3, 7, 8, 9, 10,
  17, 18, 17, 18, 7, 8, 7, 8, 3, 4, 3, 4, 13, 14, 13, 14,
};
static const int8 kSoldierB_Xvel[8] = {1, 1, -1, -1, -1, -1, 1, 1};
static const int8 kSoldierB_Yvel[8] = {-1, 1, 1, -1, -1, 1, 1, -1};
static const int8 kSoldierB_Xvel2[8] = {8, 0, -8, 0, -8, 0, 8, 0};
static const int8 kSoldierB_Yvel2[8] = {0, 8, 0, -8, 0, 8, 0, -8};
static const uint8 kSoldierB_Dir[8] = {0, 2, 1, 3, 1, 2, 0, 3};
static const uint8 kSoldierB_Mask2[8] = {1, 4, 2, 8, 2, 4, 1, 8};
static const uint8 kSoldierB_Mask[8] = {8, 1, 4, 2, 8, 2, 4, 1};
static const uint8 kSoldierB_NextB2[8] = {1, 2, 3, 0, 5, 6, 7, 4};
static const uint8 kSoldierB_NextB[8] = {3, 0, 1, 2, 7, 4, 5, 6};
static const uint8 kSoldier_HeadDirs[32] = {
  0, 2, 2, 2, 0, 3, 3, 3, 1, 3, 3, 3, 1, 2, 2, 2,
  2, 0, 0, 0, 2, 1, 1, 1, 3, 1, 1, 1, 3, 0, 0, 0,
};
static const uint8 kSoldier_Tab1[4] = {13, 13, 12, 12};
static const uint8 kSoldier_DrawShadow[4] = {0xc, 0xc, 0xa, 0xa};
static const int8 kSoldier_SetTowardsVel[6] = {14, -14, 0, 0, 14, -14};
static const uint8 kSprite_SpawnProbeStaggered_Tab[4] = {0x10, 0x30, 0, 0x20};
static const int8 kSpawnProbe_Xvel[64] = {
  -16, -16, -16, -16, -16, -16, -16, -16, -16, -14, -12, -10, -8, -6, -4, -2,
  0, 2, 4, 6, 8, 10, 12, 14, 16, 16, 16, 16, 16, 16, 16, 16,
  16, 16, 16, 16, 16, 16, 16, 16, 14, 12, 10, 8, 6, 4, 2, 0,
  -2, -4, -6, -8, -10, -12, -14, -16, -16, -16, -16, -16, -16, -16, -16, -16,
};
static const int8 kSpawnProbe_Yvel[64] = {
  0, 2, 4, 6, 8, 10, 12, 14, 16, 16, 16, 16, 16, 16, 16, 16,
  16, 16, 16, 16, 16, 16, 16, 16, 14, 12, 10, 8, 6, 4, 2, 0,
  -2, -4, -6, -8, -10, -12, -14, -16, -16, -16, -16, -16, -16, -16, -16, -16,
  -16, -16, -16, -16, -16, -16, -16, -16, -14, -12, -10, -8, -6, -4, -2, 0,
};
int ctr;
bool foo = false;
static const uint8 kChainBallTrooper_Tab1[4] = {0x0d, 0x60, 0x22, 0x10};  // wtf
static const uint8 kFlailTrooperGfx[32] = {
  0x10, 0x11, 0x12, 0x13, 0x10, 0x11, 0x12, 0x13, 6, 7, 8, 9, 6, 7, 8, 9,
  0, 1, 2, 3, 0, 1, 4, 5, 0xa, 0xb, 0xc, 0xd, 0xa, 0xb, 0xe, 0xf,
};
static const uint8 kJavelinTrooper_Tab2[64] = {
  25, 25, 24, 24, 23, 23, 23, 23, 19, 19, 18, 18, 17, 17, 17, 17,
  16, 16, 15, 15, 14, 14, 14, 14, 22, 22, 21, 21, 20, 20, 20, 20,
  20, 20, 18, 18, 18, 16, 16, 16, 21, 21,  8,  8,  8,  6,  6,  6,
  22, 22,  4,  4,  4,  3,  3,  3, 23, 23, 15, 15, 15, 11, 11, 11,
};
static const int8 kSprite_Recruit_Xvel[8] = {12, -12, 0, 0, 18, -18, 0, 0};
static const int8 kSprite_Recruit_Yvel[8] = {0, 0, 0xc, -0xc, 0, 0, 0x12, -0x12};
static const int8 kSprite_Recruit_Gfx[8] = {0, 2, 4, 6, 1, 3, 5, 7};
static const uint8 kRecruit_Moving_HeadDir[8] = {2, 3, 2, 3, 0, 1, 0, 1};
static const int16 kRecruit_Draw_X[8] = {2, 2, -2, -2, 0, 0, 0, 0};
static const uint8 kRecruit_Draw_Char[8] = {0x8a, 0x8c, 0x8a, 0x8c, 0x86, 0x88, 0x8e, 0xa0};
static const uint8 kRecruit_Draw_Flags[8] = {0x40, 0x40, 0, 0, 0, 0, 0, 0};
static const uint8 kSprite_Zora_SurfacingGfx[16] = {4, 3, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 0, 0};
static const uint8 kChainBallTrooperHead_Char[4] = {2, 2, 0, 4};
static const uint8 kChainBallTrooperHead_Flags[4] = {0x40, 0, 0, 0};
static const int8 kFlailTrooperBody_X[72] = {
  -4, 4, 12, -4, 4, 13, -4, 4, 13, -4, 4, 13, -4, 4, 13, -4,
  4, 13, 0, 0, 4, 0, 0, 5, 0, 0, 6, 0, 0, 4, -4, 4,
  -6, -4, 4, -5, -4, 4, -5, -4, 4, -6, -4, 4, -5, -4, 4, -6,
  0, 0, 4, 0, 0, 3, 0, 0, 2, 0, 0, 4, 0, 0, 0, 0,
  0, 0, -4, 4, 4, -4, 4, 4,
};
static const int8 kFlailTrooperBody_Y[72] = {
  0, 0, -4, 0, 0, -4, 0, 0, -3, 0, 0, -2, 0, 0, -3, 0,
  0, -2, 0, 0, 1, 0, 0, 1, 0, 0, 2, 0, 0, 2, 0, 0,
  -2, 0, 0, -2, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1,
  0, 0, 1, 0, 0, 1, 0, 0, 2, 0, 0, 2, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
};
static const uint8 kFlailTrooperBody_Char[72] = {
  0x46, 6, 0x2f, 0x46, 6, 0x2f, 0x48, 0xd, 0x2f, 0x48, 0xd, 0x2f, 0x49, 0xc, 0x2f, 0x49,
  0xc, 0x2f, 8, 8, 0x2f, 8, 8, 0x2f, 0x22, 0x22, 0x2f, 0x22, 0x22, 0x2f, 0xa, 0x64,
  0x2f, 0xa, 0x64, 0x2f, 0x2c, 0x67, 0x2f, 0x2c, 0x67, 0x2f, 0x2d, 0x66, 0x2f, 0x2d, 0x66, 0x2f,
  8, 8, 0x2f, 8, 8, 0x2f, 0x22, 0x22, 0x2f, 0x22, 0x22, 0x2f, 0x62, 0x62, 0x62, 0x62,
  0x62, 0x62, 0x46, 0x4b, 0x4b, 0x69, 0x64, 0x64,
};
static const uint8 kFlailTrooperBody_Flags[72] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x40, 0x40, 0, 0x40,
  0x40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x40,
  0x40, 0, 0x40, 0x40, 0, 0, 0x40, 0, 0, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
  0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0,
  0, 0, 0, 0x40, 0x40, 0, 0x40, 0x40,
};
static const uint8 kFlailTrooperBody_Big[72] = {
  2, 2, 0, 2, 2, 0, 2, 2, 0, 2, 2, 0, 2, 2, 0, 2,
  2, 0, 2, 2, 0, 2, 2, 0, 2, 2, 0, 2, 2, 0, 2, 2,
  0, 2, 2, 0, 2, 2, 0, 2, 2, 0, 2, 2, 0, 2, 2, 0,
  2, 2, 0, 2, 2, 0, 2, 2, 0, 2, 2, 0, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2,
};
static const uint8 kFlailTrooperBody_Num[24] = {
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 1, 1, 1, 1,
};
static const uint8 kFlailTrooperBody_SprOffs[24] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 8, 8, 8, 8,
};
const uint16 kSinusLookupTable[256] = {
  0, 3, 6, 9, 12, 15, 18, 21, 25, 28, 31, 34, 37, 40, 40, 46,
  49, 53, 56, 59, 62, 65, 68, 71, 74, 77, 80, 83, 86, 89, 92, 95,
  97, 100, 103, 106, 109, 112, 115, 117, 120, 123, 126, 128, 131, 134, 136, 139,
  142, 144, 147, 149, 152, 155, 157, 159, 162, 164, 167, 169, 171, 174, 176, 178,
  181, 183, 185, 187, 189, 191, 193, 195, 197, 199, 201, 203, 205, 207, 209, 211,
  212, 214, 216, 217, 219, 221, 222, 224, 225, 227, 228, 230, 231, 232, 234, 235,
  236, 237, 238, 239, 241, 242, 243, 244, 244, 245, 246, 247, 248, 249, 249, 250,
  251, 251, 252, 252, 253, 253, 254, 254, 254, 255, 255, 255, 255, 255, 255, 255,
  256, 255, 255, 255, 255, 255, 255, 255, 254, 254, 254, 253, 253, 252, 252, 251,
  251, 250, 249, 249, 248, 247, 246, 245, 244, 244, 243, 242, 241, 239, 238, 237,
  236, 235, 234, 232, 231, 230, 228, 227, 225, 224, 222, 221, 219, 217, 216, 214,
  212, 211, 209, 207, 205, 203, 201, 199, 197, 195, 193, 191, 189, 187, 185, 183,
  181, 178, 176, 174, 171, 169, 167, 164, 162, 159, 157, 155, 152, 149, 147, 144,
  142, 139, 136, 134, 131, 128, 126, 123, 120, 117, 115, 112, 109, 106, 103, 100,
  97, 95, 92, 89, 86, 83, 80, 77, 74, 71, 68, 65, 62, 59, 56, 53,
  49, 46, 43, 40, 37, 34, 31, 28, 25, 21, 18, 15, 12, 9, 6, 3,
};
static const uint8 kFlailTrooperWeapon_Tab4[4] = {0x33, 0x66, 0x99, 0xcc};
static const uint8 kFlailTrooperWeapon_Tab0[32] = {
  0x10, 0x12, 0x14, 0x16, 0x18, 0x1a, 0x1c, 0x1e, 0x20, 0x22, 0x24, 0x26, 0x28, 0x2a, 0x2c, 0x2e,
  0x30, 0x2e, 0x2c, 0x2a, 0x28, 0x26, 0x24, 0x22, 0x20, 0x1e, 0x1c, 0x1a, 0x18, 0x16, 0x14, 0x12,
};
static const int8 kFlailTrooperWeapon_Tab1[4] = {4, 4, 12, -5};
static const int8 kFlailTrooperWeapon_Tab2[4] = {-2, -2, -6, -4};
static const uint8 kFlailTrooperAttackDir[4] = {3, 1, 2, 0};
static const uint8 kSprite_WarpVortex_Flags[4] = {0, 0x40, 0xc0, 0x80};
static const int8 kSpriteRope_Gfx[8] = {0, 0, 2, 3, 2, 3, 1, 1};
static const int8 kSpriteRope_Flags[8] = {0, 0x40, 0, 0, 0x40, 0x40, 0, 0x40};
static const int8 kSpriteRope_Tab1[8] = {4, 5, 2, 3, 0, 1, 6, 7};
static const int8 kSpriteRope_Xvel[8] = {8, -8, 0, 0, 16, -16, 0, 0};
static const int8 kSpriteRope_Yvel[8] = {0, 0, 8, -8, 0, 0, 0x10, -0x10};
static const int8 kSpriteRope_Tab0[4] = {2, 3, 1, 0};
static const uint8 kSpawnBee_InitDelay[4] = {64, 64, 255, 255};
static const int8 kSpawnBee_InitVel[8] = {15, 5, -5, -15, 20, 10, -10, -20};
static const DrawMultipleData kLargeShadow_Dmd[15] = {
  {-6, 19, 0x086c, 2},
  { 0, 19, 0x086c, 2},
  { 6, 19, 0x086c, 2},
  {-5, 19, 0x086c, 2},
  { 0, 19, 0x086c, 2},
  { 5, 19, 0x086c, 2},
  {-4, 19, 0x086c, 2},
  { 0, 19, 0x086c, 2},
  { 4, 19, 0x086c, 2},
  {-3, 19, 0x086c, 2},
  { 0, 19, 0x086c, 2},
  { 3, 19, 0x086c, 2},
  {-2, 19, 0x086c, 2},
  { 0, 19, 0x086c, 2},
  { 2, 19, 0x086c, 2},
};
static const uint8 kHelmasaur_Tab0[32] = {
  0, 1, 2, 3, 4, 5, 6, 7, 8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 6, 5, 4, 3, 2, 1,
};
static const uint8 kFluteBoyAnimal_OamFlags[2] = {0x40, 0};
static const uint8 kFluteBoyAnimal_Gfx[3] = {0, 1, 2};
static const uint8 kGibo_OamFlags[4] = {0, 0x40, 0xc0, 0x80};
static const uint8 kGibo_OamFlags2[2] = {11, 7};
#define chainchomp_x_hist ((uint16*)(g_ram+0x1FC00))
#define chainchomp_y_hist ((uint16*)(g_ram+0x1FD00))
static const uint8 kBlindHead_Draw_Char[16] = {0x86, 0x86, 0x84, 0x82, 0x80, 0x82, 0x84, 0x86, 0x86, 0x86, 0x88, 0x8a, 0x8c, 0x8a, 0x88, 0x86};
static const uint8 kBlindHead_Draw_Flags[16] = {0, 0, 0, 0, 0, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0, 0, 0, 0};
static const uint8 kGanon_G_Func2[16] = { 8, 7, 6, 5, 4, 3, 2, 1, 8, 7, 6, 5, 4, 3, 2, 1 };
static const int8 kGanon_Draw_X[204] = {
   18,  -8,   8, -8,   8, -18, -18, 18,  -8,   8,  -8,   8,  18,  -8,   8, -8,
    8, -18, -18, 18,  -8,   8,  -8,  8,  16,  -8,   8,  -8,   8, -18, -18, 16,
   -8,   8, -11, 11,  16,  -8,   8, -8,   8, -18, -18,  16,  -8,   8, -11, 11,
   16,  -8,   8, -8,   8, -18, -18, 16,  -8,   8, -11,  11,  18,  -8,   8, -8,
    8, -18, -18, 18,  -8,   8,  -8,  8,  18,  -8,   8,  -8,   8, -18, -18, 18,
   -8,   8,  -8,  8,  18,  -8,   8, -8,   8, -18, -18,  18,  -8,   8, -11, 11,
   -8,   8,  -8,  8,  -8,   8,  -8,  8, -18, -18,  18,  18,  -8,   8,  -8,  8,
   -8,   8,  -8,  8, -18, -18,  18, 18,  -8,   8,  -8,   8,  -8,   8, -10, 10,
  -18, -18,  18, 18,  -8,   8,  -8,  8,  -8,   8, -10,  10, -18, -18,  18, 18,
   -8,   8,  -8,  8,  -8,   8, -10, 10, -18, -18,  18,  18,  -8,   8,  -8,  8,
   -8,   8,  -8,  8, -18, -18,  18, 18,  -8,   8,  -8,   8,  -8,   8,  -8,  8,
  -18, -18,  18, 18,  -7,  -8,   8, -8,   8,  -9,   8, -14, -14,  -8,   8,  8,
   -8,   8,  -8,  8, -18, -18,  18, 18,  -8,   8, -11,  11,
};
static const int8 kGanon_Draw_Y[204] = {
   -8, -16, -16, -13, -13,  -9,  -1, -16,   3,   3,   8,   8,  -8, -16, -16, -13,
  -13,  -9,  -1, -16,   3,   3,   8,   8,   5, -10, -10, -13, -13,  -7,   1,  -3,
    3,   3,   8,   8,   5, -10, -10, -13, -13,  -7,   1,  -3,   3,   3,   8,   8,
    5, -10, -10, -13, -13,  -7,   1,  -3,   3,   3,   8,   8,  -1, -16, -16, -13,
  -13,  -9,  -1,  -9,   3,   3,   8,   8, -10, -16, -16, -13, -13, -18, -10, -18,
    3,   3,   8,   8,   1, -10, -10, -13, -13,  -7,   1,  -7,   3,   3,   8,   8,
  -12, -12,   4,   4, -18, -18,  10,  10, -16,  -8,  -4,   4, -12, -12,   4,   4,
  -18, -18,  10,  10, -16,  -8,  -4,   4, -12, -12,   4,   4, -12, -12,  10,  10,
   -4,   4,  -4,   4, -12, -12,   4,   4, -12, -12,  10,  10,  -4,   4,  -4,   4,
  -12, -12,   4,   4, -12, -12,  10,  10,  -4,   4,  -4,   4, -12, -12,   4,   4,
  -18, -18,  10,  10,  -4,   4,  -4,   4, -12, -12,   4,   4, -18, -18,  10,  10,
  -16,  -8, -16,  -8,  -7, -12, -12,   4,   4,   7,  13, -11,  -4, -16, -16, -16,
  -10, -10, -13, -13,  -7,  -7,  -7,  -7,   3,   3,   8,   8,
};
static const uint8 kGanon_Draw_Char[204] = {
  0x16,    0,    0,    2,    2,    8, 0x18,    6, 0x22, 0x22, 0x20, 0x20, 0x46,    0,    0,    2,
     2,    8, 0x18, 0x36, 0x22, 0x22, 0x20, 0x20, 0x1a,    0,    0,    4,    4, 0x38, 0x48,  0xa,
  0x24, 0x24, 0x20, 0x20, 0x1a, 0x40, 0x42,    4,    4, 0x38, 0x48,  0xa, 0x24, 0x24, 0x20, 0x20,
  0x1a, 0x42, 0x40,    4,    4, 0x38, 0x48,  0xa, 0x24, 0x24, 0x20, 0x20, 0x18,    0,    0,    2,
     2,    8, 0x18,    8, 0x22, 0x22, 0x20, 0x20, 0x16, 0x6a, 0x6a,  0xe,  0xe,    6, 0x16,    6,
  0x22, 0x22, 0x20, 0x20, 0x48,    0,    0,    4,    4, 0x38, 0x48, 0x38, 0x24, 0x24, 0x20, 0x20,
  0x4e, 0x4e, 0x6e, 0x6e, 0x6c, 0x6c, 0xa2, 0xa2,  0xc, 0x1c, 0x3c, 0x4c, 0x4e, 0x4e, 0x6e, 0x6e,
  0x6c, 0x6c, 0xa2, 0xa2, 0x3a, 0x4a, 0x3c, 0x4c, 0x84, 0x84, 0xa4, 0xa4, 0xa0, 0xa0, 0xa2, 0xa2,
  0x3c, 0x4c, 0x3c, 0x4c, 0x84, 0x84, 0xa4, 0xa4, 0x80, 0x82, 0xa2, 0xa2, 0x3c, 0x4c, 0x3c, 0x4c,
  0x84, 0x84, 0xa4, 0xa4, 0x82, 0x80, 0xa2, 0xa2, 0x3c, 0x4c, 0x3c, 0x4c, 0x4e, 0x4e, 0x6e, 0x6e,
  0x6c, 0x6c, 0xa2, 0xa2, 0x3c, 0x4c, 0x3c, 0x4c, 0x4e, 0x4e, 0x6e, 0x6e, 0x6c, 0x6c, 0xa2, 0xa2,
   0xc, 0x1c,  0xc, 0x1c, 0xe0, 0xc6, 0xc8, 0xe6, 0xe8, 0x20, 0x20,    8, 0x18, 0xc0, 0xc2, 0xc2,
     0,    0, 0xce, 0xce, 0xec, 0xec, 0xec, 0xec, 0xee, 0xee, 0xc4, 0xc4,
};
static const uint8 kGanon_Draw_Flags[204] = {
  0x4c,  0xc, 0x4c,  0xa, 0x4a,  0xc,  0xc, 0x4c,  0xa, 0x4a,  0xc, 0x4c, 0x4c,  0xc, 0x4c,  0xa,
  0x4a,  0xc,  0xc, 0x4c,  0xa, 0x4a,  0xc, 0x4c, 0x4c,  0xc, 0x4c,  0xa, 0x4a,  0xc,  0xc, 0x4c,
   0xa, 0x4a,  0xc, 0x4c, 0x4c,  0xc,  0xc,  0xa, 0x4a,  0xc,  0xc, 0x4c,  0xa, 0x4a,  0xc, 0x4c,
  0x4c, 0x4c, 0x4c,  0xa, 0x4a,  0xc,  0xc, 0x4c,  0xa, 0x4a,  0xc, 0x4c, 0x4c,  0xc, 0x4c,  0xa,
  0x4a,  0xc,  0xc, 0x4c,  0xa, 0x4a,  0xc, 0x4c, 0x4c,  0xc, 0x4c,  0xa, 0x4a,  0xc,  0xc, 0x4c,
   0xa, 0x4a,  0xc, 0x4c, 0x4c,  0xc, 0x4c,  0xa, 0x4a,  0xc,  0xc, 0x4c,  0xa, 0x4a,  0xc, 0x4c,
   0xa, 0x4a,  0xa, 0x4a,  0xc, 0x4c,  0xc, 0x4c,  0xc,  0xc, 0x4c, 0x4c,  0xa, 0x4a,  0xa, 0x4a,
   0xc, 0x4c,  0xc, 0x4c,  0xc,  0xc, 0x4c, 0x4c,  0xa, 0x4a,  0xa, 0x4a,  0xc, 0x4c,  0xc, 0x4c,
   0xc,  0xc, 0x4c, 0x4c,  0xa, 0x4a,  0xa, 0x4a,  0xc,  0xc,  0xc, 0x4c,  0xc,  0xc, 0x4c, 0x4c,
   0xa, 0x4a,  0xa, 0x4a, 0x4c, 0x4c,  0xc, 0x4c,  0xc,  0xc, 0x4c, 0x4c,  0xa, 0x4a,  0xa, 0x4a,
   0xc, 0x4c,  0xc, 0x4c,  0xc,  0xc, 0x4c, 0x4c,  0xa, 0x4a,  0xa, 0x4a,  0xc, 0x4c,  0xc, 0x4c,
   0xc,  0xc, 0x4c, 0x4c,  0xc,  0xa,  0xa,  0xa,  0xa,  0xc, 0x4c,  0xc,  0xc,  0xc,  0xc,  0xc,
   0xc, 0x4c,  0xa, 0x4a,  0xc,  0xc, 0x4c, 0x4c,  0xa, 0x4a,  0xc, 0x4c,
};
static const uint8 kGanon_Draw_Char2[12] = { 0x40, 0x42, 0, 0, 0x42, 0x40, 0x82, 0x80, 0xa0, 0xa0, 0x80, 0x82 };
static const uint8 kGanon_Draw_Flags2[12] = { 0, 0, 0, 0x40, 0x40, 0x40, 0x40, 0x40, 0, 0x40, 0, 0 };
static HandlerFuncK *const kSpriteActiveRoutines[243] = {
  &Sprite_Raven,
  &Sprite_01_Vulture_bounce,
  &Sprite_02_StalfosHead,
  NULL,
  &Sprite_PullSwitch_bounce,
  &Sprite_PullSwitch_bounce,
  &Sprite_PullSwitch_bounce,
  &Sprite_PullSwitch_bounce,
  &Sprite_08_Octorok,
  &Sprite_09_GiantMoldorm,
  &Sprite_08_Octorok,
  &Sprite_0B_Cucco,
  &Sprite_0C_OctorokStone,
  &Sprite_0D_Buzzblob,
  &Sprite_0E_Snapdragon,
  &Sprite_0F_Octoballoon,
  &Sprite_10_OctoballoonBaby,
  &Sprite_11_Hinox,
  &Sprite_12_Moblin,
  &Sprite_13_MiniHelmasaur,
  &Sprite_14_ThievesTownGrate,
  &Sprite_15_Antifairy,
  &Sprite_16_Elder_bounce,
  &Sprite_17_Hoarder,
  &Sprite_18_MiniMoldorm,
  &Sprite_19_Poe,
  &Sprite_1A_Smithy,
  &Sprite_1B_Arrow,
  &Sprite_1C_Statue,
  &Sprite_1D_FluteQuest,
  &Sprite_1E_CrystalSwitch,
  &Sprite_1F_SickKid,
  &Sprite_20_Sluggula,
  &Sprite_21_WaterSwitch,
  &Sprite_22_Ropa,
  &Sprite_23_RedBari,
  &Sprite_23_RedBari,
  &Sprite_25_TalkingTree,
  &Sprite_26_HardhatBeetle,
  &Sprite_27_Deadrock,
  &Sprite_28_DarkWorldHintNPC,
  &Sprite_HumanMulti_1,
  &Sprite_SweepingLady,
  &Sprite_2B_Hobo,
  &Sprite_Lumberjacks,
  &Sprite_2D_TelepathicTile,
  &Sprite_2E_FluteKid,
  &Sprite_MazeGameLady,
  &Sprite_MazeGameGuy,
  &Sprite_FortuneTeller,
  &Sprite_QuarrelBros,
  &Sprite_33_RupeePull,
  &Sprite_YoungSnitchLady,
  &Sprite_InnKeeper,
  &Sprite_Witch,
  &Sprite_37_Waterfall,
  &Sprite_38_EyeStatue,
  &Sprite_39_Locksmith,
  &Sprite_3A_MagicBat,
  &Sprite_DashItem,
  &Sprite_TroughBoy,
  &Sprite_OldSnitchLady,
  &Sprite_17_Hoarder,
  &Sprite_TutorialGuardOrBarrier,
  &Sprite_TutorialGuardOrBarrier,
  // Trampoline 48 entries
  &Sprite_41_BlueGuard,
  &Sprite_41_BlueGuard,
  &Sprite_41_BlueGuard,
  &Sprite_44_BluesainBolt,
  &Sprite_45_HogSpearMan,
  &Sprite_46_BlueArcher,
  &Sprite_47_GreenBushGuard,
  &Sprite_48_RedJavelinGuard,
  &Sprite_49_RedBushGuard,
  &Sprite_4A_BombGuard,
  &Sprite_4B_GreenKnifeGuard,
  &Sprite_4C_Geldman,
  &Sprite_4D_Toppo,
  &Sprite_4E_Popo,
  &Sprite_4E_Popo,
  &Sprite_50_Cannonball,
  &Sprite_51_ArmosStatue,
  &Sprite_52_KingZora,
  &Sprite_53_ArmosKnight,
  &Sprite_54_Lanmolas,
  &Sprite_55_Zora,
  &Sprite_56_WalkingZora,
  &Sprite_57_DesertStatue,
  &Sprite_58_Crab,
  &Sprite_59_LostWoodsBird,
  &Sprite_5A_LostWoodsSquirrel,
  &Sprite_5B_Spark_Clockwise,
  &Sprite_5B_Spark_Clockwise,
  &Sprite_5D_Roller_VerticalDownFirst,
  &Sprite_5D_Roller_VerticalDownFirst,
  &Sprite_5D_Roller_VerticalDownFirst,
  &Sprite_5D_Roller_VerticalDownFirst,
  &Sprite_61_Beamos,
  &Sprite_62_MasterSword,
  &Sprite_63_DebirandoPit,
  &Sprite_64_Debirando,
  &Sprite_65_ArcheryGame,
  &Sprite_66_WallCannonVerticalLeft,
  &Sprite_66_WallCannonVerticalLeft,
  &Sprite_66_WallCannonVerticalLeft,
  &Sprite_66_WallCannonVerticalLeft,
  &Sprite_6A_BallNChain,
  &Sprite_6B_CannonTrooper,
  &Sprite_6C_MirrorPortal,
  &Sprite_6D_Rat,
  &Sprite_6E_Rope,
  &Sprite_6F_Keese,
  &Sprite_70_KingHelmasaurFireball,
  &Sprite_71_Leever,
  &Sprite_72_FairyPond,
  &Sprite_73_UncleAndPriest,
  &Sprite_RunningMan,
  &Sprite_BottleVendor,
  &Sprite_76_Zelda,
  &Sprite_15_Antifairy,
  &Sprite_78_MrsSahasrahla,
  // Trampoline 68 entries
  &Sprite_79_Bee,
  &Sprite_7A_Agahnim,
  &Sprite_7B_AgahnimBalls,
  &Sprite_7C_GreenStalfos,
  &Sprite_7D_BigSpike,
  &Sprite_7E_Firebar_Clockwise,
  &Sprite_7E_Firebar_Clockwise,
  &Sprite_80_Firesnake,
  &Sprite_81_Hover,
  &Sprite_82_AntifairyCircle,
  &Sprite_83_GreenEyegore,
  &Sprite_83_GreenEyegore,
  &Sprite_85_YellowStalfos,
  &Sprite_86_Kodongo,
  &Sprite_87_KodongoFire,
  &Sprite_88_Mothula,
  &Sprite_89_MothulaBeam,
  &Sprite_8A_SpikeBlock,
  &Sprite_8B_Gibdo,
  &Sprite_8C_Arrghus,
  &Sprite_8D_Arrghi,
  &Sprite_8E_Terrorpin,
  &Sprite_8F_Blob,
  &Sprite_90_Wallmaster,
  &Sprite_91_StalfosKnight,
  &Sprite_92_HelmasaurKing,
  &Sprite_93_Bumper,
  &Sprite_94_Pirogusu,
  &Sprite_95_LaserEyeLeft,
  &Sprite_95_LaserEyeLeft,
  &Sprite_95_LaserEyeLeft,
  &Sprite_95_LaserEyeLeft,
  &Sprite_99_Pengator,
  &Sprite_9A_Kyameron,
  &Sprite_9B_Wizzrobe,
  &Sprite_9C_Zoro,
  &Sprite_9C_Zoro,
  &Sprite_9E_HauntedGroveOstritch,
  &Sprite_9F_HauntedGroveRabbit,
  &Sprite_A0_HauntedGroveBird,
  &Sprite_A1_Freezor,
  &Sprite_A2_Kholdstare,
  &Sprite_A3_KholdstareShell,
  &Sprite_A4_FallingIce,
  &Sprite_Zazak_Main,
  &Sprite_Zazak_Main,
  &Sprite_A7_Stalfos,
  &Sprite_A8_GreenZirro,
  &Sprite_A8_GreenZirro,
  &Sprite_AA_Pikit,
  &Sprite_AB_CrystalMaiden,
  &Sprite_AC_Apple,
  &Sprite_AD_OldMan,
  &Sprite_AE_Pipe_Down,
  &Sprite_AE_Pipe_Down,
  &Sprite_AE_Pipe_Down,
  &Sprite_AE_Pipe_Down,
  &Sprite_B2_PlayerBee,
  &Sprite_B3_PedestalPlaque,
  &Sprite_B4_PurpleChest,
  &Sprite_B5_BombShop,
  &Sprite_B6_Kiki,
  &Sprite_B7_BlindMaiden,
  &Sprite_B8_DialogueTester,
  &Sprite_B9_BullyAndPinkBall,
  &Sprite_BA_Whirlpool,
  &Sprite_BB_Shopkeeper,
  &Sprite_BC_Drunkard,
  // Trampoline 4, starts at 187, 27 entries
  &Sprite_BD_Vitreous,
  &Sprite_BE_VitreousEye,
  &Sprite_BF_Lightning,
  &Sprite_C0_Catfish,
  &Sprite_C1_CutsceneAgahnim,
  &Sprite_C2_Boulder,
  &Sprite_C3_Gibo,
  &Sprite_C4_Thief,
  &Sprite_C5_Medusa,
  &Sprite_C6_4WayShooter,
  &Sprite_C7_Pokey,
  &Sprite_C8_BigFairy,
  &Sprite_C9_Tektite,
  &Sprite_CA_ChainChomp,
  &Sprite_CB_TrinexxRockHead,
  &Sprite_CC,
  &Sprite_CD,
  &Sprite_CE_Blind,
  &Sprite_CF_Swamola,
  &Sprite_D0_Lynel,
  &Sprite_D1_BunnyBeam,
  &Sprite_D2_FloppingFish,
  &Sprite_D3_Stal,
  &Sprite_D4_Landmine,
  &Sprite_D5_DigGameGuy,
  &Sprite_D6_Ganon,
  &Sprite_D6_Ganon,
  &Sprite_D8_Heart,
  &Sprite_D9_GreenRupee,
  &Sprite_D9_GreenRupee,
  &Sprite_D9_GreenRupee,
  &Sprite_D9_GreenRupee,
  &Sprite_D9_GreenRupee,
  &Sprite_D9_GreenRupee,
  &Sprite_D9_GreenRupee,
  &Sprite_D9_GreenRupee,
  &Sprite_D9_GreenRupee,
  &Sprite_D9_GreenRupee,
  &Sprite_E3_Fairy,
  &Sprite_E4_SmallKey,
  &Sprite_E4_SmallKey,
  &Sprite_D9_GreenRupee,
  &Sprite_E7_Mushroom,
  &Sprite_E8_FakeSword,
  &Sprite_E9_PotionShop,
  &Sprite_HeartContainer,
  &Sprite_HeartPiece,
  &Sprite_EC_ThrownItem,
  &Sprite_ED_SomariaPlatform,
  &Sprite_EE_MovableMantle,
  &Sprite_ED_SomariaPlatform,
  &Sprite_ED_SomariaPlatform,
  &Sprite_ED_SomariaPlatform,
  &Sprite_F2_MedallionTablet,
};
static HandlerFuncK *const kSpritePrep_Main[243] = {
  &SpritePrep_Raven,
  &SpritePrep_Vulture,
  &SpritePrep_DoNothingA,
  NULL,
  &SpritePrep_Switch,
  &SpritePrep_DoNothingA,
  &SpritePrep_Switch,
  &SpritePrep_SwitchFacingUp,
  &SpritePrep_Octorok,
  &SpritePrep_Moldorm,
  &SpritePrep_Octorok,
  &SpritePrep_DoNothingA,
  &SpritePrep_DoNothingA,
  &SpritePrep_DoNothingA,
  &SpritePrep_DoNothingA,
  &SpritePrep_Octoballoon,
  &SpritePrep_DoNothingA,
  &SpritePrep_DoNothingA,
  &SpritePrep_DoNothingA,
  &SpritePrep_MiniHelmasaur,
  &SpritePrep_ThievesTownGrate,
  &SpritePrep_Antifairy,
  &SpritePrep_Sage,
  &SpritePrep_DoNothingA,
  &SpritePrep_MiniMoldorm_bounce,
  &SpritePrep_Poe,
  &SpritePrep_Smithy,
  &SpritePrep_DoNothingA,
  &SpritePrep_Statue,
  &SpritePrep_IgnoreProjectiles,
  &SpritePrep_CrystalSwitch,
  &SpritePrep_SickKid,
  &SpritePrep_DoNothingA,
  &SpritePrep_WaterLever,
  &SpritePrep_DoNothingA,
  &SpritePrep_Bari,
  &SpritePrep_Bari,
  &SpritePrep_TalkingTree,
  &SpritePrep_HardhatBeetle,
  &SpritePrep_DoNothingA,
  &SpritePrep_Storyteller,
  &SpritePrep_Adults,
  &SpritePrep_IgnoreProjectiles,
  &SpritePrep_Hobo,
  &SpritePrep_MagicBat,
  &SpritePrep_IgnoreProjectiles,
  &SpritePrep_FluteKid,
  &SpritePrep_IgnoreProjectiles,
  &SpritePrep_IgnoreProjectiles,
  &SpritePrep_FortuneTeller,
  &SpritePrep_IgnoreProjectiles,
  &SpritePrep_RupeePull,
  &SpritePrep_Snitch_bounce_2,
  &SpritePrep_Snitch_bounce_3,
  &SpritePrep_IgnoreProjectiles,
  &SpritePrep_IgnoreProjectiles,
  &SpritePrep_DoNothingA,
  &SpritePrep_Locksmith,
  &SpritePrep_MagicBat,
  &SpritePrep_BonkItem,
  &SpritePrep_IgnoreProjectiles,
  &SpritePrep_Snitch_bounce_1,
  &SpritePrep_DoNothingA,
  &SpritePrep_DoNothingA,
  &SpritePrep_AgahnimsBarrier,
  &SpritePrep_StandardGuard,
  &SpritePrep_StandardGuard,
  &SpritePrep_StandardGuard,
  &SpritePrep_TrooperAndArcherSoldier,
  &SpritePrep_TrooperAndArcherSoldier,
  &SpritePrep_TrooperAndArcherSoldier,
  &SpritePrep_TrooperAndArcherSoldier,
  &SpritePrep_TrooperAndArcherSoldier,
  &SpritePrep_TrooperAndArcherSoldier,
  &SpritePrep_TrooperAndArcherSoldier,
  &SpritePrep_WeakGuard,
  &SpritePrep_Geldman,
  &SpritePrep_Kyameron,
  &SpritePrep_Popo,
  &SpritePrep_Popo2,
  &SpritePrep_DoNothingA,
  &SpritePrep_DoNothingD,
  &SpritePrep_KingZora,
  &SpritePrep_ArmosKnight,
  &SpritePrep_Lanmolas,
  &SpritePrep_SwimmingZora,
  &SpritePrep_WalkingZora,
  &SpritePrep_DesertStatue,
  &SpritePrep_DoNothingA,
  &SpritePrep_LostWoodsBird,
  &SpritePrep_LostWoodsSquirrel,
  &SpritePrep_Spark,
  &SpritePrep_Spark,
  &SpritePrep_Roller_VerticalDownFirst,
  &SpritePrep_RollerUpDown,
  &SpritePrep_Roller_HorizontalRightFirst,
  &SpritePrep_RollerLeftRight,
  &SpritePrep_DoNothingA,
  &SpritePrep_MasterSword,
  &SpritePrep_DebirandoPit,
  &SpritePrep_FireDebirando,
  &SpritePrep_ArrowGame_bounce,
  &SpritePrep_WallCannon,
  &SpritePrep_WallCannon,
  &SpritePrep_WallCannon,
  &SpritePrep_WallCannon,
  &SpritePrep_DoNothingA,
  &SpritePrep_DoNothingA,
  &SpritePrep_DoNothingA,
  &SpritePrep_Rat,
  &SpritePrep_Rope,
  &SpritePrep_Keese,
  &SpritePrep_DoNothingG,
  &SpritePrep_FairyPond,
  &SpritePrep_IgnoreProjectiles,
  &SpritePrep_UncleAndPriest_bounce,
  &SpritePrep_RunningMan,
  &SpritePrep_IgnoreProjectiles,
  &SpritePrep_Zelda_bounce,
  &SpritePrep_Antifairy,
  &SpritePrep_MrsSahasrahla,
  &SpritePrep_OverworldBonkItem,
  &SpritePrep_Agahnim,
  &SpritePrep_DoNothingG,
  &SpritePrep_GreenStalfos,
  &SpritePrep_BigSpike,
  &SpritePrep_FireBar,
  &SpritePrep_FireBar,
  &SpritePrep_DoNothingG,
  &SpritePrep_DoNothingG,
  &SpritePrep_AntifairyCircle,
  &SpritePrep_Eyegore,
  &SpritePrep_Eyegore,
  &SpritePrep_DoNothingG,
  &SpritePrep_Kodongo,
  &SpritePrep_DoNothingG,
  &SpritePrep_Mothula,
  &SpritePrep_DoNothingG,
  &SpritePrep_Spike,
  &SpritePrep_DoNothingG,
  &SpritePrep_Arrghus,
  &SpritePrep_Arrghi,
  &SpritePrep_DoNothingG,
  &SpritePrep_Blob,
  &SpritePrep_DoNothingG,
  &SpritePrep_DoNothingG,
  &SpritePrep_HelmasaurKing,
  &SpritePrep_Bumper,
  &SpritePrep_DoNothingA,
  &SpritePrep_LaserEye_bounce,
  &SpritePrep_LaserEye_bounce,
  &SpritePrep_LaserEye_bounce,
  &SpritePrep_LaserEye_bounce,
  &SpritePrep_DoNothingA,
  &SpritePrep_Kyameron,
  &SpritePrep_DoNothingA,
  &SpritePrep_Zoro,
  &SpritePrep_Babasu,
  &SpritePrep_HauntedGroveOstritch,
  &SpritePrep_HauntedGroveAnimal,
  &SpritePrep_HauntedGroveAnimal,
  &SpritePrep_MoveDown_8px,
  &SpritePrep_Kholdstare,
  &SpritePrep_KholdstareShell,
  &SpritePrep_FallingIce,
  &SpritePrep_Zazakku,
  &SpritePrep_Zazakku,
  &SpritePrep_Stalfos,
  &SpritePrep_Bomber,
  &SpritePrep_Bomber,
  &SpritePrep_DoNothingC,
  &SpritePrep_DoNothingH,
  &SpritePrep_OverworldBonkItem,
  &SpritePrep_OldMan_bounce,
  &SpritePrep_DoNothingA,
  &SpritePrep_DoNothingA,
  &SpritePrep_DoNothingA,
  &SpritePrep_DoNothingA,
  &SpritePrep_NiceBee,
  &SpritePrep_PedestalPlaque,
  &SpritePrep_PurpleChest,
  &SpritePrep_BombShoppe,
  &SpritePrep_Kiki,
  &SpritePrep_BlindMaiden,
  &SpritePrep_DoNothingA,
  &SpritePrep_BullyAndVictim,
  &SpritePrep_Whirlpool,
  &SpritePrep_Shopkeeper,
  &SpritePrep_IgnoreProjectiles,
  &SpritePrep_Vitreous,
  &SpritePrep_MiniVitreous,
  &SpritePrep_DoNothingA,
  &SpritePrep_Catfish,
  &SpritePrep_CutsceneAgahnim,
  &SpritePrep_DoNothingA,
  &SpritePrep_Gibo,
  &SpritePrep_DoNothingA,
  &SpritePrep_IgnoreProjectiles,
  &SpritePrep_IgnoreProjectiles,
  &SpritePrep_Pokey,
  &SpritePrep_BigFairy,
  &SpritePrep_Tektite,
  &SpritePrep_Chainchomp_bounce,
  &SpritePrep_Trinexx,
  &SpritePrep_Trinexx,
  &SpritePrep_Trinexx,
  &SpritePrep_Blind,
  &SpritePrep_Swamola,
  &SpritePrep_DoNothingA,
  &SpritePrep_DoNothingA,
  &SpritePrep_IgnoreProjectiles,
  &SpritePrep_RockStal,
  &SpritePrep_IgnoreProjectiles,
  &SpritePrep_DiggingGameGuy_bounce,
  &SpritePrep_Ganon,
  &SpritePrep_Ganon,
  &SpritePrep_Absorbable,
  &SpritePrep_Absorbable,
  &SpritePrep_Absorbable,
  &SpritePrep_Absorbable,
  &SpritePrep_Absorbable,
  &SpritePrep_Absorbable,
  &SpritePrep_Absorbable,
  &SpritePrep_Absorbable,
  &SpritePrep_Absorbable,
  &SpritePrep_Absorbable,
  &SpritePrep_Absorbable,
  &SpritePrep_Fairy,
  &SpritePrep_SmallKey,
  &SpritePrep_BigKey,
  &SpritePrep_ShieldPickup,
  &SpritePrep_Mushroom,
  &SpritePrep_FakeSword,
  &SpritePrep_PotionShop,
  &SpritePrep_HeartContainer,
  &SpritePrep_HeartPiece,
  &SpritePrep_ThrowableScenery,
  &SpritePrep_DoNothingA,
  &SpritePrep_Mantle,
  &SpritePrep_DoNothingA,
  &SpritePrep_DoNothingA,
  &SpritePrep_DoNothingA,
  &SpritePrep_MedallionTable,
};

void Sprite_PullSwitch_bounce(int k) {
  if (sprite_type[k] == 5 || sprite_type[k] == 7)
    PullSwitch_FacingUp(k);
  else
    PullSwitch_FacingDown(k);
}

void GiantMoldorm_DrawSegment_AB(int k, int lookback) {
  static const DrawMultipleData kGiantMoldorm_SegA_Dmd[8] = {
    {-8, -8, 0x0084, 2},
    { 8, -8, 0x0086, 2},
    {-8,  8, 0x00a4, 2},
    { 8,  8, 0x00a6, 2},
    {-8, -8, 0x4086, 2},
    { 8, -8, 0x4084, 2},
    {-8,  8, 0x40a6, 2},
    { 8,  8, 0x40a4, 2},
  };
  int j = sprite_subtype2[k] - lookback & 0x7f;
  cur_sprite_x = moldorm_x_lo[j] | moldorm_x_hi[j] << 8;
  cur_sprite_y = moldorm_y_lo[j] | moldorm_y_hi[j] << 8;
  oam_cur_ptr += 0x10;
  oam_ext_cur_ptr += 4;
  Sprite_DrawMultiple(k, &kGiantMoldorm_SegA_Dmd[(sprite_subtype2[k] >> 1 & 1) * 4], 4, NULL);
}

void GiantMoldorm_DrawSegment_C_OrTail(int k, int lookback) {
  static const uint8 kGiantMoldorm_OamFlags[4] = {0, 0x40, 0xc0, 0x80};
  int j = sprite_subtype2[k] - lookback & 0x7f;
  cur_sprite_x = moldorm_x_lo[j] | moldorm_x_hi[j] << 8;
  cur_sprite_y = moldorm_y_lo[j] | moldorm_y_hi[j] << 8;
  uint8 bak = sprite_oam_flags[k];
  sprite_oam_flags[k] = (bak & 0x3f) | kGiantMoldorm_OamFlags[sprite_subtype2[k] >> 1 & 3];
  SpriteDraw_SingleLarge(k);
  sprite_oam_flags[k] = bak;
}

void Chicken_IncrSubtype2(int k, int j) {
  sprite_subtype2[k] += j;
  sprite_graphics[k] = (sprite_subtype2[k] >> 4) & 1;
  Sprite_ReturnIfLifted(k);
}

bool Octoballoon_Find() {
  for (int i = 15; i >= 0; i--) {
    if (sprite_state[i] && sprite_type[i] == 0x10)
      return true;
  }
  return false;
}

bool FluteBoy_CheckIfPlayerClose(int k) {
  int xx = Sprite_GetX(k);
  int yy = Sprite_GetY(k) - 16;
  int x = link_x_coord - xx - (yy < 0); // zelda bug: carry
  int y = link_y_coord - yy - (x < 0);
  if (sign16(x)) x = ~x;
  if (sign16(y)) y = ~y;
  return (uint16)x < 48 && (uint16)y < 48;
}

void FortuneTeller_LightOrDarkWorld(int k, bool dark_world) {
  int j;
  static const uint8 kFortuneTeller_Prices[4] = {10, 15, 20, 30};

  switch (sprite_ai_state[k]) {
  case 0:  // WaitForInquiry
    sprite_graphics[k] = 0;
    sprite_A[k] = (j = (GetRandomNumber() & 3)) << 1;
    if (link_rupees_goal < kFortuneTeller_Prices[j])
      sprite_ai_state[k] = 1;
    else
      sprite_ai_state[k] = 2;
    break;
  case 1: // NotEnoughRupees
    Sprite_ShowSolicitedMessage(k, 0xf2);
    break;
  case 2:  // AskIfPlayerWantsReading
    if (Sprite_ShowSolicitedMessage(k, 0xf3) & 0x100) {
      sprite_delay_main[k] = 255;
      flag_is_link_immobilized = 1;
      sprite_ai_state[k]=3;
    }
    break;
  case 3: // ReactToPlayerResponse
    if (!choice_in_multiselect_box) {
      if (!sprite_delay_main[k])
        sprite_ai_state[k]++;
      sprite_graphics[k] = frame_counter >> 4 & 1;
    } else {
      Sprite_ShowMessageUnconditional(0xf5);
      sprite_ai_state[k] = 2;
      flag_is_link_immobilized = 0;
    }
    break;
  case 4: // FortuneTeller_PerformPseudoScience
    FortuneTeller_PerformPseudoScience(k);
    break;
  case 5:  // ShowCostMessage
    if (!dark_world)
      sprite_graphics[k] = 0;
    j = kFortuneTeller_Prices[sprite_A[k]>>1];
    dialogue_number[0] = (j / 10) | (j % 10)<< 4 ;
    dialogue_number[1] = 0;
    Sprite_ShowMessageUnconditional(0xf4);
    sprite_ai_state[k]++;
    break;
  case 6:  // DeductPayment
    link_rupees_goal -= kFortuneTeller_Prices[sprite_A[k]>>1];
    sprite_ai_state[k]++;
    link_hearts_filler = 160;
    flag_is_link_immobilized = 0;
    break;
  case 7:
    break;
  }
}

int GarnishAllocForce() {
  int k = 29;
  while (garnish_type[k--] && k >= 0) {}
  return k + 1;
}

int GarnishAlloc() {
  int k = 29;
  while (garnish_type[k] && k >= 0) k--;
  return k;
}

int GarnishAllocLow() {
  int k = 14;
  while (garnish_type[k] && k >= 0) k--;
  return k;
}

int GarnishAllocLimit(int k) {
  while (garnish_type[k] && k >= 0) k--;
  return k;
}

int GarnishAllocOverwriteOldLow() {
  int k = 14;
  while (garnish_type[k] && k >= 0) k--;
  if (k < 0) {
    if (sign8(--byte_7E0FF8))
      byte_7E0FF8 = 14;
    k = byte_7E0FF8;
  }
  return k;
}

int GarnishAllocOverwriteOld() {
  int k = 29;
  while (garnish_type[k] && k >= 0) k--;
  if (k < 0) {
    if (sign8(--byte_7E0FF8))
      byte_7E0FF8 = 29;
    k = byte_7E0FF8;
  }
  return k;
}

void Garnish_SetX(int k, uint16 x) {
  garnish_x_lo[k] = x;
  garnish_x_hi[k] = x >> 8;
}

void Garnish_SetY(int k, uint16 y) {
  garnish_y_lo[k] = y;
  garnish_y_hi[k] = y >> 8;
}

void Sprite_WishPond3(int k) {
  int j;

  switch (sprite_ai_state[k]) {
  case 0:
    flag_is_link_immobilized = 0;
    if (sprite_delay_main[k] || Sprite_CheckIfLinkIsBusy())
      return;
    if (Sprite_ShowMessageOnContact(k, 0x14a) & 0x100) {
      sprite_ai_state[k] = 1;
      Link_ResetProperties_A();
      link_direction_facing = 0;
      sprite_head_dir[k] = 0;
    }
    break;
  case 1:
    if (!choice_in_multiselect_box) {
      Sprite_ShowMessageUnconditional(0x8a);
      sprite_ai_state[k] = 2;
      flag_is_link_immobilized = 1;
    } else {
      Sprite_ShowMessageUnconditional(0x14b);
      sprite_ai_state[k] = 0;
      sprite_delay_main[k] = 255;
    }
    break;
  case 2: {
    sprite_ai_state[k] = 3;
    j = choice_in_multiselect_box;
    sprite_C[k] = j;
    uint8 item = (&link_item_bow)[j];
    (&link_item_bow)[j] = 0;
    uint8 t = kWishPondItemData[kWishPondItemOffs[j] + ((j == 3 || j == 32) ? 1 : item) - 1];
    AncillaAdd_TossedPondItem(0x28, t, 4);
    Hud_RefreshIcon();
    sprite_graphics[k] = t;
    sprite_D[k] = item;
    sprite_delay_main[k] = 255;
    break;
  }
  case 3:
    if (sprite_delay_main[k] == 0) {
      SpriteSpawnInfo info;
      int j = Sprite_SpawnDynamically(k, 0x72, &info);
      assert(j >= 0);
      Sprite_SetX(j, info.r0_x);
      Sprite_SetY(j, info.r2_y - 80);
      music_control = 0x1b;
      last_music_control = 0;
      sprite_B[j] = 1;
      Palette_AssertTranslucencySwap();
      PaletteFilter_WishPonds();
      sprite_E[k] = j;
      sprite_ai_state[k] = 4;
      sprite_delay_main[k] = 255;
    }
    break;
  case 4:
    if (!(frame_counter & 7)) {
      PaletteFilter_SP5F();
      if (!BYTE(palette_filter_countdown)) {
        Sprite_ShowMessageUnconditional(0x8b);
        Palette_RevertTranslucencySwap();
        TS_copy = 0;
        CGADSUB_copy = 0x20;
        flag_update_cgram_in_nmi++;
        sprite_ai_state[k] = 5;
      }
    }
    break;
  case 5:
    if (!choice_in_multiselect_box) {
      sprite_ai_state[k] = 6;
    } else {
      sprite_ai_state[k] = 11;
    }
    break;
  case 6:
    sprite_ai_state[k] = 7;
    if (!savegame_is_darkworld) {
      if (sprite_graphics[k] == 12) {
        sprite_graphics[k] = 42;
        sprite_head_dir[k] = 1;
      } else if (sprite_graphics[k] == 4) {
        sprite_graphics[k] = 5;
        sprite_head_dir[k] = 2;
      } else if (sprite_graphics[k] == 22) {
        sprite_graphics[k] = 44;
        sprite_head_dir[k] = 3;
      } else {
        Sprite_ShowMessageUnconditional(0x14d);
        return;
      }
    } else {
      if (sprite_graphics[k] == 58) {
        sprite_graphics[k] = 59;
        sprite_head_dir[k] = 4;
        Sprite_ShowMessageUnconditional(0x14f);
        return;
      } else if (sprite_graphics[k] == 2) {
        sprite_graphics[k] = 3;
        sprite_head_dir[k] = 5;
      } else if (sprite_graphics[k] == 22) {
        sprite_graphics[k] = 44;
        sprite_head_dir[k] = 3;
      } else {
        Sprite_ShowMessageUnconditional(0x14d);
        return;
      }
    }
    Sprite_ShowMessageUnconditional(0x8c);
    break;
  case 7:
    if (sprite_C[k] == 3)
      (&link_item_bow)[sprite_C[k]] = sprite_D[k];
    Palette_AssertTranslucencySwap();
    TS_copy = 2;
    CGADSUB_copy = 0x30;
    flag_update_cgram_in_nmi++;
    sprite_ai_state[k] = 8;
    break;
  case 8:
    if (!(frame_counter & 7)) {
      PaletteFilter_SP5F();
      if (BYTE(palette_filter_countdown) == 30) {
        sprite_state[sprite_E[k]] = 0;
      } else if (BYTE(palette_filter_countdown) == 0) {
        sprite_ai_state[k] = 9;
      }
    }
    break;
  case 9:
    PaletteFilter_RestoreSP5F();
    Palette_RevertTranslucencySwap();
    item_receipt_method = 2;
    Link_ReceiveItem(sprite_graphics[k], 0);
    sprite_ai_state[k] = 10;
    break;
  case 10: {
    static const uint8 kWishPondMsgs[5] = { 0x8f, 0x90, 0x92, 0x91, 0x93 };
    if (sprite_head_dir[k])
      Sprite_ShowMessageUnconditional(kWishPondMsgs[sprite_head_dir[k] - 1]);
    sprite_ai_state[k] = 0;
    sprite_delay_main[k] = 255;
    break;
  }
  case 11:
    Sprite_ShowMessageUnconditional(0x8d);
    sprite_ai_state[k] = 12;
    break;
  case 12:
    if (!choice_in_multiselect_box)
      sprite_ai_state[k] = 13;
    else
      sprite_ai_state[k] = 6;
    break;
  case 13:
    Sprite_ShowMessageUnconditional(0x8e);
    sprite_ai_state[k] = 7;
    break;
  }
}

int Sprite_SpawnSmallSplash(int k) {
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamicallyEx(k, 0xec, &info, 14);
  if (j >= 0) {
    Sprite_SetSpawnedCoordinates(j, &info);
    sound_effect_1 = 0;
    SpriteSfx_QueueSfx2WithPan(k, 0x28);
    sprite_state[j] = 3;
    sprite_delay_main[j] = 15;
    sprite_ai_state[j] = 0;
    sprite_flags2[j] = 3;
  }
  return j;
}

void HeartUpgrade_CheckIfAlreadyObtained(int k) {
  if (!player_is_indoors) {
    if (BYTE(overworld_screen_index) == 0x3b && !(save_ow_event_info[0x3b] & 0x20) ||
      save_ow_event_info[BYTE(overworld_screen_index)] & 0x40)
      sprite_state[k] = 0;
  } else {
    int j = sprite_x_hi[k] & 1;
    if (dung_savegame_state_bits & (j ? 0x2000 : 0x4000))
      sprite_state[k] = 0;
  }
}

void Sprite_EE_MovableMantle(int k) {
  MovableMantle_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (!Sprite_CheckDamageToLink_same_layer(k))
    return;
  Sprite_NullifyHookshotDrag();
  Sprite_RepelDash();

  if (follower_indicator != 1 || !link_item_torch || link_is_running || sprite_G[k] == 0x90 || sign8(link_actual_vel_x - 24))
    return;

  which_starting_point = 4;
  sprite_subtype2[k]++;

  if (!(sprite_subtype2[k] & 1))
    sprite_G[k]++;

  if (sprite_G[k] < 8)
    return;
  if (sound_effect_1 == 0)
    sound_effect_1 = 34;
  sprite_x_vel[k] = 2;
  Sprite_MoveXY(k);
}

void Sprite_GoodOrBadArcheryTarget(int k) {
  static const uint8 kArcheryGame_CashPrize[10] = {4, 8, 16, 32, 64, 99, 99, 99, 99, 99};
  if (sprite_A[k] == 1) {
    // good target
    if (sprite_G[k] >= 5)
      sprite_B[k] = 6;
    sprite_flags2[k] &= ~0x1f;
    int j = sprite_delay_aux2[k] ? sprite_delay_aux2[k] : (sprite_subtype2[k] >> 3);
    sprite_oam_flags[k] = sprite_oam_flags[k] & ~0x40 | (j & 4) << 4;
    BYTE(cur_sprite_y) -= 3;
    SpriteDraw_SingleLarge(k);
    if (sprite_delay_aux2[k]) {
      if (sprite_delay_aux2[k] == 96 && !submodule_index) {
        sprite_delay_main[0] = 112;
        link_rupees_goal += kArcheryGame_CashPrize[sprite_B[k] - 1];
      }
      sprite_flags2[k] |= 5;
      ArcheryGame_DrawPrize(k);
    }
  } else {
    // bad target
    sprite_flags2[k] &= ~0x1f;
    BYTE(cur_sprite_y) += 3;
    SpriteDraw_SingleLarge(k);
  }
  if (Sprite_ReturnIfInactive(k))
    return;

  if (sprite_delay_aux3[k] == 1)
    sound_effect_1 = 0x3c;
  sprite_subtype2[k]++;
  Sprite_MoveX(k);
  if (!sprite_delay_aux1[k]) {
    sprite_ignore_projectile[k] = sprite_delay_main[k];
    if (sprite_delay_main[k] == 0) {
      if (Sprite_CheckTileCollision(k)) {
        sprite_delay_main[k] = 16;
        sprite_delay_aux2[k] = 0;
      }
    } else if (sprite_delay_main[k] == 1) {
      static const int8 kArcheryTarget_X[2] = {-24, 8};
      sprite_x_lo[k] = kArcheryTarget_X[sprite_graphics[k]];
      sprite_x_hi[k] = link_x_coord >> 8;
      sprite_delay_aux1[k] = 32;
      sprite_G[k] = 0;
    }
  }
}

static inline uint8 ChainBallMult(uint16 a, uint8 b) {
  if (a >= 256)
    return b;
  int p = a * b;
  return (p >> 8) + (p >> 7 & 1);
}

void ChainBallTrooper_Draw(int k) {
  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  SpriteDraw_GuardHead(k, &info, 0x18 / 4);
  SpriteDraw_BNCBody(k, &info, 0x14 / 4);
  SpriteDraw_BNCFlail(k, &info);

  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  if (sprite_flags3[k] & 0x10)
    SpriteDraw_Shadow_custom(k, &info, kSoldier_DrawShadow[sprite_D[k]]);
}

void Sprite_6B_CannonTrooper(int k) {
  if (sprite_C[k] != 0) {
    Sprite_Cannonball(k);
    return;
  }
  assert(0);
}

void Bee_PutInBottle(int k) {
  Bee_HandleInteractions(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (!choice_in_multiselect_box) {
    int j = Sprite_Find_EmptyBottle();
    if (j >= 0) {
      link_bottle_info[j] = 7 + sprite_head_dir[k];
      Hud_RefreshIcon();
      sprite_state[k] = 0;
      return;
    }
    Sprite_ShowMessageUnconditional(0xca);
  }
  sprite_delay_aux4[k] = 64;
  sprite_ai_state[k] = 1;
}

static inline uint8 GuruguruBarMult(uint16 a, uint8 b) {
  if (a >= 256)
    return b;
  int p = a * b;
  return (p >> 8) + (p >> 7 & 1);
}

static inline int8 GuruguruBarSin(uint16 a, uint8 b) {
  uint8 t = GuruguruBarMult(kSinusLookupTable[a & 0xff], b);
  return (a & 0x100) ? -t : t;
}

static inline uint8 ArrgiMult(uint16 a, uint8 b) {
  if (a >= 256)
    return b;
  int p = a * b;
  return (p >> 8) + (p >> 7 & 1);
}

static inline int8 ArrgiSin(uint16 a, uint8 b) {
  uint8 t = ArrgiMult(kSinusLookupTable[a & 0xff], b);
  return (a & 0x100) ? -t : t;
}

static inline uint8 HelmasaurMult(uint16 a, uint8 b) {
  if (a >= 256)
    return b;
  int p = a * b;
  return (p >> 8) + (p >> 7 & 1);
}

static inline int8 HelmasaurSin(uint16 a, uint8 b) {
  uint8 t = HelmasaurMult(kSinusLookupTable[a & 0xff], b);
  return (a & 0x100) ? -t : t;
}

void Sprite_Wizzbeam(int k) {
  Wizzbeam_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  sprite_oam_flags[k] ^= 6;
  sprite_subtype2[k]++;
  if (!sprite_ai_state[k])
    Sprite_CheckDamageToLink(k);
  Sprite_MoveXY(k);
  if (Sprite_CheckTileCollision(k))
    sprite_state[k] = 0;
}

void Kiki_LyingInwait(int k) {
  PrepOamCoordsRet info;
  Sprite_PrepOamCoord(k, &info);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (link_is_bunny_mirror | link_disable_sprite_damage | countdown_for_blink || follower_indicator == 10)
    return;
  if (save_ow_event_info[BYTE(overworld_screen_index)] & 0x20)
    return;
  if (Sprite_CheckDamageToLink_same_layer(k)) {
    // Kiki Big Bomb Fix
    if (enhanced_features0 & kFeatures0_MiscBugFixes)
      follower_dropped = 0;  // defuse bomb

    follower_indicator = 10;
    tagalong_var5 = 0;
    LoadFollowerGraphics();
    Follower_Initialize();
  }
}

int ChainChomp_OneMult(uint8 a, uint8 b) {
  uint8 at = sign8(a) ? -a : a;
  uint8 prod = at * b >> 8;
  return sign8(a) ? ~prod : prod;
}

static inline uint8 TrinexxMult(uint8 a, uint8 b) {
  uint8 at = sign8(a) ? -a : a;
  int p = at * b;
  uint8 res = (p >> 8) + (p >> 7 & 1);
  return sign8(a) ? -res : res;
}

static inline uint8 TrinexxHeadMult(uint16 a, uint8 b) {
  if (a >= 256)
    return b;
  int p = a * b;
  return (p >> 8) + (p >> 7 & 1);
}

static inline int8 TrinexxHeadSin(uint16 a, uint8 b) {
  uint8 t = TrinexxHeadMult(kSinusLookupTable[a & 0xff], b);
  return (a & 0x100) ? -t : t;
}

void Sprite_CC(int k) {
  if (!sprite_E[k]) {
    Sprite_Sidenexx(k);
    return;
  }
  PrepOamCoordsRet info;
  Sprite_PrepOamCoord(k, &info);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_MoveXY(k);
  Sprite_TrinexxFire_AddFireGarnish(k);
  Sprite_CC_CD_Common(k);
}

void Sprite_CD(int k) {
  if (!sprite_E[k]) {
    Sprite_Sidenexx(k);
    return;
  }
  PrepOamCoordsRet info;
  Sprite_PrepOamCoord(k, &info);
  if (Sprite_ReturnIfInactive(k))
    return;
  uint8 old_xvel = sprite_x_vel[k];
  sprite_x_vel[k] += sprite_C[k];
  Sprite_MoveXY(k);
  sprite_x_vel[k] = old_xvel;
  Sprite_CD_SpawnGarnish(k);
  Sprite_CC_CD_Common(k);
}

static inline uint8 GanonMult(uint16 a, uint8 b) {
  if (a >= 256)
    return b;
  int p = a * b;
  return (p >> 8) + (p >> 7 & 1);
}

static inline int8 GanonSin(uint16 a, uint8 b) {
  uint8 t = GanonMult(kSinusLookupTable[a & 0xff], b);
  return (a & 0x100) ? -t : t;
}

void SpritePrep_IncrXYLow8(int k) {
  sprite_x_lo[k] += 8;
  sprite_y_lo[k] += 8;
}

void SpritePrep_FakeSword(int k) {
}

void SpritePrep_MedallionTable(int k) {
  sprite_ignore_projectile[k]++;
  if (BYTE(overworld_screen_index) != 3) {
    sprite_x_lo[k] += 8;
    if (link_item_bombos_medallion) {
      sprite_graphics[k] = 4;
      sprite_ai_state[k] = 3;
    }
  } else {
    if (link_item_ether_medallion) {
      sprite_graphics[k] = 4;
      sprite_ai_state[k] = 3;
    }
  }
}

void Hobo_Draw(int k) {  // 84ea60
  static const DrawMultipleData kHobo_Dmd[12] = {
    {-5,   3, 0x00a6, 2},
    { 3,   3, 0x00a7, 2},
    {-5,   3, 0x00a6, 2},
    { 3,   3, 0x00a7, 2},
    {-5,   3, 0x00ab, 0},
    { 3,   3, 0x00a7, 2},
    {-5,   3, 0x00a6, 2},
    { 3,   3, 0x00a7, 2},
    { 5, -11, 0x008a, 2},
    {-5,   3, 0x00ab, 0},
    { 3,   3, 0x0088, 2},
    {-5,   3, 0x00a6, 2},
  };
  Sprite_DrawMultiplePlayerDeferred(k, &kHobo_Dmd[sprite_graphics[k] * 4], 4, NULL);
}

bool Landmine_CheckDetonationFromHammer(int k) {  // 84ea81
  if (!(link_item_in_hand & 10) || player_oam_y_offset == 0x80)
    return false;
  SpriteHitBox hb;
  Player_SetupActionHitBox(&hb);
  Sprite_SetupHitBox(k, &hb);
  return CheckIfHitBoxesOverlap(&hb);
}

void Sprite_DrawLargeWaterTurbulence(int k) {  // 84ebe5
  static const DrawMultipleData kWaterTurbulence_Dmd[6] = {
    {-10, 14, 0x00c0, 2},
    { -5, 16, 0x40c0, 2},
    { -2, 18, 0x00c0, 2},
    {  2, 18, 0x40c0, 2},
    {  5, 16, 0x00c0, 2},
    { 10, 14, 0x40c0, 2},
  };
  uint8 bak = sprite_oam_flags[k];
  sprite_oam_flags[k] = (sprite_subtype2[k] >> 1 & 1) ? 0x44 : 4;
  sprite_obj_prio[k] &= ~0xf;
  Oam_AllocateFromRegionC(sprite_obj_prio[k]);  // wtf?????
  Sprite_DrawMultiple(k, kWaterTurbulence_Dmd, 6, NULL);
  sprite_oam_flags[k] = bak;
}

void Sprite_SpawnSparkleGarnish(int k) {  // 858008
  static const int8 kSparkleGarnish_Coord[4] = {-4, 0, 4, 8};
  if (frame_counter & 3)
    return;
  int j = GarnishAllocForce();
  garnish_type[j] = 0x12;
  garnish_active = 0x12;
  int x = Sprite_GetX(k) + kSparkleGarnish_Coord[GetRandomNumber() & 3];
  int y = Sprite_GetY(k) + kSparkleGarnish_Coord[GetRandomNumber() & 3];
  garnish_x_lo[j] = x;
  garnish_x_hi[j] = x >> 8;
  garnish_y_lo[j] = y;
  garnish_y_hi[j] = y >> 8;
  garnish_sprite[j] = k;
  garnish_countdown[j] = 15;
}

void Sprite_70_KingHelmasaurFireball(int k) {  // 85807f
  static const uint8 kHelmasaurFireball_Char[3] = {0xcc, 0xcc, 0xca};
  static const uint8 kHelmasaurFireball_Flags[2] = {0x33, 0x73};
  static const uint8 kHelmasaurFireball_Gfx[4] = {2, 2, 1, 0};
  OamEnt *oam = GetOamCurPtr();
  uint8 flags = kHelmasaurFireball_Flags[++sprite_subtype2[k] >> 2 & 1];

  if ((uint8)((oam->x = sprite_x_lo[k] - BG2HOFS_copy2) + 32) < 64 ||
      (uint8)((oam->y = sprite_y_lo[k] - BG2VOFS_copy2) + 16) < 32) {
    sprite_state[k] = 0;
    return;
  }
  oam->charnum = kHelmasaurFireball_Char[sprite_graphics[k]];
  oam->flags = flags;
  bytewise_extended_oam[oam - oam_buf] = 2;

  if (Sprite_ReturnIfInactive(k))
    return;
  if (!((k ^ frame_counter) & 3) &&
      (uint16)(link_x_coord - cur_sprite_x + 8) < 16 &&
      (uint16)(link_y_coord - cur_sprite_y + 16) < 16) {
    Sprite_AttemptDamageToLinkPlusRecoil(k);
  }
  switch (sprite_ai_state[k]) {
  case 0:  // pre migrate down
    if (!sprite_delay_main[k]) {
      sprite_delay_main[k] = 18;
      sprite_ai_state[k] = 1;
      sprite_y_vel[k] = 36;
    }
    break;
  case 1:  // migrate down
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k] = 2;
      sprite_delay_main[k] = 31;
    }
    sprite_y_vel[k] -= 2;
    Sprite_MoveY(k);
    break;
  case 2:  // delay tri split
    if (!sprite_delay_main[k])
      HelmasaurFireball_TriSplit(k);
    else
      sprite_graphics[k] = kHelmasaurFireball_Gfx[sprite_delay_main[k] >> 3];
    break;
  case 3:  // delay quad split
    if (!sprite_delay_main[k]) {
      HelmasaurFireball_QuadSplit(k);
    } else if (sprite_head_dir[k] < 20) {
      sprite_head_dir[k]++;
      Sprite_MoveXY(k);
    }
    break;
  case 4:  // move
    Sprite_MoveXY(k);
    break;
  }
}

void Sprite_66_WallCannonVerticalLeft(int k) {  // 858090
  static const int8 kWallCannon_Xvel[4] = {0, 0, -16, 16};
  static const int8 kWallCannon_Yvel[4] = {-16, 16, 0, 0};
  static const uint8 kWallCannon_Gfx[4] = {0, 0, 2, 2};
  static const uint8 kWallCannon_OamFlags[4] = {0x40, 0, 0, 0x80};
  static const int8 kWallCannon_Spawn_X[4] = {8, -8, 0, 0};
  static const int8 kWallCannon_Spawn_Y[4] = {0, 0, 8, -8};
  static const int8 kWallCannon_Spawn_Xvel[4] = {24, -24, 0, 0};
  static const int8 kWallCannon_Spawn_Yvel[4] = {0, 0, 24, -24};

  int j = sprite_D[k];
  sprite_graphics[k] = kWallCannon_Gfx[j] + (sprite_delay_aux2[k] != 0);
  sprite_oam_flags[k] = sprite_oam_flags[k] & ~0x40 | kWallCannon_OamFlags[j];
  SpriteDraw_SingleLarge(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (!sprite_delay_main[k]) {
    sprite_delay_main[k] = 128;
    sprite_A[k] ^= 1;
  }
  j = sprite_A[k];
  sprite_x_vel[k] = kWallCannon_Xvel[j];
  sprite_y_vel[k] = kWallCannon_Yvel[j];
  Sprite_MoveXY(k);

  if (!((k << 2) + frame_counter & 31))
    sprite_delay_aux2[k] = 16;
  if (sprite_delay_aux2[k] != 1 || sprite_pause[k])
    return;
  SpriteSpawnInfo info;
  j = Sprite_SpawnDynamicallyEx(k, 0x6B, &info, 13);
  if (j >= 0) {
    SpriteSfx_QueueSfx3WithPan(k, 0x7);
    sprite_C[j] = 1;
    sprite_graphics[j] = 1;
    int i = sprite_D[k];
    Sprite_SetX(j, info.r0_x + kWallCannon_Spawn_X[i]);
    Sprite_SetY(j, info.r2_y + kWallCannon_Spawn_Y[i]);
    sprite_x_vel[j] = kWallCannon_Spawn_Xvel[i];
    sprite_y_vel[j] = kWallCannon_Spawn_Yvel[i];
    sprite_flags2[j] = sprite_flags2[j] & 0xf0 | 1;
    sprite_flags3[j] |= 0x47;
    sprite_defl_bits[j] |= 0x44;
    sprite_delay_main[j] = 32;
  }
}

void Sprite_65_ArcheryGame(int k) {  // 8581ff
  link_num_arrows = sprite_subtype[k];
  if (sprite_A[k] == 0)
    ArcheryGame_Host(k);
  else
    Sprite_GoodOrBadArcheryTarget(k);
}

void ArcheryGame_Host(int k) {  // 858217
  if (!archery_game_arrows_left)
    archery_game_out_of_arrows++;
  ArcheryGameGuy_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  sprite_flags4[k] = 0;
  if (Sprite_CheckDamageToLink_same_layer(k)) {
    Sprite_NullifyHookshotDrag();
    link_speed_setting = 0;
    Link_CancelDash();
  }
  if (sprite_delay_main[k]) {
    if (!(sprite_delay_main[k] & 7))
      SpriteSfx_QueueSfx2WithPan(k, 0x11);
    sprite_graphics[k] = (sprite_delay_main[k] & 4) >> 2;
  } else {
    static const uint8 kArcheryGameGuy_Gfx[4] = {3, 4, 3, 2};
    sprite_graphics[k] = kArcheryGameGuy_Gfx[sprite_ai_state[k] ? (frame_counter >> 5 & 3) : 0];
  }

  switch (sprite_ai_state[k]) {
  case 0:
    sprite_flags4[k] = 10;
    if (Sprite_CheckDamageToLink_same_layer(k) && filtered_joypad_L & 0x80) {
      sprite_ai_state[k] = 1;
      ArcheryGameGuy_ShowMsg(k, 0x85);
    }
    break;
  case 1:
  case 3:
    if (!choice_in_multiselect_box && link_rupees_goal >= 20) {
      sprite_head_dir[k] = 0;
      byte_7E0B88 = 0;
      sprite_ai_state[k] = 2;
      ArcheryGameGuy_ShowMsg(k, 0x86);
    } else {
      sprite_ai_state[k] = 0;
      ArcheryGameGuy_ShowMsg(k, 0x87);
    }
    break;
  case 2:
    ArcheryGame_Host_ProctorGame(k);
    break;
  }
}

void ArcheryGameGuy_ShowMsg(int k, int msg) {  // 8582bf
  dialogue_message_index = msg;
  Sprite_ShowMessageMinimal();
  sprite_delay_main[k] = 0;
}

void ArcheryGame_Host_ProctorGame(int k) {  // 8582d4
  static const uint8 kArcheryGame_NumSpr[6] = {5, 4, 3, 2, 1, 0};
  static const int8 kArcheryGame_X[18] = { 0, 0, 0, 0, 48, 48, 48, 48, 8, 8, 16, 16, 24, 24, 32, 32, 40, 40, };
  static const int8 kArcheryGame_Y[18] = {-8, 0, 8, 16, -8, 0, 8, 16, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8 };
  static const uint8 kArcheryGame_Char[18] = { 0x2b, 0x3b, 0x3b, 0x2b, 0x2b, 0x3b, 0x3b, 0x2b, 0x63, 0x73, 0x63, 0x73, 0x63, 0x73, 0x63, 0x73, 0x63, 0x73 };
  static const uint8 kArcheryGame_Flags[18] = { 0x33, 0x33, 0xb3, 0xb3, 0x73, 0x73, 0xf3, 0xf3, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32 };

  if (!sprite_head_dir[k]) {
    archery_game_arrows_left = 5;
    Sprite_InitializeSecondaryItemMinigame(2);
    sprite_delay_aux1[k] = 39;
    link_rupees_goal -= 20;
    sprite_head_dir[k]++;
  }
  Oam_AllocateFromRegionA(0x34);
  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  OamEnt *oam = GetOamCurPtr();
  int i = sprite_delay_aux1[k] ? kArcheryGame_NumSpr[sprite_delay_aux1[k] >> 3] : archery_game_arrows_left;
  i = i * 2 + 7;
  do {
    SetOamPlain(oam, info.x - 20 + kArcheryGame_X[i] + 1, info.y - 48 + kArcheryGame_Y[i] + 1,
                 kArcheryGame_Char[i], kArcheryGame_Flags[i], 0);
  } while (oam++, --i >= 0);

  if (archery_game_arrows_left | sprite_delay_aux4[k] |
      ancilla_type[0] | ancilla_type[1] | ancilla_type[2] | ancilla_type[3] | ancilla_type[4])
    return;
  sprite_flags4[k] = 0xA;
  if (Sprite_CheckDamageToLink_same_layer(k) && filtered_joypad_L & 0x80) {
    ArcheryGameGuy_ShowMsg(k, 0x88);
    sprite_ai_state[k] = 3;
  }
}

void ArcheryGame_DrawPrize(int k) {  // 8584cf
  static const int8 kGoodArcheryTarget_X[5] = {-8, -8, 0, 8, 16};
  static const int8 kGoodArcheryTarget_Y[5] = {-24, -16, -20, -20, -20};
  static const uint8 kGoodArcheryTarget_Draw_Char[3] = {0xb, 0x1b, 0xb6};
  static const int8 kGoodArcheryTarget_Draw_Flags[5] = {0x38, 0x38, 0x34, 0x35, 0x35};
  static const uint8 kGoodArcheryTarget_Draw_Char3[6] = {0x12, 0x32, 0x31, 3, 0x22, 0x33};
  static const uint8 kGoodArcheryTarget_Draw_Char4[6] = {0x7c, 0x7c, 0x22, 2, 0x12, 0x33};
  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  OamEnt *oam = GetOamCurPtr() + 1;
  int b = sprite_B[k];
  for (int i = 4; i >= 0; i--, oam++) {
    uint8 charnum = (i == 4) ? kGoodArcheryTarget_Draw_Char4[b - 1] :
                    (i == 3) ? kGoodArcheryTarget_Draw_Char3[b - 1] : kGoodArcheryTarget_Draw_Char[i];
    SetOamPlain(oam, info.x + kGoodArcheryTarget_X[i], info.y + kGoodArcheryTarget_Y[i], charnum,
                 kGoodArcheryTarget_Draw_Flags[i] & (charnum < 0x7c ? 0xff : 0xfe), 0);
  }
  Sprite_DrawDistress_custom(info.x, info.y, frame_counter);
}

void Sprite_63_DebirandoPit(int k) {  // 858531
  static const uint8 kDebirandoPit_OpeningGfx[4] = {5, 4, 3, 3};
  static const uint8 kDebirandoPit_ClosingGfx[4] = {3, 3, 4, 5};

  PointU8 pt;
  Sprite_DirectionToFaceLink(k, &pt);
  if ((uint8)(pt.y + 0x20) < 0x40 && (uint8)(pt.x + 0x20) < 0x40)
    Oam_AllocateFromRegionB(16);

  DebirandoPit_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  int j = sprite_head_dir[k];
  if (sprite_state[j] == 6) {
    sprite_state[k] = sprite_state[j];
    sprite_delay_main[k] = sprite_delay_main[j];
    sprite_flags2[k] += 4;
    return;
  }
  if (sprite_graphics[k] < 3 && Sprite_CheckDamageToLink_same_layer(k)) {
    Link_CancelDash();
    if (!(filtered_joypad_L & 16))
      link_prevent_from_moving = 1;
    Sprite_ApplySpeedTowardsLink(k, 16);
    uint8 v = sprite_y_vel[k];
    int t;
    sprite_A[k] = (t = (sign8(v) ? (uint8)-v : v) + sprite_A[k]);
    if (t >= 256)
      drag_player_y = sign8(v) ? 1 : -1;

    v = sprite_x_vel[k];
    sprite_B[k] = (t = (sign8(v) ? (uint8)-v : v) + sprite_B[k]);
    if (t >= 256)
      drag_player_x = sign8(v) ? 1 : -1;
  }
  switch (sprite_ai_state[k]) {
  case 0:  // closed
    sprite_graphics[k] = 6;
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k] = 1;
      sprite_delay_main[k] = 63;
    }
    break;
  case 1:  // opening
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k] = 2;
      sprite_delay_main[k] = 255;
    } else {
      sprite_graphics[k] = kDebirandoPit_OpeningGfx[sprite_delay_main[k] >> 4];
    }
    break;
  case 2:  // open
    if (!(frame_counter & 15) && ++sprite_graphics[k] >= 3)
      sprite_graphics[k] = 0;
    if (!sprite_delay_main[k]) {
      sprite_delay_main[k] = 63;
      sprite_ai_state[k] = 3;
    }
    break;
  case 3:  // closing
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k] = 0;
      sprite_delay_main[k] = 32;
    } else {
      sprite_graphics[k] = kDebirandoPit_ClosingGfx[sprite_delay_main[k] >> 4];
    }
    break;
  }
}

void DebirandoPit_Draw(int k) {  // 8586e4
  static const int16 kDebirandoPit_Draw_X[24] = {
    -8, 8, -8, 8, -8, 8, -8, 8, -8, 8, -8, 8, 0, 8, 0, 8,
    0, 8,  0, 8, -8, 8, -8, 8,
  };
  static const int16 kDebirandoPit_Draw_Y[24] = {
    -8, -8, 8, 8, -8, -8, 8, 8, -8, -8, 8, 8, 0, 0, 8, 8,
    0,  0, 8, 8, -8, -8, 8, 8,
  };
  static const uint8 kDebirandoPit_Draw_Char[24] = {
    4,    4,    4,    4, 0x22, 0x22, 0x22, 0x22, 2, 2, 2, 2, 0x29, 0x29, 0x29, 0x29,
    0x39, 0x39, 0x39, 0x39, 0x2a, 0x2a, 0x2a, 0x2a,
  };
  static const uint8 kDebirandoPit_Draw_Flags[24] = {
    0, 0x40, 0x80, 0xc0, 0, 0x40, 0x80, 0xc0, 0, 0x40, 0x80, 0xc0, 0, 0x40, 0x80, 0xc0,
    0, 0x40, 0x80, 0xc0, 0, 0x40, 0x80, 0xc0,
  };
  static const uint8 kDebirandoPit_Draw_Big[6] = {2, 2, 2, 0, 0, 2};
  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  int g = sprite_graphics[k];
  if (g == 6)
    return;
  OamEnt *oam = GetOamCurPtr();
  uint8 big = kDebirandoPit_Draw_Big[g];
  for (int i = 3; i >= 0; i--, oam++) {
    int j = g * 4 + i;
    SetOamHelper0(oam, info.x + kDebirandoPit_Draw_X[j],
                       info.y + kDebirandoPit_Draw_Y[j],
                       kDebirandoPit_Draw_Char[j],
                       kDebirandoPit_Draw_Flags[j] | info.flags, big);
  }
}

void Sprite_64_Debirando(int k) {  // 85874d
  static const uint8 kDebirando_Emerge_Gfx[2] = {1, 0};
  static const uint8 kDebirando_Submerge_Gfx[2] = {0, 1};
  Debirando_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  switch (sprite_ai_state[k]) {
  case 0:  // under sand
    sprite_ignore_projectile[k] = sprite_delay_main[k];
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k]++;
      sprite_delay_main[k] = 31;
    }
    break;
  case 1:  // emerge
    Sprite_CheckDamageToAndFromLink(k);
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k]++;
      sprite_delay_main[k] = 128;
    } else {
      sprite_graphics[k] = kDebirando_Emerge_Gfx[sprite_delay_main[k] >> 4];
    }
    break;
  case 2:  // fireball
    Sprite_CheckDamageToAndFromLink(k);
    if (!sprite_delay_main[k]) {
      sprite_delay_main[k] = 31;
      sprite_ai_state[k]++;
    } else {
      if (!(sprite_delay_main[k] & 31 | sprite_G[k] | submodule_index | sprite_pause[k] | flag_unk1))
        Sprite_SpawnFireball(k);
      sprite_graphics[k] = (++sprite_subtype2[k] >> 3 & 1) + 2;
    }
    break;
  case 3:  // submerge
    Sprite_CheckDamageToAndFromLink(k);
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k] = 0;
      sprite_delay_main[k] = 223;
    } else {
      sprite_graphics[k] = kDebirando_Submerge_Gfx[sprite_delay_main[k] >> 4];
    }
    break;
  }
}

void Debirando_Draw(int k) {  // 858857
  if (!sprite_ai_state[k])
    return;
  static const int8 kDebirando_Draw_X[16] = {0, 8, 0, 8, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0};
  static const int8 kDebirando_Draw_Y[16] = {2, 2, 6, 6, -2, -2, 6, 6, -4, -4, -4, -4, -4, -4, -4, -4};
  static const uint8 kDebirando_Draw_Char[16] = {0, 0, 0xd8, 0xd8, 0, 0, 0xd9, 0xd9, 0, 0, 0, 0, 0x20, 0x20, 0x20, 0x20};
  static const uint8 kDebirando_Draw_Flags[16] = {1, 0x41, 0, 0x40, 1, 1, 0, 0x40, 1, 1, 1, 1, 1, 1, 1, 1};
  static const uint8 kDebirando_Draw_Big[16] = {0, 0, 0, 0, 2, 2, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2};
  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  OamEnt *oam = GetOamCurPtr();
  int d = sprite_graphics[k] * 4;
  for (int i = 3; i >= 0; i--, oam++) {
    int j = d + i;
    uint8 f = kDebirando_Draw_Flags[j];
    SetOamHelper0(oam,
                  info.x + kDebirando_Draw_X[j],
                  info.y + kDebirando_Draw_Y[j],
                  kDebirando_Draw_Char[j],
                  (f ^ info.flags) & ((f & 0xf) == 0 ? 0xf0 : 0xff),
                  kDebirando_Draw_Big[j]);
  }
}

void Sprite_62_MasterSword(int k) {  // 8588c5
  switch (sprite_subtype2[k]) {
  case 0: MasterSword_Main(k); break;
  case 1: Sprite_MasterSword_LightFountain(k); break;
  case 2: Sprite_MasterSword_LightBeam(k); break;
  case 3: Sprite_MasterSword_Prop(k); break;
  case 4: Sprite_MasterSword_LightWell(k); break;
  }
}

void MasterSword_Main(int k) {  // 8588d6
  if (main_module_index != 26 && save_ow_event_info[BYTE(overworld_screen_index)] & 0x40) {
    sprite_state[k] = 0;
    return;
  }
  if (sprite_ai_state[k] != 5)
    MasterSword_Draw(k);
  switch (sprite_ai_state[k]) {
  case 0:  // waiting
    if (Sprite_CheckIfLinkIsBusy() || !Sprite_CheckDamageToLink_same_layer(k) || link_direction_facing != 2 ||
      !(filtered_joypad_L & 0x80) || (link_which_pendants & 7) != 7)
      return;

    music_control = 10;
    link_disable_sprite_damage = 1;
    MasterSword_SpawnPendantProp(k, 9);
    MasterSword_SpawnPendantProp(k, 11);
    MasterSword_SpawnPendantProp(k, 15);
    MasterSword_SpawnLightWell(k);
    sprite_ai_state[k] = 1;
    sprite_delay_main[k] = 240;
    break;
  case 1:  // pendants transfer
    if (!sprite_delay_main[k]) {
      MasterSword_SpawnLightFountain(k);
      sprite_ai_state[k] = 2;
      sprite_delay_main[k] = 192;
    }
    link_unk_master_sword = 10;
    flag_is_link_immobilized = 1;
    break;
  case 2:  // light show
    if (!sprite_delay_main[k]) {
      MasterSword_SpawnLightBeam(k, 0, 0xff);
      sprite_ai_state[k] = 3;
      sprite_delay_main[k] = 8;
    }
    link_unk_master_sword = 10;
    flag_is_link_immobilized = 1;
    break;
  case 3:  //
    if (!sprite_delay_main[k]) {
      MasterSword_SpawnLightBeam(k, 1, 0xff);
      sprite_ai_state[k] = 4;
      sprite_delay_main[k] = 16;
    }
    link_unk_master_sword = 11;
    flag_is_link_immobilized = 1;
    break;
  case 4:  // give to player
    if (!sprite_delay_main[k]) {
      save_ow_event_info[BYTE(overworld_screen_index)] |= 0x40;
      item_receipt_method = 0;
      Link_ReceiveItem(1, 0);
      savegame_map_icons_indicator = 5;
      link_unk_master_sword = 0;
      sprite_ai_state[k] = 5;
    }
    break;
  case 5:  // stop
    sprite_state[k] = 0;
    break;
  }
}

void Sprite_MasterSword_LightFountain(int k) {  // 8589dc
  static const uint8 kMasterSword_Gfx1[9] = {0, 1, 1, 2, 2, 2, 1, 1, 0};
  static const uint8 kMasterSword_NumLightBeams[9] = {0, 0, 1, 1, 2, 2, 0, 0, 0};
  SpriteDraw_LightFountain(k);
  sprite_A[k]++;
  if (!sprite_A[k]) {
    sprite_C[k]++;
    sprite_state[k] = 0;
  }
  sprite_D[k] = sprite_A[k] >> 2 & 3;
  int j = sprite_A[k] >> 5 & 7;
  sprite_graphics[k] = kMasterSword_Gfx1[j];
  if (kMasterSword_NumLightBeams[j])
    MasterSword_SpawnLightBeam(k, sprite_A[k] >> 2 & 1, kMasterSword_NumLightBeams[j]);
}

void Sprite_MasterSword_LightWell(int k) {  // 858a16
  SpriteDraw_LightFountain(k);
  sprite_A[k]++;
  if (!sprite_A[k]) {
    sprite_C[k]++;
    sprite_state[k] = 0;
  }
  sprite_D[k] = sprite_A[k] >> 2 & 3;
  sprite_graphics[k] = 0;
}

void SpriteDraw_LightFountain(int k) {  // 858a94
  static const DrawMultipleData kMasterSword_LightBall_Dmd[12] = {
    {-6, 4, 0x0082, 2},
    {-6, 4, 0x4082, 2},
    {-6, 4, 0xc082, 2},
    {-6, 4, 0x8082, 2},
    {-6, 4, 0x00a0, 2},
    {-6, 4, 0x40a0, 2},
    {-6, 4, 0xc0a0, 2},
    {-6, 4, 0x80a0, 2},
    {-6, 4, 0x0080, 2},
    {-6, 4, 0x4080, 2},
    {-6, 4, 0xc080, 2},
    {-6, 4, 0x8080, 2},
  };
  Oam_AllocateFromRegionC(4);
  Sprite_DrawMultiple(k, &kMasterSword_LightBall_Dmd[sprite_graphics[k] * 4 + sprite_D[k]], 1, NULL);
}

void MasterSword_SpawnLightWell(int k) {  // 858ab6
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0x62, &info);
  Sprite_SetSpawnedCoordinates(j, &info);
  sprite_subtype2[j] = 4;
  sprite_oam_flags[j] = 5;
  sprite_flags2[j] = 0;
}

void MasterSword_SpawnLightFountain(int k) {  // 858ad0
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0x62, &info);
  Sprite_SetSpawnedCoordinates(j, &info);
  sprite_subtype2[j] = 1;
  sprite_oam_flags[j] = 5;
  sprite_flags2[j] = 0;
}

void Sprite_MasterSword_LightBeam(int k) {  // 858aea
  SpriteDraw_SingleLarge(k);
  if (sprite_A[k]) {
    Sprite_MoveXY(k);
    if (frame_counter & 3)
      return;
    MasterSword_SpawnReplacementLightBeam(k);
  }
  if (!--sprite_B[k])
    sprite_state[k] = 0;
}

void MasterSword_SpawnReplacementLightBeam(int k) {  // 858b20
  SpriteSpawnInfo info;
  int j;
  if ((j = Sprite_SpawnDynamically(k, 0x62, &info)) < 0)
    return;
  Sprite_SetX(j, info.r0_x);
  Sprite_SetY(j, info.r2_y);
  sprite_subtype2[j] = 2;
  sprite_B[j] = 3;
  sprite_graphics[j] = sprite_graphics[k];
  sprite_oam_flags[j] = sprite_oam_flags[k];
  sprite_flags2[j] = 0;
}

void MasterSword_SpawnLightBeam(int k, uint8 ain, uint8 yin) {  // 858b62
  static const int8 kMasterSword_LightBeam_Xv0[2] = {0, -48};
  static const int8 kMasterSword_LightBeam_Xv1[2] = {0, 48};
  static const int8 kMasterSword_LightBeam_Xv2[2] = {-96, -48};
  static const int8 kMasterSword_LightBeam_Xv3[2] = {96, 48};
  static const int8 kMasterSword_LightBeam_Yv0[2] = {-96, -48};
  static const int8 kMasterSword_LightBeam_Yv1[2] = {96, 48};
  static const int8 kMasterSword_LightBeam_Yv2[2] = {0, 48};
  static const int8 kMasterSword_LightBeam_Yv3[2] = {0, -48};
  static const uint8 kMasterSword_LightBeam_Gfx0[2] = {1, 0};
  static const uint8 kMasterSword_LightBeam_Gfx2[2] = {3, 2};
  static const uint8 kMasterSword_LightBeam_Flags0[2] = {5, 0x45};
  static const uint8 kMasterSword_LightBeam_Flags2[2] = {5, 5};

  SpriteSpawnInfo info;
  int j;
  if ((j = Sprite_SpawnDynamically(k, 0x62, &info)) < 0)
    return;
  Sprite_SetX(j, info.r0_x - 4);
  Sprite_SetY(j, info.r2_y + 4);
  sprite_subtype2[j] = 2;
  sprite_A[j] = 2;
  sprite_flags2[j] = 0;
  sprite_x_vel[j] = kMasterSword_LightBeam_Xv0[ain];
  sprite_y_vel[j] = kMasterSword_LightBeam_Yv0[ain];
  sprite_graphics[j] = kMasterSword_LightBeam_Gfx0[ain];
  sprite_oam_flags[j] = kMasterSword_LightBeam_Flags0[ain];
  sprite_B[j] = yin;

  if ((j = Sprite_SpawnDynamically(k, 0x62, &info)) < 0)
    return;
  Sprite_SetX(j, info.r0_x - 4);
  Sprite_SetY(j, info.r2_y + 4);
  sprite_subtype2[j] = 2;
  sprite_A[j] = 2;
  sprite_flags2[j] = 0;
  sprite_x_vel[j] = kMasterSword_LightBeam_Xv1[ain];
  sprite_y_vel[j] = kMasterSword_LightBeam_Yv1[ain];
  sprite_graphics[j] = kMasterSword_LightBeam_Gfx0[ain];
  sprite_oam_flags[j] = kMasterSword_LightBeam_Flags0[ain];
  sprite_B[j] = yin;

  if ((j = Sprite_SpawnDynamically(k, 0x62, &info)) < 0)
    return;
  Sprite_SetX(j, info.r0_x - 4);
  Sprite_SetY(j, info.r2_y + 4);
  sprite_subtype2[j] = 2;
  sprite_A[j] = 2;
  sprite_flags2[j] = 0;
  sprite_x_vel[j] = kMasterSword_LightBeam_Xv2[ain];
  sprite_y_vel[j] = kMasterSword_LightBeam_Yv2[ain];
  sprite_graphics[j] = kMasterSword_LightBeam_Gfx2[ain];
  sprite_oam_flags[j] = kMasterSword_LightBeam_Flags2[ain];
  sprite_B[j] = yin;

  if ((j = Sprite_SpawnDynamically(k, 0x62, &info)) < 0)
    return;
  Sprite_SetX(j, info.r0_x - 4);
  Sprite_SetY(j, info.r2_y + 4);
  sprite_subtype2[j] = 2;
  sprite_A[j] = 2;
  sprite_flags2[j] = 0;
  sprite_x_vel[j] = kMasterSword_LightBeam_Xv3[ain];
  sprite_y_vel[j] = kMasterSword_LightBeam_Yv3[ain];
  sprite_graphics[j] = kMasterSword_LightBeam_Gfx2[ain];
  sprite_oam_flags[j] = kMasterSword_LightBeam_Flags2[ain];
  sprite_B[j] = yin;
}

void MasterSword_SpawnPendantProp(int k, uint8 ain) {  // 858cd3
  static const int8 kMasterSword_Pendant_Xv[4] = {-4, 4, 0, 0};
  static const int8 kMasterSword_Pendant_Yv[4] = {-2, -2, -4, -4};
  SpriteSpawnInfo info;
  int j;
  if ((j = Sprite_SpawnDynamically(k, 0x62, &info)) < 0)
    return;
  sprite_oam_flags[j] = ain;
  Sprite_SetX(j, link_x_coord);
  Sprite_SetY(j, link_y_coord + 8);
  sprite_graphics[j] = 4;
  sprite_subtype2[j] = 3;
  sprite_flags2[j] = 64;
  sprite_delay_main[j] = 228;
  int i = ain >> 1 & 3;
  sprite_x_vel[j] = kMasterSword_Pendant_Xv[i];
  sprite_y_vel[j] = kMasterSword_Pendant_Yv[i];
}

void Sprite_MasterSword_Prop(int k) {  // 858d29
  Oam_AllocateFromRegionB(4);
  SpriteDraw_SingleLarge(k);
  switch (sprite_ai_state[k]) {
  case 0:  // drifting away
    Sprite_MoveXY(k);
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k] = 1;
      sprite_delay_main[k] = 208;
      sprite_A[k] = sprite_oam_flags[k];
    }
    break;
  case 1:  // flashing
    sprite_oam_flags[k] = (sprite_oam_flags[k] & ~0xe) | ((k << 1 ^ frame_counter) & 0xe);
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k] = 2;
      sprite_oam_flags[k] = sprite_A[k];
    }
    break;
  case 2:  // fly
    Sprite_MoveXY(k);
    if (!sprite_delay_main[k]) {
      sprite_x_vel[k] <<= 1;
      sprite_y_vel[k] <<= 1;
      sprite_delay_main[k] = 6;
    }
    sprite_E[k]++;
    if (sprite_E[k] == 0)
      sprite_state[k] = 0;
    break;
  }
}

void MasterSword_Draw(int k) {  // 858da8
  static const int8 kMasterSword_Draw_X[6] = {-8, 0, -8, 0, -8, 0};
  static const int8 kMasterSword_Draw_Y[6] = {-8, -8, 0, 0, 8, 8};
  static const uint8 kMasterSword_Draw_Char[6] = {0xc3, 0xc4, 0xd3, 0xd4, 0xe0, 0xf0};
  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  OamEnt *oam = GetOamCurPtr();
  for (int i = 5; i >= 0; i--, oam++) {
    oam->x = kMasterSword_Draw_X[i] + info.x;
    oam->y = kMasterSword_Draw_Y[i] + info.y;
    oam->charnum = kMasterSword_Draw_Char[i];
    oam->flags = info.flags;
  }
  Sprite_CorrectOamEntries(k, 5, 0);
}

void Sprite_5D_Roller_VerticalDownFirst(int k) {  // 858dde
  static const int8 kSpikeRoller_XYvel[6] = {-16, 16, 0, 0, -16, 16};
  sprite_graphics[k] = sprite_subtype2[k] >> 1 & 1 | sprite_D[k] & 2;
  SpikeRoller_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_CheckDamageToAndFromLink(k);
  if (!sprite_delay_main[k]) {
    sprite_delay_main[k] = 112;
    sprite_D[k] ^= 1;
  }
  int j = sprite_D[k];
  sprite_x_vel[k] = kSpikeRoller_XYvel[j];
  sprite_y_vel[k] = kSpikeRoller_XYvel[j + 2];
  Sprite_MoveXY(k);
  sprite_subtype2[k]++;
}

void SpikeRoller_Draw(int k) {  // 858ee3
  static const uint8 kSpikeRoller_Draw_X[32] = {
    0,    0,    0,    0,    0,    0,    0,    0, 0,    0,    0,    0,    0,    0,    0,    0,
    0, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70,
  };
  static const uint8 kSpikeRoller_Draw_Y[32] = {
    0, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70,
    0,    0,    0,    0,    0,    0,    0,    0, 0,    0,    0,    0,    0,    0,    0,    0,
  };
  static const uint8 kSpikeRoller_Draw_Char[32] = {
    0x8e, 0x9e, 0x9e, 0x9e, 0x9e, 0x9e, 0x9e, 0x8e, 0x8e, 0x9e, 0x9e, 0x9e, 0x9e, 0x9e, 0x9e, 0x8e,
    0x88, 0x89, 0x89, 0x89, 0x89, 0x89, 0x89, 0x88, 0x88, 0x89, 0x89, 0x89, 0x89, 0x89, 0x89, 0x88,
  };
  static const uint8 kSpikeRoller_Draw_Flags[32] = {
    0, 0, 0, 0x80, 0, 0, 0, 0x80, 0x40, 0x40, 0x40, 0xc0, 0x40, 0x40, 0x40, 0xc0,
    0, 0, 0, 0x40, 0, 0, 0, 0x40, 0x80, 0x80, 0x80, 0xc0, 0x80, 0x80, 0x80, 0xc0,
  };
  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  OamEnt *oam = GetOamCurPtr();
  int g = sprite_graphics[k];
  uint8 chr = kSpikeRoller_Draw_Char[g * 8];

  for (int i = sprite_ai_state[k] ? 7 : 3; i >= 0; i--, oam++) {
    int j = g * 8 + i;
    SetOamHelper0(oam,
                  info.x + kSpikeRoller_Draw_X[j],
                  info.y + kSpikeRoller_Draw_Y[j],
                  chr ? chr : kSpikeRoller_Draw_Char[j],
                  kSpikeRoller_Draw_Flags[j] | info.flags, 2);
    chr = 0;
  }
}

void Sprite_61_Beamos(int k) {  // 858f54
  if (sprite_C[k] == 1) {
    Sprite_Beamos_Laser(k);
    return;
  } else if (sprite_C[k] != 0) {
    Sprite_Beamos_LaserHit(k);
    return;
  }

  Beamos_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_CheckTileCollision(k);
  Sprite_CheckDamageToLink(k);
  switch (sprite_ai_state[k]) {
  case 0:
    if (!((k ^ frame_counter) & 3)) {
      Sprite_SpawnProbeAlways(k, sprite_D[k]);
      sprite_D[k]++;
    }
    sprite_D[k] &= 63;
    break;
  case 3:
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k] = 4;
      sprite_delay_main[k] = 80;
      SpritePrep_LoadPalette(k);
    } else {
      if (sprite_delay_main[k] == 15)
        Beamos_FireLaser(k);
      sprite_oam_flags[k] ^= (sprite_delay_main[k] >> 1 & 0xe);
    }
    break;
  default:
    if (!sprite_delay_main[k])
      sprite_ai_state[k] = 0;
    break;
  }
}

void Beamos_FireLaser(int k) {  // 858fc2
  if (sprite_limit_instance >= 4)
    return;
  SpriteSpawnInfo info;
  int j, t;
  if ((j = Sprite_SpawnDynamically(k, 0x61, &info)) < 0)
    return;
  SpriteSfx_QueueSfx3WithPan(k, 0x19);
  Sprite_SetX(j, info.r0_x + (int8)BYTE(dungmap_var7));
  Sprite_SetY(j, info.r2_y + (int8)HIBYTE(dungmap_var7));
  Sprite_ApplySpeedTowardsLink(j, 0x20);
  sprite_flags2[j] = 0x3f;
  sprite_flags4[j] = 0x54;
  sprite_C[j] = 1;
  sprite_defl_bits[j] = 0x48;
  sprite_oam_flags[j] = 3;
  sprite_bump_damage[j] = 4;
  sprite_delay_aux1[j] = 12;
  sprite_graphics[j] = t = sprite_limit_instance++;
  for (int i = 0; i < 32; i++) {
    beamos_x_lo[t * 32 + i] = sprite_x_lo[j];
    beamos_x_hi[t * 32 + i] = sprite_x_hi[j];
    beamos_y_lo[t * 32 + i] = sprite_y_lo[j];
    beamos_y_hi[t * 32 + i] = sprite_y_hi[j];
  }
}

void Beamos_Draw(int k) {  // 859068
  static const int8 kBeamos_Draw_Y[2] = {-16, 0};
  static const int8 kBeamos_Draw_Char[2] = {0x48, 0x68};
  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  int spr_offs = 0;
  if (sprite_D[k] < 0x20) {
    Oam_AllocateFromRegionB(12);
    spr_offs = 1;
  } else {
    Oam_AllocateFromRegionC(12);
  }
  OamEnt *oam = GetOamCurPtr() + spr_offs;
  for (int i = 1; i >= 0; i--, oam++)
    SetOamHelper0(oam, info.x, info.y + kBeamos_Draw_Y[i], kBeamos_Draw_Char[i], info.flags, 2);
  SpriteDraw_Beamos_Eyeball(k, &info);
}

void SpriteDraw_Beamos_Eyeball(int k, PrepOamCoordsRet *info) {  // 859151
  static const int8 kBeamosEyeball_Draw_X[32] = {
    -1,  0,  1,  2,  3,  4,  5, 7, 8, 10, 11, 12, 13, 14, 15, 16,
    17, 15, 14, 13, 12, 11, 10, 8, 7,  5,  4,  3,  2,  1,  0, -2,
  };
  static const int8 kBeamosEyeball_Draw_Y[32] = {
    11, 12, 13, 14, 14, 15, 15, 15, 15, 15, 15, 14, 14, 13, 12, 11,
    10,  9,  8,  7,  7,  6,  6,  6,  6,  6,  6,  7,  7,  8,  9, 10,
  };
  static const uint8 kBeamosEyeball_Draw_Char[32] = {
    0x5b, 0x5b, 0x5a, 0x5a, 0x4b, 0x4b, 0x4a, 0x4a, 0x4a, 0x4a, 0x4b, 0x4b, 0x5a, 0x5a, 0x5b, 0x5b,
    0x5b, 0x5b, 0x4c, 0x4c, 0x4c, 0x4c, 0x4c, 0x4c, 0x5b, 0x5b, 0x4c, 0x4c, 0x4c, 0x4c, 0x4c, 0x4c,
  };
  static const uint8 kBeamosEyeball_Draw_Flags[32] = {
    0,    0,    0,    0,    0,    0,    0,    0, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
    0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,    0,    0,    0,    0,    0,    0,    0,    0,
  };
  int n = (sprite_D[k] < 0x20) ? 0 : 2;
  OamEnt *oam = GetOamCurPtr() + n;
  int i = sprite_D[k] >> 1;
  BYTE(dungmap_var7) = kBeamosEyeball_Draw_X[i] - 3;
  oam->x = BYTE(dungmap_var7) + info->x;
  HIBYTE(dungmap_var7) = kBeamosEyeball_Draw_Y[i] - 18;
  oam->y = HIBYTE(dungmap_var7) + info->y;
  oam->charnum = kBeamosEyeball_Draw_Char[i];
  oam->flags = info->flags&0x31|0xA|kBeamosEyeball_Draw_Flags[i];
  oam_cur_ptr += n * 4;
  oam_ext_cur_ptr += n;
  Sprite_CorrectOamEntries(k, 0, 0);
}

void Sprite_Beamos_Laser(int k) {  // 8591b5
  if (sprite_delay_aux1[k])
    return;
  BeamosLaser_Draw(k);
  if (!sprite_state[k]) {
    sprite_limit_instance--;
    return;
  }
  if (Sprite_ReturnIfInactive(k))
    return;
  for (int i = 3; i >= 0; i--) {
    int t = (sprite_subtype2[k]++ & 31) + sprite_graphics[k] * 32;
    beamos_y_hi[t] = sprite_y_hi[k];
    beamos_y_lo[t] = sprite_y_lo[k];
    beamos_x_hi[t] = sprite_x_hi[k];
    beamos_x_lo[t] = sprite_x_lo[k];
    Sprite_MoveXY(k);
  }

  if (sprite_delay_main[k]) {
    if (sprite_delay_main[k] == 1) {
      sprite_state[k] = 0;
      sprite_limit_instance--;
    }
    return;
  }
  if (!Sprite_CheckDamageToLink_same_layer(k) && !Sprite_CheckTileCollision(k))
    return;
  SpriteSfx_QueueSfx3WithPan(k, 0x26);
  sprite_delay_main[k] = 16;
  Sprite_ZeroVelocity_XY(k);
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0x61, &info);
  if (j >= 0) {
    Sprite_SetSpawnedCoordinates(j, &info);
    sprite_delay_main[j] = 16;
    sprite_flags2[j] = 3;
    sprite_C[j] = 2;
    sprite_flags3[j] = 0x40;
  }
  sprite_y_hi[k] = 128;
}

void BeamosLaser_Draw(int k) {  // 85925b
  PrepOamCoordsRet info;
  Sprite_PrepOamCoord(k, &info);
  OamEnt *oam = GetOamCurPtr();
  int g = sprite_graphics[k];
  for (int i = 31; i >= 0; i--, oam++) {
    int j = g * 32 + i;
    uint16 x = (beamos_x_lo[j] | beamos_x_hi[j] << 8) - BG2HOFS_copy2;
    uint16 y = (beamos_y_lo[j] | beamos_y_hi[j] << 8) - BG2VOFS_copy2;
    SetOamHelper0(oam, x, y, 0x5c, info.flags, 0);
  }
}

void Sprite_Beamos_LaserHit(int k) {  // 8592da
  static const int8 kBeamosLaserHit_Draw_X[4] = {-4, 4, -4, 4};
  static const int8 kBeamosLaserHit_Draw_Y[4] = {-4, -4, 4, 4};
  static const uint8 kBeamosLaserHit_Draw_Flags[4] = {6, 0x46, 0x86, 0xc6};
  if (!sprite_delay_main[k])
    sprite_state[k] = 0;
  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  OamEnt *oam = GetOamCurPtr();
  for (int i = 3; i >= 0; i--, oam++) {
    SetOamHelper0(oam, info.x + kBeamosLaserHit_Draw_X[i], info.y + kBeamosLaserHit_Draw_Y[i],
                  0xd6, kBeamosLaserHit_Draw_Flags[i] | info.flags & 0x30, 0);
  }
}

void Sprite_5B_Spark_Clockwise(int k) {  // 85933f
  static const uint8 kSpark_OamFlags[4] = {0, 0x40, 0x80, 0xc0};
  static const uint8 kSpark_directions[8] = {1, 3, 2, 0, 7, 5, 6, 4};
  int j;

  SpriteDraw_SingleLarge(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (!(frame_counter & 1))
    sprite_oam_flags[k] ^= 6;
  if (!sprite_ai_state[k]) {
    sprite_ai_state[k] = 1;
    sprite_x_vel[k] = sprite_y_vel[k] = 1;
    uint8 coll = Sprite_CheckTileCollision(k);
    sprite_x_vel[k] = sprite_y_vel[k] = -1;
    coll |= Sprite_CheckTileCollision(k);
    if (coll < 4)
      j = (coll & 1) ? 0 : 1;
    else
      j = (coll & 4) ? 2 : 3;
    sprite_D[k] = kSpark_directions[(sprite_type[k] != 0x5c) * 4 + j];
  }

  sprite_oam_flags[k] = sprite_oam_flags[k] & 0x3f | kSpark_OamFlags[frame_counter >> 2 & 3];
  Sprite_MoveXY(k);
  Sprite_CheckDamageToLink(k);
  j = sprite_D[k];
  sprite_x_vel[k] = kSoldierB_Xvel[j];
  sprite_y_vel[k] = kSoldierB_Yvel[j];
  Sprite_CheckTileCollision(k);

  j = sprite_D[k];
  if (sprite_delay_aux2[k]) {
    if (sprite_delay_aux2[k] == 6)
      j = kSoldierB_NextB[j];
  } else {
    if (!(sprite_wallcoll[k] & kSoldierB_Mask[j]))
      sprite_delay_aux2[k] = 10;
  }
  if (sprite_wallcoll[k] & kSoldierB_Mask2[j])
    j = kSoldierB_NextB2[j];
  sprite_D[k] = j;
  sprite_x_vel[k] = kSoldierB_Xvel2[j] * 2;
  sprite_y_vel[k] = kSoldierB_Yvel2[j] * 2;
}

void Sprite_59_LostWoodsBird(int k) {  // 85940e
  if (sprite_delay_aux1[k])
    return;
  sprite_oam_flags[k] = sprite_oam_flags[k] & ~0x40 | (sign8(sprite_x_vel[k]) ? 0 : 0x40);
  SpriteDraw_SingleLarge(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_MoveXY(k);
  Sprite_MoveZ(k);
  switch (sprite_ai_state[k]) {
  case 0:
    sprite_graphics[k] = 0;
    if (sign8(--sprite_z_vel[k] - 0xf1))
      sprite_ai_state[k] = 1;
    break;
  case 1:
    sprite_z_vel[k] += 2;
    if (!sign8(sprite_z_vel[k] - 0x10))
      sprite_ai_state[k] = 0;
    sprite_graphics[k] = ++sprite_subtype2[k] >> 1 & 1;
    break;
  }
}

void Sprite_5A_LostWoodsSquirrel(int k) {  // 859468
  if (sprite_delay_aux1[k])
    return;
  sprite_oam_flags[k] = sprite_oam_flags[k] & ~0x40 | (sign8(sprite_x_vel[k]) ? 0 : 0x40);
  SpriteDraw_SingleLarge(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_MoveXY(k);
  Sprite_MoveZ(k);
  sprite_z_vel[k] -= 2;
  if (sign8(sprite_z[k])) {
    sprite_z[k] = 0;
    sprite_z_vel[k] = 16;
    sprite_delay_main[k] = 12;
  }
  sprite_graphics[k] = sprite_delay_main[k] ? 1 : 0;
}

void Sprite_58_Crab(int k) {  // 8594b5
  static const int8 kCrab_Xvel[4] = {28, -28, 0, 0};
  static const int8 kCrab_Yvel[4] = {0, 0, 12, -12};
  Crab_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (Sprite_ReturnIfRecoiling(k))
    return;
  Sprite_CheckDamageToAndFromLink(k);
  Sprite_MoveXY(k);
  if (Sprite_CheckTileCollision(k) || !sprite_delay_main[k]) {
    sprite_delay_main[k] = (GetRandomNumber() & 63) + 32;
    sprite_D[k] = sprite_delay_main[k] & 3;
  }
  int j = sprite_D[k];
  sprite_x_vel[k] = kCrab_Xvel[j & 3];
  sprite_y_vel[k] = kCrab_Yvel[j & 3];
  sprite_graphics[k] = (++sprite_subtype2[k] >> (j < 2 ? 1 : 3)) & 1;
}

void Crab_Draw(int k) {  // 859510
  static const int16 kCrab_Draw_X[4] = {-8, 8, -8, 8};
  static const uint8 kCrab_Draw_Char[4] = {0x8e, 0x8e, 0xae, 0xae};
  static const int8 kCrab_Draw_Flags[4] = {0, 0x40, 0, 0x40};
  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  OamEnt *oam = GetOamCurPtr();
  int d = sprite_graphics[k] * 2;
  for (int i = 1; i >= 0; i--, oam++) {
    int j = d + i;
    SetOamHelper0(oam, info.x + kCrab_Draw_X[j], info.y, kCrab_Draw_Char[j], kCrab_Draw_Flags[j] | info.flags, 2);
  }
  SpriteDraw_Shadow(k, &info);
}

void Sprite_57_DesertStatue(int k) {  // 85956d
  static const uint8 kDesertBarrier_NextD[4] = {3, 2, 0, 1};
  DesertBarrier_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  bool dmg = Sprite_CheckDamageToLink_same_layer(k);
  if (dmg) {
    Sprite_NullifyHookshotDrag();
    Sprite_RepelDash();
  }
  if (sprite_delay_main[k] || sign8(sprite_ai_state[k]))
    return;

  if (sprite_ai_state[k] == 0) {
    if (!byte_7E02F0)
      return;
    sprite_ai_state[k] = byte_7E02F0;
    sprite_delay_main[k] = 128;
    sound_effect_ambient = 7;
  }

  if (dmg && !link_incapacitated_timer) {
    link_incapacitated_timer = 16;
    Sprite_ApplySpeedTowardsLink(k, 32);
  }

  int j = sprite_D[k];
  sprite_x_vel[k] = kDesertBarrier_Xv[j];
  sprite_y_vel[k] = kDesertBarrier_Yv[j];
  Sprite_MoveXY(k);
  if (Sprite_CheckTileCollision(k))
    sprite_D[k] = kDesertBarrier_NextD[sprite_D[k]];
  flag_is_link_immobilized = 1;
  if (!(++sprite_subtype2[k] & 1)) {
    if (++sprite_G[k] == 130) {
      sprite_ai_state[k] = 128;
      flag_is_link_immobilized = 0;
    }
  }
}

void DesertBarrier_Draw(int k) {  // 859626
  static const DrawMultipleData kDesertBarrier_Dmd[4] = {
    {-8, -8, 0x008e, 2},
    { 8, -8, 0x408e, 2},
    {-8,  8, 0x00ae, 2},
    { 8,  8, 0x40ae, 2},
  };
  if (sprite_delay_main[k] == 1) {
    sound_effect_2 = 0x1b;
    sound_effect_ambient = 5;
  }
  BYTE(cur_sprite_x) += (sprite_delay_main[k] >> 1) & 1;
  PointU8 pt;
  Sprite_DirectionToFaceLink(k, &pt);
  if ((uint8)(pt.x + 0x20) < 0x40 && (uint8)(pt.y + 0x20) < 0x40)
    Oam_AllocateFromRegionB(16);
  Sprite_DrawMultiple(k, kDesertBarrier_Dmd, 4, NULL);
}

void Sprite_55_Zora(int k) {  // 85967b
  if (sprite_E[k])
    Sprite_Fireball(k);
  else
    Sprite_Zora_Main(k);
}

void Sprite_Fireball(int k) {  // 859683
  static const uint8 kSprite_ZoraFireball_Offs[4] = {3, 2, 0, 0};
  static const int8 kSprite_ZoraFireball_X[4] = {4, 4, -4, 16};
  static const int8 kSprite_ZoraFireball_Y[4] = {0, 16, 8, 8};
  static const uint8 kSprite_ZoraFireball_W[4] = {8, 8, 4, 4};
  static const uint8 kSprite_ZoraFireball_H[4] = {4, 4, 8, 8};

  sprite_ignore_projectile[k] = sprite_E[k];
  if (sprite_delay_main[k])
    Oam_AllocateFromRegionC(4);
  SpriteDraw_SingleSmall(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Fireball_SpawnTrailGarnish(k);
  if (Sprite_CheckDamageToLink(k)) {
    sprite_state[k] = 0;
    return;
  }
  Sprite_MoveXY(k);
  if (player_is_indoors && !sprite_delay_aux1[k] && !((k ^ frame_counter) & 3) && Sprite_CheckTileCollision(k)) {
    sprite_state[k] = 0;
    return;
  }

  if ((link_is_bunny_mirror | link_disable_sprite_damage) || sign8(link_state_bits) || link_shield_type < 2 ||
      link_is_on_lower_level != sprite_floor[k])
    return;
  SpriteHitBox hb;
  Sprite_SetupHitBox(k, &hb);
  int j = link_direction_facing >> 1;
  if (button_b_frames)
    j = kSprite_ZoraFireball_Offs[j];
  int x = link_x_coord + kSprite_ZoraFireball_X[j], y = link_y_coord + kSprite_ZoraFireball_Y[j];
  hb.r0_xlo = x;
  hb.r8_xhi = x >> 8;
  hb.r2 = kSprite_ZoraFireball_W[j];
  hb.r1_ylo = y;
  hb.r9_yhi = y >> 8;
  hb.r3 = kSprite_ZoraFireball_H[j];
  if (CheckIfHitBoxesOverlap(&hb)) {
    Sprite_PlaceRupulseSpark_2(k);
    sprite_state[k] = 0;
    SpriteSfx_QueueSfx2WithPan(k, 0x6);
  }
}

void Sprite_Zora_Main(int k) {  // 859725
  static const int8 kSprite_Zora_Surface_XY[8] = {-32, -24, -16, -8, 8, 16, 24, 32};
  static const uint8 kSprite_Zora_AttackGfx[8] = {5, 5, 6, 10, 6, 5, 5, 5};
  static const uint8 kSprite_Zora_SubmergeGfx[12] = {12, 11, 9, 8, 7, 0, 0, 0, 0, 0, 0, 0};
  PrepOamCoordsRet info;
  if (!sprite_ai_state[k])
    Sprite_PrepOamCoord(k, &info);
  else
    Zora_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  switch (sprite_ai_state[k]) {
  case 0:  // choose surfacing location
    sprite_ignore_projectile[k] = sprite_delay_main[k];
    if (!sprite_delay_main[k]) {
      int org_x = (sprite_A[k] | sprite_B[k] << 8);
      int org_y = (sprite_C[k] | sprite_head_dir[k] << 8);
      Sprite_SetX(k, org_x + kSprite_Zora_Surface_XY[GetRandomNumber() & 7]);
      Sprite_SetY(k, org_y + kSprite_Zora_Surface_XY[GetRandomNumber() & 7]);
      Sprite_Get16BitCoords(k);
      Sprite_CheckTileCollision(k);
      if (sprite_tiletype == 8) spawn_anyway: {
        sprite_delay_main[k] = 127;
        sprite_ai_state[k]++;
        sprite_flags3[k] |= 0x40;
      } else {
        // In Misery Mire some Zoras are placed in shallow water so they can't spawn
        // This fixes so they will be able to spawn in shallow water after a delay.
        if (enhanced_features0 & kFeatures0_GameChangingBugFixes) {
          if (sprite_tiletype == 9 && sprite_delay_aux2[k] == 1)
            goto spawn_anyway;
          Sprite_SetX(k, org_x);
          Sprite_SetY(k, org_y);
          sprite_ignore_projectile[k] = 1;
          if (sprite_delay_aux2[k] == 0)
            sprite_delay_aux2[k] = 32;
        }
      }
    }
    break;
  case 1:  // surfacing
    sprite_ignore_projectile[k] = sprite_delay_main[k];
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k]++;
      sprite_delay_main[k] = 127;
      sprite_flags3[k] &= ~0x40;
    } else {
      sprite_graphics[k] = kSprite_Zora_SurfacingGfx[sprite_delay_main[k] >> 3];
    }
    break;
  case 2:  // attack
    Sprite_CheckDamageToAndFromLink(k);
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k]++;
      sprite_delay_main[k] = 23;
    } else {
      if (sprite_delay_main[k] == 48)
        Sprite_SpawnFireball(k);
      sprite_graphics[k] = kSprite_Zora_AttackGfx[sprite_delay_main[k] >> 4];
    }
    break;
  case 3:  // submerging
    if (!sprite_delay_main[k]) {
      sprite_delay_main[k] = 128;
      sprite_graphics[k] = 0;
      sprite_ai_state[k] = 0;
    } else {
      sprite_graphics[k] = kSprite_Zora_SubmergeGfx[sprite_delay_main[k] >> 2];
    }
    break;
  }
}

void Zora_Draw(int k) {  // 8598f5
  static const int8 kZora_Draw_X[26] = {
    4, 4,  0,  0, 0, 0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0,
    0, 0, -4, 11, 0, 4, -8, 18, -8, 18,
  };
  static const int8 kZora_Draw_Y[26] = {
    4,  4,  0,  0,  0, 0,   0,  -3,   0,  -3, -3, -3, -3, -3, -3, -3,
    -6, -6, -8, -9, -3, 5, -10, -11, -10, -11,
  };
  static const uint8 kZora_Draw_Char[26] = {
    0xa8, 0xa8, 0x88, 0x88, 0x88, 0x88, 0x88, 0xa4, 0x88, 0xa4, 0xa4, 0xa4, 0xa6, 0xa6, 0xa4, 0xc0,
    0x8a, 0x8a, 0xae, 0xaf, 0xa6, 0x8d, 0xcf, 0xcf, 0xdf, 0xdf,
  };
  static const uint8 kZora_Draw_Flags[26] = {
    0x25, 0x25, 0x25, 0x25, 0xe5, 0xe5, 0x25, 0x20, 0xe5, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x24,
    0x25, 0x25, 0x24, 0x64, 0x20, 0x26, 0x24, 0x64, 0x24, 0x64,
  };
  static const uint8 kZora_Draw_Big[26] = {
    0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 0, 0, 2, 0, 0, 0, 0, 0,
  };
  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  OamEnt *oam = GetOamCurPtr();
  int d = sprite_graphics[k] * 2;
  for (int i = 1; i >= 0; i--, oam++) {
    int j = d + i;
    uint8 f = kZora_Draw_Flags[j];
    SetOamHelper0(oam, info.x + kZora_Draw_X[j], info.y + kZora_Draw_Y[j], kZora_Draw_Char[j], f | (f & 0xf ? 0 : info.flags), kZora_Draw_Big[j]);
  }
}

void Sprite_52_KingZora(int k) {  // 85995b
  ZoraKing_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  switch(sprite_ai_state[k]) {
  case 0:  // WaitingForPlayer
    if ((uint16)(link_x_coord - cur_sprite_x + 16) < 32 && (uint16)(link_y_coord - cur_sprite_y + 48) < 96) {
      Link_CancelDash();
      sprite_delay_main[k] = 127;
      sound_effect_1 = 0x35;
      sprite_ai_state[k] = 1;
      for (int j = 15; j >= 0; j--) {
        if (j != k && !(sprite_defl_bits[j] & 0x80)) {
          if (sprite_state[j] == 10) {
            link_state_bits = 0;
            link_picking_throw_state = 0;
          }
          Sprite_KillSelf(j);
        }
      }
    }
    break;
  case 1:  // RumblingGround
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k] = 2;
      sprite_delay_main[k] = 127;
      bg1_x_offset = 0;
      sprite_graphics[k] = 4;
    } else {
      bg1_x_offset = sprite_delay_main[k] & 1 ? -1 : 1;
      flag_is_link_immobilized = 1;
    }
    break;
  case 2: {  // Surfacing
    static const uint8 kZoraKing_Surfacing_Gfx[16] = {0, 0, 0, 3, 9, 8, 7, 6, 9, 8, 7, 6, 5, 4, 5, 4};
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k] = 3;
      sprite_delay_main[k] = 127;
    } else {
      if (sprite_delay_main[k] == 28) {
        sprite_delay_aux2[k] = 15;
        Sprite_SpawnBigSplash(k);
      }
      sprite_graphics[k] = kZoraKing_Surfacing_Gfx[sprite_delay_main[k] >> 3];
    }
    break;
  }
  case 3: { // Dialogue
    static const uint8 kZoraKing_Dialogue_Gfx[8] = {0, 0, 1, 2, 1, 2, 0, 0};
    int j = sprite_delay_main[k];
    if (!j) {
      sprite_ai_state[k] = 4;
      sprite_delay_main[k] = 36;
      return;
    }
    sprite_graphics[k] = kZoraKing_Dialogue_Gfx[j >> 4];
    if (j == 80) {
      dialogue_message_index = 0x142;
      Sprite_ShowMessageMinimal();
    } else if (j == 79) {
      if (choice_in_multiselect_box == 0) {
        dialogue_message_index = 0x143;
        Sprite_ShowMessageMinimal();
      } else {
        dialogue_message_index = 0x146;
        Sprite_ShowMessageMinimal();
        sprite_delay_main[k] = 0x30;
      }
    } else if (j == 78) {
      if (choice_in_multiselect_box == 0 && link_rupees_goal >= 500) {
        link_rupees_goal -= 500;
        dialogue_message_index = 0x144;
        Sprite_ShowMessageMinimal();
        sprite_E[k] = 1;
      } else {
        dialogue_message_index = 0x145;
        Sprite_ShowMessageMinimal();
        sprite_delay_main[k] = 0x30;
      }
    } else if (j == 77) {
      if (sprite_E[k])
        Sprite_Zora_RegurgitateFlippers(k);
    }
    break;
  }
  case 4: {  // Submerge
    static const uint8 kZoraKing_Submerge_Gfx[21] = {
      12, 12, 12, 12, 12, 12, 11, 11, 11, 11, 11, 10, 10, 10, 10, 3,
      3,  3,  3,  3,  3,
    };
    int j = sprite_delay_main[k];
    if (!j) {
      Sprite_KillSelf(k);
      flag_is_link_immobilized = 0;
    } else {
      if (j == 29) {
        sprite_delay_aux2[k] = 15;
        Sprite_SpawnBigSplash(k);
      }
      sprite_graphics[k] = kZoraKing_Submerge_Gfx[sprite_delay_main[k] >> 1];
    }
    break;
  }
  }
}

void Sprite_SpawnBigSplash(int k) {  // 859b40
  static const int8 kSpawnSplashRing_X[8] = {-8, -5, 4, 13, 16, 13, 4, -5};
  static const int8 kSpawnSplashRing_Y[8] = {4, -5, -8, -5, 4, 13, 16, 13};
  static const int8 kSpawnSplashRing_Xvel[8] = {-8, -6, 0, 6, 8, 6, 0, -6};
  static const int8 kSpawnSplashRing_Yvel[8] = {0, -6, -8, -6, 0, 6, 8, 6};
  SpriteSfx_QueueSfx2WithPan(k, 0x24);

  for (int i = 7; i >= 0; i--) {
    SpriteSpawnInfo info;
    int j = Sprite_SpawnDynamically(k, 8, &info);
    if (j >= 0) {
      sprite_state[j] = 3;
      Sprite_SetX(j, info.r0_x + kSpawnSplashRing_X[i] - 4);
      Sprite_SetY(j, info.r2_y + kSpawnSplashRing_Y[i] - 4);
      sprite_x_vel[j] = kSpawnSplashRing_Xvel[i];
      sprite_y_vel[j] = kSpawnSplashRing_Yvel[i];
      sprite_A[j] = i;
      sprite_z_vel[j] = (GetRandomNumber() & 15) + 24;
      sprite_ai_state[j] = 1;
      sprite_z[j] = 0;
      sprite_flags3[j] |= 0x40;
      sprite_ignore_projectile[j] = sprite_flags3[j];
    }
  }
}

void ZoraKing_Draw(int k) {  // 859cab
  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;

  if (sprite_ai_state[k] >= 2) {
    static const int8 kZoraKing_Draw_X0[52] = {
      -8,  8,  -8,  8, -8, 8, -8, 8, -8, 8, -8, 8,  -8,  8,  -8,  8,
      0,  0,   0,  0,  0, 0,  0, 0, -8, 8, -8, 8,  -8,  8,  -8,  8,
      -8,  8,  -8,  8, -8, 8, -8, 8, -9, 9, -9, 9, -10, 10, -10, 10,
      -11, 11, -11, 11,
    };
    static const int8 kZoraKing_Draw_Y0[52] = {
      -18, -18, -2, -2, -18, -18, -2, -2, -18, -18, -2, -2, -12, -12, 4, 4,
      0,   0,  0,  0,   0,   0,  0,  0,  -8,  -8,  8,  8,  -8,  -8, 8, 8,
      -8,  -8,  8,  8,  -8,  -8,  8,  8,  -5,  -5,  5,  5,  -5,  -5, 5, 5,
      -5,  -5,  5,  5,
    };
    static const uint8 kZoraKing_Draw_Char0[52] = {
      0xc0, 0xc0, 0xe0, 0xe0, 0xc2, 0xea, 0xe2, 0xe2, 0xea, 0xc2, 0xe2, 0xe2, 0xc0, 0xc0, 0xe4, 0xe6,
      0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0xc4, 0xc6, 0xe4, 0xe6, 0xc6, 0xc4, 0xe6, 0xe4,
      0xe6, 0xe4, 0xc6, 0xc4, 0xe4, 0xe6, 0xc4, 0xc6, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88,
      0x88, 0x88, 0x88, 0x88,
    };
    static const uint8 kZoraKing_Draw_Flags0[52] = {
      0, 0x40,    0, 0x40,    0, 0x40,    0, 0x40, 0, 0x40,    0, 0x40,    0, 0x40,    5,    5,
      5,    5,    5,    5, 0xc5, 0xc5, 0xc5, 0xc5, 5,    5,    5,    5, 0x45, 0x45, 0x45, 0x45,
      0xc5, 0xc5, 0xc5, 0xc5, 0x85, 0x85, 0x85, 0x85, 4, 0x44, 0x84, 0xc4,    4, 0x44, 0x84, 0xc4,
      4, 0x44, 0x84, 0xc4,
    };
    OamEnt *oam = GetOamCurPtr();
    int g = sprite_graphics[k];
    for (int i = 3; i >= 0; i--, oam++) {
      int j = g * 4 + i;
      oam->x = kZoraKing_Draw_X0[j] + info.x;
      oam->y = kZoraKing_Draw_Y0[j] + info.y;
      oam->charnum = kZoraKing_Draw_Char0[j];
      uint8 f = kZoraKing_Draw_Flags0[j];
      oam->flags = (f & 0xf ? f : f | info.flags) | 0x20;
    }
    Sprite_CorrectOamEntries(k, 3, 2);
    if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
      return;
  }

  if (!sprite_delay_aux2[k])
    return;

  static const int8 kZoraKing_Draw_X1[8] = {-23, 23, 23, 23, -20, -15, 13, 18};
  static const int8 kZoraKing_Draw_Y1[8] = {-8, -8, -8, -8, -7, 0, 0, -7};
  static const uint8 kZoraKing_Draw_Char1[8] = {0xae, 0xae, 0xae, 0xae, 0xac, 0xac, 0xac, 0xac};
  static const uint8 kZoraKing_Draw_Flags1[8] = {0, 0x40, 0x40, 0x40, 0, 0, 0x40, 0x40};
  Oam_AllocateFromRegionC(0x10);
  OamEnt *oam = GetOamCurPtr();
  int g = (sprite_delay_aux2[k] >> 1) & 4;
  for (int i = 3; i >= 0; i--, oam++) {
    int j = g + i;
    SetOamPlain(oam, kZoraKing_Draw_X1[j] + info.x, kZoraKing_Draw_Y1[j] + info.y,
                 kZoraKing_Draw_Char1[j], kZoraKing_Draw_Flags1[j] | 0x24, 2);
  }
}

void Sprite_56_WalkingZora(int k) {  // 859d4a
  if (sprite_F[k]) {
    sprite_F[k] = 0;
    sprite_B[k] = 3;
    sprite_G[k] = 192;
    sprite_x_vel[k] = (int8)sprite_x_recoil[k] >> 1;
    sprite_y_vel[k] = (int8)sprite_y_recoil[k] >> 1;
  }
  PrepOamCoordsRet info;
  switch(sprite_B[k]) {
  case 0:  // Waiting
    Sprite_PrepOamCoord(k, &info);
    if (Sprite_ReturnIfInactive(k))
      return;
    if (!sprite_delay_main[k]) {
      sprite_delay_main[k] = 127;
      sprite_B[k]++;
      sprite_flags3[k] |= 64;
    }
    break;
  case 1:  // Surfacing
    Zora_Draw(k);
    if (Sprite_ReturnIfInactive(k))
      return;
    sprite_ignore_projectile[k] = sprite_delay_main[k];
    if (!sprite_delay_main[k]) {
      sprite_flags3[k] &= ~0x40;
      SpriteSfx_QueueSfx2WithPan(k, 0x28);
      sprite_B[k]++;
      sprite_z_vel[k] = 48;
      sprite_head_dir[k] = sprite_D[k] = Sprite_DirectionToFaceLink(k, NULL);
    } else {
      sprite_graphics[k] = kSprite_Zora_SurfacingGfx[sprite_delay_main[k] >> 3];
    }
    break;
  case 2:  // Ambulating
    sprite_graphics[k] = kSprite_Recruit_Gfx[(sprite_subtype2[k] >> 1 & 4) + sprite_D[k]];
    WalkingZora_Draw(k);
    if (Sprite_ReturnIfInactive(k))
      return;
    Sprite_CheckDamageToAndFromLink(k);
    Sprite_MoveZ(k);
    sprite_z_vel[k] -= 2;
    if (sign8(sprite_z[k] - 1)) {
      if (sign8(sprite_z_vel[k] + 16))
        Sprite_ZeroVelocity_XY(k);
      sprite_z[k] = 0;
      sprite_z_vel[k] = 0;
      if (!((k ^ frame_counter) & 15)) {
        int j = Sprite_DirectionToFaceLink(k, NULL);
        sprite_head_dir[k] = j;
        if (!((k ^ frame_counter) & 31)) {
          sprite_D[k] = j;
          Sprite_ApplySpeedTowardsLink(k, 8);
        }
      }
    }
    Sprite_MoveXY(k);
    Sprite_CheckTileCollision(k);
    if (sign8(sprite_z[k] - 1)) {
      WalkingZora_AdjustShadow(k);
      if (sprite_tiletype == 8) {
        Sprite_KillSelf(k);
        SpriteSfx_QueueSfx2WithPan(k, 0x28);
        sprite_state[k] = 3;
        sprite_delay_main[k] = 15;
        sprite_ai_state[k] = 0;
        sprite_flags2[k] = 3;
      }
    }
    sprite_subtype2[k]++;
    break;
  case 3:  // Depressed
    Sprite_CheckDamageFromLink(k);
    if (!(frame_counter & 3) && !--sprite_G[k]) {
      sprite_B[k] = 2;
      if (sprite_state[k] == 10) {
        link_state_bits = 0;
        link_picking_throw_state = 0;
      }
      sprite_state[k] = 9;
    }
    if (sprite_G[k] < 48 && !(frame_counter & 1))
      Sprite_SetX(k, Sprite_GetX(k) + (frame_counter & 2 ? -1 : 1));
    sprite_graphics[k] = 0;
    sprite_wallcoll[k] = 0;
    WalkingZora_DrawWaterRipples(k);
    sprite_flags2[k] -= 2;
    SpriteDraw_SingleLarge(k);
    sprite_flags2[k] += 2;
    sprite_anim_clock[k] = 0;
    if (Sprite_ReturnIfInactive(k))
      return;
    if (Sprite_ReturnIfRecoiling(k))
      return;
    Sprite_MoveXY(k);
    ThrownSprite_TileAndSpriteInteraction(k);
    WalkingZora_AdjustShadow(k);
    break;
  }
}

void WalkingZora_AdjustShadow(int k) {  // 859edb
  sprite_anim_clock[k] = (sprite_z[k] == 0 && sprite_tiletype == 9);
}

void WalkingZora_Draw(int k) {  // 859f08
  static const uint8 kWalkingZora_Draw_Char[4] = {0xce, 0xce, 0xa4, 0xee};
  static const uint8 kWalkingZora_Draw_Flags[4] = {0x40, 0, 0, 0};
  static const uint8 kWalkingZora_Draw_Char2[8] = {0xcc, 0xec, 0xcc, 0xec, 0xe8, 0xe8, 0xca, 0xca};
  static const uint8 kWalkingZora_Draw_Flags2[8] = {0x40, 0x40, 0, 0, 0, 0x40, 0, 0x40};

  PrepOamCoordsRet info;
  WalkingZora_DrawWaterRipples(k);
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  OamEnt *oam = GetOamCurPtr();
  int g = sprite_graphics[k];
  if (g == 0 || g == 2)
    info.y--;

  int i = sprite_head_dir[k];
  SetOamHelper0(oam + 0, info.x, info.y - 6, kWalkingZora_Draw_Char[i], info.flags | kWalkingZora_Draw_Flags[i], 2);
  SetOamHelper0(oam + 1, info.x, info.y + 2, kWalkingZora_Draw_Char2[g], info.flags | kWalkingZora_Draw_Flags2[g], 2);

  if (!sprite_anim_clock[k])
    SpriteDraw_Shadow(k, &info);
}

void WalkingZora_DrawWaterRipples(int k) {  // 859fe0
  if (sprite_anim_clock[k])
    SpriteDraw_WaterRipple_WithOamAdjust(k);
}

void SpriteDraw_WaterRipple_WithOamAdjust(int k) {  // 859fe5
  SpriteDraw_WaterRipple(k);
  oam_cur_ptr += 8;
  oam_ext_cur_ptr += 2;
}

void SpriteDraw_WaterRipple(int k) {  // 859ffa
  static const DrawMultipleData kWaterRipple_Dmd[6] = {
    {0, 10, 0x01d8, 0},
    {8, 10, 0x41d8, 0},
    {0, 10, 0x01d9, 0},
    {8, 10, 0x41d9, 0},
    {0, 10, 0x01da, 0},
    {8, 10, 0x41da, 0},
  };
  static const uint8 kWaterRipple_Idx[4] = {0, 1, 2, 1};
  Sprite_DrawMultiple(k, &kWaterRipple_Dmd[kWaterRipple_Idx[frame_counter >> 2 & 3] * 2], 2, NULL);
  OamEnt *oam = GetOamCurPtr();
  uint8 t = (oam[0].flags & 0x30) | 0x4;
  oam[0].flags = t;
  oam[1].flags = t | 0x40;
}

void Sprite_53_ArmosKnight(int k) {  // 85a036
  static const uint8 kArmosKnight_Gfx1[5] = {5, 4, 3, 2, 1};
  static const int8 kArmosKnight_Xv[2] = {16, -16};

  sprite_obj_prio[k] |= 0x30;
  ArmosKnight_Draw(k);
  if (Sprite_ReturnIfPaused(k))
    return;
  if (sprite_state[k] != 9) {
    if (sprite_delay_main[k]) {
      sprite_graphics[k] = kArmosKnight_Gfx1[sprite_delay_main[k] >> 3];
      return;
    }
    if (--byte_7E0FF8 == 1) {
      for (int j = 5; j >= 0; j--) {
        sprite_health[j] = 48;
        sprite_x_vel[j] = sprite_y_vel[j] = sprite_z_vel[j] = 0;
      }
    }
    sprite_state[k] = 0;
    if (Sprite_CheckIfScreenIsClear()) {
      SpriteSpawnInfo info;
      int j = Sprite_SpawnDynamically(k, 0xea, &info);
      assert(j >= 0);
      Sprite_SetSpawnedCoordinates(j, &info);
      sprite_z_vel[j] = 32;
      sprite_A[j] = 1;
    }
    return;
  }
  Sprite_MoveXY(k);
  Sprite_MoveZ(k);
  sprite_z_vel[k] -= 4;
  if (sign8(sprite_z[k])) {
    sprite_z_vel[k] = 0;
    sprite_z[k] = 0;
    if (byte_7E0FF8 != 1 && sprite_A[k]) {
      sprite_z_vel[k] = 48;
      SpriteSfx_QueueSfx3WithPan(k, 0x16);
    }
  }
  if (sprite_F[k]) {
    Sprite_ZeroVelocity_XY(k);
    sprite_ai_state[k] = 0;
    sprite_G[k] = 0;
  }
  if (Sprite_ReturnIfRecoiling(k))
    return;
  if (!sprite_A[k]) {
    if (!sprite_delay_main[k]) {
      sprite_A[k]++;
      sprite_flags2[k] = (sprite_flags2[k] & 0x7f) - 2;
      sprite_defl_bits[k] &= ~4;
      sprite_flags3[k] &= ~0x40;
    } else {
      if (sprite_delay_main[k] == 64) {
        sound_effect_1 = 0x35;
      } else if (sprite_delay_main[k] < 64) {
        int j = ((sprite_delay_main[k] >> 1) ^ k) & 1;
        sprite_x_vel[k] = kArmosKnight_Xv[j];
        Sprite_MoveX(k);
        sprite_x_vel[k] = 0;
      }
      Sprite_CheckDamageFromLink(k);
      if (Sprite_CheckDamageToLink_same_layer(k)) {
        Sprite_NullifyHookshotDrag();
        Sprite_RepelDash();
      }
    }
  } else if (byte_7E0FF8 == 1) {
    Sprite_ArmosCrusher(k);
  } else {
    Sprite_CheckDamageToAndFromLink(k);
    if (!sprite_ai_state[k]) {
      uint16 x = overlord_y_hi[k] << 8 | overlord_x_hi[k];
      uint16 y = overlord_floor[k] << 8 | overlord_gen2[k];
      ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLocation(k, x, y, 16);
      sprite_x_vel[k] = pt.x;
      sprite_y_vel[k] = pt.y;
      Sprite_Get16BitCoords(k);
      if ((uint16)(x - cur_sprite_x + 2) < 4 && (uint16)(y - cur_sprite_y + 2) < 4)
        sprite_ai_state[k] = 1;
    } else {
      sprite_x_lo[k] = overlord_x_hi[k];
      sprite_x_hi[k] = overlord_y_hi[k];
      sprite_y_lo[k] = overlord_gen2[k];
      sprite_y_hi[k] = overlord_floor[k];
    }
  }
}

void ArmosKnight_Draw(int k) {  // 85a274
  static const int8 kArmosKnight_Draw_X[24] = {
    -8,  8,  -8,  8, -10, 10, -10, 10, -10, 10, -10, 10, -12, 12, -12, 12,
    -14, 14, -14, 14, -16, 24, -16, 24,
  };
  static const int8 kArmosKnight_Draw_Y[24] = {
    -8,  -8,  8,  8, -10, -10, 10, 10, -10, -10, 10, 10, -12, -12, 12, 12,
    -14, -14, 14, 14, -16, -16, 24, 24,
  };
  static const uint8 kArmosKnight_Draw_Char[24] = {
    0xc0, 0xc2, 0xe0, 0xe2, 0xc0, 0xc2, 0xe0, 0xe2, 0xc4, 0xc4, 0xc4, 0xc4, 0xc6, 0xc6, 0xc6, 0xc6,
    0xc8, 0xc8, 0xc8, 0xc8, 0xd8, 0xd8, 0xd8, 0xd8,
  };
  static const uint8 kArmosKnight_Draw_Flags[24] = {
    0, 0,    0,    0,    0, 0,    0,    0, 0x40, 0, 0xc0, 0x80, 0x40, 0, 0xc0, 0x80,
    0x40, 0, 0xc0, 0x80, 0x40, 0, 0xc0, 0x80,
  };
  static const uint8 kArmosKnight_Draw_Big[24] = {
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 0, 0, 0, 0,
  };

  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  if (!sprite_A[k] && submodule_index != 7)
    Oam_AllocateDeferToPlayer(k);
  OamEnt *oam = GetOamCurPtr();
  int g = sprite_graphics[k];
  for (int i = 3; i >= 0; i--, oam++) {
    int j = g * 4 + i;
    SetOamHelper0(oam, info.x + kArmosKnight_Draw_X[j], info.y + kArmosKnight_Draw_Y[j],
                  kArmosKnight_Draw_Char[j], kArmosKnight_Draw_Flags[j] | info.flags,
                  kArmosKnight_Draw_Big[j]);
  }
  if (g != 0)
    return;
  if (sprite_A[k]) {
    int spr_idx = 76 + k * 2;
    oam_cur_ptr = 0x800 + spr_idx * 4;
    oam_ext_cur_ptr = 0xA20 + spr_idx;
  }
  oam = GetOamCurPtr();
  int z = sprite_z[k];
  z = ((z >= 32) ? 32 : z) >> 3;
  uint16 y = Sprite_GetY(k) - BG2VOFS_copy2;
  SetOamHelper0(oam + 4, info.x - 8 + z, y + 12, 0xe4, 0x25, 2);
  SetOamHelper0(oam + 5, info.x + 8 - z, y + 12, 0xe4, 0x25 | 0x40, 2);
}

void Sprite_54_Lanmolas(int k) {  // 85a3a2
  static const uint8 kLanmola_RandB[8] = {0x58, 0x50, 0x60, 0x70, 0x80, 0x90, 0xa0, 0x98};
  static const uint8 kLanmola_RandC[8] = {0x68, 0x60, 0x70, 0x80, 0x90, 0xa0, 0xa8, 0x80};
  static const int8 kLanmola_ZVel[2] = {2, -2};
  PrepOamCoordsRet info;
  Sprite_PrepOamCoord(k, &info);
  Lanmola_Draw(k);
  if (Sprite_ReturnIfPaused(k))
    return;
  switch(sprite_ai_state[k]) {
  case 0:
    if (!(sprite_delay_main[k] | sprite_pause[k])) {
      sprite_delay_main[k] = 127;
      sprite_ai_state[k] = 1;
      SpriteSfx_QueueSfx2WithPan(k, 0x35);
    }
    break;
  case 1:
    if (!sprite_delay_main[k]) {
      Lanmola_SpawnShrapnel(k);
      sound_effect_ambient = 0x13;
      sprite_B[k] = kLanmola_RandB[GetRandomNumber() & 7];
      sprite_C[k] = kLanmola_RandC[GetRandomNumber() & 7];
      sprite_ai_state[k] = 2;
      sprite_z_vel[k] = 24;
      sprite_anim_clock[k] = 0;
      sprite_G[k] = 0;
lbl_a:
      sprite_D[k] = sprite_x_lo[k];
      sprite_wallcoll[k] = sprite_y_lo[k];
      sprite_delay_aux1[k] = 74;
    }
    break;
  case 2: {
    Sprite_CheckDamageToAndFromLink(k);
    Sprite_MoveZ(k);
    if (!sprite_anim_clock[k]) {
      if (!--sprite_z_vel[k])
        sprite_anim_clock[k]++;
    } else if ((frame_counter & 1) == 0) {
      int j = sprite_G[k] & 1;
      if ((sprite_z_vel[k] += kLanmola_ZVel[j]) == (uint8)kDesertBarrier_Xv[j])
        sprite_G[k]++;
    }
    uint16 x = Sprite_GetX(k), y = Sprite_GetY(k);
    uint16 x2 = sprite_x_hi[k] << 8 | sprite_B[k];
    uint16 y2 = sprite_y_hi[k] << 8 | sprite_C[k];
    if ((uint16)(x - x2 + 2) < 4 && (uint16)(y - y2 + 2) < 4)
      sprite_ai_state[k] = 3;
    ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLocation(k, x2, y2, 10);
    sprite_y_vel[k] = pt.y;
    sprite_x_vel[k] = pt.x;
    Sprite_MoveXY(k);
    break;
  }
  case 3:
    Sprite_CheckDamageToAndFromLink(k);
    Sprite_MoveXY(k);
    Sprite_MoveZ(k);
    if (!sign8(sprite_z_vel[k] + 20))
      sprite_z_vel[k] -= 1;
    if (sign8(sprite_z[k])) {
      sprite_ai_state[k] = 4;
      sprite_delay_main[k] = 128;
      goto lbl_a;
    }
    break;
  case 4:
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k] = 0;
      sprite_x_lo[k] = kLanmola_RandB[GetRandomNumber() & 7];
      sprite_y_lo[k] = kLanmola_RandC[GetRandomNumber() & 7];
    }
    break;
  case 5:
    if (!sprite_delay_main[k]) {
      sprite_state[k] = 0;
      if (Sprite_CheckIfScreenIsClear()) {
        SpriteSpawnInfo info;
        int j = Sprite_SpawnDynamically(k, 0xEA, &info);
        assert(j >= 0);
        Sprite_SetSpawnedCoordinates(j, &info);
        sprite_z_vel[j] = 32;
        sprite_A[j] = 3;
      }
    }
    if (sprite_delay_main[k] >= 32 && sprite_delay_main[k] < 160 && !(sprite_delay_main[k] & 15)) {
      int i = ((sprite_subtype2[k] - garnish_y_lo[k] * 8) & 0x3f) + k * 0x40;
      uint8 xlo = moldorm_x_lo[i] - BG2HOFS_copy2;
      uint8 ylo = moldorm_y_lo[i] - beamos_x_hi[i] - BG2VOFS_copy2;
      SpriteSpawnInfo info;
      int j = Sprite_SpawnDynamically(k, 0x00, &info);
      if (j >= 0) {
        load_chr_halfslot_even_odd = 11;
        sprite_state[j] = 4;
        sprite_delay_main[j] = 31;
        sprite_A[j] = 31;
        Sprite_SetX(j, BG2HOFS_copy2 + xlo);
        Sprite_SetY(j, BG2VOFS_copy2 + ylo);
        sprite_flags2[j] = 3;
        sprite_oam_flags[j] = 0xc;
        SpriteSfx_QueueSfx2WithPan(k, 0xc);
        if (!sign8(garnish_y_lo[k]))
          garnish_y_lo[k]--;
      }
    }
    break;
  }
}

void Lanmola_Draw(int k) {  // 85a64a
  static const uint8 kLanmola_SprOffs[4] = {76, 60, 44, 28};
  static const uint8 kLanmola_Draw_Char1[16] = {0xc4, 0xe2, 0xc2, 0xe0, 0xc0, 0xe0, 0xc2, 0xe2, 0xc4, 0xe2, 0xc2, 0xe0, 0xc0, 0xe0, 0xc2, 0xe2};
  static const uint8 kLanmola_Draw_Char0[16] = {0xcc, 0xe4, 0xca, 0xe6, 0xc8, 0xe6, 0xca, 0xe4, 0xcc, 0xe4, 0xca, 0xe6, 0xc8, 0xe6, 0xca, 0xe4};
  static const uint8 kLanmola_Draw_Flags[16] = {0xc0, 0xc0, 0xc0, 0xc0, 0x80, 0x80, 0x80, 0x80, 0, 0, 0, 0, 0x40, 0x40, 0x40, 0x40};

  int spr_offs = kLanmola_SprOffs[k];
  oam_cur_ptr = 0x800 + spr_offs * 4;
  oam_ext_cur_ptr = 0xA20 + spr_offs;
  sprite_graphics[k] = Sprite_ConvertVelocityToAngle(sprite_x_vel[k], sprite_y_vel[k] - sprite_z_vel[k]);
  uint8 r2 = sprite_subtype2[k], r5 = r2;
  int j = k * 64 + r2;
  moldorm_x_lo[j] = sprite_x_lo[k];
  moldorm_y_lo[j] = sprite_y_lo[k];
  beamos_x_hi[j] = sprite_z[k];
  beamos_y_hi[j] = sprite_graphics[k];
  if (sprite_state[k] == 9 && !(submodule_index | flag_unk1))
    sprite_subtype2[k] = sprite_subtype2[k] + 1 & 63;
  uint8 r3 = sprite_oam_flags[k] | sprite_obj_prio[k];
  uint8 n = garnish_y_lo[k];
  if (sign8(n))
    return;
  j = sign8(sprite_y_vel[k]) ? 1 : 0;
  int oam_step = (j ? -1 : 1);
  OamEnt *oam = GetOamCurPtr() + (j ? 7 : 0);
  uint8 i = n;
  do {
    int j = r2 + k * 64;
    r2 = r2 - 8 & 63;
    oam->x = moldorm_x_lo[j] - BG2HOFS_copy2;
    if (!sign8(beamos_x_hi[j]))
      oam->y = moldorm_y_lo[j] - beamos_x_hi[j] - BG2VOFS_copy2;
    j = beamos_y_hi[j];
    if (n != 7 || i != 0) {
      oam->charnum = (n == i) ? kLanmola_Draw_Char1[j] : 0xc6;
    } else {
      oam->charnum = kLanmola_Draw_Char0[j];
    }
    oam->flags = kLanmola_Draw_Flags[j] | r3;
    bytewise_extended_oam[oam - oam_buf] = 2;
  } while (oam += oam_step, !sign8(--i));
  oam = GetOamCurPtr() + 8;
  i = n;
  do {
    int j = r5 + k * 64;
    r5 = r5 - 8 & 63;
    oam->x = moldorm_x_lo[j] - BG2HOFS_copy2;
    if (!sign8(beamos_x_hi[j]))
      oam->y = moldorm_y_lo[j] + 10 - BG2VOFS_copy2;
    oam->charnum = 0x6c;
    oam->flags = 0x34;
    bytewise_extended_oam[oam - oam_buf] = 2;
  } while (oam++, !sign8(--i));

  if (sprite_ai_state[k] == 1) {
    static const uint8 kLanmola_Draw_Idx2[16] = {4, 5, 4, 5, 4, 5, 4, 5, 4, 3, 2, 2, 1, 1, 0, 0};
    static const uint8 kLanmola_Draw_Char2[6] = {0xee, 0xee, 0xec, 0xec, 0xce, 0xce};
    static const uint8 kLanmola_Draw_Flags2[6] = {0, 0x40, 0, 0x40, 0, 0x40};
    Oam_AllocateFromRegionB(4);
    OamEnt *oam = GetOamCurPtr();
    j = kLanmola_Draw_Idx2[sprite_delay_main[k] >> 3];
    SetOamPlain(oam, sprite_x_lo[k] - BG2HOFS_copy2, sprite_y_lo[k] - BG2VOFS_copy2,
                 kLanmola_Draw_Char2[j], kLanmola_Draw_Flags2[j] | 0x31, 2);
  } else if (sprite_ai_state[k] != 5 && sprite_delay_aux1[k] != 0) {
    static const int8 kLanmola_Draw_X4[8] = {-8, 8, -10, 10, -16, 16, -24, 32};
    static const int8 kLanmola_Draw_Y4[8] = {0, 0, -1, -1, -1, -1, 3, 3};
    static const uint8 kLanmola_Draw_Char4[8] = {0xe8, 0xe8, 0xe8, 0xe8, 0xea, 0xea, 0xea, 0xea};
    static const uint8 kLanmola_Draw_Flags4[8] = {0, 0x40, 0, 0x40, 0, 0x40, 0, 0x40};
    static const uint8 kLanmola_Draw_Big4[8] = {2, 2, 2, 2, 2, 2, 0, 0};

    if (((sprite_y_vel[k] >> 6) ^ sprite_ai_state[k]) & 2)
      Oam_AllocateFromRegionB(8);
    else
      Oam_AllocateFromRegionC(8);
    OamEnt *oam = GetOamCurPtr();
    uint8 r6 = (((sprite_delay_aux1[k] >> 2) & 3) ^ 3) * 2;
    uint8 x = sprite_D[k] - BG2HOFS_copy2;
    uint8 y = sprite_wallcoll[k] - BG2VOFS_copy2;
    for (int i = 1; i >= 0; i--, oam++) {
      int j = i + r6;
      SetOamPlain(oam, x + kLanmola_Draw_X4[j], y + kLanmola_Draw_Y4[j], kLanmola_Draw_Char4[j], kLanmola_Draw_Flags4[j] | 0x31, kLanmola_Draw_Big4[j]);
    }
  }
}

void Sprite_6D_Rat(int k) {  // 85a8b0
  static const uint8 kSpriteRat_Tab0[16] = {0, 0, 3, 3, 1, 2, 4, 5, 1, 2, 4, 5, 0, 0, 3, 3};
  static const uint8 kSpriteRat_Tab1[16] = {0, 0x40, 0, 0x40, 0, 0, 0, 0, 0x40, 0x40, 0x40, 0x40, 0x80, 0xc0, 0x80, 0xc0};
  static const uint8 kSpriteRat_Tab2[8] = {10, 11, 6, 7, 2, 3, 14, 15};
  static const uint8 kSpriteRat_Tab4[8] = {8, 9, 4, 5, 0, 1, 12, 13};
  static const int8 kSpriteRat_Xvel[4] = {24, -24, 0, 0};
  static const int8 kSpriteRat_Yvel[4] = {0, 0, 24, -24};
  static const uint8 kSpriteRat_Tab3[4] = {2, 3, 1, 0};

  int j = sprite_A[k];
  sprite_graphics[k] = kSpriteRat_Tab0[j];
  sprite_oam_flags[k] = (sprite_oam_flags[k] & 0x3f) | kSpriteRat_Tab1[j];
  SpriteDraw_SingleLarge(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (Sprite_ReturnIfRecoiling(k))
    return;
  Sprite_CheckDamageToAndFromLink(k);
  Sprite_MoveXY(k);
  Sprite_CheckTileCollision(k);
  if (sprite_ai_state[k] != 0) {
    if (sprite_delay_main[k] == 0) {
      if (is_in_dark_world == 0)
        SpriteSfx_QueueSfx3WithPan(k, 0x17);
      sprite_ai_state[k] = 0;
      sprite_delay_main[k] = 80;
    }
    j = sprite_D[k];
    if (sprite_wallcoll[k] != 0)
      sprite_D[k] = j = kSpriteRat_Tab3[j];
    sprite_x_vel[k] = kSpriteRat_Xvel[j];
    sprite_y_vel[k] = kSpriteRat_Yvel[j];
    sprite_A[k] = kSpriteRat_Tab4[sprite_D[k] * 2 + (frame_counter >> 2 & 1)];
  } else {
    Sprite_ZeroVelocity_XY(k);
    if (sprite_delay_main[k] == 0) {
      uint8 a = GetRandomNumber();
      sprite_D[k] = a & 3;
      sprite_ai_state[k]++;
      sprite_delay_main[k] = (a & 0x7f) + 0x40;
    }
    sprite_A[k] = kSpriteRat_Tab2[sprite_D[k] * 2 + (frame_counter >> 3 & 1)];
  }
}

void Sprite_6E_Rope(int k) {  // 85a973
  int j;
  j = sprite_A[k];
  sprite_graphics[k] = kSpriteRope_Gfx[j];
  sprite_oam_flags[k] = (sprite_oam_flags[k] & 0x3f) | kSpriteRope_Flags[j];
  SpriteDraw_SingleLarge(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (sprite_E[k]) {
    OamEnt *oam = GetOamCurPtr();
    oam[0].flags |= 0x30;

    uint8 old_z = sprite_z[k];
    Sprite_MoveZ(k);
    if (!sign8(sprite_z_vel[k] - 0xc0))
      sprite_z_vel[k] -= 2;
    if (!sign8(old_z ^ sprite_z[k]) || !sign8(sprite_z[k]))
      return;
    sprite_z[k] = 0;
    sprite_z_vel[k] = 0;
    sprite_E[k] = 0;
    sprite_flags3[k] &= ~0x10;
  } else {
    sprite_flags2[k] = 0;
    if (Sprite_ReturnIfRecoiling(k))
      return;
    Sprite_CheckDamageToAndFromLink(k);
    Sprite_MoveXY(k);
    Sprite_CheckTileCollision(k);
    if (sprite_ai_state[k] != 0) {
      if (sprite_delay_main[k] == 0) {
        sprite_ai_state[k] = 0;
        sprite_delay_main[k] = 32;
      }
      j = sprite_D[k];
      if (sprite_wallcoll[k] != 0)
        sprite_D[k] = j = kSpriteRope_Tab0[j];

      j += sprite_G[k];
      sprite_x_vel[k] = kSpriteRope_Xvel[j];
      sprite_y_vel[k] = kSpriteRope_Yvel[j];

      int i = frame_counter;
      if (j < 4)
        i >>= 1;

      sprite_A[k] = kSpriteRope_Tab1[sprite_D[k] * 2 + (i >> 1 & 1)];
    } else {
      Sprite_ZeroVelocity_XY(k);
      if (sprite_delay_main[k] == 0) {
        sprite_G[k] = 0;
        uint8 a = GetRandomNumber();
        sprite_D[k] = a & 3;
        sprite_ai_state[k]++;
        sprite_delay_main[k] = (a & 0x7f) + 0x40;

        PointU8 pt;
        uint8 dir = Sprite_DirectionToFaceLink(k, &pt);
        if ((uint8)(pt.y + 0x10) < 0x20 || (uint8)(pt.x + 0x18) < 0x20) {
          sprite_G[k] = 4;
          sprite_D[k] = dir;
        }
      }
      sprite_A[k] = kSpriteRope_Tab1[sprite_D[k] * 2 + (frame_counter >> 3 & 1)];
    }
  }
}

void Sprite_6F_Keese(int k) {  // 85aa8b
  static const int8 kSpriteKeese_Tab1[2] = {1, -1};
  static const int8 kSpriteKeese_Tab0[4] = {2, 10, 6, 14};

  sprite_obj_prio[k] |= 0x30;
  SpriteDraw_SingleLarge(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (Sprite_ReturnIfRecoiling(k))
    return;
  Sprite_CheckDamageToAndFromLink(k);
  Sprite_MoveXY(k);
  if (sprite_ai_state[k]) {
    if (sprite_delay_main[k] == 0) {
      sprite_ai_state[k] = 0;
      sprite_delay_main[k] = 64;
      sprite_graphics[k] = 0;
      Sprite_ZeroVelocity_XY(k);
    } else {
      if ((sprite_delay_main[k] & 7) == 0) {
        sprite_A[k] += kSpriteKeese_Tab1[sprite_B[k] & 1];
        if (!(GetRandomNumber() & 3))
          sprite_B[k]++;
      }
      int j = sprite_A[k] & 0xf;
      sprite_x_vel[k] = kSpriteKeese_Tab2[j];
      sprite_y_vel[k] = kSpriteKeese_Tab3[j];
      sprite_graphics[k] = ((frame_counter >> 2) & 1) + 1;
    }
  } else {
    if ((k ^ frame_counter) & 3 | sprite_delay_main[k])
      return;

    PointU8 pt;
    uint8 dir = Sprite_DirectionToFaceLink(k, &pt);
    if ((uint8)(pt.y + 0x28) >= 0x50 || (uint8)(pt.x + 0x28) >= 0x50)
      return;
    SpriteSfx_QueueSfx3WithPan(k, 0x1e);
    sprite_ai_state[k]++;
    sprite_delay_main[k] = 64;
    sprite_B[k] = 64;
    sprite_A[k] = kSpriteKeese_Tab0[dir];
  }
}

void Sprite_Cannonball(int k) {  // 85ab54
  SpriteDraw_SingleLarge(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_MoveXY(k);
  if (sprite_delay_main[k] == 30) {
    Sprite_SpawnPoofGarnish(k);
  } else if (sprite_delay_main[k] == 0 && Sprite_CheckTileCollision(k)) {
    sprite_state[k] = 0;
    sprite_x_lo[k] += 4;
    sprite_y_lo[k] += 4;
    Sprite_PlaceRupulseSpark_2(k);
    SpriteSfx_QueueSfx2WithPan(k, 0x5);
  }
  Sprite_CheckDamageToAndFromLink(k);
}

void Sprite_SpawnPoofGarnish(int j) {  // 85ab9c
  int k = GarnishAllocForce();
  garnish_type[k] = 10;
  garnish_active = 10;
  garnish_x_lo[k] = sprite_x_lo[j];
  garnish_x_hi[k] = sprite_x_hi[j];
  int y = Sprite_GetY(j) + 16;
  garnish_y_lo[k] = y;
  garnish_y_hi[k] = y >> 8;
  garnish_sprite[k] = sprite_floor[j];
  garnish_countdown[k] = 15;
}

void Sprite_6C_MirrorPortal(int k) {  // 85af75
  if (savegame_is_darkworld) {
    sprite_state[k] = 0;
  } else {
    if (BYTE(overworld_screen_index) >= 0x80)
      return;

    if (submodule_index != 0x23 && byte_7E0FC6 < 3)
      SpriteDraw_SingleLarge(k);
    if (Sprite_ReturnIfInactive(k))
      return;
    int j = frame_counter >> 2 & 3;
    sprite_oam_flags[k] = sprite_oam_flags[k] & 0x3f | kSprite_WarpVortex_Flags[j];
    if (Sprite_CheckIfLinkIsBusy())
      return;
    if (Sprite_CheckDamageToLink_same_layer(k)) {
      if (sprite_A[k] && (link_disable_sprite_damage | countdown_for_blink) == 0 && !flag_is_link_immobilized) {
        submodule_index = 0x23;
        link_triggered_by_whirlpool_sprite = 1;
        subsubmodule_index = 0;
        link_actual_vel_y = 0;
        link_actual_vel_x = 0;
        link_player_handler_state = kPlayerState_Mirror;
        last_light_vs_dark_world = overworld_screen_index & 0x40;
        sprite_state[k] = 0;
      }
    } else {
      sprite_A[k] = 1;
    }
  }
  if (++sprite_B[k] == 0)
    sprite_A[k] = 1;
  sprite_x_lo[k] = bird_travel_x_lo[15];
  sprite_x_hi[k] = bird_travel_x_hi[15];
  int t = (bird_travel_y_lo[15] | (bird_travel_y_hi[15] << 8)) + 8;
  sprite_y_lo[k] = t;
  sprite_y_hi[k] = t >> 8;
}

void Sprite_6A_BallNChain(int k) {  // 85b01b
  ChainBallTrooper_Draw(k);
  if (sprite_ai_state[k] < 2)
    HIBYTE(dungmap_var8) = 0x80;
  if (Sprite_ReturnIfInactive(k))
    return;
  Guard_ParrySwordAttacks(k);

  int t = sprite_B[k] << 8 | sprite_A[k];
  t += kChainBallTrooper_Tab1[sprite_ai_state[k]];
  sprite_A[k] = t;
  sprite_B[k] = t >> 8 & 1;
  if (Sprite_ReturnIfRecoiling(k))
    return;
  Sprite_CheckTileCollision(k);
  Sprite_MoveXY(k);
  Sprite_CheckDamageToLink(k);

  PointU8 pt = { 0, 0 };

  if (((k ^ frame_counter) & 0xf) == 0)
    sprite_head_dir[k] = Sprite_DirectionToFaceLink(k, &pt);

  switch (sprite_ai_state[k]) {
  case 0:
    if (((k ^ frame_counter) & 0xf) == 0) {
      sprite_D[k] = sprite_head_dir[k];
      if ((uint8)(pt.y + 0x40) < 0x68 && (uint8)(pt.x + 0x30) < 0x60) {
        sprite_ai_state[k]++;
        sprite_delay_main[k] = 24;
        return;
      }
      Sprite_ApplySpeedTowardsLink(k, 8);
    }
    BallNChain_Animate(k);
    break;
  case 1:
    Sprite_ZeroVelocity_XY(k);
    if (!sprite_delay_main[k]) {
      sprite_delay_main[k] = 48;
      sprite_ai_state[k]++;
    }
    break;
  case 2:
    if (!sprite_delay_main[k] &&
        sprite_head_dir[k] == kFlailTrooperAttackDir[(sprite_A[k] >> 7 & 1) + sprite_B[k] * 2]) {
      sprite_ai_state[k]++;
      sprite_delay_aux2[k] = 31;
    }
attack_common:
    sprite_subtype2[k]++;
    BallNChain_Animate(k);
    if (((k ^ frame_counter) & 0xf) == 0)
      SpriteSfx_QueueSfx3WithPan(k, 6);
    break;
  case 3:
    Sprite_ZeroVelocity_XY(k);
    t = sprite_delay_aux2[k];
    if (t == 0)
      sprite_ai_state[k] = 0;
    else if (t >= 0x10)
      goto attack_common;
    sprite_subtype2[k]++;
    BallNChain_Animate(k);
    break;
  }
}

void BallNChain_Animate(int k) {  // 85b0ab
  sprite_graphics[k] = kFlailTrooperGfx[sprite_D[k] * 8 + (++sprite_subtype2[k] >> 2 & 7)];
}

void SpriteDraw_GuardHead(int k, PrepOamCoordsRet *info, int spr_offs) {  // 85b160
  int j = sprite_head_dir[k];
  OamEnt *oam = GetOamCurPtr() + spr_offs;
  SetOamHelper0(oam, info->x, info->y - 9, kChainBallTrooperHead_Char[j], info->flags | kChainBallTrooperHead_Flags[j], 2);
}

void SpriteDraw_BNCBody(int k, PrepOamCoordsRet *info, int spr_offs) {  // 85b3cb
  int g = sprite_graphics[k];
  spr_offs += kFlailTrooperBody_SprOffs[g] >> 2;
  OamEnt *oam = GetOamCurPtr() + spr_offs;
  int n = kFlailTrooperBody_Num[g];
  do {
    int j = g * 3 + n;
    SetOamHelper0(oam, info->x + kFlailTrooperBody_X[j], info->y + kFlailTrooperBody_Y[j],
                  kFlailTrooperBody_Char[j], info->flags | kFlailTrooperBody_Flags[j],
                  kFlailTrooperBody_Big[j]);
    if (n == 2)
      oam++;
  } while (oam++, --n >= 0);
}

void SpriteDraw_BNCFlail(int k, PrepOamCoordsRet *info) {  // 85b468
  OamEnt *oam = GetOamCurPtr();

  BYTE(dungmap_var7) = info->x;
  HIBYTE(dungmap_var7) = info->y;

  uint8 t;
  uint16 r0 = sprite_A[k] | sprite_B[k] << 8;
  uint8 qq = sprite_ai_state[k] < 2 ? 0 : kFlailTrooperWeapon_Tab0[sprite_delay_aux2[k]];
  uint8 r12 = kFlailTrooperWeapon_Tab1[sprite_D[k]];
  uint8 r13 = kFlailTrooperWeapon_Tab2[sprite_D[k]];

  uint16 r2 = (r0 + 0x80) & 0x1ff;

  uint8 r14 = ChainBallMult(kSinusLookupTable[r0 & 0xff], qq);
  uint8 r4 = (r0 & 0x100) ? -r14 : r14;

  uint8 r15 = ChainBallMult(kSinusLookupTable[r2 & 0xff], qq);
  uint8 r6 = (r2 & 0x100) ? -r15 : r15;

  HIBYTE(dungmap_var8) = r4 - 4 + r12;
  BYTE(dungmap_var8) = r6 - 4 + r13;

  SetOamPlain(oam, HIBYTE(dungmap_var8) + BYTE(dungmap_var7), BYTE(dungmap_var8) + HIBYTE(dungmap_var7), 0x2a, 0x2d, 2);
  oam++;

  for (int i = 3; i >= 0; i--, oam++) {
    t = (kFlailTrooperWeapon_Tab4[i] * r14) >> 8;
    t = sign8(r4) ? -t : t;
    uint8 x = t + BYTE(dungmap_var7) + r12;
    uint8 ty = (kFlailTrooperWeapon_Tab4[i] * r15) >> 8;
    ty = sign8(r6) ? -ty : ty;
    uint8 y = ty + HIBYTE(dungmap_var7) + r13;
    SetOamPlain(oam, x, y, 0x3f, 0x2d, 0);
  }
  Sprite_CorrectOamEntries(k, 4, 0xff);
}

void Sprite_50_Cannonball(int k) {  // 85b648
  if (!sprite_ai_state[k])
    SpriteDraw_SingleLarge(k);
  else
    SpriteDraw_BigCannonball(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  sprite_graphics[k] = ++sprite_subtype2[k] >> 2 & 1;
  Sprite_MoveXY(k);
  if (sprite_delay_main[k]) {
    if (sprite_delay_main[k] == 1)
      sprite_state[k] = 0;
    return;
  }
  Sprite_CheckDamageToAndFromLink(k);
  if (!sprite_delay_aux2[k] && Sprite_CheckTileCollision(k))
    sprite_delay_main[k] = 16;
}

void SpriteDraw_BigCannonball(int k) {  // 85b6a4
  static const int8 kMetalBallLarge_X[4] = {-8, 8, -8, 8};
  static const int8 kMetalBallLarge_Y[4] = {-8, -8, 8, 8};
  static const uint8 kMetalBallLarge_Char[8] = {0x84, 0x88, 0x88, 0x88, 0x86, 0x88, 0x88, 0x88};
  static const uint8 kMetalBallLarge_Flags[4] = {0, 0, 0xc0, 0x80};

  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  OamEnt *oam = GetOamCurPtr();
  int g = sprite_graphics[k];
  for (int i = 3; i >= 0; i--, oam++) {
    SetOamHelper0(oam, info.x + kMetalBallLarge_X[i], info.y + kMetalBallLarge_Y[i],
                  kMetalBallLarge_Char[g * 4 + i], kMetalBallLarge_Flags[i] | info.flags, 2);
  }
}

void Sprite_51_ArmosStatue(int k) {  // 85b703
  Armos_Draw(k);
  if (sprite_F[k])
    Sprite_ZeroVelocity_XY(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_MoveZ(k);
  sprite_z_vel[k] -= 2;
  if (sign8(sprite_z[k])) {
    sprite_z[k] = 0;
    sprite_z_vel[k] = 0;
    Sprite_ZeroVelocity_XY(k);
  }
  if (!sprite_ai_state[k]) {
    sprite_flags3[k] |= 0x40;
    if (sprite_delay_main[k] == 1) {
      sprite_flags3[k] &= ~0x40;
      sprite_ai_state[k]++;
      sprite_flags2[k] &= ~0x80;
      sprite_flags3[k] &= ~0x40;
      sprite_oam_flags[k] = 0xb;
    } else {
      if (!((k ^ frame_counter) & 3) &&
          (uint16)(link_x_coord - cur_sprite_x + 31) < 62 &&
          (uint16)(link_y_coord + 8 - cur_sprite_y + 48) < 88 && !sprite_delay_main[k]) {
        sprite_delay_main[k] = 48;
        SpriteSfx_QueueSfx2WithPan(k, 0x22);
      }
      if (Sprite_CheckDamageToLink_same_layer(k)) {
        Sprite_NullifyHookshotDrag();
        Sprite_RepelDash();
      }
      if (sprite_delay_main[k])
        sprite_oam_flags[k] ^= (sprite_delay_main[k] >> 1) & 0xe;
    }
  } else {
    Sprite_CheckDamageToAndFromLink(k);
    if (Sprite_ReturnIfRecoiling(k))
      return;
    Sprite_MoveXY(k);
    Sprite_CheckTileCollision(k);
    if ((sprite_delay_main[k] | sprite_z[k]) == 0) {
      sprite_delay_main[k] = 8;
      sprite_z_vel[k] = 16;
      Sprite_ApplySpeedTowardsLink(k, 12);
    }
  }
}

void Armos_Draw(int k) {  // 85b7ef
  static const DrawMultipleData kArmos_Dmd[2] = {
    {0, -16, 0x00c0, 2},
    {0,   0, 0x00e0, 2},
  };
  PrepOamCoordsRet info;
  if (!sprite_ai_state[k]) {
    if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
      return;
  }
  Sprite_DrawMultiple(k, &kArmos_Dmd[0], 2, &info);
  SpriteDraw_Shadow(k, &info);
}

void Sprite_4E_Popo(int k) {  // 85b80a
  Bot_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (Sprite_ReturnIfRecoiling(k))
    return;
  sprite_subtype2[k]++;
  sprite_A[k] = sprite_subtype2[k] >> 4 & 3;
  Sprite_CheckDamageToAndFromLink(k);
  switch (sprite_ai_state[k]) {
  case 0:
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k] = 1;
      sprite_delay_main[k] = 105;
    }
    break;
  case 1:
    sprite_subtype2[k]++;
    if (!sprite_delay_main[k]) {
      sprite_delay_main[k] = (GetRandomNumber() & 63) + 128;
      sprite_ai_state[k]++;
      int j = GetRandomNumber() & 15;
      sprite_x_vel[k] = kSpriteKeese_Tab2[j] << 2;
      sprite_y_vel[k] = kSpriteKeese_Tab3[j] << 2;
    }
    break;
  case 2:
    sprite_subtype2[k]++;
    if (sprite_delay_main[k]) {
      if ((k ^ frame_counter) & sprite_B[k]) {
        Sprite_CheckTileCollision(k);
        return;
      }
      Sprite_MoveXY(k);
      if (!sprite_wallcoll[k]) {
        Sprite_CheckTileCollision(k);
        return;
      }
    }
    sprite_ai_state[k] = 0;
    sprite_delay_main[k] = 80;
    break;
  }
}

void Bot_Draw(int k) {  // 85b89a
  static const uint8 kBot_Gfx[4] = {0, 1, 0, 1};
  static const uint8 kBot_OamFlags[4] = {0, 0, 0x40, 0x40};
  int j = sprite_A[k];
  sprite_graphics[k] = kBot_Gfx[j];
  sprite_oam_flags[k] = sprite_oam_flags[k] & ~0x40 | kBot_OamFlags[j];
  SpriteDraw_SingleLarge(k);
}

void Sprite_4C_Geldman(int k) {  // 85b8b3
  static const uint8 kGerudoMan_EmergeGfx[8] = {3, 2, 0, 0, 0, 0, 0, 0};
  static const uint8 kGerudoMan_PursueGfx[2] = {4, 5};
  static const uint8 kGerudoMan_SubmergeGfx[5] = {0, 1, 2, 3, 3};
  PrepOamCoordsRet info;
  if (sprite_ai_state[k] < 2)
    Sprite_PrepOamCoord(k, &info);
  else
    GerudoMan_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (Sprite_ReturnIfRecoiling(k))
    return;
  sprite_ignore_projectile[k] = 1;
  switch (sprite_ai_state[k]) {
  case 0:  // return
    if (!sprite_delay_main[k]) {
      sprite_x_lo[k] = sprite_A[k];
      sprite_x_hi[k] = sprite_B[k];
      sprite_y_lo[k] = sprite_C[k];
      sprite_y_hi[k] = sprite_head_dir[k];
      sprite_ai_state[k] = 1;
    }
    break;
  case 1:  // wait
    if (!((k ^ frame_counter) & 7) &&
        (uint16)(link_x_coord - cur_sprite_x + 0x30) < 0x60 &&
        (uint16)(link_y_coord - cur_sprite_y + 0x30) < 0x60) {
      sprite_ai_state[k] = 2;
      sprite_delay_main[k] = 31;
    }
    break;
  case 2:  // emerge
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k] = 3;
      sprite_delay_main[k] = 96;
      Sprite_ApplySpeedTowardsLink(k, 16);
    } else {
      sprite_graphics[k] = kGerudoMan_EmergeGfx[sprite_delay_main[k] >> 2];
    }
    break;
  case 3:  // pursue
    sprite_ignore_projectile[k] = 0;
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k] = 4;
      sprite_delay_main[k] = 8;
    } else {
      sprite_graphics[k] = kGerudoMan_PursueGfx[sprite_delay_main[k] >> 2 & 1];
      Sprite_CheckDamageToAndFromLink(k);
      Sprite_MoveXY(k);
    }
    break;
  case 4:  // submerge
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k] = 0;
      sprite_delay_main[k] = 16;
    } else {
      sprite_graphics[k] = kGerudoMan_SubmergeGfx[sprite_delay_main[k] >> 1];
    }
    break;
  }
}

void GerudoMan_Draw(int k) {  // 85ba24
  static const int8 kGerudoMan_Draw_X[18] = { 4, 4, 4, 4, 4, 4, -8, 8, 8, -8, 8, 8, -16, 0, 16, -16, 0, 16 };
  static const int8 kGerudoMan_Draw_Y[18] = { 8, 8, 8, 8, 8, 8, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  static const uint8 kGerudoMan_Draw_Char[18] = {
    0xb8, 0xb8, 0xb8, 0xb8, 0xb8, 0xb8, 0xa6, 0xa6, 0xa6, 0xa6, 0xa6, 0xa6, 0xa4, 0xa2, 0xa0, 0xa0, 0xa2, 0xa4,
  };
  static const uint8 kGerudoMan_Draw_Flags[18] = {
    0, 0, 0, 0x40, 0x40, 0x40, 0, 0x40, 0x40, 0, 0x40, 0x40, 0x40, 0x40, 0x40, 0, 0, 0,
  };
  static const uint8 kGerudoMan_Draw_Big[18] = { 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 };
  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  OamEnt *oam = GetOamCurPtr();
  int g = sprite_graphics[k];
  for (int i = 2; i >= 0; i--, oam++) {
    int j = g * 3 + i;
    SetOamHelper0(oam, info.x + kGerudoMan_Draw_X[j], info.y + kGerudoMan_Draw_Y[j],
                  kGerudoMan_Draw_Char[j], kGerudoMan_Draw_Flags[j] | info.flags, kGerudoMan_Draw_Big[j]);
  }
}

void Sprite_4D_Toppo(int k) {  // 85ba85
  static const int8 kToppo_XOffs[4] = {-32, 32, 0, 0};
  static const int8 kToppo_YOffs[4] = {0, 0, -32, 32};

  if (sprite_ai_state[k]) {
    sprite_obj_prio[k] |= 0x30;
    Toppo_Draw(k);
  }
  if (Sprite_ReturnIfInactive(k))
    return;
  switch (sprite_ai_state[k]) {
  case 0:  // Toppo_Hiding
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k] = 1;
      sprite_delay_main[k] = 8;
      int j = GetRandomNumber() & 3;
      uint16 x = sprite_A[k] | sprite_B[k] << 8;
      uint16 y = sprite_C[k] | sprite_head_dir[k] << 8;
      Sprite_SetX(k, x + kToppo_XOffs[j]);
      Sprite_SetY(k, y + kToppo_YOffs[j]);
    }
    break;
  case 1:  // Toppo_RustleGrass
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k] = 2;
      sprite_delay_main[k] = 16;
    } else {
      sprite_graphics[k] = sprite_delay_main[k] >> 2 & 1;
      Toppo_VerifyTile(k);
    }
    break;
  case 2:  // Toppo_PokingOut
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k] = 3;
      sprite_z_vel[k] = 64;
    }
    sprite_graphics[k] = 2;
    Toppo_VerifyTile(k);
    break;
  case 3:  // Toppo_Leaping
    Sprite_CheckDamageToAndFromLink(k);
    Sprite_MoveZ(k);
    sprite_z_vel[k] -= 2;
    if (sign8(sprite_z[k])) {
      sprite_z[k] = 0;
      sprite_ai_state[k] = 4;
      sprite_delay_main[k] = 16;
      Toppo_VerifyTile(k);
    }
    break;
  case 4:  // Toppo_Retreat
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k] = 0;
      sprite_delay_main[k] = 32;
    } else {
      sprite_graphics[k] = sprite_delay_main[k] >> 2 & 1;
    }
    Toppo_VerifyTile(k);
    break;
  case 5:  // Toppo_Flustered
    Toppo_Flustered(k);
    break;
  }
}

void Toppo_VerifyTile(int k) {  // 85bb72
  uint16 x = Sprite_GetX(k);
  if (GetTileAttribute(0, &x, Sprite_GetY(k)) != 0x40)
    sprite_ai_state[k] = 5;
}

void Toppo_Draw(int k) {  // 85bbff
  static const int8 kToppo_Draw_X[15] = {0, 8, 8, 0, 8, 8, 0, 0, 8, 0, 0, 0, 0, 0, 0};
  static const int8 kToppo_Draw_Y[15] = {8, 8, 8, 8, 8, 8, 0, 8, 8, 0, 0, 0, 0, 0, 0};
  static const uint8 kToppo_Draw_Char[15] = {0xc8, 0xc8, 0xc8, 0xca, 0xca, 0xca, 0xc0, 0xc8, 0xc8, 0xc2, 0xc2, 0xc2, 0xc2, 0xc2, 0xc2};
  static const uint8 kToppo_Draw_Flags[15] = {0, 0x40, 0x40, 0, 0x40, 0x40, 0, 0, 0x40, 0, 0, 0, 0x40, 0x40, 0x40};
  static const uint8 kToppo_Draw_Big[15] = {0, 0, 0, 0, 0, 0, 2, 0, 0, 2, 2, 2, 2, 2, 2};

  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  OamEnt *oam = GetOamCurPtr();
  int g = sprite_graphics[k];
  uint16 ybase = Sprite_GetY(k) - BG2VOFS_copy2;
  for (int i = 2; i >= 0; i--, oam++) {
    int j = i + g * 3;
    uint8 big = kToppo_Draw_Big[j];
    uint8 flags = kToppo_Draw_Flags[j] | info.flags;
    if (big == 0)
      flags = flags & ~0xf | 2;
    SetOamHelper0(oam,
                  info.x + kToppo_Draw_X[j],
                  (big ? info.y : ybase) + kToppo_Draw_Y[j],
                  kToppo_Draw_Char[j], flags, big);
  }
}

void Sprite_4B_GreenKnifeGuard(int k) {  // 85bca2
  sprite_graphics[k] = kSprite_Recruit_Gfx[sprite_D[k] + (sprite_subtype2[k] >> 1 & 4) ];
  Recruit_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (Sprite_ReturnIfRecoiling(k))
    return;
  Sprite_CheckDamageToAndFromLink(k);
  Sprite_MoveXY(k);
  Sprite_CheckTileCollision(k);
  if (sprite_ai_state[k] != 0) {
    GreenKnifeGuard_Moving(k);
    return;
  }
  if (sprite_delay_main[k] != 0)
    return;

  sprite_delay_main[k] = (GetRandomNumber() & 0x3f) + 0x30;
  sprite_ai_state[k]++;
  sprite_D[k] = sprite_head_dir[k];
  PointU8 out;
  int j = sprite_D[k];
  if (j == Sprite_DirectionToFaceLink(k, &out) &&
     (((uint8)(out.x + 0x10) < 0x20) || ((uint8)(out.y + 0x10) < 0x20))) {
    j += 4;
    sprite_delay_main[k] = 128;
  }
  sprite_x_vel[k] = kSprite_Recruit_Xvel[j];
  sprite_y_vel[k] = kSprite_Recruit_Yvel[j];
}

void GreenKnifeGuard_Moving(int k) {  // 85bd1e
  uint8 t = 0x10;

  if (sprite_wallcoll[k] == 0) {
    if (sprite_delay_main[k] != 0)
      goto out;
    t = 0x30;
  }
  sprite_delay_main[k] = t;
  Sprite_ZeroVelocity_XY(k);
  sprite_head_dir[k] = kRecruit_Moving_HeadDir[sprite_D[k] * 2 | (GetRandomNumber() & 1)];
  sprite_ai_state[k] = 0;
out:
  sprite_subtype2[k] += (sprite_delay_aux1[k] != 0) ? 2 : 1;
}

void Recruit_Draw(int k) {  // 85bd7e
  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  OamEnt *oam = GetOamCurPtr();
  int hd = sprite_head_dir[k];
  SetOamHelper0(oam + 0, info.x, info.y - 11, kSoldier_Draw1_Char[hd], kSoldier_Draw1_Flags[hd] | info.flags, 2);
  int r6 = sprite_graphics[k];
  SetOamHelper0(oam + 1, info.x + kRecruit_Draw_X[r6], info.y, kRecruit_Draw_Char[r6], kRecruit_Draw_Flags[r6] | info.flags, 2);
  SpriteDraw_Shadow(k, &info);
}

void Sprite_4A_BombGuard(int k) {  // 85be0a
  if (sprite_C[k] == 0) {
    BombGuard(k);
    return;
  }
  if (sprite_C[k] < 2) {
    SpriteBomb_ExplosionIncoming(k);
    return;
  }

  if (sprite_C[k] == 2) {
    for (int j = 15; j >= 0; j--) {
      if (j != cur_object_index && sprite_state[j] >= 9 && !((frame_counter ^ j) & 7 | sprite_hit_timer[j]))
        SpriteBomb_CheckDamageToSprite(k, j);
    }
    Sprite_CheckDamageToLink(k);
  }
  SpriteDraw_SpriteBombExplosion(k);
  if (!sprite_delay_aux1[k])
    sprite_state[k] = 0;
}

void SpriteBomb_CheckDamageToSprite(int k, int j) {  // 85be49
  int x = Sprite_GetX(k) - 16, y = Sprite_GetY(k) - 16;
  SpriteHitBox hb;
  hb.r0_xlo = x;
  hb.r8_xhi = x >> 8;
  hb.r3 = hb.r2 = 48;
  hb.r1_ylo = y;
  hb.r9_yhi = y >> 8;
  Sprite_SetupHitBox(j, &hb);
  if (!CheckIfHitBoxesOverlap(&hb) || sprite_type[j] == 0x11)
    return;
  Ancilla_CheckDamageToSprite_preset(j, 8);
  x = Sprite_GetX(j);
  y = Sprite_GetY(j) - sprite_z[j];
  ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLocation(k, x, y, 32);
  sprite_y_recoil[j] = pt.y;
  sprite_x_recoil[j] = pt.x;
}

void SpriteBomb_ExplosionIncoming(int k) {  // 85bed3
  if (sprite_E[k])
    sprite_obj_prio[k] |= 48;
  SpriteDraw_SingleLarge(k);
  if (sprite_hit_timer[k] || sprite_delay_aux1[k] == 1) {
    sprite_hit_timer[k] = 0;
    if (sprite_state[k] == 10)
      link_state_bits = 0, link_picking_throw_state = 0;
    SpriteSfx_QueueSfx2WithPan(k, 0xc);
    sprite_C[k]++;
    sprite_flags4[k] = 9;
    sprite_oam_flags[k] = 2;
    sprite_delay_aux1[k] = 31;
    sprite_state[k] = 6;
    sprite_flags2[k] = 3;
    return;
  }
  if (sprite_delay_aux1[k] < 64)
    sprite_oam_flags[k] = sprite_oam_flags[k] & ~0xe | (sprite_delay_aux1[k] >> 1) & 0xe;
  if (Sprite_ReturnIfInactive(k))
    return;
  if (!sprite_delay_aux3[k])
    Sprite_CheckDamageFromLink(k);
  Sprite_MoveXY(k);
  if (player_is_indoors)
    Sprite_CheckTileCollision(k);
  ThrownSprite_TileAndSpriteInteraction(k);
}

void BombGuard(int k) {  // 85bf51
  BombTrooper_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_CheckDamageToAndFromLink(k);
  sprite_head_dir[k] = sprite_D[k] = Sprite_DirectionToFaceLink(k, NULL);
  if (sprite_ai_state[k] == 0) {
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k] = 1;
      sprite_delay_main[k] = 112;
    }
  } else {
    int j = sprite_delay_main[k];
    if (!j) {
      sprite_ai_state[k] = 0;
      sprite_delay_main[k] = 32;
      return;
    }
    sprite_subtype2[k] = (j >= 80);
    if (j == 32)
      BombGuard_CreateBomb(k);
    sprite_graphics[k] = kJavelinTrooper_Tab2[(sprite_D[k] << 3 | j >> 4) + 32];
  }
}

void BombGuard_CreateBomb(int k) {  // 85bfc1
  static const int8 kBombTrooperBomb_X[4] = {0, 1, 9, -8};
  static const int8 kBombTrooperBomb_Y[4] = {-12, -12, -15, -13};
  static const int8 kBombTrooperBomb_Zvel[16] = {32, 40, 48, 56, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64};
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0x4a, &info);
  if (j >= 0) {
    int i = sprite_D[k];
    Sprite_SetX(j, info.r0_x + kBombTrooperBomb_X[i]);
    Sprite_SetY(j, info.r2_y + kBombTrooperBomb_Y[i]);
    Sprite_ApplySpeedTowardsLink(j, 16);
    PointU8 pt;
    sprite_C[j] = 1;
    Sprite_DirectionToFaceLink(j, &pt);
    if (sign8(pt.x))
      pt.x = -pt.x;
    if (sign8(pt.y))
      pt.y = -pt.y;
    sprite_z_vel[j] = kBombTrooperBomb_Zvel[(pt.y | pt.x) >> 4];
    sprite_flags3[j] = sprite_flags3[k] & 0xee | 0x18;
    sprite_oam_flags[j] = 8;
    sprite_delay_aux1[j] = 255;
    sprite_health[j] = 0;
    SpriteSfx_QueueSfx3WithPan(j, 0x13);
  }
}

void BombTrooper_Draw(int k) {  // 85c04b
  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  SpriteDraw_GuardHead(k, &info, 2);
  SpriteDraw_BNCBody(k, &info, 1);
  if (sprite_graphics[k] < 20)
    SpriteDraw_BombGuard_Arm(k, &info);
  SpriteDraw_Shadow_custom(k, &info, 10);
}

void SpriteDraw_BombGuard_Arm(int k, PrepOamCoordsRet *info) {  // 85c089
  static const int8 kBombTrooper_DrawArm_X[8] = {-1, 1, 2, 0, 9, 9, -8, -8};
  static const int8 kBombTrooper_DrawArm_Y[8] = {-12, -12, -12, -12, -16, -14, -12, -14};
  OamEnt *oam = GetOamCurPtr();
  int j = sprite_D[k] * 2 | sprite_subtype2[k];
  SetOamHelper0(oam,
                info->x + kBombTrooper_DrawArm_X[j],
                info->y + kBombTrooper_DrawArm_Y[j], 0x6e, info->flags & 0x30 | 0x8, 2);
}

void SpriteDraw_SpriteBombExplosion(int k) {  // 85c113
  static const int8 kEnemyBombExplosion_X[16] = {-12, 12, -12, 12, -8, 8, -8, 8, -8, 8, -8, 8, 0, 0, 0, 0};
  static const int8 kEnemyBombExplosion_Y[16] = {-12, -12, 12, 12, -8, -8, 8, 8, -8, -8, 8, 8, 0, 0, 0, 0};
  static const uint8 kEnemyBombExplosion_Char[16] = {0x88, 0x88, 0x88, 0x88, 0x8a, 0x8a, 0x8a, 0x8a, 0x84, 0x84, 0x84, 0x84, 0x86, 0x86, 0x86, 0x86};
  static const uint8 kEnemyBombExplosion_Flags[16] = {0, 0x40, 0x80, 0xc0, 0, 0x40, 0x80, 0xc0, 0, 0x40, 0x80, 0xc0, 0, 0, 0, 0};
  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  OamEnt *oam = GetOamCurPtr();
  int base = sprite_delay_aux1[k] >> 1 & 0xc;
  for (int i = 3; i >= 0; i--, oam++) {
    int j = base + i;
    oam->x = kEnemyBombExplosion_X[j] + info.x;
    oam->y = kEnemyBombExplosion_Y[j] + info.y;
    oam->charnum = kEnemyBombExplosion_Char[j];
    oam->flags = kEnemyBombExplosion_Flags[j] | info.flags;
  }
  Sprite_CorrectOamEntries(k, 3, 2);
}

void Sprite_41_BlueGuard(int k) {  // 85c155
  if (sprite_C[k])
    Probe(k);
  else
    Guard_Main(k);
}

void Probe(int k) {  // 85c15d
  SpriteAddXY(k, (int8)sprite_x_vel[k], (int8)sprite_y_vel[k]);
  bool is_close;
  if (sprite_type[sprite_C[k] - 1] == 0xce) {
    // parent is blind the thief?
    uint16 x = cur_sprite_x - link_x_coord + 16;
    uint16 y = link_y_coord - cur_sprite_y + 24;
    is_close = (x < 32 && y < 32);
  } else {
    if (Probe_CheckTileSolidity(k) && sprite_tiletype != 9 || link_cape_mode != 0) {
      sprite_state[k] = 0;
      return;
    }
    uint16 x = cur_sprite_x - link_x_coord;
    uint16 y = cur_sprite_y - link_y_coord;
    is_close = (x < 16 && y < 16 && sprite_floor[k] == link_is_on_lower_level);
  }
  if (is_close) {
    int p = sprite_C[k] - 1;
    if (sprite_ai_state[p] != 3) {
      sprite_ai_state[p] = 3;
      if (sprite_type[p] != 0xce) {
        sprite_delay_main[p] = 16;
        sprite_subtype2[p] = 0;
      }
    }
    sprite_state[k] = 0;
  } else {
    PrepOamCoordsRet oam;
    if (Sprite_PrepOamCoordOrDoubleRet(k, &oam))
      return;
    if ((oam.x | oam.y) >= 256)
      sprite_state[k] = 0;
  }
}

void Guard_Main(int k) {  // 85c227
  uint8 bak1 = sprite_graphics[k];
  uint8 bak2 = sprite_D[k];

  if (sprite_delay_aux1[k]) {
    sprite_D[k] = kSoldier_DirectionLockSettings[bak2];
    sprite_graphics[k] = kSoldier_Gfx[bak2];
  }
  Guard_HandleAllAnimation(k);
  sprite_D[k] = bak2;
  sprite_graphics[k] = bak1;

  if (sprite_state[k] == 5) {
    if (submodule_index == 0) {
      sprite_subtype2[k]++;
      Guard_TickAndUpdateBody(k);
      sprite_subtype2[k]++;
      Guard_TickAndUpdateBody(k);
    }
    return;
  }
  if (Sprite_ReturnIfInactive(k))
    return;
  Guard_ParrySwordAttacks(k);
  if ((Sprite_CheckDamageToLink(k) || sprite_alert_flag) && sprite_ai_state[k] < 3) {
    sprite_ai_state[k] = 3;
    Guard_SetTimerAndAssertTileHitBox(k, 0x20);
  } else if (sprite_F[k] != 0 && sprite_F[k] >= 4) {
    sprite_ai_state[k] = 4;
    Guard_SetTimerAndAssertTileHitBox(k, 0x80);
  }
  if (Sprite_ReturnIfRecoiling(k))
    return;
  if ((sprite_subtype[k] & 7) < 5) {
    if (!sprite_wallcoll[k])
      Sprite_MoveXY(k);
    Sprite_CheckTileCollision(k);
  } else {
    Sprite_MoveXY(k);
  }
  if (sprite_ai_state[k] != 4)
    sprite_G[k] = 0;

  switch (sprite_ai_state[k]) {
  case 0:
    Sprite_ZeroVelocity_XY(k);
    if (sprite_delay_main[k])
      break;
    sprite_ai_state[k]++;
    if (sprite_subtype[k] && (sprite_subtype[k] & 7) < 5) {
      sprite_delay_main[k] = kSoldier_Delay[sprite_subtype[k] >> 3 & 3];
      sprite_D[k] ^= 1;
      sprite_subtype2[k] = 0;
    } else {
      sprite_delay_main[k] = (GetRandomNumber() & 0x3f) + 0x28; // note: adc
      uint8 t = sprite_D[k], u = GetRandomNumber() & 3;
      sprite_D[k] = u;
      if (t == u || (t ^ u) & 2)
        return;
    }
    sprite_delay_aux1[k] = 12;
    break;
  case 1: {
    Sprite_Guard_SendOutProbe(k);
    if ((sprite_subtype[k] & 7) >= 5) {
      Guard_ShootProbeAndStuff(k);
      return;
    }
    if (!sprite_delay_main[k]) {
      Sprite_ZeroVelocity_XY(k);
      sprite_ai_state[k] = 2;
      sprite_delay_main[k] = 160;
      return;
    }
    if (!(sprite_subtype2[k] & 1))
      sprite_delay_main[k]++;
    if (sprite_wallcoll[k] & 0xf) {
      sprite_D[k] ^= 1;
      Guard_SetGlanceTo12(k);
    }
    int dir = sprite_D[k];
    sprite_x_vel[k] = kSoldier_Xvel[dir];
    sprite_y_vel[k] = kSoldier_Yvel[dir];
    sprite_head_dir[k] = dir;
    Guard_TickAndUpdateBody(k);
    break;
  }
  case 2: {
    Sprite_ZeroVelocity_XY(k);
    Sprite_Guard_SendOutProbe(k);
    if (sprite_delay_main[k] == 0) {
      sprite_delay_main[k] = 0x20;
      sprite_ai_state[k] = 0;
    } else if (sprite_delay_main[k] < 0x80) {
      int t = sprite_D[k] * 8 | (sprite_delay_main[k] >> 3 & 7);
      sprite_head_dir[k] = kSoldier_HeadDirs[t];
    }
    break;
  }
  case 3:
    Sprite_ZeroVelocity_XY(k);
    sprite_head_dir[k] = Sprite_DirectionToFaceLink(k, NULL);
    if (sprite_delay_main[k] == 0) {
      sprite_ai_state[k] = 4;
      Guard_SetTimerAndAssertTileHitBox(k, 255);
    }
    break;
  case 4:
    if (sprite_delay_main[k]) {
      Soldier_Func12(k);
    } else {
      sprite_anim_clock[k] = kSoldier_Tab1[sprite_D[k]];
      Sprite_ZeroVelocity_XY(k);
      sprite_ai_state[k] = 2;
      sprite_delay_main[k] = 160;
    }
    break;
  }
}

void Guard_SetGlanceTo12(int k) {  // 85c32b
  sprite_delay_aux1[k] = 12;
}

void Guard_ShootProbeAndStuff(int k) {  // 85c3a1
  int i = sprite_B[k];
  sprite_x_vel[k] = kSoldierB_Xvel[i];
  sprite_y_vel[k] = kSoldierB_Yvel[i];
  Sprite_CheckTileCollision(k);
  if (sprite_delay_aux2[k]) {
    if (sprite_delay_aux2[k] == 44)
      sprite_B[k] = i = kSoldierB_NextB[i];
  } else if (!(sprite_wallcoll[k] & kSoldierB_Mask[i])) {
    sprite_delay_aux2[k] = 88;
  }
  if (sprite_wallcoll[k] & kSoldierB_Mask2[i])
    sprite_B[k] = i = kSoldierB_NextB2[i];
  sprite_x_vel[k] = kSoldierB_Xvel2[i];
  sprite_y_vel[k] = kSoldierB_Yvel2[i];
  sprite_head_dir[k] = sprite_D[k] = kSoldierB_Dir[i];
  Guard_TickAndUpdateBody(k);
}

void Guard_TickAndUpdateBody(int k) {  // 85c454
  sprite_subtype2[k]++;
  int t = sprite_D[k] * 4 + (sprite_subtype2[k] >> 3 & 3);
  sprite_graphics[k] = kSoldier_Gfx2[t];
}

void Guard_SetTimerAndAssertTileHitBox(int k, uint8 a) {  // 85c4d7
  sprite_delay_main[k] = a;
  sprite_subtype[k] = 0;
  sprite_flags[k] = sprite_flags[k] & 0xf | 0x60;
}

void Soldier_Func12(int k) {  // 85c500
  if (((k ^ frame_counter) & 0x1f) == 0) {
    if (!sprite_G[k]) {
      sprite_G[k] = 1;
      SpriteSfx_QueueSfx3WithPan(k, 4);
    }
    Sprite_ApplySpeedTowardsLink(k, 16);
    sprite_D[k] = sprite_head_dir[k] = Sprite_DirectionToFaceLink(k, NULL);
  }
  Guard_ApplySpeedInDirection(k);
  sprite_subtype2[k]++;
  Guard_TickAndUpdateBody(k);
}

void Guard_ApplySpeedInDirection(int k) {  // 85c542
  if (!sprite_wallcoll[k])
    return;
  int i;
  if (sprite_wallcoll[k] & 3) {
    i = 2 + Sprite_IsBelowLink(k).a;
  } else {
    i = Sprite_IsRightOfLink(k).a;
  }
  sprite_x_vel[k] = kSoldier_SetTowardsVel[i];
  sprite_y_vel[k] = kSoldier_SetTowardsVel[i + 2];
}

void Sprite_Guard_SendOutProbe(int k) {  // 85c5f2
  if ((k + frame_counter & 3) | sprite_pause[k])
    return;
  uint8 a = sprite_anim_clock[k]++;
  uint8 r15 = ((a & 0x1f) + kSprite_SpawnProbeStaggered_Tab[sprite_D[k]]) & 0x3f;
  Sprite_SpawnProbeAlways(k, r15);
}

void Sprite_SpawnProbeAlways(int k, uint8 r15) {  // 85c612
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamicallyEx(k, 0x41, &info, 10);
  if (j < 0)
    return;
  int t = info.r0_x + 8;
  sprite_x_lo[j] = t;
  sprite_x_hi[j] = t >> 8;
  t = info.r2_y + 4;
  sprite_y_lo[j] = t;
  sprite_y_hi[j] = t >> 8;
  sprite_D[j] = r15;
  sprite_x_vel[j] = kSpawnProbe_Xvel[r15];
  sprite_y_vel[j] = kSpawnProbe_Yvel[r15];
  sprite_flags2[j] = sprite_flags2[j] & 0xf0 | 0xa0;
  sprite_C[j] = k + 1;
  sprite_ignore_projectile[j] = k + 1;
  sprite_flags4[j] = 0x40;
  sprite_flags3[j] = 0x40;
  sprite_defl_bits[j] = 2;
}

void Guard_HandleAllAnimation(int k) {  // 85c680
  PrepOamCoordsRet poc;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &poc))
    return;
  Guard_AnimateHead(k, 0, &poc);
  Guard_AnimateBody(k, kSoldier_Draw2_OamIdx[sprite_D[k]] >> 2, &poc);
  Guard_AnimateWeapon(k, &poc);
  if (sprite_flags3[k] & 0x10)
    SpriteDraw_Shadow_custom(k, &poc, kSoldier_DrawShadow[sprite_D[k]]);
}

void Guard_AnimateHead(int k, int oam_offs, const PrepOamCoordsRet *poc) {  // 85c6de
  OamEnt *oam = GetOamCurPtr() + oam_offs;
  int dir = sprite_head_dir[k];
  SetOamHelper0(oam, poc->x, poc->y - kSoldier_Draw1_Yd[sprite_graphics[k]],
                kSoldier_Draw1_Char[dir], kSoldier_Draw1_Flags[dir] | poc->flags, 2);
}

void Guard_AnimateBody(int k, int oam_idx, const PrepOamCoordsRet *poc) {  // 85ca09
  int g = sprite_graphics[k] * 4;
  uint8 type = sprite_type[k];
  OamEnt *oam = GetOamCurPtr() + oam_idx;
  for (int i = 3; i >= 0; i--) {
    int j = i + g;
    if (type >= 0x46 && (!kSoldier_Draw2_Big[j] || i == 3 && kSoldier_Draw2_Char[j] == 0x20))
      continue;
    uint8 flags = kSoldier_Draw2_Flags[j] | poc->flags;
    if (kSoldier_Draw2_Char[j] == 0x20) {
      flags = flags & 0xf1 | 2;
    } else if (kSoldier_Draw2_Big[j] == 0) {
      flags = flags & 0xf1 | 8;
    }
    SetOamHelper0(oam, poc->x + kSoldier_Draw2_Xd[j], poc->y + kSoldier_Draw2_Yd[j],
                  kSoldier_Draw2_Char[j], flags, kSoldier_Draw2_Big[j]);
    if (oam->charnum == 0x20 && type == 0x46)
      oam->y = 0xf0;
    oam++;
  }
}

void Guard_AnimateWeapon(int k, const PrepOamCoordsRet *poc) {  // 85cb64
  int oam_idx = kSoldier_Draw3_OamIdx[sprite_D[k]] >> 2;
  int g = sprite_graphics[k] * 2;
  uint8 type = sprite_type[k];
  OamEnt *oam = GetOamCurPtr() + oam_idx;
  for (int i = 1; i >= 0; i--, oam++) {
    int j = i + g;
    dungmap_var8 = kSoldier_Draw3_Xd[j] << 8 | (uint8)kSoldier_Draw3_Yd[j];
    SetOamHelper0(oam, poc->x + kSoldier_Draw3_Xd[j], poc->y + kSoldier_Draw3_Yd[j],
                  kSoldier_Draw3_Char[j] + (type < 0x43 ? 3 : 0),
                  kSoldier_Draw3_Flags[j] | poc->flags, 0);
  }
}

void Sprite_45_HogSpearMan(int k) {  // 85cbe0
  Guard_HandleAllAnimation(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  BoltGuard_TriggerChaseTheme(k);
  Guard_ParrySwordAttacks(k);
  if (Sprite_ReturnIfRecoiling(k))
    return;
  if (!sprite_wallcoll[k])
    Sprite_MoveXY(k);
  Sprite_CheckTileCollision(k);
  Sprite_CheckDamageToLink(k);
  if (!((k ^ frame_counter) & 15)) {
    sprite_head_dir[k] = sprite_D[k] = Sprite_DirectionToFaceLink(k, NULL);
    Sprite_ApplySpeedTowardsLink(k, 18);
    Guard_ApplySpeedInDirection(k);
  }
  sprite_subtype2[k]++;
  Guard_TickAndUpdateBody(k);
}

void BoltGuard_TriggerChaseTheme(int k) {  // 85cc3c
  if (sprite_G[k] != 16 && sprite_G[k]++ == 15) {
    SpriteSfx_QueueSfx3WithPan(k, 0x4);
    if (sram_progress_indicator == 2 && BYTE(overworld_area_index) == 24)
      music_control = 12;
  }
}

void Sprite_44_BluesainBolt(int k) {  // 85cc65
  PsychoTrooper_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  BoltGuard_TriggerChaseTheme(k);
  Guard_ParrySwordAttacks(k);
  if (Sprite_ReturnIfRecoiling(k))
    return;
  if (!sprite_wallcoll[k])
    Sprite_MoveXY(k);
  Sprite_CheckTileCollision(k);
  Sprite_CheckDamageToLink(k);
  if (!((k ^ frame_counter) & 15)) {
    sprite_D[k] = sprite_head_dir[k] = Sprite_DirectionToFaceLink(k, NULL);
    Sprite_ApplySpeedTowardsLink(k, 18);
    Guard_ApplySpeedInDirection(k);
  }
  sprite_graphics[k] = kFlailTrooperGfx[++sprite_subtype2[k] >> 1 & 7 | sprite_D[k] << 3];
}

void PsychoTrooper_Draw(int k) {  // 85ccd5
  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  SpriteDraw_GuardHead(k, &info, 3);
  SpriteDraw_BNCBody(k, &info, 2);
  SpriteDraw_GuardSpear(k, &info, 0);
  if (sprite_flags3[k] & 0x10)
    SpriteDraw_Shadow_custom(k, &info, kSoldier_DrawShadow[sprite_D[k]]);
}

void SpriteDraw_GuardSpear(int k, PrepOamCoordsRet *info, int spr_offs) {  // 85cd54
  static const int8 kSolderThrowing_Draw_X[16] = {15, 7, 17, 9, -8, 0, -10, -2, 13, 13, 13, 13, -4, -4, -4, -4};
  static const int8 kSolderThrowing_Draw_Y[16] = {-2, -2, -2, -2, -2, -2, -2, -2, 8, 0, 10, 2, -14, -6, -16, -8};
  static const uint8 kSolderThrowing_Draw_Char[16] = {0x6f, 0x7f, 0x6f, 0x7f, 0x6f, 0x7f, 0x6f, 0x7f, 0x6e, 0x7e, 0x6e, 0x7e, 0x6e, 0x7e, 0x6e, 0x7e};
  static const uint8 kSolderThrowing_Draw_Flags[16] = {0x40, 0x40, 0x40, 0x40, 0, 0, 0, 0, 0x80, 0x80, 0x80, 0x80, 0, 0, 0, 0};

  OamEnt *oam = GetOamCurPtr() + spr_offs;
  uint8 r6 = sprite_D[k] * 4 + (((sprite_A[k] ^ 1) << 1) & 2);
  for (int i = 1; i >= 0; i--, oam++) {
    int j = r6 + i;
    uint16 x = info->x + kSolderThrowing_Draw_X[j];
    uint16 y = info->y + kSolderThrowing_Draw_Y[j];
    HIBYTE(dungmap_var8) = kSolderThrowing_Draw_X[j];
    BYTE(dungmap_var8) = kSolderThrowing_Draw_Y[j];
    SetOamHelper0(oam, x, y,
                  kSolderThrowing_Draw_Char[j] - (sprite_type[k] >= 0x48 ? 3 : 0),
                  (kSolderThrowing_Draw_Flags[j] | info->flags) & 0xf1 | 8, 0);
  }
}

void Sprite_48_RedJavelinGuard(int k) {  // 85cde1
  static const uint8 kJavelinTrooper_Gfx[4] = {12, 0, 18, 8};
  uint8 bak0 = sprite_graphics[k];
  int j = sprite_D[k];
  if (sprite_delay_aux1[k] != 0) {
    sprite_D[k] = kSoldier_DirectionLockSettings[j];
    sprite_graphics[k] = kJavelinTrooper_Gfx[j];
  }
  JavelinTrooper_Draw(k);
  sprite_D[k] = j;
  sprite_graphics[k] = bak0;
  SoldierThrowing_Common(k);
}

void Sprite_46_BlueArcher(int k) {  // 85cdff
  uint8 bak0 = sprite_graphics[k];
  int j = sprite_D[k];
  if (sprite_delay_aux1[k] != 0) {
    sprite_D[k] = kSoldier_DirectionLockSettings[j];
    sprite_graphics[k] = kSoldier_Gfx[j];
  }
  ArcherSoldier_Draw(k);
  sprite_D[k] = j;
  sprite_graphics[k] = bak0;
  SoldierThrowing_Common(k);
}

void SoldierThrowing_Common(int k) {  // 85ce23
  int j;

  static const uint8 kSolderThrowing_DirFlags[4] = {3, 3, 12, 12};
  static const int8 kSolderThrowing_Xd[8] = {-80, 80, 0, -8, -80, 80, -8, 8};
  static const int8 kSolderThrowing_Yd[8] = {8, 8, -80, 80, 8, 8, -80, 80};

  if (Sprite_ReturnIfInactive(k))
    return;

  if ((Sprite_CheckDamageToAndFromLink(k) || sprite_alert_flag) && sprite_ai_state[k] < 3) {
    sprite_ai_state[k] = 3;
    sprite_delay_main[k] = 32;
  }
  if (sprite_F[k] >= 4) {
    sprite_ai_state[k] = 4;
    sprite_delay_main[k] = 60;
    sprite_subtype2[k] = 0;
  }
  if (Sprite_ReturnIfRecoiling(k))
    return;
  if (!sprite_wallcoll[k])
    Sprite_MoveXY(k);
  Sprite_CheckTileCollision(k);
  switch (sprite_ai_state[k]) {
  case 0:  // resting
    Sprite_ZeroVelocity_XY(k);
    if (sprite_delay_main[k] == 0) {
      sprite_ai_state[k]++;
      sprite_delay_main[k] = 0x50 + (GetRandomNumber() & 0x7f);
      uint8 jbak = sprite_D[k];
      sprite_D[k] = GetRandomNumber() & 3;
      if (sprite_D[k] != jbak && !((sprite_D[k] ^ jbak) & 2))
        sprite_delay_aux1[k] = 12;
    }
    break;
  case 1:  // walking
    if (sprite_delay_main[k] == 0) {
      sprite_ai_state[k] = 2;
      sprite_delay_main[k] = 160;
      return;
    }
    Sprite_Guard_SendOutProbe(k);
    if (sprite_wallcoll[k] & 0xf) {
      sprite_D[k] ^= 1;
      Guard_SetGlanceTo12(k);
    }
    j = sprite_D[k];
    sprite_x_vel[k] = kSoldier_Xvel[j];
    sprite_y_vel[k] = kSoldier_Yvel[j];
    sprite_head_dir[k] = j;
agitated_jump_to:
    sprite_subtype2[k]++;
    if (!(sprite_subtype2[k] & 0xf) && ++sprite_A[k] == 2)
      sprite_A[k] = 0;
    sprite_graphics[k] = kSoldier_Gfx2[sprite_D[k] * 4 + sprite_A[k] + (sprite_type[k] == 0x48 ? 16 : 0)];
    break;
  case 2:  // looking
    Sprite_ZeroVelocity_XY(k);
    Sprite_Guard_SendOutProbe(k);
    if (sprite_delay_main[k] == 0) {
      sprite_delay_main[k] = 32;
      sprite_ai_state[k] = 0;
    } else if (sprite_delay_main[k] < 0x80) {
      int t = sprite_D[k] * 8 | (sprite_delay_main[k] >> 3 & 7);
      sprite_head_dir[k] = kSoldier_HeadDirs[t];
    }
    break;
  case 3:  // noticed player
    Sprite_ZeroVelocity_XY(k);
    sprite_head_dir[k] = Sprite_DirectionToFaceLink(k, NULL);
    if (sprite_delay_main[k] == 0) {
      sprite_ai_state[k] = 4;
      sprite_delay_main[k] = 60;
      sprite_subtype2[k] = 0;
    }
    break;
  case 4:  // agitated
    j = sprite_D[k];
    if (sprite_wallcoll[k] & kSolderThrowing_DirFlags[j] || !sprite_delay_main[k]) {
      sprite_ai_state[k]++;
      sprite_delay_main[k] = 24;
      return;
    }
    if (!((frame_counter ^ k) & 7)) {
      sprite_D[k] = sprite_head_dir[k] = j = Sprite_DirectionToFaceLink(k, NULL);
      if (sprite_type[k] == 0x48)
        j += 4;
      uint16 x = link_x_coord + kSolderThrowing_Xd[j];
      uint16 y = link_y_coord + kSolderThrowing_Yd[j];
      ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLocation(k, x, y, 24);
      sprite_x_vel[k] = pt.x;
      sprite_y_vel[k] = pt.y;
      if ((uint8)(pt.xdiff + 6) < 12 && (uint8)(pt.ydiff + 6) < 12) {
        sprite_ai_state[k]++;
        sprite_delay_main[k] = 24;
        return;
      }
    }
    sprite_subtype2[k]++;
    goto agitated_jump_to;
  case 5:  // attack
    sprite_anim_clock[k] = kSoldier_Tab1[sprite_D[k]];
    Sprite_ZeroVelocity_XY(k);
    if ((j = sprite_delay_main[k]) == 0) {
      sprite_ai_state[k] = 2;
      sprite_delay_main[k] = 160;
      return;
    }
    sprite_subtype2[k] = (j >= 40) ? 255 : 0;
    if (j == 12)
      Guard_LaunchProjectile(k);
    sprite_graphics[k] = kJavelinTrooper_Tab2[sprite_D[k] * 8 + (j >> 3) + (sprite_type[k] == 0x48 ? 32 : 0)];
    break;
  }
}

void Guard_LaunchProjectile(int k) {  // 85d0c5
  static const int8 kJavelinProjectile_X[8] = {16, -8, 3, 11, 12, -4, 12, -4};
  static const int8 kJavelinProjectile_Y[8] = {2, 2, 16, -8, -2, -2, 2, -8};
  static const int8 kJavelinProjectile_Xvel[8] = {48, -48, 0, 0, 32, -32, 0, 0};
  static const int8 kJavelinProjectile_Yvel[8] = {0, 0, 48, -48, 0, 0, 32, -32};
  static const uint8 kJavelinProjectile_Flags4[4] = {5, 5, 6, 6};

  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0x1b, &info);
  if (j < 0)
    return;
  SpriteSfx_QueueSfx3WithPan(k, 0x5);
  int i = sprite_D[k] + (sprite_type[k] >= 0x48 ? 4 : 0);

  Sprite_SetX(j, info.r0_x + kJavelinProjectile_X[i]);
  Sprite_SetY(j, info.r2_y + kJavelinProjectile_Y[i]);
  sprite_x_vel[j] = kJavelinProjectile_Xvel[i];
  sprite_y_vel[j] = kJavelinProjectile_Yvel[i];
  i &= 3;
  sprite_D[j] = i;
  sprite_flags4[j] = kJavelinProjectile_Flags4[i];
  sprite_z[j] = 0;
  sprite_A[j] = (sprite_type[k] >= 0x48);
  if (sprite_A[j] && link_shield_type == 0)
    sprite_flags5[j] &= ~0x20;
}

void BushJavelinSoldier_Draw(int k) {  // 85d141
  uint8 bak0 = sprite_graphics[k];
  sprite_graphics[k] = 0;
  uint8 bak1 = sprite_oam_flags[k];
  sprite_oam_flags[k] = sprite_oam_flags[k] & 0xf1 | 2;
  uint16 bak2 = cur_sprite_y;
  cur_sprite_y += 8;
  SpriteDraw_SingleLarge(k);
  cur_sprite_y = bak2;
  sprite_oam_flags[k] = bak1;
  sprite_graphics[k] = bak0;

  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  Guard_AnimateHead(k, 0x10 / 4, &info);
  SpriteDraw_BNCBody(k, &info, 0xC / 4);
  if (sprite_graphics[k] < 20)
    SpriteDraw_GuardSpear(k, &info, 4 / 4);
  if (sprite_flags3[k] & 0x10)
    SpriteDraw_Shadow_custom(k, &info, kSoldier_DrawShadow[sprite_D[k]]);

}

void JavelinTrooper_Draw(int k) {  // 85d192
  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  SpriteDraw_GuardHead(k, &info, 3);
  SpriteDraw_BNCBody(k, &info, 2);
  if (sprite_graphics[k] < 20)
    SpriteDraw_GuardSpear(k, &info, 0);
  if (sprite_flags3[k] & 0x10)
    SpriteDraw_Shadow_custom(k, &info, kSoldier_DrawShadow[sprite_D[k]]);
}

void Sprite_49_RedBushGuard(int k) {  // 85d1ac
  if (sprite_ai_state[k]) {
    if (sprite_ai_state[k] == 2)
      BushJavelinSoldier_Draw(k);
    else
      BushSoldierCommon_Draw(k);
  }
  Sprite_BushGuard_Main(k);
}

void Sprite_47_GreenBushGuard(int k) {  // 85d1bf
  if (sprite_ai_state[k]) {
    if (sprite_graphics[k] >= 14)
      ArcherSoldier_Draw(k);
    else
      BushSoldierCommon_Draw(k);
  }
  Sprite_BushGuard_Main(k);
}

void Sprite_BushGuard_Main(int k) {  // 85d1d3
  int j;
  static const uint8 kBushSoldier_Gfx[32] = {
    4, 4, 4, 4, 4, 4, 4, 4, 0, 1, 0, 1, 0, 1, 0, 1,
    0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
  };
  static const uint8 kBushSoldier_Gfx2[16] = {0, 1, 0, 1, 0, 1, 0, 1, 0, 2, 3, 4, 4, 4, 4, 4};
  if (Sprite_ReturnIfInactive(k))
    return;
  sprite_ignore_projectile[k] = 1;
  switch (sprite_ai_state[k]) {
  case 0:
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k] = 1;
      sprite_delay_main[k] = 64;
    }
    break;
  case 1:
    Sprite_CheckDamageFromLink(k);
    if (sprite_delay_main[k] == 0) {
      sprite_ai_state[k] = 2;
      sprite_delay_main[k] = 48;
      sprite_head_dir[k] = sprite_D[k] = Sprite_DirectionToFaceLink(k, NULL);;
    } else {
      if (sprite_delay_main[k] == 0x20)
        BushGuard_SpawnFoliage(k);
      sprite_graphics[k] = kBushSoldier_Gfx[sprite_delay_main[k] >> 2];
    }
    break;
  case 2:
    sprite_ignore_projectile[k] = 0;
    Sprite_CheckDamageToAndFromLink(k);
    j = sprite_delay_main[k];
    if (!j) {
      sprite_ai_state[k] = 3;
      sprite_delay_main[k] = 48;
      goto case_3;
    }
    sprite_A[k] = j < 40 ? 0xff : 0x00;
    if (j == 16)
      Guard_LaunchProjectile(k);
    sprite_graphics[k] = kJavelinTrooper_Tab2[sprite_D[k] * 8 + (j >> 3) + (sprite_type[k] == 0x49 ? 32 : 0)];
    break;
  case 3:
  case_3:
    Sprite_CheckDamageToAndFromLink(k);
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k] = 0;
      sprite_delay_main[k] = 64;
    } else {
      sprite_graphics[k] = kBushSoldier_Gfx2[sprite_delay_main[k] >> 2];
    }
    break;
  }
}

void BushGuard_SpawnFoliage(int k) {  // 85d252
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0xec, &info);
  if (j < 0)
    return;
  Sprite_SetSpawnedCoordinates(j, &info);
  sprite_state[j] = 6;
  sprite_delay_main[j] = 32;
  sprite_flags2[j] += 3;
  sprite_C[j] = 2;
}

void BushSoldierCommon_Draw(int k) {  // 85d321
  static const int8 kBushSoldierCommon_Y[14] = {8, 8, 8, 8, 2, 8, 0, 8, -3, 8, -3, 8, -3, 8};
  static const uint8 kBushSoldierCommon_Char[14] = {0x20, 0x20, 0x20, 0x20, 0x40, 0x20, 0x40, 0x20, 0x40, 0x20, 0x42, 0x20, 0x42, 0x20};
  static const uint8 kBushSoldierCommon_Flags[14] = {9, 3, 0x49, 0x43, 9, 3, 0x49, 0x43, 9, 3, 0x49, 0x43, 9, 3};
  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  OamEnt *oam = GetOamCurPtr();
  int g = sprite_graphics[k] * 2;
  for (int i = 1; i >= 0; i--, oam++) {
    int j = g + i;
    uint8 flags = kBushSoldierCommon_Flags[j] | 0x20;
    if (i == 0)
      flags = flags & ~0xe | info.flags;
    SetOamHelper0(oam, info.x, info.y + kBushSoldierCommon_Y[j], kBushSoldierCommon_Char[j], flags, 2);
  }
}

void ArcherSoldier_Draw(int k) {  // 85d38c
  static const uint8 kArcherSoldier_WeaponOamOffs[4] = {0, 0, 0, 16};
  static const uint8 kArcherSoldier_HeadOamOffs[4] = {16, 16, 16, 0};
  static const uint8 kArcherSoldier_BodyOamOffs[4] = {20, 20, 20, 4};
  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  Guard_AnimateHead(k, kArcherSoldier_HeadOamOffs[sprite_D[k]] >> 2, &info);
  Guard_AnimateBody(k, kArcherSoldier_BodyOamOffs[sprite_D[k]] >> 2, &info);
  SpriteDraw_Archer_Weapon(k, kArcherSoldier_WeaponOamOffs[sprite_D[k]] >> 2, &info);
  if (sprite_flags3[k] & 0x10)
    SpriteDraw_Shadow_custom(k, &info, kSoldier_DrawShadow[sprite_D[k]]);
}

void SpriteDraw_Archer_Weapon(int k, int spr_offs, PrepOamCoordsRet *info) {  // 85d4d4
  static const uint8 kArcherSoldier_Tab1[4] = {9, 3, 0, 6};
  static const int8 kArcherSoldier_Draw_X[48] = {
    -1,  7,  3,  3, -1,  7,  3,  3, -1,  7,  7,  7, -5, -5, -10, -2,
    -4, -4, -6,  2, -5, -5, -5, -5,  6, 14, 11, 11,  6, 14,  11, 11,
    6, 14, 14, 14, 11, 11, 18, 10, 12, 12, 14,  6, 11, 11,  11, 11,
  };
  static const int8 kArcherSoldier_Draw_Y[48] = {
    7,  7,  3, 11,  6, 6, 1, 9,  7,  7,   7,  7, -2,  6,  2,  2,
    -2,  6,  2,  2, -2, 6, 6, 6, -6, -6, -12, -4, -6, -6, -9, -1,
    -6, -6, -6, -6, -2, 6, 2, 2, -2,  6,   2,  2, -2,  6,  6,  6,
  };
  static const uint8 kArcherSoldier_Draw_Char[48] = {
    0xa,  0xa, 0x2a, 0x2b, 0x1a, 0x1a, 0x2a, 0x2b,  0xa,  0xa,  0xa,  0xa, 0xb, 0xb, 0x3d, 0x3a,
    0x1b, 0x1b, 0x3d, 0x3a,  0xb,  0xb,  0xb,  0xb,  0xa,  0xa, 0x2b, 0x2a, 0xa, 0xa, 0x2b, 0x2a,
    0xa,  0xa,  0xa,  0xa,  0xb,  0xb, 0x3d, 0x3a, 0x1b, 0x1b, 0x3d, 0x3a, 0xb, 0xb,  0xb,  0xb,
  };
  static const uint8 kArcherSoldier_Draw_Flags[48] = {
    0xd, 0x4d,    8,    8,  0xd, 0x4d,    8,    8,  0xd, 0x4d, 0x4d, 0x4d,  0xd, 0x8d, 0x48, 0x48,
    0xd, 0x8d, 0x48, 0x48,  0xd, 0x8d, 0x8d, 0x8d, 0x8d, 0xcd, 0x88, 0x88, 0x8d, 0xcd, 0x88, 0x88,
    0x8d, 0xcd, 0xcd, 0xcd, 0x4d, 0xcd,    8,    8, 0x4d, 0xcd,    8,    8, 0x4d, 0xcd, 0xcd, 0xcd,
  };
  OamEnt *oam = GetOamCurPtr() + spr_offs;
  int base = sprite_graphics[k] - 14;
  if (base < 0)
    base = kArcherSoldier_Tab1[sprite_D[k]];
  for (int i = 3; i >= 0; i--, oam++) {
    int j = base * 4 + i;
    SetOamHelper0(oam, info->x + kArcherSoldier_Draw_X[j], info->y + kArcherSoldier_Draw_Y[j], kArcherSoldier_Draw_Char[j], kArcherSoldier_Draw_Flags[j] | 0x20, 0);
  }
}

void TutorialSoldier_Draw(int k) {  // 85d64b
  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  OamEnt *oam = GetOamCurPtr();
  int d = sprite_graphics[k] * 5;
  for (int i = 4; i >= 0; i--, oam++) {
    int j = d + i;
    uint8 flags = kTutorialSoldier_Flags[j] | info.flags;
    if (kTutorialSoldier_Char[j] < 0x40)
      flags = (flags & 0xf1) | 8;
    SetOamHelper0(oam, info.x + kTutorialSoldier_X[j], info.y + kTutorialSoldier_Y[j], kTutorialSoldier_Char[j], flags, kTutorialSoldier_Big[j]);
  }
  SpriteDraw_Shadow_custom(k, &info, 12);
}

void PullSwitch_FacingUp(int k) {  // 85d6d4
  static const uint8 kBadPullSwitch_Tab1[10] = {8, 24, 4, 4, 4, 4, 4, 4, 2, 10};
  static const uint8 kBadPullSwitch_Tab0[10] = {6, 7, 8, 8, 8, 8, 8, 9, 9, 9};
  PullSwitch_HandleUpPulling(k);
  int j = sprite_graphics[k];
  if (j != 0 && j != 11) {
    link_unk_master_sword = kBadPullSwitch_Tab0[j - 1];
    link_y_coord = Sprite_GetY(k) - 19;
    link_x_coord = Sprite_GetX(k);
    if (sprite_delay_main[k] == 0) {
      sprite_graphics[k] = ++j;
      if (j == 11) {
        sound_effect_2 = 0x1b;
        dung_flag_statechange_waterpuzzle = 1;
      }
      sprite_delay_main[k] = kBadPullSwitch_Tab1[j - 2];
    }
  }
  if (sprite_type[k] != 7)
    BadPullDownSwitch_Draw(k);
  else
    BadPullUpSwitch_Draw(k);
}

void PullSwitch_HandleUpPulling(int k) {  // 85d743
  if (!Sprite_CheckDamageToLink_same_layer(k))
    return;
  link_actual_vel_y = 0;
  link_actual_vel_x = 0;
  Sprite_RepelDash();
  bitmask_of_dragstate = 0;
  uint8 y = link_y_coord - sprite_y_lo[k];
  if (!sign8(y - 2)) {
    link_y_coord = Sprite_GetY(k) + 9;
  } else if (sign8(y - 244)) {
    byte_7E0379++;
    if (joypad1L_last & 0x80 && !(joypad1H_last & 3) && sprite_graphics[k] == 0) {
      sprite_graphics[k] = 1;
      sprite_delay_main[k] = 8;
      SpriteSfx_QueueSfx2WithPan(k, 0x22);
    }
    link_y_coord = Sprite_GetY(k) - 21;
  } else {
    if (sign8(link_x_coord - sprite_x_lo[k]))
      link_x_coord = Sprite_GetX(k) - 16;
    else
      link_x_coord = Sprite_GetX(k) + 14;
  }
}

void BadPullDownSwitch_Draw(int k) {  // 85d7f9
  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  Oam_AllocateDeferToPlayer(k);
  OamEnt *oam = GetOamCurPtr();
  uint8 yoff = kBadPullSwitch_Tab5[kBadPullSwitch_Tab4[sprite_graphics[k]]];
  for (int i = 4; i >= 0; i--, oam++) {
    SetOamPlain(oam, info.x + kBadPullDownSwitch_X[i], info.y + kBadPullDownSwitch_Y[i] - (i == 2 ? yoff : 0),
                 kBadPullDownSwitch_Char[i], kBadPullDownSwitch_Flags[i] | 0x21, kBadPullDownSwitch_Big[i]);
  }
  Sprite_CorrectOamEntries(k, 4, 0xff);
}

void BadPullUpSwitch_Draw(int k) {  // 85d858
  static const uint8 kBadPullUpSwitch_Tab2[2] = {0xa2, 0xa4};
  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  Oam_AllocateDeferToPlayer(k);
  OamEnt *oam = GetOamCurPtr();
  uint8 yoff = kBadPullSwitch_Tab5[kBadPullSwitch_Tab4[sprite_graphics[k]]];
  for (int i = 1; i >= 0; i--, oam++) {
    SetOamHelper0(oam, info.x, info.y - ((i == 0) ? yoff : 0), kBadPullUpSwitch_Tab2[i], info.flags, 2);
  }
}

void PullSwitch_FacingDown(int k) {  // 85d8b5
  static const uint8 kGoodPullSwitch_Tab1[12] = {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5};
  static const uint8 kGoodPullSwitch_Tab0[12] = {1, 1, 2, 2, 3, 3, 1, 1, 4, 4, 5, 5};
  static const uint8 kGoodPullSwitch_YOffs[12] = {9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14};
  PullSwitch_HandleDownPulling(k);
  int j = sprite_graphics[k];
  if (j != 0 && j != 13) {
    link_unk_master_sword = kGoodPullSwitch_Tab0[j - 1];
    link_y_coord = Sprite_GetY(k) + kGoodPullSwitch_YOffs[j - 1];
    link_x_coord = Sprite_GetX(k);
    if (sprite_delay_main[k] == 0) {
      sprite_graphics[k] = ++j;
      if (j == 13) {
        if (sprite_type[k] == 6) {
          activate_bomb_trap_overlord = 1;
          sound_effect_1 = 0x3c;
        } else {
          dung_flag_statechange_waterpuzzle = 1;
          sound_effect_2 = 0x1b;
        }
      }
      sprite_delay_main[k] = kGoodPullSwitch_Tab1[j - 2];
    }
  }
  GoodPullSwitch_Draw(k);
  if (sprite_pause[k])
    sprite_graphics[k] = 0;
}

void GoodPullSwitch_Draw(int k) {  // 85d953
  static const uint8 kGoodPullSwitch_Tab2[14] = {1, 1, 2, 3, 2, 3, 4, 5, 6, 7, 6, 7, 7, 7};
  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  Oam_AllocateDeferToPlayer(k);
  OamEnt *oam = GetOamCurPtr();
  uint8 t = kGoodPullSwitch_Tab2[sprite_graphics[k]];
  oam[0].x = oam[1].x = info.x;
  oam[0].y = info.y - 1;
  oam[1].y = info.y - 1 + t;
  oam[0].charnum = 0xee;
  oam[1].charnum = 0xce;
  oam[0].flags = oam[1].flags = info.flags;
  Sprite_CorrectOamEntries(k, 1, 2);
}

void PullSwitch_HandleDownPulling(int k) {  // 85d999
  if (!Sprite_CheckDamageToLink_same_layer(k))
    return;
  link_actual_vel_y = 0;
  link_actual_vel_x = 0;
  Sprite_RepelDash();
  bitmask_of_dragstate = 0;
  uint8 y = link_y_coord - sprite_y_lo[k];
  if (!sign8(y - 2)) {
    byte_7E0379++;
    if (joypad1L_last & 0x80 && !(joypad1H_last & 3)) {
      link_unk_master_sword++;
      if ((joypad1H_last & 4) && sprite_graphics[k] == 0) {
        sprite_graphics[k] = 1;
        sprite_delay_main[k] = 12;
        SpriteSfx_QueueSfx2WithPan(k, 0x22);
      }
    }
    link_y_coord = Sprite_GetY(k) + 9;
  } else if (sign8(y - 244)) {
    link_y_coord = Sprite_GetY(k) - 21;
  } else {
    if (sign8(link_x_coord - sprite_x_lo[k]))
      link_x_coord = Sprite_GetX(k) - 16;
    else
      link_x_coord = Sprite_GetX(k) + 14;
  }
}

void Priest_SpawnMantle(int k) {  // 85db27
  sprite_state[15]++;
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0x73, &info);
  sprite_state[15] = 0;
  sprite_flags2[j] = sprite_flags2[j] & 0xf0 | 0x3;
  sprite_x_lo[j] = 0xF0;
  sprite_x_hi[j] = 4;
  sprite_y_lo[j] = 0x37;
  sprite_y_hi[j] = 2;
  sprite_E[j] = 2;
  sprite_flags4[j] = 11;
  sprite_defl_bits[j] |= 0x20;
  sprite_subtype2[j] = 1;
  if (link_y_coord < Sprite_GetY(j))
    sprite_C[j] = 1;
}

void Sprite_SanctuaryMantle(int k) {  // 85db9b
  SageMantle_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;

  if (sprite_C[k]) {
    sprite_A[k] = 0x40;
    goto lbl2;
  }
  if (Sprite_CheckDamageToLink_same_layer(k)) {
    Sprite_NullifyHookshotDrag();
    link_speed_setting = 0;
    Sprite_RepelDash();
    sprite_delay_aux1[k] = 7;
lbl:
    sprite_subtype2[k] = 0;
    bitmask_of_dragstate = 0x81;
    link_speed_setting = 8;
lbl2:
    switch (sprite_ai_state[k]) {
    case 0: {
      uint16 x = Sprite_GetX(k);
      Sprite_SetX(k, x + 19);
      uint8 dir = Sprite_DirectionToFaceLink(k, NULL);
      Sprite_SetX(k, x);
      if (dir == 1 || dir == 3) {
        sprite_A[k]++;
        if (sprite_A[k] >= 64) {
          sprite_ai_state[k]++;
          flag_is_link_immobilized = 1;
        }
      }
      break;
    }
    case 1:
      SpriteSfx_QueueSfx3WithPan(k, 24);
      sprite_ai_state[k]++;
      sprite_delay_main[k] = 168;
      sprite_x_vel[k] = 3;
      sprite_delay_aux1[k] = 2;
      break;
    case 2:
      Sprite_MoveXY(k);
      if (sprite_delay_main[k] == 0) {
        flag_is_link_immobilized = 0;
        sprite_x_vel[k] = 0;
        sprite_C[k] = 0;
      } else {
        sprite_delay_aux1[k] = 2;
      }
      break;
    }
  } else {  // no collision
    if (sprite_delay_aux1[k])
      goto lbl;
    switch (sprite_subtype2[k]) {
    case 0:
      sprite_A[k] = 0;
      bitmask_of_dragstate = 0;
      link_speed_setting = 0;
      sprite_subtype2[k]++;
      break;
    case 1:
      break;
    }
  }
}

void SageMantle_Draw(int k) {  // 85dc8a
  if (sprite_C[k] == 0)
    Oam_AllocateFromRegionB(0x10);
  Sprite_DrawMultiple(k, kSageMantle_Dmd, 4, NULL);
}

void Sprite_Priest(int k) {  // 85dce6
  if (sprite_A[k] == 0)
    Priest_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_BehaveAsBarrier(k);
  if (Sprite_TrackBodyToHead(k))
    Sprite_MoveXY(k);
  switch (sprite_subtype2[k]) {
  case 0: Priest_Dying(k); break;
  case 1: Priest_RunRescueCutscene(k); break;
  case 2: Priest_Chillin(k); break;
  }
}

void Priest_Dying(int k) {  // 85dd0a
  sprite_head_dir[k] = 4;
  sprite_D[k] = 4;
  switch (sprite_ai_state[k]) {
  case 0:  // Priest_LyingOnGround
    if (Sprite_ShowSolicitedMessage(k, 0x1b) & 0x100) {
      sprite_ai_state[k]++;
      sprite_graphics[k]++;
      sram_progress_flags |= 0x2;
      sprite_delay_aux2[k] = 128;
    }
    break;
  case 1:  // Priest_FinalWords
    sprite_graphics[k] = 0;
    if (sprite_delay_aux2[k] == 0)
      sprite_ai_state[k]++;
    sprite_A[k] = frame_counter & 2;
    if (!(sprite_delay_aux2[k] & 7))
      SpriteSfx_QueueSfx2WithPan(k, 0x33);
    break;
  case 2:  // Priest_Die
    sprite_state[k] = 0;
    break;
  }
}

void Priest_RunRescueCutscene(int k) {  // 85dd63
  int j;
  switch (sprite_ai_state[k]) {
  case 0:
    sprite_head_dir[k] = 0;
    sprite_D[k] = 0;
    if (sprite_delay_main[k] == 0) {
      Sprite_ShowMessageUnconditional(0x17);
      sprite_ai_state[k]++;
      byte_7FFE01 = 1;
      Priest_SpawnRescuedPrincess();
      flag_is_link_immobilized = 1;
      savegame_map_icons_indicator = 1;
    }
    break;
  case 1:
    if (byte_7FFE01 == 2) {
      Sprite_ShowMessageUnconditional(0x18);
      sprite_ai_state[k]++;
    }
    break;
  case 2:
    if (choice_in_multiselect_box == 0) {
      sprite_ai_state[k]++;
      flag_is_link_immobilized = 0;
    } else {
      sprite_ai_state[k] = 1;
    }
    break;
  case 3:
    sprite_head_dir[k] = Sprite_DirectionToFaceLink(k, NULL) ^ 3;
    j = Sprite_ShowSolicitedMessage(k, 0x16);
    if (j & 0x100)
      sprite_D[k] = sprite_head_dir[k] = (uint8)j;
    break;
  }
}

void Priest_Chillin(int k) {  // 85dde5
  sprite_head_dir[k] = Sprite_DirectionToFaceLink(k, NULL) ^ 3;
  int m = (link_which_pendants & 7) == 7 ? 0x1a :
          savegame_map_icons_indicator >= 3 ? 0x19 : 0x16;
  int j = Sprite_ShowSolicitedMessage(k, m);
  if (j & 0x100) {
    sprite_D[k] = sprite_head_dir[k] = (uint8)j;
    link_hearts_filler = 0xa0;
  }
}

void Sprite_Uncle(int k) {  // 85de2c
  Uncle_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (sprite_subtype2[k] == 0)
    Uncle_AtHouse(k);
  else
    Uncle_InPassage(k);
}

void Uncle_AtHouse(int k) {  // 85de3e
  Sprite_MoveXY(k);
  switch (sprite_ai_state[k]) {
  case 0:  // Uncle_TriggerTelepathy
    link_x_coord_prev = 0x940;
    link_y_coord_prev = 0x215a;
    Sprite_ShowMessageUnconditional(0x1f);
    sprite_ai_state[k]++;
    break;
  case 1:  // Uncle_AwakenLink
    if (frame_counter & 3)
      break;
    if (COLDATA_copy0 != 32) {
      COLDATA_copy0--;
      COLDATA_copy1--;
      break;
    }
    link_pose_during_opening++;
    player_sleep_in_bed_state++;
    link_y_coord = 0x2157;
    flag_is_link_immobilized = 1;
    sprite_ai_state[k]++;
    break;
  case 2:  // Uncle_DeclareCurfew
    Sprite_ShowMessageUnconditional(0x0d);
    music_control = 3;
    sprite_graphics[k] = 1;
    sprite_ai_state[k]++;
    break;
  case 3:  // Uncle_Embark
    sprite_graphics[k] = frame_counter >> 3 & 1;
    if (!sprite_delay_main[k]) {
      int j = sprite_A[k];
      if (j == 2) {
        sprite_ai_state[k]++;
      } else {
        sprite_A[k]++;
        if (!j)
          sprite_y_lo[k] -= 2;
        sprite_delay_main[k] = kUncle_LeaveHouse_Delay[j];
        sprite_D[k] = j = kUncle_LeaveHouse_Dir[j];
        sprite_x_vel[k] = kUncle_LeaveHouse_Xvel[j];
        sprite_y_vel[k] = kUncle_LeaveHouse_Yvel[j];
      }
    }
    break;
  case 4:  // Uncle_ApplyTelepathyFollower
    follower_indicator = 5;
    word_7E02CD = 0xdf3;
    sram_progress_flags |= 0x10;
    sprite_state[k] = 0;
    flag_is_link_immobilized = 0;
    break;
  }
}

void Uncle_InPassage(int k) {  // 85df19
  switch (sprite_ai_state[k]) {
  case 0:  // RemoveZeldaTelepathTagalong
    if (Sprite_CheckDamageToLink_same_layer(k))
      Link_CancelDash();
    if (Sprite_ShowMessageOnContact(k, 0xe) & 0x100) {
      follower_indicator = 0;
      sprite_ai_state[k]++;
    }
    break;
  case 1:  // GiveSwordAndShield
    item_receipt_method = 0;
    Link_ReceiveItem(0, 0);
    sprite_ai_state[k]++;
    sprite_graphics[k] = 1;
    which_starting_point = 3;
    sram_progress_flags |= 1;
    sram_progress_indicator = 1;
    break;
  }
}

void Sprite_QuarrelBros(int k) {  // 85e013
  QuarrelBros_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_TrackBodyToHead(k);
  sprite_head_dir[k] = Sprite_DirectionToFaceLink(k, NULL) ^ 3;
  if (!(dungeon_room_index & 1)) {
    Sprite_ShowSolicitedMessage(k, 0x131);
  } else if (!(dung_door_opened & 0xff00)) {
    Sprite_ShowSolicitedMessage(k, 0x12f);
  } else {
    Sprite_ShowSolicitedMessage(k, 0x130);
  }
  Sprite_BehaveAsBarrier(k);
}

void QuarrelBros_Draw(int k) {  // 85e17f
  static const DrawMultipleData kQuarrelBros_Dmd[16] = {
    {0, -12, 0x0004, 2},
    {0,   0, 0x000a, 2},
    {0, -11, 0x0004, 2},
    {0,   1, 0x400a, 2},
    {0, -12, 0x0004, 2},
    {0,   0, 0x000a, 2},
    {0, -11, 0x0004, 2},
    {0,   1, 0x400a, 2},
    {0, -12, 0x0008, 2},
    {0,   0, 0x000a, 2},
    {0, -11, 0x0008, 2},
    {0,   1, 0x400a, 2},
    {0, -12, 0x4008, 2},
    {0,   0, 0x000a, 2},
    {0, -11, 0x4008, 2},
    {0,   1, 0x400a, 2},
  };
  PrepOamCoordsRet info;
  Sprite_DrawMultiplePlayerDeferred(k, &kQuarrelBros_Dmd[sprite_graphics[k] * 2 + sprite_D[k] * 4], 2, &info);
  SpriteDraw_Shadow(k, &info);

}

void Sprite_YoungSnitchLady(int k) {  // 85e2f2
  Sprite_OldSnitchLady(k);
}

void YoungSnitchLady_Draw(int k) {  // 85e37f
  static const DrawMultipleData kYoungSnitchLady_Dmd[16] = {
    {0, -8, 0x0026, 2},
    {0,  0, 0x00e8, 2},
    {0, -7, 0x0026, 2},
    {0,  1, 0x40e8, 2},
    {0, -8, 0x0024, 2},
    {0,  0, 0x00c2, 2},
    {0, -7, 0x0024, 2},
    {0,  1, 0x40c2, 2},
    {0, -8, 0x0028, 2},
    {0,  0, 0x00e4, 2},
    {0, -7, 0x0028, 2},
    {0,  1, 0x00e6, 2},
    {0, -8, 0x4028, 2},
    {0,  0, 0x40e4, 2},
    {0, -7, 0x4028, 2},
    {0,  1, 0x40e6, 2},
  };
  PrepOamCoordsRet info;
  Sprite_DrawMultiplePlayerDeferred(k, &kYoungSnitchLady_Dmd[sprite_graphics[k] * 2 + sprite_D[k] * 4], 2, &info);
  SpriteDraw_Shadow(k, &info);
}

void Sprite_InnKeeper(int k) {  // 85e3af
  InnKeeper_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_BehaveAsBarrier(k);
  Sprite_ShowSolicitedMessage(k, link_item_flippers ? 0x183 : 0x182);
}

void InnKeeper_Draw(int k) {  // 85e3dc
  static const DrawMultipleData kInnKeeper_Dmd[2] = {
    {0, -8, 0x00c4, 2},
    {0,  0, 0x00ca, 2},
  };
  PrepOamCoordsRet info;
  Sprite_DrawMultiplePlayerDeferred(k, kInnKeeper_Dmd, 2, &info);
  SpriteDraw_Shadow(k, &info);
}

void Sprite_Witch(int k) {  // 85e3fb
  Witch_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  uint8 bak0 = sprite_flags4[k];
  sprite_flags4[k] = 2;
  if (Sprite_CheckDamageToLink_same_layer(k)) {
    Sprite_NullifyHookshotDrag();
    link_speed_setting = 0;
    Link_CancelDash();
  }
  sprite_flags4[k] = bak0;
  if (!frame_counter)
    sprite_A[k] = (GetRandomNumber() & 1) + 2;
  int shift = sprite_A[k] + 1;
  sprite_graphics[k] = (frame_counter >> shift) & 7;
  if (Sprite_CheckIfLinkIsBusy())
    return;
  switch (sprite_ai_state[k]) {
  case 0:  // main
    if (link_item_mushroom == 0) {
      if (save_dung_info[0x109] & 0x80)
        Sprite_ShowSolicitedMessage(k, 0x4b);
      else
        Sprite_ShowSolicitedMessage(k, 0x4a);
    } else if (link_item_mushroom == 1) {
      if (!(joypad1H_last & 0x40)) {
        Sprite_ShowSolicitedMessage(k, 0x4c);
      } else if (Sprite_CheckDamageToLink_same_layer(k) && hud_cur_item == kHudItem_Mushroom) {
        Witch_AcceptShroom(k);
      }
    } else {
      Sprite_ShowSolicitedMessage(k, 0x4a);
    }
    break;
  case 1:  // grant cane of byrna
    sprite_ai_state[k] = 0;
    item_receipt_method = 0;
    Link_ReceiveItem(0x18, 0);
    break;
  }
}

void Witch_AcceptShroom(int k) {  // 85e4cf
  link_item_mushroom = 0;
  save_dung_info[0x109] |= 0x80;
  sound_effect_1 = 0;
  Hud_RefreshIcon();
  Sprite_ShowMessageUnconditional(0x4b);
  SpriteSfx_QueueSfx1WithPan(k, 0xd);
  flag_overworld_area_did_change = 0;
}

void Witch_Draw(int k) {  // 85e55d
  static const OamEntSigned kWitch_DrawDataA[16] = {
    {-3,  8, 0xae, 0x00},
    {-3, 16, 0xbe, 0x00},
    {-2,  8, 0xae, 0x00},
    {-2, 16, 0xbe, 0x00},
    {-1,  8, 0xaf, 0x00},
    {-1, 16, 0xbf, 0x00},
    { 0,  9, 0xaf, 0x00},
    { 0, 17, 0xbf, 0x00},
    { 1, 10, 0xaf, 0x00},
    { 1, 18, 0xbf, 0x00},
    { 0, 11, 0xaf, 0x00},
    { 0, 18, 0xbf, 0x00},
    {-1, 10, 0xae, 0x00},
    {-1, 18, 0xbe, 0x00},
    {-3,  9, 0xae, 0x00},
    {-3, 17, 0xbe, 0x00},
  };
  static const OamEntSigned kWitch_DrawDataB[3] = {
      {  0, -4, 0x80, 0x00},
      {-11, 15, 0x86, 0x04},
      { -3, 15, 0x86, 0x44},
  };
  static const OamEntSigned kWitch_DrawDataC[2] = {
      {0, 4, 0x84, 0x00},
      {0, 4, 0x82, 0x00},
  };

  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  Oam_AllocateDeferToPlayer(k);
  OamEnt *oam = GetOamCurPtr();
  int g = sprite_graphics[k] * 2;
  uint8 x = BYTE(dungmap_var7);
  uint8 y = HIBYTE(dungmap_var7);

  SetOamPlain(oam + 0, x + kWitch_DrawDataA[g + 0].x, y + kWitch_DrawDataA[g + 0].y, info.r4 | kWitch_DrawDataA[g + 0].charnum, info.flags, 0);
  SetOamPlain(oam + 1, x + kWitch_DrawDataA[g + 1].x, y + kWitch_DrawDataA[g + 1].y, info.r4 | kWitch_DrawDataA[g + 1].charnum, info.flags, 0);

  for (int i = 0; i < 3; i++) {
    SetOamPlain(&oam[i + 2], x + kWitch_DrawDataB[i].x, y + kWitch_DrawDataB[i].y,
                info.r4 ^ kWitch_DrawDataB[i].charnum,
                info.flags ^ kWitch_DrawDataB[i].flags, 2);
  }
  int i = (uint16)(g - 6) < 6;
  SetOamPlain(&oam[5], x + kWitch_DrawDataC[i].x, y + kWitch_DrawDataC[i].y,
              info.r4 | kWitch_DrawDataC[i].charnum,
              info.flags, 2);
  Sprite_CorrectOamEntries(k, 5, 0xff);
}

void SpritePrep_Snitches(int k) {  // 85e67d
  sprite_D[k] = 2;
  sprite_head_dir[k] = 2;
  sprite_ignore_projectile[k]++;
  sprite_A[k] = sprite_x_lo[k];
  sprite_B[k] = sprite_x_hi[k];
  sprite_x_vel[k] = -9;
}

void Sprite_OldSnitchLady(int k) {  // 85e6aa
  static const int8 kOldSnitchLady_Xd[2] = {-32, 32};
  static const int8 kOldSnitchLady_Xvel[4] = {0, 0, -9, 9};
  static const int8 kOldSnitchLady_Yvel[4] = {-9, 9, 0, 0};

  int j;

  if (sprite_type[k] == 0x34) {
    if (sprite_ai_state[k] < 2)
      YoungSnitchLady_Draw(k);
  } else {
    if (sprite_subtype[k]) {
      Sprite_ChickenLady(k);
      return;
    }
    if (sprite_ai_state[k] < 3)
      Lady_Draw(k);
  }

  if (Sprite_ReturnIfInactive(k))
    return;
  if (sprite_ai_state[k] < 3) {
    if (player_is_indoors) {
      Sprite_TrackBodyToHead(k);
      sprite_head_dir[k] = Sprite_DirectionToFaceLink(k, NULL) ^ 3;
      Sprite_ShowSolicitedMessage(k, 0xad);
      return;
    }
    if (!sprite_ai_state[k] && Sprite_CheckDamageToLink_same_layer(k)) {
      sprite_D[k] = Sprite_DirectionToFaceLink(k, NULL) ^ 3;
      sprite_delay_main[k] = 1;
    } else {
      if (Sprite_TrackBodyToHead(k))
        Sprite_MoveXY(k);
      else
        sprite_delay_main[k] = 1;
    }
  }

  switch (sprite_ai_state[k]) {
  case 0: {
    if (sprite_delay_main[k] == 0) {
      uint16 t = (sprite_A[k] | sprite_B[k] << 8) + kOldSnitchLady_Xd[sprite_C[k]];
      if (t == Sprite_GetX(k)) {
        sprite_head_dir[k] = (j = sprite_D[k] ^ 1);
        sprite_x_vel[k] = kOldSnitchLady_Xvel[j];
        sprite_y_vel[k] = kOldSnitchLady_Yvel[j];
        sprite_C[k] ^= 1;
      }
    }
    sprite_graphics[k] = (k ^ frame_counter) >> 4 & 1;
    uint8 bak0 = sprite_flags4[k];
    sprite_flags4[k] = 3;
    j = Sprite_ShowMessageOnContact(k, 0x2f);
    sprite_flags4[k] = bak0;
    if (j & 0x100) {
      sprite_D[k] = j;
      Snitch_SpawnGuard(k);
      sprite_ai_state[k] = 1;
    }
    break;
  }
  case 1: {
    uint16 ovx = overlord_x_lo[byte_7E0FDE] | overlord_x_hi[byte_7E0FDE] << 8;
    uint16 ovy = overlord_y_lo[byte_7E0FDE] | overlord_y_hi[byte_7E0FDE] << 8;
    if (ovy >= Sprite_GetY(k)) {
      sprite_ai_state[k] = 2;
      sprite_x_vel[k] = 0;
      sprite_y_vel[k] = 0;
      sprite_flags4[k] = 2;
      uint16 pos = ((ovy - overworld_offset_base_y) & overworld_offset_mask_y) * 8;
      pos += (((ovx >> 3) - overworld_offset_base_x) & overworld_offset_mask_x);
      Overworld_DrawWoodenDoor(pos, false);
      sprite_delay_main[k] = 16;
    } else {
      flag_is_link_immobilized = 1;
      ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLocation(k, ovx, ovy, 64);
      sprite_x_vel[k] = pt.x;
      sprite_y_vel[k] = pt.y;
      sprite_D[k] = 0;
      sprite_head_dir[k] = 0;
      sprite_graphics[k] = (k ^ frame_counter) >> 3 & 1;
    }
    break;
  }
  case 2:
    if (sprite_delay_main[k] == 0) {
      uint16 ovx = overlord_x_lo[byte_7E0FDE] | overlord_x_hi[byte_7E0FDE] << 8;
      uint16 ovy = overlord_y_lo[byte_7E0FDE] | overlord_y_hi[byte_7E0FDE] << 8;
      Sprite_SetX(k, ovx);
      Sprite_SetY(k, ovy);
      uint16 pos = ((ovy - overworld_offset_base_y) & overworld_offset_mask_y) * 8;
      pos += (((ovx >> 3) - overworld_offset_base_x) & overworld_offset_mask_x);
      Overworld_DrawWoodenDoor(pos, true);
      sprite_ai_state[k] = 3;
    }
    Sprite_MoveXY(k);
    break;
  case 3:
    sprite_state[k] = 0;
    flag_is_link_immobilized = 0;
    break;
  }
}

void SpritePrep_RunningMan(int k) {  // 85e896
  sprite_head_dir[k] = 2;
  sprite_D[k] = 2;
  sprite_ignore_projectile[k]++;
}

void Sprite_RunningMan(int k) {  // 85e8b2
  static const int8 kRunningMan_Xvel2[2] = {-24, 24};
  static const int8 kRunningMan_Xvel[4] = {0, 0, -54, 54};
  static const int8 kRunningMan_Yvel[4] = {-54, 54, 0, 0};
  static const int8 kRunningMan_Dir[4] = {3, 1, 3, -1};
  static const uint8 kRunningMan_A[4] = {120, 24, 128, 3};
  int j;
  RunningMan_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_TrackBodyToHead(k);
  Sprite_BehaveAsBarrier(k);
  sprite_subtype[k] = 255;
  Sprite_CheckTileCollision(k);
  uint8 bak0 = sprite_flags4[k];
  sprite_flags4[k] = 7;
  if (Sprite_CheckDamageToLink_same_layer(k)) {
    sprite_C[k] = sprite_ai_state[k];
    sprite_ai_state[k] = 3;
  }
  sprite_flags4[k] = bak0;
  switch (sprite_ai_state[k]) {
  case 0:  // chill
    Sprite_TrackBodyToHead(k);
    j = Sprite_DirectionToFaceLink(k, NULL);
    sprite_head_dir[k] = j ^ 3;
    if (Sprite_CheckDamageToLink_same_layer(k)) {
      Link_CancelDash();
      sprite_D[k] = j ^ 3;
      sprite_head_dir[k] = j | 2;
      sprite_ai_state[k] = (j & 1) + 1;
      sprite_x_vel[k] = kRunningMan_Xvel2[j & 1];
      sprite_delay_main[k] = 32;
    } else {
      sprite_x_vel[k] = 0;
      sprite_y_vel[k] = 0;
    }
    break;
  case 1:  // run left
  case 2:  // run right
    if (sprite_delay_main[k] != 0) {
      sprite_graphics[k] = frame_counter >> 3 & 1;
      Sprite_MoveXY(k);
    } else {
      RunningBoy_SpawnDustGarnish(k);
      sprite_graphics[k] = frame_counter >> 2 & 1;
      j = sprite_head_dir[k];
      sprite_x_vel[k] = kRunningMan_Xvel[j];
      sprite_y_vel[k] = kRunningMan_Yvel[j];
      Sprite_MoveXY(k);
      if (sprite_A[k]) {
        sprite_A[k]--;
        break;
      }
      if (sprite_ai_state[k] == 1) {  // left
        sprite_A[k] = 255;
        sprite_head_dir[k] = 2;
      } else {
        j = sprite_B[k]++;
        sprite_A[k] = kRunningMan_A[j];
        if (kRunningMan_Dir[j] < 0) {
          sprite_ai_state[k] = 0;
          sprite_subtype2[k] = 0;
        } else {
          sprite_head_dir[k] = kRunningMan_Dir[j];
        }
      }
    }
    break;
  case 3:  // caught
    Sprite_ShowMessageUnconditional(0xa6);
    if (link_player_handler_state >= kPlayerState_RecoilWall)  // wtf
      sprite_D[k] = link_player_handler_state;
    sprite_ai_state[k] = sprite_C[k];
    break;

  }
}

void RunningMan_Draw(int k) {  // 85ea4d
  static const DrawMultipleData kRunningMan_Dmd[16] = {
    {0, -8, 0x002c, 2},
    {0,  0, 0x08ee, 2},
    {0, -7, 0x002c, 2},
    {0,  1, 0x48ee, 2},
    {0, -8, 0x002a, 2},
    {0,  0, 0x08ca, 2},
    {0, -7, 0x002a, 2},
    {0,  1, 0x48ca, 2},
    {0, -8, 0x002e, 2},
    {0,  0, 0x08cc, 2},
    {0, -7, 0x002e, 2},
    {0,  1, 0x08ce, 2},
    {0, -8, 0x402e, 2},
    {0,  0, 0x48cc, 2},
    {0, -7, 0x402e, 2},
    {0,  1, 0x48ce, 2},
  };
  PrepOamCoordsRet info;
  Sprite_DrawMultiplePlayerDeferred(k, &kRunningMan_Dmd[(sprite_D[k] * 4 + sprite_graphics[k] * 2) & 0xf], 2, &info);
  SpriteDraw_Shadow(k, &info);
}

void Sprite_BottleVendor(int k) {  // 85ea79
  int j;

  sprite_A[k] = BottleVendor_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  BottleMerchant_DetectFish(k);
  Sprite_BehaveAsBarrier(k);
  if (Sprite_CheckIfLinkIsBusy())
    return;
  if (GetRandomNumber() == 0) {
    sprite_delay_main[k] = 20;
    sprite_graphics[k] = 1;
  } else if (!sprite_delay_main[k]) {
    sprite_graphics[k] = 0;
  }
  switch (sprite_ai_state[k]) {
  case 0:  // base
    if (!sprite_A[k] && sprite_E[k])
      sprite_ai_state[k] = 3;
    else if (sram_progress_indicator_3 & 2)
      Sprite_ShowSolicitedMessage(k, 0xd4);
    else if (Sprite_ShowSolicitedMessage(k, 0xd1) & 0x100)
      sprite_ai_state[k] = 1;
    break;
  case 1:  // selling
    if (choice_in_multiselect_box == 0 && link_rupees_goal >= 100) {
      Sprite_ShowMessageUnconditional(0xd2);
      sprite_ai_state[k] = 2;
    } else {
      Sprite_ShowMessageUnconditional(0xd3);
      sprite_ai_state[k] = 0;
    }
    break;
  case 2:  // giving
    item_receipt_method = 0;
    Link_ReceiveItem(0x16, 0);
    sram_progress_indicator_3 |= 2;
    link_rupees_goal -= 100;
    sprite_ai_state[k] = 0;
    break;
  case 3:  // buying
    if (!sign8(sprite_E[k]))
      Sprite_ShowMessageUnconditional(0xd5);
    else
      Sprite_ShowMessageUnconditional(0xd6);
    sprite_ai_state[k] = 4;
    break;
  case 4:  // reward
    j = sprite_E[k];
    if (!sign8(j)) {
      sprite_state[j - 1] = 0;
      BottleMerchant_BuyBee(k);
    } else {
      sprite_state[j & 0xf] = 0;
      BottleMerchant_BuyFish(k);
    }
    sprite_E[k] = 0;
    sprite_ai_state[k] = 0;
    break;
  }

}

uint8 BottleVendor_Draw(int k) {  // 85eba7
  PrepOamCoordsRet info;
  static const DrawMultipleData kBottleVendor_Dmd[4] = {
    {0, -7, 0x00ac, 2},
    {0,  0, 0x0088, 2},
    {0, -6, 0x00ac, 2},
    {0,  0, 0x00a2, 2},
  };
  Sprite_DrawMultiplePlayerDeferred(k, &kBottleVendor_Dmd[sprite_graphics[k] * 2], 2, &info);
  SpriteDraw_Shadow(k, &info);
  return (info.x | info.y) >> 8;
}

void Priest_SpawnRescuedPrincess() {  // 85ec4c
  SpriteSpawnInfo info;
  int k = Sprite_SpawnDynamically(0, 0x76, &info);
  if (k < 0)
    return;
  sprite_D[k] = sprite_head_dir[k] = tagalong_layerbits[tagalong_var2] & 3;
  Sprite_SetX(k, link_x_coord);
  Sprite_SetY(k, link_y_coord);
  sprite_subtype2[k] = 1;
  follower_indicator = 0;
  sprite_ignore_projectile[k]++;
  sprite_flags4[k] = 3;
}

void Sprite_76_Zelda(int k) {  // 85ec9e
  CrystalMaiden_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_BehaveAsBarrier(k);
  if (Sprite_TrackBodyToHead(k))
    Sprite_MoveXY(k);
  switch (sprite_subtype2[k]) {
  case 0: Zelda_InCell(k); break;
  case 1: Zelda_EnteringSanctuary(k); break;
  case 2: Zelda_AtSanctuary(k); break;
  }
}

void Zelda_InCell(int k) {  // 85ecbf
  int j;
  sprite_head_dir[k] = Sprite_DirectionToFaceLink(k, NULL) ^ 3;
  switch (sprite_ai_state[k]) {
  case 0:  // AwaitingRescue
    if (!Sprite_CheckDamageToLink_same_layer(k))
      return;
    sprite_ai_state[k]++;
    flag_is_link_immobilized++;
    j = sprite_head_dir[k];
    sprite_x_vel[k] = kZelda_Xvel[j];
    sprite_y_vel[k] = kZelda_Yvel[j];
    sprite_delay_main[k] = 16;
    break;
  case 1:  // ApproachingPlayer
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k]++;
      Sprite_ShowMessageUnconditional(0x1c);
      sprite_x_vel[k] = 0;
      sprite_y_vel[k] = 0;
      music_control = 25;
    }
    sprite_graphics[k] = frame_counter >> 3 & 1;
    break;
  case 2:  // TheWizardIsBadMkay
    sprite_ai_state[k]++;
    Sprite_ShowMessageUnconditional(0x25);
    break;
  case 3:  // WaitUntilPlayerPaysAttention
    if (choice_in_multiselect_box) {
      sprite_ai_state[k] = 2;
    } else {
      sprite_ai_state[k]++;
      Sprite_ShowMessageUnconditional(0x24);
    }
    break;
  case 4:  // TransitionToTagalong
    flag_is_link_immobilized = 0;
    which_starting_point = 2;
    SavePalaceDeaths();
    follower_indicator = 1;
    Dungeon_FlagRoomData_Quadrants();
    Sprite_BecomeFollower(k);
    sprite_state[k] = 0;
    music_control = 16;
    break;
  }
}

void Zelda_EnteringSanctuary(int k) {  // 85ed69
  static const uint8 kZelda_Delay0[4] = {38, 26, 44, 1};
  static const uint8 kZelda_Dir0[4] = {1, 3, 1, 2};
  int j;
  switch (sprite_ai_state[k]) {
  case 0:  // walk to priest
    if (sprite_delay_main[k] == 0) {
      j = sprite_A[k];
      if (j >= 4) {
        sprite_ai_state[k]++;
        sprite_head_dir[k] = sprite_D[k] = 0;
        sprite_x_vel[k] = 0;
        sprite_y_vel[k] = 0;
        return;
      }
      sprite_delay_main[k] = kZelda_Delay0[j];
      sprite_D[k] = sprite_head_dir[k] = j = kZelda_Dir0[j];
      sprite_A[k]++;
      sprite_x_vel[k] = kZelda_Xvel[j];
      sprite_y_vel[k] = kZelda_Yvel[j];
    }
    sprite_graphics[k] = (frame_counter >> 3) & 1;
    break;
  case 1:  // respond to priest
    Sprite_ShowMessageUnconditional(0x1d);
    sprite_ai_state[k]++;
    byte_7FFE01 = 2;
    which_starting_point = 1;
    SavePalaceDeaths();
    sram_progress_indicator = 2;
    Sprite_LoadGraphicsProperties_light_world_only();
    break;
  case 2:  // be careful
    sprite_head_dir[k] = Sprite_DirectionToFaceLink(k, NULL) ^ 3;
    j = Sprite_ShowSolicitedMessage(k, 0x1e);
    if (j & 0x100)
      sprite_D[k] = sprite_head_dir[k] = (uint8)j;
    break;
  }
}

void Zelda_AtSanctuary(int k) {  // 85ee0c
  sprite_head_dir[k] = Sprite_DirectionToFaceLink(k, NULL) ^ 3;
  int m = (link_which_pendants & 7) == 7 ? 0x27 :
          savegame_map_icons_indicator >= 3 ? 0x26 : 0x1e;
  int j = Sprite_ShowSolicitedMessage(k, m);
  if (j & 0x100) {
    sprite_D[k] = sprite_head_dir[k] = (uint8)j;
    link_hearts_filler = 0xa0;
  }
}

void SpritePrep_Mushroom(int k) {  // 85ee53
  if (link_item_mushroom >= 2) {
    sprite_state[k] = 0;
  } else {
    sprite_graphics[k] = 0;
    sprite_oam_flags[k] |= 8;
    sprite_ignore_projectile[k]++;
  }
}

void Sprite_E7_Mushroom(int k) {  // 85ee78
  SpriteDraw_SingleLarge(k);
  if (Sprite_CheckIfLinkIsBusy())
    return;

  // If we're in the middle of a mirror warp, don't get the mushroom yet
  if (enhanced_features0 & kFeatures0_MiscBugFixes && submodule_index != 0)
    return;

  if (Sprite_CheckDamageToLink_same_layer(k)) {
    sprite_state[k] = 0;
    item_receipt_method = 0;
    Link_ReceiveItem(0x29, 0);
  } else if ((frame_counter & 0x1f) == 0) {
    sprite_oam_flags[k] ^= 0x40;
  }
}

void Sprite_E8_FakeSword(int k) {  // 85eeaf
  FakeSword_Draw(k);
  if (Sprite_ReturnIfPaused(k))
    return;
  if (sprite_unk3[k] == 3) {
    if (!sprite_C[k]) {
      sprite_C[k] = 1;
      Sprite_ShowMessageUnconditional(0x6f);
    }
  } else {
    Sprite_MoveXY(k);
    ThrownSprite_TileAndSpriteInteraction(k);
  }
}

void FakeSword_Draw(int k) {  // 85eee6
  Sprite_DrawMultiplePlayerDeferred(k, kFakeSword_Dmd, 2, NULL);
}

void SpritePrep_HeartContainer(int k) {  // 85ef01
  HeartUpgrade_CheckIfAlreadyObtained(k);
}

void Sprite_HeartContainer(int k) {  // 85ef47
  if (BYTE(cur_palace_index_x2) == 26) {
    sprite_state[k] = 0;
    return;
  }
  sprite_ignore_projectile[k] = sprite_G[k];
  if (!sprite_G[k]) {
    DecodeAnimatedSpriteTile_variable(3);
    Sprite_Get16BitCoords(k);
    sprite_G[k] = 1;
  }

  if (BYTE(dungeon_room_index2) == 6 && !sprite_z[k])
    SpriteDraw_WaterRipple_WithOamAdjust(k);
  SpriteDraw_SingleLarge(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  sprite_z_vel[k] -= 2;
  Sprite_MoveZ(k);
  if (sign8(sprite_z[k])) {
    sprite_z[k] = 0;
    sprite_z_vel[k] = (uint8)-sprite_z_vel[k] >> 2;
    if (BYTE(dungeon_room_index2) == 6 && !sprite_subtype[k]) {
      sprite_flags2[k] += 2;
      sprite_subtype[k] = 1;
      Sprite_SpawnWaterSplash(k);
    }
  }
  if (Sprite_CheckIfLinkIsBusy())
    return;
  if (!Sprite_CheckDamageToLink_same_layer(k))
    return;
  sprite_state[k] = 0;
  if (sprite_A[k]) {
    item_receipt_method = 2;
    Link_ReceiveItem(0x3e, 0);
    dung_savegame_state_bits |= 0x8000;
    return;
  }
  Link_CancelDash();
  item_receipt_method = 0;
  Link_ReceiveItem(0x26, 0);
  if (!player_is_indoors)
    save_ow_event_info[BYTE(overworld_screen_index)] |= 0x40;
  else
    dung_savegame_state_bits |= (sprite_x_hi[k] & 1) ? 0x2000 : 0x4000;
}

void Sprite_HeartPiece(int k) {  // 85f020
  static const uint16 kHeartPieceMsg[4] = {0x158, 0x155, 0x156, 0x157};
  if (!sprite_ai_state[k]) {
    sprite_ai_state[k]++;
    HeartUpgrade_CheckIfAlreadyObtained(k);
    if (!sprite_state[k])
      return;
  }
  SpriteDraw_SingleLarge(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (Sprite_CheckIfLinkIsBusy())
    return;

  if (Sprite_CheckTileCollision(k) & 3)
    sprite_x_vel[k] = -sprite_x_vel[k];

  sprite_z_vel[k]--;
  Sprite_MoveZ(k);
  Sprite_MoveXY(k);
  if (sign8(sprite_z[k])) {
    sprite_z[k] = 0;
    sprite_z_vel[k] = ((sprite_z_vel[k] ^ 255) & 248) >> 1;
    sprite_x_vel[k] = (int8)sprite_x_vel[k] >> 1;
  }

  if (sprite_delay_aux4[k] || !Sprite_CheckDamageToLink_same_layer(k))
    return;

  link_heart_pieces = link_heart_pieces + 1 & 3;
  if (link_heart_pieces == 0) {
    Link_CancelDash();
    item_receipt_method = 0;
    Link_ReceiveItem(0x26, 0);
  } else {
    SpriteSfx_QueueSfx3WithPan(k, 0x2d);
    Sprite_ShowMessageUnconditional(kHeartPieceMsg[link_heart_pieces]);
  }
  sprite_state[k] = 0;
  HeartUpgrade_SetObtainedFlag(k);
}

void HeartUpgrade_SetObtainedFlag(int k) {  // 85f0c3
  if (!player_is_indoors) {
    save_ow_event_info[BYTE(overworld_screen_index)] |= 0x40;
  } else {
    int j = sprite_x_hi[k] & 1;
    dung_savegame_state_bits |= (j ? 0x2000 : 0x4000);
  }
}

void Sprite_Aginah(int k) {  // 85f0ea
  if (!(sram_progress_flags & 0x20))
    goto default_msg;
  if (link_sword_type >= 2) {
    Sprite_ShowSolicitedMessage(k, 0x128);
  } else if ((link_which_pendants & 7) == 7) {
    Sprite_ShowSolicitedMessage(k, 0x126);
  } else if ((link_which_pendants & 2) != 0) {
    Sprite_ShowSolicitedMessage(k, 0x129);
  } else if (link_item_book_of_mudora) {
    Sprite_ShowSolicitedMessage(k, 0x127);
  } else {
default_msg:
    sram_progress_flags |= 0x20;
    Sprite_ShowSolicitedMessage(k, 0x125);
  }
  sprite_graphics[k] = frame_counter >> 5 & 1;
}

void Sprite_Sahasrahla(int k) {  // 85f14d
  switch (sprite_ai_state[k]) {
  case 0:  // dialogue
    Sasha_Idle(k);
    break;
  case 1:  // mark map
    Sprite_ShowMessageUnconditional(0x33);
    sprite_ai_state[k] = 0;
    savegame_map_icons_indicator = 3;
    break;
  case 2:  // grant boots
    item_receipt_method = 0;
    Link_ReceiveItem(0x4b, 0);
    sprite_ai_state[k] = 3;
    savegame_map_icons_indicator = 3;
    break;
  case 3:  // shamelessly promote ice rod
    Sprite_ShowMessageUnconditional(0x37);
    sprite_ai_state[k] = 0;
    break;
  }
}

void Sasha_Idle(int k) {  // 85f160
  if (!(link_which_pendants & 4)) {
    if (Sprite_ShowSolicitedMessage(k, 0x32) & 0x100)
      sprite_ai_state[k] = 1;
  } else if (!link_item_boots) {
    int m = (savegame_map_icons_indicator >= 3) ? 0x38 : 0x39;
    if (Sprite_ShowSolicitedMessage(k, m) & 0x100)
      sprite_ai_state[k] = 2;
  } else if (!link_item_ice_rod) {
    Sprite_ShowSolicitedMessage(k, 0x37);
  } else if ((link_which_pendants & 7) != 7) {
    Sprite_ShowSolicitedMessage(k, 0x34);
  } else if (link_sword_type < 2) {
    Sprite_ShowSolicitedMessage(k, 0x30);
  } else {
    Sprite_ShowSolicitedMessage(k, 0x31);
  }
  sprite_graphics[k] = frame_counter >> 5 & 1;
}

void Elder_Draw(int k) {  // 85f23a
  static const DrawMultipleData kElder_Dmd[4] = {
    {0, -9, 0x00a0, 2},
    {0,  0, 0x00a2, 2},
    {0, -8, 0x00a0, 2},
    {0,  0, 0x40a4, 2},
  };
  PrepOamCoordsRet info;
  Sprite_DrawMultiplePlayerDeferred(k, &kElder_Dmd[sprite_graphics[k] * 2], 2, &info);
  SpriteDraw_Shadow(k, &info);
}

void Sprite_DustCloud(int k) {  // 85f2b2
  static const uint8 kDustCloud_Gfx[9] = {0, 1, 2, 3, 4, 5, 1, 0, 0xff};
  DustCloud_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (sprite_delay_main[k])
    return;
  sprite_delay_main[k] = 5;
  if (!sign8(kDustCloud_Gfx[sprite_A[k]])) {
    sprite_graphics[k] = kDustCloud_Gfx[sprite_A[k]];
    sprite_A[k]++;
  } else {
    sprite_state[k] = 0;
  }
}

int Sprite_SpawnDustCloud(int k) {  // 85f2d6
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0xF2, &info);
  if (j >= 0) {
    info.r2_y += (GetRandomNumber() & 15);
    info.r0_x += (GetRandomNumber() & 15) - 8;
    Sprite_SetSpawnedCoordinates(j, &info);
    sprite_subtype2[j] = 1;
  }
  return j;
}

void MedallionTablet_Main(int k) {  // 85f30c
  MedallionTablet_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  link_position_mode &= ~0x20;
  sprite_A[k] = 0;
  if (Sprite_CheckDamageToLink_same_layer(k)) {
    Sprite_NullifyHookshotDrag();
    link_speed_setting = 0;
    Sprite_RepelDash();
    sprite_A[k]++;
  }
  if (Sprite_CheckIfLinkIsBusy())
    return;
  switch (sprite_ai_state[k]) {
  case 0:  // wait for mudora
    if (BYTE(overworld_screen_index) != 3)
      BombosTablet(k);
    else
      EtherTablet(k);
    break;
  case 1:  // delay
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k]++;
      sprite_delay_main[k] = 128;
    }
    break;
  case 2:  // crumbling
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k]++;
      sprite_delay_main[k] = 240;
    } else {
      if (sprite_delay_main[k] == 0x20 || sprite_delay_main[k] == 0x40 || sprite_delay_main[k] == 0x60)
        sprite_graphics[k]++;
      if (!(frame_counter & 7))
        Sprite_SpawnDustCloud(k);
    }
    break;
  case 3:  // final animstate
    sprite_graphics[k] = 4;
    break;
  }
}

void BombosTablet(int k) {  // 85f355
  static const uint16 kMedallionTabletMsg[2] = {0x10d, 0x10f};
  if (link_direction_facing || Sprite_DirectionToFaceLink(k, NULL) != 2)
    return;
  if ((uint16)(cur_sprite_y + 16) < link_y_coord)
    return;
  if (filtered_joypad_H & 0x80 && link_sword_type == 2)
    return;

  int j = 1;
  if (!(hud_cur_item == kHudItem_BookMudora && (filtered_joypad_H & 0x40)) && (j = 0, !(filtered_joypad_L & 0x80)))
    return;
  if (j) {
    player_handler_timer = 0;
    link_position_mode = 32;
    sound_effect_1 = 0;
    if (!sign8(link_sword_type) && link_sword_type >= 2) {
      sprite_ai_state[k]++;
      BombosTablet_StartCutscene();
      sprite_delay_main[k] = 64;
    }
  }
  Sprite_ShowMessageUnconditional(kMedallionTabletMsg[j]);
}

void EtherTablet(int k) {  // 85f3c4
  static const uint16 kMedallionTabletEtherMsg[2] = {0x10d, 0x10e};
  if (link_direction_facing || Sprite_DirectionToFaceLink(k, NULL) != 2)
    return;
  if ((uint8)(sprite_y_lo[k] + 16) < BYTE(link_y_coord))
    return;
  if (filtered_joypad_H & 0x80 && link_sword_type == 2)
    return;

  int j = 1;
  if (!(hud_cur_item == kHudItem_BookMudora && (filtered_joypad_H & 0x40)) && (j = 0, !(filtered_joypad_L & 0x80)))
    return;

  if (j) {
    player_handler_timer = 0;
    link_position_mode = 32;
    sound_effect_1 = 0;
    if (!sign8(link_sword_type) && link_sword_type >= 2) {
      sprite_ai_state[k]++;
      EtherTablet_StartCutscene();
      sprite_delay_main[k] = 64;
    }
  }
  Sprite_ShowMessageUnconditional(kMedallionTabletEtherMsg[j]);
}

void ElderWife_Draw(int k) {  // 85f505
  static const DrawMultipleData kElderWife_Dmd[4] = {
    {0, -5, 0x008e, 2},
    {0,  5, 0x0028, 2},
    {0, -4, 0x008e, 2},
    {0,  5, 0x4028, 2},
  };
  Sprite_DrawMultiplePlayerDeferred(k, &kElderWife_Dmd[sprite_graphics[k] * 2], 2, NULL);
}

void SpritePrep_PotionShop(int k) {  // 85f529
  MagicShopAssistant_SpawnPowder(k);
  MagicShopAssistant_SpawnGreenCauldron(k);
  MagicShopAssistant_SpawnBlueCauldron(k);
  MagicShopAssistant_SpawnRedCauldron(k);
  sprite_ignore_projectile[k]++;
}

void MagicShopAssistant_SpawnPowder(int k) {  // 85f539
  if (!flag_overworld_area_did_change || link_item_mushroom == 2)
    return;
  if (save_dung_info[0x109] & 0x80) {
    SpriteSpawnInfo info;
    int j = Sprite_SpawnDynamically(k, 0xe9, &info);
    sprite_subtype2[j] = 1;
    Sprite_SetX(j, info.r0_x - 16);
    Sprite_SetY(j, info.r2_y);
    sprite_flags4[j] = 3;
    sprite_defl_bits[j] |= 0x20;
  }
}

void MagicShopAssistant_SpawnGreenCauldron(int k) {  // 85f58e
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0xe9, &info);
  sprite_subtype2[j] = 2;
  Sprite_SetX(j, info.r0_x - 40);
  Sprite_SetY(j, info.r2_y - 72);
  sprite_flags4[j] = 3;
  sprite_defl_bits[j] |= 0x20;
}

void MagicShopAssistant_SpawnBlueCauldron(int k) {  // 85f5bf
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0xe9, &info);
  sprite_subtype2[j] = 3;
  Sprite_SetX(j, info.r0_x + 8);
  Sprite_SetY(j, info.r2_y - 72);
  sprite_flags4[j] = 3;
  sprite_defl_bits[j] |= 0x20;
}

void MagicShopAssistant_SpawnRedCauldron(int k) {  // 85f5f0
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0xe9, &info);
  sprite_subtype2[j] = 4;
  Sprite_SetX(j, info.r0_x - 88);
  Sprite_SetY(j, info.r2_y - 72);
  sprite_flags4[j] = 3;
  sprite_defl_bits[j] |= 0x20;
}

void Sprite_E9_PotionShop(int k) {  // 85f633
  switch(sprite_subtype2[k]) {
  case 0: Sprite_MagicShopAssistant_Main(k); return;
  case 1: Sprite_BagOfPowder(k); return;
  case 2: Sprite_GreenCauldron(k); return;
  case 3: Sprite_BlueCauldron(k); return;
  case 4: Sprite_RedCauldron(k); return;
  }
}

void Sprite_BagOfPowder(int k) {  // 85f644
  MagicPowderItem_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_BehaveAsBarrier(k);
  if (!Sprite_CheckDamageToLink_same_layer(k) || !(filtered_joypad_L & 0x80))
    return;
  Link_CancelDash();
  item_receipt_method = 0;
  Link_ReceiveItem(0xd, 0);
  sprite_state[k] = 0;
}

void MagicPowderItem_Draw(int k) {  // 85f67b
  static const DrawMultipleData kMagicPowder_Dmd[2] = {
    {0, 0, 0x04e6, 2},
    {0, 0, 0x04e6, 2},
  };
  Sprite_DrawMultiplePlayerDeferred(k, kMagicPowder_Dmd, 2, NULL);
}

void Sprite_GreenCauldron(int k) {  // 85f68e
  GreenPotionItem_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_BehaveAsBarrier(k);
  if (sprite_delay_main[k])
    return;
  if (!PotionCauldron_CheckBottles()) {
    if (Sprite_ShowMessageOnContact(k, 0x4f) & 0x100)
      PotionCauldron_GoBeep(k);
    return;
  }
  if (!Sprite_CheckDamageToLink_same_layer(k) || !(filtered_joypad_L & 0x80))
    return;
  if (link_rupees_goal < 60) {
    Sprite_ShowMessageUnconditional(0x17c);
    PotionCauldron_GoBeep(k);
    return;
  }
  if (Sprite_Find_EmptyBottle() < 0) {
    Sprite_ShowMessageUnconditional(0x50);
    PotionCauldron_GoBeep(k);
    return;
  }
  SpriteSfx_QueueSfx3WithPan(k, 0x1d);
  sprite_delay_main[k] = 64;
  link_rupees_goal -= 60;
  item_receipt_method = 0;
  Link_ReceiveItem(0x2f, 0);
}

void GreenPotionItem_Draw(int k) {  // 85f718
  static const DrawMultipleData kGreenPotionItem_Dmd[3] = {
    { 0,  0, 0x08c0, 2},
    { 8, 18, 0x0a30, 0},
    {-1, 18, 0x0a22, 0},
  };
  Sprite_DrawMultiplePlayerDeferred(k, kGreenPotionItem_Dmd, 3, NULL);
}

void Sprite_BlueCauldron(int k) {  // 85f72b
  BluePotionItem_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_BehaveAsBarrier(k);
  if (sprite_delay_main[k])
    return;
  if (!PotionCauldron_CheckBottles()) {
    if (Sprite_ShowMessageOnContact(k, 0x4f) & 0x100)
      PotionCauldron_GoBeep(k);
    return;
  }
  if (!Sprite_CheckDamageToLink_same_layer(k) || !(filtered_joypad_L & 0x80))
    return;
  if (link_rupees_goal < 160) {
    Sprite_ShowMessageUnconditional(0x17c);
    PotionCauldron_GoBeep(k);
    return;
  }
  if (Sprite_Find_EmptyBottle() < 0) {
    Sprite_ShowMessageUnconditional(0x50);
    PotionCauldron_GoBeep(k);
    return;
  }
  SpriteSfx_QueueSfx3WithPan(k, 0x1d);
  sprite_delay_main[k] = 64;
  link_rupees_goal -= 160;
  item_receipt_method = 0;
  Link_ReceiveItem(0x30, 0);
}

void BluePotionItem_Draw(int k) {  // 85f7bd
  static const DrawMultipleData kBluePotionItem_Dmd[4] = {
    { 0,  0, 0x04c0, 2},
    {13, 18, 0x0a30, 0},
    { 5, 18, 0x0a22, 0},
    {-3, 18, 0x0a31, 0},
  };
  Sprite_DrawMultiplePlayerDeferred(k, kBluePotionItem_Dmd, 4, NULL);
}

void Sprite_RedCauldron(int k) {  // 85f7d0
  RedPotionItem_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_BehaveAsBarrier(k);
  if (sprite_delay_main[k])
    return;
  if (!PotionCauldron_CheckBottles()) {
    if (Sprite_ShowMessageOnContact(k, 0x4f) & 0x100)
      PotionCauldron_GoBeep(k);
    return;
  }
  if (!Sprite_CheckDamageToLink_same_layer(k) || !(filtered_joypad_L & 0x80))
    return;
  if (link_rupees_goal < 120) {
    Sprite_ShowMessageUnconditional(0x17c);
    PotionCauldron_GoBeep(k);
    return;
  }
  if (Sprite_Find_EmptyBottle() < 0) {
    Sprite_ShowMessageUnconditional(0x50);
    PotionCauldron_GoBeep(k);
    return;
  }
  SpriteSfx_QueueSfx3WithPan(k, 0x1d);
  sprite_delay_main[k] = 64;
  link_rupees_goal -= 120;
  item_receipt_method = 0;
  Link_ReceiveItem(0x2e, 0);
}

void PotionCauldron_GoBeep(int k) {  // 85f846
  SpriteSfx_QueueSfx2WithPan(k, 0x3c);
}

void RedPotionItem_Draw(int k) {  // 85f86d
  static const DrawMultipleData kRedPotionItem_Dmd[4] = {
    { 0,  0, 0x02c0, 2},
    {13, 18, 0x0a30, 0},
    { 5, 18, 0x0a02, 0},
    {-3, 18, 0x0a31, 0},
  };
  Sprite_DrawMultiplePlayerDeferred(k, kRedPotionItem_Dmd, 4, NULL);
}

bool PotionCauldron_CheckBottles() {  // 85f880
  return (link_bottle_info[0] | link_bottle_info[1] | link_bottle_info[2] | link_bottle_info[3]) >= 2;
}

void Sprite_MagicShopAssistant_Main(int k) {  // 85f893
  Shopkeeper_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_BehaveAsBarrier(k);
  if (Sprite_CheckIfLinkIsBusy())
    return;
  if (sprite_ai_state[k]) {
    link_hearts_filler = 160;
    sprite_ai_state[k] = 0;
  }
  sprite_graphics[k] = frame_counter >> 5 & 1;
  int msg;
  if (link_bottle_info[0] >= 2 || link_bottle_info[1] >= 2 || link_bottle_info[2] >= 2 || link_bottle_info[3] >= 2 || !flag_overworld_area_did_change)
    msg = 0x4e;
  else
    msg = 0x4d;
  if (Sprite_ShowSolicitedMessage(k, msg) & 0x100)
    sprite_ai_state[k] = 1;
}

void Shopkeeper_Draw(int k) {  // 85f91b
  static const DrawMultipleData kShopkeeper_Dmd[4] = {
    {0, -8, 0x0c00, 2},
    {0,  0, 0x0c10, 2},
    {0, -8, 0x0c00, 2},
    {0,  0, 0x4c10, 2},
  };
  PrepOamCoordsRet info;
  Sprite_DrawMultiplePlayerDeferred(k, &kShopkeeper_Dmd[sprite_graphics[k] * 2], 2, &info);
  SpriteDraw_Shadow(k, &info);
}

void Sprite_DashItem(int k) {  // 85fbf7
  switch (sprite_graphics[k]) {
  case 0: Sprite_BookOfMudora(k); break;
  case 1: Sprite_BonkKey(k); break;
  case 2: Sprite_LumberjackTree(k); break;
  }
}

void Sprite_BonkKey(int k) {  // 85fc04
  Sprite_DrawThinAndTall(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (Sprite_CheckDamageToLink_same_layer(k))
    sprite_ai_state[k] = 3;
  Sprite_MoveXY(k);
  sprite_z_vel[k]--;
  Sprite_MoveZ(k);
  if (sign8(sprite_z[k])) {
    sprite_y_vel[k] = 0;
    sprite_z[k] = 0;
    sprite_z_vel[k] = (uint8)-sprite_z_vel[k] >> 2;
    if (sprite_z_vel[k] & 254)
      SpriteSfx_QueueSfx3WithPan(k, 0x14);
  }
  switch (sprite_ai_state[k]) {
  case 0:  // wait for dash
    if ((uint16)(cur_sprite_x - link_x_coord + 16) < 33 &&
        (uint16)(cur_sprite_y - link_y_coord + 24) < 41 && (bg1_x_offset | bg1_y_offset))
      sprite_ai_state[k] = 1;
    break;
  case 1:  // being falling
    sprite_z_vel[k] = 32;
    sprite_y_vel[k] = -5;
    sound_effect_2 = 27;
    sprite_ai_state[k] = 2;
    break;
  case 2:  // falling
    if (!sprite_z[k])
      sprite_floor[k] = link_is_on_lower_level;
    break;
  case 3:  // give to player
    link_num_keys++;
    sprite_state[k] = 0;
    dung_savegame_state_bits |= (sprite_die_action[k] ? 0x2000 : 0x4000);
    SpriteSfx_QueueSfx3WithPan(k, 0x2f);
    break;
  }
}

void Sprite_BookOfMudora(int k) {  // 85fc9e
  SpriteDraw_SingleLarge(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (Sprite_CheckDamageToLink_same_layer(k))
    sprite_ai_state[k] = 3;
  Sprite_MoveXY(k);
  sprite_z_vel[k]--;
  Sprite_MoveZ(k);
  if (sign8(sprite_z[k])) {
    sprite_y_vel[k] = 0;
    sprite_z[k] = 0;
    sprite_z_vel[k] = (uint8)-sprite_z_vel[k] >> 2;
    if (sprite_z_vel[k] & 254)
      SpriteSfx_QueueSfx2WithPan(k, 0x21);
  }
  switch (sprite_ai_state[k]) {
  case 0:  // wait for dash
    if (link_direction_facing == 0 &&
        (uint16)(cur_sprite_x - link_x_coord + 39) < 47 &&
        (uint16)(cur_sprite_y - link_y_coord + 40) < 46 && (bg1_x_offset | bg1_y_offset))
      sprite_ai_state[k] = 1;
    break;
  case 1:  // being falling
    sprite_z_vel[k] = 32;
    sprite_y_vel[k] = -5;
    sound_effect_2 = 27;
    sprite_ai_state[k] = 2;
    break;
  case 2:  // falling
    if (!sprite_z[k])
      sprite_floor[k] = link_is_on_lower_level;
    break;
  case 3:  // give to player
    Link_CancelDash();
    item_receipt_method = 0;
    Link_ReceiveItem(0x1d, 0);
    sprite_state[k] = 0;
    break;
  }
}

void Sprite_LumberjackTree(int k) {  // 85fd4d
  sprite_flags2[k] = 0x8f;
  sprite_flags4[k] = 0x47;
  DashTreeTop_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (Sprite_CheckDamageToLink_same_layer(k)) {
    Sprite_NullifyHookshotDrag();
    link_speed_setting = 0;
    Sprite_RepelDash();
  }
  Sprite_MoveXY(k);
  sprite_z_vel[k]--;
  Sprite_MoveZ(k);
  if (sign8(sprite_z[k])) {
    sprite_z[k] = 0;
    sprite_z_vel[k] = (uint8)-sprite_z_vel[k] >> 2;
  }
  switch (sprite_ai_state[k]) {
  case 0:  // wait for dash
    sprite_subtype2[k] = 0;
    if ((uint16)(cur_sprite_x - link_x_coord + 24) < 65 &&
        (uint16)(cur_sprite_y - link_y_coord + 32) < 81 && (bg1_x_offset | bg1_y_offset) & 0xff) {
      sprite_z_vel[k] = 20;
      sprite_ai_state[k] = 1;
    }
    break;
  case 1:  // spawn leaves
    if (!sprite_z[k]) {
      sprite_ai_state[k]++;
      sound_effect_2 = 0x1b;
      sprite_x_vel[k] = -4;
      sprite_y_vel[k] = -4;
      int j = LumberjackTree_SpawnLeaves(k);
      sprite_x_vel[j] = 5;
      sprite_y_vel[j] = 5;
      j = LumberjackTree_SpawnLeaves(k);
      sprite_x_vel[j] = 5;
      sprite_y_vel[j] = -4;
      j = LumberjackTree_SpawnLeaves(k);
      sprite_x_vel[j] = -4;
      sprite_y_vel[j] = 4;
    }
    break;
  case 2: // dancing leaves
    if (!sprite_delay_main[k]) {
      sprite_delay_main[k] = 8;
      if (sprite_subtype2[k] == 6)
        sprite_state[k] = 0;
      sprite_subtype2[k]++;
    }
    break;
  }
}

void DashTreeTop_Draw(int k) {  // 85fe6f
  static const uint16 kDashTreeTop_CharFlags[16] = {0x3100, 0x3102, 0x7102, 0x7100, 0x3120, 0x3122, 0x7122, 0x7120, 0x3104, 0x3106, 0x7106, 0x7104, 0x3124, 0x3126, 0x7126, 0x7124};
  static const int8 kDashTreeTop_X[16] = {10, 22, 30, 1, 34, 5, 13, 29, 0, 17, 27, 44, 15, 33, 18, 26};
  static const int8 kDashTreeTop_Y[16] = {0, 4, 2, 7, 10, 16, 24, 23, 34, 35, 30, 31, 46, 42, 10, 11};
  static const int8 kDashTreeTop_Char[6] = {8, 8, 0x28, 0x28, 0x2a, 0x2a};
  static const int8 kDashTreeTop_Flags[6] = {0x31, 0x71, 0x31, 0x71, 0x31, 0x71};
  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  OamEnt *oam = GetOamCurPtr();

  BYTE(dungmap_var7) -= 0x20;
  HIBYTE(dungmap_var7) -= 0x20;

  if (!sprite_subtype2[k]) {
    uint8 x = BYTE(dungmap_var7);
    uint8 y = HIBYTE(dungmap_var7);
    for (int i = 0; i < 16; i++) {
      oam[i].x = x + (i & 3) * 0x10;
      oam[i].y = y + (i >> 2) * 0x10;
      WORD(oam[i].charnum) = kDashTreeTop_CharFlags[i];
    }
    Sprite_CorrectOamEntries(k, 15, 2);
  } else {
    int j = sprite_subtype2[k] - 1;
    for (int i = 15; i >= 0; i--, oam++) {
      oam->x = BYTE(dungmap_var7) + kDashTreeTop_X[i];
      oam->y = HIBYTE(dungmap_var7) + kDashTreeTop_Y[i];
      oam->charnum = kDashTreeTop_Char[j];
      oam->flags = kDashTreeTop_Flags[j];
    }
    Sprite_CorrectOamEntries(k, 15, 2);
  }
}

int LumberjackTree_SpawnLeaves(int k) {  // 85ff39
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0x3B, &info);
  assert(j >= 0);
  sprite_graphics[j] = 2;
  sprite_z_vel[j] = sprite_z_vel[k];
  sprite_subtype2[j] = 1;
  sprite_ai_state[j] = 2;
  sprite_delay_main[j] = 8;
  Sprite_SetSpawnedCoordinates(j, &info);
  return j;
}

void Sprite_TroughBoy(int k) {  // 85ff66
  TroughBoy_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_BehaveAsBarrier(k);
  Sprite_TrackBodyToHead(k);
  sprite_head_dir[k] = Sprite_DirectionToFaceLink(k, NULL) ^ 3;

  if (savegame_map_icons_indicator < 3) {
    if (Sprite_ShowSolicitedMessage(k, 0x147) & 0x100)
      savegame_map_icons_indicator = 2;
  } else {
    Sprite_ShowSolicitedMessage(k, 0x148);
  }
}

void TroughBoy_Draw(int k) {  // 85ffdf
  static const DrawMultipleData kTroughBoy_Dmd[8] = {
    {0, -8, 0x0882, 2},
    {0,  0, 0x0aaa, 2},
    {0, -8, 0x0882, 2},
    {0,  0, 0x0aaa, 2},
    {0, -8, 0x4880, 2},
    {0,  0, 0x0aaa, 2},
    {0, -8, 0x0880, 2},
    {0,  0, 0x0aaa, 2},
  };
  PrepOamCoordsRet info;
  Sprite_DrawMultiplePlayerDeferred(k, &kTroughBoy_Dmd[sprite_D[k] * 2], 2, &info);
  SpriteDraw_Shadow(k, &info);
}

void BottleMerchant_DetectFish(int k) {  // 868000
  for (int i = 15; i >= 0; i--) {
    if (sprite_state[i] && sprite_type[i] == 0xd2) {
      SpriteHitBox hb;
      hb.r0_xlo = sprite_x_lo[k];
      hb.r8_xhi = sprite_x_hi[k];
      hb.r2 = 16;
      hb.r1_ylo = sprite_y_lo[k];
      hb.r9_yhi = sprite_y_hi[k];
      hb.r3 = 16;
      Sprite_SetupHitBox(i, &hb);
      if (CheckIfHitBoxesOverlap(&hb))
        sprite_E[k] = 0x80 | i;
      return;
    }
  }
}

void BottleMerchant_BuyFish(int k) {  // 868054
  static const uint8 kBottleVendor_FishRewardType[5] = {0xdb, 0xe0, 0xde, 0xe2, 0xd9};
  static const int8 kBottleVendor_FishRewardXv[5] = {-6, -3, 0, 4, 7};
  static const int8 kBottleVendor_FishRewardYv[5] = {11, 14, 16, 14, 11};
  SpriteSpawnInfo info;
  SpriteSfx_QueueSfx3WithPan(k, 0x13);
  tmp_counter = 4;
  do {
    int j = Sprite_SpawnDynamically(k, kBottleVendor_FishRewardType[tmp_counter], &info);
    if (j < 0)
      return;
    Sprite_SetSpawnedCoordinates(j, &info);
    sprite_x_lo[j] = info.r0_x + 4;
    sprite_stunned[j] = 0xff;
    sprite_x_vel[j] = kBottleVendor_FishRewardXv[tmp_counter];
    sprite_y_vel[j] = kBottleVendor_FishRewardYv[tmp_counter];
    sprite_z_vel[j] = 32;
    sprite_delay_aux4[j] = 32;
  } while (!sign8(--tmp_counter));
}

void SpritePrep_ThrowableScenery(int k) {  // 86850f
}

void SpriteModule_Initialize(int k) {  // 86864d
  SpritePrep_LoadProperties(k);
  sprite_state[k]++;
  kSpritePrep_Main[sprite_type[k]](k);
}

void SpritePrep_Mantle(int k) {  // 868841
  sprite_y_lo[k] += 3;
  sprite_x_lo[k] += 8;
}

void SpritePrep_Switch(int k) {  // 868859
  int j = BYTE(dungeon_room_index2);
  if (j == 0xce || j == 4 || j == 0x3f)
    sprite_oam_flags[k] = 0xD;
}

void SpritePrep_SwitchFacingUp(int k) {  // 86886d
}

void SpritePrep_Snitch_bounce_1(int k) {  // 86886e
  SpritePrep_Snitches(k);
}

void SpritePrep_DoNothingA(int k) {  // 868873
}

void SpritePrep_Rat(int k) {  // 868878
  static const uint8 kSpriteRat_BumpDamage[2] = {0, 5};
  static const uint8 kSpriteRat_Health[2] = {2, 8};
  int j = is_in_dark_world;
  sprite_bump_damage[k] = kSpriteRat_BumpDamage[j];
  sprite_health[k] = kSpriteRat_Health[j];
}

void SpritePrep_Keese(int k) {  // 86888e
  static const uint8 kSpriteKeese_BumpDamage[2] = {0x80, 0x85};
  static const uint8 kSpriteKeese_Health[2] = {1, 4};
  static const uint8 kSpriteKeese_Flags5[2] = {0, 7};
  int j = is_in_dark_world;
  sprite_bump_damage[k] = kSpriteKeese_BumpDamage[j];
  sprite_health[k] = kSpriteKeese_Health[j];
  sprite_flags5[k] = kSpriteKeese_Flags5[j];
}

void SpritePrep_Rope(int k) {  // 8688aa
  static const uint8 kSpriteRope_BumpDamage[2] = {1, 5};
  static const uint8 kSpriteRope_Health[2] = {4, 8};
  static const uint8 kSpriteRope_Flags5[2] = {1, 7};
  int j = is_in_dark_world;
  sprite_bump_damage[k] = kSpriteRope_BumpDamage[j];
  sprite_health[k] = kSpriteRope_Health[j];
  sprite_flags5[k] = kSpriteRope_Flags5[j];
}

void SpritePrep_Swamola(int k) {  // 8688c0
  SpritePrep_Swamola_InitializeSegments(k);
  SpritePrep_Kyameron(k);
}

void SpritePrep_Blind(int k) {  // 8688c7
  if (Sprite_ReturnIfBossFinished(k))
    return;
  SpritePrep_Blind_PrepareBattle(k);
}

void SpritePrep_Ganon(int k) {  // 8688cf
  if (Sprite_ReturnIfBossFinished(k))
    return;
  Ganon_HandleAnimation_Idle(k);
  sprite_delay_main[k] = 128;
  sprite_room[k] = 2;
  music_control = 0x1e;
}

void SpritePrep_Pokey(int k) {  // 8688df
  static const int8 kHokbok_InitXvel[4] = {16, -16, 16, -16};
  static const int8 kHokbok_InitYvel[4] = {16, 16, -16, -16};
  sprite_A[k] = 3;
  sprite_B[k] = 8;
  int j = GetRandomNumber() & 3;
  sprite_x_vel[k] = kHokbok_InitXvel[j];
  sprite_y_vel[k] = kHokbok_InitYvel[j];
}

void SpritePrep_MiniVitreous(int k) {  // 8688fd
  Sprite_ReturnIfBossFinished(k);
}

void SpritePrep_Gibo(int k) {  // 868901
  sprite_z[k] = 16;
  sprite_G[k] = 8;
}

void SpritePrep_Octoballoon(int k) {  // 868910
  static const uint8 kSprite_Octoballoon_Delay[4] = {192, 208, 224, 240};
  sprite_delay_main[k] = kSprite_Octoballoon_Delay[k & 3];
}

void SpritePrep_AgahnimsBarrier(int k) {  // 86891b
  if (save_ow_event_info[BYTE(overworld_screen_index)] & 0x40)
    sprite_graphics[k] = 4;
  SpritePrep_MoveDown_8px_Right8px(k);
  sprite_y_lo[k] -= 12;
  sprite_ignore_projectile[k]++;
}

void SpritePrep_Catfish(int k) {  // 86892c
  SpritePrep_MoveDown_8px_Right8px(k);
  sprite_y_lo[k] -= 12;
  SpritePrep_IgnoreProjectiles(k);
}

void SpritePrep_CutsceneAgahnim(int k) {  // 86893b
  if (dung_savegame_state_bits & 0x4000) {
    sprite_state[k] = 0;
  } else {
    CutsceneAgahnim_SpawnZeldaOnAltar(k);
    sprite_ignore_projectile[k]++;
  }
}

void SpritePrep_Vitreous(int k) {  // 86894d
  if (Sprite_ReturnIfBossFinished(k))
    return;
  SpritePrep_MoveDown_8px_Right8px(k);
  sprite_y_lo[k] -= 16;
  Vitreous_SpawnSmallerEyes(k);
  sprite_ignore_projectile[k]++;
}

void SpritePrep_Raven(int k) {  // 868969
  static const uint8 kSpriteRaven_BumpDamage[2] = {0x81, 0x88};
  static const uint8 kSpriteRaven_Health[2] = {4, 8};
  static const uint8 kSpriteRaven_Flags5[2] = {6, 2};
  int j = is_in_dark_world;
  sprite_bump_damage[k] = kSpriteRaven_BumpDamage[j];
  sprite_health[k] = kSpriteRaven_Health[j];
  sprite_flags5[k] = kSpriteRaven_Flags5[j];
  sprite_z[k] = 0;
  sprite_A[k] = (sprite_x_lo[k] & 16) >> 4;
  sprite_subtype[k] = 254;
}

void SpritePrep_Vulture(int k) {  // 86897e
  sprite_z[k] = 0;
  sprite_A[k] = (sprite_x_lo[k] & 16) >> 4;
  sprite_subtype[k] = 254;
}

void SpritePrep_Poe(int k) {  // 868991
  sprite_z[k] = 12;
  sprite_subtype[k] = 254;
}

void SpritePrep_DoNothingC(int k) {  // 86899b
  // empty
}

void SpritePrep_BlindMaiden(int k) {  // 86899c
  if (!(save_dung_info[0xac] & 0x800)) {
    sprite_ignore_projectile[k]++;
    if (follower_indicator != 6) {
      follower_indicator = 6;
      follower_dropped = 0;
      tagalong_var5 = 0;
      LoadFollowerGraphics();
      Follower_Initialize();
      follower_indicator = 0;
      return;
    }
  }
  sprite_state[k] = 0;
}

void SpritePrep_MiniMoldorm_bounce(int k) {  // 8689d3
  int j = 32 * k;
  for (int i = 0; i < 32; i++, j++) {
    moldorm_x_lo[j] = sprite_x_lo[k];
    moldorm_x_hi[j] = sprite_x_hi[k];
    moldorm_y_lo[j] = sprite_y_lo[k];
    moldorm_y_hi[j] = sprite_y_hi[k];
  }
}

void SpritePrep_Bomber(int k) {  // 8689d8
  sprite_z[k] = 16;
  sprite_subtype[k] = 254;
}

void SpritePrep_BombShoppe(int k) {  // 8689df
  sprite_ignore_projectile[k]++;
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0xb5, &info);
  if (j >= 0) {
    Sprite_SetX(j, info.r0_x - 24);
    Sprite_SetY(j, info.r2_y - 24);
    sprite_subtype2[j] = 1;
    sprite_ignore_projectile[j] = 1;
  }
  if ((link_has_crystals & 5) == 5 && sram_progress_indicator_3 & 32) {
    int j = Sprite_SpawnDynamically(k, 0xb5, &info);
    if (j >= 0) {
      Sprite_SetX(j, info.r0_x - 56);
      Sprite_SetY(j, info.r2_y - 24);
      sprite_subtype2[j] = 2;
      sprite_ignore_projectile[j] = 2;
    }
  }
}

void SpritePrep_BullyAndVictim(int k) {  // 868a51
  SpawnBully(k);
  sprite_ignore_projectile[k]++;
}

void SpritePrep_PurpleChest(int k) {  // 868a59
  if (follower_indicator != 12 && !(sram_progress_indicator_3 & 16) && sram_progress_indicator_3 & 32)
    sprite_ignore_projectile[k]++;
  else
    sprite_state[k] = 0;
}

void SpritePrep_Smithy(int k) {  // 868a79
  sprite_ignore_projectile[k]++;
  if (savegame_is_darkworld & 64) {
    if (sram_progress_indicator_3 & 32 || follower_indicator != 0)
      sprite_state[k] = 0;
    else
      sprite_subtype2[k] = 2;
    return;
  }
  Smithy_SpawnDumbBarrierSprite(k);
  if (!(sram_progress_indicator_3 & 32)) {
    sprite_x_lo[k] += 2;
    sprite_y_lo[k] -= 3;
    return;
  }
  sprite_x_lo[k] += 2;
  sprite_y_lo[k] -= 3;
  int j = Smithy_SpawnDwarfPal(k);
  Smithy_SpawnDumbBarrierSprite(j);
  sprite_E[j] = k;
  sprite_E[k] = j;

  if (sram_progress_indicator_3 & 0x80) {
    sprite_ai_state[k] = 5;
    sprite_ai_state[j] = 5;
  }
}

void SpritePrep_Babasu(int k) {  // 868af0
  SpritePrep_MoveDown_8px(k);
  SpritePrep_Zoro(k);
}

void SpritePrep_Zoro(int k) {  // 868af3
  sprite_D[k] = (sprite_type[k] - 0x9c) << 1;
  sprite_graphics[k]--;
}

void SpritePrep_LaserEye_bounce(int k) {  // 868b03
  int t = sprite_type[k];
  sprite_D[k] = t - 0x95;
  if (t >= 0x97) {
    sprite_x_lo[k] += 8;
    sprite_head_dir[k] = sprite_x_lo[k] & 16 ^ 16;
    if (!sprite_head_dir[k])
      sprite_y_lo[k] += (t & 1) ? -8 : 8;
  } else {
    sprite_head_dir[k] = sprite_y_lo[k] & 16;
    if (!sprite_head_dir[k])
      sprite_x_lo[k] += (t & 1) ? -8 : 8;
  }
}

void SpritePrep_Popo(int k) {  // 868b08
  sprite_B[k] = 7;
}

void SpritePrep_Popo2(int k) {  // 868b0c
  sprite_B[k] = 15;
}

void SpritePrep_Statue(int k) {  // 868b12
  sprite_y_lo[k] += 7;
}

void SpritePrep_Bari(int k) {  // 868b1c
  sprite_z[k] = 6;
  if (BYTE(dungeon_room_index2) == 206)
    sprite_C[k]--;
  sprite_delay_aux1[k] = (GetRandomNumber() & 63) + 128;
}

void SpritePrep_GreenStalfos(int k) {  // 868b2e
  sprite_z[k] = 9;
}

void SpritePrep_WaterLever(int k) {  // 868b34
  sprite_y_lo[k] += 5;
}

void SpritePrep_FireDebirando(int k) {  // 868b3e
  sprite_type[k] = 0x63;
  SpritePrep_LoadProperties(k);
  sprite_G[k]--;
  SpritePrep_DebirandoPit(k);
}

void SpritePrep_DebirandoPit(int k) {  // 868b4a
  static const uint8 kDebirando_OamFlags[2] = {6, 8};
  sprite_G[k]++;
  sprite_delay_main[k] = 0;
  sprite_graphics[k] = 6;
  SpritePrep_IgnoreProjectiles(k);
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0x64, &info);
  if (j >= 0) {
    Sprite_SetSpawnedCoordinates(j, &info);
    sprite_delay_main[j] = 96;
    sprite_head_dir[k] = j;
    sprite_G[j] = sprite_G[k];
    sprite_oam_flags[j] = kDebirando_OamFlags[sprite_G[j]];
  }
}

void SpritePrep_WeakGuard(int k) {  // 868b81
  sprite_D[k] = sprite_head_dir[k] = GetRandomNumber() & 3;
  sprite_delay_main[k] = 16;
}

void SpritePrep_WallCannon(int k) {  // 868b93
  sprite_D[k] = sprite_type[k] - 0x66;
  sprite_A[k] = sprite_D[k] & 2;
}

void SpritePrep_ArrowGame_bounce(int k) {  // 868ba2
  static const uint8 kArcheryGameGuy_X[8] = {0, 0x40, 0x80, 0xc0, 0x30, 0x60, 0x90, 0xc0};
  static const uint8 kArcheryGameGuy_Y[8] = {0, 0x4f, 0x4f, 0x4f, 0x5a, 0x5a, 0x5a, 0x5a};
  static const uint8 kArcheryGameGuy_A[8] = {0, 1, 1, 1, 2, 2, 2, 2};
  static const int8 kArcheryGameGuy_Xvel[2] = {-8, 12};
  static const uint8 kArcheryGameGuy_Flags4[2] = {0x1c, 0x15};
  byte_7E0B88 = 0;
  sprite_y_lo[k] -= 9;
  for (int i = 7; i != 0; i--) {
    sprite_type[i] = 0x65;
    sprite_state[i] = 9;
    SpritePrep_LoadProperties(i);
    sprite_x_hi[i] = (link_x_coord >> 8);
    sprite_x_lo[i] = kArcheryGameGuy_X[i];
    sprite_y_hi[i] = (link_y_coord >> 8);
    sprite_y_lo[i] = kArcheryGameGuy_Y[i];
    sprite_A[i] = kArcheryGameGuy_A[i];
    int j = kArcheryGameGuy_A[i] - 1;
    sprite_graphics[i] = j;
    sprite_x_vel[i] = kArcheryGameGuy_Xvel[j];
    sprite_flags4[i] = kArcheryGameGuy_Flags4[j];
    sprite_oam_flags[i] = 13;
    sprite_floor[i] = link_is_on_lower_level;
    sprite_subtype2[i] = GetRandomNumber();
  }
  sprite_ignore_projectile[k]++;
  sprite_subtype[k] = link_num_arrows;
}

void SpritePrep_IgnoreProjectiles(int k) {  // 868ba7
  sprite_ignore_projectile[k]++;
}

void SpritePrep_HauntedGroveAnimal(int k) {  // 868bab
  sprite_D[k] = Sprite_IsRightOfLink(k).a;
  SpritePrep_HauntedGroveOstritch(k);
}

void SpritePrep_HauntedGroveOstritch(int k) {  // 868bb2
  if (link_item_flute >= 2)
    sprite_state[k] = 0;
  sprite_ignore_projectile[k]++;
}

void SpritePrep_DiggingGameGuy_bounce(int k) {  // 868bbf
  if (link_y_coord < Sprite_GetY(k)) {
    sprite_ai_state[k] = 5;
    sprite_x_lo[k] -= 9;
    sprite_graphics[k] = 1;
  }
  sprite_ignore_projectile[k]++;
}

void SpritePrep_ThievesTownGrate(int k) {  // 868bc4
  if (save_ow_event_info[0x58] & 0x20)
    sprite_state[k] = 0;
  sprite_ignore_projectile[k]++;
  Sprite_SetX(k, Sprite_GetX(k) - 8);
}

void SpritePrep_RupeePull(int k) {  // 868bcf
  sprite_ignore_projectile[k]++;
  Sprite_SetX(k, Sprite_GetX(k) - 8);
}

void SpritePrep_Shopkeeper(int k) {  // 868bf1
  sprite_ignore_projectile[k]++;
  sprite_flags2[k] |= 2;
  sprite_oam_flags[k] |= 12;
  sprite_flags3[k] |= 16;
  static const uint8 kShopKeeperWhere[13] = {0xf, 0x10, 0, 6, 0x18, 0x12, 0x1e, 0xff, 0x1f, 0x23, 0x24, 0x25, 0x27};
  uint8 room = BYTE(dungeon_room_index);
  int j = FindInByteArray(kShopKeeperWhere, room, 13);
  switch (j) {
  case 0:
    ShopKeeper_SpawnShopItem(k, 0, 7);
    ShopKeeper_SpawnShopItem(k, 1, 8);
    ShopKeeper_SpawnShopItem(k, 2, 12);
    break;
  case 1:
    ShopKeeper_SpawnShopItem(k, 0, 9);
    ShopKeeper_SpawnShopItem(k, 1, 13);
    ShopKeeper_SpawnShopItem(k, 2, 11);
    break;
  case 2:
    sprite_subtype2[k] = 4;
    minigame_credits = 0xff;
    break;
  case 3:
    sprite_subtype2[k] = sprite_graphics[k] = 1;
    minigame_credits = 0xff;
    break;
  case 4:
    sprite_subtype2[k] = 3;
    minigame_credits = 0xff;
    break;
  case 5: case 7: case 8:
    ShopKeeper_SpawnShopItem(k, 0, 7);
    ShopKeeper_SpawnShopItem(k, 1, 10);
    ShopKeeper_SpawnShopItem(k, 2, 12);
    break;
  case 6: case 9: case 12:
    sprite_subtype2[k] = 2;
    break;
  case 10:
    sprite_subtype2[k] = 5;
    break;
  case 11:
    sprite_subtype2[k] = 6;
    break;
  default:
    assert(0);
  }
}

void SpritePrep_Storyteller(int k) {  // 868c9e
  static const uint8 kStoryTellerRooms[5] = {0xe, 0xe, 0x12, 0x1a, 0x14};
  int r = FindInByteArray(kStoryTellerRooms, BYTE(dungeon_room_index), 5);
  if (r == 0 && sprite_x_hi[k] & 1)
    r = 1;
  sprite_subtype2[k] = r;
  sprite_ignore_projectile[k]++;
}

void SpritePrep_Adults(int k) {  // 868cc1
  static const uint8 kHumanMultiTypes[3] = {3, 0xe1, 0x19};
  sprite_ignore_projectile[k]++;
  sprite_subtype2[k] = FindInByteArray(kHumanMultiTypes, BYTE(dungeon_room_index), 3);
}

void SpritePrep_Whirlpool(int k) {  // 868cd5
  sprite_ignore_projectile[k]++;
  sprite_A[k] = 1;
}

void SpritePrep_Sage(int k) {  // 868cde
  sprite_ignore_projectile[k]++;
  if (BYTE(dungeon_room_index) == 10) {
    sprite_subtype2[k]++;
    sprite_oam_flags[k] = 11;
  }
}

void SpritePrep_BonkItem(int k) {  // 868cf2
  static const uint16 kDashItemMask[2] = {0x4000, 0x2000};
  if (!player_is_indoors) {
    sprite_graphics[k] = 2;
    return;
  }
  sprite_floor[k] = 2;
  if (dungeon_room_index == 0x107) {
    if (link_item_book_of_mudora)
      sprite_state[k] = 0;
    else
      DecodeAnimatedSpriteTile_variable(0xe);
  } else {
    int j = byte_7E0B9B++;
    sprite_die_action[k] = j;
    if (dung_savegame_state_bits & kDashItemMask[j])
      sprite_state[k] = 0;
    sprite_graphics[k]++;
    sprite_oam_flags[k] = 8;
    sprite_flags3[k] |= 0x20;
  }
}

void SpritePrep_Kiki(int k) {  // 868d46
  sprite_ignore_projectile[k]++;
  if (save_ow_event_info[BYTE(overworld_screen_index)] & 0x20)
    sprite_state[k] = 0;
}

void SpritePrep_Locksmith(int k) {  // 868d59
  sprite_ignore_projectile[k]++;
  if (follower_indicator == 9) {
    sprite_state[k] = 0;
    return;
  }
  if (follower_indicator == 12) {
    sprite_ai_state[k] = 2;
  }
  if (sram_progress_indicator_3 & 0x10)
    sprite_ai_state[k] = 4;
}

void SpritePrep_SickKid(int k) {  // 868d7f
  if (link_item_bug_net)
    sprite_ai_state[k] = 3;
  sprite_ignore_projectile[k]++;
}

void SpritePrep_Tektite(int k) {  // 868d94
  static const uint8 kGanonHelpers_OamFlags[2] = {9, 7};
  static const uint8 kGanonHelpers_Health[2] = {8, 12};
  static const uint8 kGanonHelpers_BumpDamage[2] = {3, 5};
  int j;
  sprite_A[k] = j = sprite_x_lo[k] >> 4 & 1;
  sprite_oam_flags[k] = kGanonHelpers_OamFlags[j];
  sprite_health[k] = kGanonHelpers_Health[j];
  sprite_bump_damage[k] = kGanonHelpers_BumpDamage[j];
  Sprite_ApplySpeedTowardsLink(k, 16);
  sprite_z_vel[k] = 32;
  sprite_ai_state[k]++;
}

void SpritePrep_Chainchomp_bounce(int k) {  // 868dc1
  int i = k * 8;
  for (int j = 5; j >= 0; j--, i++) {
    chainchomp_x_hist[i] = cur_sprite_x;
    chainchomp_y_hist[i] = cur_sprite_y;
  }
  sprite_A[k] = sprite_x_lo[k];
  sprite_B[k] = sprite_x_hi[k];
  sprite_C[k] = sprite_y_lo[k];
  sprite_G[k] = sprite_y_hi[k];
}

void SpritePrep_BigFairy(int k) {  // 868dc6
  sprite_z[k] = 24;
  SpritePrep_MoveDown_8px_Right8px(k);
  sprite_ignore_projectile[k]++;
}

void SpritePrep_MrsSahasrahla(int k) {  // 868dd1
  sprite_y_lo[k] += 8;
  SpritePrep_MagicBat(k);
}

void SpritePrep_MagicBat(int k) {  // 868dda
  sprite_x_lo[k] += 8;
  sprite_ignore_projectile[k]++;
}

void SpritePrep_FortuneTeller(int k) {  // 868de0
  SpritePrep_IncrXYLow8(k);
  sprite_ignore_projectile[k]++;
}

void SpritePrep_FairyPond(int k) {  // 868de9
  static const uint8 kLeever_OamFlags[2] = {10, 2};
  int j = sprite_x_lo[k] >> 4 & 1;
  sprite_A[k] = j;
  sprite_oam_flags[k] = kLeever_OamFlags[j];
}

void SpritePrep_Hobo(int k) {  // 868dfd
  for (int i = 15; i; i--)
    SpritePrep_Hobo_SpawnSmoke(k);
  for (int i = 15; i; i--) {
    if (sprite_type[i] == 0x2b)
      sprite_state[i] = 0;
  }
  SpritePrep_Hobo_SpawnFire(k);
  if (sram_progress_indicator_3 & 1)
    sprite_ai_state[0] = 3;
  sprite_ignore_projectile[0] = 1;
}

void SpritePrep_MasterSword(int k) {  // 868e30
  sprite_x_lo[k] += 6;
  sprite_y_lo[k] += 6;
}

void SpritePrep_Roller_HorizontalRightFirst(int k) {  // 868e42
  sprite_ai_state[k] = (~sprite_x_lo[k] & 16) >> 4;
  if (sprite_ai_state[k])
    sprite_flags4[k]++;
  sprite_D[k] = 0;
}

void SpritePrep_RollerLeftRight(int k) {  // 868e46
  sprite_ai_state[k] = (~sprite_x_lo[k] & 16) >> 4;
  if (sprite_ai_state[k])
    sprite_flags4[k]++;
  sprite_D[k] = 1;
}

void SpritePrep_Roller_VerticalDownFirst(int k) {  // 868e4f
  sprite_ai_state[k] = (sprite_y_lo[k] & 16) >> 4;
  if (sprite_ai_state[k])
    sprite_flags4[k]++;
  sprite_D[k] = 2;
}

void SpritePrep_RollerUpDown(int k) {  // 868e53
  sprite_ai_state[k] = (sprite_y_lo[k] & 16) >> 4;
  if (sprite_ai_state[k])
    sprite_flags4[k]++;
  sprite_D[k] = 3;
}

void SpritePrep_Kodongo(int k) {  // 868e6b
  sprite_x_lo[k] += 4;
  Sprite_SetY(k, Sprite_GetY(k) - 5);
  sprite_subtype[k]--;
}

void SpritePrep_Spark(int k) {  // 868e85
  sprite_subtype[k]--;
}

void SpritePrep_LostWoodsBird(int k) {  // 868ec1
  sprite_z_vel[k] = (GetRandomNumber() & 0x1f) - 0x10;
  sprite_z[k] = 64;
  SpritePrep_LostWoodsSquirrel(k);
}

void SpritePrep_LostWoodsSquirrel(int k) {  // 868ed2
  sprite_x_vel[k] = Sprite_IsRightOfLink(k).a ? -16 : 16;
  sprite_ignore_projectile[k] = sprite_y_vel[k] = sign8(byte_7E069E[0]) ? 4 : -4;
}

void SpritePrep_Antifairy(int k) {  // 868ef2
  static const int8 kBubble_Xvel[2] = {16, -16};
  sprite_x_vel[k] = kBubble_Xvel[sprite_x_lo[k] >> 4 & 1];
  sprite_y_vel[k] = -16;
}

void SpritePrep_FallingIce(int k) {  // 868f08
  if (Sprite_ReturnIfBossFinished(k))
    return;
  sprite_ignore_projectile[k]++;
}

void SpritePrep_KingZora(int k) {  // 868f0f
  if (link_item_flippers)
    sprite_state[k] = 0;
  else
    sprite_ignore_projectile[k]++;
}

bool Sprite_ReturnIfBossFinished(int k) {  // 868f1c
  if (dung_savegame_state_bits & 0x8000) {
    sprite_state[k] = 0;
    return true;
  }
  for (int j = 15; j >= 0; j--) {
    if (!(kSpriteInit_BumpDamage[sprite_type[j]] & 0x10))
      sprite_state[j] = 0;
  }
  return false;
}

void SpritePrep_ArmosKnight(int k) {  // 868f3f
  if (Sprite_ReturnIfBossFinished(k))
    return;
  sprite_delay_main[k] = 255;
  byte_7E0FF8++;
  SpritePrep_MoveDown_8px_Right8px(k);
}

void SpritePrep_DesertStatue(int k) {  // 868f4d
  sprite_A[k] = sprite_limit_instance;
  sprite_limit_instance++;
  SpritePrep_MoveDown_8px_Right8px(k);
  sprite_D[k] = (sprite_x_lo[k] < 0x30) ? 1 : (sprite_x_lo[k] < 0xe0) ? 3 : 2;
}

void SpritePrep_DoNothingD(int k) {  // 868f6c
  // empty
}

void SpritePrep_Octorok(int k) {  // 868f71
  static const uint8 kOctorock_BumpDamage[2] = {3, 5};
  static const uint8 kOctorock_Health[2] = {2, 4};
  int j = is_in_dark_world;
  sprite_health[k] = kOctorock_Health[j];
  sprite_bump_damage[k] = kOctorock_BumpDamage[j];
  sprite_delay_main[k] = GetRandomNumber() & 127;
}

void SpritePrep_Moldorm(int k) {  // 868f8a
  if (Sprite_ReturnIfBossFinished(k))
    return;
  sprite_ignore_projectile[k]++;
  Sprite_InitializedSegmented(k);
}

void SpritePrep_Lanmolas(int k) {  // 868f95
  static const uint8 kLanmola_InitDelay[3] = {128, 192, 255};

  if (Sprite_ReturnIfBossFinished(k))
    return;
  sprite_delay_main[k] = kLanmola_InitDelay[k];
  sprite_z[k] = -1;
  for (int i = 0; i < 64; i++)
    beamos_x_hi[k * 0x40 + i] = 0xff;
  garnish_y_lo[k] = 7;
}

void SpritePrep_BigSpike(int k) {  // 868f9d
  SpritePrep_MoveDown_8px_Right8px(k);
  SpritePrep_Kyameron(k);
}

void SpritePrep_SwimmingZora(int k) {  // 868fa2
  sprite_delay_main[k] = 64;
  SpritePrep_Geldman(k);
}

void SpritePrep_Geldman(int k) {  // 868fa7
  sprite_x_lo[k] += 8;
  SpritePrep_Kyameron(k);
}

void SpritePrep_Kyameron(int k) {  // 868fb0
  sprite_A[k] = sprite_x_lo[k];
  sprite_B[k] = sprite_x_hi[k];
  sprite_C[k] = sprite_y_lo[k];
  sprite_head_dir[k] = sprite_y_hi[k];
}

void SpritePrep_WalkingZora(int k) {  // 868fc9
  sprite_delay_main[k] = 96;
}

void SpritePrep_StandardGuard(int k) {  // 868fd6
  static const uint8 kSpriteSoldier_Tab0[8] = {0, 2, 1, 3, 6, 4, 5, 7};

  uint8 subtype = sprite_subtype[k];
  if (subtype != 0) {
    if ((subtype & 7) >= 5) {
      int j = ((subtype & 7) != 5) * 4 + (subtype >> 3 & 3);
      sprite_B[k] = kSpriteSoldier_Tab0[j];
      sprite_flags[k] = sprite_flags[k] & 0xf | 0x50;
      SpritePrep_TrooperAndArcherSoldier(k);
      return;
    }
    sprite_D[k] = ((subtype & 7) - 1) ^ 1;
  }
  if (player_is_indoors) {
    sprite_flags5[k] &= ~0x80;
    return;
  }
  sprite_ai_state[k] = 1;
  sprite_delay_main[k] = 112;
  sprite_D[k] = sprite_head_dir[k] = Sprite_DirectionToFaceLink(k, NULL);
  SpritePrep_TrooperAndArcherSoldier(k);
}

void SpritePrep_TrooperAndArcherSoldier(int k) {  // 869001
  uint8 bak0 = submodule_index;
  submodule_index = 0;
  sprite_defl_bits[k] = (sprite_defl_bits[k] >> 1) | 0x80;
  SpriteActive_Main(k);
  SpriteActive_Main(k);
  sprite_defl_bits[k] <<= 1;
  submodule_index = bak0;
}

void SpritePrep_TalkingTree(int k) {  // 869043
  sprite_ignore_projectile[k]++;
  Sprite_SetX(k, Sprite_GetX(k) - 8);
  SpritePrep_TalkingTree_SpawnEyeball(k, 0);
  SpritePrep_TalkingTree_SpawnEyeball(k, 1);
}

void SpritePrep_CrystalSwitch(int k) {  // 869064
  sprite_oam_flags[k] |= kCrystalSwitchPal[orange_blue_barrier_state & 1];
}

void SpritePrep_FluteKid(int k) {  // 869075
  sprite_ignore_projectile[k]++;
  sprite_subtype2[k] = savegame_is_darkworld >> 6 & 1;
  if (sprite_subtype2[k]) {
    if (sram_progress_indicator_3 & 8 || link_item_flute > 2) {
      sprite_graphics[k] = 3;
      sprite_ai_state[k] = 5;
    } else if (link_item_flute == 2) {
      sprite_graphics[k] = 1;
    }
    sprite_x_lo[k] += 8;
    sprite_y_lo[k] -= 8;
  } else {
    if (link_item_flute >= 2)
      sprite_state[k] = 0;
    else
      sprite_x_lo[k] += 7;
  }
}

void SpritePrep_MoveDown_8px(int k) {  // 8690cc
  sprite_y_lo[k] += 8;
}

void SpritePrep_Zazakku(int k) {  // 8690d5
  // empty
}

void SpritePrep_PedestalPlaque(int k) {  // 8690d6
  sprite_ignore_projectile[k]++;
  if (BYTE(overworld_screen_index) == 48)
    sprite_x_lo[k] += 7;
}

void SpritePrep_Stalfos(int k) {  // 8690e0
  sprite_subtype[k] = sprite_x_lo[k] & 16;
  if (sprite_subtype[k])
    sprite_oam_flags[k] = 7;
}

void SpritePrep_KholdstareShell(int k) {  // 8690f0
  if (Sprite_ReturnIfBossFinished(k))
    return;
  sprite_delay_aux1[k] = 192;
  SpritePrep_MoveDown_8px_Right8px(k);
}

void SpritePrep_Kholdstare(int k) {  // 8690fa
  if (Sprite_ReturnIfBossFinished(k))
    return;
  sprite_ai_state[k] = 3;
  SpritePrep_IgnoreProjectiles(k);
  SpritePrep_MoveDown_8px_Right8px(k);
}

void SpritePrep_Bumper(int k) {  // 869107
  sprite_ignore_projectile[k]++;
  SpritePrep_MoveDown_8px_Right8px(k);
}

void SpritePrep_MoveDown_8px_Right8px(int k) {  // 86910a
  sprite_x_lo[k] += 8;
  sprite_y_lo[k] += 8;
}

void SpritePrep_HardhatBeetle(int k) {  // 869122
  static const uint8 kHardHatBeetle_OamFlags[2] = {6, 8};
  static const uint8 kHardHatBeetle_Health[2] = {32, 6};
  static const uint8 kHardHatBeetle_A[2] = {16, 12};
  static const uint8 kHardHatBeetle_State[2] = {1, 3};
  static const uint8 kHardHatBeetle_Flags5[2] = {2, 6};
  static const uint8 kHardHatBeetle_BumpDamage[2] = {5, 3};
  int j = (sprite_x_lo[k] & 0x10) != 0;
  sprite_oam_flags[k] = kHardHatBeetle_OamFlags[j];
  sprite_health[k] = kHardHatBeetle_Health[j];
  sprite_A[k] = kHardHatBeetle_A[j];
  sprite_ai_state[k] = kHardHatBeetle_State[j];
  sprite_flags5[k] = kHardHatBeetle_Flags5[j];
  sprite_bump_damage[k] = kHardHatBeetle_BumpDamage[j];
}

void SpritePrep_MiniHelmasaur(int k) {  // 869151
  sprite_A[k] = 16;
  sprite_ai_state[k] = 1;
}

void SpritePrep_Fairy(int k) {  // 86915c
  sprite_A[k] = GetRandomNumber() & 1;
  sprite_D[k] = sprite_A[k] ^ 1;
  SpritePrep_Absorbable(k);
}

void SpritePrep_Absorbable(int k) {  // 86916a
  if (!player_is_indoors) {
    sprite_E[k]++;
    sprite_ignore_projectile[k]++;
  }
}

void SpritePrep_OverworldBonkItem(int k) {  // 86916e
  sprite_E[k]++;
  sprite_ignore_projectile[k]++;
}

void SpritePrep_ShieldPickup(int k) {  // 869174
}

void SpritePrep_NiceBee(int k) {  // 869175
  uint8 or_bottle = link_bottle_info[0] | link_bottle_info[1] | link_bottle_info[2] | link_bottle_info[3];
  if (or_bottle & 8)
    sprite_state[k] = 0;
  sprite_E[k]++;
  sprite_ignore_projectile[k]++;
}

void SpritePrep_Agahnim(int k) {  // 869195
  static const uint8 kAgahnim_OamFlags[2] = {11, 7};
  if (Sprite_ReturnIfBossFinished(k))
    return;
  sprite_graphics[k] = 0;
  sprite_D[k] = 3;
  SpritePrep_MoveDown_8px_Right8px(k);
  sprite_oam_flags[k] = kAgahnim_OamFlags[is_in_dark_world];
}

void SpritePrep_DoNothingG(int k) {  // 8691ae
  // empty
}

void SpritePrep_FireBar(int k) {  // 8691b4
  sprite_B[k]++;
  sprite_ignore_projectile[k]++;
}

void SpritePrep_Trinexx(int k) {  // 8691ba
  if (Sprite_ReturnIfBossFinished(k))
    return;
  TrinexxComponents_Initialize(k);
  for (int i = 15; i >= 0; i--)
    alt_sprite_state[i] = 0;
}

void SpritePrep_HelmasaurKing(int k) {  // 8691c5
  if (Sprite_ReturnIfBossFinished(k))
    return;
  HelmasaurKing_Initialize(k);
  memset(alt_sprite_state, 0, 16);
}

void SpritePrep_Spike(int k) {  // 8691d7
  sprite_x_vel[k] = 32;
  sprite_y_vel[k] = -16;
  Sprite_MoveY(k);
  sprite_y_vel[k] = 0;
}

void SpritePrep_RockStal(int k) {  // 8691dc
  sprite_y_vel[k] = -16;
  Sprite_MoveY(k);
  sprite_y_vel[k] = 0;
}

void SpritePrep_Blob(int k) {  // 8691e8
  sprite_graphics[k] = 4;
  SpritePrep_IgnoreProjectiles(k);
}

void SpritePrep_Arrghus(int k) {  // 8691f1
  if (Sprite_ReturnIfBossFinished(k))
    return;
  sprite_z[k] = 24;
}

void SpritePrep_Arrghi(int k) {  // 8691fa
  if (Sprite_ReturnIfBossFinished(k))
    return;
  sprite_subtype2[k] = GetRandomNumber();
  if (k == 13) {
    overlord_x_lo[2] = 0;
    overlord_x_lo[3] = 0;
    Arrghus_HandlePuffs(0);
  }
  sprite_x_lo[k] = overlord_x_lo[k + 7];
  sprite_x_hi[k] = overlord_y_lo[k + 7];
  sprite_y_lo[k] = overlord_gen1[k + 7];
  sprite_y_hi[k] = overlord_gen3[k + 7];
}

void SpritePrep_Mothula(int k) {  // 86922f
  if (Sprite_ReturnIfBossFinished(k))
    return;
  sprite_delay_main[k] = 80;
  sprite_ignore_projectile[k]++;
  sprite_graphics[k] = 2;
  BYTE(dung_floor_move_flags)++;
  sprite_C[k] = 112;
}

void SpritePrep_DoNothingH(int k) {  // 86924d
  // empty
}

void SpritePrep_BigKey(int k) {  // 86924e
  sprite_x_lo[k] += 8;
  sprite_subtype[k] = 0xff;
  SpritePrep_BigKey_load_graphics(k);
}

void SpritePrep_BigKey_load_graphics(int k) {  // 869256
  DecodeAnimatedSpriteTile_variable(0x22);
  SpritePrep_KeySetItemDrop(k);
}

void SpritePrep_SmallKey(int k) {  // 869262
  sprite_subtype[k] = 255;
  sprite_die_action[k] = byte_7E0B9B++;
}

void SpritePrep_KeySetItemDrop(int k) {  // 869267
  sprite_die_action[k] = byte_7E0B9B;
  byte_7E0B9B++;
}

void SpriteActive_Main(int k) {  // 869271
  uint8 type = sprite_type[k];
  kSpriteActiveRoutines[type](k);
}

void Sprite_09_GiantMoldorm(int k) {  // 869469
  static const int8 kGiantMoldorm_Xvel[32] = {
    24, 22, 17,  9, 0,  -9, -17, -22, -24, -22, -17,  -9, 0,  9, 17, 22,
    36, 33, 25, 13, 0, -13, -25, -33, -36, -33, -25, -13, 0, 13, 25, 33,
  };
  static const int8 kGiantMoldorm_Yvel[32] = {
    0,  9, 17, 22, 24, 22, 17,  9, 0,  -9, -17, -22, -24, -22, -17,  -9,
    0, 13, 25, 33, 36, 33, 25, 13, 0, -13, -25, -33, -36, -33, -25, -13,
  };
  static const uint8 kGiantMoldorm_NextDir[16] = {8, 9, 10, 11, 12, 13, 14, 15, 0, 1, 2, 3, 4, 5, 6, 7};
  GiantMoldorm_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (sprite_ai_state[k] == 3) {
    // await death
    if (!sprite_delay_aux4[k]) {
      sprite_state[k] = 4;
      sprite_A[k] = 0;
      sprite_delay_main[k] = 224;
    } else {
      sprite_hit_timer[k] = sprite_delay_aux4[k] | 224;
    }
    return;
  }

  Sprite_CheckDamageFromLink(k);
  bool low_health = (sprite_health[k] < 3);
  sprite_subtype2[k] += low_health ? 2 : 1;
  if (!(frame_counter & (low_health ? 3 : 7)))
    SpriteSfx_QueueSfx3WithPan(k, 0x31);

  if (sprite_F[k]) {
    sprite_delay_aux2[k] = 64;
    if (!(frame_counter & 3))
      sprite_F[k]--;
    return;
  }

  if (!link_incapacitated_timer && Sprite_CheckDamageToLink(k)) {
    Link_CancelDash();
    ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLink(k, 0x28);
    link_actual_vel_y = pt.y;
    link_actual_vel_x = pt.x;
    link_incapacitated_timer = 24;
    sprite_delay_aux1[k] = 48;
    // For some reason they forgot to or in the sfx.
    sound_effect_2 = Sprite_CalculateSfxPan(k) | (enhanced_features0 & kFeatures0_MiscBugFixes ? 0x32 : 0);
  }

  int j = sprite_D[k] + low_health * 16;
  sprite_x_vel[k] = kGiantMoldorm_Xvel[j];
  sprite_y_vel[k] = kGiantMoldorm_Yvel[j];
  Sprite_MoveXY(k);
  if (Sprite_CheckTileCollision(k)) {
    sprite_D[k] = kGiantMoldorm_NextDir[sprite_D[k]];
    SpriteSfx_QueueSfx2WithPan(k, 0x21);
  }
  switch(sprite_ai_state[k]) {
  case 0:  // straight path
    if (!sprite_delay_main[k]) {
      j = 1;
      if (++sprite_G[k] == 3)
        sprite_G[k] = 0, j = 2;
      sprite_ai_state[k] = j;
      sprite_head_dir[k] = (GetRandomNumber() & 2) - 1;
      sprite_delay_main[k] = (GetRandomNumber() & 31) + 32;
    }
    break;
  case 1:  // spinning meander
    if (!sprite_delay_main[k]) {
      sprite_delay_main[k] = (GetRandomNumber() & 15) + 8;
      sprite_ai_state[k] = 0;
    } else if (!(sprite_delay_main[k] & 3)) {
      sprite_D[k] = (sprite_D[k] + sprite_head_dir[k]) & 0xf;
    }
    break;
  case 2:  // lunge at player
    if (!((k ^ frame_counter) & 3)) {
      Sprite_ApplySpeedTowardsLink(k, 0x1f);
      uint8 dir = Sprite_ConvertVelocityToAngle(sprite_x_vel[k], sprite_y_vel[k]) - sprite_D[k];
      if (dir == 0) {
        sprite_ai_state[k] = 0;
        sprite_delay_main[k] = 48;
      } else {
        sprite_D[k] += sign8(dir) ? -1 : 1;
      }
    }
    break;
  }
}

void Sprite_01_Vulture_bounce(int k) {  // 869473
  static const uint8 kVulture_Gfx[4] = {1, 2, 3, 2};
  sprite_obj_prio[k] |= 0x30;
  Vulture_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (Sprite_ReturnIfRecoiling(k))
    return;
  Sprite_CheckDamageToAndFromLink(k);
  Sprite_MoveXY(k);
  switch (sprite_ai_state[k]) {
  case 0:  // dormant
    if (++sprite_subtype2[k] == 160) {
      sprite_ai_state[k] = 1;
      SpriteSfx_QueueSfx3WithPan(k, 0x1e);
      sprite_delay_main[k] = 16;
    }
    break;
  case 1: { // circling
    sprite_graphics[k] = kVulture_Gfx[frame_counter >> 1 & 3];
    if (sprite_delay_main[k]) {
      sprite_z[k]++;
      return;
    }
    if ((k ^ frame_counter) & 1)
      return;
    ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLink(k, (k & 0xf) + 24);
    sprite_x_vel[k] = -pt.y;
    sprite_y_vel[k] = pt.x;
    if ((uint8)(pt.xdiff + 0x28) < 0x50 && (uint8)(pt.ydiff + 0x28) < 0x50)
      return;
    sprite_y_vel[k] += (int8)pt.y >> 2;
    sprite_x_vel[k] += (int8)pt.x >> 2;
    break;
  }
  }
}

void Sprite_27_Deadrock(int k) {  // 86948a
  static const uint8 kDeadRock_Gfx[9] = {0, 1, 0, 1, 2, 2, 3, 3, 4};
  static const uint8 kDeadRock_OamFlags[9] = {0x40, 0x40, 0, 0, 0, 0x40, 0, 0x40, 0};
  static const int8 kDeadRock_Xvel[4] = {32, -32, 0, 0};
  static const int8 kDeadRock_Yvel[4] = {0, 0, 32, -32};
  int j = (sprite_delay_aux2[k] ? (sprite_delay_aux2[k] & 4) : (sprite_ai_state[k] != 2)) ? sprite_A[k] : 8;
  sprite_graphics[k] = kDeadRock_Gfx[j];
  sprite_oam_flags[k] = sprite_oam_flags[k] & ~0x40 | kDeadRock_OamFlags[j];
  SpriteDraw_SingleLarge(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (!sprite_F[k] && (Sprite_CheckDamageFromLink(k) & kCheckDamageFromPlayer_Carry) && !sound_effect_1)
    SpriteSfx_QueueSfx2WithPan(k, 0xb);
  if (Sprite_CheckDamageToLink_same_layer(k)) {
    Sprite_NullifyHookshotDrag();
    Sprite_RepelDash();
  }
  if (sprite_F[k] == 14) {
    sprite_ai_state[k] = 2;
    sprite_delay_aux1[k] = 255;
    sprite_delay_aux2[k] = 64;
  }
  if (Sprite_ReturnIfRecoiling(k))
    return;
  switch(sprite_ai_state[k]) {
  case 0:  // pick dir
    if (!sprite_delay_main[k]) {
      sprite_flags2[k] &= ~0x80;
      sprite_defl_bits[k] &= ~4;
      sprite_flags3[k] &= ~0x40;
      sprite_ai_state[k]++;
      sprite_delay_main[k] = (GetRandomNumber() & 31) + 32;
      if (++sprite_B[k] == 4) {
        sprite_B[k] = 0;
        j = Sprite_DirectionToFaceLink(k, NULL);
      } else {
        j = GetRandomNumber() & 3;
      }
set_dir:
      sprite_D[k] = j;
      sprite_x_vel[k] = kDeadRock_Xvel[j];
      sprite_y_vel[k] = kDeadRock_Yvel[j];
    }
    break;
  case 1:  // walk
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k] = 0;
      sprite_delay_main[k] = 32;
    } else {
      Sprite_MoveXY(k);
      if (Sprite_CheckTileCollision(k)) {
        j = sprite_D[k] ^ 1;
        goto set_dir;
      }
      sprite_A[k] = sprite_D[k] << 1 | ++sprite_subtype2[k] >> 2 & 1;
    }
    break;
  case 2:  // petrified
    sprite_flags2[k] |= 0x80;
    sprite_defl_bits[k] |= 4;
    sprite_flags3[k] |= 0x40;
    if (!(frame_counter & 1)) {
      if (sprite_delay_aux1[k] == 0) {
        sprite_ai_state[k] = 0;
        sprite_delay_main[k] = 16;
      } else if (sprite_delay_aux1[k] == 0x20) {
        sprite_delay_aux2[k] = 0x40;
      }
    } else {
      sprite_delay_aux1[k]++;
    }
    break;
  }
}

void Sprite_20_Sluggula(int k) {  // 8695d9
  static const uint8 kSluggula_Gfx[8] = {0, 1, 0, 1, 2, 3, 4, 5};
  static const uint8 kSluggula_OamFlags[8] = {0x40, 0x40, 0, 0, 0, 0, 0, 0};
  static const int8 kSluggula_XYvel[6] = {16, -16, 0, 0, 16, -16};
  int j = sprite_D[k] << 1 | (sprite_subtype2[k] & 8) >> 3;
  sprite_graphics[k] = kSluggula_Gfx[j];
  sprite_oam_flags[k] = sprite_oam_flags[k] & 191 | kSluggula_OamFlags[j];
  SpriteDraw_SingleLarge(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (Sprite_ReturnIfRecoiling(k))
    return;
  Sprite_CheckDamageToAndFromLink(k);
  sprite_subtype2[k]++;
  switch(sprite_ai_state[k]) {
  case 0:  // normal
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k] = 1;
      sprite_delay_main[k] = (GetRandomNumber() & 31) + 32;
      sprite_D[k] = j = sprite_delay_main[k] & 3;
set_vel:
      sprite_x_vel[k] = kSluggula_XYvel[j];
      sprite_y_vel[k] = kSluggula_XYvel[j + 2];
    } else if (sprite_delay_main[k] == 16 && !(GetRandomNumber() & 1)) {
      Sluggula_DropBomb(k);
    }
    break;
  case 1:  // break from bombing
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k] = 0;
      sprite_delay_main[k] = 32;
    }
    Sprite_MoveXY(k);
    if (!Sprite_CheckTileCollision(k))
      return;
    j = (sprite_D[k] ^= 1);
    goto set_vel;
  }
}

void Sluggula_DropBomb(int k) {  // 869673
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamicallyEx(k, 0x4a, &info, 11);
  if (j >= 0) {
    Sprite_SetSpawnedCoordinates(j, &info);
    Sprite_TransmuteToBomb(j);
  }
}

void Sprite_19_Poe(int k) {  // 869688
  static const int8 kPoe_Accel[4] = {1, -1, 2, -2};
  static const int8 kPoe_ZvelTarget[2] = {8, -8};
  static const int8 kPoe_XvelTarget[4] = {16, -16, 28, -28};
  static const uint8 kPoe_OamFlags[2] = {0x40, 0};
  static const int8 kPoe_Yvel[2] = {8, -8};
  int j;
  sprite_D[k] = j = sprite_x_vel[k] >> 7;
  sprite_oam_flags[k] = sprite_oam_flags[k] & ~0x40 | kPoe_OamFlags[j];
  if (!sprite_E[k])
    sprite_obj_prio[k] |= 0x30;
  Poe_Draw(k);
  oam_cur_ptr += 4;
  oam_ext_cur_ptr++;
  sprite_flags2[k]--;
  SpriteDraw_SingleLarge(k);
  sprite_flags2[k]++;
  if (Sprite_ReturnIfInactive(k))
    return;
  if (Sprite_ReturnIfRecoiling(k))
    return;
  if (sprite_E[k]) {
    if (++sprite_z[k] == 12)
      sprite_E[k] = 0;
    return;
  }
  Sprite_CheckDamageToAndFromLink(k);
  sprite_subtype2[k]++;
  Sprite_MoveXY(k);
  if (!(frame_counter & 1)) {
    j = sprite_G[k] & 1;
    sprite_z_vel[k] += kPoe_Accel[j];
    if (sprite_z_vel[k] == (uint8)kPoe_ZvelTarget[j])
      sprite_G[k]++;
  }
  Sprite_MoveZ(k);
  sprite_y_vel[k] = 0;
  switch(sprite_ai_state[k]) {
  case 0:  // select vertical dir
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k] = 1;
      if (!(GetRandomNumber() & 12))
        sprite_head_dir[k] = Sprite_IsBelowLink(k).a;
      else
        sprite_head_dir[k] = GetRandomNumber() & 1;
    }
    break;
  case 1:  // roaming
    if (!(frame_counter & 1)) {
      int j = (sprite_anim_clock[k] & 1) + is_in_dark_world * 2;
      sprite_x_vel[k] += kPoe_Accel[j];
      if (sprite_x_vel[k] == (uint8)kPoe_XvelTarget[j]) {
        sprite_anim_clock[k]++;
        sprite_ai_state[k] = 0;
        sprite_delay_main[k] = (GetRandomNumber() & 31) + 16;
      }
    }
    sprite_y_vel[k] = kPoe_Yvel[sprite_head_dir[k]];
    break;
  }
}

void Poe_Draw(int k) {  // 869786
  static const int8 kPoe_Draw_X[2] = {9, -1};
  static const uint8 kPoe_Draw_Char[4] = {0x7c, 0x80, 0xb7, 0x80};
  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  OamEnt *oam = GetOamCurPtr();
  SetOamHelper0(oam, info.x + kPoe_Draw_X[sprite_D[k]], info.y + 9,
                kPoe_Draw_Char[sprite_subtype2[k] >> 3 & 3], info.flags & 0xf0 | 2, 0);
}

void Sprite_18_MiniMoldorm(int k) {  // 869808
  static const int8 kMoldorm_Xvel[16] = {24, 22, 17, 9, 0, -9, -17, -22, -24, -22, -17, -9, 0, 9, 17, 22};
  static const int8 kMoldorm_Yvel[16] = {0, 9, 17, 22, 24, 22, 17, 9, 0, -9, -17, -22, -24, -22, -17, -9};
  static const uint8 kMoldorm_NextDir[16] = {8, 9, 10, 11, 12, 13, 14, 15, 0, 1, 2, 3, 4, 5, 6, 7};

  Moldorm_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (sprite_F[k])
    SpritePrep_MiniMoldorm_bounce(k);
  if (Sprite_ReturnIfRecoiling(k))
    return;
  Sprite_CheckDamageToAndFromLink(k);
  sprite_subtype2[k]++;
  int j = sprite_D[k];
  sprite_x_vel[k] = kMoldorm_Xvel[j];
  sprite_y_vel[k] = kMoldorm_Yvel[j];
  Sprite_MoveXY(k);
  if (Sprite_CheckTileCollision(k)) {
    if (GetRandomNumber() & 1)
      sprite_head_dir[k] = -sprite_head_dir[k];
    sprite_D[k] = kMoldorm_NextDir[sprite_D[k]];
  }
  switch (sprite_ai_state[k]) {
  case 0:  // configure
    if (!sprite_delay_main[k]) {
      if (++sprite_G[k] == 6)
        sprite_G[k] = 0, sprite_ai_state[k] = 2;
      else
        sprite_ai_state[k] = 1;
      sprite_head_dir[k] = (GetRandomNumber() & 2) - 1;
      sprite_delay_main[k] = (GetRandomNumber() & 0x1f) + 0x20;
    }
    break;
  case 1:  // meander
    if (!sprite_delay_main[k]) {
      sprite_delay_main[k] = (GetRandomNumber() & 15) + 8;
      sprite_ai_state[k] = 0;
    } else if ((sprite_delay_main[k] & 3) == 0) {
      sprite_D[k] = (sprite_D[k] + sprite_head_dir[k]) & 0xf;
    }
    break;
  case 2:  // seek player
    if (!((k ^ frame_counter) & 3)) {
      Sprite_ApplySpeedTowardsLink(k, 31);
      uint8 d = Sprite_ConvertVelocityToAngle(sprite_x_vel[k], sprite_y_vel[k]) - sprite_D[k];
      if (d == 0) {
        sprite_ai_state[k] = 0;
        sprite_delay_main[k] = 48;
      } else {
        sprite_D[k] = (sprite_D[k] + (sign8(d) ? -1 : 1)) & 0xf;
      }
    }
    break;
  }
}

void Sprite_12_Moblin(int k) {  // 8698e4
  static const int8 kMoblin_Xvel[4] = {16, -16, 0, 0};
  static const int8 kMoblin_Yvel[4] = {0, 0, 16, -16};
  static const uint8 kMoblin_Delay[4] = {0x10, 0x20, 0x30, 0x40};
  static const uint8 kMoblin_Gfx2[8] = {11, 10, 8, 9, 7, 5, 0, 2};
  static const uint8 kMoblin_Dirs[8] = {2, 3, 2, 3, 0, 1, 0, 1};
  static const uint8 kMoblin_Gfx[4] = {6, 4, 0, 2};
  int j;
  Moblin_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (Sprite_ReturnIfRecoiling(k))
    return;
  Sprite_CheckDamageToAndFromLink(k);
  Sprite_MoveXY(k);
  Sprite_CheckTileCollision2(k);
  switch(sprite_ai_state[k]) {
  case 0:  // select dir
    if (!sprite_delay_main[k]) {
      sprite_delay_main[k] = kMoblin_Delay[GetRandomNumber() & 3];
      sprite_ai_state[k]++;
      sprite_D[k] = sprite_head_dir[k];
      j = sprite_head_dir[k];
      sprite_x_vel[k] = kMoblin_Xvel[j];
      sprite_y_vel[k] = kMoblin_Yvel[j];
    }
    break;
  case 1:  // walk
    sprite_graphics[k] = (sprite_subtype2[k] & 1) + kMoblin_Gfx[sprite_D[k]];
    if (!sprite_wallcoll[k]) {
      if (sprite_delay_main[k]) {
        if (sign8(--sprite_E[k])) {
          sprite_E[k] = 11;
          sprite_subtype2[k]++;
        }
        return;
      }
      if (sprite_D[k] == Sprite_DirectionToFaceLink(k, NULL)) {
        sprite_ai_state[k]++;
        sprite_delay_main[k] = 32;
        Sprite_ZeroVelocity_XY(k);
        sprite_z_vel[k] = 0;
        return;
      }
      sprite_delay_main[k] = 0x10;
    } else {
      sprite_delay_main[k] = 0xc;
    }
    sprite_head_dir[k] = kMoblin_Dirs[sprite_D[k] << 1 | GetRandomNumber() & 1];
    sprite_ai_state[k] = 0;
    if (++sprite_C[k] == 4) {
      sprite_C[k] = 0;
      sprite_head_dir[k] = Sprite_DirectionToFaceLink(k, NULL);
    }
    Sprite_ZeroVelocity_XY(k);
    sprite_z_vel[k] = 0;
    break;
  case 2:  // throw spear
    j = sprite_D[k];
    if (!sprite_delay_main[k])
      sprite_ai_state[k] = 0;
    if (sprite_delay_main[k] < 16) {
      if (sprite_delay_main[k] == 15) {
        Moblin_MaterializeSpear(k);
        sprite_delay_aux1[k] = 32;
      }
      j += 4;
    }
    sprite_graphics[k] = kMoblin_Gfx2[j];
    break;
  }
}

void Moblin_MaterializeSpear(int k) {  // 8699eb
  static const int8 kMoblinSpear_X[4] = {11, -2, -3, 11};
  static const int8 kMoblinSpear_Y[4] = {-3, -3, 3, -11};
  static const int8 kMoblinSpear_Xvel[4] = {32, -32, 0, 0};
  static const int8 kMoblinSpear_Yvel[4] = {0, 0, 32, -32};
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0x1b, &info), i;
  if (j >= 0) {
    sprite_A[j] = 3;
    sprite_D[j] = i = sprite_D[k];
    Sprite_SetX(j, info.r0_x + kMoblinSpear_X[i]);
    Sprite_SetY(j, info.r2_y + kMoblinSpear_Y[i]);
    sprite_x_vel[j] = kMoblinSpear_Xvel[i];
    sprite_y_vel[j] = kMoblinSpear_Yvel[i];
  }
}

void Moblin_Draw(int k) {  // 869bc4
  static const DrawMultipleData kMoblin_Dmd[48] = {
    {-2,   3, 0x8091, 0},
    {-2,  11, 0x8090, 0},
    { 0, -10, 0x0086, 2},
    { 0,   0, 0x008a, 2},
    {-2,   7, 0x8091, 0},
    {-2,  15, 0x8090, 0},
    { 0, -10, 0x0086, 2},
    { 0,   0, 0x408a, 2},
    { 0,  -9, 0x0084, 2},
    { 0,   0, 0x00a0, 2},
    {11,  -5, 0x0090, 0},
    {11,   3, 0x0091, 0},
    { 0,  -9, 0x0084, 2},
    { 0,   0, 0x40a0, 2},
    {11,  -8, 0x0090, 0},
    {11,   0, 0x0091, 0},
    {-4,   8, 0x0080, 0},
    { 4,   8, 0x0081, 0},
    { 0,  -9, 0x0088, 2},
    { 0,   0, 0x00a6, 2},
    {-9,   6, 0x0080, 0},
    {-1,   6, 0x0081, 0},
    { 0,  -8, 0x0088, 2},
    { 0,   0, 0x00a4, 2},
    {12,   8, 0x4080, 0},
    { 4,   8, 0x4081, 0},
    { 0,  -9, 0x4088, 2},
    { 0,   0, 0x40a6, 2},
    {17,   6, 0x4080, 0},
    { 9,   6, 0x4081, 0},
    { 0,  -8, 0x4088, 2},
    { 0,   0, 0x40a4, 2},
    {-3,  -5, 0x8091, 0},
    {-3,   3, 0x8090, 0},
    { 0, -10, 0x0086, 2},
    { 0,   0, 0x00a8, 2},
    {11, -11, 0x0090, 0},
    {11,  -3, 0x0091, 0},
    { 0,  -9, 0x0084, 2},
    { 0,   0, 0x4082, 2},
    {-2,  -3, 0x0080, 0},
    { 6,  -3, 0x0081, 0},
    { 0,  -9, 0x0088, 2},
    { 0,   0, 0x00a2, 2},
    {10,  -3, 0x4080, 0},
    { 2,  -3, 0x4081, 0},
    { 0,  -9, 0x4088, 2},
    { 0,   0, 0x40a2, 2},
  };
  static const uint8 kMoblin_ObjOffs[12] = {2, 2, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2};
  static const uint8 kMoblin_HeadChar[4] = {0x88, 0x88, 0x86, 0x84};
  static const uint8 kMoblin_HeadFlags[4] = {0x40, 0, 0, 0};
  PrepOamCoordsRet info;
  Sprite_DrawMultiple(k, &kMoblin_Dmd[sprite_graphics[k] * 4], 4, &info);
  if (sprite_pause[k])
    return;
  OamEnt *oam = GetOamCurPtr();
  if (sprite_delay_aux1[k]) {
    for (int i = 0; i < 4; i++, oam++) {
      if (!(bytewise_extended_oam[oam - oam_buf] & 2))
        oam->y = 0xf0;
    }
  }
  oam = GetOamCurPtr() + kMoblin_ObjOffs[sprite_graphics[k]];
  int j = sprite_head_dir[k];
  oam->charnum = kMoblin_HeadChar[j];
  oam->flags = oam->flags &~0x40 | kMoblin_HeadFlags[j];
  SpriteDraw_Shadow(k, &info);
}

void Sprite_0E_Snapdragon(int k) {  // 869c24
  static const uint8 kSnapDragon_Delay[4] = {0x20, 0x30, 0x40, 0x50};
  static const uint8 kSnapDragon_Gfx[4] = {4, 0, 6, 2};
  static const int8 kSnapDragon_Xvel[8] = {8, -8, 8, -8, 16, -16, 16, -16};
  static const int8 kSnapDragon_Yvel[8] = {8, 8, -8, -8, 16, 16, -16, -16};
  int j;
  sprite_graphics[k] = sprite_B[k] + kSnapDragon_Gfx[sprite_D[k]];
  SnapDragon_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (Sprite_ReturnIfRecoiling(k))
    return;
  Sprite_CheckDamageToAndFromLink(k);
  sprite_B[k] = 0;
  switch(sprite_ai_state[k]) {
  case 0:  // resting
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k]++;
      sprite_delay_main[k] = kSnapDragon_Delay[(GetRandomNumber() & 12) >> 2];
      if (sign8(--sprite_A[k])) {
        sprite_A[k] = 3;
        sprite_delay_main[k] = 96;
        sprite_C[k]++;
        sprite_D[k] = Sprite_IsBelowLink(k).a * 2 + Sprite_IsRightOfLink(k).a;
      } else {
        sprite_D[k] = GetRandomNumber() & 3;
      }
    } else if (sprite_delay_main[k] & 0x18) {
      sprite_B[k]++;
    }
    break;
  case 1:  // attack
    sprite_B[k]++;
    Sprite_MoveXY(k);
    if (Sprite_CheckTileCollision(k))
      sprite_D[k] ^= 3;
    j = sprite_D[k] + (sprite_C[k] ? 4 : 0);
    sprite_x_vel[k] = kSnapDragon_Xvel[j];
    sprite_y_vel[k] = kSnapDragon_Yvel[j];
    Sprite_MoveZ(k);
    sprite_z_vel[k] -= 4;
    if (sign8(sprite_z[k])) {
      sprite_z[k] = 0;
      if (!sprite_delay_main[k]) {
        sprite_ai_state[k] = 0;
        sprite_C[k] = 0;
        sprite_delay_main[k] = 63;
      } else {
        sprite_z_vel[k] = 20;
      }
    }
    break;
  }
}

void SnapDragon_Draw(int k) {  // 869e02
  static const DrawMultipleData kSnapDragon_Dmd[32] = {
    { 4, -8, 0x008f, 0},
    {12, -8, 0x009f, 0},
    {-4,  0, 0x008c, 2},
    { 4,  0, 0x008d, 2},
    { 4, -8, 0x002b, 0},
    {12, -8, 0x003b, 0},
    {-4,  0, 0x0028, 2},
    { 4,  0, 0x0029, 2},
    {-4, -8, 0x003c, 0},
    { 4, -8, 0x003d, 0},
    {-4,  0, 0x00aa, 2},
    { 4,  0, 0x00ab, 2},
    {-4, -8, 0x003e, 0},
    { 4, -8, 0x003f, 0},
    {-4,  0, 0x00ad, 2},
    { 4,  0, 0x00ae, 2},
    {-4, -8, 0x409f, 0},
    { 4, -8, 0x408f, 0},
    {-4,  0, 0x408d, 2},
    { 4,  0, 0x408c, 2},
    {-4, -8, 0x403b, 0},
    { 4, -8, 0x402b, 0},
    {-4,  0, 0x4029, 2},
    { 4,  0, 0x4028, 2},
    { 4, -8, 0x403d, 0},
    {12, -8, 0x403c, 0},
    {-4,  0, 0x40ab, 2},
    { 4,  0, 0x40aa, 2},
    { 4, -8, 0x403f, 0},
    {12, -8, 0x403e, 0},
    {-4,  0, 0x40ae, 2},
    { 4,  0, 0x40ad, 2},
  };
  PrepOamCoordsRet info;
  Sprite_DrawMultiple(k, &kSnapDragon_Dmd[sprite_graphics[k] * 4], 4, &info);
  SpriteDraw_Shadow(k, &info);
}

void Sprite_22_Ropa(int k) {  // 869e1f
  Ropa_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (Sprite_ReturnIfRecoiling(k))
    return;
  Sprite_CheckDamageToAndFromLink(k);
  sprite_subtype2[k]++;
  sprite_graphics[k] = sprite_subtype2[k] >> 3 & 3;
  switch(sprite_ai_state[k]) {
  case 0:  // stationary
    if (!sprite_delay_main[k]) {
      Sprite_ApplySpeedTowardsLink(k, 16);
      sprite_z_vel[k] = (GetRandomNumber() & 15) + 20;
      sprite_ai_state[k]++;
    }
    break;
  case 1:  // pounce
    Sprite_MoveXY(k);
    if (Sprite_CheckTileCollision(k))
      Sprite_ZeroVelocity_XY(k);
    Sprite_MoveZ(k);
    sprite_z_vel[k] -= 2;
    if (sign8(sprite_z[k])) {
      sprite_z[k] = 0;
      sprite_delay_main[k] = 48;
      sprite_ai_state[k] = 0;
    }
    break;
  }
}

void Ropa_Draw(int k) {  // 869ee5
  static const DrawMultipleData kRopa_Dmd[12] = {
    {0, -8, 0x0026, 0},
    {8, -8, 0x0027, 0},
    {0,  0, 0x0008, 2},
    {0, -8, 0x0036, 0},
    {8, -8, 0x0037, 0},
    {0,  0, 0x000a, 2},
    {0, -8, 0x4027, 0},
    {8, -8, 0x4026, 0},
    {0,  0, 0x4008, 2},
    {0, -8, 0x4037, 0},
    {8, -8, 0x4036, 0},
    {0,  0, 0x4008, 2},
  };
  PrepOamCoordsRet info;
  Sprite_DrawMultiple(k, &kRopa_Dmd[sprite_graphics[k] * 3], 3, &info);
  SpriteDraw_Shadow(k, &info);

}

void Sprite_11_Hinox(int k) {  // 869f05
  Hinox_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (sprite_F[k]) {
    Hinox_FaceLink(k);
    sprite_ai_state[k] = 2;
    sprite_delay_main[k] = 48;
  }
  if (Sprite_ReturnIfRecoiling(k))
    return;
  Sprite_CheckDamageToAndFromLink(k);
  switch(sprite_ai_state[k]) {
  case 0:  // select dir
    if (!sprite_delay_main[k]) {
      if (!(GetRandomNumber() & 3)) {
        sprite_ai_state[k] = 2;
        sprite_delay_main[k] = 64;
      } else {
        if (++sprite_C[k] == 4) {
          sprite_C[k] = 0;
          Hinox_FaceLink(k);
        } else {
          static const uint8 kHinox_RandomDirs[8] = {2, 3, 3, 2, 0, 1, 1, 0};
          Hinox_SetDirection(k, kHinox_RandomDirs[sprite_D[k] * 2 + (GetRandomNumber() & 1)]);
        }
      }
    }
    break;
  case 1:  // walk
    if (sprite_delay_main[k]) {
      if (sign8(--sprite_A[k])) {
        sprite_A[k] = 11;
        sprite_subtype2[k]++;
      }
      Sprite_MoveXY(k);
      if (!Sprite_CheckTileCollision(k)) {
        static const uint8 kHinox_WalkGfx[4] = {6, 4, 0, 2};
        sprite_graphics[k] = kHinox_WalkGfx[sprite_D[k]] + (sprite_subtype2[k] & 1);
        return;
      }
    }
    sprite_delay_main[k] = 16;
    sprite_ai_state[k] = 0;
    break;
  case 2:  // throw bomb
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k] = 0;
      sprite_delay_main[k] = 2;
      return;
    }
    if (sprite_delay_main[k] == 32) {
      static const int8 kHinox_BombX[4] = {8, -8, -13, 13};
      static const int8 kHinox_BombY[4] = {-11, -11, -16, -16};
      static const int8 kHinox_BombXvel[4] = {24, -24, 0, 0};
      static const int8 kHinox_BombYvel[4] = {0, 0, 24, -24};
      SpriteSpawnInfo info;
      int j = Sprite_SpawnDynamically(k, 0x4a, &info);
      if (j >= 0) {
        Sprite_TransmuteToBomb(j);
        sprite_delay_aux1[j] = 64;
        int i = sprite_D[k];
        Sprite_SetX(j, info.r0_x + kHinox_BombX[i]);
        Sprite_SetY(j, info.r2_y + kHinox_BombY[i]);
        sprite_x_vel[j] = kHinox_BombXvel[i];
        sprite_y_vel[j] = kHinox_BombYvel[i];
        sprite_z_vel[j] = 40;
      }
    } else {
      static const uint8 kHinox_Gfx[8] = {11, 10, 8, 9, 7, 5, 1, 3};
      sprite_graphics[k] = kHinox_Gfx[sprite_D[k] + (sprite_delay_main[k] < 32 ? 4 : 0)];
    }
    break;
  }

}

void Hinox_ThrowBomb(int k) {  // 869f4a

}

void Hinox_FaceLink(int k) {  // 869fe1
  Hinox_SetDirection(k, Sprite_DirectionToFaceLink(k, NULL));
  sprite_x_vel[k] <<= 1;
  sprite_y_vel[k] <<= 1;
}

void Hinox_SetDirection(int k, uint8 dir) {  // 86a004
  static const int8 kHinox_Xvel[4] = {8, -8, 0, 0};
  static const int8 kHinox_Yvel[4] = {0, 0, 8, -8};
  sprite_D[k] = dir;
  sprite_delay_main[k] = (GetRandomNumber() & 63) + 96;
  sprite_ai_state[k]++;
  sprite_x_vel[k] = kHinox_Xvel[dir];
  sprite_y_vel[k] = kHinox_Yvel[dir];
}

void Hinox_Draw(int k) {  // 86a1f9
  static const DrawMultipleData kHinox_Dmd[46] = {
    {  0, -13, 0x0600, 2},
    { -8,  -5, 0x0624, 2},
    {  8,  -5, 0x4624, 2},
    {  0,   1, 0x0606, 2},
    {  0, -13, 0x0600, 2},
    { -8,  -5, 0x0624, 2},
    {  8,  -5, 0x4624, 2},
    {  0,   1, 0x4606, 2},
    { -8,  -6, 0x0624, 2},
    {  8,  -6, 0x4624, 2},
    {  0,   0, 0x0606, 2},
    {  0, -13, 0x0604, 2},
    { -8,  -6, 0x0624, 2},
    {  8,  -6, 0x4624, 2},
    {  0,   0, 0x4606, 2},
    {  0, -13, 0x0604, 2},
    { -3, -13, 0x0602, 2},
    {  0,  -8, 0x060c, 2},
    {  0,   0, 0x061c, 2},
    { -3, -12, 0x0602, 2},
    {  0,  -8, 0x060e, 2},
    {  0,   0, 0x061e, 2},
    {  3, -13, 0x4602, 2},
    {  0,  -8, 0x460c, 2},
    {  0,   0, 0x461c, 2},
    {  3, -12, 0x4602, 2},
    {  0,  -8, 0x460e, 2},
    {  0,   0, 0x461e, 2},
    {-13, -16, 0x056e, 2},
    {  0, -13, 0x0600, 2},
    { -8,  -5, 0x0620, 2},
    {  8,  -5, 0x4624, 2},
    {  0,   1, 0x0606, 2},
    { -8,  -5, 0x0624, 2},
    {  8,  -5, 0x4620, 2},
    {  0,   1, 0x0606, 2},
    {  0, -13, 0x0604, 2},
    { 13, -16, 0x056e, 2},
    { -8, -11, 0x056e, 2},
    { -3, -13, 0x0602, 2},
    {  0,   0, 0x0622, 2},
    {  0,  -8, 0x060c, 2},
    {  8, -11, 0x056e, 2},
    {  3, -13, 0x4602, 2},
    {  0,   0, 0x4622, 2},
    {  0,  -8, 0x460c, 2},
  };
  PrepOamCoordsRet info;
  static const uint8 kHinoxNum[12] = { 4, 4, 4, 4, 3, 3, 3, 3, 5, 5, 4, 4 };
  static const uint8 kHinoxOffs[12] = { 0, 4, 8, 12, 16, 19, 22, 25, 28, 33, 38, 42 };
  int j = sprite_graphics[k];
  Sprite_DrawMultiple(k, &kHinox_Dmd[kHinoxOffs[j]], kHinoxNum[j], &info);
  SpriteDraw_Shadow(k, &info);
}

void Sprite_23_RedBari(int k) {  // 86a23d
  static const int8 kBari_Xvel2[16] = {0, 8, 11, 14, 16, 14, 11, 8, 0, -8, -11, -14, -16, -14, -11, -8};
  static const int8 kBari_Yvel2[16] = {-16, -14, -11, -8, 0, 8, 11, 14, 16, 14, 11, 8, 0, -9, -11, -14};
  static const uint8 kBari_Gfx[2] = {0, 3};
  int j;

  if (sign8(sprite_C[k])) {
    if (sprite_head_dir[k] != 16) {
      sprite_head_dir[k]++;
    } else {
      sprite_x_vel[k] = 255;
      sprite_subtype[k] = 255;
      Sprite_CheckTileCollision2(k);
      sprite_subtype[k] = 0;
      if (!sprite_tiletype) {
        sprite_C[k] = 0;
        sprite_ignore_projectile[k] = 0;
        goto set_electrocute_delay;
      }
      sprite_ignore_projectile[k] = sprite_tiletype;
    }
    return;
  }
  if (sprite_C[k] != 0) {
    SpriteDraw_SingleSmall(k);
  } else if (sprite_graphics[k] >= 2) {
    SpriteDraw_SingleLarge(k);
  } else {
    RedBari_Draw(k);
  }

  if (Sprite_ReturnIfInactive(k))
    return;
  if (Sprite_ReturnIfRecoiling(k))
    return;
  if (sprite_delay_aux2[k])
    goto recoil_from_split;

  if (sprite_ai_state[k] == 2) {
    static const int8 kBari_Xvel[2] = {8, -8};
    sprite_ignore_projectile[k] = sprite_ai_state[k];
    sprite_x_vel[k] = kBari_Xvel[frame_counter >> 1 & 1];
    Sprite_MoveX(k);
    if (!sprite_delay_main[k]) {
      RedBari_Split(k);
      sprite_state[k] = 0;
    }
    return;
  }

  Sprite_CheckDamageToAndFromLink(k);
  if (!((k ^ frame_counter) & 15)) {
    sprite_A[k] += (sprite_B[k] & 1) ? -1 : 1;
    if (!(GetRandomNumber() & 3))
      sprite_B[k]++;
  }
  j = sprite_A[k] & 15;
  sprite_x_vel[k] = kBari_Xvel2[j];
  sprite_y_vel[k] = kBari_Yvel2[j];

  if (!((k ^ frame_counter) & 3 | sprite_delay_main[k])) {
recoil_from_split:
    if (!sprite_wallcoll[k])
      Sprite_MoveXY(k);
    Sprite_CheckTileCollision2(k);
  }
  j = sprite_C[k];
  sprite_graphics[k] = (frame_counter >> 3 & 1) + kBari_Gfx[j];
  if (sprite_ai_state[k]) {
    if (sprite_delay_main[k]) {
      sprite_graphics[k] = (frame_counter >> 1 & 2) + kBari_Gfx[j];
      return;
    }
    sprite_ai_state[k] = 0;
  } else if (sprite_delay_aux1[k]) {
    return;
  } else if (!(GetRandomNumber() & 1)) {
    sprite_delay_main[k] = 128;
    sprite_ai_state[k]++;
    return;
  }
set_electrocute_delay:
  sprite_delay_aux1[k] = (GetRandomNumber() & 63) + 128;
}

void RedBari_Split(int k) {  // 86a34e
  static const int8 kRedBari_SplitX[2] = {0, 8};
  static const int8 kRedBari_SplitXvel[2] = {-32, 32};

  tmp_counter = 1;
  do {
    SpriteSpawnInfo info;
    int j = Sprite_SpawnDynamically(k, 0x23, &info);
    if (j >= 0) {
      Sprite_SetSpawnedCoordinates(j, &info);
      sprite_flags3[j] = 0x33;
      sprite_oam_flags[j] = 3;
      sprite_flags4[j] = 1;
      sprite_C[j] = 1;
      Sprite_SetX(j, info.r0_x + kRedBari_SplitX[tmp_counter]);
      sprite_x_vel[j] = kRedBari_SplitXvel[tmp_counter];
      sprite_delay_aux2[j] = 8;
      sprite_delay_aux1[j] = 64;
    }
  } while (!sign8(--tmp_counter));

}

void RedBari_Draw(int k) {  // 86a3dc
  static const DrawMultipleData kRedBari_Dmd[8] = {
    {0, 0, 0x0022, 0},
    {8, 0, 0x4022, 0},
    {0, 8, 0x0032, 0},
    {8, 8, 0x4032, 0},
    {0, 0, 0x0023, 0},
    {8, 0, 0x4023, 0},
    {0, 8, 0x0033, 0},
    {8, 8, 0x4033, 0},
  };
  PrepOamCoordsRet info;
  Sprite_DrawMultiple(k, &kRedBari_Dmd[sprite_graphics[k] * 4], 4, &info);
  SpriteDraw_Shadow(k, &info);
}

void Sprite_13_MiniHelmasaur(int k) {  // 86a409
  static const uint8 kHelmasaur_Gfx[8] = {3, 4, 3, 4, 2, 2, 5, 5};
  static const uint8 kHelmasaur_OamFlags[8] = {0x40, 0x40, 0, 0, 0, 0x40, 0x40, 0};
  int j = sprite_subtype2[k] >> 2 & 1 | sprite_D[k] << 1;
  sprite_graphics[k] = kHelmasaur_Gfx[j];
  sprite_oam_flags[k] = sprite_oam_flags[k] & ~0x40 | kHelmasaur_OamFlags[j];
  if (!((k ^ frame_counter) & 15)) {
    uint8 x = sprite_x_vel[k];
    if (sign8(x)) x = -x;
    uint8 y = sprite_y_vel[k];
    if (sign8(y)) y = -y;
    sprite_D[k] = (x >= y) ? (sprite_x_vel[k] >> 7) : (sprite_y_vel[k] >> 7) + 2;
  }
  SpriteDraw_SingleLarge(k);
  HelmasaurHardHatBeetleCommon(k);
}

void Sprite_26_HardhatBeetle(int k) {  // 86a460
  sprite_graphics[k] = sprite_subtype2[k] >> 2 & 1;
  HardHatBeetle_Draw(k);
  HelmasaurHardHatBeetleCommon(k);
}

void HelmasaurHardHatBeetleCommon(int k) {  // 86a46d
  if (Sprite_ReturnIfInactive(k))
    return;
  sprite_subtype2[k]++;
  if (Sprite_ReturnIfRecoiling(k))
    return;
  Sprite_CheckDamageToAndFromLink(k);
  if (sprite_wallcoll[k] & 15) {
    if (sprite_wallcoll[k] & 3)
      sprite_x_vel[k] = 0;
    sprite_y_vel[k] = 0;
  } else {
    Sprite_MoveXY(k);
  }
  Sprite_CheckTileCollision(k);
  if (!((k ^ frame_counter) & 31)) {
    ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLink(k, sprite_A[k]);
    sprite_B[k] = pt.y;
    sprite_C[k] = pt.x;
  }
  if ((k ^ frame_counter) & sprite_ai_state[k])
    return;
  sprite_y_vel[k] += sign8(sprite_y_vel[k] - sprite_B[k]) ? 1 : -1;
  sprite_x_vel[k] += sign8(sprite_x_vel[k] - sprite_C[k]) ? 1 : -1;
}

void HardHatBeetle_Draw(int k) {  // 86a4f2
  static const DrawMultipleData kHardHatBeetle_Dmd[4] = {
    {0, -4, 0x0140, 2},
    {0,  2, 0x0142, 2},
    {0, -5, 0x0140, 2},
    {0,  2, 0x0144, 2},
  };
  PrepOamCoordsRet info;
  Sprite_DrawMultiple(k, &kHardHatBeetle_Dmd[sprite_graphics[k] * 2], 2, &info);
  if (sprite_flags3[k] & 0x10)
    SpriteDraw_Shadow(k, &info);
}

void Sprite_15_Antifairy(int k) {  // 86a50c
  SpriteDraw_Antfairy(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (Sprite_CheckDamageToLink(k) && sprite_delay_main[k] == 0) {
    sprite_delay_main[k] = 16;
    int t = link_magic_power - 8;
    if (t < 0)
      t = 0;
    else
      sound_effect_2 = 0x1d;
    link_magic_power = t;
  }
  Sprite_MoveXY(k);
  Sprite_BounceFromTileCollision(k);
}

void Sprite_0B_Cucco(int k) {  // 86a5c2
  if (sprite_x_vel[k] != 0)
    sprite_oam_flags[k] = sprite_oam_flags[k] & ~0x40 | (sign8(sprite_x_vel[k]) ? 0 : 0x40);

  SpriteDraw_SingleLarge(k);
  if (sprite_head_dir[k] != 0) {
    sprite_type[k] = 0x3d;
    SpritePrep_LoadProperties(k);
    sprite_subtype[k]++;
    sprite_delay_main[k] = 48;
    sound_effect_1 = 21;
    sprite_ignore_projectile[k] = 21;
    return;
  }
  if (sprite_state[k] == 10) {
    sprite_ai_state[k] = 3;
    if (submodule_index == 0) {
      Chicken_IncrSubtype2(k, 3);
      Cucco_DrawPANIC(k);
      if (!(frame_counter & 0xf))
        BawkBawk(k);
    }
  }
  if (Sprite_ReturnIfInactive(k))
    return;
  if (sprite_C[k] != 0) {
    sprite_oam_flags[k] |= 0x10;
    Sprite_MoveXY(k);
    sprite_z[k] = 12;
    sprite_ignore_projectile[k] = 12;
    if (((k ^ frame_counter) & 7) == 0)
      Sprite_CheckDamageToLink(k);
    Chicken_IncrSubtype2(k, 4);
  } else {
    sprite_health[k] = 255;
    if (sprite_B[k] >= 35)
      Cucco_SummonAvenger(k);
    if (sprite_F[k] != 0) {
      sprite_F[k] = 0;
      if (sprite_B[k] < 35) {
        sprite_B[k]++;
        BawkBawk(k);
      }
      sprite_ai_state[k] = 2;
    }
    Sprite_CheckDamageFromLink(k);
    switch (sprite_ai_state[k]) {
    case 0: Cucco_Calm(k); break;
    case 1: Chicken_Hopping(k); break;
    case 2: Cucco_Flee(k); break;
    case 3: Cucco_Carried(k); break;
    }
  }
}

void Cucco_Calm(int k) {  // 86a67f
  if (sprite_delay_main[k] == 0) {
    int j = GetRandomNumber() & 0xf;
    sprite_x_vel[k] = kSpriteKeese_Tab2[j];
    sprite_y_vel[k] = kSpriteKeese_Tab3[j];
    sprite_delay_main[k] = (GetRandomNumber() & 0x1f) + 0x10;
    sprite_ai_state[k]++;
  }
  sprite_graphics[k] = 0;
  Sprite_ReturnIfLifted(k);
}

void Chicken_Hopping(int k) {  // 86a6b1
  if ((k ^ frame_counter) & 1 && Cucco_DoMovement_XY(k))
    sprite_ai_state[k] = 0;
  Sprite_MoveZ(k);
  sprite_z_vel[k] -= 2;
  if (sign8(sprite_z[k])) {
    sprite_z[k] = 0;
    if (sprite_delay_main[k] == 0) {
      sprite_delay_main[k] = 32;
      sprite_ai_state[k] = 0;
    }
    sprite_z_vel[k] = 10;
  }
  Chicken_IncrSubtype2(k, 4);
}

void Cucco_Flee(int k) {  // 86a6fc
  Sprite_ReturnIfLifted(k);
  Cucco_DoMovement_XY(k);
  sprite_z[k] = 0;
  if (!((k ^ frame_counter) & 0x1f)) {
    ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLink(k, 16);
    sprite_x_vel[k] = -pt.x;
    sprite_y_vel[k] = -pt.y;
  }
  Chicken_IncrSubtype2(k, 5);
  Cucco_DrawPANIC(k);
}

void Cucco_DrawPANIC(int k) {  // 86a727
  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  Sprite_DrawDistress_custom(info.x, info.y, frame_counter);
}

void Cucco_Carried(int k) {  // 86a78e
  Sprite_MoveZ(k);
  if (Cucco_DoMovement_XY(k)) {
    sprite_x_vel[k] = -sprite_x_vel[k];
    sprite_y_vel[k] = -sprite_y_vel[k];
    Sprite_MoveXY(k);
    Sprite_HalveSpeed_XY(k);
    Sprite_HalveSpeed_XY(k);
    BawkBawk(k);
  }
  sprite_z_vel[k]--;
  if (sign8(sprite_z[k])) {
    sprite_z[k] = 0;
    sprite_ai_state[k] = 2;
    ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLink(k, 16);
    sprite_x_vel[k] = -pt.x;
    sprite_y_vel[k] = -pt.y;
    Chicken_IncrSubtype2(k, 5);
    Cucco_DrawPANIC(k);
  } else {
    Chicken_IncrSubtype2(k, 4);
  }
}

uint8 Cucco_DoMovement_XY(int k) {  // 86a7bb
  Sprite_MoveXY(k);
  return Sprite_CheckTileCollision(k);
}

void Cucco_SummonAvenger(int k) {  // 86a7d3
  static const uint8 kChicken_Avenger[2] = {0, 0xff};
  if ((k ^ frame_counter) & 0xf | player_is_indoors)
    return;
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamicallyEx(k, 0xB, &info, 10);
  if (j < 0)
    return;
  SpriteSfx_QueueSfx3WithPan(j, 0x1e);
  sprite_C[j] = 1;
  uint8 t = GetRandomNumber();
  uint16 x = BG2HOFS_copy2, y = BG2VOFS_copy2;
  if (t & 2)
    x += t, y += kChicken_Avenger[t & 1];
  else
    y += t, x += kChicken_Avenger[t & 1];
  Sprite_SetX(j, x);
  Sprite_SetY(j, y);
  Sprite_ApplySpeedTowardsLink(j, 32);
  BawkBawk(k);
}

void BawkBawk(int k) {  // 86a84c
  SpriteSfx_QueueSfx2WithPan(k, 0x30);
}

void Sprite_17_Hoarder(int k) {  // 86a86c
  if (sprite_ai_state[k])
    Sprite_Hoarder_Frantic(k);
  else
    Sprite_Hoarder_Covered(k);
}

void Sprite_Hoarder_Covered(int k) {  // 86a874
  static const uint8 kRupeeCoveredGrab_Gfx[4] = {3, 4, 5, 4};
  static const int8 kRupeeCoveredGrab_Xvel[4] = {-12, 12, 0, 0};
  static const int8 kRupeeCoveredGrab_Yvel[4] = {0, 0, -12, 12};
  CoveredRupeeCrab_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  sprite_graphics[k] = 0;
  PointU8 pt;
  uint8 dir = Sprite_DirectionToFaceLink(k, &pt);
  if (sprite_delay_main[k])
    goto lbl;
  if ((uint8)(pt.y + 0x30) < 0x60 && (uint8)(pt.x + 0x20) < 0x40) {
    sprite_delay_main[k] = 32;
lbl:sprite_x_vel[k] = kRupeeCoveredGrab_Xvel[dir];
    sprite_y_vel[k] = kRupeeCoveredGrab_Yvel[dir];
    if (!sprite_wallcoll[k])
      Sprite_MoveXY(k);
    Sprite_CheckTileCollision2(k);
    Sprite_CheckDamageFromLink(k);
    sprite_graphics[k] = kRupeeCoveredGrab_Gfx[++sprite_subtype2[k] >> 1 & 3];
  }
  if (sprite_type[k] != 0x3e || link_item_gloves >= 1)
    Sprite_ReturnIfLiftedPermissive(k);  // note, dont ret
  if (sprite_state[k] != 9) {
    sprite_C[k] = (sprite_type[k] == 0x17) ? 2 : 1;
    sprite_type[k] = 0xec;
    sprite_oam_flags[k] &= ~1;
    sprite_graphics[k] = 0;
    SpriteSpawnInfo info;
    int j = Sprite_SpawnDynamically(k, 0x3e, &info);
    if (j >= 0) {
      Sprite_SetSpawnedCoordinates(j, &info);
      sprite_flags2[j] &= ~0x80;
      sprite_delay_aux2[j] = 128;
      sprite_oam_flags[j] = 9;
      sprite_ai_state[j] = 9;
    }
  }
}

void Sprite_Hoarder_Frantic(int k) {  // 86a91d
  static const uint8 kRupeeCrab_Gfx[4] = {0, 1, 0, 1};
  static const uint8 kRupeeCrab_OamFlags[4] = {0, 0, 0x40, 0};
  static const int8 kRupeeCrab_Xvel[4] = {-16, 16, -16, 16};
  static const int8 kRupeeCrab_Yvel[4] = {-16, -16, 16, 16};

  SpriteDraw_SingleLarge(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (Sprite_ReturnIfRecoiling(k))
    return;
  Sprite_CheckDamageFromLink(k);
  if (!sprite_delay_aux2[k])
    Sprite_CheckDamageToLink(k);
  int j = ++sprite_subtype2[k] >> 1 & 3;
  sprite_graphics[k] = kRupeeCrab_Gfx[j];
  sprite_oam_flags[k] = sprite_oam_flags[k] & ~0x40 | kRupeeCrab_OamFlags[j];
  if (sprite_wallcoll[k]) {
    sprite_delay_aux4[k] = 16;
    j = GetRandomNumber() & 3;
    sprite_x_vel[k] = kRupeeCrab_Xvel[j];
    sprite_y_vel[k] = kRupeeCrab_Yvel[j];
  } else {
    Sprite_MoveXY(k);
  }
  Sprite_CheckTileCollision2(k);
  if (!sprite_delay_aux4[k] && !((k ^ frame_counter) & 31)) {
    ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLink(k, 16);
    sprite_y_vel[k] = -pt.y;
    sprite_x_vel[k] = -pt.x;
  }
  if (frame_counter & 1)
    return;

  int end;
  uint8 type;
  if (++sprite_G[k] == 192) {
    sprite_delay_main[k] = 15;
    sprite_state[k] = 6;
    sprite_flags2[k] += 4;
    end = 1;
    type = 0xd9;
  } else {
    if (sprite_G[k] & 15)
      return;
    end = 0;
    type = sprite_head_dir[k] == 6 ? 0xdb : 0xd9;
  }
  SpriteSpawnInfo info;
  j = Sprite_SpawnDynamicallyEx(k, type, &info, end);
  if (j >= 0) {
    sprite_head_dir[k]++;
    Sprite_SetSpawnedCoordinates(j, &info);
    Sprite_SetX(j, info.r0_x + 8);
    sprite_z_vel[j] = 32;
    sprite_delay_aux4[j] = 16;
    ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLink(j, 16);
    sprite_y_vel[j] = ~pt.y;
    sprite_x_vel[j] = ~pt.x;
    SpriteSfx_QueueSfx3WithPan(k, 0x30);
  }
}

void CoveredRupeeCrab_Draw(int k) {  // 86aa48
  static const int8 kCoveredRupeeCrab_DrawY[12] = {0, 0, 0, -3, 0, -5, 0, -6, 0, -6, 0, -6};
  static const uint8 kCoveredRupeeCrab_DrawChar[12] = {0x44, 0x44, 0xe8, 0x44, 0xe8, 0x44, 0xe6, 0x44, 0xe8, 0x44, 0xe6, 0x44};
  static const uint8 kCoveredRupeeCrab_DrawFlags[12] = {0, 0xc, 3, 0xc, 3, 0xc, 3, 0xc, 3, 0xc, 0x43, 0xc};
  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  if (byte_7E0FC6 >= 3)
    return;
  OamEnt *oam = GetOamCurPtr();
  uint8 r7 = (sprite_type[k] == 0x17) ? 2 : 0;
  uint8 r6 = sprite_graphics[k] * 2;
  for (int i = 1; i >= 0; i--, oam++) {
    int j = i + r6;
    uint8 ch = kCoveredRupeeCrab_DrawChar[j];
    SetOamHelper0(oam, info.x, info.y + kCoveredRupeeCrab_DrawY[j],
                  ch + (ch == 0x44 ? r7 : 0),
                  (info.flags & ~1) | kCoveredRupeeCrab_DrawFlags[j], 2);
  }
}

void Sprite_EC_ThrownItem(int k) {  // 86aae0
  if (byte_7E0FC6 < 3) {
    if (sort_sprites_setting && sprite_floor[k]) {
      int spr_slot = 0x2c + (k & 3);
      oam_cur_ptr = 0x0800 + spr_slot * 4;
      oam_ext_cur_ptr = 0x0A20 + spr_slot;
    }
    sprite_ignore_projectile[k] = sprite_state[k];
    if (sprite_C[k] >= 6) {
      SpriteDraw_ThrownItem_Gigantic(k);
    } else {
      SpriteDraw_SingleLarge(k);
      OamEnt *oam = GetOamCurPtr();
      uint8 t = player_is_indoors + is_in_dark_world;
      int j = sprite_C[k];
      oam->charnum = kThrowableScenery_Char[j + ((t >= 2) ? 6 : 0)];
      oam->flags = (oam->flags & 0xf0) | kThrowableScenery_Flags[j];
      sprite_oam_flags[k] = (sprite_oam_flags[k] & 0xc0) | (oam->flags & 0xf);
    }
  }
  if (sprite_state[k] == 9) {
    if (Sprite_ReturnIfInactive(k))
      return;
    ThrowableScenery_InteractWithSpritesAndTiles(k);
  }
}

void SpriteDraw_ThrownItem_Gigantic(int k) {  // 86ab76
  PrepOamCoordsRet info;
  sprite_oam_flags[k] = kThrowableScenery_DrawLarge_OamFlags[sprite_C[k] - 6];

  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;

  OamEnt *oam = GetOamCurPtr();
  for (int i = 3; i >= 0; i--, oam++) {
    SetOamHelper0(oam, info.x + kThrowableScenery_DrawLarge_X[i], info.y + kThrowableScenery_DrawLarge_Y[i],
                  0x4a, kThrowableScenery_DrawLarge_Flags[i] | info.flags, 2);
  }
  Oam_AllocateFromRegionB(12);
  oam = GetOamCurPtr();
  info.y = Sprite_GetY(k) - BG2VOFS_copy2;
  for (int i = 2; i >= 0; i--, oam++) {
    SetOamHelper0(oam, info.x + kThrowableScenery_DrawLarge_X2[i], info.y + 12, 0x6c, 0x24, 2);
  }
}

void ThrowableScenery_ScatterIntoDebris(int k) {  // 86ac41
  if (!sign8(sprite_C[k]) && sprite_C[k] >= 6) {
    for (int i = 3; i >= 0; i--) {
      SpriteSpawnInfo info;
      int j = Sprite_SpawnDynamically(k, 0xec, &info);
      if (j >= 0) {
        sprite_z[j] = sprite_z[k];
        Sprite_SetX(j, info.r0_x + kScatterDebris_X[i]);
        Sprite_SetY(j, info.r2_y + kScatterDebris_Y[i]);
        sprite_C[j] = 1;
        Sprite_ScheduleForBreakage(j);
        sprite_oam_flags[j] = (sprite_C[k] < 7) ? 12 : 0;
      }
    }
    sprite_state[k] = 0;
  } else {
    sprite_state[k] = 0;
    PrepOamCoordsRet info;
    if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
      return;
    int j = 29;
    while (garnish_type[j--] && j >= 0) {}
    j++;
    garnish_type[j] = 22;
    garnish_active = 22;
    garnish_x_lo[j] = sprite_x_lo[k];
    garnish_x_hi[j] = sprite_x_hi[k];
    uint16 y = Sprite_GetY(k) - sprite_z[k] + 0x10;
    garnish_y_lo[j] = (uint8)y;
    garnish_y_hi[j] = (uint8)(y >> 8);
    garnish_oam_flags[j] = info.flags;
    garnish_floor[j] = sprite_floor[k];
    garnish_countdown[j] = 31;
    garnish_sprite[j] = sprite_C[k];
  }
}

void Sprite_TransmuteToBomb(int k) {  // 86ad50
  sprite_type[k] = 0x4a;
  sprite_C[k] = 1;
  sprite_delay_aux1[k] = 255;
  sprite_flags3[k] = 0x18;
  sprite_oam_flags[k] = 8;
  sprite_health[k] = 0;
}

void Sprite_28_DarkWorldHintNPC(int k) {  // 86ad6f
  StoryTeller_1_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_BehaveAsBarrier(k);
  if (!sprite_delay_main[k])
    sprite_graphics[k] = frame_counter >> 4 & 1;

  switch(sprite_subtype2[k]) {
  case 0:
    switch (sprite_ai_state[k]) {
    case 0: DarkWorldHintNPC_Idle(k); break;
    case 1:
      if (choice_in_multiselect_box == 0 && DarkWorldHintNPC_HandlePayment()) {
        Sprite_ShowMessageUnconditional(0xff);
        sprite_ai_state[k] = 2;
      } else {
        Sprite_ShowMessageUnconditional(0x100);
        sprite_ai_state[k] = 0;
      }
      break;
    case 2: DarkWorldHintNPC_RestoreHealth(k); break;
    }
    break;
  case 1:
    switch (sprite_ai_state[k]) {
    case 0: DarkWorldHintNPC_Idle(k); break;
    case 1:
      if (choice_in_multiselect_box == 0 && DarkWorldHintNPC_HandlePayment()) {
        Sprite_ShowMessageUnconditional(0x101);
        sprite_ai_state[k] = 2;
      } else {
        Sprite_ShowMessageUnconditional(0x100);
        sprite_ai_state[k] = 0;
      }
      break;
    case 2: DarkWorldHintNPC_RestoreHealth(k); break;
    }
    break;
  case 2:
    switch (sprite_ai_state[k]) {
    case 0: DarkWorldHintNPC_Idle(k); break;
    case 1:
      if (choice_in_multiselect_box == 0 && DarkWorldHintNPC_HandlePayment()) {
        Sprite_ShowMessageUnconditional(0x102);
        sprite_ai_state[k] = 2;
      } else {
        Sprite_ShowMessageUnconditional(0x100);
        sprite_ai_state[k] = 0;
      }
      break;
    case 2: DarkWorldHintNPC_RestoreHealth(k); break;
    }
    break;
  case 3:
    if (sprite_delay_main[k] == 0) {
      if ((frame_counter & 0x3f) == 0)
        sprite_oam_flags[k] ^= 0x40;
      if (GetRandomNumber() == 0)
        sprite_delay_main[k] = 32;
    }
    Sprite_ShowSolicitedMessage(k, 0x149);
    break;
  case 4:
    sprite_graphics[k] = frame_counter >> 1 & 1;
    Sprite_MoveZ(k);
    if (sign8(sprite_z[k]))
      sprite_z[k] = 0;
    sprite_z_vel[k] += (sprite_z[k] >= 4) ? -1 : 1;
    switch (sprite_ai_state[k]) {
    case 0: DarkWorldHintNPC_Idle(k); break;
    case 1:
      if (choice_in_multiselect_box == 0 && DarkWorldHintNPC_HandlePayment()) {
        Sprite_ShowMessageUnconditional(0x103);
        sprite_ai_state[k] = 2;
      } else {
        Sprite_ShowMessageUnconditional(0x100);
        sprite_ai_state[k] = 0;
      }
      break;
    case 2: DarkWorldHintNPC_RestoreHealth(k); break;
    }
    break;
  }
}

void DarkWorldHintNPC_Idle(int k) {  // 86ada7
  if (Sprite_ShowSolicitedMessage(k, 0xfe) & 0x100)
    sprite_ai_state[k] = 1;
}

void DarkWorldHintNPC_RestoreHealth(int k) {  // 86adb5
  link_hearts_filler = 0xa0;
  sprite_ai_state[k] = 0;
}

bool DarkWorldHintNPC_HandlePayment() {  // 86aeab
  if (link_rupees_goal < 20)
    return false;
  link_rupees_goal -= 20;
  return true;
}

void StoryTeller_1_Draw(int k) {  // 86af1a
  static const DrawMultipleData kStoryTeller_Dmd[10] = {
    {0, 0, 0x0a4a, 2},
    {0, 0, 0x4a6e, 2},
    {0, 0, 0x0a24, 2},
    {0, 0, 0x4a24, 2},
    {0, 0, 0x0804, 2},
    {0, 0, 0x4804, 2},
    {0, 0, 0x0a6a, 2},
    {0, 0, 0x0a6c, 2},
    {0, 0, 0x0a0e, 2},
    {0, 0, 0x0a2e, 2},
  };
  PrepOamCoordsRet info;
  Sprite_DrawMultiplePlayerDeferred(k, &kStoryTeller_Dmd[sprite_subtype2[k] * 2 + sprite_graphics[k]], 1, &info);
  SpriteDraw_Shadow(k, &info);
}

void Sprite_2E_FluteKid(int k) {  // 86af3b
  switch (sprite_head_dir[k]) {
  case 0:
    switch (sprite_subtype2[k]) {
    case 0: FluteKid_Human(k); break;
    case 1: Sprite_FluteKid_Stumpy(k); break;
    }
    break;
  case 1: Sprite_FluteKid_Quaver(k); break;
  }
}

void FluteKid_Human(int k) {  // 86af51
  if (sprite_ai_state[k] != 3)
    sprite_C[k] = FluteBoy_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (!sprite_C[k] && !sprite_B[k]) {
    sound_effect_ambient = 11;
    sprite_B[k] = 11;
  }
  sprite_graphics[k] = frame_counter >> 5 & 1;
  switch (sprite_ai_state[k]) {
  case 0:  // wait
    if (link_item_flute >= 2 || FluteBoy_CheckIfPlayerClose(k)) {
      sprite_ai_state[k] = 1;
      sprite_D[k]++;
      byte_7E0FDD++;
      sprite_delay_main[k] = 176;
      flag_is_link_immobilized = 1;
    }
    if (!sprite_delay_main[k]) {
      sprite_delay_main[k] = 25;
      FluteKid_SpawnQuaver(k);
    }
    break;
  case 1:  // prep phase out
    flag_is_link_immobilized = 1;
    if (!sprite_delay_main[k]) {
      TS_copy = 2;
      CGADSUB_copy = 48;
      BYTE(palette_filter_countdown) = 0;
      BYTE(darkening_or_lightening_screen) = 0;
      Palette_AssertTranslucencySwap();
      sprite_ai_state[k] = 2;
      sound_effect_ambient = 128;
      SpriteSfx_QueueSfx2WithPan(k, 0x33);
    }
    break;
  case 2:  // phase out
    if (!(frame_counter & 15)) {
      PaletteFilter_SP5F();
      if (!BYTE(palette_filter_countdown))
        sprite_ai_state[k] = 3;
    }
    break;
  case 3:  // phased out
    PaletteFilter_RestoreSP5F();
    Palette_RevertTranslucencySwap();
    sprite_state[k] = 0;
    flag_is_link_immobilized = 0;
    break;
  }
}

void Sprite_FluteKid_Stumpy(int k) {  // 86b040
  static const int8 kFluteAardvark_Gfx[20] = {
    1, 1, 1, 1, 2, 1, 2, 1, 2, 1, 2, 3, 2, 3, 2, 3, 2, 3, 2, -1,
  };
  static const int8 kFluteAardvark_Delay[19] = {
    -1, -1, -1, 16, 2, 12, 6, 8, 10, 4, 14, 2, 10, 6, 6, 10, 2, 14,  2,
  };
  FluteAardvark_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  switch (sprite_ai_state[k]) {
  case 0:  //
    switch (link_item_flute & 3) {
    case 0:  // supplicate
      if (Sprite_ShowSolicitedMessage(k, 0xe5) & 0x100)
        sprite_ai_state[k] = 1;
      break;
    case 1:  // give me flute
      Sprite_ShowSolicitedMessage(k, 0xe8);
      break;
    case 2:  // thanks
      sprite_graphics[k] = 1;
      if (Sprite_ShowSolicitedMessage(k, 0xe9) & 0x100)
        sprite_ai_state[k] = 3;
      break;
    case 3:  // already did
      sprite_graphics[k] = 3;
      break;
    }
    break;
  case 1:  //
    if (!choice_in_multiselect_box) {
      Sprite_ShowMessageUnconditional(0xe6);
      sprite_ai_state[k] = 2;
    } else {
      Sprite_ShowMessageUnconditional(0xe7);
      sprite_ai_state[k] = 0;
    }
    break;
  case 2:  // grant shovel
    item_receipt_method = 0;
    Link_ReceiveItem(0x13, 0);
    sprite_ai_state[k] = 0;
    break;
  case 3:  // wait for music
    if (hud_cur_item == kHudItem_Flute && joypad1H_last & 0x40) {
      sprite_ai_state[k] = 4;
      music_control = 0xf2;
      sound_effect_1 = 0;
      sound_effect_ambient = 23;
      flag_is_link_immobilized++;
    }
    break;
  case 4:  //
    if (!sprite_delay_main[k]) {
      if (sprite_A[k] >= 3)
        SpriteSfx_QueueSfx2WithPan(k, 0x33);
      int j = sprite_A[k]++;
      if (kFluteAardvark_Gfx[j] >= 0) {
        sprite_graphics[k] = kFluteAardvark_Gfx[j];
        sprite_delay_main[k] = kFluteAardvark_Delay[j];
      } else {
        music_control = 0xf3;
        sprite_ai_state[k] = 5;
        flag_is_link_immobilized = 0;
      }
    }
    break;
  case 5:  // done
    sprite_graphics[k] = 3;
    sram_progress_indicator_3 |= 8;
    break;
  }
}

void Sprite_FluteKid_Quaver(int k) {  // 86b173
  SpriteDraw_SingleSmall(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_MoveXY(k);
  Sprite_MoveZ(k);
  if (!sprite_delay_main[k])
    sprite_state[k] = 0;
  if (!(frame_counter & 1))
    sprite_x_vel[k] += (frame_counter >> 5 ^ cur_object_index) & 1 ? -1 : 1;
}

void FluteKid_SpawnQuaver(int k) {  // 86b1a5
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0x2e, &info);
  if (j >= 0) {
    Sprite_SetX(j, info.r0_x + 4);
    Sprite_SetY(j, info.r2_y - 4);
    sprite_head_dir[j] = 1;
    sprite_z_vel[j] = 8;
    sprite_delay_main[j] = 96;
    sprite_ignore_projectile[j] = 96;
  }
}

void Sprite_1A_Smithy(int k) {  // 86b1ee
  switch (sprite_subtype2[k]) {
  case 0: Smithy_Main(k); break;
  case 1: Smithy_Spark(k); break;
  case 2: Smithy_Frog(k); break;
  case 3: Smithy_Homecoming(k); break;
  }
}

void Smithy_Homecoming(int k) {  // 86b1fd
  ReturningSmithy_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  switch(sprite_ai_state[k]) {
  case 0: { // ApproachTheBench
    static const int8 kReturningSmithy_Delay[3] = {104, 12, 0};
    static const int8 kReturningSmithy_Dir[3] = {0, 2, -1};
    static const int8 kReturningSmithy_Xvel[4] = {0, 0, -13, 13};
    static const int8 kReturningSmithy_Yvel[4] = {-13, 13, 0, 0};
    Sprite_MoveXY(k);
    sprite_graphics[k] = frame_counter >> 3 & 1;
    if (sprite_delay_main[k])
      return;
    int j = sprite_A[k]++;
    sprite_delay_main[k] = kReturningSmithy_Delay[j];
    if ((j = kReturningSmithy_Dir[j]) >= 0) {
      sprite_D[k] = j;
      sprite_x_vel[k] = kReturningSmithy_Xvel[j];
      sprite_y_vel[k] = kReturningSmithy_Yvel[j];
    } else {
      sprite_ai_state[k] = 1;
    }
    break;
  }
  case 1: // Thankful
    Sprite_BehaveAsBarrier(k);
    Sprite_ShowSolicitedMessage(k, 0xe3);
    flag_is_link_immobilized = 0;
    sprite_D[k] = 1;
    sram_progress_indicator_3 |= 32;
    break;
  }
}

void Smithy_Frog(int k) {  // 86b274
  SmithyFrog_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_BehaveAsBarrier(k);
  sprite_z_vel[k] -= 2;
  Sprite_MoveZ(k);
  if (sign8(sprite_z[k])) {
    sprite_z[k] = 0;
    sprite_z_vel[k] = 16;
  }
  if (!sprite_ai_state[k]) {
    sprite_D[k] = 1;
    if (Sprite_ShowSolicitedMessage(k, 0xe1) & 0x100)
      sprite_ai_state[k] = 1;
  } else {
    follower_indicator = 7;
    LoadFollowerGraphics();
    Sprite_BecomeFollower(k);  // zelda bug: doesn't save X
    sprite_state[k] = 0;
  }
}

void ReturningSmithy_Draw(int k) {  // 86b308
  static const DrawMultipleData kReturningSmithy_Dmd[8] = {
    {0, 0, 0x4122, 2},
    {0, 0, 0x0122, 2},
    {0, 0, 0x4122, 2},
    {0, 0, 0x0122, 2},
    {0, 0, 0x0122, 2},
    {0, 0, 0x0122, 2},
    {0, 0, 0x4122, 2},
    {0, 0, 0x4122, 2},
  };
  static const uint8 kReturningSmithy_Dma[8] = {0xc0, 0xc0, 0xa0, 0xa0, 0x80, 0x60, 0x80, 0x60};
  int j = sprite_D[k] * 2 + sprite_graphics[k];
  PrepOamCoordsRet info;
  BYTE(dma_var7) = kReturningSmithy_Dma[j];
  Sprite_DrawMultiplePlayerDeferred(k, &kReturningSmithy_Dmd[j], 1, &info);
  SpriteDraw_Shadow(k, &info);
}

void SmithyFrog_Draw(int k) {  // 86b339
  static const DrawMultipleData kSmithyFrog_Dmd[1] = {
    {0, 0, 0x00c8, 2},
  };
  PrepOamCoordsRet info;
  Sprite_DrawMultiplePlayerDeferred(k, kSmithyFrog_Dmd, 1, &info);
  SpriteDraw_Shadow(k, &info);
}

void Smithy_Main(int k) {  // 86b34e
  Smithy_Draw(k);
  sprite_z_vel[k] -= 2;
  Sprite_MoveZ(k);
  if (sign8(sprite_z[k])) {
    sprite_z[k] = 0;
    sprite_z_vel[k] = 0;
  }
  if (Sprite_ReturnIfInactive(k))
    return;

  int j = sprite_ai_state[sprite_E[k]];
  int i = sprite_ai_state[k];
  if ((j == 5 || j == 7 || j == 9 || i == 5 || i == 7 || i == 9 || (i | j) == 0) && sprite_B[k]-- == 0) {
    static const uint8 kSmithy_Gfx[8] = {0, 1, 2, 3, 3, 2, 1, 0};
    static const uint8 kSmithy_B[8] = {24, 4, 1, 16, 16, 5, 10, 16};
    j = sprite_A[k];
    sprite_A[k] = j + 1 & 7;
    sprite_graphics[k] = kSmithy_Gfx[j];
    sprite_B[k] = kSmithy_B[j];
    if (j == 1)
      sprite_z_vel[k] = 16;
    if (j == 3) {
      Smithy_SpawnSpark(k);
      SpriteSfx_QueueSfx2WithPan(k, 0x5);
    }
  }
  switch(sprite_ai_state[k]) {
  case 0:  // ConversationStart
    sprite_C[k] = 0;
    if (follower_indicator != 8) {
      if (Smithy_ListenForHammer(k)) {
        Sprite_ShowMessageUnconditional(0xe4);
        sprite_delay_aux1[k] = 96;
        sprite_C[k]++;
      } else if (sram_progress_indicator_3 & 0x20) {
        if (Sprite_ShowSolicitedMessage(k, 0xd8) & 0x100) {
          sprite_ai_state[k]++;
          sprite_C[k]++;
        }
      } else {
        Sprite_ShowSolicitedMessage(k, 0xdf);
      }
    } else {
      if (BYTE(link_y_coord) < 0xc2) {
        Sprite_ShowMessageUnconditional(0xe0);
        sprite_ai_state[k] = 10;
        flag_is_link_immobilized++;
      }
    }
    break;
  case 1:  // ProvideTemperingChoice
    if (!choice_in_multiselect_box) {
      Sprite_ShowMessageUnconditional(0xd9);
      sprite_ai_state[k] = 2;
    } else {
      Sprite_ShowMessageUnconditional(0xdc);
      sprite_ai_state[k] = 0;
    }
    break;
  case 2:  // HandleTemperingChoice
    if (choice_in_multiselect_box == 0) {
      if (link_sword_type < 3) {
        Sprite_ShowMessageUnconditional(0xda);
        sprite_ai_state[k] = 3;
      } else {
        Sprite_ShowMessageUnconditional(0xdb);
        sprite_ai_state[k] = 0;
      }
    } else {
      Sprite_ShowMessageUnconditional(0xdc);
      sprite_ai_state[k] = 0;
    }
    break;
  case 3:  // HandleTemperingCost
    if (choice_in_multiselect_box || link_rupees_goal < 10) {
      Sprite_ShowMessageUnconditional(0xdc);
      sprite_ai_state[k] = 0;
    } else {
      link_rupees_goal -= 10;
      Sprite_ShowMessageUnconditional(0xdd);
      sprite_ai_state[sprite_E[k]] = 5;
      sprite_ai_state[k] = 5;
      flag_overworld_area_did_change = 0;
      link_sword_type = 255;
      sram_progress_indicator_3 |= 128;
    }
    break;
  case 4:  // TemperingSword
  case 5:  //
    sprite_C[k] = 0;
    if (Smithy_ListenForHammer(k)) {
      Sprite_ShowMessageUnconditional(0xe4);
      sprite_delay_aux1[k] = 96;
      sprite_C[k]++;
    } else if (flag_overworld_area_did_change) {
      if (Sprite_ShowSolicitedMessage(k, 0xde) & 0x100) {
        sprite_ai_state[k]++;
        sprite_graphics[k] = 4;
      }
    } else {
      Sprite_ShowSolicitedMessage(k, 0xe2);
    }
    break;
  case 6:  // Smithy_GiveTemperedSword
    sprite_ai_state[k] = 0;
    sprite_ai_state[sprite_E[k]] = 0;
    item_receipt_method = 0;
    Link_ReceiveItem(2, 0);
    sram_progress_indicator_3 &= ~0x80;
    break;
  case 7:  //
  case 8:  //
  case 9:  //
    break;
  case 10: { // Smithy_SpawnFriend
    SpriteSpawnInfo info;
    int j = Sprite_SpawnDynamically(k, 0x1a, &info);
    if (j >= 0) {
      Sprite_SetX(j, link_x_coord);
      Sprite_SetY(j, link_y_coord);
      sprite_subtype2[j] = 3;
      sprite_ignore_projectile[j] = 3;
    }
    sprite_ai_state[k] = 11;
    follower_indicator = 0;
    sprite_graphics[k] = 4;
    break;
  }
  case 11:  // Smithy_CopiouslyThankful
    Sprite_ShowSolicitedMessage(k, 0xe3);
    break;

  }
}

bool Smithy_ListenForHammer(int k) {  // 86b43d
  return sprite_delay_aux1[k] == 0 && hud_cur_item == kHudItem_Hammer && (link_item_in_hand & 2) && player_handler_timer == 2 && Sprite_CheckDamageToLink_same_layer(k);
}

int Smithy_SpawnDwarfPal(int k) {  // 86b5a6
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0x1a, &info);
  if (j < 0)
    return j;
  Sprite_SetX(j, info.r0_x);
  Sprite_SetY(j, info.r2_y);
  sprite_x_lo[j] += 0x2C;
  sprite_D[j] = 1;
  sprite_A[j] = 4;
  sprite_ignore_projectile[j] = 4;
  return j;
}

void Smithy_Draw(int k) {  // 86b673
  static const DrawMultipleData kSmithy_Dmd[20] = {
    {  1,   0, 0x4040, 2},
    {-11, -10, 0x4060, 2},
    { -1,   0, 0x0040, 2},
    { 11, -10, 0x0060, 2},
    {  1,   0, 0x4040, 2},
    { -3, -14, 0x4044, 2},
    { -1,   0, 0x0040, 2},
    {  3, -14, 0x0044, 2},
    {  1,   0, 0x4042, 2},
    { 11, -10, 0x0060, 2},
    { -1,   0, 0x0042, 2},
    {-11, -10, 0x4060, 2},
    {  1,   0, 0x4042, 2},
    { 13,   2, 0x4062, 2},
    { -1,   0, 0x0042, 2},
    {-13,   2, 0x0062, 2},
    {  0,   0, 0x4064, 2},
    {  0,   0, 0x4062, 2},
    {  0,   0, 0x0064, 2},
    {  0,   0, 0x0064, 2},
  };
  PrepOamCoordsRet info;
  Sprite_DrawMultiplePlayerDeferred(k, &kSmithy_Dmd[sprite_graphics[k] * 4 + sprite_D[k] * 2], 2, &info);
  SpriteDraw_Shadow(k, &info);
}

void Smithy_Spark(int k) {  // 86b6a3
  static const int8 kSmithySpark_Gfx[7] = {0, 1, 2, 1, 2, 1, -1};
  static const int8 kSmithySpark_Delay[6] = {4, 1, 3, 2, 1, 1};
  SmithySpark_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (!sprite_delay_main[k]) {
    int j = sprite_A[k];
    sprite_A[k] = j + 1 & 7;
    if (sign8(kSmithySpark_Gfx[j])) {
      sprite_state[k] = 0;
      return;
    }
    sprite_graphics[k] = kSmithySpark_Gfx[j];
    sprite_delay_main[k] = kSmithySpark_Delay[j];
  }
}

void Smithy_SpawnSpark(int k) {  // 86b6cd
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0x1a, &info);
  if (j >= 0) {
    Sprite_SetX(j, info.r0_x);
    Sprite_SetY(j, info.r2_y);
    sprite_x_lo[j] += sprite_D[k] ? -15 : 15;
    sprite_y_lo[j] += 2;
    sprite_subtype2[j] = 1;
  }
}

void SmithySpark_Draw(int k) {  // 86b72c
  Oam_AllocateFromRegionB(8);
  static const DrawMultipleData kSmithySpark_Dmd[6] = {
    { 0,  3, 0x41aa, 2},
    { 0, -1, 0x41aa, 2},
    {-4,  0, 0x0190, 0},
    {12,  0, 0x4190, 0},
    {-5, -2, 0x0191, 0},
    {13, -2, 0x0191, 0},
  };
  Sprite_DrawMultiple(k, &kSmithySpark_Dmd[sprite_graphics[k] * 2], 2, NULL);
}

void Sprite_1B_Arrow(int k) {  // 86b754
  int j;
  static const int8 kEnemyArrow_Xvel[8] = {0, 0, 16, 16, 0, 0, -16, -16};
  static const int8 kEnemyArrow_Yvel[8] = {16, 16, 0, 0, -16, -16, 0, 0};
  static const uint8 kEnemyArrow_Dirs[4] = {0, 2, 1, 3};

  EnemyArrow_Draw(k);
  if (Sprite_ReturnIfPaused(k))
    return;
  if (sprite_state[k] == 9) {
    j = sprite_delay_main[k];
    if (j != 0) {
      if (--j == 0) {
        sprite_state[k] = 0;
      } else if (j >= 32 && !(j & 1)) {
        j = (frame_counter << 1 & 4) | sprite_D[k];
        sprite_x_vel[k] = kEnemyArrow_Xvel[j];
        sprite_y_vel[k] = kEnemyArrow_Yvel[j];
        Sprite_MoveXY(k);
      }
      return;
    }
    Sprite_CheckDamageToLink_same_layer(k);
    if (sprite_E[k] == 0 && Sprite_CheckTileCollision(k)) {
      if (sprite_A[k]) {
        SpriteSfx_QueueSfx2WithPan(k, 0x5);
        Sprite_ScheduleForBreakage(k);
        Sprite_PlaceWeaponTink(k);
      } else {
        sprite_delay_main[k] = 48;
        sprite_A[k] = 2;
        SpriteSfx_QueueSfx2WithPan(k, 0x8);
      }
    } else {
      Sprite_MoveXY(k);
    }
  } else {
    if (sprite_ai_state[k] == 0) {
      Sprite_ApplyRicochet(k);
      sprite_z_vel[k] = 24;
      sprite_delay_main[k] = 255;
      sprite_ai_state[k]++;
      sprite_hit_timer[k] = 0;
    }
    sprite_D[k] = kEnemyArrow_Dirs[sprite_delay_main[k] >> 3 & 3];
    Sprite_MoveZ(k);
    Sprite_MoveXY(k);
    sprite_z_vel[k] -= 2;
    if (sign8(sprite_z[k]))
      sprite_state[k] = 0;
  }
}

void EnemyArrow_Draw(int k) {  // 86b867
  static const int16 kEnemyArrow_Draw_X[8] = {-8, 0, 0, 8, 0, 0, 0, 0};
  static const int16 kEnemyArrow_Draw_Y[8] = {0, 0, 0, 0, -8, 0, 0, 8};
  static const uint8 kEnemyArrow_Draw_Char[32] = {
    0x3a, 0x3d, 0x3d, 0x3a, 0x2a, 0x2b, 0x2b, 0x2a, 0x7c, 0x6c, 0x6c, 0x7c, 0x7b, 0x6b, 0x6b, 0x7b,
    0x3a, 0x3b, 0x3b, 0x3a, 0x2a, 0x3c, 0x3c, 0x2a, 0x81, 0x80, 0x80, 0x81, 0x91, 0x90, 0x90, 0x91,
  };
  static const uint8 kEnemyArrow_Draw_Flags[32] = {
    8,    8, 0x48, 0x48, 8, 8, 0x88, 0x88,    9, 0x49, 9, 0x49,    9, 0x89, 9, 0x89,
    8, 0x88, 0xc8, 0x48, 8, 8, 0x88, 0x88, 0x49, 0x49, 9,    9, 0x89, 0x89, 9,    9,
  };

  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  OamEnt *oam = GetOamCurPtr();
  uint8 r6 = sprite_D[k] * 2, r7 = sprite_A[k] * 8;
  for (int i = 1; i >= 0; i--, oam++) {
    int j = r6 + i;
    SetOamHelper0(oam, info.x + kEnemyArrow_Draw_X[j], info.y + kEnemyArrow_Draw_Y[j],
                  kEnemyArrow_Draw_Char[j + r7], kEnemyArrow_Draw_Flags[j + r7] | info.flags, 0);
  }
}

void Sprite_1E_CrystalSwitch(int k) {  // 86b8d0
  sprite_oam_flags[k] = sprite_oam_flags[k] & ~0xe | kCrystalSwitchPal[orange_blue_barrier_state & 1];
  Oam_AllocateDeferToPlayer(k);
  SpriteDraw_SingleLarge(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (Sprite_CheckDamageToLink_same_layer(k)) {
    Sprite_NullifyHookshotDrag();
    link_speed_setting = 0;
    Sprite_RepelDash();
  }
  if (sprite_delay_main[k] == 0) {
    Sprite_GarnishSpawn_Sparkle(k, frame_counter & 7, GetRandomNumber() & 7);
    sprite_delay_main[k] = 31;
  }
  if (sprite_F[k] == 0) {
    if (sign8(button_b_frames - 9))
      Sprite_CheckDamageFromLink(k);
  } else if (sprite_F[k]-- == 11) {
    orange_blue_barrier_state ^= 1;
    submodule_index = 22;
    SpriteSfx_QueueSfx3WithPan(k, 0x25);
  }
}

void Sprite_1F_SickKid(int k) {  // 86b94c
  static const int8 kBugNetKid_Gfx[8] = {0, 1, 0, 1, 0, 1, 2, -1};
  static const uint8 kBugNetKid_Delay[7] = {8, 12, 8, 12, 8, 96, 16};
  int j;
  BugNetKid_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  switch (sprite_ai_state[k]) {
  case 0:  // resting
    if (Sprite_CheckIfLinkIsBusy() || !Sprite_CheckDamageToLink_same_layer(k))
      return;
    if ((link_bottle_info[0] | link_bottle_info[1] | link_bottle_info[2] | link_bottle_info[3]) < 2) {
      Sprite_ShowSolicitedMessage(k, 0x104);
    } else {
      flag_is_link_immobilized++;
      sprite_ai_state[k] = 1;
    }
    break;
  case 1:  // perk
    if (sprite_delay_main[k])
      return;
    j = sprite_A[k];
    if (kBugNetKid_Gfx[j] >= 0) {
      sprite_graphics[k] = kBugNetKid_Gfx[j];
      sprite_delay_main[k] = kBugNetKid_Delay[j];
      sprite_A[k] = j + 1;
    } else {
      Sprite_ShowMessageUnconditional(0x105);
      sprite_ai_state[k] = 2;
    }
    break;
  case 2:  // grant
    item_receipt_method = 0;
    Link_ReceiveItem(0x21, 0);
    flag_is_link_immobilized = 0;
    sprite_ai_state[k] = 3;
    break;
  case 3:  // back to rest
    sprite_graphics[k] = 1;
    Sprite_ShowSolicitedMessage(k, 0x106);
    break;
  }
}

void Sprite_21_WaterSwitch(int k) {  // 86b9fa
  PushSwitch_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  switch(sprite_ai_state[k]) {
  case 0:
    if (sprite_C[k]) {
      if (!--sprite_B[k])
        sprite_ai_state[k] = 1;
      if (!(frame_counter & 3))
        SpriteSfx_QueueSfx2WithPan(k, 0x22);
    } else {
      sprite_B[k] = 48;
    }
    break;
  case 1:
    if (!sprite_delay_main[k]) {
      static const uint8 kPushSwitch_Delay[10] = {40, 6, 3, 3, 3, 5, 1, 1, 3, 12};
      static const uint8 kPushSwitch_Dir[10] = {0, 1, 2, 3, 4, 5, 5, 6, 7, 6};
      int j = ++sprite_A[k];
      if (j == 10) {
        sprite_ai_state[k] = 2;
        dung_flag_statechange_waterpuzzle++;
        SpriteSfx_QueueSfx3WithPan(k, 0x25);
      } else {
        sprite_delay_main[k] = kPushSwitch_Delay[j];
        sprite_D[k] = kPushSwitch_Dir[j];
        SpriteSfx_QueueSfx2WithPan(k, 0x22);
      }
    }
    break;
  case 2:
    break;
  }
}

void PushSwitch_Draw(int k) {  // 86bb22
  static const OamEntSigned kPushSwitch_Oam[40] = {
    {  4, 20, 0xdc, 0x20},
    {  4, 12, 0xdd, 0x20},
    {  4, 12, 0xdd, 0x20},
    {  4, 12, 0xdd, 0x20},
    {  0,  0, 0xca, 0x20},
    {  3, 12, 0xdd, 0x20},
    {  3, 20, 0xdc, 0x20},
    {  3, 20, 0xdc, 0x20},
    {  3, 20, 0xdc, 0x20},
    {  0,  0, 0xca, 0x20},
    { -8,  8, 0xea, 0x20},
    {  0,  8, 0xeb, 0x20},
    { -8, 16, 0xfa, 0x20},
    {  0, 16, 0xfb, 0x20},
    {  0,  0, 0xca, 0x20},
    {-12,  4, 0xcc, 0x20},
    { -4,  4, 0xcd, 0x20},
    { -4,  4, 0xcd, 0x20},
    { -4,  4, 0xcd, 0x20},
    {  0,  0, 0xca, 0x20},
    {-10,  4, 0xcc, 0x20},
    { -4,  4, 0xcd, 0x20},
    { -4,  4, 0xcd, 0x20},
    { -4,  4, 0xcd, 0x20},
    {  0,  0, 0xca, 0x20},
    { -8,  4, 0xcc, 0x20},
    { -4,  4, 0xcd, 0x20},
    { -4,  4, 0xcd, 0x20},
    { -4,  4, 0xcd, 0x20},
    {  0,  0, 0xca, 0x20},
    {  4,  3, 0xe2, 0x20},
    { -6,  4, 0xcc, 0x20},
    { -4,  4, 0xcd, 0x20},
    { -4,  4, 0xcd, 0x20},
    {  0,  0, 0xca, 0x20},
    {  4,  3, 0xf1, 0x20},
    { -6,  4, 0xcc, 0x20},
    { -4,  4, 0xcd, 0x20},
    { -4,  4, 0xcd, 0x20},
    {  0,  0, 0xca, 0x20},
  };
  static const uint8 kPushSwitch_WH[16] = {8, 6, 0x10, 0x10, 0x10, 8, 0x10, 8, 0x10, 8, 0x10, 8, 0x10, 3, 0x10, 8};
  Oam_AllocateDeferToPlayer(k);
  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  uint8 flags;
  sprite_oam_flags[k] = flags = palette_swap_flag ? sprite_oam_flags[k] | 0xe : sprite_oam_flags[k] & ~0xe;
  uint8 r1 = sprite_B[k] >> 2 & 3;

  oam_cur_ptr += 4, oam_ext_cur_ptr += 1;

  OamEnt *oam = GetOamCurPtr();
  memcpy(oam, &kPushSwitch_Oam[sprite_D[k] * 5], 20);

  uint8 xv = -r1 + BYTE(dungmap_var7);
  uint8 yv = HIBYTE(dungmap_var7) - (r1 >> 1);

  oam[0].x += xv;
  oam[1].x += xv;
  oam[2].x += xv;
  oam[3].x += xv;
  oam[4].x += BYTE(dungmap_var7);

  oam[0].y += yv;
  oam[1].y += yv;
  oam[2].y += yv;
  oam[3].y += yv;
  oam[4].y += HIBYTE(dungmap_var7);

  oam[0].flags |= flags;
  oam[1].flags |= flags;
  oam[2].flags |= flags;
  oam[3].flags |= flags;
  oam[4].flags |= flags;

  uint8 *big = &g_ram[oam_ext_cur_ptr];
  big[0] = big[1] = big[2] = big[3] = 0;
  big[4] = 2;

  Sprite_CorrectOamEntries(k, 4, 0xff);

  if (sprite_floor[k] == link_is_on_lower_level) {
    sprite_C[k] = 0;
    int d = sprite_D[k];
    int x = Sprite_GetX(k) + (int8)kPushSwitch_Oam[d * 4].x;
    int y = Sprite_GetY(k) + (int8)kPushSwitch_Oam[d * 4].y;

    SpriteHitBox hb;
    hb.r4_spr_xlo = x;
    hb.r10_spr_xhi = x >> 8;
    hb.r5_spr_ylo = y;
    hb.r11_spr_yhi = y >> 8;
    hb.r6_spr_xsize = kPushSwitch_WH[d * 2 + 0];
    hb.r7_spr_ysize = kPushSwitch_WH[d * 2 + 1];
    Link_SetupHitBox(&hb);
    if (CheckIfHitBoxesOverlap(&hb)) {
      uint16 oldy = Sprite_GetY(k);
      Sprite_SetY(k, oldy + 19);
      uint8 new_dir = Sprite_DirectionToFaceLink(k, NULL);
      Sprite_SetY(k, oldy);
      if (new_dir == 0 && link_direction_facing == 4)
        sprite_C[k]++;
    } else {
      if (!Sprite_CheckDamageToLink_same_layer(k))
        return;
    }
    Sprite_NullifyHookshotDrag();
    link_speed_setting = 0;
    Sprite_RepelDash();
  }
}

void Sprite_39_Locksmith(int k) {  // 86bcac
  uint8 bak;
  int j;

  MiddleAgedMan_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_BehaveAsBarrier(k);

  switch (sprite_ai_state[k]) {
  case 0:  // chilling
    Sprite_ShowSolicitedMessage(k, 0x107);
    bak = sprite_x_lo[k];
    sprite_x_lo[k] -= 16;
    Sprite_Get16BitCoords(k);
    sprite_x_vel[k] = 1;
    sprite_y_vel[k] = 1;
    if (!Sprite_CheckTileCollision(k)) {
      sprite_ai_state[k]++;
      if (follower_indicator != 0)
        sprite_ai_state[k] = 5;
    }
    sprite_x_lo[k] = bak;
    break;
  case 1:  // transition to tagalong
    follower_indicator = 9;
    tagalong_var5 = 0;
    LoadFollowerGraphics();
    Follower_Initialize();
    word_7E02CD = 0x40;
    sprite_state[k] = 0;
    break;
  case 2:  // offer chest
    if (Sprite_CheckIfLinkIsBusy())
      return;
    if (follower_dropped) {
      j = Sprite_ShowSolicitedMessage(k, 0x109);
    } else {
      j = Sprite_ShowMessageOnContact(k, 0x109);
    }
    if (j & 0x100)
      sprite_ai_state[k] = 3;
    break;
  case 3:  // react to secret keeping
    if (!choice_in_multiselect_box) {
      if (follower_dropped) {
        Sprite_ShowMessageUnconditional(0x10c);
        sprite_ai_state[k] = 2;
      } else {
        item_receipt_method = 0;
        Link_ReceiveItem(0x16, 0);
        sram_progress_indicator_3 |= 0x10;
        sprite_ai_state[k] = 4;
        follower_indicator = 0;
      }
    } else {
      Sprite_ShowMessageUnconditional(0x10a);
      sprite_ai_state[k] = 2;
    }
    break;
  case 4:  // promise reminder
    Sprite_ShowSolicitedMessage(k, 0x10b);
    break;
  case 5:  // silence other tagalong
    Sprite_ShowSolicitedMessage(k, 0x107);
    break;
  }
}

void MiddleAgedMan_Draw(int k) {  // 86bdac
  static const DrawMultipleData kMiddleAgedMan_Dmd[2] = {
    {0, -8, 0x00ea, 2},
    {0,  0, 0x00ec, 2},
  };
  PrepOamCoordsRet info;
  Sprite_DrawMultiplePlayerDeferred(k, &kMiddleAgedMan_Dmd[0], 2, &info);
  SpriteDraw_Shadow(k, &info);
}

void Sprite_2B_Hobo(int k) {  // 86bdc1
  switch (sprite_subtype2[k]) {
  case 0:
    Sprite_Hobo_Bum(k);
    break;
  case 1:
    Sprite_Hobo_Bubble(k);
    break;
  case 2:
    Sprite_Hobo_Fire(k);
    break;
  case 3:
    Sprite_Hobo_Smoke(k);
    break;
  }
}

void Sprite_Hobo_Bum(int k) {  // 86bdd0
  Hobo_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  sprite_flags4[k] = 3;
  if (Sprite_CheckDamageToLink_same_layer(k)) {
    Sprite_NullifyHookshotDrag();
    link_speed_setting = 0;
    Link_CancelDash();
  }
  switch(sprite_ai_state[k]) {
  case 0:  // sleeping
    sprite_flags4[k] = 7;
    if (Sprite_CheckDamageToLink_same_layer(k) && (filtered_joypad_L & 0x80)) {
      sprite_ai_state[k] = 1;
      int j = sprite_E[k];
      sprite_delay_main[j] = 4;
      flag_is_link_immobilized = 1;
    }
    if (!sprite_delay_aux2[k]) {
      sprite_delay_aux2[k] = 160;
      sprite_E[k] = Hobo_SpawnBubble(k);
    }
    break;
  case 1: // wake up
    if (!sprite_delay_main[k]) {
      static const int8 kHobo_Gfx[7] = {0, 1, 0, 1, 0, 1, 2};
      static const int8 kHobo_Delay[7] = {6, 2, 6, 6, 2, 100, 30};
      int j = sprite_A[k];
      if (j != 7) {
        sprite_graphics[k] = kHobo_Gfx[j];
        sprite_delay_main[k] = kHobo_Delay[j];
        sprite_A[k]++;
      } else {
        Sprite_ShowMessageUnconditional(0xd7);
        sprite_ai_state[k] = 2;
      }
    }
    break;
  case 2:  // grant bottle
    sprite_ai_state[k] = 3;
    sprite_graphics[k] = 1;
    save_ow_event_info[BYTE(overworld_screen_index)] |= 0x20;
    item_receipt_method = 0;
    Link_ReceiveItem(0x16, 0);
    sram_progress_indicator_3 |= 1;
    break;
  case 3:  // back to sleep
    flag_is_link_immobilized = 0;
    sprite_graphics[k] = 0;
    if (!sprite_delay_main[k]) {
      sprite_delay_main[k] = 160;
      Hobo_SpawnBubble(k);
    }
    break;
  }
}

void SpritePrep_Hobo_SpawnSmoke(int k) {  // 86be9d
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0x2b, &info);
  if (j >= 0) {
    Sprite_SetSpawnedCoordinates(j, &info);
    sprite_subtype2[j] = 0;
    sprite_ignore_projectile[j] = 0;
  }
}

void Sprite_Hobo_Bubble(int k) {  // 86beb4
  Oam_AllocateFromRegionC(4);
  SpriteDraw_SingleSmall(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  sprite_graphics[k] = (frame_counter >> 4 & 1) + 2;
  if (!sprite_delay_aux1[k]) {
    sprite_graphics[k]++;
    Sprite_MoveZ(k);
    if (!sprite_delay_main[k])
      sprite_state[k] = 0;
  }
  if (sprite_delay_main[k] < 4)
    sprite_graphics[k] = 3;
}

int Hobo_SpawnBubble(int k) {  // 86beed
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0x2b, &info);
  if (j >= 0) {
    Sprite_SetSpawnedCoordinates(j, &info);
    sprite_subtype2[j] = 1;
    sprite_z_vel[j] = 2;
    sprite_delay_main[j] = 96;
    sprite_delay_aux1[j] = 96 >> 1;
    sprite_ignore_projectile[j] = 96 >> 1;
    sprite_flags2[j] = 0;
  }
  return j;
}

void Sprite_Hobo_Fire(int k) {  // 86bf15
  SpriteDraw_SingleSmall(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  sprite_graphics[k] = frame_counter >> 3 & 1;
  sprite_oam_flags[k] &= ~0x40;
  if (!sprite_delay_main[k]) {
    Hobo_SpawnSmoke(k);
    sprite_delay_main[k] = 47;
  }
}

void SpritePrep_Hobo_SpawnFire(int k) {  // 86bf4b
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0x2b, &info);
  if (j >= 0) {
    Sprite_SetX(j, 0x194);
    Sprite_SetY(j, 0x03f);
    sprite_subtype2[j] = 2;
    sprite_ignore_projectile[j] = 2;
    sprite_flags2[j] = 0;
    sprite_oam_flags[j] = sprite_oam_flags[j] & ~0xE | 2;
  }
}

void Sprite_Hobo_Smoke(int k) {  // 86bf81
  static const uint8 kHoboSmoke_OamFlags[4] = {0, 64, 128, 192};
  sprite_graphics[k] = 6;
  SpriteDraw_SingleSmall(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_MoveXY(k);
  Sprite_MoveZ(k);
  sprite_oam_flags[k] = sprite_oam_flags[k] & 0x3f | kHoboSmoke_OamFlags[frame_counter >> 4 & 3];
  if (!sprite_delay_main[k])
    sprite_state[k] = 0;
}

void Hobo_SpawnSmoke(int k) {  // 86bfaf
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0x2b, &info);
  if (j >= 0) {
    Sprite_SetSpawnedCoordinates(j, &info);
    Sprite_SetY(j, info.r2_y - 4);
    sprite_subtype2[j] = 3;
    sprite_z_vel[j] = 7;
    sprite_delay_main[j] = 96;
    sprite_ignore_projectile[j] = 96;
    sprite_flags2[j] = 0;
  }
}

void Sprite_73_UncleAndPriest(int k) {  // 86bfe0
  switch (sprite_E[k]) {
  case 0:
    Sprite_Uncle(k);
    break;
  case 1:
    Sprite_Priest(k);
    break;
  case 2:
    Sprite_SanctuaryMantle(k);
    break;
  }
}

void SpritePrep_UncleAndPriest_bounce(int k) {  // 86bfe5
  if (BYTE(dungeon_room_index) == 18) {
    Priest_SpawnMantle(k);
    if (sram_progress_indicator >= 3)
      sram_progress_flags |= 2;
    if (sram_progress_flags & 2) {
      sprite_state[k] = 0;
      return;
    }
    sprite_E[k] = 1;
    sprite_flags2[k] = sprite_flags2[k] & 0xf0 | 0x2;
    sprite_flags4[k] = 3;
    int j;
    if (link_sword_type >= 2) {
      sprite_D[k] = 4;
      sprite_graphics[k] = 0;
      j = 0;
    } else {
      sprite_D[k] = sprite_head_dir[k] = Sprite_DirectionToFaceLink(k, NULL) ^ 3;
      if (follower_indicator == 1) {
        sram_progress_flags |= 0x4;
        save_ow_event_info[0x1b] |= 0x20;
        sprite_delay_main[k] = 170;
        j = 1;
      } else {
        j = 2;
      }
    }
    sprite_subtype2[k] = j;
    static const int16 kUncleAndSage_Y[3] = {0, -9, 0};
    Sprite_SetX(k, Sprite_GetX(k) - 6);
    Sprite_SetY(k, Sprite_GetY(k) + kUncleAndSage_Y[j]);
    sprite_ignore_projectile[k]++;
    byte_7FFE01 = 0;
  } else if (BYTE(dungeon_room_index) == 4) {
    if (!(sram_progress_flags & 0x10))
      sprite_x_lo[k] += 8;
    else
      sprite_state[k] = 0;
  } else {
    if (!(sram_progress_flags & 1)) {
      sprite_D[k] = 3;
      sprite_subtype2[k] = 1;
    } else {
      sprite_state[k] = 0;
    }
  }
}

void SpritePrep_OldMan_bounce(int k) {  // 86bff9
  sprite_ignore_projectile[k]++;
  if (BYTE(dungeon_room_index) == 0xe4) {
    sprite_subtype2[k] = 2;
    return;
  }
  if (follower_indicator == 0) {
    if (link_item_mirror == 2)
      sprite_state[k] = 0;
    follower_indicator = 4;
    LoadFollowerGraphics();
    follower_indicator = 0;
  } else {
    sprite_state[k] = 0;
    LoadFollowerGraphics();
  }
}

void Sprite_TutorialGuardOrBarrier(int k) {  // 86bffe
  if (sprite_type[k] == 0x40) {
    Sprite_EvilBarrier(k);
    return;
  }
  int jbak = sprite_D[k];
  if (sprite_delay_aux1[k])
    sprite_D[k] = kSoldier_DirectionLockSettings[jbak];

  sprite_graphics[k] = kSprite_TutorialEntities_Tab[sprite_D[k]];
  TutorialSoldier_Draw(k);
  sprite_D[k] = jbak;

  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_CheckDamageFromLink(k);

  if (BYTE(overworld_area_index) == 0x1b && (sprite_y_lo[k] == 0x50 || sprite_y_lo[k] == 0x90)) {
    Sprite_TutorialGuard_ShowMessageOnContact(k, sprite_y_lo[k] == 0x50 ? 0xB2 : 0xB3);
  } else {
    if (Sprite_TutorialGuard_ShowMessageOnContact(k, byte_7E0B69 + 0xf)) {
      byte_7E0B69 = byte_7E0B69 != 6 ? byte_7E0B69 + 1 : 0;
    }
  }
  Sprite_CheckDamageToAndFromLink(k);
  if (((k ^ frame_counter) & 0x1f) == 0) {
    uint8 jbak = sprite_D[k];
    sprite_D[k] = Sprite_DirectionToFaceLink(k, NULL);
    if (sprite_D[k] != jbak && !((sprite_D[k] ^ jbak) & 2))
      sprite_delay_aux1[k] = 12;
  }
}

void Sprite_F2_MedallionTablet(int k) {  // 86c00d
  switch (sprite_subtype2[k]) {
  case 0:
    MedallionTablet_Main(k);
    break;
  case 1:
    Sprite_DustCloud(k);
    break;
  }
}

void Sprite_33_RupeePull(int k) {  // 86c017
  PrepOamCoordsRet info;
  Sprite_PrepOamCoord(k, &info);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (Sprite_CheckIfLinkIsBusy())
    return;
  if (Sprite_CheckDamageToLink_same_layer(k)) {
    link_need_for_pullforrupees_sprite = 1;
    sprite_A[k] = 1;
  } else {
    if (sprite_A[k]) {
      link_need_for_pullforrupees_sprite = 0;
      if (link_state_bits & 1) {
        sprite_state[k] = 0;
        RupeePull_SpawnPrize(k);
        Sprite_SpawnPoofGarnish(k);
      }
    }
  }
}

void Sprite_14_ThievesTownGrate(int k) {  // 86c01c
  PrepOamCoordsRet info;
  Sprite_PrepOamCoord(k, &info);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (Sprite_CheckIfLinkIsBusy())
    return;
  if (Sprite_CheckDamageToLink_same_layer(k)) {
    link_need_for_pullforrupees_sprite = 1;
    sprite_A[k] = 1;
  } else {
    if (!sprite_A[k])
      return;
    link_need_for_pullforrupees_sprite = 0;
    if (!(link_state_bits & 1))
      return;
    SpriteSfx_QueueSfx2WithPan(k, 0x1f);
    OpenGargoylesDomain();
    int j = Sprite_SpawnDustCloud(k);
    Sprite_SetX(j, Sprite_GetX(k));
    Sprite_SetY(j, Sprite_GetY(k));
    sprite_state[k] = 0;
  }
}

void SpritePrep_Snitch_bounce_2(int k) {  // 86c026
  SpritePrep_Snitches(k);
}

void SpritePrep_Snitch_bounce_3(int k) {  // 86c030
  SpritePrep_Snitches(k);
}

void Sprite_37_Waterfall(int k) {  // 86c03a
  switch (sprite_subtype2[k]) {
  case 0: Waterfall(k); break;
  case 1: Sprite_BatCrash(k); break;
  }
}

void Sprite_38_EyeStatue(int k) {  // 86c03f
  if (!sprite_B[k]) {
    PrepOamCoordsRet info;
    Sprite_PrepOamCoord(k, &info);
    if (Sprite_ReturnIfInactive(k))
      return;
    if (Sprite_DirectionToFaceLink(k, NULL) == 2 && sprite_unk2[k] == 9) {
      dung_flag_statechange_waterpuzzle++;
      sprite_B[k] = 1;
    }
  }
}

void Sprite_3A_MagicBat(int k) {  // 86c044
  if (sprite_head_dir[k]) {
    Sprite_MadBatterBolt(k);
    return;
  }

  if (sprite_ai_state[k])
    SpriteDraw_SingleLarge(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_MoveXY(k);
  Sprite_MoveZ(k);
  switch(sprite_ai_state[k]) {
  case 0:  // wait for summon
    if (link_magic_consumption >= 2)
      return;
    if (!Sprite_CheckDamageToLink_same_layer(k))
      return;
    for (int i = 4; i >= 0; i--) {
      if (ancilla_type[i] == 0x1a) {
        Sprite_SpawnSuperficialBombBlast(k);
        SpriteSfx_QueueSfx1WithPan(k, 0xd);
        sprite_ai_state[k]++;
        sprite_A[k] = 20;
        flag_is_link_immobilized = 1;
        sprite_oam_flags[k] |= 32;
        return;
      }
    }
    break;
  case 1:  // RisingUp
    if (!sprite_delay_main[k]) {
      sprite_delay_main[k] = --sprite_A[k];
      if (sprite_delay_main[k] != 1) {
        static const int8 kMadBatter_RisingUp_XAccel[2] = {-8, 7};
        sprite_z_vel[k] = sprite_delay_main[k] >> 2;
        sprite_x_vel[k] += kMadBatter_RisingUp_XAccel[sprite_A[k] & 1];
        sprite_graphics[k] ^= 1;
      } else {
        Sprite_ShowMessageUnconditional(0x110);
        sprite_ai_state[k]++;
        sprite_graphics[k] = 0;
        sprite_z_vel[k] = 0;
        sprite_x_vel[k] = 0;
        sprite_delay_main[k] = 255;
      }
    }
    break;
  case 2: { // PseudoAttackPlayer
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k]++;
      sprite_delay_aux1[k] = 64;
    }
    static const int8 kMadBatter_PseudoAttack_OamFlags[8] = {0xa, 4, 2, 4, 2, 0xa, 4, 2};
    sprite_oam_flags[k] = sprite_oam_flags[k] & ~0xE | kMadBatter_PseudoAttack_OamFlags[sprite_delay_main[k] >> 1 & 7];
    if (sprite_delay_main[k] == 240)
      Sprite_MagicBat_SpawnLightning(k);
    break;
  }
  case 3:  // DoublePlayerMagicPower
    if (!sprite_delay_aux1[k]) {
      Sprite_ShowMessageUnconditional(0x111);
      Palette_Restore_BG_And_HUD();
      flag_update_cgram_in_nmi++;
      sprite_ai_state[k]++;
      link_magic_consumption = 1;
      Hud_RefreshIcon();
    } else if (sprite_delay_aux1[k] == 0x10) {
      intro_times_pal_flash = 0x10;
    }
    break;
  case 4:  // LaterBitches
    Sprite_SpawnDummyDeathAnimation(k);
    sprite_state[k] = 0;
    flag_is_link_immobilized = 0;
    break;
  }
}

void SpritePrep_Zelda_bounce(int k) {  // 86c06c
  if (link_sword_type >= 2) {
    sprite_state[k] = 0;
    return;
  }
  sprite_ignore_projectile[k]++;
  sprite_D[k] = sprite_head_dir[k] = Sprite_DirectionToFaceLink(k, NULL) ^ 3;
  uint8 bak0 = follower_indicator;
  follower_indicator = 1;
  LoadFollowerGraphics();
  follower_indicator = bak0;

  if (BYTE(dungeon_room_index) == 0x12) {
    sprite_subtype2[k] = 2;
    if (!(sram_progress_flags & 4)) {
      sprite_state[k] = 0;
    } else {
      Sprite_SetX(k, Sprite_GetX(k) + 6);
      Sprite_SetY(k, Sprite_GetY(k) + 15);
      sprite_flags4[k] = 3;
    }
  } else {
    sprite_subtype2[k] = 0;
    if (follower_indicator == 1 || (sram_progress_flags & 4))
      sprite_state[k] = 0;
  }
}

void Sprite_78_MrsSahasrahla(int k) {  // 86c071
  ElderWife_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_BehaveAsBarrier(k);
  switch (sprite_ai_state[k]) {
  case 0:  // initial
    if (link_sword_type < 2) {
      if (Sprite_ShowSolicitedMessage(k, 0x2b) & 0x100)
        sprite_ai_state[k] = 1;
    } else {
      Sprite_ShowSolicitedMessage(k, 0x2e);
    }
    sprite_graphics[k] = frame_counter >> 4 & 1;
    break;
  case 1:  // tell legend
    Sprite_ShowMessageUnconditional(0x2c);
    sprite_ai_state[k] = 2;
    break;
  case 2:  // loop until player not dumb
    if (choice_in_multiselect_box == 0) {
      sprite_ai_state[k] = 3;
      Sprite_ShowMessageUnconditional(0x2d);
    } else {
      Sprite_ShowMessageUnconditional(0x2c);
    }
    break;
  case 3:  // go away find old man
    Sprite_ShowSolicitedMessage(k, 0x2d);
    sprite_graphics[k] = frame_counter >> 4 & 1;
    break;
  }
}

void Sprite_16_Elder_bounce(int k) {  // 86c08a
  Elder_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_BehaveAsBarrier(k);
  switch (sprite_subtype2[k]) {
  case 0:
    Sprite_Sahasrahla(k);
    break;
  case 1:
    Sprite_Aginah(k);
    break;
  }
}

void SpritePrep_HeartPiece(int k) {  // 86c0a8
  HeartUpgrade_CheckIfAlreadyObtained(k);
}

void Sprite_2D_TelepathicTile(int k) {  // 86c0b2
  assert(0);
}

void Sprite_25_TalkingTree(int k) {  // 86c0d5
  switch (sprite_subtype2[k]) {
  case 0: TalkingTree_Mouth(k); break;
  case 1: TalkingTree_Eye(k); break;
  }
}

void Sprite_1C_Statue(int k) {  // 86c0e8
  static const uint8 kMovableStatue_Dir[4] = {4, 6, 0, 2};
  static const uint8 kMovableStatue_Joypad[4] = {1, 2, 4, 8};
  static const int8 kMovableStatue_Xvel[4] = {-16, 16, 0, 0};
  static const int8 kMovableStatue_Yvel[4] = {0, 0, -16, 16};
  int j;
  if (sprite_D[k]) {
    sprite_D[k] = 0;
    link_speed_setting = 0;
    bitmask_of_dragstate = 0;
  }
  if (sprite_delay_main[k]) {
    sprite_D[k] = 1;
    bitmask_of_dragstate = 129;
    link_speed_setting = 8;
  }
  MovableStatue_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Statue_BlockSprites(k);
  dung_flag_statechange_waterpuzzle = 0;
  if (Statue_CheckForSwitch(k))
    dung_flag_statechange_waterpuzzle = 1;
  Sprite_MoveXY(k);
  Sprite_Get16BitCoords(k);
  Sprite_CheckTileCollision2(k);
  Sprite_ZeroVelocity_XY(k);
  if (Sprite_CheckDamageToLink_same_layer(k)) {
    sprite_delay_main[k] = 7;
    Sprite_RepelDash();
    if (sprite_delay_aux1[k]) {
      Sprite_NullifyHookshotDrag();
      return;
    }
    j = Sprite_DirectionToFaceLink(k, NULL);
    sprite_x_vel[k] = kMovableStatue_Xvel[j];
    sprite_y_vel[k] = kMovableStatue_Yvel[j];
  } else {
    if (!sprite_delay_main[k])
      sprite_delay_aux1[k] = 13;
    if ((uint16)(cur_sprite_x - link_x_coord + 16) < 35 &&
        (uint16)(cur_sprite_y - link_y_coord + 12) < 36 &&
        link_direction_facing == kMovableStatue_Dir[j = Sprite_DirectionToFaceLink(k, NULL)] &&
        !link_is_running) {
      link_is_near_moveable_statue = 1;
      sprite_A[k] = 1;
      if (!(link_grabbing_wall & 2) || !(kMovableStatue_Joypad[j] & joypad1H_last) || (link_x_vel | link_y_vel) == 0)
        return;
      j ^= 1;
      sprite_x_vel[k] = kMovableStatue_Xvel[j];
      sprite_y_vel[k] = kMovableStatue_Yvel[j];
    } else {
      if (sprite_A[k]) {
        sprite_A[k] = 0;
        link_speed_setting = 0;
        link_grabbing_wall = 0;
        link_is_near_moveable_statue = 0;
        link_cant_change_direction &= ~1;
      }
      return;
    }
  }
  if (!(link_grabbing_wall & 2))
    Sprite_NullifyHookshotDrag();
  if (!(sprite_wallcoll[k] & 15) && !sprite_delay_aux4[k]) {
    SpriteSfx_QueueSfx2WithPan(k, 0x22);
    sprite_delay_aux4[k] = 8;
  }
}

bool Statue_CheckForSwitch(int k) {  // 86c203
  static const int8 kMovableStatue_SwitchX[4] = {3, 12, 3, 12};
  static const int8 kMovableStatue_SwitchY[4] = {3, 3, 12, 12};
  for (int j = 3; j >= 0; j--) {
    uint16 x = Sprite_GetX(k) + kMovableStatue_SwitchX[j];
    uint16 y = Sprite_GetY(k) + kMovableStatue_SwitchY[j];
    uint8 t = GetTileAttribute(sprite_floor[k], &x, y);
    if (t != 0x23 && t != 0x24 && t != 0x25 && t != 0x3b)
      return false;
  }
  return true;
}

void MovableStatue_Draw(int k) {  // 86c264
  static const DrawMultipleData kMovableStatue_Dmd[3] = {
    {0, -8, 0x00c2, 0},
    {8, -8, 0x40c2, 0},
    {0,  0, 0x00c0, 2},
  };
  Sprite_DrawMultiplePlayerDeferred(k, &kMovableStatue_Dmd[0], 3, NULL);
}

void Statue_BlockSprites(int k) {  // 86c277
  for (int j = 15; j >= 0; j--) {
    if (sprite_type[j] == 0x1c || j == k || (j ^ frame_counter) & 1 || sprite_state[j] < 9)
      continue;
    int x = Sprite_GetX(j), y = Sprite_GetY(j);
    if ((uint16)(cur_sprite_x - x + 12) < 24 &&
        (uint16)(cur_sprite_y - y + 12) < 36) {
      sprite_F[j] = 4;
      ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLocation(k, x, y, 32);
      sprite_y_recoil[j] = pt.y;
      sprite_x_recoil[j] = pt.x;
    }
  }
}

void Sprite_1D_FluteQuest(int k) {  // 86c2e5
  PrepOamCoordsRet info;
  Sprite_PrepOamCoord(k, &info);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (BYTE(overworld_screen_index) == 0x18) {
    if (link_item_flute == 3)
      sprite_state[k] = 0;
  } else {
    if (link_item_flute & 2)
      sprite_state[k] = 0;
  }
}

void Sprite_72_FairyPond(int k) {  // 86c319
  if (sprite_A[k]) {
    if (!--sprite_C[k])
      sprite_state[k] = 0;
    sprite_graphics[k] = sprite_C[k] >> 3;
    Oam_AllocateFromRegionC(4);
    SpriteDraw_SingleSmall(k);
    return;
  }
  if (sprite_B[k]) {
    FaerieQueen_Draw(k);
    sprite_graphics[k] = frame_counter >> 4 & 1;
    if (frame_counter & 15)
      return;
    SpriteSpawnInfo info;
    int j = Sprite_SpawnDynamically(k, 0x72, &info);
    if (j >= 0) {
      Sprite_SetX(j, info.r0_x + kWishPond_X[GetRandomNumber() & 7]);
      Sprite_SetY(j, info.r2_y + kWishPond_Y[GetRandomNumber() & 7]);
      sprite_C[j] = 31;
      sprite_A[j] = 31;
      sprite_flags2[j] = 0;
      sprite_flags3[j] = 0x48;
      sprite_oam_flags[j] = 0x48 & 0xf;
      sprite_B[j] = 1;
    }
    return;
  }
  PrepOamCoordsRet info;
  Sprite_PrepOamCoord(k, &info);
  Sprite_WishPond2(k);
}

void Sprite_WishPond2(int k) {  // 86c41d
  WishPond2_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (BYTE(dungeon_room_index) != 21)
    Sprite_WishPond3(k);
  else
    Sprite_HappinessPond(k);
}

void Sprite_HappinessPond(int k) {  // 86c44c
  static const uint8 kHappinessPondCost[4] = {5, 20, 25, 50};
  static const uint8 kHappinessPondCostHex[4] = {5, 0x20, 0x25, 0x50};
  switch (sprite_ai_state[k]) {
  case 0:
    flag_is_link_immobilized = 0;
    if (sprite_delay_main[k] || Sprite_CheckIfLinkIsBusy())
      return;
    if (Sprite_ShowMessageOnContact(k, 0x89) & 0x100) {
      sprite_ai_state[k] = 1;
      Link_ResetProperties_A();
      Ancilla_TerminateSparkleObjects();
      link_direction_facing = 0;
    }
    break;
  case 1:
    if (choice_in_multiselect_box == 0) {
      int i = (link_bomb_upgrades | link_arrow_upgrades) != 0;
      sprite_graphics[k] = i * 2;
      WORD(dialogue_number[0]) = WORD(kHappinessPondCostHex[i * 2]);
      Sprite_ShowMessageUnconditional(0x14e);
      sprite_ai_state[k] = 2;
      flag_is_link_immobilized = 1;
    } else {
show_later_msg:
      Sprite_ShowMessageUnconditional(0x14c);
      sprite_ai_state[k] = 0;
      sprite_delay_main[k] = 255;
    }
    break;
  case 2: {
    int i = sprite_graphics[k] + choice_in_multiselect_box;
    dialogue_number[1] = kHappinessPondCostHex[i];
    if (link_rupees_goal < kHappinessPondCost[i]) {
      goto show_later_msg;
    } else {
      sprite_D[k] = kHappinessPondCost[i];
      sprite_head_dir[k] = i;
      sprite_ai_state[k] = 3;
    }
    break;
  }
  case 3: {
    sprite_delay_main[k] = 80;
    int i = sprite_D[k];
    link_rupees_goal -= i;
    link_rupees_in_pond += i;
    AddHappinessPondRupees(sprite_head_dir[k]);
    if (link_rupees_in_pond >= 100) {
      link_rupees_in_pond -= 100;
      sprite_ai_state[k] = 5;
      return;
    }
    dialogue_number[0] = (link_rupees_in_pond / 10) * 16 + (link_rupees_in_pond % 10);
    sprite_ai_state[k] = 4;
    break;
  }
  case 4:
    if (sprite_delay_main[k] == 0) {
      Sprite_ShowMessageUnconditional(0x94);
      sprite_ai_state[k] = 13;
    }
    break;
  case 5:
    if (sprite_delay_main[k] == 0) {
      SpriteSpawnInfo info;
      int j = Sprite_SpawnDynamically(k, 0x72, &info);
      assert(j >= 0);
      Sprite_SetX(j, info.r0_x);
      Sprite_SetY(j, info.r2_y - 80);
      music_control = 0x1b;
      last_music_control = 0;
      sprite_B[j] = 1;
      Palette_AssertTranslucencySwap();
      PaletteFilter_WishPonds();
      sprite_E[k] = j;
      sprite_ai_state[k] = 6;
      sprite_delay_main[k] = 255;
    }
    break;
  case 6:
    if (!(frame_counter & 7)) {
      PaletteFilter_SP5F();
      if (!BYTE(palette_filter_countdown)) {
        Sprite_ShowMessageUnconditional(0x95);
        Palette_RevertTranslucencySwap();
        TS_copy = 0;
        CGADSUB_copy = 0x20;
        flag_update_cgram_in_nmi++;
        sprite_ai_state[k] = 7;
      }
    }
    break;
  case 7:
    if (!choice_in_multiselect_box)
      sprite_ai_state[k] = 8;
    else
      sprite_ai_state[k] = 12;
    break;
  case 8: {
    static const uint8 kMaxBombsForLevelHex[8] = {0x10, 0x15, 0x20, 0x25, 0x30, 0x35, 0x40, 0x50};
    int i = link_bomb_upgrades + 1;
    if (i != 8) {
      link_bomb_upgrades = i;
      dialogue_number[0] = link_bomb_filler = kMaxBombsForLevelHex[i];
      Sprite_ShowMessageUnconditional(0x96);
    } else {
      link_rupees_goal += 100;
      Sprite_ShowMessageUnconditional(0x98);
    }
    sprite_ai_state[k] = 9;
    break;
  }
  case 9:
    Palette_AssertTranslucencySwap();
    TS_copy = 2;
    CGADSUB_copy = 0x30;
    flag_update_cgram_in_nmi++;
    sprite_ai_state[k] = 10;
    break;
  case 10:
    if (!(frame_counter & 7)) {
      PaletteFilter_SP5F();
      if (BYTE(palette_filter_countdown) == 30) {
        sprite_state[sprite_E[k]] = 0;
      } else if (BYTE(palette_filter_countdown) == 0) {
        sprite_ai_state[k] = 11;
      }
    }
    break;
  case 11:
    PaletteFilter_RestoreSP5F();
    Palette_RevertTranslucencySwap();
    sprite_ai_state[k] = 0;
    sprite_delay_main[k] = 255;
    break;
  case 12: {
    static const uint8 kMaxArrowsForLevelHex[8] = {0x30, 0x35, 0x40, 0x45, 0x50, 0x55, 0x60, 0x70};
    int i = link_arrow_upgrades + 1;
    if (i != 8) {
      link_arrow_upgrades = i;
      dialogue_number[0] = link_arrow_filler = kMaxArrowsForLevelHex[i];
      Sprite_ShowMessageUnconditional(0x97);
    } else {
      link_rupees_goal += 100;
      Sprite_ShowMessageUnconditional(0x98);
    }
    sprite_ai_state[k] = 9;
    break;
  }
  case 13:
    Sprite_ShowMessageUnconditional(0x154);
    sprite_ai_state[k] = 14;
    break;
  case 14: {
    static const uint16 kHappinessPondLuckMsg[4] = {0x150, 0x151, 0x152, 0x153};
    static const uint8 kHappinessPondLuck[4] = {1, 0, 0, 2};
    int i = GetRandomNumber() & 3;
    item_drop_luck = kHappinessPondLuck[i];
    luck_kill_counter = 0;
    Sprite_ShowMessageUnconditional(kHappinessPondLuckMsg[i]);
    sprite_ai_state[k] = 0;
    sprite_delay_main[k] = 255;
    break;
  }
  }
}

void WishPond2_Draw(int k) {  // 86c4b5
  static const DrawMultipleData kWishPond2_Dmd[8] = {
    {32, -64, 0x0024, 0},
    {32, -56, 0x0034, 0},
    {32, -64, 0x0024, 0},
    {32, -56, 0x0034, 0},
    {32, -64, 0x0024, 2},
    {32, -64, 0x0024, 2},
    {32, -64, 0x0024, 2},
    {32, -64, 0x0024, 2},
  };
  if (BYTE(dungeon_room_index) == 21)
    return;
  uint8 t = sprite_ai_state[k];
  if (t != 5 && t != 6 && t != 11 && t != 12)
    return;
  int g = sprite_graphics[k];
  uint8 f = kWishPond2_OamFlags[g];
  if (f == 0xff)
    f = 5;
  sprite_oam_flags[k] = (f & 7) * 2;
  Sprite_DrawMultiple(k, &kWishPond2_Dmd[(kReceiveItem_Tab1[g] >> 1) * 4], 4, NULL);
}

void FaerieQueen_Draw(int k) {  // 86cb26
  static const uint8 kFaerieQueen_Draw_X[24] = {
    0, 16, 0, 8, 16, 24, 0,  8, 16, 24, 0, 16, 0, 16, 0, 8, 16, 24, 0, 8, 16, 24, 0, 16,
  };
  static const uint8 kFaerieQueen_Draw_Y[24] = {
    0,  0, 16, 16, 16, 16, 24, 24, 24, 24, 32, 32, 0, 0, 16, 16, 16, 16, 24, 24, 24, 24, 32, 32,
  };
  static const uint8 kFaerieQueen_Draw_Char[24] = {
    0xc7, 0xc7, 0xcf, 0xca, 0xca, 0xcf, 0xdf, 0xda, 0xda, 0xdf, 0xcb, 0xcb, 0xcd, 0xcd, 0xc9, 0xca,
    0xca, 0xc9, 0xd9, 0xda, 0xda, 0xd9, 0xcb, 0xcb,
  };
  static const uint8 kFaerieQueen_Draw_Flags[24] = {
    0, 0x40, 0, 0, 0x40, 0x40, 0, 0, 0x40, 0x40, 0, 0x40, 0, 0x40, 0, 0, 0x40, 0x40, 0, 0, 0x40, 0x40, 0, 0x40,
  };
  static const uint8 kFaerieQueen_Draw_Big[24] = {
    2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2,
  };
  static const DrawMultipleData kFaerieQueen_Dmd[20] = {
    { 0,  0, 0x00e9, 2},
    {16,  0, 0x40e9, 2},
    { 0,  0, 0x00e9, 2},
    {16,  0, 0x40e9, 2},
    { 0,  0, 0x00e9, 2},
    {16,  0, 0x40e9, 2},
    { 0, 16, 0x00eb, 2},
    {16, 16, 0x40eb, 2},
    { 0, 32, 0x00ed, 2},
    {16, 32, 0x40ed, 2},
    { 0,  0, 0x00ef, 0},
    {24,  0, 0x40ef, 0},
    { 0,  8, 0x00ff, 0},
    {24,  8, 0x40ff, 0},
    { 0,  0, 0x00e9, 2},
    {16,  0, 0x40e9, 2},
    { 0, 16, 0x00eb, 2},
    {16, 16, 0x40eb, 2},
    { 0, 32, 0x00ed, 2},
    {16, 32, 0x40ed, 2},
  };
  if (!savegame_is_darkworld) {
    PrepOamCoordsRet info;
    if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
      return;
    OamEnt *oam = GetOamCurPtr();
    int g = sprite_graphics[k];
    for (int i = 11; i >= 0; i--, oam++) {
      int j = g * 12 + i;
      SetOamPlain(oam, kFaerieQueen_Draw_X[j] + info.x, kFaerieQueen_Draw_Y[j] + info.y,
                   kFaerieQueen_Draw_Char[j], info.flags | kFaerieQueen_Draw_Flags[j], kFaerieQueen_Draw_Big[j]);
    }
    Sprite_CorrectOamEntries(k, 11, 0xff);
  } else {
    Sprite_DrawMultiple(k, &kFaerieQueen_Dmd[sprite_graphics[k] * 10], 10, NULL);
  }
}

void Sprite_71_Leever(int k) {  // 86cba2
  static const uint8 kLeever_EmergeGfx[16] = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 2, 1, 2, 1, 0, 0};
  static const uint8 kLeever_AttackGfx[4] = {9, 10, 11, 12};
  static const uint8 kLeever_AttackSpd[2] = {12, 8};
  static const uint8 kLeever_SubmergeGfx[16] = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 2, 1, 2, 1, 0, 0};

  if (sprite_ai_state[k])
    Leever_Draw(k);
  else {
    PrepOamCoordsRet info;
    Sprite_PrepOamCoord(k, &info);
  }
  if (sprite_pause[k])
    sprite_state[k] = 8;
  if (Sprite_ReturnIfInactive(k))
    return;
  if (Sprite_ReturnIfRecoiling(k))
    return;
  switch(sprite_ai_state[k]) {
  case 0:  // under sand
    sprite_ignore_projectile[k] = sprite_delay_main[k];
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k] = 1;
      sprite_delay_main[k] = 127;
    } else {
      Sprite_ApplySpeedTowardsLink(k, 16);
      Sprite_MoveXY(k);
      Sprite_CheckTileCollision2(k);
    }
    break;
  case 1:  // emerge
    sprite_ignore_projectile[k] = sprite_delay_main[k];
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k]++;
      sprite_delay_main[k] = (GetRandomNumber() & 63) + 160;
      Sprite_ZeroVelocity_XY(k);
    } else {
      sprite_graphics[k] = kLeever_EmergeGfx[sprite_delay_main[k] >> 3];
    }
    break;
  case 2:  // attack
    Sprite_CheckDamageToAndFromLink(k);
    if (!sprite_delay_main[k]) {
stop_attack:
      sprite_ai_state[k]++;
      sprite_delay_main[k] = 127;
    } else {
      if (!(sprite_subtype2[k] & 7))
        Sprite_ApplySpeedTowardsLink(k, kLeever_AttackSpd[sprite_A[k]]);
      Sprite_MoveXY(k);
      if (Sprite_CheckTileCollision(k))
        goto stop_attack;
      sprite_graphics[k] = kLeever_AttackGfx[++sprite_subtype2[k] >> 2 & 3];
    }
    break;
  case 3:  // submerge
    sprite_ignore_projectile[k] = sprite_delay_main[k];
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k] = 0;
      sprite_delay_main[k] = (GetRandomNumber() & 31) + 64;
    } else {
      sprite_graphics[k] = kLeever_SubmergeGfx[sprite_delay_main[k] >> 3 ^ 15];
    }
    break;
  }

}

void Leever_Draw(int k) {  // 86ce45
  static const uint8 kLeever_Draw_Num[14] = {1, 1, 1, 3, 3, 3, 3, 3, 3, 1, 1, 1, 1, 1};
  static const int8 kLeever_Draw_X[56] = {
    2, 6, 6, 6, 0, 8, 8, 8, 0, 8, 8, 8, 0, 8, 0, 8,
    0, 8, 0, 8, 0, 0, 0, 8, 0, 0, 0, 8, 0, 0, 0, 8,
    0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
  };
  static const int8 kLeever_Draw_Y[56] = {
    8,  8,  8, 8, 8,  8,  8, 8, 8,  8,  8, 8, 5,  5,  8, 8,
    5,  5,  8, 8, 2,  2,  8, 8, 1,  1,  8, 8, 0,  0,  8, 8,
    -1, -1,  8, 8, 8, -2, -2, 0, 8, -2, -2, 0, 8, -2, -2, 0,
    8, -2, -2, 0, 8, -2, -2, 0,
  };
  static const uint8 kLeever_Draw_Char[56] = {
    0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x38, 0x38, 0x38, 0x38,    8, 9, 0x28, 0x28,
    8,    9, 0xd9, 0xd9,    8,    8, 0xd8, 0xd8,    8,    8, 0xda, 0xda,    6, 6, 0xd9, 0xd9,
    0x26, 0x26, 0xd8, 0xd8, 0x6c,    6,    6,    0, 0x6c, 0x26, 0x26,    0, 0x6c, 6,    6,    0,
    0x6c, 0x26, 0x26,    0, 0x6c,    8,    8,    0,
  };
  static const uint8 kLeever_Draw_Flags[56] = {
    1, 0x41, 0x41, 0x41, 1, 0x41, 0x41, 0x41, 1, 0x41, 0x41, 0x41, 1, 1, 1, 0x41,
    1,    1,    0, 0x40, 1,    1,    0, 0x40, 1,    1,    0, 0x40, 1, 1, 0, 0x40,
    0,    1,    0, 0x40, 6, 0x41, 0x41,    0, 6, 0x41, 0x41,    0, 6, 1, 1,    0,
    6,    1,    1,    0, 6,    1,    1,    0,
  };
  static const uint8 kLeever_Draw_Big[56] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 2, 2, 0, 0, 2, 2, 0, 0, 2, 2, 0, 0,
    2, 2, 0, 0, 2, 2, 2, 0, 2, 2, 2, 0, 2, 2, 2, 0,
    2, 2, 2, 0, 2, 2, 2, 0,
  };
  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  OamEnt *oam = GetOamCurPtr();
  int d = sprite_graphics[k];
  for (int i = kLeever_Draw_Num[d]; i >= 0; i--, oam++) {
    int j = d * 4 + i;
    uint8 charnum = kLeever_Draw_Char[j];
    uint8 f = info.flags;
    if (charnum >= 0x60 || charnum == 0x28 || charnum == 0x38)
      f &= 0xf0;
    SetOamHelper0(oam, info.x + kLeever_Draw_X[j], info.y + kLeever_Draw_Y[j],
                  charnum, kLeever_Draw_Flags[j] | f, kLeever_Draw_Big[j]);
  }
}

void Sprite_D8_Heart(int k) {  // 86cec0
  if (SpriteDraw_AbsorbableTransient(k, true))
    return;
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_CheckAbsorptionByPlayer(k);

  // Avoid calling Sprite_HandleAbsorptionByPlayer twice, it's called
  // also from within Sprite_HandleDraggingByAncilla
  if (sprite_state[k] == 0 && (enhanced_features0 & kFeatures0_MiscBugFixes))
    return;

  if (Sprite_HandleDraggingByAncilla(k))
    return;
  Sprite_MoveXY(k);
  Sprite_MoveZ(k);
  if (sign8(sprite_z[k])) {
    sprite_z[k] = 0;
    sprite_ai_state[k]++;
    sprite_graphics[k] = 0;
  }
  sprite_oam_flags[k] &= ~0x40;
  if (!sign8(sprite_x_vel[k]))
    sprite_oam_flags[k] |= 0x40;
  switch (sprite_ai_state[k] >= 3 ? 3 : sprite_ai_state[k]) {
  case 0:  // InitializeAscent
    sprite_ai_state[k]++;
    sprite_delay_main[k] = 18;
    sprite_z_vel[k] = 20;
    sprite_graphics[k] = 1;
    sprite_D[k] = 0;
    break;
  case 1:  // BeginDescending
    if (sprite_delay_main[k] != 0) {
      sprite_z_vel[k]--;
    } else {
      sprite_ai_state[k]++;
      sprite_z_vel[k] = 253;
      sprite_x_vel[k] = 0;
    }
    break;
  case 2:  // GlideGroundward
    if (sprite_delay_main[k] == 0) {
      int j = sprite_D[k] & 1;
      sprite_x_vel[k] += kHeartRefill_AccelX[j];
      if (sprite_x_vel[k] == (uint8)kHeartRefill_VelTarget[j]) {
        sprite_D[k]++;
        sprite_delay_main[k] = 8;
      }
    }
    break;
  case 3:  // Grounded
    sprite_z_vel[k] = sprite_x_vel[k] = sprite_y_vel[k] = 0;
    break;
  }
}

void Sprite_E3_Fairy(int k) {  // 86cf94
  sprite_ignore_projectile[k] = 1;
  if (!sprite_ai_state[k]) {
    if (!player_is_indoors)
      sprite_obj_prio[k] = 48;
    if (SpriteDraw_AbsorbableTransient(k, true))
      return;
  }
  Fairy_CheckIfTouchable(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  switch (sprite_ai_state[k]) {
  case 0: // normal
    if (!sprite_delay_aux4[k]) {
      if (Sprite_CheckDamageToLink(k)) {
        Sprite_HandleAbsorptionByPlayer(k);
      } else if (Sprite_CheckDamageFromLink(k) & kCheckDamageFromPlayer_Ne) {
        sprite_ai_state[k]++;
        Sprite_ShowMessageUnconditional(0xc9);
        return;
      }
    }
    // Avoid calling Sprite_HandleAbsorptionByPlayer twice, it's called
    // also from within Sprite_HandleDraggingByAncilla
    if (sprite_state[k] == 0 && (enhanced_features0 & kFeatures0_MiscBugFixes))
      return;
    if (Sprite_HandleDraggingByAncilla(k))
      return;
    Faerie_HandleMovement(k);
    break;
  case 1: // capture
    if (choice_in_multiselect_box == 0) {
      int j = Sprite_Find_EmptyBottle();
      if (j >= 0) {
        link_bottle_info[j] = 6;
        Hud_RefreshIcon();
        sprite_state[k] = 0;
        return;
      }
      Sprite_ShowMessageUnconditional(0xca);
    }
    sprite_delay_aux4[k] = 48;
    sprite_ai_state[k] = 0;
    break;
  }
}

void Fairy_CheckIfTouchable(int k) {  // 86d011
  if (submodule_index == 2 && (dialogue_message_index == 0xc9 || dialogue_message_index == 0xca))
    sprite_delay_aux4[k] = 40;
}

void Sprite_E4_SmallKey(int k) {  // 86d032
  if (dung_savegame_state_bits & (kAbsorbBigKey[sprite_die_action[k]] << 8)) {
    sprite_state[k] = 0;
    return;
  }
  Sprite_DrawRippleIfInWater(k);
  if (SpriteDraw_AbsorbableTransient(k, false))
    return;
  Sprite_Absorbable_Main(k);
}

void Sprite_D9_GreenRupee(int k) {  // 86d04a
  Sprite_DrawRippleIfInWater(k);
  if (SpriteDraw_AbsorbableTransient(k, true))
    return;
  Sprite_Absorbable_Main(k);
}

void Sprite_Absorbable_Main(int k) {  // 86d051
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_MoveZ(k);
  Sprite_MoveXY(k);
  if (sprite_delay_aux3[k] == 0) {
    Sprite_CheckTileCollision2(k);
    Sprite_BounceOffWall(k);
  }
  sprite_z_vel[k] -= 2;
  if (sign8(sprite_z[k])) {
    sprite_z[k] = 0;
    sprite_x_vel[k] = (int8)sprite_x_vel[k] >> 1;
    sprite_y_vel[k] = (int8)sprite_y_vel[k] >> 1;
    uint8 t = -sprite_z_vel[k];
    t >>= 1;
    if (t < 9) {
      sprite_z_vel[k] = sprite_x_vel[k] = sprite_y_vel[k] = 0;
    } else {
      sprite_z_vel[k] = t;
      if (sprite_I[k] == 8 || sprite_I[k] == 9) {
        sprite_z_vel[k] = 0;
        int j = Sprite_SpawnSmallSplash(k);
        if (j >= 0 && sprite_flags3[k] & 0x20) {
          // wtf carry propagation
          Sprite_SetX(j, Sprite_GetX(j) - 4);
          Sprite_SetY(j, Sprite_GetY(j) - 4);
        }
      } else {
        if (sprite_type[k] >= 0xe4 && player_is_indoors)
          SpriteSfx_QueueSfx2WithPan(k, 5);
      }
    }
  }
  if (Sprite_HandleDraggingByAncilla(k))
    return;
  Sprite_CheckAbsorptionByPlayer(k);
}

void Sprite_08_Octorok(int k) {  // 86d377
  static const uint8 kOctorock_Tab0[20] = {
    0, 2, 2, 2, 1, 1, 1, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 1, 1, 0,
  };
  static const uint8 kOctorock_Tab1[10] = {2, 2, 2, 2, 2, 2, 2, 2, 1, 0};
  static const uint8 kOctorock_NextDir[4] = {2, 3, 1, 0};
  static const uint8 kOctorock_Dir[4] = {3, 2, 0, 1};
  static const int8 kOctorock_Xvel[4] = {24, -24, 0, 0};
  static const int8 kOctorock_Yvel[4] = {0, 0, 24, -24};
  static const uint8 kOctorock_OamFlags[4] = {0x40, 0, 0, 0};

  int j = sprite_D[k];
  if (sprite_delay_aux1[k])
    sprite_D[k] = kOctorock_Dir[j];
  sprite_oam_flags[k] = sprite_oam_flags[k] & ~0x40 | kOctorock_OamFlags[j] | (sprite_graphics[k] == 7 ? 0x40 : 0);
  Octorock_Draw(k);
  sprite_D[k] = j;

  if (Sprite_ReturnIfInactive(k))
    return;
  if (Sprite_ReturnIfRecoiling(k))
    return;
  Sprite_MoveXY(k);
  Sprite_CheckDamageToAndFromLink(k);
  if (!(sprite_ai_state[k] & 1)) {
    sprite_graphics[k] = ++sprite_subtype2[k] >> 3 & 3 | (sprite_D[k] & 2) << 1;
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k]++;
      sprite_delay_main[k] = sprite_type[k] == 8 ? 60 : 160;
    } else {
      j = sprite_D[k];
      sprite_x_vel[k] = kOctorock_Xvel[j];
      sprite_y_vel[k] = kOctorock_Yvel[j];
      if (Sprite_CheckTileCollision(k))
        sprite_D[k] ^= 1;
    }
    return;
  } else {
    Sprite_ZeroVelocity_XY(k);
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k]++;
      sprite_delay_main[k] = (GetRandomNumber() & 0x3f) + 48;
      sprite_D[k] = sprite_delay_main[k] & 3;
    } else {
      switch (sprite_type[k]) {
      case 8:  // normal
        j = sprite_delay_main[k];
        if (j == 28)
          Octorok_FireLoogie(k);
        sprite_C[k] = kOctorock_Tab0[j >> 3];
        break;
      case 10:  // four shooter
        j = sprite_delay_main[k];
        if (j < 128) {
          if (!(j & 15))
            sprite_D[k] = kOctorock_NextDir[sprite_D[k]];
          if ((j & 15) == 8)
            Octorok_FireLoogie(k);
        }
        sprite_C[k] = kOctorock_Tab1[j >> 4];
        break;
      }
    }
  }
}

void Octorok_FireLoogie(int k) {  // 86d4cd
  static const int8 kOctorock_Spit_X[4] = {12, -12, 0, 0};
  static const int8 kOctorock_Spit_Y[4] = {4, 4, 12, -12};
  static const int8 kOctorock_Spit_Xvel[4] = {44, -44, 0, 0};
  static const int8 kOctorock_Spit_Yvel[4] = {0, 0, 44, -44};
  SpriteSpawnInfo info;
  SpriteSfx_QueueSfx2WithPan(k, 0x7);
  int j = Sprite_SpawnDynamically(k, 0xc, &info);
  if (j >= 0) {
    int i = sprite_D[k];
    Sprite_SetX(j, info.r0_x + kOctorock_Spit_X[i]);
    Sprite_SetY(j, info.r2_y + kOctorock_Spit_Y[i]);
    sprite_x_vel[j] = kOctorock_Spit_Xvel[i];
    sprite_y_vel[j] = kOctorock_Spit_Yvel[i];
  }
}

void Octorock_Draw(int k) {  // 86d54a
  static const int8 kOctorock_Draw_X[9] = {8, 0, 4, 8, 0, 4, 9, -1, 4};
  static const int8 kOctorock_Draw_Y[9] = {6, 6, 9, 6, 6, 9, 6, 6, 9};
  static const uint8 kOctorock_Draw_Char[9] = {0xbb, 0xbb, 0xba, 0xab, 0xab, 0xaa, 0xa9, 0xa9, 0xb9};
  static const uint8 kOctorock_Draw_Flags[9] = {0x65, 0x25, 0x25, 0x65, 0x25, 0x25, 0x65, 0x25, 0x25};
  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  if (sprite_D[k] != 3) {
    OamEnt *oam = GetOamCurPtr();
    int j = sprite_C[k] * 3 + sprite_D[k];
    SetOamHelper0(oam, info.x + kOctorock_Draw_X[j], info.y + kOctorock_Draw_Y[j],
                  kOctorock_Draw_Char[j], kOctorock_Draw_Flags[j] | info.flags, 0);
  }
  oam_cur_ptr += 4;
  oam_ext_cur_ptr++;
  sprite_flags2[k]--;
  Sprite_PrepAndDrawSingleLargeNoPrep(k, &info);
  sprite_flags2[k]++;
}

void Sprite_0C_OctorokStone(int k) {  // 86d5b9
  if (sprite_state[k] == 6) {
    SpriteDraw_OctorokStoneCrumbling(k);
    if (Sprite_ReturnIfPaused(k))
      return;
    if (sprite_delay_main[k] == 30)
      SpriteSfx_QueueSfx2WithPan(k, 0x1f);
  } else {
    SpriteDraw_SingleLarge(k);
    if (Sprite_ReturnIfInactive(k))
      return;
    Sprite_CheckDamageToLink(k);
    Sprite_MoveXY(k);
    if (!((k ^ frame_counter) & 3) && Sprite_CheckTileCollision(k))
      Sprite_Func3(k);
  }
}

void SpriteDraw_OctorokStoneCrumbling(int k) {  // 86d643
  static const int8 kOctostone_Draw_X[16] = {0, 8, 0, 8, -8, 16, -8, 16, -12, 20, -12, 20, -14, 22, -14, 22};
  static const int8 kOctostone_Draw_Y[16] = {0, 0, 8, 8, -8, -8, 16, 16, -12, -12, 20, 20, -14, -14, 22, 22};
  static const uint8 kOctostone_Draw_Flags[16] = {0, 0x40, 0x80, 0xc0, 0, 0x40, 0x80, 0xc0, 0, 0x40, 0x80, 0xc0, 0, 0x40, 0x80, 0xc0};

  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  OamEnt *oam = GetOamCurPtr();
  int g = ((sprite_delay_main[k] >> 1 & 0xc) ^ 0xc);
  for (int i = 3; i >= 0; i--, oam++) {
    int j = g + i;
    SetOamHelper0(oam, info.x + kOctostone_Draw_X[j], info.y + kOctostone_Draw_Y[j],
                  0xbc, kOctostone_Draw_Flags[j] | 0x2d, 0);
  }

}

void Sprite_0F_Octoballoon(int k) {  // 86d6aa
  static const uint8 kSprite_Octoballoon_Z[8] = {16, 17, 18, 19, 20, 19, 18, 17};
  sprite_z[k] = kSprite_Octoballoon_Z[sprite_subtype2[k] >> 3 & 7];
  Octoballoon_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (!sprite_delay_main[k]) {
    sprite_delay_main[k] = 3;
    if (!Octoballoon_Find()) {
      sprite_state[k] = 6;
      sprite_hit_timer[k] = 0;
      sprite_delay_main[k] = 15;
      return;
    }
  }
  if (Sprite_ReturnIfRecoiling(k))
    return;
  sprite_subtype2[k]++;
  if (!((k ^ frame_counter) & 15)) {
    ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLink(k, 4);
    if (sprite_x_vel[k] - pt.x)
      sprite_x_vel[k] += sign8(sprite_x_vel[k] - pt.x) ? 1 : -1;
    if (sprite_y_vel[k] - pt.y)
      sprite_y_vel[k] += sign8(sprite_y_vel[k] - pt.y) ? 1 : -1;
  }
  Sprite_MoveXY(k);
  if (Sprite_CheckDamageToLink(k))
    Octoballoon_RecoilLink(k);
  Sprite_CheckDamageFromLink(k);
  Sprite_CheckTileCollision2(k);
  Sprite_BounceOffWall(k);
}

void Octoballoon_RecoilLink(int k) {  // 86d72b
  if (!link_incapacitated_timer) {
    link_incapacitated_timer = 4;
    Sprite_ApplyRecoilToLink(k, 16);
    Sprite_InvertSpeed_XY(k);
  }
}

void Octoballoon_Draw(int k) {  // 86d784
  static const int8 kOctoballoon_Draw_X[12] = {-4, 4, -4, 4, -8, 8, -8, 8, -4, 4, -4, 4};
  static const int8 kOctoballoon_Draw_Y[12] = {-4, -4, 4, 4, -8, -8, 8, 8, -4, -4, 4, 4};
  static const uint8 kOctoballoon_Draw_Char[12] = {0x8c, 0x8c, 0x9c, 0x9c, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86};
  static const uint8 kOctoballoon_Draw_Flags[12] = {0, 0x40, 0, 0x40, 0, 0x40, 0x80, 0xc0, 0, 0x40, 0x80, 0xc0};
  int d = 0;
  if (sprite_state[k] == 6) {
    if (sprite_delay_main[k] == 6 && !submodule_index)
      Octoballoon_FormBabby(k);
    d = (sprite_delay_main[k] >> 1 & 4) + 4;
  }
  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  OamEnt *oam = GetOamCurPtr();
  for (int i = 3; i >= 0; i--, oam++) {
    int j = d + i;
    SetOamHelper0(oam, info.x + kOctoballoon_Draw_X[j], info.y + kOctoballoon_Draw_Y[j],
                  kOctoballoon_Draw_Char[j], kOctoballoon_Draw_Flags[j] | info.flags, 2);
  }
  SpriteDraw_Shadow(k, &info);
}

void Octoballoon_FormBabby(int k) {  // 86d80e
  static const int8 kOctoballoon_Spawn_Xv[6] = {16, 11, -11, -16, -11, 11};
  static const int8 kOctoballoon_Spawn_Yv[6] = {0, 11, 11, 0, -11, -11};

  SpriteSfx_QueueSfx2WithPan(k, 0xc);
  for (int i = 5; i >= 0; i--) {
    SpriteSpawnInfo info;
    int j = Sprite_SpawnDynamically(k, 0x10, &info);
    if (j >= 0) {
      Sprite_SetSpawnedCoordinates(j, &info);
      sprite_x_vel[j] = kOctoballoon_Spawn_Xv[i];
      sprite_y_vel[j] = kOctoballoon_Spawn_Yv[i];
      sprite_z_vel[j] = 48;
      sprite_subtype2[j] = 255;
    }
  }
}

void Sprite_10_OctoballoonBaby(int k) {  // 86d853
  if (!sprite_subtype2[k])
    sprite_state[k] = 0;
  if (sprite_subtype2[k] >= 64 || !(sprite_subtype2[k] & 1))
    SpriteDraw_SingleSmall(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  sprite_subtype2[k]--;
  if (Sprite_ReturnIfRecoiling(k))
    return;
  sprite_z_vel[k]--;
  Sprite_MoveZ(k);
  if (sign8(sprite_z[k])) {
    sprite_z[k] = 0;
    sprite_z_vel[k] = 16;
  }
  Sprite_MoveXY(k);
  Sprite_CheckTileCollision2(k);
  Sprite_BounceOffWall(k);
  Sprite_CheckDamageToAndFromLink(k);
}

void Sprite_0D_Buzzblob(int k) {  // 86d89a
  static const uint8 kBuzzBlob_Gfx[4] = {0, 1, 0, 2};
  static const uint8 kBuzzBlob_ObjPrio[4] = {10, 2, 8, 2};
  if (sprite_delay_aux1[k])
    sprite_obj_prio[k] = sprite_obj_prio[k] & 0xf1 | kBuzzBlob_ObjPrio[sprite_delay_aux1[k] >> 1 & 3];
  Sprite_Cukeman(k);
  BuzzBlob_Draw(k);
  sprite_graphics[k] = kBuzzBlob_Gfx[sprite_subtype2[k] >> 3 & 3] + (sprite_delay_aux1[k] ? 3 : 0);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (Sprite_ReturnIfRecoiling(k))
    return;
  sprite_subtype2[k]++;
  if (!sprite_delay_main[k])
    Buzzblob_SelectNewDirection(k);
  if (!sprite_delay_aux1[k])
    Sprite_MoveXY(k);
  Sprite_CheckTileCollision2(k);
  Sprite_BounceOffWall(k);
  Sprite_CheckDamageToAndFromLink(k);
}

void Buzzblob_SelectNewDirection(int k) {  // 86d906
  static const int8 kBuzzBlob_Xvel[8] = {3, 2, -2, -3, -2, 2, 0, 0};
  static const int8 kBuzzBlob_Yvel[8] = {0, 2, 2, 0, -2, -2, 0, 0};
  static const uint8 kBuzzBlob_Delay[8] = {48, 48, 48, 48, 48, 48, 64, 64};
  int j = GetRandomNumber() & 7;
  sprite_x_vel[k] = kBuzzBlob_Xvel[j];
  sprite_y_vel[k] = kBuzzBlob_Yvel[j];
  sprite_delay_main[k] = kBuzzBlob_Delay[j];
}

void BuzzBlob_Draw(int k) {  // 86d953
  static const uint16 kBuzzBlob_DrawX[3] = {0, 8, 0};
  static const int16 kBuzzBlob_DrawY[3] = {-8, -8, 0};
  static const uint8 kBuzzBlob_DrawChar[18] = { 0xf0, 0xf0, 0xe1, 0, 0, 0xce, 0, 0, 0xce, 0xe3, 0xe3, 0xca, 0xe4, 0xe5, 0xcc, 0xe5, 0xe4, 0xcc };
  static const uint8 kBuzzBlob_DrawFlags[18] = { 0, 0x40, 0, 0, 0, 0, 0, 0, 0x40, 0, 0x40, 0, 0, 0, 0, 0x40, 0x40, 0x40 };
  static const uint8 kBuzzBlob_DrawExt[3] = {0, 0, 2};

  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  OamEnt *oam = GetOamCurPtr();
  int g = sprite_graphics[k];
  for (int i = 2; i >= 0; i--, oam++) {
    SetOamHelper0(oam, info.x + kBuzzBlob_DrawX[i], info.y + kBuzzBlob_DrawY[i],
                  kBuzzBlob_DrawChar[g * 3 + i],
                  kBuzzBlob_DrawFlags[g * 3 + i] | info.flags, kBuzzBlob_DrawExt[i]);
    if (oam->charnum == 0)
      oam->y = 240;
  }
  SpriteDraw_Shadow(k, &info);
}

void Sprite_02_StalfosHead(int k) {  // 86ddb7
  static const uint8 kStalfosHead_OamFlags[4] = {0, 0, 0, 0x40};
  static const uint8 kStalfosHead_Gfx[4] = {0, 1, 2, 1};

  sprite_floor[k] = link_is_on_lower_level;
  if (sprite_delay_aux1[k])
    Oam_AllocateFromRegionC(8);
  int j = sprite_subtype2[k] >> 3 & 3;
  sprite_oam_flags[k] = sprite_oam_flags[k] & ~0x40 | kStalfosHead_OamFlags[j];
  sprite_graphics[k] = kStalfosHead_Gfx[j];
  sprite_obj_prio[k] = 0x30;
  SpriteDraw_SingleLarge(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (Sprite_ReturnIfRecoiling(k))
    return;
  Sprite_CheckDamageToAndFromLink(k);
  if (sprite_F[k])
    Sprite_ZeroVelocity_XY(k);
  Sprite_MoveXY(k);
  sprite_subtype2[k]++;
  ProjectSpeedRet pt;
  if (sprite_delay_main[k]) {
    if (sprite_delay_main[k] & 1)
      return;
    pt = Sprite_ProjectSpeedTowardsLink(k, 16);
  } else {
    if ((k ^ frame_counter) & 3)
      return;
    pt = Sprite_ProjectSpeedTowardsLink(k, 16);
    pt.x = -pt.x;
    pt.y = -pt.y;
  }
  if (sprite_x_vel[k] - pt.x != 0)
    sprite_x_vel[k] += sign8(sprite_x_vel[k] - pt.x) ? 1 : -1;
  if (sprite_y_vel[k] - pt.y != 0)
    sprite_y_vel[k] += sign8(sprite_y_vel[k] - pt.y) ? 1 : -1;
}

bool Pipe_ValidateEntry() {  // 87f4f1
  for (int k = 4; k >= 0; k--) {
    if (ancilla_type[k] == 0x31) {
      link_position_mode = 0;
      link_cant_change_direction = 0;
      ancilla_type[k] = 0;
      break;
    }
  }
  return ((link_state_bits & 0x80) | link_auxiliary_state) != 0;
}

void Ancilla_TerminateSparkleObjects() {  // 89adc7
  for (int i = 4; i >= 0; i--) {
    uint8 t = ancilla_type[i];
    if (t == 0x2a || t == 0x2b || t == 0x30 || t == 0x31 || t == 0x18 || t == 0x19 || t == 0xc)
      ancilla_type[i] = 0;
  }
}

int Sprite_SpawnSuperficialBombBlast(int k) {  // 89ae40
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0x4a, &info);
  if (j >= 0) {
    sprite_state[j] = 6;
    sprite_delay_aux1[j] = 31;
    sprite_C[j] = 3;
    sprite_flags2[j] = 3;
    sprite_oam_flags[j] = 4;
    SpriteSfx_QueueSfx2WithPan(k, 0x15);
    Sprite_SetSpawnedCoordinates(j, &info);
  }
  return j;
}

void Sprite_SpawnDummyDeathAnimation(int k) {  // 89ae7e
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0xb, &info);
  if (j >= 0) {
    Sprite_SetSpawnedCoordinates(j, &info);
    sprite_state[j] = 6;
    sprite_delay_main[j] = 15;
    SpriteSfx_QueueSfx2WithPan(k, 0x14);
    sprite_floor[j] = 2;
  }
}

void Sprite_MagicBat_SpawnLightning(int k) {  // 89aea8
  static const int8 kSpawnMadderBolts_Xvel[4] = {-8, -4, 4, 8};
  static const int8 kSpawnMadderBolts_St2[4] = {0, 0x11, 0x22, 0x33};
  for (int i = 0; i < 4; i++) {
    SpriteSpawnInfo info;
    int j = Sprite_SpawnDynamically(k, 0x3a, &info);
    if (j >= 0) {
      SpriteSfx_QueueSfx3WithPan(k, 0x1);
      Sprite_SetSpawnedCoordinates(j, &info);
      Sprite_SetX(j, info.r0_x + 4);
      Sprite_SetY(j, info.r2_y + 12 - sprite_z[k]);
      sprite_z[j] = 0;
      sprite_y_vel[j] = 24;
      sprite_head_dir[j] = 24;
      sprite_ignore_projectile[j] = 24;
      sprite_flags2[j] = 0x80;
      sprite_flags3[j] = 3;
      sprite_oam_flags[j] = 3;
      sprite_delay_main[j] = 32;
      sprite_graphics[j] = 2;
      int i = sprite_G[k];
      sprite_x_vel[j] = kSpawnMadderBolts_Xvel[i];
      sprite_subtype2[j] = kSpawnMadderBolts_St2[i];
      sprite_floor[j] = 2;
      sprite_G[k]++;
    }
  }
}

void Fireball_SpawnTrailGarnish(int k) {  // 89b020
  if ((k ^ frame_counter) & 3)
    return;
  int j = GarnishAlloc();
  garnish_type[j] = 8;
  garnish_active = 8;
  garnish_countdown[j] = 11;
  garnish_x_lo[j] = cur_sprite_x;
  garnish_x_hi[j] = cur_sprite_x >> 8;
  garnish_y_lo[j] = cur_sprite_y + 16;
  garnish_y_hi[j] = (cur_sprite_y + 16) >> 8;
  garnish_sprite[j] = k;
}

void GarnishSpawn_PyramidDebris(int8 x, int8 y, int8 xvel, int8 yvel) {  // 89b1bd
  int k = GarnishAllocForce();
  sound_effect_2 = 3;
  sound_effect_1 = 31;
  sound_effect_ambient = 5;

  garnish_type[k] = 19;
  garnish_active = 19;
  garnish_x_lo[k] = 232 + x;
  garnish_y_lo[k] = 96 + y;
  garnish_x_vel[k] = xvel;
  garnish_y_vel[k] = yvel;
  garnish_countdown[k] = (GetRandomNumber() & 31) + 48;
}

void Snitch_SpawnGuard(int k) {  // 89c02f
  static const uint16 kCrazyVillageSoldier_X[3] = {0x120, 0x340, 0x2e0};
  static const uint16 kCrazyVillageSoldier_Y[3] = {0x100, 0x3b0, 0x160};
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamicallyEx(k, 0x45, &info, 0);
  if (j < 0)
    return;
  int i = sprite_type[k] == 0x3d ? 0 :
          sprite_type[k] == 0x35 ? 1 : 2;
  Sprite_SetX(j, kCrazyVillageSoldier_X[i] + (sprcoll_x_base & 0xff00));
  Sprite_SetY(j, kCrazyVillageSoldier_Y[i] + (sprcoll_y_base & 0xff00));
  sprite_floor[j] = 0;
  sprite_health[j] = 4;
  sprite_defl_bits[j] = 0x80;
  sprite_flags5[j] = 0x90;
  sprite_oam_flags[j] = 0xb;
}

void Babusu_Draw(int k) {  // 8dbd20
  static const DrawMultipleData kBabusu_Dmd[40] = {
    { 0,  4, 0x4380, 0},
    { 0,  4, 0x4380, 0},
    { 0,  4, 0x43b6, 0},
    { 0,  4, 0x43b6, 0},
    { 0,  4, 0x43b7, 0},
    { 8,  4, 0x0380, 0},
    { 0,  4, 0x4380, 0},
    { 8,  4, 0x03b6, 0},
    { 8,  4, 0x03b7, 0},
    { 8,  4, 0x03b7, 0},
    { 8,  4, 0x0380, 0},
    { 8,  4, 0x0380, 0},
    { 4,  0, 0x8380, 0},
    { 4,  0, 0x8380, 0},
    { 4,  0, 0x83b6, 0},
    { 4,  0, 0x83b6, 0},
    { 4,  0, 0x83b7, 0},
    { 4,  8, 0x0380, 0},
    { 4,  0, 0x8380, 0},
    { 4,  8, 0x03b6, 0},
    { 4,  8, 0x03b7, 0},
    { 4,  8, 0x03b7, 0},
    { 4,  8, 0x0380, 0},
    { 4,  8, 0x0380, 0},
    { 0, -8, 0x0a4e, 2},
    { 0,  0, 0x0a5e, 2},
    { 0, -8, 0x4a4e, 2},
    { 0,  0, 0x4a5e, 2},
    { 8,  0, 0x0a6c, 2},
    { 0,  0, 0x0a6b, 2},
    { 8,  0, 0x8a6c, 2},
    { 0,  0, 0x8a6b, 2},
    { 0,  8, 0x8a4e, 2},
    { 0,  0, 0x8a5e, 2},
    { 0,  8, 0xca4e, 2},
    { 0,  0, 0xca5e, 2},
    {-8,  0, 0x4a6c, 2},
    { 0,  0, 0x4a6b, 2},
    {-8,  0, 0xca6c, 2},
    { 0,  0, 0xca6b, 2},
  };
  if (sprite_graphics[k] != 0xff) {
    Sprite_DrawMultiple(k, &kBabusu_Dmd[sprite_graphics[k] * 2], 2, NULL);
  } else {
    PrepOamCoordsRet info;
    Sprite_PrepOamCoord(k, &info);
  }
}

void Wizzrobe_Draw(int k) {  // 8dbe06
  static const DrawMultipleData kWizzrobe_Dmd[24] = {
    {0, -8, 0x00b2, 0},
    {8, -8, 0x00b3, 0},
    {0,  0, 0x0088, 2},
    {0, -8, 0x00b2, 0},
    {8, -8, 0x00b3, 0},
    {0,  0, 0x0086, 2},
    {0, -8, 0x00b2, 0},
    {8, -8, 0x00b3, 0},
    {0,  0, 0x008c, 2},
    {0, -8, 0x00b2, 0},
    {8, -8, 0x00b3, 0},
    {0,  0, 0x008a, 2},
    {0, -8, 0x00b2, 0},
    {8, -8, 0x00b3, 0},
    {0,  0, 0x408c, 2},
    {0, -8, 0x00b2, 0},
    {8, -8, 0x00b3, 0},
    {0,  0, 0x408a, 2},
    {0, -8, 0x00b2, 0},
    {8, -8, 0x00b3, 0},
    {0,  0, 0x00a4, 2},
    {0, -8, 0x00b2, 0},
    {8, -8, 0x00b3, 0},
    {0,  0, 0x008e, 2},
  };
  Sprite_DrawMultiple(k, &kWizzrobe_Dmd[sprite_graphics[k] * 3], 3, NULL);
}

void Wizzbeam_Draw(int k) {  // 8dbe68
  static const DrawMultipleData kWizzbeam_Dmd[8] = {
    { 0, -4, 0x00c5, 0},
    { 0,  4, 0x80c5, 0},
    { 0, -4, 0x40c5, 0},
    { 0,  4, 0xc0c5, 0},
    {-4,  0, 0x40d2, 0},
    { 4,  0, 0x00d2, 0},
    {-4,  0, 0xc0d2, 0},
    { 4,  0, 0x80d2, 0},
  };
  Sprite_DrawMultiple(k, &kWizzbeam_Dmd[sprite_D[k] * 2], 2, NULL);
}

void Freezor_Draw(int k) {  // 8dbfa6

  static const DrawMultipleData kFreezor_Dmd0[28] = {
    {-8,  0, 0x00a6, 2},
    { 8,  0, 0x40a6, 2},
    {-8,  0, 0x00a6, 2},
    { 8,  0, 0x40a6, 2},
    {-8,  0, 0x00a6, 2},
    { 8,  0, 0x40a6, 2},
    { 0, 11, 0x00ab, 0},
    { 8, 11, 0x40ab, 0},
    {-8,  0, 0x00ac, 2},
    { 8,  0, 0x40a8, 2},
    { 0, 11, 0x00ba, 0},
    { 8, 11, 0x00bb, 0},
    {-8,  0, 0x00a8, 2},
    { 8,  0, 0x40ac, 2},
    { 0, 11, 0x40bb, 0},
    { 8, 11, 0x40ba, 0},
    { 0,  2, 0x00ae, 0},
    { 8,  2, 0x40ae, 0},
    { 0, 10, 0x00be, 0},
    { 8, 10, 0x40be, 0},
    { 0,  4, 0x00af, 0},
    { 8,  4, 0x40af, 0},
    { 0, 12, 0x00bf, 0},
    { 8, 12, 0x40bf, 0},
    { 0,  8, 0x00aa, 0},
    { 8,  8, 0x40aa, 0},
    { 0,  8, 0x00aa, 0},
    { 8,  8, 0x40aa, 0},
  };
  static const DrawMultipleData kFreezor_Dmd1[8] = {
    { 0, 0, 0x00ae, 0},
    { 8, 0, 0x40ae, 0},
    { 0, 8, 0x00be, 0},
    { 8, 8, 0x40be, 0},
    {-2, 0, 0x00ae, 0},
    {10, 0, 0x40ae, 0},
    {-2, 8, 0x00be, 0},
    {10, 8, 0x40be, 0},
  };
  if (sprite_graphics[k] != 7) {
    Sprite_DrawMultiple(k, &kFreezor_Dmd0[sprite_graphics[k] * 4], 4, NULL);
  } else {
    Sprite_DrawMultiple(k, kFreezor_Dmd1, 8, NULL);
  }
}

void Zazak_Draw(int k) {  // 8dc0a6
  static const uint8 kZazak_Char[8] = {0x82, 0x82, 0x80, 0x84, 0x88, 0x88, 0x86, 0x84};
  static const uint8 kZazak_Flags[8] = {0x40, 0, 0, 0, 0x40, 0, 0, 0};
  static const DrawMultipleData kZazak_Dmd[24] = {
    { 0, -8, 0x0008, 2},
    {-4,  0, 0x00a0, 2},
    { 4,  0, 0x00a1, 2},
    { 0, -7, 0x0008, 2},
    {-4,  1, 0x40a1, 2},
    { 4,  1, 0x40a0, 2},
    { 0, -8, 0x000e, 2},
    {-4,  0, 0x00a3, 2},
    { 4,  0, 0x00a4, 2},
    { 0, -7, 0x000e, 2},
    {-4,  1, 0x40a4, 2},
    { 4,  1, 0x40a3, 2},
    { 0, -9, 0x000c, 2},
    { 0,  0, 0x00a6, 2},
    { 0,  0, 0x00a6, 2},
    { 0, -8, 0x000c, 2},
    { 0,  0, 0x00a8, 2},
    { 0,  0, 0x00a8, 2},
    { 0, -9, 0x400c, 2},
    { 0,  0, 0x40a6, 2},
    { 0,  0, 0x40a6, 2},
    { 0, -8, 0x400c, 2},
    { 0,  0, 0x40a8, 2},
    { 0,  0, 0x40a8, 2},
  };
  PrepOamCoordsRet info;
  Sprite_DrawMultiple(k, &kZazak_Dmd[sprite_graphics[k] * 3], 3, &info);
  if (sprite_pause[k])
    return;
  int i = sprite_head_dir[k] + (sprite_delay_aux1[k] == 0 ? 0 : 4);
  OamEnt *oam = GetOamCurPtr();
  oam->charnum = kZazak_Char[i];
  oam->flags = (oam->flags & ~0x40) | kZazak_Flags[i];
  SpriteDraw_Shadow(k, &info);
}

void Stalfos_Draw(int k) {  // 8dc21c
  static const uint8 kStalfos_Char[4] = {2, 2, 0, 4};
  static const uint8 kStalfos_Flags[4] = {0x70, 0x30, 0x30, 0x30};
  static const DrawMultipleData kStalfos_Dmd[36] = {
    { 0, -10, 0x0000, 2},
    { 0,   0, 0x0006, 2},
    { 0,   0, 0x0006, 2},
    { 0,  -9, 0x0000, 2},
    { 0,   1, 0x4006, 2},
    { 0,   1, 0x4006, 2},
    { 0, -10, 0x0004, 2},
    { 0,   0, 0x0006, 2},
    { 0,   0, 0x0006, 2},
    { 0,  -9, 0x0004, 2},
    { 0,   1, 0x4006, 2},
    { 0,   1, 0x4006, 2},
    { 0, -10, 0x0002, 2},
    { 5,   5, 0x002e, 0},
    { 0,   0, 0x0024, 2},
    { 0, -10, 0x0002, 2},
    { 0,   0, 0x000e, 2},
    { 0,   0, 0x000e, 2},
    { 0, -10, 0x4002, 2},
    { 3,   5, 0x402e, 0},
    { 0,   0, 0x4024, 2},
    { 0, -10, 0x4002, 2},
    { 0,   0, 0x400e, 2},
    { 0,   0, 0x000e, 2},
    { 2,  -8, 0x4002, 2},
    { 0,   0, 0x4008, 2},
    { 0,   0, 0x4008, 2},
    {-2,  -8, 0x0002, 2},
    { 0,   0, 0x0008, 2},
    { 0,   0, 0x0008, 2},
    { 0,  -6, 0x0000, 2},
    { 0,   0, 0x000a, 2},
    { 0,   0, 0x000a, 2},
    { 0,   0, 0x000a, 2},
    { 0,  -6, 0x0004, 2},
    { 0,  -6, 0x0004, 2},
  };
  PrepOamCoordsRet info;
  if (sprite_delay_aux2[k]) {
    Sprite_PrepOamCoord(k, &info);
    return;
  }
  Sprite_DrawMultiple(k, &kStalfos_Dmd[sprite_graphics[k] * 3], 3, &info);
  if (sprite_graphics[k] < 8 && !sprite_pause[k]) {
    OamEnt *oam = GetOamCurPtr();
    int i = sprite_head_dir[k];
    oam->charnum = kStalfos_Char[i];
    oam->flags = (oam->flags & ~0x70) | kStalfos_Flags[i];
  }
  SpriteDraw_Shadow(k, &info);
}

bool Probe_CheckTileSolidity(int k) {  // 8dc26e
  uint8 tiletype;
  if (player_is_indoors) {
    int t = (sprite_floor[k] >= 1) ? 0x1000 : 0;
    t += (cur_sprite_x & 0x1f8) >> 3;
    t += (cur_sprite_y & 0x1f8) << 3;
    tiletype = dung_bg2_attr_table[t];
  } else {
    tiletype = Overworld_ReadTileAttribute(cur_sprite_x >> 3, cur_sprite_y);
  }
  sprite_tiletype = tiletype;
  return kSprite_SimplifiedTileAttr[tiletype] >= 1;
}

void Sprite_HumanMulti_1(int k) {  // 8dc2d9
  switch (sprite_subtype2[k]) {
  case 0: Sprite_FluteDad(k); break;
  case 1: Sprite_ThiefHideoutGuy(k); break;
  case 2: Sprite_BlindsHutGuy(k); break;
  }

}

void Sprite_BlindsHutGuy(int k) {  // 8dc2e6
  BlindHideoutGuy_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_BehaveAsBarrier(k);
  Sprite_TrackBodyToHead(k);
  sprite_head_dir[k] = 0;
  int j = Sprite_ShowSolicitedMessage(k, 0x172);
  if (j & 0x100)
    sprite_D[k] = sprite_head_dir[k] = j;
}

void Sprite_ThiefHideoutGuy(int k) {  // 8dc308
  if (!(frame_counter & 3)) {
    sprite_graphics[k] = 2;
    uint8 dir = Sprite_DirectionToFaceLink(k, NULL);
    sprite_head_dir[k] = dir == 3 ? 2 : dir;
  }
  sprite_oam_flags[k] = 15;
  Oam_AllocateDeferToPlayer(k);
  Thief_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_BehaveAsBarrier(k);
  Sprite_ShowSolicitedMessage(k, 0x171);
  sprite_graphics[k] = 2;
}

void Sprite_FluteDad(int k) {  // 8dc343
  FluteBoyFather_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_BehaveAsBarrier(k);
  sprite_graphics[k] = (frame_counter < 48) ? 2 : (frame_counter >> 7) & 1;

  if (sprite_ai_state[k]) {
    Sprite_ShowSolicitedMessage(k, 0xa3);
    sprite_graphics[k] = 2;
  } else if (link_item_flute < 2) {
    Sprite_ShowSolicitedMessage(k, 0xa1);
  } else if (!(Sprite_ShowSolicitedMessage(k, 0xa4) & 0x100) &&
             hud_cur_item == kHudItem_Flute && (joypad1H_last&0x40) && Sprite_CheckDamageToLink_same_layer(k)) {
    Sprite_ShowMessageUnconditional(0xa2);
    sprite_ai_state[k]++;
    sprite_graphics[k] = 2;
  }
}

void FluteBoyFather_Draw(int k) {  // 8dc3e1
  static const DrawMultipleData kFluteBoyFather_Dmd[6] = {
    {0, -7, 0x0086, 2},
    {0,  0, 0x0088, 2},
    {0, -6, 0x0086, 2},
    {0,  0, 0x0088, 2},
    {0, -8, 0x0084, 2},
    {0,  0, 0x0088, 2},
  };
  PrepOamCoordsRet info;
  Sprite_DrawMultiplePlayerDeferred(k, &kFluteBoyFather_Dmd[sprite_graphics[k] * 2], 2, &info);
  SpriteDraw_Shadow(k, &info);
}

void BlindHideoutGuy_Draw(int k) {  // 8dc481
  static const DrawMultipleData kBlindHideoutGuy_Dmd[16] = {
    {0, -8, 0x000c, 2},
    {0,  0, 0x00ca, 2},
    {0, -8, 0x000c, 2},
    {0,  0, 0x40ca, 2},
    {0, -8, 0x000c, 2},
    {0,  0, 0x00ca, 2},
    {0, -8, 0x000c, 2},
    {0,  0, 0x40ca, 2},
    {0, -8, 0x000e, 2},
    {0,  0, 0x00ca, 2},
    {0, -8, 0x000e, 2},
    {0,  0, 0x40ca, 2},
    {0, -8, 0x400e, 2},
    {0,  0, 0x00ca, 2},
    {0, -8, 0x400e, 2},
    {0,  0, 0x40ca, 2},
  };
  PrepOamCoordsRet info;
  Sprite_DrawMultiplePlayerDeferred(k, &kBlindHideoutGuy_Dmd[sprite_graphics[k] * 2 + sprite_D[k] * 4], 2, &info);
  SpriteDraw_Shadow(k, &info);
}

void Sprite_SweepingLady(int k) {  // 8dc4ad
  SweepingLady_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_ShowSolicitedMessage(k, 0xa5);
  Sprite_BehaveAsBarrier(k);
  sprite_graphics[k] = frame_counter >> 4 & 1;
}

void SweepingLady_Draw(int k) {  // 8dc4eb
  static const DrawMultipleData kSweepingLadyDmd[4] = {
    {0, -7, 0x008e, 2},
    {0,  5, 0x008a, 2},
    {0, -8, 0x008e, 2},
    {0,  4, 0x008c, 2},
  };
  PrepOamCoordsRet info;
  Sprite_DrawMultiplePlayerDeferred(k, &kSweepingLadyDmd[sprite_graphics[k] * 2], 2, &info);
  SpriteDraw_Shadow(k, &info);
}

void Sprite_Lumberjacks(int k) {  // 8dc51b
  static const uint16 kLumberJackMsg[4] = {0x12c, 0x12d, 0x12e, 0x12d};
  Lumberjacks_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (Lumberjack_CheckProximity(k, 0)) {
    Sprite_NullifyHookshotDrag();
    link_speed_setting = 0;
    Link_CancelDash();
  }
  if (!Sprite_CheckIfLinkIsBusy() && Lumberjack_CheckProximity(k, 1) && (filtered_joypad_L & 0x80)) {
    int msg = (BYTE(link_x_coord) >= sprite_x_lo[k]) + (link_sword_type >= 2) * 2;
    Sprite_ShowMessageUnconditional(kLumberJackMsg[msg]);
  }
  sprite_graphics[k] = frame_counter >> 5 & 1;
}

bool Lumberjack_CheckProximity(int k, int j) {  // 8dc58f
  static const uint8 kLumberJacks_X[2] = {48, 52};
  static const uint8 kLumberJacks_Y[2] = {19, 20};
  static const uint8 kLumberJacks_W[2] = {98, 106};
  static const uint8 kLumberJacks_H[2] = {37, 40};
  return (uint16)(cur_sprite_x - link_x_coord + kLumberJacks_X[j]) < kLumberJacks_W[j] &&
         (uint16)(cur_sprite_y - link_y_coord + kLumberJacks_Y[j]) < kLumberJacks_H[j];
}

void Lumberjacks_Draw(int k) {  // 8dc6ba
  static const DrawMultipleData kLumberJacks_Dmd[33] = {
    {-23,  5, 0x02be, 0},
    {-15,  5, 0x02bf, 0},
    { -7,  5, 0x02bf, 0},
    {  1,  5, 0x02bf, 0},
    {  9,  5, 0x02bf, 0},
    { 17,  5, 0x02bf, 0},
    { 25,  5, 0x42be, 0},
    {-32, -8, 0x40a8, 2},
    {-32,  4, 0x40a6, 2},
    { 30, -8, 0x00a8, 2},
    { 31,  4, 0x00a4, 2},
    {-19,  5, 0x02be, 0},
    {-11,  5, 0x02bf, 0},
    { -3,  5, 0x02bf, 0},
    {  5,  5, 0x02bf, 0},
    { 13,  5, 0x02bf, 0},
    { 21,  5, 0x02bf, 0},
    { 29,  5, 0x42be, 0},
    {-31, -8, 0x40a8, 2},
    {-32,  4, 0x40a4, 2},
    { 31, -8, 0x00a8, 2},
    { 31,  4, 0x00a6, 2},
    {-19,  5, 0x02be, 0},
    {-11,  5, 0x02bf, 0},
    { -3,  5, 0x02bf, 0},
    {  5,  5, 0x02bf, 0},
    { 13,  5, 0x02bf, 0},
    { 21,  5, 0x02bf, 0},
    { 29,  5, 0x42be, 0},
    {-32, -8, 0x400e, 2},
    {-32,  4, 0x40a4, 2},
    { 32, -8, 0x000e, 2},
    { 31,  4, 0x00a6, 2},
  };
  Sprite_DrawMultiple(k, &kLumberJacks_Dmd[sprite_graphics[k] * 11], 11, NULL);
}

void Sprite_FortuneTeller(int k) {  // 8dc762
  switch (sprite_subtype2[k]) {
  case 0: // fortuneteller main
    FortuneTeller_Draw(k);
    if (Sprite_ReturnIfInactive(k))
      return;
    FortuneTeller_LightOrDarkWorld(k, savegame_is_darkworld >> 6 & 1);
    break;
  case 1: // dwarf solidity
    if (Sprite_ReturnIfInactive(k))
      return;
    if (Sprite_CheckDamageToLink_same_layer(k)) {
      Sprite_NullifyHookshotDrag();
      link_speed_setting = 0;
      Link_CancelDash();
    }
    break;
  }
}

void FortuneTeller_PerformPseudoScience(int k) {  // 8dc849
  sprite_graphics[k] = 0;
  sprite_ai_state[k]++;

  static const uint8 kFortuneTeller_Readings[16] = {0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0, 0xf1, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd};

  uint8 slots[2] = { 0, 0 };
  int n = 0;
#define ADD_MSG(q) { slots[n++]=(q); if (n==2) goto done; }

  if (savegame_map_icons_indicator < 3)
    goto done;
  if (!link_item_book_of_mudora) ADD_MSG(2);
  if (!(link_which_pendants & 2)) ADD_MSG(1);
  if (link_item_mushroom < 2) ADD_MSG(3);
  if (!link_item_flippers) ADD_MSG(4);
  if (!link_item_moon_pearl) ADD_MSG(5);
  if (sram_progress_indicator < 3) ADD_MSG(6);
  if (!link_magic_consumption) ADD_MSG(7);
  if (!link_item_bombos_medallion) ADD_MSG(8);
  if (!(sram_progress_indicator_3 & 0x10)) ADD_MSG(9);
  if (!(sram_progress_indicator_3 & 0x20)) ADD_MSG(10);
  if (!link_item_cape) ADD_MSG(11);
  if (!(save_ow_event_info[0x5b] & 2)) ADD_MSG(12);
  if (link_sword_type < 4) ADD_MSG(13);
  ADD_MSG(14);
  ADD_MSG(15);
  int j;
done:
  j = ((sram_progress_flags ^= 0x40) & 0x40) != 0;
  Sprite_ShowMessageUnconditional(kFortuneTeller_Readings[slots[j]]);
}

void FortuneTeller_Draw(int k) {  // 8dcb01
  static const DrawMultipleData kFortuneTeller_Dmd[12] = {
    { 0, -48, 0x000c, 2},
    { 0, -32, 0x002c, 0},
    { 8, -32, 0x402c, 0},
    { 0, -48, 0x000a, 2},
    { 0, -32, 0x002a, 0},
    { 8, -32, 0x402a, 0},
    {-4, -40, 0x0066, 2},
    { 4, -40, 0x4066, 2},
    {-4, -40, 0x0066, 2},
    {-4, -40, 0x0068, 2},
    { 4, -40, 0x4068, 2},
    {-4, -40, 0x0068, 2},
  };
  int j = (savegame_is_darkworld >> 6 & 1) * 2 + sprite_graphics[k];
  Sprite_DrawMultiple(k, &kFortuneTeller_Dmd[j * 3], 3, NULL);
}

void Smithy_SpawnDumbBarrierSprite(int k) {  // 8dcb2a
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0x31, &info);
  if (j < 0)
    return;
  Sprite_SetX(j, info.r0_x);
  Sprite_SetY(j, info.r2_y);
  sprite_subtype2[j] = 1;
  sprite_flags4[j] = 0;
  sprite_ignore_projectile[j] = 1;
}

void Sprite_MazeGameLady(int k) {  // 8dcb5c
  Lady_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_TrackBodyToHead(k);
  sprite_head_dir[k] = Sprite_DirectionToFaceLink(k, NULL) ^ 3;
  switch (sprite_ai_state[k]) {
  case 0:  // startup
    if (sprite_x_lo[k] < BYTE(link_x_coord)) {
      int j = Sprite_ShowMessageOnContact(k, 0xcc);
      if (j & 0x100) {
        sprite_D[k] = sprite_head_dir[k] = (uint8)j;
        sprite_ai_state[k] = 1;
        word_7FFE00 = 0;
        word_7FFE02 = 0;
        sprite_A[k] = 0;
        flag_overworld_area_did_change = 0;
      }
    } else {
      Sprite_ShowMessageOnContact(k, 0xd0);
    }
    break;
  case 1:  // sound
    SpriteSfx_QueueSfx3WithPan(k, 0x7);
    sprite_ai_state[k] = 2;
    break;
  case 2:  // accumulate time
    if (++sprite_A[k] == 63) {
      sprite_A[k] = 0;
      word_7FFE00 += 1;
      if (word_7FFE00 == 0)
        word_7FFE02++;
    }
    break;
  }
}

void Sprite_MazeGameGuy(int k) {  // 8dcbf2
  int j;
  MazeGameGuy_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_TrackBodyToHead(k);
  sprite_head_dir[k] = 0;
  Sprite_BehaveAsBarrier(k);
  sprite_graphics[k] = frame_counter >> 3 & 1;
  if (flag_overworld_area_did_change) {
    Sprite_ShowMessageOnContact(k, 0xd0);
    return;
  }
  switch (sprite_ai_state[k]) {
  case 0: {  // parse time
    word_7FFE04 = word_7FFE00;
    word_7FFE06 = word_7FFE02;
    int t = word_7FFE04 % 6000;
    int a = t / 600;
    t %= 600;
    int b = t / 60;
    t %= 60;
    int c = t / 10;
    t %= 10;
    dialogue_number[0] = t | c << 4;
    dialogue_number[1] = b | a << 4;
    t = Sprite_ShowMessageOnContact(k, 0xcb);
    if (t & 0x100) {
      sprite_D[k] = sprite_head_dir[k] = (uint8)t;
      sprite_ai_state[k] = 1;
    }
    break;
  }
  case 1:  // check qualify
    if (save_ow_event_info[BYTE(overworld_screen_index)] & 0x40) {
      Sprite_ShowMessageUnconditional(0xcf);
      sprite_ai_state[k] = 4;
    } else if (word_7FFE04 < 16) {
      Sprite_ShowMessageUnconditional(0xcd);
      sprite_D[k] = sprite_head_dir[k] = (uint8)link_player_handler_state;  // wtf
      sprite_ai_state[k] = 3;
    } else {
      Sprite_ShowMessageUnconditional(0xce);
      sprite_D[k] = sprite_head_dir[k] = (uint8)link_player_handler_state;  // wtf
      sprite_ai_state[k] = 2;
    }
    break;
  case 2:  // sorry
    j = Sprite_ShowMessageOnContact(k, 0xce);
    if (j & 0x100)
      sprite_D[k] = sprite_head_dir[k] = (uint8)j;
    break;
  case 3:  // can have it
    j = Sprite_ShowSolicitedMessage(k, 0xcd);
    if (j & 0x100)
      sprite_D[k] = sprite_head_dir[k] = (uint8)j;
    break;
  case 4:  // nothing more
    j = Sprite_ShowSolicitedMessage(k, 0xcf);
    if (j & 0x100)
      sprite_D[k] = sprite_head_dir[k] = (uint8)j;
    break;
  }
}

void MazeGameGuy_Draw(int k) {  // 8dcda7
  static const DrawMultipleData kMazeGameGuy_Dmd[16] = {
    {0, -10, 0x0000, 2},
    {0,   0, 0x0020, 2},
    {0, -10, 0x0000, 2},
    {0,   0, 0x0020, 2},
    {0, -10, 0x0000, 2},
    {0,   0, 0x0020, 2},
    {0, -10, 0x0000, 2},
    {0,   0, 0x0020, 2},
    {0, -10, 0x4002, 2},
    {0,   0, 0x0020, 2},
    {0, -10, 0x4002, 2},
    {0,   0, 0x0020, 2},
    {0, -10, 0x0002, 2},
    {0,   0, 0x0020, 2},
    {0, -10, 0x0002, 2},
    {0,   0, 0x0020, 2},
  };
  PrepOamCoordsRet info;
  Sprite_DrawMultiplePlayerDeferred(k, &kMazeGameGuy_Dmd[sprite_graphics[k] * 2 + sprite_D[k] * 4], 2, &info);
  SpriteDraw_Shadow(k, &info);
}

void CrystalMaiden_Draw(int k) {  // 8dce5f
  int j = sprite_D[k] * 2 + sprite_graphics[k];
  BYTE(dma_var6) = kCrystalMaiden_Dma[j * 2 + 0];
  BYTE(dma_var7) = kCrystalMaiden_Dma[j * 2 + 1];
  Sprite_DrawMultiplePlayerDeferred(k, kCrystalMaiden_SpriteData + j * 2, 2, NULL);
}

void Priest_Draw(int k) {  // 8dcf31
  int j = sprite_D[k] * 2 + sprite_graphics[k];
  PrepOamCoordsRet info;
  Sprite_DrawMultiplePlayerDeferred(k, kPriest_Dmd + j * 2, 2, &info);
  SpriteDraw_Shadow(k, &info);
}

uint8 FluteBoy_Draw(int k) {  // 8dcfd9
  static const DrawMultipleData kFluteBoy_Dmd[16] = {
    {-1,  -1, 0x0abe, 0},
    { 0,   0, 0x0aaa, 2},
    { 0, -10, 0x0aa8, 2},
    { 0,   0, 0x0aaa, 2},
    {-1,  -1, 0x0abe, 0},
    { 0,   8, 0x0abf, 0},
    { 0, -10, 0x0aa8, 2},
    { 0,   0, 0x0aaa, 2},
    {-1,  -1, 0x0abe, 0},
    { 0,   0, 0x0aaa, 2},
    { 0, -10, 0x0aa8, 2},
    { 0,   0, 0x0aaa, 2},
    {-1,  -1, 0x0abe, 0},
    { 0,   8, 0x0abf, 0},
    { 0, -10, 0x0aa8, 2},
    { 0,   0, 0x0aaa, 2},
  };
  Oam_AllocateFromRegionB(0x10);
  PrepOamCoordsRet info;
  Sprite_DrawMultiple(k, &kFluteBoy_Dmd[sprite_D[k] * 8 + sprite_graphics[k] * 4], 4, &info);
  return (info.x | info.y) >> 8;
}

void FluteAardvark_Draw(int k) {  // 8dd040
  static const DrawMultipleData kFluteAardvark_Dmd[8] = {
    {0, -16, 0x06e6, 2},
    {0,  -8, 0x06c8, 2},
    {0, -16, 0x06e6, 2},
    {0,  -8, 0x06ca, 2},
    {0, -16, 0x06e8, 2},
    {0,  -8, 0x06ca, 2},
    {0, -16, 0x00cc, 2},
    {0,  -8, 0x00dc, 2},
  };
  Sprite_DrawMultiplePlayerDeferred(k, &kFluteAardvark_Dmd[sprite_graphics[k] * 2], 2, NULL);
}

void DustCloud_Draw(int k) {  // 8dd120
  static const DrawMultipleData kDustCloud_Dmd[24] = {
    { 0, -3, 0x008b, 0},
    { 3,  0, 0x009b, 0},
    {-3,  0, 0xc08b, 0},
    { 0,  3, 0xc09b, 0},
    { 0, -5, 0x008a, 2},
    { 5,  0, 0x008a, 2},
    {-5,  0, 0x008a, 2},
    { 0,  5, 0x008a, 2},
    { 0, -7, 0x0086, 2},
    { 7,  0, 0x0086, 2},
    {-7,  0, 0x0086, 2},
    { 0,  7, 0x0086, 2},
    { 0, -9, 0x8086, 2},
    { 9,  0, 0x8086, 2},
    {-9,  0, 0x8086, 2},
    { 0,  9, 0x8086, 2},
    { 0, -9, 0xc086, 2},
    { 9,  0, 0xc086, 2},
    {-9,  0, 0xc086, 2},
    { 0,  9, 0xc086, 2},
    { 0, -7, 0x4086, 2},
    { 7,  0, 0x4086, 2},
    {-7,  0, 0x4086, 2},
    { 0,  7, 0x4086, 2},
  };
  sprite_oam_flags[k] = 0x14;
  Sprite_DrawMultiple(k, &kDustCloud_Dmd[sprite_graphics[k] * 4], 4, NULL);
}

void MedallionTablet_Draw(int k) {  // 8dd1e2
  static const DrawMultipleData kMedallionTablet_Dmd[20] = {
    {-8, -16, 0x008c, 2},
    { 8, -16, 0x408c, 2},
    {-8,   0, 0x00ac, 2},
    { 8,   0, 0x40ac, 2},
    {-8, -13, 0x008a, 2},
    { 8, -13, 0x408a, 2},
    {-8,   0, 0x00ac, 2},
    { 8,   0, 0x40ac, 2},
    {-8,  -8, 0x008a, 2},
    { 8,  -8, 0x408a, 2},
    {-8,   0, 0x00ac, 2},
    { 8,   0, 0x40ac, 2},
    {-8,  -4, 0x008a, 2},
    { 8,  -4, 0x408a, 2},
    {-8,   0, 0x00aa, 2},
    { 8,   0, 0x40aa, 2},
    {-8,   0, 0x00aa, 2},
    { 8,   0, 0x40aa, 2},
    {-8,   0, 0x00aa, 2},
    { 8,   0, 0x40aa, 2},
  };
  Sprite_DrawMultiplePlayerDeferred(k, &kMedallionTablet_Dmd[sprite_graphics[k] * 4], 4, NULL);
}

void Uncle_Draw(int k) {  // 8dd391
  static const DrawMultipleData kUncleDraw_Table[48] = {
    {  0, -10, 0x0e00, 2},
    {  0,   0, 0x0c06, 2},
    {  0, -10, 0x0e00, 2},
    {  0,   0, 0x0c06, 2},
    {  0, -10, 0x0e00, 2},
    {  0,   0, 0x0c06, 2},
    {  0, -10, 0x0e02, 2},
    {  0,   0, 0x0c06, 2},
    {  0, -10, 0x0e02, 2},
    {  0,   0, 0x0c06, 2},
    {  0, -10, 0x0e02, 2},
    {  0,   0, 0x0c06, 2},
    { -7,   2, 0x0d07, 2},
    { -7,   2, 0x0d07, 2},
    { 10,  12, 0x8d05, 0},
    { 10,   4, 0x8d15, 0},
    {  0, -10, 0x0e00, 2},
    {  0,   0, 0x0c04, 2},
    { -7,   1, 0x0d07, 2},
    { -7,   1, 0x0d07, 2},
    { 10,  13, 0x8d05, 0},
    { 10,   5, 0x8d15, 0},
    {  0,  -9, 0x0e00, 2},
    {  0,   1, 0x4c04, 2},
    { -7,   8, 0x8d05, 0},
    {  1,   8, 0x8d06, 0},
    {  0, -10, 0x0e02, 2},
    { -6,  -1, 0x4d07, 2},
    {  0,   0, 0x0c23, 2},
    {  0,   0, 0x0c23, 2},
    { -9,   7, 0x8d05, 0},
    { -1,   7, 0x8d06, 0},
    {  0,  -9, 0x0e02, 2},
    { -6,   0, 0x4d07, 2},
    {  0,   1, 0x0c25, 2},
    {  0,   1, 0x0c25, 2},
    {-10, -17, 0x0d07, 2},
    { 15, -12, 0x8d15, 0},
    { 15,  -4, 0x8d05, 0},
    {  0, -28, 0x0e08, 2},
    { -8, -19, 0x0c20, 2},
    {  8, -19, 0x4c20, 2},
    {  0, -28, 0x0e08, 2},
    {  0, -28, 0x0e08, 2},
    { -8, -19, 0x0c20, 2},
    {  8, -19, 0x4c20, 2},
    { -8, -19, 0x0c20, 2},
    {  8, -19, 0x4c20, 2},
  };
  static const uint8 kUncleDraw_Dma3[8] = { 8, 8, 0, 0, 6, 6, 0, 0 };
  static const uint8 kUncleDraw_Dma4[8] = { 0, 0, 0, 0, 4, 4, 0, 0x8b }; // wtf

  Oam_AllocateFromRegionB(0x18);
  const DrawMultipleData *src = &kUncleDraw_Table[sprite_D[k] * 12 + sprite_graphics[k] * 6];

  int j = sprite_D[k] * 2 + sprite_graphics[k];
  link_dma_var3 = kUncleDraw_Dma3[j];
  link_dma_var4 = kUncleDraw_Dma4[j];
  PrepOamCoordsRet info;
  Sprite_DrawMultiple(k, src, 6, &info);

  if (sprite_D[k] != 0 && sprite_D[k] != 3)
    SpriteDraw_Shadow(k, &info);
}

void BugNetKid_Draw(int k) {  // 8dd47b
  static const DrawMultipleData kBugNetKid_Dmd[18] = {
    { 4,  0, 0x0027, 0},
    { 0, -5, 0x000e, 2},
    {-8,  6, 0x040a, 2},
    { 8,  6, 0x440a, 2},
    {-8, 14, 0x840a, 2},
    { 8, 14, 0xc40a, 2},
    { 0, -5, 0x000e, 2},
    { 0, -5, 0x000e, 2},
    {-8,  6, 0x040a, 2},
    { 8,  6, 0x440a, 2},
    {-8, 14, 0x840a, 2},
    { 8, 14, 0xc40a, 2},
    { 0, -5, 0x002e, 2},
    { 0, -5, 0x002e, 2},
    {-8,  7, 0x040a, 2},
    { 8,  7, 0x440a, 2},
    {-8, 14, 0x840a, 2},
    { 8, 14, 0xc40a, 2},
  };
  Sprite_DrawMultiplePlayerDeferred(k, &kBugNetKid_Dmd[sprite_graphics[k] * 6], 6, NULL);
}

void Bomber_Draw(int k) {  // 8dd56c
  static const DrawMultipleData kBomber_Dmd[22] = {
    { 0, 0, 0x40c6, 2},
    { 0, 0, 0x40c6, 2},
    { 0, 0, 0x40c4, 2},
    { 0, 0, 0x40c4, 2},
    { 0, 0, 0x00c6, 2},
    { 0, 0, 0x00c6, 2},
    { 0, 0, 0x00c4, 2},
    { 0, 0, 0x00c4, 2},
    {-8, 0, 0x00c0, 2},
    { 8, 0, 0x40c0, 2},
    {-8, 0, 0x00c2, 2},
    { 8, 0, 0x40c2, 2},
    {-8, 0, 0x00e0, 2},
    { 8, 0, 0x40e0, 2},
    {-8, 0, 0x00e2, 2},
    { 8, 0, 0x40e2, 2},
    {-8, 0, 0x00e4, 2},
    { 8, 0, 0x40e4, 2},
    { 0, 0, 0x40e6, 2},
    { 0, 0, 0x40e6, 2},
    { 0, 0, 0x00e6, 2},
    { 0, 0, 0x00e6, 2},
  };
  PrepOamCoordsRet info;
  Sprite_DrawMultiple(k, &kBomber_Dmd[sprite_graphics[k] * 2], 2, &info);
  SpriteDraw_Shadow(k, &info);
}

void SpriteDraw_ZirroBomb(int k) {  // 8dd606
  static const DrawMultipleData kBomberPellet_Dmd[15] = {
    {-11,   0, 0x019b, 0},
    {  0,  -8, 0xc19b, 0},
    {  6,   6, 0x419b, 0},
    {-15,  -6, 0x018a, 2},
    { -4, -14, 0x018a, 2},
    {  2,   0, 0x018a, 2},
    {-15,  -6, 0x0186, 2},
    { -4, -14, 0x0186, 2},
    {  2,   0, 0x0186, 2},
    { -4,  -4, 0x0186, 2},
    { -4,  -4, 0x0186, 2},
    { -4,  -4, 0x0186, 2},
    { -4,  -4, 0x01aa, 2},
    { -4,  -4, 0x01aa, 2},
    { -4,  -4, 0x01aa, 2},
  };
  if (!sprite_delay_main[k])
    sprite_state[k] = 0;
  Sprite_DrawMultiple(k, &kBomberPellet_Dmd[(sprite_delay_main[k] >> 2) * 3], 3, NULL);
}

void PlayerBee_HoneInOnTarget(int j, int k) {  // 8dd631
  if (sprite_type[j] != 0x88 && (sprite_flags[j] & 2))
    return;
  uint16 x = Sprite_GetX(j);
  uint16 y = Sprite_GetY(j);
  if ((uint16)(cur_sprite_x - x + 16) >= 24 ||
      (uint16)(cur_sprite_y - y - 8) >= 24)
    return;
  if (sprite_type[j] == 0x75) {
    sprite_E[j] = k + 1;
    return;
  }
  Ancilla_CheckDamageToSprite_preset(j, 1);
  sprite_F[j] = 15;
  sprite_x_recoil[j] = sprite_x_vel[k] << 1;
  sprite_y_recoil[j] = sprite_y_vel[k] << 1;
  sprite_B[k]++;
}

void Pikit_Draw(int k) {  // 8dd6e6
  static const DrawMultipleData kPikit_Dmd[8] = {
    { 0, 0, 0x00c8, 2},
    { 0, 0, 0x00c8, 2},
    { 0, 0, 0x00ca, 2},
    { 0, 0, 0x00ca, 2},
    {-8, 0, 0x00cc, 2},
    { 8, 0, 0x40cc, 2},
    {-8, 0, 0x00ce, 2},
    { 8, 0, 0x40ce, 2},
  };
  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  SpriteDraw_Pikit_Tongue(k, &info);
  OamEnt *oam = GetOamCurPtr();
  tmp_counter = oam->x;
  byte_7E0FB6 = oam->y;
  oam_cur_ptr += 24;
  oam_ext_cur_ptr += 6;
  Sprite_DrawMultiple(k, &kPikit_Dmd[sprite_graphics[k] * 2], 2, &info);
  uint8 bak = sprite_flags2[k];
  sprite_flags2[k] -= 6;
  SpriteDraw_Shadow(k, &info);
  sprite_flags2[k] = bak;
  SpriteDraw_Pikit_Loot(k, &info);
}

void SpriteDraw_Pikit_Tongue(int k, PrepOamCoordsRet *info) {  // 8dd74a
  static const uint8 kPikit_TongueMult[4] = {0x33, 0x66, 0x99, 0xcc};
  static const uint8 kPikit_Draw_Char[8] = {0xee, 0xfd, 0xed, 0xfd, 0xee, 0xfd, 0xed, 0xfd};
  static const uint8 kPikit_Draw_Flags[8] = {0, 0, 0, 0x40, 0x40, 0xc0, 0x80, 0x80};
  if (sprite_ai_state[k] != 2 || sprite_pause[k])
    return;
  OamEnt *oam = GetOamCurPtr();
  int x = info->x + 4, y = info->y + 3;
  oam[5].x = x;
  oam[5].y = y;
  oam[0].x = x + sprite_A[k];
  oam[0].y = y + sprite_B[k];
  oam[0].charnum = oam[5].charnum = 0xfe;
  oam[0].flags = oam[5].flags = info->flags;
  oam++;
  int g = sprite_D[k];
  for (int i = 3; i >= 0; i--, oam++) {
    oam->x = x + (int8)sprite_A[k] * kPikit_TongueMult[i] / 256;
    oam->y = y + (int8)sprite_B[k] * kPikit_TongueMult[i] / 256;
    oam->charnum = kPikit_Draw_Char[g];
    oam->flags = kPikit_Draw_Flags[g] | info->flags;
  }
  Sprite_CorrectOamEntries(k, 5, 0);
}

void SpriteDraw_Pikit_Loot(int k, PrepOamCoordsRet *info) {  // 8dd858
  static const int8 kPikit_DrawGrabbedItem_X[20] = {
    -4, 4, -4, 4, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, -4, 4, -4, 4,
  };
  static const int8 kPikit_DrawGrabbedItem_Y[20] = {
    -4, -4, 4, 4, -4, -4, 4, 4, -4, -4, 4, 4, -4, -4, 4, 4, -4, -4, 4, 4,
  };
  static const uint8 kPikit_DrawGrabbedItem_Char[20] = {
    0x6e, 0x6f, 0x7e, 0x7f, 0x63, 0x7c, 0x73, 0x7c, 0xb, 0x7c, 0x1b, 0x7c, 0xec, 0xf9, 0xfc, 0xf9,
    0xea, 0xeb, 0xfa, 0xfb,
  };
  static const uint8 kPikit_DrawGrabbedItem_Flags[5] = {0x24, 0x24, 0x28, 0x29, 0x2f};
  if (!sprite_G[k])
    return;
  int g = sprite_G[k] - 1;
  if (g == 3)
    g = sprite_subtype[k] + 2;
  Oam_AllocateFromRegionC(0x10);
  OamEnt *oam = GetOamCurPtr();
  for (int i = 3; i >= 0; i--, oam++) {
    int j = g * 4 + i;
    oam->x = tmp_counter + kPikit_DrawGrabbedItem_X[j];
    oam->y = byte_7E0FB6 + kPikit_DrawGrabbedItem_Y[j];
    oam->charnum = kPikit_DrawGrabbedItem_Char[j];
    oam->flags = kPikit_DrawGrabbedItem_Flags[g];
  }
  Sprite_CorrectOamEntries(k, 3, 0);
}

void Kholdstare_Draw(int k) {  // 8dd98f
  static const DrawMultipleData kKholdstare_Dmd[16] = {
    {-8, -8, 0x0080, 2},
    { 8, -8, 0x0082, 2},
    {-8,  8, 0x00a0, 2},
    { 8,  8, 0x00a2, 2},
    {-7, -7, 0x0080, 2},
    { 7, -7, 0x0082, 2},
    {-7,  7, 0x00a0, 2},
    { 7,  7, 0x00a2, 2},
    {-7, -7, 0x0084, 2},
    { 7, -7, 0x0086, 2},
    {-7,  7, 0x00a4, 2},
    { 7,  7, 0x00a6, 2},
    {-8, -8, 0x0084, 2},
    { 8, -8, 0x0086, 2},
    {-8,  8, 0x00a4, 2},
    { 8,  8, 0x00a6, 2},
  };
  static const int8 kKholdstare_Draw_X[16] = {8, 7, 4, 2, 0, -2, -4, -7, -8, -7, -4, -2, 0, 2, 4, 7};
  static const int8 kKholdstare_Draw_Y[16] = {0, 2, 4, 7, 8, 7, 4, 2, 0, -2, -4, -7, -8, -7, -4, -2};
  static const uint8 kKholdstare_Draw_Char[16] = {0xac, 0xac, 0xaa, 0x8c, 0x8c, 0x8c, 0xaa, 0xac, 0xac, 0xaa, 0xaa, 0x8c, 0x8c, 0x8c, 0xaa, 0xac};
  static const uint8 kKholdstare_Draw_Flags[16] = {0x40, 0x40, 0x40, 0, 0, 0, 0, 0, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xc0, 0xc0};
  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  OamEnt *oam = GetOamCurPtr();
  int j = sprite_A[k];
  SetOamHelper0(oam, info.x + kKholdstare_Draw_X[j], info.y + kKholdstare_Draw_Y[j],
                kKholdstare_Draw_Char[j], kKholdstare_Draw_Flags[j] | info.flags, 2);
  oam_cur_ptr += 4, oam_ext_cur_ptr += 1;
  Sprite_DrawMultiple(k, &kKholdstare_Dmd[sprite_graphics[k] * 4], 4, &info);
}

int Sprite_SpawnFireball(int k) {  // 8dda06
  SpriteSpawnInfo info;
  SpriteSfx_QueueSfx3WithPan(k, 0x19);
  int j = Sprite_SpawnDynamicallyEx(k, 0x55, &info, 13);
  if (j < 0)
    return j;
  Sprite_SetX(j, info.r0_x + 4);
  Sprite_SetY(j, info.r2_y + 4 - info.r4_z);

  sprite_flags3[j] = sprite_flags3[j] & 0xfe | 0x40;
  sprite_oam_flags[j] = 6;
  sprite_flags4[j] = 0x54;
  sprite_E[j] = 0x54;
  sprite_flags2[j] = 0x20;
  Sprite_ApplySpeedTowardsLink(j, 0x20);
  sprite_delay_main[j] = 20;
  sprite_delay_aux1[j] = 16;
  sprite_flags5[j] = 0;
  sprite_defl_bits[j] = 0x48;
  return j;
}

void ArcheryGameGuy_Draw(int k) {  // 8ddac4
  static const int8 kArcheryGameGuy_Draw_X[15] = {0, 0, 0, 0, 0, -5, 0, -1, -1, 0, 0, 0, 0, 1, 1};
  static const int8 kArcheryGameGuy_Draw_Y[15] = {0, -10, -10, 0, -10, -3, 0, -10, -10, 0, -10, -10, 0, -10, -10};
  static const uint8 kArcheryGameGuy_Draw_Char[15] = {0x26, 6, 6, 8, 6, 0x3a, 0x26, 6, 6, 0x26, 6, 6, 0x26, 6, 6};
  static const uint8 kArcheryGameGuy_Draw_Flags[15] = {8, 6, 6, 8, 6, 8, 8, 6, 6, 8, 6, 6, 8, 6, 6};
  static const uint8 kArcheryGameGuy_Draw_Big[15] = {2, 2, 2, 2, 2, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2};

  Oam_AllocateDeferToPlayer(k);
  PrepOamCoordsRet info;
  Sprite_PrepOamCoord(k, &info);
  OamEnt *oam = GetOamCurPtr();
  int g = sprite_graphics[k];
  for (int i = 2; i >= 0; i--, oam++) {
    int j = g * 3 + i;
    SetOamPlain(oam, info.x + kArcheryGameGuy_Draw_X[j], info.y + kArcheryGameGuy_Draw_Y[j],
                 kArcheryGameGuy_Draw_Char[j], kArcheryGameGuy_Draw_Flags[j] | info.flags, kArcheryGameGuy_Draw_Big[j]);
  }
  SpriteDraw_Shadow(k, &info);
}

void ShopKeeper_RapidTerminateReceiveItem() {  // 8ffaea
  for (int i = 4; i >= 0; i--) {
    if (ancilla_type[i] == 0x22)
      ancilla_aux_timer[i] = 1;
  }
}

void Sprite_InitializeSecondaryItemMinigame(int what) {  // 8ffd86
  is_archer_or_shovel_game = what;
  Link_ResetProperties_C();
  for (int k = 4; k >= 0; k--) {
    if (ancilla_type[k] == 0x30 || ancilla_type[k] == 0x31) {
      ancilla_type[k] = 0;
    } else if (ancilla_type[k] == 5) {
      flag_for_boomerang_in_place = 0;
      ancilla_type[k] = 0;
    }
  }
}

void Waterfall(int k) {  // 9af5b8
  if (Sprite_ReturnIfInactive(k))
    return;
  if (Sprite_CheckDamageToLink_same_layer(k)) {
    if (BYTE(overworld_screen_index) == 0x43)
      AncillaAdd_GTCutscene();
    else
      AncillaAdd_WaterfallSplash();
  }
}

void Sprite_BatCrash(int k) {  // 9af5d9
  static const uint16 kRetreatBat_Xpos[4] = {0x7dc, 0x7f0, 0x820, 0x818};
  static const uint16 kRetreatBat_Ypos[4] = {0x62e, 0x636, 0x630, 0x5e0};
  static const uint8 kRetreatBat_Delay[5] = {4, 3, 4, 6, 0};
  int j;
  RetreatBat_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_MoveXY(k);
  BatCrash_DrawHardcodedGarbage(k);
  bg1_y_offset = 0;
  if (sprite_delay_aux3[k]) {
    if (sprite_delay_aux3[k] == 1)
      sound_effect_ambient = 5;
    bg1_y_offset = sprite_delay_aux3[k] & 1 ? 1 : -1;
  }
  if (!sprite_delay_main[k]) {
    sprite_graphics[k] = sprite_graphics[k] + 1 & 3;
    if (sprite_graphics[k] == 0 && sprite_ai_state[k] < 2)
      SpriteSfx_QueueSfx2WithPan(k, 0x3);
    sprite_delay_main[k] = kRetreatBat_Delay[sprite_D[k]];
  }
  switch(sprite_ai_state[k]) {
  case 0: {
    j = sprite_A[k];
    if (kRetreatBat_Xpos[j] < cur_sprite_x) {
      if (j >= 2) {
        sprite_ai_state[k]++;
        sprite_delay_aux1[k] = 208;
      }
      sprite_A[k]++;
      sprite_D[k]++;
    }
update_pos:
    if (!(frame_counter & 7))
      sprite_y_vel[k] += (kRetreatBat_Ypos[j] >= cur_sprite_y) ? 1 : -1;
    if (!(frame_counter & 15))
      sprite_x_vel[k]++;
    break;
  }
  case 1:
    if (!sprite_delay_aux1[k]) {
      sprite_ai_state[k]++;
      SpriteSfx_QueueSfx3WithPan(k, 0x26);
      sprite_D[k]++;
      sprite_x_lo[k] = 232;
      sprite_x_hi[k] = 7;
      sprite_y_lo[k] = 224;
      sprite_y_hi[k] = 5;
      sprite_x_vel[k] = 0;
      sprite_y_vel[k] = 64;
      sprite_delay_aux1[k] = 45;
    } else {
      if (!(frame_counter & 3))
        sprite_x_vel[k]--;
      j = sprite_A[k];
      goto update_pos;
    }
    break;
  case 2:
    if (!sprite_delay_aux1[k]) {
      sprite_y_vel[k] = 0;
      sprite_delay_aux1[k] = 96;
      sprite_ai_state[k]++;
    }
    if (sprite_delay_aux1[k] == 9) {
      BatCrash_SpawnDebris(k);
      CreatePyramidHole();
    }
    break;
  case 3:  // Finishing Up
    if (!sprite_delay_aux1[k]) {
      sprite_state[k] = 0;
      overworld_map_state++;
    }
    break;
  }
}

void Sprite_SpawnBatCrashCutscene() {  // 9af6f5
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(0, 0x37, &info);
  if (j >= 0) {
    sprite_y_vel[j] = 0;
    sprite_B[j] = 0;
    sprite_D[j] = 0;
    sprite_floor[j] = 0;
    sprite_subtype2[j] = 1;
    sprite_flags2[j] = 1;
    sprite_flags3[j] = 1;
    sprite_oam_flags[j] = 1;
    sprite_x_lo[j] = 204;
    sprite_x_hi[j] = 7;
    sprite_y_lo[j] = 50;
    sprite_y_hi[j] = 6;
    sprite_defl_bits[j] = 128;
  }
}

void BatCrash_DrawHardcodedGarbage(int k) {  // 9af750
  static const OamEntSigned kRetreatBat_Oams[8] = {
    { 104, -105, 0x57, 0x01},
    { 120, -105, 0x57, 0x01},
    {-120, -105, 0x57, 0x01},
    { 104,  -89, 0x57, 0x01},
    { 120,  -89, 0x57, 0x01},
    {-120,  -89, 0x57, 0x01},
    { 101, -112, 0x57, 0x01},
    {-117, -112, 0x57, 0x01},
  };
  memcpy(oam_buf + 76, kRetreatBat_Oams, 32);
  for (int i = 0; i < 9; i++) // wtf 9
    bytewise_extended_oam[i + 76] = 2;
}

void BatCrash_SpawnDebris(int k) {  // 9af7e5
  static const int8 kPyramidDebris_X[30] = {
    -8,  0,  8, 16, 24, 32, -8,  0,  8, 16, 24, 32, -8,  0, 8, 16,
    24, 32, -8,  0,  8, 16, 24, 32, -8,  0,  8, 16, 24, 32,
  };
  static const int8 kPyramidDebris_Y[30] = {
    0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
  };
  static const int8 kPyramidDebris_Xvel[30] = {
    -30, -25,  -8,   8,  25, 30, -50, -45, -20,  20,  45, 50, -50, -35, -25, 25,
     35,  50, -45, -50, -60, 60,  50,  45, -30, -35, -40, 40,  35,  30,
  };
  static const int8 kPyramidDebris_Yvel[30] = {
     2,  5,  10,  10,   5,   2,   5,  20,  30,  30,  20,   5,  10,  30, 40, 40,
    30, 10, -20, -40, -60, -60, -40, -20, -10, -20, -40, -40, -20, -10,
  };
  for (int j = 29; j >= 0; j--) {
    GarnishSpawn_PyramidDebris(kPyramidDebris_X[j], kPyramidDebris_Y[j], kPyramidDebris_Xvel[j], kPyramidDebris_Yvel[j]);
  }
  sprite_delay_aux3[k] = 32;
}

void RetreatBat_Draw(int k) {  // 9af833
  static const DrawMultipleData kRetreatBat_Dmds[18] = {
    { 0,  0, 0x044b, 0},
    { 5, -4, 0x045b, 0},
    {-2, -4, 0x0464, 2},
    {-2, -4, 0x0449, 2},
    {-8, -9, 0x046c, 2},
    { 8, -9, 0x446c, 2},
    {-8, -7, 0x044c, 2},
    { 8, -7, 0x444c, 2},
    {-8, -9, 0x0444, 2},
    { 8, -9, 0x4444, 2},
    {-8, -8, 0x0462, 2},
    { 8, -8, 0x4462, 2},
    {-8, -7, 0x0460, 2},
    { 8, -7, 0x4460, 2},
    { 0,  0, 0x044e, 2},
    {16,  0, 0x444e, 2},
    { 0, 16, 0x046e, 2},
    {16, 16, 0x446e, 2},
  };
  static const uint8 kOffs[] = {0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 6, 6, 8, 10, 12, 10, 14, 14, 14, 14};
  static const uint8 kCount[20] = { 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 4, 4, 4, 4, };
  oam_cur_ptr = 0x960;
  oam_ext_cur_ptr = 0xa78;
  int j = sprite_D[k] * 4 + sprite_graphics[k];
  Sprite_DrawMultiple(k, &kRetreatBat_Dmds[kOffs[j]], kCount[j], NULL);
}

void DrinkingGuy_Draw(int k) {  // 9af88c
  static const DrawMultipleData kDrinkingGuy_Dmd[6] = {
    {8,  2, 0x00ae, 0},
    {0, -9, 0x0822, 2},
    {0,  0, 0x0006, 2},
    {7,  0, 0x00af, 0},
    {0, -9, 0x0822, 2},
    {0,  0, 0x0006, 2},
  };
  PrepOamCoordsRet info;
  Sprite_DrawMultiplePlayerDeferred(k, &kDrinkingGuy_Dmd[sprite_graphics[k] * 3], 3, &info);
  SpriteDraw_Shadow(k, &info);
}

void Lady_Draw(int k) {  // 9af92c
  static const DrawMultipleData kLadyDmd[16] = {
    {0, -8, 0x00e0, 2},
    {0,  0, 0x00e8, 2},
    {0, -7, 0x00e0, 2},
    {0,  1, 0x40e8, 2},
    {0, -8, 0x00c0, 2},
    {0,  0, 0x00c2, 2},
    {0, -7, 0x00c0, 2},
    {0,  1, 0x40c2, 2},
    {0, -8, 0x00e2, 2},
    {0,  0, 0x00e4, 2},
    {0, -7, 0x00e2, 2},
    {0,  1, 0x00e6, 2},
    {0, -8, 0x40e2, 2},
    {0,  0, 0x40e4, 2},
    {0, -7, 0x40e2, 2},
    {0,  1, 0x40e6, 2},
  };
  PrepOamCoordsRet info;
  Sprite_DrawMultiplePlayerDeferred(k, &kLadyDmd[sprite_graphics[k] * 2 + sprite_D[k] * 4], 2, &info);
  SpriteDraw_Shadow(k, &info);
}

void Lanmola_SpawnShrapnel(int k) {  // 9af981
  static const int8 kLanmolaShrapnel_Yvel[8] = {28, -28, 28, -28, 0, 36, 0, -36};
  static const int8 kLanmolaShrapnel_Xvel[8] = {-28, -28, 28, 28, -36, 0, 36, 0};

  tmp_counter = (sprite_state[0] + sprite_state[1] + sprite_state[2]) < 10 ? 7 : 3;
  do {
    SpriteSpawnInfo info;
    int j = Sprite_SpawnDynamically(k, 0xC2, &info);
    if (j >= 0) {
      Sprite_SetSpawnedCoordinates(j, &info);
      sprite_x_lo[j] = info.r0_x + 4;
      sprite_y_lo[j] = info.r2_y + 4;
      sprite_ignore_projectile[j] = 1;
      sprite_bump_damage[j] = 1;
      sprite_flags4[j] = 1;
      sprite_z[j] = 0;
      sprite_flags2[j] = 0x20;
      sprite_x_vel[j] = kLanmolaShrapnel_Xvel[tmp_counter];
      sprite_y_vel[j] = kLanmolaShrapnel_Yvel[tmp_counter];
      sprite_graphics[j] = GetRandomNumber() & 1;
    }
  } while (!sign8(--tmp_counter));
}

void Sprite_Cukeman(int k) {  // 9afa0c
  if (sprite_head_dir[k] == 0)
    return;

  if (sprite_state[k] == 9 && !(submodule_index | flag_unk1) &&
      (uint16)(cur_sprite_x - link_x_coord + 0x18) < 0x30 &&
      (uint16)(link_y_coord - cur_sprite_y + 0x20) < 0x30 &&
      (filtered_joypad_L & 0x80)) {
    dialogue_message_index = 0x17a + (sprite_subtype[k]++ & 1);
    Sprite_ShowMessageMinimal();
  }

  uint8 old = sprite_oam_flags[k] & 0xf0;
  sprite_oam_flags[k] = old | 8;
  Cukeman_Draw(k);
  sprite_oam_flags[k] = old | 0xd;
  Oam_AllocateFromRegionA(0x10);
}

void Cukeman_Draw(int k) {  // 9afb0e
  static const DrawMultipleData kCukeman_Dmd[18] = {
    { 0, 0, 0x01f3, 0},
    { 7, 0, 0x41f3, 0},
    { 4, 7, 0x07e0, 0},
    {-1, 2, 0x01f3, 0},
    { 6, 1, 0x41f3, 0},
    { 4, 8, 0x07e0, 0},
    { 1, 1, 0x01f3, 0},
    { 8, 2, 0x41f3, 0},
    { 4, 8, 0x07e0, 0},
    {-2, 0, 0x01f3, 0},
    {10, 0, 0x41f3, 0},
    { 4, 7, 0x07e0, 0},
    { 0, 0, 0x01f3, 0},
    { 8, 0, 0x41f3, 0},
    { 4, 6, 0x07e0, 0},
    {-5, 0, 0x01f3, 0},
    {16, 0, 0x41f3, 0},
    { 4, 8, 0x07e0, 0},
  };
  Sprite_DrawMultiple(k, &kCukeman_Dmd[sprite_graphics[k] * 3], 3, NULL);
}

void RunningBoy_SpawnDustGarnish(int k) {  // 9afb2c
  if (++sprite_die_action[k] & 0xf)
    return;
  int j = GarnishAllocForce();
  garnish_type[j] = 20;
  garnish_active = 20;
  Garnish_SetX(j, Sprite_GetX(k) + 4);
  Garnish_SetY(j, Sprite_GetY(k) + 28);
  garnish_countdown[j] = 10;
}

void MovableMantle_Draw(int k) {  // 9afcb3
  Oam_AllocateFromRegionB(0x20);
  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  OamEnt *oam = GetOamCurPtr();
  for (int i = 5; i >= 0; i--, oam++) {
    oam->x = kMovableMantle_X[i] + info.x;
    oam->y = kMovableMantle_Y[i] + info.y;
    oam->charnum = kMovableMantle_Char[i];
    oam->flags = kMovableMantle_Flags[i];
  }
  Sprite_CorrectOamEntries(k, 5, 2);
}

void Mothula_Draw(int k) {  // 9afdb5
  static const DrawMultipleData kMothula_Dmd[24] = {
    {-24, -8, 0x0080, 2},
    { -8, -8, 0x0082, 2},
    {  8, -8, 0x4082, 2},
    { 24, -8, 0x4080, 2},
    {-24,  8, 0x00a0, 2},
    { -8,  8, 0x00a2, 2},
    {  8,  8, 0x40a2, 2},
    { 24,  8, 0x40a0, 2},
    {-24, -8, 0x0084, 2},
    { -8, -8, 0x0086, 2},
    {  8, -8, 0x4086, 2},
    { 24, -8, 0x4084, 2},
    {-24,  8, 0x00a4, 2},
    { -8,  8, 0x00a6, 2},
    {  8,  8, 0x40a6, 2},
    { 24,  8, 0x40a4, 2},
    { -8, -8, 0x0088, 2},
    { -8, -8, 0x0088, 2},
    {  8, -8, 0x4088, 2},
    {  8, -8, 0x4088, 2},
    { -8,  8, 0x00a8, 2},
    { -8,  8, 0x00a8, 2},
    {  8,  8, 0x40a8, 2},
    {  8,  8, 0x40a8, 2},
  };
  oam_cur_ptr = 0x920;
  oam_ext_cur_ptr = 0xa68;
  PrepOamCoordsRet info;
  Sprite_DrawMultiple(k, &kMothula_Dmd[sprite_graphics[k] * 8], 8, &info);
  if (sprite_pause[k])
    return;
  info.y += sprite_z[k];
  static const int8 kMothula_Draw_X[27] = {
    0,  3, 6, 9, 12, -3, -6, -9, -12,  0,  2, 4, 6, 8, -2, -4,
    -6, -8, 0, 1,  2,  3,  4, -1,  -2, -3, -4,
  };
  OamEnt *oam = GetOamCurPtr() + 10;
  int g = sprite_graphics[k];
  for (int i = 8; i >= 0; i--, oam++) {
    SetOamHelper0(oam, info.x + kMothula_Draw_X[g * 9 + i], info.y + 16,
                  0x6c, 0x24, 2);
  }
}

void BottleMerchant_BuyBee(int k) {  // 9afe88
  static const int8 kBottleVendor_GoodBeeX[5] = {-6, -3, 0, 4, 7};
  static const int8 kBottleVendor_GoodBeeY[5] = {11, 14, 16, 14, 11};
  SpriteSpawnInfo info;
  SpriteSfx_QueueSfx3WithPan(k, 0x13);
  tmp_counter = 4;
  do {
    int j = Sprite_SpawnDynamically(k, 0xdb, &info);
    if (j >= 0) {
      Sprite_SetSpawnedCoordinates(j, &info);
      sprite_x_lo[j] = info.r0_x + 4;
      sprite_stunned[j] = 0xff;
      sprite_x_vel[j] = kBottleVendor_GoodBeeX[tmp_counter];
      sprite_y_vel[j] = kBottleVendor_GoodBeeY[tmp_counter];
      sprite_z_vel[j] = 32;
      sprite_delay_aux4[j] = 32;
    }
  } while (!sign8(--tmp_counter));
}

void Sprite_ChickenLady(int k) {  // 9afed3
  sprite_D[k] = 1;
  Lady_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (sprite_delay_main[k] == 1) {
    dialogue_message_index = 0x17d;
    Sprite_ShowMessageMinimal();
  }
  sprite_graphics[k] = frame_counter >> 4 & 1;
}

void Overworld_DrawWoodenDoor(uint16 pos, bool unlocked) {  // 9bc952
  Overworld_DrawMap16_Persist(pos, unlocked ? 0xda5 : 0xda4);
  Overworld_DrawMap16_Persist(pos+2, unlocked ? 0xda7 : 0xda6);
  nmi_load_bg_from_vram = 1;
}

void Sprite_D4_Landmine(int k) {  // 9d8099
  static const uint8 kLandMine_OamFlags[4] = {4, 2, 8, 2};

  Landmine_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (!Landmine_CheckDetonationFromHammer(k)) {
    if (!sprite_delay_main[k]) {
      sprite_oam_flags[k] = 4;
      if (Sprite_CheckDamageToLink(k))
        sprite_delay_main[k] = 8;
      return;
    }
    if (sprite_delay_main[k] != 1) {
      sprite_oam_flags[k] = kLandMine_OamFlags[sprite_delay_main[k] >> 1 & 3];
      return;
    }
  }
  sprite_state[k] = 0;
  int j = Sprite_SpawnBomb(k);
  if (j >= 0) {
    sprite_state[j] = 6;
    sprite_C[j] = 2;
    sprite_oam_flags[j] = 2;
    sprite_flags4[j] = 9;
    sprite_delay_aux1[j] = 31;
    sprite_flags2[j] = 3;
    sound_effect_1 = Sprite_CalculateSfxPan(k) | 12;
  }
}

void Landmine_Draw(int k) {  // 9d810c
  static const DrawMultipleData kLandmine_Dmd[2] = {
    {0, 4, 0x0070, 0},
    {8, 4, 0x4070, 0},
  };
  Oam_AllocateFromRegionB(8);
  if (byte_7E0FC6 >= 3)
    return;
  Sprite_DrawMultiple(k, kLandmine_Dmd, 2, NULL);
}

void Sprite_D3_Stal(int k) {  // 9d8129
  static const uint8 kStal_Gfx[5] = {2, 2, 1, 0, 1};
  if (byte_7E0FC6 < 3) {
    if (!sprite_ai_state[k])
      Oam_AllocateFromRegionB(4);
    Stal_Draw(k);
  }
  if (Sprite_ReturnIfInactive(k))
    return;
  if (Sprite_ReturnIfRecoiling(k))
    return;
  switch(sprite_ai_state[k]) {
  case 0: // dormant
    sprite_ignore_projectile[k] = 1;
    if (Sprite_CheckDamageToLink_same_layer(k)) {
      Sprite_NullifyHookshotDrag();
      Sprite_RepelDash();
      if (!sprite_delay_main[k]) {
        sprite_delay_main[k] = 64;
        SpriteSfx_QueueSfx2WithPan(k, 0x22);
      }
    }
    if (sprite_delay_main[k] != 0) {
      if (sprite_delay_main[k] - 1) {
        sprite_hit_timer[k] = (sprite_delay_main[k] - 1) | 64;
      } else {
        sprite_ignore_projectile[k] = 0;
        sprite_ai_state[k]++;
        sprite_hit_timer[k] = 0;
        sprite_flags3[k] &= ~0x40;
        sprite_flags2[k] &= ~0x80;
      }
    }
    break;
  case 1: // active
    Sprite_CheckDamageToAndFromLink(k);
    Sprite_MoveXYZ(k);
    Sprite_CheckTileCollision(k);
    sprite_z_vel[k]-=2;
    if (sign8(sprite_z[k])) {
      sprite_z[k] = 0;
      sprite_z_vel[k] = 16;
      Sprite_ApplySpeedTowardsLink(k, 12);
    }
    if (!(frame_counter & 3) && ++sprite_subtype2[k] == 5)
      sprite_subtype2[k] = 0;
    sprite_graphics[k] = kStal_Gfx[sprite_subtype2[k]];
    break;
  }
}

void Stal_Draw(int k) {  // 9d820c
  static const DrawMultipleData kStal_Dmd[6] = {
    {0,  0, 0x0044, 2},
    {4, 11, 0x0070, 0},
    {0,  0, 0x0044, 2},
    {4, 12, 0x0070, 0},
    {0,  0, 0x0044, 2},
    {4, 13, 0x0070, 0},
  };
  PrepOamCoordsRet info;
  int n = sprite_ai_state[k] ? 2 : 1;
  Sprite_DrawMultiple(k, &kStal_Dmd[sprite_graphics[k] * 2], n, &info);
  if (sprite_ai_state[k])
    SpriteDraw_Shadow(k, &info);
}

void Sprite_D2_FloppingFish(int k) {  // 9d8235
  static const int8 kFish_Xvel[8] = {0, 12, 16, 12, 0, -12, -16, -12};
  static const int8 kFish_Yvel[8] = {-16, -12, 0, 12, 16, 12, 0, -12};
  static const uint8 kFish_Tab1[2] = {2, 0};
  static const uint8 kFish_Gfx[3] = {1, 5, 3};
  static const uint8 kFish_Gfx2[17] = {5, 5, 6, 6, 5, 5, 4, 4, 3, 7, 7, 8, 8, 7, 7, 8, 8 };

  if (byte_7E0FC6 < 3)
    Fish_Draw(k);
  if (sprite_state[k] == 10) {
    sprite_ai_state[k] = 4;
    sprite_graphics[k] = (frame_counter >> 4 & 1) + 3;
  }
  if (Sprite_ReturnIfInactive(k))
    return;
  switch (sprite_ai_state[k]) {
  case 0:  // check deep water
    Sprite_CheckTileCollision(k);
    if (sprite_tiletype == 8)
      sprite_state[k] = 0;
    else
      sprite_ai_state[k] = 1;
    break;
  case 1:  // flop around
    Sprite_CheckIfLifted_permissive(k);
    Sprite_BounceFromTileCollision(k);
    Sprite_MoveXYZ(k);
    sprite_z_vel[k] -= 2;
    if (sign8(sprite_z[k])) {
      sprite_z[k] = 0;
      if (sprite_tiletype == 9) {
        Sprite_SpawnSmallSplash(k);
      } else if (sprite_tiletype == 8) {
        sprite_state[k] = 0;
        Sprite_SpawnSmallSplash(k);
      }
      sprite_z_vel[k] = (GetRandomNumber() & 15) + 16;
      int j = GetRandomNumber() & 7;
      sprite_x_vel[k] = kFish_Xvel[j];
      sprite_y_vel[k] = kFish_Yvel[j];
      sprite_D[k]++;
      sprite_subtype2[k] = 3;
    }
    sprite_subtype2[k]++;
    if (!(sprite_subtype2[k] & 7)) {
      int j = sprite_D[k] & 1;
      if (sprite_A[k] != kFish_Tab1[j])
        sprite_A[k] += j ? -1 : 1;
    }
    sprite_graphics[k] = kFish_Gfx[sprite_A[k]] + (frame_counter >> 3 & 1);
    break;
  case 2:  // pause before leap
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k] = 3;
      sprite_z_vel[k] = 48;
      Sprite_SpawnSmallSplash(k);
    }
    break;
  case 3:  // leaping
    Sprite_MoveZ(k);
    sprite_z_vel[k] -= 2;
    if (sprite_z_vel[k] == 0 && sprite_A[k] != 0) {
      dialogue_message_index = 0x176;
      Sprite_ShowMessageMinimal();
    }
    if (sign8(sprite_z[k])) {
      sprite_z[k] = 0;
      Sprite_SpawnSmallSplash(k);
      if (sprite_A[k]) {
        SpriteSpawnInfo info;
        int j = Sprite_SpawnDynamically(k, 0xdb, &info);
        if (j >= 0) {
          Sprite_SetSpawnedCoordinates(j, &info);
          Sprite_SetX(j, info.r0_x + 4);
          sprite_stunned[j] = 255;
          sprite_z_vel[j] = 48;
          sprite_delay_aux3[j] = 48;
          Sprite_ApplySpeedTowardsLink(j, 16);
        }
      }
      sprite_state[k] = 0;
    }
    sprite_graphics[k] = kFish_Gfx2[++sprite_subtype2[k] >> 2];
    break;
  case 4:  // wiggle
    if (!sprite_z[k])
      sprite_ai_state[k] = 1;
    Sprite_MoveXY(k);
    ThrownSprite_TileAndSpriteInteraction(k);
    break;
  }
}

void Fish_Draw(int k) {  // 9d8483
  static const DrawMultipleData kFish_Dmd[16] = {
    {-4, 8, 0x045e, 0},
    { 4, 8, 0x045f, 0},
    {-4, 8, 0x845e, 0},
    { 4, 8, 0x845f, 0},
    {-4, 8, 0x445f, 0},
    { 4, 8, 0x445e, 0},
    {-4, 8, 0xc45f, 0},
    { 4, 8, 0xc45e, 0},
    { 0, 0, 0x0461, 0},
    { 0, 8, 0x0471, 0},
    { 0, 0, 0x4461, 0},
    { 0, 8, 0x4471, 0},
    { 0, 0, 0x8471, 0},
    { 0, 8, 0x8461, 0},
    { 0, 0, 0xc471, 0},
    { 0, 8, 0xc461, 0},
  };
  static const DrawMultipleData kFish_Dmd2[9] = {
    {-2, 11, 0x0438, 0},
    { 0, 11, 0x0438, 0},
    { 2, 11, 0x0438, 0},
    {-1, 11, 0x0438, 0},
    { 0, 11, 0x0438, 0},
    { 1, 11, 0x0438, 0},
    { 0, 11, 0x0438, 0},
    { 0, 11, 0x0438, 0},
    { 0, 11, 0x0438, 0},
  };
  PrepOamCoordsRet info;
  if (sprite_graphics[k] == 0) {
    Sprite_PrepOamCoord(k, &info);
    return;
  }
  cur_sprite_x += 4;
  Sprite_DrawMultiple(k, &kFish_Dmd[(sprite_graphics[k] - 1) * 2], 2, &info);
  cur_sprite_y += sprite_z[k];
  int j = sprite_z[k] >> 2;
  oam_cur_ptr += 8, oam_ext_cur_ptr += 2;
  Sprite_DrawMultiple(k, &kFish_Dmd2[((j >= 2) ? 2 : j) * 3], 3, &info);
  Sprite_Get16BitCoords(k);
}

void ChimneySmoke_Draw(int k) {  // 9d8531
  static const DrawMultipleData kChimneySmoke_Dmd[8] = {
    {0, 0, 0x0086, 0},
    {8, 0, 0x0087, 0},
    {0, 8, 0x0096, 0},
    {8, 8, 0x0097, 0},
    {1, 1, 0x0086, 0},
    {7, 1, 0x0087, 0},
    {1, 7, 0x0096, 0},
    {7, 7, 0x0097, 0},
  };
  Sprite_DrawMultiple(k, &kChimneySmoke_Dmd[(sprite_graphics[k] & 1) * 4], 4, NULL);
}

void Sprite_D1_BunnyBeam(int k) {  // 9d858b
  if (player_is_indoors)
    Sprite_BunnyBeam(k);
  else
    Sprite_Chimney(k);

}

void Sprite_Chimney(int k) {  // 9d858f
  sprite_flags3[k] = 64;
  sprite_ignore_projectile[k] = 64;
  if (!sprite_ai_state[k]) {
    if (Sprite_ReturnIfInactive(k))
      return;
    if (sprite_delay_main[k])
      return;
    sprite_delay_main[k] = 67;
    SpriteSpawnInfo info;
    int j = Sprite_SpawnDynamically(k, 0xd1, &info);
    if (j < 0)
      return;
    Sprite_SetSpawnedCoordinates(j, &info);
    int t = (uint8)info.r0_x + 8;
    sprite_x_lo[j] = t;
    sprite_y_lo[j] = info.r2_y + 4 + (t >> 8);
    sprite_oam_flags[j] = 4;
    sprite_ai_state[j] = 4;
    sprite_flags2[j] = 67;
    sprite_flags3[j] = 67;
    sprite_x_vel[j] = -4;
    sprite_y_vel[j] = -6;
  } else {
    sprite_obj_prio[k] = 0x30;
    ChimneySmoke_Draw(k);
    if (Sprite_ReturnIfInactive(k))
      return;
    Sprite_MoveXY(k);
    if (!(++sprite_subtype2[k] & 7)) {
      int j = sprite_D[k] & 1;
      sprite_x_vel[k] += j ? -1 : 1;
      if (sprite_x_vel[k] == (uint8)(j ? -4 : 4))
        sprite_D[k]++;
    }
    if (!(sprite_subtype2[k] & 31))
      sprite_graphics[k]++;
  }
}

void Sprite_BunnyBeam(int k) {  // 9d85e0
  static const uint8 kRabbitBeam_Gfx[6] = {0xd7, 0xd7, 0xd7, 0x91, 0x91, 0x91};
  PrepOamCoordsRet info;
  if (!sprite_ai_state[k]) {
    Sprite_PrepOamCoord(k, &info);
    if (Sprite_ReturnIfInactive(k))
      return;
    if (!Sprite_CheckTileCollision(k)) {
      sprite_ai_state[k]++;
      sprite_delay_main[k] = 128;
    }
    return;
  }

  SpriteDraw_Antfairy(k);
  if (!sprite_pause[k]) {
    OamEnt *oam = GetOamCurPtr();
    uint8 charnum = kRabbitBeam_Gfx[sprite_graphics[k]];
    for (int i = 0; i < 5; i++) {
      oam[i].charnum = charnum;
      oam[i].flags = oam[i].flags & 0xf0 | 2;
    }
  }
  if (Sprite_ReturnIfInactive(k))
    return;
  if (!sprite_delay_main[k]) {
    sprite_bump_damage[k] = 0x30;
    if (Sprite_CheckDamageToLink(k)) {
      sprite_state[k] = 0;
      link_timer_tempbunny = 256;
    }
    if (link_is_on_lower_level == sprite_floor[k])
      Sprite_ApplySpeedTowardsLink(k, 16);
    Sprite_MoveXY(k);
    if (Sprite_CheckTileCollision(k)) {
      sprite_state[k] = 0;
      Sprite_SpawnPoofGarnish(k);
      SpriteSfx_QueueSfx2WithPan(k, 0x15);
    }
  }
}

void Sprite_D0_Lynel(int k) {  // 9d866a
  static const int8 kLynel_AttackGfx[4] = {5, 2, 8, 10};
  static const int8 kLynel_Gfx[8] = {3, 0, 6, 9, 4, 1, 7, 10};
  Lynel_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (Sprite_ReturnIfRecoiling(k))
    return;
  sprite_D[k] = Sprite_DirectionToFaceLink(k, NULL);
  Sprite_CheckDamageToAndFromLink(k);
  switch(sprite_ai_state[k]) {
  case 0:  // target
    if (!sprite_delay_main[k]) {
      static const int8 kLynel_Xtarget[4] = {-96, 96, 0, 0};
      static const int8 kLynel_Ytarget[4] = {8, 8, -96, 112};
      int j = sprite_D[k];
      int x = link_x_coord + kLynel_Xtarget[j];
      sprite_A[k] = x, sprite_B[k] = x >> 8;
      int y = link_y_coord + kLynel_Ytarget[j];
      sprite_C[k] = y, sprite_E[k] = y >> 8;
      sprite_ai_state[k]++;
      sprite_delay_main[k] = 80;
    }
    sprite_graphics[k] = kLynel_Gfx[sprite_subtype2[k] & 4 | sprite_D[k]];
    break;
  case 1:  // approach
    if (sprite_delay_main[k]) {
      if (!((k ^ frame_counter) & 3)) {
        uint16 x = sprite_A[k] | sprite_B[k] << 8;
        uint16 y = sprite_C[k] | sprite_E[k] << 8;
        if ((uint16)(x - cur_sprite_x + 5) < 10 && (uint16)(y - cur_sprite_y + 5) < 10)
          goto incr_state;
        ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLocation(k, x, y, 24);
        sprite_x_vel[k] = pt.x;
        sprite_y_vel[k] = pt.y;
      }
      Sprite_MoveXY(k);
      if (Sprite_CheckTileCollision(k))
        goto incr_state;
      sprite_graphics[k] = kLynel_Gfx[++sprite_subtype2[k] & 4 | sprite_D[k]];
    } else {
incr_state:
      sprite_ai_state[k]++;
      sprite_delay_main[k] = 32;
    }
    break;
  case 2:  // attack
    if (!sprite_delay_main[k]) {
      sprite_delay_main[k] = (GetRandomNumber() & 15) + 16;
      sprite_ai_state[k] = 0;
      return;
    }
    if (sprite_delay_main[k] == 16) {
      int j = Sprite_SpawnFirePhlegm(k);
      if (j >= 0 && link_shield_type != 3)
        sprite_flags5[j] = 0;
    }
    sprite_graphics[k] = kLynel_AttackGfx[sprite_D[k]];
    Sprite_CheckTileCollision(k);
    break;
  }
}

void Lynel_Draw(int k) {  // 9d8880
  static const DrawMultipleData kLynel_Dmd[33] = {
    {-5, -11, 0x00cc, 2},
    {-4,   0, 0x00e4, 2},
    { 4,   0, 0x00e5, 2},
    {-5, -10, 0x00cc, 2},
    {-4,   0, 0x00e7, 2},
    { 4,   0, 0x00e8, 2},
    {-5, -11, 0x00c8, 2},
    {-4,   0, 0x00e4, 2},
    { 4,   0, 0x00e5, 2},
    { 5, -11, 0x40cc, 2},
    {-4,   0, 0x40e5, 2},
    { 4,   0, 0x40e4, 2},
    { 5, -10, 0x40cc, 2},
    {-4,   0, 0x40e8, 2},
    { 4,   0, 0x40e7, 2},
    { 5, -11, 0x40c8, 2},
    {-4,   0, 0x40e8, 2},
    { 4,   0, 0x40e7, 2},
    { 0,  -9, 0x00ce, 2},
    {-4,   0, 0x00ea, 2},
    { 4,   0, 0x00eb, 2},
    { 0,  -9, 0x00ce, 2},
    {-4,   0, 0x40eb, 2},
    { 4,   0, 0x40ea, 2},
    { 0,  -9, 0x00ca, 2},
    {-4,   0, 0x40eb, 2},
    { 4,   0, 0x00eb, 2},
    { 0, -14, 0x00c6, 2},
    {-4,   0, 0x00ed, 2},
    { 4,   0, 0x00ee, 2},
    { 0, -14, 0x00c6, 2},
    {-4,   0, 0x40ee, 2},
    { 4,   0, 0x40ed, 2},
  };
  PrepOamCoordsRet info;
  Sprite_DrawMultiple(k, &kLynel_Dmd[sprite_graphics[k] * 3], 3, &info);
  SpriteDraw_Shadow(k, &info);
}

void Sprite_SpawnPhantomGanon(int k) {  // 9d88a1
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0xc9, &info);
  Sprite_SetSpawnedCoordinates(j, &info);
  sprite_flags2[j] = 2;
  sprite_ignore_projectile[j] = 2;
  sprite_anim_clock[j] = 1;
  sprite_oam_flags[j] = 0;
}

void Sprite_PhantomGanon(int k) {  // 9d88bc
  static const uint8 kGanonBat_Gfx[4] = {0, 1, 2, 1};
  static const int8 kGanonBat_TargetXvel[2] = {32, -32};
  static const int8 kGanonBat_TargetYvel[2] = {16, -16};

  if (!sprite_ai_state[k]) {
    PhantomGanon_Draw(k);
    if (Sprite_ReturnIfInactive(k))
      return;
    Sprite_MoveY(k);
    if (!(++sprite_subtype2[k] & 31)) {
      if (--sprite_y_vel[k] == 252) {
        int j = SpawnBossPoof(k);
        Sprite_SetY(j, Sprite_GetY(j) - 20);
      } else if (sprite_y_vel[k] == 251) {
        sprite_ai_state[k]++;
        sprite_delay_main[k] = 255;
        sprite_y_vel[k] = -4;
      }
    }
  } else {
    GanonBat_Draw(k);
    if (sprite_pause[k]) {
      sprite_state[k] = 0;
      dung_savegame_state_bits |= 0x8000;
    }
    if (Sprite_ReturnIfInactive(k))
      return;
    sprite_graphics[k] = kGanonBat_Gfx[frame_counter >> 2 & 3];
    if (sprite_delay_main[k]) {
      if (sprite_delay_main[k] < 208) {
        int j = sprite_head_dir[k] & 1;
        sprite_y_vel[k] += j ? -1 : 1;
        if (sprite_y_vel[k] == (uint8)kGanonBat_TargetYvel[j])
          sprite_head_dir[k]++;
        j = sprite_D[k] & 1;
        sprite_x_vel[k] += j ? -1 : 1;
        if (sprite_x_vel[k] == (uint8)kGanonBat_TargetXvel[j])
          sprite_D[k]++;
        if (sprite_x_vel[k] == 0)
          SpriteSfx_QueueSfx3WithPan(k, 0x1e);
      }
      ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLocation(k, link_x_coord & 0xff00 | 0x78, link_y_coord & 0xff00 | 0x50, 5);
      uint8 xvel = sprite_x_vel[k], yvel = sprite_y_vel[k];
      sprite_x_vel[k] = xvel + pt.x, sprite_y_vel[k] = yvel + pt.y;
      Sprite_MoveXY(k);
      sprite_x_vel[k] = xvel, sprite_y_vel[k] = yvel;
    } else {
      Sprite_MoveXY(k);
      if (sprite_x_vel[k] != 64) {
        sprite_x_vel[k]++;
        sprite_y_vel[k]--;
      }
    }
  }
}

void GanonBat_Draw(int k) {  // 9d89eb
  static const DrawMultipleData kGanonBat_Dmd[6] = {
    {-8, 0, 0x0560, 2},
    { 8, 0, 0x4560, 2},
    {-8, 0, 0x0562, 2},
    { 8, 0, 0x4562, 2},
    {-8, 0, 0x0544, 2},
    { 8, 0, 0x4544, 2},
  };
  Sprite_DrawMultiple(k, &kGanonBat_Dmd[sprite_graphics[k] * 2], 2, NULL);
}

void PhantomGanon_Draw(int k) {  // 9d8a84
  static const DrawMultipleData kPhantomGanon_Dmd[16] = {
    {-16, -8, 0x0d46, 2},
    { -8, -8, 0x0d47, 2},
    {  8, -8, 0x4d47, 2},
    { 16, -8, 0x4d46, 2},
    {-16,  8, 0x0d69, 2},
    { -8,  8, 0x0d6a, 2},
    {  8,  8, 0x4d6a, 2},
    { 16,  8, 0x4d69, 2},
    {-16, -8, 0x0d46, 2},
    { -8, -8, 0x0d47, 2},
    {  8, -8, 0x4d47, 2},
    { 16, -8, 0x4d46, 2},
    {-16,  8, 0x0d66, 2},
    { -8,  8, 0x0d67, 2},
    {  8,  8, 0x4d67, 2},
    { 16,  8, 0x4d66, 2},
  };
  oam_cur_ptr = 0x950;
  oam_ext_cur_ptr = 0xa74;
  Sprite_DrawMultiple(k, &kPhantomGanon_Dmd[sprite_graphics[k] * 8], 8, NULL);
}

void SwishEvery16Frames(int k) {  // 9d8aa9
  if (!(frame_counter & 15))
    SpriteSfx_QueueSfx3WithPan(k, 0x6);
}

void Sprite_GanonTrident(int k) {  // 9d8ab6
  Trident_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_CheckDamageToAndFromLink(k);
  SwishEvery16Frames(k);
  Sprite_MoveXY(k);
  sprite_G[k] = kGanon_G_Func2[--sprite_subtype2[k] >> 2 & 7];
  if (sprite_delay_main[k]) {
    if (sprite_delay_main[k] & 1)
      return;
    ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLink(k, 32);
    Sprite_ApproachTargetSpeed(k, pt.x, pt.y);
  } else {
    int x = Sprite_GetX(0) + (sprite_D[0] ? -16 : 24);
    int y = Sprite_GetY(0) - 16;
    if (Ganon_AttemptTridentCatch(x, y)) {
      sprite_state[k] = 0;
      sprite_ai_state[0] = 3;
      sprite_delay_main[0] = 16;
    }
    ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLocation(k, x, y, 32);
    Sprite_ApproachTargetSpeed(k, pt.x, pt.y);
  }
}

void Sprite_FireBat_Trailer(int k) {  // 9d8b49
  FireBat_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  FireBat_Move(k);
}

void Sprite_SpiralFireBat(int k) {  // 9d8b52
  FireBat_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  {
    uint16 x = sprite_A[k] | sprite_B[k] << 8;
    uint16 y = sprite_C[k] | sprite_E[k] << 8;
    ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLocation(k, x, y, 2);
    ProjectSpeedRet pt2 = Sprite_ProjectSpeedTowardsLocation(k, x, y, 80);
    sprite_x_vel[k] = pt2.y - pt.x;
    sprite_y_vel[k] = -pt2.x - pt.y;
  }
  FireBat_Move(k);
}

void FireBat_Move(int k) {  // 9d8b90
  FireBat_Animate(k);
  Sprite_MoveXY(k);
  if (sprite_subtype2[k] & 7)
    return;
  int j = Garnish_FlameTrail(k, true);
//  garnish_type[j] = 0x10;
  garnish_countdown[j] = (sprite_anim_clock[k] == 5) ? 0x2f : 0x4f;
}

void Sprite_FireBat_Launched(int k) {  // 9d8bd7
  FireBat_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_CheckDamageToLink(k);
  switch (sprite_ai_state[k]) {
  case 0:
    GetPositionRelativeToTheGreatOverlordGanon(k);
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k] = 1;
    } else {
      sprite_graphics[k] = sprite_delay_main[k] >> 2 & 1;
    }
    break;
  case 1:
    GetPositionRelativeToTheGreatOverlordGanon(k);
    sprite_graphics[k] = ++sprite_subtype2[k] >> 2 & 1;
    break;
  case 2:
    Sprite_MoveXY(k);
    sprite_defl_bits[k] = 64;
    if (sprite_delay_aux1[k] == 0) {
      if (sprite_delay_main[k] == 0) {
        FireBat_Animate(k);
        FireBat_Animate(k);

      } else {
        uint8 t = sprite_delay_main[k] - 1;
        if (t == 0)
          sprite_delay_aux1[k] = t = 35;
        sprite_graphics[k] = t >> 2 & 1;
      }
    } else if (sprite_delay_aux1[k] == 1) {
      Sprite_ApplySpeedTowardsLink(k, 48);
      SpriteSfx_QueueSfx3WithPan(k, 0x1e);
      FireBat_Animate(k);
      FireBat_Animate(k);
    } else {
      static const uint8 kFirebat_Gfx2[9] = { 4, 4, 4, 3, 3, 3, 2, 2, 2 };
      sprite_graphics[k] = kFirebat_Gfx2[sprite_delay_aux1[k] >> 2];
    }
    break;
  }
}

void GetPositionRelativeToTheGreatOverlordGanon(int k) {  // 9d8bee
  static const int8 kFirebat_X[2] = { 20, -18 };
  static const int8 kFirebat_Y[2] = { -20, -20 };

  int j = sprite_D[0];
  Sprite_SetX(k, (overlord_x_hi[k] | overlord_y_hi[k] << 8) + kFirebat_X[j]);
  Sprite_SetY(k, (overlord_gen2[k] | overlord_floor[k] << 8) + kFirebat_Y[j]);
}

void FireBat_Animate(int k) {  // 9d8c43
  static const uint8 kFirebat_Gfx[4] = { 4, 5, 6, 5 };
  sprite_graphics[k] = kFirebat_Gfx[++sprite_subtype2[k] >> 2 & 3];
}

void FireBat_Draw(int k) {  // 9d8ca9
  static const int8 kFirebat_Draw_X[2] = { -8, 8 };
  static const uint8 kFirebat_Draw_Char[7] = { 0x88, 0x88, 0x8a, 0x8c, 0x68, 0xaa, 0xa8 };
  static const uint8 kFirebat_Draw_Flags[14] = { 0, 0xc0, 0x80, 0x40, 0, 0x40, 0, 0x40, 0, 0x40, 0, 0x40, 0, 0x40 };

  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  OamEnt *oam = GetOamCurPtr();
  int g = sprite_graphics[k];
  for (int i = 1; i >= 0; i--, oam++) {
    SetOamHelper0(oam, info.x + kFirebat_Draw_X[i], info.y, kFirebat_Draw_Char[g],
                  kFirebat_Draw_Flags[g * 2 + i] | info.flags, 2);
  }
}

bool Ganon_AttemptTridentCatch(uint16 x, uint16 y) {  // 9d8d06
  return (uint16)(cur_sprite_x - x + 4) < 8 && (uint16)(cur_sprite_y - y + 4) < 8;
}

void Ganon_HandleFireBatCircle(int k) {  // 9d8d70
  static const int8 kGanonMath_X[16] = { 0, 16, 24, 28, 32, 28, 24, 16, 0, -16, -24, -28, -32, -28, -24, -16 };
  static const int8 kGanonMath_Y[16] = { 32, 28, 24, 16, 0, -16, -24, -28, -32, -28, -24, -16, 0, 16, 24, 28 };

  WORD(overlord_x_lo[0]) -= 4;

  for (int i = 0; i != 8; i++) {
    int t = WORD(overlord_x_lo[0]) + i * 64 & 0x1ff;
    if (sprite_ai_state[i + 1] != 2) {
      int j = (t >> 5) - 4 & 0xf;
      sprite_x_vel[i + 1] = (int8)kGanonMath_X[j] >> 2;
      sprite_y_vel[i + 1] = (int8)kGanonMath_Y[j] >> 2;
    }
    int x = Sprite_GetX(0) + GanonSin(t, overlord_x_lo[2]);
    overlord_x_hi[i + 1] = x;
    overlord_y_hi[i + 1] = x >> 8;

    int y = Sprite_GetY(0) + GanonSin(t + 0x80, overlord_x_lo[2]);
    overlord_gen2[i + 1] = y;
    overlord_floor[i + 1] = y >> 8;
  }
  tmp_counter = 8;
}

void Ganon_SpawnSpiralBat(int k) {  // 9d8e7c
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamicallyEx(k, 0xc9, &info, 8);
  if (j < 0)
    return;
  Sprite_SetSpawnedCoordinates(j, &info);
  sprite_anim_clock[j] = 4;
  sprite_oam_flags[j] = 3;
  sprite_flags3[j] = 0x40;
  sprite_flags2[j] = 1;
  sprite_defl_bits[j] = 0x80;
  sprite_y_hi[j] = 128;
  sprite_delay_main[j] = 48;
  sprite_bump_damage[j] = 7;
  sprite_ignore_projectile[j] = 7;
}

void Sprite_D6_Ganon(int k) {  // 9d8eb4
  int j;

  if (sign8(sprite_ai_state[k])) {
    if (Sprite_ReturnIfInactive(k))
      return;
    if (!sprite_delay_main[k])
      sprite_state[k] = 0;
    if (!(sprite_delay_main[k] & 1))
      Ganon_Draw(k);
    return;
  }

  if (sprite_delay_aux4[k]) {
    static const uint8 kGanon_GfxB[2] = { 16, 10 };
    sprite_graphics[k] = kGanon_GfxB[sprite_D[k]];
  }

  if (byte_7E04C5 == 2 && byte_7E04C5 != sprite_room[k])
    sprite_delay_aux1[k] = 64;

  sprite_room[k] = byte_7E04C5;

  Ganon_Draw(k);
  if (sprite_delay_aux1[k]) {
    sprite_graphics[k] = 15;
    Ganon_EnableInvincibility(k);
    Sprite_CheckDamageToAndFromLink(k);
    return;
  }

  if (Sprite_ReturnIfInactive(k))
    return;

  if (sprite_delay_aux2[k] == 1)
    Ganon_ExtinguishTorch();
  else if (sprite_delay_aux2[k] == 16)
    Ganon_ExtinguishTorch_adjust_translucency();

  PairU8 pair = Sprite_IsRightOfLink(k);
  static const uint8 kGanon_HeadDir0[2] = { 2, 0 };
  sprite_head_dir[k] = (uint8)(pair.b + 32) < 64 ? 1 : kGanon_HeadDir0[pair.a];

  if (sprite_delay_aux4[k]) {
    sprite_ignore_projectile[k] = sprite_delay_aux4[k];
    if (Sprite_ReturnIfRecoiling(k))
      return;
    sprite_delay_main[k] = 0;
    return;
  }

  if (!(sprite_ignore_projectile[k] | flag_is_link_immobilized) && byte_7E04C5 == 2)
    Sprite_CheckDamageToAndFromLink(k);
  sprite_ignore_projectile[k] = 0;
  switch (sprite_ai_state[k]) {
  case 0:  //
    if (sprite_delay_main[k] == 0) {
      sprite_ai_state[k] = 1;
      sprite_delay_main[k] = 128;
    } else if (sprite_delay_main[k] == 32) {
      music_control = 0x1f;
    } else if (sprite_delay_main[k] == 64) {
      dialogue_message_index = 0x16f;
      Sprite_ShowMessageMinimal();
    }
    break;
  case 1:  //
    if (sprite_health[k] < 209)
      sprite_health[k] = 208;
    if (sprite_delay_main[k] < 64) {
      static const uint8 kGanon_Gfx1[2] = { 2, 10 };
      if (sprite_delay_main[k] == 0) {
        Ganon_SelectWarpLocation(k, 5);
      } else {
        sprite_graphics[k] = kGanon_Gfx1[sprite_D[k]];
      }
    } else if (sprite_delay_main[k] != 64) {
      Ganon_Phase1_AnimateTridentSpin(k);
    } else {
      static const int8 kGanon_X1[2] = { 24, -16 };
      static const int8 kGanon_Y1[2] = { 4, 4 };
      static const int8 kGanon_Xvel1[16] = { 32, 28, 24, 16, 0, -16, -24, -28, -32, -28, -24, -16, 0, 16, 24, 28 };
      static const int8 kGanon_Yvel1[16] = { 0, 16, 24, 28, 32, 28, 24, 16, 0, -16, -24, -28, -32, -28, -24, -16 };
      sprite_G[k] = 0;
      SpriteSpawnInfo info;
      int j = Sprite_SpawnDynamically(k, 0xc9, &info);
      int i = sprite_D[k];
      Sprite_SetX(j, info.r0_x + kGanon_X1[i]);
      Sprite_SetY(j, info.r2_y + kGanon_Y1[i]);
      Sprite_ApplySpeedTowardsLink(k, 31);
      uint8 angle = Sprite_ConvertVelocityToAngle(sprite_x_vel[k], sprite_y_vel[k]);
      sprite_x_vel[j] = kGanon_Xvel1[angle - 2 & 0xf];
      sprite_y_vel[j] = kGanon_Yvel1[angle - 2 & 0xf];
      sprite_delay_main[j] = 112;
      sprite_anim_clock[j] = 2;
      sprite_oam_flags[j] = 1;
      sprite_flags2[j] = 4;
      sprite_defl_bits[j] = 0x84;
      sprite_D[j] = 2;
      sprite_bump_damage[j] = 7;
      sprite_ignore_projectile[j] = 7;
    }
    break;
  case 2: {  //
    static const uint8 kGanon_Gfx2_0[2] = { 0, 8 };
    if (sprite_health[k] < 209)
      sprite_health[k] = 208;
    sprite_graphics[k] = kGanon_Gfx2_0[sprite_D[k]];
    if (sprite_delay_main[k]) {
      sprite_ignore_projectile[k]++;
      if (sprite_delay_main[k] & 1)
        sprite_graphics[k] = 255;
    }
    break;
  }
  case 3:  //
    if (sprite_health[k] < 209)
      sprite_health[k] = 208;
    if (sprite_delay_main[k] != 0) {
      Ganon_Phase1_AnimateTridentSpin(k);
    } else {
      sprite_ai_state[k] = 6;
      sprite_delay_main[k] = 127;
      Ganon_HandleAnimation_Idle(k);
    }
    break;
  case 4:  //
    if (sprite_health[k] < 209)
      sprite_health[k] = 208;
    if (sprite_delay_main[k] != 0)
      Ganon_ShakeHead(k);
    else
      Ganon_SelectWarpLocation(k, 5);
    break;
  case 13:  //
    sprite_health[k] = 100;
    // fall through
  case 5:  //
  case 10:  //
  case 18: { //
    sprite_ignore_projectile[k]++;
    uint16 x = sprite_x_hi[k] << 8 | swamola_target_x_lo[0];
    uint16 y = sprite_y_hi[k] << 8 | swamola_target_y_lo[0];
    if (Ganon_AttemptTridentCatch(x, y)) {
      sprite_D[k] = sprite_subtype[k] >> 2;
      if (sprite_ai_state[k] == 5) {
        sprite_ai_state[k] = 2;
        sprite_delay_main[k] = 32;
      } else if (sprite_health[k] >= 161) {
        sprite_ai_state[k] = 11;
        sprite_delay_main[k] = 40;
      } else if (sprite_health[k] >= 97) {
        sprite_ai_state[k] = 14;
        sprite_delay_main[k] = 40;
      } else {
        sprite_ai_state[k] = 17;
        sprite_delay_main[k] = 104;
      }
    } else {
      ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLocation(k, x, y, 32);
      Sprite_ApproachTargetSpeed(k, pt.x, pt.y);
      Sprite_MoveXY(k);
      if (sprite_delay_main[k] == 0 || frame_counter & 1) {
        sprite_graphics[k] = 255;
        return;
      }
      static const uint8 kGanon_Gfx5[2] = { 2, 10 };
      sprite_graphics[k] = kGanon_Gfx5[sprite_D[k]];
      if (!(frame_counter & 7)) {
        SpriteSpawnInfo info;
        int j = Sprite_SpawnDynamically(k, 0xd6, &info);
        if (j >= 0) {
          Sprite_SetSpawnedCoordinates(j, &info);
          sprite_ignore_projectile[j] = 24;
          sprite_delay_main[j] = 24;
          sprite_ai_state[j] = 255;
          sprite_graphics[j] = sprite_graphics[k];
          sprite_head_dir[j] = sprite_head_dir[k];
        }
      }
    }
    break;
  }
  case 6:  //
    if (sprite_health[k] < 209)
      sprite_health[k] = 208;
    if (!sprite_delay_main[k]) {
      if (sprite_health[k] >= 209) {
        sprite_ai_state[k] = 1;
        sprite_delay_main[k] = 128;
      } else {
        sprite_delay_main[k] = 255;
        sprite_ai_state[k] = 7;
      }
    } else {
      Ganon_ShakeHead(k);
    }
    break;
  case 7:  //
    if (sprite_health[k] < 161)
      sprite_health[k] = 160;
    overlord_x_lo[2] = 40;
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k] = 8;
      sprite_delay_main[k] = 255;
    } else {
      if (sprite_delay_main[k] < 0xc0 && (sprite_delay_main[k] & 0xf) == 0)
        Ganon_SpawnSpiralBat(k);
      Ganon_Phase1_AnimateTridentSpin(k);
      Ganon_HandleFireBatCircle(k);
    }
    break;
  case 8: {  //
    static const int8 kGanon_Tab2[16] = { 0, 0, 0, 0, -1, -1, -2, -1, 0, 0, 0, 0, 1, 2, 1, 1 };
    static const uint8 kGanon_Delay8[8] = { 0x10, 0x30, 0x50, 0x70, 0x90, 0xb0, 0xd0, 0xbd };

    if (sprite_health[k] < 161)
      sprite_health[k] = 160;
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k] = 9;
      sprite_delay_main[k] = 127;
      Ganon_HandleAnimation_Idle(k);
      for (int j = 8; j != 0; j--) {
        sprite_ai_state[j] = 2;
        sprite_delay_main[j] = kGanon_Delay8[j - 1];
      }
    } else {
      overlord_x_lo[2] += kGanon_Tab2[sprite_delay_main[k] >> 4 & 15];
      Ganon_Phase1_AnimateTridentSpin(k);
      Ganon_HandleFireBatCircle(k);
    }
    break;
  }
  case 9:  //
    if (sprite_health[k] < 161)
      sprite_health[k] = 160;
    if (!sprite_delay_main[k]) {
      Ganon_SelectWarpLocation(k, 10);
    } else {
      Ganon_ShakeHead(k);
    }
    break;
  case 11:  //
    sprite_ignore_projectile[k]++;
    Ganon_HandleAnimation_Idle(k);
    if (!sprite_delay_main[k]) {
      sprite_delay_main[k] = 255;
      sprite_ai_state[k] = 7;
    } else if (sprite_delay_main[k] & 1) {
      sprite_graphics[k] = 255;
    }
    break;
  case 12: { //
    j = sprite_delay_main[k];
    if (j == 0) {
      Ganon_SelectWarpLocation(k, 13);
      return;
    }
    int t = 0;
    if (j < 96) {
      t = 1;
      if (j < 72) {
        if (j == 66)
          Ganon_Func1(k, 3);
        t = 2;
      }
    }
    if (sprite_D[k])
      t += 3;
    static const uint8 kGanon_Gfx12[6] = { 5, 6, 7, 13, 14, 10 };
    sprite_graphics[k] = kGanon_Gfx12[t];
    if ((sprite_hit_timer[k] & 127) == 1) {
      sprite_ai_state[k] = 15;
      sprite_z_vel[k] = 24;
      sprite_delay_main[k] = 0;
    }
    break;
  }
  case 14:  //
    sprite_ignore_projectile[k]++;
    Ganon_HandleAnimation_Idle(k);
    sprite_G[k] = 0;
    if (!sprite_delay_main[k]) {
      if (GetRandomNumber() & 1) {
        Ganon_SelectWarpLocation(k, 13);
      } else {
        sprite_delay_main[k] = 127;
        sprite_ai_state[k] = 12;
      }
    } else if (sprite_delay_main[k] & 1) {
      sprite_graphics[k] = 255;
    }
    break;
  case 15: {  //
    static const uint8 kGanon_Gfx15[2] = { 6, 14 };
    if (sprite_delay_main[k]) {
      if (sprite_delay_main[k] == 1) {
        sprite_ai_state[k] = 16;
        sprite_z_vel[k] = 160;
        return;
      }
    } else {
      Sprite_MoveZ(k);
      if (--sprite_z_vel[k] == 0)
        sprite_delay_main[k] = 32;

    }
    sprite_graphics[k] = kGanon_Gfx15[sprite_D[k]];
    break;
  }
  case 16: { //
    bg1_y_offset = 0;
    if (sprite_delay_main[k]) {
      if (sprite_delay_main[k] == 1) {
        sound_effect_ambient = 5;
        Ganon_SelectWarpLocation(k, 13);
        flag_is_link_immobilized = 0;
        Ganon_SpawnFallingTilesOverlord(k);
        if (sprite_anim_clock[k] >= 4) {
          Ganon_SelectWarpLocation(k, 10);
          sprite_health[k] = 96;
          sprite_delay_aux2[k] = 224;
          dialogue_message_index = 0x170;
          Sprite_ShowMessageMinimal();
        }
      } else {
        bg1_y_offset = (sprite_delay_main[k] - 1) & 1 ? -1 : 1;
        flag_is_link_immobilized = 1;
      }
    } else {
      static const uint8 kGanon_Gfx16[2] = { 2, 10 };

      Sprite_MoveZ(k);
      if (sign8(sprite_z[k])) {
        sprite_z_vel[k] = 0;
        sprite_z[k] = 0;
        sprite_delay_main[k] = 96;
        sound_effect_ambient = 7;
        SpriteSfx_QueueSfx2WithPan(k, 0xc);
      }
      sprite_graphics[k] = kGanon_Gfx16[sprite_D[k]];
    }
    break;
  }
  case 17: {  //
    static const uint8 kGanon_Gfx17b[2] = { 6, 14 };
    static const uint8 kGanon_Gfx17[2] = { 7, 10 };
    sprite_graphics[k] = kGanon_Gfx17b[sprite_D[k]];
    if (!sprite_delay_main[k]) {
      Ganon_SelectWarpLocation(k, 0x12);
      return;
    } else if (sprite_delay_main[k] == 52) {
      Ganon_Func1(k, 5);
    } else if (sprite_delay_main[k] < 52) {
      sprite_graphics[k] = kGanon_Gfx17[sprite_D[k]];
    }
    if (sprite_delay_main[k] >= 72 || sprite_delay_main[k] < 40) {
      sprite_ignore_projectile[k]++;
      if (sprite_delay_main[k] & 1)
        sprite_graphics[k] = 0xff;
    }
    Ganon_EnableInvincibility(k);
    break;
  }
  case 19:  //
    sprite_oam_flags[k] = 5;
    sprite_flags[k] = 2;
    if (!sprite_delay_main[k]) {
      sprite_oam_flags[k] = 1;
      Ganon_SelectWarpLocation(k, 18);
      sprite_type[k] = 0xd6;
      sprite_hit_timer[k] = 0;
    } else {
      static const uint8 kGanon_Gfx19[2] = { 5, 13 };
      sprite_graphics[k] = kGanon_Gfx19[sprite_D[k]];
    }
    break;
  }
}

void Ganon_EnableInvincibility(int k) {  // 9d8ffa
  if ((sprite_hit_timer[k] & 127) == 26) {
    sprite_hit_timer[k] = 0;
    sprite_ai_state[k] = 19;
    sprite_delay_main[k] = 127;
    sprite_type[k] = 215;
  }
}

void Ganon_SpawnFallingTilesOverlord(int k) {  // 9d90d0
  static const uint8 kGanon_Ov_Type[4] = { 12, 13, 14, 15 };
  static const uint8 kGanon_Ov_X[4] = { 0x18, 0xd8, 0xd8, 0x18 };
  static const uint8 kGanon_Ov_Y[4] = { 0x28, 0x28, 0xd8, 0xd8 };

  int j;
  for (j = 7; j >= 0 && overlord_type[j] != 0; j--);

  int t = sprite_anim_clock[k];
  if (t >= 4)
    return;
  sprite_anim_clock[k] = t + 1;

  overlord_type[j] = kGanon_Ov_Type[t];
  overlord_x_lo[j] = kGanon_Ov_X[t];
  overlord_x_hi[j] = link_x_coord >> 8;
  overlord_y_lo[j] = kGanon_Ov_Y[t];
  overlord_y_hi[j] = link_y_coord >> 8;
  overlord_gen1[j] = 0;
  overlord_gen2[j] = 0;
}

void Ganon_Func1(int k, int t) {  // 9d9162
  tmp_counter = t;
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamicallyEx(k, 0xC9, &info, 8);
  if (j < 0)
    return;
  SpriteSfx_QueueSfx2WithPan(k, 0x2a);
  Sprite_SetSpawnedCoordinates(j, &info);
  sprite_ignore_projectile[j] = sprite_anim_clock[j] = t;
  sprite_oam_flags[j] = 3;
  sprite_flags3[j] = 0x40;
  sprite_flags2[j] = 0x21;
  sprite_defl_bits[j] = 0x40;
  static const int8 kGanon_Gfx16_Y[2] = { 0, -16 };
  Sprite_SetY(j, info.r2_y + kGanon_Gfx16_Y[sprite_D[k]]);
  Sprite_ApplySpeedTowardsLink(j, 32);
  sprite_delay_main[j] = 16;
  sprite_A[j] = sprite_x_lo[0];
  sprite_B[j] = sprite_x_hi[0];
  sprite_C[j] = sprite_y_lo[0];
  sprite_E[j] = sprite_y_hi[0];
  sprite_bump_damage[j] = 7;
  sprite_ignore_projectile[j] = 7;
}

void Ganon_Phase1_AnimateTridentSpin(int k) {  // 9d93db
  static const uint8 kGanon_GfxFunc2[16] = { 0, 0, 1, 1, 0, 0, 1, 1, 8, 8, 9, 9, 8, 8, 9, 9 };

  int j = (sprite_delay_main[k] >> 2 & 7) + (sprite_D[k] ? 8 : 0);
  sprite_G[k] = kGanon_G_Func2[j];
  sprite_graphics[k] = kGanon_GfxFunc2[j];
  SwishEvery16Frames(k);
}

void Ganon_HandleAnimation_Idle(int k) {  // 9d9443
  static const uint8 kGanon_G[2] = { 9, 10 };
  static const uint8 kGanon_Gfx[2] = { 2, 10 };
  sprite_G[k] = kGanon_G[sprite_D[k]];
  sprite_graphics[k] = kGanon_Gfx[sprite_D[k]];
}

void Ganon_SelectWarpLocation(int k, int a) {  // 9d947f
  int j;
  static const uint8 kGanon_NextSubtype[32] = {
    4, 5, 6, 7, 4, 5, 6, 7, 4, 5, 6, 7, 4, 5, 6, 7,
    0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3,
  };
  static const uint8 kGanon_NextY[8] = { 0x40, 0x30, 0x30, 0x40, 0xb0, 0xc0, 0xc0, 0xb0 };
  static const uint8 kGanon_NextX[8] = { 0x30, 0x50, 0xa0, 0xc0, 0x40, 0x60, 0x90, 0xb0 };

  sprite_subtype[k] = j = kGanon_NextSubtype[GetRandomNumber() & 3 | sprite_subtype[k] << 2];
  swamola_target_x_lo[0] = kGanon_NextX[j];
  swamola_target_y_lo[0] = kGanon_NextY[j];
  sprite_ai_state[k] = a;
  sprite_x_vel[k] = sprite_y_vel[k] = 0;
  sprite_delay_main[k] = 48;
  SpriteSfx_QueueSfx3WithPan(k, 0x28);
}

void Ganon_ShakeHead(int k) {  // 9d94ba
  static const uint8 kGanon_HeadDir[18] = {
    0,  0, 0, 1, 2, 2, 2, 1, 0, 0, 0, 1, 1, 1, 1, 1,
    0, 16,
  };
  sprite_head_dir[k] = kGanon_HeadDir[sprite_delay_main[k] >> 3];
}

void Ganon_Draw(int k) {  // 9d9adf
  PrepOamCoordsRet info;
  if (sign8(sprite_graphics[k]) ||
      sprite_ai_state[k] != 19 && sprite_delay_aux4[k] == 0 && byte_7E04C5 == 0) {
    Sprite_PrepOamCoordOrDoubleRet(k, &info);
    return;
  }
  Trident_Draw(k);
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  OamEnt *oam = GetOamCurPtr() + 5;
  int g = sprite_graphics[k];
  for (int i = 0; i < 12; i++, oam++) {
    int j = g * 12 + i;
    SetOamPlain(oam, info.x + kGanon_Draw_X[j], info.y + kGanon_Draw_Y[j], kGanon_Draw_Char[j],
                 info.flags | (kGanon_Draw_Flags[j] & ((info.flags & 0xf) >= 5 ? 0xf0 : 0xff)), 2);
  }
  static const uint8 kGanon_SprOffs[17] = {
    1, 1, 1, 1, 1, 1, 15, 1, 4, 4, 4, 4, 4, 4, 4, 15, 15,
  };
  if (kGanon_SprOffs[g] != 15) {
    oam = oam - 12 + kGanon_SprOffs[g];
    int j = sprite_head_dir[k] * 2 + (sprite_D[k] ? 6 : 0);
    oam[0].charnum = kGanon_Draw_Char2[j];
    oam[0].flags = (oam[0].flags & 0x3f) | kGanon_Draw_Flags2[j];

    oam[1].charnum = kGanon_Draw_Char2[j + 1];
    oam[1].flags = (oam[1].flags & 0x3f) | kGanon_Draw_Flags2[j + 1];

  }
  if (submodule_index)
    Sprite_CorrectOamEntries(k, 9, 0xff);

  if (sprite_G[k] == 9) {
    static const DrawMultipleData kGanon_Dmd[2] = {
      {16, -3, 0x4c0a, 2},
      {16,  5, 0x4c1a, 2},
    };
    oam_cur_ptr = 0x828, oam_ext_cur_ptr = 0xa2a;
    Sprite_DrawMultiple(k, kGanon_Dmd, 2, NULL);
  }

  uint16 z = sprite_z[k] - 1;
  int frame = (z >> 11) > 4 ? 4 : (z >> 11);
  cur_sprite_y += z;
  oam_cur_ptr = 0x9f4;
  oam_ext_cur_ptr = 0xa9d;
  uint8 bak = sprite_oam_flags[k];
  sprite_oam_flags[k] = 0;
  sprite_obj_prio[k] = 48;
  Sprite_DrawMultiple(k, &kLargeShadow_Dmd[frame * 3], 3, NULL);
  sprite_oam_flags[k] = bak;
  Sprite_Get16BitCoords(k);
}

void Trident_Draw(int k) {  // 9d9c1c
  static const DrawMultipleData kTrident_Dmd[50] = {
  { 10, -10, 0x0864, 0},
  {  5, -15, 0x0864, 0},
  {  0, -20, 0x0864, 0},
  { -5, -25, 0x0864, 0},
  {-18, -38, 0x0844, 2},
  {  1,  -4, 0x0865, 0},
  {  1, -11, 0x0865, 0},
  {  1, -18, 0x0865, 0},
  {  1, -25, 0x0865, 0},
  { -3, -40, 0x0862, 2},
  { -8,  -9, 0x4864, 0},
  { -3, -14, 0x4864, 0},
  {  3, -20, 0x4864, 0},
  {  9, -26, 0x4864, 0},
  { 12, -37, 0x4844, 2},
  {-10, -20, 0x4874, 0},
  { -3, -20, 0x4874, 0},
  {  4, -20, 0x4874, 0},
  { 11, -20, 0x4874, 0},
  { 18, -23, 0x4860, 2},
  {-10, -30, 0xc864, 0},
  { -4, -24, 0xc864, 0},
  {  2, -18, 0xc864, 0},
  {  8, -12, 0xc864, 0},
  { 12,  -8, 0xc844, 2},
  {  1, -32, 0x8865, 0},
  {  1, -25, 0x8865, 0},
  {  1, -18, 0x8865, 0},
  {  1, -11, 0x8865, 0},
  { -3,  -5, 0x8862, 2},
  { 13, -30, 0x8864, 0},
  {  8, -25, 0x8864, 0},
  {  2, -19, 0x8864, 0},
  { -4, -13, 0x8864, 0},
  {-16,  -9, 0x8844, 2},
  { 14, -20, 0x0874, 0},
  {  7, -20, 0x0874, 0},
  {  0, -20, 0x0874, 0},
  { -7, -20, 0x0874, 0},
  {-21, -23, 0x0860, 2},
  { 13, -30, 0x8864, 0},
  {  8, -25, 0x8864, 0},
  {  2, -19, 0x8864, 0},
  { -4, -13, 0x8864, 0},
  {-16,  -9, 0x8844, 2},
  {-10, -30, 0xc864, 0},
  { -4, -24, 0xc864, 0},
  { -4, -24, 0xc864, 0},
  { -4, -24, 0xc864, 0},
  { -4, -24, 0xc864, 0},
  };
  int g = sprite_G[k];
  if (g == 0)
    return;
  static const int8 kTrident_Draw_X[5] = { 24, -16, 0, 16, -8 };
  static const int8 kTrident_Draw_Y[5] = { 4, 4, 16, 21, 19 };

  int j = sprite_G[k] == 9 ? 3 :
          sprite_G[k] >= 9 ? 4 : sprite_D[k];
  cur_sprite_x += kTrident_Draw_X[j];
  cur_sprite_y += kTrident_Draw_Y[j];
  uint8 bak = sprite_obj_prio[k];
  sprite_obj_prio[k] &= ~0xf;
  Sprite_DrawMultiple(k, &kTrident_Dmd[(g - 1) * 5], 5, NULL);
  sprite_obj_prio[k] = bak;
  Sprite_Get16BitCoords(k);
}

void SpritePrep_Swamola_InitializeSegments(int k) {  // 9d9c80
  int j;

  // The snes code uses the wrong bank.. Causes glitches with lanmolas in misery mire
  if (enhanced_features0 & kFeatures0_MiscBugFixes) {
    j = k * 32;
  } else {
    static const uint8 kBuggySwamolaLookup[6] = { 0x1c, 0xa9, 0x03, 0x9d, 0x90, 0x0d };  // wrong bank
    j = kBuggySwamolaLookup[k];
  }
  for (int i = 0; i < 32; i++, j++) {
    swamola_x_lo[j] = sprite_x_lo[k];
    swamola_x_hi[j] = sprite_x_hi[k];
    swamola_y_lo[j] = sprite_y_lo[k];
    swamola_y_hi[j] = sprite_y_hi[k];
  }
}

void Sprite_CF_Swamola(int k) {  // 9d9cb0
  static const uint8 kSwamola_Target_Dir[8] = {1, 2, 3, 4, 5, 6, 7, 8};
  static const int8 kSwamola_Target_X[9] = {0, 0, 32, 32, 32, 0, -32, -32, -32};
  static const int8 kSwamola_Target_Y[9] = {0, -32, -32, 0, 32, 32, 32, 0, -32};
  int j;

  if (sprite_ai_state[k]) {
    if (sign8(sprite_ai_state[k])) {
      Sprite_Swamola_Ripples(k);
      return;
    }
    Swamola_Draw(k);
  }
  Sprite_Get16BitCoords(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  sprite_subtype2[k]++;
  Sprite_CheckDamageToAndFromLink(k);
  uint8 old_vel = sprite_y_vel[k];
  sprite_y_vel[k] += sprite_z_vel[k];
  Sprite_MoveXY(k);
  sprite_y_vel[k] = old_vel;
  switch(sprite_ai_state[k]) {
  case 0: {  // emerge
    if (!sprite_delay_main[k] && (j = kSwamola_Target_Dir[GetRandomNumber() & 7]) != sprite_D[k]) {
      int t = (sprite_B[k] << 8 | sprite_A[k]) + kSwamola_Target_X[j];
      swamola_target_x_lo[k] = t, swamola_target_x_hi[k] = t >> 8;
      t = (sprite_head_dir[k] << 8 | sprite_C[k]) + kSwamola_Target_Y[j];
      swamola_target_y_lo[k] = t, swamola_target_y_hi[k] = t >> 8;
      sprite_ai_state[k] = 1;
      sprite_x_vel[k] = sprite_y_vel[k] = 0;
      sprite_z_vel[k] = -15;
      Swamola_SpawnRipples(k);
    }
    break;
  }
  case 1:  // ascending
    if (!(sprite_subtype2[k] & 3)) {
      if (!++sprite_z_vel[k])
        sprite_ai_state[k] = 2;
      ProjectSpeedRet pt = Swamola_ProjectVelocityTowardsTarget(k);
      Sprite_ApproachTargetSpeed(k, pt.x, pt.y);
    }
    break;
  case 2: {  // wiggle
    static const int8 kSwamola_Z_Accel[2] = {2, -2};
    static const int8 kSwamola_Z_Vel_Target[2] = {12, -12};
    int j = sprite_G[k] & 1;
    sprite_z_vel[k] += kSwamola_Z_Accel[j];
    if (sprite_z_vel[k] == (uint8)kSwamola_Z_Vel_Target[j])
      sprite_G[k]++;
    uint16 x = swamola_target_x_hi[k] << 8 | swamola_target_x_lo[k];
    uint16 y = swamola_target_y_hi[k] << 8 | swamola_target_y_lo[k];
    if ((uint16)(cur_sprite_x - x + 8) < 16 && (uint16)(cur_sprite_y - y + 8) < 16)
      sprite_ai_state[k] = 3;
    ProjectSpeedRet pt = Swamola_ProjectVelocityTowardsTarget(k);
    sprite_x_vel[k] = pt.x;
    sprite_y_vel[k] = pt.y;
    break;
  }
  case 3:  // descending
    if (!(sprite_subtype2[k] & 3) && ++sprite_z_vel[k] == 16) {
      sprite_ai_state[k] = 4;
      Swamola_SpawnRipples(k);
      sprite_y_hi[k] = 128;
      sprite_delay_main[k] = 80;
    }
    if (!(sprite_subtype2[k] & 3))
      Sprite_ApproachTargetSpeed(k, 0, 0);
    break;
  case 4:  // submerge
    if (!sprite_delay_main[k]) {
      sprite_D[k] = j = kSwamola_Target_Dir[GetRandomNumber() & 7];
      Sprite_SetX(k, (sprite_B[k] << 8 | sprite_A[k]) + kSwamola_Target_X[j]);
      Sprite_SetY(k, (sprite_head_dir[k] << 8 | sprite_C[k]) + kSwamola_Target_Y[j]);
      sprite_ai_state[k] = 0;
      sprite_delay_main[k] = 48;
      sprite_x_vel[k] = sprite_y_vel[k] = 0;
      sprite_z_vel[k] = 0;
    }
    break;
  }
}

ProjectSpeedRet Swamola_ProjectVelocityTowardsTarget(int k) {  // 9d9e13
  uint16 x = swamola_target_x_hi[k] << 8 | swamola_target_x_lo[k];
  uint16 y = swamola_target_y_hi[k] << 8 | swamola_target_y_lo[k];
  return Sprite_ProjectSpeedTowardsLocation(k, x, y, 15);
}

void Swamola_SpawnRipples(int k) {  // 9d9eaa
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0xcf, &info);
  if (j >= 0) {
    Sprite_SetSpawnedCoordinates(j, &info);
    sprite_ai_state[j] = 128;
    sprite_delay_main[j] = 32;
    sprite_oam_flags[j] = 4;
    sprite_ignore_projectile[j] = 4;
    sprite_flags2[j] = 0;
  }
}

void Sprite_Swamola_Ripples(int k) {  // 9d9ece
  SwamolaRipples_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (!sprite_delay_main[k])
    sprite_state[k] = 0;
}

void SwamolaRipples_Draw(int k) {  // 9d9f1d
  static const DrawMultipleData kSwamolaRipples_Dmd[8] = {
    {0, 4, 0x00d8, 0},
    {8, 4, 0x40d8, 0},
    {0, 4, 0x00d9, 0},
    {8, 4, 0x40d9, 0},
    {0, 4, 0x00da, 0},
    {8, 4, 0x40da, 0},
    {0, 4, 0x00d9, 0},
    {8, 4, 0x40d9, 0},
  };
  Oam_AllocateFromRegionB(8);
  Sprite_DrawMultiple(k, &kSwamolaRipples_Dmd[(sprite_delay_main[k] >> 2 & 3) * 2], 2, NULL);
}

void Swamola_Draw(int k) {  // 9d9f64
  static const uint8 kSwamola_Gfx[16] = {7, 6, 5, 4, 3, 4, 5, 6, 7, 6, 5, 4, 3, 4, 5, 6};
  static const uint8 kSwamola_Gfx2[4] = {0, 0, 1, 2};
  static const uint8 kSwamola_Draw_OamFlags[16] = {0xc0, 0xc0, 0xc0, 0xc0, 0x80, 0x80, 0x80, 0x80, 0, 0, 0, 0, 0, 0x40, 0x40, 0x40};
  static const uint8 kSwamola_HistOffs[4] = {8, 16, 22, 26};
  int j = Sprite_ConvertVelocityToAngle(sprite_x_vel[k], sprite_y_vel[k] + sprite_z_vel[k]);
  sprite_graphics[k] = kSwamola_Gfx[j];
  sprite_oam_flags[k] = sprite_oam_flags[k] & 63 | kSwamola_Draw_OamFlags[j];
  SpriteDraw_SingleLarge(k);
  j = (sprite_subtype2[k] & 0x1f) + k * 32;
  swamola_x_lo[j] = sprite_x_lo[k];
  swamola_x_hi[j] = sprite_x_hi[k];
  swamola_y_lo[j] = sprite_y_lo[k];
  swamola_y_hi[j] = sprite_y_hi[k];
  j = sign8(sprite_y_vel[k]) ? 5 : 0;
  int delta = sign8(sprite_y_vel[k]) ? -1 : 1;
  oam_cur_ptr += j * 4, oam_ext_cur_ptr += j;
  for (int i = 0; i < 4; i++) {
    sprite_graphics[k] = kSwamola_Gfx2[i];
    j = (sprite_subtype2[k] - kSwamola_HistOffs[i] & 31) + k * 32;
    cur_sprite_x = swamola_x_hi[j] << 8 | swamola_x_lo[j];
    cur_sprite_y = swamola_y_hi[j] << 8 | swamola_y_lo[j];
    oam_cur_ptr += delta * 4, oam_ext_cur_ptr += delta;
    SpriteDraw_SingleLarge(k);
  }
  byte_7E0FB6 = 4;
}

void SpritePrep_Blind_PrepareBattle(int k) {  // 9da081
  if (follower_indicator != 6 && dung_savegame_state_bits & 0x2000) {
    sprite_delay_aux2[k] = 96;
    sprite_C[k] = 1;
    sprite_D[k] = 2;
    sprite_head_dir[k] = 4;
    sprite_graphics[k] = 7;
    byte_7E0B69 = 0;
  } else {
    sprite_state[k] = 0;
  }
}

void BlindLaser_SpawnTrailGarnish(int j) {  // 9da0b1
  int k = GarnishAllocOverwriteOld();
  garnish_type[k] = 15;
  garnish_active = 15;
  garnish_oam_flags[k] = sprite_graphics[j];
  garnish_sprite[k] = j;
  garnish_x_lo[k] = sprite_x_lo[j];
  garnish_x_hi[k] = sprite_x_hi[j];
  uint16 y = Sprite_GetY(j) + 16;
  garnish_y_lo[k] = y;
  garnish_y_hi[k] = y >> 8;
  garnish_countdown[k] = 10;
}

void Sprite_Blind_Head(int k) {  // 9da118
  static const uint8 kBlindHead_XposLimit[2] = {0x98, 0x58};
  static const uint8 kBlindHead_YposLimit[2] = {0xb0, 0x50};
  static const int8 kBlindHead_YvelLimit[2] = {24, -24};
  static const int8 kBlindHead_XvelLimit[2] = {32, -32};

  sprite_obj_prio[k] |= 48;
  SpriteDraw_SingleLarge(k);
  OamEnt *oam = GetOamCurPtr();
  int j = sprite_head_dir[k];
  oam->charnum = kBlindHead_Draw_Char[j];
  oam->flags = oam->flags & 0x3f | kBlindHead_Draw_Flags[j];

  if (Sprite_ReturnIfInactive(k))
    return;
  if (sprite_F[k] == 14)
    sprite_F[k] = 8;
  if (Sprite_ReturnIfRecoiling(k))
    return;
  if (sign8(--sprite_subtype[k])) {
    sprite_subtype[k] = 2;
    sprite_head_dir[k] = sprite_head_dir[k] + 1 & 15;
  }
  if (sprite_delay_main[k])
    return;
  Sprite_CheckDamageToAndFromLink(k);
  sprite_subtype2[k]++;
  j = Blind_SpitFireball(k, 0x1f);
  if (j >= 0 && sign8(--sprite_z_subpos[k])) {
    sprite_z_subpos[k] = 4;
    ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLink(k, 32);
    sprite_x_vel[j] = pt.x;
    sprite_y_vel[j] = pt.y;
  }
  j = sprite_G[k] & 1;
  if (sprite_x_vel[k] != (uint8)kBlindHead_XvelLimit[j])
    sprite_x_vel[k] += j ? -1 : 1;
  if ((sprite_x_lo[k] & ~1) == kBlindHead_XposLimit[j])
    sprite_G[k]++;
  j = sprite_anim_clock[k] & 1;
  if (sprite_y_vel[k] != (uint8)kBlindHead_YvelLimit[j])
    sprite_y_vel[k] += j ? -1 : 1;
  if ((sprite_y_lo[k] & ~1) == kBlindHead_YposLimit[j])
    sprite_anim_clock[k]++;
  if (!sprite_F[k])
    Sprite_MoveXY(k);
}

void Blind_SpawnHead(int k) {  // 9da1ed
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0xce, &info);
  if (j >= 0) {
    Sprite_SetSpawnedCoordinates(j, &info);
    sprite_flags3[j] = 0x5b;
    sprite_oam_flags[j] = 0x5b & 15;
    sprite_defl_bits[j] = 4;
    sprite_A[j] = 2;
    sprite_flags2[j] = 1;
    sprite_flags4[j] = 0;
    sprite_flags[j] = 0;
    sprite_z[j] = 23;
    sprite_y_lo[j] = 23 + info.r2_y;
    sprite_G[j] = (info.r0_x >> 7) & 1;
    sprite_anim_clock[j] = (info.r2_y >> 7) & 1;
    sprite_delay_main[j] = 48;
  }
}

void Sprite_CE_Blind(int k) {  // 9da263
  if (sign8(sprite_A[k]))
    Sprite_BlindLaser(k);
  else if (sprite_A[k] == 2)
    Sprite_Blind_Head(k);
  else
    Sprite_Blind_Blind_Blind(k);
}

void Sprite_BlindLaser(int k) {  // 9da268
  static const uint8 kBlindLaser_Gfx[16] = {7, 7, 8, 9, 10, 9, 8, 7, 7, 7, 8, 9, 10, 9, 8, 7};
  static const uint8 kBlindLaser_OamFlags[16] = {0, 0, 0, 0, 0, 0x40, 0x40, 0x40, 0x40, 0x40, 0xc0, 0xc0, 0x80, 0x80, 0x80, 0x80};
  int j = sprite_head_dir[k];
  sprite_graphics[k] = kBlindLaser_Gfx[j];
  sprite_oam_flags[k] = kBlindLaser_OamFlags[j] | 3;
  PrepOamCoordsRet info;
  Sprite_PrepOamCoord(k, &info);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (sprite_delay_main[k]) {
    if (sprite_delay_main[k] == 1)
      sprite_state[k] = 0;
    return;
  }
  Sprite_CheckDamageToLink_same_layer(k);
  Sprite_SetX(k, Sprite_GetX(k) + (int8)sprite_x_vel[k]);
  Sprite_SetY(k, Sprite_GetY(k) + (int8)sprite_y_vel[k]);
  if (Sprite_CheckTileCollision(k))
    sprite_delay_main[k] = 12;
  BlindLaser_SpawnTrailGarnish(k);
}

void Sprite_Blind_Blind_Blind(int k) {  // 9da2d2
  int j;

  sprite_obj_prio[k] |= 0x30;
  Blind_Draw(k);
  sprite_oam_flags[k] = 1;
  if (Sprite_ReturnIfInactive(k))
    return;
  uint8 a = sprite_F[k];
  if (a)
    sprite_F[k]--;

  if (a == 11) {
    sprite_hit_timer[k] = 0;
    sprite_wallcoll[k] = 0;
    if (!sprite_delay_aux4[k]) {
      sprite_health[k] = 128;
      sprite_delay_aux4[k] = 48;
      sprite_oam_flags[k] &= 1;
      if (++sprite_z_subpos[k] < 3) {
        sprite_wallcoll[k] = 96;
        sprite_subtype[k] = 1;
      } else {
        sprite_z_subpos[k] = 0;
        if (++sprite_limit_instance == 3) {
          Sprite_KillFriends();
          sprite_state[k] = 4;
          sprite_A[k] = 0;
          sprite_delay_main[k] = 255;
          sprite_hit_timer[k] = 255;
          flag_block_link_menu++;
          SpriteSfx_QueueSfx3WithPan(k, 0x22);
          return;
        }
        sprite_y_vel[k] = sprite_x_vel[k] = 0;
        sprite_C[k] = 6;
        sprite_delay_aux2[k] = 255;
        sprite_ignore_projectile[k] = 255;
        Blind_SpawnHead(k);
      }
    }
  }

  if (sprite_A[k]) {
    static const uint8 kBlind_Gfx0[7] = {20, 19, 18, 17, 16, 15, 15};
    if (!sprite_delay_main[k])
      sprite_state[k] = 0;
    sprite_graphics[k] = kBlind_Gfx0[sprite_delay_main[k] >> 3];
    return;
  }
  if (!(++sprite_subtype2[k] & 1))
    sprite_delay_main[k]++;

  if (sprite_delay_aux1[k]) {
    sprite_ai_state[k] = 0;
    if (sprite_delay_aux1[k] == 8)
      Blind_SpawnLaser(k);
    Blind_CheckBumpDamage(k);
    return;
  }
  byte_7E0B69++;
  if (!sprite_stunned[k]) {
    if (sprite_ai_state[k]) {
      sprite_delay_aux1[k] = 16;
      sprite_stunned[k] = 128;
      sprite_ai_state[k] = 0;
    }
  } else {
    sprite_stunned[k]--;
    sprite_ai_state[k] = 0;
  }
  sprite_x_hi[k] = HIBYTE(link_x_coord);
  sprite_y_hi[k] = HIBYTE(link_y_coord);
  switch(sprite_C[k]) {
  case 0:  // blinded
    BYTE(dma_var6) = 0;
    BYTE(dma_var7) = 0xA0;
    if (sprite_delay_aux2[k] == 0) {
      sprite_C[k]++;
      sprite_delay_aux2[k] = 96;
    } else if (sprite_delay_aux2[k] == 80) {
      dialogue_message_index = 0x123;
      Sprite_ShowMessageMinimal();
    } else if (sprite_delay_aux2[k] == 24) {
      SpawnBossPoof(k);
    }
    break;
  case 1:  // retreat to back wall
    Blind_CheckBumpDamage(k);
    sprite_graphics[k] = 9;
    if (sprite_delay_aux2[k] == 0) {
      sprite_C[k]++;
      sprite_delay_main[k] = 255;
      sprite_ignore_projectile[k] = 0;
    } else if (sprite_delay_aux2[k] < 64) {
      sprite_y_vel[k] = -8;
      Sprite_MoveY(k);
    }
    Blind_Animate(k);
    sprite_head_dir[k] = 4;
    break;
  case 2: { // oscillate
    static const int8 kBlind_Oscillate_YVelTarget[2] = {18, -18};
    static const int8 kBlind_Oscillate_XVelTarget[2] = {24, -24};
    static const uint8 kBlind_Oscillate_XPosTarget[2] = {164, 76};
    Blind_CheckBumpDamage(k);
    Blind_Animate(k);
    if ((!(sprite_subtype2[k] & 127) && Sprite_IsBelowLink(k).a + 2 != sprite_D[k] || sprite_delay_main[k] == 0) && sprite_x_lo[k] < 0x78) {
      sprite_C[k]++;
      sprite_y_vel[k] &= ~1;
      sprite_x_vel[k] &= ~1;
      sprite_delay_aux2[k] = 0x30;
      return;
    }
    j = sprite_B[k] & 1;
    sprite_y_vel[k] += j ? -1 : 1;
    if (sprite_y_vel[k] == (uint8)kBlind_Oscillate_YVelTarget[j])
      sprite_B[k]++;
    j = sprite_G[k] & 1;
    if (sprite_x_vel[k] != (uint8)kBlind_Oscillate_XVelTarget[j])
      sprite_x_vel[k] += j ? -1 : 1;
    if ((sprite_x_lo[k] & ~1) == kBlind_Oscillate_XPosTarget[j])
      sprite_G[k]++;
    Sprite_MoveXY(k);
    if (sprite_wallcoll[k]) {
      Blind_FireballFlurry(k, sprite_wallcoll[k]);
    } else if (!(sprite_subtype2[k] & 7)) {
      Sprite_SpawnProbeAlways(k, sprite_head_dir[k] << 2);
    }
    break;
  }
  case 3:  // switch walls
    Blind_CheckBumpDamage(k);
    if (sprite_delay_aux2[k]) {
      Blind_Decelerate_X(k);
      Sprite_MoveX(k);
      Blind_Decelerate_Y(k);
    } else {
      static const int8 kBlind_SwitchWall_YVelTarget[2] = {64, -64};
      static const uint8 kBlind_SwitchWall_YPosTarget[2] = {0x90, 0x50};
      j = sprite_D[k] - 2;
      if (sprite_y_vel[k] != (uint8)kBlind_SwitchWall_YVelTarget[j])
        sprite_y_vel[k] += j ? -2 : 2;
      if ((sprite_y_lo[k] & ~3) == kBlind_SwitchWall_YPosTarget[j]) {
        sprite_C[k]++;
        sprite_B[k] = sprite_D[k] - 1;
      }
      Sprite_MoveXY(k);
      Blind_Decelerate_X(k);
    }
    break;
  case 4: { // whirl around
    Blind_CheckBumpDamage(k);
    if (!(sprite_subtype2[k] & 7)) {
      static const uint8 kBlind_WhirlAround_Gfx[2] = {0, 9};
      j = sprite_D[k] - 2;
      if (sprite_graphics[k] == kBlind_WhirlAround_Gfx[j]) {
        sprite_delay_main[k] = 254;
        sprite_C[k] = 2;
        sprite_D[k] ^= 1;
        sprite_G[k] = sprite_x_lo[k] >> 7;
      } else {
        sprite_graphics[k] += j ? 1 : -1;
      }
    }
    Blind_Decelerate_Y(k);
    break;
  }
  case 5:  // fireball reprisal
    Blind_FireballFlurry(k, 0x65); // wtf: argument
    break;
  case 6:  // behind the curtain
    sprite_hit_timer[k] = 0;
    sprite_head_dir[k] = 12;
    if (sprite_delay_aux2[k] == 0) {
      sprite_C[k]++;
      sprite_delay_aux2[k] = 39;
      SpriteSfx_QueueSfx1WithPan(k, 0x13);
    } else if (sprite_delay_aux2[k] >= 224) {
      static const uint8 kBlind_Gfx_BehindCurtain[4] = {14, 13, 12, 10};
      sprite_graphics[k] = kBlind_Gfx_BehindCurtain[(sprite_delay_aux2[k] - 224) >> 3];
    } else {
      sprite_graphics[k] = 14;
    }
    break;
  case 7:  // rerobe
    if (sprite_delay_aux2[k] == 0) {
      sprite_C[k] = 2;
      sprite_delay_main[k] = 128;
      sprite_D[k] = (sprite_y_lo[k] >> 7) + 2;
      sprite_G[k] = (sprite_x_lo[k] << 2) | (sprite_x_lo[k] >> 7);
      sprite_x_vel[k] = sprite_y_vel[k] = 0;
      sprite_ignore_projectile[k] = 0;
    } else {
      static const uint8 kBlind_Gfx_Rerobe[5] = {10, 11, 12, 13, 14};
      sprite_graphics[k] = kBlind_Gfx_Rerobe[sprite_delay_aux2[k] >> 3];
    }
    break;
  }


}

void Blind_FireballFlurry(int k, uint8 a) {  // 9da465
  sprite_wallcoll[k]--;
  sprite_oam_flags[k] = (a & 7) * 2 + 1;
  if (sign8(--sprite_E[k])) {
    sprite_E[k] = sprite_subtype[k];
    sprite_head_dir[k] = sprite_head_dir[k] + 1 & 15;
  }
  if (!(sprite_subtype2[k] & 31) && sprite_subtype[k] != 5)
    sprite_subtype[k]++;
  Blind_AnimateRobes(k);
  Blind_SpitFireball(k, 0xf);
}

int Blind_SpitFireball(int k, uint8 a) {  // 9da49d
  static const int8 kBlindHead_SpawnFireball_Xvel[16] = {-32, -28, -24, -16, 0, 16, 24, 28, 32, 28, 24, 16, 0, -16, -24, -28};
  static const int8 kBlindHead_SpawnFireball_Yvel[16] = {0, 16, 24, 28, 32, 28, 24, 16, 0, -16, -24, -28, -32, -28, -24, -16};
  if (sprite_subtype2[k] & a)
    return -1;
  int j = Sprite_SpawnFireball(k);
  if (j >= 0) {
    SpriteSfx_QueueSfx3WithPan(k, 0x19);
    int i = sprite_head_dir[k];
    sprite_x_vel[j] = kBlindHead_SpawnFireball_Xvel[i];
    sprite_y_vel[j] = kBlindHead_SpawnFireball_Yvel[i];
    sprite_defl_bits[j] |= 8;
    sprite_bump_damage[j] = 4;
  }
  return j;
}

int SpawnBossPoof(int k) {  // 9da4f9
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0xce, &info);
  Sprite_SetX(j, info.r0_x + 16);
  Sprite_SetY(j, info.r2_y + 40);
  sprite_graphics[j] = 0xf;
  sprite_A[j] = 1;
  sprite_delay_main[j] = 47;
  sprite_flags2[j] = 9;
  sprite_ignore_projectile[j] = 9;
  sound_effect_1 = 12;
  return j;
}

void Blind_Decelerate_X(int k) {  // 9da647
  if (sprite_x_vel[k] != 0)
    sprite_x_vel[k] += sign8(sprite_x_vel[k]) ? 2 : -2;
  Blind_AnimateRobes(k);
  if (sprite_wallcoll[k])
    Blind_FireballFlurry(k, sprite_wallcoll[k]);
}

void Blind_Decelerate_Y(int k) {  // 9da6a4
  if (sprite_y_vel[k] != 0)
    sprite_y_vel[k] += sign8(sprite_y_vel[k]) ? 4 : -4;
  Sprite_MoveY(k);
  if (sprite_wallcoll[k])
    Blind_FireballFlurry(k, sprite_wallcoll[k]);
}

void Blind_CheckBumpDamage(int k) {  // 9da6c0
  if (!(sprite_delay_aux4[k] | sprite_F[k]))
    Sprite_CheckDamageToAndFromLink(k);
  if ((uint16)(link_x_coord - cur_sprite_x + 14) < 28 &&
      (uint16)(link_y_coord - cur_sprite_y) < 28 &&
      !(countdown_for_blink | link_disable_sprite_damage)) {
    link_auxiliary_state = 1;
    link_give_damage = 8;
    link_incapacitated_timer = 16;
    link_actual_vel_x ^= 255;
    link_actual_vel_y ^= 255;
  }
}

void Blind_Animate(int k) {  // 9da6ef
  static const uint8 kBlind_HeadDir[17] = {0, 1, 2, 3, 4, 3, 2, 1, 0, 15, 14, 13, 12, 13, 14, 15, 0};
  static const uint8 kBlind_Animate_Tab[8] = {0, 1, 1, 2, 2, 3, 3, 4};

  if (!sprite_wallcoll[k]) {
    int t1 = kBlind_Animate_Tab[BYTE(link_x_coord) >> 5];
    t1 = (sprite_D[k] == 3) ? -t1 : t1;
    int t0 = (sprite_D[k] - 2) * 8;
    int idx = (byte_7E0B69 >> 3 & 7) + (byte_7E0B69 >> 2 & 1) + t0;
    sprite_head_dir[k] = (kBlind_HeadDir[idx] + t1) & 15;
  }
  Blind_AnimateRobes(k);
}

void Blind_AnimateRobes(int k) {  // 9da729
  static const uint8 kBlind_Gfx_Animate[8] = {7, 8, 9, 8, 0, 1, 2, 1};
  sprite_graphics[k] = kBlind_Gfx_Animate[(sprite_subtype2[k] >> 3 & 3) + ((sprite_D[k] - 2) << 2)];
}

void Blind_SpawnLaser(int k) {  // 9da765
  static const int8 kBlind_Laser_Xvel[16] = {-8, -8, -8, -4, 0, 4, 8, 8, 8, 8, 8, 4, 0, -4, -8, -8};
  static const int8 kBlind_Laser_Yvel[16] = {0, 0, 4, 8, 8, 8, 4, 0, 0, 0, -4, -8, -8, -8, -4, 0};
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0xce, &info), i;
  if (j >= 0) {
    sound_effect_2 = Sprite_CalculateSfxPan(k) | 0x26;
    Sprite_SetSpawnedCoordinates(j, &info);
    sprite_x_lo[j] = info.r0_x + 4;
    sprite_head_dir[j] = i = sprite_head_dir[k];
    sprite_x_vel[j] = kBlind_Laser_Xvel[i];
    sprite_y_vel[j] = kBlind_Laser_Yvel[i];
    sprite_A[j] = 128;
    sprite_ignore_projectile[j] = 128;
    sprite_flags2[j] = 0x40;
    sprite_flags4[j] = 0x14;
  }
}

void Blind_Draw(int k) {  // 9dac6c

  if (sprite_graphics[k] >= 15) {
    static const DrawMultipleData kBlindPoof_Dmd[37] = {
      {-16, -20, 0x0586, 2},
      {-11, -28, 0x0586, 2},
      {-23, -26, 0x0586, 2},
      { -8, -17, 0x0586, 2},
      {-20, -13, 0x0586, 2},
      {-16, -37, 0x0586, 2},
      {-27, -31, 0x0586, 2},
      {-10, -28, 0x0586, 2},
      { -5, -28, 0x0586, 2},
      {-20, -27, 0x0586, 2},
      {-27, -17, 0x0586, 2},
      { -4, -17, 0x0586, 2},
      {-16, -13, 0x0586, 2},
      {-18, -37, 0x458a, 2},
      { -5, -33, 0x458a, 2},
      {-32, -32, 0x058a, 2},
      {-23, -31, 0x458a, 2},
      {-15, -24, 0x458a, 2},
      {-23, -31, 0x458a, 2},
      {-15, -24, 0x458a, 2},
      {-29, -22, 0x058a, 2},
      { -5, -22, 0x058a, 2},
      {-16, -14, 0x058a, 2},
      {-12, -32, 0x458a, 2},
      {-26, -29, 0x458a, 2},
      { -6, -22, 0x458a, 2},
      {-19, -20, 0x058a, 2},
      {-26, -29, 0x458a, 2},
      { -6, -22, 0x458a, 2},
      {-19, -20, 0x058a, 2},
      {-17, -27, 0x059b, 0},
      {-10, -26, 0x059b, 0},
      {  0, -22, 0x459b, 0},
      {-19, -16, 0x459b, 0},
      { -6, -12, 0x059b, 0},
      {  0,  13, 0x0b20, 2},
      {  0,  23, 0x0b22, 2},
    };
    static const uint8 kOffs[] = { 0, 1, 5, 13, 23, 30, 35, 37 };
    int j = sprite_graphics[k] - 15;
    Sprite_DrawMultiple(k, &kBlindPoof_Dmd[kOffs[j]], kOffs[j + 1] - kOffs[j], NULL);
    return;
  }
  static const DrawMultipleData kBlind_Dmd[105] = {
    { -8,  7, 0x0c8e, 2},
    {  8,  7, 0x4c8e, 2},
    { -8, 23, 0x0ca0, 2},
    {  8, 23, 0x4ca4, 2},
    {  0,  0, 0x0a8c, 2},
    {-19,  3, 0x0aa6, 2},
    { 19,  3, 0x4aa6, 2},
    { -8,  7, 0x0c8e, 2},
    {  8,  7, 0x4c8e, 2},
    { -8, 23, 0x0ca2, 2},
    {  8, 23, 0x4ca0, 2},
    {  0,  0, 0x0a8c, 2},
    {-19,  3, 0x0aa8, 2},
    { 19,  3, 0x4aa8, 2},
    { -8,  7, 0x0c8e, 2},
    {  8,  7, 0x4c8e, 2},
    { -8, 23, 0x0ca4, 2},
    {  8, 23, 0x4ca2, 2},
    {  0,  0, 0x0a8c, 2},
    {-19,  3, 0x0aaa, 2},
    { 19,  3, 0x4aaa, 2},
    {-15,  5, 0x0aa6, 2},
    { -6,  7, 0x0c8e, 2},
    {  6,  7, 0x4c8e, 2},
    { -6, 23, 0x0ca4, 2},
    {  6, 23, 0x4ca0, 2},
    {  0,  0, 0x0a8a, 2},
    { 16, -1, 0x4aa6, 2},
    {-11,  9, 0x0aa6, 2},
    { -4,  7, 0x0c8e, 2},
    {  5,  7, 0x4c8e, 2},
    { -4, 23, 0x0ca4, 2},
    {  5, 23, 0x4ca0, 2},
    {  0,  0, 0x0a88, 2},
    { 10, -2, 0x4aa6, 2},
    {  0,  0, 0x0a84, 2},
    { 13,  8, 0x4aa6, 2},
    {-10, -2, 0x0aa6, 2},
    { -5,  7, 0x0c8e, 2},
    {  5,  7, 0x4c8e, 2},
    { -5, 23, 0x0ca0, 2},
    {  5, 23, 0x4ca4, 2},
    {  0,  0, 0x0a82, 2},
    { 18,  4, 0x4aa6, 2},
    {-15, -1, 0x0aa6, 2},
    { -6,  7, 0x0c8e, 2},
    {  6,  7, 0x4c8e, 2},
    { -6, 23, 0x0ca0, 2},
    {  6, 23, 0x4ca4, 2},
    {  0,  0, 0x0a80, 2},
    {-19,  3, 0x0aa6, 2},
    { 19,  3, 0x4aa6, 2},
    { -8,  7, 0x0c8e, 2},
    {  8,  7, 0x4c8e, 2},
    { -8, 23, 0x0ca0, 2},
    {  8, 23, 0x4ca4, 2},
    {  0,  0, 0x0a80, 2},
    {-19,  3, 0x0aa8, 2},
    { 19,  3, 0x4aa8, 2},
    { -8,  7, 0x0c8e, 2},
    {  8,  7, 0x4c8e, 2},
    { -8, 23, 0x0ca2, 2},
    {  8, 23, 0x4ca0, 2},
    {  0,  0, 0x0a80, 2},
    {-19,  3, 0x0aaa, 2},
    { 19,  3, 0x4aaa, 2},
    { -8,  7, 0x0c8e, 2},
    {  8,  7, 0x4c8e, 2},
    { -8, 23, 0x0ca0, 2},
    {  8, 23, 0x4ca4, 2},
    { -8,  9, 0x0c8e, 2},
    {  8,  9, 0x4c8e, 2},
    { -8, 23, 0x0cae, 2},
    {  8, 23, 0x4cae, 2},
    {  8, 23, 0x4cae, 2},
    {  8, 23, 0x4cae, 2},
    {  0,  2, 0x0a8c, 2},
    { -8, 16, 0x0c8e, 2},
    {  8, 16, 0x4c8e, 2},
    { -8, 23, 0x0cae, 2},
    {  8, 23, 0x4cae, 2},
    {  8, 23, 0x4cae, 2},
    {  8, 23, 0x4cae, 2},
    {  0,  9, 0x0a8c, 2},
    { -8, 23, 0x0cae, 2},
    {  8, 23, 0x4cae, 2},
    {  8, 23, 0x4cae, 2},
    {  8, 23, 0x4cae, 2},
    {  8, 23, 0x4cae, 2},
    {  8, 23, 0x4cae, 2},
    {  0, 16, 0x0a8c, 2},
    { -8, 23, 0x0cac, 2},
    {  8, 23, 0x4cac, 2},
    {  8, 23, 0x4cac, 2},
    {  8, 23, 0x4cac, 2},
    {  8, 23, 0x4cac, 2},
    {  8, 23, 0x4cac, 2},
    {  0, 20, 0x0a8c, 2},
    { -8, 23, 0x0cac, 2},
    {  8, 23, 0x4cac, 2},
    {  8, 23, 0x4cac, 2},
    {  8, 23, 0x4cac, 2},
    {  8, 23, 0x4cac, 2},
    {  8, 23, 0x4cac, 2},
    {  0, 23, 0x0a8c, 2},
  };
  Sprite_DrawMultiple(k, &kBlind_Dmd[sprite_graphics[k] * 7], 7, NULL);
  OamEnt *oam = GetOamCurPtr();
  if (sprite_wallcoll[k] == 0) {
    if (sprite_C[k] == 6) {
      oam[6].y = 0xf0;
      return;
    }
    if (sprite_C[k] == 4)
      return;
  }
  if (sprite_graphics[k] >= 10)
    return;
  static const uint8 kBlind_OamIdx[10] = {4, 4, 4, 5, 5, 0, 0, 0, 0, 0};
  oam += kBlind_OamIdx[sprite_graphics[k]];
  int j = sprite_head_dir[k];
  oam->charnum = kBlindHead_Draw_Char[j];
  oam->flags = oam->flags & 0x3f | kBlindHead_Draw_Flags[j];
}

void TrinexxComponents_Initialize(int k) {  // 9dad16
  switch (sprite_type[k]) {
  case 0xcb: {
    sprite_x_lo[k] += 8;
    sprite_y_lo[k] += 16;
    Trinexx_CachePosition(k);
    overlord_x_lo[2] = 0;
    overlord_x_lo[3] = 0;
    overlord_x_lo[5] = 0;
    overlord_x_lo[7] = 0;
    overlord_x_hi[0] = 0;
    overlord_x_lo[6] = 255;
    Trinexx_RestoreXY(k);
    break;
  }
  case 0xcc:
    sprite_graphics[k] = 3;
    sprite_delay_main[k] = 128;
common:
    for (int j = 0x1a; j >= 0; j--) {
      alt_sprite_type[j] = 0x40;
      alt_sprite_x_hi[j] = 0;
      alt_sprite_y_hi[j] = 0;
    }
    sprite_subtype2[k] = 1;
    Trinexx_CachePosition(k);
    break;
  case 0xcd:
    sprite_delay_main[k] = 255;
    goto common;
  }
}

void Trinexx_RestoreXY(int k) {  // 9dad4f
  sprite_x_lo[k] = sprite_A[k];
  Sprite_SetY(k, (sprite_G[k] << 8) + sprite_C[k] + 12);
}

void Trinexx_CachePosition(int k) {  // 9dad8c
  sprite_A[k] = sprite_x_lo[k];
  sprite_B[k] = sprite_x_hi[k];
  sprite_C[k] = sprite_y_lo[k];
  sprite_G[k] = sprite_y_hi[k];
}

void Sprite_Trinexx_FinalPhase(int k) {  // 9dadb5
  static const uint8 kSprite_TrinexxD_Gfx3[8] = {6, 7, 0, 1, 2, 3, 4, 5};
  static const uint8 kSprite_TrinexxD_Gfx[8] = {7, 7, 1, 1, 3, 3, 5, 5};
  static const int8 kSprite_TrinexxD_Xvel[4] = {0, -31, 0, 31};
  static const int8 kSprite_TrinexxD_Yvel[4] = {31, 0, -31, 0};

  int j = Sprite_ConvertVelocityToAngle(sprite_x_vel[k], sprite_y_vel[k]) >> 1;
  uint8 gfx = kSprite_TrinexxD_Gfx3[j];
  sprite_graphics[k] = sprite_delay_aux1[k] ? kSprite_TrinexxD_Gfx[gfx] : gfx;

  Sprite_TrinexxD_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (sign8(sprite_ai_state[k])) {
    uint8 t = sprite_delay_main[k];
    sprite_hit_timer[k] = t | 0xe0;
    if (t == 0) {
      sprite_delay_main[k] = 12;
      if (sprite_anim_clock[k] == 0) {
        sprite_hit_timer[k] = 255;
        Sprite_ScheduleBossForDeath(k);
      } else {
        sprite_anim_clock[k]--;
        Sprite_MakeBossExplosion(k);
      }
    }
    return;
  }
  if (!(frame_counter & 7))
    SpriteSfx_QueueSfx3WithPan(k, 0x31);

  j = ++sprite_subtype2[k] & 0x7f;
  moldorm_x_lo[j] = sprite_x_lo[k];
  moldorm_y_lo[j] = sprite_y_lo[k];
  moldorm_x_hi[j] = sprite_x_hi[k];
  moldorm_y_hi[j] = sprite_y_hi[k];

  if (sprite_F[k] == 14) {
    sprite_F[k] = 8;
    if (sprite_ai_state[k] == 0)
      sprite_ai_state[k] = 2;
  }
  Sprite_MoveXY(k);
  Sprite_CheckDamageToAndFromLink(k);
  switch(sprite_ai_state[k]) {
  case 0:
    if (!--sprite_A[k]) {
      sprite_ai_state[k] = 1;
      sprite_delay_main[k] = 192;
    }
    Sprite_Get16BitCoords(k);
    if (Sprite_CheckTileCollision(k)) {
      sprite_D[k] = sprite_D[k] + 1 & 3;
      sprite_delay_aux1[k] = 8;
    }
    j = sprite_D[k];
    sprite_x_vel[k] = kSprite_TrinexxD_Xvel[j];
    sprite_y_vel[k] = kSprite_TrinexxD_Yvel[j];
    break;
  case 1:
    if (!(frame_counter & 1)) {
      ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLink(k, 31);
      Sprite_ApproachTargetSpeed(k, pt.x, pt.y);
    }
    break;
  }
}

void Sprite_TrinexxD_Draw(int k) {  // 9daf84
  static const int8 kTrinexxD_HistPos[24] = {
    8,  0xc, 0x10, 0x18, 0x20, 0x28, 0x30, 0x34, 0x38, 0x3c, 0x40, 0x44, 0x48, 0x4c, 0x50, 0x54,
    0x58, 0x5c, 0x60, 0x64, 0x68, 0x6c, 0x70, 0x74,
  };
  static const int8 kSprite_TrinexxD_Gfx2[24] = {
    2, 2, 2, 3, 3, 3, 2, 2, 1, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
  };
  static const int16 kTrinexxD_OamOffs[24] = {
    0x10, 4, 4, 4, 0x10, 0x10, 0x10, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4,    4,    4,    4, 4,
  };

  sprite_obj_prio[k] |= 0x30;
  PrepOamCoordsRet info;
  SpriteDraw_TrinexxRockHead(k, &info);
  for (int i = 0; i != sprite_anim_clock[k]; i++) {
    int j = sprite_subtype2[k] - kTrinexxD_HistPos[i] & 0x7f;
    cur_sprite_x = moldorm_x_hi[j] << 8 | moldorm_x_lo[j];
    cur_sprite_y = moldorm_y_hi[j] << 8 | moldorm_y_lo[j];

    if ((uint16)(link_x_coord - cur_sprite_x + 8) < 16 &&
        (uint16)(link_y_coord - cur_sprite_y + 16) < 16 &&
        !sign8(sprite_ai_state[k]) &&
        !(countdown_for_blink | link_disable_sprite_damage | submodule_index | flag_unk1)) {
      link_auxiliary_state = 1;
      link_give_damage = 8;
      link_incapacitated_timer = 16;
      link_actual_vel_x ^= 255;
      link_actual_vel_y ^= 255;
    }
    oam_cur_ptr += kTrinexxD_OamOffs[i];
    oam_ext_cur_ptr += (kTrinexxD_OamOffs[i] >> 2);
    sprite_oam_flags[k] = 1;
    if (i == 4 && sprite_ai_state[k] != 0) {
      Sprite_Trinexx_CheckDamageToFlashingSegment(k);
      sprite_oam_flags[k] = (sprite_subtype2[k] & 6) ^ sprite_oam_flags[k];
    }
    sprite_graphics[k] = kSprite_TrinexxD_Gfx2[i];
    if (sprite_graphics[k] != 3) {
      SpriteDraw_SingleLarge(k);
    } else {
      sprite_graphics[k] = 8;
      SpriteDraw_TrinexxRockHead(k, &info);
    }
  }
  byte_7E0FB6 = sprite_anim_clock[k];
}

void Sprite_Trinexx_CheckDamageToFlashingSegment(int k) {  // 9db079
  uint16 old_x = Sprite_GetX(k);
  uint16 old_y = Sprite_GetY(k);
  Sprite_SetX(k, cur_sprite_x);
  Sprite_SetY(k, cur_sprite_y);
  sprite_defl_bits[k] = 0x80;
  sprite_flags3[k] = 0;
  Sprite_CheckDamageFromLink(k);
  sprite_defl_bits[k] = 0x84;
  sprite_flags3[k] = 0x40;
  Sprite_SetX(k, old_x);
  Sprite_SetY(k, old_y);
}

void Sprite_CB_TrinexxRockHead(int k) {  // 9db0ca
  if (overlord_x_hi[0]) {
    Sprite_Trinexx_FinalPhase(k);
    return;
  }
  TM_copy = 0x17;
  TS_copy = 0;
  SpriteDraw_TrinexxRockHeadAndBody(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (sign8(sprite_ai_state[k])) {
    flag_block_link_menu = sprite_ai_state[k];
    if (!sprite_delay_main[k]) {
      overlord_x_hi[0]++;
      Sprite_InitializedSegmented(k);
      sprite_subtype2[k] = 0;
      sprite_head_dir[k] = 0;
      sprite_flags3[k] &= ~0x40;
      sprite_defl_bits[k] = 0x80;
      sprite_ai_state[k] = 0;
      sprite_D[k] = 0;
      sprite_A[k] = 0;
      sprite_anim_clock[k] = 16;
      sprite_x_vel[k] = sprite_y_vel[k] = 0;
      sprite_A[k] = 128;
      HIBYTE(dung_floor_y_vel) = 255;
    } else if (sprite_delay_main[k] >= 0xff) {
    } else if (sprite_delay_main[k] >= 0xe0) {
      if (!(sprite_delay_main[k] & 3)) {
        dung_floor_y_vel = -1;
        dung_hdr_collision_2_mirror = 1;
      }
      sprite_y_vel[k] = -8;
      Sprite_MoveY(k);
      Trinexx_CachePosition(k);
      sprite_C[k] = sprite_y_lo[k] - 12;
      overlord_x_lo[7] += 2;
    } else {
      if (!(sprite_delay_main[k] & 3))
        SpriteSfx_QueueSfx2WithPan(k, 0xc);
      if (!(sprite_delay_main[k] & 1)) {
        static const int8 kTrinexx_X0[8] = {0, 8, 16, 24, -24, -16, -8, 0};
        static const int8 kTrinexx_Y0[8] = {0, 8, 16, 24, -24, -16, -8, 0};
        cur_sprite_x = Sprite_GetX(k) + kTrinexx_X0[GetRandomNumber() & 7];
        cur_sprite_y = Sprite_GetY(k) + kTrinexx_Y0[GetRandomNumber() & 7] - 8;
        Sprite_MakeBossDeathExplosion_NoSound(k);
      }
      sprite_head_dir[k] = 255;
    }
    return;
  }
  if ((sprite_state[1] | sprite_state[2]) == 0 && sprite_ai_state[k] < 2) {
    sprite_delay_main[k] = 255;
    sprite_ai_state[k] = 255;
    sound_effect_2 = 0x22;
    return;
  }

  Trinexx_WagTail(k);
  Trinexx_HandleShellCollision(k);
  Sprite_CheckDamageToAndFromLink(k);
  if (!(frame_counter & 63)) {
    PairU8 pair = Sprite_IsRightOfLink(k);
    sprite_graphics[k] = (uint8)(pair.b + 24) < 48 ? 0 : pair.a ? 1 : 7;
  }
  if (overlord_x_lo[6]) {
    if (!(frame_counter & 1))
      overlord_x_lo[6]--;
    return;
  }
  if (sprite_state[1] && sprite_ai_state[1] == 3)
    return;
  if (sprite_state[2] && sprite_ai_state[2] == 3)
    return;

  switch (sprite_ai_state[k]) {
  case 0:
    if (!sprite_delay_main[k]) {
      int j = GetRandomNumber() & 3;
      if ((sprite_subtype[k] & 0x7f) == j)
        return;
      if (++sprite_anim_clock[k] == 2) {
        sprite_anim_clock[k] = 0;
        sprite_ai_state[k] = 2;
        sprite_delay_main[k] = 80;
        return;
      }
      static const uint8 kTrinexx_Tab0[4] = {0x60, 0x78, 0x78, 0x90};
      static const uint8 kTrinexx_Tab1[4] = {0x80, 0x70, 0x60, 0x80};
      overlord_x_lo[0] = kTrinexx_Tab0[j];
      overlord_x_lo[1] = kTrinexx_Tab1[j];
      sprite_subtype[k] = j + ((GetRandomNumber() & 3) == 0) * 0x80;
      sprite_ai_state[k] = 1;
    }
    break;
  case 1: {
    if (sprite_subtype[k] == 0xff && (sprite_delay_main[k] == 0 || Sprite_IsBelowLink(k).a == 0)) {
      sprite_subtype[k] = 0;
      sprite_ai_state[k] = 0;
      sprite_delay_main[k] = 48;
    } else {
      uint16 x = sprite_x_hi[k] << 8 | overlord_x_lo[0];
      uint16 y = sprite_y_hi[k] << 8 | overlord_x_lo[1];
      ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLocation(k, x, y, sign8(sprite_subtype[k]) ? 16 : 8);
      sprite_x_vel[k] = pt.x;
      sprite_y_vel[k] = pt.y;

      uint8 bakx = sprite_x_lo[k];
      uint8 baky = sprite_y_lo[k];
      Sprite_MoveXY(k);
      dung_floor_y_vel = (int8)(baky - sprite_y_lo[k]);
      dung_floor_x_vel = (int8)(bakx - sprite_x_lo[k]);

      dung_hdr_collision_2_mirror = 1;
      Trinexx_CachePosition(k);
      sprite_C[k] = sprite_y_lo[k] - 12;
      if ((uint8)(overlord_x_lo[0] - sprite_x_lo[k] + 2) < 4 &&
          (uint8)(overlord_x_lo[1] - sprite_y_lo[k] + 2) < 4) {
        sprite_ai_state[k] = 0;
        sprite_delay_main[k] = 48;
      }
    }
    int i = sign8(sprite_subtype[k]) ? 2 : 1;
    do {
      sprite_subtype2[k] += sign8(sprite_x_vel[k]) ? 1 : -1;
      if (!(sprite_subtype2[k] & 0xf))
        SpriteSfx_QueueSfx2WithPan(k, 0x21);
    } while (--i);
    break;
  }
  case 2:
    Trinexx_WagTail(k);
    Trinexx_WagTail(k);
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k] = 3;
      Sprite_ApplySpeedTowardsLink(k, 48);
      sprite_delay_main[k] = 64;
      sound_effect_2 = 0x26;
    }
    break;
  case 3:
    Sprite_MoveXY(k);
    if (!sprite_delay_main[k]) {
      Trinexx_RestoreXY(k);
      sprite_ai_state[k] = 0;
      sprite_delay_main[k] = 48;
    } else if (sprite_delay_main[k] == 0x20) {
      sprite_x_vel[k] = -sprite_x_vel[k];
      sprite_y_vel[k] = -sprite_y_vel[k];
    }
    break;
  }
}

void Trinexx_WagTail(int k) {  // 9db3b5
  if (!overlord_x_lo[5]) {
    if (!(++overlord_x_lo[4] & 3)) {
      int j = overlord_x_lo[3] & 1;
      overlord_x_lo[2] += j ? -1 : 1;
      if (overlord_x_lo[2] == (j ? 0 : 6)) {
        overlord_x_lo[3] += 1;
        overlord_x_lo[5] = 8;
      }
    }
  } else {
    --overlord_x_lo[5];
  }
}

void Trinexx_HandleShellCollision(int k) {  // 9db3e6
  uint16 x = sprite_A[k] | sprite_B[k] << 8;
  uint16 y = sprite_C[k] | sprite_G[k] << 8;
  if ((uint16)(x - link_x_coord + 40) < 80 && (uint16)(y - link_y_coord + 16) < 64 && !(countdown_for_blink | link_disable_sprite_damage)) {
    link_auxiliary_state = 1;
    link_give_damage = 8;
    link_incapacitated_timer = 16;
    ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLink(k, 32);
    link_actual_vel_x = pt.x;
    link_actual_vel_y = pt.y;
  }
}

void SpriteDraw_TrinexxRockHead(int k, PrepOamCoordsRet *info) {  // 9db560
  static const DrawMultipleData kTrinexx_Draw1_Dmd[36] = {
    {-8, -8, 0x40c0, 2},
    { 8, -8, 0x00c0, 2},
    {-8,  8, 0x40e0, 2},
    { 8,  8, 0x00e0, 2},

    {-8, -8, 0x0000, 2},
    { 8, -8, 0x0002, 2},
    {-8,  8, 0x0020, 2},
    { 8,  8, 0x0022, 2},

    {-8, -8, 0x00c2, 2},
    { 8, -8, 0x00c4, 2},
    {-8,  8, 0x80c2, 2},
    { 8,  8, 0x80c4, 2},

    {-8, -8, 0x8020, 2},
    { 8, -8, 0x8022, 2},
    {-8,  8, 0x8000, 2},
    { 8,  8, 0x8002, 2},
    {-8, -8, 0xc0e0, 2},
    { 8, -8, 0x80e0, 2},
    {-8,  8, 0xc0c0, 2},
    { 8,  8, 0x80c0, 2},
    {-8, -8, 0xc022, 2},
    { 8, -8, 0xc020, 2},
    {-8,  8, 0xc002, 2},
    { 8,  8, 0xc000, 2},
    {-8, -8, 0x40c4, 2},
    { 8, -8, 0x40c2, 2},
    {-8,  8, 0xc0c4, 2},
    { 8,  8, 0xc0c2, 2},
    {-8, -8, 0x4002, 2},
    { 8, -8, 0x4000, 2},
    {-8,  8, 0x4022, 2},
    { 8,  8, 0x4020, 2},
    {-8, -8, 0x0026, 2},
    { 8, -8, 0x4026, 2},
    {-8,  8, 0x8026, 2},
    { 8,  8, 0xc026, 2},
  };
  if (!sign8(sprite_ai_state[k]))
    sprite_obj_prio[k] |= 0x30;
  Sprite_DrawMultiple(k, &kTrinexx_Draw1_Dmd[sprite_graphics[k] * 4], 4, info);
}

void SpriteDraw_TrinexxRockHeadAndBody(int k) {  // 9db587
  static const int8 kTrinexx_Draw_X[35] = {
    0,   3,   9, 16, 24,  0,  2,  7,  13, 20,  0,  1,   4,   9, 15,  0,
    0,   0,   0,  0,  0, -1, -4, -9, -15,  0, -2, -7, -13, -20,  0, -3,
    -9, -16, -24,
  };
  static const int8 kTrinexx_Draw_Y[35] = {
    0x18, 0x20, 0x25, 0x25, 0x21, 0x18, 0x20, 0x27, 0x2a, 0x2c, 0x18, 0x20, 0x28, 0x2f, 0x34, 0x18,
    0x21, 0x2a, 0x34, 0x3d, 0x18, 0x20, 0x28, 0x2f, 0x34, 0x18, 0x20, 0x27, 0x2a, 0x2c, 0x18, 0x20,
    0x25, 0x25, 0x21,
  };
  static const uint8 kTrinexx_Draw_Char[5] = {6, 0x28, 0x28, 0x2c, 0x2c};
  static const uint8 kTrinexx_Mults[8] = {0xfc, 0xe0, 0xc0, 0xa0, 0x80, 0x60, 0x40, 0x20};
  static const int8 kTrinexx_Draw_Xoffs[16] = {0, 2, 3, 4, 4, 4, 3, 2, 0, -2, -3, -4, -4, -4, -3, -2};
  static const int8 kTrinexx_Draw_Yoffs[16] = {-4, -4, -3, -2, 0, 2, 3, 4, 4, 4, 3, 2, 0, -2, -3, -4};
  if (sign8(sprite_head_dir[k]))
    return;

  PrepOamCoordsRet info;
  SpriteDraw_TrinexxRockHead(k, &info);

  info.flags &= ~0x10;

  if (sprite_ai_state[k] == 3) {
    OamEnt *oam = GetOamCurPtr() + 4;
    uint8 xb = sprite_A[k] - sprite_x_lo[k];
    uint8 yb = sprite_C[k] - sprite_y_lo[k];
    for (int i = 7; i >= 0; i--, oam++) {
      SetOamPlain(oam, info.x + TrinexxMult(xb, kTrinexx_Mults[i]), info.y + TrinexxMult(yb, kTrinexx_Mults[i]), 0x28, info.flags, 2);
    }
    byte_7E0FB6 = 0x30;
  }
  oam_cur_ptr = 0x9f0;
  oam_ext_cur_ptr = 0xa9c;
  OamEnt *oam = GetOamCurPtr();
  uint8 xb = sprite_A[k] - BG2HOFS_copy2;
  uint16 yb = (sprite_C[k] | sprite_y_hi[k] << 8) - BG2VOFS_copy2;

  uint8 xidx = (uint8)(sprite_x_vel[k] + 3) < 7 ? 0 : (sprite_subtype2[k] >> 2);
  uint8 yidx = sprite_subtype2[k] >> 2;

  for (int i = 1; i >= 0; i--, oam += 2) {
    uint8 x = xb + (i ? -28 : 28) + kTrinexx_Draw_Xoffs[xidx + (1 - i) * 8 & 0xf];
    uint8 y = yb - 8 + kTrinexx_Draw_Yoffs[yidx + i * 8 & 0xf];
    uint8 f = info.flags | (i ? 0 : 0x40);
    SetOamPlain(oam + 0, x, y, 0xc, f, 2);
    SetOamPlain(oam + 1, x, y + 16, 0x2a, f, 2);
  }

  oam = (OamEnt *)&g_ram[0x800] + 91;
  int g = overlord_x_lo[2];
  for (int i = 0; i < 5; i++, oam++) {
    int j = g * 5 + i;
    SetOamHelper0(oam,
                  xb + kTrinexx_Draw_X[j], yb - kTrinexx_Draw_Y[j] - 0x20 + WORD(overlord_x_lo[7]),
                  kTrinexx_Draw_Char[i], info.flags, 2);
  }
  tmp_counter = 0xff;

  if (submodule_index)
    Sprite_CorrectOamEntries(k, 3, 2);
}

void Sprite_Sidenexx(int k) {  // 9db8a7
  static const int8 kTrinexxHead_Xoffs[2] = {-14, 13};

  int xx = (sprite_B[0] << 8 | sprite_A[0]) + kTrinexxHead_Xoffs[sprite_type[k] - 0xcc];
  sprite_A[k] = xx, sprite_B[k] = xx >> 8;

  int yy = (sprite_G[0] << 8 | sprite_C[0]) - 0x20;
  sprite_C[k] = yy, sprite_G[k] = yy >> 8;
  sprite_obj_prio[k] |= 0x30;
  TrinexxHead_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (sign8(sprite_ai_state[k])) {
    sprite_ignore_projectile[k] = sprite_ai_state[k];
    Sidenexx_Explode(k);
    return;
  }

  if (sprite_hit_timer[k] && sprite_ai_state[k] != 4) {
    sprite_hit_timer[k] = 0;
    sprite_delay_main[k] = 128;
    sprite_ai_state[k] = 4;
    sprite_z_vel[k] = sprite_oam_flags[k];
    sprite_oam_flags[k] = 3;
  }
  Sprite_CheckDamageToAndFromLink(k);
  sprite_defl_bits[k] |= 4;
  switch(sprite_ai_state[k]) {
  case 0:
    sprite_flags3[k] |= 0x40;
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k] = 2;
      sprite_subtype2[k] = 9;
      sprite_flags3[k] &= ~0x40;
    }
    break;
  case 1:
    if (!sprite_delay_main[k]) {
      int i = (GetRandomNumber() & 7) + 1;
      int j = sprite_D[k];
      if (i < 5 && sprite_D[k] != i) {
        sprite_D[k] = i;
        sprite_ai_state[k] = 2;
        if (j == 1 && !(GetRandomNumber() & 1) && sprite_ai_state[0] < 2) {
          sprite_graphics[k] = 0;
          sprite_ai_state[k] = 3;
          sprite_delay_main[k] = 127;
        }
      }
    }
    break;
  case 2: {
    static const uint8 kTrinexxHead_Target0[45] = {
      0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x58, 0x64, 0x6a, 0x6f, 0x74, 0x7a, 0x7e,
      0x80, 0x80, 0x39, 0x48, 0x52, 0x5c, 0x65, 0x73, 0x77, 0x7a, 0x80, 0x1e, 0x24, 0x29, 0x2e, 0x34,
      0x3a, 0x44, 0x4d, 0x80,  0xa, 0x11, 0x17, 0x1c, 0x22, 0x2a, 0x36, 0x3a, 0x80,
    };
    static const uint8 kTrinexxHead_Target1[45] = {
      0x30, 0x28, 0x23, 0x1e, 0x19, 0x13,  0xc,    6,    0, 0x2f, 0x26, 0x21, 0x1d, 0x18, 0x12,  0xc,
      6,    0, 0x2f, 0x27, 0x22, 0x1d, 0x18, 0x12,  0xc,    6,    0, 0x2f, 0x27, 0x22, 0x1d, 0x18,
      0x12,  0xc,    6,    0, 0x48, 0x3a, 0x32, 0x29, 0x22, 0x19, 0x10,    7,    0,
    };

    int j = sprite_D[k] * 9, f = k * 9;
    int n = 0;
    for (int i = 8; i >= 0; i--, j++, f++) {
      if (alt_sprite_type[f] - kTrinexxHead_Target0[j]) {
        alt_sprite_type[f] += sign8(alt_sprite_type[f] - kTrinexxHead_Target0[j]) ? 1 : -1;
        n++;
      }
      if (alt_sprite_type[f] - kTrinexxHead_Target0[j]) {
        alt_sprite_type[f] += sign8(alt_sprite_type[f] - kTrinexxHead_Target0[j]) ? 1 : -1;
        n++;
      }
      if (alt_sprite_y_hi[f] - kTrinexxHead_Target1[j]) {
        alt_sprite_y_hi[f] += sign8(alt_sprite_y_hi[f] - kTrinexxHead_Target1[j]) ? 1 : -1;
        n++;
      }
    }
    if (n == 0) {
      sprite_ai_state[k] = 1;
      sprite_delay_main[k] = GetRandomNumber() & 15;
    }
    break;
  }
  case 3: {
    static const uint8 kTrinexxHead_FrameMask[8] = {1, 1, 3, 3, 7, 0xf, 0x1f, 0x1f};
    int j = sprite_delay_main[k];
    if (!j) {
      sprite_ai_state[k] = 0;
      sprite_delay_main[k] = 32;
      return;
    }
    if (j == 64)
      Sidenexx_ExhaleDanger(k);
    sprite_subtype[k] = (j < 8) ? j : (j < 121) ? 8 : ~(sprite_delay_main[k] + 0x80);
    if (j >= 64 && !(frame_counter & kTrinexxHead_FrameMask[(j - 64) >> 3])) {
      int x = (GetRandomNumber() & 0xf) - 3;
      int y = (GetRandomNumber() & 0xf) + 12;
      int j = Sprite_GarnishSpawn_Sparkle(k, x, y);
      if (sprite_type[k] == 0xcc)
        garnish_type[j] = 0xe;
    }
    break;
  }
  case 4: {
    sprite_defl_bits[k] &= ~4;
    sprite_subtype[k] = 0;
    int j = sprite_delay_main[k];
    if (!j) {
      sprite_ai_state[k] = 1;
      sprite_delay_main[k] = 32;
      sprite_oam_flags[k] = sprite_z_vel[k];
      sprite_hit_timer[k] = 0;
    }
    if (j >= 15) {
      if (j >= 63 && j < 78) {
        if (sprite_type[k] == 0xcd)
          Trinexx_FlashShellPalette_Blue();
        else
          Trinexx_FlashShellPalette_Red();
      }
    } else {
      if (sprite_type[k] == 0xcd)
        Trinexx_UnflashShellPalette_Blue();
      else
        Trinexx_UnflashShellPalette_Red();
    }
  }
  }
}

void Sidenexx_ExhaleDanger(int k) {  // 9dbae8
  SpriteSpawnInfo info;
  if (sprite_type[k] == 0xcd) {
    for (int i = 0; i < 2; i++) {
      int j = Sprite_SpawnDynamically(k, 0xcd, &info);
      if (j >= 0) {
        Sprite_SetSpawnedCoordinates(j, &info);
        sprite_C[j] = i ? 1 : -2;
        SpriteSfx_QueueSfx3WithPan(k, 0x19);
        sprite_ignore_projectile[j] = sprite_E[j] = 1;
        sprite_y_vel[j] = 24;
        sprite_flags2[j] = 0;
        sprite_flags3[j] = 0x40;
      }
    }
    byte_7E0FB6 = 1;
  } else {
    int j = Sprite_SpawnDynamically(k, sprite_type[k], &info);
    if (j >= 0) {
      Sprite_SetSpawnedCoordinates(j, &info);
      SpriteSfx_QueueSfx2WithPan(k, 0x2a);
      sprite_ignore_projectile[j] = sprite_E[j] = 1;
      sprite_y_vel[j] = 24;
      sprite_flags2[j] = 0;
      sprite_flags3[j] = 0x40;
    }
  }
}

void Sidenexx_Explode(int k) {  // 9dbb3f
  if (!sprite_delay_main[k]) {
    sprite_delay_main[k] = 12;
    if (sprite_subtype2[k] == 1)
      sprite_state[k] = 0;
    sprite_subtype2[k]--;
    BYTE(cur_sprite_x) += BG2HOFS_copy2;
    BYTE(cur_sprite_y) += BG2VOFS_copy2;
    Sprite_MakeBossExplosion(k);
  }
}

void TrinexxHead_Draw(int k) {  // 9dbb70
  sprite_x_lo[k] = sprite_A[k];
  sprite_x_hi[k] = sprite_B[k];
  sprite_y_lo[k] = sprite_C[k];
  sprite_y_hi[k] = sprite_G[k];
  Sprite_Get16BitCoords(k);
  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  OamEnt *oam = GetOamCurPtr();
  int i = 0;
  do {
    int j = i + k * 9;

    uint16 r6 = (k != 2) ? 0x100 + (uint8)(-alt_sprite_type[j]) : alt_sprite_type[j];
    uint8 r15 = alt_sprite_y_hi[j];

    BYTE(dungmap_var7) = TrinexxHeadSin(r6, r15);
    HIBYTE(dungmap_var7) = TrinexxHeadSin(r6 + 0x80, r15);

    if (i == 0) {
      static const int8 kTrinexxHead_FirstPart_X[5] = {-8, 8, -8, 8, 0};
      static const int8 kTrinexxHead_FirstPart_Y[5] = {-8, -8, 8, 8, 2};
      static const uint8 kTrinexxHead_FirstPart_Char[5] = {4, 4, 0x24, 0x24, 0xa};
      static const uint8 kTrinexxHead_FirstPart_Flags[5] = {0x40, 0, 0x40, 0, 0};

      for (int m = 0; m < 5; m++) {
        BYTE(cur_sprite_x) = info.x + BYTE(dungmap_var7);
        uint8 x = BYTE(cur_sprite_x) + kTrinexxHead_FirstPart_X[m];

        BYTE(cur_sprite_y) = info.y + HIBYTE(dungmap_var7);
        uint8 y = BYTE(cur_sprite_y) + kTrinexxHead_FirstPart_Y[m] + (m == 4 ? sprite_subtype[k] : 0);

        SetOamPlain(oam, x, y, kTrinexxHead_FirstPart_Char[m], info.flags | kTrinexxHead_FirstPart_Flags[m], 2);
        oam++;
      }
      Sprite_SetX(k, (sprite_B[k] << 8 | sprite_A[k]) + (int8)BYTE(dungmap_var7));
      Sprite_SetY(k, (sprite_G[k] << 8 | sprite_C[k]) + (int8)HIBYTE(dungmap_var7));
    } else {
      uint8 x = info.x + BYTE(dungmap_var7);
      uint8 y = info.y + HIBYTE(dungmap_var7);
      BYTE(cur_sprite_x) = x;
      BYTE(cur_sprite_y) = y;
      SetOamPlain(oam, x, y, 8, info.flags, 2);
      oam++;
    }
  } while (++i != sprite_subtype2[k]);

  tmp_counter = i;
  byte_7E0FB6 = i * 4 + 16;
  if (submodule_index)
    Sprite_CorrectOamEntries(k, 4, 2);
}

void Sprite_CC_CD_Common(int k) {  // 9dbd44
  if (!(frame_counter & 3)) {
    int m = Sprite_IsRightOfLink(k).a ? -1 : 1;
    if (sprite_x_vel[k] != (uint8)(m * 16))
      sprite_x_vel[k] += m;
  }
  if (Sprite_CheckTileCollision(k))
    sprite_state[k] = 0;
}

void Sprite_CD_SpawnGarnish(int k) {  // 9dbd65
  if (++sprite_subtype2[k] & 7)
    return;
  SpriteSfx_QueueSfx3WithPan(k, 0x14);
  int j = GarnishAllocOverwriteOld();
  garnish_active = garnish_type[j] = 0xc;
  garnish_sprite[j] = k;
  Garnish_SetX(j, Sprite_GetX(k));
  Garnish_SetY(j, Sprite_GetY(k) + 16);
  garnish_countdown[j] = 127;
}

void Sprite_TrinexxFire_AddFireGarnish(int k) {  // 9dbdd6
  if (++sprite_subtype2[k] & 7)
    return;
  SpriteSfx_QueueSfx2WithPan(k, 0x2a);
  Garnish_FlameTrail(k, false);
}

int Garnish_FlameTrail(int k, bool is_low) {  // 9dbde8
  int j = is_low ? GarnishAllocOverwriteOldLow() : GarnishAllocOverwriteOld();
  garnish_active = garnish_type[j] = 0x10;
  garnish_sprite[j] = k;
  Garnish_SetX(j, Sprite_GetX(k));
  Garnish_SetY(j, Sprite_GetY(k) + 16);
  garnish_countdown[j] = 127;
  return j;
}

void Sprite_CA_ChainChomp(int k) {  // 9dbe7d
  ChainChomp_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_CheckDamageToAndFromLink(k);
  ChainChomp_HandleLeash(k);
  if (!((k ^ frame_counter) & 3) && (sprite_x_vel[k] | sprite_y_vel[k]))
    sprite_D[k] = Sprite_ConvertVelocityToAngle(sprite_x_vel[k], sprite_y_vel[k]) & 0xf;
  Sprite_MoveXYZ(k);
  sprite_z_vel[k] -= 2;
  if (sign8(sprite_z[k])) {
    sprite_z[k] = 0;
    sprite_z_vel[k] = 0;
  }
  Sprite_Get16BitCoords(k);
  uint16 x = sprite_A[k] | sprite_B[k] << 8;
  uint16 y = sprite_C[k] | sprite_G[k] << 8;
  sprite_anim_clock[k] = (uint16)(cur_sprite_x - x + 48) < 96 && (uint16)(cur_sprite_y - y + 48) < 96;
  switch(sprite_ai_state[k]) {
  case 0:  //
    if (!sprite_delay_main[k]) {
      static const int8 kChainChomp_Xvel[16] = {0, 8, 11, 14, 16, 14, 11, 8, 0, -8, -11, -14, -16, -14, -11, -8};
      static const int8 kChainChomp_Yvel[16] = {-16, -14, -11, -8, 0, 8, 11, 14, 16, 14, 11, 8, 0, -9, -11, -14};
      if (++sprite_subtype2[k] == 4) {
        sprite_subtype2[k] = 0;
        sprite_ai_state[k] = 2;
        int j = GetRandomNumber() & 15;
        sprite_x_vel[k] = kChainChomp_Xvel[j] << 2;
        sprite_y_vel[k] = kChainChomp_Yvel[j] << 2;
        GetRandomNumber();
        Sprite_ApplySpeedTowardsLink(k, 64);
        SpriteSfx_QueueSfx3WithPan(k, 0x4);
      } else {
        sprite_delay_main[k] = (GetRandomNumber() & 31) + 16;
        int j = GetRandomNumber() & 15;
        sprite_x_vel[k] = kChainChomp_Xvel[j];
        sprite_y_vel[k] = kChainChomp_Yvel[j];
        sprite_ai_state[k] = 1;
      }
    } else {
      sprite_x_vel[k] = 0;
      sprite_y_vel[k] = 0;
    }
    break;
  case 1:  //
    if (!sprite_delay_main[k]) {
      sprite_delay_main[k] = 32;
      sprite_ai_state[k] = 0;
    }
    if (!(sprite_delay_main[k] & 15))
      ChainChomp_MoveChain(k);
    if (!sprite_z[k])
      sprite_z_vel[k] = 16;
    if (!sprite_anim_clock[k]) {
      uint16 x = sprite_A[k] | sprite_B[k] << 8;
      uint16 y = sprite_C[k] | sprite_G[k] << 8;
      ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLocation(k, x, y, 16);
      sprite_x_vel[k] = pt.x;
      sprite_y_vel[k] = pt.y;
      Sprite_MoveXY(k);
      sprite_delay_main[k] = 12;
    }
    break;
  case 2:  //
    if (!sprite_anim_clock[k]) {
      sprite_x_vel[k] = -sprite_x_vel[k];
      sprite_y_vel[k] = -sprite_y_vel[k];
      Sprite_MoveXY(k);
      sprite_x_vel[k] = sprite_y_vel[k] = 0;
      sprite_ai_state[k] = 3;
      sprite_delay_aux1[k] = 48;
    }
    ChainChomp_MoveChain(k);
    ChainChomp_MoveChain(k);
    break;
  case 3:  //
    if (!sprite_delay_aux1[k]) {
      sprite_ai_state[k] = 0;
      sprite_delay_main[k] = 48;
    }
    ChainChomp_MoveChain(k);
    ChainChomp_MoveChain(k);
    break;
  }

}

void ChainChomp_MoveChain(int k) {  // 9dc02a
  static const uint8 kChainChomp_Muls[6] = {205, 154, 102, 51, 8, 0xbd}; // wtf
  uint16 x = sprite_A[k] | sprite_B[k] << 8;
  uint16 y = sprite_C[k] | sprite_G[k] << 8;
  int pos = k * 8;
  uint16 x2 = chainchomp_x_hist[pos] - x;
  uint16 y2 = chainchomp_y_hist[pos] - y;
  pos++;
  for (int i = 5; i >= 0; i--) {
    uint16 x3 = x + ChainChomp_OneMult(x2, kChainChomp_Muls[(pos & 7) - 1]);
    uint16 y3 = y + ChainChomp_OneMult(y2, kChainChomp_Muls[(pos & 7) - 1]);
    if (chainchomp_x_hist[pos] - x3)
      chainchomp_x_hist[pos] += sign16(chainchomp_x_hist[pos] - x3) ? 1 : -1;
    if (chainchomp_y_hist[pos] - y3)
      chainchomp_y_hist[pos] += sign16(chainchomp_y_hist[pos] - y3) ? 1 : -1;
    pos++;
  }
}

void ChainChomp_HandleLeash(int k) {  // 9dc0f2
  int pos = k * 8;
  chainchomp_x_hist[pos] = cur_sprite_x;
  chainchomp_y_hist[pos] = cur_sprite_y;
  for (int i = 0; i < 6; i++, pos++) {
    uint16 x = chainchomp_x_hist[pos] - chainchomp_x_hist[pos + 1];
    if (!sign16(x - 8))
      chainchomp_x_hist[pos + 1] = chainchomp_x_hist[pos] - 8;
    else if (sign16(x + 8))
      chainchomp_x_hist[pos + 1] = chainchomp_x_hist[pos] + 8;

    uint16 y = chainchomp_y_hist[pos] - chainchomp_y_hist[pos + 1];
    if (!sign16(y - 8))
      chainchomp_y_hist[pos + 1] = chainchomp_y_hist[pos] - 8;
    else if (sign16(y + 8))
      chainchomp_y_hist[pos + 1] = chainchomp_y_hist[pos] + 8;
  }
}

void ChainChomp_Draw(int k) {  // 9dc192
  static const uint8 kChainChomp_Gfx[16] = {0, 1, 2, 3, 3, 3, 2, 1, 0, 0, 0, 4, 4, 4, 0, 0};
  static const uint8 kChainChomp_OamFlags[16] = {0x40, 0x40, 0x40, 0x40, 0, 0, 0, 0, 0, 0, 0, 0, 0x40, 0x40, 0x40, 0x40};

  int j = sprite_D[k];
  sprite_graphics[k] = kChainChomp_Gfx[j];
  sprite_oam_flags[k] = sprite_oam_flags[k] & 0x3f | kChainChomp_OamFlags[j];
  SpriteDraw_SingleLarge(k);
  OamEnt *oam = GetOamCurPtr() + 1;
  uint8 flags = sprite_oam_flags[k] ^ sprite_obj_prio[k];
  int r8 = (sprite_delay_aux1[k] & 1) + 4;
  int pos = k * 8;
  for (int i = 5; i >= 0; i--, pos++, oam++) {
    SetOamHelper0(oam,
                  chainchomp_x_hist[pos] + r8 - BG2HOFS_copy2,
                  chainchomp_y_hist[pos] + r8 - BG2VOFS_copy2,
                  0x8b,
                  flags & 0xf0 | 0xd, 0);
  }
}

void Sprite_C9_Tektite(int k) {  // 9dc275
  int j = sprite_anim_clock[k];
  if (j) {
    sprite_ignore_projectile[k] = j;
    sprite_obj_prio[k] = 0x30;
  }
  switch (j) {
  case 0: Sprite_Tektite(k); break;
  case 1: Sprite_PhantomGanon(k); break;
  case 2: Sprite_GanonTrident(k); break;
  case 3: Sprite_SpiralFireBat(k); break;
  case 4: Sprite_FireBat_Launched(k); break;
  case 5: Sprite_FireBat_Trailer(k); break;
  }
}

void Sprite_Tektite(int k) {  // 9dc293
  int j;

  if (sprite_delay_aux1[k])
    sprite_graphics[k] = 0;
  Tektite_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (Sprite_ReturnIfRecoiling(k))
    return;
  Sprite_CheckDamageToAndFromLink(k);
  Sprite_MoveXYZ(k);
  Sprite_BounceFromTileCollision(k);
  sprite_z_vel[k] -= 1;
  if (sign8(sprite_z[k])) {
    sprite_z[k] = 0;
    sprite_z_vel[k] = 0;
  }
  switch(sprite_ai_state[k]) {
  case 0: { // Stationary
    static const uint8 kTektite_Dir[4] = {3, 2, 1, 0};
    static const int8 kTektite_Xvel[4] = {16, -16, 16, -16};
    static const int8 kTektite_Yvel[4] = {16, 16, -16, -16};
    PointU8 pt;
    j = Sprite_DirectionToFaceLink(k, &pt);
    if ((uint8)(pt.x + 40) < 80 && (uint8)(pt.y + 40) < 80 && player_oam_y_offset != 0x80 &&
        !(sprite_z[k] | sprite_pause[k]) && link_is_on_lower_level == sprite_floor[k] && j != kTektite_Dir[link_direction_facing >> 1]) {
      ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLink(k, 32);
      sprite_x_vel[k] = -pt.x;
      sprite_y_vel[k] = -pt.y;
      sprite_z_vel[k] = 16;
      sprite_ai_state[k] = 1;
      return;
    }
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k]++;
      sprite_B[k]++;
      if (sprite_B[k] == 4) {
        sprite_B[k] = 0;
        sprite_ai_state[k]++;
        sprite_delay_main[k] = (GetRandomNumber() & 63) + 48;
        sprite_z_vel[k] = 12;
        j = Sprite_IsBelowLink(k).a * 2 + Sprite_IsRightOfLink(k).a;
      } else {
        sprite_z_vel[k] = (GetRandomNumber() & 7) + 24;
        j = GetRandomNumber() & 3;
      }
      sprite_x_vel[k] = kTektite_Xvel[j];
      sprite_y_vel[k] = kTektite_Yvel[j];
    } else {
      sprite_graphics[k] = sprite_delay_main[k] >> 4 & 1;
    }
    break;
  }
  case 1: // Aloft
    if (!sprite_z[k]) {
reset_state:
      sprite_ai_state[k] = 0;
      sprite_delay_main[k] = (GetRandomNumber() & 63) + 72;
      sprite_y_vel[k] = 0;
      sprite_x_vel[k] = 0;
    } else {
      sprite_graphics[k] = 2;
    }
    break;
  case 2: // RepeatingHop
    if (!sprite_delay_main[k])
      goto reset_state;
    if (!sprite_z[k]) {
      sprite_z_vel[k] = 12;
      sprite_z[k]++;
      sprite_delay_aux1[k] = 8;
    }
    sprite_graphics[k] = 2;
    break;
  }
}

void Tektite_Draw(int k) {  // 9dc3f5
  static const DrawMultipleData kTektite_Dmd[6] = {
    {-8, 0, 0x00c8, 2},
    { 8, 0, 0x40c8, 2},
    {-8, 0, 0x00ca, 2},
    { 8, 0, 0x40ca, 2},
    {-8, 0, 0x00ea, 2},
    { 8, 0, 0x40ea, 2},
  };
  PrepOamCoordsRet info;
  Sprite_DrawMultiple(k, &kTektite_Dmd[sprite_graphics[k] * 2], 2, &info);
  SpriteDraw_Shadow(k, &info);
}

void Sprite_C8_BigFairy(int k) {  // 9dc414
  if (sprite_head_dir[k])
    Sprite_FairyCloud(k);
  else
    Sprite_BigFairy(k);
}

void Sprite_FairyCloud(int k) {  // 9dc41c
  PrepOamCoordsRet info;
  Sprite_PrepOamCoord(k, &info);
  if (Sprite_ReturnIfInactive(k))
    return;
  sprite_subtype2[k]++;
  FaerieCloud_Draw(k);
  if (!(sprite_subtype2[k] & 31))
    SpriteSfx_QueueSfx2WithPan(k, 0x31);
  switch (sprite_ai_state[k]) {
  case 0:
    sprite_A[k] = 0;
    Sprite_ApplySpeedTowardsLink(k, 8);
    Sprite_MoveXY(k);
    Sprite_Get16BitCoords(k);
    if ((uint16)(link_x_coord - cur_sprite_x + 3) < 6 &&
        (uint16)(link_y_coord - cur_sprite_y + 11) < 6) {
      WORD(link_hearts_filler) += 0xa0;
      sprite_ai_state[k] = 1;
    }
    break;
  case 1:
    if (link_health_current == link_health_capacity) {
      sprite_ai_state[k]++;
      sprite_delay_aux2[0] = 112; // zelda bug
    }
    break;
  case 2:
    if ((sprite_subtype2[k] & 15) || sign8(sprite_A[k]))
      return;
    sprite_A[k] = sprite_A[k] * 2 + 1;
    if (sprite_A[k] >= 0x80) {
      sprite_A[k] = 255;
      flag_is_link_immobilized = 0;
      sprite_state[k] = 0;
    }
    break;
  }
}

void Sprite_BigFairy(int k) {  // 9dc4bf
  PointU8 pt;
  int i = sprite_delay_aux2[k];
  if (i != 0 && i < 0x40) {
    if (--i == 0)
      sprite_state[k] = 0;
    if (i & 1)
      return;
  }
  BigFaerie_Draw(k);
  if (sign8(--sprite_G[k])) {
    sprite_G[k] = 5;
    sprite_graphics[k] = sprite_graphics[k] + 1 & 3;
  }
  if (Sprite_ReturnIfInactive(k))
    return;
  sprite_subtype2[k]++;
  switch (sprite_ai_state[k]) {
  case 0:  // await close player
    FaerieCloud_Draw(k);
    sprite_A[k] = 1;
    Sprite_DirectionToFaceLink(k, &pt);
    if ((uint8)(pt.x + 0x30) < 0x60 && (uint8)(pt.y + 0x30) < 0x60) {
      Link_CancelDash();
      sprite_ai_state[k] = 1;
      dialogue_message_index = 0x15a;
      Sprite_ShowMessageMinimal();
      flag_is_link_immobilized = 1;
      SpriteSpawnInfo info;
      int j = Sprite_SpawnDynamically(k, 0xC8, &info);
      Sprite_SetSpawnedCoordinates(j, &info);
      sprite_head_dir[j] = 1;
      sprite_y_lo[j] -= sprite_z[k];
      sprite_z[j] = 0;
    }
    break;
  case 1:  // dormant
    break;
  }
}

void BigFaerie_Draw(int k) {  // 9dc5d0
  static const DrawMultipleData kBigFaerie_Dmd[16] = {
    {-4, -8, 0x008e, 2},
    { 4, -8, 0x408e, 2},
    {-4,  8, 0x00ae, 2},
    { 4,  8, 0x40ae, 2},
    {-4, -8, 0x008c, 2},
    { 4, -8, 0x408c, 2},
    {-4,  8, 0x00ac, 2},
    { 4,  8, 0x40ac, 2},
    {-4, -8, 0x008a, 2},
    { 4, -8, 0x408a, 2},
    {-4,  8, 0x00aa, 2},
    { 4,  8, 0x40aa, 2},
    {-4, -8, 0x008c, 2},
    { 4, -8, 0x408c, 2},
    {-4,  8, 0x00ac, 2},
    { 4,  8, 0x40ac, 2},
  };
  PrepOamCoordsRet info;
  Sprite_DrawMultiple(k, &kBigFaerie_Dmd[sprite_graphics[k] * 4], 4, &info);
  SpriteDraw_Shadow(k, &info);
}

void FaerieCloud_Draw(int k) {  // 9dc616
  static const int8 kFaerieCloud_Draw_XY[8] = {-12, -6, 0, 6, 12, 18, 0, 6};
  if (!sign8(sprite_A[k]) && !(sprite_A[k] & sprite_subtype2[k])) {
    int x = kFaerieCloud_Draw_XY[GetRandomNumber() & 7];
    int y = kFaerieCloud_Draw_XY[GetRandomNumber() & 7];
    Sprite_GarnishSpawn_Sparkle(k, x, y);
  }
}

void Sprite_C7_Pokey(int k) {  // 9dc64f
  if (sprite_C[k]) {
    SpriteDraw_SingleLarge(k);
    if (Sprite_ReturnIfInactive(k))
      return;
    Sprite_CheckDamageToAndFromLink(k);
    Sprite_MoveXYZ(k);
    sprite_z_vel[k] -= 2;
    if (sign8(sprite_z[k])) {
      sprite_z_vel[k] = 16;
      sprite_z[k] = 0;
    }
    if (Sprite_BounceFromTileCollision(k))
      SpriteSfx_QueueSfx2WithPan(k, 0x21);
    if (sprite_G[k] >= 3) {
      sprite_state[k] = 6;
      sprite_delay_main[k] = 10;
      sprite_flags5[k] = 0;
      SpriteSfx_QueueSfx2WithPan(k, 0x1e);
    }
    return;
  }

  Hokbok_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (sprite_A[k] && sprite_F[k] == 15) {
    sprite_F[k] = 6;
    sprite_z[k] += sprite_B[k];
    if (!--sprite_A[k])
      sprite_health[k] = 17;
    sprite_x_vel[k] += sign8(sprite_x_vel[k]) ? -4 : 4;
    sprite_y_vel[k] += sign8(sprite_y_vel[k]) ? -4 : 4;

    SpriteSpawnInfo info;
    int j = Sprite_SpawnDynamically(k, 0xc7, &info);
    if (j >= 0) {
      Sprite_SetSpawnedCoordinates(j, &info);
      sprite_C[j] = 1;
      sprite_health[j] = 1;
      sprite_x_vel[j] = sprite_x_recoil[k];
      sprite_y_vel[j] = sprite_y_recoil[k];
      sprite_defl_bits[j] = 64;
    }
  }
  if (Sprite_ReturnIfRecoiling(k))
    return;
  Sprite_CheckDamageToAndFromLink(k);
  switch(sprite_ai_state[k]) {
  case 0:
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k] = 1;
      sprite_z_vel[k] = 16;
    } else {
      static const uint8 kHokbok_B[8] = {8, 7, 6, 5, 4, 5, 6, 7};
      sprite_B[k] = kHokbok_B[sprite_delay_main[k] >> 1];
    }
    break;
  case 1:
    Sprite_MoveXYZ(k);
    sprite_z_vel[k] -= 2;
    if (sign8(sprite_z[k])) {
      sprite_z[k] = 0;
      sprite_ai_state[k] = 0;
      sprite_delay_main[k] = 15;
    }
    Sprite_BounceFromTileCollision(k);
    break;
  }
}

void Hokbok_Draw(int k) {  // 9dc77d
  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  OamEnt *oam = GetOamCurPtr() + 3;
  int d = sprite_B[k];
  uint16 y = info.y;
  for (int i = sprite_A[k]; i >= 0; i--, oam--) {
    SetOamHelper0(oam, info.x, y,
                  ((i == 0) ? 0xa2 : 0xa0) - ((d < 7) ? 0x20 : 0), info.flags, 2);
    y -= d;
  }
  SpriteDraw_Shadow(k, &info);
}

void Sprite_C5_Medusa(int k) {  // 9dc7eb
  PrepOamCoordsRet info;
  Sprite_PrepOamCoord(k, &info);
  if (!player_is_indoors) {
    sprite_x_vel[k] = 255;
    sprite_subtype[k] = 255;
    if (!Sprite_CheckTileCollision(k))
      return;
    if (Sprite_ReturnIfInactive(k))
      return;
    sprite_type[k] = 0x19;
    SpritePrep_LoadProperties(k);
    sprite_E[k]++;
    sprite_x_lo[k] += 8;
    sprite_y_lo[k] -= 8;
    SpriteSfx_QueueSfx3WithPan(k, 0x19);
    sprite_defl_bits[k] = 0x80;
  } else {
    if (Sprite_ReturnIfInactive(k))
      return;
    sprite_subtype2[k]++;
    if (!(sprite_subtype2[k] & 0x7f) && sprite_floor[k] == link_is_on_lower_level) {
      int j = Sprite_SpawnFireball(k);
      if (j >= 0) {
        sprite_defl_bits[j] = sprite_defl_bits[j] | 8;
        sprite_bump_damage[j] = 4;
      }
    }
  }
}

void Sprite_C6_4WayShooter(int k) {  // 9dc869
  static const int8 kFireballJunction_X[4] = {12, -12, 0, 0};
  static const int8 kFireballJunction_Y[4] = {0, 0, 12, -12};
  static const int8 kFireballJunction_XYvel[6] = {0, 0, 40, -40, 0, 0};
  PrepOamCoordsRet info;
  Sprite_PrepOamCoord(k, &info);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (sprite_delay_main[k] == 24) {
    int j = Sprite_SpawnFireball(k);
    if (j >= 0) {
      sprite_defl_bits[j] |= 8;
      sprite_bump_damage[j] = 4;
      int i = Sprite_DirectionToFaceLink(j, NULL);
      sprite_x_vel[j] = kFireballJunction_XYvel[i + 2];
      sprite_y_vel[j] = kFireballJunction_XYvel[i];
      Sprite_SetX(j, Sprite_GetX(j) + kFireballJunction_X[i]);
      Sprite_SetY(j, Sprite_GetY(j) + kFireballJunction_Y[i]);
    }
  } else if (sprite_delay_main[k] == 0) {
    if (button_b_frames && sprite_floor[k] == link_is_on_lower_level)
      sprite_delay_main[k] = 32;
  }
}

void Sprite_C4_Thief(int k) {  // 9dc8d8
  int j;



  Thief_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (Sprite_ReturnIfRecoiling(k))
    return;
  Sprite_CheckDamageFromLink(k);
  if (sprite_ai_state[k] != 3) {
    j = Sprite_DirectionToFaceLink(k, NULL);
    sprite_head_dir[k] = j;
    if ((j ^ sprite_D[k]) == 1)
      sprite_D[k] = j;
  }
  switch (sprite_ai_state[k]) {
  case 0:  // loitering
    Thief_CheckCollisionWithLink(k);
    if (!sprite_delay_main[k]) {
      if ((uint16)(link_x_coord - cur_sprite_x + 0x50) < 0xa0 &&
        (uint16)(link_y_coord - cur_sprite_y + 0x50) < 0xa0) {
        sprite_ai_state[k]++;
        sprite_delay_main[k] = 16;
      }
    }
    sprite_graphics[k] = kThief_Gfx[sprite_D[k]];
    break;
  case 1:  // watch player
    Thief_CheckCollisionWithLink(k);
    sprite_D[k] = sprite_head_dir[k] = Sprite_DirectionToFaceLink(k, NULL);
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k] = 2;
      sprite_delay_main[k] = 32;
    }
thief_common:
    if (!(frame_counter & 31))
      sprite_D[k] = sprite_head_dir[k];
    sprite_graphics[k] = kThief_Gfx[4 + sprite_D[k] + (++sprite_subtype2[k] & 4)];
    break;
  case 2:  // chase player
    Sprite_ApplySpeedTowardsLink(k, 18);
    if (!sprite_wallcoll[k])
      Sprite_MoveXY(k);
    Sprite_CheckTileCollision(k);
    if (!sprite_delay_main[k]) {
      if ((uint16)(link_x_coord - cur_sprite_x + 0x50) >= 0xa0 ||
        (uint16)(link_y_coord - cur_sprite_y + 0x50) >= 0xa0) {
        sprite_ai_state[k] = 0;
        sprite_delay_main[k] = 128;
      }
    }
    if (Sprite_CheckDamageToLink(k)) {
      sprite_ai_state[k]++;
      sprite_delay_main[k] = 32;
      Thief_SpillItems(k);
      SpriteSfx_QueueSfx2WithPan(k, 0xb);
    }
    goto thief_common;
  case 3:  // steal
    Thief_CheckCollisionWithLink(k);
    j = Thief_ScanForBooty(k);

    if (!sprite_delay_main[k]) {
      sprite_graphics[k] = kThief_Gfx[4 + sprite_D[k] + (++sprite_subtype2[k] & 4)];
      if (!sprite_wallcoll[k])
        Sprite_MoveXY(k);
      Sprite_CheckTileCollision(k);
      sprite_D[k] = sprite_head_dir[k];
    }
    if (!((k ^ frame_counter) & 3))
      sprite_head_dir[k] = Sprite_DirectionToFaceLocation(k, Sprite_GetX(j), Sprite_GetY(j));
    break;
  }
}

uint8 Thief_ScanForBooty(int k) {  // 9dca24
  for (int j = 15; j >= 0; j--) {
    if (sprite_state[j] && (sprite_type[j] == 0xdc || sprite_type[j] == 0xe1 || sprite_type[j] == 0xd9)) {
      Thief_TargetBooty(k, j);
      return j;
    }
  }
  sprite_ai_state[k] = 0;
  sprite_delay_main[k] = 64;
  return 0xff;
}

void Thief_TargetBooty(int k, int j) {  // 9dca4c
  if (!((k ^ frame_counter) & 3)) {
    ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLocation(k, Sprite_GetX(j), Sprite_GetY(j), 19);
    sprite_x_vel[k] = pt.x;
    sprite_y_vel[k] = pt.y;
  }
  for (j = 15; j >= 0; j--) {
    if (!((j ^ frame_counter) & 3 | sprite_delay_aux4[j]) && sprite_state[j] &&
        (sprite_type[j] == 0xdc || sprite_type[j] == 0xe1 || sprite_type[j] == 0xd9)) {
      Thief_GrabBooty(k, j);
    }
  }
}

void Thief_GrabBooty(int k, int j) {  // 9dca9e
  if ((uint16)(Sprite_GetX(j) - cur_sprite_x + 8) < 16 &&
      (uint16)(Sprite_GetY(j) - cur_sprite_y + 12) < 24) {
    sprite_state[j] = 0;

    int t = sprite_type[j] - 0xd8;
    SpriteSfx_QueueSfx3WithPan(t, kAbsorptionSfx[t]);
    sprite_delay_main[k] = 14;
  }
}

void Thief_CheckCollisionWithLink(int k) {  // 9dcaf2
  if (Sprite_CheckDamageToLink_same_layer(k)) {
    ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLink(k, 32);
    link_actual_vel_y = pt.y;
    sprite_y_recoil[k] = pt.y ^ 0xff;
    link_actual_vel_x = pt.x;
    sprite_x_recoil[k] = pt.x ^ 0xff;
    link_incapacitated_timer = 4;
    sprite_F[k] = 12;
    SpriteSfx_QueueSfx2WithPan(k, 0xb);
  }
}

void Thief_SpillItems(int k) {  // 9dcb30
  static const uint8 kThiefSpawn_Items[4] = {0xd9, 0xe1, 0xdc, 0xd9};
  static const int8 kThiefSpawn_Xvel[6] = {0, 24, 24, 0, -24, -24};
  static const int8 kThiefSpawn_Yvel[6] = {-32, -16, 16, 32, 16, -16};

  tmp_counter = 5;
  do {
    byte_7E0FB6 = GetRandomNumber() & 3;
    int j;
    if (byte_7E0FB6 == 1) {
      j = link_num_arrows;
    } else if (byte_7E0FB6 == 2) {
      j = link_item_bombs;
    } else {
      j = link_rupees_goal;
    }
    if (!j)
      return;
    SpriteSpawnInfo info;
    j = Sprite_SpawnDynamicallyEx(k, kThiefSpawn_Items[byte_7E0FB6], &info, 7);
    if (j < 0)
      return;
    if (byte_7E0FB6 == 1)
      link_num_arrows--;
    else if (byte_7E0FB6 == 2)
      link_item_bombs--;
    else
      link_rupees_goal--;
    Sprite_SetX(j, link_x_coord);
    Sprite_SetY(j, link_y_coord);
    sprite_z_vel[j] = 0x18;
    sprite_x_vel[j] = kThiefSpawn_Xvel[tmp_counter];
    sprite_y_vel[j] = kThiefSpawn_Yvel[tmp_counter];
    sprite_delay_aux4[j] = 32;
    sprite_head_dir[j] = 1;
    sprite_stunned[j] = 255;
  } while (!sign8(--tmp_counter));
}

void Thief_Draw(int k) {  // 9dcc9e
  static const DrawMultipleData kThief_Dmd[24] = {
    {0, -6, 0x0000, 2},
    {0,  0, 0x0006, 2},
    {0, -6, 0x0000, 2},
    {0,  0, 0x4006, 2},
    {0, -6, 0x0000, 2},
    {0,  0, 0x0020, 2},
    {0, -7, 0x0004, 2},
    {0,  0, 0x0022, 2},
    {0, -7, 0x0004, 2},
    {0,  0, 0x4022, 2},
    {0, -7, 0x0004, 2},
    {0,  0, 0x0024, 2},
    {0, -8, 0x0002, 2},
    {0,  0, 0x000a, 2},
    {0, -7, 0x0002, 2},
    {0,  0, 0x000e, 2},
    {0, -7, 0x0002, 2},
    {0,  0, 0x000a, 2},
    {0, -8, 0x4002, 2},
    {0,  0, 0x400a, 2},
    {0, -7, 0x4002, 2},
    {0,  0, 0x400e, 2},
    {0, -7, 0x4002, 2},
    {0,  0, 0x400a, 2},
  };
  static const uint8 kThief_DrawChar[4] = {2, 2, 0, 4};
  static const uint8 kThief_DrawFlags[4] = {0x40, 0, 0, 0};
  PrepOamCoordsRet info;
  Sprite_DrawMultiple(k, &kThief_Dmd[sprite_graphics[k] * 2], 2, &info);
  if (!sprite_pause[k]) {
    OamEnt *oam = GetOamCurPtr();
    int j = sprite_head_dir[k];
    oam->charnum = kThief_DrawChar[j];
    oam->flags = (oam->flags & ~0x40) | kThief_DrawFlags[j];
    SpriteDraw_Shadow(k, &info);
  }
}

void Sprite_C3_Gibo(int k) {  // 9dcce1
  if (sprite_B[k]) {
    SpriteDraw_SingleLarge(k);
    if (Sprite_ReturnIfInactive(k))
      return;
    Sprite_CheckDamageToAndFromLink(k);
    sprite_oam_flags[k] = sprite_oam_flags[k] & 0x3f | kGibo_OamFlags[++sprite_subtype2[k] >> 2 & 3];
    if (sprite_delay_main[k]) {
      Sprite_MoveXY(k);
      Sprite_BounceFromTileCollision(k);
    }
    return;
  }
  Gibo_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  sprite_anim_clock[k]++;
  int j = sprite_head_dir[k], i;
  if (sprite_state[j] == 6) {
    sprite_state[k] = sprite_state[j];
    sprite_delay_main[k] = sprite_delay_main[j];
    sprite_flags2[k] += 4;
    return;
  }
  sprite_subtype2[k] = frame_counter >> 3 & 3;
  if (!(frame_counter & 63))
    sprite_D[k] = Sprite_IsRightOfLink(k).a << 2;
  Sprite_CheckDamageToLink(k);  // original destroys y which is a bug
  switch(sprite_ai_state[k]) {
  case 0:  // expel nucleus
    if (sprite_delay_main[k] == 0) {
      sprite_ai_state[k]++;
      sprite_delay_main[k] = 48;
      sprite_A[k]++;
      SpriteSpawnInfo info;
      j = Sprite_SpawnDynamically(k, 0xc3, &info);
      if (j >= 0) {
        static const int8 kGibo_Xvel[8] = {16, 16, 0, -16, -16, -16, 0, 16};
        static const int8 kGibo_Yvel[8] = {0, 0, 16, -16, 16, 16, -16, -16};
        Sprite_SetSpawnedCoordinates(j, &info);
        sprite_head_dir[k] = j;
        sprite_flags2[j] = 1;
        sprite_B[j] = 1;
        sprite_flags3[j] = 16;
        sprite_health[j] = sprite_G[k];
        sprite_oam_flags[j] = 7;
        sprite_delay_main[j] = 48;
        if (++sprite_C[k] == 3) {
          sprite_C[k] = 0;
          i = Sprite_DirectionToFaceLink(k, NULL);
        } else {
          i = GetRandomNumber() & 7;
        }
        sprite_x_vel[j] = kGibo_Xvel[i];
        sprite_y_vel[j] = kGibo_Yvel[i];
      }
    } else if (sprite_delay_main[k] == 32) {
      sprite_delay_aux1[k] = 32;
    }
    break;
  case 1:  // delay pursuit
    if (!sprite_delay_main[k])
      sprite_ai_state[k]++;
    break;
  case 2:  // pursue nucleus
    if (!((k ^ frame_counter) & 3)) {
      uint16 x = Sprite_GetX(j), y = Sprite_GetY(j);
      if ((uint16)(cur_sprite_x - x + 2) < 4 &&
          (uint16)(cur_sprite_y - y + 2) < 4) {
        j = sprite_head_dir[k];
        sprite_state[j] = 0;
        sprite_A[k] = 0;
        sprite_ai_state[k] = 0;
        sprite_G[k] = sprite_health[j];
        sprite_delay_main[k] = (GetRandomNumber() & 31) + 32;
        return;
      }
      ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLocation(k, x, y, 16);
      sprite_x_vel[k] = pt.x;
      sprite_y_vel[k] = pt.y;
    }
    Sprite_MoveXY(k);
    break;
  }
}

void Gibo_Draw(int k) {  // 9dcf5e
  static const DrawMultipleData kGibo_Dmd[32] = {
    { 4, -4, 0x408a, 2},
    {-4, -4, 0x408f, 0},
    {12, 12, 0x408e, 0},
    {-4,  4, 0x408c, 2},
    { 4, -4, 0x40aa, 2},
    {-4, -4, 0x409f, 0},
    {12, 12, 0x409e, 0},
    {-4,  4, 0x40ac, 2},
    { 3, -3, 0x40aa, 2},
    {-3, -3, 0x409f, 0},
    {11, 11, 0x409e, 0},
    {-3,  3, 0x40ac, 2},
    { 3, -3, 0x408a, 2},
    {-3, -3, 0x408f, 0},
    {11, 11, 0x408e, 0},
    {-3,  3, 0x408c, 2},
    {-3, -4, 0x008a, 2},
    {13, -4, 0x008f, 0},
    {-3, 12, 0x008e, 0},
    { 5,  4, 0x008c, 2},
    {-3, -4, 0x00aa, 2},
    {13, -4, 0x009f, 0},
    {-3, 12, 0x009e, 0},
    { 5,  4, 0x00ac, 2},
    {-2, -3, 0x00aa, 2},
    {12, -3, 0x009f, 0},
    {-2, 11, 0x009e, 0},
    { 4,  3, 0x00ac, 2},
    {-2, -3, 0x008a, 2},
    {12, -3, 0x008f, 0},
    {-2, 11, 0x008e, 0},
    { 4,  3, 0x008c, 2},
  };
  if (!sprite_A[k]) {
    uint8 bak0 = sprite_flags2[k];
    sprite_flags2[k] = 1;
    uint8 bak1 = sprite_oam_flags[k];
    sprite_oam_flags[k] = kGibo_OamFlags[sprite_anim_clock[k] >> 2 & 3] |
                          kGibo_OamFlags2[sprite_delay_aux1[k] >> 2 & 1];
    SpriteDraw_SingleLarge(k);
    sprite_oam_flags[k] = bak1;
    sprite_flags2[k] = bak0;
  }
  oam_cur_ptr += 8;
  oam_ext_cur_ptr += 2;
  Sprite_DrawMultiple(k, &kGibo_Dmd[(sprite_subtype2[k] + sprite_D[k]) * 4], 4, NULL);
}

void Sprite_C2_Boulder(int k) {  // 9dcfcb
  if (!player_is_indoors) {
    Boulder_OutdoorsMain(k);
    return;
  }
  if (byte_7E0FC6 < 3)
    SpriteDraw_SingleSmall(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  sprite_oam_flags[k] = frame_counter << 2 & 0xc0;
  Sprite_MoveXYZ(k);
  if ((k ^ frame_counter) & 3)
    return;
  if ((uint16)(cur_sprite_x - link_x_coord + 4) < 16 && (uint16)(cur_sprite_y - link_y_coord - 4) < 12)
    Sprite_AttemptDamageToLinkPlusRecoil(k);
  if (!((k ^ frame_counter) & 3) && Sprite_CheckTileCollision(k))
    sprite_state[k] = 0;
}

void Boulder_OutdoorsMain(int k) {  // 9dd02a
  static const int8 kBoulder_Zvel[2] = {32, 48};
  static const int8 kBoulder_Yvel[2] = {8, 32};
  static const int8 kBoulder_Xvel[4] = {24, 16, -24, -16};
  sprite_obj_prio[k] = 0x30;
  Boulder_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  sprite_subtype2[k] -= sprite_D[k];
  Sprite_CheckDamageToAndFromLink(k);
  Sprite_MoveXYZ(k);
  sprite_z_vel[k]-=2;
  if (sign8(sprite_z[k])) {
    sprite_z[k] = 0;
    int j = Sprite_CheckTileCollision(k) != 0;
    sprite_z_vel[k] = kBoulder_Zvel[j];
    sprite_y_vel[k] = kBoulder_Yvel[j];
    j += (GetRandomNumber() & 1) * 2;
    sprite_x_vel[k] = kBoulder_Xvel[j];
    sprite_D[k] = (j & 2) - 1;
    SpriteSfx_QueueSfx2WithPan(k, 0xb);

  }
}

void Boulder_Draw(int k) {  // 9dd185
  static const DrawMultipleData kBoulder_Dmd[16] = {
    {-8, -8, 0x01cc, 2},
    { 8, -8, 0x01ce, 2},
    {-8,  8, 0x01ec, 2},
    { 8,  8, 0x01ee, 2},
    {-8, -8, 0x41ce, 2},
    { 8, -8, 0x41cc, 2},
    {-8,  8, 0x41ee, 2},
    { 8,  8, 0x41ec, 2},
    {-8, -8, 0xc1ee, 2},
    { 8, -8, 0xc1ec, 2},
    {-8,  8, 0xc1ce, 2},
    { 8,  8, 0xc1cc, 2},
    {-8, -8, 0x81ec, 2},
    { 8, -8, 0x81ee, 2},
    {-8,  8, 0x81cc, 2},
    { 8,  8, 0x81ce, 2},
  };
  PrepOamCoordsRet info;
  Sprite_DrawMultiple(k, &kBoulder_Dmd[(sprite_subtype2[k] >> 3 & 3) * 4], 4, &info);
  Sprite_DrawLargeShadow2(k);
}

void SpriteDraw_BigShadow(int k, int anim) {  // 9dd1a8
  cur_sprite_y += sprite_z[k];
  oam_cur_ptr += 16;
  oam_ext_cur_ptr += 4;
  Sprite_DrawMultiple(k, &kLargeShadow_Dmd[anim * 3], 3, NULL);
  Sprite_Get16BitCoords(k);
}

void Sprite_DrawLargeShadow2(int k) {  // 9dd1af
  int z = sprite_z[k] >> 3;
  SpriteDraw_BigShadow(k, (z > 4) ? 4 : z);
}

void CutsceneAgahnim_SpawnZeldaOnAltar(int k) {  // 9dd1fd
  sprite_x_lo[k] += 8;
  sprite_y_lo[k] += 6;
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0xc1, &info);
  sprite_A[j] = 1;
  sprite_ignore_projectile[j] = 1;
  Sprite_SetSpawnedCoordinates(j, &info);
  sprite_y_lo[j] = info.r2_y + 40;
  sprite_flags2[j] = 0;
  sprite_oam_flags[j] = 12;
}

void Sprite_C1_CutsceneAgahnim(int k) {  // 9dd234
  switch (sprite_A[k]) {
  case 0: CutsceneAgahnim_Agahnim(k); break;
  case 1: Sprite_CutsceneAgahnim_Zelda(k); break;
  }

}

void CutsceneAgahnim_Agahnim(int k) {  // 9dd23f
  static const uint8 kChattyAgahnim_LevitateGfx[4] = {2, 0, 3, 0};
  int j;
  PrepOamCoordsRet info;

  if (sprite_C[k]) {
    if (!sprite_delay_main[k])
      sprite_state[k] = 0;
    if (!(sprite_delay_main[k] & 1))
      ChattyAgahnim_Draw(k, &info);
    return;
  }
  ChattyAgahnim_Draw(k, &info);
  SpriteDraw_CutsceneAgahnimSpell(k, &info);
  if (sprite_pause[k]) {
    sprite_ai_state[k] = 0;
    sprite_B[k] = 0;
    sprite_graphics[k] = 0;
    sprite_delay_main[k] = 64;
  }
  if (Sprite_ReturnIfInactive(k))
    return;
  switch(sprite_ai_state[k]) {
  case 0:  // problab
    if (!sprite_delay_main[k]) {
      flag_is_link_immobilized = 1;
      dialogue_message_index = 0x13d;
      Sprite_ShowMessageMinimal();
      sprite_ai_state[k]++;
    }
    break;
  case 1:  // levitate zelda
    j = ++sprite_B[k];
    sprite_graphics[k] = sprite_z[15] < 16 ? kChattyAgahnim_LevitateGfx[j >> 5 & 3] : 1;
    if ((j & 15) == 0) {
      sprite_graphics[15] = 1;
      if (++sprite_z[15] == 22) {
        sound_effect_2 = 0x27;
        sprite_ai_state[k]++;
        sprite_delay_main[k] = 255;
        sprite_subtype2[k] = 2;
        sprite_subtype[k] = 255;
      }
    }
    break;
  case 2:  // do telewarp
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k]++;
      sprite_delay_main[k] = 80;
    } else if (sprite_delay_main[k] == 120) {
      intro_times_pal_flash = 120;
    } else if (sprite_delay_main[k] < 128 && (sprite_delay_main[k] & 3) == 0) {
      sound_effect_2 = 0x2b;
      if (sprite_subtype2[k] != 14)
        sprite_subtype2[k] += 4;
    }
    break;
  case 3:  // complete telewarp
    if (sprite_delay_main[k]) {
      if (!(sprite_delay_main[k] & 3) && sprite_subtype[k] != 9)
        sprite_subtype[k] += 2;
    } else {
      sprite_delay_main[15] = 19;
      sprite_ai_state[k]++;
      sprite_delay_main[k] = 80;
      sprite_subtype2[k] = 0;
      sound_effect_1 = 0x33;
    }
    break;
  case 4:  // epiblab
    if (!sprite_delay_main[k]) {
      dialogue_message_index = 0x13e;
      Sprite_ShowMessageMinimal();
      sprite_ai_state[k]++;
      sprite_delay_main[k] = 2;
    }
    break;
  case 5:  // teleport to curtains
    if (sprite_delay_main[k] == 1)
      sound_effect_2 = 0x28;
    sprite_y_vel[k] = -32;
    Sprite_MoveY(k);
    if (sprite_y_lo[k] < 48) {
      sprite_delay_aux4[k] = 66;
      sprite_ai_state[k]++;
    }
    Sprite_Agahnim_ApplyMotionBlur(k);
    break;
  case 6:  // linger then terminate
    if (!sprite_delay_aux4[k]) {
      flag_is_link_immobilized = 0;
      sprite_state[k] = 0;
      Sprite_ManuallySetDeathFlagUW(k);
      dung_savegame_state_bits |= 0x4000;
    }
    break;
  }
}

int Sprite_Agahnim_ApplyMotionBlur(int k) {  // 9dd392
  if (frame_counter & 3)
    return -1;
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0xc1, &info);
  if (j >= 0) {
    Sprite_SetSpawnedCoordinates(j, &info);
    sprite_graphics[j] = sprite_graphics[k];
    sprite_delay_main[j] = 32;
    sprite_ignore_projectile[j] = 32;
    sprite_C[j] = 32;
  }
  return j;
}

void ChattyAgahnim_Draw(int k, PrepOamCoordsRet *info) {  // 9dd451
  static const DrawMultipleData kChattyAgahnim_Dmd[16] = {
    {-8, -8, 0x0b82, 2},
    { 8, -8, 0x4b82, 2},
    {-8,  8, 0x0ba2, 2},
    { 8,  8, 0x4ba2, 2},
    {-8, -8, 0x0b80, 2},
    { 8, -8, 0x4b80, 2},
    {-8,  8, 0x0ba0, 2},
    { 8,  8, 0x4ba0, 2},
    {-8, -8, 0x0b80, 2},
    { 8, -8, 0x4b82, 2},
    {-8,  8, 0x0ba0, 2},
    { 8,  8, 0x4ba2, 2},
    {-8, -8, 0x0b82, 2},
    { 8, -8, 0x4b80, 2},
    {-8,  8, 0x0ba2, 2},
    { 8,  8, 0x4ba0, 2},
  };
  if (sprite_delay_aux4[k] & 1)
    return;

  if (sprite_C[k] == 0) {
    oam_cur_ptr = 0x900;
    oam_ext_cur_ptr = 0xa60;
  }
  Sprite_DrawMultiple(k, &kChattyAgahnim_Dmd[sprite_graphics[k] * 4], 4, info);
  SpriteDraw_Shadow_custom(k, info, 18);
}

void SpriteDraw_CutsceneAgahnimSpell(int k, PrepOamCoordsRet *info) {  // 9dd516
  static const OamEntSigned kChattyAgahnim_Telewarp_Data[28] = {
    {-10, -16, 0xce, 0x06},
    { 18, -16, 0xce, 0x06},
    { 20, -13, 0x26, 0x06},
    { 20,  -5, 0x36, 0x06},
    {-12, -13, 0x26, 0x46},
    {-12,  -5, 0x36, 0x46},
    { 18,   0, 0x26, 0x06},
    { 18,   8, 0x36, 0x06},
    {-10,   0, 0x26, 0x46},
    {-10,   8, 0x36, 0x46},
    { -8,   0, 0x22, 0x06},
    {  8,   0, 0x22, 0x46},
    { -8,  16, 0x22, 0x86},
    {  8,  16, 0x22, 0xc6},
    {-10, -16, 0xce, 0x04},
    { 18, -16, 0xce, 0x04},
    { 20, -13, 0x26, 0x44},
    { 20,  -5, 0x36, 0x44},
    {-12, -13, 0x26, 0x04},
    {-12,  -5, 0x36, 0x04},
    { 18,   0, 0x26, 0x44},
    { 18,   8, 0x36, 0x44},
    {-10,   0, 0x26, 0x04},
    {-10,   8, 0x36, 0x04},
    { -8,   0, 0x20, 0x04},
    {  8,   0, 0x20, 0x44},
    { -8,  16, 0x20, 0x84},
    {  8,  16, 0x20, 0xc4},
  };
  static const uint8 kChattyAgahnim_Telewarp_Data_Big[14] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2};
  Oam_AllocateFromRegionA(0x38);
  const OamEntSigned *data = kChattyAgahnim_Telewarp_Data;
  if (!(frame_counter & 2))
    data += 14;
  const uint8 *bigp = kChattyAgahnim_Telewarp_Data_Big;
  if (!sprite_subtype2[k])
    return;
  OamEnt *oam = GetOamCurPtr();
  uint8 kn = sprite_subtype2[k] - 1;
  uint8 end = sprite_subtype[k];
  uint8 t = end + 1;
  oam += t;
  bigp += t;
  data += t;
  do {
    SetOamPlain(oam, info->x + data->x, info->y + data->y - 8, data->charnum, data->flags | 0x31, *bigp);
  } while (data++, bigp++, oam++, --kn != end);
}

void Sprite_CutsceneAgahnim_Zelda(int k) {  // 9dd57d
  static const DrawMultipleData kAltarZelda_Dmd[4] = {
    {-4, 0, 0x0103, 2},
    { 4, 0, 0x0104, 2},
    {-4, 0, 0x0100, 2},
    { 4, 0, 0x0101, 2},
  };
  int j = sprite_delay_main[k];
  if (j != 0) {
    SpriteDraw_AltarZeldaWarp(k);
    if (j == 1)
      sprite_state[k] = 0;
    if (j < 12)
      return;
  }
  Oam_AllocateFromRegionA(8);
  PrepOamCoordsRet info;
  Sprite_DrawMultiple(k, &kAltarZelda_Dmd[sprite_graphics[k] * 2], 2, &info);
  AltarZelda_DrawBody(k, &info);
}

void AltarZelda_DrawBody(int k, PrepOamCoordsRet *info) {  // 9dd5e9
  static const uint8 kAltarZelda_XOffs[16] = {4, 4, 3, 3, 2, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0};
  Oam_AllocateFromRegionA(8);
  int z = sprite_z[k] < 31 ? sprite_z[k] : 31;
  uint8 xoffs = kAltarZelda_XOffs[z >> 1];
  int y = Sprite_GetY(k) - BG2VOFS_copy2;
  OamEnt *oam = GetOamCurPtr();
  SetOamHelper0(oam + 0, info->x + xoffs, y + 7, 0x6c, 0x24, 2);
  SetOamHelper0(oam + 1, info->x - xoffs, y + 7, 0x6c, 0x24, 2);
}

void SpriteDraw_AltarZeldaWarp(int k) {  // 9dd6b1
  static const DrawMultipleData kAltarZelda_Warp_Dmd[10] = {
    { 4, 4, 0x0480, 0},
    { 4, 4, 0x0480, 0},
    { 4, 4, 0x04b7, 0},
    { 4, 4, 0x04b7, 0},
    {-6, 0, 0x0524, 2},
    { 6, 0, 0x4524, 2},
    {-8, 0, 0x0524, 2},
    { 8, 0, 0x4524, 2},
    { 0, 0, 0x05c6, 2},
    { 0, 0, 0x05c6, 2},
  };
  Oam_AllocateFromRegionA(8);
  Sprite_DrawMultiple(k, &kAltarZelda_Warp_Dmd[(sprite_delay_main[k] >> 2) * 2], 2, NULL);
}

void Sprite_InitializedSegmented(int k) {  // 9dd6d1
  for (int i = 0; i < 128; i++) {
    moldorm_x_lo[i] = sprite_x_lo[k];
    moldorm_x_hi[i] = sprite_x_hi[k];
    moldorm_y_lo[i] = sprite_y_lo[k];
    moldorm_y_hi[i] = sprite_y_hi[k];
  }
}

void GiantMoldorm_Draw(int k) {  // 9dd881
  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  sprite_oam_flags[k] = 11;
  SpriteDraw_Moldorm_Eyeballs(k, &info);
  oam_cur_ptr += 8, oam_ext_cur_ptr += 2;

  int j = sprite_subtype2[k] & 0x7f;
  moldorm_x_lo[j] = sprite_x_lo[k];
  moldorm_x_hi[j] = sprite_x_hi[k];
  moldorm_y_lo[j] = sprite_y_lo[k];
  moldorm_y_hi[j] = sprite_y_hi[k];

  SpriteDraw_Moldorm_Head(k);
  if (sprite_B[k] < 4) {
    GiantMoldorm_DrawSegment_AB(k, 16);
    if (sprite_B[k] < 3) {
      GiantMoldorm_DrawSegment_AB(k, 28);
      if (sprite_B[k] < 2) {
        SpriteDraw_Moldorm_SegmentC(k);
        if (sprite_B[k] == 0)
          Moldorm_HandleTail(k);
      }
    }
  }
  GiantMoldorm_IncrementalSegmentExplosion(k);
  Sprite_Get16BitCoords(k);
}

void GiantMoldorm_IncrementalSegmentExplosion(int k) {  // 9dd8f2
  if (sprite_state[k] == 9 && sprite_delay_aux4[k] && sprite_delay_aux4[k] < 80 &&
      !(sprite_delay_aux4[k] & 15 | submodule_index | flag_unk1)) {
    sprite_B[k]++;
    Sprite_MakeBossExplosion(k);
  }
}

void SpriteDraw_Moldorm_Head(int k) {  // 9dd993
  static const DrawMultipleData kGiantMoldorm_Head_Dmd[16] = {
    {-8, -8, 0x0080, 2},
    { 8, -8, 0x0082, 2},
    {-8,  8, 0x00a0, 2},
    { 8,  8, 0x00a2, 2},
    {-8, -8, 0x4082, 2},
    { 8, -8, 0x4080, 2},
    {-8,  8, 0x40a2, 2},
    { 8,  8, 0x40a0, 2},
    {-6, -6, 0x0080, 2},
    { 6, -6, 0x0082, 2},
    {-6,  6, 0x00a0, 2},
    { 6,  6, 0x00a2, 2},
    {-6, -6, 0x4082, 2},
    { 6, -6, 0x4080, 2},
    {-6,  6, 0x40a2, 2},
    { 6,  6, 0x40a0, 2},
  };
  int t = (sprite_subtype2[k] >> 1 & 1) + (sprite_delay_aux1[k] & 2);
  Sprite_DrawMultiple(k, &kGiantMoldorm_Head_Dmd[t * 4], 4, NULL);
}

void SpriteDraw_Moldorm_SegmentC(int k) {  // 9dda5f
  sprite_graphics[k] = 0;
  oam_cur_ptr += 0x10;
  oam_ext_cur_ptr += 4;
  GiantMoldorm_DrawSegment_C_OrTail(k, 0x28);
}

void Moldorm_HandleTail(int k) {  // 9ddaba
  SpriteDraw_Moldorm_Tail(k);
  if (!sprite_delay_aux2[k]) {
    sprite_A[k] = 1;
    sprite_flags4[k] = 0;
    sprite_defl_bits[k] = 0;
    uint16 oldx = Sprite_GetX(k);
    uint16 oldy = Sprite_GetY(k);
    Sprite_SetX(k, cur_sprite_x);
    Sprite_SetY(k, cur_sprite_y);
    Sprite_CheckDamageFromLink(k);
    sprite_A[k] = 0;
    sprite_flags4[k] = 9;
    sprite_defl_bits[k] = 4;
    Sprite_SetX(k, oldx);
    Sprite_SetY(k, oldy);
  }
}

void SpriteDraw_Moldorm_Tail(int k) {  // 9ddb17
  oam_cur_ptr += 4;
  oam_ext_cur_ptr += 1;
  sprite_graphics[k]++;
  sprite_oam_flags[k] = 13;
  GiantMoldorm_DrawSegment_C_OrTail(k, 0x30);
}

void SpriteDraw_Moldorm_Eyeballs(int k, PrepOamCoordsRet *info) {  // 9ddb9e
  static const int16 kGiantMoldorm_Eye_X[16] = {16, 15, 12, 6, 0, -6, -12, -13, -16, -13, -12, -6, 0, 6, 12, 15};
  static const int16 kGiantMoldorm_Eye_Y[16] = {0, 6, 12, 15, 16, 15, 12, 6, 0, -6, -12, -13, -16, -13, -12, -6};
  static const uint8 kGiantMoldorm_Eye_Char[16] = {0xaa, 0xaa, 0xa8, 0xa8, 0x8a, 0x8a, 0xa8, 0xa8, 0xaa, 0xaa, 0xa8, 0xa8, 0x8a, 0x8a, 0xa8, 0xa8};
  static const uint8 kGiantMoldorm_Eye_Flags[16] = {0, 0, 0, 0, 0x80, 0x80, 0x40, 0x40, 0x40, 0x40, 0xc0, 0xc0, 0, 0, 0x80, 0x80};
  OamEnt *oam = GetOamCurPtr();
  int r7 = sprite_F[k] ? frame_counter : 0;
  int r6 = sprite_D[k] - 1;
  for (int i = 1; i >= 0; i--, oam++, r6 += 2) {
    uint16 x = info->x + kGiantMoldorm_Eye_X[r6 & 0xf];
    uint16 y = info->y + kGiantMoldorm_Eye_Y[r6 & 0xf];
    SetOamHelper0(oam, x, y, kGiantMoldorm_Eye_Char[(r6 + r7) & 0xf], info->flags | kGiantMoldorm_Eye_Flags[(r6 + r7) & 0xf], 2);
  }
}

void Sprite_ScheduleBossForDeath(int k) {  // 9ddc16
  sprite_state[k] = 4;
  sprite_A[k] = 0;
  sprite_delay_main[k] = 224;
}

void Sprite_MakeBossExplosion(int k) {  // 9ddc2a
  SpriteSfx_QueueSfx2WithPan(k, 0xc);
  Sprite_MakeBossDeathExplosion_NoSound(k);
}

void Sprite_MakeBossDeathExplosion_NoSound(int k) {  // 9ddc30
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0x00, &info);
  if (j >= 0) {
    load_chr_halfslot_even_odd = 11;
    sprite_state[j] = 4;
    sprite_flags2[j] = 3;
    sprite_oam_flags[j] = 12;
    Sprite_SetX(j, cur_sprite_x);
    Sprite_SetY(j, cur_sprite_y);
    sprite_delay_main[j] = 31;
    sprite_A[j] = 31;
    sprite_floor[j] = 2;
  }
}

void Vulture_Draw(int k) {  // 9ddd5e
  static const DrawMultipleData kVulture_Dmd[8] = {
    {-8, 0, 0x0086, 2},
    { 8, 0, 0x4086, 2},
    {-8, 0, 0x0080, 2},
    { 8, 0, 0x4080, 2},
    {-8, 0, 0x0082, 2},
    { 8, 0, 0x4082, 2},
    {-8, 0, 0x0084, 2},
    { 8, 0, 0x4084, 2},
  };
  PrepOamCoordsRet info;
  Sprite_DrawMultiple(k, &kVulture_Dmd[sprite_graphics[k] * 2], 2, &info);
  SpriteDraw_Shadow(k, &info);

}

void Sprite_Raven(int k) {  // 9ddd85
  static const uint8 kRaven_AscendTime[2] = {16, 248};
  int j;
  bool fleeing = false;
  sprite_obj_prio[k] |= 0x30;
  SpriteDraw_SingleLarge(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (Sprite_ReturnIfRecoiling(k))
    return;
  Sprite_CheckDamageToAndFromLink(k);
  Sprite_MoveXY(k);
  switch (sprite_ai_state[k]) {
  case 0: { // inwait
    PairU8 r = Sprite_IsRightOfLink(k);
    sprite_oam_flags[k] = sprite_oam_flags[k] & ~0x40 | r.a * 0x40;
    int x = link_x_coord - cur_sprite_x;
    int y = link_y_coord - cur_sprite_y;
    if ((uint16)(x + 0x50 + (x >= 0)) < 0xa0 && (uint16)(y + 0x58 + (y >= 0)) < 0xa0) {
      sprite_ai_state[k]++;
      sprite_delay_main[k] = 24;
      SpriteSfx_QueueSfx3WithPan(k, 0x1e);
    }
    break;
  }
  case 1:  // ascend
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k]++;
      int j = sprite_A[k];
      sprite_delay_main[k] = kRaven_AscendTime[j];
      Sprite_ApplySpeedTowardsLink(k, 32);
    }
    sprite_z[k]++;
    sprite_graphics[k] = (frame_counter >> 1 & 1) + 1;
    break;
  case 2:  // attack
    if (!sprite_delay_main[k] && !(is_in_dark_world && sprite_A[k]) )
      sprite_ai_state[k]++;
fly:
    if (!((k ^ frame_counter) & 1)) {
      ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLink(k, fleeing ? 48 : 32);
      if (fleeing)
        pt.x = -pt.x, pt.y = -pt.y;
      if (sprite_x_vel[k] - pt.x)
        sprite_x_vel[k] += sign8(sprite_x_vel[k] - pt.x) ? 1 : -1;
      if (sprite_y_vel[k] - pt.y)
        sprite_y_vel[k] += sign8(sprite_y_vel[k] - pt.y) ? 1 : -1;
    }
    sprite_graphics[k] = (frame_counter >> 1 & 1) + 1;
    j = (sprite_x_vel[k] >> 7) & 1;
    sprite_oam_flags[k] = sprite_oam_flags[k] & ~0x40 | j * 0x40;
    break;
  case 3:  // flee
    fleeing = true;
    goto fly;
  }
}

void Vitreous_SpawnSmallerEyes(int k) {  // 9ddecb
  sprite_G[k] = 9;
  sprite_graphics[k] = 4;

  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamicallyEx(k, 0x4, &info, 13);

  static const int8 kVitreous_SpawnSmallerEyes_X[13] = {8, 22, -8, -22, 0, 14, 19, 33, 26, -14, -19, -33, -26};
  static const int8 kVitreous_SpawnSmallerEyes_Y[13] = {-8, -12, -8, -12, 0, -20, -1, -12, -24, -20, -1, -12, -24};
  static const int8 kVitreous_SpawnSmallerEyes_Gfx[13] = {1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  for (j = 13; j != 0; j--) {
    sprite_state[j] = 9;
    sprite_type[j] = 0xbe;
    SpritePrep_LoadProperties(j);
    sprite_floor[j] = 0;
    Sprite_SetX(j, info.r0_x + kVitreous_SpawnSmallerEyes_X[j - 1]);
    Sprite_SetY(j, info.r2_y + kVitreous_SpawnSmallerEyes_Y[j - 1] + 32);
    sprite_A[j] = sprite_x_lo[j];
    sprite_B[j] = sprite_x_hi[j];
    sprite_C[j] = sprite_y_lo[j];
    sprite_D[j] = sprite_y_hi[j];
    sprite_ignore_projectile[j] = sprite_graphics[j] = kVitreous_SpawnSmallerEyes_Gfx[j - 1];
    sprite_subtype2[j] = (j - 1) * 8 + GetRandomNumber();
  }
}

void Sprite_C0_Catfish(int k) {  // 9ddf49
  if (sprite_A[k] & 0x80)
    Sprite_Catfish_SplashOfWater(k);
  else if (sprite_A[k] == 0)
    Catfish_BigFish(k);
  else
    Sprite_Catfish_QuakeMedallion(k);
}

void Sprite_Catfish_QuakeMedallion(int k) {  // 9ddf54
  if (!sprite_z[k]) {
    SpriteDraw_WaterRipple_WithOamAdjust(k);
    if (!submodule_index && Sprite_CheckDamageToLink_same_layer(k)) {
      sprite_state[k] = 0;
      item_receipt_method = 0;
      Link_ReceiveItem(sprite_A[k], 0);
    }
  }
  if (sprite_delay_aux3[k])
    Oam_AllocateFromRegionC(8);
  SpriteDraw_SingleLarge(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_MoveXYZ(k);
  sprite_z_vel[k] -= 2;
  if (sign8(sprite_z[k])) {
    sprite_z[k] = 0;
    sprite_x_vel[k] = (int8)sprite_x_vel[k] >> 1;
    sprite_y_vel[k] = (int8)sprite_y_vel[k] >> 1;
    int j = sprite_ai_state[k];
    if (j == 4) {
      sprite_x_vel[k] = 0;
      sprite_y_vel[k] = 0;
      sprite_z_vel[k] = 0;
    } else {
      sprite_ai_state[k]++;
      static const uint8 kStandaloneItem_Zvel[4] = {0x20, 0x10, 8, 0};
      sprite_z_vel[k] = kStandaloneItem_Zvel[j];
      if (j < 2 && (j = Sprite_SpawnWaterSplash(k)) >= 0)
        sprite_delay_main[j] = 16;
    }
  }
}

void Catfish_BigFish(int k) {  // 9ddfd1
  GreatCatfish_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  switch(sprite_ai_state[k]) {
  case 0:  // AwaitSpriteThrownInCircle
    for (int j = 15; j >= 0; j--) {
      if (j == k || sprite_state[j] != 3)
        continue;
      if ((uint16)(cur_sprite_x - Sprite_GetX(j) + 32) < 64 && (uint16)(cur_sprite_y - Sprite_GetY(j) + 32) < 64) {
        sprite_ai_state[k] = 1;
        sprite_delay_main[k] = 255;
        return;
      }
    }
    break;
  case 1: { // RumbleBeforeEmergence
    int j = sprite_delay_main[k];
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k] = 2;
      sprite_delay_main[k] = 255;
      bg1_x_offset = 0;
      sound_effect_ambient = 5;
      sprite_z_vel[k] = 48;
      sprite_x_vel[k] = 0;
      Catfish_SpawnPlop(k);
    } else if (sprite_delay_main[k] < 0xc0) {
      if (sprite_delay_main[k] == 0xbf)
        sound_effect_ambient = 7;
      bg1_x_offset = (j & 1) ? -1 : 1;
      flag_is_link_immobilized = 1;
    }
    break;
  }
  case 2: { // Emerge
    static const uint8 kGreatCatfish_Emerge_Gfx[16] = {1, 2, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 0, 0, 0, 0};
    sprite_subtype2[k]++;
    Sprite_MoveXYZ(k);
    sprite_z_vel[k] -= 2;
    if (sprite_z_vel[k] == (uint8)(-48))
      Catfish_SpawnPlop(k);
    if (sign8(sprite_z[k])) {
      sprite_z[k] = 0;
      sprite_ai_state[k]++;
      sprite_delay_main[k] = 255;
    }
    sprite_graphics[k] = kGreatCatfish_Emerge_Gfx[sprite_subtype2[k] >> 2];
    break;
  }
  case 3: { // ConversateThenSubmerge
    int j = sprite_delay_main[k];
    static const uint8 kGreatCatfish_Conversate_Gfx[20] = { 0, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 6, 6 };
    if (j == 0) {
      sprite_state[k] = 0;
    } else {
      if (j == 160 || j == 252 || j == 4) {
        Sprite_SpawnWaterSplash(k);
      } else if (j == 10) {
        Catfish_SpawnPlop(k);
      } else if (j == 96) {
        flag_is_link_immobilized = 0;
        dialogue_message_index = link_item_quake_medallion ? 0x12b : 0x12a;
        Sprite_ShowMessageMinimal();
        return;
      } else if (j == 80) {
        if (link_item_quake_medallion) {
          if (GetRandomNumber() & 1)
            Sprite_SpawnBomb(k);
          else
            Sprite_SpawnFireball(k);
        } else {
          Catfish_RegurgitateMedallion(k);
        }
      }
      if (j < 160)
        sprite_graphics[k] = kGreatCatfish_Conversate_Gfx[j >> 3];
    }
    break;
  }
  }
}

int Sprite_SpawnBomb(int k) {  // 9de144
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0x4a, &info);
  if (j >= 0) {
    Sprite_SetSpawnedCoordinates(j, &info);
    Sprite_TransmuteToBomb(j);
    sprite_delay_aux1[j] = 80;
    sprite_x_vel[j] = 24;
    sprite_z_vel[j] = 48;
  }
  return j;
}

void Catfish_RegurgitateMedallion(int k) {  // 9de16c
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0xc0, &info);
  if (j >= 0) {
    Sprite_SetSpawnedCoordinates(j, &info);
    sprite_x_vel[j] = 24;
    sprite_z_vel[j] = 48;
    sprite_A[j] = 17;
    SpriteSfx_QueueSfx2WithPan(j, 0x20);
    sprite_flags2[j] = 0x83;
    sprite_flags3[j] = 0x58;
    sprite_oam_flags[j] = 0x58 & 0xf;
    DecodeAnimatedSpriteTile_variable(0x1c);
  }
}

void Sprite_Zora_RegurgitateFlippers(int k) {  // 9de1aa
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0xc0, &info);
  if (j < 0)
    return;
  Sprite_SetSpawnedCoordinates(j, &info);
  sprite_z_vel[j] = 32;
  sprite_y_vel[j] = 16;
  sprite_A[j] = 30;
  SpriteSfx_QueueSfx2WithPan(j, 0x20);
  sprite_flags2[j] = 0x83;
  sprite_flags3[j] = 0x54;
  sprite_oam_flags[j] = 0x54 & 15;
  sprite_delay_aux3[j] = 0x30;
  DecodeAnimatedSpriteTile_variable(0x11);
}

void Catfish_SpawnPlop(int k) {  // 9de1ed
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0xec, &info);
  if (j >= 0) {
    Sprite_SetSpawnedCoordinates(j, &info);
    sprite_state[j] = 3;
    sprite_delay_main[j] = 15;
    sprite_ai_state[j] = 0;
    sprite_flags2[j] = 3;
    SpriteSfx_QueueSfx2WithPan(k, 0x28);
  }
}

int Sprite_SpawnWaterSplash(int k) {  // 9de21c
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0xc0, &info);
  if (j >= 0) {
    Sprite_SetSpawnedCoordinates(j, &info);
    sprite_A[j] = 0x80;
    sprite_flags2[j] = 2;
    sprite_ignore_projectile[j] = 2;
    sprite_oam_flags[j] = 4;
    sprite_delay_main[j] = 31;
  }
  return j;
}

void GreatCatfish_Draw(int k) {  // 9de320
  static const DrawMultipleData kGreatCatfish_Dmd[28] = {
    {-4,  4, 0x008c, 2},
    { 4,  4, 0x008d, 2},
    {-4,  4, 0x008c, 2},
    { 4,  4, 0x008d, 2},
    {-4, -4, 0x008c, 2},
    { 4, -4, 0x008d, 2},
    {-4,  4, 0x009c, 2},
    { 4,  4, 0x009d, 2},
    {-4, -4, 0x408d, 2},
    { 4, -4, 0x408c, 2},
    {-4,  4, 0x409d, 2},
    { 4,  4, 0x409c, 2},
    {-4, -4, 0xc09d, 2},
    { 4, -4, 0xc09c, 2},
    {-4,  4, 0xc08d, 2},
    { 4,  4, 0xc08c, 2},
    {-4,  4, 0xc09d, 2},
    { 4,  4, 0xc09c, 2},
    {-4,  4, 0xc09d, 2},
    { 4,  4, 0xc09c, 2},
    { 0,  8, 0x00bd, 0},
    { 8,  8, 0x40bd, 0},
    { 8,  8, 0x40bd, 0},
    { 8,  8, 0x40bd, 0},
    {-8,  0, 0x0086, 2},
    { 8,  0, 0x4086, 2},
    { 8,  0, 0x4086, 2},
    { 8,  0, 0x4086, 2},
  };
  if (sprite_graphics[k])
    Sprite_DrawMultiple(k, &kGreatCatfish_Dmd[(sprite_graphics[k] - 1) * 4], 4, NULL);
}

void Sprite_Catfish_SplashOfWater(int k) {  // 9de37d
  static const DrawMultipleData kWaterSplash_Dmd[8] = {
    {-8, -4, 0x0080, 0},
    {18, -7, 0x0080, 0},
    {-5, -2, 0x00bf, 0},
    {15, -4, 0x40af, 0},
    { 0, -4, 0x00e7, 2},
    { 0, -4, 0x00e7, 2},
    { 0, -4, 0x00c0, 2},
    { 0, -4, 0x00c0, 2},
  };
  if (!sprite_delay_main[k])
    sprite_state[k] = 0;
  Sprite_DrawMultiple(k, &kWaterSplash_Dmd[(sprite_delay_main[k] >> 3) * 2], 2, NULL);
}

void Sprite_BF_Lightning(int k) {  // 9de3ed
  static const uint8 kSpriteLightning_Gfx[8] = {0, 1, 2, 3, 0, 1, 2, 3};
  static const uint8 kSpriteLightning_OamFlags[8] = {0, 0, 0, 0, 0x40, 0x40, 0x40, 0x40};
  static const int8 kSpriteLightning_Xoff[64] = {
    -15,  0,  0, -15,  0, -15, -15,  0, -15,  0,  0, -15,  0, -15, -15,  0,
    0, 15, 15,   0, 15,   0,   0, 15,   0, 15, 15,   0, 15,   0,   0, 15,
    0, 15, 15,   0, 15,   0,   0, 15,   0, 15, 15,   0, 15,   0,   0, 15,
    -15,  0,  0, -15,  0, -15, -15,  0, -15,  0,  0, -15,  0, -15, -15,  0,
  };
  int j = sprite_A[k];
  sprite_oam_flags[k] = sprite_oam_flags[k] & 0xb1 | kSpriteLightning_OamFlags[j] | frame_counter << 1 & 14;
  sprite_graphics[k] = kSpriteLightning_Gfx[j] + (BYTE(dungeon_room_index2) == 0x20 ? 4 : 0);
  SpriteDraw_SingleLarge(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (sprite_delay_main[k])
    return;
  Lightning_SpawnGarnish(k);
  sprite_delay_main[k] = 2;
  Sprite_SetY(k, Sprite_GetY(k) + 16);
  if ((uint8)(sprite_y_lo[k] - BG2VOFS_copy2) >= 0xd0) {
    sprite_state[k] = 0;
    return;
  }
  int rr = GetRandomNumber() & 7;
  Sprite_SetX(k, Sprite_GetX(k) + kSpriteLightning_Xoff[sprite_A[k] << 3 | rr]);
  sprite_A[k] = rr;
}

void Lightning_SpawnGarnish(int k) {  // 9de475
  int j = GarnishAllocOverwriteOld();
  garnish_type[j] = 9;
  garnish_active = 9;
  garnish_sprite[j] = sprite_A[k];
  garnish_x_lo[j] = sprite_x_lo[k];
  garnish_x_hi[j] = sprite_x_hi[k];
  int y = Sprite_GetY(k) + 16;
  garnish_y_lo[j] = y;
  garnish_y_hi[j] = y >> 8;
  garnish_countdown[j] = 32;
}

void Sprite_BD_Vitreous(int k) {  // 9de4c8
  if (sprite_delay_aux4[k])
    sprite_graphics[k] = 3;
  Vitreous_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Vitreous_SetMinionsForth(k);
  Sprite_CheckDamageToAndFromLink(k);
  switch(sprite_ai_state[k]) {
  case 0:  // dormant
    byte_7E0FF8 = 0;
    sprite_F[k] = 0;
    sprite_flags3[k] |= 64;
    if (!(frame_counter & 1) && !--sprite_A[k]) {
      sprite_flags3[k] &= ~0x40;
      sprite_delay_aux4[k] = 16;
      sprite_ai_state[k] = 1;
      sprite_delay_main[k] = 128;
      if (!sprite_G[k]) {
        sprite_ai_state[k] = 2;
        sprite_delay_main[k] = 64;
        sprite_ignore_projectile[k] = 0;
        sound_effect_1 = 0x35;
        return;
      }
    }
    sprite_graphics[k] = frame_counter & 0x30 ? 4 : 5;
    break;
  case 1:  // spew lighting
    sprite_F[k] = 0;
    if (!sprite_delay_main[k]) {
      static const uint8 kVitreous_AfromG[10] = {0x20, 0x20, 0x20, 0x40, 0x60, 0x80, 0xa0, 0xc0, 0xe0, 0};
      sprite_delay_aux4[k] = 16;
      sprite_ai_state[k] = 0;
      sprite_A[k] = kVitreous_AfromG[sprite_G[k]];
    } else {
      Vitreous_Animate(k, sprite_delay_main[k]);
    }
    break;
  case 2:  // pursue player
    Vitreous_Animate(k, 0x8B);
    if (Sprite_ReturnIfRecoiling(k))
      return;
    if (sprite_delay_main[k]) {
      static const int8 kVitreous_Xvel[2] = {8, -8};
      sprite_x_vel[k] = kVitreous_Xvel[(sprite_delay_main[k] & 2) >> 1];
      Sprite_MoveX(k);
    } else {
      Sprite_MoveXYZ(k);
      Sprite_CheckTileCollision(k);
      sprite_z_vel[k] -= 2;
      if (sign8(sprite_z[k])) {
        sprite_z[k] = 0;
        sprite_z_vel[k] = 32;
        Sprite_ApplySpeedTowardsLink(k, 16);
        SpriteSfx_QueueSfx2WithPan(k, 0x21);
      }
    }
    break;
  }
}

void Vitreous_Animate(int k, uint8 a) {  // 9de563
  static const int8 kVitreous_Animate_Gfx[2] = {2, 1};
  if (a == 0x40 || a == 0x41 || a == 0x42)
    Sprite_SpawnLightning(k);
  sprite_graphics[k] = 0;
  PairU8 pair = Sprite_IsRightOfLink(k);
  if ((uint8)(pair.b + 16) >= 32)
    sprite_graphics[k] = kVitreous_Animate_Gfx[pair.a];
}

void Vitreous_SetMinionsForth(int k) {  // 9de5da
  static const uint8 kVitreous_WhichToActivate[16] = {5, 6, 7, 8, 9, 10, 11, 12, 13, 5, 6, 7, 8, 9, 10, 11};
  if (!(++sprite_subtype2[k] & 63)) {
    int j = kVitreous_WhichToActivate[GetRandomNumber() & 15];
    if (sprite_ai_state[j] == 0) {
      sprite_ai_state[j] = 1;
      sound_effect_1 = 0x15;
    } else {
      sprite_subtype2[k]--;
    }
  }
}

void Sprite_SpawnLightning(int k) {  // 9de612
  static const int8 kAgahnim_Lighting_X[8] = {-8, 8, 8, -8, 8, -8, -8, 8};

  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0xBF, &info), i;
  if (j >= 0) {
    sound_effect_2 = 0x26;
    Sprite_SetSpawnedCoordinates(j, &info);
    sprite_A[j] = i = GetRandomNumber() & 7;
    int t = info.r0_x + (uint16)kAgahnim_Lighting_X[i];
    Sprite_SetX(j, info.r0_x + kAgahnim_Lighting_X[i]);
    sprite_y_lo[j] = info.r2_y + 12 + (t >> 16);
    sprite_delay_main[j] = 2;
    intro_times_pal_flash = 32;
  }
}

void Vitreous_Draw(int k) {  // 9de716
  static const DrawMultipleData kVitreous_Dmd[24] = {
    {-8, -8, 0x01c0, 2},
    { 8, -8, 0x41c0, 2},
    {-8,  8, 0x01e0, 2},
    { 8,  8, 0x41e0, 2},
    {-8, -8, 0x01c8, 2},
    { 8, -8, 0x01ca, 2},
    {-8,  8, 0x01e8, 2},
    { 8,  8, 0x01ea, 2},
    {-8, -8, 0x41ca, 2},
    { 8, -8, 0x41c8, 2},
    {-8,  8, 0x41ea, 2},
    { 8,  8, 0x41e8, 2},
    {-8, -8, 0x01c2, 2},
    { 8, -8, 0x41c2, 2},
    {-8,  8, 0x01e2, 2},
    { 8,  8, 0x41e2, 2},
    {-8, -8, 0x01c4, 2},
    { 8, -8, 0x41c4, 2},
    {-8,  8, 0x01e4, 2},
    { 8,  8, 0x41e4, 2},
    {-7, -7, 0x01c4, 2},
    { 7, -7, 0x41c4, 2},
    {-7,  7, 0x01e4, 2},
    { 7,  7, 0x41e4, 2},
  };
  if (sprite_ai_state[k] == 2 && sprite_state[k] == 9)
    oam_cur_ptr = 0x800, oam_ext_cur_ptr = 0xa20;
  Sprite_DrawMultiple(k, &kVitreous_Dmd[sprite_graphics[k] * 4], 4, NULL);
  if (sprite_ai_state[k] == 2) {
    sprite_obj_prio[k] &= ~0xe;
    Sprite_DrawLargeShadow2(k);
  }
}

void Sprite_BE_VitreousEye(int k) {  // 9de773
  static const int8 kSprite_Vitreolus_Dx[4] = {1, 0, -1, 0};
  static const int8 kSprite_Vitreolus_Dy[4] = {0, 1, 0, -1};
  int j = sprite_subtype2[k] >> 4 & 3;
  cur_sprite_x += kSprite_Vitreolus_Dx[j];
  cur_sprite_y += kSprite_Vitreolus_Dy[j];
  SpriteDraw_SingleLarge(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  sprite_subtype2[k]++;
  if (sprite_graphics[k])
    return;
  Sprite_CheckDamageFromLink(k);
  Sprite_CheckDamageToLink(k);
  if (sprite_F[k] == 14)
    sprite_F[k] = 5;
  switch (sprite_ai_state[k]) {
  case 0:  // target player
    sprite_G[k] = link_x_coord;
    sprite_head_dir[k] = link_x_coord >> 8;
    sprite_anim_clock[k] = link_y_coord;
    sprite_subtype[k] = link_y_coord >> 8;
    break;
  case 1:  // pursue player
    if (Sprite_ReturnIfRecoiling(k))
      return;
    if (!((k ^ frame_counter) & 1)) {
      uint16 x = sprite_head_dir[k] << 8 | sprite_G[k];
      uint16 y = sprite_subtype[k] << 8 | sprite_anim_clock[k];
      ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLocation(k, x, y, 16);
      sprite_x_vel[k] = pt.x;
      sprite_y_vel[k] = pt.y;
    }
    Sprite_MoveXY(k);
    if ((uint8)(sprite_G[k] - sprite_x_lo[k] + 4) < 8 && (uint8)(sprite_anim_clock[k] - sprite_y_lo[k] + 4) < 8)
      sprite_ai_state[k] = 2;
    break;
  case 2:  // return
    if (Sprite_ReturnIfRecoiling(k))
      return;
    if (!((k ^ frame_counter) & 1)) {
      uint16 x = sprite_A[k] | sprite_B[k] << 8;
      uint16 y = sprite_C[k] | sprite_D[k] << 8;
      ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLocation(k, x, y, 16);
      sprite_x_vel[k] = pt.x;
      sprite_y_vel[k] = pt.y;
    }
    Sprite_MoveXY(k);
    if ((uint8)(sprite_A[k] - sprite_x_lo[k] + 4) < 8 && (uint8)(sprite_C[k] - sprite_y_lo[k] + 4) < 8) {
      sprite_x_lo[k] = sprite_A[k];
      sprite_x_hi[k] = sprite_B[k];
      sprite_y_lo[k] = sprite_C[k];
      sprite_y_hi[k] = sprite_D[k];
      sprite_ai_state[k] = 0;
    }
    break;
  }
}

void HelmasaurFireball_TriSplit(int k) {  // 9deed3
  static const int8 kHelmasaurFireball_TriSplit_Xvel[3] = {0, 28, -28};
  static const int8 kHelmasaurFireball_TriSplit_Yvel[3] = {-32, 24, 24};
  static const uint8 kHelmasaurFireball_TriSplit_Delay[6] = {32, 80, 128, 32, 80, 128};

  SpriteSfx_QueueSfx3WithPan(k, 0x36);
  sprite_state[k] = 0;

  byte_7E0FB6 = GetRandomNumber();
  for (int i = 2; i >= 0; i--) {
    SpriteSpawnInfo info;
    int j = Sprite_SpawnDynamically(k, 0x70, &info);
    if (j >= 0) {
      Sprite_SetSpawnedCoordinates(j, &info);
      sprite_x_vel[j] = kHelmasaurFireball_TriSplit_Xvel[i];
      sprite_y_vel[j] = kHelmasaurFireball_TriSplit_Yvel[i];
      sprite_ai_state[j] = 3;
      sprite_ignore_projectile[j] = 3;
      sprite_delay_main[j] = kHelmasaurFireball_TriSplit_Delay[(byte_7E0FB6 & 3) + i];
      sprite_head_dir[j] = 0;
      sprite_graphics[j] = 1;
    }
  }
  tmp_counter = -1;
}

void HelmasaurFireball_QuadSplit(int k) {  // 9def3d
  static const int8 kHelmasaurFireball_QuadSplit_Xvel[4] = {32, 32, -32, -32};
  static const int8 kHelmasaurFireball_QuadSplit_Yvel[4] = {-32, 32, -32, 32};
  SpriteSfx_QueueSfx3WithPan(k, 0x36);
  sprite_state[k] = 0;
  for (int i = 3; i >= 0; i--) {
    SpriteSpawnInfo info;
    int j = Sprite_SpawnDynamically(k, 0x70, &info);
    if (j >= 0) {
      Sprite_SetSpawnedCoordinates(j, &info);
      sprite_x_vel[j] = kHelmasaurFireball_QuadSplit_Xvel[i];
      sprite_y_vel[j] = kHelmasaurFireball_QuadSplit_Yvel[i];
      sprite_ai_state[j] = 4;
      sprite_ignore_projectile[j] = 4;
    }
  }
  tmp_counter = -1;
}

void Sprite_ArmosCrusher(int k) {  // 9def7e
  sprite_oam_flags[k] = 7;
  bg1_y_offset = sprite_delay_aux4[k] ? (sprite_delay_aux4[k] & 1 ? -1 : 1) : 0;
  switch (sprite_G[k]) {
  case 0:  // retarget
    Sprite_CheckDamageToAndFromLink(k);
    if (!(sprite_delay_main[k] | sprite_z[k])) {
      Sprite_ApplySpeedTowardsLink(k, 32);
      sprite_z_vel[k] = 32;
      sprite_G[k]++;
      sprite_B[k] = link_x_coord;
      sprite_C[k] = link_x_coord >> 8;
      sprite_E[k] = link_y_coord;
      sprite_head_dir[k] = link_y_coord >> 8;
      SpriteSfx_QueueSfx2WithPan(k, 0x20);
    }
    break;
  case 1:  // approach target
    sprite_z_vel[k] += 3;
    if (Sprite_CheckTileCollision(k))
      goto advance;
    Sprite_Get16BitCoords(k);
    uint16 x, y;
    x = sprite_B[k] | sprite_C[k] << 8;
    y = sprite_E[k] | sprite_head_dir[k] << 8;
    if ((uint16)(x - cur_sprite_x + 16) < 32 && (uint16)(y - cur_sprite_y + 16) < 32) {
advance:
      sprite_G[k]++;
      sprite_delay_main[k] = 16;
      sprite_x_vel[k] = 0;
      sprite_y_vel[k] = 0;
    }
    break;
  case 2:  // hover
    sprite_z_vel[k] = 0;
    if (!sprite_delay_main[k])
      sprite_G[k]++;
    break;
  case 3:  // crush
    sprite_z_vel[k] = -104;
    if (!sign8(sprite_z[k])) {
      sprite_delay_main[k] = 32;
      sprite_delay_aux4[k] = 32;
      sprite_G[k] = 0;
      SpriteSfx_QueueSfx2WithPan(k, 0xc);
    }
    break;
  }
}

void Sprite_EvilBarrier(int k) {  // 9df06b
  EvilBarrier_Draw(k);
  if (sprite_graphics[k] == 4)
    return;

  sprite_graphics[k] = frame_counter >> 1 & 3;
  if (Sprite_ReturnIfInactive(k))
    return;
  if (Sprite_CheckDamageFromLink(k) && link_sword_type < 2) {
    sprite_hit_timer[k] = 0;
    Sprite_AttemptDamageToLinkPlusRecoil(k);
    if (!countdown_for_blink)
      link_electrocute_on_touch = 64;
  }

  if ((uint16)(link_y_coord - cur_sprite_y + 8) < 24 &&
      (uint16)(link_x_coord - cur_sprite_x + 32) < 64 &&
      sign8(link_actual_vel_y - 1)) {
    link_electrocute_on_touch = 64;
    link_incapacitated_timer = 12;
    link_auxiliary_state = 1;
    link_give_damage = 2;
    link_actual_vel_x = 0;
    link_actual_vel_y = 48;
  }
}

void EvilBarrier_Draw(int k) {  // 9df249
  static const DrawMultipleData kEvilBarrier_Dmd[45] = {
    {  0,  0, 0x00e8, 2},
    {-29,  3, 0x00ca, 0},
    {-29, 11, 0x00da, 0},
    { 37,  3, 0x40ca, 0},
    { 37, 11, 0x40da, 0},
    {-24, -2, 0x00e6, 2},
    { -8, -2, 0x00e6, 2},
    {  8, -2, 0x40e6, 2},
    { 24, -2, 0x40e6, 2},
    {  0,  0, 0x00cc, 2},
    {-29,  3, 0x00cb, 0},
    {-29, 11, 0x00db, 0},
    { 37,  3, 0x40cb, 0},
    { 37, 11, 0x40db, 0},
    {  0,  0, 0x00cc, 2},
    {  0,  0, 0x00cc, 2},
    {  0,  0, 0x00cc, 2},
    {  0,  0, 0x00cc, 2},
    {  0,  0, 0x00cc, 2},
    {-29,  3, 0x00cb, 0},
    {-29, 11, 0x00db, 0},
    { 37,  3, 0x40cb, 0},
    { 37, 11, 0x40db, 0},
    {-24, -2, 0x80e6, 2},
    { -8, -2, 0x80e6, 2},
    {  8, -2, 0xc0e6, 2},
    { 24, -2, 0xc0e6, 2},
    {  0,  0, 0x00e8, 2},
    {-29,  3, 0x00ca, 0},
    {-29, 11, 0x00da, 0},
    { 37,  3, 0x40ca, 0},
    { 37, 11, 0x40da, 0},
    {  0,  0, 0x00e8, 2},
    {  0,  0, 0x00e8, 2},
    {  0,  0, 0x00e8, 2},
    {  0,  0, 0x00e8, 2},
    {-29,  3, 0x00cb, 0},
    {-29, 11, 0x00db, 0},
    { 37,  3, 0x40cb, 0},
    { 37, 11, 0x40db, 0},
    { 37, 11, 0x40db, 0},
    { 37, 11, 0x40db, 0},
    { 37, 11, 0x40db, 0},
    { 37, 11, 0x40db, 0},
    { 37, 11, 0x40db, 0},
  };
  cur_sprite_y += 8;
  Sprite_DrawMultiple(k, &kEvilBarrier_Dmd[sprite_graphics[k] * 9], 9, NULL);
  Sprite_Get16BitCoords(k);
}

void SpriteDraw_Antfairy(int k) {  // 9df395
  static const DrawMultipleData kDrawFourAroundOne_Dmd[30] = {
    { 4,  2, 0x02e1, 0},
    { 4, -3, 0x02e3, 0},
    {-1,  2, 0x02e3, 0},
    { 9,  2, 0x02e3, 0},
    { 4,  7, 0x02e3, 0},
    { 4,  2, 0x02e1, 0},
    { 3, -3, 0x02e3, 0},
    { 9,  1, 0x02e3, 0},
    {-1,  3, 0x02e3, 0},
    { 5,  7, 0x02e3, 0},
    { 4,  2, 0x02e1, 0},
    { 1, -3, 0x02e3, 0},
    { 9, -1, 0x02e3, 0},
    {-1,  5, 0x02e3, 0},
    { 7,  7, 0x02e3, 0},
    { 4,  2, 0x02e1, 0},
    { 0, -2, 0x02e3, 0},
    { 8, -2, 0x02e3, 0},
    { 0,  6, 0x02e3, 0},
    { 8,  6, 0x02e3, 0},
    { 4,  2, 0x02e1, 0},
    { 7, -3, 0x02e3, 0},
    {-1, -1, 0x02e3, 0},
    { 9,  5, 0x02e3, 0},
    { 1,  7, 0x02e3, 0},
    { 4,  2, 0x02e1, 0},
    { 5, -3, 0x02e3, 0},
    {-1,  1, 0x02e3, 0},
    { 9,  3, 0x02e3, 0},
    { 3,  7, 0x02e3, 0},
  };
  if (!(++sprite_subtype2[k] & 1 | submodule_index | flag_unk1)) {
    if (++sprite_graphics[k] == 6)
      sprite_graphics[k] = 0;
  }
  Sprite_DrawMultiple(k, &kDrawFourAroundOne_Dmd[sprite_graphics[k] * 5], 5, NULL);
}

void Toppo_Flustered(int k) {  // 9df3d4
  sprite_flags2[k] = 130;
  sprite_ignore_projectile[k] = 130;
  sprite_flags3[k] = 73;
  if (!sprite_subtype[k]) {
    if (Sprite_CheckDamageToLink(k)) {
      dialogue_message_index = 0x174;
      Sprite_ShowMessageMinimal();
      sprite_subtype[k] = 1;
    }
  } else if (sprite_subtype[k] < 16) {
    sprite_subtype[k]++;
  } else if (sprite_subtype[k] == 16) {
    sprite_flags5[k] = 0;
    sprite_state[k] = 6;
    sprite_delay_main[k] = 15;
    sprite_flags2[k] += 4;
    SpriteSfx_QueueSfx2WithPan(k, 0x15);
    SpriteSpawnInfo info;
    int j = Sprite_SpawnDynamically(k, 0x4d, &info);
    if (j >= 0) {
      Sprite_SetSpawnedCoordinates(j, &info);
      ForcePrizeDrop(j, 6, 6);
    }
    sprite_subtype[k]++;
  }
  sprite_graphics[k] = ((++sprite_subtype2[k] & 4) >> 2) + 3;
}

void Goriya_Draw(int k) {  // 9df589
  static const DrawMultipleData kGoriya_Dmd2[3] = {
    {10, 4, 0x4077, 0},
    {-2, 4, 0x0077, 0},
    { 4, 4, 0x0076, 0},
  };
  static const DrawMultipleData kGoriya_Dmd[32] = {
    {-4, -8, 0x0044, 2},
    {12, -8, 0x4044, 0},
    {-4,  8, 0x0064, 0},
    { 4,  0, 0x4054, 2},
    {-4, -8, 0x0044, 2},
    {12, -8, 0x4044, 0},
    {-4,  8, 0x4074, 0},
    { 4,  0, 0x4062, 2},
    {-4, -8, 0x0044, 0},
    { 4, -8, 0x4044, 2},
    {-4,  0, 0x0062, 2},
    {12,  8, 0x4064, 0},
    {-4, -8, 0x0046, 2},
    {12, -8, 0x4046, 0},
    {-4,  8, 0x0066, 0},
    { 4,  0, 0x4056, 2},
    {-4, -8, 0x0046, 2},
    {12, -8, 0x4046, 0},
    {-4,  8, 0x4075, 0},
    { 4,  0, 0x406a, 2},
    {-4, -8, 0x0046, 0},
    { 4, -8, 0x4046, 2},
    {-4,  0, 0x006a, 2},
    {12,  8, 0x0075, 0},
    {-2, -8, 0x004e, 2},
    { 0,  0, 0x006c, 2},
    {-2, -7, 0x004e, 2},
    { 0,  0, 0x006e, 2},
    { 2, -8, 0x404e, 2},
    { 0,  0, 0x406c, 2},
    { 2, -7, 0x404e, 2},
    { 0,  0, 0x406e, 2},
  };
  static const uint8 kGoriyaDmdOffs[] = { 0, 4, 8, 12, 16, 20, 24, 26, 28, 30, 32 };
  if (sprite_delay_aux1[k] && sprite_D[k] != 3)
    Sprite_DrawMultiple(k, &kGoriya_Dmd2[sprite_D[k]], 1, NULL);

  PrepOamCoordsRet info;
  oam_cur_ptr += 4, oam_ext_cur_ptr++;
  int g = sprite_graphics[k];
  Sprite_DrawMultiple(k, &kGoriya_Dmd[kGoriyaDmdOffs[g]], kGoriyaDmdOffs[g + 1] - kGoriyaDmdOffs[g], &info);
  sprite_flags2[k]--;
  SpriteDraw_Shadow(k, &info);
  sprite_flags2[k]++;
}

void Moldorm_Draw(int k) {  // 9df822
  static const int8 kMoldorm_Draw_X[16] = {11, 10, 9, 6, 3, 0, -2, -3, -4, -3, -2, 1, 4, 7, 9, 10};
  static const int8 kMoldorm_Draw_Y[16] = {4, 6, 9, 10, 11, 10, 9, 6, 3, 0, -2, -3, -4, -3, -2, 1};
  static const uint8 kMoldorm_Draw_Char[3] = {0x5d, 0x62, 0x60};
  static const int8 kMoldorm_Draw_XY[3] = {4, 0, 0};
  static const uint8 kMoldorm_Draw_Big[3] = {0, 2, 2};
  static const uint8 kMoldorm_Draw_GetOffs[3] = {21, 26, 0};

  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  OamEnt *oam = GetOamCurPtr();
  uint8 base = sprite_D[k] - 1;
  for (int i = 1; i >= 0; i--, oam++, base += 2) {
    SetOamHelper0(oam, info.x + kMoldorm_Draw_X[base & 0xf], info.y + kMoldorm_Draw_Y[base & 0xf],
                  0x4d, info.flags, 0);
  }
  oam_cur_ptr += 8;
  oam_ext_cur_ptr += 2;

  int j = (sprite_subtype2[k] & 0x1f) + k * 32;
  moldorm_x_lo[j] = sprite_x_lo[k];
  moldorm_x_hi[j] = sprite_x_hi[k];
  moldorm_y_lo[j] = sprite_y_lo[k];
  moldorm_y_hi[j] = sprite_y_hi[k];

  for (int i = 2; i >= 0; i--, oam++) {
    j = ((sprite_subtype2[k] + kMoldorm_Draw_GetOffs[i]) & 0x1f) + k * 32;
    uint16 x = (moldorm_x_lo[j] | moldorm_x_hi[j] << 8) - BG2HOFS_copy2 + kMoldorm_Draw_XY[i];
    uint16 y = (moldorm_y_lo[j] | moldorm_y_hi[j] << 8) - BG2VOFS_copy2 + kMoldorm_Draw_XY[i];
    SetOamHelper0(oam, x, y, kMoldorm_Draw_Char[i], info.flags, kMoldorm_Draw_Big[i]);
  }
}

void TalkingTree_Mouth(int k) {  // 9df956
  static const int8 kTalkingTree_Gfx2[4] = {0, 2, 3, 1};
  int j;
  TalkingTree_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  sprite_flags4[k] = 0;
  switch(sprite_ai_state[k]) {
  case 0:
    sprite_graphics[k] = 0;
    if (Sprite_CheckDamageToLink_same_layer(k)) {
      Link_CancelDash();
      link_incapacitated_timer = 16;
      ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLink(k, 48);
      link_actual_vel_y = pt.y;
      link_actual_vel_x = pt.x;
      SpriteSfx_QueueSfx3WithPan(k, 0x32);
      sprite_ai_state[k]++;
      sprite_delay_main[k] = 48;
    }
    break;
  case 1:
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k]++;
      sprite_delay_main[k] = 8;
    }
    sprite_graphics[k] = sprite_delay_main[k] >> 1 & 3;
    break;
  case 2:
    sprite_graphics[k] = kTalkingTree_Gfx2[sprite_delay_main[k] >> 1]; // zelda bug wtf
    if (sprite_delay_main[k] == 7)
      TalkingTree_SpawnBomb(k);
    if (!sprite_delay_main[k])
      sprite_ai_state[k]++;
    break;
  case 3:
    sprite_flags4[k] = 7;
    if (!sprite_A[k]) {
      static const uint8 kTalkingTree_Msgs2[2] = { 0x82, 0x7d };
      j = sprite_x_lo[k] >> 4 & 1 ^ 1;
      sprite_A[k] = j;
      if (!(Sprite_ShowSolicitedMessage(k, kTalkingTree_Msgs2[j]) & 0x100))
        sprite_A[k] = 0;
    } else {
      static const uint8 kTalkingTree_Msgs[4] = {0x7e, 0x7f, 0x80, 0x81};
      static const uint8 kTalkingTree_Screens[4] = {0x58, 0x5d, 0x72, 0x6b};
      j = FindInByteArray(kTalkingTree_Screens, BYTE(overworld_screen_index), 4);
      Sprite_ShowMessageUnconditional(kTalkingTree_Msgs[j]);
      sprite_A[k] = 0;
    }
    if (!sprite_delay_main[k]) {
      static const uint8 kTalkingTree_Gfx[8] = { 1, 2, 3, 1, 3, 1, 2, 3 };
      static const uint8 kTalkingTree_Delay[8] = { 13, 13, 13, 11, 11, 6, 16, 8 };
      sprite_B[k] = j = sprite_B[k] + 1 & 7;
      sprite_graphics[k] = kTalkingTree_Gfx[j];
      sprite_delay_main[k] = kTalkingTree_Delay[j];
    }
    break;
  }
}

void TalkingTree_SpawnBomb(int k) {  // 9dfa4e
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0x4a, &info);
  if (j >= 0) {
    Sprite_TransmuteToBomb(j);
    Sprite_SetSpawnedCoordinates(j, &info);
    sprite_delay_aux1[j] = 64;
    sprite_y_vel[j] = 24;
    sprite_z_vel[j] = 18;
  }
}

void TalkingTree_Draw(int k) {  // 9dfadb
  static const DrawMultipleData kTalkingTree_Dmd[12] = {
    {1, -1, 0x00e8, 0},
    {1,  7, 0x00f8, 0},
    {7, -1, 0x40e8, 0},
    {7,  7, 0x40f8, 0},
    {0, -1, 0x00e8, 0},
    {0,  7, 0x00f8, 0},
    {8, -1, 0x40e8, 0},
    {8,  7, 0x40f8, 0},
    {0,  0, 0x00e8, 0},
    {0,  7, 0x00f8, 0},
    {8,  0, 0x40e8, 0},
    {8,  7, 0x40f8, 0},
  };
  int g = sprite_graphics[k] - 1;
  if (g < 0)
    return;
  Sprite_DrawMultiplePlayerDeferred(k, &kTalkingTree_Dmd[g * 4], 4, NULL);
}

void TalkingTree_Eye(int k) {  // 9dfb0a
  static const int8 kTalkingTree_Type1_X[2] = {9, -9};
  SpriteDraw_SingleSmall(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  int j = sprite_head_dir[k];
  Sprite_SetX(k, (sprite_A[k] | sprite_B[k] << 8) + kTalkingTree_Type1_X[j]);
  Sprite_SetY(k, (sprite_C[k] | sprite_E[k] << 8));
  ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLink(k, 2);
  if (!sign8(pt.y)) {
    sprite_D[k] = pt.x + 2;
  } else if (sprite_D[k] != 2) {
    sprite_D[k] += (sprite_D[k] >= 2) ? -1 : 1;
  }
  static const int8 kTalkingTree_X1[5] = {-2, -1, 0, 1, 2};
  static const int8 kTalkingTree_Y1[5] = {-1, 0, 0, 0, -1};
  j = sprite_D[k];
  Sprite_SetX(k, (sprite_A[k] | sprite_B[k] << 8) + kTalkingTree_X1[j]);
  Sprite_SetY(k, (sprite_C[k] | sprite_E[k] << 8) + kTalkingTree_Y1[j]);
}

void SpritePrep_TalkingTree_SpawnEyeball(int k, int dir) {  // 9dfb8a
  static const int8 kTalkingTree_SpawnX[2] = {-4, 14};
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0x25, &info);
  if (j >= 0) {
    sprite_head_dir[j] = dir;
    int x = info.r0_x + kTalkingTree_SpawnX[dir];
    int y = info.r2_y - 11;
    Sprite_SetX(j, x);
    Sprite_SetY(j, y);
    sprite_A[j] = x;
    sprite_B[j] = x >> 8;
    sprite_C[j] = y;
    sprite_E[j] = y >> 8;
    sprite_subtype2[j] = 1;
  }
}

void RupeePull_SpawnPrize(int k) {  // 9dfbd7
  static const int8 kSpawnRupees_Xvel[4] = {-18, -12, 12, 18};
  static const int8 kSpawnRupees_Yvel[4] = {16, 24, 24, 16};
  static const uint8 kSpawnRupees_Type[3] = {0xd9, 0xda, 0xdb};
  if (num_sprites_killed) {
    byte_7E0FB6 = num_sprites_killed < 4 ? 0 :
                  number_of_times_hurt_by_sprites ? 1 : 2;
    tmp_counter = 3;
    do {
      SpriteSpawnInfo info;
      int j = Sprite_SpawnDynamically(k, kSpawnRupees_Type[byte_7E0FB6], &info);
      if (j < 0)
        break;
      Sprite_SetSpawnedCoordinates(j, &info);
      sprite_x_vel[j] = kSpawnRupees_Xvel[tmp_counter];
      sprite_y_vel[j] = kSpawnRupees_Yvel[tmp_counter];
      sprite_stunned[j] = 255;
      sprite_delay_aux4[j] = 32;
      sprite_delay_aux3[j] = 32;
      sprite_z_vel[j] = 32;
    } while (!sign8(--tmp_counter));
  }
  num_sprites_killed = 0;
  number_of_times_hurt_by_sprites = 0;
}

void Sprite_D5_DigGameGuy(int k) {  // 9dfc38
  DiggingGameGuy_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_BehaveAsBarrier(k);
  Sprite_MoveXY(k);
  sprite_x_vel[k] = 0;
  switch(sprite_ai_state[k]) {
  case 0:  // intro
    if ((uint8)(sprite_y_lo[k] + 7) < BYTE(link_y_coord) && Sprite_DirectionToFaceLink(k, NULL) == 2) {
      if (follower_indicator == 0) {
        if (Sprite_ShowSolicitedMessage(k, 0x187) & 0x100)
          sprite_ai_state[k]++;
      } else {
        Sprite_ShowSolicitedMessage(k, 0x18c);
      }
    }
    break;
  case 1:  // do you want to play
    if (choice_in_multiselect_box == 0 && link_rupees_goal >= 80) {
      link_rupees_goal -= 80;
      Sprite_ShowMessageUnconditional(0x188);
      sprite_ai_state[k] = 2;
      sprite_graphics[k] = 1;
      sprite_delay_main[k] = 80;
      beamos_x_hi[0] = 0;
      beamos_x_hi[1] = 0;
      sprite_delay_aux1[k] = 5;
      Sprite_InitializeSecondaryItemMinigame(1);
      music_control = 14;
    } else {
      Sprite_ShowMessageUnconditional(0x189);
      sprite_ai_state[k] = 0;
    }
    break;
  case 2:  // move out of the way
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k]++;
      sprite_graphics[k] = 1;
    } else if (!sprite_delay_aux1[k]) {
      sprite_graphics[k] ^= 3;
      if (sprite_graphics[k] & 1)
        sprite_x_vel[k] = -16;
      sprite_delay_aux1[k] = 5;
    }
    break;
  case 3:  // start timer
    sprite_ai_state[k]++;
    super_bomb_indicator_unk1 = 0;
    super_bomb_indicator_unk2 = 30;
    break;
  case 4:  // terminate
    if ((int8)super_bomb_indicator_unk2 > 0 || link_position_mode & 1)
      return;
    music_control = 9;
    sprite_ai_state[k]++;
    is_archer_or_shovel_game = 0;
    dialogue_message_index = 0x18a;
    Sprite_ShowMessageMinimal();
    super_bomb_indicator_unk2 = 254;
    break;
  case 5:  // come back later
    Sprite_ShowSolicitedMessage(k, 0x18b);
    break;
  }
}

void DiggingGameGuy_Draw(int k) {  // 9dfe4b
  static const DrawMultipleData kDiggingGameGuy_Dmd[9] = {
    { 0, -8, 0x0a40, 2},
    { 4,  9, 0x0c56, 0},
    { 0,  0, 0x0a42, 2},
    { 0, -8, 0x0a40, 2},
    { 0,  0, 0x0a42, 2},
    { 0,  0, 0x0a42, 2},
    {-1, -7, 0x0a40, 2},
    {-1,  0, 0x0a44, 2},
    {-1,  0, 0x0a44, 2},
  };
  PrepOamCoordsRet info;
  Sprite_DrawMultiplePlayerDeferred(k, &kDiggingGameGuy_Dmd[sprite_graphics[k] * 3], 3, &info);
  SpriteDraw_Shadow(k, &info);
}

void OldMountainMan_Draw(int k) {  // 9dff0e
  static const DrawMultipleData kOldMountainMan_Dmd0[2] = {
    {0, 0, 0x00ac, 2},
    {0, 8, 0x00ae, 2},
  };
  static const DrawMultipleData kOldMountainMan_Dmd1[16] = {
    { 0, 0, 0x0120, 2},
    { 0, 8, 0x0122, 2},
    { 0, 1, 0x0120, 2},
    { 0, 9, 0x4122, 2},
    { 0, 0, 0x0120, 2},
    { 0, 8, 0x0122, 2},
    { 0, 1, 0x0120, 2},
    { 0, 9, 0x4122, 2},
    {-2, 0, 0x0120, 2},
    { 0, 8, 0x0122, 2},
    {-2, 1, 0x0120, 2},
    { 0, 9, 0x0122, 2},
    { 2, 0, 0x4120, 2},
    { 0, 8, 0x4122, 2},
    { 2, 1, 0x4120, 2},
    { 0, 9, 0x4122, 2},
  };
  static const uint8 kOldMountainMan_Dma[16] = {0x20, 0xc0, 0x20, 0xc0, 0, 0xa0, 0, 0xa0, 0x40, 0x80, 0x40, 0x60, 0x40, 0x80, 0x40, 0x60};
  if (sprite_subtype2[k] != 2) {
    int j = sprite_D[k] * 4 + sprite_graphics[k] * 2;
    BYTE(dma_var6) = kOldMountainMan_Dma[j + 0];
    BYTE(dma_var7) = kOldMountainMan_Dma[j + 1];
    Sprite_DrawMultiplePlayerDeferred(k, &kOldMountainMan_Dmd1[j], 2, NULL);
  } else {
    Sprite_DrawMultiplePlayerDeferred(k, kOldMountainMan_Dmd0, 2, NULL);
  }

}

void HelmasaurKing_Initialize(int k) {  // 9e8000
  overlord_gen1[7] = 0x30;
  overlord_gen1[5] = 0x80;
  overlord_gen1[6] = 0;
  overlord_gen2[0] = 0;
  overlord_gen2[3] = 0;
  overlord_gen2[1] = 0;
  overlord_gen2[2] = 0;
  HelmasaurKing_Reinitialize(k);
}

void HelmasaurKing_Reinitialize(int k) {  // 9e8019
  uint8 t = sprite_subtype2[k];
  for (int i = 3; i >= 0; i--) {
    overlord_x_lo[i] = kHelmasaur_Tab0[t + i * 8 & 0x1f];
  }
}

void Sprite_92_HelmasaurKing(int k) {  // 9e8039
  int t, j;

  if (sign8(sprite_C[k])) {
    if (sprite_delay_main[k] == 1)
      sprite_state[k] = 0;
    SpriteDraw_SingleLarge(k);
    if (Sprite_ReturnIfInactive(k))
      return;
    if (!(frame_counter & 7 | sprite_delay_aux1[k]))
      sprite_oam_flags[k] ^= 0x40;
    Sprite_MoveXYZ(k);
    sprite_z_vel[k] -= 2;
    if (sign8(sprite_z[k])) {
      sprite_z[k] = 0;
      sprite_delay_main[k] = 12;
      sprite_z_vel[k] = 24;
      sprite_graphics[k] = 6;
    }
    return;
  }
  if (sprite_C[k] < 3) {
    sprite_obj_prio[k] &= ~0xE;
    sprite_flags[k] = 0xA;
  } else {
    sprite_flags4[k] = 0x1F;
    sprite_flags[k] = 2;
  }
  HelmasaurKing_Draw(k);
  if (sprite_state[k] == 6) {
    t = sprite_delay_main[k];
    if (!t) {
      sprite_state[k] = 4;
      sprite_A[k] = 0;
      sprite_delay_main[k] = 224;
      return;
    }
    sprite_hit_timer[k] = t | 240;
    if (t < 128 && (t & 7) == 0 && (j = overlord_gen2[3]) != 0x10) {
      overlord_gen2[3]++;
      cur_sprite_x = Sprite_GetX(k) + (int8)overlord_x_lo[5 + j];
      cur_sprite_y = Sprite_GetY(k) + (int8)overlord_y_lo[5 + j];
      Sprite_MakeBossExplosion(k);
    }
    return;
  }

  if (Sprite_ReturnIfInactive(k))
    return;
  static const uint8 kHelmasaurKing_Tab1[13] = {3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 1, 1, 0};
  t = kHelmasaurKing_Tab1[sprite_health[k] >> 2];
  sprite_C[k] = t;
  if (t == 3) {
    if (t != sprite_E[k]) {
      sprite_hit_timer[k] = 0;
      HelmasaurKing_ExplodeMask(k);
    }
  } else {
    if (t != sprite_E[k])
      HelmasaurKing_ChipAwayAtMask(k);
  }
  sprite_E[k] = sprite_C[k];
  Sprite_CheckDamageFromLink(k);
  HelmasaurKing_SwingTail(k);
  HelmasaurKing_AttemptDamage(k);
  HelmasaurKing_CheckMaskDamageFromHammer(k);
  if (sprite_delay_aux1[k] == 0) {
    if (sprite_delay_aux2[k] != 0) {
      if (sprite_delay_aux2[k] == 0x40) {
        HelmasaurKing_SpitFireball(k);
        if (sprite_C[k] >= 3) {
anim_clk:
          if (!sprite_anim_clock[k]) {
            sprite_anim_clock[k]++;
            sprite_delay_aux3[k] = 32;
          }
        }
      }
      return;
    }
  } else {
    if (sprite_delay_aux1[k] == 96)
      goto anim_clk;
    return;
  }
  static const int8 kHelmasaurKing_Xvel0[8] = {-12, -12, -4, 0, 4, 12, 12, 0};
  static const int8 kHelmasaurKing_Yvel0[8] = {0, 4, 12, 12, 12, 4, 0, 12};
  switch (sprite_ai_state[k]) {
  case 0:
    if ((sprite_hit_timer[k] || !sprite_delay_main[k]) && !HelmasaurKing_MaybeFireball(k)) {
      j = GetRandomNumber() & 7;
      sprite_x_vel[k] = kHelmasaurKing_Xvel0[j];
      sprite_y_vel[k] = kHelmasaurKing_Yvel0[j];
      sprite_delay_main[k] = 64;
      if (sprite_C[k] >= 3) {
        sprite_x_vel[k] <<= 1;
        sprite_y_vel[k] <<= 1;
        sprite_delay_main[k] >>= 1;
      }
      sprite_ai_state[k]++;
    }
    break;
  case 1:
    HelmasaurKing_HandleMovement(k);
    if (!sprite_delay_main[k]) {
      sprite_delay_main[k] = 32;
      sprite_ai_state[k]++;
    }
    break;
  case 2:
    if ((sprite_hit_timer[k] || !sprite_delay_main[k]) && !HelmasaurKing_MaybeFireball(k)) {
      sprite_delay_main[k] = 64;
      if (sprite_E[k] >= 3)
        sprite_delay_main[k] >>= 1;
      sprite_x_vel[k] = -sprite_x_vel[k];
      sprite_y_vel[k] = -sprite_y_vel[k];
      sprite_ai_state[k]++;
    }
    break;
  case 3:
    HelmasaurKing_HandleMovement(k);
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k] = 0;
      sprite_delay_main[k] = 64;
    }
    break;
  }
}

void HelmasaurKing_HandleMovement(int k) {  // 9e81e6
  int n = 1 + ((frame_counter & 3) == 0) + (sprite_C[k] >= 3);
  do {
    if (!(++sprite_subtype2[k] & 15))
      sound_effect_1 = 0x21;
  } while (--n);
  Sprite_MoveXY(k);
}

bool HelmasaurKing_MaybeFireball(int k) {  // 9e8253
  if (++sprite_subtype[k] != 4)
    return false;
  sprite_subtype[k] = 0;
  if (GetRandomNumber() & 1) {
    sprite_delay_aux2[k] = 127;
    SpriteSfx_QueueSfx3WithPan(k, 0x2a);
  } else {
    sprite_delay_aux1[k] = 160;
  }
  return true;
}

void HelmasaurKing_SwingTail(int k) {  // 9e82a0
  overlord_x_lo[4]++;
  HelmasaurKing_Reinitialize(k);
  uint8 mask = sprite_anim_clock[k] ? 0 : 1;
  if (!(frame_counter & mask)) {
    int j = sprite_D[k] & 1;
    overlord_gen2[0] += j ? -1 : 1;
    if (overlord_gen2[0] == (uint8)kFluteBoyAnimal_Xvel[j])
      sprite_D[k]++;
    WORD(overlord_gen1[5]) += (int8)overlord_gen2[0];
  }
  if (!sprite_anim_clock[k])
    return;
  if (!overlord_gen2[0])
    SpriteSfx_QueueSfx3WithPan(k, 0x6);

  if (sprite_anim_clock[k] == 2) {
    int j = sprite_head_dir[k];
    WORD(overlord_gen2[1]) += j ? -4 : 4;
    if (overlord_gen2[1] == (uint8)(j ? -124 : 124))
      sprite_anim_clock[k] = 3;
    overlord_gen1[7] += 3;
  } else if (sprite_anim_clock[k] == 3) {
    int j = sprite_head_dir[k] ^ 1;
    WORD(overlord_gen2[1]) += j ? -4 : 4;
    if (overlord_gen2[1] == 0)
      sprite_anim_clock[k] = 0;
    overlord_gen1[7] -= 3;
  } else {
    if (!(overlord_gen2[0] | sprite_delay_aux3[k])) {
      sprite_head_dir[k] = overlord_gen1[6] & 1;
      uint8 dir = Sprite_IsRightOfLink(k).a ^ 1;
      if (dir == sprite_head_dir[k]) {
        sprite_anim_clock[k] = 2;
        sound_effect_2 = Sprite_CalculateSfxPan(k) | 0x26;
      }
    }
  }
}

void HelmasaurKing_CheckMaskDamageFromHammer(int k) {  // 9e8385
  if (sprite_C[k] >= 3 || !(link_item_in_hand & 10) || (player_oam_y_offset == 0x80))
    return;
  SpriteHitBox hb;
  Player_SetupActionHitBox(&hb);
  uint8 bak  = sprite_y_lo[k];
  sprite_y_lo[k] += 8;
  Sprite_SetupHitBox(k, &hb);
  sprite_y_lo[k] = bak;
  if (CheckIfHitBoxesOverlap(&hb)) {
    sprite_health[k]--;
    sound_effect_2 = 0x21;
    ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLink(k, 0x30);
    link_actual_vel_y = pt.y;
    link_actual_vel_x = pt.x;
    link_incapacitated_timer = 8;
    if (!repulsespark_timer) {
      repulsespark_x_lo = pt.y;
      repulsespark_y_lo = pt.x;
      repulsespark_timer = 5;
    }
    SpriteSfx_QueueSfx2WithPan(k, 0x5);
  }
}

void HelmasaurKing_AttemptDamage(int k) {  // 9e83eb
  if (!(frame_counter & 7) &&
      (uint16)(link_x_coord - cur_sprite_x + 36) < 72 &&
      (uint16)(link_y_coord - cur_sprite_y + 40) < 64)
    Sprite_AttemptDamageToLinkPlusRecoil(k);
}

void HelmasaurKing_ChipAwayAtMask(int k) {  // 9e847e
  tmp_counter = sprite_C[k] + 7;
  HelmasaurKing_SpawnMaskDebris(k);
  SpriteSfx_QueueSfx2WithPan(k, 0x1f);
}

void HelmasaurKing_ExplodeMask(int k) {  // 9e848c
  for (int j = 1; j < 16; j++)
    sprite_state[j] = 0;
  tmp_counter = 7;
  do {
    HelmasaurKing_SpawnMaskDebris(k);
  } while (!sign8(--tmp_counter));
  SpriteSfx_QueueSfx2WithPan(k, 0x1f);
}

void HelmasaurKing_SpawnMaskDebris(int k) {  // 9e84aa
  static const int8 kHelmasaurKing_Mask_X[10] = {-16, 0, 16, -16, 0, 16, -8, 8, -16, 16};
  static const int8 kHelmasaurKing_Mask_Y[10] = {24, 27, 24, 24, 27, 24, 27, 27, 24, 24};
  static const int8 kHelmasaurKing_Mask_Z[10] = {29, 32, 29, 13, 16, 13, 0, 0, 13, 13};
  static const int8 kHelmasaurKing_Mask_Xvel[10] = {-16, -4, 14, -12, 4, 18, -2, 2, -12, 18};
  static const int8 kHelmasaurKing_Mask_Yvel[10] = {-8, -4, -6, 4, 2, 7, 6, 8, 4, 7};
  static const int8 kHelmasaurKing_Mask_Zvel[10] = {32, 40, 36, 37, 39, 34, 30, 33, 37, 34};
  static const uint8 kHelmasaurKing_Mask_OamFlags[10] = {0, 0, 0x40, 0, 0, 0x40, 0, 0x40, 0, 0x40};
  static const uint8 kHelmasaurKing_Mask_Gfx[10] = {0, 1, 0, 2, 3, 2, 4, 4, 5, 5};
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0x92, &info);
  if (j >= 0) {
    int i = tmp_counter;
    Sprite_SetX(j, info.r0_x + kHelmasaurKing_Mask_X[i]);
    Sprite_SetY(j, info.r2_y + kHelmasaurKing_Mask_Y[i]);
    sprite_z[j] = kHelmasaurKing_Mask_Z[i];
    sprite_x_vel[j] = kHelmasaurKing_Mask_Xvel[i];
    sprite_y_vel[j] = kHelmasaurKing_Mask_Yvel[i];
    sprite_z_vel[j] = kHelmasaurKing_Mask_Zvel[i];
    sprite_oam_flags[j] = kHelmasaurKing_Mask_OamFlags[i] | 13;
    sprite_graphics[j] = kHelmasaurKing_Mask_Gfx[i];
    sprite_C[j] = 128;
    sprite_flags2[j] = 0;
    sprite_delay_aux1[j] = 12;
    sprite_ignore_projectile[j] = 12;
    sprite_subtype[j] = tmp_counter;
  }
}

void HelmasaurKing_SpitFireball(int k) {  // 9e8517
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0x70, &info);
  if (j >= 0) {
    Sprite_SetSpawnedCoordinates(j, &info);
    Sprite_SetY(j, info.r2_y + 28);
    sprite_delay_main[j] = 32;
    sprite_ignore_projectile[j] = 32;
  }
}

void HelmasaurKing_Draw(int k) {  // 9e853b
  oam_cur_ptr = 0x89c;
  oam_ext_cur_ptr = 0xa47;
  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  KingHelmasaur_OperateTail(k, &info);
  SpriteDraw_KingHelmasaur_Eyes(k, &info);
  KingHelmasaurMask(k, &info);
  SpriteDraw_KingHelmasaur_Body(k, &info);
  SpriteDraw_KingHelmasaur_Legs(k, &info);
  SpriteDraw_KingHelmasaur_Mouth(k, &info);
}

void SpriteDraw_KingHelmasaur_Eyes(int k, PrepOamCoordsRet *info) {  // 9e856b
  static const int8 kHelmasaurKing_DrawB_X[2] = {-3, 11};
  static const uint8 kHelmasaurKing_DrawB_Char[8] = {0xce, 0xcf, 0xde, 0xde, 0xde, 0xde, 0xcf, 0xce};
  static const uint8 kHelmasaurKing_DrawB_Flags[2] = {0x3b, 0x7b};
  oam_cur_ptr += 0x40, oam_ext_cur_ptr += 0x10;
  OamEnt *oam = GetOamCurPtr();
  for (int i = 1; i >= 0; i--, oam++) {
    int j = overlord_x_lo[4] >> 2 & 7;
    SetOamPlain(oam, info->x + kHelmasaurKing_DrawB_X[i], info->y + 0x14, kHelmasaurKing_DrawB_Char[j], kHelmasaurKing_DrawB_Flags[i], 0);
  }
  if (submodule_index)
    Sprite_CorrectOamEntries(k, 1, 0);
}

void KingHelmasaurMask(int k, PrepOamCoordsRet *info) {  // 9e8686
  static const DrawMultipleData kHelmasaurKing_DrawC_Dmd[24] = {
    {-16, -5, 0x0dae, 2},
    {  0, -5, 0x0dc0, 2},
    { 16, -5, 0x4dae, 2},
    {-16, 11, 0x0dc2, 2},
    {  0, 11, 0x0dc4, 2},
    { 16, 11, 0x4dc2, 2},
    { -8, 27, 0x0dc6, 2},
    {  8, 27, 0x4dc6, 2},
    {-16, -5, 0x0dae, 2},
    {  0, -5, 0x0dc0, 2},
    { 16, -5, 0x4dae, 2},
    {-16, 11, 0x0dc8, 2},
    {  0, 11, 0x0dc4, 2},
    { 16, 11, 0x4dc2, 2},
    { -8, 27, 0x0dc6, 2},
    {  8, 27, 0x4dc6, 2},
    {-16, -5, 0x0dae, 2},
    {  0, -5, 0x0dc0, 2},
    { 16, -5, 0x4dae, 2},
    {-16, 11, 0x0dc8, 2},
    {  0, 11, 0x0dc4, 2},
    { 16, 11, 0x4dc8, 2},
    { -8, 27, 0x0dc6, 2},
    {  8, 27, 0x4dc6, 2},
  };
  oam_cur_ptr += 8, oam_ext_cur_ptr += 2;
  if (sprite_C[k] >= 3)
    return;
  Sprite_DrawMultiple(k, &kHelmasaurKing_DrawC_Dmd[sprite_C[k] * 8], 8, info);
  oam_cur_ptr += 0x20, oam_ext_cur_ptr += 8;
  if (sprite_delay_aux4[k])
    return;
  for (int i = 1; i >= 0; i--) {
    if (ancilla_type[i] == 7 && (ancilla_x_vel[i] | ancilla_y_vel[i])) {
      KingHelmasaur_CheckBombDamage(k, i);
    }
  }
}

void KingHelmasaur_CheckBombDamage(int k, int j) {  // 9e86e5
  SpriteHitBox hb;
  Sprite_SetupHitBox(k, &hb);
  int x = (ancilla_x_lo[j] | ancilla_x_hi[j] << 8) - 6;
  int y = (ancilla_y_lo[j] | ancilla_y_hi[j] << 8) - ancilla_z[j];
  hb.r0_xlo = x, hb.r8_xhi = x >> 8;
  hb.r1_ylo = y, hb.r9_yhi = y >> 8;
  hb.r2 = 2;
  hb.r3 = 15;
  if (CheckIfHitBoxesOverlap(&hb)) {
    ancilla_x_vel[j] = -ancilla_x_vel[j];
    ancilla_y_vel[j] = (int8)-ancilla_y_vel[j] >> 1;
    sprite_delay_aux4[k] = 32;
    repulsespark_timer = 5;
    repulsespark_x_lo = ancilla_x_lo[j];
    repulsespark_y_lo = ancilla_y_lo[j] - ancilla_z[j];
    sound_effect_1 = 5;
  }
}

void SpriteDraw_KingHelmasaur_Body(int k, PrepOamCoordsRet *info) {  // 9e87e5
  static const DrawMultipleData kHelmasaurKing_DrawD_Dmd[19] = {
    {-24, -32, 0x0b80, 2},
    { -8, -32, 0x0b82, 2},
    {  8, -32, 0x4b82, 2},
    { 24, -32, 0x4b80, 2},
    {-24, -16, 0x0b84, 2},
    { -8, -16, 0x0b86, 2},
    {  8, -16, 0x4b86, 2},
    { 24, -16, 0x4b84, 2},
    {-24,   0, 0x0b88, 2},
    { -8,   0, 0x0b8a, 2},
    {  8,   0, 0x4b8a, 2},
    { 24,   0, 0x4b88, 2},
    {-24,  16, 0x0b8c, 2},
    { -8,  16, 0x0b8e, 2},
    {  8,  16, 0x4b8e, 2},
    { 24,  16, 0x4b8c, 2},
    { -8,  32, 0x0ba0, 2},
    {  8,  32, 0x4ba0, 2},
    {  0, -40, 0x0bac, 2},
  };
  Sprite_DrawMultiple(k, kHelmasaurKing_DrawD_Dmd, 19, info);
}

void SpriteDraw_KingHelmasaur_Legs(int k, PrepOamCoordsRet *info) {  // 9e8805
  static const int8 kHelmasaurKing_DrawE_X[4] = {-28, -28, 28, 28};
  static const int8 kHelmasaurKing_DrawE_Y[4] = {-28, 4, -28, 4};
  static const uint8 kHelmasaurKing_DrawE_Char[4] = {0xa2, 0xa6, 0xa2, 0xa6};
  static const uint8 kHelmasaurKing_DrawE_Flags[4] = {0xb, 0xb, 0x4b, 0x4b};

  oam_cur_ptr += 19 * 4;
  oam_ext_cur_ptr += 19;
  OamEnt *oam = GetOamCurPtr();
  for (int i = 3; i >= 0; i--, oam += 2) {
    uint8 x = info->x + kHelmasaurKing_DrawE_X[i];
    uint8 y = info->y + kHelmasaurKing_DrawE_Y[i] + overlord_x_lo[i];
    uint8 f = kHelmasaurKing_DrawE_Flags[i] ^ info->flags;
    SetOamPlain(oam + 0, x, y, kHelmasaurKing_DrawE_Char[i], f, 2);
    SetOamPlain(oam + 1, x, y + 16, kHelmasaurKing_DrawE_Char[i] + 2, f, 2);
  }
  tmp_counter = 0xff;
  if (submodule_index) {
    Sprite_CorrectOamEntries(k, 7, 2);
    Sprite_PrepOamCoordOrDoubleRet(k, info);
  }
}

void SpriteDraw_KingHelmasaur_Mouth(int k, PrepOamCoordsRet *info) {  // 9e88bc
  static const uint8 kHelmasaurKing_DrawF_Y[32] = {
    1,  2,  3,  4,  5,  6,  7, 8, 9, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 9, 8,  7,  6,  5,  4,  3,  2,  1,
  };
  if (!sprite_delay_aux2[k])
    return;
  uint8 yd = kHelmasaurKing_DrawF_Y[sprite_delay_aux2[k] >> 2];
  Oam_AllocateFromRegionB(4);
  OamEnt *oam = GetOamCurPtr();
  SetOamPlain(oam, info->x, info->y + yd + 0x13, 0xaa, info->flags ^ 0xb, 2);
}

void KingHelmasaur_OperateTail(int k, PrepOamCoordsRet *info) {  // 9e8920
  static const uint8 kHelmasaurKing_DrawA_Mult[32] = {
    0xff, 0xf0, 0xe0, 0xd0, 0xc0, 0xb0, 0xa0, 0x90, 0x80, 0x70, 0x60, 0x50, 0x40, 0x30, 0x20, 0x10,
    0xff, 0xf8, 0xf0, 0xe8, 0xe0, 0xd8, 0xd0, 0xc8, 0xbc, 0xb0, 0xa0, 0x90, 0x70, 0x40, 0x20, 0x10,
  };
  static const uint8 kHelmasaurKing_DrawA_MultB[16] = {
    0xff, 0xf0, 0xe0, 0xd0, 0xc0, 0xb0, 0xa0, 0x90, 0x80, 0x70, 0x60, 0x50, 0x40, 0x30, 0x20, 0x10
  };

  for (int i = 0; i < 16; i++) {
    int j = i + (sprite_anim_clock[k] ? 16 : 0);
    uint16 rs = WORD(overlord_gen1[5]) + WORD(overlord_gen2[1]);
    uint8 f = (rs >> 8) - 1;
    uint8 r6 = (uint8)(sign8(f) ? -rs : rs) * kHelmasaurKing_DrawA_Mult[j] >> 8;
    uint16 angle = (rs & 0xff00) | (sign8(f) ? r6 ^ 0xff : r6);
    uint8 r15 = overlord_gen1[7] * kHelmasaurKing_DrawA_MultB[i] >> 8;
    overlord_x_lo[i+5] = HelmasaurSin(angle, r15);
    overlord_y_lo[i+5] = HelmasaurSin(angle + 0x80, r15) - 40;
  }

  OamEnt *oam = GetOamCurPtr();
  bool is_hit = false;
  for (int i = overlord_gen2[3]; i != 16; i++, oam++) {
    uint8 x = overlord_x_lo[i + 5] + info->x;
    uint8 y = overlord_y_lo[i + 5] + info->y;
    oam->x = x;
    oam->y = y;
    oam->charnum = (i == overlord_gen2[3]) ? 0xe4 : 0xac;
    oam->flags = info->flags ^ 0x1b;

    if (!countdown_for_blink && sprite_anim_clock[k]) {
      if ((uint8)(link_x_coord - BG2HOFS_copy2 - x + 12) < 24 &&
          (uint8)(link_y_coord - BG2VOFS_copy2 + 8 - y + 8) < 16) {
        is_hit = true;
        link_actual_vel_x = 0;
        link_actual_vel_y = 56;
      }
    }
  }

  if (is_hit && !flag_block_link_menu)
    Sprite_AttemptDamageToLinkPlusRecoil(k);
  Sprite_CorrectOamEntries(k, 16, 2);
  Sprite_PrepOamCoordOrDoubleRet(k, info);
  tmp_counter = 16;
}

void Sprite_MadBatterBolt(int k) {  // 9e8a96
  static const int8 kMadderBolt_X[8] = {0, 4, 8, 12, 12, 4, 8, 0};
  static const int8 kMadderBolt_Y[8] = {0, 4, 8, 12, 12, 4, 8, 0};

  if (sprite_subtype2[k] & 16)
    Oam_AllocateFromRegionB(4);
  SpriteDraw_SingleSmall(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (!sprite_ai_state[k]) {
    Sprite_MoveXY(k);
    if (!sprite_delay_main[k])
      sprite_ai_state[k] = 1;
  } else {
    if (++sprite_ai_state[k] == 0)
      sprite_state[k] = 0;
    int j = ++sprite_subtype2[k];
    if (!(j & 7))
      sound_effect_2 = 48;
    Sprite_SetX(k, link_x_coord + kMadderBolt_X[j >> 2 & 7]);
    Sprite_SetY(k, link_y_coord + kMadderBolt_Y[j >> 4 & 7]);
  }
}

void Sprite_AA_Pikit(int k) {  // 9e8bbf
  static const uint8 kPikit_Gfx[24] = {
    2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    3, 3, 3, 3, 2, 2, 2, 2,
  };
  static const int8 kPikit_XyOffs[72] = {
     0,  0,  0,  0,  0,  0,  0,  0,   0,   0,   0,   0,   0,   0,   0,   0,
     0,  0,  0,  0,  0,  0,  0,  0,   0,   0,   0,   0,   0,   0,   0,   0,
    12, 16, 24, 32, 32, 24, 16, 12,   0,   0,   0,   0,   0,   0,   0,   0,
     0,  0,  0,  0,  0,  0,  0,  0, -12, -16, -24, -32, -32, -24, -16, -12,
     0,  0,  0,  0,  0,  0,  0,  0,
  };
  static const uint8 kPikit_Tab0[8] = {24, 24, 0, 48, 48, 48, 0, 24};
  static const uint8 kPikit_Tab1[8] = {0, 24, 24, 24, 0, 48, 48, 48};
  int j;
  Pikit_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (Sprite_ReturnIfRecoiling(k))
    return;
  Sprite_CheckDamageToAndFromLink(k);
  switch(sprite_ai_state[k]) {
  case 0:  // set next vel
    if (!sprite_delay_main[k]) {
      int j;
      sprite_ai_state[k]++;
      if (++sprite_C[k] == 4) {
        sprite_C[k] = 0;
        j = Sprite_DirectionToFaceLink(k, NULL);
      } else {
        j = GetRandomNumber() & 3;
      }
      sprite_x_vel[k] = kFluteBoyAnimal_Xvel[j];
      sprite_y_vel[k] = kZazak_Yvel[j];
      sprite_z_vel[k] = (GetRandomNumber() & 7) + 19;
    }
    sprite_graphics[k] = ++sprite_subtype2[k] >> 3 & 1;
    break;
  case 1:  // finish jump then attack
    Sprite_MoveXYZ(k);
    Sprite_CheckTileCollision(k);
    sprite_z_vel[k]--;
    sprite_z_vel[k]--;
    if (sign8(sprite_z[k])) {
      sprite_z[k] = 0;
      sprite_z_vel[k] = 0;
      PointU8 pt;
      Sprite_DirectionToFaceLink(k, &pt);
      if ((uint8)(pt.x + 48) < 96 && (uint8)(pt.y + 48) < 96) {
        sprite_ai_state[k]++;
        ProjectSpeedRet pp = Sprite_ProjectSpeedTowardsLink(k, 31);
        sprite_D[k] = Sprite_ConvertVelocityToAngle(pp.x, pp.y) >> 1;
        sprite_delay_main[k] = 95;
        return;
      }
      sprite_ai_state[k] = 0;
      sprite_delay_main[k] = 16;
    }
    sprite_graphics[k] = ++sprite_subtype2[k] >> 3 & 1;
    break;
  case 2: { // attempt item grab
    j = sprite_delay_main[k];
    if (!j) {
      sprite_ai_state[k] = 0;
      sprite_delay_main[k] = 16;
      sprite_A[k] = 0;
      sprite_B[k] = 0;
      sprite_G[k] = 0;
      return;
    }
    j >>= 2;
    sprite_graphics[k] = kPikit_Gfx[j];
    int8 xo = kPikit_XyOffs[j + kPikit_Tab0[sprite_D[k]]];
    int8 yo = kPikit_XyOffs[j + kPikit_Tab1[sprite_D[k]]];
    sprite_A[k] = xo;
    sprite_B[k] = yo;
    if (!sprite_G[k] &&
        (uint16)(cur_sprite_x + xo - link_x_coord + 12) < 24 &&
        (uint16)(cur_sprite_y + yo - link_y_coord + 12) < 32 &&
        sprite_delay_main[k] < 46) {
      sound_effect_1 = Link_CalculateSfxPan() | 0x26;
      j = (GetRandomNumber() & 3) + 1;
      sprite_E[k] = sprite_G[k] = j;
      if (j == 1) {
        if (link_item_bombs)
          link_item_bombs--;
        else
          sprite_G[k] = 0;
      } else if (j == 2) {
        if (link_num_arrows)
          link_num_arrows--;
        else
          sprite_G[k] = 0;
      } else if (j == 3) {
        if (link_rupees_goal)
          link_rupees_goal--;
        else
          sprite_G[k] = 0;
      } else {
        sprite_subtype[k] = link_shield_type;
        if (link_shield_type != 0 && link_shield_type != 3)
          link_shield_type = 0;
        else
          sprite_G[k] = 0;
      }
    }
    break;
  }
  }
}

void Sprite_A8_GreenZirro(int k) {  // 9e8dd2
  static const uint8 kBomber_Gfx[4] = {9, 10, 8, 7};

  sprite_obj_prio[k] = 0x30;
  if (sprite_A[k]) {
    switch (sprite_ai_state[k]) {
    case 0: // bomberpellet falling
      SpriteDraw_SingleSmall(k);
      if (Sprite_ReturnIfInactive(k))
        return;
      Sprite_MoveXY(k);
      Sprite_MoveZ(k);
      sprite_z_vel[k] -= 2;
      if (sign8(sprite_z[k])) {
        sprite_z[k] = 0;
        sprite_ai_state[k]++;
        sprite_delay_main[k] = 19;
        sprite_flags2[k]++;
        SpriteSfx_QueueSfx2WithPan(k, 0xc);
      }
      break;
    case 1: // bomberpellet exploding
      SpriteDraw_ZirroBomb(k);
      if (Sprite_ReturnIfInactive(k))
        return;
      if (!(frame_counter & 3))
        sprite_delay_main[k]++;
      Sprite_CheckDamageToLink(k);
      break;
    }
    return;
  }

  if (sprite_delay_aux1[k])
    sprite_graphics[k] = kBomber_Gfx[sprite_D[k]];
  sprite_obj_prio[k] |= 0x30;
  Bomber_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (Sprite_ReturnIfRecoiling(k))
    return;
  if (sprite_delay_aux1[k] == 8)
    Zirro_DropBomb(k);
  Sprite_CheckDamageToAndFromLink(k);
  if (!(frame_counter & 1)) {
    int j = sprite_G[k] & 1;
    sprite_z_vel[k] += j ? -1 : 1;
    if (sprite_z_vel[k] == (uint8)(j ? -8 : 8))
      sprite_G[k]++;
  }
  Sprite_MoveZ(k);
  PointU8 pt;
  Sprite_DirectionToFaceLink(k, &pt);
  if ((uint8)(pt.x + 40) < 80 && (uint8)(pt.y + 40) < 80 && player_oam_y_offset != 0x80 &&
      (link_is_running || sign8(button_b_frames - 9))) {
    ProjectSpeedRet pp = Sprite_ProjectSpeedTowardsLink(k, 0x30);
    sprite_x_vel[k] = -pp.x;
    sprite_y_vel[k] = -pp.y;
    sprite_delay_main[k] = 8;
    sprite_ai_state[k] = 2;
  }
  switch(sprite_ai_state[k]) {
  case 0:
    if (!sprite_delay_main[k]) {
      static const int8 kBomber_Xvel[8] = {16, 12, 0, -12, -16, -12, 0, 12};
      static const int8 kBomber_Yvel[8] = {0, 12, 16, 12, 0, -12, -16, -12};
      static const uint8 kBomber_Tab0[4] = {0, 4, 2, 6};
      int j;
      sprite_ai_state[k]++;
      sprite_B[k]++;
      if (sprite_B[k] == 3) {
        sprite_B[k] = 0;
        sprite_delay_main[k] = 48;
        j = kBomber_Tab0[Sprite_DirectionToFaceLink(k, NULL)];
      } else {
        j = GetRandomNumber();
        sprite_delay_main[k] = j & 0x1f | 0x20;
        j &= 7;
      }
      sprite_x_vel[k] = kBomber_Xvel[j];
      sprite_y_vel[k] = kBomber_Yvel[j];
    }
    goto set_dir;
  case 1:
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k] = 0;
      sprite_delay_main[k] = 10;
      if (sprite_type[k] == 0xa8)
        sprite_delay_aux1[k] = 16;
    } else {
      Sprite_MoveXY(k);
set_dir:
      sprite_D[k] = Sprite_DirectionToFaceLink(k, NULL);
      sprite_graphics[k] = sprite_D[k] << 1 | ++sprite_subtype2[k] >> 3 & 1;
    }
    break;
  case 2:
    if (!sprite_delay_main[k])
      sprite_ai_state[k] = 0;
    sprite_subtype2[k] += 2;
    Sprite_MoveXY(k);
    goto set_dir;
  }
}

void Zirro_DropBomb(int k) {  // 9e8f81
  static const int8 kBomber_SpawnPellet_X[4] = {14, -6, 4, 4};
  static const int8 kBomber_SpawnPellet_Y[4] = {7, 7, 12, -4};
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0xa8, &info);
  if (j >= 0) {
    SpriteSfx_QueueSfx2WithPan(k, 0x20);
    sprite_z[j] = info.r4_z;
    int i = sprite_D[j];
    Sprite_SetX(j, info.r0_x + kBomber_SpawnPellet_X[i]);
    Sprite_SetY(j, info.r2_y + kBomber_SpawnPellet_Y[i]);
    sprite_x_vel[j] = kFluteBoyAnimal_Xvel[i];
    sprite_y_vel[j] = kZazak_Yvel[i];
    sprite_A[j] = 1;
    sprite_ignore_projectile[j] = 1;
    sprite_flags4[j] = 9;
    sprite_flags3[j] = 0x33;
    sprite_oam_flags[j] = 0x33 & 15;
  }
}

void Sprite_StalfosBone(int k) {  // 9e8fdf
  StalfosBone_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_CheckDamageToLink(k);
  sprite_subtype2[k]++;
  Sprite_MoveXY(k);
  if (!sprite_delay_main[k] && Sprite_CheckTileCollision(k)) {
    sprite_state[k] = 0;
    Sprite_PlaceWeaponTink(k);
  }
}

void StalfosBone_Draw(int k) {  // 9e9040
  static const DrawMultipleData kStalfosBone_Dmd[8] = {
    {-4, -2, 0x802f, 0},
    { 4,  2, 0x402f, 0},
    {-4,  2, 0x002f, 0},
    { 4, -2, 0xc02f, 0},
    { 2, -4, 0x403f, 0},
    {-2,  4, 0x803f, 0},
    {-2, -4, 0x003f, 0},
    { 2,  4, 0xc03f, 0},
  };
  Sprite_DrawMultiple(k, &kStalfosBone_Dmd[(sprite_subtype2[k] >> 2 & 3) * 2], 2, NULL);
}

void Sprite_A7_Stalfos(int k) {  // 9e906c
  if (sprite_A[k]) {
    Sprite_StalfosBone(k);
    return;
  }
  if (!sprite_E[k]) {
    Stalfos_Skellington(k);
    return;
  }
  if (!sprite_delay_main[k]) {
    sprite_x_vel[k] = 1;
    sprite_y_vel[k] = 1;
    if (Sprite_CheckTileCollision(k)) {
      sprite_state[k] = 0;
      return;
    }
    sprite_E[k] = 0;
    SpriteSfx_QueueSfx2WithPan(k, 0x15);
    Sprite_SpawnPoofGarnish(k);
    sprite_delay_aux2[k] = 8;
    sprite_delay_main[k] = 64;
    sprite_y_vel[k] = 0;
    sprite_x_vel[k] = 0;
  }
  PrepOamCoordsRet info;
  Sprite_PrepOamCoord(k, &info);
}

void Stalfos_Skellington(int k) {  // 9e90b5
  static const uint8 kStalfos_AnimState2[4] = {8, 9, 10, 11};
  static const uint8 kStalfos_CheckDir[4] = {3, 2, 1, 0};
  if (sprite_state[k] == 9 &&
      (uint16)(link_x_coord - cur_sprite_x + 40) < 80 &&
      (uint16)(link_y_coord - cur_sprite_y + 48) < 80 &&
      player_oam_y_offset != 0x80 &&
      !(sprite_z[k] | sprite_pause[k]) &&
      sprite_floor[k] == link_is_on_lower_level) {
    uint8 dir = Sprite_DirectionToFaceLink(k, NULL);
    if (!link_is_running) {
      if (button_b_frames >= 0x90)
        goto if_1;
      if (!sign8(button_b_frames - 9))
        goto endif_1;
    }
    if (dir != kStalfos_CheckDir[link_direction_facing >> 1]) {
if_1:
      sprite_D[k] = dir;
      ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLink(k, 32);
      sprite_x_vel[k] = -pt.x;
      sprite_y_vel[k] = -pt.y;
      sprite_z_vel[k] = 32;
      SpriteSfx_QueueSfx3WithPan(k, 0x13);
      sprite_z[k]++;
    }
  }
endif_1:
  if (sprite_z[k] == 0) {
    Sprite_Zazak_Main(k);
    return;
  }
  sprite_graphics[k] = kStalfos_AnimState2[sprite_D[k]];
  Stalfos_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (sprite_F[k])
    sprite_z_vel[k] = 0;
  if (Sprite_ReturnIfRecoiling(k))
    return;
  Sprite_CheckDamageToAndFromLink(k);
  uint8 t = Sprite_CheckTileCollision(k);
  if (t & 3)
    sprite_x_vel[k] = 0;
  if (t & 12)
    sprite_y_vel[k] = 0;
  Sprite_MoveXYZ(k);
  sprite_z_vel[k] -= 2;
  if (sign8(sprite_z[k] - 1)) {
    sprite_z[k] = 0;
    Sprite_ZeroVelocity_XY(k);
    SpriteSfx_QueueSfx2WithPan(k, 0x21);
    if (sprite_subtype[k]) {
      sprite_delay_aux3[k] = 16;
      sprite_subtype2[k] = 0;
    }
  }
}

void Sprite_Zazak_Main(int k) {  // 9e919f
  static const uint8 kStalfos_AnimState1[8] = {6, 4, 0, 2, 7, 5, 1, 3};
  static const uint8 kStalfos_Delay[4] = {16, 32, 64, 32};

  static const int8 kZazak_Yvel[4] = {0, 0, 16, -16};
  if (sprite_B[k]) {
    FirePhlegm_Draw(k);
    if (Sprite_ReturnIfInactive(k))
      return;
    sprite_graphics[k] = frame_counter >> 1 & 1;
    Sprite_CheckDamageToLink(k);
    Sprite_MoveXY(k);
    if (Sprite_CheckTileCollision(k)) {
      sprite_state[k] = 0;
      Sprite_PlaceRupulseSpark_2(k);
    }
    return;
  }
  int t = sprite_delay_aux3[k];
  if (t != 0) {
    sprite_ai_state[k] = 0;
    sprite_delay_main[k] = 32;
    Sprite_ZeroVelocity_XY(k);
    sprite_head_dir[k] = Sprite_DirectionToFaceLink(k, NULL);
  }
  if (t == 1) {
    Stalfos_ThrowBone(k);
    sprite_subtype2[k] = 1;
  }
  sprite_graphics[k] = kStalfos_AnimState1[(sprite_subtype2[k] & 1) * 4 + sprite_D[k]];
  if (sprite_type[k] == 0xa7)
    Stalfos_Draw(k);
  else
    Zazak_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (Sprite_ReturnIfRecoiling(k))
    return;
  Sprite_CheckDamageToAndFromLink(k);
  Sprite_MoveXY(k);
  Sprite_CheckTileCollision(k);
  switch (sprite_ai_state[k]) {
  case 0:  // walk then track head
    if (!sprite_delay_main[k]) {
      sprite_delay_main[k] = kStalfos_Delay[GetRandomNumber() & 3];
      sprite_ai_state[k] = 1;
      int j = sprite_head_dir[k];
      sprite_D[k] = j;
      sprite_x_vel[k] = kFluteBoyAnimal_Xvel[j];
      sprite_y_vel[k] = kZazak_Yvel[j];
    }
    break;
  case 1:  // halt and change dir
    if (sprite_wallcoll[k] != 0) {
      sprite_delay_main[k] = 16;
    } else {
      if (sprite_delay_main[k] != 0) {
        if (sign8(--sprite_G[k])) {
          sprite_G[k] = 11;
          sprite_subtype2[k]++;
        }
        return;
      } else if (sprite_type[k] == 0xa6 &&
                 sprite_D[k] == Sprite_DirectionToFaceLink(k, NULL) &&
                 sprite_floor[k] == link_is_on_lower_level) {
        sprite_ai_state[k] = 2;
        sprite_delay_main[k] = 48;
        sprite_delay_aux1[k] = 48;
        sprite_y_vel[k] = sprite_x_vel[k] = 0;
        return;
      }
      sprite_delay_main[k] = 32;
    }
    sprite_head_dir[k] = kZazak_Dir2[sprite_D[k] * 2 + (GetRandomNumber() & 1)];
    sprite_ai_state[k] = 0;
    if (++sprite_C[k] == 4) {
      sprite_C[k] = 0;
      sprite_head_dir[k] = Sprite_DirectionToFaceLink(k, NULL);
      sprite_delay_main[k] = 24;
    }
    sprite_y_vel[k] = sprite_x_vel[k] = 0;
    break;
  case 2:  // shoot
    if (!sprite_delay_main[k])
      sprite_ai_state[k] = 0;
    else if (sprite_delay_main[k] == 24)
      Sprite_SpawnFirePhlegm(k);
    break;
  }
}

int Sprite_SpawnFirePhlegm(int k) {  // 9e92e4
  static const int8 kSpawnFirePhlegm_X[4] = {16, -8, 4, 4};
  static const int8 kSpawnFirePhlegm_Y[4] = {-2, -2, 8, -20};
  static const int8 kSpawnFirePhlegm_Xvel[4] = {48, -48, 0, 0};
  static const int8 kSpawnFirePhlegm_Yvel[4] = {0, 0, 48, -48};
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0xa5, &info);
  if (j >= 0) {
    SpriteSfx_QueueSfx3WithPan(k, 0x5);
    Sprite_SetSpawnedCoordinates(j, &info);
    int i = sprite_D[k];
    Sprite_SetX(j, info.r0_x + kSpawnFirePhlegm_X[i]);
    Sprite_SetY(j, info.r2_y + kSpawnFirePhlegm_Y[i]);
    sprite_x_vel[j] = kSpawnFirePhlegm_Xvel[i];
    sprite_y_vel[j] = kSpawnFirePhlegm_Yvel[i];
    sprite_flags3[j] |= 0x40;
    sprite_defl_bits[j] = 0x40;
    sprite_flags2[j] = 0x21;
    sprite_B[j] = 0x21;
    sprite_oam_flags[j] = 2;
    sprite_flags4[j] = 0x14;
    sprite_ignore_projectile[j] = 20;
    sprite_bump_damage[j] = 37;
    if (link_shield_type >= 3)
      sprite_flags5[j] = 0x20;
  }
  return j;
}

void Stalfos_ThrowBone(int k) {  // 9e9379
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0xa7, &info);
  if (j >= 0) {
    sprite_A[j] = 1;
    Sprite_SetSpawnedCoordinates(j, &info);
    Sprite_ApplySpeedTowardsLink(j, 32);
    sprite_flags2[j] = 0x21;
    sprite_ignore_projectile[j] = 33;
    sprite_flags3[j] |= 0x40;
    sprite_defl_bits[j] = 0x48;
    sprite_delay_main[j] = 16;
    sprite_flags4[j] = 0x14;
    sprite_oam_flags[j] = 7;
    sprite_bump_damage[j] = 32;
    SpriteSfx_QueueSfx2WithPan(k, 0x2);
  }
}

void FirePhlegm_Draw(int k) {  // 9e9443
  static const DrawMultipleData kFirePhlegm_Dmd[16] = {
    { 0,  0, 0x00c3, 0},
    {-8,  0, 0x00c2, 0},
    { 0,  0, 0x80c3, 0},
    {-8,  0, 0x80c2, 0},
    { 0,  0, 0x40c3, 0},
    { 8,  0, 0x40c2, 0},
    { 0,  0, 0xc0c3, 0},
    { 8,  0, 0xc0c2, 0},
    { 0,  0, 0x00d4, 0},
    { 0, -8, 0x00d3, 0},
    { 0,  0, 0x40d4, 0},
    { 0, -8, 0x40d3, 0},
    { 0,  0, 0x80d4, 0},
    { 0,  8, 0x80d3, 0},
    { 0,  0, 0xc0d4, 0},
    { 0,  8, 0xc0d3, 0},
  };
  Sprite_DrawMultiple(k, &kFirePhlegm_Dmd[sprite_D[k] * 4 + sprite_graphics[k] * 2], 2, NULL);
}

void Sprite_A3_KholdstareShell(int k) {  // 9e9460
  if (Sprite_ReturnIfPaused(k))
    return;
  PointU8 pt;
  Sprite_DirectionToFaceLink(k, &pt);
  if ((uint8)(pt.x + 32) < 64 && (uint8)(pt.y + 32) < 64) {
    Sprite_NullifyHookshotDrag();
    Sprite_RepelDash();
  }
  Sprite_CheckDamageFromLink(k);
  if (sprite_ai_state[k] == 0) {
    if (sprite_state[k] == 6) {
      sprite_flags3[k] = 0xc0;
      sprite_ai_state[k] = 1;
      sprite_state[k] = 9;
    } else if (sprite_hit_timer[k] != 0) {
      dung_floor_x_offs = (sprite_hit_timer[k] & 2) ? -1 : 1;
      dung_hdr_collision_2_mirror = 1;
    } else {
      dung_hdr_collision_2_mirror = 0;
    }
  } else if (sprite_ai_state[k]++ != 18) {
    KholdstareShell_PaletteFiltering();
  } else {
    sprite_state[k] = 0;
    sprite_ai_state[2] = 2;
    sprite_delay_main[2] = 128;
  }
}

void GenerateIceball(int k) {  // 9e94dd
  if (++sprite_subtype2[k] & 127 | sprite_delay_aux1[k])
    return;
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0xa4, &info);
  if (j >= 0) {
    Sprite_SetX(j, link_x_coord);
    Sprite_SetY(j, link_y_coord);
    sprite_z[j] = -32;
    sprite_C[j] = -32;
    SpriteSfx_QueueSfx2WithPan(j, 0x20);
  }
}

void Sprite_A2_Kholdstare(int k) {  // 9e9518
  int j;

  Kholdstare_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (sprite_ai_state[k] < 2) {
    Kholdstare_SpawnPuffCloudGarnish(k);
    if (!(frame_counter & 7))
      sound_effect_1 = 2;
  }
  if (Sprite_ReturnIfRecoiling(k))
    return;

  if (sign8(--sprite_subtype2[k])) {
    sprite_subtype2[k] = 10;
    sprite_graphics[k] = sprite_graphics[k] + 1 & 3;
  }

  if (!(frame_counter & 3)) {
    ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLink(k, 31);
    sprite_A[k] = Sprite_ConvertVelocityToAngle(pt.x, pt.y);
  }

  Sprite_MoveXY(k);
  switch(sprite_ai_state[k]) {
  case 0:  // Accelerate
    Sprite_CheckDamageToAndFromLink(k);
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k] = 1;
      sprite_delay_main[k] = (GetRandomNumber() & 63) + 32;
      return;
    }
    if (sprite_x_vel[k] - sprite_z_vel[k])
      sprite_x_vel[k] += sign8(sprite_x_vel[k] - sprite_z_vel[k]) ? 1 : -1;
    if (sprite_y_vel[k] - sprite_z_subpos[k])
      sprite_y_vel[k] += sign8(sprite_y_vel[k] - sprite_z_subpos[k]) ? 1 : -1;
check_coll:
    j = Sprite_CheckTileCollision(k);
    if (j & 3) {
      sprite_x_vel[k] = -sprite_x_vel[k];
      sprite_z_vel[k] = -sprite_z_vel[k];
    }
    if (j & 12) {
      sprite_y_vel[k] = -sprite_y_vel[k];
      sprite_z_subpos[k] = -sprite_z_subpos[k];
    }
    break;
  case 1:  // Decelerate
    Sprite_CheckDamageToAndFromLink(k);
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k] = 0;
      sprite_delay_main[k] = (GetRandomNumber() & 63) + 96;
      j = GetRandomNumber();
      if (!(j & 0x1c)) {
        ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLink(k, 24);
        sprite_z_vel[k] = pt.x;
        sprite_z_subpos[k] = pt.y;
      } else {
        static const int8 kKholdstare_Target_Xvel[4] = {16, 16, -16, -16};
        static const int8 kKholdstare_Target_Yvel[4] = {-16, 16, 16, -16};
        sprite_z_vel[k] = kKholdstare_Target_Xvel[j & 3];
        sprite_z_subpos[k] = kKholdstare_Target_Yvel[j & 3];
      }
    } else {
      if (sprite_x_vel[k])
        sprite_x_vel[k] += sign8(sprite_x_vel[k]) ? 1 : -1;
      if (sprite_y_vel[k])
        sprite_y_vel[k] += sign8(sprite_y_vel[k]) ? 1 : -1;
      goto check_coll;
    }
    break;
  case 2:  // Triplicate
    if (sprite_delay_main[k] == 1) {
      sprite_state[k] = 0;
      sprite_state[k + 1] = 0;
      sprite_state[k + 2] = 0;
      for (int i = 2; i >= 0; i--) {
        static const int8 kKholdstare_Triplicate_Tab0[3] = {32, -32, 0};
        static const int8 kKholdstare_Triplicate_Tab1[3] = {-32, -32, 48};
        SpriteSpawnInfo info;
        j = Sprite_SpawnDynamicallyEx(k, 0xa2, &info, 4);
        assert(j >= 0);
        if (j >= 0) {
          Sprite_SetSpawnedCoordinates(j, &info);
          sprite_z_vel[j] = kKholdstare_Triplicate_Tab0[i];
          sprite_z_subpos[j] = kKholdstare_Triplicate_Tab1[i];
          sprite_delay_main[j] = 32;
        }
      }
      tmp_counter = 0xff;
    } else {
      sprite_hit_timer[k] |= 0xe0;
    }
    break;
  }
}

void Kholdstare_SpawnPuffCloudGarnish(int k) {  // 9e96a5
  static const int8 kNebuleGarnish_XY[8] = {-8, -6, -4, -2, 0, 2, 4, 6};
  if ((k ^ frame_counter) & 3)
    return;
  int j = GarnishAllocLow();
  if (j < 0)
    return;
  garnish_type[j] = 7;
  garnish_active = 7;
  garnish_countdown[j] = 31;
  Garnish_SetX(j, cur_sprite_x + kNebuleGarnish_XY[GetRandomNumber() & 7]);
  Garnish_SetY(j, cur_sprite_y + kNebuleGarnish_XY[GetRandomNumber() & 7] + 16);
  garnish_floor[j] = 0;
}

void Sprite_A4_FallingIce(int k) {  // 9e9710
  if (!sprite_C[k]) {
    if (Sprite_ReturnIfInactive(k))
      return;
    if (sprite_state[2] < 9 && sprite_state[3] < 9 && sprite_state[4] < 9)
      sprite_state[k] = 0;
    GenerateIceball(k);
    return;
  }

  sprite_ignore_projectile[k] = 1;
  sprite_obj_prio[k] = 0x30;
  SpriteDraw_SingleLarge(k);
  if (!sprite_ai_state[k])
    sprite_flags3[k] ^= 16;
  if (Sprite_ReturnIfInactive(k))
    return;
  if (sprite_delay_main[k]) {
    if (sprite_delay_main[k] == 1)
      sprite_state[k] = 0;
    sprite_graphics[k] = (sprite_delay_main[k] >> 3) + 2;
    return;
  }

  Sprite_MoveXY(k);
  if (!sprite_ai_state[k] || (Sprite_CheckDamageToLink(k), !Sprite_CheckTileCollision(k))) {
    uint8 old_z = sprite_z[k];
    Sprite_MoveZ(k);
    if (!sign8(sprite_z_vel[k] + 64))
      sprite_z_vel[k] -= 3;
    if (!(sign8(old_z ^ sprite_z[k]) && sign8(sprite_z[k])))
      return;
    sprite_z[k] = 0;
    if (!sprite_ai_state[k]) {
      sprite_state[k] = 0;
      IceBall_Split(k);
      return;
    }
  }
  sprite_delay_main[k] = 15;
  sprite_oam_flags[k] = 4;
  if (!sound_effect_1) {
    SpriteSfx_QueueSfx2WithPan(k, 0x1e);
    sprite_graphics[k] = 3;
  }
}

void IceBall_Split(int k) {  // 9e97cf
  static const int8 kIceBall_Quadruplicate_Xvel[8] = {0, 32, 0, -32, 24, 24, -24, -24};
  static const int8 kIceBall_Quadruplicate_Yvel[8] = {-32, 0, 32, 0, -24, 24, -24, 24};
  SpriteSfx_QueueSfx2WithPan(k, 0x1f);
  int b = GetRandomNumber() & 4;
  for (int i = 3; i >= 0; i--) {
    SpriteSpawnInfo info;
    int j = Sprite_SpawnDynamically(k, 0xa4, &info);
    if (j >= 0) {
      Sprite_SetSpawnedCoordinates(j, &info);
      sprite_ai_state[j] = 1;
      sprite_graphics[j] = 1;
      sprite_C[j] = 1;
      sprite_z_vel[j] = 32;
      sprite_x_vel[j] = kIceBall_Quadruplicate_Xvel[i + b];
      sprite_y_vel[j] = kIceBall_Quadruplicate_Yvel[i + b];
      sprite_flags4[j] = 0x1c;
    }
  }
  tmp_counter = 0xff;
}

void Sprite_A1_Freezor(int k) {  // 9e981d
  Freezor_Draw(k);
  if (sprite_state[k] != 9) {
    sprite_ai_state[k] = 3;
    sprite_delay_main[k] = 31;
    sprite_ignore_projectile[k] = 31;
    sprite_state[k] = 9;
    sprite_hit_timer[k] = 0;
  }
  if (Sprite_ReturnIfInactive(k))
    return;
  if (sprite_ai_state[k] != 3) {
    if (Sprite_ReturnIfRecoiling(k))
      return;
  }
  switch(sprite_ai_state[k]) {
  case 0:  // stasis
    sprite_ignore_projectile[k]++;
    if ((uint8)(Sprite_IsRightOfLink(k).b + 16) < 32) {
      sprite_ai_state[k] = 1;
      sprite_delay_main[k] = 32;
    }
    break;
  case 1: { // awakening
    sprite_ignore_projectile[k] = sprite_delay_main[k];
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k] = 2;
      uint16 x = Sprite_GetX(k) - 5;
      uint16 y = Sprite_GetY(k);
      Dungeon_UpdateTileMapWithCommonTile(x, y, 8);
      sprite_delay_aux1[k] = 96;
      sprite_D[k] = 2;
      sprite_delay_main[k] = 80;
    } else {
      sprite_x_vel[k] = (sprite_delay_main[k] & 1) ? -16 : 16;
      Sprite_MoveX(k);
    }
    break;
  }
  case 2: { // moving
    static const int8 kFreezor_Xvel[4] = {8, -8, 0, 0};
    static const int8 kFreezor_Yvel[4] = {0, 0, 18, -18};
    static const uint8 kFreezor_Moving_Gfx[4] = {1, 2, 1, 3};
    static const int8 kFreezor_Sparkle_X[8] = {-4, -2, 0, 2, 4, 6, 8, 10};
    Sprite_CheckDamageToLink(k);
    if (Sprite_CheckDamageFromLink(k))
      sprite_hit_timer[k] = 0;
    if (sprite_delay_aux1[k] && !((k ^ frame_counter) & 7)) {
      Sprite_GarnishSpawn_Sparkle(k, kFreezor_Sparkle_X[GetRandomNumber() & 7], -4);
    }
    if (!sprite_delay_main[k])
      sprite_D[k] = Sprite_DirectionToFaceLink(k, NULL);
    int j = sprite_D[k];
    sprite_x_vel[k] = kFreezor_Xvel[j];
    sprite_y_vel[k] = kFreezor_Yvel[j];
    if (!(sprite_wallcoll[k] & 15))
      Sprite_MoveXY(k);
    Sprite_CheckTileCollision(k);
    sprite_graphics[k] = kFreezor_Moving_Gfx[(k ^ frame_counter) >> 2 & 3];
    break;
  }
  case 3: {  // melting
    static const uint8 kFreezor_Melting_Gfx[4] = { 6, 5, 4, 7 };
    if (!sprite_delay_main[k]) {
      Sprite_ManuallySetDeathFlagUW(k);
      sprite_state[k] = 0;
    }
    sprite_graphics[k] = kFreezor_Melting_Gfx[sprite_delay_main[k] >> 3];
    break;
  }
  }
}

void Sprite_9E_HauntedGroveOstritch(int k) {  // 9e995b
  static const uint8 kFluteBoyOstrich_Gfx[4] = {0, 1, 0, 2};
  FluteBoyOstrich_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  switch (sprite_ai_state[k]) {
  case 0:  // wait
    sprite_graphics[k] = (frame_counter & 0x18) ? 3 : 0;
    if (byte_7E0FDD) {
      sprite_ai_state[k] = 1;
      sprite_y_vel[k] = -8;
      sprite_x_vel[k] = -16;
    }
    break;
  case 1:  // run away
    Sprite_MoveXYZ(k);
    sprite_z_vel[k] -= 2;
    if (sign8(sprite_z[k])) {
      sprite_z_vel[k] = 32;
      sprite_z[k] = 0;
      sprite_subtype2[k] = 0;
      sprite_A[k] = 0;
    }
    if (!(++sprite_subtype2[k] & 7) && sprite_A[k] != 3)
      sprite_A[k]++;
    sprite_graphics[k] = kFluteBoyOstrich_Gfx[sprite_A[k]];
    break;
  }
}

void FluteBoyOstrich_Draw(int k) {  // 9e9a4b
  static const DrawMultipleData kFluteBoyOstrich_Dmd[16] = {
    {-4, -8, 0x0080, 2},
    { 4, -8, 0x0081, 2},
    {-4,  8, 0x00a3, 2},
    { 4,  8, 0x00a4, 2},
    {-4, -8, 0x0080, 2},
    { 4, -8, 0x0081, 2},
    {-4,  8, 0x00a0, 2},
    { 4,  8, 0x00a1, 2},
    {-4, -8, 0x0080, 2},
    { 4, -8, 0x0081, 2},
    {-4,  8, 0x0083, 2},
    { 4,  8, 0x0084, 2},
    {-4, -7, 0x0080, 2},
    { 4, -7, 0x0081, 2},
    {-4,  9, 0x00a3, 2},
    { 4,  9, 0x00a4, 2},
  };
  PrepOamCoordsRet info;
  Sprite_DrawMultiple(k, &kFluteBoyOstrich_Dmd[sprite_graphics[k] * 4], 4, &info);
  SpriteDraw_Shadow_custom(k, &info, 18);
}

void Sprite_9F_HauntedGroveRabbit(int k) {  // 9e9a6d
  sprite_oam_flags[k] = sprite_oam_flags[k] & ~0x40 | kFluteBoyAnimal_OamFlags[sprite_D[k]];
  SpriteDraw_SingleLarge(k);
  switch (sprite_ai_state[k]) {
  case 0:  // wait
    sprite_graphics[k] = 3;
    if (byte_7E0FDD) {
      sprite_ai_state[k] = 1;
      sprite_D[k] ^= 1;
      sprite_x_vel[k] = kFluteBoyAnimal_Xvel[sprite_D[k]];
      sprite_y_vel[k] = -8;
    }
    break;
  case 1:  // run
    Sprite_MoveXYZ(k);
    sprite_z_vel[k]-=3;
    if (sign8(sprite_z[k])) {
      sprite_z_vel[k] = 24;
      sprite_z[k] = 0;
      sprite_subtype2[k] = 0;
      sprite_A[k] = 0;
    }
    sprite_subtype2[k]++;
    if (!(sprite_subtype2[k] & 3) && sprite_A[k] != 2)
      sprite_A[k]++;
    sprite_graphics[k] = kFluteBoyAnimal_Gfx[sprite_A[k]];
    break;
  }
}

void Sprite_A0_HauntedGroveBird(int k) {  // 9e9aec
  if (sprite_graphics[k] == 3)
    HauntedGroveBird_Blink(k);
  sprite_oam_flags[k] = sprite_oam_flags[k] & ~0x40 | kFluteBoyAnimal_OamFlags[sprite_D[k]];
  oam_cur_ptr += 4;
  oam_ext_cur_ptr++;
  sprite_flags2[k]--;
  SpriteDraw_SingleLarge(k);
  sprite_flags2[k]++;
  Sprite_MoveXYZ(k);
  switch (sprite_ai_state[k]) {
  case 0:  // wait
    sprite_graphics[k] = (frame_counter & 0x18) ? 0 : 3;
    if (byte_7E0FDD) {
      sprite_ai_state[k] = 1;
      sprite_D[k] ^= 1;
      sprite_x_vel[k] = kFluteBoyAnimal_Xvel[sprite_D[k]];
      sprite_delay_main[k] = 32;
      sprite_z_vel[k] = 16;
      sprite_y_vel[k] = -8;
    }
    break;
  case 1:  // rising
    if (!sprite_delay_main[k]) {
      sprite_z_vel[k] += 2;
      if (!sign8(sprite_z_vel[k] - 0x10))
        sprite_ai_state[k] = 2;
    }
    sprite_graphics[k] = (++sprite_subtype2[k] >> 1 & 1) + 1;
    break;
  case 2:  // falling
    sprite_graphics[k] = 1;
    sprite_z_vel[k] -= 1;
    if (sign8(sprite_z_vel[k] + 15))
      sprite_ai_state[k] = 1;
    break;
  }
}

void HauntedGroveBird_Blink(int k) {  // 9e9b9c
  static const int8 kFluteBoyBird_X[2] = {8, 0};
  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  OamEnt *oam = GetOamCurPtr();
  int j = sprite_D[k];
  oam->x = info.x + kFluteBoyBird_X[j];
  oam->y = info.y;
  oam->charnum = 0xae;
  oam->flags = info.flags | kFluteBoyAnimal_OamFlags[j];
  Sprite_CorrectOamEntries(k, 0, 0);
}

void Sprite_9C_Zoro(int k) {  // 9e9bc8
  if (sprite_E[k])
    Zoro(k);
  else
    Babasu(k);
//
}

void Zoro(int k) {  // 9e9bd0
  if (!sprite_C[k]) {
    sprite_C[k]++;
    if (Sprite_IsBelowLink(k).a != 0) {
      sprite_state[k] = 0;
      return;
    }
  }
  SpriteDraw_SingleSmall(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_CheckDamageToAndFromLink(k);
  sprite_graphics[k] = ++sprite_subtype2[k] >> 1 & 1;
  sprite_x_vel[k] = kFluteBoyAnimal_Xvel[sprite_subtype2[k] >> 2 & 1];
  Sprite_MoveXY(k);
  if (!sprite_delay_main[k] && Sprite_CheckTileCollision(k))
    sprite_state[k] = 0;

  if (sprite_subtype2[k] & 3)
    return;

  int j = GarnishAlloc();
  if (j < 0)
    return;
  garnish_type[j] = 6;
  garnish_active = 6;
  Garnish_SetX(j, Sprite_GetX(k));
  Garnish_SetY(j, Sprite_GetY(k) + 16);
  garnish_countdown[j] = 10;
  garnish_sprite[j] = k;
  garnish_floor[j] = sprite_floor[k];
}

void Babasu(int k) {  // 9e9c6b
  static const uint8 kBabusu_Gfx[6] = {5, 4, 3, 2, 1, 0};
  static const uint8 kBabusu_DirGfx[4] = {6, 6, 0, 0};
  static const int8 kBabusu_XyVel[6] = {32, -32, 0, 0, 32, -32};

  Babusu_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  switch(sprite_ai_state[k]) {
  case 0:  // reset
    sprite_ai_state[k]++;
    sprite_delay_main[k] = 128;
    sprite_graphics[k] = 255;
    break;
  case 1:  // hiding
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k]++;
      sprite_delay_main[k] = 55;
    }
    break;
  case 2: { // terror sprinkles
    int j = sprite_delay_main[k], i = sprite_D[k];
    if (j == 0) {
      sprite_ai_state[k] = 3;
      sprite_x_vel[k] = kBabusu_XyVel[i];
      sprite_y_vel[k] = kBabusu_XyVel[i + 2];
      sprite_delay_main[k] = 32;
    }
    if (j >= 32) {
      sprite_graphics[k] = kBabusu_Gfx[(j - 32) >> 2] + kBabusu_DirGfx[i];
    } else {
      sprite_graphics[k] = 0xff;
    }
    break;
  }
  case 3: { // scurry across
    static const uint8 kBabusu_Scurry_Gfx[4] = {18, 14, 12, 16};
    Sprite_CheckDamageToAndFromLink(k);
    Sprite_MoveXY(k);
    sprite_graphics[k] = (frame_counter >> 1 & 1) + kBabusu_Scurry_Gfx[sprite_D[k]];
    if (!sprite_delay_main[k] && Sprite_CheckTileCollision(k)) {
      sprite_D[k] ^= 1;
      sprite_ai_state[k] = 0;
    }
    break;
  }
  }
}

void Sprite_9B_Wizzrobe(int k) {  // 9e9d1b
  int j;
  if (sprite_C[k]) {
    Sprite_Wizzbeam(k);
    return;
  }
  if (sprite_ai_state[k] == 0 || sprite_ai_state[k] & 1 && sprite_delay_main[k] & 1) {
    PrepOamCoordsRet info;
    Sprite_PrepOamCoord(k, &info);
  } else {
    Wizzrobe_Draw(k);
  }
  if (Sprite_ReturnIfInactive(k))
    return;
  if (Sprite_ReturnIfRecoiling(k))
    return;
  sprite_ignore_projectile[k] = 1;
  switch(sprite_ai_state[k]) {
  case 0:  // cloaked
    if (!sprite_delay_main[k]) {
      sprite_y_vel[k] = sprite_x_vel[k] = 1;
      if (!Sprite_CheckTileCollision(k)) {
        static const uint8 kWizzrobe_Cloak_Gfx[4] = {4, 2, 0, 6};
        sprite_ai_state[k] = 1;
        sprite_delay_main[k] = 63;
        sprite_D[k] = j = Sprite_DirectionToFaceLink(k, NULL);
        sprite_graphics[k] = kWizzrobe_Cloak_Gfx[j];
      } else {
        sprite_state[k] = 0;
      }
    }
    break;
  case 1:  // phasing in
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k] = 2;
      sprite_delay_main[k] = 63;
    }
    break;
  case 2: {  // attack
    static const uint8 kWizzrobe_Attack_Gfx[8] = {0, 1, 1, 1, 1, 1, 1, 0};
    static const uint8 kWizzrobe_Attack_DirGfx[4] = {4, 2, 0, 6};
    sprite_ignore_projectile[k] = 0;
    Sprite_CheckDamageToAndFromLink(k);
    j = sprite_delay_main[k];
    if (!j) {
      sprite_ai_state[k] = 3;
      sprite_delay_main[k] = 63;
      return;
    }
    if (j == 32)
      Wizzrobe_FireBeam(k);
    sprite_graphics[k] = kWizzrobe_Attack_Gfx[j >> 3] + kWizzrobe_Attack_DirGfx[sprite_D[k]];
    break;
  }
  case 3:  // phasing out
    if (!sprite_delay_main[k]) {
      if (sprite_B[k])
        sprite_state[k] = 0;
      sprite_ai_state[k] = 0;
      sprite_delay_main[k] = (GetRandomNumber() & 31) + 32;
    }
    break;
  }
}

void Wizzrobe_FireBeam(int k) {  // 9e9e15
  static const int8 kWizzrobe_Beam_XYvel[6] = {32, -32, 0, 0, 32, -32};
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0x9b, &info);
  if (j >= 0) {
    SpriteSfx_QueueSfx3WithPan(k, 0x36);
    sprite_C[j] = 1;
    sprite_ignore_projectile[j] = 1;
    Sprite_SetX(j, info.r0_x + 4);
    Sprite_SetY(j, info.r2_y);
    int i = sprite_D[k];
    sprite_x_vel[j] = kWizzrobe_Beam_XYvel[i];
    sprite_y_vel[j] = kWizzrobe_Beam_XYvel[i + 2];
    sprite_defl_bits[j] = 0x48;
    sprite_oam_flags[j] = 2;
    sprite_flags5[j] = link_shield_type == 3 ? 0x20 : 0;
    sprite_flags4[j] = 0x14;
  }
}

void Sprite_9A_Kyameron(int k) {  // 9e9e7b
  PrepOamCoordsRet info;
  if (!sprite_ai_state[k])
    Sprite_PrepOamCoord(k, &info);
  else
    Kyameron_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (Sprite_ReturnIfRecoiling(k))
    return;
  sprite_ignore_projectile[k] = 1;
  switch(sprite_ai_state[k]) {
  case 0:  // reset
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k] = 1;
      sprite_delay_main[k] = (GetRandomNumber() & 63) + 96;
      sprite_x_lo[k] = sprite_A[k];
      sprite_x_hi[k] = sprite_B[k];
      sprite_y_lo[k] = sprite_C[k];
      sprite_y_hi[k] = sprite_head_dir[k];
      sprite_subtype2[k] = 5;
      sprite_graphics[k] = 8;
    }
    break;
  case 1:  // puddleup
    if (!sprite_delay_main[k]) {
      sprite_delay_main[k] = 31;
      sprite_ai_state[k] = 2;
    }
    if (sign8(--sprite_subtype2[k])) {
      sprite_subtype2[k] = 5;
      sprite_graphics[k] = (sprite_graphics[k] + 1 & 3) + 8;
    }
    break;
  case 2: {  // coagulate
    static const int8 kKyameron_Coagulate_Gfx[8] = {4, 7, 14, 13, 12, 6, 6, 5};
    static const int8 kKyameron_Xvel[4] = {32, -32, 32, -32};
    static const int8 kKyameron_Yvel[4] = {32, 32, -32, -32};
    int j = sprite_delay_main[k];
    if (j == 0) {
      sprite_ai_state[k] = 3;
      j = Sprite_IsBelowLink(k).a * 2 + Sprite_IsRightOfLink(k).a;
      sprite_x_vel[k] = kKyameron_Xvel[j];
      sprite_y_vel[k] = kKyameron_Yvel[j];
    } else {
      if (j == 7)
        Sprite_SetY(k, Sprite_GetY(k) - 29);
      sprite_graphics[k] = kKyameron_Coagulate_Gfx[j >> 2];
    }
    break;
  }
  case 3: {  // moving
    sprite_ignore_projectile[k] = 0;
    if (!Sprite_CheckDamageToAndFromLink(k)) {
      Sprite_MoveXY(k);
      int j = Sprite_CheckTileCollision(k);
      if (j & 3) {
        sprite_x_vel[k] = -sprite_x_vel[k];
        sprite_anim_clock[k]++;
      }
      if (j & 12) {
        sprite_y_vel[k] = -sprite_y_vel[k];
        sprite_anim_clock[k]++;
      }
      if (sprite_anim_clock[k] < 3)
        goto skip_sound;
    }
    sprite_ai_state[k] = 4;
    sprite_delay_main[k] = 15;
    SpriteSfx_QueueSfx2WithPan(k, 0x28);
    static const uint8 kKyameron_Moving_Gfx[4] = {3, 2, 1, 0};
skip_sound:
    sprite_graphics[k] = kKyameron_Moving_Gfx[++sprite_subtype2[k] >> 3 & 3];
    if (!((k ^ frame_counter) & 7)) {
      uint16 x = (GetRandomNumber() & 0xf) - 4;
      uint16 y = (GetRandomNumber() & 0xf) - 4;
      Sprite_GarnishSpawn_Sparkle(k, x, y);
    }
    break;
  }
  case 4:  // disperse
    if (!sprite_delay_main[k]) {
      sprite_anim_clock[k] = 0;
      sprite_ai_state[k] = 0;
      sprite_z[k] = 0;
      sprite_delay_main[k] = 64;
    } else {
      sprite_graphics[k] = (sprite_delay_main[k] >> 2) + 15;
    }
    break;
  }
}

void Kyameron_Draw(int k) {  // 9ea158
  static const uint8 kKyameron_OamFlags[12] = {0x40, 0, 0, 0, 0, 0, 0, 0, 0, 0x40, 0xc0, 0x80};
  static const DrawMultipleData kKyameron_Dmd[28] = {
    { 1,   8, 0x00b4, 0},
    { 7,   8, 0x00b5, 0},
    { 4,  -3, 0x0086, 0},
    { 0, -13, 0x80a2, 2},
    { 2,   8, 0x00b4, 0},
    { 6,   8, 0x00b5, 0},
    { 4,  -6, 0x0096, 0},
    { 0, -20, 0x00a2, 2},
    { 4,  -1, 0x0096, 0},
    { 0, -27, 0x00a2, 2},
    { 0, -27, 0x00a2, 2},
    { 0, -27, 0x00a2, 2},
    {-6,  -6, 0x01df, 0},
    {14,  -6, 0x41df, 0},
    {-6,  14, 0x81df, 0},
    {14,  14, 0xc1df, 0},
    {-6,  -6, 0x0096, 0},
    {14,  -6, 0x4096, 0},
    {-6,  14, 0x8096, 0},
    {14,  14, 0xc096, 0},
    {-4,  -4, 0x018d, 0},
    {12,  -4, 0x418d, 0},
    {-4,  12, 0x818d, 0},
    {12,  12, 0xc18d, 0},
    { 0,   0, 0x018d, 0},
    { 8,   0, 0x418d, 0},
    { 0,   8, 0x818d, 0},
    { 8,   8, 0xc18d, 0},
  };
  int j = sprite_graphics[k];
  if (j < 12) {
    uint8 bak = sprite_oam_flags[k];
    sprite_oam_flags[k] = sprite_oam_flags[k] & 0x3f | kKyameron_OamFlags[j];
    SpriteDraw_SingleLarge(k);
    sprite_oam_flags[k] = bak;
  } else {
    Sprite_DrawMultiple(k, &kKyameron_Dmd[(j - 12) * 4], 4, NULL);
  }
}

void Sprite_99_Pengator(int k) {  // 9ea196
  static const uint8 kPengator_Gfx[4] = {5, 0, 10, 15};
  sprite_graphics[k] = sprite_A[k] + kPengator_Gfx[sprite_D[k]];
  Pengator_Draw(k);
  if (sprite_F[k] || sprite_wallcoll[k] & 15) {
    sprite_ai_state[k] = 0;
    sprite_x_vel[k] = 0;
    sprite_y_vel[k] = 0;
  }
  if (Sprite_ReturnIfInactive(k))
    return;
  if (Sprite_ReturnIfRecoiling(k))
    return;
  Sprite_CheckDamageToAndFromLink(k);
  Sprite_MoveXYZ(k);
  sprite_z_vel[k] -= 2;
  if (sign8(sprite_z[k])) {
    sprite_z_vel[k] = 0;
    sprite_z[k] = 0;
  }
  Sprite_CheckTileCollision(k);
  switch(sprite_ai_state[k]) {
  case 0:  // face player
    sprite_D[k] = Sprite_DirectionToFaceLink(k, NULL);
    sprite_ai_state[k] = 1;
    break;
  case 1:  // speedup
    if (!((k ^ frame_counter) & 3)) {
      static const int8 kPengator_XYVel[6] = { 1, -1, 0, 0, 1, -1 };
      bool flag = false;
      int j = sprite_D[k];
      if (sprite_x_vel[k] != (uint8)kFluteBoyAnimal_Xvel[j])
        sprite_x_vel[k] += kPengator_XYVel[j], flag = true;
      if (sprite_y_vel[k] != (uint8)kZazak_Yvel[j])
        sprite_y_vel[k] += kPengator_XYVel[j + 2], flag = true;
      if (!flag) {
        sprite_delay_main[k] = 15;
        sprite_ai_state[k] = 2;
      }
    }
    sprite_A[k] = (frame_counter & 4) >> 2;
    break;
  case 2: { // jump
    static const uint8 kPengator_Jump[4] = {4, 4, 3, 2};
    if (!sprite_delay_main[k])
      sprite_ai_state[k]++;
    else if (sprite_delay_main[k] == 5)
      sprite_z_vel[k] = 24;
    sprite_A[k] = kPengator_Jump[sprite_delay_main[k] >> 2];
    break;
  }
  case 3:  // slide and sparkle
    if (!((k ^ frame_counter) & 7 | sprite_z[k])) {
      static const int8 kPengator_Garnish_Y[8] = { 8, 10, 12, 14, 12, 12, 12, 12 };
      static const int8 kPengator_Garnish_X[8] = { 4, 4, 4, 4, 0, 4, 8, 12 };
      int i = sprite_D[k];
      int x = kPengator_Garnish_X[(GetRandomNumber() & 3) + (i >= 2) * 4];
      int y = kPengator_Garnish_Y[(GetRandomNumber() & 3) + (i >= 2) * 4];
      Sprite_GarnishSpawn_Sparkle_limited(k, x, y);
    }
    break;
  }
}

void Pengator_Draw(int k) {  // 9ea415
  static const DrawMultipleData kPengator_Dmd0[40] = {
    {-1, -8, 0x0082, 2},
    { 0,  0, 0x0088, 2},
    {-1, -7, 0x0082, 2},
    { 0,  0, 0x008a, 2},
    {-3, -6, 0x0082, 2},
    { 0,  0, 0x0088, 2},
    {-6, -4, 0x0082, 2},
    { 0,  0, 0x008a, 2},
    {-4,  0, 0x00a2, 2},
    { 4,  0, 0x00a3, 2},
    { 1, -8, 0x4082, 2},
    { 0,  0, 0x4088, 2},
    { 1, -7, 0x4082, 2},
    { 0,  0, 0x408a, 2},
    { 3, -6, 0x4082, 2},
    { 0,  0, 0x4088, 2},
    { 6, -4, 0x4082, 2},
    { 0,  0, 0x408a, 2},
    { 4,  0, 0x40a2, 2},
    {-4,  0, 0x40a3, 2},
    { 0, -7, 0x0080, 2},
    { 0,  0, 0x0086, 2},
    { 0, -7, 0x4080, 2},
    { 0,  0, 0x4086, 2},
    { 0, -4, 0x0080, 2},
    { 0,  0, 0x0086, 2},
    { 0, -1, 0x0080, 2},
    { 0,  0, 0x0086, 2},
    {-8,  0, 0x008e, 2},
    { 8,  0, 0x408e, 2},
    { 0, -8, 0x0084, 2},
    { 0,  0, 0x008c, 2},
    { 0, -8, 0x4084, 2},
    { 0,  0, 0x408c, 2},
    { 0, -7, 0x0084, 2},
    { 0,  0, 0x008c, 2},
    { 0,  0, 0x408c, 2},
    { 0, -6, 0x4084, 2},
    {-8,  0, 0x00a0, 2},
    { 8,  0, 0x40a0, 2},
  };
  static const DrawMultipleData kPengator_Dmd1[4] = {
    {0, 16, 0x00b5, 0},
    {8, 16, 0x40b5, 0},
    {0, -8, 0x00a5, 0},
    {8, -8, 0x40a5, 0},
  };
  PrepOamCoordsRet info;
  Sprite_DrawMultiple(k, &kPengator_Dmd0[sprite_graphics[k] * 2], 2, &info);
  int i;
  if ((i = 0, sprite_graphics[k] == 14) || (i = 1, sprite_graphics[k] == 19)) {
    oam_cur_ptr += 8, oam_ext_cur_ptr += 2;
    Sprite_DrawMultiple(k, &kPengator_Dmd1[i * 2], 2, &info);
  }
  SpriteDraw_Shadow(k, &info);
}

void Sprite_LaserBeam(int k) {  // 9ea462
  SpriteDraw_SingleSmall(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  LaserBeam_BuildUpGarnish(k);
  Sprite_MoveXY(k);
  Sprite_CheckDamageToLink_same_layer(k);
  if (!sprite_delay_main[k] && Sprite_CheckTileCollision(k)) {
    sprite_state[k] = 0;
    SpriteSfx_QueueSfx3WithPan(k, 0x26);
  }
}

void LaserBeam_BuildUpGarnish(int k) {  // 9ea488
  int j = GarnishAllocOverwriteOld();
  garnish_type[j] = 4;
  garnish_active = 4;
  Garnish_SetX(j, Sprite_GetX(k));
  Garnish_SetY(j, Sprite_GetY(k) + 16);
  garnish_countdown[j] = 16;
  garnish_oam_flags[j] = sprite_graphics[k];
  garnish_sprite[j] = k;
  garnish_floor[j] = sprite_floor[k];
}

void Sprite_95_LaserEyeLeft(int k) {  // 9ea541
  static const uint8 kLaserEye_Dirs[4] = {2, 3, 0, 1};
  if (sprite_A[k]) {
    Sprite_LaserBeam(k);
    return;
  }
  LaserEye_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  switch(sprite_ai_state[k]) {
  case 0:  // monitor firing zone
    if (sprite_head_dir[k] == 0 && sprite_D[k] != kLaserEye_Dirs[link_direction_facing >> 1]) {
      sprite_graphics[k] = 0;
    } else {
      uint16 j = (sprite_D[k] < 2) ? link_y_coord - cur_sprite_y : link_x_coord - cur_sprite_x;
      if ((uint16)(j + 16) < 32) {
        sprite_delay_main[k] = 32;
        sprite_ai_state[k] = 1;
      } else {
        sprite_graphics[k] = 0;
      }
    }
    break;
  case 1:  // firing beam
    sprite_graphics[k] = 1;
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k] = 0;
      LaserEye_FireBeam(k);
      sprite_delay_aux4[k] = 12;
    }
    break;
  }
}

void LaserEye_FireBeam(int k) {  // 9ea5d8
  static const int8 kLaserEye_SpawnXY[6] = {12, -4, 4, 4, 12, -4};
  static const int8 kLaserEye_SpawnXYVel[6] = {112, -112, 0, 0, 112, -112};
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0x95, &info);
  if (j >= 0) {
    int i = sprite_D[k];
    sprite_graphics[j] = (i & 2) >> 1;
    Sprite_SetX(j, info.r0_x + kLaserEye_SpawnXY[i]);
    Sprite_SetY(j, info.r2_y + kLaserEye_SpawnXY[i + 2]);
    sprite_x_vel[j] = kLaserEye_SpawnXYVel[i];
    sprite_y_vel[j] = kLaserEye_SpawnXYVel[i + 2];
    sprite_flags2[j] = 0x20;
    sprite_A[j] = 0x20;
    sprite_oam_flags[j] = 5;
    sprite_defl_bits[j] = 0x48;
    sprite_ignore_projectile[j] = 0x48;
    sprite_delay_main[j] = 5;
    if (link_shield_type == 3)
      sprite_flags5[j] = 32;
    SpriteSfx_QueueSfx3WithPan(k, 0x19);
  }
}

void LaserEye_Draw(int k) {  // 9ea708
  static const DrawMultipleData kLaserEye_Dmd[24] = {
    { 8, -4, 0x40c8, 0},
    { 8,  4, 0x40d8, 0},
    { 8, 12, 0xc0c8, 0},
    { 8, -4, 0x40c9, 0},
    { 8,  4, 0x40d9, 0},
    { 8, 12, 0xc0c9, 0},
    { 0, -4, 0x00c8, 0},
    { 0,  4, 0x00d8, 0},
    { 0, 12, 0x80c8, 0},
    { 0, -4, 0x00c9, 0},
    { 0,  4, 0x00d9, 0},
    { 0, 12, 0x80c9, 0},
    {-4,  8, 0x00d6, 0},
    { 4,  8, 0x00d7, 0},
    {12,  8, 0x40d6, 0},
    {-4,  8, 0x00c6, 0},
    { 4,  8, 0x00c7, 0},
    {12,  8, 0x40c6, 0},
    {-4,  0, 0x80d6, 0},
    { 4,  0, 0x80d7, 0},
    {12,  0, 0xc0d6, 0},
    {-4,  0, 0x80c6, 0},
    { 4,  0, 0x80c7, 0},
    {12,  0, 0xc0c6, 0},
  };
  if (sprite_head_dir[k])
    sprite_graphics[k] = (sprite_delay_aux4[k] == 0);
  sprite_obj_prio[k] = 0x30;
  Sprite_DrawMultiple(k, &kLaserEye_Dmd[(sprite_graphics[k] + sprite_D[k] * 2) * 3], 3, NULL);
}

void Sprite_94_Pirogusu(int k) {  // 9ea742
  static const uint8 kPirogusu_A0[4] = {2, 3, 0, 1};
  static const uint8 kPirogusu_A1[8] = {9, 11, 5, 7, 5, 11, 7, 9};
  static const uint8 kPirogusu_A2[8] = {16, 17, 18, 19, 12, 13, 14, 15};
  static const int8 kPirogusu_XYvel[6] = {0, 0, 4, -4, 0, 0};
  static const int8 kPirogusu_XYvel2[6] = {2, -2, 0, 0, 2, -2};
  static const int8 kPirogusu_XYvel3[6] = {24, -24, 0, 0, 24, -24};

  if (sprite_E[k]) {
    Sprite_94_Tile(k);
    return;
  }
  sprite_obj_prio[k] |= 0x30;
  Pirogusu_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  switch(sprite_ai_state[k]) {
  case 0:  // wriggle in hole
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k] = 1;
      sprite_delay_main[k] = 31;
    }
    sprite_ignore_projectile[k] = sprite_delay_main[k];
    sprite_A[k] = kPirogusu_A0[sprite_D[k]];
    break;
  case 1: { // emerge
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k] = 2;
      sprite_delay_main[k] = 32;
      sprite_ignore_projectile[k] = 0;
      Sprite_ZeroVelocity_XY(k);
    } else {
      sprite_A[k] = kPirogusu_A1[sprite_delay_main[k] >> 3 & 1 | sprite_D[k] << 1];
      int j = sprite_D[k];
      sprite_x_vel[k] = kPirogusu_XYvel[j + 2];
      sprite_y_vel[k] = kPirogusu_XYvel[j];
      Sprite_MoveXY(k);
    }
    break;
  }
  case 2: {  // splash into play
    Sprite_CheckDamageToAndFromLink(k);
    Sprite_MoveXY(k);
    int j = sprite_D[k];
    sprite_x_vel[k] += kPirogusu_XYvel2[j];
    sprite_y_vel[k] += kPirogusu_XYvel2[j + 2];
    if (!sprite_delay_main[k]) {
      Sprite_SpawnSmallSplash(k);
      sprite_delay_aux1[k] = 16;
      sprite_ai_state[k] = 3;
    }
    sprite_A[k] = kPirogusu_A2[frame_counter >> 2 & 1 | sprite_D[k] << 1];
    break;
  }
  case 3: { // swim
    if (Sprite_ReturnIfRecoiling(k))
      return;
    Sprite_CheckDamageToAndFromLink(k);
    sprite_A[k] = kPirogusu_A2[frame_counter >> 2 & 1 | sprite_D[k] << 1] + 8;
    if (!sprite_delay_aux1[k]) {
      Pirogusu_SpawnSplash(k);
      Sprite_MoveXY(k);
      if (Sprite_CheckTileCollision(k) & 15) {
        static const uint8 kPirogusu_Dir[8] = {2, 3, 2, 3, 0, 1, 0, 1};
        sprite_D[k] = kPirogusu_Dir[sprite_D[k] << 1 | GetRandomNumber() & 1];
      }
      int j = sprite_D[k];
      sprite_x_vel[k] = kPirogusu_XYvel3[j];
      sprite_y_vel[k] = kPirogusu_XYvel3[j + 2];
    }
    break;
  }
  }
}

void Pirogusu_SpawnSplash(int k) {  // 9ea897
  static const uint8 kPirogusu_Tab0[4] = {3, 4, 5, 4};
  if ((k ^ frame_counter) & 3)
    return;
  int x = kPirogusu_Tab0[GetRandomNumber() & 3];
  int y = kPirogusu_Tab0[GetRandomNumber() & 3];
  int j = GarnishAllocLow();
  if (j >= 0) {
    garnish_type[j] = 11;
    garnish_active = 11;
    Garnish_SetX(j, Sprite_GetX(k) + x);
    Garnish_SetY(j, Sprite_GetY(k) + y + 16);
    garnish_countdown[j] = 15;
  }
}

void Pirogusu_Draw(int k) {  // 9ea93b
  static const uint8 kPirogusu_OamFlags[28] = {
       0, 0x80, 0x40,    0, 0,    0,    0, 0x80, 0x80, 0xc0, 0x40, 0x40, 0, 0x40, 0x80, 0xc0,
    0x40, 0xc0,    0, 0x80, 0, 0x40, 0x80, 0xc0, 0x40, 0xc0,    0, 0x80,
  };
  static const uint8 kPirogusu_Gfx[28] = {
    0, 0, 1, 1, 2, 3, 4, 3, 2, 3, 4, 3, 5, 5, 5, 5,
    7, 7, 7, 7, 6, 6, 6, 6, 8, 8, 8, 8,
  };
  int j = sprite_A[k];
  sprite_oam_flags[k] = sprite_oam_flags[k] & 0x3f | kPirogusu_OamFlags[j];
  sprite_graphics[k] = kPirogusu_Gfx[j];
  if (j < 4) {
    cur_sprite_x += 4, cur_sprite_y += 4;
    SpriteDraw_SingleSmall(k);
  } else {
    SpriteDraw_SingleLarge(k);
  }
}

void Sprite_93_Bumper(int k) {  // 9ea982
  static const int8 kBumper_Vels[4] = { 0, 2, -2, 0 };
  Bumper_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_CheckTileCollision(k);
  if (!link_cape_mode && Sprite_CheckDamageToLink_same_layer(k)) {
    Link_CancelDash();
    sprite_delay_main[k] = 32;
    ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLink(k, 0x30);
    link_actual_vel_y = pt.y + kBumper_Vels[joypad1H_last >> 2 & 3];
    link_actual_vel_x = pt.x + kBumper_Vels[joypad1H_last & 3];
    link_incapacitated_timer = 20;
    Link_ResetSwimmingState();
    SpriteSfx_QueueSfx3WithPan(k, 0x32);
  }
  for (int j = 15; j >= 0; j--) {
    if ((j ^ frame_counter) & 3 | sprite_z[j])
      continue;
    if (sprite_state[j] < 9 || (sprite_flags3[j] | sprite_flags4[j]) & 0x40)
      continue;
    int x = Sprite_GetX(j), y = Sprite_GetY(j);
    if ((uint16)(cur_sprite_x - x + 16) < 32 && (uint16)(cur_sprite_y - y + 16) < 32) {
      sprite_F[j] = 15;
      ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLocation(k, x, y, 0x40);
      sprite_y_recoil[j] = pt.y;
      sprite_x_recoil[j] = pt.x;
      sprite_delay_main[k] = 32;
      SpriteSfx_QueueSfx3WithPan(k, 0x32);
    }
  }
}

void Bumper_Draw(int k) {  // 9eaa8b
  static const DrawMultipleData kBumper_Dmd[8] = {
    {-8, -8, 0x00ec, 2},
    { 8, -8, 0x40ec, 2},
    {-8,  8, 0x80ec, 2},
    { 8,  8, 0xc0ec, 2},
    {-7, -7, 0x00ec, 2},
    { 7, -7, 0x40ec, 2},
    {-7,  7, 0x80ec, 2},
    { 7,  7, 0xc0ec, 2},
  };
  Sprite_DrawMultiple(k, &kBumper_Dmd[(sprite_delay_main[k] >> 1 & 1) * 4], 4, NULL);
}

void Sprite_91_StalfosKnight(int k) {  // 9eaaa7
  int j;
  if (!sprite_ai_state[k]) {
    PrepOamCoordsRet info;
    Sprite_PrepOamCoord(k, &info);
  } else {
    StalfosKnight_Draw(k);
  }
  if (Sprite_ReturnIfInactive(k))
    return;
  if ((sprite_hit_timer[k] & 127) == 1) {
    sprite_hit_timer[k] = 0;
    sprite_ai_state[k] = 6;
    sprite_delay_main[k] = 255;
    sprite_x_vel[k] = 0;
    sprite_y_vel[k] = 0;
    enemy_damage_data[0x918] = 2;
  }
  if (Sprite_ReturnIfRecoiling(k))
    return;
  switch(sprite_ai_state[k]) {
  case 0: { // waiting for player
    sprite_flags4[k] = 9;
    sprite_ignore_projectile[k] = 9;
    uint8 bak0 = sprite_flags2[k];
    sprite_flags2[k] |= 128;
    bool flag = Sprite_CheckDamageToLink(k);
    sprite_flags2[k] = bak0;
    if (flag) {
      sprite_z[k] = 144;
      sprite_ai_state[k] = 1;
      sprite_head_dir[k] = 2;
      sprite_graphics[k] = 2;
      SpriteSfx_QueueSfx2WithPan(k, 0x20);
    }
    break;
  }
  case 1: { // falling
    uint8 old_z = sprite_z[k];
    Sprite_MoveZ(k);
    if (!sign8(sprite_z_vel[k] + 64))
      sprite_z_vel[k] -= 3;
    if (sign8(old_z ^ sprite_z[k]) && sign8(sprite_z[k])) {
      sprite_ai_state[k] = 2;
      sprite_ignore_projectile[k] = 0;
      sprite_z[k] = 0;
      sprite_z_vel[k] = 0;
      sprite_delay_main[k] = 63;
    }
    break;
  }
  case 2: {  //
    static const uint8 kStalfosKnight_Case2_Gfx[2] = {0, 1};
    enemy_damage_data[0x918] = 0;
    Sprite_CheckDamageToAndFromLink(k);
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k] = 3;
      sprite_B[k] = GetRandomNumber() & 63;
      sprite_delay_main[k] = 127;
    } else {
      sprite_C[k] = sprite_graphics[k] = kStalfosKnight_Case2_Gfx[sprite_delay_main[k] >> 5];
      sprite_head_dir[k] = 2;
    }
    break;
  }
  case 3:  //
    Sprite_CheckDamageToAndFromLink(k);
    if (sprite_delay_main[k] == sprite_B[k]) {
      sprite_head_dir[k] = Sprite_IsRightOfLink(k).a;
      sprite_ai_state[k] = 4;
      sprite_delay_main[k] = 32;
    } else {
      static const uint8 kStalfosKnight_Case2_Dir[16] = {0, 0, 0, 2, 1, 1, 1, 2, 0, 0, 0, 2, 1, 1, 1, 2};
      sprite_head_dir[k] = kStalfosKnight_Case2_Dir[sprite_delay_main[k] >> 3];
      sprite_C[k] = 0;
      sprite_graphics[k] = 0;
    }
    break;
  case 4:  //
    Sprite_CheckDamageToAndFromLink(k);
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k] = 5;
      sprite_delay_main[k] = 255;
      sprite_delay_aux1[k] = 32;
    }
    sprite_C[k] = 1;
    sprite_graphics[k] = 1;
    break;
  case 5:  //
    Sprite_CheckDamageToAndFromLink(k);
    if (sprite_delay_aux1[k] == 0) {
      Sprite_MoveXYZ(k);
      Sprite_CheckTileCollision(k);
      if (!sign8(sprite_z_vel[k] + 64))
        sprite_z_vel[k] -= 2;
      if (sign8(sprite_z[k] - 1)) {
        sprite_z[k] = 0;
        sprite_z_vel[k] = 0;
        if (!sprite_delay_main[k])
          goto SetToGround;
        sprite_delay_aux1[k] = 16;
      }
      sprite_graphics[k] = sign8(sprite_z_vel[k] - 24) ? 2 : 0;
    } else {
      if (sprite_delay_aux1[k] == 1) {
        sprite_z_vel[k] = 48;
        Sprite_ApplySpeedTowardsLink(k, 16);
        sprite_head_dir[k] = Sprite_IsRightOfLink(k).a;
        SpriteSfx_QueueSfx3WithPan(k, 0x13);
      }
      sprite_C[k] = 1;
      sprite_graphics[k] = 1;
    }
    break;
  case 6:  //
    Sprite_MoveXYZ(k);
    Sprite_CheckTileCollision(k);
    if (!sign8(sprite_z_vel[k] + 64))
      sprite_z_vel[k] -= 2;
    if (sign8(sprite_z[k] - 1)) {
      sprite_z[k] = 0;
      sprite_z_vel[k] = 0;
    }
    j = sprite_delay_main[k];
    if (!j) {
      if (GetRandomNumber() & 1)
        goto SetToGround;
      sprite_ai_state[k] = 7;
      sprite_delay_main[k] = 80;
    } else {
      if (j >= 224 && (j & 3) == 0)
        SpriteSfx_QueueSfx3WithPan(k, 0x14);
      static const uint8 kStalfosKnight_Case6_C[32] = {
        0,  4,  8, 12, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
        14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 15, 14, 12,  8,  4,  0,
      };
      sprite_C[k] = kStalfosKnight_Case6_C[j >> 3];
      sprite_graphics[k] = 3;
      sprite_head_dir[k] = 2;
    }
    break;
  case 7:  //
    if (!sprite_delay_main[k]) {
SetToGround:
      sprite_ai_state[k] = 2;
      sprite_ignore_projectile[k] = 0;
      sprite_z[k] = 0;
      sprite_z_vel[k] = 0;
      sprite_delay_main[k] = 63;
    } else {
      static const uint8 kStalfosKnight_Case7_Gfx[2] = {1, 4};
      sprite_graphics[k] = kStalfosKnight_Case7_Gfx[sprite_delay_main[k] >> 2 & 1];
    }
    break;
  }
}

void StalfosKnight_Draw(int k) {  // 9eae04
  static const DrawMultipleData kStalfosKnight_Dmd[35] = {
    {-4, -8, 0x0064, 0},
    {-4,  0, 0x0061, 2},
    { 4,  0, 0x0062, 2},
    {-3, 16, 0x0074, 0},
    {11, 16, 0x4074, 0},
    {-4, -7, 0x0064, 0},
    {-4,  1, 0x0061, 2},
    { 4,  1, 0x0062, 2},
    {-3, 16, 0x0065, 0},
    {11, 16, 0x4065, 0},
    {-4, -8, 0x0048, 2},
    { 4, -8, 0x0049, 2},
    {-4,  8, 0x004b, 2},
    { 4,  8, 0x004c, 2},
    { 4,  8, 0x004c, 2},
    {-4,  8, 0x0068, 2},
    { 4,  8, 0x0069, 2},
    { 4,  8, 0x0069, 2},
    { 4,  8, 0x0069, 2},
    { 4,  8, 0x0069, 2},
    {12, -7, 0x4064, 0},
    {-4,  1, 0x4062, 2},
    { 4,  1, 0x4061, 2},
    {-3, 16, 0x0065, 0},
    {11, 16, 0x4065, 0},
    {12, -8, 0x4064, 0},
    {-4,  0, 0x4062, 2},
    { 4,  0, 0x4061, 2},
    {-3, 16, 0x0074, 0},
    {11, 16, 0x4074, 0},
    {-4, -8, 0x4049, 2},
    { 4, -8, 0x4048, 2},
    {-4,  8, 0x404c, 2},
    { 4,  8, 0x404b, 2},
    { 4,  8, 0x404b, 2},
  };
  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  SpriteDraw_StalfosKnight_Head(k, &info);
  oam_cur_ptr += 4, oam_ext_cur_ptr += 1;
  Sprite_DrawMultiple(k, &kStalfosKnight_Dmd[sprite_graphics[k] * 5], 5, &info);
  oam_cur_ptr -= 4, oam_ext_cur_ptr -= 1;
  SpriteDraw_Shadow_custom(k, &info, 18);
}

void SpriteDraw_StalfosKnight_Head(int k, PrepOamCoordsRet *info) {  // 9eae4e
  static const uint8 kStalfosKnight_DrawHead_Char[4] = {0x66, 0x66, 0x46, 0x46};
  static const uint8 kStalfosKnight_DrawHead_Flags[4] = {0x40, 0, 0, 0};
  if (sprite_graphics[k] == 2)
    return;
  int i = sprite_head_dir[k];
  OamEnt *oam = GetOamCurPtr();
  SetOamHelper0(oam, info->x, info->y + sprite_C[k] - 12,
                kStalfosKnight_DrawHead_Char[i],
                info->flags | kStalfosKnight_DrawHead_Flags[i], 2);
}

void Sprite_90_Wallmaster(int k) {  // 9eaea4
  sprite_obj_prio[k] |= 0x30;
  WallMaster_Draw(k);
  if (sprite_state[k] != 9) {
    flag_is_link_immobilized = 0;
    link_disable_sprite_damage = 0;
  }
  if (Sprite_ReturnIfInactive(k))
    return;
  if (sprite_A[k]) {
    link_x_coord = Sprite_GetX(k);
    link_y_coord = Sprite_GetY(k) - sprite_z[k] + 3;
    flag_is_link_immobilized = 1;
    link_disable_sprite_damage = 1;
    link_incapacitated_timer = 0;
    link_actual_vel_x = link_actual_vel_y = 0;
    link_y_vel = link_x_vel = 0;
    if ((uint16)(link_y_coord - BG2VOFS_copy2 - 16) >= 0x100) {
      flag_is_link_immobilized = 0;
      link_disable_sprite_damage = 0;
      WallMaster_SendPlayerToLastEntrance();
      Link_Initialize();
      return;
    }
  } else {
    Sprite_CheckDamageFromLink(k);
  }
  switch(sprite_ai_state[k]) {
  case 0: {  // Descend
    uint8 old_z = sprite_z[k];
    Sprite_MoveZ(k);
    if (!sign8(sprite_z_vel[k] + 64))
      sprite_z_vel[k] -= 3;
    if (sign8(old_z ^ sprite_z[k]) && sign8(sprite_z[k])) {
      sprite_ai_state[k] = 1;
      sprite_z[k] = 0;
      sprite_z_vel[k] = 0;
      sprite_delay_main[k] = 63;
    }
    break;
  }
  case 1:  // Attempt Grab
    if (!sprite_delay_main[k])
      sprite_ai_state[k] = 2;

    sprite_graphics[k] = (sprite_delay_main[k] & 0x20) ? 0 : 1;
    if (Sprite_CheckDamageToLink(k)) {
      sprite_A[k] = 1;
      sprite_flags3[k] = 64;
      SpriteSfx_QueueSfx3WithPan(k, 0x2a);
    }
    break;
  case 2: {  // Ascend
    uint8 old_z = sprite_z[k];
    Sprite_MoveZ(k);
    if (sign8(sprite_z_vel[k] - 64))
      sprite_z_vel[k] += 2;
    if (sign8(old_z ^ sprite_z[k]) && !sign8(sprite_z[k])) {
      sprite_state[k] = 0;
    }
    break;
  }
  }
}

void WallMaster_Draw(int k) {  // 9eafe4
  static const DrawMultipleData kWallMaster_Dmd[8] = {
    {-4,  0, 0x01a6, 2},
    {12,  0, 0x01aa, 0},
    {-4, 16, 0x01ba, 0},
    { 4,  8, 0x01a8, 2},
    {-4,  0, 0x01ab, 2},
    {12,  0, 0x01af, 0},
    {-4, 16, 0x01bf, 0},
    { 4,  8, 0x01ad, 2},
  };
  Sprite_DrawMultiple(k, &kWallMaster_Dmd[sprite_graphics[k] * 4], 4, NULL);
  Sprite_DrawLargeShadow2(k);
}

void Sprite_8F_Blob(int k) {  // 9eb002
  if (sprite_state[k] == 9 && sprite_E[k]) {
    sprite_E[k] = 0;
    sprite_x_vel[k] = 1;
    uint8 t = Sprite_CheckTileCollision(k);
    sprite_x_vel[k] = 0;
    if (t) {
      sprite_state[k] = 0;
      return;
    }
    SpriteSfx_QueueSfx2WithPan(k, 0x20);
  }
  if (sprite_C[k])
    sprite_obj_prio[k] = 0x30;
  Zol_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;

  if (sprite_ai_state[k] >= 2) {
    Sprite_CheckDamageFromLink(k);
  }
  if (Sprite_ReturnIfRecoiling(k))
    return;
  switch(sprite_ai_state[k]) {
  case 0: { // hiding unseen
    uint8 bak = sprite_flags4[k];
    sprite_flags4[k] |= 9;
    sprite_flags2[k] |= 0x80;
    uint8 t = Sprite_CheckDamageToLink(k);
    sprite_flags4[k] = bak;
    if (t) {
      sprite_ai_state[k]++;
      sprite_delay_main[k] = 127;
      sprite_flags2[k] &= ~0x80;
      Sprite_SetX(k, link_x_coord);
      Sprite_SetY(k, link_y_coord + 8);
      sprite_delay_aux4[k] = 48;
      sprite_ignore_projectile[k] = 0;
    }
    break;
  }
  case 1: { // popping out
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k]++;
      sprite_z_vel[k] = 32;
      Sprite_ApplySpeedTowardsLink(k, 16);
      SpriteSfx_QueueSfx3WithPan(k, 0x30);
    } else {
      static const int8 kZol_PoppingOutGfx[16] = {0, 1, 7, 7, 6, 6, 5, 5, 6, 6, 5, 5, 4, 4, 4, 4};
      sprite_graphics[k] = kZol_PoppingOutGfx[sprite_delay_main[k] >> 3];
    }
    break;
  }
  case 2: { // falling
    if (sprite_delay_main[k] == 0) {
      Sprite_CheckDamageFromLink(k);
      Sprite_MoveXY(k);
      Sprite_CheckTileCollision(k);
      uint8 oldz = sprite_z[k];
      Sprite_MoveZ(k);
      if (!sign8(sprite_z_vel[k] + 64))
        sprite_z_vel[k] -= 2;
      if (sign8(sprite_z[k] ^ oldz) && sign8(sprite_z[k])) {
        sprite_z_vel[k] = 0;
        sprite_z[k] = 0;
        sprite_C[k] = 0;
        sprite_delay_main[k] = 31;
        sprite_head_dir[k] = 8;
      }
    } else if (sprite_delay_main[k] == 1) {
      sprite_delay_main[k] = 32;
      sprite_ai_state[k]++;
      sprite_graphics[k] = 0;
    } else {
      static const int8 kZol_FallingXvel[2] = {-8, 8};
      static const int8 kZol_FallingGfx[2] = {0, 1};
      sprite_graphics[k] = kZol_FallingGfx[(sprite_delay_main[k] - 1) >> 4];
      sprite_x_vel[k] = kZol_FallingXvel[frame_counter >> 1 & 1];
      Sprite_MoveX(k);
    }
    break;
  }
  case 3:  // active
    Sprite_CheckDamageToLink(k);
    if (!sprite_delay_aux1[k]) {
      Sprite_ApplySpeedTowardsLink(k, 48);
      sprite_delay_aux1[k] = GetRandomNumber() & 63 | 96;
      sprite_oam_flags[k] = sprite_oam_flags[k] & 0x3f | (sign8(sprite_x_vel[k]) ? 0x40 : 0);
    }
    if (!sprite_delay_aux2[k]) {
      sprite_subtype2[k]++;
      if (!(sprite_subtype2[k] & 14 | sprite_wallcoll[k])) {
        Sprite_MoveXY(k);
        if (++sprite_G[k] == sprite_head_dir[k]) {
          sprite_G[k] = 0;
          sprite_delay_aux2[k] = (GetRandomNumber() & 31) + 64;
          sprite_head_dir[k] = GetRandomNumber() & 31 | 16;
        }
      }
      Sprite_CheckTileCollision(k);
      sprite_graphics[k] = (sprite_subtype2[k] & 8) >> 3;
    } else {
      sprite_graphics[k] = sprite_delay_aux2[k] & 0x10 ? 1 : 0;
    }
    break;
  }
}

void Zol_Draw(int k) {  // 9eb1c5
  PrepOamCoordsRet info;
  if (!(sprite_oam_flags[k] & 1) && byte_7E0FC6 >= 3)
    return;

  if (sprite_delay_aux4[k])
    Oam_AllocateFromRegionB(8);

  if (!sprite_ai_state[k]) {
    Sprite_PrepOamCoord(k, &info);
    return;
  }
  uint8 gfx = sprite_graphics[k];
  if (gfx < 4) {
    static const uint8 kZol_OamFlags[4] = {0, 0, 0x40, 0x40};
    uint8 bak1 = sprite_oam_flags[k];
    sprite_oam_flags[k] = bak1 ^ kZol_OamFlags[gfx];
    sprite_graphics[k] = gfx + ((sprite_oam_flags[k] & 1 ^ 1) << 2);
    SpriteDraw_SingleLarge(k);
    sprite_graphics[k] = gfx;
    sprite_oam_flags[k] = bak1;
  } else {
    static const DrawMultipleData kZol_Dmd[8] = {
      {0, 8, 0x036c, 0},
      {8, 8, 0x036d, 0},
      {0, 8, 0x0060, 0},
      {8, 8, 0x0070, 0},
      {0, 8, 0x4070, 0},
      {8, 8, 0x4060, 0},
      {0, 0, 0x0040, 2},
      {0, 0, 0x0040, 2},
    };
    Sprite_DrawMultiple(k, &kZol_Dmd[(gfx - 4) * 2], 2, NULL);
  }
}

void Sprite_8E_Terrorpin(int k) {  // 9eb26f
  int j;
  static const int8 kTerrorpin_Xvel[8] = {8, -8, 0, 0, 12, -12, 0, 0};
  static const int8 kTerrorpin_Yvel[8] = {0, 0, 8, -8, 0, 0, 12, -12};
  static const uint8 kTerrorpin_Oamflags[2] = {0, 0x40};
  static const int8 kTerrorpin_Overturned_Xvel[2] = {8, -8};
  SpriteDraw_SingleLarge(k);
  Sprite_CheckTileCollision(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (Sprite_ReturnIfRecoiling(k))
    return;
  if (!sprite_delay_aux2[k])
    Sprite_CheckDamageFromLink(k);
  Terrorpin_CheckForHammer(k);
  Sprite_MoveXYZ(k);
  switch (sprite_B[k]) {
  case 0:  // upright
    if (!sprite_delay_aux4[k]) {
      sprite_delay_aux4[k] = (GetRandomNumber() & 31) + 32;
      sprite_D[k] = Sprite_DirectionToFaceLink(k, NULL);
    }
    j = sprite_D[k] + sprite_G[k];
    sprite_x_vel[k] = kTerrorpin_Xvel[j];
    sprite_y_vel[k] = kTerrorpin_Yvel[j];
    sprite_z_vel[k] -= 2;
    if (sign8(sprite_z[k])) {
      sprite_z[k] = 0;
      sprite_z_vel[k] = 0;
    }
    sprite_graphics[k] = frame_counter >> (sprite_G[k] ? 2 : 3) & 1;
    sprite_flags3[k] |= 64;
    sprite_defl_bits[k] = 4;
    Sprite_CheckDamageToLink(k);
    break;
  case 1:  // overturned
    sprite_flags3[k] &= 191;
    sprite_defl_bits[k] = 0;
    if (!sprite_delay_aux4[k]) {
      sprite_B[k] = 0;
      sprite_z_vel[k] = 32;
      sprite_delay_aux4[k] = 64;
      return;
    }
    sprite_z_vel[k] -= 2;
    if (sign8(sprite_z[k])) {
      sprite_z[k] = 0;
      uint8 t = (uint8)-sprite_z_vel[k] >> 1;
      sprite_z_vel[k] = (t < 9) ? 0 : t;
      sprite_x_vel[k] = (int8)sprite_x_vel[k] >> 1;
      if (sprite_x_vel[k] == 0xff)
        sprite_x_vel[k] = 0;
      sprite_y_vel[k] = (int8)sprite_y_vel[k] >> 1;
      if (sprite_y_vel[k] == 0xff)
        sprite_y_vel[k] = 0;
    }
    if (sprite_delay_aux4[k] < 64) {
      sprite_x_vel[k] = kTerrorpin_Overturned_Xvel[sprite_delay_aux4[k] >> 1 & 1];
      sprite_subtype2[k]++;
    }
    sprite_graphics[k] = 2;
    sprite_oam_flags[k] = sprite_oam_flags[k] & ~0x40 | kTerrorpin_Oamflags[++sprite_subtype2[k] >> 3 & 1];
    break;
  }
}

void Terrorpin_CheckForHammer(int k) {  // 9eb3a3
  if (!(sprite_z[k] | sprite_delay_aux2[k]) &&
      sprite_floor[k] == link_is_on_lower_level &&
      player_oam_y_offset != 0x80 &&
      link_item_in_hand & 0xa) {
    SpriteHitBox hb;
    Player_SetupActionHitBox(&hb);
    Terrorpin_SetUpHammerHitBox(k, &hb);
    if (CheckIfHitBoxesOverlap(&hb)) {
      sprite_x_vel[k] = -sprite_x_vel[k];
      sprite_y_vel[k] = -sprite_y_vel[k];
      sprite_delay_aux2[k] = 32;
      sprite_z_vel[k] = 32;
      sprite_G[k] = 4;
      sprite_B[k] ^= 1;
      sprite_delay_aux4[k] = sprite_B[k] ? 0xff : 0x40;
    }
  }
  sprite_head_dir[k] = 0;
}

void Terrorpin_SetUpHammerHitBox(int k, SpriteHitBox *hb) {  // 9eb405
  int x = Sprite_GetX(k) - 16;
  int y = Sprite_GetY(k) - 16;
  hb->r4_spr_xlo = x;
  hb->r10_spr_xhi = x >> 8;
  hb->r5_spr_ylo = y;
  hb->r11_spr_yhi = y >> 8;
  hb->r6_spr_xsize = hb->r7_spr_ysize = 48;
}

void Sprite_8C_Arrghus(int k) {  // 9eb433
  static const uint8 kArrghus_Gfx[9] = {1, 1, 1, 2, 2, 1, 1, 0, 0};
  sprite_obj_prio[k] |= 0x30;
  Arrghus_Draw(k);
  if (sprite_state[k] != 9 || sprite_z[k] < 96) {
    if (Sprite_ReturnIfInactive(k))
      return;
  }
  Arrghus_HandlePuffs(k);
  overlord_x_lo[4] = 1;
  if ((sprite_hit_timer[k] & 127) == 2) {
    sprite_ai_state[k] = 3;
    SpriteSfx_QueueSfx3WithPan(k, 0x32);
    sprite_subtype2[k] = 0;
    sprite_flags3[k] = 64;
  }
  if (Sprite_ReturnIfRecoiling(k))
    return;
  Sprite_CheckDamageToLink(k);
  if (!(sprite_subtype2[k]++ & 3)) {
    if (++sprite_G[k] == 9)
      sprite_G[k] = 0;
    sprite_graphics[k] = kArrghus_Gfx[sprite_G[k]];
  }

  uint8 a = Sprite_CheckTileCollision(k);
  if (a) {
    if (sprite_ai_state[k] == 5) {
      if (a & 3)
        sprite_x_vel[k] = -sprite_x_vel[k];
      else
        sprite_y_vel[k] = -sprite_y_vel[k];
    } else {
      Sprite_ZeroVelocity_XY(k);
    }
  }

  switch (sprite_ai_state[k]) {
  case 0:  // approach speed
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k] = 1;
      sprite_delay_main[k] = 48;
    }
    Sprite_MoveXY(k);
    Sprite_ApproachTargetSpeed(k, sprite_head_dir[k], sprite_D[k]);
    break;
  case 1:  // decelerate
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k] = 0;
      if (!Sprite_CheckIfScreenIsClear()) {
        if (++overlord_x_lo[3] == 4) {
          overlord_x_lo[3] = 0;
          sprite_ai_state[k] = 2;
          sprite_delay_main[k] = 176;
        } else {
          sprite_delay_main[k] = (GetRandomNumber() & 63) + 48;
          ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLink(k, (sprite_delay_main[k] & 3) + 8);
          sprite_head_dir[k] = pt.x;
          sprite_D[k] = pt.y;
        }
      } else {
        sprite_ai_state[k] = 3;
        SpriteSfx_QueueSfx3WithPan(k, 0x32);
        sprite_subtype2[k] = 0;
      }
    } else {
      Sprite_MoveXY(k);
      Sprite_ApproachTargetSpeed(k, 0, 0);
    }
    break;
  case 2:  // case2
    overlord_x_lo[4] = 8;
    if (sprite_delay_main[k] < 32) {
      if (sign8(--overlord_x_lo[2])) {
        overlord_x_lo[2] = 0;
        sprite_ai_state[k] = 1;
        sprite_delay_main[k] = 112;
      }
    } else if (sprite_delay_main[k] < 96) {
      overlord_x_lo[2]++;
    } else if (sprite_delay_main[k] == 96) {
      SpriteSfx_QueueSfx3WithPan(k, 0x26);
    } else if ((sprite_delay_main[k] & 0xf) == 0) {
      SpriteSfx_QueueSfx3WithPan(k, 0x6);
    }
    break;
  case 3:  // jump way up
    sprite_z_vel[k] = 120;
    Sprite_MoveZ(k);
    if (sprite_z[k] >= 224) {
      sprite_delay_main[k] = 64;
      sprite_ai_state[k] = 4;
      sprite_z_vel[k] = 0;
      sprite_x_lo[k] = link_x_coord;
      sprite_y_lo[k] = link_y_coord;
    }
    break;
  case 4: {  // swoosh from above
    if (!(a = sprite_delay_main[k])) {
      sprite_z_vel[k] = 144;
      uint8 old_z = sprite_z[k];
      Sprite_MoveZ(k);
      if (sign8(a = old_z ^ sprite_z[k]) && sign8(a = sprite_z[k])) {
        sprite_z[k] = 0;
        Sprite_SpawnBigSplash(k);
        sprite_ai_state[k] = 5;
        sprite_delay_main[k] = 32;
        SpriteSfx_QueueSfx3WithPan(k, 0x3);
        sprite_x_vel[k] = 32;
        sprite_y_vel[k] = 32;
      }
    }
    if (a == 1) {  // wtf
      SpriteSfx_QueueSfx2WithPan(k, 0x20);
    }
    break;
  }
  case 5:  // swim frantically
    if (!sprite_delay_main[k]) {
      sprite_flags3[k] = 0;
      Sprite_MoveXY(k);
      Sprite_CheckDamageFromLink(k);
      if (!(frame_counter & 7)) {
        SpriteSfx_QueueSfx2WithPan(k, 0x28);
        int j = GarnishAllocLimit(sign8(sprite_y_vel[k]) ? 29 : 14);
        if (j >= 0) {
          garnish_type[j] = 21;
          garnish_active = 21;
          garnish_x_lo[j] = sprite_x_lo[k];
          garnish_x_hi[j] = sprite_x_hi[k];
          garnish_y_lo[j] = sprite_y_lo[k] + 24;  // why no carry propagation
          garnish_y_hi[j] = sprite_y_hi[k];
          garnish_countdown[j] = 15;
        }
      }
    }
    break;
  }
}

void Arrghus_Draw(int k) {  // 9eb840
  static const DrawMultipleData kArrghus_Dmd[5] = {
    {-8, -4, 0x0080, 2},
    { 8, -4, 0x4080, 2},
    {-8, 12, 0x00a0, 2},
    { 8, 12, 0x40a0, 2},
    { 0, 24, 0x00a8, 2},
  };
  Sprite_DrawMultiple(k, kArrghus_Dmd, 5, NULL);
  OamEnt *oam = GetOamCurPtr();
  uint8 chr = sprite_graphics[k] * 2;
  for (int i = 0; i < 4; i++)
    oam[i].charnum += chr;
  if (sprite_ai_state[k] == 5)
    oam[4].y = 0xf0;
  if (sprite_subtype2[k] & 8)
    oam[4].flags |= 0x40;

  if (sprite_ai_state[k] != 5) {
    oam_cur_ptr += 4, oam_ext_cur_ptr += 1;
    if (sprite_z[k] < 0xa0) {
      uint8 bak = sprite_oam_flags[k];
      sprite_oam_flags[k] &= ~1;
      SpriteDraw_BigShadow(k, 0);
      sprite_oam_flags[k] = bak;
    }
  } else {
    Sprite_DrawLargeWaterTurbulence(k);
  }

}

void Arrghus_HandlePuffs(int k) {  // 9eb8b4
  static const uint16 kArrgi_Tab0[13] = {0, 0x40, 0x80, 0xc0, 0x100, 0x140, 0x180, 0x1c0, 0, 0x66, 0xcc, 0x132, 0x198};
  static const uint16 kArrgi_Tab1[13] = {0, 0, 0, 0, 0, 0, 0, 0, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff};
  static const uint8 kArrgi_Tab2[13] = {0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0xc, 0xc, 0xc, 0xc, 0xc};
  static const int8 kArrgi_Tab3[52] = {
    0, -1, -2, -3, -4, -5, -6, -6, -5, -4, -3, -2, -1,  0, -1, -2,
    -3, -4, -5, -6, -6, -5, -4, -3, -2, -1,  0, -1, -2, -3, -4, -5,
    -6, -6, -5, -4, -3, -2, -1,  0, -1, -2, -3, -4, -5, -6, -6, -5,
    -4, -3, -2, -1,
  };

  WORD(overlord_x_lo[0]) += overlord_x_lo[4];
  if (!(frame_counter & 3) && ++sprite_A[k] == 13)
    sprite_A[k] = 0;
  if (!(frame_counter & 7) && ++sprite_B[k] == 13)
    sprite_B[k] = 0;
  for(int i = 0; i != 13; i++) {
    uint16 r0 = (WORD(overlord_x_lo[0]) + kArrgi_Tab0[i]) ^ kArrgi_Tab1[i];
    uint8 r14 = overlord_x_lo[2] + kArrgi_Tab2[i];

    int8 sin_val = ArrgiSin(r0 + 0x00, r14 + kArrgi_Tab3[sprite_A[k] + i]);
    int8 cos_val = ArrgiSin(r0 + 0x80, r14 + kArrgi_Tab3[sprite_B[k] + i]);

    int tx = Sprite_GetX(k) + sin_val;
    overlord_x_hi[i] = tx;
    overlord_y_hi[i] = tx >> 8;

    int ty = Sprite_GetY(k) + cos_val - 0x10;
    overlord_gen2[i] = ty;
    overlord_floor[i] = ty >> 8;
  }
  tmp_counter = 13;
}

void Sprite_8D_Arrghi(int k) {  // 9eb8c4
  static const uint8 kArrgi_Gfx[8] = {0, 1, 2, 2, 2, 2, 2, 1};

  sprite_obj_prio[k] |= 0x30;
  SpriteDraw_SingleLarge(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  sprite_graphics[k] = kArrgi_Gfx[++sprite_subtype2[k] >> 3 & 7];
  if (sprite_B[k]) {
    int j = sprite_B[k] - 1;
    if (ancilla_type[j]) {
      sprite_x_lo[k] = ancilla_x_lo[j];
      sprite_x_hi[k] = ancilla_x_hi[j];
      sprite_y_lo[k] = ancilla_y_lo[j];
      sprite_y_hi[k] = ancilla_y_hi[j];
      sprite_oam_flags[k] = 5;
      sprite_flags3[k] &= ~0x40;
      return;
    }
    sprite_ai_state[k] = 1;
    sprite_B[k] = 0;
    sprite_delay_main[k] = 32;
  }
  if (!sprite_delay_main[k])
    Sprite_CheckDamageToLink(k);

  if (!sprite_ai_state[k]) {
    sprite_x_lo[k] = overlord_x_lo[k + 7];
    sprite_x_hi[k] = overlord_y_lo[k + 7];
    sprite_y_lo[k] = overlord_gen1[k + 7];
    sprite_y_hi[k] = overlord_gen3[k + 7];
    return;
  }
  Sprite_CheckDamageFromLink(k);
  if (!((k ^ frame_counter) & 3)) {
    uint16 x = overlord_y_lo[k + 7] << 8 | overlord_x_lo[k + 7];
    uint16 y = overlord_gen3[k + 7] << 8 | overlord_gen1[k + 7];
    ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLocation(k, x, y, 4);
    sprite_y_vel[k] = pt.y;
    sprite_x_vel[k] = pt.x;
    if ((uint8)(sprite_x_lo[k] - overlord_x_lo[k + 7] + 8) < 16 && (uint8)(sprite_y_lo[k] - overlord_gen1[k + 7] + 8) < 16) {
      sprite_ai_state[k] = 0;
      sprite_oam_flags[k] = 0xd;
      sprite_flags3[k] |= 0x40;
    }
  }
  Sprite_MoveXY(k);

}

void Sprite_8B_Gibdo(int k) {  // 9eb9a9
  Gibdo_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (Sprite_ReturnIfRecoiling(k))
    return;
  Sprite_CheckDamageToAndFromLink(k);
  switch(sprite_ai_state[k]) {
  case 0: {
    static const uint8 kGibdo_DirTarget[4] = {2, 6, 4, 0};
    static const uint8 kGibdo_Gfx[8] = {4, 8, 11, 10, 0, 6, 3, 7};
    sprite_graphics[k] = kGibdo_Gfx[sprite_D[k]];
    if (!(frame_counter & 7)) {
      int j = sprite_A[k];
      if (sprite_D[k] - kGibdo_DirTarget[j]) {
        sprite_D[k] += sign8(sprite_D[k] - kGibdo_DirTarget[j]) ? 1 : -1;
      } else {
        sprite_delay_main[k] = (GetRandomNumber() & 31) + 48;
        sprite_ai_state[k] = 1;
      }
    }
    break;
  }
  case 1: {
    static const int8 kGibdo_XyVel[10] = {-16, 0, 0, 0, 16, 0, 0, 0, -16, 0};
    static const uint8 kGibdo_Gfx2[8] = {9, 2, 0, 4, 11, 3, 1, 5};
    int j = sprite_D[k];
    sprite_x_vel[k] = kGibdo_XyVel[j + 2];
    sprite_y_vel[k] = kGibdo_XyVel[j];
    Sprite_MoveXY(k);
    Sprite_CheckTileCollision(k);
    if ((!sprite_delay_main[k] || sprite_wallcoll[k]) && (j = Sprite_DirectionToFaceLink(k, NULL)) != sprite_A[k]) {
      sprite_A[k] = j;
      sprite_ai_state[k] = 0;
    } else {
      if (sign8(--sprite_B[k])) {
        sprite_B[k] = 14;
        sprite_subtype2[k]++;
      }
      sprite_graphics[k] = kGibdo_Gfx2[(sprite_subtype2[k] & 1) << 2 | sprite_A[k]];
    }
    break;
  }
  }
}

void Gibdo_Draw(int k) {  // 9ebb20
  static const DrawMultipleData kGibdo_Dmd[24] = {
    {0, -9, 0x0080, 2},
    {0,  0, 0x008a, 2},
    {0, -8, 0x0080, 2},
    {0,  1, 0x408a, 2},
    {0, -9, 0x0082, 2},
    {0,  0, 0x008c, 2},
    {0, -8, 0x0082, 2},
    {0,  0, 0x008e, 2},
    {0, -9, 0x0084, 2},
    {0,  0, 0x00a0, 2},
    {0, -8, 0x0084, 2},
    {0,  1, 0x40a0, 2},
    {0, -9, 0x0086, 2},
    {0,  0, 0x00a2, 2},
    {0, -9, 0x0088, 2},
    {0,  0, 0x00a4, 2},
    {0, -9, 0x4088, 2},
    {0,  0, 0x40a4, 2},
    {0, -9, 0x4082, 2},
    {0,  0, 0x408c, 2},
    {0, -9, 0x4086, 2},
    {0,  0, 0x40a2, 2},
    {0, -8, 0x4082, 2},
    {0,  1, 0x408e, 2},
  };
  PrepOamCoordsRet info;
  Sprite_DrawMultiple(k, &kGibdo_Dmd[sprite_graphics[k] * 2], 2, &info);
  if (!sprite_pause[k])
    SpriteDraw_Shadow(k, &info);
}

void Sprite_89_MothulaBeam(int k) {  // 9ebb42
  SpriteDraw_SingleLarge(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_CheckDamageToLink(k);
  if (!(frame_counter & 1))
    sprite_oam_flags[k] ^= 0x80;
  Sprite_MoveXY(k);
  if (!sprite_delay_main[k] && Sprite_CheckTileCollision(k))
    sprite_state[k] = 0;
  if ((k ^ frame_counter) & 3)
    return;
  for (int i = 14; i >= 0; i--) {
    if (garnish_type[i] == 0) {
      garnish_type[i] = 2;
      garnish_active = 2;
      garnish_x_lo[i] = sprite_x_lo[k];
      garnish_x_hi[i] = sprite_x_hi[k];
      garnish_y_lo[i] = sprite_y_lo[k];
      garnish_y_hi[i] = sprite_y_hi[k];
      garnish_countdown[i] = 16;
      garnish_sprite[i] = k;
      garnish_floor[i] = sprite_floor[k];
      break;
    }
  }
}

void Sprite_94_Tile(int k) {  // 9ebbb9
  sprite_obj_prio[k] = 0x30;
  FlyingTile_Draw(k);
  if (Sprite_ReturnIfPaused(k))
    return;
  if (sprite_hit_timer[k])
    goto lbl_a;
  sprite_ignore_projectile[k] = 1;
  switch (sprite_ai_state[k]) {
  case 0:  // erase tilemap
    Dungeon_UpdateTileMapWithCommonTile(Sprite_GetX(k), (sprite_y_lo[k] + 8) & 0xff | (sprite_y_hi[k] << 8), 6);
    sprite_ai_state[k] = 1;
    sprite_delay_main[k] = 128;
    break;
  case 1:  // rise up
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k] = 2;
      sprite_delay_main[k] = 16;
      Sprite_ApplySpeedTowardsLink(k, 32);
    } else {
      if (sprite_delay_main[k] >= 0x40) {
        sprite_z_vel[k] = 4;
        Sprite_MoveZ(k);
      }
lbl_b:
      sprite_graphics[k] = ++sprite_subtype2[k] >> 2 & 1;
      if (!((k ^ frame_counter) & 7))
        SpriteSfx_QueueSfx2WithPan(k, 0x7);
    }
    break;
  case 2:  // towards player
    sprite_ignore_projectile[k] = 0;
    if (sprite_delay_main[k] && (sprite_delay_main[k] & 3) == 0)
      Sprite_ApplySpeedTowardsLink(k, 32);
    if (!Sprite_CheckDamageToAndFromLink(k)) {
      Sprite_MoveXY(k);
      cur_sprite_y -= sprite_z[k];
      if (!Sprite_CheckTileCollision(k))
        goto lbl_b;
    }
lbl_a:
    SpriteSfx_QueueSfx2WithPan(k, 0x1f);
    sprite_state[k] = 6;
    sprite_delay_main[k] = 31;
    sprite_type[k] = 0xec;
    sprite_hit_timer[k] = 0;
    sprite_C[k] = 0x80;
    break;
  }
}

void FlyingTile_Draw(int k) {  // 9ebcca
  static const DrawMultipleData kFlyingTile_Dmd[8] = {
    {0, 0, 0x00d3, 0},
    {8, 0, 0x40d3, 0},
    {0, 8, 0x80d3, 0},
    {8, 8, 0xc0d3, 0},
    {0, 0, 0x00c3, 0},
    {8, 0, 0x40c3, 0},
    {0, 8, 0x80c3, 0},
    {8, 8, 0xc0c3, 0},
  };
  PrepOamCoordsRet info;
  Sprite_DrawMultiple(k, &kFlyingTile_Dmd[sprite_graphics[k] * 4], 4, &info);
  SpriteDraw_Shadow(k, &info);
}

void Sprite_8A_SpikeBlock(int k) {  // 9ebce8
  if (!sprite_E[k]) {
    SpriteDraw_SingleLarge(k);
    if (Sprite_ReturnIfInactive(k))
      return;
    Sprite_CheckDamageToAndFromLink(k);
    Sprite_MoveXY(k);
    Sprite_CheckTileCollision(k);
    if (!sprite_delay_main[k] && (!SpikeBlock_CheckStatueCollision(k) || (sprite_wallcoll[k] & 0xf))) {
      sprite_delay_main[k] = 4;
      sprite_x_vel[k] = -sprite_x_vel[k];
      SpriteSfx_QueueSfx2WithPan(k, 0x5);
    }
    return;
  }
  Oam_AllocateFromRegionB(4);
  SpriteDraw_SingleLarge(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_CheckDamageToAndFromLink(k);
  if (sprite_ai_state[k] == 0) {
    Dungeon_UpdateTileMapWithCommonTile(Sprite_GetX(k), Sprite_GetY(k), 0);
    sprite_ai_state[k] = 1;
    sprite_delay_main[k] = 64;
    sprite_delay_aux1[k] = 105;
  } else if (sprite_delay_main[k] != 0) {
    if (sprite_delay_main[k] == 1) {
      sprite_x_lo[k] = sprite_A[k];
      sprite_y_lo[k] = sprite_B[k];
    } else {
      sprite_x_vel[k] = (sprite_delay_main[k] >> 1) & 1 ? -8 : 8;
      Sprite_MoveX(k);
      sprite_x_vel[k] = 0;
    }
  } else if (sprite_ai_state[k] == 1) {
    static const int8 kSpikeBlock_XVelTarget[4] = {32, -32, 0, 0};
    static const int8 kSpikeBlock_YVelTarget[4] = {0, 0, 32, -32};
    static const int8 kSpikeBlock_XVelDelta[4] = {1, -1, 0, 0};
    static const int8 kSpikeBlock_YVelDelta[4] = {0, 0, 1, -1};

    int j = sprite_D[k];
    if (sprite_x_vel[k] != (uint8)kSpikeBlock_XVelTarget[j])
      sprite_x_vel[k] += kSpikeBlock_XVelDelta[j];
    if (sprite_y_vel[k] != (uint8)kSpikeBlock_YVelTarget[j])
      sprite_y_vel[k] += kSpikeBlock_YVelDelta[j];
    Sprite_MoveXY(k);
    if (!sprite_delay_aux1[k]) {
      Sprite_Get16BitCoords(k);
      if (Sprite_CheckTileCollision(k)) {
        sprite_ai_state[k] = 2;
        sprite_delay_aux1[k] = 64;
      }
    }
  } else if (sprite_delay_aux1[k] == 0) {
    static const int8 kSpikeBlock_XVel[4] = {-16, 16, 0, 0};
    static const int8 kSpikeBlock_YVel[4] = {0, 0, -16, 16};
    int j = sprite_D[k];
    sprite_x_vel[k] = kSpikeBlock_XVel[j];
    sprite_y_vel[k] = kSpikeBlock_YVel[j];
    Sprite_MoveXY(k);
    if (sprite_x_lo[k] == sprite_A[k] && sprite_y_lo[k] == sprite_B[k]) {
      sprite_state[k] = 0;
      Dungeon_UpdateTileMapWithCommonTile(Sprite_GetX(k), Sprite_GetY(k), 2);
    }
  }
}

bool SpikeBlock_CheckStatueCollision(int k) {  // 9ebe19
  for (int j = 15; j >= 0; j--) {
    if (!((j ^ frame_counter) & 1) && sprite_state[j] && sprite_type[j] == 0x1c) {
      int x0 = Sprite_GetX(k), y0 = Sprite_GetY(k);
      int x1 = Sprite_GetX(j), y1 = Sprite_GetY(j);
      if ((uint16)(x0 - x1 + 16) < 32 && (uint16)(y0 - y1 + 16) < 32)
        return false;
    }
  }
  return true;
}

void Sprite_88_Mothula(int k) {  // 9ebe7e
  if (enhanced_features0 & kFeatures0_MiscBugFixes) {
    // L4 sword and L3 spin slash can now damage Mothula
    enemy_damage_data[0x884] = 1;
    enemy_damage_data[0x885] = 1;
  }
  Mothula_Main(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Mothula_HandleSpikes(k);
}

void Mothula_Main(int k) {  // 9ebe88
  int j;
  Mothula_Draw(k);
  if (sprite_state[k] == 11)
    sprite_ai_state[k] = 0;
  if (Sprite_ReturnIfInactive(k))
    return;
  sprite_flags3[k] = 0;
  if (sprite_delay_aux3[k])
    sprite_flags3[k] = 64;
  if ((sprite_F[k] & 127) == 6) {
    sprite_F[k] = 0;
    sprite_delay_aux3[k] = 32;
    sprite_ai_state[k] = 2;
    sprite_delay_main[k] = 0;
    sprite_G[k] = 64;
  }
  if (Sprite_ReturnIfRecoiling(k))
    return;
  switch(sprite_ai_state[k]) {
  case 0: // Delay
    if (!sprite_delay_main[k])
      sprite_ai_state[k] = 1;
    break;
  case 1: // Ascend
    sprite_z_vel[k] = 8;
    Sprite_MoveZ(k);
    sprite_z_vel[k] = 0;
    if (sprite_z[k] >= 24) {
      sprite_G[k] = 128;
      sprite_ai_state[k] = 2;
      sprite_ignore_projectile[k] = 0;
      sprite_delay_main[k] = 64;
    }
    Mothula_FlapWings(k);
    break;
  case 2: // FlyAbout
    if (!sprite_G[k]) {
      sprite_delay_main[k] = 63;
      sprite_ai_state[k] = 3;
      return;
    }
    sprite_G[k]--;
    Mothula_FlapWings(k);
    j = sprite_A[k] & 1;
    sprite_z_vel[k] += j ? -1 : 1;
    if (sprite_z_vel[k] == (uint8)(j ? -16 : 16))
      sprite_A[k]++;
    if (!sprite_delay_main[k]) {
      if (++sprite_C[k] == 7) {
        sprite_C[k] = 0;
        Sprite_ApplySpeedTowardsLink(k, 32);
        sprite_delay_main[k] = 128;
      } else {
        static const int8 kMothula_XYvel[10] = {-16, -12, 0, 12, 16, 12, 0, -12, -16, -12};
        j = GetRandomNumber() & 7;
        sprite_x_vel[k] = kMothula_XYvel[j + 2];
        sprite_y_vel[k] = kMothula_XYvel[j];
        sprite_delay_main[k] = (GetRandomNumber() & 31) + 64;
      }
    }
    if (!sprite_wallcoll[k])
      Sprite_MoveXY(k);
    Sprite_MoveZ(k);
    if (Sprite_CheckTileCollision(k))
      sprite_delay_main[k] = 0;
    Sprite_CheckDamageToAndFromLink(k);
    sprite_subtype2[k] += 2;
    break;
  case 3: // FireBeams
    Sprite_CheckDamageToAndFromLink(k);
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k]--;
      sprite_G[k] = GetRandomNumber() & 31 | 64;
    } else {
      if (sprite_delay_main[k] == 0x20)
        Mothula_SpawnBeams(k);
      Mothula_FlapWings(k);
    }
    break;
  }
}

void Mothula_FlapWings(int k) {  // 9ebf9f
  static const uint8 kMothula_FlapWingsGfx[4] = {0, 1, 2, 1};
  int j = ++sprite_subtype2[k] >> 2 & 3;
  if (j == 0)
    SpriteSfx_QueueSfx3WithPan(k, 0x2);
  sprite_graphics[k] = kMothula_FlapWingsGfx[j];
}

void Mothula_SpawnBeams(int k) {  // 9ebfdf
  static const int8 kMothula_Beam_Xvel[3] = {-16, 0, 16};
  static const int8 kMothula_Beam_Yvel[3] = {24, 32, 24};
  SpriteSfx_QueueSfx3WithPan(k, 0x36);
  for (int i = 2; i >= 0; i--) {
    SpriteSpawnInfo info;
    int j = Sprite_SpawnDynamically(k, 0x89, &info);
    if (j >= 0) {
      Sprite_SetSpawnedCoordinates(j, &info);
      sprite_y_lo[j] = info.r2_y - info.r4_z + 3;
      sprite_delay_main[j] = 16;
      sprite_ignore_projectile[j] = 16;
      sprite_x_lo[j] = info.r0_x + kMothula_Beam_Xvel[i];
      sprite_x_vel[j] = kMothula_Beam_Xvel[i];
      sprite_y_vel[j] = kMothula_Beam_Yvel[i];
      sprite_z[j] = 0;
    }
  }
  tmp_counter = 0xff;
}

void Mothula_HandleSpikes(int k) {  // 9ec088
  static const uint8 kMothula_Spike_XLo[30] = {
    0x38, 0x48, 0x58, 0x68, 0x88, 0x98, 0xa8, 0xb8, 0xc8, 0xc8, 0xc8, 0xc8, 0xc8, 0xc8, 0xc8, 0xb8,
    0xa8, 0x98, 0x78, 0x68, 0x58, 0x48, 0x38, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28,
  };
  static const uint8 kMothula_Spike_YLo[30] = {
    0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x48, 0x58, 0x68, 0x78, 0x98, 0xa8, 0xb8, 0xc8,
    0xc8, 0xc8, 0xc8, 0xc8, 0xc8, 0xc8, 0xc8, 0xb8, 0xa8, 0x98, 0x78, 0x68, 0x58, 0x48,
  };
  static const uint8 kMothula_Spike_Dir[30] = {
    2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 3,
    3, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0,
  };

  if (--sprite_head_dir[k])
    return;
  sprite_head_dir[k] = 0x40;
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0x8a, &info);
  if (j < 0)
    return;
  int i = GetRandomNumber() & 0x1f;
  if (i >= 30) i -= 30;
  sprite_A[j] = sprite_x_lo[j] = kMothula_Spike_XLo[i];
  sprite_B[j] = sprite_y_lo[j] = kMothula_Spike_YLo[i] - 1;
  sprite_D[j] = kMothula_Spike_Dir[i];
  sprite_E[j] = 1;
  sprite_x_hi[j] = byte_7E0FB0 + 1;
  sprite_y_hi[j] = byte_7E0FB1 + 1;
  sprite_x_vel[j] = 1;
  Sprite_Get16BitCoords(j);
  Sprite_CheckTileCollision(j);
  sprite_x_vel[j] = 0;
  sprite_x_lo[j] = sprite_A[j];
  sprite_y_lo[j] = sprite_B[j];
  if (!sprite_wallcoll[j]) {
    sprite_state[j] = 0;
    sprite_head_dir[k] = 1;
  }
}

void Sprite_86_Kodongo(int k) {  // 9ec103
  static const int8 kKodondo_Xvel[4] = {1, -1, 0, 0};
  static const int8 kKodondo_Yvel[4] = {0, 0, 1, -1};
  static const uint8 kKodondo_Gfx[8] = {2, 2, 0, 5, 3, 3, 0, 5};
  static const uint8 kKodondo_OamFlags[8] = {0x40, 0, 0, 0, 0x40, 0, 0x40, 0x40};
  static const uint8 kKodondo_FlameGfx[8] = {2, 2, 0, 5, 4, 4, 1, 6};
  int j;
  SpriteDraw_SingleLarge(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (Sprite_ReturnIfRecoiling(k))
    return;
  Sprite_CheckDamageToAndFromLink(k);
  sprite_flags[k] = 0;
  switch(sprite_ai_state[k]) {
  case 0:  // choose dir
    sprite_ai_state[k]++;
    sprite_D[k] = GetRandomNumber() & 3;
    sprite_flags[k] = 176;
    for(;;) {
      j = sprite_D[k];
      sprite_x_vel[k] = kKodondo_Xvel[j];
      sprite_y_vel[k] = kKodondo_Yvel[j];
      if (!Sprite_CheckTileCollision(k))
        break;
      sprite_D[k] = (sprite_D[k] + 1) & 3;
    }
    Kodongo_SetDirection(k);
    break;
  case 1:  // move
    Sprite_MoveXY(k);
    if (Sprite_CheckTileCollision(k)) {
      sprite_D[k] ^= 1;
      Kodongo_SetDirection(k);
    }
    if ((sprite_x_lo[k] & 0x1f) == 4 && (sprite_y_lo[k] & 0x1f) == 0x1b && (GetRandomNumber() & 3) == 0) {
      sprite_delay_main[k] = 111;
      sprite_ai_state[k] = 2;
      sprite_A[k] = 0;
    }
    j = ++sprite_subtype2[k] & 4 | sprite_D[k];
    sprite_graphics[k] = kKodondo_Gfx[j];
    sprite_oam_flags[k] = sprite_oam_flags[k] & ~0x40 | kKodondo_OamFlags[j];
    break;
  case 2:  // breathe flame
    if (!sprite_delay_main[k])
      sprite_ai_state[k] = 0;
    j = (uint8)(sprite_delay_main[k] - 0x20) < 0x30;
    if (j && (sprite_delay_main[k] & 0xf) == 0)
      Kodongo_SpawnFire(k);
    sprite_graphics[k] = kKodondo_FlameGfx[j * 4 + sprite_D[k]];
    break;
  }
}

void Kodongo_SetDirection(int k) {  // 9ec158
  int j = sprite_D[k];
  sprite_x_vel[k] = kFluteBoyAnimal_Xvel[j];
  sprite_y_vel[k] = kZazak_Yvel[j];
}

void Kodongo_SpawnFire(int k) {  // 9ec223
  static const int8 kKodondo_Flame_X[4] = {8, -8, 0, 0};
  static const int8 kKodondo_Flame_Y[4] = {0, 0, 8, -8};
  static const int8 kKodondo_Flame_Xvel[4] = {24, -24, 0, 0};
  static const int8 kKodondo_Flame_Yvel[4] = {0, 0, 24, -24};
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamicallyEx(k, 0x87, &info, 13);
  if (j >= 0) {
    int i = sprite_D[k];
    Sprite_SetX(j, info.r0_x + kKodondo_Flame_X[i]);
    Sprite_SetY(j, info.r2_y + kKodondo_Flame_Y[i]);
    sprite_x_vel[j] = kKodondo_Flame_Xvel[i];
    sprite_y_vel[j] = kKodondo_Flame_Yvel[i];
    sprite_ignore_projectile[j] = 1;
  }
}

void Sprite_87_KodongoFire(int k) {  // 9ec274
  static const uint8 kFlame_OamFlags[4] = { 0, 0x40, 0xc0, 0x80 };
  static const int8 kFlame_Gfx[32] = {
    5, 4, 3, 1, 2, 0, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0,
    1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0,
  };

  if (!sprite_delay_main[k]) {
    SpriteDraw_SingleLarge(k);
    if (Sprite_ReturnIfInactive(k))
      return;
    sprite_oam_flags[k] = sprite_oam_flags[k] & 0x3f | kFlame_OamFlags[frame_counter >> 2 & 3];
    if (!Sprite_CheckDamageToLink(k)) {
      Sprite_MoveXY(k);
      if (!Sprite_CheckTileCollision(k))
        return;
    }
    sprite_delay_main[k] = 127;
    sprite_oam_flags[k] &= 63;
    SpriteSfx_QueueSfx2WithPan(k, 0x2a);
  } else {
    if (Sprite_CheckDamageFromLink(k) & kCheckDamageFromPlayer_Carry && !--sprite_delay_main[k] || sprite_delay_main[k] == 1)
      sprite_state[k] = 0;
    sprite_graphics[k] = kFlame_Gfx[sprite_delay_main[k] >> 3];
    Flame_Draw(k);
    Sprite_CheckDamageToLink(k);
  }
}

void Flame_Draw(int k) {  // 9ec35c
  static const DrawMultipleData kFlame_Dmd[12] = {
    {0,  0, 0x018e, 2},
    {0,  0, 0x018e, 2},
    {0,  0, 0x01a0, 2},
    {0,  0, 0x01a0, 2},
    {0,  0, 0x418e, 2},
    {0, 0, 0x418e, 2},
    { 0,  0, 0x41a0, 2 },
    { 0,  0, 0x41a0, 2 },
    { 0,  0, 0x01a2, 2 },
    { 0,  0, 0x01a2, 2 },
    { 0, -6, 0x01a4, 0 },
    { 8, -6, 0x01a5, 0 },
  };
  Sprite_DrawMultiple(k, &kFlame_Dmd[sprite_graphics[k] * 2], 2, NULL);
}

void Sprite_85_YellowStalfos(int k) {  // 9ec37f
  static const int8 kYellowStalfos_ObjPrio[6] = {0x30, 0, 0, 0, 0x30, 0};
  static const int8 kYellowStalfos_Gfx[32] = {
    8, 5, 1, 1, 8, 5, 1, 1, 8, 5, 1, 1, 7, 4, 2, 2,
    7, 4, 2, 2, 7, 4, 2, 2, 7, 4, 2, 2, 7, 4, 2, 2,
  };
  static const int8 kYellowStalfos_HeadX[32] = {
    -0x80, -0x80, -0x80, -0x80, -0x80, -0x80, -0x80, -0x80, -0x80, -0x80, -0x80, -0x80, 0, 0, 0, 0,
    0,     0,     0,     0,    -1,     0,     1,     0,    -1,     0,     1,     0, 0, 0, 0, 0,
  };
  static const uint8 kYellowStalfos_HeadY[32] = {
    13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
    13, 12, 11, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
  };
  static const int8 kYellowStalfos_NeutralizedGfx[16] = {1, 1, 1, 9, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 9};
  static const int8 kYellowStalfos_NeutralizedHeadY[16] = {10, 10, 10, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7};
  int j;

  if (!sprite_A[k]) {
    sprite_x_vel[k] = 1;
    sprite_y_vel[k] = 1;
    if (Sprite_CheckTileCollision(k)) {
      sprite_state[k] = 0;
      return;
    }
    sprite_A[k]++;
    sprite_C[k] = 10;
    sprite_flags3[k] |= 64;
    SpriteSfx_QueueSfx2WithPan(k, 0x20);
  }

  sprite_obj_prio[k] |= kYellowStalfos_ObjPrio[sprite_ai_state[k]];
  YellowStalfos_Draw(k);  
  if (Sprite_ReturnIfInactive(k))
    return;
  if (link_sword_type >= 3) {
    if (Sprite_ReturnIfRecoiling(k))
      return;
  } else if (sprite_ai_state[k] != 5 && sprite_hit_timer[k]) {
    sprite_hit_timer[k] = 0;
    sprite_ai_state[k] = 5;
    sprite_delay_main[k] = 255;
  }
  sprite_ignore_projectile[k] = 1;
  switch (sprite_ai_state[k]) {
  case 0: { // descend
    sprite_head_dir[k] = 2;
    uint8 bak0 = sprite_z[k];
    Sprite_MoveZ(k);
    if (!sign8(sprite_z_vel[k] - 192))
      sprite_z_vel[k] -= 3;
    if (!sign8(bak0) && sign8(sprite_z[k])) {
      sprite_ai_state[k]++;
      sprite_z[k] = 0;
      sprite_z_vel[k] = 0;
      sprite_delay_main[k] = 64;
      YellowStalfos_Animate(k);
    }
    break;
  }
  case 1:  // face player
    sprite_ignore_projectile[k] = 0;
    Sprite_CheckDamageToAndFromLink(k);
    sprite_head_dir[k] = sprite_D[k] = Sprite_DirectionToFaceLink(k, NULL);
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k]++;
      sprite_delay_main[k] = 127;
    }
    sprite_flags3[k] &= ~0x40;
    break;
  case 2:  // pause then detach head
    sprite_ignore_projectile[k] = 0;
    Sprite_CheckDamageToAndFromLink(k);
    j = sprite_delay_main[k];
    if (!j) {
      sprite_ai_state[k]++;
      sprite_delay_main[k] = 64;
      return;
    }
    if (j == 48)
      YellowStalfos_EmancipateHead(k);
    sprite_graphics[k] = kYellowStalfos_Gfx[j >> 2 & ~3 | sprite_D[k]];
    j = sprite_delay_main[k] >> 2;
    sprite_B[k] = kYellowStalfos_HeadX[j];
    sprite_C[k] = kYellowStalfos_HeadY[j];
    sprite_flags3[k] &= ~0x40;
    break;
  case 3:  // delay before ascending
    sprite_ignore_projectile[k] = 0;
    Sprite_CheckDamageToAndFromLink(k);
    if (!sprite_delay_main[k])
      sprite_ai_state[k]++;
    YellowStalfos_Animate(k);
    break;
  case 4:  // ascend
    sprite_graphics[k] = 0;
    sprite_head_dir[k] = 2;
    j = sprite_z[k];
    Sprite_MoveZ(k);
    if (sign8(sprite_z_vel[k] - 64))
      sprite_z_vel[k] += 2;
    if (sign8(j) && !sign8(sprite_z[k]))
      sprite_state[k] = 0;
    break;
  case 5:  // neutralized
    sprite_ignore_projectile[k] = 0;
    Sprite_CheckDamageFromLink(k);
    j = sprite_delay_main[k];
    if (!j)
      sprite_ai_state[k]--;
    sprite_graphics[k] = kYellowStalfos_NeutralizedGfx[j >> 4];
    sprite_C[k] = kYellowStalfos_NeutralizedHeadY[j >> 4];
    break;
  }
}

void YellowStalfos_Animate(int k) {  // 9ec509
  static const uint8 kYellowStalfos_Gfx2[4] = {6, 3, 1, 1};
  sprite_graphics[k] = kYellowStalfos_Gfx2[sprite_D[k]];
  sprite_flags3[k] &= ~0x40;
}

void YellowStalfos_EmancipateHead(int k) {  // 9ec580
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 2, &info);
  if (j >= 0) {
    Sprite_SetSpawnedCoordinates(j, &info);
    sprite_z[j] = 13;
    Sprite_ApplySpeedTowardsLink(j, 16);
    sprite_delay_main[j] = 255;
    sprite_delay_aux1[j] = 32;
  }
}

void YellowStalfos_Draw(int k) {  // 9ec655
  static const DrawMultipleData kYellowStalfos_Dmd[22] = {
    {0, 0, 0x000a, 2},
    {0, 0, 0x000a, 2},
    {0, 0, 0x000c, 2},
    {0, 0, 0x000c, 2},
    {0, 0, 0x002c, 2},
    {0, 0, 0x002c, 2},
    {5, 5, 0x002e, 0},
    {0, 0, 0x0024, 2},
    {4, 1, 0x003e, 0},
    {0, 0, 0x0024, 2},
    {0, 0, 0x000e, 2},
    {0, 0, 0x000e, 2},
    {3, 5, 0x402e, 0},
    {0, 0, 0x4024, 2},
    {4, 1, 0x403e, 0},
    {0, 0, 0x4024, 2},
    {0, 0, 0x400e, 2},
    {0, 0, 0x400e, 2},
    {0, 0, 0x002a, 2},
    {0, 0, 0x002a, 2},
    {0, 0, 0x002a, 2},
    {0, 0, 0x002a, 2},
  };
  oam_cur_ptr += 4, oam_ext_cur_ptr++;
  PrepOamCoordsRet info;
  Sprite_DrawMultiple(k, &kYellowStalfos_Dmd[sprite_graphics[k] * 2], 2, &info);
  oam_cur_ptr -= 4, oam_ext_cur_ptr--;
  if (!sprite_pause[k]) {
    YellowStalfos_DrawHead(k, &info);
    SpriteDraw_Shadow(k, &info);
  }
}

void YellowStalfos_DrawHead(int k, PrepOamCoordsRet *info) {  // 9ec69a
  static const uint8 kYellowStalfos_Head_Char[4] = {2, 2, 0, 4};
  static const uint8 kYellowStalfos_Head_Flags[4] = {0x40, 0, 0, 0};
  OamEnt *oam = GetOamCurPtr();
  if (sprite_graphics[k] == 10 || sprite_B[k] == 0x80)
    return;
  int j = sprite_head_dir[k];
  SetOamHelper0(oam, info->x + (int8)sprite_B[k], info->y - sprite_C[k],
                kYellowStalfos_Head_Char[j],
                kYellowStalfos_Head_Flags[j] | info->flags, 2);
}

void SpritePrep_Eyegore(int k) {  // 9ec700
  uint8 t = BYTE(dungeon_room_index2);
  if (t == 12 || t == 27 || t == 75 || t == 107) {
    sprite_B[k]++;
    if (sprite_type[k] == 0x83)
      sprite_defl_bits[k] = 0;
  }
}

void Sprite_83_GreenEyegore(int k) {  // 9ec79b
  static const int8 kGoriya_Xvel[32] = {
    0,  16, -16, 0, 0,  13, -13, 0, 0,  13, -13, 0, 0, 0, 0, 0,
    0, -24,  24, 0, 0, -16,  16, 0, 0, -16,  16, 0, 0, 0, 0, 0,
  };
  static const int8 kGoriya_Yvel[32] = {
    0, 0, 0, 0, -16,  -5,  -5, 0, 16, 13, 13, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -24, -16, -16, 0, 24, 16, 16, 0, 0, 0, 0, 0,
  };
  static const uint8 kGoriya_Dir[32] = {
    0, 0, 1, 0, 3, 3, 3, 0, 2, 2, 2, 0, 0, 0, 0, 0,
    0, 1, 0, 0, 3, 3, 3, 0, 2, 2, 2, 0, 0, 0, 0, 0,
  };
  static const uint8 kGoriya_Gfx[16] = {8, 6, 0, 3, 9, 7, 1, 4, 8, 6, 0, 3, 9, 7, 2, 5};

  if (!sprite_B[k]) {
    Eyegore_Main(k);
    return;
  }
  Goriya_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (Sprite_ReturnIfRecoiling(k))
    return;
  if (sprite_delay_aux1[k] == 8)
    Sprite_SpawnFirePhlegm(k);
  if (bitmask_of_dragstate != 0 || !(joypad1H_last & 0xf)) {
    sprite_A[k] = 0;
    Sprite_CheckDamageToAndFromLink(k);
    Sprite_CheckTileCollision(k);
    return;
  }

  int j = (joypad1H_last & 0xf) | (sprite_type[k] == 0x84) * 16;
  sprite_D[k] = kGoriya_Dir[j];
  sprite_x_vel[k] = kGoriya_Xvel[j];
  sprite_y_vel[k] = kGoriya_Yvel[j];
  if (!sprite_wallcoll[k])
    Sprite_MoveXY(k);
  Sprite_CheckDamageToAndFromLink(k);
  Sprite_CheckTileCollision(k);
  sprite_graphics[k] = kGoriya_Gfx[++sprite_subtype2[k] & 12 | sprite_D[k]];
  if (sprite_type[k] == 0x84) {
    PointU8 pt;
    uint8 dir = Sprite_DirectionToFaceLink(k, &pt);
    if (((uint8)(pt.x + 8) < 16 || (uint8)(pt.y + 8) < 16) && sprite_D[k] == dir) {
      if (!(sprite_A[k] & 0x1f))
        sprite_delay_aux1[k] = 16;
      sprite_A[k]++;
      return;
    }
  }
  sprite_A[k] = 0;
}

void Eyegore_Main(int k) {  // 9ec839
  static const uint8 kEyeGore_Closing_Gfx[8] = {0, 0, 1, 1, 2, 2, 2, 2};
  static const uint8 kEyeGore_Opening_Gfx[8] = {2, 2, 2, 2, 1, 1, 0, 0};
  static const uint8 kEyeGore_Chasing_Gfx[16] = {7, 5, 2, 9, 8, 6, 3, 10, 7, 5, 2, 9, 8, 6, 4, 11};
  static const uint8 kEyeGore_Opening_Delay[4] = {0x60, 0x80, 0xa0, 0x80};
  Eyegore_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (Sprite_ReturnIfRecoiling(k))
    return;
  Sprite_CheckDamageToAndFromLink(k);
  sprite_flags3[k] |= 64;
  sprite_defl_bits[k] |= 4;
  switch (sprite_ai_state[k]) {
  case 0:  // wait until player
    if (!sprite_delay_main[k]) {
      PointU8 pt;
      Sprite_DirectionToFaceLink(k, &pt);
      if ((uint8)(pt.x + 48) < 96 &&
          (uint8)(pt.y + 48) < 96) {
        sprite_ai_state[k]++;
        sprite_delay_main[k] = 63;
      }
    }
    break;
  case 1:  // openig eye
    if (!sprite_delay_main[k]) {
      sprite_D[k] = Sprite_DirectionToFaceLink(k, NULL);
      sprite_ai_state[k] = 2;
      sprite_delay_main[k] = kEyeGore_Opening_Delay[GetRandomNumber() & 3];
    } else {
      sprite_graphics[k] = kEyeGore_Opening_Gfx[sprite_delay_main[k] >> 3];
    }
    break;
  case 2:  // chase player
    sprite_flags3[k] &= ~0x40;
    if (sprite_type[k] != 0x84)
      sprite_defl_bits[k] &= ~4;
    if (!sprite_delay_main[k]) {
      sprite_delay_main[k] = 63;
      sprite_ai_state[k]++;
      sprite_graphics[k] = 0;
    } else {
      if (!((k ^ frame_counter) & 31))
        sprite_D[k] = Sprite_DirectionToFaceLink(k, NULL);
      int j = sprite_D[k];
      sprite_x_vel[k] = kFluteBoyAnimal_Xvel[j];
      sprite_y_vel[k] = kZazak_Yvel[j];
      if (!sprite_wallcoll[k])
        Sprite_MoveXY(k);
      Sprite_CheckTileCollision(k);
      sprite_graphics[k] = kEyeGore_Chasing_Gfx[++sprite_subtype2[k] & 12 | sprite_D[k]];
    }
    break;
  case 3:  // closing eye
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k] = 0;
      sprite_delay_main[k] = 96;
    } else {
      sprite_graphics[k] = kEyeGore_Closing_Gfx[sprite_delay_main[k] >> 3];
    }
    break;
  }
}

void Eyegore_Draw(int k) {  // 9ecacf
  static const DrawMultipleData kEyeGore_Dmd[48] = {
    {-4, -4, 0x00a2, 2},
    { 4, -4, 0x40a2, 2},
    {-4,  4, 0x009c, 2},
    { 4,  4, 0x409c, 2},
    {-4, -4, 0x00a4, 2},
    { 4, -4, 0x40a4, 2},
    {-4,  4, 0x009c, 2},
    { 4,  4, 0x409c, 2},
    {-4, -4, 0x008c, 2},
    { 4, -4, 0x408c, 2},
    {-4,  4, 0x009c, 2},
    { 4,  4, 0x409c, 2},
    {-4, -3, 0x008c, 2},
    {12, -3, 0x408c, 0},
    {-4, 13, 0x00bc, 0},
    { 4,  5, 0x408a, 2},
    {-4, -3, 0x008c, 0},
    { 4, -3, 0x408c, 2},
    {-4,  5, 0x008a, 2},
    {12, 13, 0x40bc, 0},
    { 0, -4, 0x00aa, 2},
    { 0,  4, 0x00a6, 2},
    { 0, -4, 0x00aa, 2},
    { 0,  4, 0x00a6, 2},
    { 0, -3, 0x00aa, 2},
    { 0,  4, 0x00a8, 2},
    { 0, -3, 0x00aa, 2},
    { 0,  4, 0x00a8, 2},
    { 0, -4, 0x40aa, 2},
    { 0,  4, 0x40a6, 2},
    { 0, -4, 0x40aa, 2},
    { 0,  4, 0x40a6, 2},
    { 0, -3, 0x40aa, 2},
    { 0,  4, 0x40a8, 2},
    { 0, -3, 0x40aa, 2},
    { 0,  4, 0x40a8, 2},
    {-4, -4, 0x008e, 2},
    { 4, -4, 0x408e, 2},
    {-4,  4, 0x009e, 2},
    { 4,  4, 0x409e, 2},
    {-4, -3, 0x008e, 2},
    {12, -3, 0x408e, 0},
    {-4, 13, 0x00bd, 0},
    { 4,  5, 0x40a0, 2},
    {-4, -3, 0x008e, 0},
    { 4, -3, 0x408e, 2},
    {-4,  5, 0x00a0, 2},
    {12, 13, 0x40bd, 0},
  };
  PrepOamCoordsRet info;
  Sprite_DrawMultiple(k, &kEyeGore_Dmd[sprite_graphics[k] * 4], 4, &info);
  if (!sprite_pause[k])
    SpriteDraw_Shadow_custom(k, &info, 14);
}

void SpritePrep_AntifairyCircle(int k) {  // 9ecb0c
  static const int8 kBubbleGroup_X[3] = {10, 20, 10};
  static const int8 kBubbleGroup_Y[3] = {-10, 0, 10};
  static const int8 kBubbleGroup_Xvel[3] = {18, 0, -18};
  static const int8 kBubbleGroup_Yvel[3] = {0, 18, 0};
  static const int8 kBubbleGroup_A[3] = {1, 1, 0};
  static const int8 kBubbleGroup_B[3] = { 0, 1, 1 };
  Sprite_SetX(k, Sprite_GetX(k) - 10);
  sprite_y_vel[k] = -18;
  sprite_x_vel[k] = 0;
  sprite_A[k] = 0;
  sprite_B[k] = 0;
  tmp_counter = 2;
  do {
    SpriteSpawnInfo info;
    int j = Sprite_SpawnDynamically(k, 0x82, &info);
    if (j >= 0) {
      int i = tmp_counter;
      Sprite_SetX(j, info.r0_x + kBubbleGroup_X[i]);
      Sprite_SetY(j, info.r2_y + kBubbleGroup_Y[i]);
      sprite_x_vel[j] = kBubbleGroup_Xvel[i];
      sprite_y_vel[j] = kBubbleGroup_Yvel[i];
      sprite_A[j] = kBubbleGroup_A[i];
      sprite_B[j] = kBubbleGroup_B[i];
    }
  } while (!sign8(--tmp_counter));
}

void Sprite_82_AntifairyCircle(int k) {  // 9ecb97
  static const int8 kBubbleGroup_Vel[2] = {1, -1};
  static const uint8 kBubbleGroup_VelTarget[2] = {18, (uint8)-18};

  SpriteDraw_Antfairy(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  int j = sprite_A[k] & 1;
  if ((sprite_x_vel[k] += kBubbleGroup_Vel[j]) == kBubbleGroup_VelTarget[j])
    sprite_A[k]++;

  j = sprite_B[k] & 1;
  if ((sprite_y_vel[k] += kBubbleGroup_Vel[j]) == kBubbleGroup_VelTarget[j])
    sprite_B[k]++;

  Sprite_MoveXY(k);
  if (sprite_x_vel[k] && sprite_y_vel[k] && Sprite_CheckIfRoomIsClear()) {
    sprite_type[k] = 0x15;
    sprite_x_vel[k] = sign8(sprite_x_vel[k]) ? -16 : 16;
    sprite_y_vel[k] = sign8(sprite_y_vel[k]) ? -16 : 16;
  }
  Sprite_CheckDamageToLink(k);
}

void Sprite_81_Hover(int k) {  // 9ecc02
  static const int8 kHover_OamFlags[4] = {0x40, 0, 0x40, 0};
  sprite_obj_prio[k] |= 48;
  SpriteDraw_SingleLarge(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (sprite_F[k])
    sprite_ai_state[k] = 0;
  if (Sprite_ReturnIfRecoiling(k))
    return;
  Sprite_CheckDamageToAndFromLink(k);
  if (!sprite_wallcoll[k])
    Sprite_MoveXY(k);
  Sprite_CheckTileCollision(k);
  sprite_graphics[k] = ++sprite_subtype2[k] >> 3 & 2;
  switch(sprite_ai_state[k]) {
  case 0:  // stopped
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k] = 1;
      int j = Sprite_IsRightOfLink(k).a + Sprite_IsBelowLink(k).a * 2;
      sprite_D[k] = j;
      sprite_oam_flags[k] = sprite_oam_flags[k] & ~0x40 | kHover_OamFlags[j];
      sprite_delay_main[k] = (GetRandomNumber() & 15) + 12;
      Sprite_ZeroVelocity_XY(k);
    }
    break;
  case 1: {  // moving
    static const int8 kHover_AccelX0[4] = {1, -1, 1, -1};
    static const int8 kHover_AccelY0[4] = {1, 1, -1, -1};
    static const int8 kHover_AccelX1[4] = {-1, 1, -1, 1};
    static const int8 kHover_AccelY1[4] = {-1, -1, 1, 1};
    int j = sprite_D[k];
    if (sprite_delay_main[k]) {
      sprite_x_vel[k] += kHover_AccelX0[j];
      sprite_y_vel[k] += kHover_AccelY0[j];
      sprite_graphics[k] = sprite_subtype2[k] >> 3 & 1;
    } else {
      sprite_x_vel[k] += kHover_AccelX1[j];
      sprite_y_vel[k] += kHover_AccelY1[j];
      if (!sprite_y_vel[k]) {
        sprite_ai_state[k] = 0;
        sprite_delay_main[k] = 64;
      }
    }
    break;
  }
  }
}

void Sprite_AB_CrystalMaiden(int k) {  // 9ece03
  cur_sprite_x -= dung_floor_x_offs;
  cur_sprite_y -= dung_floor_y_offs;

  if (sprite_ai_state[k] >= 3)
    CrystalMaiden_Draw(k);
  is_nmi_thread_active = 1;
  if (!BYTE(intro_did_run_step)) {
    CrystalMaiden_RunCutscene(k);
    BYTE(intro_did_run_step) = 1;
  }
}

void CrystalMaiden_RunCutscene(int k) {  // 9ece39
  sprite_E[k]++;
  poly_b += 6;
  if (submodule_index)
    return;
  switch (sprite_ai_state[k]) {
  case 0:  // disable subscreen
    TS_copy = 0;
    sprite_ai_state[k]++;
    break;
  case 1:  // enable subscreen
    TS_copy = 1;
    sprite_ai_state[k]++;
    break;
  case 2:  // generate sparkles
    if (poly_config1 < 6) {
      poly_config1 = 0;
      sprite_ai_state[k]++;
    } else {
      poly_config1 -= 3;
      if (poly_config1 >= 64)
        AncillaAdd_SwordChargeSparkle(sprite_A[k]);
    }
    break;
  case 3:  // filter palette
    sprite_ai_state[k]++;
  case 4:
    if (!(sprite_E[k] & 1)) {
      PaletteFilter_SP5F();
      if (!BYTE(palette_filter_countdown)) {
        sprite_ai_state[k]++;
        flag_is_link_immobilized = 1;
        link_receiveitem_index = 0;
        link_pose_for_item = 0;
        link_animation_steps = 0;
        link_direction_facing = 0;
      }
    }
    break;
  case 5: { // show message
    static const uint16 kCrystalMaiden_Msgs[9] = {0x133, 0x132, 0x137, 0x134, 0x136, 0x132, 0x135, 0x138, 0x13c};
    int j = BYTE(cur_palace_index_x2) - 10;
    if (j == 2 && savegame_map_icons_indicator < 7)
      savegame_map_icons_indicator = 7;
    if (j == 14 && (link_has_crystals & 0x7f) != 0x7f)
      j = 16;
    Sprite_ShowMessageUnconditional(kCrystalMaiden_Msgs[j >> 1]);
    sprite_ai_state[k]++;
    if ((link_has_crystals & 0x7f) == 0x7f)
      savegame_map_icons_indicator = 8;
    break;
  }
  case 6:  // reading comprehension exam
    Sprite_ShowMessageUnconditional(0x13a);
    sprite_ai_state[k]++;
    break;
  case 7:  // may the way of the hero
    if (choice_in_multiselect_box) {
      sprite_ai_state[k] = 5;
    } else {
      Sprite_ShowMessageUnconditional(0x139);
      sprite_ai_state[k]++;
    }
    break;
  case 8:  // initiate dungeon exit
    TS_copy = 0;
    PrepareDungeonExitFromBossFight();
    sprite_state[k] = 0;
    break;
  }
}

void Sprite_7D_BigSpike(int k) {  // 9ecf47
  static const int8 kSpikeTrap_Xvel[4] = {32, -32, 0, 0};
  static const int8 kSpikeTrap_Xvel2[4] = {-16, 16, 0, 0};
  static const int8 kSpikeTrap_Yvel[4] = {0, 0, 32, -32};
  static const int8 kSpikeTrap_Yvel2[4] = {0, 0, -16, 16};
  static const uint8 kSpikeTrap_Delay[4] = {0x40, 0x40, 0x38, 0x38};
  int j;

  SpikeTrap_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_CheckDamageToAndFromLink(k);
  if (sprite_ai_state[k] == 0) {
    PointU8 pt;
    sprite_D[k] = j = Sprite_DirectionToFaceLink(k, &pt);
    if ((uint8)(pt.x + 16) < 32 || (uint8)(pt.y + 16) < 32) {
      sprite_delay_main[k] = kSpikeTrap_Delay[j];
      sprite_ai_state[k] = 1;
      sprite_x_vel[k] = kSpikeTrap_Xvel[j];
      sprite_y_vel[k] = kSpikeTrap_Yvel[j];
    }
  } else if (sprite_ai_state[k] == 1) {
    if (Sprite_CheckTileCollision(k) || !sprite_delay_main[k]) {
      sprite_ai_state[k] = 2;
      sprite_delay_main[k] = 96;
    }
    Sprite_MoveXY(k);
  } else {
    if (!sprite_delay_main[k]) {
      j = sprite_D[k];
      sprite_x_vel[k] = kSpikeTrap_Xvel2[j];
      sprite_y_vel[k] = kSpikeTrap_Yvel2[j];
      Sprite_MoveXY(k);
      if (sprite_x_lo[k] == sprite_A[k] && sprite_y_lo[k] == sprite_C[k])
        sprite_ai_state[k] = 0;
    }
  }
}

void SpikeTrap_Draw(int k) {  // 9ecfff
  static const DrawMultipleData kSpikeTrap_Dmd[4] = {
    {-8, -8, 0x00c4, 2},
    { 8, -8, 0x40c4, 2},
    {-8,  8, 0x80c4, 2},
    { 8,  8, 0xc0c4, 2},
  };
  Sprite_DrawMultiple(k, kSpikeTrap_Dmd, 4, NULL);
}

void Sprite_7E_Firebar_Clockwise(int k) {  // 9ed01a
  static const int8 kGuruguruBar_incr[4] = {-2, 2, -1, 1};
  Firebar_Main(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  sprite_subtype2[k]++;
  int j = (sprite_type[k] - 0x7e) + (BYTE(cur_palace_index_x2) == 18) * 2;

  int t = sprite_A[k] | sprite_B[k] << 8;
  t = (t + kGuruguruBar_incr[j]) & 0x1ff;
  sprite_A[k] = t;
  sprite_B[k] = t >> 8;
}

void Firebar_Main(int k) {  // 9ed049
  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  OamEnt *oam = GetOamCurPtr();
  byte_7E0FB6 = info.flags;
  BYTE(dungmap_var7) = info.x;
  HIBYTE(dungmap_var7) = info.y;
  int angle = sprite_A[k] | sprite_B[k] << 8;
  int8 sinval = GuruguruBarSin(angle, 0x40);
  int8 cosval = GuruguruBarSin((angle + 0x80) & 0x1ff, 0x40);
  uint8 flags = (sprite_subtype2[k] << 4 & 0xc0) | info.flags;
  for (int i = 3; i >= 0; i--, oam++) {
    SetOamPlain(oam, info.x + sinval * (i + 1) / 4, info.y + cosval * (i + 1) / 4, 0x28, flags, 2);
  }
  Sprite_CorrectOamEntries(k, 3, 0xff);
  if (!((k ^ frame_counter) & 3 | submodule_index | flag_unk1)) {
    OamEnt *oam = GetOamCurPtr();
    for (int i = 0; i < 4; i++, oam++) {
      if (bytewise_extended_oam[oam - oam_buf] & 1)
        continue;
      if ((uint8)(oam->x + BG2HOFS_copy2 - link_x_coord + 12) < 24 &&
          oam->y < 0xf0 &&
          (uint8)(oam->y + BG2VOFS_copy2 - link_y_coord + 4) < 16)
        Sprite_AttemptDamageToLinkPlusRecoil(k);
    }
  }
}

void Sprite_80_Firesnake(int k) {  // 9ed1d1
  static const uint8 kWinder_OamFlags[4] = {0, 0x40, 0x80, 0xc0};
  static const int8 kWinder_Xvel[4] = {24, -24, 0, 0};
  static const int8 kWinder_Yvel[4] = {0, 0, 24, -24};
  SpriteDraw_SingleLarge(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (Sprite_ReturnIfRecoiling(k))
    return;
  sprite_oam_flags[k] = sprite_oam_flags[k] & 0x3f | kWinder_OamFlags[frame_counter >> 2 & 3];
  if (sprite_A[k]) {
    sprite_ignore_projectile[k] = sprite_delay_main[k];
    if (sprite_delay_main[k] == 0)
      sprite_state[k] = 0;
    return;
  }
  Sprite_CheckDamageToAndFromLink(k);
  Firesnake_SpawnFireball(k);
  if (!sprite_wallcoll[k])
    Sprite_MoveXY(k);
  if (Sprite_CheckTileCollision(k))
    sprite_D[k] = kZazak_Dir2[sprite_D[k] * 2 + (GetRandomNumber() & 1)];
  int j = sprite_D[k];
  sprite_x_vel[k] = kWinder_Xvel[j];
  sprite_y_vel[k] = kWinder_Yvel[j];
}

void Firesnake_SpawnFireball(int j) {  // 9ed239
  if ((j ^ frame_counter) & 7)
    return;
  int k = GarnishAlloc();
  garnish_type[k] = 1;
  garnish_active = 1;
  garnish_x_lo[k] = sprite_x_lo[j];
  garnish_x_hi[k] = sprite_x_hi[j];
  int y = Sprite_GetY(j) + 16;
  garnish_y_lo[k] = y;
  garnish_y_hi[k] = y >> 8;
  garnish_countdown[k] = 32;
  garnish_sprite[k] = j;
  garnish_floor[k] = sprite_floor[j];
}

void Sprite_7C_GreenStalfos(int k) {  // 9ed299
  static const uint8 kGreenStalfos_Dir[4] = {4, 6, 0, 2};
  static const uint8 kGreenStalfos_OamFlags[4] = {0x40, 0, 0, 0};
  static const uint8 kGreenStalfos_Gfx[4] = {0, 0, 1, 2};
  int j = sprite_D[k];
  sprite_oam_flags[k] = sprite_oam_flags[k] & ~0x40 | kGreenStalfos_OamFlags[j];
  sprite_graphics[k] = kGreenStalfos_Gfx[j];
  SpriteDraw_SingleLarge(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (Sprite_ReturnIfRecoiling(k))
    return;
  Sprite_CheckDamageToAndFromLink(k);
  j = Sprite_DirectionToFaceLink(k, NULL);
  if (kGreenStalfos_Dir[j] != link_direction_facing) {
    sprite_A[k] = 0;
    if (!((k ^ frame_counter) & 7)) {
      uint8 vel = sprite_B[k];
      if (vel != 4)
        sprite_B[k]++;
      Sprite_ApplySpeedTowardsLink(k, vel);
      sprite_D[k] = Sprite_IsRightOfLink(k).a;
    }
  } else {
    sprite_A[k] = 1;
    if (!((k ^ frame_counter) & 15)) {
      uint8 vel = sprite_B[k];
      if (vel)
        sprite_B[k]--;
      Sprite_ApplySpeedTowardsLink(k, vel);
      sprite_D[k] = Sprite_IsRightOfLink(k).a;
    }
  }
  Sprite_MoveXY(k);
}

void Sprite_7A_Agahnim(int k) {  // 9ed330
  int j;
  uint8 t;
  static const uint8 kAgahnim_StartState[2] = {1, 6};
  static const uint8 kAgahnim_Gfx0[5] = {12, 13, 14, 15, 16};
  static const int8 kAgahnim_Tab5[2] = {32, -32};
  static const uint8 kAgahnim_Tab6[2] = {9, 11};
  static const uint8 kAgahnim_Dir[25] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 5, 0, 1, 1, 4,
    4, 0, 2, 2, 4, 4, 3, 2, 2,
  };
  static const uint8 kAgahnim_Gfx1[6] = {2, 10, 8, 0, 4, 6};
  static const uint8 kAgahnim_Tab0[16] = {0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0};
  static const uint8 kAgahnim_Tab1[16] = {0, 0, 0, 0, 0, 0, 0, 6, 5, 4, 3, 2, 1, 0, 0, 0};
  static const uint8 kAgahnim_Tab2[6] = {30, 24, 12, 0, 6, 18};
  static const uint8 kAgahnim_Gfx2[5] = {16, 15, 14, 13, 12};
  static const uint8 kAgahnim_Tab3[16] = {0x38, 0x38, 0x38, 0x58, 0x78, 0x98, 0xb8, 0xb8, 0xb8, 0x98, 0x58, 0x58, 0x60, 0x90, 0x98, 0x78};
  static const uint8 kAgahnim_Tab4[16] = {0xb8, 0x78, 0x58, 0x48, 0x48, 0x48, 0x58, 0x78, 0xb8, 0xb8, 0xb8, 0x90, 0x70, 0x70, 0x90, 0xa0};
  static const uint8 kAgahnim_Gfx3[7] = {0, 8, 10, 2, 2, 6, 4};
  Agahnim_Draw(k);
  if (sprite_pause[k]) {
    sprite_delay_main[k] = 32;
    sprite_graphics[k] = 0;
    sprite_D[k] = 3;
  }
  if (Sprite_ReturnIfInactive(k))
    return;
  if (Sprite_ReturnIfRecoiling(k))
    return;
  sprite_ignore_projectile[k] = 1;
  switch (sprite_ai_state[k]) {
  case 0:
    sprite_ai_state[k] = kAgahnim_StartState[is_in_dark_world];
    break;
  case 1:
    if (!sprite_delay_main[k]) {
      dialogue_message_index = 0x13f;
      Sprite_ShowMessageMinimal();
      sprite_ai_state[k] = 3;
      sprite_delay_main[k] = 32;
    }
    break;
  case 2:
    byte_7E0FF8 = 0;
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k]++;
      sprite_delay_main[k] = 255;
    } else {
      sprite_graphics[k] = kAgahnim_Gfx0[sprite_delay_main[k] >> 3];
    }
    break;
  case 3:
    j = sprite_delay_main[k];
    if (j == 192)
      SpriteSfx_QueueSfx3WithPan(k, 0x27);
    if (j >= 239 || j < 16) {
      AgahnimWarpShadowFilter(is_in_dark_world ? k : 2);
    } else {
      if (!k) {
        Sprite_CheckDamageToAndFromLink(k);
        sprite_ignore_projectile[k] = 0;
      }
    }
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k]++;
      sprite_delay_main[k] = 39;
      return;
    }
    if (sprite_delay_main[k] >= 128) {
      Sprite_ApplySpeedTowardsLink(k, 2);
      sprite_D[k] = kAgahnim_Dir[((int8)sprite_y_vel[k] + 2) * 5 + 2 + (int8)sprite_x_vel[k]];
      Sprite_ApplySpeedTowardsLink(k, 32);
      if (sprite_subtype[k] == 4)
        sprite_D[k] = 3;
    } else if (sprite_delay_main[k] == 112) {
      Agahnim_PerformAttack(k);
    }
    j = sprite_delay_main[k] >> 4;
    sprite_A[k] = kAgahnim_Tab0[j];
    t = kAgahnim_Tab1[j];
    sprite_head_dir[k] = t ? t + kAgahnim_Tab2[sprite_D[k]] : t ;
    sprite_graphics[k] = kAgahnim_Gfx1[sprite_D[k]] + sprite_A[k];
    break;
  case 4:
    sprite_ignore_projectile[k] = sprite_delay_main[k];
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k]++;
      j = sprite_subtype[k] == 4 ? 4 : GetRandomNumber() & 0xf;
      sprite_C[k] = kAgahnim_Tab3[j];
      sprite_E[k] = kAgahnim_Tab4[j];
      sprite_G[k] = 8;
    } else {
      sprite_graphics[k] = kAgahnim_Gfx2[sprite_delay_main[k] >> 3];
    }
    break;
  case 5: {
    if ((uint16)(sprite_x_lo[k] - sprite_C[k] + 7) < 14 &&
        (uint16)(sprite_y_lo[k] - sprite_E[k] + 7) < 14) {
      sprite_x_lo[k] = sprite_C[k];
      sprite_y_lo[k] = sprite_E[k];
      sprite_ai_state[k] = 2;
      sprite_delay_main[k] = 39;
      return;
    }
    int x = sprite_x_hi[k] << 8 | sprite_C[k];
    int y = sprite_y_hi[k] << 8 | sprite_E[k];
    ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLocation(k, x, y, sprite_G[k]);
    sprite_y_vel[k] = pt.y;
    sprite_x_vel[k] = pt.x;
    if (sprite_G[k] < 64)
      sprite_G[k]++;
    Sprite_MoveXY(k);
    break;
  }
  case 6:
    if (!sprite_delay_main[k]) {
      dialogue_message_index = 0x141;
      Sprite_ShowMessageMinimal();
      sprite_ai_state[k]++;
      sprite_delay_main[k] = 80;
    }
    break;
  case 7:
    if (sprite_anim_clock[k]) {
      if (!sprite_delay_main[k]) {
        sprite_ai_state[k] = 3;
        sprite_delay_main[k] = 32;
      } else {
        sprite_x_vel[k] = kAgahnim_Tab5[k - 1];
        sprite_y_vel[k] += 2;
        Sprite_MoveXY(k);
        j = Sprite_Agahnim_ApplyMotionBlur(k);
        if (j >= 0)
          sprite_oam_flags[j] = 4;
      }
    } else {
      if (!sprite_delay_main[k]) {
        sprite_ai_state[k] = 3;
        sprite_delay_main[k] = 32;
      } else if (sprite_delay_main[k] == 64) {
        sound_effect_2 = 0x28;
        tmp_counter = 1;
        do {
          SpriteSpawnInfo info;
          j = Sprite_SpawnDynamicallyEx(k, 0x7A, &info, 2);
          Sprite_SetSpawnedCoordinates(j, &info);
          sprite_flags3[j] = kAgahnim_Tab6[j - 1];
          sprite_anim_clock[j] = sprite_oam_flags[j] = sprite_flags3[j] & 15;
          sprite_ai_state[j] = sprite_ai_state[k];
          sprite_delay_main[j] = 32;
        } while (!sign8(--tmp_counter));
      }
    }
    break;
  case 8:
    flag_block_link_menu = 2;
    sprite_head_dir[k] = 0;
    if (sprite_delay_main[k] >= 64) {
      sprite_hit_timer[k] |= 0xe0;
    } else {
      if (sprite_delay_main[k] == 1) {
        Sprite_SpawnPhantomGanon(k);
        music_control = 0x1d;
      }
      sprite_hit_timer[k] = 0;
      sprite_graphics[k] = 17;
    }
    break;
  case 9: {
    sprite_head_dir[k] = 0;
    int x = Sprite_GetX(0), y = Sprite_GetY(0);
    if ((uint16)(cur_sprite_x - x + 4) < 8 &&
        (uint16)(cur_sprite_y - y + 4) < 8)
      sprite_state[k] = 0;
    ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLocation(k, x, y, 0x20);
    sprite_y_vel[k] = pt.y;
    sprite_x_vel[k] = pt.x;
    Sprite_MoveXY(k);
    Sprite_Agahnim_ApplyMotionBlur(k);
    break;
  }
  case 10:
    if (!sprite_delay_main[k]) {
      sprite_state[k] = 0;
      PrepareDungeonExitFromBossFight();
    }
    if (sprite_delay_main[k] < 16) {
      CGADSUB_copy = 0x7f;
      TM_copy = 6;
      TS_copy = 0x10;
      PaletteFilter_SP5F();
    }
    if (sprite_z_vel[k] != 0xff)
      sprite_z_vel[k]++;
    j = sprite_z_subpos[k] + sprite_z_vel[k];
    sprite_z_subpos[k] = j;
    if (j & 0x100 && ++sprite_subtype2[k] == 7) {
      sprite_subtype2[k] = 0;
      SpriteSfx_QueueSfx2WithPan(k, 0x4);
    }
    sprite_graphics[k] = kAgahnim_Gfx3[sprite_subtype2[k]];
    break;
  }
}

void Agahnim_PerformAttack(int k) {  // 9ed67a
  static const int8 kAgahnim_X0[6] = {0, 10, 8, 0, -10, -10};
  static const int8 kAgahnim_Y0[6] = {-9, -2, -2, -9, -2, -2};

  if (!k) {
    sprite_subtype[k]++;
    if (is_in_dark_world)
      sprite_subtype[k] &= 3;
  }
  if (sprite_subtype[k] == 5) {
    sprite_subtype[k] = 0;
    SpriteSfx_QueueSfx3WithPan(k, 0x26);
    for (int i = 0; i < 4; i++)
      Sprite_SpawnLightning(k);
  } else {
    SpriteSpawnInfo info;
    int j = Sprite_SpawnDynamically(k, 0x7B, &info);
    if (j >= 0) {
      SpriteSfx_QueueSfx3WithPan(k, 0x29);
      int i = sprite_D[k];
      Sprite_SetX(j, info.r0_x + kAgahnim_X0[i]);
      Sprite_SetY(j, info.r2_y + kAgahnim_Y0[i]);
      sprite_ignore_projectile[j] = sprite_y_hi[j];
      sprite_x_vel[j] = sprite_x_vel[k];
      sprite_y_vel[j] = sprite_y_vel[k];
      if (sprite_subtype[k] >= 2 && !(GetRandomNumber() & 1)) {
        sprite_B[j] = 1;
        sprite_delay_main[j] = 32;
      }
    }
  }
}

void Agahnim_Draw(int k) {  // 9ed978
  static const int8 kAgahnim_Draw_X0[72] = {
    -8, 8, -8, 8, -8, 8, -8, 8, -8, 8, -8, 8, -8, 8, -8, 8,
    -8, 8, -8, 8, -8, 8, -8, 8, -8, 8, -8, 8, -8, 8, -8, 8,
    -8, 8, -8, 8, -8, 8, -8, 8, -8, 8, -8, 8, -8, 8, -8, 8,
    -8, 8, -8, 8, -6, 6, -6, 6, -8, 8, -8, 8, -6, 6, -6, 6,
    0, 8,  0, 8, -8, 8, -8, 8,
  };
  static const int8 kAgahnim_Draw_Y0[72] = {
    -8, -8, 8, 8, -8, -8, 8, 8, -8, -8, 8, 8, -8, -8, 8, 8,
    -8, -8, 8, 8, -8, -8, 8, 8, -8, -8, 8, 8, -8, -8, 8, 8,
    -8, -8, 8, 8, -8, -8, 8, 8, -8, -8, 8, 8, -8, -8, 8, 8,
    -8, -8, 8, 8, -6, -6, 6, 6, -8, -8, 8, 8, -6, -6, 6, 6,
    0,  0, 8, 8,  8,  8, 8, 8,
  };
  static const uint8 kAgahnim_Draw_Char0[72] = {
    0x82, 0x82, 0xa2, 0xa2, 0x80, 0x80, 0xa0, 0xa0, 0x84, 0x84, 0xa4, 0xa4, 0x86, 0x86, 0xa6, 0xa6,
    0x88, 0x8a, 0xa8, 0xaa, 0x8c, 0x8e, 0xac, 0xae, 0xc4, 0xc2, 0xe4, 0xe6, 0xc0, 0xc2, 0xe0, 0xe2,
    0x8a, 0x88, 0xaa, 0xa8, 0x8e, 0x8c, 0xae, 0xac, 0xc2, 0xc4, 0xe6, 0xe4, 0xc2, 0xc0, 0xe2, 0xe0,
    0xec, 0xec, 0xec, 0xec, 0xec, 0xec, 0xec, 0xec, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee,
    0xdf, 0xdf, 0xdf, 0xdf, 0x40, 0x42, 0x40, 0x42,
  };
  static const uint8 kAgahnim_Draw_Flags0[72] = {
    0, 0x40,    0, 0x40,    0, 0x40,    0, 0x40,    0, 0x40,    0, 0x40,    0, 0x40,    0, 0x40,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
    0, 0x40, 0x80, 0xc0,    0, 0x40, 0x80, 0xc0,    0, 0x40, 0x80, 0xc0,    0, 0x40, 0x80, 0xc0,
    0, 0x40, 0x80, 0xc0,    0,    0,    0,    0,
  };
  static const int8 kAgahnim_Draw_X1[72] = {
    -7, 15, -11, 11, -11, 11,  -8,   8,  -4,   4,   0,   0, -10,  -1, -14,  -5,
    -14, -5, -12, -7, -10, -7, -10, -10,  16,   8,  12,   4,  12,   4,  10,   6,
    9,  7,   8,  8,  -6, -6, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10,
    14, 14,  10, 10,  10, 10,  10,  10,  10,  10,  10,  10,  -7,  15, -11,  11,
    -11, 11,  -8,  8,  -4,  4,   0,   0,
  };
  static const int8 kAgahnim_Draw_Y1[72] = {
    -5, -5, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -3,  9, -7,  5,
    -7,  5, -5,  3, -3,  3, -2, -2, -3,  9, -7,  5, -7,  5, -5,  3,
    -3,  3, -2, -2, -3,  9, -7,  5, -7,  5, -5,  3, -3,  3, -2, -2,
    -3,  9, -7,  5, -7,  5, -5,  3, -3,  3, -2, -2, -5, -5, -9, -9,
    -9, -9, -9, -9, -9, -9, -9, -9,
  };
  static const uint8 kAgahnim_Draw_Char1[36] = {
    0xce, 0xcc, 0xc6, 0xc6, 0xc6, 0xc6, 0xce, 0xcc, 0xc6, 0xc6, 0xc6, 0xc6, 0xce, 0xcc, 0xc6, 0xc6,
    0xc6, 0xc6, 0xce, 0xcc, 0xc6, 0xc6, 0xc6, 0xc6, 0xce, 0xcc, 0xc6, 0xc6, 0xc6, 0xc6, 0xce, 0xcc,
    0xc6, 0xc6, 0xc6, 0xc6,
  };
  static const uint8 kAgahnim_Draw_Big1[36] = {
    0, 2, 2, 2, 2, 2, 0, 2, 2, 2, 2, 2, 0, 2, 2, 2,
    2, 2, 0, 2, 2, 2, 2, 2, 0, 2, 2, 2, 2, 2, 0, 2,
    2, 2, 2, 2,
  };
  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  OamEnt *oam = GetOamCurPtr();
  int g = sprite_graphics[k];
  for (int i = 3; i >= 0; i--, oam++) {
    int j = g * 4 + i;
    SetOamPlain(oam, info.x + kAgahnim_Draw_X0[j], info.y + kAgahnim_Draw_Y0[j],
                 kAgahnim_Draw_Char0[j], info.flags | kAgahnim_Draw_Flags0[j],
                 (j >= 0x40 && j < 0x44) ? 0 : 2);
  }
  if (g < 12)
    SpriteDraw_Shadow_custom(k, &info, 18);
  if (submodule_index)
    Sprite_CorrectOamEntries(k, 3, 0xff);

  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;

  if (sprite_D[k])
    Oam_AllocateFromRegionC(8);
  else
    Oam_AllocateFromRegionB(8);
  oam = GetOamCurPtr();

  if (!sprite_head_dir[k])
    return;
  g = sprite_head_dir[k] - 1;
  uint8 flags = (((frame_counter >> 1) & 2) + 2) + 0x31;
  for (int i = 1; i >= 0; i--, oam++) {
    SetOamPlain(oam, info.x + kAgahnim_Draw_X1[g * 2 + i], info.y + kAgahnim_Draw_Y1[g * 2 + i],
                 kAgahnim_Draw_Char1[g], flags, kAgahnim_Draw_Big1[g]);
  }
}

void Sprite_7B_AgahnimBalls(int k) {  // 9eda42
  if (sprite_B[k]) {
    if (sprite_delay_main[k])
      Sprite_ApplySpeedTowardsLink(k, 32);
    sprite_oam_flags[k] = 5;
  } else {
    sprite_oam_flags[k] = (frame_counter >> 1 & 2) + 3;
  }

  if (sprite_ai_state[k]) {
    static const uint8 kEnergyBall_Gfx[16] = {2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 0, 0, 0, 0};
    if (sprite_graphics[k] != 2)
      SpriteDraw_SingleLarge(k);
    else
      SpriteDraw_SingleSmall(k);
    if (Sprite_ReturnIfInactive(k))
      return;
    sprite_ignore_projectile[k] = sprite_delay_main[k];
    if (!sprite_delay_main[k]) {
      sprite_state[k] = 0;
    } else if (sprite_delay_main[k] == 6) {
      sprite_x_vel[k] = sprite_y_vel[k] = 64;
      Sprite_MoveXY(k);
    }
    sprite_graphics[k] = kEnergyBall_Gfx[sprite_delay_main[k]];
    return;
  }
  if (sprite_B[k])
    SeekerEnergyBall_Draw(k);
  else
    SpriteDraw_SingleLarge(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  sprite_subtype2[k]++;
  Sprite_MoveXY(k);
  if (Sprite_CheckTileCollision(k)) {
    sprite_state[k] = 0;
    if (sprite_B[k]) {
      SpriteSfx_QueueSfx3WithPan(k, 0x36);
      CreateSixBlueBalls(k);
      return;
    }
  }

  if (sprite_A[k] && !sprite_ignore_projectile[0]) {
    SpriteHitBox hb;
    hb.r0_xlo = sprite_x_lo[k];
    hb.r8_xhi = sprite_x_hi[k];
    hb.r2 = hb.r3 = 15;
    hb.r1_ylo = sprite_y_lo[k];
    hb.r9_yhi = sprite_y_hi[k];
    Sprite_SetupHitBox(0, &hb);
    if (CheckIfHitBoxesOverlap(&hb)) {
      Sprite_GiveDamage(0, 16, 0xa0);
      sprite_state[k] = 0;
      sprite_x_recoil[0] = sprite_x_vel[k];
      sprite_y_recoil[0] = sprite_y_vel[k];
    }
  } else {
    Sprite_CheckDamageToLink(k);
    if (Sprite_CheckDamageFromLink(k) & kCheckDamageFromPlayer_Carry) {
      if (sprite_B[k]) {
        sprite_state[k] = 0;
        SpriteSfx_QueueSfx3WithPan(k, 0x36);
        CreateSixBlueBalls(k);
        return;
      }
      SpriteSfx_QueueSfx2WithPan(k, 0x5);
      SpriteSfx_QueueSfx3WithPan(k, 0x29);
      Sprite_ApplySpeedTowardsLink(k, 0x30);
      sprite_x_vel[k] = -sprite_x_vel[k];
      sprite_y_vel[k] = -sprite_y_vel[k];
      sprite_A[k]++;
    }
  }
  if ((k ^ frame_counter) & 3 | sprite_B[k])
    return;
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0x7B, &info);
  if (j < 0)
    return;
  Sprite_SetSpawnedCoordinates(j, &info);
  sprite_delay_main[j] = 15;
  sprite_ai_state[j] = 15;
  sprite_B[j] = sprite_B[k];
}

void CreateSixBlueBalls(int k) {  // 9edb96
  static const int8 kEnergyBall_SplitXVel[6] = {0, 24, 24, 0, -24, -24};
  static const int8 kEnergyBall_SplitYVel[6] = {-32, -16, 16, 32, 16, -16};
  SpriteSfx_QueueSfx3WithPan(k, 0x36);
  tmp_counter = 5;
  do {
    SpriteSpawnInfo info;
    int j = Sprite_SpawnDynamically(k, 0x55, &info);
    if (j >= 0) {
      Sprite_SetX(j, info.r0_x + 4);
      Sprite_SetY(j, info.r2_y + 4);
      sprite_flags3[j] = sprite_flags3[j] & ~1 | 0x40;
      sprite_oam_flags[j] = 4;
      sprite_delay_aux1[j] = 4;
      sprite_flags4[j] = 20;
      sprite_C[j] = 20;
      sprite_E[j] = 20;
      sprite_x_vel[j] = kEnergyBall_SplitXVel[tmp_counter];
      sprite_y_vel[j] = kEnergyBall_SplitYVel[tmp_counter];
    }
  } while (!sign8(--tmp_counter));
  tmp_counter = 0;
}

void SeekerEnergyBall_Draw(int k) {  // 9edc3e
  static const DrawMultipleData kEnergyBall_Dmd[8] = {
    { 4, -3, 0x00ce, 0},
    {11,  4, 0x00ce, 0},
    { 4, 11, 0x00ce, 0},
    {-3,  4, 0x00ce, 0},
    {-1, -1, 0x00ce, 0},
    { 9, -1, 0x00ce, 0},
    {-1,  9, 0x00ce, 0},
    { 9,  9, 0x00ce, 0},
  };
  Sprite_DrawMultiple(k, &kEnergyBall_Dmd[(sprite_subtype2[k] >> 2 & 1) * 4], 4, NULL);
}

void Sprite_79_Bee(int k) {  // 9edc5b
  switch (sprite_ai_state[k]) {
  case 0:
    Bee_DormantHive(k);
    break;
  case 1:
    Bee_Main(k);
    break;
  case 2:
    Bee_PutInBottle(k);
    break;
  }
}

void Bee_DormantHive(int k) {  // 9edc68
  if (sprite_E[k])
    return;
  sprite_state[k] = 0;
  for (int i = 11; i >= 0; i--)
    SpawnBeeFromHive(k);
}

void SpawnBeeFromHive(int k) {  // 9edc8f
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0x79, &info);
  if (j >= 0) {
    Sprite_SetSpawnedCoordinates(j, &info);
    InitializeSpawnedBee(j);
  }
}

void InitializeSpawnedBee(int k) {  // 9edc9b
  sprite_ai_state[k] = 1;
  sprite_A[k] = sprite_delay_main[k] = kSpawnBee_InitDelay[k & 3];
  sprite_delay_aux4[k] = 96;
  sprite_x_vel[k] = kSpawnBee_InitVel[GetRandomNumber() & 7];
  sprite_y_vel[k] = kSpawnBee_InitVel[GetRandomNumber() & 7];
}

int ReleaseBeeFromBottle(int x_value) {  // 9edccf
  static const int8 kSpawnBee_XY[8] = {8, 2, -2, -8, 10, 5, -5, -10};

  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(x_value, 0xb2, &info);
  if (j >= 0) {
    sprite_floor[j] = link_is_on_lower_level;
    Sprite_SetX(j, link_x_coord + 8);
    Sprite_SetY(j, link_y_coord + 16);
    if (link_bottle_info[link_item_bottle_index - 1] == 8)
      sprite_head_dir[j] = 1;
    InitializeSpawnedBee(j);
    sprite_x_vel[j] = kSpawnBee_XY[GetRandomNumber() & 7];
    sprite_y_vel[j] = kSpawnBee_XY[GetRandomNumber() & 7];
    sprite_delay_main[j] = 64;
    sprite_A[j] = 64;
  }
  return j;
}

void Bee_Main(int k) {  // 9edd45
  Bee_HandleZ(k);
  SpriteDraw_SingleSmall(k);
  Bee_HandleInteractions(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (Sprite_ReturnIfRecoiling(k))
    return;
  if (sprite_head_dir[k])
    Sprite_SpawnSparkleGarnish(k);
  Bee_Bzzt(k);
  Sprite_MoveXY(k);
  sprite_graphics[k] = (k ^ frame_counter) >> 1 & 1;
  if (!sprite_delay_aux4[k]) {
    Sprite_CheckDamageToLink(k);
    if (Sprite_CheckDamageFromLink(k) & kCheckDamageFromPlayer_Ne) {
      Sprite_ShowMessageUnconditional(0xc8);
      sprite_ai_state[k] = 2; // put in bottle
      return;
    }
  }

  if (!frame_counter && sprite_A[k] != 16)
    sprite_A[k] -= 8;

  if (sprite_delay_main[k] == 0) {
    uint16 x = link_x_coord + (GetRandomNumber() & 3) * 5;
    uint16 y = link_y_coord + (GetRandomNumber() & 3) * 5;
    ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLocation(k, x, y, 20);
    sprite_y_vel[k] = pt.y;
    sprite_x_vel[k] = pt.x;
    sprite_oam_flags[k] = sprite_oam_flags[k] & ~0x40 | (sign8(pt.x) ? 0 : 0x40);
    sprite_delay_main[k] = k + sprite_A[k];
  }
}

int Sprite_Find_EmptyBottle() {  // 9ede2e
  for (int i = 0; i != 4; i++)
    if (link_bottle_info[i] == 2)
      return i;
  return -1;
}

void Bee_HandleInteractions(int k) {  // 9ede44
  if (submodule_index == 2 && (dialogue_message_index == 0xc8 || dialogue_message_index == 0xca))
    sprite_delay_aux4[k] = 40;
}

void Sprite_B2_PlayerBee(int k) {  // 9ede63
  static const uint8 kGoodBee_Tab0[2] = {0xa, 0x14};

  switch (sprite_ai_state[k]) {
  case 0:  // wait
    if (!sprite_E[k]) {
      sprite_state[k] = 0;
      uint8 or_bottle = link_bottle_info[0] | link_bottle_info[1] | link_bottle_info[2] | link_bottle_info[3];
      if (!(or_bottle & 8))
        GoldBee_SpawnSelf(k);
    }
    break;
  case 1: {// activated
    sprite_ignore_projectile[k] = 1;
    Bee_HandleZ(k);
    SpriteDraw_SingleSmall(k);
    Bee_HandleInteractions(k);
    if (Sprite_ReturnIfInactive(k))
      return;
    Bee_Bzzt(k);
    Sprite_MoveXY(k);
    sprite_graphics[k] = (k ^ frame_counter) >> 1 & 1;
    if (sprite_head_dir[k])
      Sprite_SpawnSparkleGarnish(k);
    if (sprite_B[k] >= kGoodBee_Tab0[sprite_head_dir[k]]) {
      sprite_defl_bits[k] = 64;
      return;
    }
    if (sprite_delay_aux4[k])
      return;
    if (Sprite_CheckDamageFromLink(k) & kCheckDamageFromPlayer_Ne) {
      Sprite_ShowMessageUnconditional(0xc8);
      sprite_ai_state[k]++;
      return;
    }
    if ((k ^ frame_counter) & 3)
      return;
    Point16U pt2;
    if (!PlayerBee_FindTarget(k, &pt2)) {
      pt2.x = link_x_coord + (GetRandomNumber() & 3) * 5;
      pt2.y = link_y_coord + (GetRandomNumber() & 3) * 5;
    }
    if ((k ^ frame_counter) & 7)
      return;
    ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLocation(k, pt2.x, pt2.y, 32);
    sprite_x_vel[k] = pt.x;
    sprite_y_vel[k] = pt.y;
    sprite_oam_flags[k] = sprite_oam_flags[k] & ~0x40 | (sign8(pt.x) ? 0 : 0x40);
    break;
  }
  case 2:  // bottle
    Bee_PutInBottle(k);
    break;
  }
}

void GoldBee_SpawnSelf(int k) {  // 9ede90
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0x79, &info);
  if (j >= 0) {
    Sprite_SetSpawnedCoordinates(j, &info);
    sprite_ai_state[j] = 1;
    sprite_delay_main[j] = 64;
    sprite_A[j] = 64;
    sprite_delay_aux4[j] = 96;
    sprite_head_dir[j] = 1;
    sprite_x_vel[j] = kSpawnBee_InitVel[GetRandomNumber() & 7];
    sprite_y_vel[j] = kSpawnBee_InitVel[GetRandomNumber() & 7];
  }
}

void Bee_HandleZ(int k) {  // 9edf8a
  sprite_z[k] = 16;
  if (sprite_head_dir[k])
    sprite_oam_flags[k] = (sprite_oam_flags[k] & 0xf1) | (((frame_counter >> 4 & 3) + 1) << 1);
}

bool PlayerBee_FindTarget(int k, Point16U *pt) {  // 9edfab
  int n = 16;
  int j = k * 4 & 0xf;
  do {
    if (j == k || sprite_state[j] < 9 || sprite_pause[j])
      continue;
    if (!(sprite_flags2[j] & 0x80)) {
      if (sprite_floor[k] != sprite_floor[j] || sprite_flags4[j] & 0x40 || sprite_ignore_projectile[j])
        continue;
    } else {
      if (!sprite_head_dir[k] || !(sprite_bump_damage[j] & 0x40))
        continue;
    }
    PlayerBee_HoneInOnTarget(j, k);
    pt->x = Sprite_GetX(j) + (GetRandomNumber() & 3) * 5;
    pt->y = Sprite_GetY(j) + (GetRandomNumber() & 3) * 5;
    return true;
  } while (j = (j - 1) & 0xf, --n);
  return false;
}

void Bee_Bzzt(int k) {  // 9ee02e
  if (!((k ^ frame_counter) & 31))
    SpriteSfx_QueueSfx3WithPan(k, 0x2c);
}

void Sprite_B3_PedestalPlaque(int k) {  // 9ee044
  PrepOamCoordsRet info;
  Sprite_PrepOamCoord(k, &info);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (!flag_is_link_immobilized && Sprite_CheckIfLinkIsBusy())
    return;

  link_position_mode &= ~0x20;
  if (BYTE(overworld_screen_index) != 48) {
    if (link_direction_facing || !Sprite_CheckDamageToLink_same_layer(k))
      return;
    if (hud_cur_item != kHudItem_BookMudora || !(filtered_joypad_H & 0x40)) {
      if (!(filtered_joypad_L & 0x80))
        return;
      Sprite_ShowMessageUnconditional(0xb6);
    } else {
      player_handler_timer = 0;
      link_position_mode = 32;
      sound_effect_1 = 0;
      Sprite_ShowMessageUnconditional(0xb7);
    }
  } else {
    if (link_direction_facing || !Sprite_CheckDamageToLink_same_layer(k))
      return;
    if (hud_cur_item != kHudItem_BookMudora || !(filtered_joypad_H & 0x40)) {
      if (!(filtered_joypad_L & 0x80))
        return;
      Sprite_ShowMessageUnconditional(0xbc);
    } else {
      player_handler_timer = 0;
      link_position_mode = 32;
      sound_effect_1 = 0;
      button_b_frames = 1;
      link_delay_timer_spin_attack = 0;
      link_player_handler_state = kPlayerState_OpeningDesertPalace;
      Sprite_ShowMessageUnconditional(0xbd);
    }
  }
}

void Sprite_B4_PurpleChest(int k) {  // 9ee0dd
  SpriteDraw_SingleLarge(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (!sprite_ai_state[k]) {
    if (Sprite_ShowMessageOnContact(k, 0x116) & 0x100 && follower_indicator == 0)
      sprite_ai_state[k] = 1;
  } else {
    sprite_state[k] = 0;
    follower_indicator = 12;
    LoadFollowerGraphics();
    Sprite_BecomeFollower(k);
  }
}

void Sprite_B5_BombShop(int k) {  // 9ee111
  switch (sprite_subtype2[k]) {
  case 0: Sprite_BombShop_Clerk(k); break;
  case 1: Sprite_BombShop_Bomb(k); break;
  case 2: Sprite_BombShop_SuperBomb(k); break;
  case 3: Sprite_BombShop_Huff(k); break;
  }
}

void Sprite_BombShop_Clerk(int k) {  // 9ee134
  static const uint8 kBombShopGuy_Gfx[8] = {0, 1, 0, 1, 0, 1, 0, 1};
  static const uint8 kBombShopGuy_Delay[8] = {255, 32, 255, 24, 15, 24, 255, 15};

  BombShopEntity_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (!sprite_delay_main[k]) {
    int j = sprite_E[k];
    sprite_E[k] = j + 1 & 7;
    sprite_delay_main[k] = kBombShopGuy_Delay[j];
    sprite_graphics[k] = kBombShopGuy_Gfx[j];
    if (sprite_graphics[k] == 0) {
      SpriteSfx_QueueSfx3WithPan(k, 0x11);
      BombShop_ClerkExhalation(k);
    } else {
      SpriteSfx_QueueSfx3WithPan(k, 0x12);
    }
  }
  bool flag = ((link_has_crystals & 5) == 5 && sram_progress_indicator_3 & 32);
  Sprite_ShowSolicitedMessage(k, flag ? 0x118 : 0x117);
  Sprite_BehaveAsBarrier(k);
}

void Sprite_BombShop_Bomb(int k) {  // 9ee190
  BombShopEntity_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_BehaveAsBarrier(k);
  if (!ShopItem_CheckForAPress(k))
    return;

  if (link_item_bombs != kMaxBombsForLevel[link_bomb_upgrades]) {
    if (!ShopItem_HandleCost(100)) {
      Sprite_ShowMessageUnconditional(0x17c);
      ShopItem_PlayBeep(k);
    } else {
      link_bomb_filler = 27;
      sprite_state[k] = 0;
      Sprite_ShowMessageUnconditional(0x119);
      ShopItem_HandleReceipt(k, 0x28);
    }
  } else {
    Sprite_ShowMessageUnconditional(0x16e);
    ShopItem_PlayBeep(k);
  }

}

void Sprite_BombShop_SuperBomb(int k) {  // 9ee1df
  BombShopEntity_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_BehaveAsBarrier(k);
  if (ShopItem_CheckForAPress(k)) {
    if (!ShopItem_HandleCost(100)) {
      Sprite_ShowMessageUnconditional(0x17c);
      ShopItem_PlayBeep(k);
    } else {
      follower_indicator = 13;
      LoadFollowerGraphics();
      Sprite_BecomeFollower(k);
      sprite_state[k] = 0;
      Sprite_ShowMessageUnconditional(0x11a);
    }
  }
}

void Sprite_BombShop_Huff(int k) {  // 9ee21a
  static const uint8 kSnoutPutt_Dmd[4] = {4, 0x44, 0xc4, 0x84};
  Oam_AllocateFromRegionC(4);
  SpriteDraw_SingleSmall(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  sprite_oam_flags[k] &= 0x30;
  sprite_oam_flags[k] |= kSnoutPutt_Dmd[frame_counter >> 2 & 3];
  sprite_z_vel[k]++;
  Sprite_MoveZ(k);
  if (!sprite_delay_main[k])
    sprite_state[k] = 0;
  sprite_graphics[k] = sprite_delay_main[k] >> 3 & 3;
}

void BombShop_ClerkExhalation(int k) {  // 9ee256
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0xb5, &info);
  if (j >= 0) {
    Sprite_SetX(j, info.r0_x + 4);
    Sprite_SetY(j, info.r2_y + 16);
    sprite_subtype2[j] = 3;
    sprite_ignore_projectile[j] = 3;
    sprite_z[j] = 4;
    sprite_z_vel[j] = -12;
    sprite_delay_main[j] = 23;
    sprite_flags3[j] &= ~0x11;
  }
}

void BombShopEntity_Draw(int k) {  // 9ee2c6
  static const DrawMultipleData kBombShopEntity_Dmd[6] = {
    {0, 0, 0x0a48, 2},
    {0, 0, 0x0a4c, 2},
    {0, 0, 0x04c2, 2},
    {0, 0, 0x04c2, 2},
    {0, 0, 0x084e, 2},
    {0, 0, 0x084e, 2},
  };
  PrepOamCoordsRet info;
  Sprite_DrawMultiplePlayerDeferred(k, &kBombShopEntity_Dmd[sprite_subtype2[k] * 2 + sprite_graphics[k]], 1, &info);
  SpriteDraw_Shadow(k, &info);

}

void Sprite_B6_Kiki(int k) {  // 9ee2ef
  switch(sprite_subtype2[k]) {
  case 0: Kiki_LyingInwait(k); break;
  case 1: Kiki_OfferEntranceService(k); break;
  case 2: Kiki_OfferInitialService(k); break;
  case 3: Kiki_Flee(k); break;
  }
}

void Kiki_Flee(int k) {  // 9ee2fe
  bool flag = Kiki_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (!sprite_z[k] &&
      (uint16)(cur_sprite_x - 0xc98) < 0xd0 &&
      (uint16)(cur_sprite_y - 0x6a5) < 0xd0)
    flag = true;

  if (flag)
    sprite_state[k] = 0;
  sprite_z_vel[k]-=2;
  Sprite_MoveXYZ(k);
  if (sign8(sprite_z[k])) {
    sprite_z[k] = 0;
    sprite_z_vel[k] = GetRandomNumber() & 15 | 16;
  }
  ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLocation(k, 0xcf5, 0x6fe, 16);
  sprite_x_vel[k] = pt.x << 1;
  sprite_y_vel[k] = pt.y << 1;
  tagalong_event_flags &= ~3;
  if (sign8(pt.x)) pt.x = -pt.x;
  if (sign8(pt.y)) pt.y = -pt.y;
  sprite_D[k] = (pt.x >= pt.y) ? (sprite_x_vel[k] >> 7) ^ 3 : (sprite_y_vel[k] >> 7) ^ 1;
  sprite_graphics[k] = frame_counter >> 3 & 1;
}

void Kiki_OfferInitialService(int k) {  // 9ee3af
  if (!sign8(sprite_ai_state[k] - 2))
    Kiki_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_MoveXYZ(k);
  sprite_z_vel[k]--;
  if (sign8(sprite_z[k])) {
    sprite_z_vel[k] = 0;
    sprite_z[k] = 0;
  }
  sprite_graphics[k] = frame_counter >> 3 & 1;
  switch(sprite_ai_state[k]) {
  case 0:
    Sprite_ShowMessageUnconditional(0x11e);
    sprite_ai_state[k]++;
    break;
  case 1:
    if (!choice_in_multiselect_box && ShopItem_HandleCost(10)) {
      Sprite_ShowMessageUnconditional(0x11f);
      tagalong_event_flags |= 3;
      sprite_state[k] = 0;
    } else {
      Sprite_ShowMessageUnconditional(0x120);
      tagalong_event_flags &= ~3;
      follower_indicator = 0;
      sprite_ai_state[k]++;
      flag_is_link_immobilized++;
    }
    break;
  case 2: {
    sprite_ai_state[k]++;
    ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLocation(k, 0xc45, 0x6fe, 9);
    sprite_y_vel[k] = pt.y;
    sprite_x_vel[k] = pt.x;
    sprite_D[k] = (pt.x >> 7) ^ 3;
    sprite_delay_main[k] = 32;
    break;
  }
  case 3:
    if (!sprite_delay_main[k]) {
      sprite_ai_state[k]++;
      sprite_z_vel[k] = 16;
      sprite_delay_main[k] = 16;
    }
    break;
  case 4:
    if (!sprite_delay_main[k] && !sprite_z[k]) {
      sprite_state[k] = 0;
      flag_is_link_immobilized = 0;
    }
    break;
  }
}

void Kiki_OfferEntranceService(int k) {  // 9ee4c9
  int j, t;
  Kiki_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_MoveXYZ(k);
  sprite_z_vel[k]--;
  if (sign8(sprite_z[k])) {
    sprite_z_vel[k] = 0;
    sprite_z[k] = 0;
  }
  switch(sprite_ai_state[k]) {
  case 0:  //
    Sprite_ShowMessageUnconditional(0x11b);
    sprite_ai_state[k]++;
    break;
  case 1:  //
    if (choice_in_multiselect_box || !ShopItem_HandleCost(100)) {
      Sprite_ShowMessageUnconditional(0x11c);
      sprite_subtype2[k] = 3;
    } else {
      Sprite_ShowMessageUnconditional(0x11d);
      flag_is_link_immobilized++;
      sprite_ai_state[k]++;
      sprite_D[k] = 0;
    }
    break;
  case 2:  //
  case 4:  //
  case 6: { //
    static const uint16 kKiki_Leave_X[3] = {0xf4f, 0xf70, 0xf5d};
    static const uint16 kKiki_Leave_Y[3] = { 0x661, 0x64c, 0x624 };

    sprite_graphics[k] = frame_counter >> 3 & 1;
    j = (sprite_ai_state[k] >> 1) - 1;
    if ((uint8)(kKiki_Leave_X[j] - sprite_x_lo[k] + 2) < 4 &&
        (uint8)(kKiki_Leave_Y[j] - sprite_y_lo[k] + 2) < 4) {
      sprite_ai_state[k]++;
      sprite_x_vel[k] = sprite_y_vel[k] = 0;
      sprite_delay_aux1[k] = 32;
      SpriteSfx_QueueSfx2WithPan(k, 0x21);
      return;
    }
    ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLocation(k, kKiki_Leave_X[j], kKiki_Leave_Y[j], 9);
    sprite_x_vel[k] = pt.x;
    sprite_y_vel[k] = pt.y;
    break;
  }
  case 3:  //
  case 5:  //
    if (!sprite_delay_aux1[k]) {
      static const uint8 kKiki_Zvel[2] = {32, 28};
      sprite_z_vel[k] = kKiki_Zvel[sprite_ai_state[k]++ >> 1 & 1];
      SpriteSfx_QueueSfx2WithPan(k, 0x20);
      sprite_D[k] = sprite_ai_state[k] >> 1 & 1 | 4;
    } else {
      sprite_D[k] = sprite_ai_state[k] >> 1 & 1 | 6;
      sprite_graphics[k] = frame_counter >> 3 & 1;
    }
    break;
  case 7: { //
    static const uint8 kKiki_Tab7[3] = {2, 1, 0xff};
    static const uint8 kKiki_Delay7[2] = {82, 0};
    static const int8 kKiki_Xvel7[4] = {0, 0, -9, 9};
    static const int8 kKiki_Yvel7[4] = {-9, 9, 0, 0};
    sprite_graphics[k] = frame_counter >> 3 & 1;
    if (sprite_z[k] || sprite_delay_main[k])
      return;
    j = sprite_A[k]++;
    t = kKiki_Tab7[j];
    if (!sign8(t)) {
      sprite_D[k] = t;
      sprite_delay_main[k] = kKiki_Delay7[j];
      sprite_x_vel[k] = kKiki_Xvel7[t];
      sprite_y_vel[k] = kKiki_Yvel7[t];
    } else {
      sprite_ai_state[k]++;
      sprite_x_vel[k] = sprite_y_vel[k] = 0;
      trigger_special_entrance = 1;
      subsubmodule_index = 0;
      overworld_entrance_sequence_counter = 0;
      sprite_D[k] = 0;
      flag_is_link_immobilized = 0;
    }
    break;
  }
  case 8:  //
    sprite_D[k] = 8;
    sprite_graphics[k] = 0;
    sprite_z_vel[k] = (GetRandomNumber() & 15) + 16;
    sprite_ai_state[k]++;
    break;
  case 9:  //
    if (sign8(sprite_z_vel[k]) && !sprite_z[k]) {
      sprite_ai_state[k]++;
      SpriteSfx_QueueSfx3WithPan(k, 0x25);
    }
    break;
  case 10:  //
    break;
  }
}

bool Kiki_Draw(int k) {  // 9ee859
  static const DrawMultipleData kKiki_Dmd1[32] = {
    { 0, -6, 0x0020, 2},
    { 0,  0, 0x0022, 2},
    { 0, -6, 0x0020, 2},
    { 0,  0, 0x4022, 2},
    { 0, -6, 0x0020, 2},
    { 0,  0, 0x0022, 2},
    { 0, -6, 0x0020, 2},
    { 0,  0, 0x4022, 2},
    {-1, -6, 0x0020, 2},
    { 0,  0, 0x0022, 2},
    {-1, -6, 0x0020, 2},
    { 0,  0, 0x0022, 2},
    { 1, -6, 0x4020, 2},
    { 0,  0, 0x4022, 2},
    { 1, -6, 0x4020, 2},
    { 0,  0, 0x4022, 2},
    { 0, -6, 0x01ce, 2},
    { 0,  0, 0x01ee, 2},
    { 0, -6, 0x01ce, 2},
    { 0,  0, 0x01ee, 2},
    { 0, -6, 0x41ce, 2},
    { 0,  0, 0x41ee, 2},
    { 0, -6, 0x41ce, 2},
    { 0,  0, 0x41ee, 2},
    {-1, -6, 0x01ce, 2},
    { 0,  0, 0x01ec, 2},
    {-1, -6, 0x41ce, 2},
    { 0,  0, 0x01ec, 2},
    { 1, -6, 0x41ce, 2},
    { 0,  0, 0x41ec, 2},
    { 1, -6, 0x01ce, 2},
    { 0,  0, 0x41ec, 2},
  };
  static const DrawMultipleData kKiki_Dmd2[12] = {
    {0, -6, 0x01ca, 0},
    {8, -6, 0x41ca, 0},
    {0,  2, 0x01da, 0},
    {8,  2, 0x41da, 0},
    {0, 10, 0x01cb, 0},
    {8, 10, 0x41cb, 0},
    {0, -6, 0x01db, 0},
    {8, -6, 0x41db, 0},
    {0,  2, 0x01cc, 0},
    {8,  2, 0x41cc, 0},
    {0, 10, 0x01dc, 0},
    {8, 10, 0x41dd, 0},
  };
  static const uint8 kKikiDma[32] = {
    0x20, 0xc0, 0x20, 0xc0, 0, 0xa0, 0, 0xa0, 0x40, 0x80, 0x40, 0x60, 0x40, 0x80, 0x40, 0x60,
    0, 0, 0xfa, 0xff, 0x20, 0, 0, 2, 0, 0, 0, 0, 0x22, 0, 0, 2  // zelda bug: OOB read
  };
  PrepOamCoordsRet info;
  if (sprite_D[k] < 8) {
    int j = sprite_D[k] * 2 + sprite_graphics[k];
    BYTE(dma_var6) = kKikiDma[j * 2 + 0];
    BYTE(dma_var7) = kKikiDma[j * 2 + 1];
    Sprite_DrawMultiple(k, &kKiki_Dmd1[j * 2], 2, &info);
    if (!sprite_pause[k])
      SpriteDraw_Shadow(k, &info);
  } else {
    Sprite_DrawMultiple(k, &kKiki_Dmd2[sprite_graphics[k] * 6], 6, &info);
    if (!sprite_pause[k])
      SpriteDraw_Shadow(k, &info);
  }

  return ((info.x | info.y) & 0xff00) != 0;
}

void Sprite_B7_BlindMaiden(int k) {  // 9ee8b6
  CrystalMaiden_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_TrackBodyToHead(k);
  sprite_head_dir[k] = Sprite_DirectionToFaceLink(k, NULL) ^ 3;
  if (!sprite_ai_state[k]) {
    if (Sprite_ShowMessageOnContact(k, 0x122) & 0x100)
      sprite_ai_state[k] = 1;
  } else {
    sprite_state[k] = 0;
    follower_indicator = 6;
    LoadFollowerGraphics();
    Sprite_BecomeFollower(k);
  }
}

void OldMan_RevertToSprite(int k) {  // 9ee938
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0xAD, &info);
  sprite_D[j] = sprite_head_dir[j] = tagalong_layerbits[k] & 3;
  Sprite_SetY(j, (tagalong_y_lo[k] | tagalong_y_hi[k] << 8) + 2);
  Sprite_SetX(j, (tagalong_x_lo[k] | tagalong_x_hi[k] << 8) + 2);
  sprite_floor[j] = link_is_on_lower_level;
  sprite_ignore_projectile[j] = 1;
  sprite_subtype2[j] = 1;
  OldMan_EnableCutscene();
  follower_indicator = 0;
  link_speed_setting = 0;
}

void OldMan_EnableCutscene() {  // 9ee989
  flag_is_link_immobilized = 1;
  link_disable_sprite_damage = 1;
}

void Sprite_AD_OldMan(int k) {  // 9ee992
  static const uint16 kOldMountainManMsgs[3] = {0x9e, 0x9f, 0xa0};
  int j;
  OldMountainMan_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  switch(sprite_subtype2[k]) {
  case 0: // lost
    switch (sprite_ai_state[k]) {
    case 0: // supplicate
      Sprite_TrackBodyToHead(k);
      sprite_head_dir[k] = Sprite_DirectionToFaceLink(k, NULL) ^ 3;
      j = Sprite_ShowMessageOnContact(k, 0x9c);
      if (j & 0x100) {
        sprite_D[k] = sprite_head_dir[k] = j;
        sprite_ai_state[k] = 1;
      }
      break;
    case 1: // switch to tagalong
      follower_indicator = 4;
      Sprite_BecomeFollower(k);
      which_starting_point = 5;
      sprite_state[k] = 0;
      CacheCameraProperties();
      break;
    }
    break;
  case 1: // entering domicile
    Sprite_MoveXY(k);
    switch(sprite_ai_state[k]) {
    case 0:  // grant mirror
      sprite_ai_state[k]++;
      item_receipt_method = 0;
      Link_ReceiveItem(0x1a, 0);
      which_starting_point = 1;
      OldMan_EnableCutscene();
      sprite_delay_main[k] = 48;
      sprite_x_vel[k] = 8;
      sprite_y_vel[k] = 4;
      sprite_D[k] = sprite_head_dir[k] = 3;
      break;
    case 1:  // shuffle away
      OldMan_EnableCutscene();
      if (!sprite_delay_main[k])
        sprite_ai_state[k]++;
      sprite_graphics[k] = (k ^ frame_counter) >> 3 & 1;
      break;
    case 2: { // approach door
      sprite_head_dir[k] = 0;
      sprite_D[k] = 0;
      j = byte_7E0FDE;
      int x = overlord_x_lo[j] | overlord_x_hi[j] << 8;
      int y = overlord_y_lo[j] | overlord_y_hi[j] << 8;
      if (y >= Sprite_GetY(k)) {
        sprite_ai_state[k]++;
        sprite_y_vel[k] = sprite_x_vel[k] = 0;
      } else {
        ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLocation(k, x, y, 8);
        sprite_y_vel[k] = pt.y;
        sprite_x_vel[k] = pt.x;
        sprite_graphics[k] = (k ^ frame_counter) >> 3 & 1;
        OldMan_EnableCutscene();
      }
      break;
    }
    case 3:  // made it inside
      sprite_state[k] = 0;
      flag_is_link_immobilized = 0;
      link_disable_sprite_damage = 0;
      break;
    }
    break;
  case 2: // sitting at home
    Sprite_BehaveAsBarrier(k);
    if (sprite_ai_state[k]) {
      link_hearts_filler = 160;
      sprite_ai_state[k] = 0;
    }
    j = sram_progress_indicator >= 3 ? 2 : link_item_moon_pearl;
    if (Sprite_ShowSolicitedMessage(k, kOldMountainManMsgs[j]) & 0x100)
      sprite_ai_state[k]++;

    break;
  }
}

void Sprite_B8_DialogueTester(int k) {  // 9eeae7
  assert(0);
}

void Sprite_B9_BullyAndPinkBall(int k) {  // 9eeb33
  switch(sprite_subtype2[k]) {
  case 0: Sprite_PinkBall(k); return;
  case 1: PinkBall_Distress(k); return;
  case 2: Sprite_Bully(k); return;
  }
}

void Sprite_PinkBall(int k) {  // 9eeb40
  Oam_AllocateDeferToPlayer(k);
  SpriteDraw_SingleLarge(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  PinkBall_HandleMessage(k);
  sprite_oam_flags[k] = sprite_oam_flags[k] & ~0x80 | sprite_head_dir[k];
  Sprite_MoveXYZ(k);
  int t = Sprite_CheckTileCollision(k);
  if (t) {
    if (!(t & 3)) {
      sprite_y_vel[k] = -sprite_y_vel[k];
      if (sprite_E[k])
        goto play_sound;
    }
    sprite_x_vel[k] = -sprite_x_vel[k];
    if (sprite_E[k]) {
play_sound:
      BallGuy_PlayBounceNoise(k);
    }
  }

  sprite_z_vel[k]--;
  if (sign8(sprite_z[k])) {
    sprite_z[k] = 0;
    sprite_z_vel[k] = (uint8)-sprite_z_vel[k] >> 2;
    if (sprite_z_vel[k] & 0xfc)
      BallGuy_PlayBounceNoise(k);
    PinkBall_HandleDeceleration(k);
  }
  if (!sprite_E[k]) {
    if (!sprite_head_dir[k]) {
      PinkBall_Distress(k);
      sprite_graphics[k] = (k ^ frame_counter) >> 3 & 1;
      if (!((k ^ frame_counter) & 0x3f)) {
        uint16 x = (link_x_coord & 0xff00) | GetRandomNumber();
        uint16 y = (link_y_coord & 0xff00) | GetRandomNumber();
        ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLocation(k, x, y, 8);
        sprite_B[k] = pt.x;
        sprite_A[k] = pt.y;
        if (pt.y) {
          sprite_oam_flags[k] |= 64;
          sprite_oam_flags[k] ^= sprite_x_vel[k] >> 1 & 64;
        }
      }
      sprite_x_vel[k] = sprite_B[k];
      sprite_y_vel[k] = sprite_A[k];
    } else {
      PinkBall_Distress(k);
      if (k ^ frame_counter) {
        sprite_graphics[k] = (k ^ frame_counter) >> 2 & 1;
        sprite_x_vel[k] = 0;
        sprite_y_vel[k] = 0;
      } else {
        sprite_head_dir[k] = 0;
      }
    }
  } else {
    if ((sprite_x_vel[k] | sprite_y_vel[k]) == 0) {
      sprite_E[k] = 0;
    } else {
      sprite_graphics[k] = (k ^ frame_counter) >> 2 & 1;
      sprite_head_dir[k] = (k ^ frame_counter) << 2 & 128;
    }
  }
}

void PinkBall_HandleDeceleration(int k) {  // 9eec4d
  if (sprite_x_vel[k])
    sprite_x_vel[k] += sign8(sprite_x_vel[k]) ? 2 : -2;
  if (sprite_y_vel[k])
    sprite_y_vel[k] += sign8(sprite_y_vel[k]) ? 2 : -2;
}

void PinkBall_Distress(int k) {  // 9eec74
  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  Sprite_DrawDistress_custom(info.x, info.y, frame_counter);
}

void Sprite_Bully(int k) {  // 9eec7c
  Bully_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Bully_HandleMessage(k);
  Sprite_MoveXYZ(k);
  int t = Sprite_CheckTileCollision(k);
  if (t) {
    if (!(t & 3))
      sprite_y_vel[k] = -sprite_y_vel[k];
    else
      sprite_x_vel[k] = -sprite_x_vel[k];
  }
  switch(sprite_ai_state[k]) {
  case 0: { // chase
    sprite_graphics[k] = (k ^ frame_counter) >> 3 & 1;
    int j = sprite_head_dir[k];
    if (!((k ^ frame_counter) & 0x1f)) {
      ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLocation(k, Sprite_GetX(j), Sprite_GetY(j), 14);
      sprite_y_vel[k] = pt.y;
      sprite_x_vel[k] = pt.x;
      if (pt.x)
        sprite_D[k] = sprite_x_vel[k] >> 7;
    }
    if (!sprite_z[j]) {
      if ((uint8)(sprite_x_lo[k] - sprite_x_lo[j] + 8) < 16 &&
          (uint8)(sprite_y_lo[k] - sprite_y_lo[j] + 8) < 16) {
        sprite_ai_state[k]++;
        BallGuy_PlayBounceNoise(k);
      }
    }
    break;
  }
  case 1: { // kick ball
    sprite_ai_state[k] = 2;
    int j = sprite_head_dir[k];
    sprite_x_vel[j] = sprite_x_vel[k] << 1;
    sprite_y_vel[j] = sprite_y_vel[k] << 1;
    sprite_x_vel[k] = 0;
    sprite_y_vel[k] = 0;
    sprite_z_vel[j] = GetRandomNumber() & 31;
    sprite_delay_main[k] = 96;
    sprite_graphics[k] = 1;
    sprite_E[j] = 1;
    break;
  }
  case 2:  // waiting
    if (!sprite_delay_main[k])
      sprite_ai_state[k] = 0;
    break;
  }
}

void Bully_Draw(int k) {  // 9eed9e
  static const DrawMultipleData kBully_Dmd[8] = {
    {0, -7, 0x46e0, 2},
    {0,  0, 0x46e2, 2},
    {0, -7, 0x46e0, 2},
    {0,  0, 0x46c4, 2},
    {0, -7, 0x06e0, 2},
    {0,  0, 0x06e2, 2},
    {0, -7, 0x06e0, 2},
    {0,  0, 0x06c4, 2},
  };
  PrepOamCoordsRet info;
  Sprite_DrawMultiplePlayerDeferred(k, &kBully_Dmd[sprite_D[k] * 4 + sprite_graphics[k] * 2], 2, &info);
  SpriteDraw_Shadow(k, &info);

}

void BallGuy_PlayBounceNoise(int k) {  // 9eedc2
  SpriteSfx_QueueSfx3WithPan(k, 0x32);
}

void SpawnBully(int k) {  // 9eedc9
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0xB9, &info);
  if (j >= 0) {
    Sprite_SetSpawnedCoordinates(j, &info);
    sprite_subtype2[j] = 2;
    sprite_head_dir[j] = k;
    sprite_ignore_projectile[j] = 1;
  }
}

void PinkBall_HandleMessage(int k) {  // 9eede8
  if (sprite_delay_aux4[k])
    return;
  int msg = link_item_moon_pearl & 1 ? 0x15c : 0x15b;
  if (Sprite_ShowMessageOnContact(k, msg) & 0x100) {
    sprite_x_vel[k] ^= 255;
    sprite_y_vel[k] ^= 255;
    if (sprite_E[k])
      BallGuy_PlayBounceNoise(k);
    sprite_delay_aux4[k] = 64;
  }
}

void Bully_HandleMessage(int k) {  // 9eee25
  if (sprite_delay_aux4[k])
    return;
  int msg = link_item_moon_pearl & 1 ? 0x15e : 0x15d;
  if (Sprite_ShowMessageOnContact(k, msg) & 0x100) {
    sprite_x_vel[k] ^= 255;
    sprite_y_vel[k] ^= 255;
    sprite_delay_aux4[k] = 64;
  }
}

void Sprite_BA_Whirlpool(int k) {  // 9eee5a
  static const uint8 kWhirlpool_OamFlags[4] = {0, 0x40, 0xc0, 0x80};

  if (BYTE(overworld_screen_index) == 0x1b) {
    PrepOamCoordsRet info;
    Sprite_PrepOamCoord(k, &info);
    if (Sprite_ReturnIfInactive(k))
      return;
    uint16 x = cur_sprite_x - link_x_coord + 0x40;
    uint16 y = cur_sprite_y - link_y_coord + 0xf;
    if (x < 0x51 && y < 0x12) {
      submodule_index = 35;
      link_triggered_by_whirlpool_sprite = 1;
      subsubmodule_index = 0;
      link_actual_vel_y = 0;
      link_actual_vel_x = 0;
      link_player_handler_state = 20;
      last_light_vs_dark_world = overworld_screen_index & 0x40;
    }
  } else {
    sprite_oam_flags[k] = (sprite_oam_flags[k] & 0x3f) | kWhirlpool_OamFlags[frame_counter >> 3 & 3];
    Oam_AllocateFromRegionB(4);
    cur_sprite_x -= 5;
    SpriteDraw_SingleLarge(k);
    if (Sprite_ReturnIfInactive(k))
      return;
    if (Sprite_CheckDamageToLink_same_layer(k)) {
      if (sprite_A[k] == 0) {
        submodule_index = 46;
        subsubmodule_index = 0;
      }
    } else {
      sprite_A[k] = 0;
    }
  }
}

void Sprite_BB_Shopkeeper(int k) {  // 9eeeef
  switch (sprite_subtype2[k]) {
  case 0: Shopkeeper_StandardClerk(k); break;
  case 1: ChestGameGuy(k); break;
  case 2: NiceThiefWithGift(k); break;
  case 3: MiniChestGameGuy(k); break;
  case 4: LostWoodsChestGameGuy(k); break;
  case 5:
  case 6: NiceThiefUnderRock(k); break;
  case 7: ShopItem_RedPotion150(k); break;
  case 8: ShopItem_FighterShield(k); break;
  case 9: ShopItem_FireShield(k); break;
  case 10: ShopItem_Heart(k); break;
  case 11: ShopItem_Arrows(k); break;
  case 12: ShopItem_Bombs(k); break;
  case 13: ShopItem_Bee(k); break;
  }
}

void Shopkeeper_StandardClerk(int k) {  // 9eef12
  if (is_in_dark_world) {
    Oam_AllocateDeferToPlayer(k);
    SpriteDraw_SingleLarge(k);
    if (Sprite_ReturnIfInactive(k))
      return;
    sprite_oam_flags[k] = (sprite_oam_flags[k] & 63) | (frame_counter << 3 & 64);
  } else {
    sprite_oam_flags[k] = 7;
    Shopkeeper_Draw(k);
    if (Sprite_ReturnIfInactive(k))
      return;
    sprite_graphics[k] = frame_counter >> 4 & 1;
  }
  Sprite_BehaveAsBarrier(k);
  int msg = is_in_dark_world == 0 ? 0x165 : 0x15f;
  Sprite_ShowSolicitedMessage(k, msg);
  if (sprite_ai_state[k] == 0 && (uint16)(cur_sprite_y + 0x60) >= link_y_coord) {
    Sprite_ShowMessageUnconditional(msg);
    sprite_ai_state[k] = 1;
  }
}

void ChestGameGuy(int k) {  // 9eef90
  Oam_AllocateDeferToPlayer(k);
  SpriteDraw_SingleLarge(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_BehaveAsBarrier(k);
  sprite_oam_flags[k] = (sprite_oam_flags[k] & 63) | (frame_counter << 3 & 64);
  switch (sprite_ai_state[k]) {
  case 0:
    if ((uint8)(minigame_credits - 1) >= 2 && Sprite_ShowSolicitedMessage(k, 0x160) & 0x100)
      sprite_ai_state[k] = 1;
    break;
  case 1:
    if (choice_in_multiselect_box == 0 && ShopItem_HandleCost(30)) {
      minigame_credits = 2;
      Sprite_ShowMessageUnconditional(0x164);
      sprite_ai_state[k] = 2;
    } else {
      Sprite_ShowMessageUnconditional(0x161);
      sprite_ai_state[k] = 0;
    }
    break;
  case 2:
    if (minigame_credits == 0)
      Sprite_ShowSolicitedMessage(k, 0x163);
    else
      Sprite_ShowSolicitedMessage(k, 0x17f);
    break;
  }
}

void NiceThief_Animate(int k) {  // 9ef017
  if (!(frame_counter & 3)) {
    sprite_graphics[k] = 2;
    uint8 dir = Sprite_DirectionToFaceLink(k, NULL);
    sprite_head_dir[k] = (dir == 3) ? 2 : dir;
  }
  Oam_AllocateDeferToPlayer(k);
  Thief_Draw(k);
}

void NiceThiefWithGift(int k) {  // 9ef038
  NiceThief_Animate(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_BehaveAsBarrier(k);
  switch (sprite_ai_state[k]) {
  case 0:
    if (Sprite_ShowSolicitedMessage(k, 0x176) & 0x100)
      sprite_ai_state[k] = 1;
    break;
  case 1:
    if (!(dung_savegame_state_bits & 0x4000)) {
      dung_savegame_state_bits |= 0x4000;
      sprite_ai_state[k] = 2;
      ShopItem_HandleReceipt(k, 0x46);
    } else {
      sprite_ai_state[k] = 0;
    }
    break;
  case 2:
    sprite_ai_state[k] = 0;
    break;
  }
}

void MiniChestGameGuy(int k) {  // 9ef078
  sprite_D[k] = Sprite_DirectionToFaceLink(k, NULL) ^ 3;
  sprite_graphics[k] = 0;
  MazeGameGuy_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_BehaveAsBarrier(k);
  switch (sprite_ai_state[k]) {
  case 0:
    if ((uint8)(minigame_credits - 1) >= 2 && Sprite_ShowSolicitedMessage(k, 0x17e) & 0x100)
      sprite_ai_state[k] = 1;
    break;
  case 1:
    if (choice_in_multiselect_box == 0 && ShopItem_HandleCost(20)) {
      minigame_credits = 1;
      Sprite_ShowMessageUnconditional(0x17f);
      sprite_ai_state[k] = 2;
    } else {
      Sprite_ShowMessageUnconditional(0x180);
      sprite_ai_state[k] = 0;
    }
    break;
  case 2:
    Sprite_ShowSolicitedMessage(k, (minigame_credits == 0) ? 0x163 : 0x17f);
    break;
  }

}

void LostWoodsChestGameGuy(int k) {  // 9ef0f3
  NiceThief_Animate(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_BehaveAsBarrier(k);
  switch (sprite_ai_state[k]) {
  case 0:
    if ((uint8)(minigame_credits - 1) >= 2 && Sprite_ShowSolicitedMessage(k, 0x181) & 0x100)
      sprite_ai_state[k] = 1;
    break;
  case 1:
    if (choice_in_multiselect_box == 0 && ShopItem_HandleCost(100)) {
      minigame_credits = 1;
      Sprite_ShowMessageUnconditional(0x17f);
      sprite_ai_state[k] = 2;
    } else {
      Sprite_ShowMessageUnconditional(0x180);
      sprite_ai_state[k] = 0;
    }
    break;
  case 2:
    Sprite_ShowSolicitedMessage(k, (minigame_credits == 0) ? 0x163 : 0x17f);
    break;
  }
}

void NiceThiefUnderRock(int k) {  // 9ef14f
  NiceThief_Animate(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_BehaveAsBarrier(k);
  Sprite_ShowSolicitedMessage(k, sprite_subtype2[k] == 5 ? 0x177 : 0x178);
}

void ShopItem_RedPotion150(int k) {  // 9ef16e
  SpriteDraw_ShopItem(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_BehaveAsBarrier(k);
  if (ShopItem_CheckForAPress(k)) {
    if (Sprite_Find_EmptyBottle() < 0) {
      Sprite_ShowMessageUnconditional(0x16d);
      ShopItem_PlayBeep(k);
    } else if (ShopItem_HandleCost(150)) {
      sprite_state[k] = 0;
      ShopItem_HandleReceipt(k, 0x2e);
    } else {
      Sprite_ShowMessageUnconditional(0x17c);
      ShopItem_PlayBeep(k);
    }
  }
}

void ShopKeeper_SpawnShopItem(int k, int pos, int what) {  // 9ef1b3
  static const int8 kShopKeeper_ItemX[3] = {-44, 8, 60};
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamicallyEx(k, 0xbb, &info, 12);
  assert(j >= 0);
  sprite_ignore_projectile[j] = sprite_subtype2[j] = what;
  Sprite_SetX(j, info.r0_x + kShopKeeper_ItemX[pos]);
  Sprite_SetY(j, info.r2_y + 0x27);
  sprite_flags2[j] |= 4;
}

void ShopItem_FighterShield(int k) {  // 9ef1f2
  SpriteDraw_ShopItem(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_BehaveAsBarrier(k);
  ShopItem_MakeShieldsDeflect(k);
  if (ShopItem_CheckForAPress(k)) {
    if (link_shield_type) {
      Sprite_ShowMessageUnconditional(0x166);
      ShopItem_PlayBeep(k);
      return;
    }
    if (!ShopItem_HandleCost(50)) {
      Sprite_ShowMessageUnconditional(0x17c);
      ShopItem_PlayBeep(k);
      return;
    }
    sprite_state[k] = 0;
    ShopItem_HandleReceipt(k, 4);
  }
  sprite_flags4[k] = 0x1c;
}

void ShopItem_FireShield(int k) {  // 9ef230
  SpriteDraw_ShopItem(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_BehaveAsBarrier(k);
  ShopItem_MakeShieldsDeflect(k);
  if (ShopItem_CheckForAPress(k)) {
    if (link_shield_type >= 2) {
      Sprite_ShowMessageUnconditional(0x166);
      ShopItem_PlayBeep(k);
      return;
    }
    if (!ShopItem_HandleCost(500)) {
      Sprite_ShowMessageUnconditional(0x17c);
      ShopItem_PlayBeep(k);
      return;
    }
    sprite_state[k] = 0;
    ShopItem_HandleReceipt(k, 5);
  }
  sprite_flags4[k] = 0x1c;
}

void ShopItem_MakeShieldsDeflect(int k) {  // 9ef261
  sprite_ignore_projectile[k] = 0;
  sprite_flags[k] = 8;
  sprite_defl_bits[k] = 4;
  sprite_flags4[k] = 0x1c;
  Sprite_CheckDamageFromLink(k);
  sprite_flags4[k] = 0xa;
}

void ShopItem_Heart(int k) {  // 9ef27d
  SpriteDraw_ShopItem(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_BehaveAsBarrier(k);
  if (ShopItem_CheckForAPress(k)) {
    if (link_health_current == link_health_capacity) {
      ShopItem_PlayBeep(k);
    } else if (ShopItem_HandleCost(10)) {
      sprite_state[k] = 0;
      ShopItem_HandleReceipt(k, 0x42);
    } else {
      Sprite_ShowMessageUnconditional(0x17c);
      ShopItem_PlayBeep(k);
    }
  }
}

void ShopItem_Arrows(int k) {  // 9ef2af
  SpriteDraw_ShopItem(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_BehaveAsBarrier(k);
  if (ShopItem_CheckForAPress(k)) {
    if (link_num_arrows == kMaxArrowsForLevel[link_arrow_upgrades]) {
      Sprite_ShowSolicitedMessage(k, 0x16e);
      ShopItem_PlayBeep(k);
    } else if (ShopItem_HandleCost(30)) {
      sprite_state[k] = 0;
      ShopItem_HandleReceipt(k, 0x44);
    } else {
      Sprite_ShowMessageUnconditional(0x17c);
      ShopItem_PlayBeep(k);
    }
  }
}

void ShopItem_Bombs(int k) {  // 9ef2f0
  SpriteDraw_ShopItem(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_BehaveAsBarrier(k);
  if (ShopItem_CheckForAPress(k)) {
    if (link_item_bombs == kMaxBombsForLevel[link_bomb_upgrades]) {
      Sprite_ShowSolicitedMessage(k, 0x16e);
      ShopItem_PlayBeep(k);
    } else if (ShopItem_HandleCost(50)) {
      sprite_state[k] = 0;
      ShopItem_HandleReceipt(k, 0x31);
    } else {
      Sprite_ShowMessageUnconditional(0x17c);
      ShopItem_PlayBeep(k);
    }
  }
}

void ShopItem_Bee(int k) {  // 9ef322
  SpriteDraw_ShopItem(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_BehaveAsBarrier(k);
  if (ShopItem_CheckForAPress(k)) {
    if (Sprite_Find_EmptyBottle() < 0) {
      Sprite_ShowSolicitedMessage(k, 0x16d);
      ShopItem_PlayBeep(k);
    } else if (ShopItem_HandleCost(10)) {
      sprite_state[k] = 0;
      ShopItem_HandleReceipt(k, 0xe);
    } else {
      Sprite_ShowMessageUnconditional(0x17c);
      ShopItem_PlayBeep(k);
    }
  }
}

void ShopItem_HandleReceipt(int k, uint8 item) {  // 9ef366
  static const uint16 kShopKeeper_GiveItemMsgs[7] = {0x168, 0x167, 0x167, 0x16c, 0x169, 0x16a, 0x16b};
  item_receipt_method = 0;
  Link_ReceiveItem(item, 0);
  int j = sprite_subtype2[k];
  if (j >= 7) {
    Sprite_ShowMessageUnconditional(kShopKeeper_GiveItemMsgs[j - 7]);
    ShopKeeper_RapidTerminateReceiveItem();
  }
}

void ShopItem_PlayBeep(int k) {  // 9ef38a
  SpriteSfx_QueueSfx2WithPan(k, 0x3c);
}

bool ShopItem_CheckForAPress(int k) {  // 9ef391
  if (!(filtered_joypad_L & 0x80))
    return false;
  return Sprite_CheckDamageToLink_same_layer(k);
}

bool ShopItem_HandleCost(int amt) {  // 9ef39e
  if (amt > link_rupees_goal)
    return false;
  link_rupees_goal -= amt;
  return true;
}

void SpriteDraw_ShopItem(int k) {  // 9ef4ce
  static const DrawMultipleData kShopKeeper_ItemWithPrice_Dmd[35] = {
    {-4, 16, 0x0231, 0},
    { 4, 16, 0x0213, 0},
    {12, 16, 0x0230, 0},
    { 0,  0, 0x02c0, 2},
    { 0, 11, 0x036c, 2},
    { 0, 16, 0x0213, 0},
    { 0, 16, 0x0213, 0},
    { 8, 16, 0x0230, 0},
    { 0,  0, 0x04ce, 2},
    { 4, 12, 0x0338, 0},
    {-4, 16, 0x0213, 0},
    { 4, 16, 0x0230, 0},
    {12, 16, 0x0230, 0},
    { 0,  0, 0x08cc, 2},
    { 4, 12, 0x0338, 0},
    { 0, 16, 0x0231, 0},
    { 0, 16, 0x0231, 0},
    { 8, 16, 0x0230, 0},
    { 4,  8, 0x0329, 0},
    { 4, 11, 0x0338, 0},
    {-4, 16, 0x0203, 0},
    {-4, 16, 0x0203, 0},
    { 4, 16, 0x0230, 0},
    { 0,  0, 0x04c4, 2},
    { 0, 11, 0x0338, 0},
    { 0, 16, 0x0213, 0},
    { 0, 16, 0x0213, 0},
    { 8, 16, 0x0230, 0},
    { 0,  0, 0x04e8, 2},
    { 0, 11, 0x036c, 2},
    { 0, 16, 0x0231, 0},
    { 0, 16, 0x0231, 0},
    { 8, 16, 0x0230, 0},
    { 4,  8, 0x0ff4, 0},
    { 4, 11, 0x0338, 0},
  };
  Sprite_DrawMultiplePlayerDeferred(k, &kShopKeeper_ItemWithPrice_Dmd[(sprite_subtype2[k] - 7) * 5], 5, NULL);
}

void Sprite_AC_Apple(int k) {  // 9ef515
  if (sprite_ai_state[k]) {
    Sprite_Apple(k);
    return;
  }
  if (sprite_E[k] == 0) {
    sprite_state[k] = 0;
    int n = (GetRandomNumber() & 3) + 2;
    do {
      SpawnApple(k);
    } while (--n >= 0);
  }
}

void SpawnApple(int k) {  // 9ef535
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0xac, &info);
  if (j < 0)
    return;
  Sprite_SetSpawnedCoordinates(j, &info);
  sprite_ai_state[j] = 1;
  sprite_A[j] = 255;
  sprite_z[j] = 8;
  sprite_z_vel[j] = 22;
  uint16 x = info.r0_x & ~0xff | GetRandomNumber();
  uint16 y = info.r2_y & ~0xff | GetRandomNumber();
  ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLocation(k, x, y, 10);
  sprite_x_vel[j] = pt.x;
  sprite_y_vel[j] = pt.y;
}

void Sprite_Apple(int k) {  // 9ef57c
  if (sprite_A[k] >= 16 || frame_counter & 2)
    SpriteDraw_SingleLarge(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  if (sprite_A[k] == 0) {
    sprite_state[k] = 0;
    return;
  }
  Sprite_MoveXYZ(k);
  if (Sprite_CheckDamageToLink(k)) {
    SpriteSfx_QueueSfx3WithPan(k, 0xb);
    link_hearts_filler += 8;
    sprite_state[k] = 0;
    return;
  }
  if (!(frame_counter & 1))
    sprite_A[k]--;

  if (!sign8(sprite_z[k] - 1)) {
    sprite_z_vel[k] -= 1;
    return;
  }
  sprite_z[k] = 0;
  uint8 a = sign8(sprite_z_vel[k]) ? sprite_z_vel[k] : 0;
  sprite_z_vel[k] = (uint8)(-a) >> 1;
  if (sprite_x_vel[k])
    sprite_x_vel[k] += sign8(sprite_x_vel[k]) ? 1 : -1;
  if (sprite_y_vel[k])
    sprite_y_vel[k] += sign8(sprite_y_vel[k]) ? 1 : -1;
}

void Sprite_BC_Drunkard(int k) {  // 9ef603
  DrinkingGuy_Draw(k);
  if (Sprite_ReturnIfInactive(k))
    return;
  Sprite_BehaveAsBarrier(k);
  if (!GetRandomNumber())
    sprite_delay_main[k] = 32;
  sprite_graphics[k] = sprite_delay_main[k] ? 1 : 0;
  if (Sprite_ShowSolicitedMessage(k, 0x175) & 0x100)
    sprite_graphics[k] = 0;
}

void SomariaPlatform_LocatePath(int k) {  // 9ef640
  for (;;) {
    uint8 tiletype = SomariaPlatformAndPipe_CheckTile(k);
    sprite_E[k] = tiletype;
    if (tiletype >= 0xb0 && tiletype < 0xbf)
      break;
    Sprite_SetX(k, Sprite_GetX(k) + 8);
    Sprite_SetY(k, Sprite_GetY(k) + 8);
  }
  sprite_x_lo[k] = (sprite_x_lo[k] & ~7) + 4;
  sprite_y_lo[k] = (sprite_y_lo[k] & ~7) + 4;
  sprite_head_dir[k] = sprite_D[k];
  SomariaPlatformAndPipe_HandleMovement(k);
  sprite_ignore_projectile[k]++;
  player_on_somaria_platform = 0;
  sprite_delay_aux4[k] = 14;
  sprite_graphics[k]++;
}

void Sprite_ED_SomariaPlatform(int k) {  // 9ef6d4
  switch(sprite_graphics[k]) {
  case 0: {
    SomariaPlatform_LocatePath(k);
    int j = Sprite_SpawnSuperficialBombBlast(k);
    if (j >= 0) {
      Sprite_SetX(j, Sprite_GetX(j) - 8);
      Sprite_SetY(j, Sprite_GetY(j) - 8);
    }
    break;
  }
  case 1:
    SomariaPlatform_Draw(k);
    if (Sprite_ReturnIfInactive(k))
      return;
    if (!(drag_player_x | drag_player_y) && sign8(player_near_pit_state - 2) && Sprite_CheckDamageToLink_ignore_layer(k)) {
      sprite_C[k] = 1;
      Link_CancelDash();
      if (link_player_handler_state != kPlayerState_Hookshot && link_player_handler_state != kPlayerState_SpinAttacking) {
        if (sprite_ai_state[k]) {
          SomariaPlatformAndPipe_HandleMovement(k);
          return;
        }
        sprite_A[k]++;
        player_on_somaria_platform = 2;
        if (!(sprite_A[k] & 7)) {
          uint8 a = SomariaPlatformAndPipe_CheckTile(k);
          if (a != sprite_E[k]) {
            sprite_E[k] = a;
            sprite_head_dir[k] = sprite_D[k];
            SomariaPlatformAndPipe_HandleMovement(k);
            SomariaPlatform_HandleDrag(k);
          }
        }
        if (BYTE(dungeon_room_index) != 36) {
          static const int8 kSomariaPlatform_DragX[8] = {0, 0, -1, 1, -1, 1, 1, -1};
          static const int8 kSomariaPlatform_DragY[8] = {-1, 1, 0, 0, -1, 1, -1, 1};
          int j = sprite_D[k];
          drag_player_x += kSomariaPlatform_DragX[j];
          drag_player_y += kSomariaPlatform_DragY[j];
          Sprite_MoveXY(k);
          SomariaPlatform_DragLink(k);
        } else {
          player_on_somaria_platform = 1;
        }
        return;
      }
    }
    if (sprite_C[k]) {
      player_on_somaria_platform = 0;
      sprite_C[k] = 0;
    }
    break;
  }
}

void SomariaPlatformAndPipe_HandleMovement(int k) {  // 9ef7af
  static const int8 kSomariaPlatform_Xvel[8] = {0, 0, -16, 16, -16, 16, 16, -16};
  static const int8 kSomariaPlatform_Yvel[8] = {-16, 16, 0, 0, -16, 16, -16, 16};
  SomariaPlatform_HandleJunctions(k);
  int j = sprite_D[k];
  sprite_x_vel[k] = kSomariaPlatform_Xvel[j];
  sprite_y_vel[k] = kSomariaPlatform_Yvel[j];
}

uint8 SomariaPlatformAndPipe_CheckTile(int k) {  // 9ef7c2
  uint16 x = Sprite_GetX(k), y = Sprite_GetY(k);
  return GetTileAttribute(0, &x, y);
}

void SomariaPlatform_Draw(int k) {  // 9ef860
  static const DrawMultipleData kSomariaPlatform_Dmd[16] = {
    {-16, -16, 0x00ac, 2},
    {  0, -16, 0x40ac, 2},
    {-16,   0, 0x80ac, 2},
    {  0,   0, 0xc0ac, 2},
    {-13, -13, 0x00ac, 2},
    { -3, -13, 0x40ac, 2},
    {-13,  -3, 0x80ac, 2},
    { -3,  -3, 0xc0ac, 2},
    {-10, -10, 0x00ac, 2},
    { -6, -10, 0x40ac, 2},
    {-10,  -6, 0x80ac, 2},
    { -6,  -6, 0xc0ac, 2},
    { -8,  -8, 0x00ac, 2},
    { -8,  -8, 0x40ac, 2},
    { -8,  -8, 0x80ac, 2},
    { -8,  -8, 0xc0ac, 2},
  };
  Oam_AllocateFromRegionB(0x10);
  Sprite_DrawMultiple(k, &kSomariaPlatform_Dmd[sprite_delay_aux4[k] & 12], 4, NULL);
}

void SomariaPlatform_HandleJunctions(int k) {  // 9ef87d
  switch (sprite_E[k]) {
  case 0xb2:
  case 0xb5:  // ZigZagRisingSlope
    sprite_D[k] ^= 3;
    break;
  case 0xb3:
  case 0xb4:  // ZigZagFallingSlope
    sprite_D[k] ^= 2;
    break;
  case 0xb6: { // TransitTile
    static const uint8 kSomariaPlatform_TransitDir[4] = {4, 8, 1, 2};
    sprite_ai_state[k] = 1;
    if (!link_auxiliary_state && joypad1H_last & kSomariaPlatform_TransitDir[sprite_D[k]]) {
      sprite_ai_state[k] = 0;
      sprite_D[k] ^= 1;
    }
    link_visibility_status = 0;
    player_on_somaria_platform = 1;
    break;
  }
  case 0xb7: {  // Tjunc_NoUp
    static const uint8 kSomariaPlatform_Keys1[4] = {3, 7, 6, 5};
    uint8 t = joypad1H_last & kSomariaPlatform_Keys1[sprite_D[k]];
    if (t & 8) {
      sprite_D[k] = 0;
    } else if (t & 4) {
      sprite_D[k] = 1;
    } else if (t & 2) {
      sprite_D[k] = 2;
    } else if (t & 1) {
      sprite_D[k] = 3;
    } else if (sprite_D[k] == 0) {
      sprite_D[k] = 2;
    }
    sprite_ai_state[k] = 0;
    break;
  }
  case 0xb8: {  // Tjunc_NoDown
    static const uint8 kSomariaPlatform_Keys2[4] = {11, 3, 10, 9};
    uint8 t = joypad1H_last & kSomariaPlatform_Keys2[sprite_D[k]];
    if (t & 8) {
      sprite_D[k] = 0;
    } else if (t & 4) {
      sprite_D[k] = 1;
    } else if (t & 2) {
      sprite_D[k] = 2;
    } else if (t & 1) {
      sprite_D[k] = 3;
    } else if (sprite_D[k] == 1) {
      sprite_D[k] = 2;
    }
    sprite_ai_state[k] = 0;
    break;
  }
  case 0xb9: {  // Tjunc_NoLeft
    static const uint8 kSomariaPlatform_Keys3[4] = {9, 5, 12, 13};
    uint8 t = joypad1H_last & kSomariaPlatform_Keys3[sprite_D[k]];
    if (t & 8) {
      sprite_D[k] = 0;
    } else if (t & 4) {
      sprite_D[k] = 1;
    } else if (t & 2) {
      sprite_D[k] = 2;
    } else if (t & 1) {
      sprite_D[k] = 3;
    } else if (sprite_D[k] == 2) {
      sprite_D[k] = 0;
    }
    sprite_ai_state[k] = 0;
    break;
  }
  case 0xba: {  // Tjunc_NoRight
    static const uint8 kSomariaPlatform_Keys4[4] = {0xa, 6, 0xe, 0xc};
    uint8 t = joypad1H_last & kSomariaPlatform_Keys4[sprite_D[k]];
    if (t & 8) {
      sprite_D[k] = 0;
    } else if (t & 4) {
      sprite_D[k] = 1;
    } else if (t & 2) {
      sprite_D[k] = 2;
    } else if (t & 1) {
      sprite_D[k] = 3;
    } else if (sprite_D[k] == 3) {
      sprite_D[k] = 0;
    }
    sprite_ai_state[k] = 0;
    break;
  }
  case 0xbb: {  // TransitTileNoBack
    static const uint8 kSomariaPlatform_Keys5[4] = {0xb, 7, 0xe, 0xd};
    uint8 t = joypad1H_last & kSomariaPlatform_Keys5[sprite_D[k]];
    if (t & 8) {
      sprite_D[k] = 0;
    } else if (t & 4) {
      sprite_D[k] = 1;
    } else if (t & 2) {
      sprite_D[k] = 2;
    } else if (t & 1) {
      sprite_D[k] = 3;
    }
    break;
  }
  case 0xbc: {  // TransitTileQuestion
    static const uint8 kSomariaPlatform_Keys6[4] = {0xc, 0xc, 3, 3};
    sprite_ai_state[k] = 1;
    uint8 t = joypad1H_last & kSomariaPlatform_Keys6[sprite_D[k]];
    if (t) {
      if (t & 8) {
        sprite_D[k] = 0;
      } else if (t & 4) {
        sprite_D[k] = 1;
      } else if (t & 2) {
        sprite_D[k] = 2;
      } else {
        sprite_D[k] = 3;
      }
      sprite_ai_state[k] = 0;
    }
    player_on_somaria_platform = 1;
    break;
  }
  case 0xbe:  // endpoint
    sprite_ai_state[k] = 0;
    sprite_D[k] ^= 1;
    link_visibility_status = 0;
    player_on_somaria_platform = 1;
    break;
  }
}

void SomariaPlatform_HandleDragX(int k) {  // 9ef8ad
  if ((sprite_D[k] ^ sprite_head_dir[k]) & 2) {
    uint8 x = (sprite_x_lo[k] & ~7) + 4;
    uint8 t = x - sprite_x_lo[k];
    if (!t)
      return;
    drag_player_x = (int8)t;
    sprite_x_lo[k] = x;
  }
}

void SomariaPlatform_HandleDragY(int k) {  // 9ef8d7
  if ((sprite_D[k] ^ sprite_head_dir[k]) & 2) {
    uint8 y = (sprite_y_lo[k] & ~7) + 4;
    uint8 t = y - sprite_y_lo[k];
    if (!t)
      return;
    drag_player_y = (int8)t;
    sprite_y_lo[k] = y;
  }
}

void SomariaPlatform_HandleDrag(int k) {  // 9ef901
  SomariaPlatform_HandleDragX(k);
  SomariaPlatform_HandleDragY(k);
}

void SomariaPlatform_DragLink(int k) {  // 9efb49
  uint16 x = cur_sprite_x - 8 - link_x_coord;
  if (x)
    drag_player_x += sign16(x) ? -1 : 1;
  uint16 y = cur_sprite_y - 16 - link_y_coord;
  if (y)
    drag_player_y += sign16(y) ? -1 : 1;
}

void Sprite_AE_Pipe_Down(int k) {  // 9efb7e
  static const uint8 kPipe_Dirs[4] = {8, 4, 2, 1};

  uint8 t;
  if (Sprite_ReturnIfInactive(k))
    return;
  switch(sprite_graphics[k]) {
  case 0:  // locate transit tile
    alt_sprite_spawned_flag[0] = 255;
    sprite_D[k] = sprite_type[k] - 0xae;
    SomariaPlatform_LocatePath(k);
    break;
  case 1:  // locate endpoint
    t = SomariaPlatformAndPipe_CheckTile(k);
    if (t == 0xbe) {
      sprite_graphics[k]++;
      t = (sprite_D[k] ^= 1);
    }
    sprite_E[k] = t;
    sprite_head_dir[k] = sprite_D[k];
    SomariaPlatformAndPipe_HandleMovement(k);
    Sprite_MoveXY(k);
    break;
  case 2:  // wait for player
    if (alt_sprite_spawned_flag[0] == 255 && Sprite_CheckDamageToLink_ignore_layer(k)) {
      if (!Pipe_ValidateEntry()) {
        sprite_graphics[k]++;
        sprite_delay_aux1[k] = 4;
        Link_ResetProperties_A();
        flag_is_link_immobilized = 1;
        link_disable_sprite_damage = 1;
        alt_sprite_spawned_flag[0] = k;
      } else {
        Sprite_HaltAllMovement();
      }
    }
    break;
  case 3:  // draw player in
    if (!sprite_delay_aux1[k]) {
      sprite_graphics[k]++;
      link_visibility_status = 12;
    } else {
      flag_is_link_immobilized = 1;
      link_disable_sprite_damage = 1;
      Pipe_HandlePlayerMovement(kPipe_Dirs[sprite_D[k]]);
    }
    break;
  case 4: {  // draw player along
    sprite_subtype2[k] = 3;
    link_x_coord_safe_return_lo = link_x_coord;
    link_x_coord_safe_return_hi = link_x_coord >> 8;
    link_y_coord_safe_return_lo = link_y_coord;
    link_y_coord_safe_return_hi = link_y_coord >> 8;
    do {
      if (!(++sprite_A[k] & 7)) {
        uint8 t = SomariaPlatformAndPipe_CheckTile(k);
        if (t >= 0xb2 && t < 0xb6)
          SpriteSfx_QueueSfx2WithPan(k, 0xb);
        if (t != sprite_E[k]) {
          sprite_E[k] = t;
          if (t == 0xbe) {
            sprite_graphics[k]++;
            sprite_delay_aux1[k] = 24;
          }
          sprite_head_dir[k] = sprite_D[k];
          SomariaPlatformAndPipe_HandleMovement(k);
          SomariaPlatform_HandleDrag(k);
        }
      }
      Sprite_MoveXY(k);
      uint16 x = Sprite_GetX(k) - 8;
      uint16 y = Sprite_GetY(k) - 14;
      if (x != link_x_coord)
        link_x_coord += (x < link_x_coord) ? -1 : 1;
      if (y != link_y_coord)
        link_y_coord += (y < link_y_coord) ? -1 : 1;
    } while (--sprite_subtype2[k]);
    link_x_vel = link_x_coord - link_x_coord_safe_return_lo;
    link_y_vel = link_y_coord - link_y_coord_safe_return_lo;
    link_direction_last = kPipe_Dirs[sprite_D[k]];
    Link_HandleMovingAnimation_FullLongEntry();
    HandleIndoorCameraAndDoors();
    Link_CancelDash();
    break;
  }
  case 5:  //
    if (!sprite_delay_aux1[k]) {
      flag_is_link_immobilized = 0;
      player_on_somaria_platform = 0;
      link_disable_sprite_damage = 0;
      link_visibility_status = 0;
      link_x_vel = link_y_vel = 0;
      alt_sprite_spawned_flag[0] = 255;
      sprite_graphics[k] = 2;
    } else {
      Pipe_HandlePlayerMovement(kPipe_Dirs[sprite_D[k] ^ 1]);
    }
    break;
  }
}

void Pipe_HandlePlayerMovement(uint8 dir) {  // 9efcff
  link_direction_last = link_direction = dir;
  Link_HandleVelocity();
  Link_HandleMovingAnimation_FullLongEntry();
  HandleIndoorCameraAndDoors();
}

void Faerie_HandleMovement(int k) {  // 9efd1c
  sprite_graphics[k] = frame_counter >> 3 & 1;
  if (player_is_indoors && !sprite_delay_aux1[k]) {
    if (Sprite_CheckTileCollision(k) & 3) {
      sprite_x_vel[k] = -sprite_x_vel[k];
      sprite_D[k] = -sprite_D[k];
      sprite_delay_aux1[k] = 32;
    }
    if (sprite_wallcoll[k] & 12) {
      sprite_y_vel[k] = -sprite_y_vel[k];
      sprite_A[k] = -sprite_A[k];
      sprite_delay_aux1[k] = 32;
    }
  }
  if (sprite_x_vel[k]) {
    if (sign8(sprite_x_vel[k]))
      sprite_oam_flags[k] &= ~0x40;
    else
      sprite_oam_flags[k] |= 0x40;
  }
  Sprite_MoveXY(k);
  if (!(frame_counter & 63)) {
    uint16 x = (link_x_coord & ~0xff) + GetRandomNumber();
    uint16 y = (link_y_coord & ~0xff) + GetRandomNumber();
    ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLocation(k, x, y, 16);
    sprite_A[k] = pt.y;
    sprite_D[k] = pt.x;
  }
  if (!(frame_counter & 15)) {
    sprite_y_vel[k] = ((int8)sprite_A[k] + (int8)sprite_y_vel[k]) >> 1;
    sprite_x_vel[k] = ((int8)sprite_D[k] + (int8)sprite_x_vel[k]) >> 1;
  }
  Sprite_MoveZ(k);
  sprite_z_vel[k] += (GetRandomNumber() & 1) ? -1 : 1;
  if (sprite_z[k] < 8) {
    sprite_z[k] = 8;
    sprite_z_vel[k] = 5;
  } else if (sprite_z[k] >= 24) {
    sprite_z[k] = 24;
    sprite_z_vel[k] = -5;
  }
}

