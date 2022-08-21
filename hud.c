#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include "zelda_rtl.h"

#include "variables.h"
#include "hud.h"
const uint8 kMaxBombsForLevel[] = { 10, 15, 20, 25, 30, 35, 40, 50 };
const uint8 kMaxArrowsForLevel[] = { 30, 35, 40, 45, 50, 55, 60, 70 };
static const uint8 kMaxHealthForLevel[] = { 9, 9, 9, 9, 9, 9, 9, 9, 17, 17, 17, 17, 17, 17, 17, 25, 25, 25, 25, 25, 25 };
static const uint16 kHudItemInVramPtr[20] = {
  0x11c8, 0x11ce, 0x11d4, 0x11da,
  0x11e0, 0x1288, 0x128e, 0x1294,
  0x129a, 0x12a0, 0x1348, 0x134e,
  0x1354, 0x135a, 0x1360, 0x1408,
  0x140e, 0x1414, 0x141a, 0x1420,
};
static const uint16 kHudBottlesGfx[128] = {
  0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x255c, 0x2564, 0x2562, 0x2557, 0x2561, 0x255e, 0x255e, 0x255c,
  0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x2551, 0x255e, 0x2563, 0x2563, 0x255b, 0x2554, 0x24f5, 0x24f5,
  0x255b, 0x2558, 0x2555, 0x2554, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x255c, 0x2554, 0x2553, 0x2558, 0x2552, 0x2558, 0x255d, 0x2554,
  0x255c, 0x2550, 0x2556, 0x2558, 0x2552, 0x24f5, 0x24f5, 0x24f5, 0x255c, 0x2554, 0x2553, 0x2558, 0x2552, 0x2558, 0x255d, 0x2554,
  0x2552, 0x2564, 0x2561, 0x2554, 0x256a, 0x2550, 0x255b, 0x255b, 0x255c, 0x2554, 0x2553, 0x2558, 0x2552, 0x2558, 0x255d, 0x2554,
  0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x2555, 0x2550, 0x2554, 0x2561, 0x2558, 0x2554, 0x24f5, 0x24f5,
  0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x2551, 0x2554, 0x2554, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5,
  0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x2556, 0x255e, 0x255e, 0x2553, 0x24f5, 0x2551, 0x2554, 0x2554,
};
static const gfx kHudItemBottles[9] = {
  {0x20f5, 0x20f5, 0x20f5, 0x20f5},
  {0x2044, 0x2045, 0x2046, 0x2047},
  {0x2837, 0x2838, 0x2cc3, 0x2cd3},
  {0x24d2, 0x64d2, 0x24e2, 0x24e3},
  {0x3cd2, 0x7cd2, 0x3ce2, 0x3ce3},
  {0x2cd2, 0x6cd2, 0x2ce2, 0x2ce3},
  {0x2855, 0x6855, 0x2c57, 0x2c5a},
  {0x2837, 0x2838, 0x2839, 0x283a},
  {0x2837, 0x2838, 0x2839, 0x283a},
};
static const gfx kHudItemBow[5] = {
  {0x20f5, 0x20f5, 0x20f5, 0x20f5},
  {0x28ba, 0x28e9, 0x28e8, 0x28cb},
  {0x28ba, 0x284a, 0x2849, 0x28cb},
  {0x28ba, 0x28e9, 0x28e8, 0x28cb},
  {0x28ba, 0x28bb, 0x24ca, 0x28cb},
};
static const gfx kHudItemBoomerang[3] = {
  {0x20f5, 0x20f5, 0x20f5, 0x20f5},
  {0x2cb8, 0x2cb9, 0x2cf5, 0x2cc9},
  {0x24b8, 0x24b9, 0x24f5, 0x24c9},
};
static const gfx kHudItemHookshot[2] = {
  {0x20f5, 0x20f5, 0x20f5, 0x20f5},
  {0x24f5, 0x24f6, 0x24c0, 0x24f5},
};
static const gfx kHudItemBombs[2] = {
  {0x20f5, 0x20f5, 0x20f5, 0x20f5},
  {0x2cb2, 0x2cb3, 0x2cc2, 0x6cc2},
};
static const gfx kHudItemMushroom[3] = {
  {0x20f5, 0x20f5, 0x20f5, 0x20f5},
  {0x2444, 0x2445, 0x2446, 0x2447},
  {0x203b, 0x203c, 0x203d, 0x203e},
};
static const gfx kHudItemFireRod[2] = {
  {0x20f5, 0x20f5, 0x20f5, 0x20f5},
  {0x24b0, 0x24b1, 0x24c0, 0x24c1},
};
static const gfx kHudItemIceRod[2] = {
  {0x20f5, 0x20f5, 0x20f5, 0x20f5},
  {0x2cb0, 0x2cbe, 0x2cc0, 0x2cc1},
};
static const gfx kHudItemBombos[2] = {
  {0x20f5, 0x20f5, 0x20f5,  0x20f5},
  {0x287d, 0x287e, 0xe87e, 0xe87d},
};
static const gfx kHudItemEther[2] = {
  {0x20f5, 0x20f5,  0x20f5,  0x20f5},
  {0x2876, 0x2877, 0xE877, 0xE876},
};
static const gfx kHudItemQuake[2] = {
  {0x20f5, 0x20f5,  0x20f5,  0x20f5},
  {0x2866, 0x2867, 0xE867, 0xE866},
};
static const gfx kHudItemTorch[2] = {
  {0x20f5, 0x20f5, 0x20f5, 0x20f5},
  {0x24bc, 0x24bd, 0x24cc, 0x24cd},
};
static const gfx kHudItemHammer[2] = {
  {0x20f5, 0x20f5, 0x20f5, 0x20f5},
  {0x20b6, 0x20b7, 0x20c6, 0x20c7},
};
static const gfx kHudItemFlute[4] = {
  {0x20f5, 0x20f5, 0x20f5, 0x20f5},
  {0x20d0, 0x20d1, 0x20e0, 0x20e1},
  {0x2cd4, 0x2cd5, 0x2ce4, 0x2ce5},
  {0x2cd4, 0x2cd5, 0x2ce4, 0x2ce5},
};
static const gfx kHudItemBugNet[2] = {
  {0x20f5, 0x20f5, 0x20f5, 0x20f5},
  {0x3c40, 0x3c41, 0x2842, 0x3c43},
};
static const gfx kHudItemBookMudora[2] = {
  {0x20f5, 0x20f5, 0x20f5, 0x20f5},
  {0x3ca5, 0x3ca6, 0x3cd8, 0x3cd9},
};
static const gfx kHudItemCaneSomaria[2] = {
  {0x20f5, 0x20f5, 0x20f5, 0x20f5},
  {0x24dc, 0x24dd, 0x24ec, 0x24ed},
};
static const gfx kHudItemCaneByrna[2] = {
  {0x20f5, 0x20f5, 0x20f5, 0x20f5},
  {0x2cdc, 0x2cdd, 0x2cec, 0x2ced},
};
static const gfx kHudItemCape[2] = {
  {0x20f5, 0x20f5, 0x20f5, 0x20f5},
  {0x24b4, 0x24b5, 0x24c4, 0x24c5},
};
static const gfx kHudItemMirror[4] = {
  {0x20f5, 0x20f5, 0x20f5, 0x20f5},
  {0x28de, 0x28df, 0x28ee, 0x28ef},
  {0x2c62, 0x2c63, 0x2c72, 0x2c73},
  {0x2886, 0x2887, 0x2888, 0x2889},
};
static const gfx kHudItemGloves[3] = {
  {0x20f5, 0x20f5, 0x20f5, 0x20f5},
  {0x2130, 0x2131, 0x2140, 0x2141},
  {0x28da, 0x28db, 0x28ea, 0x28eb},
};
static const gfx kHudItemBoots[2] = {
  {0x20f5, 0x20f5, 0x20f5, 0x20f5},
  {0x3429, 0x342a, 0x342b, 0x342c},
};
static const gfx kHudItemFlippers[2] = {
  {0x20f5, 0x20f5, 0x20f5, 0x20f5},
  {0x2c9a, 0x2c9b, 0x2c9d, 0x2c9e},
};
static const gfx kHudItemMoonPearl[2] = {
  {0x20f5, 0x20f5, 0x20f5, 0x20f5},
  {0x2433, 0x2434, 0x2435, 0x2436},
};
static const gfx kHudItemEmpty[1] = {
  {0x20f5, 0x20f5, 0x20f5, 0x20f5},
};
static const gfx kHudItemSword[5] = {
  {0x20f5, 0x20f5, 0x20f5, 0x20f5},
  {0x2c64, 0x2cce, 0x2c75, 0x3d25},
  {0x2c8a, 0x2c65, 0x2474, 0x3d26},
  {0x248a, 0x2465, 0x3c74, 0x2d48},
  {0x288a, 0x2865, 0x2c74, 0x2d39},
};
static const gfx kHudItemShield[4] = {
  {0x24f5, 0x24f5, 0x24f5, 0x24f5},
  {0x2cfd, 0x6cfd, 0x2cfe, 0x6cfe},
  {0x34ff, 0x74ff, 0x349f, 0x749f},
  {0x2880, 0x2881, 0x288d, 0x288e},
};
static const gfx kHudItemArmor[5] = {
  {0x3c68, 0x7c68, 0x3c78, 0x7c78},
  {0x2c68, 0x6c68, 0x2c78, 0x6c78},
  {0x2468, 0x6468, 0x2478, 0x6478},
};
static const gfx kHudItemDungeonCompass[2] = {
  {0x20f5, 0x20f5, 0x20f5, 0x20f5},
  {0x24bf, 0x64bf, 0x2ccf, 0x6ccf},
};
static const gfx kHudItemPalaceItem[3] = {
  {0x20f5, 0x20f5, 0x20f5, 0x20f5},
  {0x28d6, 0x68d6, 0x28e6, 0x28e7},
  {0x354b, 0x354c, 0x354d, 0x354e},
};
static const gfx kHudItemDungeonMap[2] = {
  {0x20f5, 0x20f5, 0x20f5, 0x20f5},
  {0x28de, 0x28df, 0x28ee, 0x28ef},
};
static const gfx kHudPendants0[2] = {
  {0x313b, 0x313c, 0x313d, 0x313e},
  {0x252b, 0x252c, 0x252d, 0x252e}
};
static const gfx kHudPendants1[2] = {
  {0x313b, 0x313c, 0x313d, 0x313e},
  {0x2d2b, 0x2d2c, 0x2d2d, 0x2d2e}
};
static const gfx kHudPendants2[2] = {
  {0x313b, 0x313c, 0x313d, 0x313e},
  {0x3d2b, 0x3d2c, 0x3d2d, 0x3d2e}
};
static const gfx kHudItemHeartPieces[4] = {
  {0x2484, 0x6484, 0x2485, 0x6485},
  {0x24ad, 0x6484, 0x2485, 0x6485},
  {0x24ad, 0x6484, 0x24ae, 0x6485},
  {0x24ad, 0x64ad, 0x24ae, 0x6485},
};
static const uint16 kHudAbilityText[80] = {
  0x2cf5, 0x2cf5, 0x2cf5, 0x2cf5, 0x2cf5, 0x2d5b, 0x2d58, 0x2d55, 0x2d63, 0x2d27,
  0x2cf5, 0x2cf5, 0x2cf5, 0x2cf5, 0x2cf5, 0x2cf5, 0x2d61, 0x2d54, 0x2d50, 0x2d53,
  0x2cf5, 0x2cf5, 0x2cf5, 0x2cf5, 0x2cf5, 0x2cf5, 0x2d63, 0x2d50, 0x2d5b, 0x2d5a,
  0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f,
  0x2cf5, 0x2cf5, 0x2c2e, 0x2cf5, 0x2cf5, 0x2d5f, 0x2d64, 0x2d5b, 0x2d5b, 0x2cf5,
  0x2cf5, 0x2cf5, 0x2cf5, 0x2cf5, 0x2cf5, 0x2cf5, 0x2d61, 0x2d64, 0x2d5d, 0x2cf5,
  0x2cf5, 0x2cf5, 0x2cf5, 0x2cf5, 0x2cf5, 0x2cf5, 0x2d62, 0x2d66, 0x2d58, 0x2d5c,
  0x2cf5, 0x2cf5, 0x2cf5, 0x207f, 0x207f, 0x2c01, 0x2c18, 0x2c28, 0x207f, 0x207f,
};
static const uint16 kHudGlovesText[20] = {
  0x2cf5, 0x2cf5, 0x2cf5, 0x2cf5, 0x2cf5, 0x2d5b, 0x2d58, 0x2d55, 0x2d63, 0x2d28,
  0x2cf5, 0x2cf5, 0x2cf5, 0x2cf5, 0x2cf5, 0x2d5b, 0x2d58, 0x2d55, 0x2d63, 0x2d29,
};
static const uint16 kProgressIconPendantsBg[90] = {
  0x28fb, 0x28f9, 0x28f9, 0x28f9, 0x28f9, 0x28f9, 0x28f9, 0x28f9, 0x28f9, 0x68fb,
  0x28fc, 0x2521, 0x2522, 0x2523, 0x2524, 0x253f, 0x24f5, 0x24f5, 0x24f5, 0x68fc,
  0x28fc, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x68fc,
  0x28fc, 0x24f5, 0x24f5, 0x24f5, 0x213b, 0x213c, 0x24f5, 0x24f5, 0x24f5, 0x68fc,
  0x28fc, 0x24f5, 0x24f5, 0x24f5, 0x213d, 0x213e, 0x24f5, 0x24f5, 0x24f5, 0x68fc,
  0x28fc, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x68fc,
  0x28fc, 0x24f5, 0x213b, 0x213c, 0x24f5, 0x24f5, 0x213b, 0x213c, 0x24f5, 0x68fc,
  0x28fc, 0x24f5, 0x213d, 0x213e, 0x24f5, 0x24f5, 0x213d, 0x213e, 0x24f5, 0x68fc,
  0xa8fb, 0xa8f9, 0xa8f9, 0xa8f9, 0xa8f9, 0xa8f9, 0xa8f9, 0xa8f9, 0xa8f9, 0xe8fb,
};
static const uint16 kProgressIconCrystalsBg[90] = {
  0x28fb, 0x28f9, 0x28f9, 0x28f9, 0x28f9, 0x28f9, 0x28f9, 0x28f9, 0x28f9, 0x68fb,
  0x28fc, 0x252f, 0x2534, 0x2535, 0x2536, 0x2537, 0x24f5, 0x24f5, 0x24f5, 0x68fc,
  0x28fc, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x68fc,
  0x28fc, 0x24f5, 0x24f5, 0x3146, 0x3147, 0x3146, 0x3147, 0x24f5, 0x24f5, 0x68fc,
  0x28fc, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x68fc,
  0x28fc, 0x24f5, 0x3146, 0x3147, 0x3146, 0x3147, 0x3146, 0x3147, 0x24f5, 0x68fc,
  0x28fc, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x68fc,
  0x28fc, 0x24f5, 0x24f5, 0x3146, 0x3147, 0x3146, 0x3147, 0x24f5, 0x24f5, 0x68fc,
  0xa8fb, 0xa8f9, 0xa8f9, 0xa8f9, 0xa8f9, 0xa8f9, 0xa8f9, 0xa8f9, 0xa8f9, 0xe8fb,
};
static const uint16 kHudItemText[320] = {
  0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x256b, 0x256c, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5,
  0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x2570, 0x2571, 0x2572, 0x2573, 0x2574, 0x2575, 0x2576, 0x2577,
  0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x2557, 0x255e, 0x255e, 0x255a, 0x2562, 0x2557, 0x255e, 0x2563,
  0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x2551, 0x255e, 0x255c, 0x2551, 0x24f5, 0x24f5, 0x24f5, 0x24f5,
  0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x255c, 0x2564, 0x2562, 0x2557, 0x2561, 0x255e, 0x255e, 0x255c,
  0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x2555, 0x2558, 0x2561, 0x2554, 0x2561, 0x255e, 0x2553, 0x24f5,
  0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x2558, 0x2552, 0x2554, 0x2561, 0x255e, 0x2553, 0x24f5, 0x24f5,
  0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x2551, 0x255e, 0x255c, 0x2551, 0x255e, 0x2562, 0x24f5, 0x24f5,
  0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x2554, 0x2563, 0x2557, 0x2554, 0x2561, 0x24f5, 0x24f5, 0x24f5,
  0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x2560, 0x2564, 0x2550, 0x255a, 0x2554, 0x24f5, 0x24f5, 0x24f5,
  0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x255b, 0x2550, 0x255c, 0x255f, 0x24f5, 0x24f5, 0x24f5, 0x24f5,
  0x255c, 0x2550, 0x2556, 0x2558, 0x2552, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x2557, 0x2550, 0x255c, 0x255c, 0x2554, 0x2561,
  0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x2562, 0x2557, 0x255e, 0x2565, 0x2554, 0x255b, 0x24f5, 0x24f5,
  0x2400, 0x2401, 0x2402, 0x2403, 0x2404, 0x2405, 0x2406, 0x2407, 0x2408, 0x2409, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5,
  0x2551, 0x255e, 0x255e, 0x255a, 0x24f5, 0x255e, 0x2555, 0x24f5, 0x255c, 0x2564, 0x2553, 0x255e, 0x2561, 0x2550, 0x24f5, 0x24f5,
  0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x255c, 0x2564, 0x2562, 0x2557, 0x2561, 0x255e, 0x255e, 0x255c,
  0x2552, 0x2550, 0x255d, 0x2554, 0x24f5, 0x255e, 0x2555, 0x24f5, 0x24f5, 0x2562, 0x255e, 0x255c, 0x2550, 0x2561, 0x2558, 0x2550,
  0x2552, 0x2550, 0x255d, 0x2554, 0x24f5, 0x255e, 0x2555, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x2551, 0x2568, 0x2561, 0x255d, 0x2550,
  0x255c, 0x2550, 0x2556, 0x2558, 0x2552, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x2552, 0x2550, 0x255f, 0x2554, 0x24f5,
  0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5,
};
static const uint16 kHudBottlesItemText[128] = {
  0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x255c, 0x2564, 0x2562, 0x2557, 0x2561, 0x255e, 0x255e, 0x255c,
  0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x2551, 0x255e, 0x2563, 0x2563, 0x255b, 0x2554, 0x24f5, 0x24f5,
  0x255b, 0x2558, 0x2555, 0x2554, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x255c, 0x2554, 0x2553, 0x2558, 0x2552, 0x2558, 0x255d, 0x2554,
  0x255c, 0x2550, 0x2556, 0x2558, 0x2552, 0x24f5, 0x24f5, 0x24f5, 0x255c, 0x2554, 0x2553, 0x2558, 0x2552, 0x2558, 0x255d, 0x2554,
  0x2552, 0x2564, 0x2561, 0x2554, 0x256a, 0x2550, 0x255b, 0x255b, 0x255c, 0x2554, 0x2553, 0x2558, 0x2552, 0x2558, 0x255d, 0x2554,
  0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x2555, 0x2550, 0x2554, 0x2561, 0x2558, 0x2554, 0x24f5, 0x24f5,
  0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x2551, 0x2554, 0x2554, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5,
  0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x2556, 0x255e, 0x255e, 0x2553, 0x24f5, 0x2551, 0x2554, 0x2554,
};
static const uint16 kHudMushroomItemText[16] = {
  0x255c, 0x2550, 0x2556, 0x2558, 0x2552, 0x24f5, 0x24f5, 0x24f5,
  0x24f5, 0x255f, 0x255e, 0x2566, 0x2553, 0x2554, 0x2561, 0x24f5,
};
static const uint16 kHudFluteItemText[32] = {
  0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x2555, 0x255b, 0x2564, 0x2563, 0x2554, 0x24f5, 0x24f5, 0x24f5,
  0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x2555, 0x255b, 0x2564, 0x2563, 0x2554, 0x24f5, 0x24f5, 0x24f5
};
static const uint16 kHudMirrorItemText[16] = {
  0x255c, 0x2550, 0x2556, 0x2558, 0x2552, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x255c, 0x2558, 0x2561, 0x2561, 0x255e, 0x2561
};
static const uint16 kHudBowItemText[48] = {
  0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x256b, 0x256c, 0x256e, 0x256f, 0x257c, 0x257d, 0x257e, 0x257f,
  0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x256b, 0x256c, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5, 0x24f5,
  0x256b, 0x256c, 0x24f5, 0x256e, 0x256f, 0x24f5, 0x24f5, 0x24f5, 0x2578, 0x2579, 0x257a, 0x257b, 0x257c, 0x257d, 0x257e, 0x257f,
};
static const uint16 kHudTilemap[165] = {
  0x207f, 0x207f, 0x2850, 0xa856, 0x2852, 0x285b, 0x285b, 0x285c, 0x207f, 0x3ca8, 0x207f, 0x207f, 0x2c88, 0x2c89, 0x207f, 0x20a7, 0x20a9, 0x207f, 0x2871, 0x207f, 0x207f, 0x207f, 0x288b, 0x288f, 0x24ab, 0x24ac, 0x688f, 0x688b, 0x207f, 0x207f, 0x207f, 0x207f,
  0x207f, 0x207f, 0x2854, 0x2871, 0x2858, 0x207f, 0x207f, 0x285d, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f,
  0x207f, 0x207f, 0x2854, 0x304e, 0x2858, 0x207f, 0x207f, 0x285d, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f,
  0x207f, 0x207f, 0x2854, 0x305e, 0x2859, 0xa85b, 0xa85b, 0xa85c, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f,
  0x207f, 0x207f, 0x2854, 0x305e, 0x6854, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f, 0x207f,
  0x207f, 0x207f, 0xa850, 0x2856, 0xe850,
};
const gfx * kHudItemBoxGfxPtrs[] = {
  kHudItemBow,
  kHudItemBoomerang,
  kHudItemHookshot,
  kHudItemBombs,
  kHudItemMushroom,
  kHudItemFireRod,
  kHudItemIceRod,
  kHudItemBombos,
  kHudItemEther,
  kHudItemQuake,
  kHudItemTorch,
  kHudItemHammer,
  kHudItemFlute,
  kHudItemBugNet,
  kHudItemBookMudora,
  kHudItemBottles,
  kHudItemCaneSomaria,
  kHudItemCaneByrna,
  kHudItemCape,
  kHudItemMirror,
  kHudItemGloves,
  kHudItemBoots,
  kHudItemFlippers,
  kHudItemMoonPearl,
  kHudItemEmpty,
  kHudItemSword,
  kHudItemShield,
  kHudItemArmor,
  kHudItemBottles,
  kHudItemBottles,
  kHudItemBottles,
  kHudItemBottles,
};
static const uint16 kUpdateMagicPowerTilemap[17][4] = {
  {0x3cf5, 0x3cf5, 0x3cf5, 0x3cf5},
  {0x3cf5, 0x3cf5, 0x3cf5, 0x3c5f},
  {0x3cf5, 0x3cf5, 0x3cf5, 0x3c4c},
  {0x3cf5, 0x3cf5, 0x3cf5, 0x3c4d},
  {0x3cf5, 0x3cf5, 0x3cf5, 0x3c4e},
  {0x3cf5, 0x3cf5, 0x3c5f, 0x3c5e},
  {0x3cf5, 0x3cf5, 0x3c4c, 0x3c5e},
  {0x3cf5, 0x3cf5, 0x3c4d, 0x3c5e},
  {0x3cf5, 0x3cf5, 0x3c4e, 0x3c5e},
  {0x3cf5, 0x3c5f, 0x3c5e, 0x3c5e},
  {0x3cf5, 0x3c4c, 0x3c5e, 0x3c5e},
  {0x3cf5, 0x3c4d, 0x3c5e, 0x3c5e},
  {0x3cf5, 0x3c4e, 0x3c5e, 0x3c5e},
  {0x3c5f, 0x3c5e, 0x3c5e, 0x3c5e},
  {0x3c4c, 0x3c5e, 0x3c5e, 0x3c5e},
  {0x3c4d, 0x3c5e, 0x3c5e, 0x3c5e},
  {0x3c4e, 0x3c5e, 0x3c5e, 0x3c5e},
};
static const uint16 kDungFloorIndicator_Gfx0[11] = { 0x2508, 0x2509, 0x2509, 0x250a, 0x250b, 0x250c, 0x250d, 0x251d, 0xe51c, 0x250e, 0x7f };
static const uint16 kDungFloorIndicator_Gfx1[11] = { 0x2518, 0x2519, 0xa509, 0x251a, 0x251b, 0x251c, 0x2518, 0xa51d, 0xe50c, 0xa50e, 0x7f };
void Hud_RefreshIcon() {
  Hud_SearchForEquippedItem();
  Hud_UpdateHud();
  Hud_Rebuild();
  overworld_map_state = 0;
}

