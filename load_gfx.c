#include "zelda_rtl.h"
#include "variables.h"
#include "snes/snes_regs.h"
#include "overworld.h"
#include "load_gfx.h"
#include "player.h"
#include "tables/generated_images.h"
#include "tables/generated_font.h"
#include "tables/generated_palettes.h"
#include "sprite.h"

static const uint16 kGlovesColor[2] = {0x52f6, 0x376};
static const uint8 kGraphics_IncrementalVramUpload_Dst[16] = {0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f};
static const uint8 kGraphics_IncrementalVramUpload_Src[16] = {0x0, 0x2, 0x4, 0x6, 0x8, 0xa, 0xc, 0xe, 0x10, 0x12, 0x14, 0x16, 0x18, 0x1a, 0x1c, 0x1e};
static const uint16 kPaletteFilteringBits[64] = {
  0xffff, 0xffff, 0xfffe, 0xffff, 0x7fff, 0x7fff, 0x7fdf, 0xfbff, 0x7f7f, 0x7f7f, 0x7df7, 0xefbf, 0x7bdf, 0x7bdf, 0x77bb, 0xddef,
  0x7777, 0x7777, 0x6edd, 0xbb77, 0x6db7, 0x6db7, 0x5b6d, 0xb6db, 0x5b5b, 0x5b5b, 0x56b6, 0xad6b, 0x5555, 0xad6b, 0x5555, 0xaaab,
  0x5555, 0x5555, 0x2a55, 0x5555, 0x2a55, 0x2a55, 0x294a, 0x5295, 0x2525, 0x2525, 0x2492, 0x4925, 0x1249, 0x1249, 0x1122, 0x4489,
  0x1111, 0x1111,  0x844, 0x2211,  0x421,  0x421,  0x208, 0x1041,  0x101,  0x101,   0x20,  0x401,      1,      1,      0,      1,
};
static const uint16 kPaletteFilter_Agahnim_Tab[3] = {0x160, 0x180, 0x1a0};
static const uint8 kMainTilesets[37][8] = {
  {  0,   1,  16,   6, 14, 31, 24, 15},
  {  0,   1,  16,   8, 14, 34, 27, 15},
  {  0,   1,  16,   6, 14, 31, 24, 15},
  {  0,   1,  19,   7, 14, 35, 28, 15},
  {  0,   1,  16,   7, 14, 33, 24, 15},
  {  0,   1,  16,   9, 14, 32, 25, 15},
  {  2,   3,  18,  11, 14, 33, 26, 15},
  {  0,   1,  17,  12, 14, 36, 27, 15},
  {  0,   1,  17,   8, 14, 34, 27, 15},
  {  0,   1,  17,  12, 14, 37, 26, 15},
  {  0,   1,  17,  12, 14, 38, 27, 15},
  {  0,   1,  20,  10, 14, 39, 29, 15},
  {  0,   1,  17,  10, 14, 40, 30, 15},
  {  2,   3,  18,  11, 14, 41, 22, 15},
  {  0,   1,  21,  13, 14, 42, 24, 15},
  {  0,   1,  16,   7, 14, 35, 28, 15},
  {  0,   1,  19,   7, 14,  4,  5, 15},
  {  0,   1,  19,   7, 14,  4,  5, 15},
  {  0,   1,  16,   9, 14, 32, 27, 15},
  {  0,   1,  16,   9, 14, 42, 23, 15},
  {  2,   3,  18,  11, 14, 33, 28, 15},
  {  0,   8,  17,  27, 34, 46, 93, 91},
  {  0,   8,  16,  24, 32, 43, 93, 91},
  {  0,   8,  16,  24, 32, 43, 93, 91},
  { 58,  59,  60,  61, 83, 77, 62, 91},
  { 66,  67,  68,  69, 32, 43, 63, 93},
  {  0,   8,  16,  24, 32, 43, 93, 91},
  {  0,   8,  16,  24, 32, 43, 93, 91},
  {  0,   8,  16,  24, 32, 43, 93, 91},
  {  0,   8,  16,  24, 32, 43, 93, 91},
  {  0,   8,  16,  24, 32, 43, 93, 91},
  {113, 114, 113, 114, 32, 43, 93, 91},
  { 58,  59,  60,  61, 83, 77, 62, 91},
  { 66,  67,  68,  69, 32, 43, 63, 89},
  {  0, 114, 113, 114, 32, 43, 93, 15},
  { 22,  57,  29,  23, 64, 65, 57, 30},
  {  0,  70,  57, 114, 64, 65, 57, 15},
};
static const uint8 kSpriteTilesets[144][4] = {
  { 0, 73,   0,  0},
  {70, 73,  12, 29},
  {72, 73,  19, 29},
  {70, 73,  19, 14},
  {72, 73,  12, 17},
  {72, 73,  12, 16},
  {79, 73,  74, 80},
  {14, 73,  74, 17},
  {70, 73,  18,  0},
  { 0, 73,   0, 80},
  { 0, 73,   0, 17},
  {72, 73,  12,  0},
  { 0,  0,  55, 54},
  {72, 73,  76, 17},
  {93, 44,  12, 68},
  { 0,  0,  78,  0},
  {15,  0,  18, 16},
  { 0,  0,   0, 76},
  { 0, 13,  23,  0},
  {22, 13,  23, 27},
  {22, 13,  23, 20},
  {21, 13,  23, 21},
  {22, 13,  24, 25},
  {22, 13,  23, 25},
  {22, 13,   0,  0},
  {22, 13,  24, 27},
  {15, 73,  74, 17},
  {75, 42,  92, 21},
  {22, 73,  23, 29},
  { 0,  0,   0, 21},
  {22, 13,  23, 16},
  {22, 73,  18,  0},
  {22, 73,  12, 17},
  { 0,  0,  18, 16},
  {22, 13,   0, 17},
  {22, 73,  12,  0},
  {22, 13,  76, 17},
  {14, 13,  74, 17},
  {22, 26,  23, 27},
  {79, 52,  74, 80},
  {53, 77, 101, 54},
  {74, 52,  78,  0},
  {14, 52,  74, 17},
  {81, 52,  93, 89},
  {75, 73,  76, 17},
  {45,  0,   0,  0},
  {93,  0,  18, 89},
  { 0,  0,   0,  0},
  { 0,  0,   0,  0},
  { 0,  0,   0,  0},
  { 0,  0,   0,  0},
  { 0,  0,   0,  0},
  { 0,  0,   0,  0},
  { 0,  0,   0,  0},
  { 0,  0,   0,  0},
  { 0,  0,   0,  0},
  { 0,  0,   0,  0},
  { 0,  0,   0,  0},
  { 0,  0,   0,  0},
  { 0,  0,   0,  0},
  { 0,  0,   0,  0},
  { 0,  0,   0,  0},
  { 0,  0,   0,  0},
  { 0,  0,   0,  0},
  {71, 73,  43, 45},
  {70, 73,  28, 82},
  { 0, 73,  28, 82},
  {93, 73,   0, 82},
  {70, 73,  19, 82},
  {75, 77,  74, 90},
  {71, 73,  28, 82},
  {75, 77,  57, 54},
  {31, 44,  46, 82},
  {31, 44,  46, 29},
  {47, 44,  46, 82},
  {47, 44,  46, 49},
  {31, 30,  48, 82},
  {81, 73,  19,  0},
  {79, 73,  19, 80},
  {79, 77,  74, 80},
  {75, 73,  76, 43},
  {31, 32,  34, 83},
  {85, 61,  66, 67},
  {31, 30,  35, 82},
  {31, 30,  57, 58},
  {31, 30,  58, 62},
  {31, 30,  60, 61},
  {64, 30,  39, 63},
  {85, 26,  66, 67},
  {31, 30,  42, 82},
  {31, 30,  56, 82},
  {31, 32,  40, 82},
  {31, 32,  38, 82},
  {31, 44,  37, 82},
  {31, 32,  39, 82},
  {31, 30,  41, 82},
  {31, 44,  59, 82},
  {70, 73,  36, 82},
  {33, 65,  69, 51},
  {31, 44,  40, 49},
  {31, 13,  41, 82},
  {31, 30,  39, 82},
  {31, 32,  39, 83},
  {72, 73,  19, 82},
  {14, 30,  74, 80},
  {31, 32,  38, 83},
  {21,  0,   0,  0},
  {31,  0,  42, 82},
  { 0,  0,   0,  0},
  { 0,  0,   0,  0},
  { 0,  0,   0,  0},
  { 0,  0,   0,  0},
  { 0,  0,   0,  0},
  { 0,  0,   0,  0},
  { 0,  0,   0,  0},
  { 0,  0,   0,  0},
  { 0,  0,   0,  0},
  { 0,  0,   0,  0},
  { 0,  0,   0,  0},
  { 0,  0,   0,  0},
  { 0,  0,   0,  0},
  { 0,  0,   0,  0},
  { 0,  0,   0,  0},
  { 0,  0,   0,  0},
  { 0,  0,   0,  0},
  {50,  0,   0,  8},
  {93, 73,   0, 82},
  {85, 73,  66, 67},
  {97, 98,  99, 80},
  {97, 98,  99, 80},
  {97, 98,  99, 80},
  {97, 98,  99, 80},
  {97, 98,  99, 80},
  {97, 98,  99, 80},
  {97, 86,  87, 80},
  {97, 98,  99, 80},
  {97, 98,  99, 80},
  {97, 86,  87, 80},
  {97, 86,  99, 80},
  {97, 86,  87, 80},
  {97, 86,  51, 80},
  {97, 86,  87, 80},
  {97, 98,  99, 80},
  {97, 98,  99, 80},
};
static const uint8 kAuxTilesets[82][4] = {
  {  6,   0,  31,  24},
  {  8,   0,  34,  27},
  {  6,   0,  31,  24},
  {  7,   0,  35,  28},
  {  7,   0,  33,  24},
  {  9,   0,  32,  25},
  { 11,   0,  33,  26},
  { 12,   0,  36,  25},
  {  8,   0,  34,  27},
  { 12,   0,  37,  27},
  { 12,   0,  38,  27},
  { 10,   0,  39,  29},
  { 10,   0,  40,  30},
  { 11,   0,  41,  22},
  { 13,   0,  42,  24},
  {  7,   0,  35,  28},
  {  7,   0,   4,   5},
  {  7,   0,   4,   5},
  {  9,   0,  32,  27},
  {  9,   0,  42,  23},
  { 11,   0,  33,  28},
  {  9,   0,  32,  25},
  { 11,   0,  33,  26},
  {  9,   0,  36,  27},
  {  8,   0,  34,  27},
  {  9,   0,  37,  27},
  {  9,   0,  38,  27},
  { 10,   0,  39,  29},
  {  9,   0,  40,  30},
  { 12,   0,  41,  22},
  { 13,   0,  42,  23},
  {114,   0,  43,  93},
  {  0,   0,   0,   0},
  {  0,  87,  76,   0},
  {  0,  86,  79,   0},
  {  0,  83,  77,   0},
  {  0,  82,  73,   0},
  {  0,  85,  74,   0},
  {  0,  83,  84,   0},
  {  0,  81,  78,   0},
  {  0,   0,   0,   0},
  {  0,  80,  75,   0},
  {  0,  83,  77,   0},
  {  0,  85,  84,   0},
  {  0,   0,   0,   0},
  {  0,   0,   0,   0},
  {  0,   0,   0,   0},
  {  0,  71,  72,   0},
  {  0,   0,   0,   0},
  {  0,  87,  76,   0},
  {  0,  86,  79,   0},
  {  0,  83,  77,   0},
  {  0,  82,  73,   0},
  {  0,  85,  74,   0},
  {  0,  83,  84,   0},
  {  0,  81,  78,   0},
  {  0,   0,   0,   0},
  {  0,  80,  75,   0},
  {  0,  83,   0,   0},
  {  0,  53,  54,   0},
  {  0,  96,  52,   0},
  {  0,  43,  44,   0},
  {  0,  45,  46,   0},
  {  0,  47,  48,   0},
  {  0,  55,  56,   0},
  {  0,  51,  52,   0},
  {  0,  49,  50,   0},
  {  0,   0,   0,   0},
  {  0,   0,   0,   0},
  {  0,   0,   0,   0},
  {  0,   0,   0,   0},
  {  0,   0,   0,   0},
  {  0,   0,   0,   0},
  {  0,   0,   0,   0},
  {  0,   0,   0,   0},
  {  0,   0,   0,   0},
  {  0,   0,   0,   0},
  {  0,   0,   0,   0},
  {  0,   0,   0,   0},
  {  0,   0,   0,   0},
  {114, 113, 114, 113},
  { 23,  64,  65,  57},
};
static const uint16 kTagalongWhich[14] = {0, 0x600, 0x300, 0x300, 0x300, 0, 0, 0x900, 0x600, 0x600, 0x900, 0x900, 0x600, 0x900};
static const uint16 kDecodeAnimatedSpriteTile_Tab[57] = {
  0x9c0, 0x30, 0x60, 0x90, 0xc0, 0x300, 0x318, 0x330, 0x348, 0x360, 0x378, 0x390, 0x930, 0x3f0, 0x420, 0x450,
  0x468, 0x600, 0x630, 0x660, 0x690, 0x6c0, 0x6f0, 0x720, 0x750, 0x768, 0x900, 0x930, 0x960, 0x990, 0x9f0, 0,
  0xf0, 0xa20, 0xa50, 0x660, 0x600, 0x618, 0x630, 0x648, 0x678, 0x6d8, 0x6a8, 0x708, 0x738, 0x768, 0x960, 0x900,
  0x3c0, 0x990, 0x9a8, 0x9c0, 0x9d8, 0xa08, 0xa38, 0x600, 0x630,
};
static const uint16 kSwordTypeToGfxOffs[5] = {0, 0, 0x120, 0x120, 0x120};
static const uint16 kShieldTypeToGfxOffs[4] = {0x660, 0x660, 0x6f0, 0x900};
static const int8 kOwBgPalInfo[93] = {
  0, -1, 7, 0, 1, 7, 0, 2, 7, 0, 3, 7, 0, 4, 7, 0, 5, 7, 0, 6, 7, 7, 6, 5,
  0, 8, 7, 0, 9, 7, 0, 10, 7, 0, 11, 7, 0, -1, 7, 0, -1, 7, 3, 4, 7, 4, 4, 3,
  16, -1, 6, 16, 1, 6, 16, 17, 6, 16, 3, 6, 16, 4, 6, 16, 5, 6, 16, 6, 6, 18, 19, 4,
  18, 5, 4, 16, 9, 6, 16, 11, 6, 16, 12, 6, 16, 13, 6, 16, 14, 6, 16, 15, 6,
};
static const int8 kOwSprPalInfo[40] = {
  -1, -1, 3, 10, 3, 6, 3, 1, 0, 2, 3, 14, 3, 2, 19, 1, 11, 12, 17, 1, 7, 5, 17, 0,
  9, 11, 15, 5, 3, 5, 3, 7, 15, 2, 10, 2, 5, 1, 12, 14,
};
static const int8 kSpotlight_delta_size[4] = {-7, 7, 7, 7};
static const uint8 kSpotlight_goal[4] = {0, 126, 35, 126};
static const uint8 kConfigureSpotlightTable_Helper_Tab[129] = {
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xfe, 0xfe, 0xfe, 0xfd, 0xfd, 0xfd, 0xfd, 0xfc, 0xfc, 0xfc, 0xfb, 0xfb, 0xfb, 0xfa, 0xfa, 0xf9, 0xf9, 0xf8, 0xf8,
  0xf7, 0xf7, 0xf6, 0xf6, 0xf5, 0xf5, 0xf4, 0xf3, 0xf3, 0xf2, 0xf1, 0xf1, 0xf0, 0xef, 0xee, 0xee, 0xed, 0xec, 0xeb, 0xea, 0xe9, 0xe9, 0xe8, 0xe7, 0xe6, 0xe5, 0xe4, 0xe3, 0xe2, 0xe1, 0xdf, 0xde,
  0xdd, 0xdc, 0xdb, 0xda, 0xd8, 0xd7, 0xd6, 0xd5, 0xd3, 0xd2, 0xd0, 0xcf, 0xcd, 0xcc, 0xca, 0xc9, 0xc7, 0xc6, 0xc4, 0xc2, 0xc1, 0xbf, 0xbd, 0xbb, 0xb9, 0xb7, 0xb6, 0xb4, 0xb1, 0xaf, 0xad, 0xab,
  0xa9, 0xa7, 0xa4, 0xa2, 0x9f, 0x9d, 0x9a, 0x97, 0x95, 0x92, 0x8f, 0x8c, 0x89, 0x86, 0x82, 0x7f, 0x7b, 0x78, 0x74, 0x70, 0x6c, 0x67, 0x63, 0x5e, 0x59, 0x53, 0x4d, 0x46, 0x3f, 0x37, 0x2d, 0x1f,
  0,
};
static const uint8 kGraphicsHalfSlotPacks[20] = {
  1, 1, 8, 8, 9, 9, 2, 2, 2, 2, 3, 3, 4, 4, 5, 5,
  8, 8, 8, 8,
};
static const int8 kGraphicsLoadSp6[20] = {
  10, -1, 3, -1, 0, -1, -1, -1, 1, -1, 2, -1, 0, -1, -1,
  -1, -1, -1, -1, -1,
};
static const uint8 kMirrorWarp_LoadNext_NmiLoad[15] = {0, 14, 15, 16, 17, 0, 0, 0, 0, 0, 0, 18, 19, 20, 0};
const uint16 *GetFontPtr() {
  return kFontData;
}
static const uint8 *GetCompSpritePtr(int i) {
  return kSprGfx[i];
}

