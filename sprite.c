#include "sprite.h"
#include "dungeon.h"
#include "hud.h"
#include "load_gfx.h"
#include "overworld.h"
#include "variables.h"
#include "tagalong.h"
#include "overlord.h"
#include "ancilla.h"
#include "player.h"
#include "misc.h"
#include "overlord.h"
#include "tile_detect.h"
#include "tables/generated_dungeon_sprites.h"
#include "sprite_main.h"

static const uint16 kOamGetBufferPos_Tab0[6] = {0x171, 0x201, 0x31, 0xc1, 0x141, 0x1d1};
static const uint16 kOamGetBufferPos_Tab1[48] = {
   0x30,  0x50,  0x80,  0xb0,  0xe0, 0x110, 0x140, 0x170, 0x1d0, 0x1d4, 0x1dc, 0x1e0, 0x1e4, 0x1ec, 0x1f0, 0x1f8,
      0,     4,     8,   0xc,  0x10,  0x14,  0x18,  0x1c,  0x30,  0x38,  0x50,  0x68,  0x80,  0x98,  0xb0,  0xc8,
  0x120, 0x124, 0x128, 0x12c, 0x130, 0x134, 0x138, 0x13c, 0x140, 0x150, 0x160, 0x170, 0x180, 0x190, 0x1a0, 0x1b8,
};
static const uint8 kSprite2_ReturnIfRecoiling_Masks[6] = {3, 1, 0, 0, 0xc, 3};
static const int8 kSpriteHitbox_XLo[32] = {
  2, 3, 0, -3, -6, 0, 2, -8, 0, -4, -8, 0, -8, -16, 2, 2,
  2, 2, 2, -8, 2, 2, -16, -8, -12, 4, -4, -12, 5, -32, -2, 4,
};
static const int8 kSpriteHitbox_XHi[32] = {
  0, 0, 0, -1, -1, 0, 0, -1, 0, -1, -1, 0, -1, -1, 0, 0,
  0, 0, 0, -1, 0, 0, -1, -1, -1, 0, -1, -1, 0, -1, -1, 0,
};
static const uint8 kSpriteHitbox_XSize[32] = {
  12, 1, 16, 20, 20, 8, 4, 32, 48, 24, 32, 32, 32, 48, 12, 12,
  60, 124, 12, 32, 4, 12, 48, 32, 40, 8, 24, 24, 5, 80, 4, 8,
};
static const int8 kSpriteHitbox_YLo[32] = {
  0, 3, 4, -4, -8, 2, 0, -16, 12, -4, -8, 0, -10, -16, 2, 2,
  2, 2, -3, -12, 2, 10, 0, -12, 16, 4, -4, -12, 3, -16, -8, 10,
};
static const int8 kSpriteHitbox_YHi[32] = {
  0, 0, 0, -1, -1, 0, 0, -1, 0, -1, -1, 0, -1, -1, 0, 0,
  0, 0, -1, -1, 0, 0, 0, -1, 0, 0, -1, -1, 0, -1, -1, 0,
};
static const uint8 kSpriteHitbox_YSize[32] = {
  14, 1, 16, 21, 24, 4, 8, 40, 20, 24, 40, 29, 36, 48, 60, 124,
  12, 12, 17, 28, 4, 2, 28, 20, 10, 4, 24, 16, 5, 48, 8, 12,
};
static const uint8 kSpriteDamage_Tab2[4] = {6, 4, 0, 0};
static const uint8 kSpriteDamage_Tab3[4] = {4, 6, 0, 2};
static const uint8 kSprite_Func21_Sfx[9] = {0x1f, 0x1f, 0x1e, 0x1e, 0x1e, 0x1f, 0x1f, 0x1f, 0x1f};
static const int8 kSparkleGarnish_XY[4] = {-4, 12, 3, 8};
static const uint8 kSpriteStunned_Main_Func1_Masks[7] = {0x7f, 0xf, 3, 1, 0, 0, 0};
static const uint8 kSprite_Func7_Tab[4] = {8, 4, 2, 1};
static const int8 kSprite_Func5_X[54] = {
  8, 8, 2, 14, 8, 8, -2, 10, 8, 8, 1, 14, 4, 4, 4, 4,
  4, 4, -2, 10, 8, 8, -25, 40, 8, 8, 2, 14, 8, 8, -8, 23,
  8, 8, -20, 36, 8, 8, -1, 16, 8, 8, -1, 16, 8, 8, -8, 24,
  8, 8, -8, 24, 8, 3,
};
static const int8 kSprite_Func5_Y[54] = {
  6, 20, 13, 13, 0, 8, 4, 4, 1, 14, 8, 8, 4, 4, 4, 4,
  -2, 10, 4, 4, -25, 40, 8, 8, 3, 16, 10, 10, -8, 25, 8, 8,
  -20, 36, 8, 8, -1, 16, 8, 8, 14, 3, 8, 8, -8, 24, 8, 8,
  -8, 32, 8, 8, 12, 4,
};
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
static const int8 kSprite_Func5_Tab3[256] = {
  0, 1, 2, 3, 2, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1,
  1, 1, 1, 0, 0, 0, 1, 2, -1, -1, -1, -1, -1, -1, -1, -1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1,
  0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 0, -1, -1, -1, -1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  0, 0, 0, 0, 0, 1, 0, 2, 0, 0, 0, 0, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};
static const int8 kSlopedTile[32] = {
  7, 6, 5, 4, 3, 2, 1, 0,
  0, 1, 2, 3, 4, 5, 6, 7,
  0, 1, 2, 3, 4, 5, 6, 7,
  7, 6, 5, 4, 3, 2, 1, 0,
};
static const uint8 kSprite_Func1_Tab[8] = {15, 15, 24, 15, 15, 19, 15, 15};
static const uint8 kSprite_Func1_Tab2[8] = {6, 6, 6, 12, 6, 6, 6, 15};
static const uint8 kSprite_Func14_Damage[12] = {1, 2, 3, 4, 2, 3, 4, 5, 1, 1, 2, 3};
static const uint8 kEnemyDamages[128] = {
  0, 1, 32, 255, 252, 251, 0, 0, 0, 2, 64, 4, 0, 0, 0, 0,
  0, 4, 64, 2, 3, 0, 0, 0, 0, 8, 64, 4, 0, 0, 0, 0,
  0, 16, 64, 8, 0, 0, 0, 0, 0, 16, 64, 8, 0, 0, 0, 0,
  0, 4, 64, 16, 0, 0, 0, 0, 0, 255, 64, 255, 252, 251, 0, 0,
  0, 4, 64, 255, 252, 251, 32, 0, 0, 100, 24, 100, 0, 0, 0, 0,
  0, 249, 250, 255, 100, 0, 0, 0, 0, 8, 64, 253, 4, 16, 0, 0,
  0, 8, 64, 254, 4, 0, 0, 0, 0, 16, 64, 253, 0, 0, 0, 0,
  0, 254, 64, 16, 0, 0, 0, 0, 0, 32, 64, 255, 0, 0, 0, 250,
};
static const uint8 kSpriteInit_Flags2[243] = {
     1,    2,    1, 0x82, 0x81, 0x84, 0x84, 0x84,    2,  0xf,    2,    1, 0x20,    3,    4, 0x84,
     1,    5,    4,    1, 0x80,    4, 0xa2, 0x83,    4,    2, 0x82, 0x62, 0x82, 0x80, 0x80, 0x85,
     1, 0xa5,    3,    4,    4, 0x83,    2,    1, 0x82, 0xa2, 0xa2, 0xa3, 0xaa, 0xa3, 0xa4, 0x82,
  0x82, 0x83, 0x82, 0x80, 0x82, 0x82, 0xa5, 0x80, 0xa4, 0x82, 0x81, 0x82, 0x82, 0x82, 0x81,    6,
     8,    8,    8,    8,    6,    8,    8,    8,    6,    7,    7,    2,    2, 0x22,    1,    1,
  0x20, 0x82,    7, 0x85,  0xf, 0x21,    5, 0x83,    2,    1,    1,    1,    1,    7,    7,    7,
     7,    0, 0x85, 0x83,    3, 0xa4,    0,    0,    0,    0,    9,    4, 0xa0,    0,    1,    0,
     0,    3, 0x8b, 0x86, 0xc2, 0x82, 0x81,    4, 0x82, 0x21,    6,    3,    1,    3,    3,    3,
     0,    0,    4,    5,    5,    3,    1,    2,    0,    0,    0,    2,    7,    0,    1,    1,
  0x87,    6,    0, 0x83,    2, 0x22, 0x22, 0x22, 0x22,    4,    3,    5,    1,    1,    4,    1,
     2,    8,    8, 0x80, 0x21,    3,    3,    3,    2,    2,    8, 0x8f, 0xa1, 0x81, 0x80, 0x80,
  0x80, 0x80, 0xa1, 0x80, 0x81, 0x81, 0x86, 0x81, 0x82, 0x82, 0x80, 0x80, 0x83,    6,    0,    0,
     5,    4,    6,    5,    2,    0,    0,    5,    4,    4,    7,  0xb,  0xc,  0xc,    6,    6,
     3, 0xa4,    4, 0x82, 0x81, 0x83, 0x10, 0x10, 0x81, 0x82, 0x82, 0x82, 0x83, 0x83, 0x83, 0x81,
  0x82, 0x83, 0x83, 0x81, 0x82, 0x81, 0x82, 0xa0, 0xa1, 0xa3, 0xa1, 0xa1, 0xa1, 0x83, 0x85, 0x83,
  0x83, 0x83, 0x83,
};
static const uint8 kSpriteInit_Health[243] = {
   12,   6, 255,   3,  3,   3,   3,   3,   2,  12,  4, 255,   0,   3,  12,   2,
    0,  20,   4,   4,  0, 255,   0,   2,   3,   8,  0,   0,   0,   0,   0,   0,
    8,   3,   8,   2,  2,   0,   3, 255,   0,   3,  3,   3,   3,   3,   3,   3,
    3,   0,   3,   0,  3,   3,   3,   0,   3,   0,  0,   0,   0,   3,   2, 255,
    2,   6,   4,   8,  6,   8,   6,   4,   8,   8,  8,   4,   4,   2,   2,   2,
  255,   8, 255,  48, 16,   8,   8, 255,   2,   0,  0, 255, 255, 255, 255, 255,
  255, 255, 255, 255,  4,   4, 255, 255, 255, 255, 16,   3,   0,   2,   4,   1,
  255,   4, 255,   0,  0,   0,   0, 255,   0,   0, 96, 255,  24, 255, 255, 255,
    3,   4, 255,  16,  8,   8,   0, 255,  32,  32, 32,  32,  32,   8,   8,   4,
    8,  64,  48, 255,  2, 255, 255, 255, 255,  16,  4,   2,   4,   4,   8,   8,
    8,  16,  64,  64,  8,   4,   8,   4,   4,   8, 12,  16,   0,   0,   0,   0,
    0,   0,   0,   0,  0,   0,   0,   0,   0,   0,  0,   0,   0, 128,  48, 255,
  255, 255, 255,   8,  0,   0,   0,  32,   0,   8,  5,  40,  40,  40,  90,  16,
   24,  64,   0,   4,  0,   0, 255, 255,   0,   0,  0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,  0,   0,   0,   0,   0,   0,  0,   0,   0,   0,   0,   0,
    0,   0,   0,
};
const uint8 kSpriteInit_BumpDamage[243] = {
  0x83, 0x83, 0x81,    2,    2,    2,    2,    2,    1, 0x13,    1,    1,    1,    1,    8,    1,
     1,    8,    5,    3, 0x40,    4,    0,    2,    3, 0x85,    0,    1,    0, 0x40,    0,    0,
     6,    0,    5,    3,    1,    0,    0,    3,    0,    0,    0,    0,    0,    0,    0,    0,
     0,    0,    0,    0,    0,    0,    0, 0x40,    0,    0,    0,    0,    0,    0,    2,    2,
     0,    1,    1,    3,    1,    3,    1,    1,    3,    3,    3,    1,    3,    1,    1,    1,
     1,    1,    1, 0x11, 0x14,    1,    1,    2,    5,    0,    0,    4,    4,    8,    8,    8,
     8,    4,    0,    4,    3,    2,    2,    2,    2,    2,    3,    1,    0,    0,    1, 0x80,
     5,    1,    0,    0,    0, 0x40,    0,    4,    0,    0, 0x14,    4,    6,    4,    4,    4,
     4,    3,    4,    4,    4,    1,    4,    4, 0x15,    5,    4,    5, 0x15, 0x15,    3,    5,
     0,    5, 0x15,    5,    5,    6,    6,    6,    6,    5,    3,    6,    5,    5,    3,    3,
     3,    6, 0x17, 0x15, 0x15,    5,    5,    1, 0x85, 0x83,    5,    4,    0,    0,    0,    0,
     0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 0x17, 0x17,    5,
     5,    5,    4,    3,    2, 0x10,    0,    6,    0,    5,    7, 0x17, 0x17, 0x17, 0x15,    7,
     6, 0x10,    0,    3,    3,    0, 0x19, 0x19,    0,    0,    0,    0,    0,    0,    0,    0,
     0,    0,    0, 0x10,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
     0,    0,    0,
};
static const uint8 kSpriteInit_Flags3[243] = {
  0x19,  0xb, 0x1b, 0x4b, 0x41, 0x41, 0x41, 0x4d, 0x1d,    1, 0x1d, 0x19, 0x8d, 0x1b,    9, 0x9d,
  0x3d,    1,    9, 0x11, 0x40,    1, 0x4d, 0x19,    7, 0x1d, 0x59, 0x80, 0x4d, 0x40,    1, 0x49,
  0x1b, 0x41,    3, 0x13, 0x15, 0x41, 0x18, 0x1b, 0x41, 0x47,  0xf, 0x49, 0x4b, 0x4d, 0x41, 0x47,
  0x49, 0x4d, 0x49, 0x40, 0x4d, 0x47, 0x49, 0x41, 0x74, 0x47, 0x5b, 0x58, 0x51, 0x49, 0x1d, 0x5d,
     3, 0x19, 0x1b, 0x17, 0x19, 0x17, 0x19, 0x1b, 0x17, 0x17, 0x17, 0x1b,  0xd,    9, 0x19, 0x19,
  0x49, 0x5d, 0x5b, 0x49,  0xd,    3, 0x13, 0x41, 0x1b, 0x5b, 0x5d, 0x43, 0x43, 0x4d, 0x4d, 0x4d,
  0x4d, 0x4d, 0x49,    1,    0, 0x41, 0x4d, 0x4d, 0x4d, 0x4d, 0x1d,    9, 0xc4,  0xd,  0xd,    9,
     3,    3, 0x4b, 0x47, 0x47, 0x49, 0x49, 0x41, 0x47, 0x36, 0x8b, 0x49, 0x1d, 0x49, 0x43, 0x43,
  0x43,  0xb, 0x41,  0xd,    7,  0xb, 0x1d, 0x43,  0xd, 0x43,  0xd, 0x1d, 0x4d, 0x4d, 0x1b, 0x1b,
   0xa,  0xb,    0,    5,  0xd,    1,    1,    1,    1,  0xb,    5,    1,    1,    1,    7, 0x17,
  0x19,  0xd,  0xd, 0x80, 0x4d, 0x19, 0x17, 0x19,  0xb,    9,  0xd, 0x4a, 0x12, 0x49, 0xc3, 0xc3,
  0xc3, 0xc3, 0x76, 0x40, 0x59, 0x41, 0x58, 0x4f, 0x73, 0x5b, 0x44, 0x41, 0x51,  0xa,  0xb,  0xb,
  0x4b,    0, 0x40, 0x5b,  0xd,    0,    0,  0xd, 0x4b,  0xb, 0x59, 0x41,  0xb,  0xd,    1,  0xd,
   0xd,    0, 0x50, 0x4c, 0x44, 0x51,    1,    1, 0xf2, 0xf8, 0xf4, 0xf2, 0xd4, 0xd4, 0xd4, 0xf8,
  0xf8, 0xf4, 0xf4, 0xd8, 0xf8, 0xd8, 0xdf, 0xc8, 0x69, 0xc1, 0xd2, 0xd2, 0xdc, 0xc7, 0xc1, 0xc7,
  0xc7, 0xc7, 0xc1,
};
static const uint8 kSpriteInit_Flags4[243] = {
     0,    0,    0, 0x43, 0x43, 0x43, 0x43, 0x43,    0,    0,    0,    0, 0x1c,    0,    0,    2,
     1,    3,    0,    0,    3, 0xc0,    7,    0,    0,    0,    7, 0x45, 0x43,    0, 0x40,  0xd,
     0,    0,    0,    0,    0,    0,    0,    0,    7,    7,    7,    7,    7,    7,  0xd,    7,
     7,    7,    7,    3,    7,    7,    7, 0x40,    3,    7,  0xd,    0,    7,    7,    0,    0,
     9, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12,    0,    0,    0,    0,
  0x80, 0x12,    9,    9,    0, 0x40,    0,  0xc,    0,    0,    0, 0x40, 0x40, 0x10, 0x10, 0x2e,
  0x2e, 0x40, 0x1e, 0x53,    0,  0xa,    0,    0,    0,    0, 0x12, 0x12, 0x40,    0,    0, 0x40,
  0x19,    0,    0,  0xa,  0xd,  0xa,  0xa, 0x80,  0xa, 0x41,    0, 0x40,    0, 0x49,    0,    0,
  0xc0,    0, 0x40,    0,    0, 0x40,    0,    0,    9, 0x80, 0xc0,    0, 0x40,    0,    0, 0x80,
     0,    0, 0x18, 0x5a,    0, 0xd4, 0xd4, 0xd4, 0xd4,    0, 0x40,    0, 0x80, 0x80, 0x40, 0x40,
  0x40,    0,    9, 0x1d,    0,    0,    0,    0,    0,    0,    0,    0,    0,  0xa, 0x1b, 0x1b,
  0x1b, 0x1b, 0x41,    0,    3,    7,    7,    3,  0xa,    0,    1,  0xa,  0xa,    9,    0,    0,
     0,    0,    9,    0,    0, 0x40, 0x40,    0,    0,    0,    0, 0x89, 0x80, 0x80,    0, 0x1c,
     0, 0x40,    0,    0, 0x1c,    7,    3,    3, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,
  0x44, 0x44, 0x44, 0x43, 0x44, 0x43, 0x40, 0xc0, 0xc0, 0xc7, 0xc3, 0xc3, 0xc0, 0x1b,    8, 0x1b,
  0x1b, 0x1b,    3,
};
static const uint8 kSpriteInit_Flags[243] = {
     0,    0,    0,    0,    0,    0,    0,    0,    0,  0xa,    0,    1, 0x30,    0, 0, 0x20,
  0x10,    0,    0,    1,    0,    0,    0,    0,    0,    0,    0,    8, 0x20,    0, 4,    0,
     0,    0,    0,    0,    0,    0,    1,    4,    0,    0,    0,    0,    0,    0, 0,    0,
     0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 0, 0x68,
  0x60, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61,    0,    0, 0,    0,
     0,    0,    2,    2,    2,    0,    0, 0x70,    0,    0,    0, 0x90, 0x90,    0, 0,    0,
     0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 0x60, 0x60,    0,    0, 0,    0,
     0,    0,    0,    0,    0,    0,    0, 0x80,    0,    0,    2,    0,    0, 0x70, 0,    0,
     0,    0,    0,    0,    0,    0, 0xb0,    0, 0xc2,    0, 0x20,    0,    2,    0, 0,    0,
     0,    0,    2,    0, 0xb0,    0,    0,    0,    0,    0,    0,    0, 0xa0, 0xa0, 0,    0,
     0,    4,    2,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 0,    0,
     0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 0xc2, 0,    0,
     0,    0,    0,    0,    4,    0,    0,    0,    0,    0,    0,    2,    2,    2, 2,    0,
     0,    0,    0,    0,    0,    0,  0xa,  0xa, 0x10, 0x10, 0x10, 0x10,    0,    0, 0, 0x10,
  0x10, 0x10, 0x10,    0, 0x10,    0,    0,    0,    0,    0,    0,    0,    0,    0, 0,    0,
     0,    0,    0,
};
static const uint8 kSpriteInit_Flags5[243] = {
  0x83, 0x96, 0x84, 0x80, 0x80, 0x80, 0x80, 0x80,    2,    0,    2, 0x80, 0xa0, 0x83, 0x97, 0x80,
  0x80, 0x94, 0x91,    7,    0, 0x80,    0, 0x80, 0x92, 0x96, 0x80, 0xa0,    0,    0,    0, 0x80,
     4, 0x80, 0x82,    6,    6,    0,    0, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,    0,    0, 0x80, 0x80, 0x90,
  0x80, 0x91, 0x91, 0x91, 0x97, 0x91, 0x95, 0x95, 0x93, 0x97, 0x14, 0x91, 0x92, 0x81, 0x82, 0x82,
  0x80, 0x85, 0x80, 0x80, 0x80,    4,    4, 0x80, 0x91, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
  0x80,    0, 0x80, 0x80, 0x82, 0x8a, 0x80, 0x80, 0x80, 0x80, 0x92, 0x91, 0x80, 0x82, 0x81, 0x81,
  0x80, 0x81, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x97, 0x80, 0x80, 0x80,
  0x80, 0xc2, 0x80, 0x15, 0x15, 0x17,    6,    0, 0x80,    0, 0xc0, 0x13, 0x40,    0,    2,    6,
  0x10, 0x14,    0,    0, 0x40,    0,    0,    0,    0, 0x13, 0x46, 0x11, 0x80, 0x80,    0,    0,
     0, 0x10,    0,    0,    0, 0x16, 0x16, 0x16, 0x81, 0x87, 0x82,    0, 0x80, 0x80,    0,    0,
     0,    0, 0x80, 0x80,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
     0,    0,    0, 0x80,    0,    0,    0, 0x17,    0, 0x12,    0,    0,    0,    0,    0, 0x10,
  0x17,    0, 0x40,    1,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
     0,    0,    0, 0x40,    0,    0,    0,    0,    0,    0,    0,    0, 0x80,    0,    0,    0,
     0,    0,    0,
};
static const uint8 kSpriteInit_DeflBits[243] = {
     0,    0, 0x44, 0x20, 0x20, 0x20, 0x20, 0x20,    0, 0x81,    0,    0, 0x48,    0,    0,    0,
     0,    0,    0,    0,    0,    0,    4,    0,    0,    0,    0, 0x48, 0x24, 0x80,    0,    0,
     0, 0x20,    0,    0,    0, 0x80,    0,    0,    0,    0,    0,    0,    0,    0,    0, 0x80,
  0x80,    0,    0,    0,    0,    0,    0, 0x80,    0, 0x80,    0,    2,    0,    0,    0,    4,
  0x80,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  0x84,    0, 0x81,    5,    1, 0x40,    8, 0xa0,    0,    0,    0,    0,    0, 0x84, 0x84, 0x84,
  0x84,    8, 0x80, 0x80, 0x80,    0, 0x80, 0x80, 0x80, 0x80,    0,    8, 0x80,    0,    0,    0,
  0x40,    0,    0,    0,    0,    0,    0,    0,    0,    2,    1,    0,    0,    4,    0,    0,
     0,    0, 0x80,    4,    4,    0,    0, 0x48,    0,    0,    4,    0,    1,    1,    0,    0,
  0x80,    0,    0,    0, 0x40,    8,    8,    8,    8,    0,    0,    0, 0x80, 0x80,    0,    0,
     0,    4,    1,    5,    0,    0,    0,    0,    0,    0,    0,    4,    2,    0, 0x80, 0x80,
  0x80, 0x80, 0x82, 0x80,    0,    0, 0x80,    0,    0, 0x80, 0x80,    0,    0,    1,    1, 0x40,
     0,    0,    4,    0,    0,    0,    0,    0,    0,    0,    4,    5,    5,    5, 0x80, 0x80,
     0,    0,    0,    0,    0,    0,    0,    0,    2,    2,    2,    2,    2,    2,    2,    2,
     2,    2,    2,    2,    2,    2,    2,    2,    2,    0, 0x82, 0x82,    8, 0x80, 0x20, 0x80,
  0x80, 0x80, 0x20,
};
static const int8 kPlayerDamages[30] = {
  2, 1, 1, 4, 4, 4, 0, 0, 0, 8, 4, 2, 8, 8, 8, 16,
  8, 4, 32, 16, 8, 32, 24, 16, 24, 16, 8, 64, 48, 24,
};
static const uint8 kPlayerActionBoxRun_YHi[4] = {0xff, 0, 0, 0};
static const uint8 kPlayerActionBoxRun_YLo[4] = {(uint8)-8, 16, 8, 8};
static const uint8 kPlayerActionBoxRun_XHi[4] = {0, 0, 0xff, 0};
static const uint8 kPlayerActionBoxRun_XLo[4] = {0, 0, (uint8)-8, 8};
static const int8 kPlayer_SetupActionHitBox_Tab0[65] = {
  0, 2, 0, 0, -8, 0, 2, 0, 2, 2, 1, 1, 0, 0, 0, 0,
  0, 2, 4, 4, 0, 0, -4, -4, -6, 2, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 0, 2, 2, 4, 4, 2, 2, 2, 2, 0, 0, 0, 0,
  0, 0, 0, 0, -4, -4, -10, 0, 2, 2, 0, 0, 0, 0, 0, 0,
  0,
};
static const int8 kPlayer_SetupActionHitBox_Tab1[65] = {
  15, 4, 8, 8, 8, 8, 12, 8, 4, 4, 6, 6, 0, 0, 0, 0,
  0, 4, 16, 12, 8, 8, 12, 11, 12, 4, 6, 6, 0, 0, 0, 0,
  0, 8, 8, 8, 10, 14, 15, 4, 4, 4, 6, 6, 0, 0, 0, 0,
  0, 8, 8, 8, 10, 14, 15, 4, 4, 4, 6, 6, 0, 0, 0, 0,
  0,
};
static const int8 kPlayer_SetupActionHitBox_Tab2[65] = {
  0, 2, 0, 2, 4, 4, 4, 7, 2, 2, 1, 1, 0, 0, 0, 0,
  0, 2, 0, 2, -4, -3, -8, 0, 0, 2, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 0, -2, 0, -4, 1, 2, 2, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 0, -2, 0, -4, 1, 2, 2, 1, 1, 0, 0, 0, 0,
  0,
};
static const int8 kPlayer_SetupActionHitBox_Tab3[65] = {
  15, 4, 8, 2, 12, 8, 12, 8, 4, 4, 6, 6, 0, 0, 0, 0,
  0, 4, 8, 4, 12, 12, 12, 4, 8, 4, 6, 4, 0, 0, 0, 0,
  0, 8, 8, 8, 8, 8, 12, 4, 4, 4, 6, 6, 0, 0, 0, 0,
  0, 8, 8, 8, 8, 8, 12, 4, 4, 4, 6, 6, 0, 0, 0, 0,
  0,
};
static const int8 kPlayer_SetupActionHitBox_Tab4[13] = {
  1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1
};
static const uint8 kSprite_PrepAndDrawSingleLarge_Tab1[236] = {
  200, 0, 107, 0, 0, 0, 0, 0, 0, 203, 0, 8, 10, 11, 0, 0,
  13, 0, 0, 86, 0, 0, 15, 17, 0, 19, 0, 0, 0, 0, 20, 0,
  21, 27, 0, 42, 42, 248, 0, 182, 0, 0, 0, 170, 0, 0, 28, 0,
  0, 0, 0, 0, 0, 0, 0, 243, 243, 0, 187, 39, 0, 0, 66, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 15, 63, 0, 0, 0, 64, 64,
  68, 0, 0, 0, 0, 71, 70, 0, 0, 72, 74, 101, 101, 0, 0, 0,
  0, 0, 143, 0, 0, 76, 78, 78, 78, 78, 0, 48, 36, 50, 56, 60,
  129, 0, 82, 0, 0, 0, 0, 0, 0, 92, 0, 98, 94, 0, 0, 0,
  101, 102, 0, 0, 0, 0, 110, 14, 0, 59, 66, 0, 0, 117, 120, 123,
  0, 0, 207, 0, 132, 141, 141, 141, 141, 0, 148, 117, 160, 0, 0, 162,
  166, 0, 0, 0, 177, 0, 181, 0, 189, 0, 0, 0, 105, 0, 0, 0,
  0, 0, 92, 0, 214, 230, 0, 0, 0, 219, 218, 233, 0, 0, 190, 192,
  106, 0, 249, 215, 0, 0, 0, 216, 0, 0, 222, 227, 0, 0, 0, 235,
  0, 0, 0, 0, 0, 0, 244, 244, 29, 31, 31, 31, 32, 32, 32, 33,
  34, 35, 35, 37, 40, 106, 246, 41, 0, 0, 205, 206,
};
static const uint8 kSprite_PrepAndDrawSingleLarge_Tab2[251] = {
  0xa0, 0xa2, 0xa0, 0xa2, 0x80, 0x82, 0x80, 0x82, 0xea, 0xec, 0x84, 0x4e, 0x61, 0xbd, 0x8c, 0x20,
  0x22, 0xc0, 0xc2, 0xe6, 0xe4, 0x82, 0xaa, 0x84, 0xac, 0x80, 0xa0, 0xca, 0xaf, 0x29, 0x39, 0xb,
  0x6e, 0x60, 0x62, 0x63, 0x4c, 0xea, 0xec, 0x24, 0x6b, 0x24, 0x22, 0x24, 0x26, 0x20, 0x30, 0x21,
  0x2a, 0x24, 0x86, 0x88, 0x8a, 0x8c, 0x8e, 0xa2, 0xa4, 0xa6, 0xa8, 0xaa, 0x84, 0x80, 0x82, 0x6e,
  0x40, 0x42, 0xe6, 0xe8, 0x80, 0x82, 0xc8, 0x8d, 0xe3, 0xe5, 0xc5, 0xe1, 4, 0x24, 0xe, 0x2e,
  0xc, 0xa, 0x9c, 0xc7, 0xb6, 0xb7, 0x60, 0x62, 0x64, 0x66, 0x68, 0x6a, 0xe4, 0xf4, 2, 2,
  0, 4, 0xc6, 0xcc, 0xce, 0x28, 0x84, 0x82, 0x80, 0xe5, 0x24, 0, 2, 4, 0xa0, 0xaa,
  0xa4, 0xa6, 0xac, 0xa2, 0xa8, 0xa6, 0x88, 0x86, 0x8e, 0xae, 0x8a, 0x42, 0x44, 0x42, 0x44, 0x64,
  0x66, 0xcc, 0xcc, 0xca, 0x87, 0x97, 0x8e, 0xae, 0xac, 0x8c, 0x8e, 0xaa, 0xac, 0xd2, 0xf3, 0x84,
  0xa2, 0x84, 0xa4, 0xe7, 0x8a, 0xa8, 0x8a, 0xa8, 0x88, 0xa0, 0xa4, 0xa2, 0xa6, 0xa6, 0xa6, 0xa6,
  0x7e, 0x7f, 0x8a, 0x88, 0x8c, 0xa6, 0x86, 0x8e, 0xac, 0x86, 0xbb, 0xac, 0xa9, 0xb9, 0xaa, 0xba,
  0xbc, 0x8a, 0x8e, 0x8a, 0x86, 0xa, 0xc2, 0xc4, 0xe2, 0xe4, 0xc6, 0xea, 0xec, 0xff, 0xe6, 0xc6,
  0xcc, 0xec, 0xce, 0xee, 0x4c, 0x6c, 0x4e, 0x6e, 0xc8, 0xc4, 0xc6, 0x88, 0x8c, 0x24, 0xe0, 0xae,
  0xc0, 0xc8, 0xc4, 0xc6, 0xe2, 0xe0, 0xee, 0xae, 0xa0, 0x80, 0xee, 0xc0, 0xc2, 0xbf, 0x8c, 0xaa,
  0x86, 0xa8, 0xa6, 0x2c, 0x28, 6, 0xdf, 0xcf, 0xa9, 0x46, 0x46, 0xea, 0xc0, 0xc2, 0xe0, 0xe8,
  0xe2, 0xe6, 0xe4, 0xb, 0x8e, 0xa0, 0xec, 0xea, 0xe9, 0x48, 0x58,
};
static const uint8 kOverworldAreaSprcollSizes[192] = {
  4, 4, 2, 4, 4, 4, 4, 2, 4, 4, 4, 4, 4, 4, 4, 4,
  2, 2, 2, 2, 2, 2, 2, 2, 4, 4, 2, 4, 4, 2, 4, 4,
  4, 4, 2, 4, 4, 2, 4, 4, 2, 2, 2, 2, 2, 2, 2, 2,
  4, 4, 2, 2, 2, 4, 4, 2, 4, 4, 2, 2, 2, 4, 4, 2,
  4, 4, 2, 4, 4, 4, 4, 2, 4, 4, 4, 4, 4, 4, 4, 4,
  2, 2, 2, 2, 2, 2, 2, 2, 4, 4, 2, 4, 4, 2, 4, 4,
  4, 4, 2, 4, 4, 2, 4, 4, 2, 2, 2, 2, 2, 2, 2, 2,
  4, 4, 2, 2, 2, 4, 4, 2, 4, 4, 2, 2, 2, 4, 4, 2,
  4, 4, 2, 4, 4, 4, 4, 2, 4, 4, 4, 4, 4, 4, 4, 4,
  2, 2, 2, 2, 2, 2, 2, 2, 4, 4, 2, 4, 4, 2, 4, 4,
  4, 4, 2, 4, 4, 2, 4, 4, 2, 2, 2, 2, 2, 2, 2, 2,
  4, 4, 2, 2, 2, 4, 4, 2, 4, 4, 2, 2, 2, 4, 4, 2,
};
static const uint16 kOam_ResetRegionBases[6] = {0x30, 0x1d0, 0, 0x30, 0x120, 0x140};
static const uint8 kGarnish_OamMemSize[23] = {
  0, 4, 4, 4, 4, 4,  4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
  8, 4, 4, 4, 8, 16,
};
static HandlerFuncK *const kGarnish_Funcs[22] = {
  &Garnish01_FireSnakeTail,
  &Garnish02_MothulaBeamTrail,
  &Garnish03_FallingTile,
  &Garnish04_LaserTrail,
  &Garnish_SimpleSparkle,
  &Garnish06_ZoroTrail,
  &Garnish07_BabasuFlash,
  &Garnish08_KholdstareTrail,
  &Garnish09_LightningTrail,
  &Garnish0A_CannonSmoke,
  &Garnish_WaterTrail,
  &Garnish0C_TrinexxIceBreath,
  NULL,
  &Garnish0E_TrinexxFireBreath,
  &Garnish0F_BlindLaserTrail,
  &Garnish10_GanonBatFlame,
  &Garnish11_WitheringGanonBatFlame,
  &Garnish12_Sparkle,
  &Garnish13_PyramidDebris,
  &Garnish14_KakKidDashDust,
  &Garnish15_ArrghusSplash,
  &Garnish16_ThrownItemDebris,
};
static const uint8 kSpriteFall_Tab1[32] = {
  13, 13, 13, 13, 13, 13, 13, 12, 12, 12, 12, 12, 3, 3, 3, 3,
  3, 2, 2, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
};
static const uint8 kSpriteFall_Tab2[32] = {
  5, 5, 5, 5, 5, 5, 5, 4, 4, 4, 4, 4, 3, 3, 3, 3,
  3, 2, 2, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
};
static const uint8 kSpriteFall_Tab3[16] = {0xff, 0x3f, 0x1f, 0xf, 0xf, 7, 3, 1, 0xff, 0x3f, 0x1f, 0xf, 7, 3, 1, 0};
static const int8 kSpriteFall_Tab4[4] = {0, 4, 8, 0};
static const DrawMultipleData kSpriteDrawFall0Data[12] = {
  {0, 0, 0x0146, 2},
  {0, 0, 0x0148, 2},
  {0, 0, 0x014a, 2},
  {4, 4, 0x014c, 0},
  {4, 4, 0x00b7, 0},
  {4, 4, 0x0080, 0},
  {0, 0, 0x016c, 2},
  {0, 0, 0x016e, 2},
  {0, 0, 0x014e, 2},
  {4, 4, 0x015c, 0},
  {4, 4, 0x00b7, 0},
  {4, 4, 0x0080, 0},
};
static const int8 kSpriteDrawFall1_X[56] = {
  -4, 4, -4, 12, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0,
  -4, 12, -4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0,
  -4, 12, -4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0,
  4, 0, 0, 0, 4, 0, 0, 0,
};
static const int8 kSpriteDrawFall1_Y[56] = {
  -4, -4, 4, 12, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0,
  -4, -4, 12, 4, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0,
  -4, -4, 12, 4, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0,
  4, 0, 0, 0, 4, 0, 0, 0,
};
static const uint8 kSpriteDrawFall1_Char[56] = {
  0xae, 0xa8, 0xa6, 0xaf, 0xaa, 0, 0, 0, 0xac, 0, 0, 0, 0xbe, 0, 0, 0,
  0xa8, 0xae, 0xaf, 0xa6, 0xaa, 0, 0, 0, 0xac, 0, 0, 0, 0xbe, 0, 0, 0,
  0xa6, 0xaf, 0xae, 0xa8, 0xaa, 0, 0, 0, 0xac, 0, 0, 0, 0xbe, 0, 0, 0,
  0xb6, 0, 0, 0, 0x80, 0, 0, 0,
};
static const uint8 kSpriteDrawFall1_Flags[56] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0x40, 0x40, 0x40, 0x40, 0x40, 0, 0, 0, 0x40, 0, 0, 0, 0x40, 0, 0, 0,
  0x80, 0x80, 0x80, 0x80, 0x80, 0, 0, 0, 0x80, 0, 0, 0, 0x80, 0, 0, 0,
  1, 0, 0, 0, 1, 0, 0, 0,
};
static const uint8 kSpriteDrawFall1_Ext[56] = {
  0, 2, 2, 0, 2, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0,
  2, 0, 0, 2, 2, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0,
  2, 0, 0, 2, 2, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
};
static const int8 kSpriteDistress_X[4] = {-3, 2, 7, 11};
static const int8 kSpriteDistress_Y[4] = {-5, -7, -7, -5};
static const uint8 kPikitDropItems[4] = {0xdc, 0xe1, 0xd9, 0xe6};
static const uint8 kPrizeMasks[7] = { 1, 1, 1, 0, 1, 1, 1 };
static const uint8 kPrizeItems[56] = {
  0xd8, 0xd8, 0xd8, 0xd8, 0xd9, 0xd8, 0xd8, 0xd9, 0xda, 0xd9, 0xda, 0xdb, 0xda, 0xd9, 0xda, 0xda,
  0xe0, 0xdf, 0xdf, 0xda, 0xe0, 0xdf, 0xd8, 0xdf, 0xdc, 0xdc, 0xdc, 0xdd, 0xdc, 0xdc, 0xde, 0xdc,
  0xe1, 0xd8, 0xe1, 0xe2, 0xe1, 0xd8, 0xe1, 0xe2, 0xdf, 0xd9, 0xd8, 0xe1, 0xdf, 0xdc, 0xd9, 0xd8,
  0xd8, 0xe3, 0xe0, 0xdb, 0xde, 0xd8, 0xdb, 0xe2,
};
static const uint8 kPrizeZ[15] = {0, 0x24, 0x24, 0x24, 0x20, 0x20, 0x20, 0x24, 0x24, 0x24, 0x24, 0, 0x24, 0x20, 0x20};
static const int8 kPerishOverlay_X[32] = {
  0, 0, 0, 8, 0, 8, 0, 8, 8, 8, 0, 8, 0, 8, 0, 8,
  0, 8, 0, 8, 0, 8, 0, 8, -3, 11, -3, 11, -6, 14, -6, 14,
};
static const int8 kPerishOverlay_Y[32] = {
  0, 0, 8, 8, 0, 0, 8, 8, 0, 0, 8, 8, 0, 0, 8, 8,
  0, 0, 8, 8, 0, 0, 8, 8, -3, -3, 11, 11, -6, -6, 14, 14,
};
static const uint8 kPerishOverlay_Char[32] = {
  0, 0xb9, 0, 0, 0xb4, 0xb5, 0xb5, 0xb4, 0xb9, 0, 0, 0, 0xb5, 0xb4, 0xb4, 0xb5,
  0xa8, 0xa8, 0xb8, 0xb8, 0xa8, 0xa8, 0xb8, 0xb8, 0xa9, 0xa9, 0xa9, 0xa9, 0x9b, 0x9b, 0x9b, 0x9b,
};
static const uint8 kPerishOverlay_Flags[32] = {
  4, 4, 4, 4, 4, 4, 0xc4, 0xc4, 0x44, 4, 4, 4, 0x44, 0x44, 0x84, 0x84,
  4, 0x44, 4, 0x44, 4, 0x44, 4, 0x44, 0x44, 4, 0xc4, 0x84, 4, 0x44, 0x84, 0xc4,
};
static HandlerFuncK *const kSprite_ExecuteSingle[12] = {
  &Sprite_inactiveSprite,
  &SpriteModule_Fall1,
  &SpriteModule_Poof,
  &SpriteModule_Drown,
  &SpriteModule_Explode,
  &SpriteModule_Fall2,
  &SpriteModule_Die,
  &SpriteModule_Burn,
  &SpriteModule_Initialize,
  &SpriteActive_Main,
  &SpriteModule_Carried,
  &SpriteModule_Stunned,
};
// it's not my job to tell you what to think, my job is to think about what you tell me'
#define R0 WORD(g_ram[0])
#define R2 WORD(g_ram[2])
static const uint8 kSpawnSecretItems[22] = {
  0xd9, 0x3e, 0x79, 0xd9, 0xdc, 0xd8, 0xda, 0xe4, 0xe1, 0xdc, 0xd8, 0xdf, 0xe0, 0xb, 0x42, 0xd3,
  0x41, 0xd4, 0xd9, 0xe3, 0xd8, 0,
};
static const uint8 kSpawnSecretItem_SpawnFlag[22] = {
  0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 1, 0, 0,
  0, 0, 0, 0, 0, 0,
};
static const uint8 kSpawnSecretItem_XLo[22] = {
  4, 0, 4, 4, 0, 4, 4, 4, 4, 0, 4, 4, 4, 0, 0, 0,
  0, 0, 4, 0, 4, 4,
};
static const uint8 kSpawnSecretItem_IgnoreProj[22] = {
  1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
  0, 0, 1, 1, 1, 1,
};
static const uint8 kSpawnSecretItem_ZVel[22] = {
  16, 0, 0, 16, 0, 0, 16, 16, 16, 16, 0, 16, 10, 16, 0, 0,
  0, 0, 16, 0, 0, 0,
};
static const uint8 kSprite_ReturnIfLifted_Dirs[4] = {4, 6, 0, 2};
static const uint8 kAbsorbable_Tab1[15] = {0, 1, 1, 1, 2, 2, 2, 0, 1, 1, 2, 2, 1, 2, 2};
static const uint8 kAbsorbable_Tab2[19] = {0, 0, 0, 0, 1, 2, 3, 0, 0, 4, 5, 0, 0, 0, 0, 2, 4, 6, 2};
static const int16 kNumberedAbsorbable_X[18] = { 0, 0, 8, 0, 0, 8, 0, 0, 8, 0, 0, 2, 0, 0, 2, 0, 0, 0, };
static const int16 kNumberedAbsorbable_Y[18] = { 0, 0, 8, 0, 0, 8, 0, 0, 8, 0, 8, 8, 0, 8, 8, 0, 8, 8, };
static const uint8 kNumberedAbsorbable_Char[18] = { 0x6e, 0x6e, 0x68, 0x6e, 0x6e, 0x78, 0x6e, 0x6e, 0x79, 0x63, 0x73, 0x69, 0x63, 0x73, 0x6a, 0x63, 0x73, 0x73, };
static const uint8 kNumberedAbsorbable_Ext[18] = { 2, 2, 0, 2, 2, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, };
static const uint8 kRupeesAbsorption[3] = {1, 5, 20};
const uint8 kAbsorptionSfx[15] = {0xb, 0xa, 0xa, 0xa, 0xb, 0xb, 0xb, 0xb, 0xb, 0xb, 0xb, 0xb, 0x2f, 0x2f, 0xb};
static const uint8 kBombsAbsorption[3] = {1, 4, 8};
const uint8 kAbsorbBigKey[2] = {0x40, 0x20};
static int AllocOverlord();
static int Overworld_AllocSprite(uint8 type);
uint16 Sprite_GetX(int k) {
  return sprite_x_lo[k] | sprite_x_hi[k] << 8;
}