uint8 CheckPalaceItemPosession() {
  switch (cur_palace_index_x2 >> 1) {
  case 2: return link_item_bow != 0;
  case 3: return link_item_gloves != 0;
  case 5: return link_item_hookshot != 0;
  case 6: return link_item_hammer != 0;
  case 7: return link_item_cane_somaria != 0;
  case 8: return link_item_fire_rod != 0;
  case 9: return link_armor != 0;
  case 10: return link_item_moon_pearl != 0;
  case 11: return link_item_gloves != 1;
  case 12: return link_shield_type == 3;
  case 13: return link_armor == 2;
  default:
    return 0;
  }
}

void Hud_GotoPrevItem() {
  if (--hud_cur_item < 1)
    hud_cur_item = 20;
}

void Hud_GotoNextItem() {
  if (++hud_cur_item >= 21)
    hud_cur_item = 1;
}

void Hud_FloorIndicator() {  // 8afd0c
  uint16 a = hud_floor_changed_timer;
  if (a == 0) {
    Hud_RemoveSuperBombIndicator();
    return;
  }
  a += 1;
  if (a == 0xc0)
    a = 0;
  WORD(hud_floor_changed_timer) = a;

  hud_tile_indices_buffer[0xf2 / 2] = 0x251e;
  hud_tile_indices_buffer[0x134 / 2] = 0x251f;
  hud_tile_indices_buffer[0x132 / 2] = 0x2520;
  hud_tile_indices_buffer[0xf4 / 2] = 0x250f;

  int k = 0, j;

  if (!sign8(dung_cur_floor)) {
    if (!WORD(dung_cur_floor) && dungeon_room_index != 2 && sram_progress_indicator < 2)
      sound_effect_ambient = 3;
    j = dung_cur_floor;
  } else {
    sound_effect_ambient = 5;
    k++;
    j = dung_cur_floor ^ 0xff;
  }
  hud_tile_indices_buffer[k + 0xf2 / 2] = kDungFloorIndicator_Gfx0[j];
  hud_tile_indices_buffer[k + 0x132 / 2] = kDungFloorIndicator_Gfx1[j];
  flag_update_hud_in_nmi++;
}

