#include "zelda_rtl.h"
#include "variables.h"
#include "snes/snes_regs.h"
#include "load_gfx.h"
#include "dungeon.h"
#include "sprite.h"
#include "ending.h"
#include "messaging.h"
#include "attract.h"
#include "sprite_main.h"

const uint16 kMapMode_Zooms1[224] = {
  375, 374, 373, 373, 372, 371, 371, 370, 369, 369, 368, 367, 367, 366, 365, 365,
  364, 363, 363, 361, 361, 360, 359, 359, 358, 357, 357, 356, 355, 355, 354, 354,
  353, 352, 352, 351, 351, 350, 349, 349, 348, 348, 347, 346, 346, 345, 345, 344,
  343, 343, 342, 342, 341, 341, 340, 339, 339, 338, 338, 337, 337, 336, 335, 335,
  334, 334, 333, 333, 332, 332, 331, 331, 330, 330, 328, 327, 327, 326, 326, 325,
  325, 324, 324, 323, 323, 322, 322, 321, 321, 320, 320, 319, 319, 318, 318, 317,
  317, 316, 316, 315, 315, 314, 314, 313, 313, 312, 312, 311, 311, 310, 310, 309,
  309, 309, 308, 308, 307, 307, 306, 306, 305, 305, 304, 304, 303, 303, 303, 302,
  302, 301, 301, 300, 300, 299, 299, 299, 298, 298, 297, 297, 295, 295, 294, 294,
  294, 293, 293, 292, 292, 292, 291, 291, 290, 290, 289, 289, 289, 288, 288, 287,
  287, 287, 286, 286, 285, 285, 285, 284, 284, 283, 283, 283, 282, 282, 281, 281,
  281, 280, 280, 279, 279, 279, 278, 278, 278, 277, 277, 276, 276, 276, 275, 275,
  275, 274, 274, 273, 273, 273, 272, 272, 272, 271, 271, 271, 270, 270, 269, 269,
  269, 268, 268, 268, 267, 267, 267, 266, 266, 266, 265, 265, 265, 264, 264, 264,
};
const uint16 kMapMode_Zooms2[224] = {
  136, 136, 135, 135, 135, 135, 135, 134, 134, 134, 133, 133, 133, 133, 132, 132,
  132, 132, 132, 131, 131, 131, 130, 130, 130, 130, 130, 129, 129, 129, 129, 129,
  128, 128, 128, 127, 127, 127, 127, 127, 126, 126, 126, 126, 126, 125, 125, 125,
  124, 124, 124, 124, 124, 124, 123, 123, 123, 123, 123, 122, 122, 122, 121, 121,
  121, 121, 121, 121, 120, 120, 120, 120, 120, 120, 119, 119, 119, 118, 118, 118,
  118, 118, 118, 117, 117, 117, 117, 117, 117, 116, 116, 116, 116, 115, 115, 115,
  115, 115, 115, 114, 114, 114, 114, 114, 114, 113, 113, 113, 113, 112, 112, 112,
  112, 112, 112, 112, 111, 111, 111, 111, 111, 111, 110, 110, 110, 110, 110, 109,
  109, 109, 109, 109, 109, 108, 108, 108, 108, 108, 108, 108, 107, 107, 107, 107,
  107, 106, 106, 106, 106, 106, 106, 106, 105, 105, 105, 105, 105, 105, 105, 104,
  104, 104, 104, 104, 103, 103, 103, 103, 103, 103, 103, 103, 102, 102, 102, 102,
  102, 102, 102, 101, 101, 101, 101, 101, 101, 100, 100, 100, 100, 100, 100, 100,
  100,  99,  99,  99,  99,  99,  99,  99,  99,  98,  98,  98,  98,  98,  97,  97,
  97,  97,  97,  97,  97,  97,  97,  96,  96,  96,  96,  96,  96,  96,  96,  96,
};
static const uint8 kAttract_Legendgraphics_0[157+1] = {
  0x61, 0x65, 0x40, 0x28,    0, 0x35, 0x61, 0x85, 0x40, 0x28, 0x10, 0x35, 0x61, 0xa5,    0, 0x29,
     1, 0x35,    2, 0x35,    1, 0x35,    2, 0x35,    1, 0x35,    2, 0x35,    1, 0x35,    2, 0x35,
     1, 0x35,    3, 0x31,    3, 0x71,    2, 0x35,    1, 0x35,    2, 0x35,    1, 0x35,    2, 0x35,
     1, 0x35,    2, 0x35,    1, 0x35,    2, 0x35,    1, 0x35, 0x61, 0xc5,    0, 0x29, 0x11, 0x35,
  0x12, 0x35, 0x11, 0x35, 0x12, 0x35, 0x11, 0x35, 0x12, 0x35, 0x11, 0x35, 0x12, 0x35, 0x11, 0x35,
  0x13, 0x35, 0x13, 0x75, 0x12, 0x35, 0x11, 0x35, 0x12, 0x35, 0x11, 0x35, 0x12, 0x35, 0x11, 0x35,
  0x12, 0x35, 0x11, 0x35, 0x12, 0x35, 0x11, 0x35, 0x61, 0xe5,    0, 0x29, 0x20, 0x35, 0x21, 0x35,
  0x20, 0x35, 0x21, 0x35, 0x20, 0x35, 0x21, 0x35, 0x20, 0x35, 0x21, 0x35, 0x20, 0x35, 0x21, 0x35,
  0x20, 0x35, 0x21, 0x35, 0x20, 0x35, 0x21, 0x35, 0x20, 0x35, 0x21, 0x35, 0x20, 0x35, 0x21, 0x35,
  0x20, 0x35, 0x21, 0x35, 0x20, 0x35, 0x62,    5, 0x40, 0x28,    0, 0xb5, 0xff, 0x61,
};
static const uint8 kAttract_Legendgraphics_1[237+1] = {
  0x61, 0x65, 0x40, 0x28,    0, 0x35, 0x61, 0x85,    0, 0x13, 0x10, 0x35, 0x4e, 0x75, 0x6e, 0x35,
  0x10, 0x35, 0x4e, 0x35, 0x10, 0x35, 0x4c, 0x35, 0x10, 0x35, 0x4e, 0x75, 0x49, 0x35, 0x61, 0x8f,
  0x40,    8, 0x10, 0x35, 0x61, 0x94,    0,  0xb, 0x4e, 0x75, 0x6e, 0x35, 0x10, 0x35, 0x4e, 0x35,
  0x10, 0x35, 0x4c, 0x35, 0x61, 0xa5,    0, 0x29, 0x5f, 0x75, 0x5e, 0x75, 0x7e, 0x35, 0x7f, 0x35,
  0x5e, 0x35, 0x5f, 0x35, 0x4d, 0x35, 0x5f, 0x75, 0x5e, 0x75, 0x4a, 0x35, 0x4b, 0x35, 0x10, 0x35,
  0x49, 0x75, 0x10, 0x35, 0x5f, 0x75, 0x5e, 0x75, 0x7e, 0x35, 0x7f, 0x35, 0x5e, 0x35, 0x5f, 0x35,
  0x4d, 0x35, 0x61, 0xc5,    0, 0x29, 0x50, 0x35, 0x51, 0x35, 0x52, 0x35, 0x53, 0x35, 0x54, 0x35,
  0x55, 0x35, 0x56, 0x35, 0x57, 0x35, 0x58, 0x35, 0x59, 0x35, 0x5a, 0x35, 0x5b, 0x35, 0x5c, 0x35,
  0x5d, 0x35, 0x50, 0x35, 0x51, 0x35, 0x52, 0x35, 0x53, 0x35, 0x54, 0x35, 0x55, 0x35, 0x56, 0x35,
  0x61, 0xe5,    0, 0x29, 0x60, 0x35, 0x61, 0x35, 0x62, 0x35, 0x63, 0x35, 0x64, 0x35, 0x65, 0x35,
  0x66, 0x35, 0x67, 0x35, 0x68, 0x35, 0x69, 0x35, 0x6a, 0x35, 0x6b, 0x35, 0x6c, 0x35, 0x6d, 0x35,
  0x60, 0x35, 0x61, 0x35, 0x62, 0x35, 0x63, 0x35, 0x64, 0x35, 0x65, 0x35, 0x66, 0x35, 0x62,    5,
     0, 0x29, 0x70, 0x35, 0x71, 0x35, 0x72, 0x35, 0x73, 0x35, 0x74, 0x35, 0x75, 0x35, 0x76, 0x35,
  0x77, 0x35, 0x78, 0x35, 0x79, 0x35, 0x7a, 0x35, 0x7b, 0x35, 0x7c, 0x35, 0x7d, 0x35, 0x70, 0x35,
  0x71, 0x35, 0x72, 0x35, 0x73, 0x35, 0x74, 0x35, 0x75, 0x35, 0x76, 0x35, 0xff, 0x61,
};
static const uint8 kAttract_Legendgraphics_2[199+1] = {
  0x61, 0x65, 0x40, 0x28,    0, 0x35, 0x61, 0x85, 0x40, 0x28, 0x10, 0x35, 0x61, 0xa5,    0, 0x1d,
  0x22, 0x35, 0x23, 0x35, 0x10, 0x35, 0x22, 0x35, 0x23, 0x35, 0x10, 0x35, 0x22, 0x35, 0x23, 0x35,
  0x10, 0x35, 0x22, 0x35, 0x23, 0x35, 0x10, 0x35, 0x10, 0x75, 0x23, 0x75, 0x22, 0x75, 0x61, 0xb4,
  0x40,    6, 0x10, 0x35, 0x61, 0xb8,    0,    3, 0x23, 0x75, 0x22, 0x75, 0x61, 0xc5,    0, 0x29,
     4, 0x35,    5, 0x35,    6, 0x35,    4, 0x35,    5, 0x35,    6, 0x35,    4, 0x35,    5, 0x35,
     6, 0x35,    4, 0x35,    5, 0x35,    6, 0x35,    6, 0x75,    5, 0x75,    4, 0x75, 0x10, 0x75,
  0x23, 0x75, 0x22, 0x75,    6, 0x75,    5, 0x75,    4, 0x75, 0x61, 0xe5,    0, 0x29, 0x14, 0x35,
  0x15, 0x35, 0x16, 0x35, 0x14, 0x35, 0x15, 0x35, 0x16, 0x35, 0x14, 0x35, 0x15, 0x35, 0x16, 0x35,
  0x14, 0x35, 0x15, 0x35, 0x16, 0x35, 0x16, 0x75, 0x15, 0x75, 0x14, 0x75,    6, 0x75,    5, 0x75,
     4, 0x75, 0x16, 0x75, 0x15, 0x75, 0x14, 0x75, 0x62,    5,    0, 0x29, 0x24, 0x35, 0x25, 0x35,
  0x26, 0x35, 0x24, 0x35, 0x25, 0x35, 0x26, 0x35, 0x24, 0x35, 0x25, 0x35, 0x26, 0x35, 0x24, 0x35,
  0x25, 0x35, 0x26, 0x35, 0x26, 0x75, 0x25, 0x75, 0x24, 0x75, 0x26, 0x75, 0x25, 0x75, 0x24, 0x75,
  0x26, 0x75, 0x25, 0x75, 0x24, 0x75, 0xff, 0x61,
};
static const uint8 kAttract_Legendgraphics_3[265+1] = {
  0x61, 0x65,    0, 0x29,    0, 0x35,    0, 0x35, 0x1b, 0x35, 0x30, 0x35, 0x31, 0x35, 0x32, 0x35,
     0, 0x35,    0, 0x35,    0, 0x35, 0x33, 0x35, 0x41, 0x35, 0x41, 0x75, 0x33, 0x75,    0, 0x75,
     0, 0x75,    0, 0x75, 0x32, 0x75, 0x31, 0x75, 0x30, 0x75, 0x1b, 0x75,    0, 0x75, 0x61, 0x85,
  0x40, 0x1e, 0x10, 0x35, 0x61, 0x86,    0,    9, 0x34, 0x35,  0xb, 0x35, 0x40, 0x35, 0x41, 0x35,
  0x42, 0x35, 0x61, 0x95,    0,    9, 0x42, 0x75, 0x41, 0x75, 0x40, 0x75,  0xb, 0x75, 0x34, 0x75,
  0x61, 0xa5,    0, 0x29, 0x43, 0x35, 0x44, 0x35,    7, 0x35,    8, 0x35,    9, 0x35,  0xa, 0x35,
  0x10, 0x35,  0xc, 0x35,  0xd, 0x35,  0xe, 0x35,  0xf, 0x35,  0xf, 0x75,  0xe, 0x75,  0xd, 0x75,
   0xc, 0x75, 0x10, 0x75,  0xa, 0x75,    9, 0x75,    8, 0x75,    7, 0x75, 0x44, 0x75, 0x61, 0xc5,
     0, 0x29, 0x35, 0x35, 0x36, 0x35, 0x17, 0x35, 0x18, 0x35, 0x19, 0x35, 0x1a, 0x35, 0x10, 0x35,
  0x1c, 0x35, 0x1d, 0x35, 0x1e, 0x35, 0x1f, 0x35, 0x1f, 0x75, 0x1e, 0x75, 0x1d, 0x75, 0x1c, 0x75,
  0x10, 0x75, 0x1a, 0x75, 0x19, 0x75, 0x18, 0x75, 0x17, 0x75, 0x36, 0x75, 0x61, 0xe5,    0, 0x29,
  0x45, 0x35, 0x46, 0x35, 0x27, 0x35, 0x28, 0x35, 0x29, 0x35, 0x2a, 0x35, 0x2b, 0x35, 0x2c, 0x35,
  0x2d, 0x35, 0x2e, 0x35, 0x2f, 0x35, 0x2f, 0x75, 0x2e, 0x75, 0x2d, 0x75, 0x2c, 0x75, 0x2b, 0x75,
  0x2a, 0x75, 0x29, 0x75, 0x28, 0x75, 0x27, 0x75, 0x46, 0x75, 0x62,    5,    0, 0x29, 0x47, 0x35,
  0x48, 0x35, 0x37, 0x35, 0x38, 0x35, 0x39, 0x35, 0x3a, 0x35, 0x3b, 0x35, 0x3c, 0x35, 0x3d, 0x35,
  0x3e, 0x35, 0x3f, 0x35, 0x3f, 0x75, 0x3e, 0x75, 0x3d, 0x75, 0x3c, 0x75, 0x3b, 0x75, 0x3a, 0x75,
  0x39, 0x75, 0x38, 0x75, 0x37, 0x75, 0x48, 0x75, 0xff, 0x0
};
void Attract_DrawSpriteSet2(const AttractOamInfo *p, int n) {
  OamEnt *oam = &oam_buf[attract_oam_idx + 64];
  attract_oam_idx += n;
  for (; n--; oam++) {
    oam->x = attract_x_base + p[n].x;
    oam->y = attract_y_base + p[n].y;
    oam->charnum = p[n].c;
    oam->flags = p[n].f;
    bytewise_extended_oam[oam - oam_buf] = p[n].e;
  }
}