uint16 Sprite_GetY(int k) {
  return sprite_y_lo[k] | sprite_y_hi[k] << 8;
}

void Sprite_SetX(int k, uint16 x) {
  sprite_x_lo[k] = x;
  sprite_x_hi[k] = x >> 8;
}

void Sprite_SetY(int k, uint16 y) {
  sprite_y_lo[k] = y;
  sprite_y_hi[k] = y >> 8;
}

void Sprite_ApproachTargetSpeed(int k, uint8 x, uint8 y) {
  if (sprite_x_vel[k] - x)
    sprite_x_vel[k] += sign8(sprite_x_vel[k] - x) ? 1 : -1;
  if (sprite_y_vel[k] - y)
    sprite_y_vel[k] += sign8(sprite_y_vel[k] - y) ? 1 : -1;
}

void SpriteAddXY(int k, int xv, int yv) {
  Sprite_SetX(k, Sprite_GetX(k) + xv);
  Sprite_SetY(k, Sprite_GetY(k) + yv);
}

void Sprite_MoveXYZ(int k) {
  Sprite_MoveZ(k);
  Sprite_MoveX(k);
  Sprite_MoveY(k);
}

void Sprite_Invert_XY_Speeds(int k) {
  sprite_x_vel[k] = -sprite_x_vel[k];
  sprite_y_vel[k] = -sprite_y_vel[k];
}

int Sprite_SpawnSimpleSparkleGarnishEx(int k, uint16 x, uint16 y, int limit) {
  int j = GarnishAllocLimit(limit);
  if (j >= 0) {
    garnish_type[j] = 5;
    garnish_active = 5;
    Garnish_SetX(j, Sprite_GetX(k) + x);
    Garnish_SetY(j, Sprite_GetY(k) + y - sprite_z[k] + 16);
    garnish_countdown[j] = 31;
    garnish_sprite[j] = k;
    garnish_floor[j] = sprite_floor[k];
  }
  g_ram[15] = j;
  return j;
}

static int AllocOverlord() {
  int i = 7;
  while (i >= 0 && overlord_type[i] != 0)
    i--;
  return i;
}

static int Overworld_AllocSprite(uint8 type) {
  int i = (type == 0x58) ? 4 :
          (type == 0xd0) ? 5 :
          (type == 0xeb || type == 0x53 || type  == 0xf3) ? 14 : 13;
  for (; i >= 0; i--) {
    if (sprite_state[i] == 0 || sprite_type[i] == 0x41 && sprite_C[i] != 0)
      break;
  }
  return i;
}

uint16 Garnish_GetX(int k) {
  return garnish_x_lo[k] | garnish_x_hi[k] << 8;
}

uint16 Garnish_GetY(int k) {
  return garnish_y_lo[k] | garnish_y_hi[k] << 8;
}

void Garnish_SparkleCommon(int k, uint8 shift) {
  static const uint8 kGarnishSparkle_Char[4] = {0x83, 0xc7, 0x80, 0xb7};
  uint8 t = garnish_countdown[k] >> shift;
  Point16U pt;
  if (Garnish_ReturnIfPrepFails(k, &pt))
    return;
  OamEnt *oam = GetOamCurPtr();
  oam->x = pt.x;
  oam->y = pt.y;
  oam->charnum = kGarnishSparkle_Char[t];
  int j = garnish_sprite[k];
  oam->flags = (sprite_oam_flags[j] | sprite_obj_prio[j]) & 0xf0 | 4;
  bytewise_extended_oam[oam - oam_buf] = 0;
}

void Garnish_DustCommon(int k, uint8 shift) {
  static const uint8 kRunningManDust_Char[3] = {0xdf, 0xcf, 0xa9};
  tmp_counter = garnish_countdown[k] >> shift;
  Point16U pt;
  if (Garnish_ReturnIfPrepFails(k, &pt))
    return;
  OamEnt *oam = GetOamCurPtr();
  oam->x = pt.x;
  oam->y = pt.y;
  oam->charnum = kRunningManDust_Char[tmp_counter];
  oam->flags = 0x24;
  bytewise_extended_oam[oam - oam_buf] = 0;
}

void SpriteModule_Explode(int k) {
  static const DrawMultipleData kSpriteExplode_Dmd[32] = {
    { 0,  0, 0x0060, 2},
    { 0,  0, 0x0060, 2},
    { 0,  0, 0x0060, 2},
    { 0,  0, 0x0060, 2},
    {-5, -5, 0x0062, 2},
    { 5, -5, 0x4062, 2},
    {-5,  5, 0x8062, 2},
    { 5,  5, 0xc062, 2},
    {-8, -8, 0x0062, 2},
    { 8, -8, 0x4062, 2},
    {-8,  8, 0x8062, 2},
    { 8,  8, 0xc062, 2},
    {-8, -8, 0x0064, 2},
    { 8, -8, 0x4064, 2},
    {-8,  8, 0x8064, 2},
    { 8,  8, 0xc064, 2},
    {-8, -8, 0x0066, 2},
    { 8, -8, 0x4066, 2},
    {-8,  8, 0x8066, 2},
    { 8,  8, 0xc066, 2},
    {-8, -8, 0x0068, 2},
    { 8, -8, 0x0068, 2},
    {-8,  8, 0x0068, 2},
    { 8,  8, 0x0068, 2},
    {-8, -8, 0x006a, 2},
    { 8, -8, 0x406a, 2},
    {-8,  8, 0x806a, 2},
    { 8,  8, 0xc06a, 2},
    {-8, -8, 0x004e, 2},
    { 8, -8, 0x404e, 2},
    {-8,  8, 0x804e, 2},
    { 8,  8, 0xc04e, 2},
  };

  if (sprite_A[k]) {
    if (!sprite_delay_main[k]) {
      sprite_state[k] = 0;
      for (int j = 15; j >= 0; j--) {
        if (sprite_state[j] == 4)
          return;
      }
      load_chr_halfslot_even_odd = 1;
      if (!Sprite_CheckIfScreenIsClear())
        flag_block_link_menu = 0;
    } else {
      Sprite_DrawMultiple(k, &kSpriteExplode_Dmd[((sprite_delay_main[k] >> 2) ^ 7) * 4], 4, NULL);
    }
    return;
  }
  sprite_floor[k] = 2;

  if (sprite_delay_main[k] == 32) {
    sprite_state[k] = 0;
    flag_is_link_immobilized = 0;
    if (player_near_pit_state != 2 && Sprite_CheckIfScreenIsClear()) {
      if (sprite_type[k] >= 0xd6) {
        music_control = 0x13;
      } else if (sprite_type[k] == 0x7a) {
        PrepareDungeonExitFromBossFight();
      } else {
        SpriteExplode_SpawnEA(k);
        return;
      }
    }
  }

  if (sprite_delay_main[k] >= 64 && (sprite_delay_main[k] >= 0x70 || !(sprite_delay_main[k] & 1)))
    SpriteActive_Main(k);

  uint8 type = sprite_type[k];
  if (sprite_delay_main[k] >= 0xc0)
    return;
  if ((sprite_delay_main[k] & 3) == 0)
    SpriteSfx_QueueSfx2WithPan(k, 0xc);
  if (sprite_delay_main[k] & ((type == 0x92) ? 3 : 7))
    return;

  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0x1c, &info);
  if (j >= 0) {
    static const int8 kSpriteExplode_RandomXY[16] = {0, 4, 8, 12, -4, -8, -12, 0, 0, 8, 16, 24, -24, -16, -8, 0};
    load_chr_halfslot_even_odd = 11;
    sprite_state[j] = 4;
    sprite_flags2[j] = 3;
    sprite_oam_flags[j] = 0xc;
    int xoff = kSpriteExplode_RandomXY[(GetRandomNumber() & 7) | ((type == 0x92) ? 8 : 0)];
    int yoff = kSpriteExplode_RandomXY[(GetRandomNumber() & 7) | ((type == 0x92) ? 8 : 0)];
    Sprite_SetX(j, info.r0_x + xoff);
    Sprite_SetY(j, info.r2_y + yoff - info.r4_z);
    sprite_delay_main[j] = 31;
    sprite_A[j] = 31;
  }
  // endif_1
}