void Hud_RemoveSuperBombIndicator() {  // 8afd90
  hud_tile_indices_buffer[0xf2 / 2] = 0x7f;
  hud_tile_indices_buffer[0x132 / 2] = 0x7f;
  hud_tile_indices_buffer[0xf4 / 2] = 0x7f;
  hud_tile_indices_buffer[0x134 / 2] = 0x7f;
}

void Hud_SuperBombIndicator() {  // 8afda8
  if (!super_bomb_indicator_unk1) {
    if (sign8(super_bomb_indicator_unk2))
      goto remove;
    super_bomb_indicator_unk2--;
    super_bomb_indicator_unk1 = 62;
  }
  super_bomb_indicator_unk1--;
  if (sign8(super_bomb_indicator_unk2)) {
remove:
    super_bomb_indicator_unk2 = 0xff;
    Hud_RemoveSuperBombIndicator();
    return;
  }

  int r = super_bomb_indicator_unk2 % 10;
  int q = super_bomb_indicator_unk2 / 10;

  int j = sign8(r - 1) ? 9 : r - 1;
  hud_tile_indices_buffer[0xf4 / 2] = kDungFloorIndicator_Gfx0[j];
  hud_tile_indices_buffer[0x134 / 2] = kDungFloorIndicator_Gfx1[j];

  j = sign8(q - 1) ? 10 : q - 1;
  hud_tile_indices_buffer[0xf2 / 2] = kDungFloorIndicator_Gfx0[j];
  hud_tile_indices_buffer[0x132 / 2] = kDungFloorIndicator_Gfx1[j];

}