void Attract_ZeldaPrison_Case0() {
  static const AttractOamInfo kZeldaPrison_Oams0[] = {
    { 5, 25, 0x6c, 0x38, 2},
    {11, 25, 0x6c, 0x38, 2},
    { 0,  0, 0x84, 0x3b, 2},
    {16,  0, 0x84, 0x7b, 2},
    { 0, 16, 0xa4, 0x3b, 2},
    {16, 16, 0xa4, 0x7b, 2},
  };
  if (!attract_var4)
    attract_var5++;
  if (frame_counter & 1)
    attract_vram_dst--;
  attract_x_base = 0x58;
  attract_y_base = attract_var9;
  Attract_DrawSpriteSet2(kZeldaPrison_Oams0, 6);
  attract_var7 = 0xf8d9;
}

void Attract_ZeldaPrison_Case1() {
  int k;
  static const AttractOamInfo kZeldaPrison_Oams1[] = {
    { 5, 25, 0x6c, 0x38, 2},
    {11, 25, 0x6c, 0x38, 2},
    { 0,  0, 0x84, 0x3b, 2},
    {16,  0, 0x84, 0x7b, 2},
    { 0, 16, 0xa4, 0x3b, 2},
    {16, 16, 0xa4, 0x7b, 2},

    { 5, 25, 0x6c, 0x38, 2},
    {11, 25, 0x6c, 0x38, 2},
    { 0,  0, 0xc4, 0x3b, 2},
    {16,  0, 0xc2, 0x3b, 2},
    { 0, 16, 0xe4, 0x3b, 2},
    {16, 16, 0xe6, 0x3b, 2},

    { 5, 25, 0x6c, 0x38, 2},
    {11, 25, 0x6c, 0x38, 2},
    { 0,  0, 0x88, 0x3b, 2},
    {16,  0, 0x8a, 0x3b, 2},
    { 0, 16, 0xa8, 0x3b, 2},
    {16, 16, 0xaa, 0x3b, 2},

    { 5, 25, 0x6c, 0x38, 2},
    {11, 25, 0x6c, 0x38, 2},
    { 0,  0, 0x82, 0x3b, 2},
    {16,  0, 0x82, 0x7b, 2},
    { 0, 16, 0xa2, 0x3b, 2},
    {16, 16, 0xa2, 0x7b, 2},

    { 5, 25, 0x6c, 0x38, 2},
    {11, 25, 0x6c, 0x38, 2},
    { 0,  0, 0x80, 0x3b, 2},
    {16,  0, 0x80, 0x7b, 2},
    { 0, 16, 0xa0, 0x3b, 2},
    {16, 16, 0xa0, 0x7b, 2},
  };
  if (attract_var10 < 0x80 && (Attract_ShowTimedTextMessage(), oam_priority_value != 0)) {
    k = 4;
  } else if (attract_var9 != 0x6e) {
    attract_var9--;
    k = 0;
  } else {
    if (attract_var10 < 31 && !(attract_var10 & 1))
      INIDISP_copy--;
    if (!--attract_var10) {
      attract_sequence++;
      attract_state -= 2;
      return;
    }

    k = attract_var10 >= 0xc0 ? 0 :
      attract_var10 >= 0xb8 ? 1 :
      attract_var10 >= 0xb0 ? 2 :
      attract_var10 >= 0xa0 ? 3 : 4;
  }
  if (frame_counter & 1)
    attract_vram_dst--;
  attract_x_base = 0x58;
  attract_y_base = attract_var9;
  Attract_DrawSpriteSet2(&kZeldaPrison_Oams1[k * 6], 6);
}

