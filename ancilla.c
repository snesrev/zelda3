#include "ancilla.h"
#include "variables.h"
#include "sprite.h"
#include "hud.h"
#include "load_gfx.h"
#include "tagalong.h"
#include "overworld.h"
#include "tile_detect.h"
#include "player.h"
#include "misc.h"
#include "dungeon.h"
#include "sprite_main.h"
#include "assets.h"

static const uint8 kAncilla_Pflags[68] = {
  0,    8,  0xc, 0x10, 0x10,    4, 0x10, 0x18,    8,    8,    8,    0, 0x14, 0, 0x10, 0x28,
  0x18, 0x10, 0x10, 0x10, 0x10,  0xc,    8,    8, 0x50,    0, 0x10,    8, 0x40, 0,  0xc, 0x24,
  0x10,  0xc,    8, 0x10, 0x10,    4,  0xc, 0x1c,    0, 0x10, 0x14, 0x14, 0x10, 8, 0x20, 0x10,
  0x10, 0x10,    4,    0, 0x80, 0x10,    4, 0x30, 0x14, 0x10,    0, 0x10,    0, 0,    8,    0,
  0x10,    8, 0x78, 0x80,
};
static const int8 kFireRod_Xvel2[12] = {0, 0, -40, 40, 0, 0, -48, 48, 0, 0, -64, 64};
static const int8 kFireRod_Yvel2[12] = {-40, 40, 0, 0, -48, 48, 0, 0, -64, 64, 0, 0};
static const uint8 kTagalongLayerBits[4] = {0x20, 0x10, 0x30, 0x20};
static const uint8 kBombos_Sfx[8] = {0x80, 0x80, 0x80, 0, 0, 0x40, 0x40, 0x40};
const uint8 kBomb_Tab0[11] = {0xA0, 6, 4, 4, 4, 4, 4, 6, 6, 6, 6};

#define swordbeam_temp_x (*(uint16*)(g_ram+0x1580E))
#define swordbeam_temp_y (*(uint16*)(g_ram+0x15810))
#define swordbeam_arr ((uint8*)(g_ram+0x15800))
#define swordbeam_var1 (*(uint8*)(g_ram+0x15804))
#define swordbeam_var2 (*(uint8*)(g_ram+0x15808))
static const int8 kAncilla_TileColl_Attrs[256] = {
  0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 0, 0, 0, 0, 2, 2, 2, 2, 0, 3, 3, 3,
  0, 0, 0, 0, 0, 0, 1, 1, 4, 4, 4, 4, 4, 4, 4, 4,
  1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 3, 3, 3,
  0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 4,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};
static const uint8 kAncilla_TileColl0_Attrs[256] = {
  0, 1, 0, 3, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 0, 0, 0, 0, 2, 2, 2, 2, 0, 3, 3, 3,
  0, 0, 0, 0, 0, 0, 1, 1, 4, 4, 4, 4, 4, 4, 4, 4,
  1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 3, 3, 3,
  0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 4,
  0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0,
  0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 1,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};
static const uint8 kBomb_Draw_Tab0[12] = {0, 1, 2, 3, 2, 3, 4, 5, 6, 7, 8, 9};
static const uint8 kBomb_Draw_Tab2[11] = {1, 4, 4, 4, 4, 4, 5, 4, 6, 6, 6};

static const uint8 kMagicPowder_Tab0[40] = {
  13, 14, 15, 0,  1,  2,  3, 4, 5, 6, 10, 11, 12, 0, 1, 2,
  3,  4,  5, 6, 16, 17, 18, 0, 1, 2,  3,  4,  5, 6, 7, 8,
  9,  0,  1, 2,  3,  4,  5, 6,
};
#define ether_arr1 ((uint8*)(g_ram+0x15800))
#define ether_var2 (*(uint8*)(g_ram+0x15808))
#define ether_y2 (*(uint16*)(g_ram+0x1580A))
#define ether_y_adjusted (*(uint16*)(g_ram+0x1580C))
#define ether_x2 (*(uint16*)(g_ram+0x1580E))
#define ether_y3 (*(uint16*)(g_ram+0x15810))
#define ether_var1 (*(uint8*)(g_ram+0x15812))
#define ether_y (*(uint16*)(g_ram+0x15813))
#define ether_x (*(uint16*)(g_ram+0x15815))
static const uint8 kEther_BlitzOrb_Char[8] = {0x48, 0x48, 0x4a, 0x4a, 0x4c, 0x4c, 0x4e, 0x4e};
static const uint8 kEther_BlitzOrb_Flags[8] = {0x3c, 0x7c, 0x3c, 0x7c, 0x3c, 0x7c, 0x3c, 0x7c};
static const uint8 kEther_BlitzSegment_Char[4] = {0x40, 0x42, 0x44, 0x46};
#define bombos_arr1 ((uint8*)(g_ram+0x15800))
#define bombos_arr2 ((uint8*)(g_ram+0x15810))
static const uint8 kBombosBlasts_Tab[72] = {
  0xb6, 0x5d, 0xa1, 0x30, 0x69, 0xb5, 0xa3, 0x24, 0x96, 0xac, 0x73, 0x5f, 0x92, 0x48, 0x52, 0x81,
  0x39, 0x95, 0x7f, 0x20, 0x88, 0x5d, 0x34, 0x98, 0xbc, 0xd2, 0x51, 0x77, 0xa2, 0x47, 0x94, 0xb2,
  0x34, 0xda, 0x30, 0x62, 0x9f, 0x76, 0x51, 0x46, 0x98, 0x5c, 0x9b, 0x61, 0x58, 0x95, 0x4c, 0xba,
  0x7e, 0xcb, 0x12, 0xd0, 0x70, 0xa6, 0x46, 0xbf, 0x40, 0x50, 0x7e, 0x8c, 0x2d, 0x61, 0xac, 0x88,
  0x20, 0x6a, 0x72, 0x5f, 0xd2, 0x28, 0x52, 0x80,
};
static const uint8 kQuake_Tab1[5] = {0x17, 0x16, 0x17, 0x16, 0x10};
static const uint8 kQuakeDrawGroundBolts_Char[15] = { 0x40, 0x42, 0x44, 0x46, 0x48, 0x4a, 0x4c, 0x4e, 0x60, 0x62, 0x64, 0x66, 0x68, 0x6a, 0x63 };
typedef struct QuakeItem {
  int8 x, y;
  uint8 f;
} QuakeItem;
static const QuakeItem kQuakeItems[] = {
  {0, -16, 0x00},
  {0, -16, 0x01},
  {0, -16, 0x02},
  {0, -16, 0x03},
  {0, -16, 0x43},
  {0, -16, 0x42},
  {0, -16, 0x41},
  {0, -16, 0x40},
  {0, -16, 0x40}, {14, -8, 0x84},
  {29, -8, 0x44}, {13, -7, 0x84},
  {31, -7, 0x44}, {47, -4, 0x84},
  {49, -11, 0x06}, {63, -5, 0x44}, {47, -4, 0x84},
  {36, -17, 0x08}, {49, -11, 0x06}, {63, -5, 0x44}, {78, 4, 0x08},
  {22, -31, 0x08}, {36, -17, 0x08}, {78, 4, 0x08}, {93, 20, 0x08},
  {7, -46, 0x08}, {23, -45, 0x48}, {22, -31, 0x08}, {93, 20, 0x08}, {93, 36, 0x48},
  {-7, -61, 0x08}, {37, -59, 0x48}, {7, -46, 0x08}, {23, -45, 0x48}, {93, 36, 0x48}, {93, 52, 0x08},
  {-22, -75, 0x08}, {47, -74, 0x01}, {-8, -61, 0x08}, {36, -60, 0x48}, {93, 52, 0x08}, {108, 67, 0x08},
  {-37, -90, 0x08}, {-22, -75, 0x08}, {47, -74, 0x01}, {59, -62, 0x81}, {108, 67, 0x08}, {121, 80, 0x08},
  {-44, -104, 0xc9}, {-37, -90, 0x08}, {73, -74, 0x48}, {59, -62, 0x81}, {121, 80, 0x08},
  {-44, -120, 0x09}, {-44, -104, 0xc9}, {87, -89, 0x48}, {73, -74, 0x48},
  {-44, -120, 0x09}, {102, -104, 0x48}, {87, -89, 0x48},
  {102, -104, 0x48}, {87, -89, 0x48},
  {112, -116, 0x48}, {102, -104, 0x48},
  {112, -116, 0x48},
  {-13, -16, 0x00},
  {-13, -16, 0x01},
  {-13, -16, 0x02},
  {-13, -16, 0x03},
  {-11, -16, 0x43},
  {-11, -16, 0x42},
  {-11, -16, 0x41},
  {-11, -16, 0x40}, {-24, -10, 0x04},
  {-38, -18, 0x08}, {-24, -10, 0x04}, {-40, -7, 0xc4},
  {-45, -33, 0xc9}, {-38, -18, 0x08}, {-57, -7, 0x04}, {-40, -7, 0xc4},
  {-48, -45, 0x07}, {-45, -33, 0xc9}, {-57, -7, 0x04}, {-71, 2, 0x48},
  {-48, -45, 0x06}, {-71, 2, 0x48}, {-70, 18, 0x08},
  {-48, -45, 0x05}, {-70, 18, 0x08}, {-56, 33, 0x08},
  {-48, -45, 0x07}, {-54, 34, 0x08}, {-54, 49, 0x88},
  {-48, -45, 0x06}, {-54, 49, 0x88}, {-69, 64, 0x88},
  {-48, -45, 0x07}, {-69, 64, 0x88}, {-85, 73, 0xc4},
  {-48, -45, 0x05}, {-101, 73, 0x04}, {-85, 73, 0xc4},
  {-60, -53, 0x08}, {-48, -45, 0x06}, {-101, 73, 0x04}, {-116, 77, 0xc4},
  {-75, -67, 0x08}, {-60, -53, 0x08}, {-128, 76, 0x04}, {-116, 77, 0xc4},
  {-90, -82, 0x08}, {-75, -67, 0x08}, {-128, 76, 0x04},
  {-105, -97, 0x08}, {-90, -82, 0x08},
  {-120, -111, 0x08}, {-105, -97, 0x08},
  {-120, -111, 0x08},
  {0, -5, 0x0a},
  {0, -5, 0x0b},
  {2, -3, 0x0c},
  {1, -3, 0x0d},
  {0, -3, 0x8d},
  {1, -3, 0x8c},
  {1, -3, 0x8b},
  {1, -3, 0x8a}, {-6, 12, 0x89},
  {-6, 12, 0x89}, {-10, 28, 0xc9},
  {-10, 28, 0x49}, {-8, 44, 0x89},
  {-8, 44, 0x89}, {-10, 56, 0x02},
  {-10, 56, 0x02}, {-23, 70, 0x48}, {5, 70, 0x08},
  {-23, 70, 0x48}, {5, 70, 0x08}, {-38, 85, 0x48}, {19, 85, 0x08},
  {-38, 85, 0x48}, {19, 85, 0x08}, {-52, 99, 0x48}, {33, 101, 0x08},
  {-52, 99, 0x48}, {33, 101, 0x08}, {-66, 113, 0x48}, {47, 115, 0x08},
  {-66, 113, 0x48}, {47, 115, 0x08},
};
static const uint8 kQuakeItemPos[] = {
  0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 12, 14, 17, 21, 25, 30,
  36, 42, 48, 53, 57, 60, 62, 64, 65, 66, 67, 68, 69, 70, 71, 72, 74, 77, 81, 85, 88, 91, 94, 97, 100, 103, 107, 111, 114, 116, 118, 119, 120, 121, 122, 123, 124, 125, 126, 128, 130, 132, 134, 137, 141, 145, 149, 151
};
static const QuakeItem kQuakeItems2[] = {
  {-96, 112, 0x20},
  {-96, 112, 0x21},
  {-96, 112, 0x66},
  {-96, 112, 0x22},
  {-96, 112, 0x23},
  {-96, 112, 0x63},
  {-96, 112, 0x62},
  {-96, 112, 0x26},
  {-96, 112, 0x27}, {-86, 124, 0x28},
  {-86, 124, 0x28}, {-72, -117, 0x28},
  {-72, -117, 0x28}, {-59, -102, 0xa1},
  {-59, -102, 0xa1}, {-44, -116, 0x68},
  {-44, -116, 0x68}, {-29, 126, 0x68},
  {-29, 126, 0x68},
  {-19, 125, 0xc5},
  {-112, 96, 0x2a},
  {-112, 96, 0x2b},
  {-112, 96, 0x2c},
  {-112, 96, 0x2d},
  {-119, 82, 0x29}, {-112, 96, 0x2a},
  {-123, 66, 0xe9}, {-119, 82, 0x29},
  {-121, 50, 0x29}, {-123, 66, 0xe9},
  {126, 34, 0x28}, {-115, 34, 0x68}, {-121, 50, 0x29},
  {-106, 18, 0xa9}, {111, 19, 0x28}, {126, 34, 0x28}, {-115, 34, 0x68},
  {-100, 2, 0x68}, {102, 4, 0xe9}, {-106, 18, 0xa9}, {111, 19, 0x28},
  {-91, -14, 0xa9}, {95, -11, 0x28}, {-100, 2, 0x68}, {102, 4, 0xe9},
  {96, 112, 0x60},
  {96, 112, 0x61},
  {96, 112, 0x26},
  {96, 112, 0x62},
  {96, 112, 0x63},
  {96, 112, 0x23},
  {96, 112, 0x22},
  {96, 112, 0x66},
  {85, 111, 0xe8}, {96, 112, 0x67},
  {70, 104, 0x24}, {85, 111, 0xe8},
  {70, 104, 0x24}, {54, 108, 0xe4},
  {40, 100, 0x28}, {38, 107, 0x24}, {54, 108, 0xe4},
  {25, 85, 0x28}, {40, 100, 0x28}, {38, 107, 0x24}, {22, 110, 0xe4},
  {11, 70, 0x28}, {25, 85, 0x28}, {7, 108, 0x24}, {22, 110, 0xe4},
  {11, 70, 0x28}, {7, 108, 0x24},
  {112, 112, 0x2a},
  {112, 112, 0x2b},
  {112, 112, 0x2c},
  {112, 112, 0x2d},
  {112, 112, 0x2a}, {108, 125, 0x29},
  {108, 125, 0x29}, {114, -116, 0x28},
  {114, -116, 0x28}, {124, -100, 0x29},
  {124, -100, 0x29}, {123, -84, 0xe9},
  {123, -84, 0xe9}, {117, -74, 0xe4}, {-124, -69, 0x28},
  {117, -74, 0xe4}, {-124, -69, 0x28}, {103, -67, 0x68}, {-110, -54, 0x28},
  {103, -67, 0x68}, {-110, -54, 0x28}, {95, -52, 0x69}, {-102, -39, 0x29},
  {95, -52, 0x69}, {-102, -39, 0x29}, {96, -36, 0xe9}, {-102, -24, 0xe9},
  {96, -36, 0xe9}, {-102, -24, 0xe9},
  {-123, -14, 0x29}, {-115, -14, 0x2e}, {49, -12, 0x28},
};
static const uint8 kQuakeItemPos2[] = {
  0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 12, 14, 16, 18, 19, 20, 21, 22, 23, 24, 26, 28, 30, 33, 37, 41, 45, 46, 47, 48, 49, 50, 51, 52, 53, 55, 57, 59, 62, 66, 70, 72, 73, 74, 75, 76, 78, 80, 82, 84, 87, 91, 95, 99, 101, 104
};
static const uint8 kReceiveItem_Tab4[3] = {9, 5, 5};
static const uint8 kReceiveItem_Tab5[3] = {0x24, 0x25, 0x26};
static const uint8 kReceiveItem_Tab0[3] = {5, 1, 4};
static const int16 kReceiveItemMsgs[76] = {
  -1, 0x70, 0x77, 0x52,   -1, 0x78,  0x78, 0x62, 0x61, 0x66, 0x69, 0x53, 0x52, 0x56,   -1,  0x64,
  0x63, 0x65, 0x51, 0x54, 0x67, 0x68,  0x6b, 0x77, 0x79, 0x55, 0x6e, 0x58, 0x6d, 0x5d, 0x57,  0x5e,
  -1, 0x74, 0x75, 0x76,   -1, 0x5f, 0x158,   -1, 0x6a, 0x5c, 0x8f, 0x71, 0x72, 0x73, 0x71,  0x72,
  0x73, 0x6a, 0x6c, 0x60,   -1,   -1,    -1, 0x59, 0x84, 0x5a,   -1,   -1,   -1,   -1,   -1, 0x159,
  -1,   -1,   -1,   -1,   -1,   -1,    -1,   -1,   -1, 0xdb, 0x67, 0x7c,
};
static const int16 kReceiveItemMsgs2[2] = {0x5b, 0x83};
static const int16 kReceiveItemMsgs3[4] = {-1, 0x155, 0x156, 0x157};
static const uint8 kTravelBird_DmaStuffs[4] = {0, 0x20, 0x40, 0xe0};
static const int8 kTravelBird_Draw_X[3] = {0, -9, -9};
static const int8 kTravelBird_Draw_Y[3] = {0, 12, 20};
static const uint8 kTravelBird_Draw_Char[3] = {0xe, 0, 2};
static const uint8 kTravelBird_Draw_Flags[3] = {0x22, 0x2e, 0x2e};
static const int8 kSomarianBlock_Coll_X[12] = {0, 0, -8, 8, 0, 0, 0, 0, 8, -8, -8, 8};
static const int8 kSomarianBlock_Coll_Y[12] = {-8, 8, 0, 0, 0, 0, 0, 0, -8, 8, -8, 8};
static HandlerFuncK *const kAncilla_Funcs[67] = {
  &Ancilla01_SomariaBullet,
  &Ancilla02_FireRodShot,
  &Ancilla_Empty,
  &Ancilla04_BeamHit,
  &Ancilla05_Boomerang,
  &Ancilla06_WallHit,
  &Ancilla07_Bomb,
  &Ancilla08_DoorDebris,
  &Ancilla09_Arrow,
  &Ancilla0A_ArrowInTheWall,
  &Ancilla0B_IceRodShot,
  &Ancilla_SwordBeam,
  &Ancilla0D_SpinAttackFullChargeSpark,
  &Ancilla33_BlastWallExplosion,
  &Ancilla33_BlastWallExplosion,
  &Ancilla33_BlastWallExplosion,
  &Ancilla11_IceRodWallHit,
  &Ancilla33_BlastWallExplosion,
  &Ancilla13_IceRodSparkle,
  &Ancilla_Unused_14,
  &Ancilla15_JumpSplash,
  &Ancilla16_HitStars,
  &Ancilla17_ShovelDirt,
  &Ancilla18_EtherSpell,
  &Ancilla19_BombosSpell,
  &Ancilla1A_PowderDust,
  &Ancilla_SwordWallHit,
  &Ancilla1C_QuakeSpell,
  &Ancilla1D_ScreenShake,
  &Ancilla1E_DashDust,
  &Ancilla1F_Hookshot,
  &Ancilla20_Blanket,
  &Ancilla21_Snore,
  &Ancilla22_ItemReceipt,
  &Ancilla23_LinkPoof,
  &Ancilla24_Gravestone,
  &Ancilla_Unused_25,
  &Ancilla26_SwordSwingSparkle,
  &Ancilla27_Duck,
  &Ancilla28_WishPondItem,
  &Ancilla29_MilestoneItemReceipt,
  &Ancilla2A_SpinAttackSparkleA,
  &Ancilla2B_SpinAttackSparkleB,
  &Ancilla2C_SomariaBlock,
  &Ancilla2D_SomariaBlockFizz,
  &Ancilla2E_SomariaBlockFission,
  &Ancilla2F_LampFlame,
  &Ancilla30_ByrnaWindupSpark,
  &Ancilla31_ByrnaSpark,
  &Ancilla32_BlastWallFireball,
  &Ancilla33_BlastWallExplosion,
  &Ancilla34_SkullWoodsFire,
  &Ancilla35_MasterSwordReceipt,
  &Ancilla36_Flute,
  &Ancilla37_WeathervaneExplosion,
  &Ancilla38_CutsceneDuck,
  &Ancilla39_SomariaPlatformPoof,
  &Ancilla3A_BigBombExplosion,
  &Ancilla3B_SwordUpSparkle,
  &Ancilla3C_SpinAttackChargeSparkle,
  &Ancilla3D_ItemSplash,
  &Ancilla_RisingCrystal,
  &Ancilla3F_BushPoof,
  &Ancilla40_DwarfPoof,
  &Ancilla41_WaterfallSplash,
  &Ancilla42_HappinessPondRupees,
  &Ancilla43_GanonsTowerCutscene,
};
uint16 Ancilla_GetX(int k) {
  return ancilla_x_lo[k] | ancilla_x_hi[k] << 8;
}

uint16 Ancilla_GetY(int k) {
  return ancilla_y_lo[k] | ancilla_y_hi[k] << 8;
}

void Ancilla_SetX(int k, uint16 x) {
  ancilla_x_lo[k] = x;
  ancilla_x_hi[k] = x >> 8;
}

void Ancilla_SetY(int k, uint16 y) {
  ancilla_y_lo[k] = y;
  ancilla_y_hi[k] = y >> 8;
}

int Ancilla_AllocHigh() {
  for (int k = 9; k >= 0; k--) {
    if (ancilla_type[k] == 0)
      return k;
  }
  return -1;
}

static void Ancilla_SetOam(OamEnt *oam, uint16 x, uint16 y, uint8 charnum, uint8 flags, uint8 big) {
  uint8 yval = 0xf0;
  int xt = enhanced_features0 & kFeatures0_ExtendScreen64 ? 0x40 : 0;
  if ((uint16)(x + xt) < 256 + xt * 2 && y < 256) {
    big |= (x >> 8) & 1;
    oam->x = x;
    if (y < 0xf0)
      yval = y;
  }
  oam->y = yval;
  oam->charnum = charnum;
  oam->flags = flags;
  bytewise_extended_oam[oam - oam_buf] = big;
}

static void Ancilla_SetOam_Safe(OamEnt *oam, uint16 x, uint16 y, uint8 charnum, uint8 flags, uint8 big) {
  uint8 yval = 0xf0;
  oam->x = x;
  int xt = enhanced_features0 & kFeatures0_ExtendScreen64 ? 0x48 : 0;
  if ((uint16)(x + 0x80) < (0x180 + xt)) {
    big |= (x >> 8) & 1;
    if ((uint16)(y + 0x10) < 0x100)
      yval = y;
  }
  oam->y = yval;
  oam->charnum = charnum;
  oam->flags = flags;
  bytewise_extended_oam[oam - oam_buf] = big;
}

void Ancilla_Empty(int k) {
}

void Ancilla_Unused_14(int k) {
  assert(0);
}

void Ancilla_Unused_25(int k) {
  assert(0);
}

void SpinSpark_Draw(int k, int offs) {
  static const uint8 kInitialSpinSpark_Char[32] = {
    0x92, 0xff, 0xff, 0xff, 0x8c, 0x8c, 0x8c, 0x8c, 0xd6, 0xd6, 0xd6, 0xd6, 0x93, 0x93, 0x93, 0x93,
    0xd6, 0xd6, 0xd6, 0xd6, 0xd7, 0xff, 0xff, 0xff, 0x80, 0xff, 0xff, 0xff, 0x22, 0xff, 0xff, 0xff,  // wtf oob
  };
  static const uint8 kInitialSpinSpark_Flags[29] = {
    0x22, 0xff, 0xff, 0xff, 0x22, 0x62, 0xa2, 0xe2, 0x24, 0x64, 0xa4, 0xe4, 0x22, 0x62, 0xa2, 0xe2,
    0x22, 0x62, 0xa2, 0xe2, 0x22, 0xff, 0xff, 0xff, 0x22, 0xff, 0xff, 0xff,
    0xfc,
  };
  static const int8 kInitialSpinSpark_Y[29] = {
    -4,  0, 0, 0, -8, -8, 0, 0, -8, -8, 0, 0, -8, -8, 0, 0,
    -8, -8, 0, 0, -4,  0, 0, 0, -4,  0, 0, 0,
    -4,
  };
  static const int16 kInitialSpinSpark_X[29] = {
    -4, 0,  0, 0, -8, 0, -8, 0, -8, 0, -8, 0, -8, 0, -8, 0,
    -8, 0, -8, 0, -4, 0,  0, 0, -4, 0,  0, 0,
    0x11a5
  };
  Point16U info;
  Ancilla_PrepOamCoord(k, &info);
  OamEnt *oam = GetOamCurPtr();
  int t = (ancilla_item_to_link[k] + offs) * 4;
  assert(t < 32);
  for(int i = 0; i < 4; i++, t++) {
    if (kInitialSpinSpark_Char[t] != 0xff) {
      Ancilla_SetOam(oam, info.x + kInitialSpinSpark_X[t], info.y + kInitialSpinSpark_Y[t],
                     kInitialSpinSpark_Char[t], kInitialSpinSpark_Flags[t] & ~0x30 | HIBYTE(oam_priority_value), 0);
      oam++;
    }
  }
}

bool SomarianBlock_CheckEmpty(OamEnt *oam) {
  for (int i = 0; i != 4; i++) {
    if (oam[i].y == 0xf0)
      continue;
    for (i = 0; i < 4; i++)
      if (!(bytewise_extended_oam[oam + i - oam_buf] & 1))
        return false;
    break;
  }
  return true;
}

void AddDashingDustEx(uint8 a, uint8 y, uint8 flag) {
  static const int8 kAddDashingDust_X[4] = {4, 4, 6, 0};
  static const int8 kAddDashingDust_Y[4] = {20, 4, 16, 16};
  int k = Ancilla_AddAncilla(a, y);
  if (k >= 0) {
    ancilla_step[k] = flag;
    ancilla_item_to_link[k] = 0;
    ancilla_timer[k] = 3;
    int j = link_direction_facing >> 1;
    ancilla_dir[k] = j;
    if (!flag) {
      Ancilla_SetXY(k, link_x_coord, link_y_coord + 20);
    } else {
      Ancilla_SetXY(k, link_x_coord + kAddDashingDust_X[j], link_y_coord + kAddDashingDust_Y[j]);
    }
  }
}

void AddBirdCommon(int k) {
  ancilla_y_vel[k] = 0;
  ancilla_item_to_link[k] = 0;
  ancilla_aux_timer[k] = 1;
  ancilla_x_vel[k] = 56;
  ancilla_arr3[k] = 3;
  ancilla_K[k] = 0;
  ancilla_G[k] = 0;

  int xt = (enhanced_features0 & kFeatures0_ExtendScreen64) ? 0x40 : 0;
  Ancilla_SetXY(k, BG2HOFS_copy2 - 16 - xt, link_y_coord - 8);
}

ProjectSpeedRet Bomb_ProjectSpeedTowardsPlayer(int k, uint16 x, uint16 y, uint8 vel) {  // 84eb63
  uint16 old_x = Sprite_GetX(0), old_y = Sprite_GetY(0), old_z = sprite_z[0];
  Sprite_SetX(0, x);
  Sprite_SetY(0, y);
  sprite_z[0] = 0;
  ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLink(0, vel);
  sprite_z[0] = old_z;
  Sprite_SetX(0, old_x);
  Sprite_SetY(0, old_y);
  return pt;
}

void Boomerang_CheatWhenNoOnesLooking(int k, ProjectSpeedRet *pt) {  // 86809f
  uint16 x = link_x_coord - Ancilla_GetX(k) + 0xf0;
  uint16 y = link_y_coord - Ancilla_GetY(k) + 0xf0;
  if (x >= 0x1e0) {
    pt->x = sign16(x - 0x1e0) ? 0x90 : 0x70;
  } else if (y >= 0x1e0) {
    pt->y = sign16(y - 0x1e0) ? 0x90 : 0x70;
  }
}

void Medallion_CheckSpriteDamage(int k) {  // 86ec5c
  tmp_counter = ancilla_type[k];
  for (int j = 15; j >= 0; j--) {
    if (sprite_state[j] >= 9 && !(sprite_ignore_projectile[j] | sprite_pause[j])) {
      Ancilla_CheckDamageToSprite_aggressive(j, tmp_counter);
    }
  }
}

void Ancilla_CheckDamageToSprite(int k, uint8 type) {  // 86ecb7
  if (!sign8(sprite_hit_timer[k]))
    Ancilla_CheckDamageToSprite_aggressive(k, type);
}

void Ancilla_CheckDamageToSprite_aggressive(int k, uint8 type) {  // 86ecbd
  static const uint8 kAncilla_Damage[57] = {
    6, 1, 11, 0, 0, 0, 0, 8,  0,  6, 0, 12,  1, 0, 0,  0,
    0, 1,  0, 0, 0, 0, 0, 0, 14, 13, 0,  0, 15, 0, 0,  7,
    1, 1,  1, 1, 1, 1, 1, 1,  1,  1, 1,  1,  1, 1, 1, 11,
    0, 1,  1, 1, 1, 1, 1, 1,  1,
  };
  uint8 dmg = kAncilla_Damage[type];
  if (dmg == 6 && link_item_bow >= 3) {
    if (sprite_type[k] == 0xd7)
      sprite_delay_aux4[k] = 32;
    dmg = 9;
  }
  Ancilla_CheckDamageToSprite_preset(k, dmg);
}

void CallForDuckIndoors() {  // 87a45f
  Ancilla_Sfx2_Near(0x13);
  AncillaAdd_Duck_take_off(0x27, 4);
}

void Ancilla_Sfx1_Pan(int k, uint8 v) {  // 888020
  byte_7E0CF8 = v;
  sound_effect_ambient = v | Ancilla_CalculateSfxPan(k);
}

void Ancilla_Sfx2_Pan(int k, uint8 v) {  // 888027
  byte_7E0CF8 = v;
  sound_effect_1 = v | Ancilla_CalculateSfxPan(k);
}

void Ancilla_Sfx3_Pan(int k, uint8 v) {  // 88802e
  byte_7E0CF8 = v;
  sound_effect_2 = v | Ancilla_CalculateSfxPan(k);
}

void AncillaAdd_FireRodShot(uint8 type, uint8 y) {  // 8880b3
  static const int8 kFireRod_X[4] = {0, 0, -8, 16};
  static const int8 kFireRod_Y[4] = {-8, 16, 3, 3};
  static const int8 kFireRod_Xvel[4] = {0, 0, -64, 64};
  static const int8 kFireRod_Yvel[4] = {-64, 64, 0, 0};

  y = 1;
  int j= Ancilla_AllocInit(type, 1);
  if (j < 0) {
    if (type != 1)
      Refund_Magic(0);
    return;
  }

  if (type != 1)
    Ancilla_Sfx2_Near(0xe);

  ancilla_type[j] = type;
  ancilla_numspr[j] = kAncilla_Pflags[type];
  ancilla_timer[j] = 3;
  ancilla_step[j] = 0;
  ancilla_item_to_link[j] = 0;
  ancilla_objprio[j] = 0;
  ancilla_U[j] = 0;
  int i = link_direction_facing >> 1;
  ancilla_dir[j] = i;

  if (Ancilla_CheckInitialTile_A(j) < 0) {
    Ancilla_SetXY(j, link_x_coord + kFireRod_X[i], link_y_coord + kFireRod_Y[i]);
    if (type != 1) {
      ancilla_x_vel[j] = kFireRod_Xvel[i];
      ancilla_y_vel[j] = kFireRod_Yvel[i];
    } else {
      i += (link_sword_type - 2) * 4;
      ancilla_x_vel[j] = kFireRod_Xvel2[i];
      ancilla_y_vel[j] = kFireRod_Yvel2[i];
    }
    ancilla_floor[j] = link_is_on_lower_level;
    ancilla_floor2[j] = link_is_on_lower_level_mirror;
  } else {
    if (type == 1) {
      ancilla_type[j] = 4;
      ancilla_timer[j] = 7;
      ancilla_numspr[j] = 16;
    } else {
      ancilla_step[j] = 1;
      ancilla_timer[j] = 31;
      ancilla_numspr[j] = 8;
      j = link_direction_facing >> 1; // wtf
      Ancilla_Sfx2_Pan(j, 0x2a);
    }
  }
}

void SomariaBlock_SpawnBullets(int k) {  // 8881a7
  static const int8 kSpawnCentrifugalQuad_X[4] = {-8, -8, -9, -4};
  static const int8 kSpawnCentrifugalQuad_Y[4] = {-15, -4, -8, -8};

  uint8 z = (ancilla_z[k] == 0xff) ? 0 : ancilla_z[k];
  uint16 x = Ancilla_GetX(k);
  uint16 y = Ancilla_GetY(k) - z;

  for (int i = 3; i >= 0; i--) {
    int j = Ancilla_AllocInit(1, 4);
    if (j >= 0) {
      ancilla_type[j] = 0x1;
      ancilla_numspr[j] = kAncilla_Pflags[0x1];
      ancilla_step[j] = 4;
      ancilla_item_to_link[j] = 0;
      ancilla_objprio[j] = 0;
      ancilla_dir[j] = i;
      Ancilla_SetXY(j, x + kSpawnCentrifugalQuad_X[i], y + kSpawnCentrifugalQuad_Y[i]);
      Ancilla_TerminateIfOffscreen(j);
      ancilla_x_vel[j] = kFireRod_Xvel2[i];
      ancilla_y_vel[j] = kFireRod_Yvel2[i];
      ancilla_floor[j] = ancilla_floor[k];
      ancilla_floor2[j] = link_is_on_lower_level_mirror;
    }
  }
  tmp_counter = 0xff;
}

void Ancilla_Main() {  // 888242
  Ancilla_WeaponTink();
  Ancilla_ExecuteAll();
}

ProjectSpeedRet Ancilla_ProjectReflexiveSpeedOntoSprite(int k, uint16 x, uint16 y, uint8 vel) {  // 88824d
  uint16 old_x = link_x_coord, old_y = link_y_coord;
  link_x_coord = x;
  link_y_coord = y;
  ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLink(k, vel);
  link_x_coord = old_x;
  link_y_coord = old_y;
  return pt;
}

void Bomb_CheckSpriteDamage(int k) {  // 888287
  for (int j = 15; j >= 0; j--) {
    if ((j ^ frame_counter) & 3 | sprite_hit_timer[j] | sprite_ignore_projectile[j])
      continue;
    if (sprite_floor[j] != ancilla_floor[k] || sprite_state[j] < 9)
      continue;
    SpriteHitBox hb;
    int ax = Ancilla_GetX(k) - 24;
    int ay = Ancilla_GetY(k) - 24 - ancilla_z[k];
    hb.r0_xlo = ax;
    hb.r8_xhi = ax >> 8;
    hb.r1_ylo = ay;
    hb.r9_yhi = ay >> 8;
    hb.r2 = 48;
    hb.r3 = 48;
    Sprite_SetupHitBox(j, &hb);
    if (!CheckIfHitBoxesOverlap(&hb))
      continue;
    if (sprite_type[j] == 0x92 && sprite_C[j] >= 3)
      continue;
    Ancilla_CheckDamageToSprite(j, ancilla_type[k]);
    ProjectSpeedRet pt = Ancilla_ProjectReflexiveSpeedOntoSprite(j, Ancilla_GetX(k), Ancilla_GetY(k), 64);
    sprite_x_recoil[j] = -pt.x;
    sprite_y_recoil[j] = -pt.y;
  }
}

void Ancilla_ExecuteAll() {  // 88832b
  for (int i = 9; i >= 0; i--) {
    cur_object_index = i;
    if (ancilla_type[i])
      Ancilla_ExecuteOne(ancilla_type[i], i);
  }
}

void Ancilla_ExecuteOne(uint8 type, int k) {  // 88833c
  if (k < 6) {
    ancilla_oam_idx[k] = Ancilla_AllocateOamFromRegion_A_or_D_or_F(k, ancilla_numspr[k]);
  }

  if (submodule_index == 0 && ancilla_timer[k] != 0)
    ancilla_timer[k]--;

  kAncilla_Funcs[type - 1](k);
}

void Ancilla13_IceRodSparkle(int k) {  // 888435
  static const uint8 kIceShotSparkle_X[16] = {2, 7, 6, 1, 1, 7, 7, 1, 0, 7, 8, 1, 4, 9, 4, 0xff};
  static const uint8 kIceShotSparkle_Y[16] = {2, 3, 8, 7, 1, 1, 7, 7, 1, 0, 7, 8, 0xff, 4, 9, 4};
  static const uint8 kIceShotSparkle_Char[16] = {0x83, 0x83, 0x83, 0x83, 0xb6, 0x80, 0xb6, 0x80, 0xb7, 0xb6, 0xb7, 0xb6, 0xb7, 0xb6, 0xb7, 0xb6};

  if (!ancilla_timer[k])
    ancilla_type[k] = 0;
  if (!submodule_index) {
    Ancilla_MoveX(k);
    Ancilla_MoveY(k);
  }
  AncillaOamInfo info;
  if (Ancilla_ReturnIfOutsideBounds(k, &info))
    return;

  int j;
  for (j = 4; j >= 0 && ancilla_type[j] != 0xb; j--) {}
  if (j >= 0 && ancilla_objprio[j])
    info.flags = 0x30;

  if (sort_sprites_setting) {
    if (ancilla_floor[k])
      Oam_AllocateFromRegionE(0x10);
    else
      Oam_AllocateFromRegionD(0x10);
  } else {
    Oam_AllocateFromRegionA(0x10);
  }

  OamEnt *oam = GetOamCurPtr();
  j = ancilla_timer[k] & 0x1c;
  for (int i = 3; i >= 0; i--, oam++)
    SetOamPlain(oam, info.x + kIceShotSparkle_X[i + j], info.y + kIceShotSparkle_Y[i + j], kIceShotSparkle_Char[i + j], info.flags | 4, 0);
}

void AncillaAdd_IceRodSparkle(int k) {  // 8884c8
  static const int8 kIceShotSparkle_Xvel[4] = {0, 0, -4, 4};
  static const int8 kIceShotSparkle_Yvel[4] = {-4, 4, 0, 0};

  if (submodule_index || !sign8(--ancilla_arr4[k]))
    return;

  ancilla_arr4[k] = 5;
  int j = Ancilla_AllocHigh();
  if (j >= 0) {
    ancilla_type[j] = 0x13;
    ancilla_timer[j] = 15;

    int i = ancilla_dir[k];
    ancilla_x_vel[j] = kIceShotSparkle_Xvel[i];
    ancilla_y_vel[j] = kIceShotSparkle_Yvel[i];

    ancilla_x_lo[j] = ancilla_x_lo[k];
    ancilla_y_lo[j] = ancilla_y_lo[k];
    ancilla_floor[j] = ancilla_floor[k];
    ancilla_numspr[j] = 0;
  }

}

void Ancilla01_SomariaBullet(int k) {  // 88851b
  static const uint8 kSomarianBlast_Mask[6] = {7, 3, 1, 0, 0, 0};

  if (!submodule_index) {
    if (!(frame_counter & kSomarianBlast_Mask[ancilla_step[k]])) {
      Ancilla_MoveX(k);
      Ancilla_MoveY(k);
    }
    if (ancilla_timer[k] == 0) {
      ancilla_timer[k] = 3;
      uint8 a = ancilla_step[k] + 1;
      if (a >= 6)
        a = 4;
      ancilla_step[k] = a;
    }
    if (Ancilla_CheckSpriteCollision(k) >= 0 || Ancilla_CheckTileCollision_staggered(k)) {
      ancilla_type[k] = 4;
      ancilla_timer[k] = 7;
      ancilla_numspr[k] = 16;
    }
  }
  SomarianBlast_Draw(k);
}

bool Ancilla_ReturnIfOutsideBounds(int k, AncillaOamInfo *info) {  // 88862a
  static const uint8 kAncilla_FloorFlags[2] = {0x20, 0x10};
  info->flags = kAncilla_FloorFlags[ancilla_floor[k]];
  if ((info->x = ancilla_x_lo[k] - BG2HOFS_copy2) >= 0xf4 ||
      (info->y = ancilla_y_lo[k] - BG2VOFS_copy2) >= 0xf0) {
    ancilla_type[k] = 0;
    return true;
  }
  return false;
}

void SomarianBlast_Draw(int k) {  // 888650
  static const uint8 kSomarianBlast_Flags[2] = {2, 6};
  AncillaOamInfo info;
  if (Ancilla_ReturnIfOutsideBounds(k, &info))
    return;
  info.flags |= kSomarianBlast_Flags[ancilla_item_to_link[k]];
  if (ancilla_objprio[k])
    info.flags |= 0x30;
  static const int8 kSomarianBlast_Draw_X0[24] = {
    0, 0, 0, 0, 4, 4, 0, 0, 0, 0, 4, 4, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
  };
  static const int8 kSomarianBlast_Draw_X1[24] = {
    8, 8, 8, 8, 4, 4, 8, 8, 8, 8, 4, 4, 0, 0, 0, 0,
    8, 8, 0, 0, 0, 0, 8, 8,
  };
  static const uint8 kSomarianBlast_Draw_Y0[24] = {
    0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    4, 4, 0, 0, 0, 0, 4, 4,
  };
  static const uint8 kSomarianBlast_Draw_Y1[24] = {
    0, 0,    0, 0, 8, 8, 0x80, 0, 0, 0, 8, 8, 0x80, 8, 8, 8,
    4, 4, 0x80, 8, 8, 8,    4, 4,
  };
  static const uint8 kSomarianBlast_Draw_Flags0[24] = {
    0xc0, 0xc0, 0xc0, 0xc0, 0x80, 0xc0, 0x40, 0x40, 0x40, 0x40, 0, 0x40, 0x40, 0x40, 0x40, 0x40,
    0x40, 0xc0,    0,    0,    0,    0,    0, 0x80,
  };
  static const uint8 kSomarianBlast_Draw_Flags1[24] = {
    0x80, 0x80, 0x80, 0x80, 0x80, 0xc0, 0,    0, 0, 0, 0, 0x40, 0xc0, 0xc0, 0xc0, 0xc0,
    0x40, 0xc0, 0x80, 0x80, 0x80, 0x80, 0, 0x80,
  };
  static const uint8 kSomarianBlast_Draw_Char0[24] = {
    0x50, 0x50, 0x44, 0x44, 0x52, 0x52, 0x50, 0x50, 0x44, 0x44, 0x51, 0x51, 0x43, 0x43, 0x42, 0x42,
    0x41, 0x41, 0x43, 0x43, 0x42, 0x42, 0x40, 0x40,
  };
  static const uint8 kSomarianBlast_Draw_Char1[24] = {
    0x50, 0x50, 0x44, 0x44, 0x51, 0x51, 0x50, 0x50, 0x44, 0x44, 0x52, 0x52, 0x43, 0x43, 0x42, 0x42,
    0x40, 0x40, 0x43, 0x43, 0x42, 0x42, 0x41, 0x41,
  };
  OamEnt *oam = GetOamCurPtr();
  int j = ancilla_dir[k] * 6 + ancilla_step[k];
  SetOamPlain(oam + 0, info.x + kSomarianBlast_Draw_X0[j], sign8(kSomarianBlast_Draw_Y0[j]) ? 0xf0 : info.y + kSomarianBlast_Draw_Y0[j],
              0x82 + kSomarianBlast_Draw_Char0[j], info.flags | kSomarianBlast_Draw_Flags0[j], 0);
  SetOamPlain(oam + 1, info.x + kSomarianBlast_Draw_X1[j], sign8(kSomarianBlast_Draw_Y1[j]) ? 0xf0 : info.y + kSomarianBlast_Draw_Y1[j],
              0x82 + kSomarianBlast_Draw_Char1[j], info.flags | kSomarianBlast_Draw_Flags1[j], 0);
}

void Ancilla02_FireRodShot(int k) {  // 8886d2
  if (ancilla_step[k] == 0) {
    if (!submodule_index) {
      ancilla_L[k] = 0;
      Ancilla_MoveX(k);
      Ancilla_MoveY(k);
      uint8 coll = Ancilla_CheckSpriteCollision(k) >= 0;
      if (!coll) {
        ancilla_dir[k] |= 8;
        coll = Ancilla_CheckTileCollision(k);
        ancilla_L[k] = ancilla_tile_attr[k];
        if (!coll) {
          ancilla_dir[k] |= 12;
          uint8 bak = ancilla_U[k];
          coll = Ancilla_CheckTileCollision(k);
          ancilla_U[k] = bak;
        }
      }
      if (coll) {
        ancilla_step[k]++;
        ancilla_timer[k] = 31;
        ancilla_numspr[k] = 8;
        Ancilla_Sfx2_Pan(k, 0x2a);
      }
      ancilla_item_to_link[k]++;
      ancilla_dir[k] &= ~0xC;
      if (((byte_7E0333 = ancilla_L[k]) & 0xf0) == 0xc0 || ((byte_7E0333 = ancilla_tile_attr[k]) & 0xf0) == 0xc0)
        Dungeon_LightTorch();
    }
    FireShot_Draw(k);
  } else {
    AncillaOamInfo info;
    Ancilla_CheckBasicSpriteCollision(k);
    if (Ancilla_ReturnIfOutsideBounds(k, &info))
      return;
    OamEnt *oam = GetOamCurPtr();
    if (!ancilla_timer[k]) {
      uint8 old_type = ancilla_type[k];
      ancilla_type[k] = 0;
      if (old_type != 0x2f && BYTE(overworld_screen_index) == 64 && ancilla_tile_attr[k] == 0x43)
        FireRodShot_BecomeSkullWoodsFire(k);
      return;
    }
    int j = ancilla_timer[k] >> 3;
    if (j != 0) {
      static const uint8 kFireShot_Draw_Char[3] = {0xa2, 0xa0, 0x8e};
      SetOamPlain(oam, info.x, info.y, kFireShot_Draw_Char[j - 1], info.flags | 2, 2);
    } else {
      SetOamPlain(oam + 0, info.x + 0, info.y - 3, 0xa4, info.flags | 2, 0);
      SetOamPlain(oam + 1, info.x + 8, info.y - 3, 0xa5, info.flags | 2, 0);
    }
  }
}

void FireShot_Draw(int k) {  // 88877c
  static const uint8 kFireShot_Draw_X2[16] = {7, 0, 8, 0, 8, 4, 0, 0, 2, 8, 0, 0, 1, 4, 9, 0};
  static const uint8 kFireShot_Draw_Y2[16] = {1, 4, 9, 0, 7, 0, 8, 0, 8, 4, 0, 0, 2, 8, 0, 0};
  static const uint8 kFireShot_Draw_Char2[3] = {0x8d, 0x9d, 0x9c};
  AncillaOamInfo info;
  if (Ancilla_ReturnIfOutsideBounds(k, &info))
    return;
  if (ancilla_objprio[k])
    info.flags |= 0x30;

  OamEnt *oam = GetOamCurPtr();
  int j = ancilla_item_to_link[k] & 0xc;
  for (int i = 2; i >= 0; i--, oam++)
    SetOamPlain(oam, info.x + kFireShot_Draw_X2[j + i], info.y + kFireShot_Draw_Y2[j + i], kFireShot_Draw_Char2[i], info.flags | 2, 0);
}

uint8 Ancilla_CheckTileCollision_staggered(int k) {  // 88897b
  if ((frame_counter ^ k) & 1)
    return Ancilla_CheckTileCollision(k);
  return 0;
}

uint8 Ancilla_CheckTileCollision(int k) {  // 888981
  if (!player_is_indoors && ancilla_objprio[k]) {
    ancilla_tile_attr[k] = 0;
    return 0;
  }
  if (!dung_hdr_collision)
    return Ancilla_CheckTileCollisionOneFloor(k);
  uint16 x = 0, y = 0;
  if (dung_hdr_collision < 3) {
    x = BG1HOFS_copy2 - BG2HOFS_copy2;
    y = BG1VOFS_copy2 - BG2VOFS_copy2;
  }
  uint16 oldx = Ancilla_GetX(k), oldy = Ancilla_GetY(k);
  Ancilla_SetX(k, oldx + x);
  Ancilla_SetY(k, oldy + y);
  ancilla_floor[k] = 1;
  uint8 b = Ancilla_CheckTileCollisionOneFloor(k);
  ancilla_floor[k] = 0;
  Ancilla_SetX(k, oldx);
  Ancilla_SetY(k, oldy);
  return (b << 1) | (uint8)Ancilla_CheckTileCollisionOneFloor(k);
}

bool Ancilla_CheckTileCollisionOneFloor(int k) {  // 888a03
  static const int8 kAncilla_CheckTileColl0_X[20] = {
    8, 8, 0, 16, 4, 4, 0, 16, 4, 4, 4, 12, 12, 12, 4, 12, 0, 0, 0, 0,
  };
  static const int8 kAncilla_CheckTileColl0_Y[20] = {
    0, 16, 5, 5, 0, 16, 4, 4, 4, 12, 5, 5, 4, 12, 12, 12, 0, 0, 0, 0,
  };
  uint16 x = Ancilla_GetX(k) + kAncilla_CheckTileColl0_X[ancilla_dir[k]];
  uint16 y = Ancilla_GetY(k) + kAncilla_CheckTileColl0_Y[ancilla_dir[k]];
  return Ancilla_CheckTileCollision_targeted(k, x, y);
}

bool Ancilla_CheckTileCollision_targeted(int k, uint16 x, uint16 y) {  // 888a26
  if ((uint16)(y - BG2VOFS_copy2) >= 224 || (uint16)(x - BG2HOFS_copy2) >= 256)
    return 0;
  uint8 tile_attr;
  if (!player_is_indoors) {
    x >>= 3;
    tile_attr = Overworld_GetTileAttributeAtLocation(x, y);
  } else {
    tile_attr = GetTileAttribute(ancilla_floor[k], &x, y);
  }

  ancilla_tile_attr[k] = tile_attr;
  if (tile_attr == 3 && ancilla_floor2[k])
    return 0;

  uint8 t = kAncilla_TileColl0_Attrs[tile_attr];

  if (ancilla_type[k] == 2 && (tile_attr & 0xf0) == 0xc0)
    t = 0;

  if (!ancilla_objprio[k]) {
    if (t == 0)
      return false;
    if (t == 1)
      goto return_true_set_alert;
    if (t == 2)
      return Entity_CheckSlopedTileCollision(x, y);
    if (t == 3) {
      if (ancilla_floor2[k])
        goto return_true_set_alert;
      return 0;
    }
  }
  if (sign8(--ancilla_U[k])) {
    ancilla_U[k] = 0;
    if (t == 4) {
      ancilla_U[k] = 6;
      ancilla_objprio[k] ^= 1;
    }
  }
  return 0;

return_true_set_alert:
  sprite_alert_flag = 3;
  return 1;
}

bool Ancilla_CheckTileCollision_Class2(int k) {  // 888bcf
  if (!dung_hdr_collision)
    return Ancilla_CheckTileCollision_Class2_Inner(k);
  uint16 x = 0, y = 0;
  if (dung_hdr_collision < 3) {
    x = BG1HOFS_copy2 - BG2HOFS_copy2;
    y = BG1VOFS_copy2 - BG2VOFS_copy2;
  }
  uint16 oldx = Ancilla_GetX(k), oldy = Ancilla_GetY(k);
  Ancilla_SetX(k, oldx + x);
  Ancilla_SetY(k, oldy + y);
  ancilla_floor[k] = 1;
  bool b = Ancilla_CheckTileCollision_Class2_Inner(k);
  ancilla_floor[k] = 0;
  Ancilla_SetX(k, oldx);
  Ancilla_SetY(k, oldy);
  return (b | Ancilla_CheckTileCollision_Class2_Inner(k)) != 0;
}

bool Ancilla_CheckTileCollision_Class2_Inner(int k) {  // 888c43
  static const int8 kAncilla_CheckTileColl_Y[4] = {-8, 8, 0, 0};
  static const int8 kAncilla_CheckTileColl_X[4] = {0, 0, -8, 8};

  uint16 x = Ancilla_GetX(k) + kAncilla_CheckTileColl_X[ancilla_dir[k]];
  uint16 y = Ancilla_GetY(k) + kAncilla_CheckTileColl_Y[ancilla_dir[k]];

  if ((uint16)(y - BG2VOFS_copy2) >= 224 || (uint16)(x - BG2HOFS_copy2) >= 256)
    return false;
  uint8 tile_attr;
  if (!player_is_indoors) {
    x >>= 3;
    tile_attr = Overworld_GetTileAttributeAtLocation(x, y);
  } else {
    tile_attr = GetTileAttribute(ancilla_floor[k], &x, y);
  }

  ancilla_tile_attr[k] = tile_attr;
  if (tile_attr == 3 && ancilla_floor2[k])
    return false;

  uint8 t = kAncilla_TileColl_Attrs[tile_attr];
  if (t == 0)
    return false;
  if (t == 2)
    return Entity_CheckSlopedTileCollision(x, y);
  if (t == 4) {
    if (ancilla_floor2[k])
      return true;
    ancilla_objprio[k] = 1;
    return false;
  }
  if (t == 3)
    return ancilla_floor2[k] != 0;
  return true;
}

void Ancilla04_BeamHit(int k) {  // 888d19
  static const int8 kBeamHit_X[16] = {-12, 20, -12, 20, -8, 16, -8, 16, -4, 12, -4, 12, 0, 8, 0, 8};
  static const int8 kBeamHit_Y[16] = {-12, -12, 20, 20, -8, -8, 16, 16, -4, -4, 12, 12, 0, 0, 8, 8};
  static const uint8 kBeamHit_Char[16] = {0x53, 0x53, 0x53, 0x53, 0x53, 0x53, 0x53, 0x53, 0x53, 0x53, 0x53, 0x53, 0x54, 0x54, 0x54, 0x54};
  static const uint8 kBeamHit_Flags[16] = {0x40, 0, 0xc0, 0x80, 0x40, 0, 0xc0, 0x80, 0x40, 0, 0xc0, 0x80, 0, 0x40, 0x80, 0xc0};
  AncillaOamInfo info;
  if (Ancilla_ReturnIfOutsideBounds(k, &info))
    return;
  if (!ancilla_timer[k]) {
    ancilla_type[k] = 0;
    return;
  }
  OamEnt *oam = GetOamCurPtr();
  int j = ancilla_timer[k] >> 1;
  uint16 ancilla_x = Ancilla_GetX(k);
  uint16 ancilla_y = Ancilla_GetY(k);
  uint8 r7 = ancilla_x - BG2HOFS_copy2;
  uint8 r6 = ancilla_y - BG2VOFS_copy2;
  for (int i = 3; i >= 0; i--, oam++) {
    int m = j * 4 + i;
    uint8 x = info.x + kBeamHit_X[m];
    uint8 y = info.y + kBeamHit_Y[m];
    uint16 x_adj = (uint16)(ancilla_x + (int8)(x - r7) - BG2HOFS_copy2);
    uint16 y_adj = (uint16)(ancilla_y + (int8)(y - r6) - BG2VOFS_copy2 + 0x10);
    SetOamPlain(oam, x, (y_adj >= 0x100) ? 0xf0 : y,
                kBeamHit_Char[m] + 0x82, kBeamHit_Flags[m] | 2 | info.flags, (x_adj >= 0x100) ? 1 : 0);
  }
}

int Ancilla_CheckSpriteCollision(int k) {  // 888d68
  for (int j = 15; j >= 0; j--) {
    if (ancilla_type[k] == 9 || ancilla_type[k] == 0x1f || ((j ^ frame_counter) & 3 | sprite_pause[j]) == 0) {
      if ((sprite_state[j] >= 9 && (sprite_defl_bits[j] & 2 || !ancilla_objprio[k])) && ancilla_floor[k] == sprite_floor[j]) {
        if (Ancilla_CheckSpriteCollision_Single(k, j))
          return j;
      }
    }
  }
  return -1;
}

bool Ancilla_CheckSpriteCollision_Single(int k, int j) {  // 888dae
  int i;
  SpriteHitBox hb;
  Ancilla_SetupHitBox(k, &hb);

  Sprite_SetupHitBox(j, &hb);
  if (!CheckIfHitBoxesOverlap(&hb))
    return false;

  bool return_value = true;
  if (sprite_flags[j] & 8 && ancilla_type[k] == 9) {
    if (sprite_type[j] != 0x1b) {
      Sprite_CreateDeflectedArrow(k);
      return false;
    }
    if (link_item_bow < 3) {
      Sprite_CreateDeflectedArrow(k);
    } else {
      return_value = false;
    }
  }

  if (sprite_defl_bits[j] & 0x10) {
    static const uint8 kAncilla_CheckSpriteColl_Dir[4] = {2, 3, 0, 1};
    ancilla_dir[k] &= 3;
    if (ancilla_dir[k] == kAncilla_CheckSpriteColl_Dir[ancilla_dir[k]])
      goto return_true_set_alert;
  }

  if (ancilla_type[k] == 5 || ancilla_type[k] == 0x1f) {
    if (ancilla_type[k] == 0x1f && sprite_type[j] == 0x8d)
      goto skip;
    if (sprite_hit_timer[j])
      goto return_true_set_alert;
    if (sprite_defl_bits[j] & 2) {
skip:
      sprite_B[j] = k + 1;
      sprite_unk2[j] = ancilla_type[k];
      goto return_true_set_alert;
    }
  }

  if (!sprite_ignore_projectile[j]) {
    static const int8 kAncilla_CheckSpriteColl_RecoilX[4] = {0, 0, -64, 64};
    static const int8 kAncilla_CheckSpriteColl_RecoilY[4] = {-64, 64, 0, 0};
    if (sprite_type[j] == 0x92 && sprite_C[j] < 3)
      goto return_true_set_alert;
    i = ancilla_dir[k] & 3;
    sprite_x_recoil[j] = kAncilla_CheckSpriteColl_RecoilX[i];
    sprite_y_recoil[j] = kAncilla_CheckSpriteColl_RecoilY[i];
    byte_7E0FB6 = k;
    Ancilla_CheckDamageToSprite(j, ancilla_type[k]);
return_true_set_alert:
    sprite_unk2[j] = ancilla_type[k];
    sprite_alert_flag = 3;
    return return_value;
  }
  return false;
}

void Ancilla_SetupHitBox(int k, SpriteHitBox *hb) {  // 888ead
  static const int8 kAncilla_HitBox_X[12] = {4, 4, 4, 4, 3, 3, 2, 11, -16, -16, -1, -8};
  static const int8 kAncilla_HitBox_Y[12] = {4, 4, 4, 4, 2, 11, 3, 3, -1, -8, -16, -16};
  static const uint8 kAncilla_HitBox_W[12] = {8, 8, 8, 8, 1, 1, 1, 1, 32, 32, 8, 8};
  static const uint8 kAncilla_HitBox_H[12] = {8, 8, 8, 8, 1, 1, 1, 1, 8, 8, 32, 32};
  int j = ancilla_dir[k];
  if (ancilla_type[k] == 0xc)
    j |= 8;
  int x = Ancilla_GetX(k) + kAncilla_HitBox_X[j];
  hb->r0_xlo = x;
  hb->r8_xhi = x >> 8;
  int y = Ancilla_GetY(k) + kAncilla_HitBox_Y[j];
  hb->r1_ylo = y;
  hb->r9_yhi = y >> 8;
  hb->r2 = kAncilla_HitBox_W[j];
  hb->r3 = kAncilla_HitBox_H[j];
}

ProjectSpeedRet Ancilla_ProjectSpeedTowardsPlayer(int k, uint8 vel) {  // 888eed
  if (vel == 0) {
    ProjectSpeedRet rv = { 0, 0, 0, 0 };
    return rv;
  }
  PairU8 below = Ancilla_IsBelowLink(k);
  uint8 r12 = sign8(below.b) ? -below.b : below.b;

  PairU8 right = Ancilla_IsRightOfLink(k);
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

PairU8 Ancilla_IsRightOfLink(int k) {  // 888f5c
  uint16 x = link_x_coord - Ancilla_GetX(k);
  PairU8 rv = { (uint8)(sign16(x) ? 1 : 0), (uint8)x };
  return rv;
}

PairU8 Ancilla_IsBelowLink(int k) {  // 888f6f
  int y = link_y_coord - Ancilla_GetY(k);
  PairU8 rv = { (uint8)(sign16(y) ? 1 : 0), (uint8)y };
  return rv;
}

void Ancilla_WeaponTink() {  // 888f89
  if (!repulsespark_timer)
    return;
  sprite_alert_flag = 2;
  if (sign8(--repulsespark_anim_delay)) {
    repulsespark_timer--;
    repulsespark_anim_delay = 1;
  }

  if (sort_sprites_setting) {
    if (repulsespark_floor_status)
      Oam_AllocateFromRegionF(0x10);
    else
      Oam_AllocateFromRegionD(0x10);
  } else {
    Oam_AllocateFromRegionA(0x10);
  }

  uint8 x = repulsespark_x_lo - BG2HOFS_copy2;
  uint8 y = repulsespark_y_lo - BG2VOFS_copy2;

  if (x >= 0xf8 || y >= 0xf0) {
    repulsespark_timer = 0;
    return;
  }

  OamEnt *oam = GetOamCurPtr();
  static const uint8 kRepulseSpark_Flags[4] = {0x22, 0x12, 0x22, 0x22};
  uint8 flags = kRepulseSpark_Flags[repulsespark_floor_status];
  if (repulsespark_timer >= 3) {
    SetOamPlain(oam, x, y, (repulsespark_timer < 9) ? 0x92 : 0x80, flags, 0);
    return;
  }
  static const uint8 kRepulseSpark_Char[3] = { 0x93, 0x82, 0x81 };
  uint8 c = kRepulseSpark_Char[repulsespark_timer];
  SetOamPlain(oam + 0, x - 4, y - 4, c, flags | 0x00, 0);
  SetOamPlain(oam + 1, x + 4, y - 4, c, flags | 0x40, 0);
  SetOamPlain(oam + 2, x - 4, y + 4, c, flags | 0x80, 0);
  SetOamPlain(oam + 3, x + 4, y + 4, c, flags | 0xc0, 0);
}

void Ancilla_MoveX(int k) {  // 889080
  uint32 t = ancilla_x_subpixel[k] + (ancilla_x_lo[k] << 8) + (ancilla_x_hi[k] << 16) + ((int8)ancilla_x_vel[k] << 4);
  ancilla_x_subpixel[k] = t, ancilla_x_lo[k] = t >> 8, ancilla_x_hi[k] = t >> 16;
}

void Ancilla_MoveY(int k) {  // 88908b
  uint32 t = ancilla_y_subpixel[k] + (ancilla_y_lo[k] << 8) + (ancilla_y_hi[k] << 16) + ((int8)ancilla_y_vel[k] << 4);
  ancilla_y_subpixel[k] = t, ancilla_y_lo[k] = t >> 8, ancilla_y_hi[k] = t >> 16;
}

void Ancilla_MoveZ(int k) {  // 8890b7
  uint32 t = ancilla_z_subpixel[k] + (ancilla_z[k] << 8) + ((int8)ancilla_z_vel[k] << 4);
  ancilla_z_subpixel[k] = t, ancilla_z[k] = t >> 8;
}

void Ancilla05_Boomerang(int k) {  // 8890fc
  int hit_spr;
  static const int8 kBoomerang_X0[8] = {0, 0, -8, 8, 8, 8, -8, -8};
  static const int8 kBoomerang_Y0[8] = {-16, 6, 0, 0, -8, 8, -8, 8};

  for (int j = 4; j >= 0; j--) {
    if (ancilla_type[j] == 0x22)
      goto exit_and_draw;
  }
  if (submodule_index)
    goto exit_and_draw;

  if (!(frame_counter & 7))
    Ancilla_Sfx2_Pan(k, 0x9);

  if (!ancilla_aux_timer[k]) {
    if (button_b_frames < 9 && player_handler_timer == 0) {
      if (link_is_bunny_mirror || link_auxiliary_state || link_item_in_hand == 0 && (enhanced_features0 & kFeatures0_MiscBugFixes)) {
        Boomerang_Terminate(k);
        return;
      }
      goto exit_and_draw;
    }
    int j = ancilla_arr23[k] >> 1;
    Ancilla_SetXY(k, link_x_coord + kBoomerang_X0[j], link_y_coord + 8 + kBoomerang_Y0[j]);
    ancilla_aux_timer[k]++;
  }
  // endif_2
  if (ancilla_G[k] && !(frame_counter & 1))
    AncillaAdd_SwordChargeSparkle(k);

  if (ancilla_item_to_link[k]) {
    if (ancilla_K[k])
      ancilla_K[k]++;
    WORD(ancilla_A[k]) = link_y_coord;
    link_y_coord += 8;
    ProjectSpeedRet pt = Ancilla_ProjectSpeedTowardsPlayer(k, ancilla_H[k]);
    Boomerang_CheatWhenNoOnesLooking(k, &pt);
    ancilla_x_vel[k] = pt.x;
    ancilla_y_vel[k] = pt.y;
    link_y_coord = WORD(ancilla_A[k]);
  }

  if (ancilla_y_vel[k])
    ancilla_y_vel[k] += ancilla_K[k];
  Ancilla_MoveY(k);

  if (ancilla_x_vel[k])
    ancilla_x_vel[k] += ancilla_K[k];
  Ancilla_MoveX(k);
  hit_spr = Ancilla_CheckSpriteCollision(k);

  if (!ancilla_item_to_link[k]) {
    if (hit_spr >= 0) {
      ancilla_item_to_link[k] ^= 1;
    } else if (Ancilla_CheckTileCollision(k)) {
      AncillaAdd_BoomerangWallClink(k);
      Ancilla_Sfx2_Pan(k, (ancilla_tile_attr[k] == 0xf0) ? 6 : 5);
      ancilla_item_to_link[k] ^= 1;
    } else if (Boomerang_ScreenEdge(k) || --ancilla_step[k] == 0) {
      ancilla_item_to_link[k] ^= 1;
    } else {
      if (ancilla_step[k] < 5)
        ancilla_K[k]--;
    }
  } else {
    uint8 bak0 = ancilla_objprio[k];
    uint8 bak1 = ancilla_floor[k];
    ancilla_floor[k] = 0;
    Ancilla_CheckTileCollision(k);
    ancilla_floor[k] = bak1;
    ancilla_objprio[k] = bak0;
    Boomerang_StopOffScreen(k);
  }

exit_and_draw:
  Boomerang_Draw(k);
}

bool Boomerang_ScreenEdge(int k) {  // 88924b
  uint16 x = Ancilla_GetX(k), y = Ancilla_GetY(k);
  if (hookshot_effect_index & 3) {
    uint16 t = x + (hookshot_effect_index & 1 ? 16 : 0) - BG2HOFS_copy2;
    if (t >= 0x100)
      return true;
  }
  if (hookshot_effect_index & 12) {
    uint16 t = y + (hookshot_effect_index & 4 ? 16 : 0) - BG2VOFS_copy2;
    if (t >= 0xe2)
      return true;
  }
  return false;
}

void Boomerang_StopOffScreen(int k) {  // 8892ab
  uint16 x = Ancilla_GetX(k) + 8, y = Ancilla_GetY(k) + 8;
  if (x >= link_x_coord && x < (uint16)(link_x_coord + 16) &&
      y >= link_y_coord && y < (uint16)(link_y_coord + 24))
    Boomerang_Terminate(k);
}

void Boomerang_Terminate(int k) {  // 8892f5
  ancilla_type[k] = 0;
  flag_for_boomerang_in_place = 0;
  if (link_item_in_hand & 0x80) {
    link_item_in_hand = 0;
    button_mask_b_y &= ~0x40;
    if (!(button_mask_b_y & 0x80))
      link_cant_change_direction &= ~1;
  }
}

void Boomerang_Draw(int k) {  // 889338
  static const uint8 kBoomerang_Flags[8] = {0xa4, 0xe4, 0x64, 0x24, 0xa2, 0xe2, 0x62, 0x22};
  static const int8 kBoomerang_Draw_XY[8] = {2, -2, 2, 2, -2, 2, -2, -2};
  static const uint16 kBoomerang_Draw_OamIdx[2] = {0x180, 0xd0};
  static const uint8 kBoomerang_Draw_Tab0[2] = {3, 2};
  Point16U info;
  Ancilla_PrepOamCoord(k, &info);

  if (ancilla_item_to_link[k]) {
    ancilla_floor[k] = link_is_on_lower_level;
    oam_priority_value = kTagalongLayerBits[link_is_on_lower_level] << 8;
  }

  if (ancilla_objprio[k])
    oam_priority_value = 0x3000;

  if (!submodule_index && ancilla_aux_timer[k] && sign8(--ancilla_arr3[k])) {
    ancilla_arr3[k] = kBoomerang_Draw_Tab0[ancilla_G[k]];
    ancilla_arr1[k] = (ancilla_arr1[k] + (ancilla_S[k] ? -1 : 1)) & 3;
  }

  int j = ancilla_arr1[k];
  uint16 x = info.x + kBoomerang_Draw_XY[j * 2 + 1];
  uint16 y = info.y + kBoomerang_Draw_XY[j * 2 + 0];
  if (!ancilla_aux_timer[k]) {
    int i = kBoomerang_Draw_OamIdx[sort_sprites_setting];
    oam_ext_cur_ptr = (i >> 2) + 0xa20;
    oam_cur_ptr = i + 0x800;
  }
  Ancilla_SetOam_Safe(GetOamCurPtr(), x, y, 0x26, kBoomerang_Flags[ancilla_G[k] * 4 + j] & ~0x30 | HIBYTE(oam_priority_value), 2);
}

void Ancilla06_WallHit(int k) {  // 8893e8
  if (sign8(--ancilla_arr3[k])) {
    uint8 t = ancilla_item_to_link[k] + 1;
    if (t == 5) {
      ancilla_type[k] = 0;
      return;
    }
    ancilla_item_to_link[k] = t;
    ancilla_arr3[k] = 1;
  }
  WallHit_Draw(k);
}

void Ancilla_SwordWallHit(int k) {  // 8893ff
  sprite_alert_flag = 3;
  if (sign8(--ancilla_aux_timer[k])) {
    uint8 t = ancilla_item_to_link[k] + 1;
    if (t == 8) {
      ancilla_type[k] = 0;
      return;
    }
    ancilla_item_to_link[k] = t;
    ancilla_aux_timer[k] = 1;
  }
  WallHit_Draw(k);
}

void WallHit_Draw(int k) {  // 8894df
  static const int8 kWallHit_X[32] = {
    -4, 0,  0, 0, -4, 0, 0, 0, -8, 0, -8, 0, -8, 0, -8, 0,
    -8, 0, -8, 0, -4, 0, 0, 0, -4, 0,  0, 0, -8, 0,  0, 0,
  };
  static const int8 kWallHit_Y[32] = {
    -4,  0, 0, 0, -4, 0, 0, 0, -8, -8, 0, 0, -8, -8, 0, 0,
    -8, -8, 0, 0, -4, 0, 0, 0, -4,  0, 0, 0, -8,  0, 0, 0,
  };
  static const uint8 kWallHit_Char[32] = {
    0x80,    0,    0,    0, 0x92, 0, 0, 0, 0x81, 0x81, 0x81, 0x81, 0x82, 0x82, 0x82, 0x82,
    0x93, 0x93, 0x93, 0x93, 0x92, 0, 0, 0, 0xb9,    0,    0,    0, 0x90, 0x90,    0,    0,
  };
  static const uint8 kWallHit_Flags[32] = {
    0x32,    0,    0,    0, 0x32, 0, 0, 0, 0x32, 0x72, 0xb2, 0xf2, 0x32, 0x72, 0xb2, 0xf2,
    0x32, 0x72, 0xb2, 0xf2, 0x32, 0, 0, 0, 0x72,    0,    0,    0, 0x32, 0xf2,    0,    0,
  };
  Point16U info;
  Ancilla_PrepOamCoord(k, &info);

  int t = ancilla_item_to_link[k] * 4;

  OamEnt *oam = GetOamCurPtr();
  for (int n = 3; n >= 0; n--, t++) {
    if (kWallHit_Char[t] != 0) {
      Ancilla_SetOam(oam, info.x + kWallHit_X[t], info.y + kWallHit_Y[t], kWallHit_Char[t],
                     kWallHit_Flags[t] & ~0x30 | HIBYTE(oam_priority_value), 0);
      oam++;
    }
    oam = Ancilla_AllocateOamFromCustomRegion(oam);
  }

}

void Ancilla07_Bomb(int k) {  // 88955a
  if (submodule_index) {
    if (submodule_index == 8 || submodule_index == 16) {
      Ancilla_HandleLiftLogic(k);
    } else if (k + 1 == flag_is_ancilla_to_pick_up && ancilla_K[k] != 0) {
      if (ancilla_K[k] != 3) {
        Ancilla_LatchLinkCoordinates(k, 3);
        Ancilla_LatchAltitudeAboveLink(k);
        ancilla_K[k] = 3;
      }
      Ancilla_LatchCarriedPosition(k);
    }
    Bomb_Draw(k);
    return;
  }
  Ancilla_HandleLiftLogic(k);

  uint16 old_y = Ancilla_LatchYCoordToZ(k);
  uint8 s1a = ancilla_dir[k];
  uint8 s1b = ancilla_objprio[k];
  ancilla_objprio[k] = 0;
  bool flag = Ancilla_CheckTileCollision_Class2(k);

  if (player_is_indoors && ancilla_L[k] && ancilla_tile_attr[k] == 0x1c)
    ancilla_T[k] = 1;

label1:
  if (flag && (!(link_state_bits & 0x80) || link_picking_throw_state)) {
    if (!s1b && !ancilla_arr4[k]) {
      ancilla_arr4[k] = 1;
      int qq = (ancilla_dir[k] == 1) ? 16 : 4;
      if (ancilla_y_vel[k])
        ancilla_y_vel[k] = sign8(ancilla_y_vel[k]) ? qq : -qq;
      if (ancilla_x_vel[k])
        ancilla_x_vel[k] = sign8(ancilla_x_vel[k]) ? 4 : -4;
      if (ancilla_dir[k] == 1 && ancilla_z[k]) {
        ancilla_y_vel[k] = -4;
        ancilla_L[k] = 2;
      }
    }
  } else if (!((k + 1 == flag_is_ancilla_to_pick_up) && (link_state_bits & 0x80)) && (ancilla_z[k] == 0 || ancilla_z[k] == 0xff)) {
    ancilla_dir[k] = 16;
    uint8 bak0 = ancilla_objprio[k];
    Ancilla_CheckTileCollision(k);
    ancilla_objprio[k] = bak0;
    uint8 a = ancilla_tile_attr[k];
    if (a == 0x26) {
      flag = true;
      goto label1;
    } else if (a == 0xc || a == 0x1c) {
      if (dung_hdr_collision != 3) {
        if (ancilla_floor[k] == 0 && ancilla_z[k] != 0 && ancilla_z[k] != 0xff)
          ancilla_floor[k] = 1;
      } else {
        old_y = Ancilla_GetY(k) + dung_floor_y_vel;
        Ancilla_SetX(k, Ancilla_GetX(k) + dung_floor_x_vel);
      }
    } else if (a == 0x20 || (a & 0xf0) == 0xb0 && a != 0xb6 && a != 0xbc) {
      if (!(link_state_bits & 0x80)) {
        if (k + 1 == flag_is_ancilla_to_pick_up)
          flag_is_ancilla_to_pick_up = 0;
        if (!ancilla_timer[k]) {
          ancilla_type[k] = 0;
          return;
        }
      }
    } else if (a == 8) {
      if (k + 1 == flag_is_ancilla_to_pick_up)
        flag_is_ancilla_to_pick_up = 0;
      if (ancilla_timer[k] == 0) {
        Ancilla_SetY(k, Ancilla_GetY(k) - 24);
        Ancilla_TransmuteToSplash(k);
        return;
      }
    } else if (a == 0x68 || a == 0x69 || a == 0x6a || a == 0x6b) {
      Ancilla_ApplyConveyor(k);
      old_y = Ancilla_GetY(k);
    } else {
      ancilla_timer[k] = ancilla_L[k] ? 0 : 2;
    }
  }
  // endif_3

  Ancilla_SetY(k, old_y);
  ancilla_dir[k] = s1a;
  ancilla_objprio[k] |= s1b;
  Bomb_CheckSpriteAndPlayerDamage(k);
  if (!--ancilla_arr3[k]) {
    if (++ancilla_item_to_link[k] == 1) {
      Ancilla_Sfx2_Pan(k, 0xc);
      if (k + 1 == flag_is_ancilla_to_pick_up) {
        flag_is_ancilla_to_pick_up = 0;
        if (link_state_bits & 0x80) {
          link_state_bits = 0;
          link_cant_change_direction = 0;
        }
      }
    }

    if (ancilla_item_to_link[k] == 11) {
      // transmute to door debris?
      ancilla_type[k] = ancilla_step[k] ? 8 : 0;
      return;
    }
    ancilla_arr3[k] = kBomb_Tab0[ancilla_item_to_link[k]];
  }

  if (ancilla_item_to_link[k] == 7 && ancilla_arr3[k] == 2) {
    // check whether the bomb causes any door debris, the bomb
    // will transmute to debris later on.
    door_debris_x[k] = 0;
    Bomb_CheckForDestructibles(Ancilla_GetX(k), Ancilla_GetY(k), k);
    if (door_debris_x[k])
      ancilla_step[k] = 1;
  }
  Bomb_Draw(k);
}

void Ancilla_ApplyConveyor(int k) {  // 8897be
  static const int8 kAncilla_Belt_Xvel[4] = {0, 0, -8, 8};
  static const int8 kAncilla_Belt_Yvel[4] = {-8, 8, 0, 0};
  int j = ancilla_tile_attr[k] - 0x68;
  ancilla_y_vel[k] = kAncilla_Belt_Yvel[j];
  ancilla_x_vel[k] = kAncilla_Belt_Xvel[j];
  Ancilla_MoveY(k);
  Ancilla_MoveX(k);
}

void Bomb_CheckSpriteAndPlayerDamage(int k) {  // 889815
  static const uint8 kBomb_Dmg_Speed[16] = {32, 32, 32, 32, 32, 32, 28, 28, 28, 28, 28, 28, 24, 24, 24, 24};
  static const uint8 kBomb_Dmg_Zvel[16] = {16, 16, 16, 16, 16, 16, 12, 12, 12, 12, 8, 8, 8, 8, 8, 8};
  static const uint8 kBomb_Dmg_Delay[16] = {32, 32, 32, 32, 32, 32, 24, 24, 24, 24, 24, 24, 16, 16, 16, 16};
  static const uint8 kBomb_Dmg_ToLink[3] = {8, 4, 2};

  if (ancilla_item_to_link[k] == 0 || ancilla_item_to_link[k] >= 9)
    return;
  Bomb_CheckSpriteDamage(k);
  if (link_disable_sprite_damage) {
    if (k + 1 == flag_is_ancilla_to_pick_up && link_state_bits & 0x80) {
      link_state_bits &= ~0x80;
      link_cant_change_direction = 0;
    }
    return;
  }

  if (link_auxiliary_state || link_incapacitated_timer || ancilla_floor[k] != link_is_on_lower_level)
    return;

  SpriteHitBox hb;
  hb.r0_xlo = link_x_coord;
  hb.r8_xhi = link_x_coord >> 8;
  hb.r1_ylo = link_y_coord;
  hb.r9_yhi = link_y_coord >> 8;
  hb.r2 = 0x10;
  hb.r3 = 0x18;

  int ax = Ancilla_GetX(k) - 16, ay = Ancilla_GetY(k) - 16;
  hb.r6_spr_xsize = 32;
  hb.r7_spr_ysize = 32;
  hb.r4_spr_xlo = ax;
  hb.r10_spr_xhi = ax >> 8;
  hb.r5_spr_ylo = ay;
  hb.r11_spr_yhi = ay >> 8;

  if (!CheckIfHitBoxesOverlap(&hb))
    return;

  int x = Ancilla_GetX(k) - 8, y = Ancilla_GetY(k) - 12;

  int j = Bomb_GetDisplacementFromLink(k);
  ProjectSpeedRet pt = Bomb_ProjectSpeedTowardsPlayer(k, x, y, kBomb_Dmg_Speed[j]);
  if (countdown_for_blink || flag_block_link_menu == 2)
    return;
  link_actual_vel_x = pt.x;
  link_actual_vel_y = pt.y;

  link_actual_vel_z_copy = link_actual_vel_z = kBomb_Dmg_Zvel[j];
  link_incapacitated_timer = kBomb_Dmg_Delay[j];
  link_auxiliary_state = 1;
  countdown_for_blink = 58;
  if (!(dung_savegame_state_bits & 0x8000))
    link_give_damage = kBomb_Dmg_ToLink[link_armor];

}

void Ancilla_HandleLiftLogic(int k) {  // 889976
  static const uint8 kAncilla_Liftable_Delay[3] = {16, 8, 9};

  if (ancilla_R[k]) {
label_6:
    if (ancilla_item_to_link[k])
      return;
    if (ancilla_K[k] == 3) {
      ancilla_z_vel[k] -= 2;
      Ancilla_MoveZ(k);
      if (ancilla_z[k] && ancilla_z[k] < 252)
        return;
      ancilla_z[k] = 0;
      if (++ancilla_R[k] != 3) {
        ancilla_z_vel[k] = 24;
        return;
      }
      ancilla_K[k] = 0;
    }
    ancilla_R[k] = 0;
    link_speed_setting = 0;
    return;
  }
  if (!ancilla_L[k]) {
    if (!flag_is_ancilla_to_pick_up) {
clear_pickup_item:
      flag_is_ancilla_to_pick_up = 0;
      CheckPlayerCollOut coll;
      if (ancilla_item_to_link[k] || link_state_bits || !Ancilla_CheckLinkCollision(k, 0, &coll) || ancilla_floor[k] != link_is_on_lower_level)
        return;
      if (coll.r8 >= 16 || coll.r10 >= 12) {
        int j = (coll.r8 >= coll.r10) ? (sign8(coll.r4) ? 1 : 0) : (sign8(coll.r6) ? 3 : 2);
        if (j * 2 != link_direction_facing)
          return;
      }
      flag_is_ancilla_to_pick_up = k + 1;
      ancilla_K[k] = 0;
      ancilla_aux_timer[k] = kAncilla_Liftable_Delay[0];
      ancilla_L[k] = 0;
      ancilla_z[k] = 0;
      return;
    }

    if (flag_is_ancilla_to_pick_up != k + 1)
      return;
    if (!link_disable_sprite_damage && link_incapacitated_timer || byte_7E03FD || link_auxiliary_state == 1) {
      ancilla_R[k] = 1;
      ancilla_z_vel[k] = 0;
      flag_is_ancilla_to_pick_up = 0;
      ancilla_arr4[k] = 0;
      goto label_6;
    }
    if (!(link_state_bits & 0x80))
      goto clear_pickup_item;
    int j = ancilla_K[k];
    if (link_picking_throw_state != 2 && flag_is_ancilla_to_pick_up != 0 && j != 3) {
      if (j == 0 && ancilla_aux_timer[k] == 16)
        Ancilla_Sfx2_Pan(k, 0x1d);
      if (sign8(--ancilla_aux_timer[k])) {
        ancilla_K[k] = ++j;
        ancilla_aux_timer[k] = j == 3 ? -2 : kAncilla_Liftable_Delay[j];
        if (j == 3) {
          Ancilla_LatchAltitudeAboveLink(k);
          return;
        }
      }
      Ancilla_LatchLinkCoordinates(k, j);
      return;
    }
    if (j != 3)
      return;

    if (link_picking_throw_state != 2 && (submodule_index != 0 || !((filtered_joypad_L | filtered_joypad_H) & 0x80))) {
      if (ancilla_item_to_link[k])
        return;
      if (player_near_pit_state >= 2) {
        link_speed_setting = 0;
        if (k + 1 == flag_is_ancilla_to_pick_up) {
          flag_is_ancilla_to_pick_up = 0;
          ancilla_type[k] = 0;
        }
        return;
      }
      if (!(link_is_in_deep_water | link_is_bunny_mirror)) {
        Ancilla_LatchCarriedPosition(k);
        return;
      }
      link_state_bits = 0;
    }
    static const int8 kAncilla_Liftable_Yvel[4] = {-32, 32, 0, 0};
    static const int8 kAncilla_Liftable_Xvel[4] = {0, 0, -32, 32};
    j = link_direction_facing >> 1;
    ancilla_dir[k] = j;
    ancilla_z_vel[k] = 24;
    ancilla_y_vel[k] = kAncilla_Liftable_Yvel[j];
    ancilla_x_vel[k] = kAncilla_Liftable_Xvel[j];
    link_picking_throw_state = 2;
    ancilla_L[k] = 1;
    flag_is_ancilla_to_pick_up = 0;
    ancilla_arr4[k] = 0;
    ancilla_K[k] = 0;
    ancilla_objprio[k] = 0;
    Ancilla_Sfx3_Pan(k, 0x13);
  }
  // endif_1
  if (!ancilla_item_to_link[k]) {
    ancilla_z_vel[k] -= 2;
    Ancilla_MoveY(k);
    Ancilla_MoveX(k);
    uint8 old_z = ancilla_z[k];
    Ancilla_MoveZ(k);
    if (ancilla_arr4[k] && ancilla_dir[k] == 1 && !sign8(ancilla_z[k]))
      Ancilla_SetY(k, Ancilla_GetY(k) + (int8)(ancilla_z[k] - old_z));
    if (!sign8(ancilla_z[k]) || ancilla_z[k] == 0xff)
      return;
    ancilla_z[k] = 0;
    Ancilla_Sfx2_Pan(k, 0x21);
    if (++ancilla_L[k] != 3) {
      ancilla_y_vel[k] = (int8)ancilla_y_vel[k] / 2;
      ancilla_x_vel[k] = (int8)ancilla_x_vel[k] / 2;
      ancilla_z_vel[k] = 16;
      ancilla_arr4[k] = 0;
    } else {
      ancilla_z[k] = 0;
      ancilla_L[k] = 0;
      ancilla_arr4[k] = 0;
      link_speed_setting = 0;
      ancilla_y_vel[k] = 0;
      ancilla_x_vel[k] = 0;
      ancilla_z_vel[k] = 0;
      if (ancilla_T[k]) {
        ancilla_floor[k] = ancilla_T[k];
        ancilla_T[k] = 0;
      }
    }
  }
}

void Ancilla_LatchAltitudeAboveLink(int k) {  // 889a4f
  ancilla_z[k] = 17;
  Ancilla_SetY(k, Ancilla_GetY(k) + 17);
  ancilla_objprio[k] = 0;
}

void Ancilla_LatchLinkCoordinates(int k, int j) {  // 889a6a
  static const int8 kAncilla_Func3_X[12] = {8, 8, -4, 20, 8, 8, 8, 8, 8, 8, 8, 8};
  static const int8 kAncilla_Func3_Y[12] = {16, 8, 4, 4, 8, 2, -1, -1, 2, 2, -1, -1};
  j = j * 4 + (link_direction_facing >> 1);
  Ancilla_SetXY(k,
      link_x_coord + kAncilla_Func3_X[j],
      link_y_coord + kAncilla_Func3_Y[j]);
}

void Ancilla_LatchCarriedPosition(int k) {  // 889bef
  static const int8 kAncilla_Func2_Y[6] = {-2, -1, 0, -2, -1, 0};
  link_speed_setting = 12;
  ancilla_floor[k] = link_is_on_lower_level;
  ancilla_floor2[k] = link_is_on_lower_level_mirror;
  uint16 z = link_z_coord;
  if (z == 0xffff)
    z = 0;
  Ancilla_SetXY(k,
    link_x_coord + 8,
    link_y_coord - z + 18 + kAncilla_Func2_Y[link_animation_steps]);
}

uint16 Ancilla_LatchYCoordToZ(int k) {  // 889c7f
  uint16 y = Ancilla_GetY(k);
  int8 z = ancilla_z[k];
  if (ancilla_dir[k] == 1 && z != -1)
    Ancilla_SetY(k, y - z);
  return y;
}

int Bomb_GetDisplacementFromLink(int k) {  // 889cce
  int x = Ancilla_GetX(k), y = Ancilla_GetY(k);
  return ((abs16(link_x_coord + 8 - x) + abs16(link_y_coord + 12 - y)) & 0xfc) >> 2;
}

void Bomb_Draw(int k) {  // 889e9e
  Point16U pt;
  Ancilla_PrepAdjustedOamCoord(k, &pt);

  int z = (int8)ancilla_z[k];
  if (z != 0 && z != -1 && ancilla_K[k] != 3 && ancilla_objprio[k])
    oam_priority_value = 0x3000;
  pt.y -= z;
  int j = kBomb_Draw_Tab0[ancilla_item_to_link[k]] * 6;

  uint8 r11 = 2;
  if (ancilla_item_to_link[k] == 0) {
    r11 = (ancilla_arr3[k] < 0x20) ? ancilla_arr3[k] & 0xe : 4;
  }

  if (ancilla_item_to_link[k] == 0) {
    if (ancilla_L[k] == 0 && (sprite_type[0] == 0x92 || k + 1 == flag_is_ancilla_to_pick_up ) && (!(link_state_bits & 0x80) || ancilla_K[k] != 3 && link_direction_facing == 0)) {
      Ancilla_AllocateOamFromRegion_B_or_E(12);
    } else if (sort_sprites_setting && ancilla_floor[k] && (ancilla_L[k] || k + 1 == flag_is_ancilla_to_pick_up && (link_state_bits & 0x80))) {
      oam_cur_ptr = 0x800 + 0x34 * 4;
      oam_ext_cur_ptr = 0xa20 + 0x34;
    }
  }

  OamEnt *oam = GetOamCurPtr(), *oam_org = oam;
  uint8 numframes = kBomb_Draw_Tab2[ancilla_item_to_link[k]];

  oam += (ancilla_item_to_link[k] == 0 && (ancilla_tile_attr[k] == 9 || ancilla_tile_attr[k] == 0x40)) ? 2 : 0;

  AncillaDraw_Explosion(oam, j, 0, numframes, r11, pt.x, pt.y);
  oam += numframes;

  uint8 r10;
  if (!Bomb_CheckUndersideSpriteStatus(k, &pt, &r10)) {
    if (oam != oam_org + 1)
      oam = oam_org;
    AncillaDraw_Shadow(oam, r10, pt.x, pt.y, HIBYTE(oam_priority_value));
  }
}

void Ancilla08_DoorDebris(int k) {  // 889fb6
  DoorDebris_Draw(k);
  if (sign8(--ancilla_arr26[k])) {
    ancilla_arr26[k] = 7;
    if (++ancilla_arr25[k] == 4)
      ancilla_type[k] = 0;
  }
}

void DoorDebris_Draw(int k) {  // 88a091
  static const uint16 kDoorDebris_XY[64] = {
     4,  7,  3, 17,  8,  8,  7, 17, 11,  7, 10, 16, 16,  7, 17, 17,
    20,  7, 21, 17, 16,  8, 17, 17, 13,  7, 14, 16,  8,  7,  7, 17,
     7,  4, 17,  3,  8,  8, 17,  7,  7, 11, 16, 10,  7, 16, 17, 17,
     7, 20, 17, 21,  8, 16, 17, 17,  7, 13, 16, 14,  7,  8, 17,  7,
  };
  static const uint16 kDoorDebris_CharFlags[32] = {
    0x205e, 0xe05e, 0xa05e, 0x605e, 0x204f, 0x204f, 0x204f, 0x204f, 0x605e, 0x605e, 0x205e, 0xe05e, 0x604f, 0x604f, 0x604f, 0x604f,
    0x205e, 0xe05e, 0xa05e, 0x605e, 0x204f, 0xe04f, 0x204f, 0x204f, 0x605e, 0x605e, 0x205e, 0xe05e, 0x604f, 0x604f, 0x604f, 0x604f,
  };
  Point16U pt;
  Ancilla_PrepAdjustedOamCoord(k, &pt);
  OamEnt *oam = GetOamCurPtr();
  int y = door_debris_y[k] - BG2VOFS_copy2;
  int x = door_debris_x[k] - BG2HOFS_copy2;
  int j = ancilla_arr25[k] + door_debris_direction[k] * 4;

  for (int i = 0; i != 2; i++) {
    int t = j * 2 + i;
    //kDoorDebris_XY
    uint16 d = kDoorDebris_CharFlags[t];
    Ancilla_SetOam(oam, x + kDoorDebris_XY[t * 2 + 1], y + kDoorDebris_XY[t * 2 + 0],
                   d, (d >> 8) & 0xc0 | HIBYTE(oam_priority_value), 0);
    oam = Ancilla_AllocateOamFromCustomRegion(oam + 1);
  }
}

void Ancilla09_Arrow(int k) {  // 88a131
  static const int8 kArrow_Y[4] = {-4, 2, 0, 0};
  static const int8 kArrow_X[4] = {0, 0, -4, 4};
  int j;

  if (submodule_index != 0) {
    Arrow_Draw(k);
    return;
  }

  if (!sign8(--ancilla_item_to_link[k])) {
    if (ancilla_item_to_link[k] >= 4)
      return;
  } else {
    ancilla_item_to_link[k] = 0xff;
  }
  Ancilla_MoveY(k);
  Ancilla_MoveX(k);
  if (link_item_bow & 4 && !(frame_counter & 1))
    AncillaAdd_SilverArrowSparkle(k);
  ancilla_S[k] = 255;
  if ((j = Ancilla_CheckSpriteCollision(k)) >= 0) {
    ancilla_x_vel[k] = ancilla_x_lo[k] - sprite_x_lo[j];
    ancilla_y_vel[k] = ancilla_y_lo[k] - sprite_y_lo[j] + sprite_z[j];
    ancilla_S[k] = j;
    if (sprite_type[j] == 0x65) {
      if (sprite_A[j] == 1) {
        sound_effect_2 = 0x2d;
        sprite_delay_aux2[j] = 0x80;
        sprite_delay_aux4[0] = 128;
        if (byte_7E0B88 < 9)
          byte_7E0B88++;
        sprite_B[j] = byte_7E0B88;
        sprite_G[j] += 1;
      } else {
        sprite_delay_aux3[j] = 4;
        byte_7E0B88 = 0;
      }
    } else {
      byte_7E0B88 = 0;
    }
  } else if ((j = Ancilla_CheckTileCollision(k)) != 0) {
    ancilla_H[k] = j >> 1;
    j = ancilla_dir[k] & 3;
    Ancilla_SetX(k, Ancilla_GetX(k) + kArrow_X[j]);
    Ancilla_SetY(k, Ancilla_GetY(k) + kArrow_Y[j]);
    byte_7E0B88 = 0;
  } else {
    Arrow_Draw(k);
    return;
  }
  if (sprite_type[j] != 0x1b)
    Ancilla_Sfx2_Pan(k, 8);
  ancilla_item_to_link[k] = 0;
  ancilla_type[k] = 10;
  ancilla_aux_timer[k] = 1;
  if (ancilla_H[k]) {
    ancilla_x_lo[k] += BG1HOFS_copy2 - BG2HOFS_copy2;
    ancilla_y_lo[k] += BG1VOFS_copy2 - BG2VOFS_copy2;
  }
  Arrow_Draw(k);
}

void Arrow_Draw(int k) {  // 88a36e
  static const uint8 kArrow_Draw_Char[48] = {
    0x2b, 0x2a, 0x2a, 0x2b, 0x3d, 0x3a, 0x3a, 0x3d, 0x2b, 0xff, 0x2b, 0xff, 0x3d, 0xff, 0x3d, 0xff,
    0x3c, 0x2c, 0x3c, 0x2a, 0x3c, 0x2c, 0x3c, 0x2a, 0x2c, 0x3c, 0x2a, 0x3c, 0x2c, 0x3c, 0x2a, 0x3c,
    0x3b, 0x2d, 0x3b, 0x3a, 0x3b, 0x2d, 0x3b, 0x3a, 0x2d, 0x3b, 0x3a, 0x3b, 0x2d, 0x3b, 0x3a, 0x3b,
  };
  static const uint8 kArrow_Draw_Flags[48] = {
    0xa4, 0xa4, 0x24, 0x24, 0x64, 0x64, 0x24, 0x24, 0xa4, 0xff, 0x24, 0xff, 0x64, 0xff, 0x24, 0xff,
    0xa4, 0xa4, 0xa4, 0xa4, 0xa4, 0xe4, 0xa4, 0xa4, 0x24, 0x24, 0x24, 0x24, 0x64, 0x24, 0x24, 0x24,
    0x64, 0x64, 0x64, 0xe4, 0x64, 0xe4, 0x64, 0xe4, 0x24, 0x24, 0x24, 0xa4, 0xa4, 0x24, 0x24, 0xa4,
  };
  static const int8 kArrow_Draw_Y[48] = {
    0,  8, 0, 8, 0, 0, 0, 0,  0,  0, 0, 0, 0, 0, 0, 0,
    0,  8, 0, 8, 0, 8, 0, 8,  0,  8, 0, 8, 0, 8, 0, 8,
    -1, -1, 0, 0, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 0, 0,
  };
  static const int8 kArrow_Draw_X[48] = {
    0, 0, 0, 0,  0,  8, 0, 8, 0, 0, 0, 0,  0,  0, 0, 0,
    1, 1, 0, 0, -1, -2, 0, 0, 1, 1, 0, 0, -2, -1, 0, 0,
    0, 8, 0, 8,  0,  8, 0, 8, 0, 8, 0, 8,  0,  8, 0, 8,
  };
  Point16U pt;
  Ancilla_PrepAdjustedOamCoord(k, &pt);
  if (ancilla_objprio[k])
    HIBYTE(oam_priority_value) = 0x30;
  uint16 x = pt.x, y = pt.y;
  if (ancilla_H[k] != 0) {
    x += BG2VOFS_copy2 - BG1VOFS_copy2;
    y += BG2HOFS_copy2 - BG1HOFS_copy2;
  }
  uint8 r7 = ancilla_item_to_link[k];
  int j = ancilla_dir[k] & ~4;
  if (ancilla_type[k] == 0xa) {
    j = j * 4 + 8 + ((r7 & 8) ? 1 : (r7 & 3));
  } else if (!sign8(r7)) {
    j |= 4;
  }

  j *= 2;

  OamEnt *oam = GetOamCurPtr(), *oam_org = oam;
  uint8 flags = (link_item_bow & 4) ? 2 : 4;
  for (int i = 0; i != 2; i++, j++) {
    if (kArrow_Draw_Char[j] != 0xff) {
      Ancilla_SetOam(oam, x + kArrow_Draw_X[j], y + kArrow_Draw_Y[j],
                     kArrow_Draw_Char[j], kArrow_Draw_Flags[j] & ~0x3E | flags | HIBYTE(oam_priority_value), 0);
      oam++;
    }
  }

  if (oam_org[0].y == 0xf0 && oam_org[1].y == 0xf0)
    ancilla_type[k] = 0;
}

void Ancilla0A_ArrowInTheWall(int k) {  // 88a45b
  int j = ancilla_S[k];
  if (!sign8(j)) {
    if (sprite_state[j] < 9 || sign8(sprite_z[j]) || sprite_ignore_projectile[j] || sprite_defl_bits[j] & 2) {
      ancilla_type[k] = 0;
      return;
    }
    Ancilla_SetX(k, Sprite_GetX(j) + (int8)ancilla_x_vel[k]);
    Ancilla_SetY(k, Sprite_GetY(j) + (int8)ancilla_y_vel[k] - sprite_z[j]);
  }
  if (submodule_index == 0 && --ancilla_aux_timer[k] == 0) {
    ancilla_aux_timer[k] = 2;
    if (++ancilla_item_to_link[k] == 9) {
      ancilla_type[k] = 0;
      return;
    } else if (ancilla_item_to_link[k] & 8) {
      ancilla_aux_timer[k] = 0x80;
    }
  }
  Arrow_Draw(k);
}

void Ancilla0B_IceRodShot(int k) {  // 88a4dd
  if (submodule_index == 0) {
    if (sign8(--ancilla_aux_timer[k])) {
      if (++ancilla_item_to_link[k] & ~1) {
        ancilla_step[k] = 1;
        ancilla_item_to_link[k] = ancilla_item_to_link[k] & 7 | 4;
      }
      ancilla_aux_timer[k] = 3;
    }
    if (ancilla_step[k]) {
      AncillaOamInfo info;
      if (Ancilla_ReturnIfOutsideBounds(k, &info))
        return;
      Ancilla_MoveY(k);
      Ancilla_MoveX(k);
      if (Ancilla_CheckSpriteCollision(k) >= 0 || Ancilla_CheckTileCollision(k)) {
        ancilla_type[k] = 0x11;
        ancilla_numspr[k] = kAncilla_Pflags[0x11];
        ancilla_item_to_link[k] = 0;
        ancilla_aux_timer[k] = 4;
      }
    }
  }
  AncillaAdd_IceRodSparkle(k);
}

void Ancilla11_IceRodWallHit(int k) {  // 88a536
  if (sign8(--ancilla_aux_timer[k])) {
    ancilla_aux_timer[k] = 7;
    if (++ancilla_item_to_link[k] == 2) {
      ancilla_type[k] = 0;
      return;
    }
  }
  IceShotSpread_Draw(k);
}

void IceShotSpread_Draw(int k) {  // 88a571
  static const uint8 kIceShotSpread_CharFlags[16] = {0xcf, 0x24, 0xcf, 0x24, 0xcf, 0x24, 0xcf, 0x24, 0xdf, 0x24, 0xdf, 0x24, 0xdf, 0x24, 0xdf, 0x24};
  static const uint8 kIceShotSpread_XY[16] = {0, 0, 0, 8, 8, 0, 8, 8, 0xf8, 0xf8, 0xf8, 0x10, 0x10, 0xf8, 0x10, 0x10};

  Point16U info;
  Ancilla_PrepOamCoord(k, &info);

  Ancilla_AllocateOamFromRegion_A_or_D_or_F(k, ancilla_numspr[k]);
  OamEnt *oam = GetOamCurPtr();
  int j = ancilla_item_to_link[k] * 4;
  for (int i = 0; i != 4; i++, j++) {
    uint16 y = info.y + (int8)kIceShotSpread_XY[j * 2 + 0];
    uint16 x = info.x + (int8)kIceShotSpread_XY[j * 2 + 1];
    uint8 yv = 0xf0;
    if (x < 256 && y < 256) {
      oam->x = x;
      if (y < 224)
        yv = y;
    }
    oam->y = yv;
    oam->charnum = kIceShotSpread_CharFlags[j * 2 + 0];
    oam->flags = kIceShotSpread_CharFlags[j * 2 + 1] & ~0x30 | HIBYTE(oam_priority_value);
    bytewise_extended_oam[oam - oam_buf] = 0;
    oam = Ancilla_AllocateOamFromCustomRegion(oam + 1);
  }
  oam = GetOamCurPtr();
  if (oam[0].y == 0xf0 && oam[1].y == 0xf0)
    ancilla_type[k] = 0;
}

void Ancilla33_BlastWallExplosion(int k) {  // 88a60e
  if (submodule_index == 0) {
    if (blastwall_var5[k]) {
      if (--blastwall_var6[k] == 0) {
        if (++blastwall_var5[k] != 0 && blastwall_var5[k] < 9) {
          AncillaAdd_BlastWallFireball(0x32, 10, k * 4);
        }
        if (blastwall_var5[k] == 11) {
          blastwall_var5[k] = 0;
          blastwall_var6[k] = 0;
        } else {
          blastwall_var6[k] = 3;
        }
      }
    } else if ((k ^= 1), blastwall_var5[k] == 6 && blastwall_var6[k] == 2 && (uint8)(ancilla_item_to_link[0] + 1) < 7) {
      ancilla_item_to_link[0]++;
      blastwall_var5[k] = 1;
      blastwall_var6[k] = 3;
      for (int i = 3; i >= 0; i--) {
        int8 arr[2] = { 0, 0 };
        int j = blastwall_var7 < 4 ? 1 : 0;
        arr[j] = (i & 2) ? -13 : 13;
        j = k * 4 + i;
        blastwall_var10[j] += arr[0];
        blastwall_var11[j] += arr[1];
        uint16 x = blastwall_var11[j] - BG2HOFS_copy2;
        if (x < 256)
          sound_effect_1 = kBombos_Sfx[x >> 5] | 0xc;
      }
    }
  }

  if (blastwall_var5[ancilla_K[0]]) {
    int i = (ancilla_K[0] == 1) ? 7 : 3;
    do {
      AncillaDraw_BlastWallBlast(ancilla_K[0], blastwall_var11[i], blastwall_var10[i]);
    } while ((--i & 3) != 3);
  }
  if (ancilla_item_to_link[0] == 6) {
    if (blastwall_var5[0] == 0 && blastwall_var5[1] == 0) {
      ancilla_type[0] = 0;
      ancilla_type[1] = 0;
      flag_custom_spell_anim_active = 0;
    }
  }
}

void AncillaDraw_BlastWallBlast(int k, int x, int y) {  // 88a756
  oam_priority_value = 0x3000;
  if (sort_sprites_setting)
    Oam_AllocateFromRegionD(0x18);
  else
    Oam_AllocateFromRegionA(0x18);
  OamEnt *oam = GetOamCurPtr();
  int i = blastwall_var5[k];
  AncillaDraw_Explosion(oam, kBomb_Draw_Tab0[i] * 6, 0, kBomb_Draw_Tab2[i], 0x32,
      x - BG2HOFS_copy2, y - BG2VOFS_copy2);
}

OamEnt *AncillaDraw_Explosion(OamEnt *oam, int frame, int idx, int idx_end, uint8 r11, int x, int y) {  // 88a7ab
  static const int8 kBomb_DrawExplosion_XY[108] = {
     -8,  -8,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,  -8,  -8,  -8,  0,
      0,  -8,   0,   0,   0,   0,   0,   0, -16, -16, -16,  0,   0, -16,   0,  0,
      0,   0,   0,   0, -16, -16, -16,   0,   0, -16,   0,  0,   0,   0,   0,  0,
     -8,  -8, -21, -22, -21,   8,   9, -22,   9,   8,   0,  0,  -6, -15,   0, -1,
    -16,  -2,  -8,  -7,   0,   0,   0,   0,  -9,  -4, -21, -5, -12, -18, -11,  7,
      0, -15,   4,  -2,  -9,  -4, -22,  -5, -13, -20, -11,  8,   1, -16,   5, -2,
    -20,   4, -12, -19,  -9,  16,  -5,  -2,   2,  -9,  10,  6,
  };
  static const uint8 kBomb_DrawExplosion_CharFlags[108] = {
    0x6e, 0x26, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x8c, 0x22, 0x8c, 0x62,
    0x8c, 0xa2, 0x8c, 0xe2, 0xff, 0xff, 0xff, 0xff, 0x84, 0x22, 0x84, 0x62, 0x84, 0xa2, 0x84, 0xe2,
    0xff, 0xff, 0xff, 0xff, 0x88, 0x22, 0x88, 0x62, 0x88, 0xa2, 0x88, 0xe2, 0xff, 0xff, 0xff, 0xff,
    0x86, 0x22, 0x88, 0x22, 0x88, 0x62, 0x88, 0xa2, 0x88, 0xe2, 0xff, 0xff, 0x86, 0x22, 0x86, 0x62,
    0x86, 0xe2, 0x86, 0xe2, 0xff, 0xff, 0xff, 0xff, 0x86, 0xe2, 0x86, 0x22, 0x86, 0x22, 0x86, 0x62,
    0x86, 0xa2, 0x86, 0xa2, 0x8a, 0xa2, 0x8a, 0x62, 0x8a, 0x22, 0x8a, 0x62, 0x8a, 0x62, 0x8a, 0xe2,
    0x9b, 0x22, 0x9b, 0xa2, 0x9b, 0x62, 0x9b, 0xe2, 0x9b, 0xa2, 0x9b, 0x22,
  };
  static const uint8 kBomb_DrawExplosion_Ext[54] = {
    2, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 2, 2, 2, 2,
    1, 1, 2, 2, 2, 2, 1, 1, 2, 2, 2, 2, 2, 1, 2, 2,
    2, 2, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    0, 0, 0, 0, 0, 0,
  };
  int base_frame = frame;
  do {
    if (kBomb_DrawExplosion_CharFlags[frame * 2] != 0xff) {
      int i = idx + base_frame;
      Ancilla_SetOam_Safe(oam, x + kBomb_DrawExplosion_XY[i * 2 + 1], y + kBomb_DrawExplosion_XY[i * 2 + 0],
                          kBomb_DrawExplosion_CharFlags[frame * 2],
                          kBomb_DrawExplosion_CharFlags[frame * 2 + 1] & ~0x3E | HIBYTE(oam_priority_value) | r11,
                          kBomb_DrawExplosion_Ext[frame]);
      oam++;
    }
  } while (frame++, ++idx != idx_end);
  return oam;
}

void Ancilla15_JumpSplash(int k) {  // 88a80f
  static const uint8 kAncilla_JumpSplash_Char[2] = {0xac, 0xae};

  if (!submodule_index) {
    if (sign8(--ancilla_aux_timer[k])) {
      ancilla_aux_timer[k] = 0;
      ancilla_item_to_link[k] = 1;
    }
    if (ancilla_item_to_link[k]) {
      ancilla_x_vel[k] = ancilla_y_vel[k] = ancilla_y_vel[k] - 4;
      if (ancilla_y_vel[k] < 232) {
        ancilla_type[k] = 0;
        if ((link_is_bunny_mirror || link_player_handler_state == kPlayerState_Swimming) && link_is_in_deep_water)
          CheckAbilityToSwim();
        return;
      }
      Ancilla_MoveX(k);
      Ancilla_MoveY(k);
    }
  }
  Point16U pt;
  Ancilla_PrepOamCoord(k, &pt);
  OamEnt *oam = GetOamCurPtr();
  int ax = Ancilla_GetX(k);
  int x8 = link_x_coord * 2 - ax - BG2HOFS_copy2;
  int x6 = ax + 12 - BG2HOFS_copy2;
  int j = ancilla_item_to_link[k];
  uint8 flags = 0;
  for (int i = 0; i < 2; i++) {
    Ancilla_SetOam(oam, pt.x, pt.y, kAncilla_JumpSplash_Char[j], 0x24 | flags, 2);
    oam = Ancilla_AllocateOamFromCustomRegion(oam + 1);
    pt.x = x8;
    flags = 0x40;
  }
  Ancilla_SetOam(oam, x6, pt.y, 0xc0, 0x24, ((j == 1) ? 1 : 2));
}

void Ancilla16_HitStars(int k) {  // 88a8e5
  static const uint8 kAncilla_HitStars_Char[2] = {0x90, 0x91};

  if (!sign8(--ancilla_arr3[k]))
    return;

  ancilla_arr3[k] = 0;
  if (!submodule_index) {
    if (sign8(--ancilla_aux_timer[k])) {
      ancilla_aux_timer[k] = 0;
      ancilla_item_to_link[k] = 1;
    }
    if (ancilla_item_to_link[k]) {
      ancilla_x_vel[k] = (ancilla_y_vel[k] -= 4);
      if (ancilla_y_vel[k] < 232) {
        ancilla_type[k] = 0;
        return;
      }
      Ancilla_MoveY(k);
      Ancilla_MoveX(k);
    }
  }
  Point16U info;
  Ancilla_PrepOamCoord(k, &info);

  uint16 ax = Ancilla_GetX(k);
  uint16 tt = ancilla_B[k] << 8 | ancilla_A[k];

  uint16 r8 = 2 * tt - ax - 8 - BG2HOFS_copy2;

  if (ancilla_step[k] == 2)
    Ancilla_AllocateOamFromRegion_B_or_E(8);

  OamEnt *oam = GetOamCurPtr();
  uint16 x = info.x, y = info.y;
  uint8 flags = 0;
  for (int i = 1; i >= 0; i--) {
    Ancilla_SetOam(oam, x, y,
                   kAncilla_HitStars_Char[ancilla_item_to_link[k]],
                   HIBYTE(oam_priority_value) | 4 | flags, 0);
    flags = 0x40;
    BYTE(x) = r8;
    oam = HitStars_UpdateOamBufferPosition(oam + 1);
  }
}

void Ancilla17_ShovelDirt(int k) {  // 88a9a9
  static const int8 kShovelDirt_XY[8] = {18, -13, -9, 4, 18, 13, -9, -11};
  static const int8 kShovelDirt_Char[2] = {0x40, 0x50};
  Point16U pt;
  Ancilla_PrepOamCoord(k, &pt);
  OamEnt *oam = GetOamCurPtr();
  if (!ancilla_timer[k]) {
    ancilla_timer[k] = 8;
    if (++ancilla_item_to_link[k] == 2) {
      ancilla_type[k] = 0;
      return;
    }
  }
  int b = ancilla_item_to_link[k];
  int j = b + ((link_direction_facing == 4) ? 0 : 2);
  pt.x += kShovelDirt_XY[j * 2 + 1];
  pt.y += kShovelDirt_XY[j * 2 + 0];
  for (int i = 0; i < 2; i++) {
    Ancilla_SetOam(oam, pt.x + i * 8, pt.y, kShovelDirt_Char[b] + i, 4 | HIBYTE(oam_priority_value), 0);
    oam = Ancilla_AllocateOamFromCustomRegion(oam + 1);
  }
}

void Ancilla32_BlastWallFireball(int k) {  // 88aa35
  static const uint8 kBlastWallFireball_Char[3] = {0x9d, 0x9c, 0x8d};

  if (!submodule_index) {
    ancilla_item_to_link[k] += 2;
    ancilla_y_vel[k] += ancilla_item_to_link[k];
    Ancilla_MoveY(k);
    Ancilla_MoveX(k);
    if (sign8(--blastwall_var12[k])) {
      ancilla_type[k] = 0;
      return;
    }
  }

  if (sort_sprites_setting)
    Oam_AllocateFromRegionD(4);
  else
    Oam_AllocateFromRegionA(4);

  Point16U pt;
  Ancilla_PrepOamCoord(k, &pt);
  Ancilla_SetOam(GetOamCurPtr(), pt.x, pt.y, kBlastWallFireball_Char[blastwall_var12[k] & 8 ? 0 : blastwall_var12[k] & 4 ? 1 : 2], 0x22, 0);
}

void Ancilla18_EtherSpell(int k) {  // 88aaa0
  if (submodule_index)
    return;

  if (ancilla_step[k] != 0) {
    uint8 flag;

    if (step_counter_for_spin_attack == 0) {
      flag = (++ancilla_arr4[k] & 4) == 0;
    } else {
      flag = step_counter_for_spin_attack == 11;
    }
    if (flag) {
      Palette_ElectroThemedGear();
      Filter_Majorly_Whiten_Bg();
    } else {
      LoadActualGearPalettes();
      Palette_Restore_BG_From_Flash();
    }
  }

  if (ancilla_step[k] == 2) {
    if (sign8(--ancilla_aux_timer[k])) {
      ancilla_aux_timer[k] = 2;
      if (++ancilla_item_to_link[k] == 2) {
        ancilla_item_to_link[k]--;
        ancilla_x_vel[k] = 16;
        ancilla_step[k] = 3;
      }
    }
    ancilla_x_vel[k] += 1;
    EtherSpell_HandleRadialSpin(k);
    return;
  } else {
    if (sign8(--ancilla_aux_timer[k])) {
      ancilla_aux_timer[k] = 2;
      ancilla_item_to_link[k] ^= 1;
    }
    if (ancilla_step[k] == 0) {
      EtherSpell_HandleLightningStroke(k);
    } else if (ancilla_step[k] == 1) {
      EtherSpell_HandleOrbPulse(k);
    } else if (ancilla_step[k] == 3) {
      EtherSpell_HandleRadialSpin(k);
    } else if (ancilla_step[k] == 4) {
      if (!--ether_var1)
        ancilla_step[k] = 5;
      EtherSpell_HandleRadialSpin(k);
    } else {
      uint8 vel = ancilla_x_vel[k] + 0x10;
      if (sign8(vel)) vel = 0x7f;
      ancilla_x_vel[k] = vel;
      EtherSpell_HandleRadialSpin(k);
    }
  }
}

void EtherSpell_HandleLightningStroke(int k) {  // 88ab63
  Ancilla_MoveY(k);
  uint16 y = Ancilla_GetY(k);

  if (BYTE(ether_y_adjusted) != (y & 0xf0)) {
    BYTE(ether_y_adjusted) = y & 0xf0;
    ancilla_arr25[k]++;
  }
  if (y < 0xe000 && ether_y2 < 0xe000 && ether_y2 <= y) {
    ancilla_step[k] = 1;
  }
  AncillaDraw_EtherBlitz(k);
}

void EtherSpell_HandleOrbPulse(int k) {  // 88aba7
  if (!sign8(ancilla_arr25[k])) {
    if (!sign8(--ancilla_arr3[k])) {
      AncillaDraw_EtherBlitz(k);
      return;
    }
    ancilla_arr3[k] = 3;
    if (!sign8(--ancilla_arr25[k])) {
      AncillaDraw_EtherBlitz(k);
      return;
    }
    ancilla_arr3[k] = 9;
  }
  if (sign8(--ancilla_arr3[k])) {
    ancilla_step[k] = 2;
    ancilla_y_vel[k] = 0;
    ancilla_x_vel[k] = 16;
    ancilla_item_to_link[k] = 0;
    ancilla_aux_timer[k] = 2;
    if (step_counter_for_spin_attack)
      Medallion_CheckSpriteDamage(k);
  }
  AncillaDraw_EtherOrb(k, GetOamCurPtr());
}

void EtherSpell_HandleRadialSpin(int k) {  // 88abef
  if (ancilla_step[k] == 4) {
    if ((frame_counter & 7) == 0)
      sound_effect_2 = 0x2a;
    else if ((frame_counter & 7) == 4)
      sound_effect_2 = 0xaa;
    else if ((frame_counter & 7) == 7)
      sound_effect_2 = 0x6a;
  } else {
    ancilla_x_lo[k] = ether_var2;
    ancilla_x_hi[k] = 0;
    Ancilla_MoveX(k);
    ether_var2 = ancilla_x_lo[k];
    if (ether_var2 == 0x40)
      ancilla_step[k] = 4;
  }

  uint8 sb = ancilla_step[k];
  uint8 sa = ancilla_item_to_link[k];
  OamEnt *oam = GetOamCurPtr();
  for (int i = 7; i >= 0; i--) {
    if (sb != 2 && sb != 5) {
      ether_arr1[i] = (ether_arr1[i] + 1) & 0x3f;
    }
    AncillaRadialProjection arp = Ancilla_GetRadialProjection(ether_arr1[i], ether_var2);
    if (sb != 2)
      oam = AncillaDraw_EtherBlitzBall(oam, &arp, sa);
    else
      oam = AncillaDraw_EtherBlitzSegment(oam, &arp, sa, i);
  }
  if (ether_var2 < 0xf0) {
    OamEnt *oam = GetOamCurPtr();
    for (int i = 0; i != 8; i++) {
      if (oam[i].y != 0xf0)
        return;
    }
  }
  ancilla_type[k] = 0;
  load_chr_halfslot_even_odd = 1;
  byte_7E0324 = 0;
  state_for_spin_attack = 0;
  step_counter_for_spin_attack = 0;
  link_cant_change_direction = 0;
  flag_unk1 = 0;

  if (BYTE(overworld_screen_index) == 0x70 && !(save_ow_event_info[0x70] & 0x20) && Ancilla_CheckForEntranceTrigger(2)) {
    trigger_special_entrance = 3;
    subsubmodule_index = 0;
    BYTE(R16) = 0;
  }

  if (link_player_handler_state != kPlayerState_ReceivingEther) {
    link_player_handler_state = kPlayerState_Ground;
    link_delay_timer_spin_attack = 0;
    button_mask_b_y = button_b_frames ? (joypad1H_last & 0x80) : 0;
  }
  link_speed_setting = 0;
  byte_7E0325 = 0;
  LoadActualGearPalettes();
  Palette_Restore_BG_And_HUD();
}

OamEnt *AncillaDraw_EtherBlitzBall(OamEnt *oam, const AncillaRadialProjection *arp, int s) {  // 88aced
  static const uint8 kEther_BlitzBall_Char[2] = {0x68, 0x6a};
  int x = (arp->r6 ? -arp->r4 : arp->r4) + ether_x2 - 8 - BG2HOFS_copy2;
  int y = (arp->r2 ? -arp->r0 : arp->r0) + ether_y3 - 8 - BG2VOFS_copy2;
  Ancilla_SetOam(oam, x, y, kEther_BlitzBall_Char[s], 0x3c, 2);
  return Ancilla_AllocateOamFromCustomRegion(oam + 1);
}

OamEnt *AncillaDraw_EtherBlitzSegment(OamEnt *oam, const AncillaRadialProjection *arp, int s, int k) {  // 88adc9
  static const int8 kEther_SpllittingBlitzSegment_X[16] = {-8, -16, -24, -16, -8, 0, 8, -16, -8, -16, -24, -16, -8, 0, 8, 0};
  static const int8 kEther_SpllittingBlitzSegment_Y[16] = {8, 0, -8, -16, -24, -16, -8, -16, 8, 0, -8, -16, -24, -16, -8, 0};
  static const uint8 kEther_SpllittingBlitzSegment_Char[32] = {
    0x40, 0x42, 0x66, 0x64, 0x62, 0x60, 0x64, 0x66, 0x42, 0x40, 0x66, 0x64, 0x60, 0x62, 0x64, 0x66,
    0x68, 0x42, 0x68, 0x64, 0x68, 0x60, 0x68, 0x64, 0x68, 0x40, 0x68, 0x66, 0x68, 0x62, 0x68, 0x64,
  };
  static const uint8 kEther_SpllittingBlitzSegment_Flags[32] = {
    0x3c, 0x3c, 0xfc, 0xfc, 0x3c, 0x3c, 0xbc, 0xbc, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x7c, 0x7c,
    0x3c, 0x7c, 0x3c, 0x3c, 0x3c, 0xbc, 0x3c, 0x7c, 0x3c, 0x7c, 0x3c, 0xfc, 0x3c, 0xbc, 0x3c, 0xbc,
  };
  int x = (arp->r6 ? -arp->r4 : arp->r4);
  int y = (arp->r2 ? -arp->r0 : arp->r0);
  int t = s * 8 + k;
  Ancilla_SetOam(oam, x + ether_x2 - 8 - BG2HOFS_copy2, y + ether_y3 - 8 - BG2VOFS_copy2,
                 kEther_SpllittingBlitzSegment_Char[t * 2], kEther_SpllittingBlitzSegment_Flags[t * 2], 2);
  Ancilla_SetOam(oam + 1,
      x + ether_x2 + kEther_SpllittingBlitzSegment_X[t] - BG2HOFS_copy2,
      y + ether_y3 + kEther_SpllittingBlitzSegment_Y[t] - BG2VOFS_copy2,
      kEther_SpllittingBlitzSegment_Char[t * 2 + 1],
      kEther_SpllittingBlitzSegment_Flags[t * 2 + 1], 2);
  return Ancilla_AllocateOamFromCustomRegion(oam + 2);
}

void AncillaDraw_EtherBlitz(int k) {  // 88ae87
  Point16U info;
  Ancilla_PrepOamCoord(k, &info);
  OamEnt *oam = GetOamCurPtr();
  int t = ancilla_item_to_link[k];
  int i = ancilla_arr25[k];
  int m = 0;
  do {
    Ancilla_SetOam(oam, info.x, info.y,
                   kEther_BlitzSegment_Char[t * 2 + m],
                   kEther_BlitzOrb_Flags[0] | HIBYTE(oam_priority_value), 2);
    info.y -= 16;
    oam++;
    m ^= 1;
  } while (--i >= 0);
  if (ancilla_step[k] == 1)
    AncillaDraw_EtherOrb(k, oam);
}

void AncillaDraw_EtherOrb(int k, OamEnt *oam) {  // 88aedd
  uint16 y = ether_y - 1 - BG2VOFS_copy2;
  uint16 x = ether_x - 8 - BG2HOFS_copy2;
  int t = ancilla_item_to_link[k] * 4;

  for (int i = 0; i < 4; i++) {
    Ancilla_SetOam(oam, x, y, kEther_BlitzOrb_Char[t + i], kEther_BlitzOrb_Flags[t + i], 2);
    oam++;
    oam = Ancilla_AllocateOamFromCustomRegion(oam);
    x += 16;
    if (i == 1)
      x -= 32, y += 16;
  }
}

void AncillaAdd_BombosSpell(uint8 a, uint8 y) {  // 88af66
  int k = AncillaAdd_AddAncilla_Bank08(a, y);
  if (k < 0)
    return;
  for (int i = 0; i < 10; i++) {
    bombos_arr2[i] = 0;
    bombos_arr1[i] = 3;
  }
  for (int i = 0; i < 8; i++) {
    bombos_arr3[i] = 0;
    bombos_arr4[i] = 3;
  }
  bombos_var4 = 0;
  bombos_var2 = 0;
  bombos_var3 = 0x80;
  bombos_arr7[0] = 0x10;
  load_chr_halfslot_even_odd = 11;
  flag_custom_spell_anim_active = 1;
  ancilla_step[k] = 0;
  ancilla_item_to_link[k] = 0;
  Ancilla_Sfx2_Near(0x2a);

  uint8 t = kGeneratedBombosArr[frame_counter];
  t = (t < 0xe0) ? t : t & 0x7f;
  bombos_x_coord[0] = link_x_coord & ~0xff | t;
  bombos_y_coord[0] = link_y_coord & ~0xff | t;

  static const int16 kBombos_YDelta[4] = {16, 24, -128, -16};
  static const int16 kBombos_XDelta[4] = {-16, -128, 0, 128};

  for (int i = 0; i < 1 ; i++) {
    bombos_x_coord2[i] = link_x_coord + kBombos_XDelta[i];
    bombos_y_coord2[i] = link_y_coord + kBombos_YDelta[i];
    bombos_var1 = 16;
    AncillaRadialProjection arp = Ancilla_GetRadialProjection(bombos_arr7[i], 16);
    int x = (arp.r6 ? -arp.r4 : arp.r4) + bombos_x_coord2[i];
    int y = (arp.r2 ? -arp.r0 : arp.r0) + bombos_y_coord2[i];
    bombos_x_lo[i] = (uint8)x;
    bombos_x_hi[i] = x >> 8;
    bombos_y_lo[i] = (uint8)y;
    bombos_y_hi[i] = y >> 8;
  }
}

void Ancilla19_BombosSpell(int k) {  // 88b0ce
  if (bombos_var4 == 0) {
    if (submodule_index == 0) {
      BombosSpell_ControlFireColumns(k);
      return;
    }
    for (int i = 9; i >= 0; i--)
      AncillaDraw_BombosFireColumn(i);
  } else if (bombos_var4 != 2) {
    if (submodule_index == 0) {
      BombosSpell_FinishFireColumns(k);
      return;
    }
    for (int i = 9; i >= 0; i--)
      AncillaDraw_BombosFireColumn(i);
  } else {
    if (submodule_index == 0) {
      BombosSpell_ControlBlasting(k);
      return;
    }
    int i = ancilla_step[k];
    do {
      AncillaDraw_BombosBlast(i);
    } while (--i >= 0);
  }
}

void BombosSpell_ControlFireColumns(int k) {  // 88b10a
  uint8 sa = ancilla_item_to_link[k];
  uint8 sb = ancilla_step[k];

  int j, i = sb;
  do {
    if (bombos_arr2[i] == 13)
      continue;

    if (sign8(--bombos_arr1[i])) {
      bombos_arr1[i] = 3;
      if (++bombos_arr2[i] == 13)
        continue;

      if (bombos_arr2[i] == 2) {
        if (sa)
          continue;

        // pushed x
        if (sb == 9) {
          for (j = 9; j >= 0; j--) {
            if (bombos_arr2[j] == 13) {
              bombos_arr2[j] = 0;
              goto exit_loop;
            }
          }
        }
        sb = j = (sb + 1) != 10 ? sb + 1 : 9;
exit_loop:
        bombos_var1 = (bombos_var1 + 3 >= 207) ? 207 : bombos_var1 + 3;
        bombos_arr7[0] += 6;
        AncillaRadialProjection arp = Ancilla_GetRadialProjection(bombos_arr7[0] & 0x3f, bombos_var1);
        int x = (arp.r6 ? -arp.r4 : arp.r4) + bombos_x_coord2[0];
        int y = (arp.r2 ? -arp.r0 : arp.r0) + bombos_y_coord2[0];
        bombos_x_lo[j] = (uint8)x;
        bombos_x_hi[j] = x >> 8;
        bombos_y_lo[j] = (uint8)y;
        bombos_y_hi[j] = y >> 8;

        uint16 t = x - BG2HOFS_copy2 + 8;
        if (t < 256)
          sound_effect_1 = kBombos_Sfx[t >> 5] | 0x2a;
      }
    }
    AncillaDraw_BombosFireColumn(i);

  } while (--i >= 0);
  if (bombos_arr7[0] >= 0x80)
    bombos_var4 = 1;
  ancilla_step[k] = sb;
}

void BombosSpell_FinishFireColumns(int kk) {  // 88b236
  int k = ancilla_step[kk];
  do {
    if (sign8(--bombos_arr1[k])) {
      bombos_arr1[k] = 3;
      if (++bombos_arr2[k] >= 13)
        bombos_arr2[k] = 13;
    }
    AncillaDraw_BombosFireColumn(k);
  } while (--k >= 0);
  for (int k = 9; k >= 0; k--) {
    if (bombos_arr2[k] != 13)
      return;
  }
  bombos_var4 = 2;
  Medallion_CheckSpriteDamage(kk);
  ancilla_step[kk] = 0;
}

void AncillaDraw_BombosFireColumn(int kk) {  // 88b373
  static const int8 kBombosSpell_FireColumn_X[39] = {
     0, -1, -1,  0, 0, -1,  0, 0, -1, 0, 0, -1, 0, 0, -1, 0,
     0,  0,  0,  0, 0,  0,  0, 0,  0, 0, 0,  0, 0, 0,  0, 0,
    -1,  1, -1, -1, 2, -1, -1,
  };
  static const int8 kBombosSpell_FireColumn_Y[39] = {
     0,  -1, -1,  0,  -4, -1,   0,  -8, -1,   0, -12, -1,   0, -16,  -1,   0,
    -4, -20,  0, -8, -24,  0, -12, -28,  0, -16, -32,  0, -16, -32, -18, -34,
    -1, -35, -1, -1, -36, -1,  -1,
  };
  static const uint8 kBombosSpell_FireColumn_Flags[39] = {
    0x3c, 0xff, 0xff, 0x3c, 0x3c, 0xff, 0x3c, 0x3c, 0xff, 0x7c, 0x7c, 0xff, 0x3c, 0x7c, 0xff, 0x3c,
    0x3c, 0x3c, 0xbc, 0x3c, 0x3c, 0x7c, 0x3c, 0x3c, 0x3c, 0x3c, 0x7c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c,
    0xff, 0x3c, 0xff, 0xff, 0x3c, 0xff, 0xff,
  };
  static const uint8 kBombosSpell_FireColumn_Char[39] = {
    0x40, 0xff, 0xff, 0x42, 0x44, 0xff, 0x42, 0x44, 0xff, 0x42, 0x44, 0xff, 0x42, 0x44, 0xff, 0x40,
    0x46, 0x44, 0x4a, 0x4a, 0x48, 0x4c, 0x4c, 0x4a, 0x4e, 0x4c, 0x4a, 0x4e, 0x6a, 0x4c, 0x4e, 0x68,
    0xff, 0x6a, 0xff, 0xff, 0x4e, 0xff, 0xff,
  };
  Ancilla_AllocateOamFromRegion_A_or_D_or_F(kk, 0x10);
  OamEnt *oam = GetOamCurPtr();
  for (int i = 0; i < 1; i++) {
    int k = bombos_arr2[kk];
    if (k == 13)
      continue;
    k = k * 3 + 2;
    for (int j = 0; j < 3; j++, k--) {
      if (kBombosSpell_FireColumn_Char[k] != 0xff) {
        uint16 x = bombos_x_lo[kk] | bombos_x_hi[kk] << 8;
        uint16 y = bombos_y_lo[kk] | bombos_y_hi[kk] << 8;
        y += kBombosSpell_FireColumn_Y[k] - BG2VOFS_copy2;
        x += kBombosSpell_FireColumn_X[k] - BG2HOFS_copy2;
        Ancilla_SetOam(oam, x, y,
                       kBombosSpell_FireColumn_Char[k],
                       kBombosSpell_FireColumn_Flags[k], 2);
        oam++;
      }
      oam = Ancilla_AllocateOamFromCustomRegion(oam);
    }
  }
}

void BombosSpell_ControlBlasting(int kk) {  // 88b40d
  int k = ancilla_step[kk], sb = k;
  for (; k >= 0; k--) {
    if (bombos_arr3[k] != 8 && sign8(--bombos_arr4[k])) {
      bombos_arr4[k] = 3;
      if (++bombos_arr3[k] == 1 && !bombos_var2) {
        int j = sb;
        if (j != 15) {
          j = ++sb;
        } else {
          for (; j >= 0 && bombos_arr3[j] != 8; j--) {}
        }
        bombos_arr3[j] = 0;
        bombos_arr4[j] = 3;

        uint16 y = kBombosBlasts_Tab[frame_counter & 0x3f];
        uint16 x = kBombosBlasts_Tab[(frame_counter & 0x3f) + 3];
        bombos_y_coord[j] = y + BG2VOFS_copy2;
        bombos_x_coord[j] = x + BG2HOFS_copy2;

        sound_effect_1 = 0xc | kBombos_Sfx[bombos_x_coord[j] >> 5 & 7];
      }
    }
    AncillaDraw_BombosBlast(k);
  }

  for (int j = 15; j >= 0; j--) {
    if (bombos_arr3[j] != 8) {
      ancilla_step[kk] = sb;
      goto getout;
    }
  }
  ancilla_type[kk] = 0;
  load_chr_halfslot_even_odd = 1;
  byte_7E0324 = 0;
  state_for_spin_attack = 0;
  step_counter_for_spin_attack = 0;
  link_cant_change_direction = 0;
  flag_unk1 = 0;
  if (link_player_handler_state != kPlayerState_ReceivingBombos) {
    link_player_handler_state = kPlayerState_Ground;
    link_delay_timer_spin_attack = 0;
    button_mask_b_y = button_b_frames ? (joypad1H_last & 0x80) : 0;
  }
  link_speed_setting = 0;
  byte_7E0325 = 0;
getout:
  if (--bombos_var3 == 0)
    bombos_var2 = bombos_var3 = 1;
}

void AncillaDraw_BombosBlast(int k) {  // 88b5e1
  static const int8 kBombosSpell_DrawBlast_X[32] = {
     -8, -1,  -1, -1, -12, -4, -12, -4, -16, 0, -16, 0, -16, 0, -16, 0,
    -17,  1, -17,  1, -19,  3, -19,  3, -19, 3, -19, 3, -19, 3, -19, 3,
  };
  static const int8 kBombosSpell_DrawBlast_Y[32] = {
     -8,  -1, -1, -1, -12, -12, -4, -4, -16, -16, 0, 0, -16, -16, 0, 0,
    -17, -17,  1,  1, -19, -19,  3,  3, -19, -19, 3, 3, -19, -19, 3, 3,
  };
  static const uint8 kBombosSpell_DrawBlast_Flags[32] = {
    0x3c, 0xff, 0xff, 0xff, 0x3c, 0x7c, 0xbc, 0xfc, 0x3c, 0x7c, 0xbc, 0xfc, 0x3c, 0x7c, 0xbc, 0xfc,
    0x3c, 0x7c, 0xbc, 0xfc, 0x3c, 0x7c, 0xbc, 0xfc, 0x3c, 0x7c, 0xbc, 0xfc, 0x3c, 0x7c, 0xbc, 0xfc,
  };
  static const uint8 kBombosSpell_DrawBlast_Char[32] = {
    0x60, 0xff, 0xff, 0xff, 0x62, 0x62, 0x62, 0x62, 0x64, 0x64, 0x64, 0x64, 0x66, 0x66, 0x66, 0x66,
    0x68, 0x68, 0x68, 0x68, 0x68, 0x68, 0x68, 0x68, 0x6a, 0x6a, 0x6a, 0x6a, 0x4e, 0x4e, 0x4e, 0x4e,
  };
  uint16 x = bombos_x_coord[k];
  uint16 y = bombos_y_coord[k];
  if (bombos_arr3[k] == 8)
    return;

  Ancilla_AllocateOamFromRegion_A_or_D_or_F(k, 0x10);
  OamEnt *oam = GetOamCurPtr();

  int t = bombos_arr3[k] * 4 + 3;
  for (int j = 0; j < 4; j++, t--) {
    if (kBombosSpell_DrawBlast_Char[t] != 0xff) {
      Ancilla_SetOam(oam,
        x + kBombosSpell_DrawBlast_X[t] - BG2HOFS_copy2,
        y + kBombosSpell_DrawBlast_Y[t] - BG2VOFS_copy2,
        kBombosSpell_DrawBlast_Char[t],
        kBombosSpell_DrawBlast_Flags[t], 2);
      oam++;
    }
    oam = Ancilla_AllocateOamFromCustomRegion(oam);
  }

}

void Ancilla1C_QuakeSpell(int k) {  // 88b66a
  if (submodule_index != 0) {
    if (quake_arr2[4] != kQuake_Tab1[4])
      AncillaDraw_QuakeInitialBolts(k);
    return;
  }
  if (ancilla_step[k] != 2) {
    QuakeSpell_ShakeScreen(k);
    QuakeSpell_ControlBolts(k);
    QuakeSpell_SpreadBolts(k);
    return;
  }
  Medallion_CheckSpriteDamage(k);
  Prepare_ApplyRumbleToSprites();
  ancilla_type[k] = 0;
  link_player_handler_state = 0;
  load_chr_halfslot_even_odd = 1;
  byte_7E0324 = 0;
  state_for_spin_attack = 0;
  step_counter_for_spin_attack = 0;
  link_cant_change_direction = 0;
  link_delay_timer_spin_attack = 0;
  flag_unk1 = 0;
  bg1_x_offset = 0;
  bg1_y_offset = 0;
  if (BYTE(overworld_screen_index) == 0x47 && !(save_ow_event_info[0x47] & 0x20) && Ancilla_CheckForEntranceTrigger(3)) {
    trigger_special_entrance = 4;
    subsubmodule_index = 0;
    BYTE(R16) = 0;
  }
  button_mask_b_y = button_b_frames ? (joypad1H_last & 0x80) : 0;
  link_speed_setting = 0;
  byte_7E0325 = 0;
}

void QuakeSpell_ShakeScreen(int k) {  // 88b6f7
  bg1_y_offset = quake_var3;
  quake_var3 = -quake_var3;
  link_y_vel += bg1_y_offset;
}

void QuakeSpell_ControlBolts(int k) {  // 88b718
  quake_var4 = ancilla_step[k];
  int j = quake_var5;
  do {
    if (quake_arr2[j] == kQuake_Tab1[j])
      continue;

    if (sign8(--quake_arr1[j])) {
      quake_arr1[j] = 1;
      if (++quake_arr2[j] == kQuake_Tab1[j])
        continue;

      if (j == 0 && quake_arr2[j] == 2) {
        Ancilla_Sfx2_Near(0xc);
        quake_var5 = 1;
      } else if (j == 1 && quake_arr2[j] == 2) {
        quake_var5 = 4;
      } else if (j == 4 && quake_arr2[j] == 7) {
        quake_var4 = 1;
      }
    }
    AncillaDraw_QuakeInitialBolts(j);
  } while (--j >= 0);
  ancilla_step[k] = quake_var4;
}

void AncillaDraw_QuakeInitialBolts(int k) {  // 88b793
  static const uint8 kQuakeDrawGroundBolts_Tab[5] = {0, 0x18, 0, 0x18, 0x2f};

  int t = quake_arr2[k] + kQuakeDrawGroundBolts_Tab[k];
  OamEnt *oam = GetOamCurPtr();
  int idx = kQuakeItemPos[t], num = kQuakeItemPos[t + 1] - idx;
  const QuakeItem *p = &kQuakeItems[idx], *pend = p + num;
  do {
    uint16 x = p->x + quake_var2 - BG2HOFS_copy2;
    uint16 y = p->y + quake_var1 - BG2VOFS_copy2;

    uint8 xval = oam->x, yval = 0xf0;
    if (x < 256 && y < 256) {
      xval = x;
      if (y < 0xf0)
        yval = y;
    }
    SetOamPlain(oam, xval, yval, kQuakeDrawGroundBolts_Char[p->f & 0x0f], p->f & 0xc0 | 0x3c, 2);
    oam++, oam_cur_ptr += 4, oam_ext_cur_ptr += 1;
  } while (++p != pend);
}

void QuakeSpell_SpreadBolts(int k) {  // 88b84f
  if (ancilla_step[k] != 1)
    return;
  if (ancilla_timer[k] == 0) {
    ancilla_timer[k] = 2;
    if (++ancilla_item_to_link[k] == 55) {
      ancilla_step[k] = 2;
      return;
    }
  }
  int t = ancilla_item_to_link[k];
  int idx = kQuakeItemPos2[t], num = kQuakeItemPos2[t + 1] - idx;
  const QuakeItem *p = &kQuakeItems2[idx], *pend = p + num;
  OamEnt *oam = GetOamCurPtr();
  do {
    SetOamPlain(oam, p->x, p->y, kQuakeDrawGroundBolts_Char[p->f & 0x0f], p->f & 0xc0 | 0x3c, p->f >> 4 & 3);
    oam_cur_ptr += 4, oam_ext_cur_ptr += 1;
    oam = Ancilla_AllocateOamFromCustomRegion(oam + 1);
  } while (++p != pend);
}

void Ancilla1A_PowderDust(int k) {  // 88bab0
  if (submodule_index == 0) {
    Powder_ApplyDamageToSprites(k);
    if (sign8(--ancilla_aux_timer[k])) {
      ancilla_aux_timer[k] = 1;
      int j = ancilla_dir[k];
      if (ancilla_item_to_link[k] == 9) {
        ancilla_type[k] = 0;
        byte_7E0333 = 0;
        return;
      }
      ancilla_arr25[k] = kMagicPowder_Tab0[++ancilla_item_to_link[k] + j * 10];
    }
  }
  Ancilla_AllocateOamFromRegion_B_or_E(ancilla_numspr[k]);
  Ancilla_MagicPowder_Draw(k);
}

void Ancilla_MagicPowder_Draw(int k) {  // 88baeb
  static const int8 kMagicPowder_DrawX[76] = {
    -5, -12,  2,  -9, -7, -10, -6, -2, -6, -12,  1, -6,  -6, -12,   1,  -6,
    -6, -12,  1,  -6, -6, -12,  1, -6, -6, -12,  1, -6, -17, -23, -14, -19,
    -11, -18, -9, -13, -4, -13, -1, -8, -3,  -9,  0, -5,  -3, -10,  -1,  -5,
    -4, -13, -1,  -8, -3,  -9,  0, -5, -3, -10, -1, -5,  -3, -13,  -1,  -8,
    9,  15,  6,  11,  3,  10,  1,  5, -4,   5, -7,  0,
  };
  static const int8 kMagicPowder_DrawY[76] = {
    -20, -15, -13,  -7, -18, -13, -13, -13, -20, -13, -13,  -8, -20, -13, -13,  -8,
    -19, -12, -12,  -7, -18, -11, -11,  -6, -17, -10, -10,  -5, -16, -14, -12,  -9,
    -17, -14, -12,  -8, -18, -14, -13,  -6, -33, -31, -29, -26, -28, -25, -23, -19,
    -22, -18, -17, -10,  -2,   0,   2,   5,  -9,  -6,  -4,   0, -16, -12, -11,  -4,
    -16, -14, -12,  -9, -17, -14, -12,  -8, -18, -14, -13,  -6,
  };
  static const uint8 kMagicPowder_Draw_Char[19] = {
    9, 10, 10, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
  };
  static const uint8 kMagicPowder_Draw_Flags[76] = {
    0x68, 0x24, 0xa2, 0x28, 0x68, 0xe2, 0x28, 0xa4, 0x68, 0xe2, 0xa4, 0x28, 0x22, 0xa4, 0xe8, 0x62,
    0x24, 0xa8, 0xe2, 0x64, 0x28, 0xa2, 0xe4, 0x68, 0x22, 0xa4, 0xe8, 0x62, 0xe2, 0xa4, 0xe8, 0x64,
    0xe8, 0xa8, 0xe4, 0x62, 0xe4, 0xa8, 0xe2, 0x68, 0xe2, 0xa4, 0xe8, 0x64, 0xe8, 0xa8, 0xe4, 0x62,
    0xe4, 0xa8, 0xe2, 0x68, 0xe2, 0xa4, 0xe8, 0x64, 0xe8, 0xa8, 0xe4, 0x62, 0xe4, 0xa8, 0xe2, 0x68,
    0xe2, 0xa4, 0xe8, 0x64, 0xe8, 0xa8, 0xe4, 0x62, 0xe4, 0xa8, 0xe2, 0x68,
  };
  Point16U info;
  Ancilla_PrepOamCoord(k, &info);
  OamEnt *oam = GetOamCurPtr();
  int b = ancilla_arr25[k];
  for (int i = 0; i < 4; i++, oam++) {
    Ancilla_SetOam(oam, info.x + kMagicPowder_DrawX[b * 4 + i], info.y + kMagicPowder_DrawY[b * 4 + i],
                   kMagicPowder_Draw_Char[b], kMagicPowder_Draw_Flags[b * 4 + i] & ~0x30 | HIBYTE(oam_priority_value), 0);
  }
}

void Powder_ApplyDamageToSprites(int k) {  // 88bb58
  uint8 a;
  for (int j = 15; j >= 0; j--) {
    if ((frame_counter ^ j) & 3 || sprite_state[j] != 9 || sprite_bump_damage[j] & 0x20)
      continue;
    SpriteHitBox hb;
    Ancilla_SetupBasicHitBox(k, &hb);
    Sprite_SetupHitBox(j, &hb);
    if (!CheckIfHitBoxesOverlap(&hb))
      continue;

    if ((a = sprite_type[j]) != 0xb || (a = player_is_indoors) == 0 || (a = dungeon_room_index2 - 1) != 0) {
      if (a != 0xd) {
        Ancilla_CheckDamageToSprite_preset(j, 10);
        continue;
      }
      if (sprite_head_dir[j] != 0)
        continue;
    }
    sprite_head_dir[j] = 1;
    Sprite_SpawnPoofGarnish(j);
  }
}

void Ancilla1D_ScreenShake(int k) {  // 88bbbc
  if (submodule_index == 0) {
    if (sign8(--ancilla_item_to_link[k])) {
      bg1_x_offset = 0;
      bg1_y_offset = 0;
      ancilla_type[k] = 0;
      return;
    }
    int offs = DashTremor_TwiddleOffset(k);
    int j = ancilla_dir[k];
    if (j == 0) {
      bg1_x_offset = offs;
      link_x_vel += offs;
    } else {
      bg1_y_offset = offs;
      link_y_vel += offs;
    }
  }
  sprite_alert_flag = 3;
}

void Ancilla1E_DashDust(int k) {  // 88bc92
  if (ancilla_step[k]) {
    DashDust_Motive(k);
    return;
  }
  if (!ancilla_timer[k]) {
    ancilla_timer[k] = 3;
    if (++ancilla_item_to_link[k] == 5)
      return;
    if (ancilla_item_to_link[k] == 6) {
      ancilla_type[k] = 0;
      return;
    }
  }
  if (ancilla_item_to_link[k] == 5)
    return;

  Point16U info;
  Ancilla_PrepOamCoord(k, &info);
  OamEnt *oam = GetOamCurPtr();

  static const int8 kDashDust_Draw_X1[4] = {0, 0, 4, -4};
  static const int16 kDashDust_Draw_X[30] = {
    10,  5, -1,  0, 10, 5, 0,  5, -1,  0, -1, -1,  9, -1, -1, 10,
    5, -1,  0, 10,  5, 0, 5, -1,  0, -1, -1,  9, -1, -1,
  };
  static const int16 kDashDust_Draw_Y[30] = {
    -2,  0, -1, -3, -2,  0, -3,  0, -1, -3, -1, -1, -2, -1, -1, -2,
    0, -1, -3, -2,  0, -3,  0, -1, -3, -1, -1, -2, -1, -1,
  };
  static const uint8 kDashDust_Draw_Char[30] = {
    0xcf, 0xa9, 0xff, 0xa9, 0xdf, 0xcf, 0xcf, 0xdf, 0xff, 0xdf, 0xff, 0xff, 0xa9, 0xff, 0xff, 0xcf,
    0xcf, 0xff, 0xcf, 0xdf, 0xcf, 0xcf, 0xdf, 0xff, 0xdf, 0xff, 0xff, 0xcf, 0xff, 0xff,
  };
  int r12 = kDashDust_Draw_X1[link_direction_facing >> 1];
  int t = 3 * (ancilla_item_to_link[k] + (draw_water_ripples_or_grass == 1 ? 5 : 0));

  for (int n = 2; n >= 0; n--, t++) {
    if (kDashDust_Draw_Char[t] != 0xff) {
      Ancilla_SetOam(oam, info.x + r12 + kDashDust_Draw_X[t], info.y + kDashDust_Draw_Y[t], kDashDust_Draw_Char[t], 4 | HIBYTE(oam_priority_value), 0);
      oam++;
    }
  }
}

void Ancilla1F_Hookshot(int k) {  // 88bd74
  if (submodule_index != 0)
    goto do_draw;

  if (!ancilla_timer[k]) {
    ancilla_timer[k] = 7;
    Ancilla_Sfx2_Pan(k, 0xa);
  }

  if (related_to_hookshot)
    goto do_draw;
  Ancilla_MoveY(k);
  Ancilla_MoveX(k);
  if (ancilla_step[k]) {
    if (sign8(--ancilla_item_to_link[k])) {
      ancilla_type[k] = 0;
      return;
    }
    goto do_draw;
  }

  if (++ancilla_item_to_link[k] == 32) {
    ancilla_step[k] = 1;
    ancilla_x_vel[k] = -ancilla_x_vel[k];
    ancilla_y_vel[k] = -ancilla_y_vel[k];
  }

  if (Hookshot_ShouldIEvenBotherWithTiles(k))
    goto do_draw;

  if (!ancilla_L[k] && !ancilla_step[k] && Ancilla_CheckSpriteCollision(k) >= 0 && !ancilla_step[k]) {
    ancilla_step[k] = 1;
    ancilla_y_vel[k] = -ancilla_y_vel[k];
    ancilla_x_vel[k] = -ancilla_x_vel[k];
  }

  Hookshot_CheckTileCollision(k);

  uint8 r0;

  r0 = 0;

  if (player_is_indoors) {
    if (!(ancilla_dir[k] & 2)) {
      r0 = (tiledetect_vertical_ledge | (tiledetect_vertical_ledge >> 4)) & 3;
    } else {
      r0 = detection_of_ledge_tiles_horiz_uphoriz & 3;
    }
    if (r0 == 0)
      goto endif_7;
  } else {
    if (!((detection_of_ledge_tiles_horiz_uphoriz & 3 | tiledetect_vertical_ledge | detection_of_unknown_tile_types) & 0x33))
      goto endif_7;
  }
  if (sign8(--ancilla_G[k])) {
    if (ancilla_K[k] && ((r0 & 3) || ancilla_K[k] != BYTE(index_of_interacting_tile))) {
      ancilla_G[k] = 2;
      if (sign8(--ancilla_L[k]))
        ancilla_L[k] = 0;
    } else {
      ancilla_L[k]++;
      ancilla_K[k] = index_of_interacting_tile;
      ancilla_G[k] = 1;
    }
  }
endif_7:
  if (ancilla_L[k])
    goto do_draw;
  if (!sign8(ancilla_G[k])) {
    ancilla_G[k]--;
    goto do_draw;
  }

  if ((R14 >> 4 | R14 | tiledetect_stair_tile | R12) & 3 && !ancilla_step[k]) {
    ancilla_step[k] = 1;
    ancilla_y_vel[k] = -ancilla_y_vel[k];
    ancilla_x_vel[k] = -ancilla_x_vel[k];
    if (!(tiledetect_misc_tiles & 3)) {
      AncillaAdd_HookshotWallClink(k, 6, 1);
      Ancilla_Sfx2_Pan(k, (tiledetect_misc_tiles & 0x30) ? 6 : 5);
    }
  }

  if (tiledetect_misc_tiles & 3) {
    if (ancilla_item_to_link[k] < 4) {
      ancilla_type[k] = 0;
      return;
    }
    related_to_hookshot = 1;
    hookshot_effect_index = k;
  }

  static const int8 kHookShot_Move_X[4] = {0, 0, 8, -8};
  static const int8 kHookShot_Move_Y[4] = {8, -9, 0, 0};
  static const uint8 kHookShot_Draw_Flags[12] = {0, 0, 0xff, 0x80, 0x80, 0xff, 0x40, 0xff, 0x40, 0, 0xff, 0};
  static const uint8 kHookShot_Draw_Char[12] = {9, 0xa, 0xff, 9, 0xa, 0xff, 9, 0xff, 0xa, 9, 0xff, 0xa};

  Point16U info;
do_draw:
  Ancilla_PrepOamCoord(k, &info);
  if (ancilla_L[k])
    oam_priority_value = 0x3000;
  OamEnt *oam = GetOamCurPtr();

  int j = ancilla_dir[k] * 3;
  int x = info.x, y = info.y;
  for (int i = 2; i >= 0; i--, j++) {
    if (kHookShot_Draw_Char[j] != 0xff) {
      Ancilla_SetOam(oam, x, y, kHookShot_Draw_Char[j], kHookShot_Draw_Flags[j] | 2 | HIBYTE(oam_priority_value), 0);
      oam++;
    }
    if (i == 1)
      x -= 8, y += 8;
    else
      x += 8;
  }

  int r10 = 0;
  int n = ancilla_item_to_link[k] >> 1;
  if (n >= 7) {
    r10 = n - 7;
    n = 6;
  }
  if (n == 0)
    return;
  if (ancilla_dir[k] & 1)
    r10 = -r10;
  x = info.x, y = info.y;
  j = ancilla_dir[k];
  if (kHookShot_Move_Y[j] == 0)
    y += 4;
  if (kHookShot_Move_X[j] == 0)
    x += 4;
  do {
    if (kHookShot_Move_Y[j])
      y += kHookShot_Move_Y[j] + r10;
    if (kHookShot_Move_X[j])
      x += kHookShot_Move_X[j] + r10;
    if (!Hookshot_CheckProximityToLink(x, y)) {
      Ancilla_SetOam(oam, x, y, 0x19, (frame_counter & 2) << 6 | 2 | HIBYTE(oam_priority_value), 0);
      oam++;
    }
  } while (--n >= 0);
}

void Ancilla20_Blanket(int k) {  // 88c013
  static const uint8 kBedSpread_Char[8] = {0xa, 0xa, 0xa, 0xa, 0xc, 0xc, 0xa, 0xa};
  static const uint8 kBedSpread_Flags[8] = {0, 0x60, 0xa0, 0xe0, 0, 0x60, 0xa0, 0xe0};
  Point16U pt;
  Ancilla_PrepOamCoord(k, &pt);

  if (!link_pose_during_opening) {
    Oam_AllocateFromRegionB(0x10);
  } else {
    Oam_AllocateFromRegionA(0x10);
  }

  OamEnt *oam = GetOamCurPtr();
  int j = link_pose_during_opening ? 4 : 0;
  uint16 x = pt.x, y = pt.y;
  for (int i = 3; i >= 0; i--, j++, oam++) {
    Ancilla_SetOam(oam, x, y, kBedSpread_Char[j], kBedSpread_Flags[j] | 0xd | HIBYTE(oam_priority_value), 2);
    x += 16;
    if (i == 2)
      x -= 32, y += 8;
  }
}

void Ancilla21_Snore(int k) {  // 88c094
  static const uint8 kBedSpread_Dma[3] = {0x44, 0x43, 0x42};
  if (sign8(--ancilla_aux_timer[k])) {
    if (ancilla_item_to_link[k] != 2)
      ancilla_item_to_link[k]++;
    ancilla_aux_timer[k] = 7;
  }
  ancilla_x_vel[k] += ancilla_step[k];
  if (abs8(ancilla_x_vel[k]) >= 8)
    ancilla_step[k] = -ancilla_step[k];
  Ancilla_MoveY(k);
  Ancilla_MoveX(k);
  if (Ancilla_GetY(k) <= (uint16)(link_y_coord - 24))
    ancilla_type[k] = 0;
  link_dma_var5 = kBedSpread_Dma[ancilla_item_to_link[k]];
  Point16U pt;
  Ancilla_PrepOamCoord(k, &pt);
  Ancilla_SetOam(GetOamCurPtr(), pt.x, pt.y, 9, 0x24, 0);
}

void Ancilla3B_SwordUpSparkle(int k) {  // 88c167
  static const int8 kAncilla_VictorySparkle_X[16] = {16, 0, 0, 0, 8, 16, 8, 16, 9, 15, 0, 0, 12, 0, 0, 0};
  static const int8 kAncilla_VictorySparkle_Y[16] = {-7, 0, 0, 0, -11, -11, -3, -3, -7, -7, 0, 0, -7, 0, 0, 0};
  static const uint8 kAncilla_VictorySparkle_Char[16] = {0x92, 0xff, 0xff, 0xff, 0x93, 0x93, 0x93, 0x93, 0xf9, 0xf9, 0xff, 0xff, 0x80, 0xff, 0xff, 0xff};
  static const uint8 kAncilla_VictorySparkle_Flags[16] = {0, 0xff, 0xff, 0xff, 0, 0x40, 0x80, 0xc0, 0, 0x40, 0xff, 0xff, 0, 0xff, 0xff, 0xff};

  if (ancilla_aux_timer[k]) {
    ancilla_aux_timer[k]--;
    return;
  }

  if (sign8(--ancilla_arr3[k])) {
    ancilla_arr3[k] = 1;
    if (++ancilla_item_to_link[k] == 4) {
      ancilla_type[k] = 0;
      ancilla_aux_timer[k]--;
      return;
    }
  }
  Point16U pt;
  Ancilla_PrepOamCoord(k, &pt);
  OamEnt *oam = GetOamCurPtr();
  int j = ancilla_item_to_link[k] * 4;
  for (int i = 0; i < 4; i++, j++) {
    if (kAncilla_VictorySparkle_Char[j] != 0xff) {
      Ancilla_SetOam(oam,
                     link_x_coord + kAncilla_VictorySparkle_X[j] - BG2HOFS_copy2,
                     link_y_coord + kAncilla_VictorySparkle_Y[j] - BG2VOFS_copy2,
                     kAncilla_VictorySparkle_Char[j],
                     kAncilla_VictorySparkle_Flags[j] | 4 | HIBYTE(oam_priority_value), 0);
      oam++;
    }
  }
}

void Ancilla3C_SpinAttackChargeSparkle(int k) {  // 88c1ea
  static const uint8 kSwordChargeSpark_Char[3] = {0xb7, 0x80, 0x83};
  static const uint8 kSwordChargeSpark_Flags[3] = {4, 4, 0x84};

  if (!submodule_index && !ancilla_timer[k]) {
    ancilla_timer[k] = 4;
    if (++ancilla_item_to_link[k] == 3) {
      ancilla_type[k] = 0;
      return;
    }
  }
  ancilla_oam_idx[k] = Ancilla_AllocateOamFromRegion_A_or_D_or_F(k, 4);
  Point16U info;
  Ancilla_PrepOamCoord(k, &info);
  int j = ancilla_item_to_link[k];
  Ancilla_SetOam(GetOamCurPtr(), info.x, info.y,
                 kSwordChargeSpark_Char[j], kSwordChargeSpark_Flags[j] | HIBYTE(oam_priority_value), 0);
}

void Ancilla35_MasterSwordReceipt(int k) {  // 88c25f
  static const int8 kSwordCeremony_X[8] = {-1, 8, -1, 8, 0, 7, 0, 7};
  static const int8 kSwordCeremony_Y[8] = {1, 1, 9, 9, 1, 1, 9, 9};
  static const uint8 kSwordCeremony_Char[8] = {0x86, 0x86, 0x96, 0x96, 0x87, 0x87, 0x97, 0x97};
  static const uint8 kSwordCeremony_Flags[8] = {1, 0x41, 1, 0x41, 1, 0x41, 1, 0x41};

  if (!ancilla_timer[k]) {
    ancilla_type[k] = 0;
    return;
  }
  if (sign8(--ancilla_aux_timer[k])) {
    ancilla_item_to_link[k] = (ancilla_item_to_link[k] == 2) ? 0 : ancilla_item_to_link[k] + 1;
  }

  Point16U pt;
  Ancilla_PrepOamCoord(k, &pt);
  OamEnt *oam = GetOamCurPtr();
  int j = (ancilla_item_to_link[k] - 1) * 4;
  if (j < 0)
    return;

  for (int i = 0; i < 4; i++, j++, oam++) {
    Ancilla_SetOam(oam, pt.x + kSwordCeremony_X[j], pt.y + kSwordCeremony_Y[j],
                   kSwordCeremony_Char[j], kSwordCeremony_Flags[j] & ~0x30 | 4 | HIBYTE(oam_priority_value), 0);
  }
}

void Ancilla22_ItemReceipt(int k) {  // 88c38a
  uint8 a;

  if (flag_is_link_immobilized == 2)
    goto endif_1;
  if (submodule_index != 0 && submodule_index != 43 && submodule_index != 9) {
    if (submodule_index == 2)
      ancilla_timer[k] = 16;
    goto endif_1;
  }
  flag_unk1++;

  if (ancilla_step[k] != 0 && ancilla_step[k] != 3) {
    if (sign8(--ancilla_aux_timer[k]))
      goto endif_11;

    if (ancilla_aux_timer[k] == 0)
      goto endif_6;

    if (ancilla_aux_timer[k] == 40 && ancilla_step[k] != 2) {
      if (Ancilla_AddRupees(k) || ancilla_item_to_link[k] != 0x17)
        Ancilla_Sfx3_Near(0xf);
    }
    goto label_b;
  }

  if (ancilla_item_to_link[k] == 1 && ancilla_step[k] != 2) {
    if (ancilla_timer[k] == 0)
      goto label_a;
    if (ancilla_timer[k] != 17)
      goto endif_1;
    word_7E02CD = 0xDF3;
    follower_indicator = 0xe;
    goto endif_6;
  }

  a = --ancilla_aux_timer[k];
  if (a == 0)
    goto label_a;
  if (a == 1) {
    if (ancilla_item_to_link[k] != 0x37 && ancilla_item_to_link[k] != 0x38 && ancilla_item_to_link[k] != 0x39 || zelda_read_apui00() == 0)
      goto endif_6;
    ancilla_aux_timer[k]++;
  }
  goto endif_1;

label_a:
  if (ancilla_item_to_link[k] == 1 && !ancilla_step[k]) {
    sound_effect_ambient = 5;
    music_control = 2;
  }
  link_player_handler_state = link_is_in_deep_water ? kPlayerState_Swimming : 0;
  link_receiveitem_index = 0;
  link_pose_for_item = 0;
  link_disable_sprite_damage = 0;
  Ancilla_AddRupees(k);
endif_11:
  item_receipt_method = 0;
  a = ancilla_item_to_link[k];
  if (a == 23 && link_heart_pieces == 0) {
    Link_ReceiveItem(0x26, 0);
    ancilla_type[k] = 0;
    flag_unk1 = 0;
    return;
  }

  if (a == 0x26 || a == 0x3f) {
    if (link_health_capacity != 0xa0) {
      link_health_capacity += 8;
      link_hearts_filler += link_health_capacity - link_health_current;
      Ancilla_Sfx3_Near(0xd);
    }
  } else if (a == 0x3e) {
    flag_is_link_immobilized = 0;
    if (link_health_capacity != 0xa0) {
      link_health_capacity += 8;
      link_hearts_filler += 8;
      Ancilla_Sfx3_Near(0xd);
    }
  } else if (a == 0x42) {
    link_hearts_filler += 8;
  } else if (a == 0x45) {
    link_magic_filler += 16;
  } else if (a == 0x22 || a == 0x23) {
    Palette_Load_LinkArmorAndGloves();
  }

  ancilla_type[k] = 0;
  flag_unk1 = 0;
  a = ancilla_item_to_link[k];
  if (ancilla_step[k] == 3 && a != 0x10 && a != 0x26 && a != 0xf && a != 0x20) {
    PrepareDungeonExitFromBossFight();
  }

  if (ancilla_step[k] != 2)
    flag_is_link_immobilized = 0;
  return;

endif_6:
  if (player_is_indoors) {
    int room = dungeon_room_index;
    if (room == 0xff || room == 0x10f || room == 0x110 || room == 0x112 || room == 0x11f)
      goto label_b;
  }
  int msg;
  msg = -1;
  if (ancilla_item_to_link[k] == 0x38 || ancilla_item_to_link[k] == 0x39) {
    if ((link_which_pendants & 7) == 7)
      msg = kReceiveItemMsgs2[ancilla_item_to_link[k] - 0x38];
    else
      msg = kReceiveItemMsgs[ancilla_item_to_link[k]];
  } else if (ancilla_step[k] != 2) {
    if (ancilla_item_to_link[k] == 0x17)
      msg = kReceiveItemMsgs3[link_heart_pieces];
    else
      msg = kReceiveItemMsgs[ancilla_item_to_link[k]];
  }
  if (msg != -1) {
    dialogue_message_index = msg;
    if (msg == 0x70)
      sound_effect_ambient = 9;
    Main_ShowTextMessage();
  }
  goto endif_1;

label_b:
  if (ancilla_aux_timer[k] >= 24) {
    a = ancilla_y_vel[k] - 1;
    if (a >= 248)
      ancilla_y_vel[k] = a;
    Ancilla_MoveY(k);
  }
endif_1:

  if (ancilla_item_to_link[k] == 0x20) {
    ancilla_z[k] = 0;
    AncillaAdd_OccasionalSparkle(k);
    if (zelda_read_apui00() == 0) {
      music_control = 0x1a;
      ItemReceipt_TransmuteToRisingCrystal(k);
      return;
    }
  } else if (ancilla_item_to_link[k] == 0x1) {
    ancilla_arr4[k] = kReceiveItem_Tab0[0];
    if (ancilla_step[k] != 2) {
      if (ancilla_timer[k] < 16) {
        a = 0;
      } else {
        if (!sign8(--ancilla_arr3[k]))
          goto skipit;
        ancilla_arr3[k] = 2;
        a = ancilla_arr1[k] + 1;
        if (a == 3)
          a = 0;
      }
      ancilla_arr1[k] = a;
      ancilla_arr4[k] = kReceiveItem_Tab0[a];
skipit:;
    }
  }

  if ((ancilla_item_to_link[k] == 0x34 || ancilla_item_to_link[k] == 0x35 || ancilla_item_to_link[k] == 0x36) && sign8(--ancilla_arr3[k])) {
    a = ancilla_arr1[k] + 1;
    if (a == 3)
      a = 0;
    ancilla_arr1[k] = a;
    ancilla_arr3[k] = kReceiveItem_Tab4[a];
    WriteTo4BPPBuffer_at_7F4000(kReceiveItem_Tab5[a]);
  }
  Point16U pt;
  Ancilla_PrepAdjustedOamCoord(k, &pt);
  Ancilla_ReceiveItem_Draw(k, pt.x, pt.y);
}

OamEnt *Ancilla_ReceiveItem_Draw(int k, int x, int y) {  // 88c690
  OamEnt *oam = GetOamCurPtr();
  int j = ancilla_item_to_link[k];
  oam->charnum = 0x24;
  uint8 a = kWishPond2_OamFlags[j];
  if (sign8(a))
    a = ancilla_arr4[k];
  Ancilla_SetOam(oam, x, y, 0x24, a * 2 | 0x30, kReceiveItem_Tab1[j]);
  oam++;
  if (kReceiveItem_Tab1[j] == 0) {
    Ancilla_SetOam(oam, x, y + 8, 0x34, a * 2 | 0x30, 0);
    oam++;
  }
  return oam;
}

void Ancilla28_WishPondItem(int k) {  // 88c6f2
  Ancilla_AllocateOamFromRegion_A_or_D_or_F(k, 0x10);

  if (submodule_index == 0 && ancilla_timer[k] == 0) {
    link_picking_throw_state = 2;
    link_state_bits = 0;
    ancilla_z_vel[k] -= 2;
    Ancilla_MoveZ(k);
    Ancilla_MoveY(k);
    Ancilla_MoveX(k);
    if (sign8(ancilla_z[k]) && ancilla_z[k] < 228) {
      ancilla_z[k] = 228;
      Ancilla_SetXY(k,
          Ancilla_GetX(k) + (kGeneratedWishPondItem[ancilla_item_to_link[k]] ? 8 : 4), // wtf
          Ancilla_GetY(k) + 18);
      Ancilla_TransmuteToSplash(k);
      return;
    }
  }
  WishPondItem_Draw(k);
}

void WishPondItem_Draw(int k) {  // 88c760
  Point16U pt;
  Ancilla_PrepAdjustedOamCoord(k, &pt);

  if (ancilla_item_to_link[k] == 1)
    ancilla_arr4[k] = 5;

  OamEnt *oam = Ancilla_ReceiveItem_Draw(k, pt.x, pt.y - (int8)ancilla_z[k]);

  if (link_picking_throw_state != 2 || !sign8(ancilla_z_vel[k]) && ancilla_z_vel[k] >= 2)
    return;

  uint8 xx = kGeneratedWishPondItem[ancilla_item_to_link[k]];
  AncillaDraw_Shadow(oam,
    (xx == 2) ? 1 : 2,
    pt.x - (xx == 2 ? 0 : 4),
    pt.y + 40, HIBYTE(oam_priority_value));
}

void Ancilla42_HappinessPondRupees(int k) {  // 88c7de
  link_picking_throw_state = 2;
  link_state_bits = 0;
  for (int i = 9; i >= 0; i--) {
    if (happiness_pond_arr1[i]) {
      HapinessPondRupees_ExecuteRupee(k, i);
      if (happiness_pond_step[i] == 2)
        happiness_pond_arr1[i] = 0;
    }
  }
  for (int i = 9; i >= 0; i--) {
    if (happiness_pond_arr1[i])
      return;
  }
  ancilla_type[k] = 0;
}

void HapinessPondRupees_ExecuteRupee(int k, int i) {  // 88c819
  Ancilla_AllocateOamFromRegion_A_or_D_or_F(k, 0x10);
  HapinessPondRupees_GetState(k, i);

  if (ancilla_step[k]) {
    if (!submodule_index && !ancilla_timer[k]) {
      ancilla_timer[k] = 6;
      if (++ancilla_item_to_link[k] == 5) {
        ancilla_step[k]++;
      } else {
        ObjectSplash_Draw(k);
      }
    } else {
      ObjectSplash_Draw(k);
    }
  } else if (submodule_index == 0 && ancilla_timer[k] == 0) {
    ancilla_z_vel[k] -= 2;
    Ancilla_MoveY(k);
    Ancilla_MoveX(k);
    Ancilla_MoveZ(k);
    if (!sign8(ancilla_z[k]) || ancilla_z[k] >= 0xe4)
      goto else_label;
    ancilla_z[k] = 0xe4;
    Ancilla_SetXY(k, Ancilla_GetX(k) - 4, Ancilla_GetY(k) + 30);
    ancilla_item_to_link[k] = 0;
    ancilla_timer[k] = 6;
    Ancilla_Sfx2_Pan(k, 0x28);
    ancilla_step[k]++;
    ObjectSplash_Draw(k);
  } else {
else_label:
    ancilla_arr4[k] = 2;
    ancilla_floor[k] = 0;
    WishPondItem_Draw(k);
  }
  HapinessPondRupees_SaveState(i, k);
}

void HapinessPondRupees_GetState(int j, int k) {  // 88c8be
  ancilla_y_lo[j] = happiness_pond_y_lo[k];
  ancilla_y_hi[j] = happiness_pond_y_hi[k];
  ancilla_x_lo[j] = happiness_pond_x_lo[k];
  ancilla_x_hi[j] = happiness_pond_x_hi[k];
  ancilla_z[j] = happiness_pond_z[k];
  ancilla_y_vel[j] = happiness_pond_y_vel[k];
  ancilla_x_vel[j] = happiness_pond_x_vel[k];
  ancilla_z_vel[j] = happiness_pond_z_vel[k];
  ancilla_y_subpixel[j] = happiness_pond_y_subpixel[k];
  ancilla_x_subpixel[j] = happiness_pond_x_subpixel[k];
  ancilla_z_subpixel[j] = happiness_pond_z_subpixel[k];
  ancilla_item_to_link[j] = happiness_pond_item_to_link[k];
  ancilla_step[j] = happiness_pond_step[k];
  ancilla_timer[j] = happiness_pond_timer[k] ? happiness_pond_timer[k] - 1 : 0;
}

void HapinessPondRupees_SaveState(int k, int j) {  // 88c924
  happiness_pond_y_lo[k] = ancilla_y_lo[j];
  happiness_pond_y_hi[k] = ancilla_y_hi[j];
  happiness_pond_x_lo[k] = ancilla_x_lo[j];
  happiness_pond_x_hi[k] = ancilla_x_hi[j];
  happiness_pond_z[k] = ancilla_z[j];
  happiness_pond_y_vel[k] = ancilla_y_vel[j];
  happiness_pond_x_vel[k] = ancilla_x_vel[j];
  happiness_pond_z_vel[k] = ancilla_z_vel[j];
  happiness_pond_y_subpixel[k] = ancilla_y_subpixel[j];
  happiness_pond_x_subpixel[k] = ancilla_x_subpixel[j];
  happiness_pond_z_subpixel[k] = ancilla_z_subpixel[j];
  happiness_pond_item_to_link[k] = ancilla_item_to_link[j];
  happiness_pond_timer[k] = ancilla_timer[j];
  happiness_pond_step[k] = ancilla_step[j];
}

void Ancilla_TransmuteToSplash(int k) {  // 88c9cd
  ancilla_type[k] = 0x3d;
  ancilla_item_to_link[k] = 0;
  ancilla_timer[k] = 6;
  Ancilla_SetXY(k, Ancilla_GetX(k) - 8, Ancilla_GetY(k) + 12);
  Ancilla_Sfx2_Pan(k, 0x28);
  Ancilla3D_ItemSplash(k);
}

void Ancilla3D_ItemSplash(int k) {  // 88ca01
  Ancilla_AllocateOamFromRegion_A_or_D_or_F(k, 8);
  if (!submodule_index && !ancilla_timer[k]) {
    ancilla_timer[k] = 6;
    if (++ancilla_item_to_link[k] == 5) {
      ancilla_type[k] = 0;
      return;
    }
  }
  ObjectSplash_Draw(k);
}

void ObjectSplash_Draw(int k) {  // 88ca22
  static const int8 kObjectSplash_Draw_X[10] = {0, 0, 0, 0, 11, -3, 15, -7, 15, -7};
  static const int8 kObjectSplash_Draw_Y[10] = {0, 0, -6, 0, -13, -8, -17, -4, -17, -4};
  static const uint8 kObjectSplash_Draw_Char[10] = {0xc0, 0xff, 0xe7, 0xff, 0xaf, 0xbf, 0x80, 0x80, 0x83, 0x83};
  static const uint8 kObjectSplash_Draw_Flags[10] = {0, 0xff, 0, 0xff, 0x40, 0, 0x40, 0, 0xc0, 0x80};
  static const uint8 kObjectSplash_Draw_Ext[10] = {2, 0, 2, 0, 0, 0, 0, 0, 0, 0};
  Point16U pt;
  Ancilla_PrepOamCoord(k, &pt);
  OamEnt *oam = GetOamCurPtr();
  int j = ancilla_item_to_link[k] * 2;
  for (int i = 0; i != 2; i++, j++) {
    if (kObjectSplash_Draw_Char[j] != 0xff) {
      Ancilla_SetOam(oam, pt.x + kObjectSplash_Draw_X[j], pt.y + kObjectSplash_Draw_Y[j],
                     kObjectSplash_Draw_Char[j], kObjectSplash_Draw_Flags[j] | 0x24, 
                     kObjectSplash_Draw_Ext[j]);
      oam++;
    }
  }
}

void Ancilla29_MilestoneItemReceipt(int k) {  // 88ca8c
  if (ancilla_item_to_link[k] != 0x10 && ancilla_item_to_link[k] != 0x0f) {
    if (dung_savegame_state_bits & 0x4000) {
      ancilla_type[k] = 0;
      return;
    }

    if (!(dung_savegame_state_bits & 0x8000))
      return;

    if (byte_7E04C2 != 0) {
      if (byte_7E04C2 == 1) {
        if (ancilla_item_to_link[k] == 0x20) {
          sound_effect_ambient = 0x0f;
          DecodeAnimatedSpriteTile_variable(0x28);
        } else {
          DecodeAnimatedSpriteTile_variable(0x23);
        }
      }
      byte_7E04C2--;
      return;
    }
    if (!ancilla_arr3[k] && ancilla_item_to_link[k] == 0x20) {
      ancilla_arr3[k] = 1;
      palette_sp6r_indoors = 4;
      overworld_palette_aux_or_main = 0x200;
      Palette_Load_SpriteEnvironment_Dungeon();
      flag_update_cgram_in_nmi++;
    }
  } else {
    if (ancilla_G[k]) {
      ancilla_G[k]--;
      return;
    }
  }

  if (ancilla_item_to_link[k] == 0x20)
    AncillaAdd_OccasionalSparkle(k);

  if (submodule_index == 0) {
    CheckPlayerCollOut coll_out;
    if (ancilla_z[k] < 24 && Ancilla_CheckLinkCollision(k, 2, &coll_out) && related_to_hookshot == 0 && link_auxiliary_state == 0) {
      ancilla_type[k] = 0;
      if (link_player_handler_state == kPlayerState_ReceivingEther || link_player_handler_state == kPlayerState_ReceivingBombos) {
        flag_custom_spell_anim_active = 0;
        link_force_hold_sword_up = 0;
        link_player_handler_state = 0;
      }
      item_receipt_method = 3;
      Link_ReceiveItem(ancilla_item_to_link[k], 0);
      return;
    }

    if (ancilla_step[k] != 2) {
      if (ancilla_step[k] != 0) {
        ancilla_z_vel[k]--;
      }
      Ancilla_MoveZ(k);
      if (ancilla_z[k] >= 0xf8) {
        ancilla_step[k]++;
        ancilla_z_vel[k] = 0x18;
        ancilla_z[k] = 0;
      }
    }
  }

  Point16U pt;
  Ancilla_PrepAdjustedOamCoord(k, &pt);
  OamEnt *oam = Ancilla_ReceiveItem_Draw(k, pt.x, pt.y - ancilla_z[k]);

  if (sign8(--ancilla_aux_timer[k])) {
    ancilla_aux_timer[k] = 9;
    if (++ancilla_L[k] == 3)
      ancilla_L[k] = 0;
  }

  int t;
  if (ancilla_z[k] == 0) {
    t = (dungeon_room_index == 6) ? ancilla_L[k] + 4 : 0;
  } else {
    t = ancilla_z[k] < 0x20 ? 1 : 2;
  }
  AncillaDraw_Shadow(oam, t, pt.x, pt.y + 12, 0x20);
}

void ItemReceipt_TransmuteToRisingCrystal(int k) {  // 88cbe4
  ancilla_type[k] = 0x3e;
  ancilla_y_vel[k] = 0;
  ancilla_x_vel[k] = 0;
  ancilla_y_subpixel[k] = 0;
  Ancilla_RisingCrystal(k);
}

void Ancilla_RisingCrystal(int k) {  // 88cbf2
  ancilla_z[k] = 0;
  AncillaAdd_OccasionalSparkle(k);
  uint8 yy = ancilla_y_vel[k] - 1;
  if (yy < 0xf0)
    yy = 0xf0;
  ancilla_y_vel[k] = yy;
  Ancilla_MoveY(k);

  uint16 y = Ancilla_GetY(k) - BG2VOFS_copy;
  if (y < 0x49) {
    Ancilla_SetY(k, 0x49 + BG2VOFS_copy);
    if (!submodule_index) {
      link_has_crystals |= kDungeonCrystalPendantBit[BYTE(cur_palace_index_x2) >> 1];
      submodule_index = 0x18;
      subsubmodule_index = 0;
      memset(aux_palette_buffer + 0x20, 0, sizeof(uint16) * 0x60);
      palette_filter_countdown = 0;
      darkening_or_lightening_screen = 0;
    }
  }

  Point16U pt;
  Ancilla_PrepAdjustedOamCoord(k, &pt);
  Ancilla_ReceiveItem_Draw(k, pt.x, pt.y);
}

void AncillaAdd_OccasionalSparkle(int k) {  // 88cc93
  if (!(frame_counter & 7))
    AncillaAdd_SwordChargeSparkle(k);
}

void Ancilla43_GanonsTowerCutscene(int k) {  // 88cca0
  OamEnt *oam = GetOamCurPtr();
  if (!ancilla_step[k]) {
    uint8 yy = ancilla_y_vel[k] - 1;
    ancilla_y_vel[k] = (yy < 0xf0) ? 0xf0 : yy;
    Ancilla_MoveY(k);
    uint16 x = Ancilla_GetX(k), y = Ancilla_GetY(k);
    if ((uint16)(y - BG2VOFS_copy) >= 0x38)
      goto lbl_else;
    breaktowerseal_y = 0x38 + 8 + BG2VOFS_copy;
    breaktowerseal_x = x + 8;
    Ancilla_SetY(k, 0x38 + BG2VOFS_copy);
    ancilla_step[k]++;
    sound_effect_ambient = 5;
    music_control = 0xf1;
    dialogue_message_index = 0x13b;
    Main_ShowTextMessage();
    goto label_a;
  }
lbl_else:
  if (ancilla_step[k] == 1 && submodule_index == 0) {
    ancilla_x_vel[k] = 16;
    uint8 bak0 = ancilla_x_lo[k];
    uint8 bak1 = ancilla_x_hi[k];
    ancilla_x_lo[k] = breaktowerseal_var4;
    ancilla_x_hi[k] = 0;
    Ancilla_MoveX(k);
    breaktowerseal_var4 = ancilla_x_lo[k];
    ancilla_x_lo[k] = bak0;
    ancilla_x_hi[k] = bak1;
    if (breaktowerseal_var4 >= 48) {
      breaktowerseal_var4 = 48;
      ancilla_step[k]++;
    }
  }
  if (submodule_index)
    goto label_b;
  if (ancilla_step[k] == 0)
    goto label_a;
  if (ancilla_step[k] == 1)
    goto label_b;
  if (ancilla_step[k] == 2) {
    if (--breaktowerseal_var5 == 0) {
      trigger_special_entrance = 5;
      subsubmodule_index = 0;
      BYTE(R16) = 0;
      ancilla_step[k]++;
    }
  } else {
    ancilla_x_vel[k] = 48;
    uint8 bak0 = ancilla_x_lo[k];
    uint8 bak1 = ancilla_x_hi[k];
    ancilla_x_lo[k] = breaktowerseal_var4;
    ancilla_x_hi[k] = 0;
    Ancilla_MoveX(k);
    breaktowerseal_var4 = ancilla_x_lo[k];
    ancilla_x_lo[k] = bak0;
    ancilla_x_hi[k] = bak1;
    if (breaktowerseal_var4 >= 240) {
      palette_sp6r_indoors = 0;
      overworld_palette_aux_or_main = 0x200;
      Palette_Load_SpriteEnvironment_Dungeon();
      flag_update_cgram_in_nmi++;
      ancilla_type[k] = 0;
      return;
    }
  }
  uint8 astep;
label_b:


  astep = ancilla_step[k];
  if (astep != 0)
    oam = GTCutscene_SparkleALot(oam);

  for (int j = 6; j >= 0; j--) {
    if (submodule_index == 0 && astep != 1 && !(frame_counter & 1))
      breaktowerseal_var3[j] = breaktowerseal_var3[j] + 1 & 63;
    AncillaRadialProjection arp = Ancilla_GetRadialProjection(breaktowerseal_var3[j], breaktowerseal_var4);
    int x = (arp.r6 ? -arp.r4 : arp.r4) + breaktowerseal_x - 8 - BG2HOFS_copy;
    int y = (arp.r2 ? -arp.r0 : arp.r0) + breaktowerseal_y - 8 - BG2VOFS_copy;

    breaktowerseal_base_sparkle_x_lo[j] = x;
    breaktowerseal_base_sparkle_x_hi[j] = x >> 8;

    breaktowerseal_base_sparkle_y_lo[j] = y;
    breaktowerseal_base_sparkle_y_hi[j] = y >> 8;

    AncillaDraw_GTCutsceneCrystal(oam, x, y);
    oam++;
  }
  Point16U info;
label_a:
  Ancilla_PrepAdjustedOamCoord(k, &info);

  breaktowerseal_base_sparkle_x_lo[7] = info.x;
  breaktowerseal_base_sparkle_x_hi[7] = info.x >> 8;
  breaktowerseal_base_sparkle_y_lo[7] = info.y;
  breaktowerseal_base_sparkle_y_hi[7] = info.y >> 8;

  AncillaDraw_GTCutsceneCrystal(oam, info.x, info.y);

  if (!ancilla_step[k])
    AncillaAdd_OccasionalSparkle(k);
  else if (!submodule_index)
    GTCutscene_ActivateSparkle();
}

void AncillaDraw_GTCutsceneCrystal(OamEnt *oam, int x, int y) {  // 88ceaa
  Ancilla_SetOam_Safe(oam, x, y, 0x24, 0x3c, 2);
}

void GTCutscene_ActivateSparkle() {  // 88cec7
  for (int k = 0x17; k >= 0; k--) {
    if (breaktowerseal_sparkle_var1[k] == 0xff) {
      breaktowerseal_sparkle_var1[k] = 0;
      breaktowerseal_sparkle_var2[k] = 4;
      int r = GetRandomNumber();
      int x = breaktowerseal_base_sparkle_x_hi[k & 7] << 8 | breaktowerseal_base_sparkle_x_lo[k & 7];
      int y = breaktowerseal_base_sparkle_y_hi[k & 7] << 8 | breaktowerseal_base_sparkle_y_lo[k & 7];
      x += r >> 4;
      y += r & 0xf;
      breaktowerseal_sparkle_x_lo[k] = x;
      breaktowerseal_sparkle_x_hi[k] = x >> 8;
      breaktowerseal_sparkle_y_lo[k] = y;
      breaktowerseal_sparkle_y_hi[k] = y >> 8;
      return;
    }
  }
}

OamEnt *GTCutscene_SparkleALot(OamEnt *oam) {  // 88cf35
  static const uint8 kSwordChargeSpark_Char[3] = {0xb7, 0x80, 0x83};
  static const uint8 kSwordChargeSpark_Flags[3] = {4, 4, 0x84};
  for (int k = 0x17; k >= 0; k--) {
    if (breaktowerseal_sparkle_var1[k] == 0xff)
      continue;

    if (sign8(--breaktowerseal_sparkle_var2[k])) {
      breaktowerseal_sparkle_var2[k] = 4;
      if (++breaktowerseal_sparkle_var1[k] == 3) {
        breaktowerseal_sparkle_var1[k] = 0xff;
        continue;
      }
    }

    int x = breaktowerseal_sparkle_x_hi[k] << 8 | breaktowerseal_sparkle_x_lo[k];
    int y = breaktowerseal_sparkle_y_hi[k] << 8 | breaktowerseal_sparkle_y_lo[k];
    int j = breaktowerseal_sparkle_var1[k];
    Ancilla_SetOam(oam, x, y, kSwordChargeSpark_Char[j], kSwordChargeSpark_Flags[j] | 0x30, 0);
    oam++;
  }
  return oam;
}

void Ancilla36_Flute(int k) {  // 88cfaa
  static const uint8 kFlute_Vels[4] = {0x18, 0x10, 0xa, 0};

  if (!submodule_index) {
    if (ancilla_step[k] != 3) {
      ancilla_z_vel[k] -= 2;
      Ancilla_MoveX(k);
      Ancilla_MoveZ(k);
      if (sign8(ancilla_z[k]) || ancilla_z[k] >= 0xf0) {
        ancilla_z_vel[k] = kFlute_Vels[++ancilla_step[k]];
        ancilla_z[k] = 0;
      }
    } else {
      CheckPlayerCollOut coll_out;
      if (Ancilla_CheckLinkCollision(k, 2, &coll_out) && !related_to_hookshot && link_auxiliary_state == 0) {
        ancilla_type[k] = 0;
        item_receipt_method = 0;
        Link_ReceiveItem(0x14, 0);
        return;
      }
    }
  }

  Point16U pt;
  Ancilla_PrepAdjustedOamCoord(k, &pt);
  OamEnt *oam = GetOamCurPtr();
  Ancilla_SetOam(oam, pt.x, pt.y - (int8)ancilla_z[k], 0x24, HIBYTE(oam_priority_value) | 4, 2);
  if (oam->y == 0xf0)
    ancilla_type[k] = 0;
}

void Ancilla37_WeathervaneExplosion(int k) {  // 88d03d
  if (--weathervane_var2)
    return;
  weathervane_var2 = 1;
  if (!weathervane_var1) {
    weathervane_var1 = 1;
    music_control = 0xf3;
  }
  if (--ancilla_G[k])
    return;
  ancilla_G[k] = 1;
  if (!ancilla_arr3[k]) {
    ancilla_arr3[k] += 1;
    Ancilla_Sfx2_Near(0xc);
  }
  if (!ancilla_step[k] && sign8(--ancilla_aux_timer[k])) {
    ancilla_step[k] = 1;
    Overworld_AlterWeathervane();
    AncillaAdd_CutsceneDuck(0x38, 0);
  }
  weathervane_var13 = k;
  weathervane_var14 = 0;
  for (int i = 11; i >= 0; i--) {
    if (weathervane_arr12[i] == 0xff)
      continue;
    if (sign8(--weathervane_arr11[i])) {
      weathervane_arr11[i] = 1;
      weathervane_arr12[i] ^= 1;
    }

    ancilla_item_to_link[k] = weathervane_arr12[i];
    ancilla_y_lo[k] = weathervane_arr6[i];
    ancilla_y_hi[k] = weathervane_arr7[i];
    ancilla_x_lo[k] = weathervane_arr8[i];
    ancilla_x_hi[k] = weathervane_arr9[i];
    ancilla_z[k] = weathervane_arr10[i];
    ancilla_y_vel[k] = weathervane_arr3[i];
    ancilla_x_vel[k] = weathervane_arr4[i];
    weathervane_arr5[i] = ancilla_z_vel[k] = weathervane_arr5[i] - 1;

    Ancilla_MoveY(k);
    Ancilla_MoveX(k);
    Ancilla_MoveZ(k);

    uint8 c = (ancilla_z[k] < 0xf0) ? 0 : 0xff;
    AncillaDraw_WeathervaneExplosionWoodDebris(k);
    if (sign8(c))
      weathervane_arr12[i] = c;
    weathervane_arr6[i] = ancilla_y_lo[k];
    weathervane_arr7[i] = ancilla_y_hi[k];
    weathervane_arr8[i] = ancilla_x_lo[k];
    weathervane_arr9[i] = ancilla_x_hi[k];
    weathervane_arr10[i] = ancilla_z[k];
  }
  for (int i = 11; i >= 0; i--) {
    if (weathervane_arr12[i] != 0xff)
      return;
  }
  ancilla_type[k] = 0;
}

void AncillaDraw_WeathervaneExplosionWoodDebris(int k) {  // 88d188
  static const uint8 kWeathervane_Explode_Char[2] = {0x4e, 0x4f};
  Point16U pt;
  Ancilla_PrepOamCoord(k, &pt);
  pt.y -= (int8)ancilla_z[k];
  int i = ancilla_item_to_link[k];
  if (sign8(i))
    return;
  Ancilla_SetOam(GetOamCurPtr() + (weathervane_var14 >> 2), pt.x, pt.y, kWeathervane_Explode_Char[i], 0x3c, 0);
  weathervane_var14 += 4;
}

void Ancilla38_CutsceneDuck(int k) {  // 88d1d8
  static const uint8 kTravelBirdIntro_Tab0[2] = {0x40, 0};
  static const uint8 kTravelBirdIntro_Tab1[2] = {28, 60};

  if (!(frame_counter & 31))
    Ancilla_Sfx3_Pan(k, 0x1e);

  if (sign8(--ancilla_arr3[k])) {
    ancilla_arr3[k] = 3;
    ancilla_K[k] ^= 1;
  }

  if (!--ancilla_aux_timer[k]) {
    ancilla_aux_timer[k] = 1;
    if (!ancilla_L[k]) {
      if (!sign8(--ancilla_item_to_link[k])) {
        ancilla_z_vel[k] += ancilla_step[k] ? 1 : -1;
        if (abs8(ancilla_z_vel[k]) >= 12)
          ancilla_step[k] ^= 1;
        goto after_stuff;
      }
      ancilla_item_to_link[k] = 0;
      ancilla_step[k] = 0;
      ancilla_x_vel[k] = kTravelBirdIntro_Tab1[0];
      ancilla_z_vel[k] = -16;
      ancilla_L[k]++;
      ancilla_step[k] = 3;
    }
    ancilla_x_vel[k] += (ancilla_step[k] & 1) == 0 ? 1 : -1;
    uint8 absx = abs8(ancilla_x_vel[k]);
    if (absx == 0 && ++ancilla_L[k] == 7)
      ancilla_S[k] = 1;
    if (absx >= kTravelBirdIntro_Tab1[ancilla_S[k]]) {
      ancilla_step[k] ^= 3;
    }
    ancilla_dir[k] = sign8(ancilla_x_vel[k]) ? 2 : 3;
    uint8 t = (uint8)(kTravelBirdIntro_Tab1[ancilla_S[k]] - absx) >> 1;
    ancilla_z_vel[k] = (ancilla_step[k] & 2) ? -t : t;
  }
after_stuff:
  Ancilla_MoveX(k);
  Ancilla_MoveZ(k);
  BYTE(flag_travel_bird) = kTravelBird_DmaStuffs[ancilla_K[k] + 1];
  Point16U info;
  Ancilla_PrepOamCoord(k, &info);
  OamEnt *oam = GetOamCurPtr();
  Ancilla_SetOam(oam, info.x + kTravelBird_Draw_X[0], info.y + (int8)ancilla_z[k] + kTravelBird_Draw_Y[0],
                 kTravelBird_Draw_Char[0],
                 kTravelBird_Draw_Flags[0] | 0x30 | kTravelBirdIntro_Tab0[ancilla_dir[k] & 1], 2);
  oam++;
  AncillaDraw_Shadow(oam, 1, info.x, info.y + 48, 0x30);
  if (!sign16(info.x) && info.x >= 248) {
    ancilla_type[k] = 0;
    submodule_index = 0;
    link_item_flute = 3;
  }
}

void Ancilla23_LinkPoof(int k) {  // 88d3bc
  if (sign8(--ancilla_aux_timer[k])) {
    ancilla_aux_timer[k] = 7;
    if (++ancilla_item_to_link[k] == 3) {
      ancilla_type[k] = 0;
      link_is_transforming = 0;
      link_cant_change_direction = 0;
      if (!ancilla_step[k]) {
        link_animation_steps = 0;
        link_visibility_status = 0;
        link_is_bunny = link_is_bunny_mirror = BYTE(overworld_screen_index) & 0x40 ? 1 : 0;
        if (link_is_bunny)
          LoadGearPalettes_bunny();
        else
          LoadActualGearPalettes();
      }
      return;
    }
  }
  MorphPoof_Draw(k);
}

void MorphPoof_Draw(int k) {  // 88d3fd
  static const int8 kMorphPoof_X[12] = {0, 0, 0, 0, 0, 8, 0, 8, -4, 12, -4, 12};
  static const int8 kMorphPoof_Y[12] = {0, 0, 0, 0, 0, 0, 8, 8, -4, -4, 12, 12};
  static const uint8 kMorphPoof_Flags[12] = {0, 0xff, 0xff, 0xff, 0x40, 0, 0xc0, 0x80, 0, 0x40, 0x80, 0xc0};
  static const uint8 kMorphPoof_Char[3] = {0x86, 0xa9, 0x9b};
  static const uint8 kMorphPoof_Ext[3] = {2, 0, 0};
  if (sort_sprites_setting && ancilla_floor[k] && (!flag_for_boomerang_in_place || !(frame_counter & 1))) {
    oam_cur_ptr = 0x8d0;
    oam_ext_cur_ptr = 0xa20 + (0xd0 >> 2);
  }
  Point16U info;
  Ancilla_PrepOamCoord(k, &info);
  OamEnt *oam = GetOamCurPtr();
  int j = ancilla_item_to_link[k];
  uint8 ext = kMorphPoof_Ext[j];
  uint8 chr = kMorphPoof_Char[j];
  for (int i = 0; i < 4; i++, oam++) {
    Ancilla_SetOam(oam, info.x + kMorphPoof_X[j * 4 + i], info.y + kMorphPoof_Y[j * 4 + i], chr,
                   kMorphPoof_Flags[j * 4 + i] | 4 | HIBYTE(oam_priority_value), ext);
    if (ext == 2)
      break;
  }
}

void Ancilla40_DwarfPoof(int k) {  // 88d49a
  if (sign8(--ancilla_aux_timer[k])) {
    ancilla_aux_timer[k] = 7;
    if (++ancilla_item_to_link[k] == 3) {
      ancilla_type[k] = 0;
      tagalong_var5 = 0;
      return;
    }
  }
  MorphPoof_Draw(k);
}

void Ancilla3F_BushPoof(int k) {  // 88d519
  static const int8 kBushPoof_Draw_X[16] = {0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, -2, 10, -2, 10};
  static const int8 kBushPoof_Draw_Y[16] = {0, 0, 8, 8, 0, 0, 8, 8, 0, 0, 8, 8, -2, -2, 10, 10};
  static const uint8 kBushPoof_Draw_Char[16] = {0x86, 0x87, 0x96, 0x97, 0xa9, 0xa9, 0xa9, 0xa9, 0x8a, 0x8b, 0x9a, 0x9b, 0x9b, 0x9b, 0x9b, 0x9b};
  static const uint8 kBushPoof_Draw_Flags[16] = {0, 0, 0, 0, 0, 0x40, 0x80, 0xc0, 0, 0, 0, 0, 0xc0, 0x80, 0x40, 0};

  if (ancilla_timer[k] == 0) {
    ancilla_timer[k] = 7;
    if (++ancilla_item_to_link[k] == 4) {
      ancilla_type[k] = 0;
      return;
    }
  }
  Oam_AllocateFromRegionC(0x10);
  Point16U pt;
  Ancilla_PrepOamCoord(k, &pt);
  OamEnt *oam = GetOamCurPtr();

  int j = ancilla_item_to_link[k] * 4;
  for (int i = 0; i < 4; i++, j++, oam++) {
    Ancilla_SetOam(oam, pt.x + kBushPoof_Draw_X[j], pt.y + kBushPoof_Draw_Y[j],
                   kBushPoof_Draw_Char[j], kBushPoof_Draw_Flags[j] | 4 | HIBYTE(oam_priority_value), 0);
  }
}

void Ancilla26_SwordSwingSparkle(int k) {  // 88d65a
  static const int8 kSwordSwingSparkle_X[48] = {
    5,  10, -1,  5, 10, -4,  5, 10,  -4,  -4, -1,  -1,   0,   5,  -1,   0,
    5,  14,  0,  5, 14, 14, -1, -1, -23, -27, -1, -23, -27, -22, -23, -27,
    -22, -22, -1, -1, 32, 35, -1, 32,  35,  30, 32,  35,  30,  30,  -1,  -1,
  };
  static const int8 kSwordSwingSparkle_Y[48] = {
    -22, -18, -1, -22, -18, -17, -22, -18, -17, -17, -1, -1, 35, 40, -1, 35,
    40,  37, 35,  40,  37,  37,  -1,  -1,   2,   7, -1,  2,  7, 19,  2,  7,
    19,  19, -1,  -1,   2,   7,  -1,   2,   7,  19,  2,  7, 19, 19, -1, -1,
  };
  static const uint8 kSwordSwingSparkle_Char[48] = {
    0xb7, 0xb7, 0xff, 0x80, 0x80, 0xb7, 0x83, 0x83, 0x80, 0x83, 0xff, 0xff, 0xb7, 0xb7, 0xff, 0x80,
    0x80, 0xb7, 0x83, 0x83, 0x80, 0x83, 0xff, 0xff, 0xb7, 0xb7, 0xff, 0x80, 0x80, 0xb7, 0x83, 0x83,
    0x80, 0x83, 0xff, 0xff, 0xb7, 0xb7, 0xff, 0x80, 0x80, 0xb7, 0x83, 0x83, 0x80, 0x83, 0xff, 0xff,
  };
  static const uint8 kSwordSwingSparkle_Flags[48] = {
    0,    0, 0xff,    0, 0,    0, 0x80, 0x80, 0, 0x80, 0xff, 0xff, 0,    0, 0xff,    0,
    0,    0, 0x80, 0x80, 0, 0x80, 0xff, 0xff, 0,    0, 0xff,    0, 0,    0, 0x80, 0x80,
    0, 0x80, 0xff, 0xff, 0,    0, 0xff,    0, 0,    0, 0x80, 0x80, 0, 0x80, 0xff, 0xff,
  };
  if (sign8(--ancilla_aux_timer[k])) {
    ancilla_aux_timer[k] = 0;
    if (++ancilla_item_to_link[k] == 4) {
      ancilla_type[k] = 0;
      return;
    }
  }
  Ancilla_SetXY(k, link_x_coord, link_y_coord);

  Point16U info;
  Ancilla_PrepOamCoord(k, &info);

  k = ancilla_item_to_link[k] * 3 + ancilla_dir[k] * 12;

  OamEnt *oam = GetOamCurPtr();
  for (int n = 2; n >= 0; n--, k++, oam++) {
    uint8 chr = kSwordSwingSparkle_Char[k];
    if (chr == 0xff)
      continue;
    Ancilla_SetOam(oam, info.x + kSwordSwingSparkle_X[k], info.y + kSwordSwingSparkle_Y[k],
                   chr, kSwordSwingSparkle_Flags[k] | 0x4 | (oam_priority_value >> 8), 0);
  }
}

void Ancilla2A_SpinAttackSparkleA(int k) {  // 88d7b2
  static const uint8 kInitialSpinSpark_Timer[6] = {4, 2, 3, 3, 2, 1};
  if (!submodule_index && sign8(--ancilla_aux_timer[k])) {
    ancilla_aux_timer[k] = 0;
    if (!ancilla_timer[k]) {
      int j = ++ancilla_item_to_link[k];
      ancilla_timer[k] = kInitialSpinSpark_Timer[j];
      if (j == 5) {
        if (ancilla_step[k])
          AddSwordBeam(j);
        else
          SpinAttackSparkleA_TransmuteToNextSpark(k);
        return;
      }
    }
  }
  if (!ancilla_item_to_link[k])
    return;
  SpinSpark_Draw(k, -1);
}

void SpinAttackSparkleA_TransmuteToNextSpark(int k) {  // 88d86d
  static const uint8 kTransmuteSpinSpark_Arr[16] = {0x21, 0x20, 0x1f, 0x1e, 3, 2, 1, 0, 0x12, 0x11, 0x10, 0xf, 0x31, 0x30, 0x2f, 0x2e};
  static const int8 kTransmuteSpinSpark_X[4] = {-3, 21, 25, -8};
  static const int8 kTransmuteSpinSpark_Y[4] = {28, -2, 24, 6};

  ancilla_type[k] = 0x2b;
  int j = link_direction_facing * 2;
  swordbeam_arr[0] = kTransmuteSpinSpark_Arr[j + 0];
  swordbeam_arr[1] = kTransmuteSpinSpark_Arr[j + 1];
  swordbeam_arr[2] = kTransmuteSpinSpark_Arr[j + 2];
  swordbeam_arr[3] = swordbeam_var1 = kTransmuteSpinSpark_Arr[j + 3];
  ancilla_aux_timer[k] = 2;
  ancilla_item_to_link[k] = 0x4c;
  ancilla_arr3[k] = 8;
  ancilla_step[k] = 0;
  ancilla_L[k] = 0;
  ancilla_arr1[k] = 255;
  swordbeam_var2 = 20;

  swordbeam_temp_x = link_x_coord + 8;
  swordbeam_temp_y = link_y_coord + 12;

  j = link_direction_facing>>1;
  Ancilla_SetXY(k,
      link_x_coord + kTransmuteSpinSpark_X[j],
      link_y_coord + kTransmuteSpinSpark_Y[j]);
  Ancilla2B_SpinAttackSparkleB(k);
}

void Ancilla2B_SpinAttackSparkleB(int k) {  // 88d8fd
  static const uint8 kSpinSpark_Char[4] = {0xd7, 0xb7, 0x80, 0x83};

  if (ancilla_L[k]) {
    SpinAttackSparkleB_Closer(k);
    return;
  }
  uint8 flags = 2;
  if (!submodule_index) {
    uint8 t = (ancilla_item_to_link[k] -= 3);
    if (t < 13) {
      ancilla_aux_timer[k] = 1;
      ancilla_L[k] = 1;
      ancilla_item_to_link[k] = 0;
      SpinAttackSparkleB_Closer(k);
      return;
    }
    ancilla_step[k] = (t < 0x42) ? 3 : (t == 0x46) ? 1 : (t == 0x43) ? 2 : 0;
    if (sign8(--ancilla_aux_timer[k])) {
      flags = 4;
      ancilla_aux_timer[k] = 2;
    }
  }
  OamEnt *oam = GetOamCurPtr(), *oam_org = oam;
  int i = ancilla_step[k];
  do {
    if (submodule_index == 0)
      swordbeam_arr[i] = (swordbeam_arr[i] + 4) & 0x3f;
    Point16U pt = Sparkle_PrepOamFromRadial(Ancilla_GetRadialProjection(swordbeam_arr[i], swordbeam_var2));
    Ancilla_SetOam(oam, pt.x, pt.y, kSpinSpark_Char[i], flags | HIBYTE(oam_priority_value), 0);
  } while (oam++, --i >= 0);

  if (submodule_index == 0) {
    if (!sign8(--ancilla_arr3[k]))
      goto endif_2;

    ancilla_arr3[k] = 0;
    ancilla_arr1[k] = (ancilla_arr1[k] + 1) & 3;
    if (ancilla_arr1[k] == 3)
      swordbeam_var1 = (swordbeam_var1 + 9) & 0x3f;
  }

  uint8 t;

  t = ancilla_arr1[k];
  if (t != 3) {
    static const uint8 kSpinSpark_Char2[3] = {0xb7, 0x80, 0x83};
    Point16U pt = Sparkle_PrepOamFromRadial(Ancilla_GetRadialProjection(swordbeam_var1, swordbeam_var2));
    Ancilla_SetOam(oam, pt.x, pt.y, kSpinSpark_Char2[t], 4 | HIBYTE(oam_priority_value), 0);
  }
endif_2:
  if (ancilla_item_to_link[k] == 7)
    bytewise_extended_oam[oam_org - oam_buf + 3] = 1;
}

Point16U Sparkle_PrepOamFromRadial(AncillaRadialProjection p) {  // 88da17
  Point16U pt;
  pt.y = (p.r2 ? -p.r0 : p.r0) + swordbeam_temp_y - 4 - BG2VOFS_copy2;
  pt.x = (p.r6 ? -p.r4 : p.r4) + swordbeam_temp_x - 4 - BG2HOFS_copy2;
  return pt;
}

void SpinAttackSparkleB_Closer(int k) {  // 88da4c
  if (sign8(--ancilla_aux_timer[k])) {
    ancilla_aux_timer[k] = 1;
    if (++ancilla_item_to_link[k] == 3)
      ancilla_type[k] = 0;
  }
  SpinSpark_Draw(k, 4);
}

void Ancilla30_ByrnaWindupSpark(int k) {  // 88db24
  static const int8 kInitialCaneSpark_X[16] = {3, 1, 0, 0, 13, 16, 12, 12, 24, 7, -4, -10, -8, 9, 22, 26};
  static const int8 kInitialCaneSpark_Y[16] = {5, 0, -3, -6, -8, -3, 12, 28, 5, 0, 8, 16, 5, 0, 8, 16};
  static const int8 kInitialCaneSpark_Draw_X[16] = {-4, 0, 0, 0, -8, 0, -8, 0, -8, 0, -8, 0, -8, 0, -8, 0};
  static const int8 kInitialCaneSpark_Draw_Y[16] = {-4, 0, 0, 0, -8, -8, 0, 0, -8, -8, 0, 0, -8, -8, 0, 0};
  static const uint8 kInitialCaneSpark_Draw_Char[16] = {0x92, 0xff, 0xff, 0xff, 0x8c, 0x8c, 0x8c, 0x8c, 0xd6, 0xd6, 0xd6, 0xd6, 0x93, 0x93, 0x93, 0x93};
  static const uint8 kInitialCaneSpark_Draw_Flags[16] = {0x22, 0xff, 0xff, 0xff, 0x22, 0x62, 0xa2, 0xe2, 0x24, 0x64, 0xa4, 0xe4, 0x22, 0x62, 0xa2, 0xe2};

  if (!submodule_index && sign8(--ancilla_aux_timer[k])) {
    ancilla_aux_timer[k] = 1;
    if (++ancilla_item_to_link[k] == 17) {
      ByrnaWindupSpark_TransmuteToNormal(k);
      return;
    }
  }
  if (!ancilla_item_to_link[k])
    return;

  int j = player_handler_timer;
  if (j == 2) {
    uint8 a = ancilla_arr3[k] - 1;
    if (sign8(a))
      a = 0, j = 3;
    ancilla_arr3[k] = a;
  }
  j += link_direction_facing * 2;
  Ancilla_SetXY(k, link_x_coord + kInitialCaneSpark_X[j], link_y_coord + kInitialCaneSpark_Y[j]);
  Point16U pt;
  Ancilla_PrepOamCoord(k, &pt);

  uint8 a = (ancilla_item_to_link[k] - 1) & 0xf;
  j = 0;
  if (a != 0)
    j = 4 * ((a != 15) ? (a & 1) + 1 : 3);

  OamEnt *oam = GetOamCurPtr();
  for (int i = 0; i < 4; i++, j++) {
    if (kInitialCaneSpark_Draw_Char[j] != 255) {
      Ancilla_SetOam(oam, pt.x + kInitialCaneSpark_Draw_X[j], pt.y + kInitialCaneSpark_Draw_Y[j],
                     kInitialCaneSpark_Draw_Char[j],
                     kInitialCaneSpark_Draw_Flags[j] & ~0x30 | HIBYTE(oam_priority_value), 0);
      oam++;
    }
  }
}

void ByrnaWindupSpark_TransmuteToNormal(int k) {  // 88dc21
  static const uint8 kCaneSpark_Transmute_Tab[16] = {0x34, 0x33, 0x32, 0x31, 0x16, 0x15, 0x14, 0x13, 0x2a, 0x29, 0x28, 0x27, 0x10, 0xf, 0xe, 0xd};

  ancilla_type[k] = 0x31;
  int j = link_direction_facing << 1;
  swordbeam_arr[0] = kCaneSpark_Transmute_Tab[j + 0];
  swordbeam_arr[1] = kCaneSpark_Transmute_Tab[j + 1];
  swordbeam_arr[2] = kCaneSpark_Transmute_Tab[j + 2];
  swordbeam_arr[3] = kCaneSpark_Transmute_Tab[j + 3];
  ancilla_aux_timer[k] = 0x17;
  ancilla_G[k] = 0;
  ancilla_item_to_link[k] = 0;
  ancilla_arr3[k] = 8;
  ancilla_step[k] = 0;
  ancilla_L[k] = 0;
  ancilla_arr1[k] = 2;
  ancilla_timer[k] = 21;
  swordbeam_var2 = 20;
  Ancilla_Sfx3_Near(0x30);
  Ancilla31_ByrnaSpark(k);
}

void Ancilla31_ByrnaSpark(int k) {  // 88dc70
  static const uint8 kCaneSpark_Magic[3] = {4, 2, 1};

  uint8 flags = 2;
  if (submodule_index == 0) {
    if (current_item_y != 13) {
kill_me:
      link_disable_sprite_damage = 0;
      ancilla_type[k] = 0;
      link_give_damage = 0;
      return;
    }
    link_disable_sprite_damage = 1;
    if (!--ancilla_aux_timer[k]) {
      ancilla_aux_timer[k] = 1;
      uint8 r0 = kCaneSpark_Magic[link_magic_consumption];
      if (!link_magic_power || (uint8)(r0 = link_magic_power - r0) >= 0x80)
        goto kill_me;

      if (sign8(--ancilla_G[k])) {
        ancilla_G[k] = 0x17;
        link_magic_power = r0;
      }
      if (filtered_joypad_H & 0x40)
        goto kill_me;
    }
    if (ancilla_step[k] != 3) {
      uint8 a = ++ancilla_item_to_link[k];
      ancilla_step[k] = (a >= 4) ? 3 : (a == 2) ? 1 : (a == 3) ? 2 : 0;
    }
    if (sign8(--ancilla_arr1[k])) {
      ancilla_arr1[k] = 2;
      flags = 4;
    }
  }

  int z = (int8)link_z_coord;
  if (z == -1)
    z = 0;
  swordbeam_temp_y = link_y_coord + 12 - z;
  swordbeam_temp_x = link_x_coord + 8;
  if (!ancilla_timer[k]) {
    ancilla_timer[k] = 21;
    Ancilla_Sfx3_Near(0x30);
  }
  OamEnt *oam = GetOamCurPtr();
  int i = ancilla_step[k];
  do {
    static const uint8 kCaneSpark_Char[4] = {0xd7, 0xb7, 0x80, 0x83};
    if (!submodule_index)
      swordbeam_arr[i] = (swordbeam_arr[i] + 3) & 0x3f;
    Point16U pt = Sparkle_PrepOamFromRadial(Ancilla_GetRadialProjection(swordbeam_arr[i], swordbeam_var2));
    Ancilla_SetOam(oam, pt.x, pt.y, kCaneSpark_Char[i], flags | HIBYTE(oam_priority_value), 0);
    Ancilla_SetXY(k, pt.x + BG2HOFS_copy2, pt.y + BG2VOFS_copy2);
    ancilla_dir[k] = 0;
    Ancilla_CheckSpriteCollision(k);
  } while (oam++, --i >= 0);
}

void Ancilla_SwordBeam(int k) {  // 88ddc5
  uint8 flags = 2;

  if (!submodule_index) {
    Ancilla_SetXY(k, swordbeam_temp_x, swordbeam_temp_y);
    Ancilla_MoveX(k);
    Ancilla_MoveY(k);
    swordbeam_temp_x = Ancilla_GetX(k);
    swordbeam_temp_y = Ancilla_GetY(k);

    if ((ancilla_G[k]++ & 0xf) == 0) {
      sound_effect_2 = Ancilla_CalculateSfxPan(k) | 1;
    }

    if (Ancilla_CheckSpriteCollision(k) >= 0 || Ancilla_CheckTileCollision(k)) {
      static const int8 kSwordBeam_Yvel2[4] = {0, 0, -6, -6};
      static const int8 kSwordBeam_Xvel2[4] = {-8, -10, 0, 0};
      int j = ancilla_dir[k];
      Ancilla_SetXY(k,
          Ancilla_GetX(k) + kSwordBeam_Xvel2[j],
          Ancilla_GetY(k) + kSwordBeam_Yvel2[j]);
      ancilla_type[k] = 4;
      ancilla_timer[k] = 7;
      ancilla_numspr[k] = 0x10;
      return;
    }
    if (sign8(--ancilla_aux_timer[k])) {
      flags = 4;
      ancilla_aux_timer[k] = 2;
    }
  }

  OamEnt *oam = GetOamCurPtr();
  uint8 s = ancilla_S[k];
  for (int i = 3; i >= 0; i--, oam++) {
    static const uint8 kSwordBeam_Char[4] = {0xd7, 0xb7, 0x80, 0x83};
    if (submodule_index == 0)
      swordbeam_arr[i] = (swordbeam_arr[i] + s) & 0x3f;
    Point16U pt = Sparkle_PrepOamFromRadial(Ancilla_GetRadialProjection(swordbeam_arr[i], swordbeam_var2));
    Ancilla_SetOam(oam, pt.x, pt.y, kSwordBeam_Char[i], flags | HIBYTE(oam_priority_value), 0);
  }

  if (submodule_index == 0) {
    if (!sign8(--ancilla_arr3[k]))
      goto endif_2;

    ancilla_arr3[k] = 0;
    ancilla_arr1[k] = (ancilla_arr1[k] + 1) & 3;
    if (ancilla_arr1[k] == 3)
      swordbeam_var1 = (swordbeam_var1 + s) & 0x3f;
  }

  uint8 t;

  t = ancilla_arr1[k];
  if (t != 3) {
    static const uint8 kSwordBeam_Char2[3] = {0xb7, 0x80, 0x83};
    Point16U pt = Sparkle_PrepOamFromRadial(Ancilla_GetRadialProjection(swordbeam_var1, swordbeam_var2));
    Ancilla_SetOam(oam, pt.x, pt.y, kSwordBeam_Char2[t], 4 | HIBYTE(oam_priority_value), 0);
  }
endif_2:
  oam -= 4;
  for (int i = 0; i < 4; i++) {
    if (oam[i].y != 0xf0)
      return;
  }
  ancilla_type[k] = 0;
}

void Ancilla0D_SpinAttackFullChargeSpark(int k) {  // 88ddca
  static const int8 kSwordFullChargeSpark_Y[4] = {-8, 27, 12, 12};
  static const int8 kSwordFullChargeSpark_X[4] = {4, 4, -13, 20};
  static const uint8 kSwordFullChargeSpark_Flags[4] = {0x20, 0x10, 0x30, 0x20};

  ancilla_oam_idx[k] = Ancilla_AllocateOamFromRegion_A_or_D_or_F(k, 4);

  if (!ancilla_timer[k]) {
    ancilla_type[k] = 0;
    return;
  }

  int j = link_direction_facing >> 1;

  uint16 x = link_x_coord + kSwordFullChargeSpark_X[j] - BG2HOFS_copy2;
  uint16 y = link_y_coord + kSwordFullChargeSpark_Y[j] - BG2VOFS_copy2;

  oam_priority_value = kSwordFullChargeSpark_Flags[ancilla_floor[k]] << 8;
  OamEnt *oam = GetOamCurPtr();
  Ancilla_SetOam(oam, x, y, 0xd7, HIBYTE(oam_priority_value) | 2, 0);
}

void Ancilla27_Duck(int k) {  // 88dde8
  CheckPlayerCollOut coll;
  int j;

  if (submodule_index)
    goto endif_1;

  if (ancilla_timer[k]) {
    int xt = (enhanced_features0 & kFeatures0_ExtendScreen64) ? 0x40 : 0;
    Ancilla_SetXY(k, BG2HOFS_copy2 - 16 - xt, link_y_coord - 8);
    return;
  }

  if (sign8(--ancilla_G[k])) {
    ancilla_G[k] = 0x28;
    Ancilla_Sfx3_Pan(k, 0x1e);
  }

  if (ancilla_L[k] || ancilla_step[k] && (flag_unk1++, true)) {
    ancilla_z_vel[k]--;
    Ancilla_MoveZ(k);
  }
  Ancilla_MoveX(k);


  if (ancilla_L[k]) {
    uint16 x = Ancilla_GetX(k);
    if (ancilla_step[k])
      flag_unk1++;
    if (!sign16(x) && x >= link_x_coord) {
      if (ancilla_step[k]) {
        ancilla_step[k] = 0;
        link_visibility_status = 0;
        tagalong_var5 = 0;
        link_pose_for_item = 0;
        ancilla_y_vel[k] = 0;
        flag_is_link_immobilized = 0;
        link_disable_sprite_damage = 0;
        byte_7E03FD = 0;
        countdown_for_blink = 144;
        if (!((follower_indicator == 12 || follower_indicator == 13) && follower_dropped)) {
          Follower_Initialize();
        }
      }
    } else if ((uint16)(link_x_coord - x) < 48) {
      j = 3;
      goto endif_5;
    }
    goto endif_1;
  }

  if (!Ancilla_CheckLinkCollision(k, 1, &coll) || main_module_index == 15)
    goto endif_1;

  if (!player_is_indoors) {
    if (link_player_handler_state == 8 || link_player_handler_state == 9 || link_player_handler_state == 10 ||
        player_near_pit_state == 2 ||
        (link_pose_for_item | related_to_hookshot | link_force_hold_sword_up | link_disable_sprite_damage) ||
        (link_state_bits & 0x80))
      goto endif_1;
    for (int i = 4; i >= 0; i--) {
      uint8 a = ancilla_type[i];
      if (a == 0x2a || a == 0x1f || a == 0x30 || a == 0x31 || a == 0x41)
        ancilla_type[i] = 0;
    }
    if (follower_indicator == 9) {
      follower_indicator = 0;
      tagalong_var5 = 0;
    }
  }
  link_state_bits = 0;
  link_picking_throw_state = 0;

  bg1_x_offset = 0;
  bg1_y_offset = 0;
  Link_ResetProperties_A();
  link_is_in_deep_water = 0;
  link_need_for_pullforrupees_sprite = 0;
  link_visibility_status = 12;
  link_player_handler_state = 0;
  link_pose_for_item = 1;
  flag_is_link_immobilized = 1;
  link_disable_sprite_damage = 1;
  tagalong_var5 = 1;
  ancilla_step[k] = 2;
  flag_unk1++;
  link_give_damage = 0;
  if (player_is_indoors)
    byte_7E03FD = player_is_indoors;
endif_1:
  if (sign8(--ancilla_arr3[k])) {
    ancilla_arr3[k] = 3;
    if (++ancilla_K[k] == 3)
      ancilla_K[k] = 0;
  }
  j = ancilla_K[k];
endif_5:
  BYTE(flag_travel_bird) = kTravelBird_DmaStuffs[j];

  Point16U info;
  Ancilla_PrepOamCoord(k, &info);

  OamEnt *oam = GetOamCurPtr();
  int z = ancilla_z[k] ? ancilla_z[k] | ~0xff : 0;
  int i = 0, n = ancilla_step[k] + 1;
  do {
    Ancilla_SetOam(oam, info.x + (int8)kTravelBird_Draw_X[i], info.y + z + (int8)kTravelBird_Draw_Y[i],
                   kTravelBird_Draw_Char[i], kTravelBird_Draw_Flags[i] | 0x30, 2);
    oam++;
  } while (++i != n);

  AncillaDraw_Shadow(oam, 1, info.x, info.y + 28, 0x30);
  oam += 2;
  if (ancilla_step[k])
    AncillaDraw_Shadow(oam, 1, info.x - 7, info.y + 28, 0x30);

  if (!sign16(info.x) && info.x >= 0x130) {
    ancilla_type[k] = 0;
    if (!ancilla_L[k] && ancilla_step[k]) {
      submodule_index = 10;
      saved_module_for_menu = main_module_index;
      main_module_index = 14;
    }
  }
}

int AncillaAdd_SomariaBlock(uint8 type, uint8 y) {  // 88e078
  int k = AncillaAdd_AddAncilla_Bank08(type, y);
  if (k < 0)
    return k;
  for (int j = 4; j >= 0; j--) {
    if (j == k || ancilla_type[j] != 0x2c)
      continue;
    if (j == flag_is_ancilla_to_pick_up - 1)
      flag_is_ancilla_to_pick_up = 0;
    AncillaAdd_ExplodingSomariaBlock(j);
    ancilla_type[k] = 0;
    dung_flag_somaria_block_switch = 0;
    if (link_speed_setting == 0x12) {
      bitmask_of_dragstate = 0;
      link_speed_setting = 0;
    }
    return k;
  }

  Ancilla_Sfx3_Near(0x2a);
  ancilla_step[k] = 0;
  ancilla_y_vel[k] = 0;
  ancilla_x_vel[k] = 0;
  ancilla_item_to_link[k] = 0;
  ancilla_aux_timer[k] = 0;
  ancilla_arr3[k] = 0;
  ancilla_arr1[k] = 0;
  ancilla_H[k] = 0;
  ancilla_G[k] = 12;
  ancilla_timer[k] = 18;
  ancilla_L[k] = 0;
  ancilla_z[k] = 0;
  ancilla_K[k] = 0;
  ancilla_R[k] = 0;
  ancilla_arr4[k] = 0;
  ancilla_S[k] = 9;
  ancilla_T[k] = 0;
  ancilla_dir[k] = link_direction_facing >> 1;
  if (Ancilla_CheckInitialTileCollision_Class2(k)) {
    Ancilla_SetX(k, link_x_coord + 8);
    Ancilla_SetY(k, link_y_coord + 16);
  } else {
    static const int8 kCaneOfSomaria_Y[4] = { -8, 31, 17, 17 };
    static const int8 kCaneOfSomaria_X[4] = { 8, 8, -8, 23 };
    int j = link_direction_facing >> 1;
    Ancilla_SetX(k, link_x_coord + kCaneOfSomaria_X[j]);
    Ancilla_SetY(k, link_y_coord + kCaneOfSomaria_Y[j]);
    SomariaBlock_CheckForTransitTile(k);
  }
  return k;
}

void SomariaBlock_CheckForTransitTile(int k) {  // 88e191
  static const int8 kSomariaTransitLine_X[12] = { -8, 0, 8, -8, 0, 8, -16, -16, -16, 16, 16, 16 };
  static const int8 kSomariaTransitLine_Y[12] = { -16, -16, -16, 16, 16, 16, -8, 0, 8, -8, 0, 8 };
  if (!dung_unk6)
    return;
  for (int j = 11; j >= 0; j--) {
    uint16 x = Ancilla_GetX(k) + kSomariaTransitLine_X[j];
    uint16 y = Ancilla_GetY(k) + kSomariaTransitLine_Y[j];
    uint8 bak = ancilla_objprio[k];
    Ancilla_CheckTileCollision_targeted(k, x, y);
    ancilla_objprio[k] = bak;
    if (ancilla_tile_attr[k] == 0xb6 || ancilla_tile_attr[k] == 0xbc) {
      Ancilla_SetX(k, x);
      Ancilla_SetY(k, y);
      AncillaAdd_SomariaPlatformPoof(k);
      return;
    }
  }
}

int Ancilla_CheckBasicSpriteCollision(int k) {  // 88e1f9
  for (int j = 15; j >= 0; j--) {
    if (((j ^ frame_counter) & 3 | sprite_pause[j] | sprite_hit_timer[j]) != 0)
      continue;
    if (sprite_state[j] < 9 || !(sprite_defl_bits[j] & 2) && ancilla_objprio[k])
      continue;
    if (ancilla_floor[k] != sprite_floor[j])
      continue;
    if (ancilla_type[k] == 0x2c && (sprite_type[j] == 0x1e || sprite_type[j] == 0x90))
      continue;
    if (Ancilla_CheckBasicSpriteCollision_Single(k, j))
      return j;
  }
  return -1;
}

bool Ancilla_CheckBasicSpriteCollision_Single(int k, int j) {  // 88e23d
  SpriteHitBox hb;
  Ancilla_SetupBasicHitBox(k, &hb);
  Sprite_SetupHitBox(j, &hb);
  if (!CheckIfHitBoxesOverlap(&hb))
    return false;
  if (sprite_type[j] == 0x92 && sprite_C[j] < 3)
    return true;
  if (sprite_type[j] == 0x80 && sprite_delay_aux4[j] == 0) {
    sprite_delay_aux4[j] = 24;
    sprite_D[j] ^= 1;
  }
  if (sprite_ignore_projectile[j])
    return false;

  int x = Ancilla_GetX(k) - 8, y = Ancilla_GetY(k) - 8 - ancilla_z[k];
  ProjectSpeedRet pt = Sprite_ProjectSpeedTowardsLocation(j, x, y, 80);
  sprite_y_recoil[j] = ~pt.y;
  sprite_x_recoil[j] = ~pt.x;
  Ancilla_CheckDamageToSprite(j, ancilla_type[k]);
  return true;
}

void Ancilla_SetupBasicHitBox(int k, SpriteHitBox *hb) {  // 88e2ca
  int x = Ancilla_GetX(k) - 8;
  hb->r0_xlo = x;
  hb->r8_xhi = x >> 8;
  int y = Ancilla_GetY(k) - 8 - ancilla_z[k];
  hb->r1_ylo = y;
  hb->r9_yhi = y >> 8;
  hb->r2 = 15;
  hb->r3 = 15;
}

void Ancilla2C_SomariaBlock(int k) {  // 88e365
  if (!sign8(--ancilla_G[k]))
    return;
  ancilla_G[k] = 0;

  if (ancilla_H[k])
    goto label_1;
  if (submodule_index == 0 || submodule_index == 8 || submodule_index == 16) {
    Ancilla_HandleLiftLogic(k);
  } else if (k + 1 == flag_is_ancilla_to_pick_up && ancilla_K[k] != 0) {
    if (ancilla_K[k] != 3) {
      Ancilla_LatchLinkCoordinates(k, 3);
      Ancilla_LatchAltitudeAboveLink(k);
      ancilla_K[k] = 3;
    }
    Ancilla_LatchCarriedPosition(k);
  }
  if (player_is_indoors) {
    if (!ancilla_K[k] && !(link_state_bits & 0x80) && (ancilla_z[k] == 0 || ancilla_z[k] == 0xff)) {
      if (dung_unk6) {
        int j = frame_counter & 3;
        do {
          uint8 bak = ancilla_objprio[k];
          uint16 x = Ancilla_GetX(k) + kSomarianBlock_Coll_X[j];
          uint16 y = Ancilla_GetY(k) + kSomarianBlock_Coll_Y[j];
          Ancilla_CheckTileCollision_targeted(k, x, y);
          ancilla_objprio[k] = bak;
          if (ancilla_tile_attr[k] == 0xb6 || ancilla_tile_attr[k] == 0xbc) {
            Ancilla_SetXY(k, x, y);
            AncillaAdd_SomariaPlatformPoof(k);
            if (k + 1 == flag_is_ancilla_to_pick_up)
              flag_is_ancilla_to_pick_up = 0;
            return;
          }
        } while ((j += 4) < 12);
      } else {
        if (!SomariaBlock_CheckForSwitch(k) && (ancilla_z[k] == 0 || ancilla_z[k] == 0xff))
          dung_flag_somaria_block_switch++;
      }
    } else {
label_1:
      if (flag_is_ancilla_to_pick_up == k + 1)
        dung_flag_somaria_block_switch = 0;
    }
  }

  uint16 old_y = Ancilla_LatchYCoordToZ(k);
  uint8 s1a = ancilla_dir[k];
  uint8 s1b = ancilla_objprio[k];
  ancilla_objprio[k] = 0;
  bool flag = Ancilla_CheckTileCollision_Class2(k);

  if (player_is_indoors && ancilla_L[k] && ancilla_tile_attr[k] == 0x1c)
    ancilla_T[k] = 1;

label1:
  if (flag && (!(link_state_bits & 0x80) || link_picking_throw_state)) {
    if (!s1b && !ancilla_arr4[k] && ancilla_z[k]) {
      ancilla_arr4[k] = 1;
      int qq = (ancilla_dir[k] == 1) ? 16 : 4;
      if (ancilla_y_vel[k])
        ancilla_y_vel[k] = sign8(ancilla_y_vel[k]) ? qq : -qq;
      if (ancilla_x_vel[k])
        ancilla_x_vel[k] = sign8(ancilla_x_vel[k]) ? 4 : -4;
      if (ancilla_dir[k] == 1 && ancilla_z[k]) {
        ancilla_y_vel[k] = -4;
        ancilla_L[k] = 2;
      }
    }
  } else if (!(link_state_bits & 0x80) && (ancilla_z[k] == 0 || ancilla_z[k] == 0xff)) {
    ancilla_dir[k] = 16;
    uint8 bak0 = ancilla_objprio[k];
    Ancilla_CheckTileCollision(k);
    ancilla_objprio[k] = bak0;
    uint8 a = ancilla_tile_attr[k];
    if (a == 0x26) {
      flag = true;
      goto label1;
    } else if (a == 0xc || a == 0x1c) {
      if (dung_hdr_collision != 3) {
        if (ancilla_floor[k] == 0 && ancilla_z[k] != 0 && ancilla_z[k] != 0xff)
          ancilla_floor[k] = 1;
      } else {
        old_y = Ancilla_GetY(k) + dung_floor_y_vel;
        Ancilla_SetX(k, Ancilla_GetX(k) + dung_floor_x_vel);
      }
    } else if (a == 0x20 || (a & 0xf0) == 0xb0 && a != 0xb6 && a != 0xbc) {
      if (!(link_state_bits & 0x80)) {
        if (k + 1 == flag_is_ancilla_to_pick_up)
          flag_is_ancilla_to_pick_up = 0;
        if (!ancilla_timer[k]) {
          if (link_speed_setting == 18) {
            link_speed_setting = 0;
            bitmask_of_dragstate = 0;
          }
          ancilla_type[k] = 0;
          return;
        }
      }
    } else if (a == 8) {
      if (k + 1 == flag_is_ancilla_to_pick_up)
        flag_is_ancilla_to_pick_up = 0;
      if (ancilla_timer[k] == 0) {
        Ancilla_SetY(k, Ancilla_GetY(k) - 24);
        Ancilla_TransmuteToSplash(k);
        return;
      }
    } else if (a == 0x68 || a == 0x69 || a == 0x6a || a == 0x6b) {
      Ancilla_ApplyConveyor(k);
      old_y = Ancilla_GetY(k);
    } else {
      ancilla_timer[k] = (ancilla_L[k] | ancilla_H[k]) ? 0 : 2;
    }
  }
  // endif_3
  s1b |= ancilla_objprio[k];

  if (!(link_state_bits & 0x80) && !--ancilla_S[k]) {
    ancilla_S[k] = 1;
    ancilla_objprio[k] = 0;
    if (Ancilla_CheckBasicSpriteCollision(k) >= 0) {
      ancilla_S[k] = 7;
      if (++ancilla_step[k] == 5) {
        SomariaBlock_FizzleAway(k);
        return;
      }
    }
  }
  Ancilla_SetY(k, old_y);
  ancilla_dir[k] = s1a;
  ancilla_objprio[k] = s1b;

  AncillaDraw_SomariaBlock(k);
}

void AncillaDraw_SomariaBlock(int k) {  // 88e61b
  static const int8 kSomarianBlock_Draw_X[12] = {-8, 0, -8, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  static const int8 kSomarianBlock_Draw_Y[12] = {-8, -8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  static const uint8 kSomarianBlock_Draw_Flags[12] = {0, 0x40, 0x80, 0xc0, 0, 0x40, 0x80, 0xc0, 0, 0x40, 0x80, 0xc0};

  if (k + 1 == flag_is_ancilla_to_pick_up && link_state_bits & 0x80 && ancilla_K[k] != 3 && link_direction_facing == 0) {
    Ancilla_AllocateOamFromRegion_B_or_E(ancilla_numspr[k]);
  } else if (sort_sprites_setting && ancilla_floor[k] && (ancilla_L[k] || k + 1 == flag_is_ancilla_to_pick_up && (link_state_bits & 0x80))) {
    oam_cur_ptr = 0x8d0;
    oam_ext_cur_ptr = 0xa20 + 0x34;
  }

  Point16U pt;
  Ancilla_PrepAdjustedOamCoord(k, &pt);
  OamEnt *oam = GetOamCurPtr(), *oam_org = oam;
  int z = (int8)ancilla_z[k];
  if (z != 0 && z != -1 && ancilla_K[k] != 3 && ancilla_objprio[k])
    oam_priority_value = 0x3000;
  pt.y -= z;
  int j = ancilla_arr1[k] * 4;
  int r8 = 0;
  do {
    Ancilla_SetOam_Safe(oam, pt.x + kSomarianBlock_Draw_X[j], pt.y + kSomarianBlock_Draw_Y[j], 0xe9,
                        kSomarianBlock_Draw_Flags[j] & ~0x30 | 2 | HIBYTE(oam_priority_value), 0);
    oam++;
  } while (j++, ++r8 & 3);

  if (SomarianBlock_CheckEmpty(oam_org)) {
    dung_flag_somaria_block_switch = 0;
    ancilla_type[k] = 0;
    if (k + 1 == flag_is_ancilla_to_pick_up) {
      flag_is_ancilla_to_pick_up = 0;
      if (link_state_bits & 128)
        link_state_bits = 0;
    }
  }
}

bool SomariaBlock_CheckForSwitch(int k) {  // 88e75c
  static const int8 kSomarianBlock_CheckCover_X[4] = {0, 0, -4, 4};
  static const int8 kSomarianBlock_CheckCover_Y[4] = {-4, 4, 0, 0};
  dung_flag_somaria_block_switch = 0;
  ancilla_arr24[k] = 0;
  for (int j = 3; j >= 0; j--) {
    uint16 y = Ancilla_GetY(k) + kSomarianBlock_CheckCover_Y[j];
    uint16 x = Ancilla_GetX(k) + kSomarianBlock_CheckCover_X[j];
    uint8 bak = ancilla_objprio[k];
    Ancilla_CheckTileCollision_targeted(k, x, y);
    ancilla_objprio[k] = bak;
    uint8 a = ancilla_tile_attr[k];
    if (a == 0x23 || a == 0x24 || a == 0x25 || a == 0x3b)
      ancilla_arr24[k]++;
  }
  return ancilla_arr24[k] != 4;
}

void SomariaBlock_FizzleAway(int k) {  // 88e9b2
  if (link_speed_setting == 18) {
    bitmask_of_dragstate = 0;
    link_speed_setting = 0;
  }
  dung_flag_somaria_block_switch = 0;
  ancilla_type[k] = 0x2d;
  ancilla_aux_timer[k] = 0;
  ancilla_step[k] = 0;
  ancilla_item_to_link[k] = 0;
  ancilla_arr3[k] = 0;
  ancilla_arr1[k] = 0;
  ancilla_R[k] = 0;
  if (k + 1 == flag_is_ancilla_to_pick_up) {
    flag_is_ancilla_to_pick_up = 0;
    link_state_bits &= 0x80;
  }
  Ancilla2D_SomariaBlockFizz(k);
}

void Ancilla2D_SomariaBlockFizz(int k) {  // 88e9e8
  static const int8 kSomariaBlockFizzle_X[6] = {-4, -1, -8, 0, -6, -2};
  static const int8 kSomariaBlockFizzle_Y[6] = {-4, -1, -4, -4, -4, -4};
  static const uint8 kSomariaBlockFizzle_Char[6] = {0x92, 0xff, 0xf9, 0xf9, 0xf9, 0xf9};
  static const uint8 kSomariaBlockFizzle_Flags[6] = {6, 0xff, 0x86, 0xc6, 0x86, 0xc6};
  if (sign8(--ancilla_aux_timer[k])) {
    ancilla_aux_timer[k] = 3;
    if (++ancilla_item_to_link[k] == 3) {
      ancilla_type[k] = 0;
      return;
    }
  }
  Point16U pt;
  Ancilla_PrepAdjustedOamCoord(k, &pt);
  OamEnt *oam = GetOamCurPtr();
  uint8 z = ancilla_z[k];
  if (z == 0xff)
    z = 0;
  int x = pt.x, y = pt.y - (int8)z;
  int j = ancilla_item_to_link[k] * 2;
  for (int i = 0; i < 2; i++, j++, oam++) {
    if (kSomariaBlockFizzle_Char[j] != 0xff) {
      Ancilla_SetOam(oam, x + kSomariaBlockFizzle_X[j], y + kSomariaBlockFizzle_Y[j],
                     kSomariaBlockFizzle_Char[j],
                     kSomariaBlockFizzle_Flags[j] & ~0x30 | HIBYTE(oam_priority_value), 0);
    }
  }
}

void Ancilla39_SomariaPlatformPoof(int k) {  // 88ea83
  static const uint8 kSomarianPlatformPoof_Tab0[4] = {1, 0, 3, 2};
  if (!sign8(--ancilla_aux_timer[k]))
    return;
  ancilla_type[k] = 0;
  SpriteSpawnInfo info;
  int x = Ancilla_GetX(k) & ~7 | 4, y = Ancilla_GetY(k) & ~7 | 4;
  uint8 floor = ancilla_floor[k];
  int j = Sprite_SpawnDynamically(k, 0xed, &info);  // wtf
  if (j >= 0) {
    player_on_somaria_platform = 0;
    Sprite_SetX(j, x);
    Sprite_SetY(j, y);

    int pos = ((x & 0x1f8) >> 3) + ((y & 0x1f8) << 3) + (floor >= 1 ? 0x1000 : 0);

    int t = 0;
    if ((dung_bg2_attr_table[pos + XY(0, -1)] & 0xf0) != 0xb0) {
      t += 1;
      if ((dung_bg2_attr_table[pos + XY(0, 1)] & 0xf0) != 0xb0) {
        t += 1;
        if ((dung_bg2_attr_table[pos + XY(-1, 0)] & 0xf0) != 0xb0) {
          t += 1;
        }
      }
    }
    sprite_D[j] = kSomarianPlatformPoof_Tab0[t];
    sprite_floor[j] = 0;
  } else {
    AncillaDraw_SomariaBlock(k);
  }
}

void Ancilla2E_SomariaBlockFission(int k) {  // 88eb3e
  static const int8 kSomarianBlockDivide_X[16] = {-8, 0, -8, 0, -10, -10, 2, 2, -8, 0, -8, 0, -12, -12, 4, 4};
  static const int8 kSomarianBlockDivide_Y[16] = {-10, -10, 2, 2, -8, 0, -8, 0, -12, -12, 4, 4, -8, 0, -8, 0};
  static const uint8 kSomarianBlockDivide_Char[16] = {0xc6, 0xc6, 0xc6, 0xc6, 0xc4, 0xc4, 0xc4, 0xc4, 0xd2, 0xd2, 0xd2, 0xd2, 0xc5, 0xc5, 0xc5, 0xc5};
  static const uint8 kSomarianBlockDivide_Flags[16] = {0xc6, 0x86, 0x46, 6, 0x46, 0xc6, 6, 0x86, 0xc6, 0x86, 0x46, 6, 0x46, 0xc6, 6, 0x86};

  if (sign8(--ancilla_aux_timer[k])) {
    ancilla_aux_timer[k] = 3;
    if (++ancilla_item_to_link[k] == 2) {
      ancilla_type[k] = 0;
      SomariaBlock_SpawnBullets(k);
      return;
    }
  }
  Point16U pt;
  Ancilla_PrepAdjustedOamCoord(k, &pt);
  OamEnt *oam = GetOamCurPtr();

  int8 z = ancilla_z[k] + (ancilla_K[k] == 3 && BYTE(link_z_coord) != 0xff ? BYTE(link_z_coord) : 0);
  int j = ancilla_item_to_link[k] * 8;
  for (int i = 0; i != 8; i++, j++, oam++) {
    Ancilla_SetOam(oam, pt.x + kSomarianBlockDivide_X[j], pt.y + kSomarianBlockDivide_Y[j] - z,
                   kSomarianBlockDivide_Char[j], kSomarianBlockDivide_Flags[j] & ~0x30 | HIBYTE(oam_priority_value), 0);
  }
}

void Ancilla2F_LampFlame(int k) {  // 88ec13
  static const uint8 kLampFlame_Draw_Char[12] = {0x9c, 0x9c, 0xff, 0xff, 0xa4, 0xa5, 0xb2, 0xb3, 0xe3, 0xf3, 0xff, 0xff};
  static const int8 kLampFlame_Draw_Y[12] = {-3, 0, 0, 0, 0, 0, 8, 8, 0, 8, 0, 0};
  static const int8 kLampFlame_Draw_X[12] = {4, 10, 0, 0, 1, 9, 2, 7, 4, 4, 0, 0};

  Point16U pt;
  Ancilla_PrepAdjustedOamCoord(k, &pt);
  OamEnt *oam = GetOamCurPtr();
  if (!ancilla_timer[k]) {
    ancilla_type[k] = 0;
    return;
  }
  int j = (ancilla_timer[k] & 0xf8) >> 1;
  do {
    if (kLampFlame_Draw_Char[j] != 0xff) {
      Ancilla_SetOam(oam, pt.x + kLampFlame_Draw_X[j], pt.y + kLampFlame_Draw_Y[j], kLampFlame_Draw_Char[j], HIBYTE(oam_priority_value) | 2, 0);
      oam++;
    }
  } while (++j & 3);
}

void Ancilla41_WaterfallSplash(int k) {  // 88ecaf
  if (!Ancilla_CheckForEntranceTrigger(player_is_indoors ? 0 : 1)) {
    ancilla_type[k] = 0;
    return;
  }

  if (!submodule_index && !(frame_counter & 7))
    Ancilla_Sfx2_Near(0x1c);

  draw_water_ripples_or_grass = 1;
  if (!sign8(link_animation_steps - 6))
    link_animation_steps -= 6;

  if (!ancilla_timer[k]) {
    ancilla_timer[k] = 2;
    ancilla_item_to_link[k] = (ancilla_item_to_link[k] + 1) & 3;
  }

  if (player_is_indoors && BYTE(link_y_coord) < 0x38) {
    Ancilla_SetY(k, 0xd38);
  } else {
    Ancilla_SetY(k, link_y_coord);
  }
  Ancilla_SetX(k, link_x_coord);

  static const int8 kWaterfallSplash_X[8] = {0, 0, -4, 4, -7, 7, -9, 17};
  static const int8 kWaterfallSplash_Y[8] = {-4, 0, -5, -5, -3, -3, 12, 12};
  static const uint8 kWaterfallSplash_Char[8] = {0xc0, 0xff, 0xac, 0xac, 0xae, 0xae, 0xbf, 0xbf};
  static const uint8 kWaterfallSplash_Flags[8] = {0x84, 0xff, 0x84, 0xc4, 0x84, 0xc4, 0x84, 0xc4};
  static const uint8 kWaterfallSplash_Ext[8] = {2, 0xff, 2, 2, 2, 2, 0, 0};

  Point16U pt;
  Ancilla_PrepAdjustedOamCoord(k, &pt);
  OamEnt *oam = GetOamCurPtr();

  uint8 z = link_z_coord;
  pt.y -= (sign8(z) ? 0 : z);

  int j = ancilla_item_to_link[k] * 2;
  for (int i = 0; i != 2; i++, j++, oam++) {
    if (kWaterfallSplash_Char[j] != 0xff) {
      Ancilla_SetOam(oam, pt.x + kWaterfallSplash_X[j], pt.y + kWaterfallSplash_Y[j], kWaterfallSplash_Char[j],
                     kWaterfallSplash_Flags[j] | 0x30, kWaterfallSplash_Ext[j]);
    }
  }
}

void Ancilla24_Gravestone(int k) {  // 88ee01
  static const uint8 kAncilla_Gravestone_Char[4] = {0xc8, 0xc8, 0xd8, 0xd8};
  static const uint8 kAncilla_Gravestone_Flags[4] = {0, 0x40, 0, 0x40};
  Point16U pt;
  Ancilla_PrepAdjustedOamCoord(k, &pt);
  Oam_AllocateFromRegionB(16);
  OamEnt *oam = GetOamCurPtr();
  uint16 x = pt.x, y = pt.y;
  for (int i = 0; i < 4; i++, oam++) {
    Ancilla_SetOam(oam, x, y, kAncilla_Gravestone_Char[i], kAncilla_Gravestone_Flags[i] | 0x3d, 2);
    x += 16;
    if (i == 1)
      x -= 32, y += 8;
  }
}

void Ancilla34_SkullWoodsFire(int k) {  // 88ef9a
  static const int8 kSkullWoodsFire_Draw_Y[4] = {0, 0, 0, -3};
  static const uint8 kSkullWoodsFire_Draw_Char[4] = {0x8e, 0xa0, 0xa2, 0xa4};
  static const uint8 kSkullWoodsFire_Draw_Ext[4] = {2, 2, 2, 0};
  static const int8 kSkullWoodsFire_Draw2_X[24] = {
    -13, -21, -10, -1,  -1,  -1, -16, -27, -4, -16, -6, -25, -16, -27, -4, -16,
    -6, -25, -13, -5, -27, -11, -22,  -3,
  };
  static const int8 kSkullWoodsFire_Draw2_Y[24] = {
    -31, -24, -22,  -1,  -1,  -1, -37, -32, -32, -23, -16, -14, -37, -32, -32, -23,
    -16, -14, -35, -29, -28, -20, -13, -11,
  };
  static const uint8 kSkullWoodsFire_Draw2_Char[24] = {
    0x86, 0x86, 0x86, 0xff, 0xff, 0xff, 0x86, 0x86, 0x86, 0x86, 0x86, 0x86, 0x8a, 0x8a, 0x8a, 0x8a,
    0x8a, 0x8a, 0x9b, 0x9b, 0x9b, 0x9b, 0x9b, 0x9b,
  };
  static const uint8 kSkullWoodsFire_Draw2_Flags[24] = {
    0, 0,    0,    0,    0,    0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0x80, 0x40, 0x40, 0x80, 0x40, 0,
  };
  static const uint8 kSkullWoodsFire_Draw2_Ext[24] = {
    2, 2, 2, 2, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 0, 0, 0, 0, 0, 0,
  };
  if (skullwoodsfire_var4 && ancilla_item_to_link[k] != 4 && sign8(--ancilla_aux_timer[k])) {
    ancilla_aux_timer[k] = 5;
    ancilla_item_to_link[k]++;
  }
  OamEnt *oam = GetOamCurPtr();
  for(int k = 3; k >= 0; k--) {
    if (sign8(--skullwoodsfire_var5[k])) {
      skullwoodsfire_var5[k] = 5;
      if (skullwoodsfire_var0[k] == 128)
        goto endif_2;
      if (++skullwoodsfire_var0[k] != 0) {
        if (skullwoodsfire_var0[k] != 4)
          goto endif_2;
        skullwoodsfire_var0[k] = 0;
      }
      skullwoodsfire_var9 -= 8;
      if (skullwoodsfire_var9 < 200 && skullwoodsfire_var4 != 1) {
        skullwoodsfire_var4 = 1;
        sound_effect_1 = kBombos_Sfx[(uint8)(0x98 - BG2HOFS_copy2) >> 5] | 0xc;
      }
      if (skullwoodsfire_var9 < 168)
        skullwoodsfire_var0[k] = 128;
      skullwoodsfire_x_arr[k] = skullwoodsfire_var11;
      skullwoodsfire_y_arr[k] = skullwoodsfire_var9;
      if (sound_effect_1 == 0)
        sound_effect_1 = kBombos_Sfx[(uint8)(skullwoodsfire_var11 - BG2HOFS_copy2) >> 5] | 0x2a;
    }
endif_2:
    if (!sign8(skullwoodsfire_var0[k])) {
      int j = skullwoodsfire_var0[k];
      uint16 x = skullwoodsfire_x_arr[k] - BG2HOFS_copy2;
      uint16 y = skullwoodsfire_y_arr[k] - BG2VOFS_copy2 + kSkullWoodsFire_Draw_Y[j];
      Ancilla_SetOam(oam, x, y, kSkullWoodsFire_Draw_Char[j], 0x32, kSkullWoodsFire_Draw_Ext[j]);
      oam++;
      if (kSkullWoodsFire_Draw_Ext[j] != 2) {
        Ancilla_SetOam(oam, x + 8, y, kSkullWoodsFire_Draw_Char[j] + 1, 0x32, kSkullWoodsFire_Draw_Ext[j]);
        oam++;
      }
    }
  }

  for (int i = 3; sign8(skullwoodsfire_var0[i]); ) {
    if (--i < 0) {
      ancilla_type[k] = 0;
      return;
    }
  }

  if (skullwoodsfire_var4 == 0 || ancilla_item_to_link[k] == 4)
    return;

  int j = ancilla_item_to_link[k] * 6;
  for (int i = 0; i < 6; i++, j++) {
    if (kSkullWoodsFire_Draw2_Char[j] != 0xff) {
      Ancilla_SetOam(oam,
          168 - BG2HOFS_copy2 + kSkullWoodsFire_Draw2_X[j],
          200 - BG2VOFS_copy2 + kSkullWoodsFire_Draw2_Y[j],
          kSkullWoodsFire_Draw2_Char[j],
          kSkullWoodsFire_Draw2_Flags[j] | 0x32, kSkullWoodsFire_Draw2_Ext[j]);
      oam++;
    }
  }
}

void Ancilla3A_BigBombExplosion(int k) {  // 88f18d
  static const int8 kSuperBombExplode_X[9] = {0, -16, 0, 16, -24, 24, -16, 0, 16};
  static const int8 kSuperBombExplode_Y[9] = {0, -16, -24, -16, 0, 0, 16, 24, 16};

  if (!submodule_index && !--ancilla_arr3[k]) {
    if (++ancilla_item_to_link[k] == 2)
      Ancilla_Sfx2_Pan(k, 0xc);
    if (ancilla_item_to_link[k] == 11) {
      ancilla_type[k] = 0;
      return;
    }
    ancilla_arr3[k] = kBomb_Tab0[ancilla_item_to_link[k]];
  }
  oam_priority_value = 0x3000;
  uint8 numframes = kBomb_Draw_Tab2[ancilla_item_to_link[k]];
  int j = kBomb_Draw_Tab0[ancilla_item_to_link[k]] * 6;
  ancilla_step[k] = j * 2;

  int yy = 0;
  for (int i = 8; i >= 0; i--) {
    uint16 x = Ancilla_GetX(k) + kSuperBombExplode_X[i] - BG2HOFS_copy2;
    uint16 y = Ancilla_GetY(k) + kSuperBombExplode_Y[i] - BG2VOFS_copy2;
    if (x < 256 && y < 256) {
      Ancilla_AllocateOamFromRegion_A_or_D_or_F((uint8)(j * 2), 0x18); // wtf
      OamEnt *oam = GetOamCurPtr() + yy;
      yy += AncillaDraw_Explosion(oam, j, 0, numframes, 0x32, x, y) - oam;

    }
  }
  if (ancilla_item_to_link[k] == 3 && ancilla_arr3[k] == 1) {
    // Changed so this is reset elsewhere. Some code depends on the value 13.
    uint8 old = (enhanced_features0 & kFeatures0_MiscBugFixes) ? follower_indicator : 0;
    follower_indicator = 13;
    Bomb_CheckForDestructibles(Ancilla_GetX(k), Ancilla_GetY(k), 0); // r14?
    follower_indicator = old;
  }
}

void RevivalFairy_Main() {  // 88f283
  static const uint8 kAncilla_RevivalFaerie_Tab0[2] = {0, 0x90};
  static const uint8 kAncilla_RevivalFaerie_Tab1[5] = {0x4b, 0x4d, 0x49, 0x47, 0x49};

  int k = 0;
  switch (ancilla_step[k]) {
  case 0:
    if (!--ancilla_arr3[k]) {
      ancilla_arr3[k] = kAncilla_RevivalFaerie_Tab0[++ancilla_step[k]];
      ancilla_K[k] = 0;
      ancilla_z_vel[k] = 0;
    } else {
      Ancilla_MoveZ(k);
    }
    break;
  case 1:
    if (!--ancilla_arr3[k]) {
      ancilla_step[k]++;
      ancilla_z_vel[k] = 0;
      ancilla_x_vel[k] = 0;
    } else {
      if (ancilla_arr3[k] == 0x4f || ancilla_arr3[k] == 0x8f) {
        ancilla_L[k]++;
        Ancilla_Sfx2_Pan(k, 0x31);
      }
      if (ancilla_L[k] != 0 && sign8(--ancilla_G[k])) {
        ancilla_G[k] = 5;
        if (++ancilla_item_to_link[k] == 3) {
          ancilla_item_to_link[k] = 0;
          ancilla_L[k] = 0;
        }
      }
      ancilla_z_vel[k] += ancilla_K[k] ? 1 : -1;
      if (abs8(ancilla_z_vel[k]) == 8)
        ancilla_K[k] ^= 1;
      Ancilla_MoveZ(k);
    }
    break;
  case 2:
    if (ancilla_z_vel[k] < 24)
      ancilla_z_vel[k] += 1;
    if (ancilla_x_vel[k] < 16)
      ancilla_x_vel[k] += 1;
    Ancilla_MoveX(k);
    Ancilla_MoveZ(k);
    break;
  case 3:
    goto skip_draw;
  }

  {
    Oam_AllocateFromRegionC(12);
    Point16U pt;
    Ancilla_PrepOamCoord(k, &pt);
    OamEnt *oam = GetOamCurPtr();
    int t = (ancilla_step[k] == 1 && ancilla_L[k]) ? ancilla_item_to_link[k] + 1 : 0;
    if (t != 0)
      t += 1;
    else
      t = (frame_counter >> 2) & 1;
    Ancilla_SetOam(oam, pt.x, pt.y - (int8)ancilla_z[k], kAncilla_RevivalFaerie_Tab1[t], 0x74, 2);
    if (oam->y == 0xf0) {
      ancilla_step[k] = 3;
      submodule_index++;
      TM_copy = mapbak_TM;
    }
  }
skip_draw:
  RevivalFairy_Dust();
  RevivalFairy_MonitorHP();
}

void RevivalFairy_Dust() {  // 88f3cf
  int k = 2;
  if (ancilla_step[0] == 0 || ancilla_step[k] == 2 || !sign8(--ancilla_arr3[k]))
    return;
  ancilla_arr3[k] = 0;
  if (!sort_sprites_setting)
    Oam_AllocateFromRegionA(16);
  else
    Oam_AllocateFromRegionD(16);
  if (sign8(--ancilla_aux_timer[k])) {
    ancilla_aux_timer[k] = 3;
    if (ancilla_item_to_link[k] == 9) {
      ancilla_arr3[k] = 32;
      ancilla_step[k]++;
      ancilla_item_to_link[k] = 2;
      return;
    }
    ancilla_arr25[k] = kMagicPowder_Tab0[30 + ++ancilla_item_to_link[k]];
  }
  Ancilla_MagicPowder_Draw(k);
}

void RevivalFairy_MonitorHP() {  // 88f430
  if ((link_health_current == link_health_capacity || link_health_current == 0x38) && !is_doing_heart_animation) {
    if (link_is_in_deep_water) {
      link_some_direction_bits = 4;
      link_player_handler_state = kPlayerState_Swimming;
    } else if (link_is_bunny) {
      link_player_handler_state = kPlayerState_PermaBunny;
      link_is_bunny_mirror = 1;
      //bugfix: dying as permabunny doesn't restore link palette during death animation
      if (enhanced_features0 & kFeatures0_MiscBugFixes)
        LoadGearPalettes_bunny();
    } else {
      link_player_handler_state = kPlayerState_Ground;
    }
    link_auxiliary_state = 0;
    player_unk1 = 0;
    link_var30d = 0;
    some_animation_timer_steps = 0;
    BYTE(link_z_coord) = 0;
    link_incapacitated_timer = 0;
    for(int i = 0; i < 5; i++)
      ancilla_type[i] = 0;
    return;
  }
  int k = 1;
  if (!ancilla_step[k]) {
    if (!--ancilla_arr3[k]) {
      ancilla_arr3[k]++;
      ancilla_z_vel[k] = 4;
      Ancilla_MoveZ(k);
      if (ancilla_z[k] >= 16) {
        ancilla_step[k]++;
        ancilla_z_vel[k] = 2;
      }
    }
  } else {
    if (sign8(--ancilla_K[k])) {
      ancilla_K[k] = 32;
      ancilla_z_vel[k] = -ancilla_z_vel[k];
    }
    Ancilla_MoveZ(k);
  }
  BYTE(link_z_coord) = ancilla_z[k];
}

void GameOverText_Draw() {  // 88f5c4
  static const uint8 kGameOverText_Chars[16] = {0x40, 0x50, 0x41, 0x51, 0x42, 0x52, 0x43, 0x53, 0x44, 0x54, 0x45, 0x55, 0x43, 0x53, 0x46, 0x56};
  oam_cur_ptr = 0x800;
  oam_ext_cur_ptr = 0xa20;
  OamEnt *oam = GetOamCurPtr();
  int k = flag_for_boomerang_in_place;
  do {
    Ancilla_SetOam(oam + 0, Ancilla_GetX(k), 0x57, kGameOverText_Chars[k * 2 + 0], 0x3c, 0);
    Ancilla_SetOam(oam + 1, Ancilla_GetX(k), 0x5f, kGameOverText_Chars[k * 2 + 1], 0x3c, 0);
  } while (oam += 2, --k >= 0);
}

int AncillaAdd_AddAncilla_Bank08(uint8 type, uint8 y) {  // 88f631
  int k = Ancilla_AllocInit(type, y);
  if (k >= 0) {
    ancilla_type[k] = type;
    ancilla_numspr[k] = kAncilla_Pflags[type];
    ancilla_floor[k] = link_is_on_lower_level;
    ancilla_floor2[k] = link_is_on_lower_level_mirror;
    ancilla_y_vel[k] = 0;
    ancilla_x_vel[k] = 0;
    ancilla_objprio[k] = 0;
    ancilla_U[k] = 0;
  }
  return k;
}

void Ancilla_PrepOamCoord(int k, Point16U *info) {  // 88f671
  oam_priority_value = kTagalongLayerBits[ancilla_floor[k]] << 8;
  info->x = Ancilla_GetX(k) - BG2HOFS_copy2;
  info->y = Ancilla_GetY(k) - BG2VOFS_copy2;
}

void Ancilla_PrepAdjustedOamCoord(int k, Point16U *info) {  // 88f6a4
  oam_priority_value = kTagalongLayerBits[ancilla_floor[k]] << 8;
  info->x = Ancilla_GetX(k) - BG2HOFS_copy;
  info->y = Ancilla_GetY(k) - BG2VOFS_copy;
}

bool Ancilla_CheckLinkCollision(int k, int j, CheckPlayerCollOut *out) {  // 88f76b
  static const int16 kAncilla_Coll_Yoffs[5] = {0, 8, 8, 8, 0};
  static const int16 kAncilla_Coll_Xoffs[5] = {0, 8, 8, 8, 0};
  static const int16 kAncilla_Coll_H[5] = {20, 20, 8, 28, 14};
  static const int16 kAncilla_Coll_W[5] = {20, 3, 8, 24, 14};
  static const int16 kAncilla_Coll_LinkYoffs[5] = {12, 12, 12, 12, 12};
  static const int16 kAncilla_Coll_LinkXoffs[5] = {8, 8, 8, 12, 8};
  uint16 x = Ancilla_GetX(k), y = Ancilla_GetY(k);
  y += kAncilla_Coll_Yoffs[j] + (int8)ancilla_z[k];
  x += kAncilla_Coll_Xoffs[j];
  out->r4 = link_y_coord + kAncilla_Coll_LinkYoffs[j] - y;
  out->r8 = abs16(out->r4);
  out->r6 = link_x_coord + kAncilla_Coll_LinkXoffs[j] - x;
  out->r10 = abs16(out->r6);
  return out->r8 < kAncilla_Coll_H[j] && out->r10 < kAncilla_Coll_W[j];
}

bool Hookshot_CheckProximityToLink(int x, int y) {  // 88f7dc
  return abs16(link_y_coord - BG2VOFS_copy2 + 12 - y - 4) < 12 &&
         abs16(link_x_coord - BG2HOFS_copy2 +  8 - x - 4) < 12;
}

bool Ancilla_CheckForEntranceTrigger(int what) {  // 88f844
  static const uint16 kEntranceTrigger_BaseY[4] = {0xd40, 0x210, 0xcfc, 0x100};
  static const uint16 kEntranceTrigger_BaseX[4] = {0xd80, 0xe68, 0x130, 0xf10};
  static const uint8 kEntranceTrigger_SizeY[4] = {11, 32, 16, 12};
  static const uint8 kEntranceTrigger_SizeX[4] = {16, 16, 16, 16};
  return
    abs16(link_y_coord + 12 - kEntranceTrigger_BaseY[what]) < kEntranceTrigger_SizeY[what] &&
    abs16(link_x_coord + 8 - kEntranceTrigger_BaseX[what]) < kEntranceTrigger_SizeX[what];
}

void AncillaDraw_Shadow(OamEnt *oam, int k, int x, int y, uint8 pal) {  // 88f897
  static const uint8 kAncilla_DrawShadow_Char[14] = {0x6c, 0x6c, 0x28, 0x28, 0x38, 0xff, 0xc8, 0xc8, 0xd8, 0xd8, 0xd9, 0xd9, 0xda, 0xda};
  static const uint8 kAncilla_DrawShadow_Flags[14] = {0x28, 0x68, 0x28, 0x68, 0x28, 0xff, 0x22, 0x22, 0x24, 0x64, 0x24, 0x64, 0x24, 0x64};

  if (k == 2)
    x += 4;
  Ancilla_SetOam_Safe(oam, x, y,
                      kAncilla_DrawShadow_Char[k * 2],
                      kAncilla_DrawShadow_Flags[k * 2] & ~0x30 | pal, 0);
  uint8 ch = kAncilla_DrawShadow_Char[k * 2 + 1];
  if (ch != 0xff) {
    x += 8;
    Ancilla_SetOam_Safe(oam + 1, x, y, ch, kAncilla_DrawShadow_Flags[k * 2 + 1] & ~0x30 | pal, 0);
  }
}

void Ancilla_AllocateOamFromRegion_B_or_E(uint8 size) {  // 88f90a
  if (!sort_sprites_setting)
    Oam_AllocateFromRegionB(size);
  else
    Oam_AllocateFromRegionE(size);
}

OamEnt *Ancilla_AllocateOamFromCustomRegion(OamEnt *oam) {  // 88f9ba
  int a = (uint8 *)oam - g_ram;
  if (sort_sprites_setting) {
    if (a < 0x900) {
      if (a < 0x8e0)
        return oam;
      a = 0x820;
    } else {
      if (a < 0x9d0)
        return oam;
      a = 0x940;
    }
  } else {
    if (a < 0x990)
      return oam;
    a = 0x820;
  }
  oam_cur_ptr = a;
  oam_ext_cur_ptr = ((a - 0x800) >> 2) + 0xa20;
  return GetOamCurPtr();
}

OamEnt *HitStars_UpdateOamBufferPosition(OamEnt *oam) {  // 88fa00
  int a = (uint8 *)oam - g_ram;
  if (!sort_sprites_setting && a >= 0x9d0) {
    oam_cur_ptr = 0x820;
    oam_ext_cur_ptr = 0xa20 + (0x20 >> 2);
    oam = GetOamCurPtr();
  }
  return oam;
}

bool Hookshot_ShouldIEvenBotherWithTiles(int k) {  // 88fa2d
  uint16 x = Ancilla_GetX(k), y = Ancilla_GetY(k);
  if (!player_is_indoors) {
    if (!(ancilla_dir[k] & 2)) {
      uint16 t = y - kOverworld_OffsetBaseY[BYTE(current_area_of_player) >> 1];
      return (t < 4) || (t >= overworld_right_bottom_bound_for_scroll);
    } else {
      uint16 t = x - kOverworld_OffsetBaseX[BYTE(current_area_of_player) >> 1];
      return (t < 6) || (t >= overworld_right_bottom_bound_for_scroll);
    }
  }
  if (!(ancilla_dir[k] & 2)) {
    return (y & 0x1ff) < 4 || (y & 0x1ff) >= 0x1e8 || (y & 0x200) != (link_y_coord & 0x200);
  } else {
    return (x & 0x1ff) < 4 || (x & 0x1ff) >= 0x1f0 || (x & 0x200) != (link_x_coord & 0x200);
  }
}

AncillaRadialProjection Ancilla_GetRadialProjection(uint8 a, uint8 r8) {  // 88fadd
  static const uint8 kRadialProjection_Tab0[64] = {
    255, 254, 251, 244, 236, 225, 212, 197, 181, 162, 142, 120,  97,  74,  49,  25,
      0,  25,  49,  74,  97, 120, 142, 162, 181, 197, 212, 225, 236, 244, 251, 254,
    255, 254, 251, 244, 236, 225, 212, 197, 181, 162, 142, 120,  97,  74,  49,  25,
      0,  25,  49,  74,  97, 120, 142, 162, 181, 197, 212, 225, 236, 244, 251, 254,
  };
  static const uint8 kRadialProjection_Tab2[64] = {
      0,  25,  49,  74,  97, 120, 142, 162, 181, 197, 212, 225, 236, 244, 251, 254,
    255, 254, 251, 244, 236, 225, 212, 197, 181, 162, 142, 120,  97,  74,  49,  25,
      0,  25,  49,  74,  97, 120, 142, 162, 181, 197, 212, 225, 236, 244, 251, 254,
    255, 254, 251, 244, 236, 225, 212, 197, 181, 162, 142, 120,  97,  74,  49,  25,
  };
  static const uint8 kRadialProjection_Tab1[64] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  };
  static const uint8 kRadialProjection_Tab3[64] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  };
  AncillaRadialProjection rv;
  int p0 = kRadialProjection_Tab0[a] * r8;
  int p1 = kRadialProjection_Tab2[a] * r8;
  rv.r0 = (p0 >> 8) + (p0 >> 7 & 1);
  rv.r2 = kRadialProjection_Tab1[a];
  rv.r4 = (p1 >> 8) + (p1 >> 7 & 1);
  rv.r6 = kRadialProjection_Tab3[a];
  return rv;
}

int Ancilla_AllocateOamFromRegion_A_or_D_or_F(int k, uint8 size) {  // 88fb2b
  if (sort_sprites_setting) {
    if (ancilla_floor[k])
      return Oam_AllocateFromRegionF(size);
    else
      return Oam_AllocateFromRegionD(size);
  } else {
    return Oam_AllocateFromRegionA(size);
  }
}

void Ancilla_AddHitStars(uint8 a, uint8 y) {  // 898024
  static const int8 kShovelHitStars_XY[12] = {21, -11, 21, 11, 3, -6, 21, 5, 16, -14, 16, 14};
  static const int8 kShovelHitStars_X2[6] = {-3, 19, 2, 13, -6, 22};
  int k = Ancilla_AddAncilla(a, y);
  if (k >= 0) {
    ancilla_item_to_link[k] = 0;
    ancilla_aux_timer[k] = 2;
    ancilla_arr3[k] = 1;
    ancilla_y_vel[k] = 0;
    ancilla_x_vel[k] = 0;
    int j = a;
    if (link_item_in_hand) {
      j = (link_direction_facing >> 1) + 2;
    } else if (link_position_mode) {
      j = link_direction_facing != 4 ? 1 : 0;
    }
    ancilla_step[k] = j;
    int t = link_x_coord + kShovelHitStars_X2[j];
    ancilla_A[k] = t;
    ancilla_B[k] = t >> 8;
    Ancilla_SetXY(k, link_x_coord + kShovelHitStars_XY[j * 2 + 1], link_y_coord + kShovelHitStars_XY[j * 2 + 0]);
  }
}

void AncillaAdd_Blanket(uint8 a) {  // 898091
  int k = 0;
  ancilla_type[k] = a;
  ancilla_numspr[k] = kAncilla_Pflags[a];
  ancilla_floor[k] = link_is_on_lower_level;
  ancilla_floor2[k] = link_is_on_lower_level_mirror;
  ancilla_objprio[k] = 0;
  Ancilla_SetXY(k, 0x938, 0x2162);
}

void AncillaAdd_Snoring(uint8 a, uint8 y) {  // 8980c8
  int k = Ancilla_AddAncilla(a, y);
  if (k >= 0) {
    ancilla_item_to_link[k] = 0;
    ancilla_y_vel[k] = -8;
    ancilla_aux_timer[k] = 7;
    ancilla_x_vel[k] = 8;
    ancilla_step[k] = 255;
    Ancilla_SetXY(k, link_x_coord + 16, link_y_coord + 4);
  }
}

void AncillaAdd_Bomb(uint8 a, uint8 y) {  // 89811f
  static const int8 kBomb_Place_X0[4] = {8, 8, 0, 16};
  static const int8 kBomb_Place_Y0[4] = {0, 24, 12, 12};
  static const int8 kBomb_Place_X1[4] = {8, 8, -6, 22};
  static const int8 kBomb_Place_Y1[4] = {4, 28, 12, 12};

  int k = Ancilla_AddAncilla(a, y);
  if (k < 0)
    return;
  if (link_item_bombs == 0) {
    ancilla_type[k] = 0;
    return;
  }

  if (--link_item_bombs == 0)
    Hud_RefreshIcon();

  ancilla_R[k] = 0;
  ancilla_step[k] = 0;
  ancilla_item_to_link[k] = 0;
  ancilla_L[k] = 0;
  ancilla_arr3[k] = kBomb_Tab0[0];

  // These are not used directly by bombs, but used by door debris
  ancilla_arr25[k] = 0;
  ancilla_arr26[k] = 7;

  ancilla_z[k] = 0;
  ancilla_timer[k] = 8;
  ancilla_dir[k] = link_direction_facing >> 1;
  ancilla_T[k] = 0;
  ancilla_arr23[k] = 0;
  ancilla_arr22[k] = 0;
  if (Ancilla_CheckInitialTileCollision_Class2(k)) {
    int j = link_direction_facing >> 1;
    Ancilla_SetXY(k, link_x_coord + kBomb_Place_X0[j], link_y_coord + kBomb_Place_Y0[j]);
  } else {
    int j = link_direction_facing >> 1;
    Ancilla_SetXY(k, link_x_coord + kBomb_Place_X1[j], link_y_coord + kBomb_Place_Y1[j]);
  }
  sound_effect_1 = Link_CalculateSfxPan() | 0xb;
}

uint8 AncillaAdd_Boomerang(uint8 a, uint8 y) {  // 89820f
  static const uint8 kBoomerang_Tab0[4] = {0x20, 0x18, 0x30, 0x28};
  static const uint8 kBoomerang_Tab1[2] = {0x20, 0x60};
  static const uint8 kBoomerang_Tab2[2] = {3, 2};
  static const uint8 kBoomerang_Tab3[4] = {8, 4, 2, 1};
  static const uint8 kBoomerang_Tab4[8] = {8, 4, 2, 1, 9, 5, 10, 6};
  static const uint8 kBoomerang_Tab5[8] = {2, 3, 3, 2, 2, 3, 3, 3};
  static const int8 kBoomerang_Tab6[8] = {-10, -8, -9, -9, -10, -8, -9, -9};
  static const int8 kBoomerang_Tab7[8] = {-10, 11, 8, -8, -10, 11, 8, -8};
  static const int8 kBoomerang_Tab8[8] = {-16, 6, 0, 0, -8, 8, -8, 8};
  static const int8 kBoomerang_Tab9[8] = {0, 0, -8, 8, 8, 8, -8, -8};

  int k = Ancilla_AddAncilla(a, y);
  if (k < 0)
    return 0;
  ancilla_aux_timer[k] = 0;
  ancilla_item_to_link[k] = 0;
  ancilla_K[k] = 0;
  ancilla_z[k] = 0;
  ancilla_L[k] = ancilla_numspr[k];
  flag_for_boomerang_in_place = 1;
  int j = link_item_boomerang - 1;
  ancilla_G[k] = j;
  ancilla_step[k] = kBoomerang_Tab1[j];
  ancilla_arr3[k] = kBoomerang_Tab2[j];

  int s = ancilla_G[k] * 2 + ((joypad1H_last & 0xc) && (joypad1H_last & 3) ? 1 : 0);
  uint8 r0 = kBoomerang_Tab0[s];
  ancilla_H[k] = r0;

  uint8 r1 = (joypad1H_last & 0xf) ? (joypad1H_last & 0xf) : kBoomerang_Tab3[link_direction_facing >> 1];
  hookshot_effect_index = 0;

  if (r1 & 0xc) {
    ancilla_y_vel[k] = r1 & 8 ? -r0 : r0;
    int i = sign8(ancilla_y_vel[k]) ? 0 : 1;
    ancilla_dir[k] = i;
    hookshot_effect_index = kBoomerang_Tab3[i];
  }
  ancilla_S[k] = 0;

  if (r1 & 3) {
    if (!(r1 & 2))
      ancilla_S[k] = 1;
    ancilla_x_vel[k] = (r1 & 2) ? -r0 : r0;
    int i = sign8(ancilla_x_vel[k]) ? 2 : 3;
    ancilla_dir[k] = i;
    hookshot_effect_index |= kBoomerang_Tab3[i];
  }

  j = FindInByteArray(kBoomerang_Tab4, r1, 8);
  if (j < 0)
    j = 0;
  ancilla_arr1[k] = kBoomerang_Tab5[j];
  ancilla_arr23[k] = j << 1;
  if (button_b_frames >= 9) {
    ancilla_aux_timer[k]++;
  } else {
    if (s || !(joypad1H_last & 0xf))
      j = link_direction_facing >> 1;
  }
  s = Ancilla_CheckInitialTile_A(k);
  if (s < 0) {
    if (ancilla_aux_timer[k]) {
      Ancilla_SetXY(k, link_x_coord + kBoomerang_Tab9[j], link_y_coord + 8 + kBoomerang_Tab8[j]);
    } else {
      Ancilla_SetXY(k, link_x_coord + kBoomerang_Tab7[j], link_y_coord + 8 + kBoomerang_Tab6[j]);
    }
  } else {
    ancilla_type[k] = 0;
    flag_for_boomerang_in_place = 0;
    if (ancilla_tile_attr[k] != 0xf0) {
      sound_effect_1 = Ancilla_CalculateSfxPan(k) | 5;
    } else {
      sound_effect_1 = Ancilla_CalculateSfxPan(k) | 6;
    }
    AncillaAdd_BoomerangWallClink(k);
  }
  return s;
}

void AncillaAdd_TossedPondItem(uint8 a, uint8 xin, uint8 yin) {  // 898a32
  static const uint8 kWishPondItem_X[76] = {
    4, 4, 4, 4,  4, 0, 0, 4, 4, 4, 4, 4, 5, 0, 0, 0,
    0, 0, 0, 4,  0, 4, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 11, 0, 0, 0, 2, 0, 5, 0, 0, 0, 0, 0,
    0, 0, 0, 0,  4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 4, 4,  0, 4, 0, 0, 0, 4, 0, 0,
  };
  static const int8 kWishPondItem_Y[76] = {
    -13, -13, -13, -13, -13, -12, -12, -13, -13, -12, -12, -12, -10, -12, -12, -12,
    -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12,
    -12, -12, -12, -13, -12, -12, -12, -12, -12, -12, -10, -12, -12, -12, -12, -12,
    -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12,
    -12, -12, -12, -12, -12, -12, -12, -12, -12, -13, -12, -12,
  };

  link_receiveitem_index = xin;
  int k = Ancilla_AddAncilla(a, yin);
  if (k >= 0) {
    sound_effect_2 = Link_CalculateSfxPan() | 0x13;
    uint8 sb = kReceiveItemGfx[xin];

    if (sb != 0xff) {
      if (sb == 0x20)
        DecompressShieldGraphics();
      DecodeAnimatedSpriteTile_variable(sb);
    } else {
      DecodeAnimatedSpriteTile_variable(0);
    }
    if (sb == 6)
      DecompressSwordGraphics();

    link_state_bits = 0x80;
    link_picking_throw_state = 0;
    link_direction_facing = 0;
    link_animation_steps = 0;
    ancilla_z_vel[k] = 20;
    ancilla_y_vel[k] = -40;
    ancilla_x_vel[k] = 0;
    ancilla_z[k] = 0;
    ancilla_timer[k] = 16;
    ancilla_item_to_link[k] = link_receiveitem_index;
    Ancilla_SetXY(k,
      link_x_coord + kWishPondItem_X[link_receiveitem_index],
      link_y_coord + kWishPondItem_Y[link_receiveitem_index]);
  }
}

void AddHappinessPondRupees(uint8 arg) {  // 898ae0
  int k = Ancilla_AddAncilla(0x42, 9);
  if (k < 0)
    return;
  sound_effect_2 = Link_CalculateSfxPan() | 0x13;
  uint8 sb = kReceiveItemGfx[0x35];
  DecodeAnimatedSpriteTile_variable(sb);
  link_state_bits = 0x80;
  link_picking_throw_state = 0;
  link_direction_facing = 0;
  link_animation_steps = 0;

  memset(happiness_pond_arr1, 0, 10);

  static const int8 kHappinessPond_Start[4] = {0, 4, 4, 9};
  static const int8 kHappinessPond_End[4] = {-1, 0, -1, -1};
  static const int8 kHappinessPond_Xvel[10] = {0, -12, -6, 6, 12, -9, -5, 0, 5, 9};
  static const int8 kHappinessPond_Yvel[10] = {-40, -40, -40, -40, -40, -32, -32, -32, -32, -32};
  static const int8 kHappinessPond_Zvel[10] = {20, 20, 20, 20, 20, 16, 16, 16, 16, 16};

  int j = kHappinessPond_Start[arg], j_end = kHappinessPond_End[arg];
  k = 9;
  do {
    happiness_pond_arr1[k] = 1;
    happiness_pond_z_vel[k] = kHappinessPond_Zvel[j];
    happiness_pond_y_vel[k] = kHappinessPond_Yvel[j];
    happiness_pond_x_vel[k] = kHappinessPond_Xvel[j];
    happiness_pond_z[k] = 0;
    happiness_pond_step[k] = 0;
    happiness_pond_timer[k] = 16;
    happiness_pond_item_to_link[k] = 53;
    int x = link_x_coord + 4;
    int y = link_y_coord - 12;
    happiness_pond_x_lo[k] = x;
    happiness_pond_x_hi[k] = x >> 8;
    happiness_pond_y_lo[k] = y;
    happiness_pond_y_hi[k] = y >> 8;
  } while (--k, --j != j_end);
}

int AncillaAdd_FallingPrize(uint8 a, uint8 item_idx, uint8 yv) {  // 898bc1
  static const int8 kFallingItem_Type[7] = {0x10, 0x37, 0x39, 0x38, 0x26, 0xf, 0x20};
  static const int8 kFallingItem_G[7] = {0x40, 0, 0, 0, 0, -1, 0};
  static const int16 kFallingItem_X[7] = {0x78, 0x78, 0x78, 0x78, 0x78, 0x80, 0x78};
  static const int16 kFallingItem_Y[7] = {0x48, 0x78, 0x78, 0x78, 0x78, 0x68, 0x78};
  static const uint8 kFallingItem_Z[7] = {0x60, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80};
  link_receiveitem_index = item_idx;
  int k = Ancilla_AddAncilla(a, yv);
  if (k < 0)
    return k;
  uint8 item_type = kFallingItem_Type[item_idx];
  ancilla_item_to_link[k] = item_type;
  if (item_type == 0x10 || item_type == 0xf)
    DecodeAnimatedSpriteTile_variable(kReceiveItemGfx[item_type]);

  ancilla_z_vel[k] = -48;
  ancilla_y_vel[k] = 0;
  ancilla_x_vel[k] = 0;
  ancilla_step[k] = 0;
  ancilla_z[k] = kFallingItem_Z[item_idx];
  ancilla_aux_timer[k] = 9;
  ancilla_arr3[k] = 0;
  ancilla_L[k] = 0;
  ancilla_G[k] = kFallingItem_G[item_idx];
  link_receiveitem_index = item_type;

  int x, y;
  if (item_idx != 0 && item_idx != 5) {
    if (BYTE(cur_palace_index_x2) == 20) {
      x = (link_x_coord & 0xff00) | 0x100;
      y = (link_y_coord & 0xff00) | 0x100;
    } else {
      x = kFallingItem_X[item_idx] + BG2HOFS_copy2;
      y = kFallingItem_Y[item_idx] + BG2VOFS_copy2;
    }
  } else {
    x = link_x_coord;
    y = kFallingItem_Y[item_idx] + BG2VOFS_copy2;
  }
  Ancilla_SetXY(k, x, y);
  return k;
}

void AncillaAdd_ChargedSpinAttackSparkle() {  // 898cb1
  for (int k = 9; k >= 0; k--) {
    if (ancilla_type[k] == 0 || ancilla_type[k] == 0x3c) {
      ancilla_type[k] = 13;
      ancilla_floor[k] = link_is_on_lower_level;
      ancilla_timer[k] = 6;
      break;
    }
  }
}

void AncillaAdd_ExplodingWeatherVane(uint8 a, uint8 y) {  // 898d11
  static const int8 kWeathervane_Tab4[12] = {8, 10, 9, 4, 11, 12, -10, -8, 4, -6, -10, -4};
  static const int8 kWeathervane_Tab5[12] = {20, 22, 20, 20, 22, 20, 20, 22, 20, 22, 20, 20};
  static const uint8 kWeathervane_Tab6[12] = {0xb0, 0xa3, 0xa0, 0xa2, 0xa0, 0xa8, 0xa0, 0xa0, 0xa8, 0xa1, 0xb0, 0xa0};
  static const uint8 kWeathervane_Tab8[12] = {0, 2, 4, 6, 3, 8, 14, 8, 12, 7, 10, 8};
  static const uint8 kWeathervane_Tab10[12] = {48, 18, 32, 20, 22, 24, 32, 20, 24, 22, 20, 32};

  int k = Ancilla_AddAncilla(a, y);
  if (k < 0)
    return;

  ancilla_aux_timer[k] = 10;
  ancilla_G[k] = 128;
  ancilla_step[k] = 0;
  ancilla_arr3[k] = 0;
  sound_effect_1 = 0;
  music_control = 0xf2;
  sound_effect_ambient = 0x17;

  weathervane_var1 = 0;
  weathervane_var2 = 0x280;

  for (int i = 11; i >= 0; i--) {
    weathervane_arr3[i] = 0;
    weathervane_arr4[i] = kWeathervane_Tab4[i];
    weathervane_arr5[i] = kWeathervane_Tab5[i];
    weathervane_arr6[i] = kWeathervane_Tab6[i];
    weathervane_arr7[i] = 7;
    weathervane_arr8[i] = kWeathervane_Tab8[i];
    weathervane_arr9[i] = 2;
    weathervane_arr10[i] = kWeathervane_Tab10[i];
    weathervane_arr11[i] = 1;
    weathervane_arr12[i] = i & 1;
  }
}

void AncillaAdd_CutsceneDuck(uint8 a, uint8 y) {  // 898d90
  if (AncillaAdd_CheckForPresence(a))
    return;
  int k = Ancilla_AddAncilla(a, y);
  if (k >= 0) {
    ancilla_dir[k] = 2;
    ancilla_arr3[k] = 3;
    ancilla_step[k] = 0;
    ancilla_aux_timer[k] = 32;
    ancilla_item_to_link[k] = 116;
    ancilla_z_vel[k] = 0;
    ancilla_L[k] = 0;
    ancilla_z[k] = 0;
    ancilla_S[k] = 0;
    Ancilla_SetXY(k, 0x200, 0x788);
  }
}

void AncillaAdd_SomariaPlatformPoof(int k) {  // 898dd2
  ancilla_type[k] = 0x39;
  ancilla_aux_timer[k] = 7;
  for (int j = 15; j >= 0; j--) {
    if (sprite_type[j] == 0xed) {
      sprite_state[j] = 0;
      player_on_somaria_platform = 0;
    }
  }
  Player_TileDetectNearby();
}

int AncillaAdd_SuperBombExplosion(uint8 a, uint8 y) {  // 898df9
  int k = Ancilla_AddAncilla(a, y);
  if (k >= 0) {
    ancilla_R[k] = 0;
    ancilla_step[k] = 0;
    ancilla_arr25[k] = 0;
    ancilla_L[k] = 0;
    ancilla_arr3[k] = kBomb_Tab0[1];
    ancilla_item_to_link[k] = 1;
    int j = WORD(tagalong_var2);
    int y = tagalong_y_lo[j] | tagalong_y_hi[j] << 8;
    int x = tagalong_x_lo[j] | tagalong_x_hi[j] << 8;
    Ancilla_SetXY(k, x + 8, y + 16);
  }
  return k;
}

void ConfigureRevivalAncillae() {  // 898e4e
  link_dma_var5 = 80;
  int k = 0;

  ancilla_arr3[k] = 64;
  ancilla_step[k] = 0;
  ancilla_z_vel[k] = 8;
  ancilla_L[k] = 0;
  ancilla_G[k] = 5;
  ancilla_item_to_link[k] = 0;
  ancilla_K[k] = 0;
  Ancilla_SetXY(k, link_x_coord, link_y_coord);
  ancilla_z[k] = 0;
  k += 1;

  ancilla_z[k] = 0;
  ancilla_arr3[k] = 240;
  ancilla_step[k] = 0;
  ancilla_K[k] = 0;
  k += 1;

  ancilla_item_to_link[k] = 2;
  ancilla_aux_timer[k] = 3;
  ancilla_arr3[k] = 8;
  ancilla_step[k] = 0;
  ancilla_dir[k] = 3;
  ancilla_arr25[k] = kMagicPowder_Tab0[30 + ancilla_item_to_link[k]];

  Ancilla_SetXY(k, link_x_coord + 20, link_y_coord + 2);
}

void AncillaAdd_LampFlame(uint8 a, uint8 y) {  // 898f1c
  static const int8 kLampFlame_X[4] = {0, 0, -20, 18};
  static const int8 kLampFlame_Y[4] = {-16, 24, 4, 4};
  int k = Ancilla_AddAncilla(a, y);
  if (k >= 0) {
    ancilla_item_to_link[k] = 0;
    ancilla_aux_timer[k] = 0;
    ancilla_timer[k] = 23;
    int j = link_direction_facing >> 1;
    ancilla_dir[k] = j;
    Ancilla_SetXY(k, link_x_coord + kLampFlame_X[j], link_y_coord + kLampFlame_Y[j]);
    sound_effect_1 = Ancilla_CalculateSfxPan(k) | 42;
  }
}

void AncillaAdd_MSCutscene(uint8 a, uint8 y) {  // 898f7c
  int k = Ancilla_AddAncilla(a, y);
  if (k >= 0) {
    ancilla_item_to_link[k] = 0;
    ancilla_aux_timer[k] = 2;
    ancilla_timer[k] = 64;
    Ancilla_SetXY(k, link_x_coord + 8, link_y_coord - 8);
  }
}

void AncillaAdd_DashDust(uint8 a, uint8 y) {  // 898fba
  AddDashingDustEx(a, y, 1);
}

void AncillaAdd_DashDust_charging(uint8 a, uint8 y) {  // 898fc1
  AddDashingDustEx(a, y, 0);
}

void AncillaAdd_BlastWallFireball(uint8 a, uint8 y, int r4) {  // 899031
  static const int8 kBlastWall_XY[32] = {
    -64, 0, -22,  42, -38,  38, -42,  22, 0,  64,  22,  42,  38,  38,  42,  22,
    64, 0,  22, -42,  38, -38,  42, -22, 0, -64, -22, -42, -38, -38, -42, -22,
  };
  for (int k = 10; k != 4; k--) {
    if (ancilla_type[k] == 0) {
      ancilla_type[k] = 0x32;
      ancilla_floor[k] = link_is_on_lower_level;
      blastwall_var12[k] = 16;
      int j = frame_counter & 15;
      ancilla_y_vel[k] = kBlastWall_XY[j * 2 + 0];
      ancilla_x_vel[k] = kBlastWall_XY[j * 2 + 1];
      Ancilla_SetXY(k, blastwall_var11[r4] + 16, blastwall_var10[r4] + 8);
      return;
    }
  }

}

int AncillaAdd_Arrow(uint8 a, uint8 ax, uint8 ay, uint16 xcoord, uint16 ycoord) {  // 8990a4
  static const int8 kShootBow_X[4] = {4, 4, 0, 4};
  static const int8 kShootBow_Y[4] = {-4, 3, 4, 4};
  static const int8 kShootBow_Xvel[4] = {0, 0, -48, 48};
  static const int8 kShootBow_Yvel[4] = {-48, 48, 0, 0};

  scratch_0 = ycoord;
  scratch_1 = xcoord;
  BYTE(index_of_interacting_tile) = ax;

  if (AncillaAdd_CheckForPresence(a))
    return -1;

  int k = AncillaAdd_ArrowFindSlot(a, ay);

  if (k >= 0) {
    sound_effect_1 = Link_CalculateSfxPan() | 7;
    ancilla_H[k] = 0;
    ancilla_item_to_link[k] = 8;
    int j = ax >> 1;
    ancilla_dir[k] = j | 4;
    ancilla_y_vel[k] = kShootBow_Yvel[j];
    ancilla_x_vel[k] = kShootBow_Xvel[j];
    Ancilla_SetXY(k, xcoord + kShootBow_X[j], ycoord + 8 +  kShootBow_Y[j]);
  }
  return k;
}

void AncillaAdd_BunnyPoof(uint8 a, uint8 y) {  // 899102
  int k = Ancilla_AddAncilla(a, y);
  if (k >= 0) {
    link_visibility_status = 0xc;
    ancilla_step[k] = 0;
    if (!link_is_bunny_mirror)
      sound_effect_1 = Link_CalculateSfxPan() | 0x14;
    else
      sound_effect_1 = Link_CalculateSfxPan() | 0x15;

    ancilla_item_to_link[k] = 0;
    ancilla_aux_timer[k] = 7;
    Ancilla_SetXY(k, link_x_coord, link_y_coord + 4);
  }
}

void AncillaAdd_CapePoof(uint8 a, uint8 y) {  // 89912c
  int k = Ancilla_AddAncilla(a, y);
  if (k >= 0) {
    ancilla_step[k] = 1;
    link_is_transforming = 1;
    link_cant_change_direction |= 1;
    link_direction = 0;
    link_direction_last = 0;
    ancilla_item_to_link[k] = 0;
    ancilla_aux_timer[k] = 7;
    Ancilla_SetXY(k, link_x_coord, link_y_coord + 4);
  }
}

void AncillaAdd_DwarfPoof(uint8 ain, uint8 yin) {  // 89915f
  int k = Ancilla_AddAncilla(ain, yin);
  if (k < 0)
    return;
  if (follower_indicator == 8)
    sound_effect_1 = Link_CalculateSfxPan() | 0x14;
  else
    sound_effect_1 = Link_CalculateSfxPan() | 0x15;

  ancilla_item_to_link[k] = 0;
  ancilla_step[k] = 0;
  ancilla_aux_timer[k] = 7;
  tagalong_var5 = 1;
  int j = tagalong_var2;
  int x = tagalong_x_lo[j] | tagalong_x_hi[j] << 8;
  int y = tagalong_y_lo[j] | tagalong_y_hi[j] << 8;
  Ancilla_SetXY(k, x, y + 4);
}

void AncillaAdd_BushPoof(uint16 x, uint16 y) {  // 8991c3
  if (!(link_item_in_hand & 0x40))
    return;
  int k = Ancilla_AddAncilla(0x3f, 4);
  if (k >= 0) {
    ancilla_item_to_link[k] = 0;
    ancilla_timer[k] = 7;
    sound_effect_1 = Link_CalculateSfxPan() | 21;
    Ancilla_SetXY(k, x, y - 2);
  }
}

void AncillaAdd_EtherSpell(uint8 a, uint8 y) {  // 8991fc
  int k = Ancilla_AddAncilla(a, y);
  if (k >= 0) {
    ancilla_item_to_link[k] = 0;
    ancilla_arr25[k] = 0;
    ancilla_step[k] = 0;
    flag_custom_spell_anim_active = 1;
    ancilla_aux_timer[k] = 2;
    ancilla_arr3[k] = 3;
    ancilla_y_vel[k] = 127;
    ether_var2 = 40;
    load_chr_halfslot_even_odd = 9;
    ether_var1 = 0x40;
    sound_effect_2 = Link_CalculateSfxPan() | 0x26;
    for(int i = 0; i < 8; i++)
      ether_arr1[i] = i * 8;
    ether_y = link_y_coord;
    uint16 y = BG2VOFS_copy2 - 16;
    ether_y_adjusted = y & 0xf0;
    ether_x = link_x_coord;
    ether_x2 = ether_x + 8;
    ether_y2 = link_y_coord - 16;
    ether_y3 = ether_y2 + 0x24;
    Ancilla_SetXY(k, link_x_coord, y);
  }
}

void AncillaAdd_VictorySpin() {  // 8992ac
  if ((link_sword_type + 1 & 0xfe) != 0) {
    int k = Ancilla_AddAncilla(0x3b, 0);
    if (k >= 0) {
      ancilla_item_to_link[k] = 0;
      ancilla_arr3[k] = 1;
      ancilla_aux_timer[k] = 34;
    }
  }
}

void AncillaAdd_MagicPowder(uint8 a, uint8 y) {  // 8992f0
  static const int8 kMagicPower_X[4] = {-2, -2, -12, 12};
  static const int8 kMagicPower_Y[4] = {0, 20, 16, 16};
  static const int8 kMagicPower_X1[4] = {10, 10, -8, 28};
  static const int8 kMagicPower_Y1[4] = {1, 40, 22, 22};

  int k = Ancilla_AddAncilla(a, y);
  if (k >= 0) {
    ancilla_item_to_link[k] = 0;
    ancilla_z[k] = 0;
    ancilla_aux_timer[k] = 1;
    link_dma_var5 = 80;
    int j = link_direction_facing >> 1;
    ancilla_dir[k] = j;
    ancilla_arr25[k] = kMagicPowder_Tab0[j * 10];
    Ancilla_SetXY(k, link_x_coord + kMagicPower_X[j], link_y_coord + kMagicPower_Y[j]);
    Ancilla_CheckTileCollision(k);
    byte_7E0333 = ancilla_tile_attr[k];
    if (current_item_active == 9) {
      ancilla_type[k] = 0;
      return;
    }
    sound_effect_1 = Link_CalculateSfxPan() | 0xd;
    Ancilla_SetXY(k, link_x_coord + kMagicPower_X1[j], link_y_coord + kMagicPower_Y1[j]);
  }
}

void AncillaAdd_WallTapSpark(uint8 a, uint8 y) {  // 899395
  static const int8 kWallTapSpark_X[4] = {11, 10, -12, 29};
  static const int8 kWallTapSpark_Y[4] = {-4, 32, 17, 17};
  int k = Ancilla_AddAncilla(a, y);
  if (k >= 0) {
    ancilla_item_to_link[k] = 5;
    ancilla_aux_timer[k] = 1;
    int i = link_direction_facing >> 1;
    Ancilla_SetXY(k, link_x_coord + kWallTapSpark_X[i], link_y_coord + kWallTapSpark_Y[i]);
  }
}

void AncillaAdd_SwordSwingSparkle(uint8 a, uint8 y) {  // 8993c2
  int k = Ancilla_AddAncilla(a, y);
  if (k >= 0) {
    ancilla_item_to_link[k] = 0;
    ancilla_aux_timer[k] = 1;
    ancilla_dir[k] = link_direction_facing >> 1;
    Ancilla_SetXY(k, link_x_coord, link_y_coord);
  }
}

void AncillaAdd_DashTremor(uint8 a, uint8 y) {  // 8993f3
  static const uint8 kAddDashTremor_Dir[4] = {2, 2, 0, 0};
  static const uint8 kAddDashTremor_Tab[2] = {0x80, 0x78};
  if (AncillaAdd_CheckForPresence(a))
    return;
  int k = Ancilla_AddAncilla(a, y);
  if (k >= 0) {
    ancilla_item_to_link[k] = 16;
    ancilla_L[k] = 0;
    int j = link_direction_facing >> 1;
    ancilla_dir[k] = j = kAddDashTremor_Dir[j];
    uint8 y = link_y_coord - BG2VOFS_copy2;
    uint8 x = link_x_coord - BG2HOFS_copy2;
    Ancilla_SetY(k, (j ? y : x) < kAddDashTremor_Tab[j >> 1] ? 3 : -3);
  }
}

void AncillaAdd_BoomerangWallClink(int k) {  // 899478
  static const int8 kBoomerangWallHit_X[8] = {8, 8, 0, 10, 12, 8, 4, 0};
  static const int8 kBoomerangWallHit_Y[8] = {0, 8, 8, 8, 4, 8, 12, 8};
  static const uint8 kBoomerangWallHit_Tab0[16] = {0, 6, 4, 0, 2, 10, 12, 0, 0, 8, 14, 0, 0, 0, 0, 0};
  boomerang_temp_x = Ancilla_GetX(k);
  boomerang_temp_y = Ancilla_GetY(k);
  k = Ancilla_AddAncilla(6, 1);
  if (k >= 0) {
    ancilla_item_to_link[k] = 0;
    ancilla_arr3[k] = 1;
    int j = kBoomerangWallHit_Tab0[hookshot_effect_index] >> 1;
    Ancilla_SetXY(k, boomerang_temp_x + kBoomerangWallHit_X[j], boomerang_temp_y + kBoomerangWallHit_Y[j]);
  }
}

void AncillaAdd_HookshotWallClink(int kin, uint8 a, uint8 y) {  // 8994c6
  static const int8 kHookshotWallHit_X[8] = {8, 8, 0, 10, 12, 8, 4, 0};
  static const int8 kHookshotWallHit_Y[8] = {0, 8, 8, 8, 4, 8, 12, 8};
  int k = Ancilla_AddAncilla(a, y);
  if (k >= 0) {
    ancilla_item_to_link[k] = 0;
    ancilla_arr3[k] = 1;
    int j = ancilla_dir[kin];
    Ancilla_SetXY(k, Ancilla_GetX(kin) + kHookshotWallHit_X[j], Ancilla_GetY(kin) + kHookshotWallHit_Y[j]);
  }
}

void AncillaAdd_Duck_take_off(uint8 a, uint8 y) {  // 8994fe
  if (AncillaAdd_CheckForPresence(a))
    return;
  int k = Ancilla_AddAncilla(a, y);
  if (k >= 0) {
    ancilla_timer[k] = 0x78;
    ancilla_L[k] = 0;
    ancilla_z_vel[k] = 0;
    ancilla_z[k] = 0;
    ancilla_step[k] = 0;
    AddBirdCommon(k);
  }
}

void AddBirdTravelSomething(uint8 a, uint8 y) {  // 89951d
  if (AncillaAdd_CheckForPresence(a))
    return;
  int k = Ancilla_AddAncilla(a, y);
  if (k >= 0) {
    link_player_handler_state = 0;
    link_speed_setting = 0;
    button_mask_b_y &= ~0x81;
    button_b_frames = 0;
    link_delay_timer_spin_attack = 0;
    link_cant_change_direction &= ~1;
    ancilla_L[k] = 1;

    if (enhanced_features0 & kFeatures0_ExtendScreen64) {
      // todo: tune these better so the angle of attack is better
      ancilla_z_vel[k] = 58;
      ancilla_z[k] = -105;
    } else {
      ancilla_z_vel[k] = 40;
      ancilla_z[k] = -51;
    }
    ancilla_step[k] = 2;
    AddBirdCommon(k);
  }
}

void AncillaAdd_QuakeSpell(uint8 a, uint8 y) {  // 899589
  int k = Ancilla_AddAncilla(a, y);
  if (k >= 0) {
    ancilla_step[k] = 0;
    ancilla_item_to_link[k] = 0;
    load_chr_halfslot_even_odd = 13;
    sound_effect_1 = 0x35;
    for(int i = 0; i < 5; i++)
      quake_arr2[i] = 0;
    quake_var5 = 0;
    for(int i = 0; i < 5; i++)
      quake_arr1[i] = 1;
    flag_custom_spell_anim_active = 1;
    ancilla_timer[k] = 2;
    quake_var1 = link_y_coord + 26;
    quake_var2 = link_x_coord + 8;
    quake_var3 = 3;
  }
}

void AncillaAdd_SpinAttackInitSpark(uint8 a, uint8 x, uint8 y) {  // 89960b
  static const int8 kSpinAttackStartSparkle_Y[4] = {32, -8, 10, 20};
  static const int8 kSpinAttackStartSparkle_X[4] = {10, 7, 28, -10};

  int k = Ancilla_AddAncilla(a, y);
  for (int i = 4; i >= 0; i--) {
    if (ancilla_type[i] == 0x31)
      ancilla_type[i] = 0;
  }
  ancilla_item_to_link[k] = 0;
  ancilla_step[k] = x;
  ancilla_timer[k] = 4;
  ancilla_aux_timer[k] = 3;
  int j = link_direction_facing >> 1;
  Ancilla_SetXY(k,
      link_x_coord + kSpinAttackStartSparkle_X[j],
      link_y_coord + kSpinAttackStartSparkle_Y[j]);
}

void AncillaAdd_BlastWall() {  // 899692
  static const int8 kBlastWall_Tab3[4] = {-16, 16, 0, 0};
  static const int8 kBlastWall_Tab4[4] = {0, 0, -16, 16};
  static const int8 kBlastWall_Tab5[16] = {-8, 0, -8, 16, 16, 0, 16, 16, 0, -8, 16, -8, 0, 16, 16, 16};

  ancilla_type[0] = 0x33;
  ancilla_type[1] = 0x33;
  ancilla_type[2] = 0;
  ancilla_type[3] = 0;
  ancilla_type[4] = 0;
  ancilla_type[5] = 0;

  ancilla_item_to_link[0] = 0;
  flag_is_ancilla_to_pick_up = 0;
  link_state_bits = 0;
  link_cant_change_direction = 0;
  ancilla_K[0] = 0;
  ancilla_floor[0] = link_is_on_lower_level;
  ancilla_floor[1] = link_is_on_lower_level;
  ancilla_floor2[0] = link_is_on_lower_level_mirror;
  blastwall_var1 = 0;
  blastwall_var6[1] = 0;
  blastwall_var5[1] = 0;
  blastwall_var4 = 0;
  blastwall_var5[0] = 1;
  flag_custom_spell_anim_active = 1;
  blastwall_var6[0] = 3;
  int j = blastwall_var7;
  blastwall_var8 += kBlastWall_Tab3[j];
  blastwall_var9 += kBlastWall_Tab4[j];
  j = (j < 4) ? 4 : 0;
  for (int k = 3; k >= 0; k--, j++) {
    blastwall_var10[k] = blastwall_var8 + kBlastWall_Tab5[j * 2 + 0];
    blastwall_var11[k] = blastwall_var9 + kBlastWall_Tab5[j * 2 + 1];
    uint16 x = blastwall_var11[k] - BG2HOFS_copy2;
    if (x < 256)
      sound_effect_1 = kBombos_Sfx[x >> 5] | 0xc;
  }
  // In dark world forest castle hole outside door

}

void AncillaAdd_SwordChargeSparkle(int k) {  // 899757
  int j;
  for (j = 9; ancilla_type[j] != 0; ) {
    if (--j < 0)
      return;
  }
  ancilla_type[j] = 60;
  ancilla_floor[j] = link_is_on_lower_level;
  ancilla_item_to_link[j] = 0;
  ancilla_timer[j] = 4;

  uint8 rand = GetRandomNumber();

  uint8 z = ancilla_z[k];
  if (z >= 0xF8)
    z = 0;
  Ancilla_SetXY(j, Ancilla_GetX(k) + 2 + (rand >> 5), Ancilla_GetY(k) - 2 - z + (rand & 0xf));
}

void AncillaAdd_SilverArrowSparkle(int kin) {  // 8997de
  static const int8 kSilverArrowSparkle_X[4] = {-4, -4, 0, 2};
  static const int8 kSilverArrowSparkle_Y[4] = {0, 2, -4, -4};
  int k = Ancilla_AllocHigh();
  if (k >= 0) {
    ancilla_type[k] = 0x3c;
    ancilla_item_to_link[k] = 0;
    ancilla_timer[k] = 4;
    ancilla_floor[k] = link_is_on_lower_level;
    int m = GetRandomNumber();
    int j = ancilla_dir[kin] & 3;
    Ancilla_SetXY(k,
      Ancilla_GetX(kin) + kSilverArrowSparkle_X[j] + (m >> 4 & 7),
      Ancilla_GetY(kin) + kSilverArrowSparkle_Y[j] + (m & 7));
  }
}

void AncillaAdd_IceRodShot(uint8 a, uint8 y) {  // 899863
  static const int8 kIceRod_X[4] = {0, 0, -20, 20};
  static const int8 kIceRod_Y[4] = {-16, 24, 8, 8};
  static const int8 kIceRod_Xvel[4] = {0, 0, -48, 48};
  static const int8 kIceRod_Yvel[4] = {-48, 48, 0, 0};

  int k = Ancilla_AddAncilla(a, y);
  if (k < 0) {
    Refund_Magic(0);
    return;
  }
  sound_effect_1 = Link_CalculateSfxPan() | 15;
  ancilla_step[k] = 0;
  ancilla_arr25[k] = 0;
  ancilla_item_to_link[k] = 255;
  ancilla_L[k] = 1;
  ancilla_aux_timer[k] = 3;
  ancilla_arr3[k] = 6;
  int j = link_direction_facing >> 1;
  ancilla_dir[k] = j;
  ancilla_y_vel[k] = kIceRod_Yvel[j];
  ancilla_x_vel[k] = kIceRod_Xvel[j];

  if (Ancilla_CheckInitialTile_A(k) < 0) {
    uint16 x = link_x_coord + kIceRod_X[j];
    uint16 y = link_y_coord + kIceRod_Y[j];

    if (((x - BG2HOFS_copy2) | (y - BG2VOFS_copy2)) & 0xff00) {
      ancilla_type[k] = 0;
      return;
    }
    Ancilla_SetXY(k, x, y);
  } else {
    ancilla_type[k] = 0x11;
    ancilla_numspr[k] = kAncilla_Pflags[0x11];
    ancilla_item_to_link[k] = 0;
    ancilla_aux_timer[k] = 4;
  }
}

bool AncillaAdd_Splash(uint8 a, uint8 y) {  // 8998fc
  int k = Ancilla_AddAncilla(a, y);
  if (k >= 0) {
    sound_effect_1 = Link_CalculateSfxPan() | 0x24;
    ancilla_item_to_link[k] = 0;
    ancilla_aux_timer[k] = 2;
    if (player_is_indoors && !link_is_in_deep_water)
      link_is_on_lower_level = 0;
    Ancilla_SetXY(k, link_x_coord - 11, link_y_coord + 8);
  }
  return k < 0;
}

void AncillaAdd_GraveStone(uint8 ain, uint8 yin) {  // 8999e9
  static const uint16 kMoveGravestone_Y[8] = {0x550, 0x540, 0x530, 0x520, 0x500, 0x4e0, 0x4c0, 0x4b0};
  static const uint16 kMoveGravestone_X[15] = {0x8b0, 0x8f0, 0x910, 0x950, 0x970, 0x9a0, 0x850, 0x870, 0x8b0, 0x8f0, 0x920, 0x950, 0x880, 0x990, 0x840};
  static const uint16 kMoveGravestone_Y1[15] = {0x540, 0x530, 0x530, 0x530, 0x520, 0x520, 0x510, 0x510, 0x4f0, 0x4f0, 0x4f0, 0x4f0, 0x4d0, 0x4b0, 0x4a0};
  static const uint16 kMoveGravestone_X1[15] = {0x8b0, 0x8f0, 0x910, 0x950, 0x970, 0x9a0, 0x850, 0x870, 0x8b0, 0x8f0, 0x920, 0x950, 0x880, 0x990, 0x840};
  static const uint16 kMoveGravestone_Pos[15] = {0xa16, 0x99e, 0x9a2, 0x9aa, 0x92e, 0x934, 0x88a, 0x88e, 0x796, 0x79e, 0x7a4, 0x7aa, 0x690, 0x5b2, 0x508};
  static const uint8 kMoveGravestone_Ctr[15] = {0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x38, 0x58};
  static const uint8 kMoveGravestone_Idx[9] = {0, 1, 4, 6, 8, 12, 13, 14, 15};

  int k = Ancilla_AddAncilla(ain, yin);
  if (k < 0)
    return;
  int t = ((link_y_coord & 0xf) < 7 ? link_y_coord : link_y_coord + 16) & ~0xf;

  int i = 7;
  while (kMoveGravestone_Y[i] != t) {
    if (--i < 0) {
      ancilla_type[k] = 0;
      return;
    }
  }

  int j = kMoveGravestone_Idx[i];
  int end = kMoveGravestone_Idx[i + 1];
  do {
    int x = kMoveGravestone_X[j];
    if (x < link_x_coord && (uint16)(x + 15) >= link_x_coord) {
      if (j == 13 ? !link_is_running : link_is_running)
        break;

      int pos = kMoveGravestone_Pos[j];
      big_rock_starting_address = pos;
      door_open_closed_counter = kMoveGravestone_Ctr[j];
      if (door_open_closed_counter == 0x58) {
        sound_effect_2 = Link_CalculateSfxPan() | 0x1b;
      } else if (door_open_closed_counter == 0x38) {
        save_ow_event_info[BYTE(overworld_screen_index)] |= 0x20;
        sound_effect_2 = Link_CalculateSfxPan() | 0x1b;
      }

      ((uint8 *)door_debris_y)[k] = (pos - 0x80);
      ((uint8 *)door_debris_x)[k] = (pos - 0x80) >> 8;

      Overworld_DoMapUpdate32x32_B();

      if ((sound_effect_2 & 0x3f) != 0x1b)
        sound_effect_1 = Link_CalculateSfxPan() | 0x22;

      int yy = kMoveGravestone_Y1[j];
      int xx = kMoveGravestone_X1[j];
      bitmask_of_dragstate = 4;
      link_something_with_hookshot = 1;
      ancilla_A[k] = (yy - 18);
      ancilla_B[k] = (yy - 18) >> 8;
      Ancilla_SetXY(k, xx, yy - 2);
      return;
    }
  } while (++j != end);
  ancilla_type[k] = 0;
}

void AncillaAdd_WaterfallSplash() {  // 899b68
  if (AncillaAdd_CheckForPresence(0x41))
    return;
  int k = Ancilla_AddAncilla(0x41, 4);
  if (k >= 0) {
    ancilla_timer[k] = 2;
    ancilla_item_to_link[k] = 0;
  }
}

void AncillaAdd_GTCutscene() {  // 899b83
  if (link_state_bits & 0x80 | link_auxiliary_state ||
     (link_has_crystals & 0x7f) != 0x7f ||
      save_ow_event_info[0x43] & 0x20)
    return;

  Ancilla_TerminateSparkleObjects();

  if (AncillaAdd_CheckForPresence(0x43))
    return;

  int k = Ancilla_AddAncilla(0x43, 4);
  if (k < 0)
    return;

  for (int i = 15; i >= 0; i--) {
    if (sprite_type[i] == 0x37)
      sprite_state[i] = 0;
  }

  for (int i = 0x17; i >= 0; i--)
    breaktowerseal_sparkle_var1[i] = 0xff;
  DecodeAnimatedSpriteTile_variable(0x28);
  palette_sp6r_indoors = 4;
  overworld_palette_aux_or_main = 0x200;
  Palette_Load_SpriteEnvironment_Dungeon();
  flag_update_cgram_in_nmi++;
  flag_is_link_immobilized = 1;
  ancilla_y_subpixel[k] = 0;
  ancilla_x_subpixel[k] = 0;
  ancilla_step[k] = 0;
  breaktowerseal_var5 = 240;
  breaktowerseal_var4 = 0;

  breaktowerseal_var3[0] = 0;

  breaktowerseal_var3[1] = 10;
  breaktowerseal_var3[2] = 22;
  breaktowerseal_var3[3] = 32;
  breaktowerseal_var3[4] = 42;
  breaktowerseal_var3[5] = 54;

  Ancilla_SetXY(k, link_x_coord, link_y_coord - 16);
}

int AncillaAdd_DoorDebris() {  // 899c38
  int k = Ancilla_AddAncilla(8, 1);
  if (k >= 0) {
    ancilla_arr25[k] = 0;
    ancilla_arr26[k] = 7;
  }
  return k;
}

void FireRodShot_BecomeSkullWoodsFire(int k) {  // 899c4f
  if (player_is_indoors || !(BYTE(overworld_screen_index) & 0x40))
    return;

  ancilla_type[0] = 0x34;
  ancilla_type[1] = 0;
  ancilla_type[2] = 0;
  ancilla_type[3] = 0;
  ancilla_type[4] = 0;
  ancilla_type[5] = 0;
  flag_for_boomerang_in_place = 0;
  ancilla_numspr[0] = kAncilla_Pflags[0x34];
  skullwoodsfire_var0[0] = 253;
  skullwoodsfire_var0[1] = 254;
  skullwoodsfire_var0[2] = 255;
  skullwoodsfire_var0[3] = 0;
  skullwoodsfire_var4 = 0;
  skullwoodsfire_var5[0] = 5;
  skullwoodsfire_var5[1] = 5;
  skullwoodsfire_var5[2] = 5;
  skullwoodsfire_var5[3] = 5;
  ancilla_aux_timer[0] = 5;
  skullwoodsfire_var9 = 0x100;
  skullwoodsfire_var10 = 0x100;
  skullwoodsfire_var11 = 0x98;
  skullwoodsfire_var12 = 0x98;

  trigger_special_entrance = 2;
  subsubmodule_index = 0;
  BYTE(R16) = 0;
  ancilla_floor[0] = link_is_on_lower_level;
  ancilla_floor2[0] = link_is_on_lower_level_mirror;
  ancilla_item_to_link[0] = 0;
  ancilla_step[0] = 0;

}

int Ancilla_AddAncilla(uint8 a, uint8 y) {  // 899ce2
  int k = Ancilla_AllocInit(a, y);
  if (k >= 0) {
    ancilla_type[k] = a;
    ancilla_floor[k] = link_is_on_lower_level;
    ancilla_floor2[k] = link_is_on_lower_level_mirror;
    ancilla_y_vel[k] = 0;
    ancilla_x_vel[k] = 0;
    ancilla_objprio[k] = 0;
    ancilla_U[k] = 0;
    ancilla_numspr[k] = kAncilla_Pflags[a];
  }
  return k;
}

bool AncillaAdd_CheckForPresence(uint8 a) {  // 899d20
  for (int k = 5; k >= 0; k--) {
    if (ancilla_type[k] == a)
      return true;
  }
  return false;
}

int AncillaAdd_ArrowFindSlot(uint8 type, uint8 ay) {  // 899d36
  int k, n = 0;
  for (k = 4; k >= 0; k--) {
    if (ancilla_type[k] == 10)
      n++;
  }
  if (n != ay + 1) {
    for (k = 4; k >= 0; k--) {
      if (ancilla_type[k] == 0)
        break;
    }
  } else {
    do {
      if (sign8(--ancilla_alloc_rotate))
        ancilla_alloc_rotate = 4;
      k = ancilla_alloc_rotate;
    } while (ancilla_type[k] != 10);
  }
  if (k >= 0) {
    ancilla_type[k] = type;
    ancilla_floor[k] = link_is_on_lower_level;
    ancilla_floor2[k] = link_is_on_lower_level_mirror;
    ancilla_y_vel[k] = 0;
    ancilla_x_vel[k] = 0;
    ancilla_objprio[k] = 0;
    ancilla_U[k] = 0;
    ancilla_numspr[k] = kAncilla_Pflags[type];
  }
  return k;
}

int Ancilla_CheckInitialTile_A(int k) {  // 899dd3
  static const int8 kAncilla_Yoffs_Hb[12] = {8, 0, -8, 8, 16, 24, 8, 8, 8, 8, 8, 8};
  static const int8 kAncilla_Xoffs_Hb[12] = {0, 0, 0, 0, 0, 0, 0, -8, -16, 0, 8, 16};
  int j = ancilla_dir[k] * 3;
  int i;
  for (i = 2; i >= 0; i--, j++) {
    uint16 x = link_x_coord + kAncilla_Xoffs_Hb[j];
    uint16 y = link_y_coord + kAncilla_Yoffs_Hb[j];
    Ancilla_SetXY(k, x, y);
    if (Ancilla_CheckTileCollision(k))
      break;
  }
  return i;
}

bool Ancilla_CheckInitialTileCollision_Class2(int k) {  // 899e44
  static const int16 kAncilla_InitialTileColl_Y[9] = {15, 16, 28, 24, 12, 12, 12, 12, 8};
  static const int16 kAncilla_InitialTileColl_X[9] = {8, 8, 8, 8, -1, 0, 17, 16, 0x4b8b}; // wtf
  int j = ancilla_dir[k] * 2;
  for (int n = 2; n >= 0; n--, j++) {
    Ancilla_SetXY(k, link_x_coord + kAncilla_InitialTileColl_X[j],
                     link_y_coord + kAncilla_InitialTileColl_Y[j]);
    if (Ancilla_CheckTileCollision_Class2(k))
      return true;
  }
  return false;
}

uint8 Ancilla_TerminateSelectInteractives(uint8 y) {  // 89ac6b
  int i = 5;

  do {
    if (ancilla_type[i] == 0x3e) {
      y = i;
    }
    else if (ancilla_type[i] == 0x2c) {
      dung_flag_somaria_block_switch = 0;
      if (bitmask_of_dragstate & 0x80) {
        bitmask_of_dragstate = 0;
        link_speed_setting = 0;
      }
    }

    if (sign8(link_state_bits)) {
      if (i + 1 != flag_is_ancilla_to_pick_up)
        ancilla_type[i] = 0;
    }
    else {
      if (i + 1 == flag_is_ancilla_to_pick_up)
        flag_is_ancilla_to_pick_up = 0;
      ancilla_type[i] = 0;
    }
  } while (--i >= 0);

  if (link_position_mode & 0x10) {
    link_incapacitated_timer = 0;
    link_position_mode = 0;
  }
  flute_countdown = 0;
  tagalong_event_flags = 0;
  byte_7E02F3 = 0;
  flag_for_boomerang_in_place = 0;
  is_archer_or_shovel_game = 0;
  link_disable_sprite_damage = 0;
  byte_7E03FD = 0;
  link_electrocute_on_touch = 0;
  if (link_player_handler_state == 19) {
    link_player_handler_state = 0;
    button_mask_b_y &= ~0x40;
    link_cant_change_direction &= ~1;
    link_position_mode &= ~4;
    related_to_hookshot = 0;
  }
  return y;
}

void Ancilla_SetXY(int k, uint16 x, uint16 y) {  // 89ad06
  Ancilla_SetX(k, x);
  Ancilla_SetY(k, y);
}

void AncillaAdd_ExplodingSomariaBlock(int k) {  // 89ad30
  ancilla_type[k] = 0x2e;
  ancilla_numspr[k] = kAncilla_Pflags[0x2e];
  ancilla_aux_timer[k] = 3;
  ancilla_step[k] = 0;
  ancilla_item_to_link[k] = 0;
  ancilla_arr3[k] = 0;
  ancilla_arr1[k] = 0;
  ancilla_R[k] = 0;
  ancilla_objprio[k] = 0;
  dung_flag_somaria_block_switch = 0;
  sound_effect_2 = Ancilla_CalculateSfxPan(k) | 1;
}

bool Ancilla_AddRupees(int k) {  // 89ad6c
  static const uint8 kGiveRupeeGift_Tab[5] = {1, 5, 20, 100, 50};
  uint8 a = ancilla_item_to_link[k];
  if (a == 0x34 || a == 0x35 || a == 0x36) {
    link_rupees_goal += kGiveRupeeGift_Tab[a - 0x34];
  } else if (a == 0x40 || a == 0x41) {
    link_rupees_goal += kGiveRupeeGift_Tab[a - 0x40 + 3];
  } else if (a == 0x46) {
    link_rupees_goal += 300;
  } else if (a == 0x47) {
    link_rupees_goal += 20;
  } else {
    return false;
  }
  return true;
}

void DashDust_Motive(int k) {  // 89adf4
  static const uint8 kMotiveDashDust_Draw_Char[3] = {0xa9, 0xcf, 0xdf};
  if (!ancilla_timer[k]) {
    ancilla_timer[k] = 3;
    if (++ancilla_item_to_link[k] == 3) {
      ancilla_type[k] = 0;
      return;
    }
  }
  if (link_direction_facing == 2)
    Oam_AllocateFromRegionB(4);
  Point16U info;
  Ancilla_PrepOamCoord(k, &info);
  Ancilla_SetOam(GetOamCurPtr(), info.x, info.y,
                 kMotiveDashDust_Draw_Char[ancilla_item_to_link[k]], 4 | HIBYTE(oam_priority_value), 0);
}

uint8 Ancilla_CalculateSfxPan(int k) {  // 8dbb5e
  return CalculateSfxPan(Ancilla_GetX(k));
}

int Ancilla_AllocInit(uint8 type, uint8 limit) {  // 8ff577
  // snes bug: R14 is used in tile detection already
  // unless this is here it the memcmp will fail when entering/leaving a water through steps quickly
  if (g_ram[kRam_BugsFixed] >= kBugFix_PolyRenderer)
    BYTE(R14) = limit + 1;

  int n = 0;
  for (int i = 0; i < 5; i++) {
    if (ancilla_type[i] == type)
      n++;
  }
  if (limit + 1 == n)
    return -1;

  // Try to reuse an empty ancilla slot
  for (int j = (type == 7 || type == 8) ? limit : 4; j >= 0; j--) {
    if (ancilla_type[j] == 0)
      return j;
  }
  int k = ancilla_alloc_rotate;
  do {
    if (--k < 0)
      k = limit;
    uint8 old_type = ancilla_type[k];
    // reuse slots for sparkles or arrows in wall
    if (old_type == 0x3c || old_type == 0x13 || old_type == 0xa) {
      ancilla_alloc_rotate = k;
      return k;
    }
  } while (k != 0);
  ancilla_alloc_rotate = 0;
  return -1;
}

void AddSwordBeam(uint8 y) {  // 8ff67b
  static const int8 kSwordBeam_X[4] = {-8, -10, -22, 4};
  static const int8 kSwordBeam_Y[4] = {-24, 8, -6, -6};
  static const int8 kSwordBeam_S[4] = {-8, -8, -8, 8};
  static const uint8 kSwordBeam_Tab[16] = {0x21, 0x1d, 0x19, 0x15, 3, 0x3e, 0x3a, 0x36, 0x12, 0xe, 0xa, 6, 0x31, 0x2d, 0x29, 0x25};
  static const int8 kSwordBeam_Yvel[4] = {-64, 64, 0, 0};
  static const int8 kSwordBeam_Xvel[4] = {0, 0, -64, 64};

  int k = Ancilla_AddAncilla(0xc, y);
  if (k < 0)
    return;
  int j = link_direction_facing * 2;
  swordbeam_arr[0] = kSwordBeam_Tab[j + 0];
  swordbeam_arr[1] = kSwordBeam_Tab[j + 1];
  swordbeam_arr[2] = kSwordBeam_Tab[j + 2];
  swordbeam_arr[3] = swordbeam_var1 = kSwordBeam_Tab[j + 3];
  ancilla_aux_timer[k] = 2;
  ancilla_item_to_link[k] = 0x4c;
  ancilla_arr3[k] = 8;
  ancilla_step[k] = 0;
  ancilla_L[k] = 0;
  ancilla_G[k] = 0;
  ancilla_arr1[k] = 0;
  swordbeam_var2 = 14;
  j = link_direction_facing >> 1;
  ancilla_dir[k] = j;
  ancilla_y_vel[k] = kSwordBeam_Yvel[j];
  ancilla_x_vel[k] = kSwordBeam_Xvel[j];
  ancilla_S[k] = kSwordBeam_S[j];

  swordbeam_temp_y = link_y_coord + 12;
  swordbeam_temp_x = link_x_coord + 8;

  if (Ancilla_CheckInitialTile_A(k) >= 0) {
    Ancilla_SetXY(k, swordbeam_temp_x + kSwordBeam_X[j], swordbeam_temp_y + kSwordBeam_Y[j]);
    sound_effect_2 = 1 | Ancilla_CalculateSfxPan(k);
    ancilla_type[k] = 4;
    ancilla_timer[k] = 7;
    ancilla_numspr[k] = 16;
  }
}

void AncillaSpawn_SwordChargeSparkle() {  // 8ff979
  static const uint8 kSwordChargeSparkle_A[4] = {0, 0, 7, 7};
  static const uint8 kSwordChargeSparkle_B[4] = {0x70, 0x70, 0, 0};
  static const uint8 kSwordChargeSparkle_X[4] = {0, 3, 4, 5};
  static const uint8 kSwordChargeSparkle_Y[4] = {5, 12, 8, 8};
  int k = Ancilla_AllocHigh();
  if (k < 0)
    return;
  ancilla_type[k] = 0x3c;
  ancilla_item_to_link[k] = 0;
  ancilla_timer[k] = 4;
  ancilla_floor[k] = link_is_on_lower_level;
  int j = link_direction_facing >> 1;
  int8 x = 0, y = 0;
  uint8 m0 = kSwordChargeSparkle_A[j];
  if (!m0) {
    y = link_spin_attack_step_counter >> 2;
    if (j == 0)
      y = -y;
  }
  uint8 m1 = kSwordChargeSparkle_B[j];
  if (!m1) {
    x = link_spin_attack_step_counter >> 2;
    if (j == 2)
      x = -x;
  }
  uint8 r = GetRandomNumber();
  Ancilla_SetXY(k,
    link_x_coord + x + kSwordChargeSparkle_X[j] + ((r & m1) >> 4),
    link_y_coord + y + kSwordChargeSparkle_Y[j] + (r & m0));
}

int DashTremor_TwiddleOffset(int k) {  // 8ffafe
  int j = ancilla_dir[k];
  uint16 y = -Ancilla_GetY(k);
  Ancilla_SetY(k, y);
  if (player_is_indoors)
    return y;
  if (j == 2) {
    uint16 start = ow_scroll_vars0.ystart + 1;
    uint16 end = ow_scroll_vars0.yend - 1;
    uint16 a = y + BG2VOFS_copy2;
    return (a <= start || a >= end) ? 0 : y;
  } else {
    uint16 start = ow_scroll_vars0.xstart + 1;
    uint16 end = ow_scroll_vars0.xend - 1;
    uint16 a = y + BG2HOFS_copy2;
    return (a <= start || a >= end) ? 0 : y;
  }
}

void Ancilla_TerminateIfOffscreen(int j) {  // 8ffd52
  int xt = (enhanced_features0 & kFeatures0_ExtendScreen64) ? 0x40 : 0;
  uint16 x = Ancilla_GetX(j) - BG2HOFS_copy2 + xt;
  uint16 y = Ancilla_GetY(j) - BG2VOFS_copy2;
  if (x >= 244 + xt * 2 || y >= 240)
    ancilla_type[j] = 0;
}

bool Bomb_CheckUndersideSpriteStatus(int k, Point16U *out_pt, uint8 *out_r10) {  // 8ffdcf
  if (ancilla_item_to_link[k] != 0)
    return true;

  uint8 r10 = 0;
  if (ancilla_tile_attr[k] == 9) {
    if (sign8(--ancilla_arr22[k])) {
      ancilla_arr22[k] = 3;
      if (++ancilla_arr23[k] == 3)
        ancilla_arr23[k] = 0;
    }
    r10 = ancilla_arr23[k] + 4;
    if ((sound_effect_1 & 0x3f) == 0xb || (sound_effect_1 & 0x3f) == 0x21)
      sound_effect_1 = Ancilla_CalculateSfxPan(k) | 0x28;
  } else if (ancilla_tile_attr[k] == 0x40) {
    r10 = 3;
  }

  if (ancilla_z[k] >= 2 && ancilla_z[k] < 252)
    r10 = 2;
  if (k + 1 == flag_is_ancilla_to_pick_up && (link_state_bits & 0x80))
    return true;
  int z = (int8)ancilla_z[k];
  out_pt->y += z + 2;
  out_pt->x += -8;
  *out_r10 = r10;
  return false;
}

void Sprite_CreateDeflectedArrow(int k) {  // 9d8040
  ancilla_type[k] = 0;
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(k, 0x1b, &info);
  if (j >= 0) {
    sprite_x_lo[j] = ancilla_x_lo[k];
    sprite_x_hi[j] = ancilla_x_hi[k];
    sprite_y_lo[j] = ancilla_y_lo[k];
    sprite_y_hi[j] = ancilla_y_hi[k];
    sprite_state[j] = 6;
    sprite_delay_main[j] = 31;
    sprite_x_vel[j] = ancilla_x_vel[k];
    sprite_y_vel[j] = ancilla_y_vel[k];
    sprite_floor[j] = link_is_on_lower_level;
    Sprite_PlaceWeaponTink(j);
  }
}