void SpriteDeath_MainEx(int k, bool second_entry) {
  if (!second_entry) {
    uint8 type = sprite_type[k];
    if (type == 0xec) {
      ThrowableScenery_ScatterIntoDebris(k);
      return;
    }
    if (type == 0x53 || type == 0x54 || type == 0x92 || type == 0x4a && sprite_C[k] >= 2) {
      SpriteActive_Main(k);
      return;
    }
    if (sprite_delay_main[k] == 0) {
      Sprite_DoTheDeath(k);
      return;
    }
  }
  if (sign8(sprite_flags3[k])) {
    SpriteActive_Main(k);
    return;
  }
  if (!((frame_counter & 3) | submodule_index | flag_unk1))
    sprite_delay_main[k]++;
  SpriteDeath_DrawPoof(k);

  if (sprite_type[k] != 0x40 && sprite_delay_main[k] < 10)
    return;
  oam_cur_ptr += 16;
  oam_ext_cur_ptr += 4;
  uint8 bak = sprite_flags2[k];
  sprite_flags2[k] -= 4;
  SpriteActive_Main(k);
  sprite_flags2[k] = bak;
}

void SpriteModule_Burn(int k) {
  static const uint8 kFlame_Gfx[32] = {
    5, 4, 3, 1, 2, 0, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0,
    1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0,
  };
  sprite_hit_timer[k] = 0;
  int j = sprite_delay_main[k] - 1;
  if (j == 0) {
    Sprite_DoTheDeath(k);
    return;
  }
  uint8 bak = sprite_graphics[k];
  uint8 bak1 = sprite_oam_flags[k];
  sprite_graphics[k] = kFlame_Gfx[j >> 3];
  sprite_oam_flags[k] = 3;
  Flame_Draw(k);
  sprite_oam_flags[k] = bak1;
  sprite_graphics[k] = bak;

  oam_cur_ptr += 8;
  oam_ext_cur_ptr += 2;
  if (sprite_delay_main[k] >= 0x10) {
    uint8 bak = sprite_flags2[k];
    sprite_flags2[k] -= 2;
    SpriteActive_Main(k);
    sprite_flags2[k] = bak;
  }
}

void Sprite_HitTimer31(int k) {
  if (sprite_type[k] != 0x7a || is_in_dark_world)
    return;
  if (sprite_health[k] <= sprite_give_damage[k]) {
    dialogue_message_index = 0x140;
    Sprite_ShowMessageMinimal();
  }
}

void SpriteStunned_MainEx(int k, bool second_entry) {
  if (second_entry)
    goto ThrownSprite_TileAndSpriteInteraction;
  Sprite_DrawRippleIfInWater(k);
  SpriteStunned_Main_Func1(k);
  if (Sprite_ReturnIfPaused(k))
    return;
  if (sprite_F[k]) {
    if (sign8(sprite_F[k]))
      sprite_F[k] = 0;
    sprite_x_vel[k] = sprite_y_vel[k] = 0;
  }
  if (sprite_delay_main[k] < 0x20)
    Sprite_CheckDamageFromLink(k);
  if (Sprite_ReturnIfRecoiling(k))
    return;
  Sprite_MoveXY(k);
  if (!sprite_E[k]) {
    Sprite_CheckTileCollision(k);
    if (!sprite_state[k])
      return;
  ThrownSprite_TileAndSpriteInteraction:
    if (sprite_wallcoll[k] & 0xf) {
      Sprite_ApplyRicochet(k);
      if (sprite_state[k] == 11)
        SpriteSfx_QueueSfx2WithPan(k, 5);
    }
  }
  Sprite_CheckTileProperty(k, 0x68);

  if (kSpriteInit_Flags3[sprite_type[k]] & 0x10) {
    sprite_flags3[k] |= 0x10;
    if (sprite_tiletype == 32)
      sprite_flags3[k] &= ~0x10;
  }
  Sprite_MoveZ(k);
  sprite_z_vel[k] -= 2;
  uint8 z = sprite_z[k] - 1;
  if (z >= 0xf0) {
    sprite_z[k] = 0;
    if (sprite_type[k] == 0xe8 && sign8(sprite_z_vel[k] - 0xe8)) {
      sprite_state[k] = 6;
      sprite_delay_main[k] = 8;
      sprite_flags2[k] = 3;
      return;
    }
    ThrowableScenery_TransmuteIfValid(k);
    uint8 a = sprite_tiletype;
    if (sprite_tiletype == 32 && !(a = sprite_flags[k] >> 1, sprite_flags[k] & 1)) {  // wtf
      Sprite_Func8(k);
      return;
    }
    if (a == 9) {
      z = sprite_z_vel[k];
      sprite_z_vel[k] = 0;
      int j;
      SpriteSpawnInfo info;

      if (sign8(z - 0xf0) && (j = Sprite_SpawnDynamically(k, 0xec, &info)) >= 0) {
        Sprite_SetSpawnedCoordinates(j, &info);
        Sprite_Func22(j);
      }
    } else if (a == 8) {
      if (sprite_type[k] == 0xd2 || (GetRandomNumber() & 1))
        Sprite_SpawnLeapingFish(k);
      Sprite_Func22(k);
      return;
    }
    z = sprite_z_vel[k];
    if (sign8(z)) {
      z = (uint8)(-z) >> 1;
      sprite_z_vel[k] = z < 9 ? 0 : z;
    }
    sprite_x_vel[k] = (int8)sprite_x_vel[k] >> 1;
    if (sprite_x_vel[k] == 255)
      sprite_x_vel[k] = 0;
    sprite_y_vel[k] = (int8)sprite_y_vel[k] >> 1;
    if (sprite_y_vel[k] == 255)
      sprite_y_vel[k] = 0;
  }
  if (sprite_state[k] != 11 || sprite_unk5[k] != 0) {
    if (Sprite_ReturnIfLifted(k))
      return;
    if (sprite_type[k] != 0x4a)
      ThrownSprite_CheckDamageToSprites(k);
  }
}

void Ancilla_SpawnFallingPrize(uint8 item) {  // 85a51d
  AncillaAdd_FallingPrize(0x29, item, 4);
}

bool Sprite_CheckDamageToAndFromLink(int k) {  // 85ab93
  Sprite_CheckDamageFromLink(k);
  return Sprite_CheckDamageToLink(k);
}

uint8 Sprite_CheckTileCollision(int k) {  // 85b88d
  Sprite_CheckTileCollision2(k);
  return sprite_wallcoll[k];
}

bool Sprite_TrackBodyToHead(int k) {  // 85dca2
  if (sprite_head_dir[k] != sprite_D[k]) {
    if (frame_counter & 0x1f)
      return false;
    if (!((sprite_head_dir[k] ^ sprite_D[k]) & 2)) {
      sprite_D[k] = (((k ^ frame_counter) >> 5 | 2) & 3) ^ (sprite_head_dir[k] & 2);
      return false;
    }
  }
  sprite_D[k] = sprite_head_dir[k];
  return true;
}

void Sprite_DrawMultiple(int k, const DrawMultipleData *src, int n, PrepOamCoordsRet *info) {  // 85df6c
  PrepOamCoordsRet info_buf;
  if (!info)
    info = &info_buf;
  if (Sprite_PrepOamCoordOrDoubleRet(k, info))
    return;
  word_7E0CFE = 0;
  uint8 a = sprite_state[k];
  if (a == 10)
    a = sprite_unk4[k];
  if (a == 11)
    BYTE(word_7E0CFE) = sprite_unk5[k];
  OamEnt *oam = GetOamCurPtr();
  do {
    uint16 x = src->x + info->x;
    uint16 y = src->y + info->y;
    oam->x = x;
    oam->y = (uint16)(y + 0x10) < 0x100 ? y : 0xf0;
    uint16 d = src->char_flags ^ WORD(info->r4);
    if (word_7E0CFE >= 1)
      d = d & ~0xE00 | 0x400;
    WORD(oam->charnum) = d;
    bytewise_extended_oam[oam - oam_buf] = (x >> 8 & 1) | src->ext;
  } while (src++, oam++, --n);
}

void Sprite_DrawMultiplePlayerDeferred(int k, const DrawMultipleData *src, int n, PrepOamCoordsRet *info) {  // 85df75
  Oam_AllocateDeferToPlayer(k);
  Sprite_DrawMultiple(k, src, n, info);
}

int Sprite_ShowSolicitedMessage(int k, uint16 msg) {  // 85e1a7
  static const uint8 kShowMessageFacing_Tab0[4] = {4, 6, 0, 2};
  dialogue_message_index = msg;
  if (!Sprite_CheckDamageToLink_same_layer(k) ||
      Sprite_CheckIfLinkIsBusy() ||
      !(filtered_joypad_L & 0x80) ||
      sprite_delay_aux4[k] || link_auxiliary_state == 2)
    return sprite_D[k];
  uint8 dir = Sprite_DirectionToFaceLink(k, NULL);
  if (link_direction_facing != kShowMessageFacing_Tab0[dir])
    return sprite_D[k];
  Sprite_ShowMessageUnconditional(dialogue_message_index);
  sprite_delay_aux4[k] = 64;
  return dir ^ 0x103;
}

int Sprite_ShowMessageOnContact(int k, uint16 msg) {  // 85e1f0
  dialogue_message_index = msg;
  if (!Sprite_CheckDamageToLink_same_layer(k) || link_auxiliary_state == 2)
    return sprite_D[k];
  Sprite_ShowMessageUnconditional(dialogue_message_index);
  return Sprite_DirectionToFaceLink(k, NULL) ^ 0x103;
}

void Sprite_ShowMessageUnconditional(uint16 msg) {  // 85e219
  dialogue_message_index = msg;
  byte_7E0223 = 0;
  messaging_module = 0;
  submodule_index = 2;
  saved_module_for_menu = main_module_index;
  main_module_index = 14;
  Sprite_NullifyHookshotDrag();
  link_speed_setting = 0;
  Link_CancelDash();
  link_auxiliary_state = 0;
  link_incapacitated_timer = 0;
  if (link_player_handler_state == kPlayerState_RecoilWall)
    link_player_handler_state = kPlayerState_Ground;
}

bool Sprite_TutorialGuard_ShowMessageOnContact(int k, uint16 msg) {  // 85fa59
  dialogue_message_index = msg;
  uint8 bak2 = sprite_flags2[k];
  uint8 bak4 = sprite_flags4[k];
  sprite_flags2[k] = 0x80;
  sprite_flags4[k] = 0x07;
  bool rv = Sprite_CheckDamageToLink_same_layer(k);
  sprite_flags2[k] = bak2;
  sprite_flags4[k] = bak4;
  if (!rv)
    return rv;
  Sprite_NullifyHookshotDrag();
  link_is_running = 0;
  link_speed_setting = 0;
  if (!link_auxiliary_state)
    Sprite_ShowMessageMinimal();
  return rv;
}

void Sprite_ShowMessageMinimal() {  // 85fa8e
  byte_7E0223 = 0;
  messaging_module = 0;
  submodule_index = 2;
  saved_module_for_menu = main_module_index;
  main_module_index = 14;
}

void Prepare_ApplyRumbleToSprites() {  // 8680fa
  static const int8 kApplyRumble_X[4] = { -32, -32, -32, 16 };
  static const int8 kApplyRumble_Y[4] = { -32, 32, -24, -24 };
  static const uint8 kApplyRumble_WH[6] = { 0x50, 0x50, 0x20, 0x20, 0x50, 0x50 };
  int j = link_direction_facing >> 1;
  SpriteHitBox hb;
  uint16 x = link_x_coord + kApplyRumble_X[j];
  uint16 y = link_y_coord + kApplyRumble_Y[j];
  hb.r0_xlo = x;
  hb.r8_xhi = x >> 8;
  hb.r1_ylo = y;
  hb.r9_yhi = y >> 8;
  hb.r2 = kApplyRumble_WH[j];
  hb.r3 = kApplyRumble_WH[j + 2];
  Entity_ApplyRumbleToSprites(&hb);
}

void Sprite_SpawnImmediatelySmashedTerrain(uint8 what, uint16 x, uint16 y) {  // 86812d
  uint8 bak1 = flag_is_sprite_to_pick_up;
  uint8 bak2 = byte_7E0FB2;
  int k = Sprite_SpawnThrowableTerrain_silently(what, x, y);
  if (k >= 0)
    ThrowableScenery_TransmuteToDebris(k);
  byte_7E0FB2 = bak2;
  flag_is_sprite_to_pick_up = bak1;
}

void Sprite_SpawnThrowableTerrain(uint8 what, uint16 x, uint16 y) {  // 86814b
  sound_effect_1 = Link_CalculateSfxPan() | 29;
  Sprite_SpawnThrowableTerrain_silently(what, x, y);
}

int Sprite_SpawnThrowableTerrain_silently(uint8 what, uint16 x, uint16 y) {  // 868156
  int k = 15;
  for (; k >= 0 && sprite_state[k] != 0; k--);
  if (k < 0)
    return k;
  sprite_state[k] = 10;
  sprite_type[k] = 0xEC;
  Sprite_SetX(k, x);
  Sprite_SetY(k, y);
  SpritePrep_LoadProperties(k);
  sprite_floor[k] = link_is_on_lower_level;
  sprite_C[k] = what;
  if (what >= 6)
    sprite_flags2[k] = 0xa6;
  // oob read, this array has only 6 elements.
  uint8 flags = kThrowableScenery_Flags[what];
  if (what == 2) {
    if (player_is_indoors)
      sprite_oam_flags[k] = 0x80, flags = 0x50;  // wtf
  }
  sprite_oam_flags[k] = flags;
  sprite_unk4[k] = 9;
  flag_is_sprite_to_pick_up = 2;
  byte_7E0FB2 = 2;
  sprite_delay_main[k] = 16;
  sprite_floor[k] = link_is_on_lower_level;
  sprite_graphics[k] = 0;
  if (BYTE(dung_secrets_unk1) != 255) {
    if (!(BYTE(dung_secrets_unk1) | player_is_indoors) && (uint8)(sprite_C[k] - 2) < 2)
      Overworld_SubstituteAlternateSecret();
    if (dung_secrets_unk1 & 0x80) {
      sprite_graphics[k] = dung_secrets_unk1 & 0x7f;
      BYTE(dung_secrets_unk1) = 0;
    }
    Sprite_SpawnSecret(k);
  }
  return k;
}

void Sprite_SpawnSecret(int k) {  // 868264
  if (!player_is_indoors && (GetRandomNumber() & 8))
    return;
  int b = BYTE(dung_secrets_unk1);
  if (b == 0)
    return;
  if (b == 4)
    b = 19 + (GetRandomNumber() & 3);
  if (!kSpawnSecretItems[b - 1])
    return;
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, kSpawnSecretItems[b - 1], &info);
  if (j < 0)
    return;
  sprite_ai_state[j] = kSpawnSecretItem_SpawnFlag[b - 1];
  sprite_ignore_projectile[j] = kSpawnSecretItem_IgnoreProj[b - 1];
  sprite_z_vel[j] = kSpawnSecretItem_ZVel[b - 1];
  Sprite_SetX(j, info.r0_x + kSpawnSecretItem_XLo[b - 1]);
  Sprite_SetY(j, info.r2_y);
  sprite_z[j] = info.r4_z;
  sprite_graphics[j] = 0;
  sprite_delay_aux4[j] = 32;
  sprite_delay_aux2[j] = 48;
  uint8 type = sprite_type[j];
  if (type == 0xe4) {
    SpritePrep_SmallKey(j);
    sprite_stunned[j] = 255;
  } else if (type == 0xb) {
    sound_effect_1 = 0x30;
    if (BYTE(dungeon_room_index2) == 1)
      sprite_subtype[j] = 1;
    sprite_stunned[j] = 255;
  } else if (type == 0x41 || type == 0x42) {
    sound_effect_2 = 4;
    sprite_give_damage[j] = 0;
    sprite_hit_timer[j] = 160;
  } else if (type == 0x3e) {
    sprite_oam_flags[j] = 9;
  } else {
    sprite_stunned[j] = 255;
    if (type == 0x79)
      sprite_A[j] = 32;
  }
}

void Sprite_Main() {  // 868328
  if (!player_is_indoors) {
    ancilla_floor[0] = 0;
    ancilla_floor[1] = 0;
    ancilla_floor[2] = 0;
    ancilla_floor[3] = 0;
    ancilla_floor[4] = 0;
    Sprite_ProximityActivation();
  }
  is_in_dark_world = (savegame_is_darkworld != 0);
  if (submodule_index == 0)
    drag_player_x = drag_player_y = 0;
  Oam_ResetRegionBases();
  Garnish_ExecuteUpperSlots();
  Follower_Main();
  byte_7E0FB2 = flag_is_sprite_to_pick_up;
  flag_is_sprite_to_pick_up = 0;
  HIBYTE(dungmap_var8) = 0x80;

  if (set_when_damaging_enemies & 0x7f)
    set_when_damaging_enemies--;
  else
    set_when_damaging_enemies = 0;
  byte_7E0379 = 0;
  link_unk_master_sword = 0;
  link_prevent_from_moving = 0;
  if (sprite_alert_flag)
    sprite_alert_flag--;
  Ancilla_Main();
  Overlord_Main();
  archery_game_out_of_arrows = 0;
  for (int i = 15; i >= 0; i--) {
    cur_object_index = i;
    Sprite_ExecuteSingle(i);
  }
  Garnish_ExecuteLowerSlots();
  byte_7E069E[0] = byte_7E069E[1] = 0;
  ExecuteCachedSprites();
  if (load_chr_halfslot_even_odd)
    byte_7E0FC6 = load_chr_halfslot_even_odd;
}

void Oam_ResetRegionBases() {  // 8683d3
  memcpy(oam_region_base, kOam_ResetRegionBases, 12);
}

void Sprite_TimersAndOam(int k) {  // 8683f2
  Sprite_Get16BitCoords(k);

  uint8 num = ((sprite_flags2[k] & 0x1f) + 1) * 4;

  if (sort_sprites_setting) {
    if (sprite_floor[k])
      Oam_AllocateFromRegionF(num);
    else
      Oam_AllocateFromRegionD(num);
  } else {
    Oam_AllocateFromRegionA(num);
  }

  if (!(submodule_index | flag_unk1)) {
    if (sprite_delay_main[k])
      sprite_delay_main[k]--;
    if (sprite_delay_aux1[k])
      sprite_delay_aux1[k]--;
    if (sprite_delay_aux2[k])
      sprite_delay_aux2[k]--;
    if (sprite_delay_aux3[k])
      sprite_delay_aux3[k]--;

    uint8 timer = sprite_hit_timer[k] & 0x7f;
    if (timer) {
      if (sprite_state[k] >= 9) {
        if (timer == 31) {
          Sprite_HitTimer31(k);
        } else if (timer == 24) {
          Sprite_MiniMoldorm_Recoil(k);
        }
      }
      if (sprite_give_damage[k] < 251)
        sprite_obj_prio[k] = sprite_hit_timer[k] * 2 & 0xe;
      sprite_hit_timer[k]--;
    } else {
      sprite_hit_timer[k] = 0;
      sprite_obj_prio[k] = 0;
    }
    if (sprite_delay_aux4[k])
      sprite_delay_aux4[k]--;
  }

  static const uint8 kSpritePrios[4] = {0x20, 0x10, 0x30, 0x30};
  int floor = link_is_on_lower_level;
  if (floor != 3)
    floor = sprite_floor[k];
  sprite_obj_prio[k] = sprite_obj_prio[k] & 0xcf | kSpritePrios[floor];
}

void Sprite_Get16BitCoords(int k) {  // 8684c1
  cur_sprite_x = sprite_x_lo[k] | sprite_x_hi[k] << 8;
  cur_sprite_y = sprite_y_lo[k] | sprite_y_hi[k] << 8;
}

void Sprite_ExecuteSingle(int k) {  // 8684e2
  uint8 st = sprite_state[k];
  if (st != 0)
    Sprite_TimersAndOam(k);
  kSprite_ExecuteSingle[st](k);
}

void Sprite_inactiveSprite(int k) {  // 868510
  if (!player_is_indoors) {
    sprite_N_word[k] = 0xffff;
  } else {
    sprite_N[k] = 0xff;
  }
}

void SpriteModule_Fall1(int k) {  // 86852e
  if (!sprite_delay_main[k]) {
    sprite_state[k] = 0;
    Sprite_ManuallySetDeathFlagUW(k);
  } else {
    PrepOamCoordsRet info;
    if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
      return;
    SpriteFall_Draw(k, &info);
  }
}

void SpriteModule_Drown(int k) {  // 86859c
  static const DrawMultipleData kSpriteDrown_Dmd[8] = {
    {-7, -7, 0x0480, 0},
    {14, -6, 0x0483, 0},
    {-6, -6, 0x04cf, 0},
    {13, -5, 0x04df, 0},
    {-4, -4, 0x04ae, 0},
    {12, -4, 0x44af, 0},
    { 0,  0, 0x04e7, 2},
    { 0,  0, 0x04e7, 2},
  };
  static const uint8 kSpriteDrown_Oam_Flags[4] = {0, 0x40, 0xc0, 0x80};
  static const uint8 kSpriteDrown_Oam_Char[11] = {0xc0, 0xc0, 0xc0, 0xc0, 0xcd, 0xcd, 0xcd, 0xcb, 0xcb, 0xcb, 0xcb};

  if (sprite_ai_state[k]) {
    if (sprite_A[k] == 6)
      Oam_AllocateFromRegionC(8);
    sprite_flags3[k] ^= 16;
    SpriteDraw_SingleLarge(k);
    OamEnt *oam = GetOamCurPtr();
    int j = sprite_delay_main[k];
    if (j == 1)
      sprite_state[k] = 0;
    if (j != 0) {
      assert((j >> 1) < 11);
      oam->charnum = kSpriteDrown_Oam_Char[j >> 1];
      oam->flags = 0x24;
      return;
    }
    oam->charnum = 0x8a;
    oam->flags = kSpriteDrown_Oam_Flags[sprite_subtype2[k] >> 2 & 3] | 0x24;

    if (Sprite_ReturnIfPaused(k))
      return;
    sprite_subtype2[k]++;
    Sprite_MoveXY(k);
    Sprite_MoveZ(k);
    sprite_z_vel[k] -= 2;
    if (sign8(sprite_z[k])) {
      sprite_z[k] = 0;
      sprite_delay_main[k] = 18;
      sprite_flags3[k] &= ~0x10;
    }
  } else {
    if (Sprite_ReturnIfPaused(k))
      return;
    if (!(frame_counter & 1))
      sprite_delay_main[k]++;
    sprite_oam_flags[k] = 0;
    sprite_hit_timer[k] = 0;
    if (!sprite_delay_main[k])
      sprite_state[k] = 0;
    Sprite_DrawMultiple(k, &kSpriteDrown_Dmd[(sprite_delay_main[k] << 1 & 0xf8) >> 2], 2, NULL);
  }
}

void Sprite_DrawDistress_custom(uint16 xin, uint16 yin, uint8 time) {  // 86a733
  Oam_AllocateFromRegionA(0x10);
  if (!(time & 0x18))
    return;
  int i = 3;
  OamEnt *oam = GetOamCurPtr();
  do {
    uint16 x = xin + kSpriteDistress_X[i];
    uint16 y = yin + kSpriteDistress_Y[i];
    oam->x = x;
    oam->y = ClampYForOam(y);
    oam->charnum = 0x83;
    oam->flags = 0x22;
    bytewise_extended_oam[oam - oam_buf] = (x >> 8 & 1);
  } while (oam++, --i >= 0);
}

void Sprite_CheckIfLifted_permissive(int k) {  // 86aa0c
  Sprite_ReturnIfLiftedPermissive(k);
}

void Entity_ApplyRumbleToSprites(SpriteHitBox *hb) {  // 86ad03
  for (int j = 15; j >= 0; j--) {
    if (!(sprite_defl_bits[j] & 2) || sprite_E[j] == 0)
      continue;
    if (byte_7E0FC6 != 0xe) {
      Sprite_SetupHitBox(j, hb);
      if (!CheckIfHitBoxesOverlap(hb))
        continue;
    }
    sprite_E[j] = 0;
    sound_effect_2 = 0x30;
    sprite_z_vel[j] = 0x30;
    sprite_x_vel[j] = 0x10;
    sprite_delay_aux3[j] = 0x30;
    sprite_stunned[j] = 255;
    if (sprite_type[j] == 0xd8)
      Sprite_TransmuteToBomb(j);
  }
}

void Sprite_ZeroVelocity_XY(int k) {  // 86cf5d
  sprite_x_vel[k] = sprite_y_vel[k] = 0;
}

bool Sprite_HandleDraggingByAncilla(int k) {  // 86cf64
  int j = sprite_B[k];
  if (j-- == 0)
    return false;
  if (ancilla_type[j] == 0) {
    Sprite_HandleAbsorptionByPlayer(k);
  } else {
    sprite_x_lo[k] = ancilla_x_lo[j];
    sprite_x_hi[k] = ancilla_x_hi[j];
    sprite_y_lo[k] = ancilla_y_lo[j];
    sprite_y_hi[k] = ancilla_y_hi[j];
    sprite_z[k] = 0;
  }
  return true;
}