void Attract_ZeldaPrison_DrawA() {
  OamEnt *oam = &oam_buf[64 + attract_oam_idx];

  uint8 ext = attract_x_base_hi ? 3 : 2;
  bytewise_extended_oam[oam - oam_buf] = ext;
  bytewise_extended_oam[oam - oam_buf + 1] = ext;

  oam[0].x = oam[1].x = attract_x_base;
  int j = (attract_var1 >> 3) & 1;
  oam[0].y = attract_y_base + j;
  oam[1].y = attract_y_base + 10;
  oam[0].charnum = 6;
  oam[1].charnum = j ? 10 : 8;
  oam[0].flags = oam[1].flags = 0x3d;

  attract_oam_idx += 2;
}

void Attract_MaidenWarp_Case0() {
  if (attract_var11)
    attract_var5++;
}

void Attract_MaidenWarp_Case1() {
  static const AttractOamInfo kZeldaPrison_MaidenWarpCase1_Oam[] = {
    { 0,  0, 0xce, 0x35, 0},
    {28,  0, 0xce, 0x35, 0},
    {-2,  3, 0x26, 0x75, 0},
    {30,  3, 0x26, 0x35, 0},
    {-2, 11, 0x36, 0x75, 0},
    {30, 11, 0x36, 0x35, 0},
    { 0, 16, 0x26, 0x75, 0},
    {28, 16, 0x26, 0x35, 0},
    { 0, 24, 0x36, 0x75, 0},
    {28, 24, 0x36, 0x35, 0},
    { 2, 16, 0x20, 0x35, 2},
    {18, 16, 0x20, 0x75, 2},
    { 2, 32, 0x20, 0xb5, 2},
    {18, 32, 0x20, 0xf5, 2},
    { 0,  0, 0xce, 0x37, 0},
    {28,  0, 0xce, 0x37, 0},
    {-2,  3, 0x26, 0x77, 0},
    {30,  3, 0x26, 0x37, 0},
    {-2, 11, 0x36, 0x77, 0},
    {30, 11, 0x36, 0x37, 0},
    { 0, 16, 0x26, 0x77, 0},
    {28, 16, 0x26, 0x37, 0},
    { 0, 24, 0x36, 0x77, 0},
    {28, 24, 0x36, 0x37, 0},
    { 2, 16, 0x22, 0x37, 2},
    {18, 16, 0x22, 0x77, 2},
    { 2, 32, 0x22, 0xb7, 2},
    {18, 32, 0x22, 0xf7, 2},
  };
  int k = frame_counter >> 2 & 1;
  static const uint8 kAttract_MaidenWarp_Case1_Num[8] = { 2, 2, 2, 6, 6, 10, 10, 14 };
  attract_x_base = 110;
  attract_y_base = 72;
  Attract_DrawSpriteSet2(kZeldaPrison_MaidenWarpCase1_Oam + k * 14, kAttract_MaidenWarp_Case1_Num[attract_var21 >> 1 & 7]);

  if (!attract_var21 && attract_var20 == 0x70)
    sound_effect_2 = 0x27;

  if (attract_var21 == 15) {
    attract_var5++;
  } else {
    if (attract_var21 == 6) {
      intro_times_pal_flash = 0x90;
      sound_effect_2 = 0x2b;
    }
    if (attract_var20)
      attract_var20--;
    else
      attract_var21++;
  }
}