void Hud_RefillLogic() {  // 8ddb92
  if (overworld_map_state)
    return;
  if (link_magic_filler) {
    if (link_magic_power >= 128) {
      link_magic_power = 128;
      link_magic_filler = 0;
    } else {
      link_magic_filler--;
      link_magic_power++;
      if ((frame_counter & 3) == 0 && sound_effect_1 == 0)
        sound_effect_1 = 45;
    }
  }

  uint16 a = link_rupees_actual;
  if (a != link_rupees_goal) {
    if (a >= link_rupees_goal) {
      if ((int16)--a < 0)
        link_rupees_goal = a = 0;
    } else {
      if (++a >= 1000)
        link_rupees_goal = a = 999;
    }
    link_rupees_actual = a;
    if (sound_effect_1 == 0) {
      if ((rupee_sfx_sound_delay++ & 7) == 0)
        sound_effect_1 = 41;
    } else {
      rupee_sfx_sound_delay = 0;
    }
  } else {
    rupee_sfx_sound_delay = 0;
  }

  if (link_bomb_filler) {
    link_bomb_filler--;
    if (link_item_bombs != kMaxBombsForLevel[link_bomb_upgrades])
      link_item_bombs++;
  }

  if (link_arrow_filler) {
    link_arrow_filler--;
    if (link_num_arrows != kMaxArrowsForLevel[link_arrow_upgrades])
      link_num_arrows++;
    if (link_item_bow && (link_item_bow & 1) == 1) {
      link_item_bow++;
      Hud_RefreshIcon();
    }
  }

  if (!flag_is_link_immobilized && !link_hearts_filler &&
      link_health_current < kMaxHealthForLevel[link_health_capacity >> 3]) {
    if (link_lowlife_countdown_timer_beep) {
      link_lowlife_countdown_timer_beep--;
    } else if (!sound_effect_1) {
      sound_effect_1 = 43;
      link_lowlife_countdown_timer_beep = 32 - 1;
    }
  }

  if (is_doing_heart_animation)
    goto doing_animation;
  if (link_hearts_filler) {
    if (link_health_current < link_health_capacity) {
      link_health_current += 8;
      if (link_health_current >= link_health_capacity)
        link_health_current = link_health_capacity;

      if (sound_effect_2 == 0)
        sound_effect_2 = 13;

      link_hearts_filler -= 8;
      is_doing_heart_animation++;
      animate_heart_refill_countdown = 7;

doing_animation:
      Hud_Update_IgnoreHealth();
      Hud_AnimateHeartRefill();
      flag_update_hud_in_nmi++;
      return;
    }
    link_health_current = link_health_capacity;
    link_hearts_filler = 0;
  }
  Hud_Update_IgnoreItemBox();
  flag_update_hud_in_nmi++;
}

void Hud_Module_Run() {  // 8ddd36
  byte_7E0206++;
  switch (overworld_map_state) {
  case 0: Hud_ClearTileMap(); break;
  case 1: Hud_Init(); break;
  case 2: Hud_BringMenuDown(); break;
  case 3: Hud_ChooseNextMode(); break;
  case 4: Hud_NormalMenu(); break;
  case 5: Hud_UpdateHud(); break;
  case 6: Hud_CloseMenu(); break;
  case 7: Hud_GotoBottleMenu(); break;
  case 8: Hud_InitBottleMenu(); break;
  case 9: Hud_ExpandBottleMenu(); break;
  case 10: Hud_BottleMenu(); break;
  case 11: Hud_EraseBottleMenu(); break;
  case 12: Hud_RestoreNormalMenu(); break;
  default:
    assert(0);
  }
}

void Hud_ClearTileMap() {  // 8ddd5a
  uint16 *target = (uint16 *)&g_ram[0x1000];
  for (int i = 0; i < 1024; i++)
    target[i] = 0x207f;
  sound_effect_2 = 17;
  nmi_subroutine_index = 1;
  BYTE(nmi_load_target_addr) = 0x22;
  overworld_map_state++;
}

void Hud_Init() {  // 8dddab
  Hud_SearchForEquippedItem();

  Hud_DrawYButtonItems(Hud_GetPaletteMask(1));
  Hud_DrawUnknownBox(Hud_GetPaletteMask(1));

  Hud_DrawAbilityText(Hud_GetPaletteMask(1));
  Hud_DrawAbilityIcons();
  Hud_DrawProgressIcons();
  Hud_DrawMoonPearl();

  Hud_DrawEquipment(Hud_GetPaletteMask(1));
  Hud_DrawShield();
  Hud_DrawArmor();
  Hud_DrawMapAndBigKey();
  Hud_DrawCompass();

  uint8 or_all = 0;
  for (int i = 0; i < 20; i++)
    or_all |= (&link_item_bow)[i];

  if (or_all) {
    uint8 or_bottle = link_bottle_info[0] | link_bottle_info[1] | link_bottle_info[2] | link_bottle_info[3];
    if (or_bottle == 0) {
      link_item_bottles = 0;
    } else if (!link_item_bottles) {
      uint8 bottle_pos = 1;
      if (!link_bottle_info[0]) {
        bottle_pos++;
        if (!link_bottle_info[1]) {
          bottle_pos++;
          if (!link_bottle_info[2])
            bottle_pos++;
        }
      }
      link_item_bottles = bottle_pos;
    }

    if (!Hud_DoWeHaveThisItem())
      Hud_EquipNextItem();

    Hud_DrawSelectedYButtonItem();
    if (hud_cur_item == 16) {
      Hud_DrawBottleMenu(Hud_GetPaletteMask(1));
    }
  }

  timer_for_flashing_circle = 16;
  nmi_subroutine_index = 1;
  BYTE(nmi_load_target_addr) = 0x22;
  overworld_map_state++;
}

void Hud_BringMenuDown() {  // 8dde59
  BG3VOFS_copy2 -= 8;
  if (BG3VOFS_copy2 == 0xff18)
    overworld_map_state++;
}

void Hud_ChooseNextMode() {  // 8dde6e
  uint8 or_all = 0;
  for (int i = 0; i < 20; i++)
    or_all |= (&link_item_bow)[i];

  if (or_all != 0) {
    nmi_subroutine_index = 1;
    BYTE(nmi_load_target_addr) = 0x22;
    if (!Hud_DoWeHaveThisItem())
      Hud_EquipNextItem();

    Hud_DrawSelectedYButtonItem();
    overworld_map_state = 4;
    if (hud_cur_item == 16)
      overworld_map_state = 10;
  } else {
    if (filtered_joypad_H)
      overworld_map_state = 5;
  }
}

bool Hud_DoWeHaveThisItem() {  // 8ddeb0
  return (&link_item_bow)[hud_cur_item - 1] != 0;
}