bool Sprite_ReturnIfPhasingOut(int k) {  // 86d0ed
  if (!sprite_stunned[k] || (submodule_index | flag_unk1))
    return false;
  if (!(frame_counter & 1))
    sprite_stunned[k]--;
  uint8 a = sprite_stunned[k];
  if (a == 0)
    sprite_state[k] = 0;
  else if (a >= 0x28 || (a & 1) != 0)
    return false;
  PrepOamCoordsRet info;
  Sprite_PrepOamCoordOrDoubleRet(k, &info);
  return true;
}

void Sprite_CheckAbsorptionByPlayer(int k) {  // 86d116
  if (!sprite_delay_aux4[k] && Sprite_CheckDamageToPlayer_1(k))
    Sprite_HandleAbsorptionByPlayer(k);
}

void Sprite_HandleAbsorptionByPlayer(int k) {  // 86d13c
  sprite_state[k] = 0;
  int t = sprite_type[k] - 0xd8;
  SpriteSfx_QueueSfx3WithPan(k, kAbsorptionSfx[t]);
  switch(t) {
  case 0:
    link_hearts_filler += 8;
    break;
  case 1: case 2: case 3:
    link_rupees_goal += kRupeesAbsorption[t - 1];
    break;
  case 4: case 5: case 6:
    link_bomb_filler += kBombsAbsorption[t - 4];
    break;
  case 7:
    link_magic_filler += 0x10;
    break;
  case 8:
    link_magic_filler = 0x80;
    break;
  case 9:
    link_arrow_filler += (sprite_head_dir[k] == 0) ? 5 : sprite_head_dir[k];
    break;
  case 10:
    link_arrow_filler += 10;
    break;
  case 11:
    SpriteSfx_QueueSfx2WithPan(k, 0x31);
    link_hearts_filler += 56;
    break;
  case 12:
    link_num_keys += 1;
    goto after_getkey;
  case 13:
    item_receipt_method = 0;
    Link_ReceiveItem(0x32, 0);
  after_getkey:
    sprite_N[k] = sprite_subtype[k];
    dung_savegame_state_bits |= kAbsorbBigKey[sprite_die_action[k]] << 8;
    Sprite_ManuallySetDeathFlagUW(k);
    break;
  case 14:
    link_shield_type = sprite_subtype[k];
    break;
  }
}

bool SpriteDraw_AbsorbableTransient(int k, bool transient) {  // 86d22f
  if (transient && Sprite_ReturnIfPhasingOut(k))
    return false;
  if (sort_sprites_setting == 0 && player_is_indoors != 0)
    sprite_obj_prio[k] = 0x30;
  if (byte_7E0FC6 >= 3)
    return false;
  if (sprite_delay_aux2[k] != 0)
    Oam_AllocateFromRegionC(12);
  if (sprite_E[k] != 0)
    return true;
  uint8 j = sprite_type[k];
  assert(j >= 0xd8 && j < 0xd8 + 19);
  uint8 a = kAbsorbable_Tab2[j - 0xd8];
  if (a != 0) {
    Sprite_DrawNumberedAbsorbable(k, a);
    return false;
  }
  uint8 t = kAbsorbable_Tab1[j - 0xd8];
  if (t == 0) {
    SpriteDraw_SingleSmall(k);
    return false;
  }
  if (t == 2) {
    if (sprite_type[k] == 0xe6) {
      if (sprite_subtype[k] == 1)
        goto draw_key;
      sprite_graphics[k] = 1;
    }
    SpriteDraw_SingleLarge(k);
    return false;
  }
draw_key:
  Sprite_DrawThinAndTall(k);
  return false;
}

void Sprite_DrawNumberedAbsorbable(int k, int a) {  // 86d2fa
  a = (a - 1) * 3;
  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  OamEnt *oam = GetOamCurPtr();
  int n = (sprite_head_dir[k] < 1) ? 2 : 1;
  do {
    int j = n + a;
    uint16 x = info.x + kNumberedAbsorbable_X[j];
    uint16 y = info.y + kNumberedAbsorbable_Y[j];
    oam->x = x;
    oam->y = ClampYForOam(y);
    oam->charnum = kNumberedAbsorbable_Char[j];
    oam->flags = info.flags;
    bytewise_extended_oam[oam - oam_buf] = kNumberedAbsorbable_Ext[j] | (x >> 8 & 1);
  } while (oam++, --n >= 0);
  SpriteDraw_Shadow(k, &info);
}

void Sprite_BounceOffWall(int k) {  // 86d9c0
  if (sprite_wallcoll[k] & 3)
    sprite_x_vel[k] = -sprite_x_vel[k];
  if (sprite_wallcoll[k] & 12)
    sprite_y_vel[k] = -sprite_y_vel[k];
}

void Sprite_InvertSpeed_XY(int k) {  // 86d9d5
  sprite_x_vel[k] = -sprite_x_vel[k];
  sprite_y_vel[k] = -sprite_y_vel[k];
}

bool Sprite_ReturnIfInactive(int k) {  // 86d9ec
  return (sprite_state[k] != 9 || flag_unk1 || submodule_index || !(sprite_defl_bits[k] & 0x80) && sprite_pause[k]);
}

bool Sprite_ReturnIfPaused(int k) {  // 86d9f3
  return (flag_unk1 || submodule_index || !(sprite_defl_bits[k] & 0x80) && sprite_pause[k]);
}

void SpriteDraw_SingleLarge(int k) {  // 86dc10
  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  Sprite_PrepAndDrawSingleLargeNoPrep(k, &info);
}

void Sprite_PrepAndDrawSingleLargeNoPrep(int k, PrepOamCoordsRet *info) {  // 86dc13
  OamEnt *oam = GetOamCurPtr();
  oam->x = info->x;
  if ((uint16)(info->y + 0x10) < 0x100) {
    oam->y = info->y;
    oam->charnum = kSprite_PrepAndDrawSingleLarge_Tab2[kSprite_PrepAndDrawSingleLarge_Tab1[sprite_type[k]] + sprite_graphics[k]];
    oam->flags = info->flags;
  }
  bytewise_extended_oam[oam - oam_buf] = 2 | ((info->x >= 256) ? 1: 0);
  if (sprite_flags3[k] & 0x10)
    SpriteDraw_Shadow(k, info);
}

void SpriteDraw_Shadow_custom(int k, PrepOamCoordsRet *info, uint8 a) {  // 86dc5c
  uint16 y = Sprite_GetY(k) + a;
  info->y = y;
  if (sprite_pause[k] || sprite_state[k] == 10 && sprite_unk3[k] == 3)
    return;
  y -= BG2VOFS_copy2;
  info->y = y;
  if ((uint16)(y + 0x10) >= 0x100)
    return;
  OamEnt *oam = GetOamCurPtr() + (sprite_flags2[k] & 0x1f);
  oam->x = info->x;
  if (sprite_flags3[k] & 0x20) {
    oam->y = y + 1;
    oam->charnum = 0x38;
    oam->flags = (info->flags & 0x30) | 8;
    bytewise_extended_oam[oam - oam_buf] = (info->x >> 8 & 1);
  } else {
    oam->y = y;
    oam->charnum = 0x6c;
    oam->flags = (info->flags & 0x30) | 8;
    bytewise_extended_oam[oam - oam_buf] = (info->x >> 8 & 1) | 2;
  }
}

void SpriteDraw_Shadow(int k, PrepOamCoordsRet *oam) {  // 86dc64
  SpriteDraw_Shadow_custom(k, oam, 10);
}

void SpriteDraw_SingleSmall(int k) {  // 86dcef
  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  OamEnt *oam = GetOamCurPtr();
  oam->x = info.x;
  if ((uint16)(info.y + 0x10) < 0x100) {
    oam->y = info.y;
    oam->charnum = kSprite_PrepAndDrawSingleLarge_Tab2[kSprite_PrepAndDrawSingleLarge_Tab1[sprite_type[k]] + sprite_graphics[k]];
    oam->flags = info.flags;
  }
  bytewise_extended_oam[oam - oam_buf] = 0 | (info.x >= 256);
  if (sprite_flags3[k] & 0x10)
    SpriteDraw_Shadow_custom(k, &info, 2);
}

void Sprite_DrawThinAndTall(int k) {  // 86dd40
  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  OamEnt *oam = GetOamCurPtr();
  oam[1].x = oam[0].x = info.x;
  bytewise_extended_oam[oam - oam_buf + 1] = bytewise_extended_oam[oam - oam_buf] = (info.x >= 256);
  oam[0].y = ClampYForOam(info.y);
  oam[1].y = ClampYForOam(info.y + 8);
  uint8 a = kSprite_PrepAndDrawSingleLarge_Tab2[kSprite_PrepAndDrawSingleLarge_Tab1[sprite_type[k]] + sprite_graphics[k]];
  oam[0].charnum = a;
  oam[1].charnum = a + 0x10;
  oam[0].flags = oam[1].flags = info.flags;
  if (sprite_flags3[k] & 0x10)
    SpriteDraw_Shadow(k, &info);
}

void SpriteModule_Carried(int k) {  // 86de83


  static const uint8 kSpriteHeld_ZForFrame[6] = {3, 2, 1, 3, 2, 1};
  static const int8 kSpriteHeld_X[16] = {0, 0, 0, 0, 0, 0, 0, 0, -13, -10, -5, 0, 13, 10, 5, 0};
  static const uint8 kSpriteHeld_Z[16] = {13, 14, 15, 16, 0, 10, 22, 16, 8, 11, 14, 16, 8, 11, 14, 16};
  sprite_room[k] = overworld_area_index;
  if (sprite_unk3[k] != 3) {
    if (sprite_delay_main[k] == 0) {
      sprite_delay_main[k] = (sprite_C[k] == 6) ? 8 : 4;
      sprite_unk3[k]++;
    }
  } else {
    sprite_flags3[k] &= ~0x10;
  }

  uint8 t = sprite_delay_aux4[k] - 1;
  uint8 r0 = t < 63 && (t & 2);
  int j = link_direction_facing * 2 + sprite_unk3[k];

  int t0 = (uint8)link_x_coord + (uint8)kSpriteHeld_X[j];
  int t1 = (uint8)t0 + (t0 >> 8 & 1) + r0;
  int t2 = HIBYTE(link_x_coord) + (t1 >> 8 & 1) + (t0 >> 8 & 1) + (uint8)(kSpriteHeld_X[j]>>8);
  sprite_x_lo[k] = t1;
  sprite_x_hi[k] = t2;

 // Sprite_SetX(k, link_x_coord + kSpriteHeld_X[j] + r0);
  sprite_z[k] = kSpriteHeld_Z[j];
  int an = link_animation_steps < 6 ? link_animation_steps : 0;
  uint16 z = link_z_coord + 1 + kSpriteHeld_ZForFrame[an];
  Sprite_SetY(k, link_y_coord + 8 - z);
  sprite_floor[k] = link_is_on_lower_level & 1;
  CarriedSprite_CheckForThrow(k);
  Sprite_Get16BitCoords(k);
  if (sprite_unk4[k] != 11) {
    SpriteActive_Main(k);
    if (sprite_delay_aux4[k] == 1) {
      sprite_state[k] = 9;
      sprite_B[k] = 0;
      sprite_delay_aux4[k] = 96;
      sprite_z_vel[k] = 32;
      sprite_flags3[k] |= 0x10;
      link_picking_throw_state = 2;
    }
  } else {
    SpriteStunned_Main_Func1(k);
  }
}

void CarriedSprite_CheckForThrow(int k) {  // 86df6d
  static const int8 kSpriteHeld_Throw_Xvel[4] = {0, 0, -62, 63};
  static const int8 kSpriteHeld_Throw_Yvel[4] = {-62, 63, 0, 0};
  static const uint8 kSpriteHeld_Throw_Zvel[4] = {4, 4, 4, 4};

  if (main_module_index == 14)
    return;

  if (player_near_pit_state != 2) {
    uint8 t = (link_auxiliary_state & 1) | link_is_in_deep_water | link_is_bunny_mirror |
              link_pose_for_item | (link_disable_sprite_damage ? 0 : link_incapacitated_timer);
    if (!t) {
      if (sprite_unk3[k] != 3 || !((filtered_joypad_H | filtered_joypad_L) & 0x80))
        return;
      filtered_joypad_L &= 0x7f;
    }
  }

  SpriteSfx_QueueSfx3WithPan(k, 0x13);
  link_picking_throw_state = 2;
  sprite_state[k] = sprite_unk4[k];
  sprite_z_vel[k] = 0;
  sprite_unk3[k] = 0;
  sprite_flags3[k] = sprite_flags3[k] & ~0x10 | kSpriteInit_Flags3[sprite_type[k]] & 0x10;
  int j = link_direction_facing >> 1;
  sprite_x_vel[k] = kSpriteHeld_Throw_Xvel[j];
  sprite_y_vel[k] = kSpriteHeld_Throw_Yvel[j];
  sprite_z_vel[k] = kSpriteHeld_Throw_Zvel[j];
  sprite_delay_aux4[k] = 0;
}

void SpriteModule_Stunned(int k) {  // 86dffa
  SpriteStunned_MainEx(k, false);
}

void ThrownSprite_TileAndSpriteInteraction(int k) {  // 86e02a
  SpriteStunned_MainEx(k, true);
}

void Sprite_Func8(int k) {  // 86e0ab
  sprite_state[k] = 1;
  sprite_delay_main[k] = 0x1f;
  sound_effect_1 = 0;
  SpriteSfx_QueueSfx2WithPan(k, 0x20);
}

void Sprite_Func22(int k) {  // 86e0f6
  sound_effect_1 = Sprite_CalculateSfxPan(k) | 0x28;
  sprite_state[k] = 3;
  sprite_delay_main[k] = 15;
  sprite_ai_state[k] = 0;
  GetRandomNumber(); // wtf
  sprite_flags2[k] = 3;
}

void ThrowableScenery_InteractWithSpritesAndTiles(int k) {  // 86e164
  Sprite_MoveXY(k);
  if (!sprite_E[k])
    Sprite_CheckTileCollision(k);
  ThrownSprite_TileAndSpriteInteraction(k);
}

void ThrownSprite_CheckDamageToSprites(int k) {  // 86e172
  if (sprite_delay_aux4[k] || !(sprite_x_vel[k] | sprite_y_vel[k]))
    return;
  for (int i = 15; i >= 0; i--) {
    if (i != cur_object_index && sprite_type[k] != 0xd2 && sprite_state[i] >= 9 &&
      ((i ^ frame_counter) & 3 | sprite_ignore_projectile[i] | sprite_hit_timer[i]) == 0 && sprite_floor[k] == sprite_floor[i])
      ThrownSprite_CheckDamageToSingleSprite(k, i);
  }
}

void ThrownSprite_CheckDamageToSingleSprite(int k, int j) {  // 86e1b2
  SpriteHitBox hb;
  hb.r0_xlo = sprite_x_lo[k];
  hb.r8_xhi = sprite_x_hi[k];
  hb.r2 = 15;
  int t = sprite_y_lo[k] - sprite_z[k];
  int u = (t & 0xff) + 8;
  hb.r1_ylo = u;
  hb.r9_yhi = sprite_y_hi[k] + (u >> 8) - (t < 0);
  hb.r3 = 8;
  Sprite_SetupHitBox(j, &hb);
  if (!CheckIfHitBoxesOverlap(&hb))
    return;
  if (sprite_type[j] == 0x3f) {
    Sprite_PlaceWeaponTink(k);
  } else {
    uint8 a = (sprite_type[k] == 0xec && sprite_C[k] == 2 && !player_is_indoors) ? 1 : 3;
    Ancilla_CheckDamageToSprite_preset(j, a);

    sprite_x_recoil[j] = sprite_x_vel[k] * 2;
    sprite_y_recoil[j] = sprite_y_vel[k] * 2;
    sprite_delay_aux4[k] = 16;
  }
  Sprite_ApplyRicochet(k);
}

void Sprite_ApplyRicochet(int k) {  // 86e229
  Sprite_InvertSpeed_XY(k);
  Sprite_HalveSpeed_XY(k);
  ThrowableScenery_TransmuteIfValid(k);
}

void ThrowableScenery_TransmuteIfValid(int k) {  // 86e22f
  if (sprite_type[k] != 0xec)
    return;
  repulsespark_timer = 0;
  ThrowableScenery_TransmuteToDebris(k);
}

void ThrowableScenery_TransmuteToDebris(int k) {  // 86e239
  uint8 a = sprite_graphics[k];
  if (a != 0) {
    BYTE(dung_secrets_unk1) = a;
    Sprite_SpawnSecret(k);
    BYTE(dung_secrets_unk1) = 0;
  }
  a = player_is_indoors ? 0 : sprite_C[k];
  sound_effect_1 = 0;
  SpriteSfx_QueueSfx2WithPan(k, kSprite_Func21_Sfx[a]);
  Sprite_ScheduleForBreakage(k);
}

void Sprite_ScheduleForBreakage(int k) {  // 86e25a
  sprite_delay_main[k] = 31;
  sprite_state[k] = 6;
  sprite_flags2[k] += 4;
}

void Sprite_HalveSpeed_XY(int k) {  // 86e26e
  sprite_x_vel[k] = (int8)sprite_x_vel[k] >> 1;
  sprite_y_vel[k] = (int8)sprite_y_vel[k] >> 1;
}

void Sprite_SpawnLeapingFish(int k) {  // 86e286
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0xd2, &info);
  if (j < 0)
    return;
  Sprite_SetSpawnedCoordinates(j, &info);
  sprite_ai_state[j] = 2;
  sprite_delay_main[j] = 48;
  if (sprite_type[k] == 0xd2)
    sprite_A[j] = 0xd2;
}

void SpriteStunned_Main_Func1(int k) {  // 86e2ba
  SpriteActive_Main(k);
  if (sprite_unk5[k]) {
    if (sprite_delay_main[k] < 32)
      sprite_oam_flags[k] = sprite_oam_flags[k] & 0xf1 | 4;
    uint8 t = ((k << 4) ^ frame_counter) | submodule_index;
    if (t & kSpriteStunned_Main_Func1_Masks[sprite_delay_main[k] >> 4])
      return;
    uint16 x = kSparkleGarnish_XY[GetRandomNumber() & 3];
    uint16 y = kSparkleGarnish_XY[GetRandomNumber() & 3];
    Sprite_GarnishSpawn_Sparkle(k, x, y);
  } else {
    if ((frame_counter & 1) | submodule_index | flag_unk1)
      return;
    uint8 t = sprite_stunned[k];
    if (t) {
      sprite_stunned[k]--;
      if (t < 0x38) {
        sprite_x_vel[k] = (t & 1) ? -8 : 8;
        Sprite_MoveX(k);
      }
      return;
    }
    sprite_state[k] = 9;
    sprite_x_recoil[k] = 0;
    sprite_y_recoil[k] = 0;
  }
}

void SpriteModule_Poof(int k) {  // 86e393
  static const int8 kSpritePoof_X[16] = {-6, 10, 1, 13, -6, 10, 1, 13, -7, 4, -5, 6, -1, 1, -2, 0};
  static const int8 kSpritePoof_Y[16] = {-6, -4, 10, 9, -6, -4, 10, 9, -8, -10, 4, 3, -1, -2, 0, 1};
  static const uint8 kSpritePoof_Char[16] = {0x9b, 0x9b, 0x9b, 0x9b, 0xb3, 0xb3, 0xb3, 0xb3, 0x8a, 0x8a, 0x8a, 0x8a, 0x8a, 0x8a, 0x8a, 0x8a};
  static const uint8 kSpritePoof_Flags[16] = {0x24, 0xa4, 0x24, 0xa4, 0xe4, 0x64, 0xa4, 0x24, 0x24, 0xe4, 0xe4, 0xe4, 0x24, 0xe4, 0xe4, 0xe4};
  static const uint8 kSpritePoof_Ext[16] = {0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2};
  // Frozen sprite pulverized by hammer
  if (sprite_delay_main[k] == 0) {
    if (sprite_type[k] == 0xd && sprite_head_dir[k] != 0) {
      // buzz blob?
      int bakx = Sprite_GetX(k);
      PrepareEnemyDrop(k, 0xd);
      Sprite_SetX(k, bakx);
      sprite_z_vel[k] = 0;
      sprite_ignore_projectile[k] = 0;
    } else {
      if (sprite_die_action[k] == 0) {
        ForcePrizeDrop(k, 2, 2);
      } else {
        Sprite_DoTheDeath(k);
      }
    }
  } else {
    PrepOamCoordsRet info;
    if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
      return;
    OamEnt *oam = GetOamCurPtr();
    int j = ((sprite_delay_main[k] >> 1) & ~3) + 3;
    for (int i = 3; i >= 0; i--, j--, oam++) {
      oam->x = kSpritePoof_X[j] + BYTE(dungmap_var7);
      oam->y = kSpritePoof_Y[j] + HIBYTE(dungmap_var7);
      oam->charnum = kSpritePoof_Char[j];
      oam->flags = kSpritePoof_Flags[j];
      bytewise_extended_oam[oam - oam_buf] = kSpritePoof_Ext[j];
    }
    Sprite_CorrectOamEntries(k, 3, 0xff);
  }
}

void Sprite_PrepOamCoord(int k, PrepOamCoordsRet *ret) {  // 86e416
  Sprite_PrepOamCoordOrDoubleRet(k, ret);
}

bool Sprite_PrepOamCoordOrDoubleRet(int k, PrepOamCoordsRet *ret) {  // 86e41e
  sprite_pause[k] = 0;
  uint16 x = cur_sprite_x - BG2HOFS_copy2;
  uint16 y = cur_sprite_y - BG2VOFS_copy2;
  bool out_of_bounds = false;
  R0 = x;
  R2 = y - sprite_z[k];
  ret->flags = sprite_oam_flags[k] ^ sprite_obj_prio[k];
  ret->r4 = 0;
  if ((uint16)(x + 0x40) >= 0x170 || (uint16)(y + 0x40) >= 0x170 && !(sprite_flags4[k] & 0x20)) {
    sprite_pause[k]++;
    if (!(sprite_defl_bits[k] & 0x80))
      Sprite_KillSelf(k);
    out_of_bounds = true;
  }
  ret->x = R0;
  ret->y = R2;
  BYTE(dungmap_var7) = ret->x;
  HIBYTE(dungmap_var7) = ret->y;
  return out_of_bounds;
}

void Sprite_CheckTileCollision2(int k) {  // 86e4ab
  sprite_wallcoll[k] = 0;
  if (sign8(sprite_flags4[k]) || !dung_hdr_collision) {
    Sprite_CheckTileCollisionSingleLayer(k);
    return;
  }
  byte_7E0FB6 = sprite_floor[k];
  sprite_floor[k] = 1;
  Sprite_CheckTileCollisionSingleLayer(k);
  if (dung_hdr_collision == 4) {
    sprite_floor[k] = byte_7E0FB6;
    return;
  }
  sprite_floor[k] = 0;
  Sprite_CheckTileCollisionSingleLayer(k);
  byte_7FFABC[k] = sprite_tiletype;
}

void Sprite_CheckTileCollisionSingleLayer(int k) {  // 86e4db
  if (sprite_flags2[k] & 0x20) {
    if (Sprite_CheckTileProperty(k, 0x6a))
      sprite_wallcoll[k]++;
    return;
  }

  if (sign8(sprite_flags4[k]) || dung_hdr_collision == 0) {
    if (sprite_y_vel[k])
      Sprite_CheckForTileInDirection_vertical(k, sign8(sprite_y_vel[k]) ? 0 : 1);
    if (sprite_x_vel[k])
      Sprite_CheckForTileInDirection_horizontal(k, sign8(sprite_x_vel[k]) ? 2 : 3);
  } else {
    Sprite_CheckForTileInDirection_vertical(k, 1);
    Sprite_CheckForTileInDirection_vertical(k, 0);
    Sprite_CheckForTileInDirection_horizontal(k, 3);
    Sprite_CheckForTileInDirection_horizontal(k, 2);
  }

  if (sign8(sprite_flags5[k]) || sprite_z[k])
    return;

  Sprite_CheckTileProperty(k, 0x68);
  sprite_I[k] = sprite_tiletype;
  if (sprite_tiletype == 0x1c) {
    if (sort_sprites_setting && sprite_state[k] == 11)
      sprite_floor[k] = 1;
  } else if (sprite_tiletype == 0x20) {
    if (sprite_flags[k] & 1) {
      if (!player_is_indoors) {
        Sprite_Func8(k);
      } else {
        sprite_state[k] = 5;
        if (sprite_type[k] == 0x13 || sprite_type[k] == 0x26) {
          sprite_oam_flags[k] &= ~1;
          sprite_delay_main[k] = 63;
        } else {
          sprite_delay_main[k] = 95;
        }
      }
    }
  } else if (sprite_tiletype == 0xc) {
    if (byte_7FFABC[k] == 0x1c) {
      SpriteFall_AdjustPosition(k);
      sprite_wallcoll[k] |= 0x20;
    }
  } else if (sprite_tiletype >= 0x68 && sprite_tiletype < 0x6c) {
    Sprite_ApplyConveyor(k, sprite_tiletype);
  } else if (sprite_tiletype == 8) {
    if (dung_hdr_collision == 4)
      Sprite_ApplyConveyor(k, 0x6a);
  }
}

void Sprite_CheckForTileInDirection_horizontal(int k, int yy) {  // 86e5b8
  if (!Sprite_CheckTileInDirection(k, yy))
    return;
  sprite_wallcoll[k] |= kSprite_Func7_Tab[yy];
  if ((sprite_subtype[k] & 7) < 5) {
    int8 n = sprite_F[k] ? 3 : 1;
    SpriteAddXY(k, (yy & 1) ? -n : n, 0);
  }
}

void Sprite_CheckForTileInDirection_vertical(int k, int yy) {  // 86e5ee
  if (!Sprite_CheckTileInDirection(k, yy))
    return;
  sprite_wallcoll[k] |= kSprite_Func7_Tab[yy];
  if ((sprite_subtype[k] & 7) < 5) {
    int8 n = sprite_F[k] ? 3 : 1;
    SpriteAddXY(k, 0, (yy & 1) ? -n : n);
  }
}