void Attract_MaidenWarp_Case2() {
  static const uint8 kMaidenWarp_Case2_Num[8] = { 4, 4, 8, 8, 12, 12, 14, 14 };
  static const AttractOamInfo kAttract_MaidenWarpCase2_Oam[] = {
    { 0,  0, 0xce, 0x35, 0},
    {28,  0, 0xce, 0x35, 0},
    {-2,  3, 0x26, 0x75, 0},
    {30,  3, 0x26, 0x35, 0},
    {-2, 11, 0x36, 0x75, 0},
    {30, 11, 0x36, 0x35, 0},
    { 0, 16, 0x26, 0x75, 0},
    {28, 16, 0x26, 0x35, 0},
    { 0, 24, 0x36, 0x75, 0},
    {28, 24, 0x36, 0x35, 0},
    { 2, 16, 0x20, 0x35, 2},
    {18, 16, 0x20, 0x75, 2},
    { 2, 32, 0x20, 0xb5, 2},
    {18, 32, 0x20, 0xf5, 2},
    { 0,  0, 0xce, 0x37, 0},
    {28,  0, 0xce, 0x37, 0},
    {-2,  3, 0x26, 0x77, 0},
    {30,  3, 0x26, 0x37, 0},
    {-2, 11, 0x36, 0x77, 0},
    {30, 11, 0x36, 0x37, 0},
    { 0, 16, 0x26, 0x77, 0},
    {28, 16, 0x26, 0x37, 0},
    { 0, 24, 0x36, 0x77, 0},
    {28, 24, 0x36, 0x37, 0},
    { 2, 16, 0x22, 0x37, 2},
    {18, 16, 0x22, 0x77, 2},
    { 2, 32, 0x22, 0xb7, 2},
    {18, 32, 0x22, 0xf7, 2},
  };
  attract_x_base = 110;
  attract_y_base = 72;
  int k = frame_counter >> 2 & 1;
  int n = kMaidenWarp_Case2_Num[attract_var21 >> 1 & 7];
  Attract_DrawSpriteSet2(kAttract_MaidenWarpCase2_Oam + k * 14 + (14 - n), n);
  if (attract_var21 == 0) {
    if (!--attract_var19)
      attract_var5++;
  } else {
    attract_var21--;
  }
}

void Attract_MaidenWarp_Case3() {
  static const AttractOamInfo kAttract_MaidenWarpCase3_Oam[] = {
    { 0,  0, 0xc6, 0x3d, 2},
    { 0,  0, 0x24, 0x35, 2},
    {16,  0, 0x24, 0x75, 2},
  };
  static const uint8 kMaidenWarp_Case3_Xbase[2] = { 0x78, 0x70 };

  if (attract_var21 == 6) {
    attract_var15++;
    sound_effect_1 = 51;
  } else if (attract_var21 == 0x40) {
    attract_var21 = 224;
    attract_var5++;
  } else if (attract_var21 < 0xf) {
    int k = attract_var21 >> 3 & 1;
    attract_x_base = kMaidenWarp_Case3_Xbase[k];
    attract_y_base = 0x60;
    Attract_DrawSpriteSet2(kAttract_MaidenWarpCase3_Oam + k, k ? 2 : 1);
  }
  attract_var21++;
}

void Attract_MaidenWarp_Case4() {
  Attract_ShowTimedTextMessage();
  if (!oam_priority_value) {
    if (attract_var21 < 31 && !(attract_var21 & 1)) {
      INIDISP_copy--;
    }
    if (!--attract_var21)
      attract_var22++;
  }
}

void Dungeon_LoadAndDrawEntranceRoom(uint8 a) {  // 82c533
  attract_room_index = a;
  Dungeon_LoadEntrance();
  dung_num_lit_torches = 0;
  hdr_dungeon_dark_with_lantern = 0;
  Dungeon_LoadAndDrawRoom();
  Dungeon_ResetTorchBackgroundAndPlayer();
}

void Dungeon_SaveAndLoadLoadAllPalettes(uint8 a, uint8 k) {  // 82c546
  sprite_graphics_index = k;
  main_tile_theme_index = a;
  aux_tile_theme_index = a;
  InitializeTilesets();
  overworld_palette_aux_or_main = 0x200;
  flag_update_cgram_in_nmi++;
  Palette_BgAndFixedColor_Black();
  Palette_Load_SpritePal0Left();
  Palette_Load_SpriteMain();
  Palette_Load_SpriteAux1();
  Palette_Load_SpriteAux2();
  Palette_Load_SpriteEnvironment_Dungeon();
  Palette_Load_HUD();
  Palette_Load_DungeonSet();
}

void Module14_Attract() {  // 8cedad
  uint8 st = attract_state;
  if (INIDISP_copy && INIDISP_copy != 128 && st && st != 2 && st != 6 && filtered_joypad_H & 0x90)
    attract_state = st = 9;

  switch (st) {
  case 0: Attract_Fade(); break;
  case 1: Attract_InitGraphics(); break;
  case 2: Attract_FadeOutSequence(); break;
  case 3: Attract_LoadNewScene(); break;
  case 4: Attract_FadeInSequence(); break;
  case 5: Attract_EnactStory(); break;
  case 6: Attract_FadeOutSequence(); break;
  case 7: Attract_LoadNewScene(); break;
  case 8: Attract_EnactStory(); break;
  case 9: Attract_SkipToFileSelect(); break;
  }
}

void Attract_Fade() {  // 8cede6
  Intro_HandleAllTriforceAnimations();
  intro_did_run_step = 0;
  is_nmi_thread_active = 0;
  Intro_PeriodicSwordAndIntroFlash();
  if (INIDISP_copy) {
    INIDISP_copy--;
    return;
  }
  EnableForceBlank();
  irq_flag = 255;
  is_nmi_thread_active = 0;
  nmi_flag_update_polyhedral = 0;
  attract_state++;
}