void ApplyPaletteFilter_bounce() {

  const uint16 *load_ptr = kPaletteFilteringBits + (palette_filter_countdown >= 0x10);

  int mask = kUpperBitmasks[palette_filter_countdown & 0xf];
  int dt = darkening_or_lightening_screen ? 1 : -1;
  int j = 0;
  for (;;) {
    uint16 c = main_palette_buffer[j], a = aux_palette_buffer[j];
    if (!(load_ptr[(a & 0x1f) * 2] & mask))
      c += dt;
    if (!(load_ptr[(a & 0x3e0) >> 4] & mask))
      c += dt << 5;
    if (!(load_ptr[(a & 0x7c00) >> 9] & mask))
      c += dt << 10;
    main_palette_buffer[j] = c;
    j++;
    if (j == 1)
      j = 0x20;
    else if (j == 0xd8)
      j = 0xe0;
    else if (j == 0xf0)
      break;
  }
  flag_update_cgram_in_nmi++;
  if (!darkening_or_lightening_screen) {
    if (++palette_filter_countdown != mosaic_target_level)
      return;
  } else {
    if (palette_filter_countdown-- != mosaic_target_level)
      return;
  }
  darkening_or_lightening_screen ^= 2;
  palette_filter_countdown = 0;
  subsubmodule_index++;
}

void PaletteFilter_Range(int from, int to) {
  const uint16 *load_ptr = kPaletteFilteringBits + (palette_filter_countdown >= 0x10);
  int mask = kUpperBitmasks[palette_filter_countdown & 0xf];
  int dt = darkening_or_lightening_screen ? 1 : -1;
  for (int j = from; j != to; j++) {
    uint16 c = main_palette_buffer[j], a = aux_palette_buffer[j];
    if (!(load_ptr[(a & 0x1f) * 2] & mask))
      c += dt;
    if (!(load_ptr[(a & 0x3e0) >> 4] & mask))
      c += dt << 5;
    if (!(load_ptr[(a & 0x7c00) >> 9] & mask))
      c += dt << 10;
    main_palette_buffer[j] = c;
  }
}

void PaletteFilter_IncrCountdown() {
  if (++palette_filter_countdown == 0x1f) {
    palette_filter_countdown = 0;
    darkening_or_lightening_screen ^= 2;
    if (darkening_or_lightening_screen)
      WORD(link_actual_vel_y)++; // wtf?
  }
  flag_update_cgram_in_nmi++;
}

uint8 *LoadItemAnimationGfxOne(uint8 *dst, int num, int r12, bool from_temp) {
  static const uint8 kIntro_LoadGfx_Tab[10] = { 0, 11, 8, 38, 42, 45, 34, 3, 33, 46 };
  const uint8 *src = from_temp ? &g_ram[0x14000] : GetCompSpritePtr(0);
  const uint8 *base_src = src;
  src += kIntro_LoadGfx_Tab[r12] * 24;
  Expand3To4High(dst, src, base_src, num);
  Expand3To4High(dst + 0x20 * num, src + 0x180, base_src, num);
  return dst + 0x40 * num;
}

uint16 snes_divide(uint16 dividend, uint8 divisor) {
  return divisor ? dividend / divisor : 0xffff;
}

void EraseTileMaps_normal() {
  EraseTileMaps(0x7f, 0x1ec);
}

void DecompAndUpload2bpp(uint8 pack) {
  Decomp_spr(&g_ram[0x14000], pack);
  const uint8 *src = &g_ram[0x14000];
  for (int i = 0; i < 1024; i++, src += 2)
    zelda_ppu_write_word(VMDATAL, WORD(src[0]));
}

void RecoverPegGFXFromMapping() {
  if (BYTE(orange_blue_barrier_state))
    Dungeon_UpdatePegGFXBuffer(0x180, 0x0);
  else
    Dungeon_UpdatePegGFXBuffer(0x0, 0x180);
}

void LoadOverworldMapPalette() {
  memcpy(main_palette_buffer, &kOverworldMapPaletteData[overworld_screen_index & 0x40 ? 0x80 : 0], 256);
}

void EraseTileMaps_triforce() {  // 808333
  EraseTileMaps(0xa9, 0x7f);
}

void EraseTileMaps_dungeonmap() {  // 80833f
  EraseTileMaps(0x7f, 0x300);
}

void EraseTileMaps(uint16 r2, uint16 r0) {  // 808355
  uint16 *dst = g_zenv.vram;
  for (int i = 0; i < 0x2000; i++)
    dst[i] = r0;

  dst = g_zenv.vram + 0x6000;
  for (int i = 0; i < 0x800; i++)
    dst[i] = r2;
}

void EnableForceBlank() {  // 80893d
  zelda_ppu_write(INIDISP, 0x80);
  INIDISP_copy = 0x80;
  zelda_snes_dummy_write(HDMAEN, 0);
  HDMAEN_copy = 0;
}

void LoadItemGFXIntoWRAM4BPPBuffer() {  // 80d231
  uint8 *dst = &g_ram[0x9000 + 0x480];
  dst = LoadItemAnimationGfxOne(dst, 7, 0, false);  // rod
  dst = LoadItemAnimationGfxOne(dst, 7, 1, false);  // hammer
  dst = LoadItemAnimationGfxOne(dst, 3, 2, false);  // bow

  Decomp_spr(&g_ram[0x14000], 95);
  dst = LoadItemAnimationGfxOne(dst, 4, 3, true);  // shovel
  dst = LoadItemAnimationGfxOne(dst, 3, 4, true);  // sleeping zzz
  dst = LoadItemAnimationGfxOne(dst, 1, 5, true);  // misc #2
  dst = LoadItemAnimationGfxOne(dst, 4, 6, false); // hookshot

  Decomp_spr(&g_ram[0x14000], 96);
  dst = LoadItemAnimationGfxOne(dst, 14, 7, true); // bugnet
  dst = LoadItemAnimationGfxOne(dst, 7, 8, true);  // cane

  Decomp_spr(&g_ram[0x14000], 95);
  dst = LoadItemAnimationGfxOne(dst, 2, 9, true);  // book of mudora
  Decomp_spr(&g_ram[0x14000], 84);

  dst = &g_ram[0xa480];
  Expand3To4High(dst, &g_ram[0x14000], g_ram, 8);
  Expand3To4High(dst + 8 * 0x20, &g_ram[0x14180], g_ram, 8);

  // rupees
  Decomp_spr(&g_ram[0x14000], 96);
  dst = &g_ram[0xb280];
  Expand3To4High(dst, &g_ram[0x14000], g_ram, 3);
  Expand3To4High(dst + 3 * 0x20, &g_ram[0x14180], g_ram, 3);

  LoadItemGFX_Auxiliary();
}