void SpriteFall_AdjustPosition(int k) {  // 86e624
  SpriteAddXY(k, dung_floor_x_vel, dung_floor_y_vel);
}

bool Sprite_CheckTileInDirection(int k, int yy) {  // 86e72f
  uint8 t = (sprite_flags[k] & 0xf0);
  yy = 2 * ((t >> 2) + yy);
  return Sprite_CheckTileProperty(k, yy);
}

bool Sprite_CheckTileProperty(int k, int j) {  // 86e73c
  uint16 x, y;
  bool in_bounds;
  j >>= 1;

  if (player_is_indoors) {
    x = (cur_sprite_x + 8 & 0x1ff) + kSprite_Func5_X[j] - 8;
    y = (cur_sprite_y + 8 & 0x1ff) + kSprite_Func5_Y[j] - 8;
    in_bounds = (x < 0x200) && (y < 0x200);
  } else {
    x = cur_sprite_x + kSprite_Func5_X[j];
    y = cur_sprite_y + kSprite_Func5_Y[j];
    in_bounds = (uint16)(x - sprcoll_x_base) < sprcoll_x_size &&
                (uint16)(y - sprcoll_y_base) < sprcoll_y_size;
  }
  if (!in_bounds) {
    if (sprite_flags2[k] & 0x40) {
      sprite_state[k] = 0;
      return false;
    } else {
      return true;
    }
  }

  int b = Sprite_GetTileAttribute(k, &x, y);

  if (sprite_defl_bits[k] & 8) {
    uint8 a = kSprite_SimplifiedTileAttr[b];
    if (a == 4) {
      if (!player_is_indoors)
        sprite_E[k] = 4;
    } else if (a >= 1) {
      return (sprite_tiletype >= 0x10 && sprite_tiletype < 0x14) ? Entity_CheckSlopedTileCollision(x, y) : true;
    }
    return false;
  }

  if (sprite_flags5[k] & 0x40) {
    uint8 type = sprite_type[k];
    if ((type == 0xd2 || type == 0x8a) && b == 9)
      return false;
    if (type == 0x94 && !sprite_E[k] || type == 0xe3 || type == 0x8c || type == 0x9a || type == 0x81)
      return (b != 8) && (b != 9);
  }

  if (kSprite_Func5_Tab3[b] == 0)
    return false;

  if (sprite_tiletype >= 0x10 && sprite_tiletype < 0x14)
    return Entity_CheckSlopedTileCollision(x, y);

  if (sprite_tiletype == 0x44) {
    if (sprite_F[k] && !sign8(sprite_give_damage[k])) {
      Ancilla_CheckDamageToSprite_preset(k, 4);
      if (sprite_hit_timer[k]) {
        sprite_hit_timer[k] = 153;
        sprite_F[k] = 0;
      }
    }
  } else if (sprite_tiletype == 0x20) {
    return !(sprite_flags[k] & 1) || !sprite_F[k];
  }
  return true;
}

uint8 GetTileAttribute(uint8 floor, uint16 *x, uint16 y) {  // 86e87b
  uint8 tiletype;
  if (player_is_indoors) {
    int t = (floor >= 1) ? 0x1000 : 0;
    t += (*x & 0x1f8) >> 3;
    t += (y & 0x1f8) << 3;
    tiletype = dung_bg2_attr_table[t];
  } else {
    tiletype = Overworld_GetTileAttributeAtLocation(*x >>= 3, y);
  }
  sprite_tiletype = tiletype;
  return tiletype;
}

uint8 Sprite_GetTileAttribute(int k, uint16 *x, uint16 y) {  // 86e883
  return GetTileAttribute(sprite_floor[k], x, y);
}

bool Entity_CheckSlopedTileCollision(uint16 x, uint16 y) {  // 86e8fe
  uint8 a = y & 7;
  uint8 r6 = sprite_tiletype - 0x10;
  uint8 b = kSlopedTile[r6 * 8 + (x & 7)];
  return (r6 < 2) ? (b >= a) : (a >= b);
}

void Sprite_MoveXY(int k) {  // 86e92c
  Sprite_MoveX(k);
  Sprite_MoveY(k);
}

void Sprite_MoveX(int k) {  // 86e932
  if (sprite_x_vel[k] != 0) {
    uint32 t = sprite_x_subpixel[k] + (sprite_x_lo[k] << 8) + (sprite_x_hi[k] << 16) + ((int8)sprite_x_vel[k] << 4);
    sprite_x_subpixel[k] = t, sprite_x_lo[k] = t >> 8, sprite_x_hi[k] = t >> 16;
  }
}

void Sprite_MoveY(int k) {  // 86e93e
  if (sprite_y_vel[k] != 0) {
    uint32 t = sprite_y_subpixel[k] + (sprite_y_lo[k] << 8) + (sprite_y_hi[k] << 16) + ((int8)sprite_y_vel[k] << 4);
    sprite_y_subpixel[k] = t, sprite_y_lo[k] = t >> 8, sprite_y_hi[k] = t >> 16;
  }
}

void Sprite_MoveZ(int k) {  // 86e96c
  uint16 z = (sprite_z[k] << 8 | sprite_z_subpos[k]) + ((int8)sprite_z_vel[k] << 4);
  sprite_z_subpos[k] = z;
  sprite_z[k] = z >> 8;
}

ProjectSpeedRet Sprite_ProjectSpeedTowardsLink(int k, uint8 vel) {  // 86e991
  if (vel == 0) {
    ProjectSpeedRet rv = { 0, 0, 0, 0 };
    return rv;
  }
  PairU8 below = Sprite_IsBelowLink(k);
  uint8 r12 = sign8(below.b) ? -below.b : below.b;

  PairU8 right = Sprite_IsRightOfLink(k);
  uint8 r13 = sign8(right.b) ? -right.b : right.b;
  uint8 t;
  bool swapped = false;
  if (r13 < r12) {
    swapped = true;
    t = r12, r12 = r13, r13 = t;
  }
  uint8 xvel = vel, yvel = 0;
  t = 0;
  do {
    t += r12;
    if (t >= r13)
      t -= r13, yvel++;
  } while (--vel);
  if (swapped)
    t = xvel, xvel = yvel, yvel = t;
  ProjectSpeedRet rv = {
    (uint8)(right.a ? -xvel : xvel),
    (uint8)(below.a ? -yvel : yvel),
    right.b,
    below.b

  };
  return rv;
}

void Sprite_ApplySpeedTowardsLink(int k, uint8 vel) {  // 86ea04
  ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLink(k, vel);
  sprite_x_vel[k] = pt.x;
  sprite_y_vel[k] = pt.y;
}

ProjectSpeedRet Sprite_ProjectSpeedTowardsLocation(int k, uint16 x, uint16 y, uint8 vel) {  // 86ea2d
  if (vel == 0) {
    ProjectSpeedRet rv = { 0, 0, 0, 0 };
    return rv;
  }
  PairU8 below = Sprite_IsBelowLocation(k, y);
  uint8 r12 = sign8(below.b) ? -below.b : below.b;

  PairU8 right = Sprite_IsRightOfLocation(k, x);
  uint8 r13 = sign8(right.b) ? -right.b : right.b;
  uint8 t;
  bool swapped = false;
  if (r13 < r12) {
    swapped = true;
    t = r12, r12 = r13, r13 = t;
  }
  uint8 xvel = vel, yvel = 0;
  t = 0;
  do {
    t += r12;
    if (t >= r13)
      t -= r13, yvel++;
  } while (--vel);
  if (swapped)
    t = xvel, xvel = yvel, yvel = t;
  ProjectSpeedRet rv = {
    (uint8)(right.a ? -xvel : xvel),
    (uint8)(below.a ? -yvel : yvel),
    right.b,
    below.b
  };
  return rv;
}

uint8 Sprite_DirectionToFaceLink(int k, PointU8 *coords_out) {  // 86eaa4
  PairU8 below = Sprite_IsBelowLink(k);
  PairU8 right = Sprite_IsRightOfLink(k);
  uint8 ym = sign8(below.b) ? -below.b : below.b;
  tmp_counter = ym;
  uint8 xm = sign8(right.b) ? -right.b : right.b;
  if (coords_out)
    coords_out->x = right.b, coords_out->y = below.b;
  return (xm >= ym) ? right.a : below.a + 2;
}

PairU8 Sprite_IsRightOfLink(int k) {  // 86ead1
  uint16 x = link_x_coord - Sprite_GetX(k);
  PairU8 rv = { (uint8)(sign16(x) ? 1 : 0), (uint8)x };
  return rv;
}

PairU8 Sprite_IsBelowLink(int k) {  // 86eae8
  int t = BYTE(link_y_coord) + 8;
  int u = (t & 0xff) + sprite_z[k];
  int v = (u & 0xff) - sprite_y_lo[k];
  int w = HIBYTE(link_y_coord) - sprite_y_hi[k] - (v < 0);
  uint8 y = (w & 0xff) + (t >> 8) + (u >> 8);
  PairU8 rv = { (uint8)(sign8(y) ? 1 : 0), (uint8)v };
  return rv;
}

PairU8 Sprite_IsRightOfLocation(int k, uint16 x) {  // 86eb0a
  uint16 xv = x - Sprite_GetX(k);
  PairU8 rv = { (uint8)(sign16(xv) ? 1 : 0), (uint8)xv };
  return rv;
}

PairU8 Sprite_IsBelowLocation(int k, uint16 y) {  // 86eb1d
  uint16 yv = y - Sprite_GetY(k);
  PairU8 rv = { (uint8)(sign16(yv) ? 1 : 0), (uint8)yv };
  return rv;
}

uint8 Sprite_DirectionToFaceLocation(int k, uint16 x, uint16 y) {  // 86eb30
  PairU8 below = Sprite_IsBelowLocation(k, y);
  PairU8 right = Sprite_IsRightOfLocation(k, x);
  uint8 ym = sign8(below.b) ? -below.b : below.b;
  tmp_counter = ym;
  uint8 xm = sign8(right.b) ? -right.b : right.b;
  return (xm >= ym) ? right.a : below.a + 2;
}

void Guard_ParrySwordAttacks(int k) {  // 86eb5e
  if (link_is_on_lower_level != sprite_floor[k] || link_incapacitated_timer | link_auxiliary_state || sign8(sprite_hit_timer[k]))
    return;
  SpriteHitBox hb;
  Sprite_DoHitBoxesFast(k, &hb);
  if (link_position_mode & 0x10 || player_oam_y_offset == 0x80) {
    Sprite_AttemptDamageToLinkWithCollisionCheck(k);
    return;
  }
  Player_SetupActionHitBox(&hb);
  if (sign8(button_b_frames) || !CheckIfHitBoxesOverlap(&hb)) {
    Sprite_SetupHitBox(k, &hb);
    if (!CheckIfHitBoxesOverlap(&hb))
      Sprite_AttemptDamageToLinkWithCollisionCheck(k);
    else
      Sprite_AttemptZapDamage(k);
    return;
  }
  if (sprite_type[k] != 0x6a)
    sprite_F[k] = kSprite_Func1_Tab[GetRandomNumber() & 7];
  link_incapacitated_timer = kSprite_Func1_Tab2[GetRandomNumber() & 7];
  ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLink(k, sign8(button_b_frames - 9) ? 32 : 24);
  sprite_x_recoil[k] = -pt.x;
  sprite_y_recoil[k] = -pt.y;
  Sprite_ApplyRecoilToLink(k, sign8(button_b_frames - 9) ? 8 : 16);
  Link_PlaceWeaponTink();
  set_when_damaging_enemies = 0x90;
}

void Sprite_AttemptZapDamage(int k) {  // 86ec02
  uint8 a = sprite_type[k];
  if ((a == 0x7a || a == 0xd && (a = link_sword_type) < 4 || (a == 0x24 || a == 0x23) && sprite_delay_main[k] != 0) && sprite_state[k] == 9) {
    if (!countdown_for_blink) {
      sprite_delay_aux1[k] = 64;
      link_electrocute_on_touch = 64;
      Sprite_AttemptDamageToLinkPlusRecoil(k);
    }
  } else {
    ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLink(k, sign8(button_b_frames - 9) ? 0x50 : 0x40);
    sprite_x_recoil[k] = -pt.x;
    sprite_y_recoil[k] = -pt.y;
    Sprite_CalculateSwordDamage(k);
  }
}

void Ancilla_CheckDamageToSprite_preset(int k, int a) {  // 86ece0
  if (a == 15 && sprite_z[k] != 0)
    return;

  if (a != 0 && a != 7) {
    Sprite_Func15(k, a);
    return;
  }
  Sprite_Func15(k, a);
  if (sprite_give_damage[k] || repulsespark_timer)
    return;
  // Called when hitting enemy which is frozen
  repulsespark_timer = 5;
  int j = byte_7E0FB6;
  repulsespark_x_lo = ancilla_x_lo[j] + 4;
  repulsespark_y_lo = ancilla_y_lo[j];
  repulsespark_floor_status = link_is_on_lower_level;
  sound_effect_1 = 0;
  SpriteSfx_QueueSfx2WithPan(k, 5);
}

void Sprite_Func15(int k, int a) {  // 86ed25
  damage_type_determiner = a;
  Sprite_ApplyCalculatedDamage(k, a == 8 ? 0x35 : 0x20);
}

void Sprite_CalculateSwordDamage(int k) {  // 86ed3f
  if (sprite_flags3[k] & 0x40)
    return;
  sprite_unk1[k] = link_is_running;
  uint8 a = link_sword_type - 1;
  if (!link_is_running)
    a |= sign8(button_b_frames) ? 4 : sign8(button_b_frames - 9) ? 0 : 8;
  damage_type_determiner = kSprite_Func14_Damage[a];
  if (link_item_in_hand & 10)
    damage_type_determiner = 3;
  link_sword_delay_timer = 4;
  set_when_damaging_enemies = 16;
  Sprite_ApplyCalculatedDamage(k, 0x9d);
}

void Sprite_ApplyCalculatedDamage(int k, int a) {  // 86ed89
  if ((sprite_flags3[k] & 0x40) || sprite_type[k] >= 0xD8)
    return;
  uint8 dmg = kEnemyDamages[damage_type_determiner * 8 | enemy_damage_data[sprite_type[k] * 16 | damage_type_determiner]];
  AgahnimBalls_DamageAgahnim(k, dmg, a);
}

void AgahnimBalls_DamageAgahnim(int k, uint8 dmg, uint8 r0_hit_timer) {  // 86edc5
  if (dmg == 249) {
    Sprite_Func18(k, 0xe3);
    return;
  }
  if (dmg == 250) {
    Sprite_Func18(k, 0x8f);
    sprite_ai_state[k] = 2;
    sprite_z_vel[k] = 32;
    sprite_oam_flags[k] = 8;
    sprite_F[k] = 0;
    sprite_hit_timer[k] = 0;
    sprite_health[k] = 0;
    sprite_bump_damage[k] = 1;
    sprite_flags5[k] = 1;
    return;
  }
  if (dmg >= sprite_give_damage[k])
    sprite_give_damage[k] = dmg;
  if (dmg == 0) {
    if (damage_type_determiner != 10) {
      if (sprite_flags[k] & 4)
        goto flag4;
      link_sword_delay_timer = 0;
    }
    sprite_hit_timer[k] = 0;
    sprite_give_damage[k] = 0;
    return;
  }
  if (dmg >= 254 && sprite_state[k] == 11) {
    sprite_hit_timer[k] = 0;
    sprite_give_damage[k] = 0;
    return;
  }
  if (sprite_type[k] == 0x9a && sprite_give_damage[k] < 0xf0) {
    sprite_state[k] = 9;
    sprite_ai_state[k] = 4;
    sprite_delay_main[k] = 15;
    SpriteSfx_QueueSfx2WithPan(k, 0x28);
    return;
  }
  if (sprite_type[k] == 0x1b) {
    SpriteSfx_QueueSfx2WithPan(k, 5);
    Sprite_ScheduleForBreakage(k);
    Sprite_PlaceWeaponTink(k);
    return;
  }
  sprite_hit_timer[k] = r0_hit_timer;
  if (sprite_type[k] != 0x92 || sprite_C[k] >= 3) {
    uint8 sfx = sprite_flags[k] & 2 ? 0x21 :
                sprite_flags5[k] & 0x10 ? 0x1c : 8;
    sound_effect_2 = sfx | Sprite_CalculateSfxPan(k);
  }
  uint8 type;
flag4:
  type = sprite_type[k];
  sprite_F[k] = (damage_type_determiner >= 13) ? 0 :
                (type == 9) ? 20 :
                (type == 0x53 || type == 0x18) ? 11 : 15;
}

void Sprite_Func18(int k, uint8 new_type) {  // 86edcb
  sprite_type[k] = new_type;
  SpritePrep_LoadProperties(k);
  Sprite_SpawnPoofGarnish(k);
  sound_effect_2 = 0;
  SpriteSfx_QueueSfx3WithPan(k, 0x32);
  sprite_hit_timer[k] = 0;
  sprite_give_damage[k] = 0;
}

void Sprite_MiniMoldorm_Recoil(int k) {  // 86eec8
  if (sprite_state[k] < 9)
    return;
  tmp_counter = sprite_state[k];

  uint8 dmg = sprite_give_damage[k];
  if (dmg == 253) {
    sprite_give_damage[k] = 0;
    SpriteSfx_QueueSfx3WithPan(k, 9);
    sprite_state[k] = 7;
    sprite_delay_main[k] = 0x70;
    sprite_flags2[k] += 2;
    sprite_give_damage[k] = 0;
    return;
  }

  if (dmg >= 251) {
    sprite_give_damage[k] = 0;
    if (sprite_state[k] == 11)
      return;
    sprite_unk5[k] = (dmg == 254);
    if (sprite_unk5[k]) {
      sprite_defl_bits[k] |= 8;
      sprite_flags5[k] &= ~0x80;
      SpriteSfx_QueueSfx2WithPan(k, 15);
      sprite_z_vel[k] = 24;
      sprite_bump_damage[k] &= ~0x80;
      Sprite_ZeroVelocity_XY(k);
    }
    sprite_state[k] = 11;
    sprite_delay_main[k] = 64;
    static const uint8 kHitTimer24StunValues[5] = {0x20, 0x80, 0, 0, 0xff};
    sprite_stunned[k] = kHitTimer24StunValues[(uint8)(dmg + 5)];
    if (sprite_type[k] == 0x23)
      sprite_type[k] = 0x24;
    return;
  }

  int t = sprite_health[k] - sprite_give_damage[k];
  sprite_health[k] = t;
  sprite_give_damage[k] = 0;
  if (t > 0)
    return;

  if (sprite_die_action[k] == 0) {
    if (sprite_state[k] == 11)
      sprite_die_action[k] = 3;
    if (sprite_unk1[k] != 0) {
      sprite_unk1[k] = 0;
      sprite_flags5[k] = 0;
    }
  }

  uint8 type = sprite_type[k];
  if (type != 0x1b)
    SpriteSfx_QueueSfx3WithPan(k, 9);

  if (type == 0x40)
    save_ow_event_info[BYTE(overworld_screen_index)] |= 0x40;
  else if (type == 0xec) {
    if (sprite_C[k] == 2)
      ThrowableScenery_TransmuteToDebris(k);
    return;
  }

  if (sprite_state[k] == 10) {
    link_state_bits = 0;
    link_picking_throw_state = 0;
  }
  sprite_state[k] = 6;

  if (type == 0xc) {
    Sprite_Func3(k);
  } else if (type == 0x92) {
    Sprite_KillFriends();
    sprite_delay_main[k] = 255;
    goto out_common;
  } else if (type == 0xcb) {
    sprite_ai_state[k] = 128;
    sprite_delay_main[k] = 128;
    sprite_state[k] = 9;
    goto out_common;
  } else if (type == 0xcc || type == 0xcd) {
    sprite_ai_state[k] = 128;
    sprite_delay_main[k] = 96;
    sprite_state[k] = 9;
    goto out_common;
  } else if (type == 0x53) {
    sprite_delay_main[k] = 35;
    sprite_hit_timer[k] = 0;
    goto out_common2;
  } else if (type == 0x54) {
    sprite_ai_state[k] = 5;
    sprite_delay_main[k] = 0xc0;
    sprite_hit_timer[k] = 0xc0;
    goto out_common;
  } else if (type == 0x9) {
    sprite_ai_state[k] = 3;
    sprite_delay_aux4[k] = 160;
    sprite_state[k] = 9;
    goto out_common;
  } else if (type == 0x7a) {
    Sprite_KillFriends();
    sprite_state[k] = 9;
    sprite_ignore_projectile[k] = 9;
    if (is_in_dark_world == 0) {
      sprite_ai_state[k] = 10;
      sprite_delay_main[k] = 255;
      sprite_z_vel[k] = 32;
    } else {
      sprite_delay_main[k] = 255;
      sprite_ai_state[k] = 8;
      sprite_ai_state[1] = 9;
      sprite_ai_state[2] = 9;
      sprite_graphics[1] = 0;
      sprite_graphics[2] = 0;
    }
    goto out_common;
  } else if (type == 0x23 && sprite_C[k] == 0) {
    sprite_ai_state[k] = 2;
    sprite_delay_main[k] = 32;
    sprite_state[k] = 9;
    sprite_hit_timer[k] = 0;
  } else if (type == 0xf) {
    sprite_hit_timer[k] = 0;
    sprite_delay_main[k] = 15;
  } else if (!(sprite_flags[k] & 2)) {
    sprite_delay_main[k] = sprite_hit_timer[k] & 0x80 ? 31 : 15;
    sprite_flags2[k] += 4;
    if (tmp_counter == 11)
      sprite_flags5[k] = 1;
  } else {
    if (type != 0xa2)
      Sprite_KillFriends();
    sprite_state[k] = 4;
    sprite_A[k] = 0;
    sprite_delay_main[k] = 255;
    sprite_hit_timer[k] = 255;
  out_common:
    flag_block_link_menu++;
  out_common2:
    sound_effect_2 = 0;
    SpriteSfx_QueueSfx3WithPan(k, 0x22);
  }
}

void Sprite_Func3(int k) {  // 86efda
  sprite_state[k] = 6;
  sprite_delay_main[k] = 31;
  sprite_flags2[k] = 3;
}

bool Sprite_CheckDamageToLink(int k) {  // 86f145
  if (link_disable_sprite_damage)
    return false;
  return Sprite_CheckDamageToPlayer_1(k);
}

bool Sprite_CheckDamageToPlayer_1(int k) {  // 86f14a
  if ((k ^ frame_counter) & 3 | sprite_hit_timer[k])
    return false;
  return Sprite_CheckDamageToLink_same_layer(k);
}

bool Sprite_CheckDamageToLink_same_layer(int k) {  // 86f154
  if (link_is_on_lower_level != sprite_floor[k])
    return false;
  return Sprite_CheckDamageToLink_ignore_layer(k);
}

bool Sprite_CheckDamageToLink_ignore_layer(int k) {  // 86f15c
  uint8 carry, t;
  if (sprite_flags4[k]) {
    SpriteHitBox hitbox;
    Link_SetupHitBox(&hitbox);
    Sprite_SetupHitBox(k, &hitbox);
    carry = CheckIfHitBoxesOverlap(&hitbox);
  } else {
    carry = Sprite_SetupHitBox00(k);
  }

  if (sign8(sprite_flags2[k]))
    return carry;

  if (!carry || link_auxiliary_state)
    return false;

  if (link_is_bunny_mirror || sign8(link_state_bits) || !(sprite_flags5[k] & 0x20) || !link_shield_type)
    goto if_3;
  sprite_state[k] = 0;

  t = button_b_frames ? kSpriteDamage_Tab2[link_direction_facing >> 1] : link_direction_facing;
  if (t != kSpriteDamage_Tab3[sprite_D[k]]) {
if_3:
    Sprite_AttemptDamageToLinkPlusRecoil(k);
    if (sprite_type[k] == 0xc)
      Sprite_Func3(k);
    return true;
  }
  SpriteSfx_QueueSfx2WithPan(k, 6);
  Sprite_PlaceRupulseSpark_2(k);
  if (sprite_type[k] == 0x95) {
    SpriteSfx_QueueSfx3WithPan(k, 0x26);
    return false;
  } else if (sprite_type[k] == 0x9B) {
    Sprite_Invert_XY_Speeds(k);
    sprite_D[k] ^= 1;
    sprite_ai_state[k]++;
    sprite_state[k] = 9;
    return false;
  } else if (sprite_type[k] == 0x1B) { // arrow
    Sprite_ScheduleForBreakage(k);
    return false;  // unk ret val
  } else if (sprite_type[k] == 0xc) {
    Sprite_Func3(k);
    return true;
  } else {
    return false;  // unk ret val
  }
}

bool Sprite_SetupHitBox00(int k) {  // 86f1f6
  return (uint16)(link_x_coord - cur_sprite_x + 11) < 23 &&
         (uint16)(link_y_coord - cur_sprite_y + sprite_z[k] + 16) < 24;
}

bool Sprite_ReturnIfLifted(int k) {  // 86f228
  if (submodule_index | button_b_frames | flag_unk1 || sprite_floor[k] != link_is_on_lower_level)
    return false;
  for (int j = 15; j >= 0; j--)
    if (sprite_state[j] == 10)
      return false;
  if (sprite_type[k] != 0xb && sprite_type[k] != 0x4a && (sprite_x_vel[k] | sprite_y_vel[k]) != 0)
    return false;
  if (link_is_running)
    return false;
  return Sprite_ReturnIfLiftedPermissive(k);
}

bool Sprite_ReturnIfLiftedPermissive(int k) {  // 86f257
  if (link_is_running)
    return false;
  if ((uint8)(flag_is_sprite_to_pick_up_cached - 1) != cur_object_index) {
    SpriteHitBox hb;
    Link_SetupHitBox_conditional(&hb);
    Sprite_SetupHitBox(k, &hb);
    if (CheckIfHitBoxesOverlap(&hb))
      byte_7E0FB2 = flag_is_sprite_to_pick_up = k + 1;
    return false;
  } else {
    BYTE(filtered_joypad_L) = 0;
    sprite_E[k] = 0;
    SpriteSfx_QueueSfx2WithPan(k, 0x1d);
    sprite_unk4[k] = sprite_state[k];
    sprite_state[k] = 10;
    sprite_delay_main[k] = 16;
    sprite_unk3[k] = 0;
    sprite_I[k] = 0;
    link_direction_facing = kSprite_ReturnIfLifted_Dirs[Sprite_DirectionToFaceLink(k, NULL)];
    return true;
  }
}