void Attract_InitGraphics() {  // 8cee0c
  memset(&attract_var12, 0, 0x51);
  EraseTileMaps_normal();
  Attract_LoadBG3GFX();
  overworld_palette_mode = 4;
  hud_palette = 1;
  overworld_palette_aux_or_main = 0;
  Palette_Load_HUD();
  overworld_palette_aux_or_main = 0x200;
  Palette_Load_OWBGMain();
  Palette_Load_HUD();
  Palette_Load_LinkArmorAndGloves();
  main_palette_buffer[0x1d] = 0x3800;
  flag_update_cgram_in_nmi++;
  BYTE(BG3VOFS_copy2) = 20;
  Attract_BuildBackgrounds();
  messaging_module = 0;
  dialogue_message_index = 0x112;
  BG2VOFS_copy2 = 0;
  attract_legend_ctr = 0x1010;
  attract_state += 3;
  HdmaSetup(0xCFA87, 0xCFA94, 1, (uint8)WH0, (uint8)WH2, 0);
  HDMAEN_copy = 0xc0;

  W12SEL_copy = 0;
  W34SEL_copy = 0;
  WOBJSEL_copy = 0xb0;
  TMW_copy = 3;
  TSW_copy = 0;
  COLDATA_copy0 = 0x25;
  COLDATA_copy1 = 0x45;
  COLDATA_copy2 = 0x85;
  CGWSEL_copy = 0x10;
  CGADSUB_copy = 0xa3;
  zelda_ppu_write(WBGLOG, 0);
  zelda_ppu_write(WOBJLOG, 0);

  music_control = 6;
  attract_legend_flag++;
}

void Attract_FadeInStep() {  // 8ceea6
  if (INIDISP_copy != 15) {
    if (sign8(--link_speed_setting)) {
      INIDISP_copy++;
      link_speed_setting = 1;
    }
  } else {
    attract_var18++;
  }
}

void Attract_FadeInSequence() {  // 8ceeba
  if (INIDISP_copy != 15) {
    if (sign8(--link_speed_setting)) {
      INIDISP_copy++;
      link_speed_setting = 1;
    }
  } else {
    attract_state++;
  }
}

void Attract_FadeOutSequence() {  // 8ceecb
  if (INIDISP_copy != 0) {
    if (sign8(--link_speed_setting)) {
      INIDISP_copy--;
      link_speed_setting = 1;
    }
  } else {
    EnableForceBlank();
    EraseTileMaps_normal();
    attract_state++;
  }
}

void Attract_LoadNewScene() {  // 8ceee5
  switch (attract_sequence) {
  case 0: AttractScene_PolkaDots(); break;
  case 1: AttractScene_WorldMap(); break;
  case 2: AttractScene_ThroneRoom(); break;
  case 3: Attract_PrepZeldaPrison(); break;
  case 4: Attract_PrepMaidenWarp(); break;
  case 5: AttractScene_EndOfStory(); break;
  }
}

void AttractScene_PolkaDots() {  // 8ceef8
  attract_next_legend_gfx = 0;
  attract_state++;
  INIDISP_copy = 0;
}

void AttractScene_WorldMap() {  // 8ceeff
  zelda_ppu_write(BG1SC, 0x13);
  zelda_ppu_write(BG2SC, 0x3);
  CGWSEL_copy = 0x80;
  CGADSUB_copy = 0x21;
  zelda_ppu_write(BGMODE, 7);
  BGMODE_copy = 7;
  zelda_ppu_write(M7SEL, 0x80);
  WorldMap_LoadLightWorldMap();
  M7Y_copy = 0xed;
  M7X_copy = 0x100;
  BG1HOFS_copy = 0x80;
  BG1VOFS_copy = 0xc0;
  timer_for_mode7_zoom = 255;
  Attract_ControlMapZoom();
  attract_var10 = 1;
  attract_state++;
  INIDISP_copy = 0;
}

void AttractScene_ThroneRoom() {  // 8cef4e
  zelda_snes_dummy_write(HDMAEN, 0);
  HDMAEN_copy = 0;
  CGWSEL_copy = 2;
  CGADSUB_copy = 0x20;
  misc_sprites_graphics_index = 10;
  LoadCommonSprites_2();
  uint16 bak0 = attract_var12;
  uint16 bak1 = WORD(attract_state);
  Dungeon_LoadAndDrawEntranceRoom(0x74);
  WORD(attract_state) = bak1;
  attract_var12 = bak0;
  dung_hdr_palette_1 = 0;
  overworld_palette_sp0 = 0;
  sprite_aux1_palette = 14;
  sprite_aux2_palette = 3;
  Dungeon_SaveAndLoadLoadAllPalettes(0, 0x7e);

  main_palette_buffer[0x1d] = 0x3800;
  messaging_module = 0;
  dialogue_message_index = 0x113;
  attract_var10 = 2;
  attract_var13 = 0xe0;
  oam_priority_value = 0x210;

  Attract_PrepFinish();
}

void Attract_PrepFinish() {  // 8cefc0
  attract_state++;
  INIDISP_copy = 0;
  BYTE(BG3VOFS_copy2) = 0;
  BG2HOFS_copy &= 0x1ff;
  BG2VOFS_copy &= 0x1ff;
  BG2HOFS_copy2 &= 0x1ff;
  BG2VOFS_copy2 &= 0x1ff;
}

void Attract_PrepZeldaPrison() {  // 8cefe3
  CGWSEL_copy = 0;
  CGADSUB_copy = 0;

  uint16 bak0 = attract_var12;
  uint16 bak1 = WORD(attract_state);
  Dungeon_LoadAndDrawEntranceRoom(0x73);
  WORD(attract_state) = bak1;
  attract_var12 = bak0;

  dung_hdr_palette_1 = 2;
  overworld_palette_sp0 = 0;
  sprite_aux1_palette = 14;
  sprite_aux2_palette = 3;
  Dungeon_SaveAndLoadLoadAllPalettes(1, 0x7f);
  main_palette_buffer[0x1d] = 0x3800;

  messaging_module = 0;
  dialogue_message_index = 0x114;

  attract_var9 = 148;
  attract_vram_dst = 0x68;
  attract_var1 = 0;
  attract_var3 = 0;
  attract_x_base_hi = 0;
  attract_var17 = 0;
  attract_var18 = 0;
  attract_var10 = 255;
  oam_priority_value = 0x240;
  Attract_PrepFinish();
}