void DecompressSwordGraphics() {  // 80d2c8
  Decomp_spr(&g_ram[0x14600], 0x5f);
  Decomp_spr(&g_ram[0x14000], 0x5e);
  const uint8 *src = &g_ram[0x14000] + kSwordTypeToGfxOffs[link_sword_type];
  Expand3To4High(&g_ram[0x9000 + 0], src, g_ram, 12);
  Expand3To4High(&g_ram[0x9000 + 0x180], src + 0x180, g_ram, 12);
}

void DecompressShieldGraphics() {  // 80d308
  Decomp_spr(&g_ram[0x14600], 0x5f);
  Decomp_spr(&g_ram[0x14000], 0x5e);
  const uint8 *src = &g_ram[0x14000] + kShieldTypeToGfxOffs[link_shield_type];
  Expand3To4High(&g_ram[0x9000 + 0x300], src, g_ram, 6);
  Expand3To4High(&g_ram[0x9000 + 0x3c0], src + 0x180, g_ram,6);
}

void DecompressAnimatedDungeonTiles(uint8 a) {  // 80d337
  Decomp_bg(&g_ram[0x14000], a);
  Do3To4Low16Bit(&g_ram[0x9000 + 0x1680], &g_ram[0x14000], 48);
  Decomp_bg(&g_ram[0x14000], 0x5c);
  Do3To4Low16Bit(&g_ram[0x9000 + 0x1C80], &g_ram[0x14000], 48);

  for (int i = 0; i < 256; i++) {
    uint8 *p = &g_ram[0x9000 + i * 2];
    uint16 x = WORD(p[0x1880]);
    WORD(p[0x1880]) = WORD(p[0x1C80]);
    WORD(p[0x1C80]) = WORD(p[0x1E80]);
    WORD(p[0x1E80]) = WORD(p[0x1A80]);
    WORD(p[0x1A80]) = x;
  }
  animated_tile_vram_addr = 0x3b00;
}

void DecompressAnimatedOverworldTiles(uint8 a) {  // 80d394
  Decomp_bg(&g_ram[0x14000], a);
  Do3To4Low16Bit(&g_ram[0x9000 + 0x1680], &g_ram[0x14000], 64);
  Decomp_bg(&g_ram[0x14000], a + 1);
  Do3To4Low16Bit(&g_ram[0x9000 + 0x1E80], &g_ram[0x14000], 32);
  animated_tile_vram_addr = 0x3c00;
}

void LoadItemGFX_Auxiliary() {  // 80d3c6
  Decomp_bg(&g_ram[0x14000], 0xf);
  Do3To4Low16Bit(&g_ram[0x9000 + 0x2340], &g_ram[0x14000], 16);

  Decomp_spr(&g_ram[0x14000], 0x58);
  Do3To4Low16Bit(&g_ram[0x9000 + 0x2540], &g_ram[0x14000], 32);

  Decomp_bg(&g_ram[0x14000], 0x5);
  Do3To4Low16Bit(&g_ram[0x9000 + 0x2dc0], &g_ram[0x14480], 2);
}

void LoadFollowerGraphics() {  // 80d423
  uint8 yv = 0x64;
  if (savegame_tagalong != 1) {
    yv = 0x66;
    if (savegame_tagalong >= 9) {
      yv = 0x59;
      if (savegame_tagalong >= 12)
        yv = 0x58;
    }
  }
  Decomp_spr(&g_ram[0x14600], yv);
  Decomp_spr(&g_ram[0x14000], 0x65);
  Do3To4Low16Bit(&g_ram[0x9000] + 0x2940, &g_ram[0x14000 + kTagalongWhich[savegame_tagalong]], 0x20);
}

void WriteTo4BPPBuffer_at_7F4000(uint8 a) {  // 80d4db
  uint8 *src = &g_ram[0x14000] + kDecodeAnimatedSpriteTile_Tab[a];
  Expand3To4High(&g_ram[0x9000] + 0x2d40, src, g_ram, 2);
  Expand3To4High(&g_ram[0x9000] + 0x2d40 + 0x40, src + 0x180, g_ram, 2);
}

void DecodeAnimatedSpriteTile_variable(uint8 a) {  // 80d4ed
  uint8 y = (a == 0x23 || a >= 0x37) ? 0x5d :
            (a == 0xc || a >= 0x24) ? 0x5c : 0x5b;
  Decomp_spr(&g_ram[0x14600], y);
  Decomp_spr(&g_ram[0x14000], 0x5a);
  WriteTo4BPPBuffer_at_7F4000(a);
}

void Expand3To4High(uint8 *dst, const uint8 *src, const uint8 *base, int num) {  // 80d61c
  do {
    const uint8 *src2 = src + 0x10;
    int n = 8;
    do {
      uint16 t = WORD(src[0]);
      uint8 u = src2[0];
      WORD(dst[0]) = t;
      WORD(dst[0x10]) = (t | (t >> 8) | u) << 8 | u;
      src += 2, src2 += 1, dst += 2;
    } while (--n);
    dst += 16, src = src2;
    if (!(src - base & 0x78))
      src += 0x180;
  } while (--num);
}

void LoadTransAuxGFX() {  // 80d66e
  uint8 *dst = &g_ram[0x6000];
  const uint8 *p = kAuxTilesets[aux_tile_theme_index];
  int len;

  if (p[0]) {
    aux_bg_subset_0 = p[0];
    len = Decomp_bg(dst, aux_bg_subset_0);
    assert(len == 0x600);
  }
  if (p[1]) {
    aux_bg_subset_1 = p[1];
    len = Decomp_bg(dst + 0x600, aux_bg_subset_1);
    assert(len == 0x600);
  }
  if (p[2]) {
    aux_bg_subset_2 = p[2];
    len = Decomp_bg(dst + 0x600*2, aux_bg_subset_2);
    assert(len == 0x600);
  }
  if (p[3]) {
    aux_bg_subset_3 = p[3];
    len = Decomp_bg(dst + 0x600*3, aux_bg_subset_3);
    assert(len == 0x600);
  }
  Gfx_LoadSpritesInner(dst + 0x600 * 4 );
}

void LoadTransAuxGFX_sprite() {  // 80d6f9
  Gfx_LoadSpritesInner(&g_ram[0x7800]);
}

void Gfx_LoadSpritesInner(uint8 *dst) {  // 80d706
  const uint8 *p = kSpriteTilesets[sprite_graphics_index];
  int len;

  if (p[0])
    sprite_gfx_subset_0 = p[0];
  len = Decomp_spr(dst, sprite_gfx_subset_0);
  assert(len == 0x600);
  if (p[1])
    sprite_gfx_subset_1 = p[1];
  len = Decomp_spr(dst + 0x600, sprite_gfx_subset_1);
  assert(len == 0x600);
  if (p[2])
    sprite_gfx_subset_2 = p[2];
  len = Decomp_spr(dst + 0x600*2, sprite_gfx_subset_2);
  assert(len == 0x600);
  if (p[3])
    sprite_gfx_subset_3 = p[3];
  len = Decomp_spr(dst + 0x600*3, sprite_gfx_subset_3);
  assert(len == 0x600);
  incremental_counter_for_vram = 0;
}

void ReloadPreviouslyLoadedSheets() {  // 80d788
  Decomp_bg(&g_ram[0x6000], aux_bg_subset_0);
  Decomp_bg(&g_ram[0x6600], aux_bg_subset_1);
  Decomp_bg(&g_ram[0x6c00], aux_bg_subset_2);
  Decomp_bg(&g_ram[0x7200], aux_bg_subset_3);
  Decomp_spr(&g_ram[0x7800], sprite_gfx_subset_0);
  Decomp_spr(&g_ram[0x7e00], sprite_gfx_subset_1);
  Decomp_spr(&g_ram[0x8400], sprite_gfx_subset_2);
  Decomp_spr(&g_ram[0x8a00], sprite_gfx_subset_3);
  incremental_counter_for_vram = 0;
}

void Attract_DecompressStoryGFX() {  // 80d80e
  Decomp_spr(&g_ram[0x14000], 0x67);
  Decomp_spr(&g_ram[0x14800], 0x68);
}

void AnimateMirrorWarp() {  // 80d864
  int st = overworld_map_state++, tt;
  nmi_subroutine_index = nmi_disable_core_updates = kMirrorWarp_LoadNext_NmiLoad[st];
  uint8 t, xt = overworld_screen_index & 0x40 ? 8 : 0;
  switch (st) {
  case 0:
    if (++mirror_vars.ctr2 != 32)
      overworld_map_state = 0;
    else
      SetTargetOverworldWarpToPyramid();
    break;
  case 1:
    AnimateMirrorWarp_DecompressNewTileSets();
    Decomp_bg(&g_ram[0x14000], kVariousPacks[xt]);
    Decomp_bg(&g_ram[0x14600], kVariousPacks[xt + 1]);
    Do3To4High16Bit(&g_ram[0x10000], &g_ram[0x14000], 64);
    Do3To4Low16Bit(&g_ram[0x10800], &g_ram[0x14600], 64);
    break;
  case 2:
    Decomp_bg(&g_ram[0x14000], kVariousPacks[xt + 2]);
    Decomp_bg(&g_ram[0x14600], kVariousPacks[xt + 3]);
    Do3To4Low16Bit(&g_ram[0x10000], &g_ram[0x14000], 64);
    Do3To4High16Bit(&g_ram[0x10800], &g_ram[0x14600], 64);
    break;
  case 3:
    Decomp_bg(&g_ram[0x14000], aux_bg_subset_1);
    Decomp_bg(&g_ram[0x14600], aux_bg_subset_2);
    Do3To4High16Bit(&g_ram[0x10000], &g_ram[0x14000], 128);
    break;
  case 4:
    Decomp_bg(&g_ram[0x14000], kVariousPacks[xt + 4]);
    Decomp_bg(&g_ram[0x14600], kVariousPacks[xt + 5]);
    Do3To4Low16Bit(&g_ram[0x10000], &g_ram[0x14000], 128);
    break;
  case 5:
    PreOverworld_LoadOverlays();
    if (BYTE(overworld_screen_index) == 27 || BYTE(overworld_screen_index) == 91)
      TS_copy = 1;
    submodule_index--;
    nmi_subroutine_index = nmi_disable_core_updates = 12;
    break;
  case 6:
  case 9:
    nmi_subroutine_index = nmi_disable_core_updates = 13;
    break;
  case 7:
    Overworld_DrawScreenAtCurrentMirrorPosition();
    nmi_disable_core_updates++;
    break;
  case 8:
    MirrorWarp_LoadSpritesAndColors();
    nmi_subroutine_index = nmi_disable_core_updates = 12;
    break;
  case 10:
    t = overworld_screen_index & 0xbf;
    DecompressAnimatedOverworldTiles(t == 3 || t == 5 || t == 7 ? 0x58 : 0x5a);
    break;
  case 11:
    t = overworld_screen_index;
    TS_copy = (t == 0 || t == 0x70 || t == 0x40 || t == 0x5b || t == 3 || t == 5 || t == 7 || t == 0x43 || t == 0x45 || t == 0x47);
    Do3To4High16Bit(&g_ram[0x10000], GetCompSpritePtr(kVariousPacks[xt + 6]), 64);
    break;
  case 12:
    Decomp_spr(&g_ram[0x14000], sprite_gfx_subset_0);
    Decomp_spr(&g_ram[0x14600], sprite_gfx_subset_1);
    tt = WORD(sprite_gfx_subset_0);
    if (tt == 0x52 || tt == 0x53 || tt == 0x5a || tt == 0x5b)
      Do3To4High16Bit(&g_ram[0x10000], &g_ram[0x14000], 64);
    else
      Do3To4Low16Bit(&g_ram[0x10000], &g_ram[0x14000], 64);
    Do3To4Low16Bit(&g_ram[0x10800], &g_ram[0x14600], 64);
    break;
  case 13:
    Decomp_spr(&g_ram[0x14000], sprite_gfx_subset_2);
    Decomp_spr(&g_ram[0x14600], sprite_gfx_subset_3);
    Do3To4Low16Bit(&g_ram[0x10000], &g_ram[0x14000], 128);
    HandleFollowersAfterMirroring();
    break;
  case 14:
    overworld_map_state = 14;
    break;
  }
}