void Hud_EquipPrevItem() {  // 8dded9
  do {
    Hud_GotoPrevItem();
  } while (!Hud_DoWeHaveThisItem());
}

void Hud_EquipNextItem() {  // 8ddee2
  do {
    Hud_GotoNextItem();
  } while (!Hud_DoWeHaveThisItem());
}

void Hud_EquipItemAbove() {  // 8ddeeb
  do {
    Hud_GotoPrevItem();
    Hud_GotoPrevItem();
    Hud_GotoPrevItem();
    Hud_GotoPrevItem();
    Hud_GotoPrevItem();
  } while (!Hud_DoWeHaveThisItem());
}

void Hud_EquipItemBelow() {  // 8ddf00
  do {
    Hud_GotoNextItem();
    Hud_GotoNextItem();
    Hud_GotoNextItem();
    Hud_GotoNextItem();
    Hud_GotoNextItem();
  } while (!Hud_DoWeHaveThisItem());
}

void Hud_NormalMenu() {  // 8ddf15
  timer_for_flashing_circle++;
  if (!BYTE(joypad1H_last))
    BYTE(tmp1) = 0;

  if (filtered_joypad_H & 0x10) {
    overworld_map_state = 5;
    sound_effect_2 = 18;
    return;
  }

  if (!BYTE(tmp1)) {
    uint16 old_item = hud_cur_item;
    if (filtered_joypad_H & 8) {
      Hud_EquipItemAbove();
    } else if (filtered_joypad_H & 4) {
      Hud_EquipItemBelow();
    } else if (filtered_joypad_H & 2) {
      Hud_EquipPrevItem();
    } else if (filtered_joypad_H & 1) {
      Hud_EquipNextItem();
    }
    BYTE(tmp1) = filtered_joypad_H;
    if (hud_cur_item != old_item) {
      timer_for_flashing_circle = 16;
      sound_effect_2 = 32;
    }
  }
  Hud_DrawYButtonItems(Hud_GetPaletteMask(1));
  Hud_DrawSelectedYButtonItem();
  if (hud_cur_item == 16)
    overworld_map_state = 7;

  nmi_subroutine_index = 1;
  BYTE(nmi_load_target_addr) = 0x22;
  //g_ram[0x15d0] = 0;
}

void Hud_UpdateHud() {  // 8ddfa9
  overworld_map_state++;
  Hud_UpdateOnly();
  Hud_UpdateEquippedItem();
}

void Hud_UpdateEquippedItem() {  // 8ddfaf
  static const uint8 kHudItemToItem[21] = { 0, 3, 2, 14, 1, 10, 5, 6, 15, 16, 17, 9, 4, 8, 7, 12, 11, 18, 13, 19, 20 };
  assert(hud_cur_item < 21);
  eq_selected_y_item = kHudItemToItem[hud_cur_item];
}

void Hud_CloseMenu() {  // 8ddfba
  BG3VOFS_copy2 += 8;
  if (BG3VOFS_copy2)
    return;
  Hud_Rebuild();
  overworld_map_state = 0;
  submodule_index = 0;
  main_module_index = saved_module_for_menu;
  if (submodule_index)
    Hud_RestoreTorchBackground();
  if (eq_selected_y_item != 5 && eq_selected_y_item != 6) {
    eq_debug_variable = 2;
    link_debug_value_1 = 0;
  } else {
    assert(!link_debug_value_1);
    eq_debug_variable = 0;
  }
}

void Hud_GotoBottleMenu() {  // 8ddffb
  byte_7E0205 = 0;
  overworld_map_state++;
}

void Hud_InitBottleMenu() {  // 8de002
  int r = byte_7E0205;
  for (int i = 21; i <= 30; i++)
    uvram_screen.row[11 + r].col[i] = 0x207f;

  if (++byte_7E0205 == 19) {
    overworld_map_state++;
    byte_7E0205 = 17;
  }
  nmi_subroutine_index = 1;
  BYTE(nmi_load_target_addr) = 0x22;
}

void Hud_ExpandBottleMenu() {  // 8de08c
  static const uint16 kBottleMenuTop[] = { 0x28FB, 0x28F9, 0x28F9, 0x28F9, 0x28F9, 0x28F9, 0x28F9, 0x28F9, 0x28F9, 0x68FB };
  static const uint16 kBottleMenuTop2[] = { 0x28FC, 0x24F5, 0x24F5, 0x24F5, 0x24F5, 0x24F5, 0x24F5, 0x24F5, 0x24F5, 0x68FC };
  static const uint16 kBottleMenuBottom[] = { 0xA8FB, 0xA8F9, 0xA8F9, 0xA8F9, 0xA8F9, 0xA8F9, 0xA8F9, 0xA8F9, 0xA8F9, 0xE8FB };

  int r = byte_7E0205;
  for (int i = 0; i < 10; i++)
    uvram_screen.row[11 + r].col[21 + i] = kBottleMenuTop[i];

  for (int i = 0; i < 10; i++)
    uvram_screen.row[12 + r].col[21 + i] = kBottleMenuTop2[i];

  for (int i = 0; i < 10; i++)
    uvram_screen.row[29].col[21 + i] = kBottleMenuBottom[i];

  if (sign8(--byte_7E0205))
    overworld_map_state++;
  nmi_subroutine_index = 1;
  BYTE(nmi_load_target_addr) = 0x22;
}

void Hud_BottleMenu() {  // 8de0df
  timer_for_flashing_circle++;
  if (filtered_joypad_H & 0x10) {
    sound_effect_2 = 18;
    overworld_map_state = 5;
  } else if (filtered_joypad_H & 3) {
    if (filtered_joypad_H & 2) {
      Hud_EquipPrevItem();
    } else {
      Hud_EquipNextItem();
    }
    timer_for_flashing_circle = 16;
    sound_effect_2 = 32;
    Hud_DrawYButtonItems(Hud_GetPaletteMask(1));
    Hud_DrawSelectedYButtonItem();
    overworld_map_state++;
    byte_7E0205 = 0;
    return;
  }
  Hud_UpdateBottleMenu();
  if (filtered_joypad_H & 12) {
    uint8 old_val = link_item_bottles - 1, val = old_val;

    if (filtered_joypad_H & 8) {
      do {
        val = (val - 1) & 3;
      } while (!link_bottle_info[val]);
    } else {
      do {
        val = (val + 1) & 3;
      } while (!link_bottle_info[val]);
    }
    if (old_val != val) {
      link_item_bottles = val + 1;
      timer_for_flashing_circle = 16;
      sound_effect_2 = 32;
    }
  }
}

void Hud_UpdateBottleMenu() {  // 8de17f

  for (int y = 12; y <= 28; y++)
    for (int x = 0; x < 8; x++)
      uvram_screen.row[y].col[22 + x] = 0x24f5;

  Hud_DrawItem(0x1372, kHudItemBottles[link_bottle_info[0]]);
  Hud_DrawItem(0x1472, kHudItemBottles[link_bottle_info[1]]);
  Hud_DrawItem(0x1572, kHudItemBottles[link_bottle_info[2]]);
  Hud_DrawItem(0x1672, kHudItemBottles[link_bottle_info[3]]);
  Hud_DrawItem(0x1408, kHudItemBottles[link_item_bottles ? link_bottle_info[link_item_bottles - 1] : 0]);

  uint16 *p = (uint16 *)&g_ram[kHudItemInVramPtr[hud_cur_item - 1]];
  uvram_screen.row[6].col[25] = p[0];
  uvram_screen.row[6].col[26] = p[1];
  uvram_screen.row[7].col[25] = p[32];
  uvram_screen.row[7].col[26] = p[33];

  if (timer_for_flashing_circle & 0x10) {
    int o = ((link_item_bottles - 1) * 0x100 + 0x88) / 2;

    uvram_screen.row[10].col[21 + o] = 0x3C61;
    uvram_screen.row[10].col[22 + o] = 0x3C61 | 0x4000;

    uvram_screen.row[11].col[20 + o] = 0x3C70;
    uvram_screen.row[11].col[23 + o] = 0x3C70 | 0x4000;

    uvram_screen.row[12].col[20 + o] = 0xBC70;
    uvram_screen.row[12].col[23 + o] = 0xBC70 | 0x4000;

    uvram_screen.row[13].col[21 + o] = 0xBC61;
    uvram_screen.row[13].col[22 + o] = 0xBC61 | 0x4000;

    uvram_screen.row[10].col[20 + o] = 0x3C60;
    uvram_screen.row[10].col[23 + o] = 0x3C60 | 0x4000;

    uvram_screen.row[13].col[23 + o] = 0x3C60 | 0xC000;
    uvram_screen.row[13].col[20 + o] = 0x3C60 | 0x8000;
  }

  if (link_item_bottles) {
    const uint16 *src = kHudBottlesGfx + (link_bottle_info[link_item_bottles - 1] - 1) * 16;
    for (int i = 0; i < 8; i++) {
      uvram_screen.row[8].col[22 + i] = src[i];
      uvram_screen.row[9].col[22 + i] = src[i + 8];
    }
  }

  nmi_subroutine_index = 1;
  BYTE(nmi_load_target_addr) = 0x22;
}