void Attract_PrepMaidenWarp() {  // 8cf058
  uint16 bak0 = attract_var12;
  uint16 bak1 = WORD(attract_state);
  Dungeon_LoadAndDrawEntranceRoom(0x75);
  WORD(attract_state) = bak1;
  attract_var12 = bak0;

  dung_hdr_palette_1 = 0;
  overworld_palette_sp0 = 0;
  sprite_aux1_palette = 14;
  sprite_aux2_palette = 3;

  overworld_palette_aux_or_main = 0;
  Palette_Load_SpritePal0Left();
  Palette_Load_SpriteMain();
  Palette_Load_SpriteAux1();
  Palette_Load_SpriteAux2();
  Palette_Load_SpriteEnvironment_Dungeon();
  Palette_Load_HUD();
  Palette_Load_DungeonSet();
  Dungeon_SaveAndLoadLoadAllPalettes(2, 0x7f);
  aux_palette_buffer[0x1d] = main_palette_buffer[0x1d] = 0x3800;

  messaging_module = 0;
  dialogue_message_index = 0x115;
  attract_var10 = 255;
  BYTE(attract_vram_dst) = 112;
  attract_var19 = 112;
  attract_var20 = 112;
  attract_var1 = 8;
  attract_var17 = 0;
  attract_var21 = 0;
  attract_var15 = 0;
  attract_var18 = 0;
  attract_var5 = 0;
  attract_var11 = 0;

  oam_priority_value = 0xc0;
  Attract_PrepFinish();
}

void AttractScene_EndOfStory() {  // 8cf0dc
  Attract_SetUpConclusionHDMA();
  Death_Func31();
}

void Death_Func31() {  // 8cf0e2
  nmi_disable_core_updates++;
  Intro_InitializeMemory_darken();
  Overworld_LoadAllPalettes();
  BYTE(BG3VOFS_copy2) = 0;
  M7Y_copy = 0;
  M7X_copy = 0;
  BG1HOFS_copy = 0;
  BG1VOFS_copy = 0;
  BG2HOFS_copy = 0;
  BG2VOFS_copy = 0;
  music_control = 0xF1;
  attract_sequence = 0;
  main_module_index = 0;
  submodule_index = 10;
  subsubmodule_index = 10;
}

void Attract_EnactStory() {  // 8cf115
  switch (attract_sequence) {
  case 0: AttractDramatize_PolkaDots(); break;
  case 1: AttractDramatize_WorldMap(); break;
  case 2: Attract_ThroneRoom(); break;
  case 3: AttractDramatize_Prison(); break;
  case 4: AttractDramatize_AgahnimAltar(); break;
  }
}

void AttractDramatize_PolkaDots() {  // 8cf126
  if (!(frame_counter & 3)) {
    BYTE(BG1VOFS_copy)++;
    BYTE(BG1HOFS_copy)++;
    BYTE(BG2VOFS_copy)++;
    BYTE(BG2HOFS_copy)--;
  }

  if (attract_legend_flag) {
    Attract_BuildNextImageTileMap();
    attract_legend_flag = 0;
    attract_next_legend_gfx += 2;
  }
  joypad1L_last = 0;
  filtered_joypad_L = 0;
  filtered_joypad_H = 0;
  RenderText();
  if (!--attract_legend_ctr) {
    attract_sequence++;
    attract_state -= 3;
  } else {
    if (attract_legend_ctr < 0x18 && attract_legend_ctr & 1)
      INIDISP_copy--;
  }
}

void AttractDramatize_WorldMap() {  // 8cf176
  if (timer_for_mode7_zoom != 0) {
    if (timer_for_mode7_zoom < 15)
      INIDISP_copy--;
    if (!--attract_var10) {
      attract_var10 = 1;
      timer_for_mode7_zoom -= 1;
      Attract_ControlMapZoom();
    }
  } else {
    EnableForceBlank();
    zelda_ppu_write(BGMODE, 9);
    BGMODE_copy = 9;
    EraseTileMaps_normal();
    attract_sequence++;
    attract_state -= 2;
  }
}

void Attract_ThroneRoom() {  // 8cf1c8
  static const AttractOamInfo kThroneRoom_Oams[] = {
    {16, 16, 0x2a, 0x7b, 2},
    { 0, 16, 0x2a, 0x3b, 2},
    {16,  0, 0x0a, 0x7b, 2},
    { 0,  0, 0x0a, 0x3b, 2},
    { 0,  0, 0x0c, 0x31, 2},
    {16,  0, 0x0e, 0x31, 2},
    {32,  0, 0x0c, 0x71, 2},
    { 0, 16, 0x2c, 0x31, 2},
    {16, 16, 0x2e, 0x31, 2},
    {32, 16, 0x2c, 0x71, 2},
  };
  static const uint8 kThroneRoom_OamOffs[3] = { 0, 4, 10 };
  static const int8 kAttract_ThroneRoom_Xbase[2] = { 80, 104 };
  static const int8 kAttract_ThroneRoom_Ybase[2] = { 88, 32 };
  attract_oam_idx = 0;
  if (!attract_var15) {
    if (INIDISP_copy != 15)
      INIDISP_copy++;
    else
      attract_var15++;
  }
  if (!BG2VOFS_copy) {
    Attract_ShowTimedTextMessage();
    if (!oam_priority_value) {
      if (attract_var13 < 31 && !(attract_var13 & 1))
        INIDISP_copy--;
      if (!--attract_var13) {
        attract_sequence++;
        attract_state++;
        return;
      }
    }
  } else {
    BG2VOFS_copy--;
    BG1VOFS_copy--;
  }
  for (int i = 1; i >= 0; i--) {
    const AttractOamInfo *oamp = &kThroneRoom_Oams[kThroneRoom_OamOffs[i]];
    int n = kThroneRoom_OamOffs[i + 1] - kThroneRoom_OamOffs[i];
    uint16 y = kAttract_ThroneRoom_Ybase[i] - BG2VOFS_copy;
    if (!sign16(y + 32)) {
      attract_x_base = kAttract_ThroneRoom_Xbase[i];
      attract_y_base = y;
      Attract_DrawSpriteSet2(oamp, n);
    }
  }

  attract_var7 = 0xf8a7;
}

void AttractDramatize_Prison() {  // 8cf27a
  static const uint8 kAttract_ZeldaPrison_Tab0[16] = { 0, 1, 2, 3, 4, 5, 5, 5, 4, 4, 3, 3, 2, 2, 1, 1 };
  static const int8 kZeldaPrison_Soldier_X[2] = { 32, -12 };
  static const int8 kZeldaPrison_Soldier_Y[2] = { 24, 24 };
  static const uint8 kZeldaPrison_Soldier_Dir[2] = { 1, 1 };
  static const uint8 kZeldaPrison_Soldier_Flags[2] = { 9, 7 };

  attract_oam_idx = 0;
  if (!attract_var18)
    Attract_FadeInStep();
  attract_x_base = 56;
  Attract_DrawZelda();
  if (attract_var10 >= 192) {
    attract_y_base = 112;
    if (sign8(--attract_var17))
      attract_var17 = 0xf;
    int t = attract_vram_dst + kAttract_ZeldaPrison_Tab0[attract_var17];
    attract_x_base_hi = t >> 8;
    attract_x_base = t;
    Attract_ZeldaPrison_DrawA();

    for (int k = 1; k >= 0; k--) {
      SpritePrep_ResetProperties(k * 2);
      uint16 x = kZeldaPrison_Soldier_X[k] + attract_vram_dst + 0x100;
      attract_var4 = x;
      Sprite_SimulateSoldier(k * 2,
                             x, attract_y_base + kZeldaPrison_Soldier_Y[k],
                             kZeldaPrison_Soldier_Dir[k], kZeldaPrison_Soldier_Flags[k], attract_var3);
    }

    if (!(++attract_var1 & 7)) {
      if (attract_var3 == 2) {
        attract_var3 = 0xff;
        if (!HIBYTE(attract_vram_dst) && attract_var1 & 8)
          sound_effect_2 = 4;
      }
      attract_var3++;
    }
  }

  switch (attract_var5) {
  case 0: Attract_ZeldaPrison_Case0(); break;
  case 1: Attract_ZeldaPrison_Case1(); break;
  }
}