void AnimateMirrorWarp_DecompressNewTileSets() {  // 80d8fe
  const uint8 *mt = kMainTilesets[main_tile_theme_index];
  const uint8 *at = kAuxTilesets[aux_tile_theme_index];

  aux_bg_subset_0 = at[0] ? at[0] : mt[3];
  aux_bg_subset_1 = at[1] ? at[1] : mt[4];
  aux_bg_subset_2 = at[2] ? at[2] : mt[5];
  aux_bg_subset_3 = at[3] ? at[3] : mt[6];

  const uint8 *p = kSpriteTilesets[sprite_graphics_index];
  if (p[0]) sprite_gfx_subset_0 = p[0];
  if (p[1]) sprite_gfx_subset_1 = p[1];
  if (p[2]) sprite_gfx_subset_2 = p[2];
  if (p[3]) sprite_gfx_subset_3 = p[3];
}

void Graphics_IncrementalVRAMUpload() {  // 80deff
  if (incremental_counter_for_vram == 16)
    return;

  nmi_update_tilemap_dst = kGraphics_IncrementalVramUpload_Dst[incremental_counter_for_vram];
  nmi_update_tilemap_src = kGraphics_IncrementalVramUpload_Src[incremental_counter_for_vram] << 8;
  incremental_counter_for_vram++;
}

void PrepTransAuxGfx() {  // 80df1a
  Do3To4High16Bit(&g_ram[0x10000], &g_ram[0x6000], 0x40);
  if (aux_tile_theme_index >= 32) {
    Do3To4High16Bit(&g_ram[0x10800], &g_ram[0x6600], 0x80);
    Do3To4Low16Bit(&g_ram[0x11800], &g_ram[0x7200], 0x40);
  } else {
    Do3To4Low16Bit(&g_ram[0x10800], &g_ram[0x6600], 0xC0);
  }
}

void Do3To4High16Bit(uint8 *dst, const uint8 *src, int num) {  // 80df4f
  do {
    const uint8 *src2 = src + 0x10;
    int n = 8;
    do {
      uint16 t = WORD(src[0]);
      uint8 u = src2[0];
      WORD(dst[0]) = t;
      WORD(dst[0x10]) = (t | (t >> 8) | u) << 8 | u;
      src += 2, src2 += 1, dst += 2;
    } while (--n);
    dst += 16, src = src2;
  } while (--num);

}

void Do3To4Low16Bit(uint8 *dst, const uint8 *src, int num) {  // 80dfb8
  do {
    const uint8 *src2 = src + 0x10;
    int n = 8;
    do {
      WORD(dst[0]) = WORD(src[0]);
      WORD(dst[0x10]) = src2[0];
      src += 2, src2 += 1, dst += 2;
    } while (--n);
    dst += 16, src = src2;
  } while (--num);
}

void LoadNewSpriteGFXSet() {  // 80e031
  Do3To4Low16Bit(&g_ram[0x10000], &g_ram[0x7800], 0xC0);
  if (sprite_gfx_subset_3 == 0x52 || sprite_gfx_subset_3 == 0x53 || sprite_gfx_subset_3 == 0x5a || sprite_gfx_subset_3 == 0x5b)
    Do3To4High16Bit(&g_ram[0x11800], &g_ram[0x8a00], 0x40);
  else
    Do3To4Low16Bit(&g_ram[0x11800], &g_ram[0x8a00], 0x40);
}

void InitializeTilesets() {  // 80e19b
  zelda_ppu_write(VMAIN, 0x80);
  zelda_ppu_write_word(VMADDL, 0x4400);
  LoadCommonSprites();
  const uint8 *p = kSpriteTilesets[sprite_graphics_index];
  if (p[0]) sprite_gfx_subset_0 = p[0];
  if (p[1]) sprite_gfx_subset_1 = p[1];
  if (p[2]) sprite_gfx_subset_2 = p[2];
  if (p[3]) sprite_gfx_subset_3 = p[3];

  LoadSpriteGraphics(sprite_gfx_subset_0, &g_ram[0x7800]);
  LoadSpriteGraphics(sprite_gfx_subset_1, &g_ram[0x7e00]);
  LoadSpriteGraphics(sprite_gfx_subset_2, &g_ram[0x8400]);
  LoadSpriteGraphics(sprite_gfx_subset_3, &g_ram[0x8a00]);

  zelda_ppu_write_word(VMADDL, 0x2000);

  const uint8 *mt = kMainTilesets[main_tile_theme_index];
  const uint8 *at = kAuxTilesets[aux_tile_theme_index];

  aux_bg_subset_0 = at[0] ? at[0] : mt[3];
  aux_bg_subset_1 = at[1] ? at[1] : mt[4];
  aux_bg_subset_2 = at[2] ? at[2] : mt[5];
  aux_bg_subset_3 = at[3] ? at[3] : mt[6];

  LoadBackgroundGraphics(mt[0], 7, &g_ram[0x14000]);
  LoadBackgroundGraphics(mt[1], 6, &g_ram[0x14000]);
  LoadBackgroundGraphics(mt[2], 5, &g_ram[0x14000]);
  LoadBackgroundGraphics(aux_bg_subset_0, 4, &g_ram[0x6000]);
  LoadBackgroundGraphics(aux_bg_subset_1, 3, &g_ram[0x6600]);
  LoadBackgroundGraphics(aux_bg_subset_2, 2, &g_ram[0x6c00]);
  LoadBackgroundGraphics(aux_bg_subset_3, 1, &g_ram[0x7200]);
  LoadBackgroundGraphics(mt[7], 0, &g_ram[0x14000]);
}

void LoadDefaultGraphics() {  // 80e2d0
  zelda_ppu_write(VMAIN, 0x80);
  zelda_ppu_write_word(VMADDL, 0x4000);
  const uint8 *src = GetCompSpritePtr(0);

  uint16 *tmp = (uint16 *)&g_ram[0xbf];
  int num = 64;
  do {
    for (int i = 7; i >= 0; i--, src += 2) {
      zelda_ppu_write_word(VMDATAL, WORD(src[0]));
      tmp[i] = src[0] | src[1];
    }
    for (int i = 7; i >= 0; i--, src++) {
      zelda_ppu_write_word(VMDATAL, src[0] | (src[0] | tmp[i]) << 8);
    }
  } while (--num);

  // Load 2bpp graphics used for hud
  zelda_ppu_write_word(VMADDL, 0x7000);
  DecompAndUpload2bpp(0x6a);
  DecompAndUpload2bpp(0x6b);
  DecompAndUpload2bpp(0x69);
}

void Attract_LoadBG3GFX() {  // 80e36d
  // load 2bpp gfx for attract images
  zelda_ppu_write(VMAIN, 0x80);
  zelda_ppu_write(VMADDL, 0);
  zelda_ppu_write(VMADDH, 0x78);
  DecompAndUpload2bpp(0x67);
}

void LoadCommonSprites_2() {  // 80e384
  zelda_ppu_write(VMAIN, 0x80);
  zelda_ppu_write(VMADDL, 0);
  zelda_ppu_write(VMADDH, 0x44);
  LoadCommonSprites();
}

void Graphics_LoadChrHalfSlot() {  // 80e3fa
  int k = load_chr_halfslot_even_odd;
  if (k == 0)
    return;

  int8 sp6 = kGraphicsLoadSp6[k - 1];
  if (sp6 >= 0) {
    palette_sp6 = sp6;
    if (k == 1) {
      palette_sp6 = 10;
      overworld_palette_aux_or_main = 0x200;
      Palette_Load_SpriteEnvironment();
      flag_update_cgram_in_nmi++;
    } else {
      overworld_palette_aux_or_main = 0x200;
      Palette_Load_SpriteEnvironment_Dungeon();
      flag_update_cgram_in_nmi++;
    }
  }
  int tilebytes = 0x44;
  int bank_offs = 0;
  load_chr_halfslot_even_odd++;

  if (load_chr_halfslot_even_odd & 1) {
    load_chr_halfslot_even_odd = 0;
    if (k != 18) {
      bank_offs = 0x300;
      tilebytes = 0x46;
      if (k == 2)
        flag_custom_spell_anim_active = 0;
    }
  }
  BYTE(nmi_load_target_addr) = tilebytes;
  nmi_subroutine_index = 11;

  k = kGraphicsHalfSlotPacks[k - 1];
  if (k == 1)
    k = misc_sprites_graphics_index;

  const uint8 *srcp = GetCompSpritePtr(k) + bank_offs;
  uint8 sprdata[24];
  int num = 32;
  uint8 *dst = &g_ram[0x11000];

  do {
    for (int i = 0; i < 24; i++)
      sprdata[i] = *srcp++;

    uint8 *src = sprdata, *src2 = sprdata + 16;
    int n = 8;
    do {
      uint16 t = WORD(src[0]);
      uint8 u = src2[0];
      WORD(dst[0]) = t;
      WORD(dst[16]) = (t | (t >> 8) | u) << 8 | u;
      src += 2, src2 += 1, dst += 2;
    } while (--n);
    dst += 16;
  } while (--num);
}

void TransferFontToVRAM() {  // 80e556
  zelda_ppu_write(OBSEL, 2);
  zelda_ppu_write(VMAIN, 0x80);
  zelda_ppu_write_word(VMADDL, 0x7000);
  const uint16 *src = GetFontPtr();
  for (int i = 0; i != 0x800; i++, src++)
    zelda_ppu_write_word(VMDATAL, *src);
}

void LoadSpriteGraphics(int gfx_pack, uint8 *decomp_addr) {  // 80e583
  Decomp_spr(decomp_addr, gfx_pack);

  if (gfx_pack == 0x52 || gfx_pack == 0x53 || gfx_pack == 0x5a || gfx_pack == 0x5b ||
      gfx_pack == 0x5c || gfx_pack == 0x5e || gfx_pack == 0x5f)
    Do3To4High(decomp_addr);
  else
    Do3To4Low(decomp_addr);
}

void Do3To4High(const uint8 *decomp_addr) {  // 80e5af
  for (int j = 0; j < 64; j++) {
    uint16 *t = (uint16 *)&dung_line_ptrs_row0;
    for (int i = 7; i >= 0; i--, decomp_addr += 2) {
      uint16 d = *(uint16 *)decomp_addr;
      t[i] = (d | (d >> 8)) & 0xff;
      zelda_ppu_write_word(VMDATAL, d);
    }
    for (int i = 7; i >= 0; i--, decomp_addr += 1) {
      uint8 d = *decomp_addr;
      zelda_ppu_write_word(VMDATAL, d | (t[i] | d) << 8);
    }
  }
}