void Hud_EraseBottleMenu() {  // 8de2fd
  int r = byte_7E0205;
  for (int i = 0; i < 10; i++)
    uvram_screen.row[11 + r].col[21 + i] = 0x207f;
  if (++byte_7E0205 == 19)
    overworld_map_state++;
  nmi_subroutine_index = 1;
  BYTE(nmi_load_target_addr) = 0x22;
}

void Hud_RestoreNormalMenu() {  // 8de346
  Hud_DrawProgressIcons();
  Hud_DrawMoonPearl();
  Hud_DrawEquipment(Hud_GetPaletteMask(1));
  Hud_DrawShield();
  Hud_DrawArmor();
  Hud_DrawMapAndBigKey();
  Hud_DrawCompass();

  overworld_map_state = 4;
  nmi_subroutine_index = 1;
  BYTE(nmi_load_target_addr) = 0x22;
}

void Hud_DrawItem(uint16 a, const uint16 *src) {  // 8de372
  uint16 *dst = (uint16 *)&g_ram[a];
  dst[0] = src[0];
  dst[1] = src[1];
  dst[32] = src[2];
  dst[33] = src[3];
}

void Hud_SearchForEquippedItem() {  // 8de399
  uint8 or_all = 0;
  for (int i = 0; i < 20; i++)
    or_all |= (&link_item_bow)[i];

  if (or_all == 0) {
    hud_cur_item = 0;
    hud_cur_item_hi = 0;
    hud_var1 = 0;
  } else {
    if (!hud_cur_item)
      hud_cur_item = 1;
    if (!Hud_DoWeHaveThisItem())
      Hud_EquipNextItem();
  }
}

uint16 Hud_GetPaletteMask(uint8 what) {  // 8de3c8
  return what == 0 ? 0xe3ff : 0xffff;
}

void Hud_DrawYButtonItems(uint16 mask) {  // 8de3d9
  uint16 t;

  t = 0x3CFB & mask;
  uvram_screen.row[5].col[1] = t;
  uvram_screen.row[19].col[1] = (t |= 0x8000);
  uvram_screen.row[19].col[19] = (t |= 0x4000);
  uvram_screen.row[5].col[19] = (t ^= 0x8000);

  for (int i = 6; i < 19; i++) {
    uvram_screen.row[i].col[1] = (t = 0x3cfc & mask);
    uvram_screen.row[i].col[19] = (t |= 0x4000);
  }

  for (int i = 2; i < 19; i++) {
    uvram_screen.row[5].col[i] = (t = 0x3CF9 & mask);
    uvram_screen.row[19].col[i] = (t |= 0x8000);
  }

  for (int y = 6; y < 19; y++) {
    for (int x = 2; x < 19; x++)
      uvram_screen.row[y].col[x] = 0x24F5;
  }
  uvram_screen.row[6].col[2] = 0x3CF0;
  uvram_screen.row[7].col[2] = 0x3CF1;
  uvram_screen.row[5].col[3] = 0x246E;
  uvram_screen.row[5].col[4] = 0x246F;

  Hud_DrawItem(0x11c8, kHudItemBow[link_item_bow]);
  Hud_DrawItem(0x11ce, kHudItemBoomerang[link_item_boomerang]);
  Hud_DrawItem(0x11d4, kHudItemHookshot[link_item_hookshot]);
  Hud_DrawItem(0x11da, kHudItemBombs[link_item_bombs ? 1 : 0]);
  Hud_DrawItem(0x11e0, kHudItemMushroom[link_item_mushroom]);
  Hud_DrawItem(0x1288, kHudItemFireRod[link_item_fire_rod]);
  Hud_DrawItem(0x128e, kHudItemIceRod[link_item_ice_rod]);
  Hud_DrawItem(0x1294, kHudItemBombos[link_item_bombos_medallion]);
  Hud_DrawItem(0x129a, kHudItemEther[link_item_ether_medallion]);
  Hud_DrawItem(0x12a0, kHudItemQuake[link_item_quake_medallion]);
  Hud_DrawItem(0x1348, kHudItemTorch[link_item_torch]);
  Hud_DrawItem(0x134e, kHudItemHammer[link_item_hammer]);
  Hud_DrawItem(0x1354, kHudItemFlute[link_item_flute]);
  Hud_DrawItem(0x135a, kHudItemBugNet[link_item_bug_net]);
  Hud_DrawItem(0x1360, kHudItemBookMudora[link_item_book_of_mudora]);
  Hud_DrawItem(0x1408, kHudItemBottles[link_item_bottles ? link_bottle_info[link_item_bottles - 1] : 0]);
  Hud_DrawItem(0x140e, kHudItemCaneSomaria[link_item_cane_somaria]);
  Hud_DrawItem(0x1414, kHudItemCaneByrna[link_item_cane_byrna]);
  Hud_DrawItem(0x141a, kHudItemCape[link_item_cape]);
  Hud_DrawItem(0x1420, kHudItemMirror[link_item_mirror]);
}

void Hud_DrawUnknownBox(uint16 palmask) {  // 8de647
  uint16 t;

  t = 0x3CFB & palmask;
  uvram_screen.row[5].col[21] = t;
  uvram_screen.row[10].col[21] = (t |= 0x8000);
  uvram_screen.row[10].col[30] = (t |= 0x4000);
  uvram_screen.row[5].col[30] = (t ^= 0x8000);

  t = 0x3CFC & palmask;
  for (int i = 0; i < 4; i++) {
    uvram_screen.row[6 + i].col[21] = t;
    uvram_screen.row[6 + i].col[30] = t | 0x4000;
  }

  t = 0x3CF9 & palmask;
  for (int i = 0; i < 8; i++) {
    uvram_screen.row[5].col[22 + i] = t;
    uvram_screen.row[10].col[22 + i] = t | 0x8000;
  }

  for (int i = 0; i < 8; i++) {
    uvram_screen.row[6].col[22 + i] = 0x24F5;
    uvram_screen.row[7].col[22 + i] = 0x24F5;
    uvram_screen.row[8].col[22 + i] = 0x24F5;
    uvram_screen.row[9].col[22 + i] = 0x24F5;
  }
}

void Hud_DrawAbilityText(uint16 palmask) {  // 8de6b6
  for (int i = 0; i < 17; i++) {
    uvram_screen.row[22].col[2 + i] = 0x24F5;
    uvram_screen.row[23].col[2 + i] = 0x24F5;
    uvram_screen.row[24].col[2 + i] = 0x24F5;
    uvram_screen.row[25].col[2 + i] = 0x24F5;
    uvram_screen.row[26].col[2 + i] = 0x24F5;
    uvram_screen.row[27].col[2 + i] = 0x24F5;
    uvram_screen.row[28].col[2 + i] = 0x24F5;
  }

  uint8 flags = link_ability_flags;
  const uint16 *src = kHudAbilityText;
  uint16 *dst = &uvram_screen.row[22].col[4];
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 4; j++) {
      if (flags & 0x80) {
        dst[0] = src[0];
        dst[1] = src[1];
        dst[2] = src[2];
        dst[3] = src[3];
        dst[4] = src[4];
        dst[32 + 0] = src[5];
        dst[32 + 1] = src[6];
        dst[32 + 2] = src[7];
        dst[32 + 3] = src[8];
        dst[32 + 4] = src[9];
      }
      src += 10;
      dst += 5;
      flags <<= 1;
    }
    dst += 2 * 32 - 5 * 4;
  }

  uint16 t;

  t = 0x24FB & palmask;
  uvram_screen.row[21].col[1] = t;
  uvram_screen.row[29].col[1] = (t |= 0x8000);
  uvram_screen.row[29].col[19] = (t |= 0x4000);
  uvram_screen.row[21].col[19] = (t ^= 0x8000);

  t = 0x24FC & palmask;
  for (int i = 0; i < 7; i++) {
    uvram_screen.row[22 + i].col[1] = t;
    uvram_screen.row[22 + i].col[19] = t | 0x4000;
  }

  t = 0x24F9 & palmask;
  for (int i = 0; i < 17; i++) {
    uvram_screen.row[21].col[2 + i] = t;
    uvram_screen.row[29].col[2 + i] = t | 0x8000;
  }

  uvram_screen.row[22].col[2] = 0xA4F0;
  uvram_screen.row[23].col[2] = 0x24F2;
  uvram_screen.row[21].col[3] = 0x2482;
  uvram_screen.row[21].col[4] = 0x2483;
}