uint8 Sprite_CheckDamageFromLink(int k) {  // 86f2b4
  if (sprite_hit_timer[k] & 0x80 || sprite_floor[k] != link_is_on_lower_level || player_oam_y_offset == 0x80)
    return 0;

  SpriteHitBox hb;
  Player_SetupActionHitBox(&hb);
  Sprite_SetupHitBox(k, &hb);
  if (!CheckIfHitBoxesOverlap(&hb))
    return 0;

  set_when_damaging_enemies = 0;
  if (link_position_mode & 0x10)
    return kCheckDamageFromPlayer_Carry | kCheckDamageFromPlayer_Ne;

  if (link_item_in_hand & 10) {
    if (sprite_type[k] >= 0xd6)
      return 0;
    if (sprite_state[k] == 11 && sprite_unk5[k] != 0) {
      sprite_state[k] = 2;
      sprite_delay_main[k] = 32;
      sprite_flags2[k] = (sprite_flags2[k] & 0xe0) | 3;
      SpriteSfx_QueueSfx2WithPan(k, 0x1f);
      return kCheckDamageFromPlayer_Carry | kCheckDamageFromPlayer_Ne;
    }
  }
  uint8 type = sprite_type[k];
  if (type == 0x7b) {
    if (!sign8(button_b_frames - 9))
      return 0;
  } else if (type == 9) {
    if (!sprite_A[k]) {
      Sprite_ApplyRecoilToLink(k, 48);
      set_when_damaging_enemies = 144;
      link_incapacitated_timer = 16;
      SpriteSfx_QueueSfx2WithPan(k, 0x21);
      sprite_delay_aux1[k] = 48;
      sound_effect_2 = Sprite_CalculateSfxPan(k);
      Link_PlaceWeaponTink();
      return kCheckDamageFromPlayer_Carry;
    }
  } else if (type == 0x92) {
    if (sprite_C[k] >= 3)
      goto is_many;
    goto getting_out;
  } else if (type == 0x26 || type == 0x13 || type == 2) {
    bool cond = (type == 0x13 && kSpriteDamage_Tab3[sprite_D[k]] == link_direction_facing) || (type == 2);
    Sprite_AttemptZapDamage(k);
    Sprite_ApplyRecoilToLink(k, 32);
    set_when_damaging_enemies = 16;
    link_incapacitated_timer = 16;
    if (cond) {
      sprite_hit_timer[k] = 0;
      Link_PlaceWeaponTink();
    }
    return 0; // what return value?
  } else if (type == 0xcb || type == 0xcd || type == 0xcc || type == 0xd6 || type == 0xd7 || type == 0xce || type == 0x54) {
is_many:
    Sprite_ApplyRecoilToLink(k, 32);
    set_when_damaging_enemies = 144;
    link_incapacitated_timer = 16;
  }
  if (!(sprite_defl_bits[k] & 4)) {
    Sprite_AttemptZapDamage(k);
    return kCheckDamageFromPlayer_Carry;
  }
getting_out:
  if (!set_when_damaging_enemies) {
    Sprite_ApplyRecoilToLink(k, 4);
    link_incapacitated_timer = 16;
    set_when_damaging_enemies = 16;
  }
  Link_PlaceWeaponTink();
  return kCheckDamageFromPlayer_Carry;
}

void Sprite_AttemptDamageToLinkWithCollisionCheck(int k) {  // 86f3ca
  if ((k ^ frame_counter) & 1)
    return;
  SpriteHitBox hb;
  Sprite_DoHitBoxesFast(k, &hb);
  Link_SetupHitBox_conditional(&hb);
  if (CheckIfHitBoxesOverlap(&hb))
    Sprite_AttemptDamageToLinkPlusRecoil(k);
}

void Sprite_AttemptDamageToLinkPlusRecoil(int k) {  // 86f3db
  if (countdown_for_blink | link_disable_sprite_damage)
    return;
  link_incapacitated_timer = 19;
  Sprite_ApplyRecoilToLink(k, 24);
  link_auxiliary_state = 1;
  link_give_damage = kPlayerDamages[3 * (sprite_bump_damage[k] & 0xf) + link_armor];
  if (sprite_type[k] == 0x61 && sprite_C[k]) {
    link_actual_vel_x = sprite_x_vel[k] * 2;
    link_actual_vel_y = sprite_y_vel[k] * 2;
  }
}

void Player_SetupActionHitBox(SpriteHitBox *hb) {  // 86f5e0
  if (link_is_running) {
    int j = link_direction_facing >> 1;
    int x = link_x_coord + (kPlayerActionBoxRun_XLo[j] | kPlayerActionBoxRun_XHi[j] << 8);
    int y = link_y_coord + (kPlayerActionBoxRun_YLo[j] | kPlayerActionBoxRun_YHi[j] << 8);
    hb->r0_xlo = x;
    hb->r8_xhi = x >> 8;
    hb->r1_ylo = y;
    hb->r9_yhi = y >> 8;
    hb->r2 = hb->r3 = 16;
  } else {
    int t = 0;
    if (!(link_item_in_hand & 10) && !(link_position_mode & 0x10)) {
      if (sign8(button_b_frames)) {
        int x = link_x_coord - 14;
        int y = link_y_coord - 10;
        hb->r0_xlo = x;
        hb->r8_xhi = x >> 8;
        hb->r1_ylo = y;
        hb->r9_yhi = y >> 8;
        hb->r2 = 44;
        hb->r3 = 45;
        return;
      } else if (kPlayer_SetupActionHitBox_Tab4[button_b_frames]) {
        hb->r8_xhi = 0x80;
        return;
      }
      t = link_direction_facing * 8 + button_b_frames + 1;
    }
    int x = link_x_coord + (int8)(kPlayer_SetupActionHitBox_Tab0[t] + player_oam_x_offset);
    int y = link_y_coord + (int8)(kPlayer_SetupActionHitBox_Tab2[t] + player_oam_y_offset);
    hb->r0_xlo = x;
    hb->r8_xhi = x >> 8;
    hb->r1_ylo = y;
    hb->r9_yhi = y >> 8;
    hb->r2 = kPlayer_SetupActionHitBox_Tab1[t];
    hb->r3 = kPlayer_SetupActionHitBox_Tab3[t];
  }
}

void Sprite_DoHitBoxesFast(int k, SpriteHitBox *hb) {  // 86f645
  if (HIBYTE(dungmap_var8) == 0x80) {
    hb->r10_spr_xhi = 0x80;
    return;
  }
  int t;
  t = Sprite_GetX(k) + (int8)HIBYTE(dungmap_var8);
  hb->r4_spr_xlo = t;
  hb->r10_spr_xhi = t >> 8;
  t = Sprite_GetY(k) + (int8)BYTE(dungmap_var8);
  hb->r5_spr_ylo = t;
  hb->r11_spr_yhi = t >> 8;
  hb->r6_spr_xsize = hb->r7_spr_ysize = (sprite_type[k] == 0x6a) ? 16 : 3;
}

void Sprite_ApplyRecoilToLink(int k, uint8 vel) {  // 86f688
  ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLink(k, vel);
  link_actual_vel_x = pt.x;
  link_actual_vel_y = pt.y;
  g_ram[0xc7] = link_actual_vel_z = vel >> 1;
  link_z_coord = 0;
}

void Link_PlaceWeaponTink() {  // 86f69f
  if (repulsespark_timer)
    return;
  repulsespark_timer = 5;
  int t = (uint8)link_x_coord + player_oam_x_offset;
  repulsespark_x_lo = t;
  t = (uint8)link_y_coord + player_oam_y_offset + (t >> 8);  // carry wtf
  repulsespark_y_lo = t;
  repulsespark_floor_status = link_is_on_lower_level;
  sound_effect_1 = Link_CalculateSfxPan() | 5;
}

void Sprite_PlaceWeaponTink(int k) {  // 86f6ca
  if (repulsespark_timer)
    return;
  SpriteSfx_QueueSfx2WithPan(k, 5);
  Sprite_PlaceRupulseSpark_2(k);
}

void Sprite_PlaceRupulseSpark_2(int k) {  // 86f6d5
  uint16 x = Sprite_GetX(k) - BG2HOFS_copy2;
  uint16 y = Sprite_GetY(k) - BG2VOFS_copy2;
  if (x & ~0xff || y & ~0xff)
    return;
  repulsespark_x_lo = sprite_x_lo[k];
  repulsespark_y_lo = sprite_y_lo[k];
  repulsespark_timer = 5;
  repulsespark_floor_status = sprite_floor[k];
}

void Link_SetupHitBox_conditional(SpriteHitBox *hb) {  // 86f705
  if (link_disable_sprite_damage)
    hb->r9_yhi = 0x80;
  else
    Link_SetupHitBox(hb);
}

void Link_SetupHitBox(SpriteHitBox *hb) {  // 86f70a
  hb->r3 = hb->r2 = 8;
  uint16 x = link_x_coord + 4;
  hb->r0_xlo = x;
  hb->r8_xhi = x >> 8;
  uint16 y = link_y_coord + 8;
  hb->r1_ylo = y;
  hb->r9_yhi = y >> 8;
}

void Sprite_SetupHitBox(int k, SpriteHitBox *hb) {  // 86f7ef
  if (sign8(sprite_z[k])) {
    hb->r10_spr_xhi = 0x80;
    return;
  }
  int i = sprite_flags4[k] & 0x1f;
  int t, u;

  t = sprite_x_lo[k] + (uint8)kSpriteHitbox_XLo[i];
  hb->r4_spr_xlo = t;
  t = sprite_x_hi[k] + (uint8)kSpriteHitbox_XHi[i] + (t >> 8);
  hb->r10_spr_xhi = t;

  t = sprite_y_lo[k] + (uint8)kSpriteHitbox_YLo[i];
  u = t >> 8;
  t = (t & 0xff) - sprite_z[k];
  hb->r5_spr_ylo = t;
  t = sprite_y_hi[k] - (t < 0);
  hb->r11_spr_yhi = t + u + (uint8)kSpriteHitbox_YHi[i];

  hb->r6_spr_xsize = kSpriteHitbox_XSize[i];
  hb->r7_spr_ysize = kSpriteHitbox_YSize[i];
}

// Returns the carry flag
bool CheckIfHitBoxesOverlap(SpriteHitBox *hb) {  // 86f836
  int t;
  uint8 r15, r12;

  if (hb->r8_xhi == 0x80 || hb->r10_spr_xhi == 0x80)
    return false;

  t = hb->r5_spr_ylo - hb->r1_ylo;
  r15 = t + hb->r7_spr_ysize;
  r12 = hb->r11_spr_yhi - hb->r9_yhi - (t < 0);
  t = r12 + (((t & 0xff) + 0x80) >> 8);
  if (t & 0xff)
    return (t >= 0x100);
  if ((uint8)(hb->r3 + hb->r7_spr_ysize) < r15)
    return false;

  t = hb->r4_spr_xlo - hb->r0_xlo;
  r15 = t + hb->r6_spr_xsize;
  r12 = hb->r10_spr_xhi - hb->r8_xhi - (t < 0);
  t = r12 + (((t & 0xff) + 0x80) >> 8);
  if (t & 0xff)
    return (t >= 0x100);
  if ((uint8)(hb->r2 + hb->r6_spr_xsize) < r15)
    return false;

  return true;
}

void Oam_AllocateDeferToPlayer(int k) {  // 86f86c
  if (sprite_floor[k] != link_is_on_lower_level)
    return;
  PairU8 right = Sprite_IsRightOfLink(k);
  if ((uint8)(right.b + 0x10) >= 0x20)
    return;
  PairU8 below = Sprite_IsBelowLink(k);
  if ((uint8)(below.b + 0x20) >= 0x48)
    return;
  uint8 nslots = ((sprite_flags2[k] & 0x1f) + 1) << 2;
  if (below.a)
    Oam_AllocateFromRegionC(nslots);
  else
    Oam_AllocateFromRegionB(nslots);
}

void SpriteModule_Die(int k) {  // 86f8a2
  SpriteDeath_MainEx(k, false);
}

void Sprite_DoTheDeath(int k) {  // 86f923
  uint8 type = sprite_type[k];
  // This is how Vitreous knows whether to come out of his slime pool
  if (type == 0xBE)
    sprite_G[0]--;

  if (type == 0xaa && sprite_E[k] != 0) {
    uint8 bak = sprite_subtype[k];
    PrepareEnemyDrop(k, kPikitDropItems[sprite_E[k] - 1]);
    sprite_subtype[k] = bak;
    if (bak == 1) {
      sprite_oam_flags[k] = 9;
      sprite_flags3[k] = 0xf0;
    }
    sprite_head_dir[k]++;
    return;
  }

  // Resets the music in the village when the crazy green guards are killed.
  if (type == 0x45 && sram_progress_indicator == 2 && BYTE(overworld_area_index) == 0x18)
    music_control = 7;

  uint8 drop_item = sprite_die_action[k];
  if (drop_item != 0) {
    sprite_subtype[k] = sprite_N[k];
    sprite_N[k] = 255;
    uint8 arg = (drop_item == 1) ? 0xe4 : // small key, big key or rupee
                (drop_item == 3) ? 0xd9 : 0xe5;
    PrepareEnemyDrop(k, arg);
    return;
  }

  uint8 prize = sprite_flags5[k] & 0xf;
  if (prize-- != 0) {
    uint8 luck = item_drop_luck;
    if (luck != 0) {
      if (++luck_kill_counter >= 10)
        item_drop_luck = 0;
      if (luck == 1) {
        ForcePrizeDrop(k, prize, 1);
        return;
      }
    } else {
      if (!(GetRandomNumber() & kPrizeMasks[prize])) {
        ForcePrizeDrop(k, prize, prize);
        return;
      }
    }
  }
  sprite_state[k] = 0;
  SpriteDeath_Func4(k);
}

void ForcePrizeDrop(int k, uint8 prize, uint8 slot) {  // 86f9bc
  prize = prize * 8 | prizes_arr1[slot];
  prizes_arr1[slot] = (prizes_arr1[slot] + 1) & 7;
  PrepareEnemyDrop(k, kPrizeItems[prize]);
}

void PrepareEnemyDrop(int k, uint8 item) {  // 86f9d1
  sprite_type[k] = item;
  if (item == 0xe5)
    SpritePrep_BigKey_load_graphics(k);
  else if (item == 0xe4)
    SpritePrep_KeySetItemDrop(k);

  sprite_state[k] = 9;
  uint8 zbak = sprite_z[k];
  SpritePrep_LoadProperties(k);
  sprite_ignore_projectile[k]++;

  uint8 pz = kPrizeZ[sprite_type[k] - 0xd8];
  sprite_z_vel[k] = pz & 0xf0;
  Sprite_SetX(k, Sprite_GetX(k) + (pz & 0xf));
  sprite_z[k] = zbak;
  sprite_delay_aux4[k] = 21;
  sprite_stunned[k] = 255;
  SpriteDeath_Func4(k);
}

void SpriteDeath_Func4(int k) {  // 86fa25
  if (sprite_type[k] == 0xa2 && Sprite_CheckIfScreenIsClear())
    Ancilla_SpawnFallingPrize(4);
  Sprite_ManuallySetDeathFlagUW(k);
  num_sprites_killed++;
  if (sprite_type[k] == 0x40) {
    // evil barrier
    sprite_state[k] = 9;
    sprite_graphics[k] = 4;
    SpriteDeath_MainEx(k, true);
  }
}

void SpriteDeath_DrawPoof(int k) {  // 86fb2a
  if (dung_hdr_collision == 4)
    sprite_obj_prio[k] = 0x30;
  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;
  OamEnt *oam = GetOamCurPtr();
  uint8 r12 = (sprite_flags3[k] & 0x20) >> 3;
  int i = ((sprite_delay_main[k] & 0x1c) ^ 0x1c) + 3, n = 3;
  do {
    if (kPerishOverlay_Char[i]) {
      oam->charnum = kPerishOverlay_Char[i];
      oam->y = HIBYTE(dungmap_var7) - r12 + kPerishOverlay_Y[i];
      oam->x = BYTE(dungmap_var7) - r12 + kPerishOverlay_X[i];
      oam->flags = (info.flags & 0x30) | kPerishOverlay_Flags[i];
    }
  } while (oam++, i--, --n >= 0);
  Sprite_CorrectOamEntries(k, 3, 0);
}

void SpriteModule_Fall2(int k) {  // 86fbea
  uint8 delay = sprite_delay_main[k];
  if (!delay) {
    sprite_state[k] = 0;
    Sprite_ManuallySetDeathFlagUW(k);
    return;
  }

  if (delay >= 0x40) {
    if (sprite_oam_flags[k] != 5) {
      if (!(delay & 7 | submodule_index | flag_unk1))
        SpriteSfx_QueueSfx3WithPan(k, 0x31);
      SpriteActive_Main(k);
      PrepOamCoordsRet info;
      if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
        return;
      Sprite_DrawDistress_custom(info.x, info.y - 8, delay + 20);
      return;
    }
    sprite_delay_main[k] = delay = 63;
  }

  if (delay == 61)
    SpriteSfx_QueueSfx2WithPan(k, 0x20);

  int j = delay >> 1;

  if (sprite_type[k] == 0x26 || sprite_type[k] == 0x13) {
    sprite_graphics[k] = kSpriteFall_Tab2[j];
    SpriteDraw_FallingHelmaBeetle(k);
  } else {
    uint8 t = kSpriteFall_Tab1[j];
    if (t < 12)
      t += kSpriteFall_Tab4[sprite_D[k]];
    sprite_graphics[k] = t;
    SpriteDraw_FallingHumanoid(k);
  }
  if (frame_counter & kSpriteFall_Tab3[sprite_delay_main[k] >> 3] | submodule_index)
    return;
  Sprite_CheckTileProperty(k, 0x68);
  if (sprite_tiletype != 0x20) {
    sprite_y_recoil[k] = 0;
    sprite_x_recoil[k] = 0;
  }
  sprite_y_vel[k] = (int8)sprite_y_recoil[k] >> 2;
  sprite_x_vel[k] = (int8)sprite_x_recoil[k] >> 2;
  Sprite_MoveXY(k);
}

void SpriteDraw_FallingHelmaBeetle(int k) {  // 86fd17
  PrepOamCoordsRet info;
  const DrawMultipleData *src = kSpriteDrawFall0Data + sprite_graphics[k];
  if (sprite_type[k] == 0x13)
    src += 6;
  Sprite_DrawMultiple(k, src, 1, &info);
}

void SpriteDraw_FallingHumanoid(int k) {  // 86fe5b
  PrepOamCoordsRet info;
  if (Sprite_PrepOamCoordOrDoubleRet(k, &info))
    return;

  int q = sprite_graphics[k];
  OamEnt *oam = GetOamCurPtr();
  int n = (q < 12 && (q & 3) == 0) ? 3 : 0, nn = n;
  do {
    int i = q * 4 + n;
    oam->x = info.x + kSpriteDrawFall1_X[i];
    oam->y = info.y + kSpriteDrawFall1_Y[i];
    oam->charnum = kSpriteDrawFall1_Char[i];
    oam->flags = info.flags ^ kSpriteDrawFall1_Flags[i];
    bytewise_extended_oam[oam - oam_buf] = kSpriteDrawFall1_Ext[i];
  } while (oam++, --n >= 0);
  Sprite_CorrectOamEntries(k, nn, 0xff);
}

void Sprite_CorrectOamEntries(int k, int n, uint8 islarge) {  // 86febc
  OamEnt *oam = GetOamCurPtr();
  uint8 *extp = &g_ram[oam_ext_cur_ptr];
  uint16 spr_x = Sprite_GetX(k);
  uint16 spr_y = Sprite_GetY(k);
  uint8 scrollx = spr_x - BG2HOFS_copy2;
  uint8 scrolly = spr_y - BG2VOFS_copy2;
  do {
    uint16 x = spr_x + (int8)(oam->x - scrollx);
    uint16 y = spr_y + (int8)(oam->y - scrolly);
    uint8 ext = sign8(islarge) ? (*extp & 2) : islarge;
    *extp = ext + ((uint16)(x - BG2HOFS_copy2) >= 0x100);
    if ((uint16)(y + 0x10 - BG2VOFS_copy2) >= 0x100)
      oam->y = 0xf0;
  } while (oam++, extp++, --n >= 0);
}

bool Sprite_ReturnIfRecoiling(int k) {  // 86ff78
  if (!sprite_F[k])
    return false;
  if (!(sprite_F[k] & 0x7f)) {
    sprite_F[k] = 0;
    return false;
  }
  uint8 yvbak = sprite_y_vel[k];
  uint8 xvbak = sprite_x_vel[k];
  if (!--sprite_F[k] && ((uint8)(sprite_x_recoil[k] + 0x20) >= 0x40 || (uint8)(sprite_y_recoil[k] + 0x20) >= 0x40))
    sprite_F[k] = 144;

  int i = sprite_F[k];
  uint8 t;
  if (!sign8(i) && !(frame_counter & kSprite2_ReturnIfRecoiling_Masks[i>>2])) {
    sprite_y_vel[k] = sprite_y_recoil[k];
    sprite_x_vel[k] = sprite_x_recoil[k];
    if (!sign8(sprite_bump_damage[k]) && (t = (Sprite_CheckTileCollision(k) & 0xf))) {
      if (t < 4)
        sprite_x_recoil[k] = sprite_x_vel[k] = 0;
      else
        sprite_y_recoil[k] = sprite_y_vel[k] = 0;
    } else {
      Sprite_MoveXY(k);
    }
  }
  sprite_y_vel[k] = yvbak;
  sprite_x_vel[k] = xvbak;
  return sprite_type[k] != 0x7a;
}

bool Sprite_CheckIfLinkIsBusy() {  // 87f4d0
  if (link_auxiliary_state | link_pose_for_item | (link_state_bits & 0x80))
    return true;
  for (int i = 4; i >= 0; i--) {
    if (ancilla_type[i] == 0x27)
      return true;
  }
  return false;
}

void Sprite_SetSpawnedCoordinates(int k, SpriteSpawnInfo *info) {  // 89ae64
  sprite_x_lo[k] = info->r0_x;
  sprite_x_hi[k] = info->r0_x >> 8;
  sprite_y_lo[k] = info->r2_y;
  sprite_y_hi[k] = info->r2_y >> 8;
  sprite_z[k] = info->r4_z;
}

bool Sprite_CheckIfScreenIsClear() {  // 89af32
  for (int i = 15; i >= 0; i--) {
    if (sprite_state[i] && !(sprite_flags4[i] & 0x40)) {
      uint16 x = Sprite_GetX(i) - BG2HOFS_copy2;
      uint16 y = Sprite_GetY(i) - BG2VOFS_copy2;
      if (x < 256 && y < 256)
        return false;
    }
  }
  return Sprite_CheckIfOverlordsClear();
}

bool Sprite_CheckIfRoomIsClear() {  // 89af61
  for (int i = 15; i >= 0; i--) {
    if (sprite_state[i] && !(sprite_flags4[i] & 0x40))
      return false;
  }
  return Sprite_CheckIfOverlordsClear();
}

bool Sprite_CheckIfOverlordsClear() {  // 89af76
  for (int i = 7; i >= 0; i--) {
    if (overlord_type[i] == 0x14 || overlord_type[i] == 0x18)
      return false;
  }
  return true;
}

void Sprite_InitializeMirrorPortal() {  // 89af89
  for (int k = 15; k >= 0; k--) {
    if (sprite_state[k] && sprite_type[k] == 0x6c)
      sprite_state[k] = 0;
  }

  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(0xff, 0x6c, &info);
  if (j < 0)
    j = 0;

  Sprite_SetX(j, bird_travel_x_hi[15] << 8 | bird_travel_x_lo[15]);
  Sprite_SetY(j, (bird_travel_y_hi[15] << 8 | bird_travel_y_lo[15]) + 8);

  sprite_floor[j] = 0;
  sprite_ignore_projectile[j] = 1;
}

void Sprite_InitializeSlots() {  // 89afd6
  for (int k = 15; k >= 0; k--) {
    uint8 st = sprite_state[k], ty = sprite_type[k];
    if (st != 0) {
      if (st == 10) {
        if (ty != 0xec && ty != 0xd2) {
          link_picking_throw_state = 0;
          link_state_bits = 0;
          sprite_state[k] = 0;
        }
      } else {
        if (ty != 0x6c && sprite_room[k] != BYTE(overworld_area_index))
          sprite_state[k] = 0;
      }
    }
  }
  for (int k = 7; k >= 0; k--) {
    if (overlord_type[k] && overlord_spawned_in_area[k] != BYTE(overworld_area_index))
      overlord_type[k] = 0;
  }
}

void Garnish_ExecuteUpperSlots() {  // 89b08c
  HandleScreenFlash();

  if (garnish_active) {
    for (int i = 29; i >= 15; i--)
      Garnish_ExecuteSingle(i);
  }
}

void Garnish_ExecuteLowerSlots() {  // 89b097
  if (garnish_active) {
    for (int i = 14; i >= 0; i--)
      Garnish_ExecuteSingle(i);
  }
}

void Garnish_ExecuteSingle(int k) {  // 89b0b6
  cur_object_index = k;
  uint8 type = garnish_type[k];
  if (type == 0)
    return;
  if ((type == 5 || (submodule_index | flag_unk1) == 0) && garnish_countdown[k] != 0 && --garnish_countdown[k] == 0) {
    garnish_type[k] = 0;
    return;
  }
  uint8 sprsize = kGarnish_OamMemSize[garnish_type[k]];
  if (sort_sprites_setting) {
    if (garnish_floor[k])
      Oam_AllocateFromRegionF(sprsize);
    else
      Oam_AllocateFromRegionD(sprsize);
  } else {
    Oam_AllocateFromRegionA(sprsize);
  }
  kGarnish_Funcs[garnish_type[k] - 1](k);
}