void LoadBackgroundGraphics(int gfx_pack, int slot, uint8 *decomp_addr) {  // 80e609
  Decomp_bg(decomp_addr, gfx_pack);
  if ((main_tile_theme_index >= 0x20) ? (slot == 7 || slot == 2 || slot == 3 || slot == 4) : (slot >= 4))
    Do3To4High(decomp_addr);
  else
    Do3To4Low(decomp_addr);
}

void Do3To4Low(const uint8 *decomp_addr) {  // 80e63c
  for (int j = 0; j < 64; j++) {
    for (int i = 0; i < 8; i++, decomp_addr += 2)
      zelda_ppu_write_word(VMDATAL, *(uint16 *)decomp_addr);
    for (int i = 0; i < 8; i++, decomp_addr += 1)
      zelda_ppu_write_word(VMDATAL, *decomp_addr);
  }
}

void LoadCommonSprites() {  // 80e6b7
  Do3To4High(GetCompSpritePtr(misc_sprites_graphics_index));
  if (main_module_index != 1) {
    Do3To4Low(GetCompSpritePtr(6));
    Do3To4Low(GetCompSpritePtr(7));
  } else {
    // select file
    LoadSpriteGraphics(94, &g_ram[0x14000]);
    LoadSpriteGraphics(95, &g_ram[0x14000]);
  }
}

int Decomp_spr(uint8 *dst, int gfx) {  // 80e772
  return Decompress(dst, kSprGfx[gfx]);
}

int Decomp_bg(uint8 *dst, int gfx) {  // 80e78f
  return Decompress(dst, kBgGfx[gfx]);
}

int Decompress(uint8 *dst, const uint8 *src) {  // 80e79e
  uint8 *dst_org = dst;
  int len;
  for (;;) {
    uint8 cmd = *src++;
    if (cmd == 0xff)
      return dst - dst_org;
    if ((cmd & 0xe0) != 0xe0) {
      len = (cmd & 0x1f) + 1;
      cmd &= 0xe0;
    } else {
      len = *src++;
      len += ((cmd & 3) << 8) + 1;
      cmd = (cmd << 3) & 0xe0;
    }
    //printf("%d: %d,%d\n", (int)(dst - dst_org), cmd, len);
    if (cmd == 0) {
      do {
        *dst++ = *src++;
      } while (--len);
    } else if (cmd & 0x80) {
      uint32 offs = *src++;
      offs |= *src++ << 8;
      do {
        *dst++ = dst_org[offs++];
      } while (--len);
    } else if (!(cmd & 0x40)) {
      uint8 v = *src++;
      do {
        *dst++ = v;
      } while (--len);
    } else if (!(cmd & 0x20)) {
      uint8 lo = *src++;
      uint8 hi = *src++;
      do {
        *dst++ = lo;
        if (--len == 0)
          break;
        *dst++ = hi;
      } while (--len);
    } else {
      // copy bytes with the byte incrementing by 1 in between
      uint8 v = *src++;
      do {
        *dst++ = v;
      } while (v++, --len);
    }
  }
}

void ResetHUDPalettes4and5() {  // 80eb29
  for (int i = 0; i < 8; i++)
    main_palette_buffer[16 + i] = 0;
  palette_filter_countdown = 0;
  darkening_or_lightening_screen = 2;
  flag_update_cgram_in_nmi++;
}

void PaletteFilterHistory() {  // 80eb5e
  PaletteFilter_Range(0x10, 0x18);
  PaletteFilter_IncrCountdown();
}

void PaletteFilter_WishPonds() {  // 80ebc5
  TS_copy = 2;
  CGADSUB_copy = 0x30;
  PaletteFilter_WishPonds_Inner();
}

void PaletteFilter_Crystal() {  // 80ebcf
  TS_copy = 1;
  PaletteFilter_WishPonds_Inner();
}

void PaletteFilter_WishPonds_Inner() {  // 80ebd3
  for (int i = 0; i < 8; i++)
    main_palette_buffer[0xd0 + i] = 0;
  palette_filter_countdown = 0;
  darkening_or_lightening_screen = 2;
  flag_update_cgram_in_nmi++;
}

void PaletteFilter_RestoreSP5F() {  // 80ebf2
  for (int i = 7; i >= 0; i--)
    main_palette_buffer[208 + i] = aux_palette_buffer[208 + i];
  TS_copy = 0;
  CGADSUB_copy = 32;
  flag_update_cgram_in_nmi++;
}

void PaletteFilter_SP5F() {  // 80ec0d
  for (int i = 0; i != 2; i++) {
    PaletteFilter_Range(208, 216);
    PaletteFilter_IncrCountdown();
    if (palette_filter_countdown == 0)
      break;
  }
}

void KholdstareShell_PaletteFiltering() {  // 80ec79
  if (subsubmodule_index == 0) {
    memcpy(main_palette_buffer + 0x40, aux_palette_buffer + 0x40, 16);
    palette_filter_countdown = 0;
    darkening_or_lightening_screen = 0;
    flag_update_cgram_in_nmi++;
    subsubmodule_index = 1;
    return;
  }
  for (int i = 0; i != 2; i++) {
    PaletteFilter_Range(0x40, 0x48);
    PaletteFilter_IncrCountdown();
    if (palette_filter_countdown == 0) {
      TS_copy = 0;
      break;
    }
  }
}

void AgahnimWarpShadowFilter(int k) {  // 80ecca
  palette_filter_countdown = agahnim_pal_setting[k];
  darkening_or_lightening_screen = agahnim_pal_setting[k + 3];
  int t = kPaletteFilter_Agahnim_Tab[k] >> 1;
  for (int i = 0; i < 2; i++) {
    PaletteFilter_Range(t, t + 8);
    if (++palette_filter_countdown == 0x1f) {
      palette_filter_countdown = 0;
      darkening_or_lightening_screen ^= 2;
      break;
    }
  }
  agahnim_pal_setting[k] = palette_filter_countdown;
  agahnim_pal_setting[k + 3] = darkening_or_lightening_screen;
  flag_update_cgram_in_nmi++;
}

void Palette_FadeIntroOneStep() {  // 80ed7c
  PaletteFilter_RestoreAdditive(0x100, 0x1a0);
  PaletteFilter_RestoreAdditive(0xc0, 0x100);
  BYTE(palette_filter_countdown) -= 1;
  flag_update_cgram_in_nmi++;
}

void Palette_FadeIntro2() {  // 80ed8f
  PaletteFilter_RestoreAdditive(0x40, 0xc0);
  PaletteFilter_RestoreAdditive(0x40, 0xc0);
  BYTE(palette_filter_countdown) -= 1;
  flag_update_cgram_in_nmi++;
}

void PaletteFilter_RestoreAdditive(int from, int to) {  // 80edca
  from >>= 1, to >>= 1;
  do {
    uint16 c = main_palette_buffer[from], cx = c;
    uint16 d = aux_palette_buffer[from];
    if ((c & 0x1f) != (d & 0x1f))
      cx += 1;
    if ((c & 0x3e0) != (d & 0x3e0))
      cx += 0x20;
    if ((c & 0x7c00) != (d & 0x7c00))
      cx += 0x400;
    main_palette_buffer[from] = cx;
  } while (++from != to);
}

void PaletteFilter_RestoreSubtractive(uint16 from, uint16 to) {  // 80ee21
  from >>= 1, to >>= 1;
  do {
    uint16 c = main_palette_buffer[from], cx = c;
    uint16 d = aux_palette_buffer[from];
    if ((c & 0x1f) != (d & 0x1f))
      cx -= 1;
    if ((c & 0x3e0) != (d & 0x3e0))
      cx -= 0x20;
    if ((c & 0x7c00) != (d & 0x7c00))
      cx -= 0x400;
    main_palette_buffer[from] = cx;
  } while (++from != to);
}

void PaletteFilter_InitializeWhiteFilter() {  // 80ee78
  for (int i = 0; i < 256; i++)
    aux_palette_buffer[i] = 0x7fff;
  main_palette_buffer[32] = main_palette_buffer[0];
  palette_filter_countdown = 0;
  darkening_or_lightening_screen = 2;
  if (overworld_screen_index == 27) {
    aux_palette_buffer[0] = aux_palette_buffer[32] = 0;
    main_palette_buffer[0] = main_palette_buffer[32] = 0;
  }
  mirror_vars.ctr = 8;
  mirror_vars.ctr2 = 0;
}

void MirrorWarp_RunAnimationSubmodules() {  // 80eee7
  if (--mirror_vars.ctr) {
    AnimateMirrorWarp();
    return;
  }
  mirror_vars.ctr = 2;
  PaletteFilter_BlindingWhite();
}

void PaletteFilter_BlindingWhite() {  // 80eef1
  if (darkening_or_lightening_screen == 0xff)
    return;

  if (darkening_or_lightening_screen == 2) {
    PaletteFilter_RestoreAdditive(0x40, 0x1b0);
    PaletteFilter_RestoreAdditive(0x1c0, 0x1e0);
  } else {
    PaletteFilter_RestoreSubtractive(0x40, 0x1b0);
    PaletteFilter_RestoreSubtractive(0x1c0, 0x1e0);
  }
  PaletteFilter_StartBlindingWhite();
}

void PaletteFilter_StartBlindingWhite() {  // 80ef27
  main_palette_buffer[0] = main_palette_buffer[32];
  if (!darkening_or_lightening_screen) {
    if (++palette_filter_countdown == 66) {
      darkening_or_lightening_screen = 0xff;
      mirror_vars.ctr = 32;
    }
  } else {
    if (++palette_filter_countdown == 31) {
      darkening_or_lightening_screen ^= 2;
      if (main_module_index != 21)
        return;
      zelda_snes_dummy_write(HDMAEN, 0);
      HDMAEN_copy = 0;
      for (int i = 0; i < 32 * 7; i++)
        mode7_hdma_table[i] = 0x778;
      HDMAEN_copy = 0xc0;
    }
  }
  flag_update_cgram_in_nmi++;
}

void PaletteFilter_BlindingWhiteTriforce() {  // 80ef8a
  PaletteFilter_RestoreAdditive(0x40, 0x200);
  PaletteFilter_StartBlindingWhite();
}

void PaletteFilter_WhirlpoolBlue() {  // 80ef97
  if (frame_counter & 1) {
    for (int i = 0x20; i != 0x100; i++) {
      uint16 t = main_palette_buffer[i];
      if ((t & 0x7C00) != 0x7C00)
        t += 0x400;
      main_palette_buffer[i] = t;
    }
    main_palette_buffer[0] = main_palette_buffer[32];
    if (!(palette_filter_countdown & 1))
      mosaic_level += 16;
    if (++palette_filter_countdown == 31) {
      palette_filter_countdown = 0;
      subsubmodule_index++;
      mosaic_level = 0xf0;
    }
  }
  BGMODE_copy = 9;
  MOSAIC_copy = mosaic_level | 3;
  flag_update_cgram_in_nmi++;
}

void PaletteFilter_IsolateWhirlpoolBlue() {  // 80f00c
  for (int i = 0x20; i != 0x100; i++) {
    uint16 t = main_palette_buffer[i];
    if (t & 0x3e0)
      t -= 0x20;
    if (t & 0x1f)
      t -= 1;
    main_palette_buffer[i] = t;
  }
  main_palette_buffer[0] = main_palette_buffer[32];
  if (++palette_filter_countdown == 31) {
    palette_filter_countdown = 0;
    subsubmodule_index++;
    mosaic_level = 0xf0;
  }
  BGMODE_copy = 9;
  MOSAIC_copy = mosaic_level | 3;
  flag_update_cgram_in_nmi++;
}