void Hud_DrawAbilityIcons() {  // 8de7b7
  Hud_DrawItem(0x16D0, kHudItemGloves[link_item_gloves]);
  Hud_DrawItem(0x16C8, kHudItemBoots[link_item_boots]);
  Hud_DrawItem(0x16D8, kHudItemFlippers[link_item_flippers]);
  if (link_item_gloves)
    Hud_DrawGlovesText(link_item_gloves != 1);
}

void Hud_DrawGlovesText(uint8 idx) {  // 8de81a
  const uint16 *src = kHudGlovesText + idx * 10;
  uint16 *dst = &uvram_screen.row[22].col[4];
  memcpy(dst, src, sizeof(uint16) * 5);
  memcpy(dst + 32, src + 5, sizeof(uint16) * 5);
}

void Hud_DrawProgressIcons() {  // 8de9c8
  if (sram_progress_indicator < 3)
    Hud_DrawProgressIcons_Pendants();
  else
    Hud_DrawProgressIcons_Crystals();
}

void Hud_DrawProgressIcons_Pendants() {  // 8de9d3
  const uint16 *src = kProgressIconPendantsBg;
  for (int y = 0; y < 9; y++) {
    memcpy(&uvram_screen.row[11 + y].col[21], src, sizeof(uint16) * 10);
    src += 10;
  }

  Hud_DrawItem(0x13B2, kHudPendants0[(link_which_pendants >> 0) & 1]);
  Hud_DrawItem(0x146E, kHudPendants1[(link_which_pendants >> 1) & 1]);
  Hud_DrawItem(0x1476, kHudPendants2[(link_which_pendants >> 2) & 1]);
}

void Hud_DrawProgressIcons_Crystals() {  // 8dea62
  const uint16 *src = kProgressIconCrystalsBg;
  for (int y = 0; y < 9; y++, src += 10)
    memcpy(&uvram_screen.row[11 + y].col[21], src, sizeof(uint16) * 10);

  uint8 f = link_has_crystals;
  if (f & 1) {
    uvram_screen.row[14].col[24] = 0x2D44;
    uvram_screen.row[14].col[25] = 0x2D45;
  }
  if (f & 2) {
    uvram_screen.row[14].col[26] = 0x2D44;
    uvram_screen.row[14].col[27] = 0x2D45;
  }
  if (f & 4) {
    uvram_screen.row[16].col[23] = 0x2D44;
    uvram_screen.row[16].col[24] = 0x2D45;
  }
  if (f & 8) {
    uvram_screen.row[16].col[25] = 0x2D44;
    uvram_screen.row[16].col[26] = 0x2D45;
  }
  if (f & 16) {
    uvram_screen.row[16].col[27] = 0x2D44;
    uvram_screen.row[16].col[28] = 0x2D45;
  }
  if (f & 32) {
    uvram_screen.row[18].col[24] = 0x2D44;
    uvram_screen.row[18].col[25] = 0x2D45;
  }
  if (f & 64) {
    uvram_screen.row[18].col[26] = 0x2D44;
    uvram_screen.row[18].col[27] = 0x2D45;
  }
}

void Hud_DrawSelectedYButtonItem() {  // 8deb3a
  uint16 *p = (uint16 *)&g_ram[kHudItemInVramPtr[hud_cur_item - 1]];
  uvram_screen.row[6].col[25] = p[0];
  uvram_screen.row[6].col[26] = p[1];
  uvram_screen.row[7].col[25] = p[32];
  uvram_screen.row[7].col[26] = p[33];

  if (timer_for_flashing_circle & 0x10) {
    p[-32] = 0x3C61;
    p[-31] = 0x3C61 | 0x4000;

    p[-1] = 0x3C70;
    p[2] = 0x3C70 | 0x4000;

    p[31] = 0xBC70;
    p[34] = 0xBC70 | 0x4000;

    p[64] = 0xBC61;
    p[65] = 0xBC61 | 0x4000;

    p[-33] = 0x3C60;
    p[-30] = 0x3C60 | 0x4000;

    p[66] = 0x3C60 | 0xC000;
    p[63] = 0x3C60 | 0x8000;
  }

  const uint16 *src_p;

  if (hud_cur_item == 16 && link_item_bottles) {
    src_p = &kHudBottlesItemText[(link_bottle_info[link_item_bottles - 1] - 1) * 16];
  } else if (hud_cur_item == 5 && link_item_mushroom != 1) {
    src_p = &kHudMushroomItemText[(link_item_mushroom - 2) * 16];
  } else if (hud_cur_item == 20 && link_item_mirror != 1) {
    src_p = &kHudMirrorItemText[(link_item_mirror - 2) * 16];
  } else if (hud_cur_item == 13 && link_item_flute != 1) {
    src_p = &kHudFluteItemText[(link_item_flute - 2) * 16];
  } else if (hud_cur_item == 1 && link_item_bow != 1) {
    src_p = &kHudBowItemText[(link_item_bow - 2) * 16];
  } else {
    src_p = &kHudItemText[(hud_cur_item - 1) * 16];
  }
  memcpy(&uvram_screen.row[8].col[22], src_p + 0, sizeof(uint16) * 8);
  memcpy(&uvram_screen.row[9].col[22], src_p + 8, sizeof(uint16) * 8);
}

void Hud_DrawMoonPearl() {  // 8dece9
  Hud_DrawItem(0x16e0, kHudItemMoonPearl[link_item_moon_pearl]);
}

void Hud_DrawEquipment(uint16 palmask) {  // 8ded29
  uint16 t = palmask & 0x28FB;
  uvram_screen.row[21].col[21] = t | 0x0000;
  uvram_screen.row[29].col[21] = t | 0x8000;
  uvram_screen.row[29].col[30] = t | 0xC000;
  uvram_screen.row[21].col[30] = t | 0x4000;

  t = 0x28FC & palmask;
  for (int i = 0; i < 7; i++) {
    uvram_screen.row[22 + i].col[21] = t;
    uvram_screen.row[22 + i].col[30] = t | 0x4000;
  }

  t = 0x28F9 & palmask;
  for (int i = 0; i < 8; i++) {
    uvram_screen.row[21].col[22 + i] = t;
    uvram_screen.row[29].col[22 + i] = t | 0x8000;
  }

  for (int i = 0; i < 7; i++) {
    for (int j = 0; j < 8; j++)
      uvram_screen.row[22 + i].col[22 + j] = 0x24F5;
  }

  // Draw dotted lines
  t = 0x28D7 & palmask;
  for (int i = 0; i < 8; i++)
    uvram_screen.row[25].col[22 + i] = t;

  static const uint16 kHudEquipmentDungeonItemText[16] = {
    0x2479, 0x247a, 0x247b, 0x247c, 0x248c, 0x24f5, 0x24f5, 0x24f5,
    0x2469, 0x246a, 0x246b, 0x246c, 0x246d, 0x246e, 0x246f, 0x24f5,
  };

  for (int i = 0; i < 8; i++) {
    uvram_screen.row[22].col[22 + i] = kHudEquipmentDungeonItemText[i + 0] & palmask;
    uvram_screen.row[26].col[22 + i] = kHudEquipmentDungeonItemText[i + 8] & palmask;
  }
  if (cur_palace_index_x2 == 0xff) {
    for (int i = 0; i < 8; i++)
      uvram_screen.row[26].col[22 + i] = 0x24F5;
    Hud_DrawItem(0x16f2, kHudItemHeartPieces[link_heart_pieces]);
  }
  Hud_DrawItem(0x15ec, kHudItemSword[link_sword_type == 0xff ? 0 : link_sword_type]);
}

void Hud_DrawShield() {  // 8dee21
  Hud_DrawItem(0x15f2, kHudItemShield[link_shield_type]);
}

void Hud_DrawArmor() {  // 8dee3c
  Hud_DrawItem(0x15f8, kHudItemArmor[link_armor]);
}

void Hud_DrawMapAndBigKey() {  // 8dee57
  if (cur_palace_index_x2 != 0xff &&
      (link_bigkey << (cur_palace_index_x2 >> 1)) & 0x8000) {
    Hud_DrawItem(0x16F8, kHudItemPalaceItem[CheckPalaceItemPosession() + 1]);
  }
  if (cur_palace_index_x2 != 0xff &&
      (link_dungeon_map << (cur_palace_index_x2 >> 1)) & 0x8000) {
    Hud_DrawItem(0x16EC, kHudItemDungeonMap[1]);
  }
}

void Hud_DrawCompass() {  // 8def39
  if (cur_palace_index_x2 != 0xff &&
      (link_compass << (cur_palace_index_x2 >> 1)) & 0x8000) {
    Hud_DrawItem(0x16F2, kHudItemDungeonCompass[1]);
  }
}