void AttractDramatize_AgahnimAltar() {  // 8cf423
  if (attract_var22) {
    attract_sequence++;
    attract_state -= 2;
    return;
  }
  attract_oam_idx = 0;
  HandleScreenFlash();
  if (!attract_var18)
    Attract_FadeInStep();
  if (attract_var17 != 255)
    attract_var17++;
  if (intro_times_pal_flash & 4)
    sound_effect_2 = 0x2b;
  switch (attract_var5) {
  case 0: Attract_MaidenWarp_Case0(); break;
  case 1: Attract_MaidenWarp_Case1(); break;
  case 2: Attract_MaidenWarp_Case2(); break;
  case 3: Attract_MaidenWarp_Case3(); break;
  case 4: Attract_MaidenWarp_Case4(); break;
  }

  static const uint8 kMaidenWarp_Soldier_X[6] = { 48, 192, 48, 192, 80, 160 };
  static const uint8 kMaidenWarp_Soldier_Y[6] = { 112, 112, 152, 152, 192, 192 };
  static const uint8 kMaidenWarp_Soldier_Dir[6] = { 0, 1, 0, 1, 3, 3 };
  static const uint8 kMaidenWarp_Soldier_Flags[6] = { 9, 9, 9, 9, 7, 9 };
  for (int k = 5; k >= 0; k--) {

    SpritePrep_ResetProperties(k);
    Sprite_SimulateSoldier(k, kMaidenWarp_Soldier_X[k], kMaidenWarp_Soldier_Y[k],
                           kMaidenWarp_Soldier_Dir[k], kMaidenWarp_Soldier_Flags[k], 0);
  }

  if (attract_var17 >= 0xa0) {
    if (BYTE(attract_vram_dst) != 0x60) {
      if (!--attract_var1) {
        BYTE(attract_vram_dst)--;
        attract_var1 = 8;
      }
    } else {
      attract_var11++;
    }
  }

  if (attract_var15 == 0) {
    static const AttractOamInfo kZeldaPrison_MaidenWarp0[] = {
      { 0,  0, 0x03, 0x3d, 2},
      { 8,  0, 0x04, 0x3d, 2},
      { 0,  0, 0x00, 0x3d, 2},
      { 8,  0, 0x01, 0x3d, 2},
    };

    attract_x_base = 116;
    attract_y_base = attract_vram_dst;
    Attract_DrawSpriteSet2(kZeldaPrison_MaidenWarp0 + (BYTE(attract_vram_dst) == 0x70 ? 0 : 2), 2);
    static const uint8 kAttract_MaidenWarp_Xbase[8] = { 4, 4, 3, 3, 2, 2, 1, 0 };
    static const AttractOamInfo kZeldaPrison_MaidenWarp1[] = {
      { 0,  0, 0x6c, 0x38, 2},
      { 0,  0, 0x6c, 0x38, 2},
      { 0,  0, 0x6c, 0x38, 2},
      { 0,  0, 0x6c, 0x38, 2},
      { 0,  0, 0x6c, 0x38, 2},
      { 2,  0, 0x6c, 0x38, 2},
      { 0,  0, 0x6c, 0x38, 2},
      { 2,  0, 0x6c, 0x38, 2},
      { 0,  0, 0x6c, 0x38, 2},
      { 4,  0, 0x6c, 0x38, 2},
      { 0,  0, 0x6c, 0x38, 2},
      { 4,  0, 0x6c, 0x38, 2},
      { 0,  0, 0x6c, 0x38, 2},
      { 6,  0, 0x6c, 0x38, 2},
      { 0,  0, 0x6c, 0x38, 2},
      { 8,  0, 0x6c, 0x38, 2},
    };
    int k = 7;
    if (BYTE(attract_vram_dst) < 0x68)
      k = (BYTE(attract_vram_dst) - 0x68) & 7;
    attract_x_base = 0x74 + kAttract_MaidenWarp_Xbase[k];
    attract_y_base = 0x76;
    Attract_DrawSpriteSet2(kZeldaPrison_MaidenWarp1 + k * 2, 2);

  }
  static const AttractOamInfo kZeldaPrison_MaidenWarp2[] = {
    { 5, 25, 0x6c, 0x38, 2},
    {11, 25, 0x6c, 0x38, 2},
    { 0,  0, 0x82, 0x3b, 2},
    {16,  0, 0x82, 0x7b, 2},
    { 0, 16, 0xa2, 0x3b, 2},
    {16, 16, 0xa2, 0x7b, 2},
    { 5, 25, 0x6c, 0x38, 2},
    {11, 25, 0x6c, 0x38, 2},
    { 0,  0, 0x80, 0x3b, 2},
    {16,  0, 0x82, 0x7b, 2},
    { 0, 16, 0xa0, 0x3b, 2},
    {16, 16, 0xa2, 0x7b, 2},
    { 5, 25, 0x6c, 0x38, 2},
    {11, 25, 0x6c, 0x38, 2},
    { 0,  0, 0x82, 0x3b, 2},
    {16,  0, 0x82, 0x7b, 2},
    { 0, 16, 0xa2, 0x3b, 2},
    {16, 16, 0xa2, 0x7b, 2},
    { 5, 25, 0x6c, 0x38, 2},
    {11, 25, 0x6c, 0x38, 2},
    { 0,  0, 0x82, 0x3b, 2},
    {16,  0, 0x80, 0x7b, 2},
    { 0, 16, 0xa2, 0x3b, 2},
    {16, 16, 0xa0, 0x7b, 2},
    { 5, 25, 0x6c, 0x38, 2},
    {11, 25, 0x6c, 0x38, 2},
    { 0,  0, 0x82, 0x3b, 2},
    {16,  0, 0x82, 0x7b, 2},
    { 0, 16, 0xa2, 0x3b, 2},
    {16, 16, 0xa2, 0x7b, 2},
    { 5, 25, 0x6c, 0x38, 2},
    {11, 25, 0x6c, 0x38, 2},
    { 0,  0, 0x80, 0x3b, 2},
    {16,  0, 0x82, 0x7b, 2},
    { 0, 16, 0xa0, 0x3b, 2},
    {16, 16, 0xa2, 0x7b, 2},
    { 5, 25, 0x6c, 0x38, 2},
    {11, 25, 0x6c, 0x38, 2},
    { 0,  0, 0x82, 0x3b, 2},
    {16,  0, 0x82, 0x7b, 2},
    { 0, 16, 0xa2, 0x3b, 2},
    {16, 16, 0xa2, 0x7b, 2},
    { 5, 25, 0x6c, 0x38, 2},
    {11, 25, 0x6c, 0x38, 2},
    { 0,  0, 0x80, 0x3b, 2},
    {16,  0, 0x80, 0x7b, 2},
    { 0, 16, 0xa0, 0x3b, 2},
    {16, 16, 0xa0, 0x7b, 2},
  };
  int k = attract_var17 >> 5 & 7;
  attract_x_base = 112;
  attract_y_base = 70;
  Attract_DrawSpriteSet2(kZeldaPrison_MaidenWarp2 + k * 6, 6);

}