void PaletteFilter_WhirlpoolRestoreBlue() {  // 80f04a
  if (frame_counter & 1) {
    for (int i = 0x20; i != 0x100; i++) {
      uint16 u = aux_palette_buffer[i] & 0x7c00;
      uint16 t = main_palette_buffer[i];
      if ((t & 0x7C00) != u)
        t -= 0x400;
      main_palette_buffer[i] = t;
    }
    main_palette_buffer[0] = main_palette_buffer[32];
    if (!(palette_filter_countdown & 1))
      mosaic_level -= 16;
    if (++palette_filter_countdown == 31) {
      palette_filter_countdown = 0;
      subsubmodule_index++;
      mosaic_level = 0;
    }
  }
  BGMODE_copy = 9;
  MOSAIC_copy = mosaic_level | 3;
  flag_update_cgram_in_nmi++;
}

void PaletteFilter_WhirlpoolRestoreRedGreen() {  // 80f0c7
  for (int i = 0x20; i != 0x100; i++) {
    uint16 u0 = aux_palette_buffer[i] & 0x3e0;
    uint16 u1 = aux_palette_buffer[i] & 0x1f;
    uint16 t = main_palette_buffer[i];
    if ((t & 0x3e0) != u0)
      t += 0x20;
    if ((t & 0x1f) != u1)
      t += 1;
    main_palette_buffer[i] = t;
  }
  main_palette_buffer[0] = main_palette_buffer[32];
  if (++palette_filter_countdown == 31) {
    palette_filter_countdown = 0;
    subsubmodule_index++;
  }
  flag_update_cgram_in_nmi++;
}

void PaletteFilter_RestoreBGSubstractiveStrict() {  // 80f135
  if (darkening_or_lightening_screen == 255)
    return;
  PaletteFilter_RestoreSubtractive(0x40, 0x100);
  if (++palette_filter_countdown == 0x20) {
    darkening_or_lightening_screen = 255;
    WORD(TS_copy) = 0;
  }
  flag_update_cgram_in_nmi++;
}

void PaletteFilter_RestoreBGAdditiveStrict() {  // 80f169
  PaletteFilter_RestoreAdditive(0x40, 0x100);
  palette_filter_countdown++;
  flag_update_cgram_in_nmi++;
}

void Trinexx_FlashShellPalette_Red() {  // 80f183
  if (!byte_7E04BE) {
    for (int i = 0; i < 7; i++) {
      uint16 v = main_palette_buffer[0x41 + i];
      main_palette_buffer[0x41 + i] = (v & 0xffe0) | ((v & 0x1f) + ((v & 0x1f) != 0x1f));
    }
    flag_update_cgram_in_nmi++;
    if (++byte_7E04C0 >= 12) {
      byte_7E04C0 = byte_7E04BE = 0;
      return;
    }
    byte_7E04BE = 3;
  }
  byte_7E04BE--;
}

void Trinexx_UnflashShellPalette_Red() {  // 80f1cf
  if (!byte_7E04BE) {
    for (int i = 0; i < 7; i++) {
      uint16 u = aux_palette_buffer[0x41 + i];
      uint16 v = main_palette_buffer[0x41 + i];
      main_palette_buffer[0x41 + i] = (v & 0xffe0) | ((v & 0x1f) - ((v & 0x1f) != (u & 0x1f)));
    }
    flag_update_cgram_in_nmi++;
    if (++byte_7E04C0 >= 12) {
      byte_7E04C0 = byte_7E04BE = 0;
      return;
    }
    byte_7E04BE = 3;
  }
  byte_7E04BE--;
}

void Trinexx_FlashShellPalette_Blue() {  // 80f207
  if (!byte_7E04BF) {
    for (int i = 0; i < 7; i++) {
      uint16 v = main_palette_buffer[0x41 + i];
      main_palette_buffer[0x41 + i] = (v & ~0x7c00) | (v & 0x7c00) + (((v & 0x7c00) != 0x7c00) << 10);
    }
    flag_update_cgram_in_nmi++;
    if (++byte_7E04C1 >= 12) {
      byte_7E04C1 = byte_7E04BF = 0;
      return;
    }
    byte_7E04BF = 3;
  }
  byte_7E04BF--;

}

void Trinexx_UnflashShellPalette_Blue() {  // 80f253
  if (!byte_7E04BF) {
    for (int i = 0; i < 7; i++) {
      uint16 u = aux_palette_buffer[0x41 + i];
      uint16 v = main_palette_buffer[0x41 + i];
      main_palette_buffer[0x41 + i] = (v & ~0x7c00) | (v & 0x7c00) - (((v & 0x7c00) != (u & 0x7c00)) << 10);
    }
    flag_update_cgram_in_nmi++;
    if (++byte_7E04C1 >= 12) {
      byte_7E04C1 = byte_7E04BF = 0;
      return;
    }
    byte_7E04BF = 3;
  }
  byte_7E04BF--;
}

void IrisSpotlight_close() {  // 80f28b
  SpotlightInternal(0x7e, 0);
}

void Spotlight_open() {  // 80f295
  SpotlightInternal(0, 2);
}

void SpotlightInternal(uint8 x, uint8 y) {  // 80f29d
  spotlight_var1 = x;
  spotlight_var2 = y;

  zelda_snes_dummy_write(HDMAEN, 0);
  HdmaSetup(0xF2FB, 0xF2FB, 0x41, (uint8)WH0, (uint8)WH0, 0);

  W12SEL_copy = 0x33;
  W34SEL_copy = 3;
  WOBJSEL_copy = 0x33;
  TMW_copy = TM_copy;
  TSW_copy = TS_copy;
  if (!player_is_indoors) {
    COLDATA_copy0 = 0x20;
    COLDATA_copy1 = 0x40;
    COLDATA_copy2 = 0x80;
  }
  IrisSpotlight_ConfigureTable();
  HDMAEN_copy = 0x80;
  INIDISP_copy = 0xf;
}

void IrisSpotlight_ConfigureTable() {  // 80f312
  uint16 r14 = link_y_coord - BG2VOFS_copy2 + 12;
  spotlight_y_lower = r14 - spotlight_var1;
  spotlight_y_upper = r14 + spotlight_var1;
  spotlight_var3 = link_x_coord - BG2HOFS_copy2 + 8;
  spotlight_var4 = spotlight_var1;
  uint16 r6 = r14 * 2;
  if (r6 < 224)
    r6 = 224;
  uint16 r10 = r6 - r14;
  uint16 r4 = r14 - r10;
  for(;;) {
    uint16 r8 = 0xff;
    if (r6 < spotlight_y_upper) {
      uint8 t = spotlight_var4;
      if (spotlight_var4)
        spotlight_var4--;
      r8 = IrisSpotlight_CalculateCircleValue(t);
    }
    if (r4 < 0xe0)
      hdma_table[r4] = r8;
    if (r6 < 0xe0)
      hdma_table[r6] = r8;
    if (r4 == r14)
      break;
    r4++, r6--;
  }

  memcpy(mode7_hdma_table, hdma_table, 224  * sizeof(uint16));

  spotlight_var1 += kSpotlight_delta_size[spotlight_var2 >> 1];

  if (spotlight_var1 != kSpotlight_goal[spotlight_var2 >> 1])
    return;

  if (!spotlight_var2) {
    INIDISP_copy = 0x80;
    zelda_ppu_write(INIDISP, 0x80);
  } else {
    IrisSpotlight_ResetTable();
  }
  subsubmodule_index = 0;
  submodule_index = 0;

  if (main_module_index == 7 || main_module_index == 16) {
    if (!player_is_indoors)
      sound_effect_ambient = overworld_music[BYTE(overworld_screen_index)] >> 4;
    if (buffer_for_playing_songs != 0xff)
      music_control = buffer_for_playing_songs;
  }
  main_module_index = saved_module_for_menu;
  if (main_module_index == 6)
    Sprite_ResetAll();
}

void IrisSpotlight_ResetTable() {  // 80f427
  for (int i = 0; i < 224; i++)
    mode7_hdma_table[i] = 0xff00;
}

uint16 IrisSpotlight_CalculateCircleValue(uint8 a) {  // 80f4cc
  uint8 t = snes_divide(a << 8, spotlight_var1) >> 1;
  uint8 r10 = kConfigureSpotlightTable_Helper_Tab[t];
  uint16 p = 2 * (uint8)(r10 * (uint8)spotlight_var1 >> 8);
  if (!r10)
    return 0xff;
  uint16 r2 = spotlight_var3 + p;
  uint16 r0 = spotlight_var3 - p;
  r0 = ((int16)r0 < 0) ? 0 :
         r0 < 255 ? r0 : 255;
  r2 = r2 < 255 ? r2 : 255;
  r0 |= r2 << 8;
  return r0 == 0xffff ? 0xff : r0;
}

void AdjustWaterHDMAWindow() {  // 80f649
  uint16 r10 = water_hdma_var1 - BG2VOFS_copy2;
  spotlight_y_lower = r10 - water_hdma_var2;
  spotlight_y_upper = r10 + water_hdma_var2;
  AdjustWaterHDMAWindow_X(r10);
}

void AdjustWaterHDMAWindow_X(uint16 r10) {  // 80f660
  spotlight_var3 = water_hdma_var0 - BG2HOFS_copy2;
  uint16 r12 = water_hdma_var3 ? water_hdma_var3 - 1 : 0;
  uint16 r2 = spotlight_var3 + r12;
  uint16 r0 = spotlight_var3 - r12;

  r0 = (r0 < 255) ? r0 : 255;
  r2 = (r2 < 255) ? r2 : 255;
  r12 = r0 | r2 << 8;

  uint16 r6 = r10 * 2;
  if (r6 < 0xe0)
    r6 = 0xe0;
  uint16 r4 = 2 * r10 - r6;
  uint16 a;

  do {
    if (!sign16(r4)) {
      if (!sign16(spotlight_y_lower) && r4 < spotlight_y_lower)
        a = 0xff;
      else
        a = r12;
      if (r4 < 224)
        mode7_hdma_table[r4] = (a != 0xffff) ? a : 0xff;
    }
    if (r6 >= spotlight_y_upper) {
      a = 0xff;
    } else {
      if (r6 >= 225 && word_7E0678)
        word_7E0678--;
      a = r12;
    }
    if (r6 < 224)
      mode7_hdma_table[r6] = (a != 0xffff) ? a : 0xff;
  } while (r6--, r10 != r4++);
}

void FloodDam_PrepFloodHDMA() {  // 80f734
  spotlight_y_lower = water_hdma_var1 - BG2VOFS_copy2;
  spotlight_var3 = water_hdma_var0 - BG2HOFS_copy2;
  uint16 r14 = water_hdma_var3 ^ 1;
  uint16 r12 = (spotlight_var3 + r14) << 8 | (uint8)(spotlight_var3 - r14);

  int r4 = 0;
  do {
    mode7_hdma_table[r4] = 0xff00;
  } while (++r4 != spotlight_y_upper);

  r12 = r14 - 7 + 8;
  r12 = (spotlight_var3 + r12) << 8 | (uint8)(spotlight_var3 - r12);
  uint16 r10 = (spotlight_y_upper + water_hdma_var2) ^ 1;

  do {
    if (r4 >= r10) {
      mode7_hdma_table[r4] = 0xff;
    } else {
      uint16 a = r4;
      do {
        a *= 2;
      } while (a >= 448);
      mode7_hdma_table[a >> 1] = r12 == 0xffff ? 0xff : r12;
    }
  } while (++r4 < 225);
}

void ResetStarTileGraphics() {  // 80fda4
  byte_7E04BC = 0;
  Dungeon_RestoreStarTileChr();
}

void Dungeon_RestoreStarTileChr() {  // 80fda7
  int xx = 0, yy = 32;
  if (byte_7E04BC)
    xx = 32, yy = 0;
  uint16 *p = messaging_buf;
  memcpy(p, g_ram + 0xbdc0 + xx, 32);
  memcpy(p + 16, g_ram + 0xbdc0 + yy, 32);
  nmi_subroutine_index = 0x18;
}