void Hud_DrawBottleMenu(uint16 palmask) {  // 8def67
  uint16 t = 0x28FB & palmask;
  uvram_screen.row[11].col[21] = t;
  uvram_screen.row[29].col[21] = t | 0x8000;
  uvram_screen.row[29].col[30] = t | 0xC000;
  uvram_screen.row[11].col[30] = t | 0x4000;

  t = 0x28FC & palmask;
  for (int i = 0; i < 17; i++) {
    uvram_screen.row[12 + i].col[21] = t;
    uvram_screen.row[12 + i].col[30] = t | 0x4000;
  }
  t = 0x28F9 & palmask;
  for (int i = 0; i < 8; i++) {
    uvram_screen.row[11].col[22 + i] = t;
    uvram_screen.row[29].col[22 + i] = t | 0x8000;
  }
  for (int y = 12; y <= 28; y++)
    for (int x = 0; x < 8; x++)
      uvram_screen.row[y].col[22 + x] = 0x24f5;

  Hud_DrawItem(0x1372, kHudItemBottles[link_bottle_info[0]]);
  Hud_DrawItem(0x1472, kHudItemBottles[link_bottle_info[1]]);
  Hud_DrawItem(0x1572, kHudItemBottles[link_bottle_info[2]]);
  Hud_DrawItem(0x1672, kHudItemBottles[link_bottle_info[3]]);
  Hud_DrawItem(0x1408, kHudItemBottles[link_bottle_info[link_item_bottles - 1]]);

  uint16 *p = (uint16 *)&g_ram[kHudItemInVramPtr[hud_cur_item - 1]];

  uvram_screen.row[6].col[25] = p[0];
  uvram_screen.row[6].col[26] = p[1];
  uvram_screen.row[7].col[25] = p[32];
  uvram_screen.row[7].col[26] = p[33];

  int o = ((link_item_bottles - 1) * 0x100 + 0x88) / 2;

  uvram_screen.row[10].col[21 + o] = 0x3C61;
  uvram_screen.row[10].col[22 + o] = 0x3C61 | 0x4000;

  uvram_screen.row[11].col[20 + o] = 0x3C70;
  uvram_screen.row[11].col[23 + o] = 0x3C70 | 0x4000;

  uvram_screen.row[12].col[20 + o] = 0xBC70;
  uvram_screen.row[12].col[23 + o] = 0xBC70 | 0x4000;

  uvram_screen.row[13].col[21 + o] = 0xBC61;
  uvram_screen.row[13].col[22 + o] = 0xBC61 | 0x4000;

  uvram_screen.row[10].col[20 + o] = 0x3C60;
  uvram_screen.row[10].col[23 + o] = 0x3C60 | 0x4000;

  uvram_screen.row[13].col[23 + o] = 0x3C60 | 0xC000;
  uvram_screen.row[13].col[20 + o] = 0x3C60 | 0x8000;

  timer_for_flashing_circle = 16;
}

void Hud_IntToDecimal(unsigned int number, uint8 *out) {  // 8df0f7
  out[0] = number / 100 + 0x90;
  out[1] = (number %= 100) / 10 + 0x90;
  out[2] = (number % 10) + 0x90;
}

bool Hud_RefillHealth() {  // 8df128
  if (link_health_current >= link_health_capacity) {
    link_health_current = link_health_capacity;
    link_hearts_filler = 0;
    return (is_doing_heart_animation == 0);
  }
  link_hearts_filler = 160;
  return false;
}

void Hud_AnimateHeartRefill() {  // 8df14f
  if (--animate_heart_refill_countdown)
    return;
  uint16 n = ((uint16)((link_health_current & ~7) - 1) >> 3) << 1;
  uint16 *p = hud_tile_indices_buffer + 0x34;
  if (n >= 20) {
    n -= 20;
    p += 0x20;
  }
  n &= 0xff;
  animate_heart_refill_countdown = 1;

  static const uint16 kAnimHeartPartial[4] = { 0x24A3, 0x24A4, 0x24A3, 0x24A0 };
  p[n >> 1] = kAnimHeartPartial[animate_heart_refill_countdown_subpos];

  animate_heart_refill_countdown_subpos = (animate_heart_refill_countdown_subpos + 1) & 3;
  if (!animate_heart_refill_countdown_subpos) {
    Hud_Rebuild();
    is_doing_heart_animation = 0;
  }
}

bool Hud_RefillMagicPower() {  // 8df1b3
  if (link_magic_power >= 0x80)
    return true;
  link_magic_filler = 0x80;
  return false;
}

void Hud_RestoreTorchBackground() {  // 8dfa33
  if (!link_item_torch || !dung_want_lights_out || hdr_dungeon_dark_with_lantern ||
      dung_num_lit_torches)
    return;
  hdr_dungeon_dark_with_lantern = 1;
  if (dung_hdr_bg2_properties != 2)
    TS_copy = 1;
}

void Hud_RebuildIndoor() {  // 8dfa60
  overworld_fixed_color_plusminus = 0;
  link_num_keys = 0xff;
  Hud_Rebuild();
}

void Hud_Rebuild() {  // 8dfa70
  memcpy(hud_tile_indices_buffer, kHudTilemap, 165 * sizeof(uint16));
  Hud_UpdateInternal();
  flag_update_hud_in_nmi++;
}

void Hud_UpdateOnly() {  // 8dfa85
  Hud_UpdateInternal();
  flag_update_hud_in_nmi++;
}

void Hud_UpdateItemBox() {  // 8dfafd
  if (link_item_bow) {
    if (link_item_bow >= 3) {
      hud_tile_indices_buffer[15] = 0x2486;
      hud_tile_indices_buffer[16] = 0x2487;
      link_item_bow = link_num_arrows ? 4 : 3;
    } else {
      link_item_bow = link_num_arrows ? 2 : 1;
    }
  }

  if (!hud_cur_item)
    return;

  uint8 item_val = (&link_item_bow)[hud_cur_item - 1];
  if (hud_cur_item == 4)
    item_val = 1;
  else if (hud_cur_item == 16)
    item_val = link_bottle_info[item_val - 1];

  const uint16 *p = kHudItemBoxGfxPtrs[hud_cur_item - 1][0] + item_val * 4;

  hud_tile_indices_buffer[37] = p[0];
  hud_tile_indices_buffer[38] = p[1];
  hud_tile_indices_buffer[37 + 32] = p[2];
  hud_tile_indices_buffer[38 + 32] = p[3];
}

void Hud_UpdateInternal() {  // 8dfb91
  Hud_UpdateItemBox();
  Hud_Update_IgnoreItemBox();
}

void Hud_Update_IgnoreItemBox() {  // 8dfb94
  static const uint16 kHudItemBoxTab1[] = { 0x24A2, 0x24A2, 0x24A2 };
  static const uint16 kHudItemBoxTab2[] = { 0x24A2, 0x24A1, 0x24A0 };

  Hud_UpdateHearts(&hud_tile_indices_buffer[0x34], kHudItemBoxTab1, link_health_capacity);
  Hud_UpdateHearts(&hud_tile_indices_buffer[0x34], kHudItemBoxTab2, (link_health_current + 3) & ~3);

  Hud_Update_IgnoreHealth();
}

void Hud_Update_IgnoreHealth() {  // 8dfc09
  if (link_magic_consumption >= 1) {
    hud_tile_indices_buffer[2] = 0x28F7;
    hud_tile_indices_buffer[3] = 0x2851;
    hud_tile_indices_buffer[4] = 0x28FA;
  }

  const uint16 *src = kUpdateMagicPowerTilemap[(link_magic_power + 7) >> 3];
  hud_tile_indices_buffer[0x23] = src[0];
  hud_tile_indices_buffer[0x43] = src[1];
  hud_tile_indices_buffer[0x63] = src[2];
  hud_tile_indices_buffer[0x83] = src[3];

  uint8 d[3];

  Hud_IntToDecimal(link_rupees_actual, d);
  hud_tile_indices_buffer[0x28] = 0x2400 | d[0];
  hud_tile_indices_buffer[0x29] = 0x2400 | d[1];
  hud_tile_indices_buffer[0x2A] = 0x2400 | d[2];

  Hud_IntToDecimal(link_item_bombs, d);
  hud_tile_indices_buffer[0x2C] = 0x2400 | d[1];
  hud_tile_indices_buffer[0x2D] = 0x2400 | d[2];

  Hud_IntToDecimal(link_num_arrows, d);
  hud_tile_indices_buffer[0x2F] = 0x2400 | d[1];
  hud_tile_indices_buffer[0x30] = 0x2400 | d[2];

  d[2] = 0x7f;
  if (link_num_keys != 0xff)
    Hud_IntToDecimal(link_num_keys, d);
  hud_tile_indices_buffer[0x32] = 0x2400 | d[2];
  if (hud_tile_indices_buffer[0x32] == 0x247f)
    hud_tile_indices_buffer[0x12] = hud_tile_indices_buffer[0x32];
}

void Hud_UpdateHearts(uint16 *dst, const uint16 *src, int n) {  // 8dfdab
  int x = 0;

  while (n > 0) {
    if (x >= 10) {
      dst += 0x20;
      x = 0;
    }
    dst[x] = src[n >= 5 ? 2 : 1];
    x++;
    n -= 8;
  }
}