void Attract_SkipToFileSelect() {  // 8cf700
  if (--INIDISP_copy)
    return;
  EnableForceBlank();
  zelda_ppu_write(BG1SC, 0x13);
  zelda_ppu_write(BG2SC, 0x3);
  Attract_SetUpConclusionHDMA();
  M7Y_copy = 0;
  M7X_copy = 0;
  BG1HOFS_copy = 0;
  BG1VOFS_copy = 0;
  BG3VOFS_copy2 = 0;
  FadeMusicAndResetSRAMMirror();
}

void Attract_BuildNextImageTileMap() {  // 8cf73e
  static const uint8 *const kAttract_LegendGraphics_pointers[4] = {
    kAttract_Legendgraphics_0,
    kAttract_Legendgraphics_1,
    kAttract_Legendgraphics_2,
    kAttract_Legendgraphics_3,
  };
  static const uint16 kAttract_LegendGraphics_sizes[4] = { 157+1, 237+1, 199+1, 265+1 };
  int i = attract_next_legend_gfx >> 1;
  memcpy(&g_ram[0x1002], kAttract_LegendGraphics_pointers[i], kAttract_LegendGraphics_sizes[i]);
  nmi_load_bg_from_vram = 1;
}

void Attract_ShowTimedTextMessage() {  // 8cf766
  attract_var12 = BG2VOFS_copy2;
  BYTE(joypad1L_last) = 0;
  BYTE(filtered_joypad_L) = 0;
  BYTE(filtered_joypad_H) = 0;
  RenderText();
  if (oam_priority_value)
    oam_priority_value--;
}

void Attract_ControlMapZoom() {  // 8cf783
  for (int i = 223; i >= 0; i--)
    mode7_hdma_table[i] = kMapMode_Zooms1[i] * timer_for_mode7_zoom >> 8;
}

void Attract_BuildBackgrounds() {  // 8cf7e6
  static const uint16 kAttract_CopyToVram_Tab0[16] = { 0x1a0, 0x9a6, 0x89a5, 0x1a0, 0x9a5, 0x1a0, 0x1a0, 0x89a6, 0x49a5, 0x1a0, 0x1a0, 0x49a5, 0x1a0, 0x89a5, 0xc9a5, 0x1a0 };
  static const uint16 kAttract_CopyToVram_Tab1[4] = { 0x9a1, 0x9a2, 0x9a3, 0x9a4 };

  BGMODE_copy = 9;
  TM_copy = 0x17;
  TS_copy = 0;

  zelda_ppu_write(BG1SC, 0x10);
  zelda_ppu_write(BG2SC, 0x0);

  {
    int k = 0;
    const uint16 *p = kAttract_CopyToVram_Tab0;
    uint16 *dst = (uint16 *)&g_ram[0x1006];
    do {
      int j = k & 3;
      do {
        dst[k++] = p[j++];
      } while (j & 3);
    } while (k & 0x1f || (p += 4, k != 0x80));
    Attract_TriggerBGDMA(0x1000);
  }

  {
    int k = 0;
    uint16 *dst = (uint16 *)&g_ram[0x1006];
    do {
      int j = k & 1;
      const uint16 *p = kAttract_CopyToVram_Tab1 + ((k & 0x20) >> 4);
      do {
        dst[k++] = p[j++];
      } while (j & 1);
    } while (k != 0x80);
    Attract_TriggerBGDMA(0);
  }
  attract_vram_dst = 0;
}

void Attract_TriggerBGDMA(uint16 dstv) {  // 8cf879
  uint16 *dst = &g_zenv.vram[dstv];
  for (int i = 0; i < 8; i++) {
    memcpy(dst, &g_ram[0x1006], 0x100);
    dst += 0x80;
  }
}

void Attract_DrawPreloadedSprite(const uint8 *xp, const uint8 *yp, const uint8 *cp, const uint8 *fp, const uint8 *ep, int n) {  // 8cf9b5
  OamEnt *oam = &oam_buf[attract_oam_idx + 64];
  attract_oam_idx += n + 1;
  do {
    oam->x = attract_x_base + xp[n];
    oam->y = attract_y_base + yp[n];
    oam->charnum = cp[n];
    oam->flags = fp[n];
    bytewise_extended_oam[oam - oam_buf] = ep[n];
  } while (oam++, --n >= 0);
}

void Attract_DrawZelda() {  // 8cf9e8
  OamEnt *oam = &oam_buf[64 + attract_oam_idx];
  bytewise_extended_oam[oam - oam_buf] = 2;
  oam[0].x = oam[1].x = 0x60;
  oam[0].y = attract_x_base;
  oam[1].y = attract_x_base + 10;
  oam[0].charnum = 0x28;
  oam[1].charnum = 0x2a;
  oam[0].flags = 0x29;
  oam[1].flags = 0x29;
  attract_oam_idx += 2;
}

void Sprite_SimulateSoldier(int k, uint16 x, uint16 y, uint8 dir, uint8 flags, uint8 gfx) {  // 9deb84
  static const uint8 kSimulateSoldier_Gfx[4] = { 11, 4, 0, 7 };
  Sprite_SetX(k, x);
  Sprite_SetY(k, y);
  sprite_z[k] = 0;
  Sprite_Get16BitCoords(k);
  sprite_D[k] = sprite_head_dir[k] = dir;
  sprite_graphics[k] = kSimulateSoldier_Gfx[dir] + gfx;
  sprite_flags3[k] = 16;
  sprite_obj_prio[k] = 0;
  sprite_oam_flags[k] = flags | 0x30;
  sprite_type[k] = (flags == 9) ? 0x41 : 0x43;
  sprite_flags2[k] = 7;
  int oam_idx = k * 8;
  oam_cur_ptr = 0x800 + oam_idx * 4;
  oam_ext_cur_ptr = 0xa20 + oam_idx;
  Guard_HandleAllAnimation(k);
}