void LinkZap_HandleMosaic() {  // 81fed2
  int level = mosaic_level;
  if (!mosaic_inc_or_dec) {
    level += 0x10;
    if (level == 0xc0)
      mosaic_inc_or_dec = 1;
  } else {
    level -= 0x10;
    if (level == 0)
      mosaic_inc_or_dec = 0;
  }
  mosaic_level = level;
  MOSAIC_copy = mosaic_level >> 1 | 3;
  BGMODE_copy = 9;
}

void Player_SetCustomMosaicLevel(uint8 a) {  // 81fef0
  mosaic_inc_or_dec = 0;
  mosaic_level = a;
  MOSAIC_copy = mosaic_level >> 1 | 3;
  BGMODE_copy = 9;
}

void Module07_16_UpdatePegs_Step1() {  // 829739
  if (BYTE(orange_blue_barrier_state))
    Dungeon_UpdatePegGFXBuffer(0x80, 0x100);
  else
    Dungeon_UpdatePegGFXBuffer(0x100, 0x80);
}

void Module07_16_UpdatePegs_Step2() {  // 82974d
  if (BYTE(orange_blue_barrier_state))
    Dungeon_UpdatePegGFXBuffer(0x100, 0x80);
  else
    Dungeon_UpdatePegGFXBuffer(0x80, 0x100);
}

void Dungeon_UpdatePegGFXBuffer(int x, int y) {  // 829773
  uint16 *src = (uint16 *)&g_ram[0xb340];
  for (int i = 0; i < 64; i++)
    messaging_buf[i] = src[(x >> 1) + i];
  for (int i = 0; i < 64; i++)
    messaging_buf[64 + i] = src[(y >> 1) + i];
  nmi_subroutine_index = 23;
}

void Dungeon_HandleTranslucencyAndPalette() {  // 82a1e9
  if (overworld_palette_swap_flag)
    Palette_RevertTranslucencySwap();

  CGWSEL_copy = 2;
  CGADSUB_copy = 0xb3;

  uint8 torch = dung_num_lit_torches;
  if (!dung_want_lights_out) {
    uint8 a = 0x20;
    if ((a = 0x20, dung_hdr_bg2_properties != 0) &&
        (a = 0x32, dung_hdr_bg2_properties != 7) &&
        (a = 0x62, dung_hdr_bg2_properties != 4) &&
        (a = 0x20, dung_hdr_bg2_properties == 2)) {
      Palette_AssertTranslucencySwap();
      if (BYTE(dungeon_room_index) == 13) {
        agahnim_pal_setting[0] = 0;
        agahnim_pal_setting[1] = 0;
        agahnim_pal_setting[2] = 0;
        agahnim_pal_setting[3] = 0;
        agahnim_pal_setting[4] = 0;
        agahnim_pal_setting[5] = 0;
        Palette_LoadAgahnim();
      }
      a = 0x70;
    }
    CGADSUB_copy = a;
    torch = 3;
  }
  overworld_fixed_color_plusminus = kLitTorchesColorPlus[torch];
  palette_filter_countdown = 31;
  mosaic_target_level = 0;
  darkening_or_lightening_screen = 2;
  overworld_palette_aux_or_main = 0;
  Palette_Load_DungeonSet();
  Palette_Load_SpritePal0Left();
  Palette_Load_SpriteAux1();
  Palette_Load_SpriteAux2();
  subsubmodule_index += 1;
}

void Overworld_LoadAllPalettes() {  // 82c5b2
  memset(aux_palette_buffer + 0x180 / 2, 0, 128);
  memset(main_palette_buffer, 0, 512);

  overworld_palette_mode = 5;
  overworld_palette_aux1_bp2to4_hi = 3;
  overworld_palette_aux2_bp5to7_hi = 3;
  overworld_palette_aux3_bp7_lo = 0;
  palette_sp6 = 5;
  overworld_palette_sp0 = 11;
  overworld_palette_swap_flag = 0;
  overworld_palette_aux_or_main = 0;
  Palette_BgAndFixedColor_Black();
  Palette_Load_SpritePal0Left();
  Palette_Load_SpriteMain();
  Palette_Load_OWBGMain();
  Palette_Load_OWBG1();
  Palette_Load_OWBG2();
  Palette_Load_OWBG3();
  Palette_Load_SpriteEnvironment_Dungeon();
  Palette_Load_HUD();

  for (int i = 0; i < 8; i++)
    main_palette_buffer[0x1b0 / 2 + i] = aux_palette_buffer[0x1d0 / 2 + i];
}

void Dungeon_LoadPalettes() {  // 82c630
  overworld_palette_aux_or_main = 0;
  Palette_BgAndFixedColor_Black();
  Palette_Load_SpritePal0Left();
  Palette_Load_SpriteMain();
  Palette_Load_SpriteAux1();
  Palette_Load_SpriteAux2();
  Palette_Load_Sword();
  Palette_Load_Shield();
  Palette_Load_SpriteEnvironment();
  Palette_Load_LinkArmorAndGloves();
  Palette_Load_HUD();
  Palette_Load_DungeonSet();
  Overworld_LoadPalettesInner();
}

void Overworld_LoadPalettesInner() {  // 82c65f
  overworld_pal_unk1 = dung_hdr_palette_1;
  overworld_pal_unk2 = overworld_palette_aux3_bp7_lo;
  overworld_pal_unk3 = byte_7E0AB7;
  darkening_or_lightening_screen = 2;
  palette_filter_countdown = 0;
  WORD(mosaic_target_level) = 0;
  Overworld_CopyPalettesToCache();
}

void OverworldLoadScreensPaletteSet() {  // 82c692
  uint8 sc = overworld_screen_index & 0x3f;
  uint8 x = (sc == 3 || sc == 5 || sc == 7) ? 2 : 0;
  x += (overworld_screen_index & 0x40) ? 1 : 0;
  Overworld_LoadAreaPalettesEx(x);
}

void Overworld_LoadAreaPalettesEx(uint8 x) {  // 82c6ad
  overworld_palette_mode = x;
  overworld_palette_aux_or_main &= 0xff;
  Palette_Load_SpriteMain();
  Palette_Load_SpriteEnvironment();
  Palette_Load_SpriteAux1();
  Palette_Load_SpriteAux2();
  Palette_Load_Sword();
  Palette_Load_Shield();
  Palette_Load_LinkArmorAndGloves();
  overworld_palette_sp0 = (savegame_is_darkworld & 0x40) ? 3 : 1;
  Palette_Load_SpritePal0Left();
  Palette_Load_HUD();
  Palette_Load_OWBGMain();
}

void SpecialOverworld_CopyPalettesToCache() {  // 82c6eb
  for (int i = 32; i < 32 * 8; i++)
    main_palette_buffer[i] = 0;
  for (int i = 0; i < 8; i++) {
    main_palette_buffer[i] = aux_palette_buffer[i];
    main_palette_buffer[i + 0x8] = aux_palette_buffer[i + 0x8];
    main_palette_buffer[i + 0x10] = aux_palette_buffer[i + 0x10];
    main_palette_buffer[i + 0x18] = aux_palette_buffer[i + 0x18];
    main_palette_buffer[i + 0xd8] = aux_palette_buffer[i + 0xd8];
    main_palette_buffer[i + 0xe8] = aux_palette_buffer[i + 0xe8];
    main_palette_buffer[i + 0xf0] = aux_palette_buffer[i + 0xf0];
    main_palette_buffer[i + 0xf8] = aux_palette_buffer[i + 0xf8];
  }
  MOSAIC_copy = 0xf7;
  mosaic_level = 0xf7;
  flag_update_cgram_in_nmi++;
}

void Overworld_CopyPalettesToCache() {  // 82c769
  memcpy(main_palette_buffer, aux_palette_buffer, 512);
  flag_update_cgram_in_nmi += 1;
}

void Overworld_LoadPalettes(uint8 bg, uint8 spr) {  // 8ed5a8
  overworld_palette_aux_or_main = 0;

  const int8 *d = kOwBgPalInfo + bg * 3;
  if (d[0] >= 0)
    overworld_palette_aux1_bp2to4_hi = d[0];
  if (d[1] >= 0)
    overworld_palette_aux2_bp5to7_hi = d[1];
  if (d[2] >= 0)
    overworld_palette_aux3_bp7_lo = d[2];

  d = kOwSprPalInfo + spr * 2;
  if (d[0] >= 0)
    sprite_aux1_palette = d[0];
  if (d[1] >= 0)
    sprite_aux2_palette = d[1];
  Palette_Load_OWBG1();
  Palette_Load_OWBG2();
  Palette_Load_OWBG3();
  Palette_Load_SpriteAux1();
  Palette_Load_SpriteAux2();
}

void Palette_BgAndFixedColor_Black() {  // 8ed5f4
  Palette_SetBgAndFixedColor(0);
}

void Palette_SetBgAndFixedColor(uint16 color) {  // 8ed5f9
  main_palette_buffer[0] = color;
  main_palette_buffer[32] = color;
  aux_palette_buffer[0] = color;
  aux_palette_buffer[32] = color;
  SetBackdropcolorBlack();
}

void SetBackdropcolorBlack() {  // 8ed60b
  COLDATA_copy0 = 0x20;
  COLDATA_copy1 = 0x40;
  COLDATA_copy2 = 0x80;
}

void Palette_SetOwBgColor() {  // 8ed618
  Palette_SetBgAndFixedColor(Palette_GetOwBgColor());
}

void Palette_SpecialOw() {  // 8ed61d
  uint16 c = Palette_GetOwBgColor();
  aux_palette_buffer[0] = c;
  aux_palette_buffer[32] = c;
  SetBackdropcolorBlack();
}

uint16 Palette_GetOwBgColor() {  // 8ed622
  if (overworld_screen_index < 0x80)
    return overworld_screen_index & 0x40 ? 0x2A32 : 0x2669;
  if (dungeon_room_index == 0x180 || dungeon_room_index == 0x182 || dungeon_room_index == 0x183)
    return 0x19C6;
  return 0x2669;
}

void Palette_AssertTranslucencySwap() {  // 8ed657
  Palette_SetTranslucencySwap(true);
}

void Palette_SetTranslucencySwap(bool v) {  // 8ed65c
  overworld_palette_swap_flag = v;
  uint16 a, b;
  for (int i = 0; i < 8; i++) {
    a = aux_palette_buffer[i + 0x80];
    b = aux_palette_buffer[i + 0xf0];
    main_palette_buffer[i + 0xf0] = aux_palette_buffer[i + 0xf0] = a;
    main_palette_buffer[i + 0x80] = aux_palette_buffer[i + 0x80] = b;

    a = aux_palette_buffer[i + 0x88];
    b = aux_palette_buffer[i + 0xf8];
    main_palette_buffer[i + 0xf8] = aux_palette_buffer[i + 0xf8] = a;
    main_palette_buffer[i + 0x88] = aux_palette_buffer[i + 0x88] = b;

    a = aux_palette_buffer[i + 0xb8];
    b = aux_palette_buffer[i + 0xd8];
    main_palette_buffer[i + 0xd8] = aux_palette_buffer[i + 0xd8] = a;
    main_palette_buffer[i + 0xb8] = aux_palette_buffer[i + 0xb8] = b;
  }
  flag_update_cgram_in_nmi++;
}

void Palette_RevertTranslucencySwap() {  // 8ed6bb
  Palette_SetTranslucencySwap(false);
}

void LoadActualGearPalettes() {  // 8ed6c0
  LoadGearPalettes(link_sword_type, link_shield_type, link_armor);
}

void Palette_ElectroThemedGear() {  // 8ed6d1
  LoadGearPalettes(2, 2, 4);
}

void LoadGearPalettes_bunny() {  // 8ed6dd
  LoadGearPalettes(link_sword_type, link_shield_type, 3);
}