void Garnish15_ArrghusSplash(int k) {  // 89b178
  static const int8 kArrghusSplash_X[8] = {-12, 20, -10, 10, -8, 8, -4, 4};
  static const int8 kArrghusSplash_Y[8] = {-4, -4, -2, -2, 0, 0, 0, 0};
  static const uint8 kArrghusSplash_Char[8] = {0xae, 0xae, 0xae, 0xae, 0xae, 0xae, 0xac, 0xac};
  static const uint8 kArrghusSplash_Flags[8] = {0x34, 0x74, 0x34, 0x74, 0x34, 0x74, 0x34, 0x74};
  static const uint8 kArrghusSplash_Ext[8] = {0, 0, 2, 2, 2, 2, 2, 2};

  Point16U pt;
  if (Garnish_ReturnIfPrepFails(k, &pt))
    return;
  OamEnt *oam = GetOamCurPtr();
  int g = (garnish_countdown[k] >> 1) & 6;
  for (int i = 1; i >= 0; i--) {
    int j = i + g;
    oam->x = pt.x + kArrghusSplash_X[j];
    oam->y = pt.y + kArrghusSplash_Y[j];
    oam->charnum = kArrghusSplash_Char[j];
    oam->flags = kArrghusSplash_Flags[j];
    bytewise_extended_oam[oam - oam_buf] = kArrghusSplash_Ext[j];
    oam++;
  }
}

void Garnish13_PyramidDebris(int k) {  // 89b216
  OamEnt *oam = GetOamCurPtr();

  int y = (garnish_y_lo[k] << 8) + garnish_y_subpixel[k] + ((int8)garnish_y_vel[k] << 4);
  garnish_y_subpixel[k] = y;
  garnish_y_lo[k] = y >> 8;

  int x = (garnish_x_lo[k] << 8) + garnish_x_subpixel[k] + ((int8)garnish_x_vel[k] << 4);
  garnish_x_subpixel[k] = x;
  garnish_x_lo[k] = x >> 8;

  garnish_y_vel[k] = garnish_y_vel[k] + 3;
  uint8 t;
  if ((t = garnish_x_lo[k] - BG2HOFS_copy2) >= 248) {
    garnish_type[k] = 0;
    return;
  }
  oam->x = t;
  if ((t = garnish_y_lo[k] - BG2VOFS_copy2) >= 240) {
    garnish_type[k] = 0;
    return;
  }
  oam->y = t;
  oam->charnum = 0x5c;
  oam->flags = (frame_counter << 3) & 0xc0 | 0x34;
  bytewise_extended_oam[oam - oam_buf] = 0;
}

void Garnish11_WitheringGanonBatFlame(int k) {  // 89b2b2
  if ((submodule_index | flag_unk1) == 0) {
    Garnish_SetY(k, Garnish_GetY(k) - 1);
  }
  Point16U pt;
  if (Garnish_ReturnIfPrepFails(k, &pt))
    return;
  OamEnt *oam = GetOamCurPtr();
  oam[0].x = pt.x;
  oam[0].y = pt.y;
  oam[1].x = pt.x + 8;
  oam[1].y = pt.y;
  oam[0].charnum = 0xa4;
  oam[1].charnum = 0xa5;
  oam[0].flags = 0x22;
  oam[1].flags = 0x22;
  bytewise_extended_oam[oam - oam_buf] = 0;
  bytewise_extended_oam[oam - oam_buf + 1] = 0;
}

void Garnish10_GanonBatFlame(int k) {  // 89b306
  static const uint8 kGanonBatFlame_Idx[32] = {
    7, 6, 5, 4, 5, 4, 5, 4, 5, 4, 5, 4, 5, 4, 5, 4,
    5, 4, 5, 4, 5, 4, 5, 4, 5, 4, 5, 4, 5, 4, 5, 4,
  };
  static const uint8 kGanonBatFlame_Char[7] = {0xac, 0xac, 0x66, 0x66, 0x8e, 0xa0, 0xa2};
  static const uint8 kGanonBatFlame_Flags[7] = {1, 0x41, 1, 0x41, 0, 0, 0};

  if (garnish_countdown[k] == 8)
    garnish_type[k] = 0x11;
  Point16U pt;
  if (Garnish_ReturnIfPrepFails(k, &pt))
    return;
  OamEnt *oam = GetOamCurPtr();
  oam->x = pt.x;
  oam->y = pt.y;
  int j = kGanonBatFlame_Idx[garnish_countdown[k] >> 3];
  oam->charnum = kGanonBatFlame_Char[j];
  oam->flags = kGanonBatFlame_Flags[j] | 0x22;
  bytewise_extended_oam[oam - oam_buf] = 2;
  Garnish_CheckPlayerCollision(k, pt.x, pt.y);
}

void Garnish0C_TrinexxIceBreath(int k) {  // 89b34f
  static const uint8 kTrinexxIce_Char[12] = {0xe8, 0xe8, 0xe6, 0xe6, 0xe4, 0xe4, 0xe4, 0xe4, 0xe4, 0xe4, 0xe4, 0xe4};
  static const uint8 kTrinexxIce_Flags[4] = {0, 0x40, 0xc0, 0x80};

  if (garnish_countdown[k] == 0x50 && (submodule_index | flag_unk1) == 0) {
    Dungeon_UpdateTileMapWithCommonTile(Garnish_GetX(k), Garnish_GetY(k) - 16, 18);
  }
  Point16U pt;
  if (Garnish_ReturnIfPrepFails(k, &pt))
    return;

  OamEnt *oam = GetOamCurPtr();
  oam->x = pt.x;
  oam->y = pt.y;
  oam->charnum = kTrinexxIce_Char[garnish_countdown[k] >> 4];
  oam->flags = kTrinexxIce_Flags[(garnish_countdown[k] >> 2) & 3] | 0x35;
  bytewise_extended_oam[oam - oam_buf] = 2;
}

void Garnish14_KakKidDashDust(int k) {  // 89b3bc
  Garnish_DustCommon(k, 2);
}

void Garnish_WaterTrail(int k) {  // 89b3c2
  Garnish_DustCommon(k, 3);
}

void Garnish0A_CannonSmoke(int k) {  // 89b3ee
  static const uint8 kGarnish_CannonPoof_Char[2] = { 0x8a, 0x86 };
  static const uint8 kGarnish_CannonPoof_Flags[4] = { 0x20, 0x10, 0x30, 0x30 };
  Point16U pt;
  if (Garnish_ReturnIfPrepFails(k, &pt))
    return;
  OamEnt *oam = GetOamCurPtr();
  oam->x = pt.x;
  oam->y = pt.y;
  oam->charnum = kGarnish_CannonPoof_Char[garnish_countdown[k] >> 3];
  int j = garnish_sprite[k];
  oam->flags = kGarnish_CannonPoof_Flags[j] | 4;
  bytewise_extended_oam[oam - oam_buf] = 2;
}

void Garnish09_LightningTrail(int k) {  // 89b429
  static const uint8 kLightningTrail_Char[8] = {0xcc, 0xec, 0xce, 0xee, 0xcc, 0xec, 0xce, 0xee};
  static const uint8 kLightningTrail_Flags[8] = {0x31, 0x31, 0x31, 0x31, 0x71, 0x71, 0x71, 0x71};
  Point16U pt;
  if (Garnish_ReturnIfPrepFails(k, &pt))
    return;
  OamEnt *oam = GetOamCurPtr();
  oam->x = pt.x;
  oam->y = pt.y;
  int j = garnish_sprite[k];
  oam->charnum = kLightningTrail_Char[j] - (BYTE(dungeon_room_index2) == 0x20 ? 0x80 : 0);
  oam->flags = (frame_counter << 1) & 0xe | kLightningTrail_Flags[j];
  bytewise_extended_oam[oam - oam_buf] = 2;
  Garnish_CheckPlayerCollision(k, pt.x, pt.y);
}

void Garnish_CheckPlayerCollision(int k, int x, int y) {  // 89b459
  if ((k ^ frame_counter) & 7 | countdown_for_blink | link_disable_sprite_damage)
    return;

  if ((uint8)(link_x_coord - BG2HOFS_copy2 - x + 12) < 24 &&
      (uint8)(link_y_coord - BG2VOFS_copy2 - y + 22) < 28) {
    link_auxiliary_state = 1;
    link_incapacitated_timer = 16;
    link_give_damage = 16;
    link_actual_vel_x ^= 255;
    link_actual_vel_y ^= 255;
  }
}

void Garnish07_BabasuFlash(int k) {  // 89b49e
  static const uint8 kBabusuFlash_Char[4] = {0xa8, 0x8a, 0x86, 0x86};
  static const uint8 kBabusuFlash_Flags[4] = {0x2d, 0x2c, 0x2c, 0x2c};
  Point16U pt;
  if (Garnish_ReturnIfPrepFails(k, &pt))
    return;
  OamEnt *oam = GetOamCurPtr();
  oam->x = pt.x;
  oam->y = pt.y;
  int j = garnish_countdown[k] >> 3;
  oam->charnum = kBabusuFlash_Char[j];
  oam->flags = kBabusuFlash_Flags[j];
  bytewise_extended_oam[oam - oam_buf] = 2;
}

void Garnish08_KholdstareTrail(int k) {  // 89b4c6
  static const int8 kGarnish_Nebule_XY[3] = { -1, -1, 0 };
  static const uint8 kGarnish_Nebule_Char[3] = { 0x9c, 0x9d, 0x8d };

  Point16U pt;
  if (Garnish_ReturnIfPrepFails(k, &pt))
    return;
  OamEnt *oam = GetOamCurPtr();
  int i = garnish_countdown[k] >> 2;
  oam->x = pt.x + kGarnish_Nebule_XY[i];
  oam->y = pt.y + kGarnish_Nebule_XY[i];
  oam->charnum = kGarnish_Nebule_Char[i];
  int j = garnish_sprite[k];
  oam->flags = (sprite_oam_flags[j] | sprite_obj_prio[j]) & ~1;
  bytewise_extended_oam[oam - oam_buf] = 0;
}

void Garnish06_ZoroTrail(int k) {  // 89b4fb
  Point16U pt;
  if (Garnish_ReturnIfPrepFails(k, &pt))
    return;
  OamEnt *oam = GetOamCurPtr();
  oam->x = pt.x;
  oam->y = pt.y;
  oam->charnum = 0x75;
  int j = garnish_sprite[k];
  oam->flags = sprite_oam_flags[j] | sprite_obj_prio[j];
  bytewise_extended_oam[oam - oam_buf] = 0;
}

void Garnish12_Sparkle(int k) {  // 89b520
  Garnish_SparkleCommon(k, 2);
}

void Garnish_SimpleSparkle(int k) {  // 89b526
  Garnish_SparkleCommon(k, 3);
}

void Garnish0E_TrinexxFireBreath(int k) {  // 89b55d
  static const uint8 kTrinexxLavaBubble_Char[4] = {0x83, 0xc7, 0x80, 0x9d};
  Point16U pt;
  if (Garnish_ReturnIfPrepFails(k, &pt))
    return;
  OamEnt *oam = GetOamCurPtr();
  oam->x = pt.x;
  oam->y = pt.y;
  oam->charnum =  kTrinexxLavaBubble_Char[garnish_countdown[k] >> 3];
  int j = garnish_sprite[k];
  oam->flags = (sprite_oam_flags[j] | sprite_obj_prio[j]) & 0xf0 | 0xe;
  bytewise_extended_oam[oam - oam_buf] = 0;

}

void Garnish0F_BlindLaserTrail(int k) {  // 89b591
  static const uint8 kBlindLaserTrail_Char[4] = {0x61, 0x71, 0x70, 0x60};
  Point16U pt;
  if (Garnish_ReturnIfPrepFails(k, &pt))
    return;
  OamEnt *oam = GetOamCurPtr();
  oam->x = pt.x;
  oam->y = pt.y;
  oam->charnum = kBlindLaserTrail_Char[garnish_oam_flags[k] - 7];
  int j = garnish_sprite[k];
  oam->flags = sprite_oam_flags[j] | sprite_obj_prio[j];
  bytewise_extended_oam[oam - oam_buf] = 0;
}

void Garnish04_LaserTrail(int k) {  // 89b5bb
  static const uint8 kLaserBeamTrail_Char[2] = {0xd2, 0xf3};
  Point16U pt;
  if (Garnish_ReturnIfPrepFails(k, &pt))
    return;
  OamEnt *oam = GetOamCurPtr();
  oam->x = pt.x;
  oam->y = pt.y;
  oam->charnum = kLaserBeamTrail_Char[garnish_oam_flags[k]];
  oam->flags = 0x25;
  bytewise_extended_oam[oam - oam_buf] = 0;
}

bool Garnish_ReturnIfPrepFails(int k, Point16U *pt) {  // 89b5de
  uint16 x = Garnish_GetX(k) - BG2HOFS_copy2;
  uint16 y = Garnish_GetY(k) - BG2VOFS_copy2;

  if (x >= 256 || y >= 256) {
    garnish_type[k] = 0;
    return true;
  }
  pt->x = x;
  pt->y = y - 16;
  return false;
}

void Garnish03_FallingTile(int k) {  // 89b627
  static const uint8 kCrumbleTile_XY[5] = {4, 0, 0, 0, 0};
  static const uint8 kCrumbleTile_Char[5] = {0x80, 0xcc, 0xcc, 0xea, 0xca};
  static const uint8 kCrumbleTile_Flags[5] = {0x30, 0x31, 0x31, 0x31, 0x31};
  static const uint8 kCrumbleTile_Ext[5] = {0, 2, 2, 2, 2};

  int j;
  if ((j = garnish_countdown[k]) == 0x1e && (j = (submodule_index | flag_unk1)) == 0)
    Dungeon_UpdateTileMapWithCommonTile(Garnish_GetX(k), Garnish_GetY(k) - 16, 4);
  j >>= 3;

  uint16 x = Garnish_GetX(k) + kCrumbleTile_XY[j] - BG2HOFS_copy2;
  uint16 y = Garnish_GetY(k) + kCrumbleTile_XY[j] - BG2VOFS_copy2;

  if (x < 256 && y < 256) {
    OamEnt *oam = GetOamCurPtr();
    oam->x = x;
    oam->y = y - 16;
    oam->charnum = kCrumbleTile_Char[j];
    oam->flags = kCrumbleTile_Flags[j];
    bytewise_extended_oam[oam - oam_buf] = kCrumbleTile_Ext[j];
  }
}

void Garnish01_FireSnakeTail(int k) {  // 89b6c0
  Point16U pt;
  if (Garnish_ReturnIfPrepFails(k, &pt))
    return;
  OamEnt *oam = GetOamCurPtr();
  oam->x = pt.x;
  oam->y = pt.y;
  oam->charnum = 0x28;
  int j = garnish_sprite[k];
  oam->flags = sprite_oam_flags[j] | sprite_obj_prio[j];
  bytewise_extended_oam[oam - oam_buf] = 2;
}

void Garnish02_MothulaBeamTrail(int k) {  // 89b6e1
  OamEnt *oam = GetOamCurPtr();
  oam->x = garnish_x_lo[k] - BG2HOFS_copy2;
  oam->y = garnish_y_lo[k] - BG2VOFS_copy2;
  oam->charnum = 0xaa;
  int j = garnish_sprite[k];
  oam->flags = sprite_oam_flags[j] | sprite_obj_prio[j];
  bytewise_extended_oam[oam - oam_buf] = 2;
}

void Dungeon_ResetSprites() {  // 89c114
  Dungeon_CacheTransSprites();
  link_picking_throw_state = 0;
  link_state_bits = 0;
  Sprite_DisableAll();
  sprcoll_x_size = sprcoll_y_size = 0xffff;
  int j = FindInWordArray(dungeon_room_history, dungeon_room_index2, 4);
  if (j < 0) {
    uint16 blk = dungeon_room_history[3];
    dungeon_room_history[3] = dungeon_room_history[2];
    dungeon_room_history[2] = dungeon_room_history[1];
    dungeon_room_history[1] = dungeon_room_history[0];
    dungeon_room_history[0] = dungeon_room_index2;
    if (blk != 0xffff)
      sprite_where_in_room[blk] = 0;
  }
  Dungeon_LoadSprites();
}

void Dungeon_CacheTransSprites() {  // 89c176
  if (!player_is_indoors)
    return;
  alt_sprites_flag = player_is_indoors;
  for (int k = 15; k >= 0; k--) {
    alt_sprite_state[k] = 0;
    alt_sprite_type[k] = sprite_type[k];
    alt_sprite_x_lo[k] = sprite_x_lo[k];
    alt_sprite_graphics[k] = sprite_graphics[k];
    alt_sprite_x_hi[k] = sprite_x_hi[k];
    alt_sprite_y_lo[k] = sprite_y_lo[k];
    alt_sprite_y_hi[k] = sprite_y_hi[k];
    if (sprite_pause[k] != 0 || sprite_state[k] == 4 || sprite_state[k] == 10)
      continue;
    alt_sprite_state[k] = sprite_state[k];
    alt_sprite_A[k] = sprite_A[k];
    alt_sprite_head_dir[k] = sprite_head_dir[k];
    alt_sprite_oam_flags[k] = sprite_oam_flags[k];
    alt_sprite_obj_prio[k] = sprite_obj_prio[k];
    alt_sprite_D[k] = sprite_D[k];
    alt_sprite_flags2[k] = sprite_flags2[k];
    alt_sprite_floor[k] = sprite_floor[k];
    alt_sprite_spawned_flag[k] = sprite_ai_state[k];
    alt_sprite_flags3[k] = sprite_flags3[k];
    alt_sprite_B[k] = sprite_B[k];
    alt_sprite_C[k] = sprite_C[k];
    alt_sprite_E[k] = sprite_E[k];
    alt_sprite_subtype2[k] = sprite_subtype2[k];
    alt_sprite_height_above_shadow[k] = sprite_z[k];
    alt_sprite_delay_main[k] = sprite_delay_main[k];
    alt_sprite_I[k] = sprite_I[k];
    alt_sprite_maybe_ignore_projectile[k] = sprite_ignore_projectile[k];
  }
}

void Sprite_DisableAll() {  // 89c22f
  for (int k = 15; k >= 0; k--) {
    if (sprite_state[k] && (player_is_indoors || sprite_type[k] != 0x6c))
      sprite_state[k] = 0;
  }
  for (int k = 9; k >= 0; k--)
    ancilla_type[k] = 0;
  flag_is_ancilla_to_pick_up = 0;
  sprite_limit_instance = 0;
  byte_7E0B9B = 0;
  byte_7E0B88 = 0;
  archery_game_arrows_left = 0;
  garnish_active = 0;
  byte_7E0B9E = 0;
  activate_bomb_trap_overlord = 0;
  intro_times_pal_flash = 0;
  byte_7E0FF8 = 0;
  byte_7E0FFB = 0;
  flag_block_link_menu = 0;
  byte_7E0FFD = 0;
  byte_7E0FC6 = 0;
  byte_7E03FC = 0;
  for (int k = 7; k >= 0; k--)
    overlord_type[k] = 0;
  for (int k = 29; k >= 0; k--)
    garnish_type[k] = 0;
}

void Dungeon_LoadSprites() {  // 89c290
  const uint8 *src = kDungeonSprites + kDungeonSpriteOffs[dungeon_room_index2];
  byte_7E0FB1 = dungeon_room_index2 >> 3 & 0xfe;
  byte_7E0FB0 = (dungeon_room_index2 & 0xf) << 1;
  sort_sprites_setting = *src++;
  for (int k = 0; *src != 0xff; src += 3)
    k = Dungeon_LoadSingleSprite(k, src) + 1;
}

void Sprite_ManuallySetDeathFlagUW(int k) {  // 89c2f5
  if (!player_is_indoors || sprite_defl_bits[k] & 1 || sign8(sprite_N[k]))
    return;
  sprite_where_in_room[dungeon_room_index2] |= 1 << sprite_N[k];
}

int Dungeon_LoadSingleSprite(int k, const uint8 *src) {  // 89c327
  uint8 y = src[0], x = src[1], type = src[2];
  if (type == 0xe4) {
    if (y == 0xfe || y == 0xfd) {
      sprite_die_action[k - 1] = (y == 0xfe) ? 1 : 2;
      return k - 1;
    }
  } else if (x >= 0xe0) {
    Dungeon_LoadSingleOverlord(src);
    return k - 1;
  }
  if (!(kSpriteInit_DeflBits[type] & 1) && (sprite_where_in_room[dungeon_room_index2] & (1 << k)))
    return k;
  sprite_state[k] = 8;
  tmp_counter = y;
  sprite_floor[k] = (y >> 7);
  Sprite_SetY(k, ((y << 4) & 0x1ff) + (byte_7E0FB1 << 8));
  byte_7E0FB6 = x;
  Sprite_SetX(k, ((x << 4) & 0x1ff) + (byte_7E0FB0 << 8));
  sprite_type[k] = type;
  tmp_counter = (tmp_counter & 0x60) >> 2;
  sprite_subtype[k] = tmp_counter | byte_7E0FB6 >> 5;
  sprite_N[k] = k;
  sprite_die_action[k] = 0;
  return k;
}

void Dungeon_LoadSingleOverlord(const uint8 *src) {  // 89c3e8
  int k = AllocOverlord();
  if (k < 0)
    return;
  uint8 y = src[0], x = src[1], type = src[2];
  overlord_type[k] = type;
  overlord_floor[k] = (y >> 7);
  int t = ((y << 4) & 0x1ff) + (byte_7E0FB1 << 8);
  overlord_y_lo[k] = t;
  overlord_y_hi[k] = t >> 8;
  t = ((x << 4) & 0x1ff) + (byte_7E0FB0 << 8);
  overlord_x_lo[k] = t;
  overlord_x_hi[k] = t >> 8;
  overlord_spawned_in_area[k] = overworld_area_index;
  overlord_gen2[k] = 0;
  overlord_gen1[k] = 0;
  overlord_gen3[k] = 0;
  if (overlord_type[k] == 10 || overlord_type[k] == 11) {
    overlord_gen2[k] = 160;
  } else if (overlord_type[k] == 3) {
    overlord_gen2[k] = 255;
    overlord_x_lo[k] -= 8;
  }
}

void Sprite_ResetAll() {  // 89c44e
  Sprite_DisableAll();
  Sprite_ResetAll_noDisable();
}

void Sprite_ResetAll_noDisable() {  // 89c452
  byte_7E0FDD = 0;
  sprite_alert_flag = 0;
  byte_7E0FFD = 0;
  byte_7E02F0 = 0;
  byte_7E0FC6 = 0;
  sprite_limit_instance = 0;
  sort_sprites_setting = 0;
  if (savegame_tagalong != 13)
    super_bomb_indicator_unk2 = 0xfe;
  memset(sprite_where_in_room, 0, 0x1000);
  memset(overworld_sprite_was_loaded, 0, 0x200);
  memset(dungeon_room_history, 0xff, 8);
}

void Sprite_ReloadAll_Overworld() {  // 89c499
  Sprite_DisableAll();
  Sprite_OverworldReloadAll_justLoad();
}

void Sprite_OverworldReloadAll_justLoad() {  // 89c49d
  Sprite_ResetAll_noDisable();
  Overworld_LoadSprites();
  Sprite_ActivateAllProxima();
}

void Overworld_LoadSprites() {  // 89c4ac
  sprcoll_x_base = (overworld_area_index & 7) << 9;
  sprcoll_y_base = ((overworld_area_index & 0x3f) >> 2 & 0xe) << 8;
  sprcoll_x_size = sprcoll_y_size = kOverworldAreaSprcollSizes[BYTE(overworld_area_index)] << 8;
  const uint8 *src = GetOverworldSpritePtr(overworld_area_index);
  uint8 b;

  for (; (b = src[0]) != 0xff; src += 3) {
    if (src[2] == 0xf4) {
      byte_7E0FFD++;
      continue;
    }
    uint8 r2 = (src[0] >> 4) << 2;
    uint8 r6 = (src[1] >> 4) + r2;
    uint8 r5 = src[1] & 0xf | src[0] << 4;
    sprite_where_in_overworld[r5 | r6 << 8] = src[2] + 1;
  }
}

void Sprite_ActivateAllProxima() {  // 89c55e
  uint16 bak0 = BG2HOFS_copy2;
  uint8 bak1 = byte_7E069E[1];
  byte_7E069E[1] = 0xff;
  for (int i = 21; i >= 0; i--) {
    Sprite_ActivateWhenProximal();
    BG2HOFS_copy2 += 16;
  }
  byte_7E069E[1] = bak1;
  BG2HOFS_copy2 = bak0;
}

void Sprite_ProximityActivation() {  // 89c58f
  if (submodule_index != 0) {
    Sprite_ActivateWhenProximal();
    Sprite_ActivateWhenProximalBig();
  } else {
    if (!(spr_ranged_based_toggler & 1))
      Sprite_ActivateWhenProximal();
    if (spr_ranged_based_toggler & 1)
      Sprite_ActivateWhenProximalBig();
    spr_ranged_based_toggler++;
  }
}

void Sprite_ActivateWhenProximal() {  // 89c5bb
  if (byte_7E069E[1]) {
    uint16 x = BG2HOFS_copy2 + (sign8(byte_7E069E[1]) ? -0x10 : 0x110);
    uint16 y = BG2VOFS_copy2 - 48;
    for (int i = 21; i >= 0; i--, y += 16)
      Sprite_Overworld_ProximityMotivatedLoad(x, y);
  }
}

void Sprite_ActivateWhenProximalBig() {  // 89c5fa
  if (byte_7E069E[0]) {
    uint16 x = BG2HOFS_copy2 - 48;
    uint16 y = BG2VOFS_copy2 + (sign8(byte_7E069E[0]) ? -0x10 : 0x110);
    for (int i = 21; i >= 0; i--, x += 16)
      Sprite_Overworld_ProximityMotivatedLoad(x, y);
  }
}

void Sprite_Overworld_ProximityMotivatedLoad(uint16 x, uint16 y) {  // 89c6f5
  uint16 xt = (uint16)(x - sprcoll_x_base);
  uint16 yt = (uint16)(y - sprcoll_y_base);
  if (xt >= sprcoll_x_size || yt >= sprcoll_y_size)
    return;

  uint8 r1 = (yt >> 8) * 4 | (xt >> 8);
  uint8 r0 = y & 0xf0 | x >> 4 & 0xf;
  Overworld_LoadProximaSpriteIfAlive(r1 << 8 | r0);
}

void Overworld_LoadProximaSpriteIfAlive(uint16 blk) {  // 89c739
  uint8 *p5 = sprite_where_in_overworld + blk;
  uint8 sprite_to_spawn = *p5;
  if (!sprite_to_spawn)
    return;

  uint8 loadedmask = (0x80 >> (blk & 7));
  uint8 *loadedp = &overworld_sprite_was_loaded[blk >> 3];

  if (*loadedp & loadedmask)
    return;

  if (sprite_to_spawn >= 0xf4) {
    // load overlord
    int k = AllocOverlord();
    if (k < 0)
      return;
    *loadedp |= loadedmask;
    overlord_offset_sprite_pos[k] = blk;
    overlord_type[k] = sprite_to_spawn - 0xf3;
    overlord_x_lo[k] = (blk << 4 & 0xf0) + (overlord_type[k] == 1 ? 8 : 0);
    overlord_y_lo[k] = blk & 0xf0;
    overlord_x_hi[k] = (blk >> 8 & 3) + HIBYTE(sprcoll_x_base);
    overlord_y_hi[k] = (blk >> 10) + HIBYTE(sprcoll_y_base);
    overlord_floor[k] = 0;
    overlord_spawned_in_area[k] = overworld_area_index;
    overlord_gen2[k] = 0;
    overlord_gen1[k] = 0;
    overlord_gen3[k] = 0;
  } else {
    // load regular sprite
    int k = Overworld_AllocSprite(sprite_to_spawn);
    if (k < 0)
      return;
    *loadedp |= loadedmask;

    sprite_N_word[k] = blk;
    sprite_type[k] = sprite_to_spawn - 1;
    sprite_state[k] = 8;
    sprite_x_lo[k] = blk << 4 & 0xf0;
    sprite_y_lo[k] = blk & 0xf0;
    sprite_x_hi[k] = (blk >> 8 & 3) + HIBYTE(sprcoll_x_base);
    sprite_y_hi[k] = (blk >> 10) + HIBYTE(sprcoll_y_base);
    sprite_floor[k] = 0;
    sprite_subtype[k] = 0;
    sprite_die_action[k] = 0;
  }
}

void SpriteExplode_SpawnEA(int k) {  // 89ee4c
  tmp_counter = sprite_type[k];
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamicallyEx(k, 0xea, &info, 14);
  Sprite_SetSpawnedCoordinates(j, &info);
  sprite_z_vel[j] = 32;
  sprite_floor[j] = link_is_on_lower_level;
  sprite_A[j] = (j == 9) ? 2 : 6;
  Sprite_SetY(j, info.r2_y + 3);
  if (tmp_counter == 0xce) {
    Sprite_SetY(j, info.r2_y + 16);
    return;
  }
  if (tmp_counter == 0xcb) {
    sprite_y_lo[j] = sprite_x_lo[j] = 0x78;
    sprite_x_hi[j] = HIBYTE(link_x_coord);
    sprite_y_hi[j] = HIBYTE(link_y_coord);
  }
}

void Sprite_KillFriends() {  // 89ef56
  for(int j = 15; j >= 0; j--) {
    if (j != cur_object_index && sprite_state[j] && !(sprite_defl_bits[j] & 2) && sprite_type[j] != 0x7a) {
      sprite_state[j] = 6;
      sprite_delay_main[j] = 15;
      sprite_flags3[j] = 0;
      sprite_flags5[j] = 0;
      sprite_flags2[j] = 3;
    }
  }
}

void Garnish16_ThrownItemDebris(int k) {  // 89f0cb
  static const int16 kScatterDebris_Draw_X[64] = {
     0,  8,  0,  8, -2,  9, -1,  9, -4,  9, -1, 10, -6,  9, -1, 12,
    -7,  9, -2, 13, -9,  9, -3, 14, -4, -4,  9, 15, -3, -3, -3,  9,
    -4,  4,  6, 10, -1,  4,  6,  7,  0,  2,  4,  7,  1,  1,  5,  7,
     0, -2,  8,  9, -1, -6,  9, 10, -2, -7, 12, 11, -3, -9,  4,  6,
  };
  static const int8 kScatterDebris_Draw_Y[64] = {
      0,  0,  8,  8,   0, -1, 10, 10,   0, -3, 11,  7,   1, -4, 12,  8,
      1, -4, 13,  9,   2, -4, 16, 10,  14, 14, -4, 11,  16, 16, 16, -1,
      2, -5,  5,  1,   3, -7,  8,  2,   4, -8,  4, 10,  -9,  4,  4, 12,
    -10,  4,  8, 14, -12,  4,  8, 15, -15,  3,  8, 17, -17,  1, 18, 15,
  };
  static const int8 kScatterDebris_Draw_Char[64] = {
    0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58,
    0x48, 0x58, 0x58, 0x58, 0x48, 0x58, 0x58, 0x48, 0x48, 0x48, 0x58, 0x48, 0x48, 0x48, 0x48, 0x48,
    0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59,
    0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59,
  };
  static const uint8 kScatterDebris_Draw_Flags[64] = {
    0x80,    0, 0x80, 0x40, 0x80, 0x40, 0x80,    0,    0, 0xc0,    0, 0x80, 0x80, 0x40, 0x80,    0,
    0x80, 0xc0,    0, 0x80,    0,    0, 0x80,    0, 0x80, 0x80, 0x80, 0x80,    0,    0,    0,    0,
    0x40, 0x40, 0x40,    0, 0x40, 0x40, 0x40,    0, 0x40, 0x40,    0,    0, 0x80,    0, 0x40, 0x40,
    0x40,    0, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,    0,    0, 0x40,    0,    0,    0,
  };
  Point16U pt;
  if (Garnish_ReturnIfPrepFails(k, &pt))
    return;
  uint8 r5 = garnish_oam_flags[k];
  if (byte_7E0FC6 >= 3)
    return;
  if (garnish_sprite[k] == 3) {
    ScatterDebris_Draw(k, pt);
    return;
  }
  OamEnt *oam = GetOamCurPtr();
  tmp_counter = garnish_sprite[k];
  uint8 base = ((garnish_countdown[k] >> 2) ^ 7) << 2;
  if (tmp_counter == 4 || tmp_counter == 2 && !player_is_indoors)
    base += 0x20;

  for (int i = 3; i >= 0; i--) {
    int j = i + base;
    uint16 x = pt.x + kScatterDebris_Draw_X[j];
    oam->x = x;
    oam->y = pt.y + kScatterDebris_Draw_Y[j];
    oam->charnum = (tmp_counter == 0) ? 0x4E : (tmp_counter >= 0x80) ? 0xF2 : kScatterDebris_Draw_Char[j];
    oam->flags = kScatterDebris_Draw_Flags[j] | r5;
    bytewise_extended_oam[oam - oam_buf] = (x >> 8) & 1;
    oam++;
  }
}

void ScatterDebris_Draw(int k, Point16U pt) {  // 89f198
  static const int8 kScatterDebris_Draw_X2[12] = {-8, 8, 16, -5, 8, 15, -1, 7, 11, 1, 3, 8};
  static const int8 kScatterDebris_Draw_Y2[12] = {7, 2, 12, 9, 2, 10, 11, 2, 11, 7, 3, 8};
  static const uint8 kScatterDebris_Draw_Char2[12] = {0xe2, 0xe2, 0xe2, 0xe2, 0xf2, 0xf2, 0xf2, 0xe2, 0xe2, 0xf2, 0xe2, 0xe2};
  static const uint8 kScatterDebris_Draw_Flags2[12] = {0, 0, 0, 0, 0x80, 0x40, 0, 0x80, 0x40, 0, 0, 0};

  if (garnish_countdown[k] == 16)
    garnish_type[k] = 0;

  OamEnt *oam = GetOamCurPtr();
  int base = ((garnish_countdown[k] & 0xf) >> 2) * 3;

  for (int i = 2; i >= 0; i--) {
    int j = i + base;
    uint16 x = pt.x + kScatterDebris_Draw_X2[j];
    oam->x = x;
    oam->y = pt.y + kScatterDebris_Draw_Y2[j];
    oam->charnum = kScatterDebris_Draw_Char2[j];
    oam->flags = kScatterDebris_Draw_Flags2[j] | 0x22;
    bytewise_extended_oam[oam - oam_buf] = (x >> 8) & 1;
    oam++;
  }
}

void Sprite_KillSelf(int k) {  // 89f1f8
  if (!(sprite_defl_bits[k] & 0x40) && player_is_indoors)
    return;
  sprite_state[k] = 0;
  uint16 blk = sprite_N_word[k];
  g_ram[0] = blk;  // Sprite_PrepOamCoordOrDoubleRet reads this!
  WORD(g_ram[1]) = (blk >> 3) + 0xef80; // Sprite_PrepOamCoordOrDoubleRet reads this!
  uint8 loadedmask = (0x80 >> (blk & 7));
  uint16 addr = 0xEF80 + (blk >> 3);  // warning: blk may be bad, seen with cannon balls in 2nd dungeon

  uint8 *loadedp = &g_ram[addr + 0x10000];

  if (blk < 0xffff)
    *loadedp &= ~loadedmask;
  if (!player_is_indoors)
    sprite_N_word[k] = 0xffff;
  else
    sprite_N[k] = 0xff;
}

void SpritePrep_LoadProperties(int k) {  // 8db818
  SpritePrep_ResetProperties(k);
  int j = sprite_type[k];
  sprite_flags2[k] = kSpriteInit_Flags2[j];
  sprite_health[k] = kSpriteInit_Health[j];
  sprite_flags4[k] = kSpriteInit_Flags4[j];
  sprite_flags5[k] = kSpriteInit_Flags5[j];
  sprite_defl_bits[k] = kSpriteInit_DeflBits[j];
  sprite_bump_damage[k] = kSpriteInit_BumpDamage[j];
  sprite_flags[k] = kSpriteInit_Flags[j];
  sprite_room[k] = player_is_indoors ? dungeon_room_index2 : overworld_area_index;
  sprite_flags3[k] = kSpriteInit_Flags3[j];
  sprite_oam_flags[k] = kSpriteInit_Flags3[j] & 0xf;
}

void SpritePrep_LoadPalette(int k) {  // 8db85c
  int f = kSpriteInit_Flags3[sprite_type[k]];
  sprite_flags3[k] = f;
  sprite_oam_flags[k] = f & 15;
}

void SpritePrep_ResetProperties(int k) {  // 8db871
  sprite_pause[k] = 0;
  sprite_E[k] = 0;
  sprite_x_vel[k] = 0;
  sprite_y_vel[k] = 0;
  sprite_z_vel[k] = 0;
  sprite_x_subpixel[k] = 0;
  sprite_y_subpixel[k] = 0;
  sprite_z_subpos[k] = 0;
  sprite_ai_state[k] = 0;
  sprite_graphics[k] = 0;
  sprite_D[k] = 0;
  sprite_delay_main[k] = 0;
  sprite_delay_aux1[k] = 0;
  sprite_delay_aux2[k] = 0;
  sprite_delay_aux4[k] = 0;
  sprite_head_dir[k] = 0;
  sprite_anim_clock[k] = 0;
  sprite_G[k] = 0;
  sprite_hit_timer[k] = 0;
  sprite_wallcoll[k] = 0;
  sprite_z[k] = 0;
  sprite_health[k] = 0;
  sprite_F[k] = 0;
  sprite_x_recoil[k] = 0;
  sprite_y_recoil[k] = 0;
  sprite_A[k] = 0;
  sprite_B[k] = 0;
  sprite_C[k] = 0;
  sprite_unk2[k] = 0;
  sprite_subtype2[k] = 0;
  sprite_ignore_projectile[k] = 0;
  sprite_obj_prio[k] = 0;
  sprite_oam_flags[k] = 0;
  sprite_stunned[k] = 0;
  sprite_give_damage[k] = 0;
  sprite_unk3[k] = 0;
  sprite_unk4[k] = 0;
  sprite_unk5[k] = 0;
  sprite_unk1[k] = 0;
  sprite_I[k] = 0;
}

uint8 Oam_AllocateFromRegionA(uint8 num) {  // 8dba80
  return Oam_GetBufferPosition(num, 0);
}

uint8 Oam_AllocateFromRegionB(uint8 num) {  // 8dba84
  return Oam_GetBufferPosition(num, 2);
}

uint8 Oam_AllocateFromRegionC(uint8 num) {  // 8dba88
  return Oam_GetBufferPosition(num, 4);
}

uint8 Oam_AllocateFromRegionD(uint8 num) {  // 8dba8c
  return Oam_GetBufferPosition(num, 6);
}

uint8 Oam_AllocateFromRegionE(uint8 num) {  // 8dba90
  return Oam_GetBufferPosition(num, 8);
}

uint8 Oam_AllocateFromRegionF(uint8 num) {  // 8dba94
  return Oam_GetBufferPosition(num, 10);
}

uint8 Oam_GetBufferPosition(uint8 num, uint8 y) {  // 8dbb0a
  y >>= 1;
  uint16 p = oam_region_base[y], pstart = p;
  p += num;
  if (p >= kOamGetBufferPos_Tab0[y]) {
    int j = oam_alloc_arr1[y]++ & 7;
    pstart = kOamGetBufferPos_Tab1[y * 8 + j];
  } else {
    oam_region_base[y] = p;
  }
  oam_ext_cur_ptr = 0xa20 + (pstart >> 2);
  oam_cur_ptr = 0x800 + pstart;
  return oam_cur_ptr;
}

void Sprite_NullifyHookshotDrag() {  // 8ff540
  for (int i = 4; i >= 0; i--) {
    if (!(ancilla_type[i] & 0x1f) && related_to_hookshot) {
      related_to_hookshot = 0;
      break;
    }
  }
  link_x_coord_safe_return_hi = link_x_coord >> 8;
  link_y_coord_safe_return_hi = link_y_coord >> 8;
  link_x_coord = link_x_coord_prev;
  link_y_coord = link_y_coord_prev;
  HandleIndoorCameraAndDoors();
}

void Overworld_SubstituteAlternateSecret() {  // 9afbdb
  static const uint8 kSecretSubst_Tab0[64] = {
    0,  0, 0, 0, 0, 0, 0, 4,  0,  0, 0, 0, 0, 0, 0, 0,
    4,  4, 6, 4, 4, 6, 0, 0, 15, 15, 4, 5, 5, 4, 6, 6,
    15, 15, 4, 5, 5, 7, 6, 6, 31, 31, 4, 7, 7, 4, 6, 6,
    6,  7, 2, 0, 0, 0, 0, 0,  6,  6, 2, 0, 0, 0, 0, 0,
  };
  static const uint8 kSecretSubst_Tab2[16] = { 1, 1, 1, 1, 15, 1, 1, 18, 16, 1, 1, 1, 17, 1, 1, 3 };
  static const uint8 kSecretSubst_Tab1[16] = { 0, 0, 0, 0, 2, 0, 0, 8, 16, 0, 0, 0, 1, 0, 0, 0 };
  if (GetRandomNumber() & 1)
    return;
  int n = 0;
  for (int j = 15; j >= 0; j--) {
    if (sprite_state[j] && sprite_type[j] != 0x6c)
      n++;
  }
  if (n >= 4 || sram_progress_indicator < 2)
    return;
  int j = (overworld_secret_subst_ctr++ & 7) + (is_in_dark_world ? 8 : 0);
  if (!(kSecretSubst_Tab0[BYTE(overworld_area_index) & 0x3f] & kSecretSubst_Tab1[j]))
    BYTE(dung_secrets_unk1) = kSecretSubst_Tab2[j];
}

void Sprite_ApplyConveyor(int k, int j) {  // 9d8010
  if (!(frame_counter & 1))
    return;
  static const int8 kConveyorAdjustment_X[] = {0, 0, -1, 1};
  static const int8 kConveyorAdjustment_Y[] = {-1, 1, 0, 0};
  Sprite_SetX(k, Sprite_GetX(k) + kConveyorAdjustment_X[j - 0x68]);
  Sprite_SetY(k, Sprite_GetY(k) + kConveyorAdjustment_Y[j - 0x68]);
}

uint8 Sprite_BounceFromTileCollision(int k) {  // 9dc751
  int j = Sprite_CheckTileCollision(k);
  if (j & 3) {
    sprite_x_vel[k] = -sprite_x_vel[k];
    sprite_G[k]++;
  }
  if (j & 12) {
    sprite_y_vel[k] = -sprite_y_vel[k];
    sprite_G[k]++;
    return sprite_G[k]; // wtf
  }
  return 0;
}

void ExecuteCachedSprites() {  // 9de9da
  if (!player_is_indoors || submodule_index == 0 || submodule_index == 14 || alt_sprites_flag == 0) {
    alt_sprites_flag = 0;
    return;
  }
  for (int i = 15; i >= 0; i--) {
    cur_object_index = i;
    if (alt_sprite_state[i])
      UncacheAndExecuteSprite(i);
  }
}

void UncacheAndExecuteSprite(int k) {  // 9dea00
  uint8 bak0 = sprite_state[k];
  uint8 bak1 = sprite_type[k];
  uint8 bak2 = sprite_x_lo[k];
  uint8 bak3 = sprite_x_hi[k];
  uint8 bak4 = sprite_y_lo[k];
  uint8 bak5 = sprite_y_hi[k];
  uint8 bak6 = sprite_graphics[k];
  uint8 bak7 = sprite_A[k];
  uint8 bak8 = sprite_head_dir[k];
  uint8 bak9 = sprite_oam_flags[k];
  uint8 bak10 = sprite_obj_prio[k];
  uint8 bak11 = sprite_D[k];
  uint8 bak12 = sprite_flags2[k];
  uint8 bak13 = sprite_floor[k];
  uint8 bak14 = sprite_ai_state[k];
  uint8 bak15 = sprite_flags3[k];
  uint8 bak16 = sprite_B[k];
  uint8 bak17 = sprite_C[k];
  uint8 bak18 = sprite_E[k];
  uint8 bak19 = sprite_subtype2[k];
  uint8 bak20 = sprite_z[k];
  uint8 bak21 = sprite_delay_main[k];
  uint8 bak22 = sprite_I[k];
  uint8 bak23 = sprite_ignore_projectile[k];
  sprite_state[k] = alt_sprite_state[k];
  sprite_type[k] = alt_sprite_type[k];
  sprite_x_lo[k] = alt_sprite_x_lo[k];
  sprite_x_hi[k] = alt_sprite_x_hi[k];
  sprite_y_lo[k] = alt_sprite_y_lo[k];
  sprite_y_hi[k] = alt_sprite_y_hi[k];
  sprite_graphics[k] = alt_sprite_graphics[k];
  sprite_A[k] = alt_sprite_A[k];
  sprite_head_dir[k] = alt_sprite_head_dir[k];
  sprite_oam_flags[k] = alt_sprite_oam_flags[k];
  sprite_obj_prio[k] = alt_sprite_obj_prio[k];
  sprite_D[k] = alt_sprite_D[k];
  sprite_flags2[k] = alt_sprite_flags2[k];
  sprite_floor[k] = alt_sprite_floor[k];
  sprite_ai_state[k] = alt_sprite_spawned_flag[k];
  sprite_flags3[k] = alt_sprite_flags3[k];
  sprite_B[k] = alt_sprite_B[k];
  sprite_C[k] = alt_sprite_C[k];
  sprite_E[k] = alt_sprite_E[k];
  sprite_subtype2[k] = alt_sprite_subtype2[k];
  sprite_z[k] = alt_sprite_height_above_shadow[k];
  sprite_delay_main[k] = alt_sprite_delay_main[k];
  sprite_I[k] = alt_sprite_I[k];
  sprite_ignore_projectile[k] = alt_sprite_maybe_ignore_projectile[k];
  Sprite_ExecuteSingle(k);
  if (sprite_pause[k] != 0)
    alt_sprite_state[k] = 0;
  sprite_ignore_projectile[k] = bak23;
  sprite_I[k] = bak22;
  sprite_delay_main[k] = bak21;
  sprite_z[k] = bak20;
  sprite_subtype2[k] = bak19;
  sprite_E[k] = bak18;
  sprite_C[k] = bak17;
  sprite_B[k] = bak16;
  sprite_flags3[k] = bak15;
  sprite_ai_state[k] = bak14;
  sprite_floor[k] = bak13;
  sprite_flags2[k] = bak12;
  sprite_D[k] = bak11;
  sprite_obj_prio[k] = bak10;
  sprite_oam_flags[k] = bak9;
  sprite_head_dir[k] = bak8;
  sprite_A[k] = bak7;
  sprite_graphics[k] = bak6;
  sprite_y_hi[k] = bak5;
  sprite_y_lo[k] = bak4;
  sprite_x_hi[k] = bak3;
  sprite_x_lo[k] = bak2;
  sprite_type[k] = bak1;
  sprite_state[k] = bak0;

}

uint8 Sprite_ConvertVelocityToAngle(uint8 x, uint8 y) {  // 9df614
  static const uint8 kConvertVelocityToAngle_Tab0[32] = {
    0, 0, 1, 1, 1, 2, 2, 2, 0, 0, 15, 15, 15, 14, 14, 14,
    8, 8, 7, 7, 7, 6, 6, 6, 8, 8,  9,  9,  9, 10, 10, 10,
  };
  static const uint8 kConvertVelocityToAngle_Tab1[32] = {
    4, 4, 3, 3, 3, 2, 2, 2, 12, 12, 13, 13, 13, 14, 14, 14,
    4, 4, 5, 5, 5, 6, 6, 6, 12, 12, 11, 11, 11, 10, 10, 10,
  };
  int s = ((y >> 7) + (x >> 7) * 2) * 8;
  if (sign8(x)) x = -x;
  if (sign8(y)) y = -y;
  if (x >= y) {
    return kConvertVelocityToAngle_Tab0[(y >> 2) + s];
  } else {
    return kConvertVelocityToAngle_Tab1[(x >> 2) + s];
  }
}

int Sprite_SpawnDynamically(int k, uint8 what, SpriteSpawnInfo *info) {  // 9df65d
  return Sprite_SpawnDynamicallyEx(k, what, info, 15);
}

int Sprite_SpawnDynamicallyEx(int k, uint8 what, SpriteSpawnInfo *info, int j) {  // 9df65f
  do {
    if (sprite_state[j] == 0) {
      sprite_type[j] = what;
      sprite_state[j] = 9;
      info->r0_x = Sprite_GetX(k);
      info->r2_y = Sprite_GetY(k);
      info->r4_z = sprite_z[k];
      info->r5_overlord_x = overlord_x_lo[k] | overlord_x_hi[k] << 8;
      info->r7_overlord_y = overlord_y_lo[k] | overlord_y_hi[k] << 8;
      SpritePrep_LoadProperties(j);
      if (!player_is_indoors) {
        sprite_N_word[j] = 0xffff;
      } else {
        sprite_N[j] = 0xff;
      }
      sprite_floor[j] = sprite_floor[k];
      sprite_D[j] = sprite_D[k];
      sprite_die_action[j] = 0;
      sprite_subtype[j] = 0;
      break;
    }
  } while (--j >= 0);
  return j;
}

void SpriteFall_Draw(int k, PrepOamCoordsRet *info) {  // 9dffc5
  static const uint8 kSpriteFall_Char[8] = {0x83, 0x83, 0x83, 0x80, 0x80, 0x80, 0xb7, 0xb7};
  OamEnt *oam = GetOamCurPtr();
  oam->x = info->x + 4;
  oam->y = info->y + 4;
  oam->charnum = kSpriteFall_Char[sprite_delay_main[k] >> 2];
  oam->flags = info->flags & 0x30 | 0x04;
  Sprite_CorrectOamEntries(k, 0, 0);
}

void Sprite_GarnishSpawn_Sparkle_limited(int k, uint16 x, uint16 y) {  // 9ea001
  Sprite_SpawnSimpleSparkleGarnishEx(k, x, y, 14);
}

int Sprite_GarnishSpawn_Sparkle(int k, uint16 x, uint16 y) {  // 9ea007
  return Sprite_SpawnSimpleSparkleGarnishEx(k, x, y, 29);
}

void Sprite_BehaveAsBarrier(int k) {  // 9ef4f3
  uint8 bak = sprite_flags4[k];
  sprite_flags4[k] = 0;
  if (Sprite_CheckDamageToLink_same_layer(k))
    Sprite_HaltAllMovement();
  sprite_flags4[k] = bak;
}

void Sprite_HaltAllMovement() {  // 9ef508
  Sprite_NullifyHookshotDrag();
  link_speed_setting = 0;
  Link_CancelDash();
}

int ReleaseFairy() {  // 9efe33
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(0, 0xe3, &info);
  if (j >= 0) {
    sprite_floor[j] = link_is_on_lower_level;
    Sprite_SetX(j, link_x_coord + 8);
    Sprite_SetY(j, link_y_coord + 16);
    sprite_D[j] = 0;
    sprite_delay_aux4[j] = 96;
  }
  return j;
}

void Sprite_DrawRippleIfInWater(int k) {  // 9eff8d
  if (sprite_I[k] != 8 && sprite_I[k] != 9)
    return;

  if (sprite_flags3[k] & 0x20) {
    cur_sprite_x -= 4;
    if (sprite_type[k] == 0xdf)
      cur_sprite_y -= 7;
  }
  SpriteDraw_WaterRipple(k);
  Sprite_Get16BitCoords(k);
  Oam_AllocateFromRegionA(((sprite_flags2[k] & 0x1f) + 1) * 4);
}