void LoadGearPalettes(uint8 sword, uint8 shield, uint8 armor) {  // 8ed6e8
  const uint16 *src = kPalette_Sword + (sword && sword != 255 ? sword - 1 : 0) * 3;
  Palette_LoadMultiple_Arbitrary(src, 0x1b2, 2);

  src = kPalette_Shield + (shield ? shield - 1 : 0) * 4;
  Palette_LoadMultiple_Arbitrary(src, 0x1b8, 3);

  src = kPalette_ArmorAndGloves + armor * 15;
  Palette_LoadMultiple_Arbitrary(src, 0x1e2, 14);
  flag_update_cgram_in_nmi++;
}

void LoadGearPalette(int dst, const uint16 *src, int n) {  // 8ed741
  memcpy(&aux_palette_buffer[dst >> 1], src, sizeof(uint16) * n);
  memcpy(&main_palette_buffer[dst >> 1], src, sizeof(uint16) * n);
}

void Filter_Majorly_Whiten_Bg() {  // 8ed757
  for (int i = 32; i < 128; i++)
    main_palette_buffer[i] = Filter_Majorly_Whiten_Color(aux_palette_buffer[i]);
  main_palette_buffer[0] = aux_palette_buffer[0] ? main_palette_buffer[32] : 0;
}

uint16 Filter_Majorly_Whiten_Color(uint16 c) {  // 8ed7fe
  int r = (c & 0x1f) + 14;
  if (r > 0x1f) r = 0x1f;
  int g = (c & 0x3e0) + 0x1c0;
  if (g > 0x3e0) g = 0x3e0;
  int b = (c & 0x7c00) + 0x3800;
  if (b > 0x7c00) b = 0x7c00;
  return r | g | b;
}

void Palette_Restore_BG_From_Flash() {  // 8ed83a
  for (int i = 32; i < 128; i++)
    main_palette_buffer[i] = aux_palette_buffer[i];
  main_palette_buffer[0] = main_palette_buffer[32];
  Palette_Restore_Coldata();
}

void Palette_Restore_Coldata() {  // 8ed8ae
  if (!player_is_indoors) {
    uint32 rgb;
    switch (BYTE(overworld_screen_index)) {
    case 3: case 5: case 7:
      rgb = 0x8c4c26;
      break;
    case 0x43: case 0x45: case 0x47:
      rgb = 0x874a26;
      break;
    case 0x5b:
      rgb = 0x894f33;
      break;
    default:
      rgb = 0x804020;
    }
    COLDATA_copy0 = (uint8)(rgb);
    COLDATA_copy1 = (uint8)(rgb >> 8);
    COLDATA_copy2 = (uint8)(rgb >> 16);
  }
}

void Palette_Restore_BG_And_HUD() {  // 8ed8fb
  memcpy(main_palette_buffer, aux_palette_buffer, 256);
  flag_update_cgram_in_nmi++;
  Palette_Restore_Coldata();
}

void Palette_Load_SpritePal0Left() {  // 9bec77
  const uint16 *src = kPalette_SpriteAux3 + overworld_palette_sp0 * 7;
  Palette_LoadSingle(src, overworld_palette_swap_flag ? 0x1e2 : 0x102, 6);
}

void Palette_Load_SpriteMain() {  // 9bec9e
  const uint16 *src = kPalette_MainSpr + (overworld_screen_index & 0x40 ? 60 : 0);
  Palette_LoadMultiple(src, 0x122, 14, 3);
}

void Palette_Load_SpriteAux1() {  // 9becc5
  const uint16 *src = kPalette_SpriteAux1 + (sprite_aux1_palette) * 7;
  Palette_LoadSingle(src, 0x1A2, 6);
}

void Palette_Load_SpriteAux2() {  // 9bece4
  const uint16 *src = kPalette_SpriteAux1 + (sprite_aux2_palette) * 7;
  Palette_LoadSingle(src, 0x1C2, 6);
}

void Palette_Load_Sword() {  // 9bed03
  const uint16 *src = kPalette_Sword + ((int8)link_sword_type > 0 ? link_sword_type - 1 : 0) * 3;  // wtf: zelda reads offset 0xff
  Palette_LoadMultiple_Arbitrary(src, 0x1b2, 2);
  flag_update_cgram_in_nmi += 1;
}

void Palette_Load_Shield() {  // 9bed29
  const uint16 *src = kPalette_Shield + (link_shield_type ? link_shield_type - 1 : 0) * 4;
  Palette_LoadMultiple_Arbitrary(src, 0x1b8, 3);
  flag_update_cgram_in_nmi += 1;
}

void Palette_Load_SpriteEnvironment() {  // 9bed6e
  if (player_is_indoors)
    Palette_Load_SpriteEnvironment_Dungeon();
  else
    Palette_MiscSprite_Outdoors();
}

void Palette_Load_SpriteEnvironment_Dungeon() {  // 9bed72
  const uint16 *src = kPalette_MiscSprite_Indoors + palette_sp6 * 7;
  Palette_LoadSingle(src, 0x1d2, 6);
}

void Palette_MiscSprite_Outdoors() {  // 9bed91
  int t = (overworld_screen_index & 0x40) ? 9 : 7;
  const uint16 *src = kPalette_MiscSprite_Indoors + t * 7;
  Palette_LoadSingle(src, overworld_palette_swap_flag ? 0x1f2 : 0x112, 6);
  src = kPalette_MiscSprite_Indoors + (t - 1) * 7;
  Palette_LoadSingle(src, 0x1d2, 6);
}

void Palette_Load_DungeonMapSprite() {  // 9beddd
  Palette_LoadMultiple(kPalette_PalaceMapSpr, 0x182, 6, 2);
}

void Palette_Load_LinkArmorAndGloves() {  // 9bedf9
  const uint16 *src = kPalette_ArmorAndGloves + link_armor * 15;
  Palette_LoadMultiple_Arbitrary(src, 0x1e2, 14);
  Palette_UpdateGlovesColor();
}

void Palette_UpdateGlovesColor() {  // 9bee1b
  if (link_item_gloves)
    main_palette_buffer[0xfd] = aux_palette_buffer[0xfd] = kGlovesColor[link_item_gloves - 1];
  flag_update_cgram_in_nmi += 1;
}

void Palette_Load_DungeonMapBG() {  // 9bee3a
  Palette_LoadMultiple(kPalette_PalaceMapBg, 0x40, 15, 5);
}

void Palette_Load_HUD() {  // 9bee52
  const uint16 *src = kHudPalData + hud_palette * 32;
  Palette_LoadMultiple(src, 0x0, 15, 1);
}

void Palette_Load_DungeonSet() {  // 9bee74
  const uint16 *src = kPalette_DungBgMain + (dung_hdr_palette_1 >> 1) * 90;
  Palette_LoadMultiple(src, 0x42, 14, 5);
  Palette_LoadSingle(src, overworld_palette_swap_flag ? 0x1f2 : 0x112, 6);
}

void Palette_Load_OWBG3() {  // 9beea8
  const uint16 *src = kPalette_OverworldBgAux3 + overworld_palette_aux3_bp7_lo * 7;
  Palette_LoadSingle(src, 0xE2, 6);
}

void Palette_Load_OWBGMain() {  // 9beec7
  const uint16 *src = kPalette_OverworldBgMain + overworld_palette_mode * 35;
  Palette_LoadMultiple(src, 0x42, 6, 4);
}

void Palette_Load_OWBG1() {  // 9beee8
  const uint16 *src = kPalette_OverworldBgAux12 + overworld_palette_aux1_bp2to4_hi * 21;
  Palette_LoadMultiple(src, 0x52, 6, 2);
}

void Palette_Load_OWBG2() {  // 9bef0c
  const uint16 *src = kPalette_OverworldBgAux12 + overworld_palette_aux2_bp5to7_hi * 21;
  Palette_LoadMultiple(src, 0xB2, 6, 2);
}

void Palette_LoadSingle(const uint16 *src, int dst, int x_ents) {  // 9bef30
  memcpy(&aux_palette_buffer[(dst + overworld_palette_aux_or_main) >> 1], src, sizeof(uint16) * (x_ents + 1));
}

void Palette_LoadMultiple(const uint16 *src, int dst, int x_ents, int y_pals) {  // 9bef4b
  x_ents++;
  do {
    memcpy(&aux_palette_buffer[(dst + overworld_palette_aux_or_main) >> 1], src, sizeof(uint16) * x_ents);
    src += x_ents;
    dst += 32;
  } while (--y_pals >= 0);
}

void Palette_LoadMultiple_Arbitrary(const uint16 *src, int dst, int x_ents) {  // 9bef7b
  memcpy(&aux_palette_buffer[dst >> 1], src, sizeof(uint16) * (x_ents + 1));
  memcpy(&main_palette_buffer[dst >> 1], src, sizeof(uint16) * (x_ents + 1));
}

void Palette_LoadForFileSelect() {  // 9bef96
  uint8 *src = g_zenv.sram;
  for (int i = 0; i < 3; i++) {
    Palette_LoadForFileSelect_Armor(i * 0x20, src[kSrmOffs_Armor], src[kSrmOffs_Gloves]);
    Palette_LoadForFileSelect_Sword(i * 0x20, src[kSrmOffs_Sword]);
    Palette_LoadForFileSelect_Shield(i * 0x20, src[kSrmOffs_Shield]);
    src += 0x500;
  }
  for (int i = 0; i < 7; i++) {
    aux_palette_buffer[0xe8 + i] = main_palette_buffer[0xe8 + i] = kPalette_MainSpr[7 + i];
    aux_palette_buffer[0xf8 + i] = main_palette_buffer[0xf8 + i] = kPalette_MainSpr[15 + 7 + i];
  }
}

void Palette_LoadForFileSelect_Armor(int k, uint8 armor, uint8 gloves) {  // 9bf032
  const uint16 *pal = kPalette_ArmorAndGloves + armor * 15;
  for (int i = 0; i != 15; i++)
    aux_palette_buffer[k + 0x81 + i] = main_palette_buffer[k + 0x81 + i] = pal[i];
  if (gloves)
    aux_palette_buffer[k + 0x8d] = main_palette_buffer[k + 0x8d] = kGlovesColor[gloves - 1];
}

void Palette_LoadForFileSelect_Sword(int k, uint8 sword) {  // 9bf072
  const uint16 *src = kPalette_Sword + (sword ? sword - 1 : 0) * 3;
  for (int i = 0; i != 3; i++)
    aux_palette_buffer[k + 0x99 + i] = main_palette_buffer[k + 0x99 + i] = src[i];
}

void Palette_LoadForFileSelect_Shield(int k, uint8 shield) {  // 9bf09a
  const uint16 *src = kPalette_Shield + (shield ? shield - 1 : 0) * 4;
  for (int i = 0; i != 4; i++)
    aux_palette_buffer[k + 0x9c + i] = main_palette_buffer[k + 0x9c + i] = src[i];
}

void Palette_LoadAgahnim() {  // 9bf0c2
  const uint16 *src = kPalette_SpriteAux1 + 14 * 7;
  Palette_LoadMultiple_Arbitrary(src, 0x162, 6);
  Palette_LoadMultiple_Arbitrary(src, 0x182, 6);
  Palette_LoadMultiple_Arbitrary(src, 0x1a2, 6);
  src = kPalette_SpriteAux1 + 21 * 7;
  Palette_LoadMultiple_Arbitrary(src, 0x1c2, 6);
  flag_update_cgram_in_nmi++;
}

void HandleScreenFlash() {  // 9de9b6
  int j = intro_times_pal_flash;
  if (!j || submodule_index != 0)
    return;
  if (!--intro_times_pal_flash) {
    Palette_Restore_BG_And_HUD();
    return;
  }

  if (j & 1)
    Filter_Majorly_Whiten_Bg();
  else
    Palette_Restore_BG_From_Flash();

  flag_update_cgram_in_nmi++;
}

