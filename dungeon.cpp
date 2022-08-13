#include "zelda_rtl.h"

#include "variables.h"
#include "dungeon.h"
#include "snes_regs.h"
#include "nmi.h"
#include "hud.h"
#include "load_gfx.h"
#include "overworld.h"
#include "sprite.h"
#include "ancilla.h"
#include "ending.h"
#include "player.h"
#include "misc.h"
#include "player_oam.h"
#include "tables/generated_dungeon_rooms.h"

// todo: move to config
static const uint16 kBossRooms[] = { 
  200, 51, 7,
  32, 
  6, 90, 41, 144, 222, 164, 172,
  13
};

static const uint8 kDungeonExit_From[12] = {200, 51, 7, 32, 6, 90, 41, 144, 222, 164, 172, 13};
static const uint8 kDungeonExit_To[12] = {201, 99, 119, 32, 40, 74, 89, 152, 14, 214, 219, 13};


void Module_PreDungeon_setAmbientSfx();
void HoleToDungeon_Helper1();
void Dungeon_Staircase_MusicFunc1();
void Dung_FillFloor(const uint16 *src);
bool Dung_CheckStarTileSwitch(uint8 *y_out);


static const uint16 kObjectSubtype1Params[] = {
  0x3d8, 0x2e8, 0x2f8, 0x328, 0x338, 0x400, 0x410, 0x388, 0x390, 0x420, 0x42a, 0x434, 0x43e, 0x448, 0x452, 0x45c,
  0x466, 0x470, 0x47a, 0x484, 0x48e, 0x498, 0x4a2, 0x4ac, 0x4b6, 0x4c0, 0x4ca, 0x4d4, 0x4de, 0x4e8, 0x4f2, 0x4fc,
  0x506, 0x598, 0x600, 0x63c, 0x63c, 0x63c, 0x63c, 0x63c, 0x642, 0x64c, 0x652, 0x658, 0x65e, 0x664, 0x66a, 0x688,
  0x694, 0x6a8, 0x6a8, 0x6a8, 0x6c8, 0x0, 0x78a, 0x7aa, 0xe26, 0x84a, 0x86a, 0x882, 0x8ca, 0x85a, 0x8fa, 0x91a,
  0x920, 0x92a, 0x930, 0x936, 0x93c, 0x942, 0x948, 0x94e, 0x96c, 0x97e, 0x98e, 0x902, 0x99e, 0x9d8, 0x9d8, 0x9d8,
  0x9fa, 0x156c, 0x1590, 0x1d86, 0x0, 0xa14, 0xa24, 0xa54, 0xa54, 0xa84, 0xa84, 0x14dc, 0x1500, 0x61e, 0xe52, 0x600,
  0x3d8, 0x2c8, 0x2d8, 0x308, 0x318, 0x3e0, 0x3f0, 0x378, 0x380, 0x5fa, 0x648, 0x64a, 0x670, 0x67c, 0x6a8, 0x6a8,
  0x6a8, 0x6c8, 0x0, 0x7aa, 0x7ca, 0x84a, 0x89a, 0x8b2, 0x90a, 0x926, 0x928, 0x912, 0x9f8, 0x1d7e, 0x0, 0xa34,
  0xa44, 0xa54, 0xa6c, 0xa84, 0xa9c, 0x1524, 0x1548, 0x85a, 0x606, 0xe52, 0x5fa, 0x6a0, 0x6a2, 0xb12, 0xb14, 0x9b0,
  0xb46, 0xb56, 0x1f52, 0x1f5a, 0x288, 0xe82, 0x1df2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x3d8, 0x3d8, 0x3d8, 0x3d8, 0x5aa, 0x5b2, 0x5b2, 0x5b2, 0x5b2, 0xe0, 0xe0, 0xe0, 0xe0, 0x110, 0x0, 0x0,
  0x6a4, 0x6a6, 0xae6, 0xb06, 0xb0c, 0xb16, 0xb26, 0xb36, 0x1f52, 0x1f5a, 0x288, 0xeba, 0xe82, 0x1df2, 0x0, 0x0,
  0x3d8, 0x510, 0x5aa, 0x5aa, 0x0, 0x168, 0xe0, 0x158, 0x100, 0x110, 0x178, 0x72a, 0x72a, 0x72a, 0x75a, 0x670,
  0x670, 0x130, 0x148, 0x72a, 0x72a, 0x72a, 0x75a, 0xe0, 0x110, 0xf0, 0x110, 0x0, 0xab4, 0x8da, 0xade, 0x188,
  0x1a0, 0x1b0, 0x1c0, 0x1d0, 0x1e0, 0x1f0, 0x200, 0x120, 0x2a8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
};
static const uint16 kObjectSubtype2Params[] = {
  0xb66, 0xb86, 0xba6, 0xbc6, 0xc66, 0xc86, 0xca6, 0xcc6, 0xbe6, 0xc06, 0xc26, 0xc46, 0xce6, 0xd06, 0xd26, 0xd46,
  0xd66, 0xd7e, 0xd96, 0xdae, 0xdc6, 0xdde, 0xdf6, 0xe0e, 0x398, 0x3a0, 0x3a8, 0x3b0, 0xe32, 0xe26, 0xea2, 0xe9a,
  0xeca, 0xed2, 0xede, 0xede, 0xf1e, 0xf3e, 0xf5e, 0xf6a, 0xef6, 0xf72, 0xf92, 0xfa2, 0xfa2, 0x1088, 0x10a8, 0x10a8,
  0x10c8, 0x10c8, 0x10c8, 0x10c8, 0xe52, 0x1108, 0x1108, 0x12a8, 0x1148, 0x1160, 0x1178, 0x1190, 0x1458, 0x1488, 0x2062, 0x2086,
};
static const uint16 kObjectSubtype3Params[] = {
  0x1614, 0x162c, 0x1654, 0xa0e, 0xa0c, 0x9fc, 0x9fe, 0xa00, 0xa02, 0xa04, 0xa06, 0xa08, 0xa0a, 0x0, 0xa10, 0xa12,
  0x1dda, 0x1de2, 0x1dd6, 0x1dea, 0x15fc, 0x1dfa, 0x1df2, 0x1488, 0x1494, 0x149c, 0x14a4, 0x10e8, 0x10e8, 0x10e8, 0x11a8, 0x11c8,
  0x11e8, 0x1208, 0x3b8, 0x3c0, 0x3c8, 0x3d0, 0x1228, 0x1248, 0x1268, 0x1288, 0x0, 0xe5a, 0xe62, 0x0, 0x0, 0xe82,
  0xe8a, 0x14ac, 0x14c4, 0x10e8, 0x1614, 0x1614, 0x1614, 0x1614, 0x1614, 0x1614, 0x1cbe, 0x1cee, 0x1d1e, 0x1d4e, 0x1d8e, 0x1d96,
  0x1d9e, 0x1da6, 0x1dae, 0x1db6, 0x1dbe, 0x1dc6, 0x1dce, 0x220, 0x260, 0x280, 0x1f3a, 0x1f62, 0x1f92, 0x1ff2, 0x2016, 0x1f42,
  0xeaa, 0x1f4a, 0x1f52, 0x1f5a, 0x202e, 0x2062, 0x9b8, 0x9c0, 0x9c8, 0x9d0, 0xfa2, 0xfb2, 0xfc4, 0xff4, 0x1018, 0x1020,
  0x15b4, 0x15d8, 0x20f6, 0xeba, 0x22e6, 0x22ee, 0x5da, 0x281e, 0x2ae0, 0x2d2a, 0x2f2a, 0x22f6, 0x2316, 0x232e, 0x2346, 0x235e,
  0x2376, 0x23b6, 0x1e9a, 0x0, 0x2436, 0x149c, 0x24b6, 0x24e6, 0x2516, 0x1028, 0x1040, 0x1060, 0x1070, 0x1078, 0x1080, 0x0,
};

static const uint16 kDoorTypeSrcData[] = {
  0x2716, 0x272e, 0x272e, 0x2746, 0x2746, 0x2746, 0x2746, 0x2746, 0x2746, 0x275e, 0x275e, 0x275e, 0x275e, 0x2776, 0x278e, 0x27a6,
  0x27be, 0x27be, 0x27d6, 0x27d6, 0x27ee, 0x2806, 0x2806, 0x281e, 0x2836, 0x2836, 0x2836, 0x2836, 0x284e, 0x2866, 0x2866, 0x2866,
  0x2866, 0x287e, 0x2896, 0x28ae, 0x28c6, 0x28de, 0x28f6, 0x28f6, 0x28f6, 0x290e, 0x2926, 0x2958, 0x2978, 0x2990, 0x2990, 0x2990,
  0x2990, 0x29a8, 0x29c0, 0x29d8,
};
static const uint16 kDoorTypeSrcData2[] = {
  0x29f0, 0x2a08, 0x2a08, 0x2a20, 0x2a20, 0x2a20, 0x2a20, 0x2a20, 0x2a20, 0x2a38, 0x2a38, 0x2a38, 0x2a38, 0x2a50, 0x2a68, 0x2a80,
  0x2a98, 0x2a98, 0x2a98, 0x2a98, 0x2a98, 0x2ab0, 0x2ac8, 0x2ae0, 0x2af8, 0x2af8, 0x2af8, 0x2af8, 0x2b10, 0x2b28, 0x2b28, 0x2b28,
  0x2b28, 0x2b40, 0x2b58, 0x2b70, 0x2b88, 0x2ba0, 0x2bb8, 0x2bb8, 0x2bb8, 0x2bd0, 0x2be8, 0x2c1a, 0x2c3a, 0x2c52, 0x2c6a, 0x2c6a,
};
static const uint16 kDoorTypeSrcData3[] = {
  0x2c6a, 0x2c82, 0x2c82, 0x2c9a, 0x2c9a, 0x2c9a, 0x2c9a, 0x2c9a, 0x2c9a, 0x2cb2, 0x2cb2, 0x2cb2, 0x2cb2, 0x2cca, 0x2ce2, 0x2cfa,
  0x2cfa, 0x2cfa, 0x2cfa, 0x2cfa, 0x2cfa, 0x2d12, 0x2d12, 0x2d2a, 0x2d42, 0x2d42, 0x2d42, 0x2d42, 0x2d5a, 0x2d72, 0x2d72, 0x2d72,
  0x2d72, 0x2d8a, 0x2da2, 0x2dba, 0x2dd2, 0x2dea, 0x2e02, 0x2e02, 0x2e02, 0x2e1a, 0x2e32, 0x2e32, 0x2e52, 0x2e6a, 0x2e6a, 0x2e6a,
};
static const uint16 kDoorTypeSrcData4[] = {
  0x2e6a, 0x2e82, 0x2e82, 0x2e9a, 0x2e9a, 0x2e9a, 0x2e9a, 0x2e9a, 0x2e9a, 0x2eb2, 0x2eb2, 0x2eb2, 0x2eb2, 0x2eca, 0x2ee2, 0x2efa,
  0x2efa, 0x2efa, 0x2efa, 0x2efa, 0x2efa, 0x2f12, 0x2f12, 0x2f2a, 0x2f42, 0x2f42, 0x2f42, 0x2f42, 0x2f5a, 0x2f72, 0x2f72, 0x2f72,
  0x2f72, 0x2f8a, 0x2fa2, 0x2fba, 0x2fd2, 0x2fea, 0x3002, 0x3002, 0x3002, 0x301a, 0x3032, 0x3032, 0x3052, 0x306a, 0x306a,
};
static const uint16 kDoorPositionToTilemapOffs_Up[] = { 0x21c, 0x23c, 0x25c, 0x39c, 0x3bc, 0x3dc, 0x121c, 0x123c, 0x125c, 0x139c, 0x13bc, 0x13dc };
static const uint16 kDoorPositionToTilemapOffs_Down[] = { 0xd1c, 0xd3c, 0xd5c, 0xb9c, 0xbbc, 0xbdc, 0x1d1c, 0x1d3c, 0x1d5c, 0x1b9c, 0x1bbc, 0x1bdc };
static const uint16 kDoorPositionToTilemapOffs_Left[] = { 0x784, 0xf84, 0x1784, 0x78a, 0xf8a, 0x178a, 0x7c4, 0xfc4, 0x17c4, 0x7ca, 0xfca, 0x17ca };
static const uint16 kDoorPositionToTilemapOffs_Right[] = { 0x7b4, 0xfb4, 0x17b4, 0x7ae, 0xfae, 0x17ae, 0x7f4, 0xff4, 0x17f4, 0x7ee, 0xfee, 0x17ee };

static const int8 kSpiralTab1[] = { 0, 1, 1, -1, 1, 1, 1, 1 };
static const int8 kTeleportPitLevel1[] = { 0, 1, 1 };
static const int8 kTeleportPitLevel2[] = { 0, 0, 1 };

static const uint8 kDoorTypeRemap[] = {
  0, 2, 0, 0, 0, 0, 0, 0, 0, 18, 0, 0, 80, 0, 80, 80,
  96, 98, 100, 102, 82, 90, 80, 82, 84, 86, 0, 80, 80, 0, 0, 0,
  64, 88, 88, 0, 88, 88, 0, 0,
};
static const int8 kStaircaseTab2[] = {
  12, 32, 48, 56, 72, -44, -40, -64, -64, -88, 12, 24, 40, 48, 64, -28,
  -40, -56, -64, -80,
};
static const int8 kStaircaseTab3[] = { 4, -4, 4, -4 };
static const int8 kStaircaseTab4[] = { 52, 52, 59, 58 };
static const int8 kStaircaseTab5[] = { 32, -64, 32, -32 };

uint16 *DstoPtr(uint16 d) {
  return (uint16 *)&(g_ram[dung_line_ptrs_row0 + d * 2]);
}

void Object_Size_1_to_15_or_32() {
  uint16 x = dung_draw_width_indicator << 2 | dung_draw_height_indicator;
  dung_draw_width_indicator = x ? x : 32;
}

void Object_Size_1_to_15_or_26() {
  uint16 x = dung_draw_width_indicator << 2 | dung_draw_height_indicator;
  dung_draw_width_indicator = x ? x : 26;
}

void Object_SizeAtoAplus15(uint8 a) {
  dung_draw_width_indicator = (dung_draw_width_indicator << 2 | dung_draw_height_indicator) + a;
  dung_draw_height_indicator = 0;
}


void Object_Size_1_to_16() {
  dung_draw_width_indicator = (dung_draw_width_indicator << 2 | dung_draw_height_indicator) + 1;
  dung_draw_height_indicator = 0;
}


void Object_Draw_2x2(const uint16 *src, uint16 *dst) {
  dst[XY(0, 0)] = src[0];
  dst[XY(0, 1)] = src[1];
  dst[XY(1, 0)] = src[2];
  dst[XY(1, 1)] = src[3];
}


uint16 *Object_Draw1x3(const uint16 *src, uint16 *dst) {
  dst[XY(0, 0)] = src[0];
  dst[XY(0, 1)] = src[1];
  dst[XY(0, 2)] = src[2];
  return dst;
}

uint16 *Object_Draw1x5(const uint16 *src, uint16 *dst) {
  dst[XY(0, 0)] = src[0];
  dst[XY(0, 1)] = src[1];
  dst[XY(0, 2)] = src[2];
  dst[XY(0, 3)] = src[3];
  dst[XY(0, 4)] = src[4];
  return dst;
}

uint16 *Object_Draw1x4(const uint16 *src, uint16 *dst) {
  dst[XY(0, 0)] = src[0];
  dst[XY(0, 1)] = src[1];
  dst[XY(0, 2)] = src[2];
  dst[XY(0, 3)] = src[3];
  return dst;
}

void Object_Draw_3x2(const uint16 *src, uint16 *dst) {
  dst[XY(0, 0)] = src[0];
  dst[XY(1, 0)] = src[1];
  dst[XY(2, 0)] = src[2];
  dst[XY(0, 1)] = src[3];
  dst[XY(1, 1)] = src[4];
  dst[XY(2, 1)] = src[5];
}


void Object_Draw_Nx3(int n, const uint16 *src, uint16 *dst) {
  do {
    dst[XY(0, 0)] = src[0];
    dst[XY(0, 1)] = src[1];
    dst[XY(0, 2)] = src[2];
    dst++, src += 3;
  } while (--n);
}

void Object_Fill_Nx1(int n, const uint16 *src, uint16 *dst) {
  int t = src[0];
  do *dst++ = t; while (--n);
}


void Object_Draw_Nx4(int n, const uint16 *src, uint16 *dst) {
  do {
    dst[XY(0, 0)] = src[0];
    dst[XY(0, 1)] = src[1];
    dst[XY(0, 2)] = src[2];
    dst[XY(0, 3)] = src[3];
    src += 4;
    dst += XY(1, 0);
  } while (--n);
}

void Object_Draw_5x4(const uint16 *src, uint16 *dst) {
  int n = 5;
  do {
    dst[XY(0, 0)] = src[0];
    dst[XY(1, 0)] = src[1];
    dst[XY(2, 0)] = src[2];
    dst[XY(3, 0)] = src[3];
    dst += XY(0, 1), src += 4;
  } while (--n);
}

void Object_Draw_N_4x4(int n, const uint16 *src, uint16 *dst) {
  do {
    // draw 4x2 twice
    for (int i = 0; i < 2; i++) {
      dst[XY(0, 0)] = src[0];
      dst[XY(1, 0)] = src[1];
      dst[XY(2, 0)] = src[2];
      dst[XY(3, 0)] = src[3];
      dst[XY(0, 1)] = src[4];
      dst[XY(1, 1)] = src[5];
      dst[XY(2, 1)] = src[6];
      dst[XY(3, 1)] = src[7];
      dst += XY(0, 2);
    }
    dst += XY(4, -4);
  } while (--n);
}

void Object_Draw4x4(const uint16 *src, uint16 *dst) {
  Object_Draw_Nx4(4, src, dst);
}

void Object_Draw_4x2_N(int increment, const uint16 *src, uint16 *dst) {
  do {
    dst[XY(0, 0)] = src[0];
    dst[XY(1, 0)] = src[1];
    dst[XY(2, 0)] = src[2];
    dst[XY(3, 0)] = src[3];
    dst[XY(0, 1)] = src[4];
    dst[XY(1, 1)] = src[5];
    dst[XY(2, 1)] = src[6];
    dst[XY(3, 1)] = src[7];
    dst += increment;
  } while (--dung_draw_width_indicator);
}

void Object_Draw_4x2_BothBgs(const uint16 *src, uint16 dsto) {
  dung_bg1[dsto + XY(0, 0)] = dung_bg2[dsto + XY(0, 0)] = src[0];
  dung_bg1[dsto + XY(1, 0)] = dung_bg2[dsto + XY(1, 0)] = src[1];
  dung_bg1[dsto + XY(2, 0)] = dung_bg2[dsto + XY(2, 0)] = src[2];
  dung_bg1[dsto + XY(3, 0)] = dung_bg2[dsto + XY(3, 0)] = src[3];
  dung_bg1[dsto + XY(0, 1)] = dung_bg2[dsto + XY(0, 1)] = src[4];
  dung_bg1[dsto + XY(1, 1)] = dung_bg2[dsto + XY(1, 1)] = src[5];
  dung_bg1[dsto + XY(2, 1)] = dung_bg2[dsto + XY(2, 1)] = src[6];
  dung_bg1[dsto + XY(3, 1)] = dung_bg2[dsto + XY(3, 1)] = src[7];
}


void Object_Pot(const uint16 *src, uint16 *dst, uint16 dsto) {
  int i = dung_misc_objs_index >> 1;
  dung_misc_objs_index += 2;
  dung_replacement_tile_state[i] = 0x1111;
  dung_object_pos_in_objdata[i] = dung_load_ptr_offs;
  dung_object_tilemap_pos[i] = (dsto * 2) | (dung_line_ptrs_row0 != 0x4000 ? 0 : 0x2000);
  replacement_tilemap_UL[i] = 0x0d0e;
  replacement_tilemap_LL[i] = 0x0d1e;
  replacement_tilemap_UR[i] = 0x4d0e;
  replacement_tilemap_LR[i] = 0x4d1e;
  if (savegame_is_darkworld)
    src = SrcPtr(0xe92);
  Object_Draw_2x2(src, dst);
}

void Object_PegBlock(const uint16 *src, uint16 *dst, uint16 dsto) {
  int i = dung_misc_objs_index >> 1;
  dung_misc_objs_index += 2;
  dung_replacement_tile_state[i] = 0x4040;
  dung_object_pos_in_objdata[i] = dung_load_ptr_offs;
  dung_object_tilemap_pos[i] = (dsto * 2) | (dung_line_ptrs_row0 != 0x4000 ? 0 : 0x2000);
  replacement_tilemap_UL[i] = 0x19d8;
  replacement_tilemap_LL[i] = 0x19d9;
  replacement_tilemap_UR[i] = 0x59d8;
  replacement_tilemap_LR[i] = 0x59d9;
  Object_Draw_2x2(src, dst);
}


void Object_ChestPlatform_Helper(const uint16 *src, int dsto) {
  dung_bg2[dsto] = src[0];
  int n = src[3];
  for (int i = dung_draw_width_indicator; i--; dsto++)
    dung_bg2[1 + dsto] = n;

  dung_bg2[1 + dsto] = src[6];
  dung_bg2[2 + dsto] = dung_bg2[3 + dsto] = dung_bg2[4 + dsto] = dung_bg2[5 + dsto] = src[9];

  dung_bg2[6 + dsto] = src[12];
  n = src[15];
  for (int i = dung_draw_width_indicator; i--; dsto++)
    dung_bg2[7 + dsto] = n;

  dung_bg2[7 + dsto] = src[18];
}

void Object_Table_Helper(const uint16 *src, uint16 *dst) {
  int n = dung_draw_width_indicator;
  dst[0] = src[0];
  do {
    dst[1] = src[1];
    dst[2] = src[2];
    dst += 2;
  } while (--n);
  dst[1] = src[3];
}

void Object_Hole(const uint16 *src, uint16 *dst) {
  Object_SizeAtoAplus15(4);
  int w = dung_draw_width_indicator;
  for (int i = 0; i < w; i++)
    Object_Fill_Nx1(w, src, dst + XY(0, i));
  // fill top/bottom
  src = SrcPtr(0x63c);
  dst[XY(0, 0)] = src[0];
  Object_Fill_Nx1(w - 2, src + 1, dst + XY(1, 0));
  dst[XY(w - 1, 0)] = src[2];

  dst[XY(0, w - 1)] = src[3];
  Object_Fill_Nx1(w - 2, src + 4, dst + XY(1, w - 1));
  dst[XY(w - 1, w - 1)] = src[5];

  // fill left/right edge
  src = SrcPtr(0x648);
  for (int i = 1; i < w - 1; i++) {
    dst[XY(0, i)] = src[0];
    dst[XY(w - 1, i)] = src[1];
  }
}

bool Object_MovingWall_IsEnabled() {
  dung_some_subpixel[0] = dung_some_subpixel[1] = 0;
  dung_floor_move_flags = 0;
  int i;
  if ((i = 0, dung_hdr_tag[0] >= 0x1c && dung_hdr_tag[0] < 0x20) ||
      (i = 1, dung_hdr_tag[1] >= 0x1c && dung_hdr_tag[1] < 0x20)) {
    if (dung_savegame_state_bits & (0x1000 >> i)) {
      dung_hdr_collision = 0;
      dung_hdr_tag[i] = 0;
      dung_hdr_bg2_properties = 0;
      return false;
    }
  }
  return true;
}

void DrawWaterThing(uint16 *dst, const uint16 *src) {
  for (int i = 3; i >= 0; i--) {
    dst[0] = src[0];
    dst[1] = src[1];
    dst[2] = src[2];
    dst[3] = src[3];
    dst += XY(0, 1), src += 4;
  }
}

static const uint8 kMovingWall_Sizes0[4] = { 5, 7, 11, 15 };
static const uint8 kMovingWall_Sizes1[4] = { 8, 16, 24, 32 };

void Object_MovingWall_Func1(int dsto) {
  for (int i = 0; i < 64; i++)
    moving_wall_arr1[i] = 0x1ec;
  moving_wall_var1 = (dsto & 0x1f) | (dsto & 0x20 ? 0x400 : 0) | 0x1000;
}

// dsto is half the value of Y
void LoadType1ObjectSubtype1(uint8 idx, uint16 *dst, uint16 dsto) {
  uint16 param1 = kObjectSubtype1Params[idx];
  const uint16 *src = SrcPtr(param1);
  int n;

  switch (idx) {
  case 0x0:  // Object_Draw2x2s_AdvanceRight - Ceiling
  case 0xb8: case 0xb9: // B8 -  Blue Switch Block [L-R]
    Object_Size_1_to_15_or_32();
    do {
      Object_Draw_2x2(src, dst), dst += XY(2, 0);
    } while (--dung_draw_width_indicator);
    break;
  case 0x1: case 0x2:  // Object_Draw_WallHorz_LR - [N]Wall Horz: [L-R]
  case 0xb6: case 0xb7:  // B6 -  [N]Wall Decor: 1/2 [L-R]
    Object_Size_1_to_15_or_26();
    do {
      Object_Draw_Nx4(2, src, dst), dst += XY(2, 0);
    } while (--dung_draw_width_indicator);
    break;

  case 0x3: case 0x4:  // Object_Draw_WallHorz_LR_BothBgs - 03 -  [N]Wall Horz: (LOW) [L-R]
    Object_Size_1_to_16();
    do {
      dung_bg1[dsto + XY(0, 0)] = dung_bg2[dsto + XY(0, 0)] = src[0];
      dung_bg1[dsto + XY(0, 1)] = dung_bg2[dsto + XY(0, 1)] = src[1];
      dung_bg1[dsto + XY(0, 2)] = dung_bg2[dsto + XY(0, 2)] = src[2];
      dung_bg1[dsto + XY(0, 3)] = dung_bg2[dsto + XY(0, 3)] = src[3];
      dung_bg1[dsto + XY(1, 0)] = dung_bg2[dsto + XY(1, 0)] = src[4];
      dung_bg1[dsto + XY(1, 1)] = dung_bg2[dsto + XY(1, 1)] = src[5];
      dung_bg1[dsto + XY(1, 2)] = dung_bg2[dsto + XY(1, 2)] = src[6];
      dung_bg1[dsto + XY(1, 3)] = dung_bg2[dsto + XY(1, 3)] = src[7];
      dsto += XY(2, 0);
    } while (--dung_draw_width_indicator);
    break;

  case 0x5: case 0x6: // Object_Draw_05 - 05 -  [N]Wall Column [L-R]
    Object_Size_1_to_16();
    do {
      Object_Draw_Nx4(2, src, dst), dst += XY(6, 0);
    } while (--dung_draw_width_indicator);
    break;

  case 0x7: case 0x8: case 0x53: // Object_Draw_07 - 07 -  [N]Wall Pit [L-R]
    Object_Size_1_to_16();
    do {
      Object_Draw_2x2(src, dst), dst += XY(2, 0);
    } while (--dung_draw_width_indicator);
    break;

  case 0x9: case 0x0c: case 0x0d: case 0x10: case 0x11: case 0x14:  // 09 -  / Wall Wood Bot (HIGH) [NW]
    Object_SizeAtoAplus15(6);
    do {
      Object_Draw1x5(src, dst), dst += XY(1, -1);
    } while (--dung_draw_width_indicator);
    break;

  case 0x0a: case 0x0b: case 0x0e: case 0x0f: case 0x12: case 0x13:  // 12 -  \ Wall Tile2 Bot (HIGH) [SW]
    Object_SizeAtoAplus15(6);
    do {
      Object_Draw1x5(src, dst), dst += XY(1, 1);
    } while (--dung_draw_width_indicator);
    break;

  case 0x15: case 0x18: case 0x19: case 0x1C: case 0x1D: case 0x20:  // 15 -  / Wall Tile Top (LOW)[NW]
    Object_SizeAtoAplus15(6);
    do {
      dung_bg1[dsto + XY(0, 0)] = dung_bg2[dsto + XY(0, 0)] = src[0];
      dung_bg1[dsto + XY(0, 1)] = dung_bg2[dsto + XY(0, 1)] = src[1];
      dung_bg1[dsto + XY(0, 2)] = dung_bg2[dsto + XY(0, 2)] = src[2];
      dung_bg1[dsto + XY(0, 3)] = dung_bg2[dsto + XY(0, 3)] = src[3];
      dung_bg1[dsto + XY(0, 4)] = dung_bg2[dsto + XY(0, 4)] = src[4];
      dsto -= 63;
    } while (--dung_draw_width_indicator);
    break;

  case 0x16: case 0x17: case 0x1A: case 0x1B: case 0x1E: case 0x1F:  // 16 -  \ Wall Tile Top (LOW)[SW]
    Object_SizeAtoAplus15(6);
    do {
      dung_bg1[dsto + XY(0, 0)] = dung_bg2[dsto + XY(0, 0)] = src[0];
      dung_bg1[dsto + XY(0, 1)] = dung_bg2[dsto + XY(0, 1)] = src[1];
      dung_bg1[dsto + XY(0, 2)] = dung_bg2[dsto + XY(0, 2)] = src[2];
      dung_bg1[dsto + XY(0, 3)] = dung_bg2[dsto + XY(0, 3)] = src[3];
      dung_bg1[dsto + XY(0, 4)] = dung_bg2[dsto + XY(0, 4)] = src[4];
      dsto += 65;
    } while (--dung_draw_width_indicator);
    break;

  case 0x21:  // 21 -  Mini Stairs [L-R]
    dung_draw_width_indicator = (dung_draw_width_indicator << 2 | dung_draw_height_indicator) * 2 + 1;
    Object_Draw_Nx3(2, src, dst), dst += XY(2, 0);
    do {
      Object_Draw_Nx3(1, src + 3, dst), dst += XY(1, 0);
    } while (--dung_draw_width_indicator);
    Object_Draw_Nx3(1, src + 6, dst);
    break;

  case 0x22: { // 22 -  Horz: Rail Thin [L-R]
    Object_SizeAtoAplus15(2);
    if ((dst[0] & 0x3ff) != 0xe2)
      dst[0] = src[0];
    n = src[1];
    do *++dst = n; while (--dung_draw_width_indicator);
    dst[1] = src[2];
    break;
  }
  case 0x23: case 0x24: case 0x25: case 0x26: case 0x27: case 0x28:
  case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e:  // 23 -  Pit [N]Edge [L-R]
  case 0x3f: case 0x40: case 0x41: case 0x42: case 0x43: case 0x44:  // 3F -  Water Edge [L-R]
  case 0x45: case 0x46: case 0xb3: case 0xb4:
    Object_Size_1_to_16();
    n = dst[0] & 0x3ff;
    if (n != 0x1db && n != 0x1a6 && n != 0x1dd && n != 0x1fc)
      dst[0] = src[0];
    n = src[1];
    do *++dst = n; while (--dung_draw_width_indicator);
    dst[1] = src[2];
    break;

  case 0x2f: // 2F -  Rail Wall [L-R]
    Object_SizeAtoAplus15(10);
    n = *src++;
    if ((dst[0] & 0x3ff) != 0xe2) {
      dst[XY(0, 0)] = src[0];
      dst[XY(1, 0)] = src[1];
      dst[XY(1, 1)] = dst[XY(0, 1)] = n;
      dst += 2;
    }
    src += 2;
    do {
      dst[XY(0, 0)] = src[0];
      dst[XY(0, 1)] = n;
    } while (dst++, --dung_draw_width_indicator);
    src++;
    dst[XY(0, 0)] = src[0];
    dst[XY(1, 0)] = src[1];
    dst[XY(1, 1)] = dst[XY(0, 1)] = n;
    break;

  case 0x30:  // 30 -  Rail Wall [L-R]
    Object_SizeAtoAplus15(10);
    n = *src++;
    if ((dst[XY(0, 1)] & 0x3ff) != 0xe2) {
      dst[XY(0, 0)] = dst[XY(1, 0)] = n;
      dst[XY(0, 1)] = src[0];
      dst[XY(1, 1)] = src[1];
      dst += 2;
    }
    src += 2;
    do {
      dst[XY(0, 0)] = n;
      dst[XY(0, 1)] = src[0];
    } while (dst++, --dung_draw_width_indicator);
    src++;
    dst[XY(0, 0)] = dst[XY(1, 0)] = n;
    dst[XY(0, 1)] = src[0];
    dst[XY(1, 1)] = src[1];
    break;
  case 0x31: case 0x32:  // 31 -  Unused -empty
  case 0x35: case 0x54: case 0x57:case 0x58:case 0x59:case 0x5A:
    break;
  case 0x33:  // 33 -  Red Carpet Floor [L-R]
  case 0xb2: case 0xba:  // B2 -  Floor? [L-R]
    Object_Size_1_to_16();
    do {
      Object_Draw4x4(src, dst), dst += XY(4, 0);
    } while (--dung_draw_width_indicator);
    break;
  case 0x34:  // 34 -  Red Carpet Floor Trim [L-R]
    Object_SizeAtoAplus15(4);
    n = src[0];
    do *dst++ = n; while (--dung_draw_width_indicator);
    break;
  case 0x36: case 0x37: // 36 -  [N]Curtain [L-R]
    Object_Size_1_to_16();
    do {
      Object_Draw4x4(src, dst), dst += XY(6, 0);
    } while (--dung_draw_width_indicator);
    break;
  case 0x38:  // 38 -  Statue [L-R]
    src = (uint16 *)((uint8 *)src - param1 + 0xe26);
    Object_Size_1_to_16();
    do {
      Object_Draw_Nx3(2, src, dst), dst += XY(4, 0);
    } while (--dung_draw_width_indicator);
    break;
  case 0x39: case 0x3d: // 39 -  Column [L-R]
    Object_Size_1_to_16();
    do {
      Object_Draw_Nx4(2, src, dst), dst += XY(6, 0);
    } while (--dung_draw_width_indicator);
    break;
  case 0x3a: case 0x3b: // 3A -  [N]Wall Decor: [L-R]
    Object_Size_1_to_16();
    do {
      Object_Draw_Nx3(4, src, dst), dst += XY(8, 0);
    } while (--dung_draw_width_indicator);
    break;
  case 0x3c:  // 3C -  Double Chair [L-R]
    Object_Size_1_to_16();
    do {
      const uint16 *src = SrcPtr(0x8ca);
      Object_Draw_2x2(src + 0, dst);
      Object_Draw_2x2(src + 4, dst + XY(0, 6));
      dst += 4;
    } while (--dung_draw_width_indicator);
    break;
  case 0x3e: case 0x4b: // 3E -  [N]Wall Column [L-R]
    Object_Size_1_to_16();
    do {
      Object_Draw_2x2(src, dst), dst += XY(14, 0);
    } while (--dung_draw_width_indicator);
    break;
  case 0x47:  // 47 -  Unused Waterfall [L-R]
    Object_Size_1_to_16();
    dung_draw_width_indicator <<= 1;
    dst = Object_Draw1x5(src, dst) + 1;
    do {
      Object_Draw1x5(src + 5, dst);
    } while (dst++, --dung_draw_width_indicator);
    Object_Draw1x5(src + 10, dst);
    break;
  case 0x48:
    Object_Size_1_to_16();
    dung_draw_width_indicator <<= 1;
    Object_Draw_Nx3(1, src, dst), dst += XY(1, 0);
    do {
      dst[XY(0, 0)] = src[3];
      dst[XY(0, 1)] = src[4];
      dst[XY(0, 2)] = src[5];
    } while (dst++, --dung_draw_width_indicator);
    Object_Draw_Nx3(1, src + 6, dst);
    break;
  case 0x49: case 0x4A:  // Object_Draw_49      ; 49 -  N/A
    Object_Size_1_to_16();
    Object_Draw_4x2_N(4, src, dst);
    break;
  case 0x4c:  // 4C -  Bar [L-R]
    Object_Size_1_to_16();
    dung_draw_width_indicator <<= 1;
    dst = Object_Draw1x3(src, dst) + 1;
    do {
      dst = Object_Draw1x3(src + 3, dst) + 1;
    } while (--dung_draw_width_indicator);
    dst = Object_Draw1x3(src + 6, dst) + 1;
    break;

  case 0x4d: case 0x4e: case 0x4f:  // 4C -  Bar [L-R]
    Object_Size_1_to_16();
    Object_Draw_Nx4(1, src, dst), dst += XY(1, 0);
    do {
      Object_Draw_Nx4(2, src + 4, dst), dst += XY(2, 0);
    } while (--dung_draw_width_indicator);
    Object_Draw1x4(src + 12, dst);
    break;

  case 0x50:   // 50 -  Cane Ride [L-R]
    Object_SizeAtoAplus15(2);
    n = src[0];
    do *dst++ = n; while (--dung_draw_width_indicator);
    break;

  case 0x51: case 0x52: // 51 -  [N]Canon Hole [L-R]
  case 0x5B: case 0x5C:
    Object_Size_1_to_16();
    Object_Draw_Nx3(2, src, dst), dst += XY(2, 0);
    while (--dung_draw_width_indicator)
      Object_Draw_Nx3(2, src + 6, dst), dst += XY(2, 0);
    Object_Draw_Nx3(2, src + 12, dst);
    break;

  case 0x55: case 0x56:  // 55 -  [N]Wall Torches [L-R]
    Object_Size_1_to_16();
    Object_Draw_4x2_N(12, src, dst);
    break;

  case 0x5D:  // 5D -  Large Horz: Rail [L-R]
    Object_Size_1_to_16();
    dung_draw_width_indicator++;
    Object_Draw_Nx3(2, src, dst), dst += XY(2, 0);
    do {
      Object_Draw1x3(src + 6, dst), dst += XY(1, 0);
    } while (--dung_draw_width_indicator);
    Object_Draw_Nx3(2, src + 9, dst);
    break;

  case 0x5E:  // 5E -  Block [L-R]
  case 0xbb:  // BB -  N/A
    Object_Size_1_to_16();
    do {
      Object_Draw_2x2(src, dst), dst += XY(4, 0);
    } while (--dung_draw_width_indicator);
    break;

  case 0x5f: { // 5F -  Long Horz: Rail [L-R]
    Object_SizeAtoAplus15(21);
    if ((dst[0] & 0x3ff) != 0xe2)
      dst[0] = src[0];
    n = src[1];
    do *++dst = n; while (--dung_draw_width_indicator);
    dst[1] = src[2];
    break;
  }

  case 0x60:  // 60 -  Ceiling [U-D]
  case 0x92: case 0x93: // 92 -  Blue Peg Block [U-D]
    Object_Size_1_to_15_or_32();
    do {
      Object_Draw_2x2(src, dst), dst += XY(0, 2);
    } while (--dung_draw_width_indicator);
    break;

  case 0x61: case 0x62: case 0x90: case 0x91:  // 61 -  [W]Wall Vert: [U-D]
    Object_Size_1_to_15_or_26();
    Object_Draw_4x2_N(2 * 64, src, dst);
    break;

  case 0x63:
  case 0x64:  // Object_Draw_WallVert_BothBg - 63 -  [W]Wall Vert: (LOW) [U-D]
    Object_Size_1_to_16();
    do {
      dung_bg1[dsto + XY(0, 0)] = dung_bg2[dsto + XY(0, 0)] = src[0];
      dung_bg1[dsto + XY(1, 0)] = dung_bg2[dsto + XY(1, 0)] = src[1];
      dung_bg1[dsto + XY(2, 0)] = dung_bg2[dsto + XY(2, 0)] = src[2];
      dung_bg1[dsto + XY(3, 0)] = dung_bg2[dsto + XY(3, 0)] = src[3];
      dung_bg1[dsto + XY(0, 1)] = dung_bg2[dsto + XY(0, 1)] = src[4];
      dung_bg1[dsto + XY(1, 1)] = dung_bg2[dsto + XY(1, 1)] = src[5];
      dung_bg1[dsto + XY(2, 1)] = dung_bg2[dsto + XY(2, 1)] = src[6];
      dung_bg1[dsto + XY(3, 1)] = dung_bg2[dsto + XY(3, 1)] = src[7];
      dsto += XY(0, 2);
    } while (--dung_draw_width_indicator);
    break;

  case 0x65: case 0x66:  // 65 -  [W]Wall Column [U-D]
    Object_Size_1_to_16();
    Object_Draw_4x2_N(6 * 64, src, dst);
    break;

  case 0x67: case 0x68: // 67 -  [W]Wall Pit [U-D]
  case 0x7d:            // 7D -  Pipe Ride [U-D]
    Object_Size_1_to_16();
    do {
      Object_Draw_2x2(src, dst), dst += XY(0, 2);
    } while (--dung_draw_width_indicator);
    break;
  case 0x69:  // 69 -  Vert: Rail Thin [U-D]
    Object_SizeAtoAplus15(2);
    if ((dst[0] & 0x3ff) != 0xe3)
      dst[0] = src[0];
    n = src[1];
    do { dst += 64; *dst = n; } while (--dung_draw_width_indicator);
    dst[64] = src[2];
    break;
  case 0x6a: case 0x6b: // 6A -  [W]Pit Edge [U-D]
  case 0x79: case 0x7a: // 79 -  Water Edge [U-D]
  case 0x8d: case 0x8e: // 8D -  [W]Edge [U-D]
    Object_Size_1_to_16();
    do {
      dst[0] = src[0], dst += XY(0, 1);
    } while (--dung_draw_width_indicator);
    break;
  case 0x6c: // 6C -  [W]Rail Wall [U-D]
    Object_SizeAtoAplus15(10);
    n = *src++;
    if ((dst[0] & 0x3ff) != 0xe3) {
      dst[XY(0, 0)] = src[0];
      dst[XY(0, 1)] = src[1];
      dst[XY(1, 0)] = dst[XY(1, 1)] = n;
      dst += XY(0, 2);
    }
    src += 2;
    do {
      dst[XY(0, 0)] = src[0];
      dst[XY(1, 0)] = n;
      dst += XY(0, 1);
    } while (--dung_draw_width_indicator);
    src += 1;
    dst[XY(0, 0)] = src[0];
    dst[XY(0, 1)] = src[1];
    dst[XY(1, 0)] = dst[XY(1, 1)] = n;
    break;
  case 0x6d:   // 6D -  [E]Rail Wall [U-D]
    Object_SizeAtoAplus15(10);
    n = *src++;
    if ((dst[XY(1, 0)] & 0x3ff) != 0xe3) {
      dst[XY(0, 0)] = dst[XY(0, 1)] = n;
      dst[XY(1, 0)] = src[0];
      dst[XY(1, 1)] = src[1];
      dst += XY(0, 2);
    }
    src += 2;
    do {
      dst[XY(0, 0)] = n;
      dst[XY(1, 0)] = src[0];
      dst += XY(0, 1);
    } while (--dung_draw_width_indicator);
    src += 1;
    dst[XY(0, 0)] = dst[XY(0, 1)] = n;
    dst[XY(1, 0)] = src[0];
    dst[XY(1, 1)] = src[1];
    break;
  case 0x6e: case 0x6f: // unused
  case 0x72: case 0x7e:
    break;
  case 0x70: case 0x94:  // 70 -  Red Floor/Wire Floor [U-D]
    Object_Size_1_to_16();
    do {
      Object_Draw4x4(src, dst), dst += XY(0, 4);
    } while (--dung_draw_width_indicator);
    break;
  case 0x71:  // 71 -  Red Carpet Floor Trim [U-D]
    Object_SizeAtoAplus15(4);
    do {
      *dst = src[0], dst += XY(0, 1);
    } while (--dung_draw_width_indicator);
    break;
  case 0x73: case 0x74:  // 73 -  [W]Curtain [U-D]
    Object_Size_1_to_16();
    do {
      Object_Draw4x4(src, dst), dst += XY(0, 6);
    } while (--dung_draw_width_indicator);
    break;
  case 0x75: case 0x87:  // 75 -  Column [U-D]
    Object_Size_1_to_16();
    do {
      Object_Draw_Nx4(2, src, dst), dst += XY(0, 6);
    } while (--dung_draw_width_indicator);
    break;
  case 0x76: case 0x77:  // 76 -  [W]Wall Decor: [U-D]
    Object_Size_1_to_16();
    do {
      Object_Draw_Nx4(3, src, dst), dst += XY(0, 8);
    } while (--dung_draw_width_indicator);
    break;
  case 0x78: case 0x7b:  // 78 -  [W]Wall Top Column [U-D]
    Object_Size_1_to_16();
    do {
      Object_Draw_2x2(src, dst), dst += XY(0, 14);
    } while (--dung_draw_width_indicator);
    break;
  case 0x7c:  // 7C -  Cane Ride [U-D]
    Object_Size_1_to_16();
    dung_draw_width_indicator += 1;
    do {
      dst[0] = src[0], dst += XY(0, 1);
    } while (--dung_draw_width_indicator);
    break;
  case 0x7f: case 0x80:  // 7F -  [W]Wall Torches [U-D]
    Object_Size_1_to_16();
    do {
      Object_Draw_Nx4(2, src, dst), dst += XY(0, 12);
    } while (--dung_draw_width_indicator);
    break;
  case 0x81: case 0x82: case 0x83: case 0x84: // 81 -  [W]Wall Decor: [U-D]
    Object_Size_1_to_16();
    do {
      Object_Draw_Nx4(3, src, dst), dst += XY(0, 6);
    } while (--dung_draw_width_indicator);
    break;
  case 0x85: case 0x86:  // 85 -  [W]Wall Canon Hole [U-D]
    Object_Size_1_to_16();
    Object_Draw_3x2(src, dst), dst += XY(0, 2);
    while (--dung_draw_width_indicator)
      Object_Draw_3x2(src + 6, dst), dst += XY(0, 2);
    Object_Draw_3x2(src + 12, dst);
    break;
  case 0x88: // 88 -  Large Vert: Rail [U-D]
    Object_Size_1_to_16();
    Object_Draw_2x2(src, dst), dst += XY(0, 2), src += 4;
    do {
      dst[XY(0, 0)] = src[0];
      dst[XY(1, 0)] = src[1];
      dst += XY(0, 1);
    } while (--dung_draw_width_indicator);
    Object_Draw_Nx3(2, src + 2, dst);
    break;
  case 0x89: // 89 -  Block Vert: [U-D]
    Object_Size_1_to_16();
    do {
      Object_Draw_2x2(src, dst), dst += XY(0, 4);
    } while (--dung_draw_width_indicator);
    break;
  case 0x8a: // 8A -  Long Vert: Rail [U-D]
    Object_SizeAtoAplus15(21);
    if ((dst[0] & 0x3ff) != 0xe3)
      dst[0] = src[0];
    n = src[1];
    do { dst += XY(0, 1); *dst = n; } while (--dung_draw_width_indicator);
    dst[XY(0, 1)] = src[2];
    break;
  case 0x8b: case 0x8c: // 8B -  [W]Vert: Jump Edge [U-D]
    Object_SizeAtoAplus15(8);
    do {
      dst[0] = src[0], dst += XY(0, 1);
    } while (--dung_draw_width_indicator);
    break;
  case 0x8f: // 8F -  N/A
    Object_SizeAtoAplus15(2);
    dung_draw_width_indicator <<= 1;
    dst[XY(0, 0)] = src[0], dst[XY(1, 0)] = src[1];
    do {
      dst[XY(0, 1)] = src[2], dst[XY(1, 1)] = src[3], dst += XY(0, 1);
    } while (--dung_draw_width_indicator);
    break;
  case 0x95:  // 95 -  Fake Pot [U-D]
    Object_Size_1_to_16();
    do {
      Object_Pot(src, dst, dsto);
      dst += XY(0, 2), dsto += XY(0, 2);
    } while (--dung_draw_width_indicator);
    break;
  case 0x96:  // 96 -  Hammer Peg Block [U-D]
    Object_Size_1_to_16();
    do {
      Object_PegBlock(src, dst, dsto);
      dst += XY(0, 2), dsto += XY(0, 2);
    } while (--dung_draw_width_indicator);
    break;
  case 0x97: case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
  case 0xad: case 0xae: case 0xaf:
  case 0xbe: case 0xbf:
    break;
  case 0xa0:  // A0 -  / Ceiling [NW]
  case 0xa5: case 0xa9:  // A5 -  / Ceiling [Trans][NW]
    Object_SizeAtoAplus15(4);
    do {
      Object_Fill_Nx1(dung_draw_width_indicator, src, dst), dst += XY(0, 1);
    } while (--dung_draw_width_indicator);
    break;
  case 0xa1:  // A1 -  \ Ceiling [SW]
  case 0xa6: case 0xaa:  // A6 -  \ Ceiling [Trans][SW]
    Object_SizeAtoAplus15(4);
    n = 1;
    do {
      Object_Fill_Nx1(n++, src, dst), dst += XY(0, 1);
    } while (--dung_draw_width_indicator);
    break;
  case 0xa2:  // A2 -  \ Ceiling [NE]
  case 0xa7: case 0xab:  // A7 -  \ Ceiling [Trans][NE]
    Object_SizeAtoAplus15(4);
    do {
      Object_Fill_Nx1(dung_draw_width_indicator, src, dst), dst += XY(1, 1);
    } while (--dung_draw_width_indicator);
    break;
  case 0xa3:             // A3 -  / Ceiling [SE]
  case 0xa8: case 0xac:  // A8 -  / Ceiling [Trans][SE]
    Object_SizeAtoAplus15(4);
    do {
      Object_Fill_Nx1(dung_draw_width_indicator, src, dst), dst += XY(1, -1);
    } while (--dung_draw_width_indicator);
    break;
  case 0xa4: // A4 -  Hole [4-way]
    Object_Hole(src, dst);
    break;
  case 0xb0: case 0xb1: // B0 -  [S]Horz: Jump Edge [L-R]
    Object_SizeAtoAplus15(8);
    Object_Fill_Nx1(dung_draw_width_indicator, src, dst);
    break;
  case 0xb5:  // B5 -  N/A
    Object_Size_1_to_16();
    do {
      src = SrcPtr(0xb16);
      Object_Draw_Nx4(2, src, dst), dst += XY(2, 0);
    } while (--dung_draw_width_indicator);
    break;
  case 0xbc: // BC -  fake pots [L-R]
    Object_Size_1_to_16();
    do {
      Object_Pot(src, dst, dsto);
      dst += XY(2, 0), dsto += XY(2, 0);
    } while (--dung_draw_width_indicator);
    break;
  case 0xbd: // BD -  Hammer Pegs [L-R]
    Object_Size_1_to_16();
    do {
      Object_PegBlock(src, dst, dsto);
      dst += XY(2, 0), dsto += XY(2, 0);
    } while (--dung_draw_width_indicator);
    break;
  case 0xc0: case 0xc2: // C0 -  Ceiling Large [4-way]
    n = src[0];
    for (int y = dung_draw_height_indicator; y-- >= 0; ) {
      uint16 *dst_org = dst;
      for (int x = dung_draw_width_indicator; x-- >= 0; dst += XY(4, 0)) {
        dst[XY(0, 0)] = dst[XY(1, 0)] = dst[XY(2, 0)] = dst[XY(3, 0)] = n;
        dst[XY(0, 1)] = dst[XY(1, 1)] = dst[XY(2, 1)] = dst[XY(3, 1)] = n;
        dst[XY(0, 2)] = dst[XY(1, 2)] = dst[XY(2, 2)] = dst[XY(3, 2)] = n;
        dst[XY(0, 3)] = dst[XY(1, 3)] = dst[XY(2, 3)] = dst[XY(3, 3)] = n;
      }
      dst = dst_org + XY(0, 4);
    }
    break;
  case 0xc1: { // C1 -  Chest Pedastal [4-way]
    dung_draw_width_indicator += 4;
    dung_draw_height_indicator += 1;
    // draw upper part
    uint16 *dsto = dst;
    Object_Draw_Nx3(3, src, dst), src += 9, dst += XY(3, 0);
    for (int i = dung_draw_width_indicator; i--; )
      Object_Draw_Nx3(2, src, dst), dst += XY(2, 0);
    Object_Draw_Nx3(3, src + 6, dst), src += 6 + 9;
    // draw center part
    dst = dsto + XY(0, 3);
    for (int i = dung_draw_height_indicator; i--; ) {
      uint16 *dt = dst;
      Object_Draw_3x2(src, dt), dt += XY(3, 0);
      for (int j = dung_draw_width_indicator; j--; )
        Object_Draw_2x2(src + 6, dt), dt += XY(2, 0);
      Object_Draw_3x2(src + 10, dt);
      dst += XY(0, 2);
    }
    dsto = dst;
    src += 6 + 4 + 6;
    Object_Draw_Nx3(3, src, dst), src += 9, dst += XY(3, 0);
    for (int i = dung_draw_width_indicator; i--; )
      Object_Draw_Nx3(2, src, dst), dst += XY(2, 0);
    Object_Draw_Nx3(3, src + 6, dst), src += 6 + 9;

    src = SrcPtr(0x590);
    Object_Draw_2x2(src, dsto + XY(dung_draw_width_indicator + 2, -(dung_draw_height_indicator + 1)));
    break;
  }
  case 0xc3:  // C3 -  Falling Edge Mask [4-way]
  case 0xd7:  // D7 -  overlay tile? [4-way]
    dung_draw_width_indicator++;
    dung_draw_height_indicator++;
    n = *src;
    do {
      uint16 *d = dst;
      for (int i = dung_draw_width_indicator; i--; d += XY(3, 0)) {
        d[XY(0, 0)] = d[XY(1, 0)] = d[XY(2, 0)] = n;
        d[XY(0, 1)] = d[XY(1, 1)] = d[XY(2, 1)] = n;
        d[XY(0, 2)] = d[XY(1, 2)] = d[XY(2, 2)] = n;
      }
      dst += XY(0, 3);
    } while (--dung_draw_height_indicator);
    break;

  case 0xc4: // C4 -  Doorless Room Transition
    src = SrcPtr(dung_floor_2_filler_tiles);
    goto fill_floor;

  case 0xdb: // C4 -  DB -  Floor2 [4-way]
    src = SrcPtr(dung_floor_1_filler_tiles);
    goto fill_floor;

  case 0xc5: case 0xc6: case 0xc7: case 0xc8: case 0xc9: case 0xca:
  case 0xd1: case 0xd2: case 0xd9:
  case 0xdf: case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4:
  case 0xe5: case 0xe6: case 0xe7: case 0xe8:
fill_floor:
    dung_draw_width_indicator++;
    dung_draw_height_indicator++;
    do {
      Object_Draw_N_4x4(dung_draw_width_indicator, src, dst);
      dst += XY(0, 4);
    } while (--dung_draw_height_indicator);
    break;

  case 0xcd: { // CD -  Moving Wall Right [4-way]
    if (!Object_MovingWall_IsEnabled())
      return;
    dung_hdr_collision_2_mirror++;
    int size0 = kMovingWall_Sizes0[dung_draw_width_indicator];
    int size1 = kMovingWall_Sizes1[dung_draw_height_indicator];
    Object_MovingWall_Func1(dsto - size1 - 1);
    moving_wall_var2 = dung_draw_height_indicator * 2;
    src = SrcPtr(0x3d8);
    uint16 *dst1 = dst - size1;
    do {
      uint16 *dst2 = dst1;
      dst2[XY(0, 0)] = src[0];
      int n1 = size0 * 2 + 4;
      do {
        dst2[XY(0, 1)] = src[1];
        dst2 += XY(0, 1);
      } while (--n1);
      dst2[XY(0, 1)] = src[2];
      dst1++;
    } while (--size1);
    src = SrcPtr(0x72a);
    Object_Draw_Nx3(3, src, dst);
    dst += XY(0, 3);
    do {
      Object_Draw_3x2(src + 9, dst);
    } while (dst += XY(0, 2), --size0);
    Object_Draw_Nx3(3, src + 9 + 6, dst);
    break;
  }

  case 0xce: { // CE -  Moving Wall Left [4-way]
    if (!Object_MovingWall_IsEnabled())
      return;
    dung_hdr_collision_2_mirror++;
    src = SrcPtr(0x75a);
    int size1 = kMovingWall_Sizes1[dung_draw_height_indicator];
    int size0 = kMovingWall_Sizes0[dung_draw_width_indicator];
    moving_wall_var2 = dung_draw_height_indicator * 2;
    Object_MovingWall_Func1(dsto + 3 + size1);
    uint16 *dst1 = dst;
    Object_Draw_Nx3(3, src, dst1);
    dst1 += XY(0, 3);
    int n = size0;
    do {
      Object_Draw_3x2(src + 9, dst1);
      dst1 += XY(0, 2);
    } while (--n);
    Object_Draw_Nx3(3, src + 15, dst1);
    src = SrcPtr(0x3d8);
    dst1 = dst + XY(3, 0);
    do {
      uint16 *dst2 = dst1;
      dst2[XY(0, 0)] = src[0];
      int n1 = size0 * 2 + 4;
      do {
        dst2[XY(0, 1)] = src[1];
        dst2 += XY(0, 1);
      } while (--n1);
      dst2[XY(0, 1)] = src[2];
      dst1++;
    } while (--size1);
    break;
  }

  case 0xd8: {
    // loads of lava/water hdma stuff
    dung_draw_width_indicator += 2;
    water_hdma_var3 = (dung_draw_width_indicator << 4);
    dung_draw_height_indicator += 2;
    water_hdma_var2 = (dung_draw_height_indicator << 4);
    water_hdma_var4 = water_hdma_var2 - 24;
    water_hdma_var0 = (dsto & 0x3f) << 3;
    water_hdma_var0 += (dung_draw_width_indicator << 4) + dung_loade_bgoffs_h_copy;
    water_hdma_var1 = (dsto & 0xfc0) >> 3;
    water_hdma_var1 += (dung_draw_height_indicator << 4) + dung_loade_bgoffs_v_copy;
    if (dung_savegame_state_bits & 0x800) {
      dung_hdr_tag[1] = 0;
      dung_hdr_bg2_properties = 0;
      dung_num_interpseudo_upnorth_stairs = dung_num_inroom_upnorth_stairs_water;
      dung_some_stairs_unk4 = dung_num_activated_water_ladders;
      dung_num_activated_water_ladders = 0;
      dung_num_inroom_upnorth_stairs_water = 0;
      dung_num_stairs_wet = dung_num_inroom_upsouth_stairs_water;
      dung_num_inroom_upsouth_stairs_water = 0;
      dsto += (dung_draw_width_indicator - 1) << 1;
      dsto += (dung_draw_height_indicator - 1) << 7;
      DrawWaterThing(&dung_bg2[dsto], SrcPtr(0x1438));
    } else {
      src = SrcPtr(0x110);
      do {
        Object_Draw_N_4x4(dung_draw_width_indicator, src, dst);
        dst += XY(0, 4);
      } while (--dung_draw_height_indicator);
    }
    break;
  }

  case 0xda: {  // water hdma stuff
    dung_draw_width_indicator += 2;
    water_hdma_var3 = (dung_draw_width_indicator << 4) - 24;

    dung_draw_height_indicator += 2;
    water_hdma_var4 = (dung_draw_height_indicator << 4) - 8;

    water_hdma_var2 = water_hdma_var4 - 24;
    water_hdma_var5 = 0;
    water_hdma_var0 = (dsto & 0x3f) << 3;
    water_hdma_var0 += (dung_draw_width_indicator << 4) + dung_loade_bgoffs_h_copy;
    water_hdma_var1 = (dsto & 0xfc0) >> 3;
    water_hdma_var1 += (dung_draw_height_indicator << 4) + dung_loade_bgoffs_v_copy - 8;
    if (dung_savegame_state_bits & 0x800) {
      dung_hdr_tag[1] = 0;
    } else {
      dung_hdr_bg2_properties = 0;
      dung_num_interpseudo_upnorth_stairs = dung_num_inroom_upnorth_stairs_water;
      dung_some_stairs_unk4 = dung_num_activated_water_ladders;
      dung_num_activated_water_ladders = 0;
      dung_num_inroom_upnorth_stairs_water = 0;
      dung_num_stairs_wet = dung_num_inroom_upsouth_stairs_water;
      dung_num_inroom_upsouth_stairs_water = 0;
    }
    int n = dung_draw_height_indicator * 2 - 1;
    src = SrcPtr(0x110);
    do {
      uint16 *dst2 = dst;
      int j = dung_draw_width_indicator;
      do {
        dst[XY(0, 0)] = src[0];
        dst[XY(1, 0)] = src[1];
        dst[XY(2, 0)] = src[2];
        dst[XY(3, 0)] = src[3];
        dst[XY(0, 1)] = src[4];
        dst[XY(1, 1)] = src[5];
        dst[XY(2, 1)] = src[6];
        dst[XY(3, 1)] = src[7];
        dst += 4;
      } while (--j);
      dst = dst2 + XY(0, 2);
    } while (--n);
    break;
  }

  case 0xdc: { // DC -  Chest Platform? [4-way]
    dsto |= (dung_line_ptrs_row0 != 0x4000 ? 0 : 0x1000);
    dung_draw_width_indicator++;
    dung_draw_height_indicator = dung_draw_height_indicator * 2 + 5;
    src = SrcPtr(0xAB4);
    do {
      Object_ChestPlatform_Helper(src, dsto), dsto += XY(0, 1);
    } while (--dung_draw_height_indicator);
    Object_ChestPlatform_Helper(src + 1, dsto), dsto += XY(0, 1);
    Object_ChestPlatform_Helper(src + 2, dsto), dsto += XY(0, 1);
    break;
  }
  case 0xdd: // DD -  Table / Rock [4-way]
    dung_draw_width_indicator++;
    dung_draw_height_indicator = dung_draw_height_indicator * 2 + 1;
    Object_Table_Helper(src, dst), dst += XY(0, 1);
    do {
      Object_Table_Helper(src + 4, dst), dst += XY(0, 1);
    } while (--dung_draw_height_indicator);
    Object_Table_Helper(src + 8, dst), dst += XY(0, 1);
    Object_Table_Helper(src + 12, dst), dst += XY(0, 1);
    break;

  case 0xde: // DE -  Spike Block [4-way]    
    dung_draw_width_indicator++;
    dung_draw_height_indicator++;
    do {
      int n = dung_draw_width_indicator;
      uint16 *dst1 = dst;
      do {
        Object_Draw_2x2(src, dst1), dst1 += XY(2, 0);
      } while (--n);
      dst += XY(0, 2);
    } while (--dung_draw_height_indicator);
    break;

  case 0xcb: case 0xcc: case 0xcf: case 0xd0:
  case 0xd3: case 0xd4: case 0xd5: case 0xd6:
  case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee: case 0xef:
  case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7:
    assert(0);
    break;
  default:
    assert(0);
  }
}

void Object_DrawNx4_BothBgs(int n, const uint16 *src, int dsto) {
  do {
    dung_bg1[dsto + XY(0, 0)] = dung_bg2[dsto + XY(0, 0)] = src[0];
    dung_bg1[dsto + XY(0, 1)] = dung_bg2[dsto + XY(0, 1)] = src[1];
    dung_bg1[dsto + XY(0, 2)] = dung_bg2[dsto + XY(0, 2)] = src[2];
    dung_bg1[dsto + XY(0, 3)] = dung_bg2[dsto + XY(0, 3)] = src[3];
    src += 4, dsto += 1;
  } while (--n);
}

void Object_DrawNx3_BothBgs(int n, const uint16 *src, int dsto) {
  do {
    dung_bg1[dsto + XY(0, 0)] = dung_bg2[dsto + XY(0, 0)] = src[0];
    dung_bg1[dsto + XY(0, 1)] = dung_bg2[dsto + XY(0, 1)] = src[1];
    dung_bg1[dsto + XY(0, 2)] = dung_bg2[dsto + XY(0, 2)] = src[2];
    src += 3, dsto += 1;
  } while (--n);
}

void Object_LanternLayer_Helper(uint16 a, uint16 y) {
  const uint16 *src = SrcPtr(y);
  uint16 *dst = &dung_bg1[a / 2];
  for (int i = 0; i < 12; i++) {
    for (int j = 0; j < 12; j++)
      *dst++ = *src++;
    dst += 64 - 12;
  }
}

static const uint8 kWatergateLayout[17] = {
  0x1b, 0xa1, 0xc9,
  0x51, 0xa1, 0xc9,
  0x92, 0xa1, 0xc9,
  0xa1, 0x33, 0xc9,
  0xa1, 0x72, 0xc9,
  0xff, 0xff,
};

void Object_WatergateChannelWater() {
  dung_load_ptr_offs = 0;
  const uint8 *layoutsrc = kWatergateLayout;
  for (;;) {
    dung_draw_width_indicator = 0;
    dung_draw_height_indicator = 0;
    uint16 t = WORD(*layoutsrc);
    if (t == 0xffff)
      break;
    dung_draw_width_indicator = (t & 3) + 1;
    dung_draw_height_indicator = (t >> 8 & 3) + 1;
    dung_load_ptr_offs += 3, layoutsrc += 3;
    const uint16 *src = SrcPtr(0x110);
    int dsto2 = (t & 0xfc) >> 2 | (t >> 10) << 6;
    do {
      int dsto = dsto2;
      int n = dung_draw_width_indicator;
      do {
        int nn = 2;
        do {
          dung_bg1[dsto + XY(0, 0)] = src[0];
          dung_bg1[dsto + XY(1, 0)] = src[1];
          dung_bg1[dsto + XY(2, 0)] = src[2];
          dung_bg1[dsto + XY(3, 0)] = src[3];
          dung_bg1[dsto + XY(0, 1)] = src[4];
          dung_bg1[dsto + XY(1, 1)] = src[5];
          dung_bg1[dsto + XY(2, 1)] = src[6];
          dung_bg1[dsto + XY(3, 1)] = src[7];
          dsto += XY(0, 2);
        } while (--nn);
        dsto += XY(4, -4);
      } while (--n);
      dsto2 += XY(0, 4);
    } while (--dung_draw_height_indicator);
  }
}


void LoadType1ObjectSubtype2(uint8 idx, uint16 *dst, uint16 dsto) {
  uint16 params = kObjectSubtype2Params[idx];
  const uint16 *src = SrcPtr(params);
  int i;
  switch (idx) {
  case 0x00: case 0x01: case 0x02: case 0x03:
  case 0x04: case 0x05: case 0x06: case 0x07:  // 00 -  Wall Outer Corner (HIGH) [NW]
  case 0x1c: case 0x24: case 0x25: case 0x29:
    Object_Draw_Nx4(4, src, dst);
    break;
  case 0x08: case 0x09: case 0x0a: case 0x0b:
  case 0x0c: case 0x0d: case 0x0e: case 0x0f:  // 08 -  Wall Outer Corner (LOW) [NW]
    Object_DrawNx4_BothBgs(4, src, dsto);
    break;
  case 0x10: case 0x11: case 0x12: case 0x13:  // 10 -  Wall S-Bend (LOW) [N1]
    Object_DrawNx4_BothBgs(3, src, dsto);
    break;
  case 0x14: case 0x15: case 0x16: case 0x17:  // 14 -  Wall S-Bend (LOW) [W1]
    Object_DrawNx3_BothBgs(4, src, dsto);
    break;
  case 0x18: case 0x19: case 0x1a: case 0x1b:  // 18 -  Wall Pit Corner (Lower) [NW]
  case 0x27: case 0x2b: case 0x34:
    Object_Draw_2x2(src, dst);
    break;
  case 0x1d: case 0x21: case 0x26:             // 1D -  Statue
    Object_Draw_Nx3(2, src, dst);
    break;
  case 0x1e:                                   // 1E -  Star Tile Off
    Object_Draw_2x2(src, dst);
    break;
  case 0x1f:                                   // 1F -  Star Tile On
    i = dung_num_star_shaped_switches >> 1;
    dung_num_star_shaped_switches += 2;
    star_shaped_switches_tile[i] = (dsto | (dung_line_ptrs_row0 != 0x4000 ? 0 : 0x1000));
    Object_Draw_2x2(src, dst);
    break;
  case 0x20:                                   // 20 -  Torch Lit
    dung_num_lit_torches++;
    Object_Draw_2x2(src, dst);
    break;
  case 0x22: case 0x28:                        // 22 -  Weird Bed
    Object_Draw_5x4(src, dst);
    break;
  case 0x23:                                   // 23 -  Table
    Object_Draw_Nx3(4, src, dst);
    break;
  case 0x2a:                                   // 2A -  Wall Painting
    dung_draw_width_indicator = 1;
    Object_Draw_4x2_N(1, src, dst);
    break;
  case 0x2c:                                   // 2C -  ???
    Object_Draw_Nx3(6, src, dst);
    break;
  case 0x2d:    // 2D -  Floor Stairs Up (room)
    i = dung_num_inter_room_upnorth_stairs >> 1;
    dung_inter_starcases[i] = (dsto | (dung_line_ptrs_row0 != 0x4000 ? 0 : 0x1000));
    dung_num_inter_room_upnorth_stairs =
      dung_num_wall_upnorth_spiral_stairs =
      dung_num_wall_upnorth_spiral_stairs_2 =
      dung_num_inter_room_upnorth_straight_stairs =
      dung_num_inter_room_upsouth_straight_stairs =
      dung_num_inter_room_southdown_stairs =
      dung_num_wall_downnorth_spiral_stairs =
      dung_num_wall_downnorth_spiral_stairs_2 =
      dung_num_inter_room_downnorth_straight_stairs =
      dung_num_inter_room_downsouth_straight_stairs = dung_num_inter_room_upnorth_stairs + 2;
    Object_Draw4x4(SrcPtr(0x1088), dst);
    break;
  case 0x2e:    // 2E -  Floor Stairs Down (room)
  case 0x2f:    // 2F -  Floor Stairs Down2 (room)
    i = dung_num_inter_room_southdown_stairs >> 1;
    dung_inter_starcases[i] = (dsto | (dung_line_ptrs_row0 != 0x4000 ? 0 : 0x1000));
    dung_num_inter_room_southdown_stairs =
      dung_num_wall_downnorth_spiral_stairs =
      dung_num_wall_downnorth_spiral_stairs_2 =
      dung_num_inter_room_downnorth_straight_stairs =
      dung_num_inter_room_downsouth_straight_stairs = dung_num_inter_room_southdown_stairs + 2;
    Object_Draw4x4(SrcPtr(0x10A8), dst);
    break;
  case 0x30:     // 30 -  Stairs [N](unused)
    assert(0);
    break;
  case 0x31:     // 31 -  Stairs [N](layer)
    i = dung_num_inroom_southdown_stairs >> 1;
    dung_stairs_table_1[i] = dsto;
    dung_num_inroom_southdown_stairs =
      dung_num_water_ladders =
      dung_some_stairs_unk4 = dung_num_inroom_southdown_stairs + 2;
    Object_DrawNx4_BothBgs(4, src, dsto);
    break;
  case 0x32:     // 32 -  Stairs [N](layer)
non_submerged:
    i = dung_num_interpseudo_upnorth_stairs >> 1;
    dung_stairs_table_1[i] = dsto;
    dung_num_interpseudo_upnorth_stairs =
      dung_num_water_ladders =
      dung_some_stairs_unk4 = dung_num_interpseudo_upnorth_stairs + 2;
    Object_Draw4x4(src, dst);
    break;
  case 0x33:       // 33 -  Stairs Submerged [N](layer)
    if (dung_hdr_tag[1] == 27 && !(save_dung_info[dungeon_room_index] & 0x100)) {
      dung_hdr_bg2_properties = 0;
      src = SrcPtr(0x10C8);
      goto non_submerged;
    } else {
      i = dung_num_inroom_upnorth_stairs_water >> 1;
      dung_stairs_table_1[i] = dsto;
      dung_num_inroom_upnorth_stairs_water =
        dung_num_activated_water_ladders = dung_num_inroom_upnorth_stairs_water + 2;
      Object_Draw4x4(SrcPtr(0x10C8), dst);
    }
    break;
  case 0x35:  // 35 -  Water Ladder
    if (dung_hdr_tag[1] == 27 && !(save_dung_info[dungeon_room_index] & 0x100))
      goto inactive_water_ladder;
    dung_stairs_table_1[dung_num_activated_water_ladders >> 1] = dsto;
    dung_num_activated_water_ladders += 2;
    dung_draw_width_indicator = 1;
    Object_Draw_4x2_N(1, SrcPtr(0x1108), dst);
    break;
  case 0x36:  // 36 -  Water Ladder Inactive
inactive_water_ladder:
    dung_stairs_table_1[dung_num_water_ladders >> 1] = dsto;
    dung_some_stairs_unk4 = (dung_num_water_ladders += 2);
    Object_Draw_4x2_BothBgs(SrcPtr(0x1108), dsto);
    break;
  case 0x37:  // 37 -  Water Gate Large
    if (!(dung_savegame_state_bits & 0x800)) {
      Object_Draw_Nx4(10, src, dst);
      watergate_var1 = 0xf;
      watergate_pos = dsto * 2;
    } else {
      Object_Draw_Nx4(10, SrcPtr(0x13e8), dst);
      uint16 bak0 = dung_load_ptr;
      uint16 bak1 = dung_load_ptr_offs;
      uint8 bak2 = dung_load_ptr_bank;
      Object_WatergateChannelWater();
      dung_load_ptr_bank = bak2;
      dung_load_ptr_offs = bak1;
      dung_load_ptr = bak0;
    }
    break;
  case 0x38:  // 38 -  Door Staircase Up R
    i = dung_num_wall_upnorth_spiral_stairs >> 1;
    dung_inter_starcases[i] = (dsto - 0x40) | (dung_line_ptrs_row0 != 0x4000 ? 0 : 0x1000);
    dung_num_wall_upnorth_spiral_stairs =
      dung_num_wall_upnorth_spiral_stairs_2 =
      dung_num_inter_room_upnorth_straight_stairs =
      dung_num_inter_room_upsouth_straight_stairs =
      dung_num_inter_room_southdown_stairs =
      dung_num_wall_downnorth_spiral_stairs =
      dung_num_wall_downnorth_spiral_stairs_2 =
      dung_num_inter_room_downnorth_straight_stairs =
      dung_num_inter_room_downsouth_straight_stairs = dung_num_wall_upnorth_spiral_stairs + 2;
    Object_Draw_Nx3(4, SrcPtr(0x1148), dst);
    dung_bg2[dsto - 1] |= 0x2000;
    dung_bg2[dsto + 4] |= 0x2000;
    break;
  case 0x39:  // 39 -  Door Staircase Down L
    i = dung_num_wall_downnorth_spiral_stairs >> 1;
    dung_inter_starcases[i] = (dsto - 0x40) | (dung_line_ptrs_row0 != 0x4000 ? 0 : 0x1000);
    dung_num_wall_downnorth_spiral_stairs =
      dung_num_wall_downnorth_spiral_stairs_2 =
      dung_num_inter_room_downnorth_straight_stairs =
      dung_num_inter_room_downsouth_straight_stairs = dung_num_wall_downnorth_spiral_stairs + 2;
    Object_Draw_Nx3(4, SrcPtr(0x1160), dst);
    dung_bg2[dsto - 1] |= 0x2000;
    dung_bg2[dsto + 4] |= 0x2000;
    break;
  case 0x3a:  // 3A -  Door Staircase Up R (Lower)
    i = dung_num_wall_upnorth_spiral_stairs_2 >> 1;
    dung_inter_starcases[i] = (dsto - 0x40) | (dung_line_ptrs_row0 != 0x4000 ? 0 : 0x1000);
    dung_num_wall_upnorth_spiral_stairs_2 =
      dung_num_inter_room_upnorth_straight_stairs =
      dung_num_inter_room_upsouth_straight_stairs =
      dung_num_inter_room_southdown_stairs =
      dung_num_wall_downnorth_spiral_stairs =
      dung_num_wall_downnorth_spiral_stairs_2 =
      dung_num_inter_room_downnorth_straight_stairs =
      dung_num_inter_room_downsouth_straight_stairs = dung_num_wall_upnorth_spiral_stairs_2 + 2;
    Object_Draw_Nx3(4, SrcPtr(0x1178), dst);
    dung_bg1[dsto - 1] |= 0x2000;
    dung_bg1[dsto + 4] |= 0x2000;
    break;
  case 0x3b:  // 3B -  Door Staircase Down L (Lower)
    i = dung_num_wall_downnorth_spiral_stairs_2 >> 1;
    dung_inter_starcases[i] = (dsto - 0x40) | (dung_line_ptrs_row0 != 0x4000 ? 0 : 0x1000);
    dung_num_wall_downnorth_spiral_stairs_2 =
      dung_num_inter_room_downnorth_straight_stairs =
      dung_num_inter_room_downsouth_straight_stairs =
      dung_num_wall_downnorth_spiral_stairs_2 + 2;
    Object_Draw_Nx3(4, SrcPtr(0x1190), dst);
    dung_bg1[dsto - 1] |= 0x2000;
    dung_bg1[dsto + 4] |= 0x2000;
    break;
  case 0x3c:  // 3C -  Sanctuary Wall
    for (int i = 0; i < 6; i++) {
      dung_bg2[dsto + 0] = dung_bg2[dsto + 4] = dung_bg2[dsto + 8] =
        dung_bg2[dsto + 14] = dung_bg2[dsto + 18] = dung_bg2[dsto + 22] = src[0];
      dung_bg2[dsto + 1] = dung_bg2[dsto + 5] = dung_bg2[dsto + 9] =
        dung_bg2[dsto + 15] = dung_bg2[dsto + 19] = dung_bg2[dsto + 23] = src[0] | 0x4000;
      dung_bg2[dsto + 2] = dung_bg2[dsto + 6] = dung_bg2[dsto + 16] = dung_bg2[dsto + 20] = src[6];
      dung_bg2[dsto + 3] = dung_bg2[dsto + 7] = dung_bg2[dsto + 17] = dung_bg2[dsto + 21] = src[6] | 0x4000;
      dsto += XY(0, 1);
      src++;
    }
    Object_Draw_Nx3(4, src + 6, dst + 10);
    break;
  case 0x3e:  // 3E -  Church Pew
    Object_Draw_Nx3(6, src, dst);
    break;
  case 0x3f: { // 3F - used in hole at the smithy dwarves
    dsto |= (dung_line_ptrs_row0 != 0x4000 ? 0 : 0x1000);
    dst = &dung_bg2[dsto];
    for (int i = 0; i < 8; i++) {
      dst[XY(0, 0)] = src[0];
      dst[XY(0, 1)] = src[1];
      dst[XY(0, 2)] = src[2];
      dst[XY(0, 3)] = src[3];
      dst[XY(0, 4)] = src[4];
      dst[XY(0, 5)] = src[5];
      dst[XY(0, 6)] = src[6];
      dst += XY(1, 0);
      src += 7;
    }
    break;
  }

  default:
    assert(0);
  }
}

void Object_Draw_4xN(int n, const uint16 *src, uint16 *dst) {
  do {
    dst[0] = src[0];
    dst[1] = src[1];
    dst[2] = src[2];
    dst[3] = src[3];
    src += 4;
    dst += XY(0, 1);
  } while (--n);
}

void Object_AgahnimRoomFrame(uint16 dsto) {
  const uint16 *src;

  uint16 *d = &dung_bg2[dsto];
  src = SrcPtr(0x1BF2);
  for (int i = 0; i < 6; i++) {
    d[XY(7, 4)] = d[XY(13, 4)] = d[XY(19, 4)] = src[0];
    d[XY(7, 5)] = d[XY(13, 5)] = d[XY(19, 5)] = src[1];
    d[XY(7, 6)] = d[XY(13, 6)] = d[XY(19, 6)] = src[2];
    d[XY(7, 7)] = d[XY(13, 7)] = d[XY(19, 7)] = src[3];
    src += 4, d += XY(1, 0);
  }
  d -= 6;

  src = SrcPtr(0x1c22);
  for (int i = 0; i < 5; i++) {
    int j = src[0];
    d[XY(2, 10)] = d[XY(3, 9)] = d[XY(4, 8)] = d[XY(5, 7)] = d[XY(6, 6)] = d[XY(7, 5)] = d[XY(8, 4)] = j;
    d[XY(23, 4)] = d[XY(24, 5)] = d[XY(25, 6)] = d[XY(26, 7)] = d[XY(27, 8)] = d[XY(28, 9)] = d[XY(29, 10)] = j | 0x4000;
    src++, d += XY(0, 1);
  }
  d -= XY(0, 1) * 5;

  src = SrcPtr(0x1c2c);
  for (int i = 0; i < 6; i++) {
    int j = src[0];
    d[XY(2, 11)] = d[XY(2, 17)] = d[XY(2, 23)] = j;
    d[XY(29, 11)] = d[XY(29, 17)] = d[XY(29, 23)] = j | 0x4000;
    j = src[1];
    d[XY(3, 11)] = d[XY(3, 17)] = d[XY(3, 23)] = j;
    d[XY(28, 11)] = d[XY(28, 17)] = d[XY(28, 23)] = j | 0x4000;
    j = src[2];
    d[XY(4, 11)] = d[XY(4, 17)] = d[XY(4, 23)] = j;
    d[XY(27, 11)] = d[XY(27, 17)] = d[XY(27, 23)] = j | 0x4000;
    j = src[3];
    d[XY(5, 11)] = d[XY(5, 17)] = d[XY(5, 23)] = j;
    d[XY(26, 11)] = d[XY(26, 17)] = d[XY(26, 23)] = j | 0x4000;
    src += 4, d += XY(0, 1);
  }
  d -= XY(0, 1) * 6;

  src = SrcPtr(0x1c5c);
  for (int i = 0; i < 6; i++) {
    d[XY(12, 9)] = d[XY(18, 9)] = src[0];
    d[XY(12, 10)] = d[XY(18, 10)] = src[6];
    src += 1, d += XY(1, 0);
  }
  d -= XY(1, 0) * 6;

  src = SrcPtr(0x1c74);
  for (int i = 0; i < 6; i++) {
    d[XY(7, 14)] = d[XY(7, 20)] = src[0];
    d[XY(8, 14)] = d[XY(8, 20)] = src[1];
    src += 2, d += XY(0, 1);
  }
  d -= XY(0, 1) * 6;

  src = SrcPtr(0x1c8c);
  for (int i = 0; i < 5; i++) {
    d[XY(7, 9)] = src[0];
    d[XY(7, 10)] = src[1];
    d[XY(7, 11)] = src[2];
    d[XY(7, 12)] = src[3];
    d[XY(7, 13)] = src[4];
    src += 5, d += XY(1, 0);
  }
  d -= XY(1, 0) * 5;

  for (int i = 0; i < 4; i++) {
    d[XY(14, 28)] |= 0x2000;
    d[XY(14, 29)] |= 0x2000;
    d += XY(1, 0);
  }
}


void Object_BombableFloorHelper(uint16 a, const uint16 *src, const uint16 *src_below, uint16 *dst, uint16 dsto) {
  int i = dung_misc_objs_index >> 1;
  dung_replacement_tile_state[i] = a;
  dung_misc_objs_index += 2;
  dung_object_pos_in_objdata[i] = dung_load_ptr_offs;
  dung_object_tilemap_pos[i] = dsto * 2 | (dung_line_ptrs_row0 != 0x4000 ? 0 : 0x2000);
  replacement_tilemap_UL[i] = src_below[0];
  replacement_tilemap_LL[i] = src_below[1];
  replacement_tilemap_UR[i] = src_below[2];
  replacement_tilemap_LR[i] = src_below[3];
  Object_Draw_2x2(src, dst);
}


void Object_BombableFloor(const uint16 *src, uint16 *dst, uint16 dsto) {
  if (dungeon_room_index == 101 && (dung_savegame_state_bits & 0x1000)) {
    dung_draw_width_indicator = 0;
    dung_draw_height_indicator = 0;
    Object_Hole(SrcPtr(0x5aa), dst);
    return;
  }

  src = SrcPtr(0x220);
  const uint16 *src_below = SrcPtr(0x5ba);

  Object_BombableFloorHelper(0x3030, src, src_below, dst, dsto);
  Object_BombableFloorHelper(0x3131, src + 4, src_below + 4, dst + XY(2, 0), dsto + XY(2, 0));
  Object_BombableFloorHelper(0x3232, src + 8, src_below + 8, dst + XY(0, 2), dsto + XY(0, 2));
  Object_BombableFloorHelper(0x3333, src + 12, src_below + 12, dst + XY(2, 2), dsto + XY(2, 2));
}


void Object_ReplacementTileHelper(uint16 a, const uint16 *src, uint16 *dst, uint16 dsto) {
  int i = dung_misc_objs_index >> 1;
  dung_replacement_tile_state[i] = a;
  dung_misc_objs_index += 2;
  dung_object_pos_in_objdata[i] = dung_load_ptr_offs;
  dung_object_tilemap_pos[i] = dsto * 2 | (dung_line_ptrs_row0 != 0x4000 ? 0 : 0x2000);
  replacement_tilemap_UL[i] = dst[XY(0, 0)];
  replacement_tilemap_LL[i] = dst[XY(0, 1)];
  replacement_tilemap_UR[i] = dst[XY(1, 0)];
  replacement_tilemap_LR[i] = dst[XY(1, 1)];
  Object_Draw_2x2(src, dst);
}

void Object_FortuneTellerTemplate(uint16 dsto) {
  const uint16 *src = SrcPtr(0x202e), *src_org = src;
  uint16 *d = &dung_bg2[dsto];
  int j;

  for (int i = 0; i < 6; i++) {
    d[XY(1, 0)] =
      d[XY(2, 0)] =
      d[XY(1, 1)] =
      d[XY(2, 1)] = src[0];
    d[XY(1, 2)] = (j = src[1]);
    d[XY(2, 2)] = j | 0x4000;
    d += XY(2, 0);
  }
  d -= XY(2, 0) * 6;

  for (int i = 0; i < 3; i++) {
    d[XY(0, 3)] = d[XY(2, 3)] = d[XY(10, 3)] = d[XY(12, 3)] = (j = src[2]);
    d[XY(1, 3)] = d[XY(3, 3)] = d[XY(11, 3)] = d[XY(13, 3)] = j | 0x4000;
    d[XY(4, 3)] = d[XY(6, 3)] = d[XY(8, 3)] = (j = src[5]);
    d[XY(5, 3)] = d[XY(7, 3)] = d[XY(9, 3)] = j | 0x4000;
    src++, d += XY(0, 1);
  }
  d -= XY(0, 1) * 3;

  d[XY(0, 0)] = d[XY(0, 1)] = (j = src[5]);
  d[XY(13, 0)] = d[XY(13, 1)] = j | 0x4000;
  d[XY(0, 2)] = (j = src[6]);
  d[XY(13, 2)] = j | 0x4000;

  src = src_org;
  for (int i = 0; i < 4; i++) {
    d[XY(3, 10)] = (j = src[10]);
    d[XY(10, 10)] = j ^ 0x4000;
    d[XY(4, 10)] = (j = src[14]);
    d[XY(9, 10)] = j ^ 0x4000;
    d[XY(5, 10)] = (j = src[18]);
    d[XY(8, 10)] = j ^ 0x4000;
    d[XY(6, 10)] = (j = src[22]);
    d[XY(7, 10)] = j ^ 0x4000;
    src++, d += XY(0, 1);
  }
}

void Object_Draw8x8(const uint16 *src, uint16 *dst) {
  Object_Draw4x4(src, dst + XY(0, 0));
  Object_Draw4x4(src + 16, dst + XY(4, 0));
  Object_Draw4x4(src + 32, dst + XY(0, 4));
  Object_Draw4x4(src + 48, dst + XY(4, 4));
}

void Object_Draw8xN_BG2(int n, const uint16 *src, uint16 dsto) {
  uint16 *dst = &dung_bg2[dsto | (dung_line_ptrs_row0 != 0x4000 ? 0 : 0x1000)];
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < n; j++)
      dst[j] = src[j];
    dst += 64, src += n;
  }
}

static const uint16 kChestOpenMasks[] = { 0x100, 0x200, 0x400, 0x800, 0x1000, 0x2000 };

void LoadType1ObjectSubtype3(uint8 idx, uint16 *dst, uint16 dsto) {
  uint16 params = kObjectSubtype3Params[idx];
  const uint16 *src = SrcPtr(params);
  int i;

  switch (idx) {
  case 0x00:  // 00 -  Water Face Closed
    if (dung_hdr_tag[1] == 27) {
      if (save_dung_info[dungeon_room_index] & 0x100)
        goto water_face_open;
    } else if (dung_hdr_tag[1] == 25) {
      if (dung_savegame_state_bits & 0x800)
        goto water_face_open;
    }
    word_7E047C = dsto * 2;
    Object_Draw_4xN(3, src, dst);
    break;
  case 0x01:  // 01 -  Waterfall Face
water_face_open:
    Object_Draw_4xN(5, SrcPtr(0x162c), dst);
    break;
  case 0x02:  // 02 -  Waterfall Face Longer
    Object_Draw_4xN(7, src, dst);
    break;
  case 0x03: case 0x0e:  // 03 -  Cane Ride Spawn [?]Block
    dung_unk6++;
    dst[0] = src[0];
    break;
  case 0x04: case 0x05: case 0x06: case 0x07:  // 04 -  Cane Ride Node [4-way]
  case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0f:
    dst[0] = src[0];
    break;
  case 0x0d: case 0x17: { // 0D -  Prison Cell
    src = SrcPtr(0x1488);
    dsto |= (dung_line_ptrs_row0 != 0x4000 ? 0 : 0x1000);
    uint16 *d = &dung_bg2[dsto], *dd = d;
    for (int i = 0; i < 5; i++, d++) {
      d[XY(2, 0)] = d[XY(9, 0)] = src[1];
      d[XY(2, 1)] = src[2];
      d[XY(9, 1)] = src[2] | 0x4000;
      d[XY(2, 2)] = src[4];
      d[XY(9, 2)] = src[4] | 0x4000;
      d[XY(2, 3)] = src[5];
      d[XY(9, 3)] = src[5] | 0x4000;
    }
    dd[XY(0, 0)] = src[0];
    dd[XY(15, 0)] = src[0] | 0x4000;
    dd[XY(1, 0)] = dd[XY(7, 0)] = dd[XY(8, 0)] = dd[XY(14, 0)] = src[1];
    dd[XY(1, 2)] = src[3];
    dd[XY(14, 2)] = src[3] | 0x4000;
    break;
  }
  case 0x10: case 0x11: case 0x13: case 0x1a: case 0x22: case 0x23:
  case 0x24: case 0x25:
  case 0x3e: case 0x3f: case 0x40: case 0x41: case 0x42:
  case 0x43: case 0x44: case 0x45: case 0x46: case 0x49: case 0x4a: case 0x4f:
  case 0x50: case 0x51: case 0x52: case 0x53: case 0x56: case 0x57: case 0x58: case 0x59:
  case 0x5e: case 0x5f: case 0x63: case 0x64: case 0x65:
  case 0x75: case 0x7c: case 0x7d: case 0x7e:
    Object_Draw_2x2(src, dst);
    break;

  case 0x12:  // 12 -  Rupee Floor
    if (dung_savegame_state_bits & 0x1000)
      return;
    src = SrcPtr(0x1dd6);
    dst = &dung_bg2[dsto | (dung_line_ptrs_row0 != 0x4000 ? 0 : 0x1000)];
    for (int i = 0; i < 3; i++) {
      dst[XY(0, 0)] = dst[XY(0, 3)] = dst[XY(0, 6)] = src[0];
      dst[XY(0, 1)] = dst[XY(0, 4)] = dst[XY(0, 7)] = src[1];
      dst += 2;
    }
    break;
  case 0x14: case 0x4E: // 14 -  Down Warp Door
  case 0x67: case 0x68: case 0x6c: case 0x6d: case 0x79:
    Object_Draw_Nx3(4, src, dst);
    break;
  case 0x15:  // 15 -  Kholdstare Shell - BG2
    if (dung_savegame_state_bits & 0x8000)
      return;
    src = SrcPtr(0x1dfa);
    Object_Draw8xN_BG2(10, src, dsto);
    break;
  case 0x16:   // 16 -  Single Hammer Peg
    Object_PegBlock(src, dst, dsto);
    break;
  case 0x18:  // 18 -  Cell Lock
    i = dung_num_bigkey_locks_x2 >> 1;
    dung_num_bigkey_locks_x2 += 2;
    if (!(dung_savegame_state_bits & kChestOpenMasks[i])) {
      dung_chest_locations[i] = dsto * 2;
      Object_Draw_2x2(SrcPtr(0x1494), dst);
    } else {
      dung_chest_locations[i] = 0;
    }
    break;
  case 0x19: {  // 19 -  Chest
    if (main_module_index == 26)
      return;
    i = dung_num_chests_x2 >> 1;
    dung_num_bigkey_locks_x2 = (dung_num_chests_x2 += 2);

    int h = -1;
    if (dung_hdr_tag[0] == 0x27 || dung_hdr_tag[0] == 0x3c ||
        dung_hdr_tag[0] == 0x3e || dung_hdr_tag[0] >= 0x29 && dung_hdr_tag[0] < 0x33)
      h = 0;
    else if (dung_hdr_tag[1] == 0x27 || dung_hdr_tag[1] == 0x3c ||
             dung_hdr_tag[1] == 0x3e || dung_hdr_tag[1] >= 0x29 && dung_hdr_tag[1] < 0x33)
      h = 1;

    dung_chest_locations[i] = 2 * (dsto | (dung_line_ptrs_row0 != 0x4000 ? 0 : 0x1000));
    if (!(dung_savegame_state_bits & kChestOpenMasks[i])) {
      if (h >= 0) {
        if (!(dung_savegame_state_bits & kChestOpenMasks[h]))
          return;
        dung_hdr_tag[h] = 0;
      }
      Object_Draw_2x2(SrcPtr(0x149c), dst);
    } else {
      dung_chest_locations[i] = 0;
      if (h >= 0)
        dung_hdr_tag[h] = 0;
      Object_Draw_2x2(SrcPtr(0x14a4), dst);
    }
    break;
  }
  case 0x1b:  // 1B -  Stair
    dung_stairs_table_1[dung_num_stairs_1 >> 1] = dsto;
    dung_num_stairs_1 += 2;
stair1b:
    for (int i = 0; i < 4; i++) {
      dung_bg1[dsto + XY(0, 0)] = dung_bg2[dsto + XY(0, 0)] = src[0];
      dung_bg1[dsto + XY(0, 1)] = dung_bg2[dsto + XY(0, 1)] = src[1];
      dung_bg1[dsto + XY(0, 2)] = dung_bg2[dsto + XY(0, 2)] = src[2];
      dung_bg1[dsto + XY(0, 3)] = dung_bg2[dsto + XY(0, 3)] = src[3];
      src += 4, dsto += 1;
    }
    break;
  case 0x1c:  // 1C -  Stair [S](Layer)
    dung_stairs_table_2[dung_num_stairs_2 >> 1] = dsto;
    dung_num_stairs_2 += 2;
    goto stair1b;
  case 0x1d:  //  1D -  Stair Wet [S](Layer)
stairs_wet:
    dung_stairs_table_2[dung_num_stairs_wet >> 1] = dsto;
    dung_num_stairs_wet += 2;
    Object_Draw4x4(src, dst);
    break;
  case 0x1e:  // 1E -  Staircase going Up(Up)
    dung_inter_starcases[dung_num_inter_room_upnorth_straight_stairs >> 1] = dsto;
    dung_num_inter_room_upnorth_straight_stairs =
      dung_num_inter_room_upsouth_straight_stairs =
      dung_num_inter_room_southdown_stairs =
      dung_num_wall_downnorth_spiral_stairs =
      dung_num_wall_downnorth_spiral_stairs_2 =
      dung_num_inter_room_downnorth_straight_stairs =
      dung_num_inter_room_downsouth_straight_stairs = dung_num_inter_room_upnorth_straight_stairs + 2;
    Object_Draw_Nx4(4, src, dst);
    break;
  case 0x1f: // 1F -  Staircase Going Down (Up)
    dung_inter_starcases[dung_num_inter_room_downnorth_straight_stairs >> 1] = dsto;
    dung_num_inter_room_downnorth_straight_stairs =
      dung_num_inter_room_downsouth_straight_stairs = dung_num_inter_room_downnorth_straight_stairs + 2;
    Object_Draw_Nx4(4, src, dst);
    break;
  case 0x20:  // 20 -  Staircase Going Up (Down)
    dung_inter_starcases[dung_num_inter_room_upsouth_straight_stairs >> 1] = dsto;
    dung_num_inter_room_upsouth_straight_stairs =
      dung_num_inter_room_southdown_stairs =
      dung_num_wall_downnorth_spiral_stairs =
      dung_num_wall_downnorth_spiral_stairs_2 =
      dung_num_inter_room_downnorth_straight_stairs =
      dung_num_inter_room_downsouth_straight_stairs = dung_num_inter_room_upsouth_straight_stairs + 2;
    Object_Draw_Nx4(4, src, dst);
    break;
  case 0x21:  // 21 -  Staircase Going Down (Down)
    dung_inter_starcases[dung_num_inter_room_downsouth_straight_stairs >> 1] = dsto;
    dung_num_inter_room_downsouth_straight_stairs = dung_num_inter_room_downsouth_straight_stairs + 2;
    Object_Draw_Nx4(4, src, dst);
    break;
  case 0x26:  // 26 -  Staircase Going Up (Lower)
    i = dung_num_inter_room_upnorth_straight_stairs >> 1;
    dung_inter_starcases[i] = dsto | (dung_line_ptrs_row0 != 0x4000 ? 0 : 0x1000);
    dung_num_inter_room_upnorth_straight_stairs =
      dung_num_inter_room_upsouth_straight_stairs =
      dung_num_inter_room_southdown_stairs =
      dung_num_wall_downnorth_spiral_stairs =
      dung_num_wall_downnorth_spiral_stairs_2 =
      dung_num_inter_room_downnorth_straight_stairs =
      dung_num_inter_room_downsouth_straight_stairs = dung_num_inter_room_upnorth_straight_stairs + 2;
door26:
    for (int i = 0; i < 4; i++) {
      dung_bg1[dsto] = dung_bg2[dsto] = src[0];
      dung_bg1[dsto + XY(0, 1)] = src[1];
      dung_bg1[dsto + XY(0, 2)] = src[2];
      dung_bg1[dsto + XY(0, 3)] = src[3];
      src += 4, dsto++;
    }
    dsto += XY(-4, -4);
copy_door_bg2:
    dung_bg2[dsto + XY(0, 0)] |= 0x2000;
    dung_bg2[dsto + XY(0, 1)] |= 0x2000;
    dung_bg2[dsto + XY(0, 2)] |= 0x2000;
    dung_bg2[dsto + XY(0, 3)] |= 0x2000;
    break;
  case 0x27:  // 27 -  Staircase Going Up (Lower)
    i = dung_num_inter_room_downnorth_straight_stairs >> 1;
    dung_inter_starcases[i] = dsto | (dung_line_ptrs_row0 != 0x4000 ? 0 : 0x1000);
    dung_num_inter_room_downnorth_straight_stairs =
      dung_num_inter_room_downsouth_straight_stairs =
      dung_num_inter_room_downnorth_straight_stairs + 2;
    goto door26;
  case 0x28:  // 28 -  Staircase Going Down (Lower)
    i = dung_num_inter_room_upsouth_straight_stairs >> 1;
    dung_inter_starcases[i] = dsto | (dung_line_ptrs_row0 != 0x4000 ? 0 : 0x1000);
    dung_num_inter_room_upsouth_straight_stairs =
      dung_num_inter_room_southdown_stairs =
      dung_num_wall_downnorth_spiral_stairs =
      dung_num_wall_downnorth_spiral_stairs_2 =
      dung_num_inter_room_downnorth_straight_stairs =
      dung_num_inter_room_downsouth_straight_stairs =
      dung_num_inter_room_upsouth_straight_stairs + 2;
door28:
    for (int i = 0; i < 4; i++) {
      dung_bg1[dsto + XY(0, 0)] = src[0];
      dung_bg1[dsto + XY(0, 1)] = src[1];
      dung_bg1[dsto + XY(0, 2)] = src[2];
      dung_bg1[dsto + XY(0, 3)] = dung_bg2[dsto + XY(0, 3)] = src[3];
      src += 4, dsto++;
    }
    dsto += XY(-4, 4);
    goto copy_door_bg2;
  case 0x29:  // 29 -  Staircase Going Down (Lower)
    i = dung_num_inter_room_downsouth_straight_stairs >> 1;
    dung_inter_starcases[i] = dsto | (dung_line_ptrs_row0 != 0x4000 ? 0 : 0x1000);
    dung_num_inter_room_downsouth_straight_stairs =
      dung_num_inter_room_downsouth_straight_stairs + 2;
    goto door28;
  case 0x2a:  // 2A -  Dark Room BG2 Mask
    Object_LanternLayer_Helper(0x514, 0x16dc);
    Object_LanternLayer_Helper(0x554, 0x17f6);
    Object_LanternLayer_Helper(0x1514, 0x1914);
    Object_LanternLayer_Helper(0x1554, 0x1a2a);
    break;
  case 0x2b:  // 2B -  Staircase Going Down (Lower) not really
    Object_ReplacementTileHelper(0x1010, src, dst, dsto);
    break;
  case 0x2c: // 2C -  Large Pick Up Block
    Object_ReplacementTileHelper(0x2020, SrcPtr(0xe62), dst, dsto);
    Object_ReplacementTileHelper(0x2121, SrcPtr(0xe6a), dst + XY(2, 0), dsto + XY(2, 0));
    Object_ReplacementTileHelper(0x2222, SrcPtr(0xe72), dst + XY(0, 2), dsto + XY(0, 2));
    Object_ReplacementTileHelper(0x2323, SrcPtr(0xe7a), dst + XY(2, 2), dsto + XY(2, 2));
    break;
  case 0x2d: { // 2D -  Agahnim Altar
    src = SrcPtr(0x1b4a);
    uint16 *d = &dung_bg2[dsto];
    for (int j = 0; j < 14; j++) {
      i = src[0], d[0] = i, d[13] = i | 0x4000;
      i = src[14], d[1] = d[2] = i, d[11] = d[12] = i ^ 0x4000;
      i = src[28], d[3] = i, d[10] = i ^ 0x4000;
      i = src[42], d[4] = i, d[9] = i ^ 0x4000;
      i = src[56], d[5] = i, d[8] = i ^ 0x4000;
      i = src[70], d[6] = i, d[7] = i ^ 0x4000;
      src++, d += 64;
    }
    break;
  }
  case 0x2e:  // 2E -  Agahnim Room
    Object_AgahnimRoomFrame(dsto);
    break;
  case 0x2f:  // 2F -  Pot
    Object_Pot(src, dst, dsto);
    break;
  case 0x30: // 30 -  ??
    Object_ReplacementTileHelper(0x1212, src, dst, dsto);
    break;
  case 0x31: // 31 -  Big Chest
    i = dung_num_chests_x2;
    dung_chest_locations[i >> 1] = dsto * 2 | 0x8000 | (dung_line_ptrs_row0 != 0x4000 ? 0 : 0x2000);
    if (dung_savegame_state_bits & kChestOpenMasks[i >> 1]) {
      dung_chest_locations[i >> 1] = 0;
      dung_num_chests_x2 = dung_num_bigkey_locks_x2 = i + 2;
      Object_Draw_Nx3(4, SrcPtr(0x14c4), dst);
    } else {
      dung_num_chests_x2 = dung_num_bigkey_locks_x2 = i + 2;
      Object_Draw_Nx3(4, SrcPtr(0x14ac), dst);
    }
    break;
  case 0x32:  // 32 -  Big Chest Open
    Object_Draw_Nx3(4, src, dst);
    break;
  case 0x33:  // 33 -  Stairs Submerged [S](layer)
    if (dung_hdr_tag[1] == 27) {
      if (!(save_dung_info[dungeon_room_index] & 0x100)) {
        dung_hdr_bg2_properties = 0;
        goto stairs_wet;
      }
      WORD(CGWSEL_copy) = 0x6202;
    }
    dung_stairs_table_2[dung_num_inroom_upsouth_stairs_water >> 1] = dsto;
    dung_num_inroom_upsouth_stairs_water += 2;
    Object_Draw4x4(src, dst);
    break;
  case 0x34: case 0x35: case 0x36: case 0x37: case 0x38: case 0x39:
    assert(0);
    break;
  case 0x3a: case 0x3b: // 3A -  Pipe Ride Mouth [S]
    Object_Draw_Nx3(4, src, dst);
    Object_Draw_Nx3(4, src + 12, dst + XY(0, 3));
    break;
  case 0x3c: case 0x3d: case 0x5c:  // 3C -  Pipe Ride Mouth [E]
    Object_Draw_Nx4(6, src, dst);
    break;
  case 0x47:  // 47 -  Bomb Floor
    Object_BombableFloor(src, dst, dsto);
    break;
  case 0x48: case 0x66: case 0x6b: case 0x7a:  // 48 -  Fake Bomb Floor
    Object_Draw4x4(src, dst);
    break;
  case 0x4b: case 0x76: case 0x77:
    Object_Draw_Nx3(8, src, dst);
    break;
  case 0x4c:
    Object_Draw8xN_BG2(6, SrcPtr(0x1f92), dsto);
    break;
  case 0x4d: case 0x5d:  // 5D -  Forge
    Object_Draw_Nx3(6, src, dst);
    break;
  case 0x54:
    Object_FortuneTellerTemplate(dsto);
    break;
  case 0x55:
  case 0x5b:  // 5B -  Water Troof
    dst[XY(0, 0)] = src[0];
    dst[XY(1, 0)] = src[1];
    dst[XY(2, 0)] = src[2];
    for (int i = 0; i < 3; i++) {
      dst[XY(0, 1)] = src[3];
      dst[XY(1, 1)] = src[4];
      dst[XY(2, 1)] = src[5];
      dst += XY(0, 1);
    }
    dst[XY(0, 1)] = src[6];
    dst[XY(1, 1)] = src[7];
    dst[XY(2, 1)] = src[8];
    break;

  case 0x5a:  // 5A -  Plate on Table
    Object_Draw_4xN(2, src, dst);
    break;

  case 0x60: case 0x61: // 60 -  Left/Right Warp Door
    Object_Draw_Nx3(3, src, dst);
    Object_Draw_Nx3(3, src + 9, dst + XY(0, 3));
    break;

  case 0x62: {  // 62 ??
    src = SrcPtr(0x20f6);
    uint16 *d = &dung_bg1[dsto];
    for (int i = 0; i < 22; i++) {
      d[XY(0, 0)] = src[0];
      d[XY(0, 1)] = src[1];
      d[XY(0, 2)] = src[2];
      d[XY(0, 3)] = src[3];
      d[XY(0, 4)] = src[4];
      d[XY(0, 5)] = src[5];
      d[XY(0, 6)] = src[6];
      d[XY(0, 7)] = src[7];
      d[XY(0, 8)] = src[8];
      d[XY(0, 9)] = src[9];
      d[XY(0, 10)] = src[10];
      d += XY(1, 0), src += 11;
    }
    d -= XY(1, 0) * 22;
    src = SrcPtr(0x22da);
    for (int i = 0; i < 3; i++) {
      d[XY(9, 11)] = src[0];
      d[XY(9, 12)] = src[3];
      d += XY(1, 0), src += 1;
    }
    break;
  }
  case 0x69: case 0x6a: case 0x6e: case 0x6f:  // 69 -  Left Crack Wall
    Object_Draw_Nx4(3, src, dst);
    break;

  case 0x70:  // 70 -  Window Light
    Object_Draw4x4(src, dst + XY(0, 0));
    Object_Draw4x4(SrcPtr(0x2376), dst + XY(0, 2));
    Object_Draw4x4(SrcPtr(0x2396), dst + XY(0, 6));
    break;

  case 0x71:  // 71 -  Floor Light Blind BG2
    if (!(save_dung_info[101] & 0x100))
      return;
    Object_Draw8x8(src, dst);
    break;
  case 0x72:  // 72 -  TrinexxShell  Boss Goo/Shell BG2
    if (dung_savegame_state_bits & 0x8000)
      return;
    Object_Draw8xN_BG2(10, src, dsto);
    break;
  case 0x73:  // 73 -  Entire floor is pit, Bg2 Full Mask
    Dung_FillFloor(SrcPtr(0xe0));
    break;
  case 0x74:  // 74 -  Boss Entrance
    Object_Draw8x8(src, dst);
    break;

  case 0x78:  // Triforce
    Object_Draw4x4(src, dst);
    Object_Draw4x4(src + 16, dst + XY(-2, 4));
    Object_Draw4x4(src + 16, dst + XY(2, 4));
    break;
  case 0x7b: // 7B -  Vitreous Boss?
    Object_Draw_N_4x4(5, src, dst);
    Object_Draw_N_4x4(5, src, dst + XY(0, 4));
    break;
  default:
    assert(0);
  }
}

void Dungeon_LoadType1Object(uint16 r0, const uint8 *level_data) {
  uint16 offs = dung_load_ptr_offs;
  uint8 idx = level_data[offs + 2];
  dung_load_ptr_offs = offs + 3;

  if ((r0 & 0xfc) != 0xfc) {
    dung_draw_width_indicator = (r0 & 3);
    dung_draw_height_indicator = (r0 >> 8) & 3;
    uint8 x = (uint8)r0 >> 2;
    uint8 y = (r0 >> 10);
    uint16 dst = y * 64 + x;
    if (idx < 0xf8) {
      LoadType1ObjectSubtype1(idx, DstoPtr(dst), dst);
    } else {
      idx = (idx & 7) << 4 | ((r0 >> 8) & 3) << 2 | (r0 & 3);
      LoadType1ObjectSubtype3(idx, DstoPtr(dst), dst);
    }
  } else {
    uint8 x = (r0 & 3) << 4 | (r0 >> 12) & 0xf;
    uint8 y = ((r0 >> 8) & 0xf) << 2 | (idx >> 6);
    uint16 dst = y * 64 + x;
    LoadType1ObjectSubtype2(idx & 0x3f, DstoPtr(dst), dst);
  }
}

void RoomBounds_AddA(RoomBounds *r) {
  r->a0 += 0x100;
  r->a1 += 0x100;
}
void RoomBounds_AddB(RoomBounds *r) {
  r->b0 += 0x200;
  r->b1 += 0x200;
}

void RoomBounds_SubB(RoomBounds *r) {
  r->b0 -= 0x200;
  r->b1 -= 0x200;
}

void RoomBounds_SubA(RoomBounds *r) {
  r->a0 -= 0x100;
  r->a1 -= 0x100;
}

void Player_CrossQuadrantBoundary_Left() {
  link_quadrant_x ^= 1;
  UpdateCompositeOfLayoutAndQuadrant();
  RoomBounds_SubA(&room_bounds_x);
  Player_UpdateQuadrantsVisited();
}

void Player_CrossQuadrantBoundary_Right() {
  link_quadrant_x ^= 1;
  UpdateCompositeOfLayoutAndQuadrant();
  RoomBounds_AddA(&room_bounds_x);
  Player_UpdateQuadrantsVisited();
}

void Player_CrossQuadrantBoundary_Up() {
  link_quadrant_y ^= 2;
  UpdateCompositeOfLayoutAndQuadrant();
  RoomBounds_SubA(&room_bounds_y);
  Player_UpdateQuadrantsVisited();
}

void Player_CrossQuadrantBoundary_Down() {
  link_quadrant_y ^= 2;
  UpdateCompositeOfLayoutAndQuadrant();
  RoomBounds_AddA(&room_bounds_y);
  Player_UpdateQuadrantsVisited();
}

void Dung_CopyInUpDownScrollTarget(uint8 arg) {
  static const uint16 kUpDownScroll[4] = { 0, 272, 256, 16 };
  up_down_scroll_target = kUpDownScroll[arg];
  up_down_scroll_target_end = kUpDownScroll[arg + 1];
}

void Dung_CopyInLeftRightScrollTarget(uint8 arg) {
  static const uint16 kUpDownScroll[4] = { 0, 256, 256, 0 };
  left_right_scroll_target = kUpDownScroll[arg * 2];
  left_right_scroll_target_end = kUpDownScroll[arg * 2 + 1];
}

void Dung_UpdateCameraScrollBounds(uint8 arg) {
  static const uint16 kCameraBoundsX[] = { 127, 383, 127, 383 };
  static const uint16 kCameraBoundsY[] = { 120, 376, 136, 392 };
  overworld_screen_transition = arg;
  if (link_direction & 3) {
    uint8 t = link_direction & 1 ? 0 : 2;
    if (link_quadrant_x) t += 1;
    camera_x_coord_scroll_low = kCameraBoundsX[t];
    camera_x_coord_scroll_hi = camera_x_coord_scroll_low + 2;
  } else {
    uint8 t = link_direction & 4 ? 0 : 2;
    if (link_quadrant_y) t += 1;
    camera_y_coord_scroll_low = kCameraBoundsY[t];
    camera_y_coord_scroll_hi = camera_y_coord_scroll_low + 2;
  }
}


void Dungeon_ResetTorchBackgroundAndPlayerInner() {
  Ancilla_TerminateSelectInteractives(0);
  if (link_is_running) {
    link_auxiliary_state = 0;
    link_incapacitated_timer = 0;
    link_actual_vel_z = 0xff;
    g_ram[0xc7] = 0xff;
    link_delay_timer_spin_attack = 0;
    link_speed_setting = 0;
    swimcoll_var5[0] &= ~0xff;
    link_is_running = 0;
    link_player_handler_state = 0;
  }
}

void Dung_HandleExitToOverworld() {
  Dungeon_SaveRoomData_justKeys();
  SaveQuadrantsToSram();
  saved_module_for_menu = 8;
  main_module_index = 15;
  submodule_index = 0;
  subsubmodule_index = 0;
  Dungeon_ResetTorchBackgroundAndPlayerInner();
}
void Dung_AdjustCoordsForNewRoom() {
  int xd = ((dungeon_room_index & 0xf) - (dungeon_room_index_prev & 0xf)) * 0x200;
  link_x_coord += xd;
  BG2HOFS_copy2 += xd;
  room_bounds_x.a1 += xd;
  room_bounds_x.b1 += xd;
  room_bounds_x.a0 += xd;
  room_bounds_x.b0 += xd;

  int yd = (((dungeon_room_index & 0xf0) >> 4) - ((dungeon_room_index_prev & 0xf0) >> 4)) * 0x200;
  link_y_coord += yd;
  BG2VOFS_copy2 += yd;
  room_bounds_y.a1 += yd;
  room_bounds_y.b1 += yd;
  room_bounds_y.a0 += yd;
  room_bounds_y.b0 += yd;
}

void Dung_AdjustCoordsForLinkedRoom(uint8 room, uint8 flag) {
  dungeon_room_index2 = room;
  dungeon_room_index_prev = room;

  uint16 xx = (room & 0xf) * 2 - (link_x_coord >> 8) + flag;
  link_x_coord += (xx << 8);
  BG2HOFS_copy2 += (xx << 8);
  room_bounds_x.a1 += (xx << 8);
  room_bounds_x.b1 += (xx << 8);
  room_bounds_x.a0 += (xx << 8);
  room_bounds_x.b0 += (xx << 8);

  xx = ((room & 0xf0) >> 3) - (link_y_coord >> 8);
  link_y_coord += (xx << 8);
  BG2VOFS_copy2 += (xx << 8);
  room_bounds_y.a1 += (xx << 8);
  room_bounds_y.b1 += (xx << 8);
  room_bounds_y.a0 += (xx << 8);
  room_bounds_y.b0 += (xx << 8);

  for (int i = 0; i < 20; i++)
    tagalong_y_hi[i] = link_y_coord >> 8;
}

void Dung_SaveDataForCurrentRoom() {
  save_dung_info[dungeon_room_index] =
    (dung_savegame_state_bits >> 4) |
    (dung_door_opened & 0xf000) |
    dung_quadrants_visited;
}
void Dungeon_SaveRoomData_justKeys() {
  uint8 idx = cur_palace_index_x2;
  if (idx == 0xff)
    return;
  if (idx == 2)
    idx = 0;
  link_keys_earned_per_dungeon[idx >> 1] = link_num_keys;
}

const uint8 kLayoutQuadrantFlags[] = { 0xF, 0xF, 0xF, 0xF, 0xB, 0xB, 7, 7, 0xF, 0xB, 0xF, 7, 0xB, 0xF, 7, 0xF, 0xE, 0xD, 0xE, 0xD, 0xF, 0xF, 0xE, 0xD, 0xE, 0xD, 0xF, 0xF, 0xA, 9, 6, 5 };



void Dungeon_StartInterRoomTrans_Left() {
  link_quadrant_x ^= 1;
  UpdateCompositeOfLayoutAndQuadrant();
  RoomBounds_SubA(&room_bounds_x);
  Dung_SaveDataForCurrentRoom();
  Dung_CopyInLeftRightScrollTarget(link_quadrant_x ^ 1);
  Dung_UpdateCameraScrollBounds(3);
  submodule_index++;
  if (link_quadrant_x) {
    RoomBounds_SubB(&room_bounds_x);
    BYTE(dungeon_room_index_prev) = dungeon_room_index;
    if ((link_tile_below & 0xcf) == 0x89) {
      dungeon_room_index = dung_hdr_travel_destinations[3];
      Dung_AdjustCoordsForLinkedRoom(dungeon_room_index + 1, 0xff);
    } else {
      if ((uint8)dungeon_room_index != (uint8)dungeon_room_index2) {
        BYTE(dungeon_room_index_prev) = dungeon_room_index2;
        Dung_AdjustCoordsForNewRoom();
      }
      dungeon_room_index--;
    }
    submodule_index += 1;
    if (room_transitioning_flags & 1) {
      link_is_on_lower_level ^= 1;
      link_is_on_lower_level_mirror = link_is_on_lower_level;
    }
    if (room_transitioning_flags & 2) {
      cur_palace_index_x2 ^= 2;
    }
  }
  room_transitioning_flags = 0;
  quadrant_fullsize_y = (dung_blastwall_flag_y || (kLayoutQuadrantFlags[composite_of_layout_and_quadrant] & (link_quadrant_y ? 8 : 4)) == 0) ? 2 : 0;
}

void Dung_StartInterRoomTrans_Left_Plus() {
  link_x_coord -= 8;
  Dungeon_StartInterRoomTrans_Left();
}

void Dungeon_StartInterRoomTrans_Right() {
  link_quadrant_x ^= 1;
  UpdateCompositeOfLayoutAndQuadrant();
  RoomBounds_AddA(&room_bounds_x);
  Dung_SaveDataForCurrentRoom();
  Dung_CopyInLeftRightScrollTarget(link_quadrant_x);
  Dung_UpdateCameraScrollBounds(2);
  submodule_index++;
  if (!link_quadrant_x) {
    RoomBounds_AddB(&room_bounds_x);
    BYTE(dungeon_room_index_prev) = dungeon_room_index;
    if ((link_tile_below & 0xcf) == 0x89) {
      dungeon_room_index = dung_hdr_travel_destinations[4];
      Dung_AdjustCoordsForLinkedRoom(dungeon_room_index - 1, 1);
    } else {
      if ((uint8)dungeon_room_index != (uint8)dungeon_room_index2) {
        BYTE(dungeon_room_index_prev) = dungeon_room_index2;
        Dung_AdjustCoordsForNewRoom();
      }
      dungeon_room_index += 1;
    }
    submodule_index += 1;
    if (room_transitioning_flags & 1) {
      link_is_on_lower_level ^= 1;
      link_is_on_lower_level_mirror = link_is_on_lower_level;
    }
    if (room_transitioning_flags & 2) {
      cur_palace_index_x2 ^= 2;
    }
  }
  room_transitioning_flags = 0;
  quadrant_fullsize_y = (dung_blastwall_flag_y || (kLayoutQuadrantFlags[composite_of_layout_and_quadrant] & (link_quadrant_y ? 8 : 4)) == 0) ? 2 : 0;
}


void Dung_StartInterRoomTrans_RightPlus() {
  link_x_coord += 8;
  Dungeon_StartInterRoomTrans_Right();
}

void Dungeon_StartInterRoomTrans_Up() {
  link_quadrant_y ^= 2;
  UpdateCompositeOfLayoutAndQuadrant();
  RoomBounds_SubA(&room_bounds_y);
  Dung_SaveDataForCurrentRoom();
  Dung_CopyInUpDownScrollTarget(link_quadrant_y ^ 2);
  Dung_UpdateCameraScrollBounds(1);
  submodule_index++;
  if (link_quadrant_y) {
    RoomBounds_SubB(&room_bounds_y);
    BYTE(dungeon_room_index_prev) = dungeon_room_index;
    if (link_tile_below == 0x8e) {
      Dung_HandleExitToOverworld();
      return;
    }

    if (dungeon_room_index == 0) {
      Dungeon_SaveRoomData_justKeys();
      main_module_index = 25;
      submodule_index = 0;
      subsubmodule_index = 0;
      return;
    }

    if (BYTE(dungeon_room_index2) == BYTE(dungeon_room_index)) {
      BYTE(dungeon_room_index_prev) = BYTE(dungeon_room_index2);
      Dung_AdjustCoordsForNewRoom();
    }
    BYTE(dungeon_room_index) -= 0x10;
    submodule_index += 1;
    if (room_transitioning_flags & 1) {
      link_is_on_lower_level ^= 1;
      link_is_on_lower_level_mirror = link_is_on_lower_level;
    }
    if (room_transitioning_flags & 2) {
      cur_palace_index_x2 ^= 2;
    }
  }
  room_transitioning_flags = 0;
  quadrant_fullsize_x = (dung_blastwall_flag_x || (kLayoutQuadrantFlags[composite_of_layout_and_quadrant] & (link_quadrant_x ? 2 : 1)) == 0) ? 2 : 0;
}

void Dungeon_StartInterRoomTrans_Down() {
  link_quadrant_y ^= 2;
  UpdateCompositeOfLayoutAndQuadrant();
  RoomBounds_AddA(&room_bounds_y);
  Dung_SaveDataForCurrentRoom();
  Dung_CopyInUpDownScrollTarget(link_quadrant_y);
  Dung_UpdateCameraScrollBounds(0);
  submodule_index++;
  if (!link_quadrant_y) {
    RoomBounds_AddB(&room_bounds_y);
    BYTE(dungeon_room_index_prev) = dungeon_room_index;
    if (link_tile_below == 0x8e) {
      Dung_HandleExitToOverworld();
      return;
    }

    if ((uint8)dungeon_room_index != (uint8)dungeon_room_index2) {
      BYTE(dungeon_room_index_prev) = dungeon_room_index2;
      Dung_AdjustCoordsForNewRoom();
    }
    BYTE(dungeon_room_index) += 16;
    submodule_index += 1;
    if (room_transitioning_flags & 1) {
      link_is_on_lower_level ^= 1;
      link_is_on_lower_level_mirror = link_is_on_lower_level;
    }
    if (room_transitioning_flags & 2) {
      cur_palace_index_x2 ^= 2;
    }
  }
  room_transitioning_flags = 0;
  quadrant_fullsize_x = (dung_blastwall_flag_x || (kLayoutQuadrantFlags[composite_of_layout_and_quadrant] & (link_quadrant_x ? 2 : 1)) == 0) ? 2 : 0;
}


void Dung_StartInterRoomTrans_DownPlus() {
  link_y_coord += 16;
  Dungeon_StartInterRoomTrans_Down();
}

void Dungeon_ClearRupeeTile(uint16 x, uint16 y) {
  int pos = (y & 0x1f8) * 8 | (x & 0x1f8) >> 3;
  uint16 *dst = &vram_upload_data[vram_upload_offset >> 1];
  dst[2] = 0x190f;
  dst[5] = 0x190f;
  dung_bg2[pos + XY(0, 0)] = 0x190f;
  dung_bg2[pos + XY(0, 1)] = 0x190f;
  uint16 attr = attributes_for_tile[0x190f & 0x3ff] * 0x101;
  WORD(dung_bg2_attr_table[pos + XY(0, 0)]) = attr;
  WORD(dung_bg2_attr_table[pos + XY(0, 1)]) = attr;
  dst[0] = Dungeon_MapVramAddr(pos + XY(0, 0));
  dst[3] = Dungeon_MapVramAddr(pos + XY(0, 1));
  dst[1] = 0x100;
  dst[4] = 0x100;
  dst[6] = 0xffff;
  vram_upload_offset += 24;
  dung_savegame_state_bits |= 0x1000;
  nmi_load_bg_from_vram = 1;
}


void SaveQuadrantsToSram() {
  save_dung_info[dungeon_room_index] |= dung_quadrants_visited;
}

void SavePalaceDeaths() {
  int j = BYTE(cur_palace_index_x2);
  deaths_per_palace[j >> 1] = death_save_counter;
  if (j != 8)
    death_save_counter = 0;
}

void PrepDungeonBossExit() {
  SavePalaceDeaths();
  Dungeon_SaveRoomData_justKeys();
  dung_savegame_state_bits |= 0x8000;
  Dungeon_SaveRoomQuadrantData();

  int j = FindInByteArray(kDungeonExit_From, BYTE(dungeon_room_index), countof(kDungeonExit_From));
  assert(j >= 0);
  BYTE(dungeon_room_index) = kDungeonExit_To[j];
  if (BYTE(dungeon_room_index) == 0x20) {
    sram_progress_indicator = 3;
    save_ow_event_info[2] |= 0x20;
    savegame_is_darkworld ^= 0x40;
    Overworld_LoadGfxProperties_justLightWorld();
    Ancilla_TerminateSelectInteractives(0);
    link_disable_sprite_damage = 0;
    button_b_frames = 0;
    button_mask_b_y = 0;
    link_force_hold_sword_up = 0;
    flag_is_link_immobilized = 1;
    saved_module_for_menu = 8;
    main_module_index = 21;
    submodule_index = 0;
    subsubmodule_index = 0;
  } else if (BYTE(dungeon_room_index) == 0xd) {
    main_module_index = 24;
    submodule_index = 0;
    overworld_map_state = 0;
    CGADSUB_copy = 0x20;
  } else {
    if (j >= 3) {
      music_control = 0xf1;
      music_unk1 = 0xf1;
      main_module_index = 22;
    } else {
      main_module_index = 19;
    }
    saved_module_for_menu = 8;
    submodule_index = 0;
    subsubmodule_index = 0;
  }
}


static const uint8 kQuadrantVisitingFlags[] = { 8, 4, 2, 1, 0xC, 0xC, 3, 3, 0xA, 5, 0xA, 5, 0xF, 0xF, 0xF, 0xF };

void Dungeon_SaveRoomQuadrantData() {
  dung_quadrants_visited |= kQuadrantVisitingFlags[(quadrant_fullsize_y << 2) + (quadrant_fullsize_x << 1) + link_quadrant_y + link_quadrant_x];
  Dung_SaveDataForCurrentRoom();
}

void Player_UpdateQuadrantsVisited() {
  dung_quadrants_visited |= kQuadrantVisitingFlags[(quadrant_fullsize_y << 2) + (quadrant_fullsize_x << 1) + link_quadrant_y + link_quadrant_x];
  save_dung_info[dungeon_room_index] |= dung_quadrants_visited;
}

void Dungeon_SaveRoomData() {
  if (cur_palace_index_x2 == 0xff) {
    sound_effect_1 = 60;
    return;
  }
  submodule_index = 25;
  subsubmodule_index = 0;
  sound_effect_1 = 51;
  Dungeon_SaveRoomQuadrantData();
  Dungeon_SaveRoomData_justKeys();
}

#define XY(x, y) ((y)*64+(x))

static const uint8 kDungeon_MinigameChestPrizes1[8] = {
  0x40, 0x41, 0x34, 0x42, 0x43, 0x44, 0x27, 0x17
};
static const uint8 kDungeon_RupeeChestMinigamePrizes[32] = {
  0x47, 0x34, 0x46, 0x34, 0x46, 0x46, 0x34, 0x47, 0x46, 0x47, 0x34, 0x46, 0x47, 0x34, 0x46, 0x47,
  0x34, 0x47, 0x41, 0x47, 0x41, 0x41, 0x47, 0x34, 0x41, 0x34, 0x47, 0x41, 0x34, 0x47, 0x41, 0x34,
};

uint16 Dungeon_GetKeyedObjectRelativeVramAddr(uint16 pos, uint16 y) {
  pos += dung_chest_locations[y >> 1];
  return swap16(((pos & 0x40) << 4) | ((pos & 0x303f) >> 1) | ((pos & 0xf80) >> 2));
}

uint8 Dungeon_OpenMiniGameChest(int *chest_position) {
  int t;
  if (minigame_credits == 0) {
    dialogue_message_index = 0x163;
    Main_ShowTextMessage();
    return 0xff;
  }
  if (minigame_credits == 255) {
    dialogue_message_index = 0x162;
    Main_ShowTextMessage();
    return 0xff;
  }
  minigame_credits--;

  int pos = ((link_y_coord - 4) & 0x1f8) * 8;
  pos |= ((link_x_coord + 7) & 0x1f8) >> 3;

  if (WORD(dung_bg2_attr_table[pos]) != 0x6363) {
    pos--;
    if (WORD(dung_bg2_attr_table[pos]) != 0x6363)
      pos -= 2;
  }

  *chest_position = pos * 2;

  WORD(dung_bg2_attr_table[pos]) = 0x202;
  WORD(dung_bg2_attr_table[pos + 64]) = 0x202;

  pos += XY(0, 2);

  const uint16 *src = SrcPtr(0x14A4);
  dung_bg2[pos + XY(0, 0)] = src[0];
  dung_bg2[pos + XY(0, 1)] = src[1];
  dung_bg2[pos + XY(1, 0)] = src[2];
  dung_bg2[pos + XY(1, 1)] = src[3];

  uint16 yy = 0x14A4; // wtf

  uint16 *dst = &vram_upload_data[vram_upload_offset >> 1];
  dst[0] = Dungeon_GetKeyedObjectRelativeVramAddr((pos + 0) * 2, yy);
  dst[3] = Dungeon_GetKeyedObjectRelativeVramAddr((pos + 64) * 2, yy);
  dst[6] = Dungeon_GetKeyedObjectRelativeVramAddr((pos + 1) * 2, yy);
  dst[9] = Dungeon_GetKeyedObjectRelativeVramAddr((pos + 65) * 2, yy);

  dst[2] = src[0];
  dst[5] = src[1];
  dst[8] = src[2];
  dst[11] = src[3];

  dst[1] = 0x100;
  dst[4] = 0x100;
  dst[7] = 0x100;
  dst[10] = 0x100;

  dst[12] = 0xffff;

  vram_upload_offset += 24;

  uint8 rv;

  uint16 r16 = g_ram[0xc8];

  t = GetRandomInt();
  if (BYTE(dungeon_room_index) == 0) {
    t = t & 0xf;
    rv = kDungeon_RupeeChestMinigamePrizes[t & 0xf];

  } else if (BYTE(dungeon_room_index) == 0x18) {
    t = 0x10 + (t & 0xf);
    rv = kDungeon_RupeeChestMinigamePrizes[0x10 + (t & 0xf)];
  } else {
    t &= 7;
    if (t >= 2 && t == r16) {
      t = (t + 1) & 7;
    }
    if (t == 7) {
      if (dung_savegame_state_bits & 0x4000) {
        t = 0;
      } else {
        dung_savegame_state_bits |= 0x4000;
      }
    }
    rv = kDungeon_MinigameChestPrizes1[t];
  }
  g_ram[0xc8] = t;
  nmi_load_bg_from_vram = 1;
  sound_effect_2 = 14;
  return rv;
}

void Dungeon_OpenBigChest(uint16 loc, int *chest_position) {
  uint16 pos = loc >> 1;
  const uint16 *src = SrcPtr(0x14C4);

  for (int i = 0; i < 4; i++) {
    dung_bg2[pos + XY(i, 0)] = src[0];
    dung_bg2[pos + XY(i, 1)] = src[1];
    dung_bg2[pos + XY(i, 2)] = src[2];
    src += 3;
  }

  Dungeon_PrepOverlayDma_nextPrep(0, loc);
  *chest_position = (loc + 2);
  WORD(dung_bg2_attr_table[pos + XY(0, 0)]) = 0x2727;
  WORD(dung_bg2_attr_table[pos + XY(2, 0)]) = 0x2727;
  WORD(dung_bg2_attr_table[pos + XY(0, 1)]) = 0x2727;
  WORD(dung_bg2_attr_table[pos + XY(2, 1)]) = 0x2727;
  WORD(dung_bg2_attr_table[pos + XY(0, 2)]) = 0x2727;
  WORD(dung_bg2_attr_table[pos + XY(2, 2)]) = 0x2727;
  Dungeon_SaveRoomQuadrantData();
  sound_effect_2 = 14;
  nmi_copy_packets_flag = 1;
  byte_7E0B9E = 1;
}

// This doesn't return exactly like the original
// Also returns in scratch_0
uint8 Dungeon_OpenKeyedObject(uint8 tile, int *chest_position) {
  static const uint16 kChestOpenMasks[] = { 0x100, 0x200, 0x400, 0x800, 0x1000, 0x2000 };
  if (tile == 0x63)
    return Dungeon_OpenMiniGameChest(chest_position);

  int chest_idx = tile - 0x58, chest_idx_org = chest_idx;

  uint16 loc = dung_chest_locations[chest_idx], pos, chest_room;
  uint8 data = 0xff;
  const uint16 *ptr;

  if (loc >= 0x8000) {
    // big key lock
    if (!(link_bigkey & kUpperBitmasks[cur_palace_index_x2 >> 1])) {
      dialogue_message_index = 0x7a;
      Main_ShowTextMessage();
      return 0xff;
    } else {
      dung_savegame_state_bits |= kChestOpenMasks[chest_idx];
      sound_effect_1 = 0x29;
      sound_effect_2 = 0x15;
      pos = (loc & 0x7fff) >> 1;

      ptr = SrcPtr(dung_floor_2_filler_tiles);
      overworld_tileattr[pos + 0] = ptr[0];
      overworld_tileattr[pos + 64] = ptr[1];
      overworld_tileattr[pos + 1] = ptr[2];
      overworld_tileattr[pos + 65] = ptr[3];
      goto afterStoreCrap;
    }
  } else {
    const uint8 *chest_data;
    int i;
    chest_data = kDungeonRoomChests;
    for (i = 0; i < countof(kDungeonRoomChests); i += 3, chest_data += 3) {
      chest_room = *(uint16 *)chest_data;
      if ((chest_room & 0x7fff) == dungeon_room_index && --chest_idx < 0) {
        data = chest_data[2];
        if (chest_room & 0x8000) {
          if (!(link_bigkey & kUpperBitmasks[cur_palace_index_x2 >> 1])) {
            dialogue_message_index = 0x7a;
            Main_ShowTextMessage();
            return 0xff;
          }
          dung_savegame_state_bits |= kChestOpenMasks[chest_idx_org];
          Dungeon_OpenBigChest(loc, chest_position);
          return data;
        } else {
          dung_savegame_state_bits |= kChestOpenMasks[chest_idx_org];
          ptr = SrcPtr(0x14A4);
          pos = loc >> 1;

          overworld_tileattr[pos + 0] = ptr[0];
          overworld_tileattr[pos + 64] = ptr[1];
          overworld_tileattr[pos + 1] = ptr[2];
          overworld_tileattr[pos + 65] = ptr[3];

afterStoreCrap:
          uint8 attr = (loc < 0x8000) ? 0x27 : 0x00;

          dung_bg2_attr_table[pos + 0] = attr;
          dung_bg2_attr_table[pos + 64] = attr;
          dung_bg2_attr_table[pos + 1] = attr;
          dung_bg2_attr_table[pos + 65] = attr;

          uint16 *dst = &vram_upload_data[vram_upload_offset >> 1];
          dst[0] = Dungeon_MapVramAddr(pos + 0);
          dst[3] = Dungeon_MapVramAddr(pos + 64);
          dst[6] = Dungeon_MapVramAddr(pos + 1);
          dst[9] = Dungeon_MapVramAddr(pos + 65);

          dst[2] = ptr[0];
          dst[5] = ptr[1];
          dst[8] = ptr[2];
          dst[11] = ptr[3];

          dst[1] = 0x100;
          dst[4] = 0x100;
          dst[7] = 0x100;
          dst[10] = 0x100;

          dst[12] = 0xffff;

          vram_upload_offset += 24;
          nmi_load_bg_from_vram = 1;
          Dungeon_SaveRoomQuadrantData();
          if (sound_effect_2 == 0)
            sound_effect_2 = 14;

          *chest_position = loc & 0x7fff;
          return data;
        }
      }
    }
    return 0xff;
  }
}

static const int8 kDungeon_QueryIfTileLiftable_x[4] = { 7, 7, -3, 16 };
static const int8 kDungeon_QueryIfTileLiftable_y[4] = { 3, 24, 14, 14 };
static const uint16 kDungeon_QueryIfTileLiftable_rv[16] = { 0x5252, 0x5050, 0x5454, 0x0, 0x2323 };

uint16 Dungeon_QueryIfTileLiftable() {
  uint16 x = (link_x_coord + kDungeon_QueryIfTileLiftable_x[link_direction_facing >> 1]) & 0x1f8;
  uint16 y = (link_y_coord + kDungeon_QueryIfTileLiftable_y[link_direction_facing >> 1]) & 0x1f8;
  uint16 xy = (y << 3) | (x >> 3) | (link_is_on_lower_level ? 0x1000 : 0x0);

  uint8 attr = dung_bg2_attr_table[xy];
  if ((attr & 0xf0) != 0x70)
    return 0xffff;  // clc

  uint16 rt = dung_replacement_tile_state[attr & 0xf];
  if (rt == 0)
    return 0xffff;
  if ((rt & 0xf0f0) == 0x2020)
    return 0x55;
  return kDungeon_QueryIfTileLiftable_rv[rt & 0xf];
}

void Dungeon_LoadSecret(uint16 pos6, uint16 pos4) {
  BYTE(dung_secrets_unk1) = 0;

  const uint8 *src_ptr = kDungeonSecrets + WORD(kDungeonSecrets[dungeon_room_index * 2]);

  int index = 0;
  for (;;) {
    uint16 test_pos = *(uint16 *)src_ptr;
    if (test_pos == 0xffff)
      return;
    assert(!(test_pos & 0x8000));
    if (test_pos == pos4)
      break;
    src_ptr += 3;
    index++;
  }

  uint8 data = src_ptr[2];
  if (data == 0)
    return;

  if (data < 0x80) {
    if (data != 8) {
      uint16 mask = 1 << index;
      uint16 *pr = &pots_revealed_in_room[dungeon_room_index];
      if (*pr & mask)
        return;
      *pr |= mask;
    }
    BYTE(dung_secrets_unk1) |= data;
  } else if (data != 0x88) {
    int j = dung_bg2_attr_table[pos6] & 0xf;
    int k = (j - (dung_replacement_tile_state[j] & 0xf));
    dung_misc_objs_index = 2 * k;
    sound_effect_2 = 0x1b;
    const uint16 *src = SrcPtr(0x5ba);
    for (int i = 0; i < 4; i++, k++, src += 4) {
      replacement_tilemap_UL[k] = src[0];
      replacement_tilemap_LL[k] = src[1];
      replacement_tilemap_UR[k] = src[2];
      replacement_tilemap_LR[k] = src[3];
    }
  } else {
    int k = dung_misc_objs_index >> 1;
    replacement_tilemap_UL[k] = 0xD0B;
    replacement_tilemap_LL[k] = 0xD1B;
    replacement_tilemap_UR[k] = 0x4D0B;
    replacement_tilemap_LR[k] = 0x4D1B;
  }
}

void Dungeon_GetUprootedTerrainSpawnCoords(Point16U *pt) {
  uint16 pos = dung_object_tilemap_pos[dung_misc_objs_index >> 1];
  pt->x = (link_x_coord & 0xfe00) | ((pos & 0x007e) << 2);
  pt->y = (link_y_coord & 0xfe00) | ((pos & 0x1f80) >> 4);
}

uint8 Dungeon_CustomIndexedRevealCoveredTiles(uint16 pos6, uint16 a, Point16U *pt) {
  dung_misc_objs_index = a;
  Dungeon_LoadSecret(pos6, dung_object_tilemap_pos[a >> 1]);
  Dungeon_EraseInteractive2x2(a);
  Dungeon_EraseInteractive2x2(a + 2);
  Dungeon_EraseInteractive2x2(a + 4);
  Dungeon_EraseInteractive2x2(a + 6);
  Dungeon_GetUprootedTerrainSpawnCoords(pt);
  return 0x55;
}

uint8 Dungeon_RevealCoveredTiles(Point16U *pt) {
  uint16 x = link_x_coord + kDungeon_QueryIfTileLiftable_x[link_direction_facing >> 1];
  uint16 y = link_y_coord + kDungeon_QueryIfTileLiftable_y[link_direction_facing >> 1];
  pt->x = x;
  pt->y = y;

  R16 = y;
  R18 = x;

  x &= 0x1f8;
  y &= 0x1f8;
  uint16 xy = (y << 3) | (x >> 3) | (link_is_on_lower_level ? 0x1000 : 0x0);

  uint8 attr = dung_bg2_attr_table[xy];

  assert((attr & 0x70) == 0x70);

  attr &= 0xf;
  uint16 rt = dung_replacement_tile_state[attr];

  if ((rt & 0xf0f0) == 0x1010) {
    dung_misc_objs_index = attr * 2;
    Dungeon_LoadSecret(xy, dung_object_tilemap_pos[attr]);
    Dungeon_EraseInteractive2x2(dung_misc_objs_index);
    Dungeon_GetUprootedTerrainSpawnCoords(pt);
    return kDungeon_QueryIfTileLiftable_rv[rt & 0xf];
  } else if ((rt & 0xf0f0) == 0x2020) {
    return Dungeon_CustomIndexedRevealCoveredTiles(xy, (attr - (rt & 0xf)) * 2, pt);
  } else {
    return 0;
  }
  return 0;
}

uint8 Dungeon_ToolAndTileInteraction(uint16 x, uint16 y) {
  if (!(link_item_in_hand & 2))
    return 0;
  uint16 pos = (y & 0x1f8) * 8 + x + (link_is_on_lower_level ? 0x1000 : 0);
  uint16 tile = dung_bg2_attr_table[pos];
  if ((tile & 0xf0) == 0x70) {
    uint16 tile2 = dung_replacement_tile_state[tile & 0xf];
    if ((tile2 & 0xf0f0) == 0x4040) {
      dung_misc_objs_index = (tile & 0xf) * 2;
      Dungeon_EraseInteractive2x2(dung_misc_objs_index);
      sound_effect_1 = 0x11;
    } else if ((tile2 & 0xf0f0) == 0x1010) {
      dung_misc_objs_index = (tile & 0xf) * 2;
      Dungeon_LoadSecret(pos, dung_object_tilemap_pos[tile & 0xf]);
      Dungeon_EraseInteractive2x2(dung_misc_objs_index);
      Point16U pt;
      Dungeon_GetUprootedTerrainSpawnCoords(&pt);
      BYTE(dung_secrets_unk1) |= 0x80;
      Sprite_SpawnImmediatelySmashedTerrain(1, pt.x, pt.y);
      AddDisintegratingBushPoof(pt.x, pt.y);  // return value wtf?
    }
  }
  return 0;
}


void Dungeon_PrepSpriteInducedDma(int x, int y, uint8 v) {
  static const uint16 kPrepSpriteInducedDma_Srcs[10] = { 0xe0, 0xade, 0x5aa, 0x198, 0x210, 0x218, 0x1f3a, 0xeaa, 0xeb2, 0x140 };
  int pos = ((y + 1) & 0x1f8) << 3 | (x & 0x1f8) >> 3;
  const uint16 *src = SrcPtr(kPrepSpriteInducedDma_Srcs[v >> 1]);
  uint16 *dst = &vram_upload_data[vram_upload_offset >> 1];
  dst[0] = Dungeon_MapVramAddr(pos + 0);
  dst[3] = Dungeon_MapVramAddr(pos + 64);
  dst[6] = Dungeon_MapVramAddr(pos + 1);
  dst[9] = Dungeon_MapVramAddr(pos + 65);
  uint8 attr = attributes_for_tile[src[3] & 0x3ff];
  dung_bg2_attr_table[pos + XY(0, 0)] = attr;
  dung_bg2_attr_table[pos + XY(0, 1)] = attr;
  dung_bg2_attr_table[pos + XY(1, 0)] = attr;
  dung_bg2_attr_table[pos + XY(1, 1)] = attr;
  dung_bg2[pos + XY(0, 0)] = dst[2] = src[0];
  dung_bg2[pos + XY(0, 1)] = dst[5] = src[1];
  dung_bg2[pos + XY(1, 0)] = dst[8] = src[2];
  dung_bg2[pos + XY(1, 1)] = dst[11] = src[3];
  dst[1] = 0x100;
  dst[4] = 0x100;
  dst[7] = 0x100;
  dst[10] = 0x100;
  dst[12] = 0xffff;
  vram_upload_offset += 24;
}

void Dungeon_SpriteInducedTilemapUpdate(int x, int y, uint8 v) {
  if (v == 8)
    Dungeon_PrepSpriteInducedDma(x + 16, y, v + 2);
  Dungeon_PrepSpriteInducedDma(x, y, v);
  nmi_load_bg_from_vram = 1;
}

void Dungeon_Store2x2(uint16 pos, uint16 t0, uint16 t1, uint16 t2, uint16 t3, uint8 attr) {
  uint16 *dst = &vram_upload_data[vram_upload_offset >> 1];
  dst[2] = t0;
  dst[5] = t1;
  dst[8] = t2;
  dst[11] = t3;
  overworld_tileattr[pos] = t0;
  overworld_tileattr[pos + 64] = t1;
  overworld_tileattr[pos + 1] = t2;
  overworld_tileattr[pos + 65] = t3;

  dung_bg2_attr_table[pos] = attr;
  dung_bg2_attr_table[pos + 64] = attr;
  dung_bg2_attr_table[pos + 1] = attr;
  dung_bg2_attr_table[pos + 65] = attr;

  dst[0] = Dungeon_MapVramAddr(pos + 0);
  dst[3] = Dungeon_MapVramAddr(pos + 64);
  dst[6] = Dungeon_MapVramAddr(pos + 1);
  dst[9] = Dungeon_MapVramAddr(pos + 65);

  dst[1] = 0x100;
  dst[4] = 0x100;
  dst[7] = 0x100;
  dst[10] = 0x100;

  dst[12] = 0xffff;

  vram_upload_offset += 24;
  nmi_load_bg_from_vram = 1;
}

void Dungeon_EraseInteractive2x2(uint8 index) {
  index >>= 1;
  uint16 pos = (dung_object_tilemap_pos[index] & 0x3fff) >> 1;
  Dungeon_Store2x2(pos,
                   replacement_tilemap_UL[index],
                   replacement_tilemap_LL[index],
                   replacement_tilemap_UR[index],
                   replacement_tilemap_LR[index],
                   attributes_for_tile[replacement_tilemap_LR[index] & 0x3ff]);
}


uint16 Dungeon_MapVramAddr(uint16 pos) {
  pos *= 2;
  return swap16(((pos & 0x40) << 4) | ((pos & 0x303f) >> 1) | ((pos & 0xf80) >> 2));
}

uint16 Dungeon_MapVramAddrNoSwap(uint16 pos) {
  pos *= 2;
  return ((pos & 0x40) << 4) | ((pos & 0x303f) >> 1) | ((pos & 0xf80) >> 2);
}



void Door_BlastWallUp_Helper2(const uint16 *src, uint16 *dst) {
  for (int i = 0; i < 6; i++) {
    dst[0] = src[0];
    dst[1] = src[6];
    dst += XY(0, 1), src += 1;
  }
}

void Door_BlastWallUp_Helper(const uint16 *src, uint16 dsto) {
  Door_BlastWallUp_Helper2(src, DstoPtr(dsto));
  src += 12, dsto += 2;
  int n = src[0];
  uint16 *d = &dung_bg2[dsto];
  dung_draw_width_indicator = 18;
  do {
    d[XY(0, 0)] = d[XY(0, 1)] = d[XY(0, 2)] = n;
    d[XY(0, 3)] = d[XY(0, 4)] = d[XY(0, 5)] = n;
    d++;
  } while (--dung_draw_width_indicator);
  Door_BlastWallUp_Helper2(src + 1, DstoPtr(dsto + 18));
}

static const uint16 kDoor_BlastWallUp_Dsts[] = { 0xd8a, 0xdaa, 0xdca, 0x2b6, 0xab6, 0x12b6 };

void Door_BlastWallUp(int pos_enum) {
  uint16 dsto = kDoor_BlastWallUp_Dsts[pos_enum] / 2;
  int i = dung_cur_door_idx >> 1;
  dung_door_tilemap_address[i] = 2 * (dsto + 10);
  door_type_and_slot[i] = i << 8 | kDoorType_LgExplosion;
  if (!(dung_door_opened_incl_adjacent & kUpperBitmasks[i & 7])) {
    dung_door_direction[i] = 0;
    dung_cur_door_idx += 2;
    return;
  }
  int slot = dung_hdr_tag[0] != 0x20 && dung_hdr_tag[0] != 0x25 && dung_hdr_tag[0] != 0x28;
  dung_hdr_tag[slot] = 0;
  quadrant_fullsize_y = 2;
  dung_blastwall_flag_y = 1;
  Door_BlastWallUp_Helper(SrcPtr(kDoorTypeSrcData2[42]), dsto);
  dung_cur_door_idx += 2;
  dung_unk2 |= 0x200;
  Door_BlastWallUp_Helper(SrcPtr(kDoorTypeSrcData[42]), dsto + XY(0, 6));
}


// returns 0x100 on inverse carry
int Door_Register(uint8 direction, uint8 door_type, uint16 dsto) {
  int slot = dung_cur_door_idx >> 1;
  dung_door_direction[slot] = direction;
  dung_door_tilemap_address[slot] = dsto * 2;
  door_type_and_slot[slot] = slot << 8 | door_type;

  uint8 door_type_remapped = door_type;

  if ((slot & 7) < 4 && (dung_door_opened_incl_adjacent & kUpperBitmasks[slot & 7])) {
    if ((door_type == kDoorType_ShuttersTwoWay || door_type == kDoorType_Shutter) && dung_flag_trapdoors_down)
      goto dont_mark_opened;
    door_type_remapped = kDoorTypeRemap[door_type >> 1];

    if (door_type != kDoorType_ShuttersTwoWay && door_type != kDoorType_Shutter &&
        door_type >= kDoorType_InvisibleDoor && door_type != kDoorType_RegularDoor33 && door_type != kDoorType_WarpRoomDoor)
      dung_door_opened |= kUpperBitmasks[slot];
  }
dont_mark_opened:
  dung_cur_door_idx = slot * 2 + 2;

  if (door_type_remapped == kDoorType_Slashable || door_type_remapped == kDoorType_WaterfallTunnel)
    return 0x100 | door_type_remapped;

  if (door_type != kDoorType_InvisibleDoor)
    return door_type_remapped;

  invisible_door_dir_and_index_x2 = (slot << 8 | direction) * 2;
  //  if (direction * 2 == link_direction_facing || ((direction * 2) ^ 2) == link_direction_facing)
  //    return door_type_remapped;
  dung_door_opened_incl_adjacent |= kUpperBitmasks[slot];
  return kDoorType_Regular;
}

void Door_Up_SwordActivated(uint16 dsto) {
  int rv = Door_Register(0, kDoorType_Slashable, dsto);
  if (rv & 0x100) {
    Object_Draw4x4(SrcPtr(0x78a), DstoPtr(dsto));
  } else {
    Object_Draw4x4(SrcPtr(kDoorTypeSrcData[rv >> 1]), DstoPtr(dsto));
  }
}

void Door_Up_EntranceDoor(uint16 dsto) {
  // NOTE: This don't pass the right value to Door_Register
  assert(0);
}

void Door_Down_EntranceDoor(uint16 dsto) {
  // NOTE: This don't pass the right value to Door_Register
  assert(0);
}

void Door_Left_EntranceDoor(uint16 dsto) {
  // NOTE: This don't pass the right value to Door_Register
  assert(0);
}

void Door_Right_EntranceDoor(uint16 dsto) {
  // NOTE: This don't pass the right value to Door_Register
  assert(0);
}

void Door_AddFloorToggleProperty(uint16 dsto) {
  dung_toggle_floor_pos[dung_num_toggle_floor >> 1] = dsto;
  dung_num_toggle_floor += 2;
}

void Door_AddPalaceToggleProperty(uint16 dsto) {
  dung_toggle_palace_pos[dung_num_toggle_palace >> 1] = dsto;
  dung_num_toggle_palace += 2;
}

void Door_Prioritize4x7(uint16 dsto) {
  uint16 *d = &dung_bg2[dsto];
  for (int i = 0; i < 7; i++) {
    d[XY(0, 0)] |= 0x2000;
    d[XY(1, 0)] |= 0x2000;
    d[XY(2, 0)] |= 0x2000;
    d[XY(3, 0)] |= 0x2000;
    d += XY(0, 1);
  }
}

void Door_Prioritize5x4(uint16 dsto) {
  uint16 *d = &dung_bg2[dsto];
  for (int i = 0; i < 5; i++) {
    d[XY(0, 0)] |= 0x2000;
    d[XY(0, 1)] |= 0x2000;
    d[XY(0, 2)] |= 0x2000;
    d[XY(0, 3)] |= 0x2000;
    d += XY(1, 0);
  }
}

void Door_PrioritizeAligned(uint16 dsto) {
  uint16 *d = &dung_bg2[dsto];
  do {
    d[XY(0, 0)] |= 0x2000;
    d[XY(0, 1)] |= 0x2000;
    d[XY(0, 2)] |= 0x2000;
    d[XY(0, 3)] |= 0x2000;
    d += XY(1, 0), dsto += 1;
  } while (dsto & 0x1f);
}

void Door_Helper_SetPriority(uint16 dsto) {
  uint16 dsto_org = dsto;
  dsto &= 0xF07F >> 1;
  do {
    dung_bg2[dsto + 0] |= 0x2000;
    dung_bg2[dsto + 1] |= 0x2000;
    dung_bg2[dsto + 2] |= 0x2000;
    dung_bg2[dsto + 3] |= 0x2000;
    dsto += XY(0, 1);
  } while (dsto != dsto_org);
}

void Door_Helper_SetPriority2(uint16 dsto) {
  do {
    dung_bg2[dsto + 0] |= 0x2000;
    dung_bg2[dsto + 1] |= 0x2000;
    dung_bg2[dsto + 2] |= 0x2000;
    dung_bg2[dsto + 3] |= 0x2000;
    dsto += XY(0, 1);
  } while (dsto & 0x7c0);
}

void Door_Helper_SetPriority3(uint16 dsto) {
  uint16 dsto_org = dsto;
  dsto &= 0xffe0;
  do {
    dung_bg2[dsto + XY(0, 0)] |= 0x2000;
    dung_bg2[dsto + XY(0, 1)] |= 0x2000;
    dung_bg2[dsto + XY(0, 2)] |= 0x2000;
    dung_bg2[dsto + XY(0, 3)] |= 0x2000;
    dsto += XY(1, 0);
  } while (dsto != dsto_org);
}



void Door_PrioritizeCurDoor() {
  dung_door_tilemap_address[(dung_cur_door_idx >> 1) - 1] |= 0x2000;
}

void Door_Draw_Helper4(uint8 door_type, uint16 dsto) {
  int t = Door_Register(1, door_type, dsto), new_type;
  if (t & 0x100)
    return;

  if ((new_type = kDoorType_Regular, t == kDoorType_1E || t == kDoorType_36) ||
      (new_type = kDoorType_ShuttersTwoWay, t == kDoorType_38)) {
    int i = (dung_cur_door_idx >> 1) - 1;
    door_type_and_slot[i] = (i << 8) | new_type;
    t = new_type;
  }

  const uint16 *src = SrcPtr(kDoorTypeSrcData2[t >> 1]);
  uint16 *dst = DstoPtr(dsto);
  for (int i = 0; i < 4; i++) {
    dst[XY(0, 1)] = src[0];
    dst[XY(0, 2)] = src[1];
    dst[XY(0, 3)] = src[2];
    dst++, src += 3;
  }
}

void Door_Down_Draw_Helper_2(uint8 door_type, uint16 dsto) {
  if (door_type == kDoorType_Regular2) {
    Door_Prioritize4x7(dsto + XY(0, 4));
    Door_Draw_Helper4(door_type, dsto);
  } else if (door_type == kDoorType_WaterfallTunnel) {
    Door_Draw_Helper4(door_type, dsto);
    Door_PrioritizeCurDoor();
  } else {
    Door_Draw_Helper4(door_type, dsto);
  }
}

void Door_Down_Draw_Types_Above_0x40(uint8 door_type, uint16 dsto) {
  uint8 t = Door_Register(1, door_type, dsto);
  if (t == kDoorType_ShutterTrapUR || t == kDoorType_ShutterTrapDL) {
    int new_type = (t == kDoorType_ShutterTrapUR) ? kDoorType_Shutter : kDoorType_RegularDoor33;
    int i = (dung_cur_door_idx >> 1) - 1;
    door_type_and_slot[i] = (i << 8) | new_type;
    t = new_type;
  }
  uint16 dsto_org = dsto;
  const uint16 *src = SrcPtr(kDoorTypeSrcData2[t >> 1]);
  for (int i = 0; i < 4; i++) {
    dung_bg1[dsto + XY(0, 1)] = src[0];
    dung_bg1[dsto + XY(0, 2)] = src[1];
    dung_bg2[dsto + XY(0, 3)] = src[2];
    dsto++, src += 3;
  }
  Door_Helper_SetPriority2(dsto_org + XY(0, 4));
  Door_PrioritizeCurDoor();
}

void Door_Up_PositionLessThan6(uint8 door_type, uint16 dsto) {
  int t = Door_Register(0, door_type, dsto);
  if (t & 0x100)
    return;
  // Remap type
  if (t == 54 || t == 56) {
    int new_type = (t == 54) ? kDoorType_ShuttersTwoWay : kDoorType_Regular;
    int i = (dung_cur_door_idx >> 1) - 1;
    door_type_and_slot[i] = (i << 8) | new_type;
    t = new_type;
  }
  const uint16 *src = SrcPtr(kDoorTypeSrcData[t >> 1]);
  uint16 *dst = DstoPtr(dsto);
  for (int i = 0; i < 4; i++) {
    dst[XY(0, 0)] = src[0];
    dst[XY(0, 1)] = src[1];
    dst[XY(0, 2)] = src[2];
    dst++, src += 3;
  }
}

void Door_Up_StairMaskLocked(uint8 door_type, uint16 dsto) {
  int i = dung_cur_door_idx >> 1;
  dung_door_direction[i] = 0;
  dung_door_tilemap_address[i] = dsto * 2;
  door_type_and_slot[i] = i << 8 | door_type;
  if (dung_door_opened_incl_adjacent & kUpperBitmasks[i & 7]) {
    dung_cur_door_idx += 2;
    return;
  }

  if (door_type < kDoorType_StairMaskLocked2) {
    Door_Up_PositionLessThan6(door_type, dsto);
    return;
  }

  uint8 t = Door_Register(0, door_type, dsto);
  const uint16 *src = SrcPtr(kDoorTypeSrcData[t >> 1]);
  for (int i = 0; i < 4; i++) {
    dung_bg1[dsto + XY(0, 0)] = src[0];
    dung_bg1[dsto + XY(0, 1)] = src[1];
    dung_bg1[dsto + XY(0, 2)] = src[2];
    dsto++, src += 3;
  }
  Door_PrioritizeCurDoor();
}

void Door_Up_Draw_Types_Below_0x40(uint8 door_type, uint16 dsto, int pos_enum) {
  if (pos_enum >= 6) {
    uint16 bak = dung_cur_door_idx;
    dung_cur_door_idx |= 0x10;
    Door_Down_Draw_Helper_2(door_type, kDoorPositionToTilemapOffs_Down[pos_enum - 6] / 2);
    dung_cur_door_idx = bak;
  }
  Door_Up_PositionLessThan6(door_type, dsto);
}

void Door_Up_Draw_Types_Above_0x40(uint8 door_type, uint16 dsto, int pos_enum) {
  if (pos_enum >= 6 && door_type != kDoorType_WarpRoomDoor) {
    uint16 bak = dung_cur_door_idx;
    dung_cur_door_idx |= 0x10;
    Door_Down_Draw_Types_Above_0x40(door_type, kDoorPositionToTilemapOffs_Down[pos_enum - 6] / 2);
    dung_cur_door_idx = bak;
  }
  uint8 t = Door_Register(0, door_type, dsto);
  if (t == kDoorType_ShutterTrapUR || t == kDoorType_ShutterTrapDL) {
    int new_type = (t == kDoorType_ShutterTrapUR) ? kDoorType_RegularDoor33 : kDoorType_Shutter;
    int i = (dung_cur_door_idx >> 1) - 1;
    door_type_and_slot[i] = (i << 8) | new_type;
    t = new_type;
  }
  uint16 dsto_org = dsto;
  const uint16 *src = SrcPtr(kDoorTypeSrcData[t >> 1]);
  for (int i = 0; i < 4; i++) {
    dung_bg2[dsto + XY(0, 0)] = src[0];
    dung_bg1[dsto + XY(0, 1)] = src[1];
    dung_bg1[dsto + XY(0, 2)] = src[2];
    dsto++, src += 3;
  }
  if (door_type != kDoorType_WarpRoomDoor)
    Door_Helper_SetPriority(dsto_org);
  Door_PrioritizeCurDoor();
}

void Door_Up(int type, int pos_enum) {
  uint16 dsto = kDoorPositionToTilemapOffs_Up[pos_enum] / 2;
  if (type == kDoorType_LgExplosion)
    Door_BlastWallUp(pos_enum);
  else if (type == kDoorType_PlayerBgChange)
    Door_AddFloorToggleProperty(dsto - 0xfe / 2);
  else if (type == kDoorType_Slashable)
    Door_Up_SwordActivated(dsto);
  else if (type == kDoorType_EntranceDoor)
    Door_Up_EntranceDoor(dsto);
  else if (type == kDoorType_ThroneRoom)
    Door_AddPalaceToggleProperty(dsto - 0xfe / 2);
  else if (type == kDoorType_Regular2) {
    Door_Prioritize4x7(dsto & (0xF07F / 2));
    Door_Up_Draw_Types_Below_0x40(type, dsto, pos_enum);
  } else if (type == kDoorType_ExitToOw) {
    dung_exit_door_addresses[dung_exit_door_count >> 1] = dsto * 2;
    dung_exit_door_count += 2;
  } else if (type == kDoorType_WaterfallTunnel) {
    Door_Up_Draw_Types_Below_0x40(type, dsto, pos_enum);
    Door_PrioritizeCurDoor();
  } else if (type >= kDoorType_StairMaskLocked0 && type <= kDoorType_StairMaskLocked3) {
    Door_Up_StairMaskLocked(type, dsto);
  } else if (type >= kDoorType_RegularDoor33)
    Door_Up_Draw_Types_Above_0x40(type, dsto, pos_enum);
  else
    Door_Up_Draw_Types_Below_0x40(type, dsto, pos_enum);

}

void Door_Down(int type, int pos_enum) {
  uint16 dsto = kDoorPositionToTilemapOffs_Down[pos_enum] / 2;
  if (type == kDoorType_PlayerBgChange)
    Door_AddFloorToggleProperty(dsto + XY(1, 4));
  else if (type == kDoorType_EntranceDoor)
    Door_Down_EntranceDoor(dsto);
  else if (type == kDoorType_ThroneRoom)
    Door_AddPalaceToggleProperty(dsto + XY(1, 4));
  else if (type == kDoorType_ExitToOw) {
    dung_exit_door_addresses[dung_exit_door_count >> 1] = dsto * 2;
    dung_exit_door_count += 2;
  } else if (type >= kDoorType_RegularDoor33) {
    Door_Down_Draw_Types_Above_0x40(type, dsto);
  } else if (type == kDoorType_EntranceLarge) {
    Door_Register(1, type, dsto);
    Object_Draw8xN_BG2(10, SrcPtr(0x2656), dsto + XY(-3, -4));
  } else if (type == kDoorType_EntranceLarge2) {
    dsto |= 0x1000;
    Door_Register(1, type, dsto);
    dsto += XY(-3, -4);
    Object_Draw8xN_BG2(10, SrcPtr(0x2656), dsto);
    dsto += -0x1000 + XY(0, 7);
    for (int i = 0; i < 10; i++) {
      dung_bg2[dsto] = dung_bg1[dsto] | 0x2000;
      dsto += 1;
    }
  } else if (type == kDoorType_EntranceCave || type == kDoorType_EntranceCave2) {
    if (type == kDoorType_EntranceCave2)
      Door_Prioritize4x7(dsto + XY(0, 4));
    Door_Register(1, type, dsto);
    Object_Draw4x4(SrcPtr(0x26f6), DstoPtr(dsto));
  } else if (type == kDoorType_4) {
    uint16 dsto_org = dsto;
    dsto |= 0x1000;
    Door_Prioritize4x7(dsto + XY(0, 4));
    Door_Register(1, type, dsto);
    Object_Draw4x4(SrcPtr(0x26f6), DstoPtr(dsto));
    for (int i = 0; i < 4; i++) {
      dung_bg2[dsto_org + XY(0, 3)] = dung_bg1[dsto_org + XY(0, 3)] | 0x2000;
      dsto_org += 1;
    }
  } else {
    Door_Down_Draw_Helper_2(type, dsto);
  }
}


void Door_Right_Helper1(uint8 door_type, uint16 dsto) {
  int t = Door_Register(3, door_type, dsto), new_type;
  if (t & 0x100)
    return;
  if ((new_type = kDoorType_Regular, t == kDoorType_36) ||
      (new_type = kDoorType_ShuttersTwoWay, t == kDoorType_38)) {
    int i = (dung_cur_door_idx >> 1) - 1;
    door_type_and_slot[i] = (i << 8) | new_type;
    t = new_type;
  }
  const uint16 *src = SrcPtr(kDoorTypeSrcData4[t >> 1]);
  uint16 *dst = DstoPtr(dsto) + 1;
  for (int i = 0; i < 3; i++) {
    dst[XY(0, 0)] = src[0];
    dst[XY(0, 1)] = src[1];
    dst[XY(0, 2)] = src[2];
    dst[XY(0, 3)] = src[3];
    dst++, src += 4;
  }
}

void Door_Right_AddNormalDoor(uint8 door_type, uint16 dsto) {
  if (door_type == kDoorType_Regular2)
    Door_Prioritize5x4(dsto + XY(4, 0));
  if (door_type == kDoorType_WaterfallTunnel) {
    Door_Right_Helper1(door_type, dsto);
    Door_PrioritizeCurDoor();
  } else {
    Door_Right_Helper1(door_type, dsto);
  }
}

void Door_Right_AddDoorAbove0x40(uint8 door_type, uint16 dsto) {
  uint8 t = Door_Register(3, door_type, dsto), new_type;
  if ((new_type = kDoorType_RegularDoor33, t == kDoorType_ShutterTrapUR) ||
      (new_type = kDoorType_Shutter, t == kDoorType_ShutterTrapDL)) {
    int i = (dung_cur_door_idx >> 1) - 1;
    door_type_and_slot[i] = (i << 8) | new_type;
    t = new_type;
  }

  uint16 dst_org = dsto;
  const uint16 *src = SrcPtr(kDoorTypeSrcData4[t >> 1]);
  for (int i = 0; i < 2; i++) {
    dung_bg1[dsto + XY(1, 0)] = src[0];
    dung_bg1[dsto + XY(1, 1)] = src[1];
    dung_bg1[dsto + XY(1, 2)] = src[2];
    dung_bg1[dsto + XY(1, 3)] = src[3];
    dsto++, src += 4;
  }
  dung_bg2[dsto + XY(1, 0)] = src[0];
  dung_bg2[dsto + XY(1, 1)] = src[1];
  dung_bg2[dsto + XY(1, 2)] = src[2];
  dung_bg2[dsto + XY(1, 3)] = src[3];
  Door_PrioritizeAligned(dst_org + XY(4, 0));
  Door_PrioritizeCurDoor();
}

void Door_Left_AddNormalDoor(uint8 door_type, uint16 dsto, int pos_enum) {
  if (pos_enum >= 6) {
    uint16 bak = dung_cur_door_idx;
    dung_cur_door_idx |= 0x10;
    Door_Right_AddNormalDoor(door_type, kDoorPositionToTilemapOffs_Right[pos_enum - 6] / 2);
    dung_cur_door_idx = bak;
  }

  int t = Door_Register(2, door_type, dsto), new_type;
  if (t & 0x100)
    return;

  if ((new_type = kDoorType_ShuttersTwoWay, t == kDoorType_36) ||
      (new_type = kDoorType_Regular, t == kDoorType_38)) {
    int i = (dung_cur_door_idx >> 1) - 1;
    door_type_and_slot[i] = (i << 8) | new_type;
    t = new_type;
  }

  const uint16 *src = SrcPtr(kDoorTypeSrcData3[t >> 1]);
  uint16 *dst = DstoPtr(dsto);
  for (int i = 0; i < 3; i++) {
    dst[XY(0, 0)] = src[0];
    dst[XY(0, 1)] = src[1];
    dst[XY(0, 2)] = src[2];
    dst[XY(0, 3)] = src[3];
    dst++, src += 4;
  }

}

void Door_Left_AddDoorAbove0x40(uint8 door_type, uint16 dsto, int pos_enum) {
  if (pos_enum >= 6) {
    uint16 bak = dung_cur_door_idx;
    dung_cur_door_idx |= 0x10;
    Door_Right_AddDoorAbove0x40(door_type, kDoorPositionToTilemapOffs_Right[pos_enum - 6] / 2);
    dung_cur_door_idx = bak;
  }

  uint8 t = Door_Register(2, door_type, dsto), new_type;
  if ((new_type = kDoorType_Shutter, t == kDoorType_ShutterTrapUR) ||
      (new_type = kDoorType_RegularDoor33, t == kDoorType_ShutterTrapDL)) {
    int i = (dung_cur_door_idx >> 1) - 1;
    door_type_and_slot[i] = (i << 8) | new_type;
    t = new_type;
  }

  const uint16 *src = SrcPtr(kDoorTypeSrcData3[t >> 1]);
  uint16 dsto_org = dsto;
  dung_bg2[dsto + XY(0, 0)] = src[0];
  dung_bg2[dsto + XY(0, 1)] = src[1];
  dung_bg2[dsto + XY(0, 2)] = src[2];
  dung_bg2[dsto + XY(0, 3)] = src[3];
  dsto++, src += 4;
  for (int i = 0; i < 2; i++) {
    dung_bg1[dsto + XY(0, 0)] = src[0];
    dung_bg1[dsto + XY(0, 1)] = src[1];
    dung_bg1[dsto + XY(0, 2)] = src[2];
    dung_bg1[dsto + XY(0, 3)] = src[3];
    dsto++, src += 4;
  }
  Door_Helper_SetPriority3(dsto_org);
  Door_PrioritizeCurDoor();
}


void Door_Left(int type, int pos_enum) {
  uint16 dsto = kDoorPositionToTilemapOffs_Left[pos_enum] / 2;
  if (type == kDoorType_PlayerBgChange)
    Door_AddFloorToggleProperty(dsto + XY(-2, 1));
  else if (type == kDoorType_EntranceDoor)
    Door_Left_EntranceDoor(dsto);
  else if (type == kDoorType_ThroneRoom)
    Door_AddPalaceToggleProperty(dsto + XY(-2, 1));
  else if (type == kDoorType_Regular2) {
    Door_Prioritize5x4(dsto & ~0x1f);
    Door_Left_AddNormalDoor(type, dsto, pos_enum);
  } else if (type == kDoorType_WaterfallTunnel) {
    Door_Left_AddNormalDoor(type, dsto, pos_enum);
    Door_PrioritizeCurDoor();
  } else if (type < kDoorType_RegularDoor33) {
    Door_Left_AddNormalDoor(type, dsto, pos_enum);
  } else {
    Door_Left_AddDoorAbove0x40(type, dsto, pos_enum);
  }
}

void Door_Right(int type, int pos_enum) {
  uint16 dsto = kDoorPositionToTilemapOffs_Right[pos_enum] / 2;
  if (type == kDoorType_PlayerBgChange)
    Door_AddFloorToggleProperty(dsto + XY(4, 1));
  else if (type == kDoorType_EntranceDoor)
    Door_Right_EntranceDoor(dsto);
  else if (type == kDoorType_ThroneRoom)
    Door_AddPalaceToggleProperty(dsto + XY(4, 1));
  else if (type < kDoorType_RegularDoor33) {
    Door_Right_AddNormalDoor(type, dsto);
  } else {
    Door_Right_AddDoorAbove0x40(type, dsto);
  }
}

void Dungeon_LoadDoor(uint16 a) {
  uint8 door_type = a >> 8;
  uint8 position = a >> 4 & 0xf;

  switch (a & 3) {
  case 0: Door_Up(door_type, position); break;
  case 1: Door_Down(door_type, position); break;
  case 2: Door_Left(door_type, position); break;
  case 3: Door_Right(door_type, position); break;
  }
}

void Dungeon_DrawObjects(const uint8 *level_data) {
  for (;;) {
    dung_draw_width_indicator = dung_draw_height_indicator = 0;
    uint16 d = WORD(level_data[dung_load_ptr_offs]);
    if (d == 0xffff)
      return;
    if (d == 0xfff0)
      break;
    Dungeon_LoadType1Object(d, level_data);
  }
  for (;;) {
    dung_load_ptr_offs += 2;
    uint16 d = WORD(level_data[dung_load_ptr_offs]);
    if (d == 0xffff)
      return;
    Dungeon_LoadDoor(d);
  }
}

#define adjacent_doors_flags (*(uint16*)(g_ram+0x1100))
#define adjacent_doors ((uint16*)(g_ram+0x1110))

const uint16 *GetRoomDoorInfo(int room) {
  return (uint16 *)(kDungeonRoom + kDungeonRoomDoorOffs[room]);
}

const uint8 *GetRoomHeaderPtr(int room) {
  return kDungeonRoomHeaders + kDungeonRoomHeadersOffs[room];
}

const uint8 *GetDefaultRoomLayout(int i) {
  return kDungeonRoomDefault + kDungeonRoomDefaultOffs[i];
}

const uint8 *GetDungeonRoomLayout(int i) {
  return kDungeonRoom + kDungeonRoomOffs[i];
}

void Dungeon_LoadAdjacentRoomDoors(int room) {
  const uint16 *dp = GetRoomDoorInfo(room);
  adjacent_doors_flags = (save_dung_info[room] & 0xf000) | 0xf00;
  for (int i = 0; ; i++) {
    uint16 a = dp[i];
    adjacent_doors[i] = a;
    if (a == 0xffff)
      break;
    if ((a & 0xff00) == 0x4000 || (a & 0xff00) < 0x200)
      adjacent_doors_flags |= kUpperBitmasks[i];
  }
}

void Dungeon_CheckAdjacentRoomOpenedDoors(int idx, int room) {
  static const uint16 kLookup[] = {
    0x00, 0x10, 0x20, 0x30, 0x40, 0x50,
    0x61, 0x71, 0x81, 0x91, 0xa1, 0xb1,
    0x02, 0x12, 0x22, 0x32, 0x42, 0x52,
    0x63, 0x73, 0x83, 0x93, 0xa3, 0xb3,
  };
  static const uint16 kLookup2[] = {
    0x61, 0x71, 0x81, 0x91, 0xa1, 0xb1,
    0x0, 0x10, 0x20, 0x30, 0x40, 0x50,
    0x63, 0x73, 0x83, 0x93, 0xa3, 0xb3,
    0x02, 0x12, 0x22, 0x32, 0x42, 0x52,
  };
  Dungeon_LoadAdjacentRoomDoors(room);
  int i, j;
  uint16 a;
  for (i = 0; i != 8 && (a = adjacent_doors[i]) != 0xffff; i++) {
    a &= 0xff;
    j = idx;
    if (a == kLookup[j] || a == kLookup[++j] || a == kLookup[++j] || a == kLookup[++j] || a == kLookup[++j] || a == kLookup[++j]) {
      uint8 rev = kLookup2[j];
      for (j = 0; j != 8; j++) {
        if ((uint8)dung_door_tilemap_address[j] == rev) {
          uint8 k = dung_door_tilemap_address[j] >> 8;
          if (k == 0x30)
            break;
          if (k == 0x44 || k == 0x18) {
            // trapdoor
            if (room != dungeon_room_index_prev)
              break;
            dung_flag_trapdoors_down = 0;
          } else {
            // not trapdoor
            if (!(adjacent_doors_flags & kUpperBitmasks[i]))
              break;
          }
          dung_door_opened_incl_adjacent |= kUpperBitmasks[j];
          break;
        }
      }
    }
  }
}

static const DungPalInfo kDungPalinfos[41] = {
  { 0,  0,  3,  1},
  { 2,  0,  3,  1},
  { 4,  0, 10,  1},
  { 6,  0,  1,  7},
  {10,  2,  2,  7},
  { 4,  4,  3, 10},
  {12,  5,  8, 20},
  {14,  0,  3, 10},
  { 2,  0, 15, 20},
  {10,  2,  0,  7},
  { 2,  0, 15, 12},
  { 6,  0,  6,  7},
  { 0,  0, 14, 18},
  {18,  5,  5, 11},
  {18,  0,  2, 12},
  {16,  5, 10,  7},
  {16,  0, 16, 12},
  {22,  7,  2,  7},
  {22,  0,  7, 15},
  { 8,  0,  4, 12},
  { 8,  0,  4,  9},
  { 4,  0,  3,  1},
  {20,  0,  4,  4},
  {20,  0, 20, 12},
  {24,  5,  7, 11},
  {24,  6, 16, 12},
  {26,  5,  8, 20},
  {26,  2,  0,  7},
  { 6,  0,  3, 10},
  {28,  0,  3,  1},
  {30,  0, 11, 17},
  { 4,  0, 11, 17},
  {14,  0,  0,  2},
  {32,  8, 19, 13},
  {10,  0,  3, 10},
  {20,  0,  4,  4},
  {26,  2,  2,  7},
  {26, 10,  0,  0},
  { 0,  0,  3,  2},
  {14,  0,  3,  7},
  {26,  5,  5, 11},
};

void Dungeon_LoadHeader() {
  dung_flag_statechange_waterpuzzle = 0;
  dung_flag_somaria_block_switch = 0;
  dung_flag_movable_block_was_pushed = 0;

  static const int16 kAdjustment[] = { 256, -256 };

  if (submodule_index == 0) {
    dung_loade_bgoffs_h_copy = BG2HOFS_copy2 & ~0x1FF;
    dung_loade_bgoffs_v_copy = BG2VOFS_copy2 & ~0x1FF;
  } else if (submodule_index == 21 || submodule_index < 18 && submodule_index >= 6) {
    dung_loade_bgoffs_h_copy = (BG2HOFS_copy2 + 0x20) & ~0x1FF;
    dung_loade_bgoffs_v_copy = (BG2VOFS_copy2 + 0x20) & ~0x1FF;
  } else {
    if (((link_direction & 0xf) >> 1) < 2) {
      dung_loade_bgoffs_h_copy = (BG2HOFS_copy2 + kAdjustment[(link_direction & 0xf) >> 1]) & ~0x1FF;
      dung_loade_bgoffs_v_copy = (BG2VOFS_copy2 + 0x20) & ~0x1FF;
    } else {
      dung_loade_bgoffs_h_copy = (BG2HOFS_copy2 + 0x20) & ~0x1FF;
      dung_loade_bgoffs_v_copy = (BG2VOFS_copy2 + kAdjustment[(link_direction & 0xf) >> 3]) & ~0x1FF;
    }
  }

  const uint8 *hdr_ptr = GetRoomHeaderPtr(dungeon_room_index);

  dung_bg2_properties_backup = dung_hdr_bg2_properties;
  dung_hdr_bg2_properties = hdr_ptr[0] >> 5;
  dung_hdr_collision = (hdr_ptr[0] >> 2) & 7;
  dung_want_lights_out_copy = dung_want_lights_out;
  dung_want_lights_out = hdr_ptr[0] & 1;
  const DungPalInfo *dpi = &kDungPalinfos[hdr_ptr[1]];
  dung_hdr_palette_1 = dpi->pal0;
  overworld_palette_sp0 = dpi->pal1;
  sprite_aux1_palette = dpi->pal2;
  sprite_aux2_palette = dpi->pal3;
  aux_tile_theme_index = hdr_ptr[2];
  sprite_graphics_index = hdr_ptr[3] + 0x40;
  dung_hdr_collision_2 = hdr_ptr[4];
  dung_hdr_tag[0] = hdr_ptr[5];
  dung_hdr_tag[1] = hdr_ptr[6];
  dung_hdr_hole_teleporter_plane = hdr_ptr[7] & 3;
  dung_hdr_staircase_plane[0] = (hdr_ptr[7] >> 2) & 3;
  dung_hdr_staircase_plane[1] = (hdr_ptr[7] >> 4) & 3;
  dung_hdr_staircase_plane[2] = (hdr_ptr[7] >> 6) & 3;
  dung_hdr_staircase_plane[3] = hdr_ptr[8] & 3;
  dung_hdr_travel_destinations[0] = hdr_ptr[9];
  dung_hdr_travel_destinations[1] = hdr_ptr[10];
  dung_hdr_travel_destinations[2] = hdr_ptr[11];
  dung_hdr_travel_destinations[3] = hdr_ptr[12];
  dung_hdr_travel_destinations[4] = hdr_ptr[13];
  dung_flag_trapdoors_down = 1;
  dung_overlay_to_load = 0;
  dung_index_x3 = dungeon_room_index * 3;

  uint16 x = save_dung_info[dungeon_room_index];
  dung_door_opened = x & 0xf000;
  dung_door_opened_incl_adjacent = dung_door_opened | 0xf00;
  dung_savegame_state_bits = (x & 0xff0) << 4;
  dung_quadrants_visited = x & 0xf;

  const uint16 *dp = GetRoomDoorInfo(dungeon_room_index);
  int i = 0;
  for (; dp[i] != 0xffff; i++)
    dung_door_tilemap_address[i] = dp[i];
  dung_door_tilemap_address[i] = 0;

  if (((dungeon_room_index - 1) & 0xf) != 0xf)
    Dungeon_CheckAdjacentRoomOpenedDoors(18, dungeon_room_index - 1);
  if (((dungeon_room_index + 1) & 0xf) != 0)
    Dungeon_CheckAdjacentRoomOpenedDoors(12, dungeon_room_index + 1);
  if (dungeon_room_index - 16 >= 0)
    Dungeon_CheckAdjacentRoomOpenedDoors(6, dungeon_room_index - 16);
  if (dungeon_room_index + 16 < 0x140)
    Dungeon_CheckAdjacentRoomOpenedDoors(0, dungeon_room_index + 16);
}

void Dung_FillFloor(const uint16 *src) {
  static const uint16 kDungeon_QuadrantOffsets[] = { 0x0, 0x40, 0x1000, 0x1040 };
  for (int i = 0; i < 4; i++) {
    uint16 *dst = DstoPtr(kDungeon_QuadrantOffsets[i] / 2);
    for (int j = 0; j < 8; j++) {
      Object_Draw_N_4x4(8, src, dst);
      dst += XY(0, 4);
    }
  }
}

// these are not used by the code, but needed for the comparison with the real rom to work.
static const uint8 kDungeon_DrawObjectOffsets_BG1[33] = {
     0, 0x20, 0x7e,    2, 0x20, 0x7e,    4, 0x20, 0x7e,    6, 0x20, 0x7e, 0x80, 0x20, 0x7e, 0x82,
  0x20, 0x7e, 0x84, 0x20, 0x7e, 0x86, 0x20, 0x7e,    0, 0x21, 0x7e, 0x80, 0x21, 0x7e,    0, 0x22,
  0x7e,
};
static const uint8 kDungeon_DrawObjectOffsets_BG2[33] = {
     0, 0x40, 0x7e,    2, 0x40, 0x7e,    4, 0x40, 0x7e,    6, 0x40, 0x7e, 0x80, 0x40, 0x7e, 0x82,
  0x40, 0x7e, 0x84, 0x40, 0x7e, 0x86, 0x40, 0x7e,    0, 0x41, 0x7e, 0x80, 0x41, 0x7e,    0, 0x42,
  0x7e,
};

void Dungeon_DrawFloors(const uint8 *level_data) {
  memcpy(&dung_line_ptrs_row0, kDungeon_DrawObjectOffsets_BG2, 33);
  uint8 ft = level_data[dung_load_ptr_offs++];
  dung_floor_1_filler_tiles = ft & 0xf0;
  Dung_FillFloor(SrcPtr(dung_floor_1_filler_tiles));
  memcpy(&dung_line_ptrs_row0, kDungeon_DrawObjectOffsets_BG1, 33);
  dung_floor_2_filler_tiles = (ft & 0xf) << 4;
  Dung_FillFloor(SrcPtr(dung_floor_2_filler_tiles));
}

void Dungeon_LoadBlock(uint16 dsto_x2, uint16 slot) {
  int x = dung_misc_objs_index >> 1;
  dung_misc_objs_index += 2;
  dung_replacement_tile_state[x] = 0;
  dung_object_pos_in_objdata[x] = slot;
  dung_object_tilemap_pos[x] = dsto_x2;
  uint16 *dst = DstoPtr((dsto_x2 >> 1) & 0x1fff);
  replacement_tilemap_UL[x] = dst[XY(0, 0)];
  replacement_tilemap_LL[x] = dst[XY(0, 1)];
  replacement_tilemap_UR[x] = dst[XY(1, 0)];
  replacement_tilemap_LR[x] = dst[XY(1, 1)];
  Object_Draw_2x2(SrcPtr(0xe52), dst);
}

void Dungeon_LoadTorch(uint16 dsto_x2, uint16 slot) {
  int x = dung_index_of_torches >> 1;
  dung_index_of_torches += 2;
  dung_object_tilemap_pos[x] = dsto_x2;
  dung_object_pos_in_objdata[x] = slot;
  uint16 src_img = 0xec2;
  uint16 *dst = DstoPtr((dsto_x2 >> 1) & 0x1fff);
  if (dsto_x2 & 0x8000) {
    src_img = 0xeca;
    if (dung_num_lit_torches < 3)
      dung_num_lit_torches++;
  }
  Object_Draw_2x2(SrcPtr(src_img), dst);
}

void Dungeon_LoadRoom() {
  Dungeon_LoadHeader();
  dung_unk6 = 0;
  
  dung_hdr_collision_2_mirror = dung_hdr_collision_2;
  dung_hdr_collision_2_mirror_PADDING = dung_hdr_tag[0];
  dung_some_subpixel[0] = 0x30;
  dung_some_subpixel[1] = 0xff;
  dung_floor_move_flags = 0;
  word_7E0420 = 0;
  dung_floor_x_vel = dung_floor_y_vel = 0;
  dung_floor_x_offs = dung_floor_y_offs = 0;
  invisible_door_dir_and_index_x2 = 0xffff;
  dung_blastwall_flag_x = dung_blastwall_flag_y = 0;
  dung_unk_blast_walls_2 = dung_unk_blast_walls_3 = 0;
  water_hdma_var5 = 0;
  dung_num_toggle_floor = 0;
  dung_num_toggle_palace = 0;
  dung_unk2 = 0;
  dung_cur_quadrant_upload = 0;
  dung_num_inter_room_upnorth_stairs = 0;
  dung_num_inter_room_southdown_stairs = 0;
  dung_num_inroom_upnorth_stairs = 0;
  dung_num_inroom_southdown_stairs = 0;
  dung_num_interpseudo_upnorth_stairs = 0;
  dung_num_inroom_upnorth_stairs_water = 0;
  dung_num_activated_water_ladders = 0;
  dung_num_water_ladders = 0;
  dung_some_stairs_unk4 = 0;
  dung_num_stairs_1 = 0;
  dung_num_stairs_2 = 0;
  dung_num_stairs_wet = 0;
  dung_num_inroom_upsouth_stairs_water = 0;
  dung_num_wall_upnorth_spiral_stairs = 0;
  dung_num_wall_downnorth_spiral_stairs = 0;
  dung_num_wall_upnorth_spiral_stairs_2 = 0;
  dung_num_wall_downnorth_spiral_stairs_2 = 0;
  dung_num_inter_room_upnorth_straight_stairs = 0;
  dung_num_inter_room_upsouth_straight_stairs = 0;
  dung_num_inter_room_downnorth_straight_stairs = 0;
  dung_num_inter_room_downsouth_straight_stairs = 0;
  dung_exit_door_addresses[0] = 0;
  dung_exit_door_addresses[1] = 0;
  dung_exit_door_addresses[2] = 0;
  dung_exit_door_addresses[3] = 0;
  dung_exit_door_count = 0;
  dung_door_switch_triggered = 0;
  dung_num_star_shaped_switches = 0;
  dung_misc_objs_index = 0;
  dung_index_of_torches = 0;
  dung_num_chests_x2 = 0;
  dung_num_bigkey_locks_x2 = 0;
  dung_unk5 = 0;
  dung_cur_door_idx = 0;

  for (int i = 0; i < 16; i++) {
    dung_door_tilemap_address[i] = 0;
    door_type_and_slot[i] = 0;
    dung_door_direction[i] = 0;
    dung_torch_timers[i] = 0;
    dung_replacement_tile_state[i] = 0;
    dung_object_pos_in_objdata[i] = 0;
    dung_object_tilemap_pos[i] = 0;
  }

  const uint8 *cur_p0 = GetDungeonRoomLayout(dungeon_room_index);
  dung_load_ptr_offs = 0;
  Dungeon_DrawFloors(cur_p0);

  uint16 old_offs = dung_load_ptr_offs;
  dung_layout_and_starting_quadrant = cur_p0[dung_load_ptr_offs];

  const uint8 *cur_p1 = GetDefaultRoomLayout(dung_layout_and_starting_quadrant >> 2);

  dung_load_ptr_offs = 0;
  Dungeon_DrawObjects(cur_p1);

  dung_load_ptr_offs = old_offs + 1;

  Dungeon_DrawObjects(cur_p0);  // Draw Layer 1 objects to BG2
  dung_load_ptr_offs += 2;

  memcpy(&dung_line_ptrs_row0, kDungeon_DrawObjectOffsets_BG2, 33);
  Dungeon_DrawObjects(cur_p0);  // Draw Layer 2 objects to BG2
  dung_load_ptr_offs += 2;

  memcpy(&dung_line_ptrs_row0, kDungeon_DrawObjectOffsets_BG1, 33);
  Dungeon_DrawObjects(cur_p0);  // Draw Layer 3 objects to BG2

  for (dung_load_ptr_offs = 0; dung_load_ptr_offs != 0x18C; dung_load_ptr_offs += 4) {
    MovableBlockData &m = movable_block_datas[dung_load_ptr_offs >> 2];
    if (m.room == dungeon_room_index)
      Dungeon_LoadBlock(m.tilemap, dung_load_ptr_offs);
  }

  uint16 t;

  dung_index_of_torches = dung_index_of_torches_start = dung_misc_objs_index;
  int i = 0;
  do {
    if (dung_torch_data[i >> 1] == dungeon_room_index) {
      i += 2;

      do {
        t = dung_torch_data[i >> 1];
        i += 2;
        Dungeon_LoadTorch(t, i - 2);
      } while (dung_torch_data[i >> 1] != 0xffff);
      break;
    }
    i += 2;
    do {
      t = dung_torch_data[i >> 1];
      i += 2;
    } while (t != 0xffff);
  } while (i != 0x120);

  dung_load_ptr_offs = 0x120;
}

static const uint16 kUploadBgSrcs[] = { 0x0, 0x1000, 0x0, 0x40, 0x40, 0x1040, 0x1000, 0x1040, 0x1000, 0x0, 0x40, 0x0, 0x1040, 0x40, 0x1040, 0x1000 };
static const uint8 kUploadBgDsts[] = { 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15, 4, 8, 12, 16 };

void Dungeon_Upload_BG1() {
  int ofs = (overworld_screen_transition & 0xf) + dung_cur_quadrant_upload;
  uint16 *src = &dung_bg1[kUploadBgSrcs[ofs] / 2];
  int p = 0;
  do {
    UploadVram_32x32 *d = (UploadVram_32x32 *)&g_ram[0x1000 + p * 2];
    do {
      d->row[0].col[0] = src[XY(0, 0)];
      d->row[0].col[1] = src[XY(1, 0)];
      d->row[1].col[0] = src[XY(0, 1)];
      d->row[1].col[1] = src[XY(1, 1)];
      d->row[2].col[0] = src[XY(0, 2)];
      d->row[2].col[1] = src[XY(1, 2)];
      d->row[3].col[0] = src[XY(0, 3)];
      d->row[3].col[1] = src[XY(1, 3)];
      d = (UploadVram_32x32 *)((uint16 *)d + 2);
      src += 2, p += 2;
    } while (p & 0x1f);
    src += 224;
    p += 128 - 32;
  } while (p != 0x400);
  BYTE(nmi_load_target_addr) = kUploadBgDsts[ofs] + 0x10;
  nmi_subroutine_index = 1;
  nmi_disable_core_updates = 1;
}

void Dungeon_Upload_BG2() {
  int ofs = (overworld_screen_transition & 0xf) + dung_cur_quadrant_upload;
  uint16 *src = &dung_bg2[kUploadBgSrcs[ofs] / 2];
  int p = 0;
  do {
    UploadVram_32x32 *d = (UploadVram_32x32 *)&g_ram[0x1000 + p * 2];
    do {
      d->row[0].col[0] = src[XY(0, 0)];
      d->row[0].col[1] = src[XY(1, 0)];
      d->row[1].col[0] = src[XY(0, 1)];
      d->row[1].col[1] = src[XY(1, 1)];
      d->row[2].col[0] = src[XY(0, 2)];
      d->row[2].col[1] = src[XY(1, 2)];
      d->row[3].col[0] = src[XY(0, 3)];
      d->row[3].col[1] = src[XY(1, 3)];
      d = (UploadVram_32x32 *)((uint16 *)d + 2);
      src += 2, p += 2;
    } while (p & 0x1f);
    src += 224;
    p += 128 - 32;
  } while (p != 0x400);
  dung_cur_quadrant_upload += 4;
  BYTE(nmi_load_target_addr) = kUploadBgDsts[ofs];
  nmi_subroutine_index = 1;
  nmi_disable_core_updates = 1;
}

void Dungeon_Upload_BG1_Outer() {

  // It never seems to be 25 here, so skip updating water stuff
  assert(dung_hdr_tag[0] != 25);
  Dungeon_Upload_BG1();
}

// This gets called when entering a dungeon from ow.
void Dungeon_LoadAndUploadRoom() {
  int bak = HDMAEN_copy;
  zelda_snes_dummy_write(HDMAEN, 0);
  HDMAEN_copy = 0;
  Dungeon_LoadRoom();
  overworld_screen_transition = 0;
  overworld_map_state = 0;
  for (dung_cur_quadrant_upload = 0; dung_cur_quadrant_upload != 16; ) {
    Dungeon_Upload_BG1();
    NMI_UploadTilemap();
    Dungeon_Upload_BG2();
    NMI_UploadTilemap();
  }
  HDMAEN_copy = bak;
  nmi_subroutine_index = 0;
  overworld_map_state = 0;
  subsubmodule_index = 0;
}

void Dungeon_LoadBasicAttr_full(uint16 loops) {
  do {
    int i = dung_draw_width_indicator / 2;
    uint8 a0 = attributes_for_tile[dung_bg2[i] & 0x3ff];
    if (a0 >= 0x10 && a0 < 0x1c)
      a0 |= (dung_bg2[i] >> 14);  // vflip/hflip
    uint8 a1 = attributes_for_tile[dung_bg2[i + 1] & 0x3ff];
    if (a1 >= 0x10 && a1 < 0x1c)
      a1 |= (dung_bg2[i + 1] >> 14);  // vflip/hflip
    int j = dung_draw_height_indicator;
    dung_bg2_attr_table[j] = a0;
    dung_bg2_attr_table[j + 1] = a1;
    dung_draw_height_indicator = j + 2;
    dung_draw_width_indicator += 4;
  } while (--loops);
  if (dung_draw_height_indicator == 0x2000)
    overworld_map_state++;
}

static inline void WriteAttr1(int j, uint16 attr) {
  dung_bg1_attr_table[j + 0] = attr;
  dung_bg1_attr_table[j + 1] = attr >> 8;
}

static inline void WriteAttr2(int j, uint16 attr) {
  dung_bg2_attr_table[j + 0] = attr;
  dung_bg2_attr_table[j + 1] = attr >> 8;
}

void Dungeon_LoadObjAttr() {
  for (int i = 0; i != dung_num_star_shaped_switches; i += 2) {
    int j = star_shaped_switches_tile[i >> 1];
    WriteAttr2(j + XY(0, 0), 0x3b3b);
    WriteAttr2(j + XY(0, 1), 0x3b3b);
  }

  int i = 0, t = 0x3030;
  for (; i != dung_num_inter_room_upnorth_stairs; i += 2, t += 0x101) {
    int j = dung_inter_starcases[i >> 1];
    WriteAttr2(j + XY(1, 2), 0);
    WriteAttr2(j + XY(1, 0), 0x2626);
    WriteAttr2(j + XY(1, 1), t);
  }
  for (; i != dung_num_wall_upnorth_spiral_stairs; i += 2, t += 0x101) {
    int j = dung_inter_starcases[i >> 1];
    WriteAttr2(j + XY(1, 0), 0x5e5e);
    WriteAttr2(j + XY(1, 2), 0x5e5e);
    WriteAttr2(j + XY(1, 3), 0x5e5e);
    WriteAttr2(j + XY(1, 1), t);
  }
  for (; i != dung_num_wall_upnorth_spiral_stairs_2; i += 2, t += 0x101) {
    int j = dung_inter_starcases[i >> 1];
    WriteAttr2(j + XY(1, 0), 0x5f5f);
    WriteAttr2(j + XY(1, 2), 0x5f5f);
    WriteAttr2(j + XY(1, 3), 0x5f5f);
    WriteAttr2(j + XY(1, 1), t);
  }
  for (; i != dung_num_inter_room_upnorth_straight_stairs; i += 2, t += 0x101) {
    int j = dung_inter_starcases[i >> 1];
    WriteAttr2(j + XY(1, 0), 0x3838);
    WriteAttr2(j + XY(1, 2), 0);
    WriteAttr2(j + XY(1, 3), 0);
    WriteAttr2(j + XY(1, 1), t);
  }
  for (; i != dung_num_inter_room_upsouth_straight_stairs; i += 2, t += 0x101) {
    int j = dung_inter_starcases[i >> 1];
    WriteAttr2(j + XY(1, 0), 0);
    WriteAttr2(j + XY(1, 1), 0);
    WriteAttr2(j + XY(1, 2), t);
    WriteAttr2(j + XY(1, 3), 0x3939);
  }
  t = (t & 0x707) | 0x3434;
  for (; i != dung_num_inter_room_southdown_stairs; i += 2, t += 0x101) {
    int j = dung_inter_starcases[i >> 1];
    WriteAttr2(j + XY(1, 2), t);
    WriteAttr2(j + XY(1, 3), 0x2626);
  }
  for (; i != dung_num_wall_downnorth_spiral_stairs; i += 2, t += 0x101) {
    int j = dung_inter_starcases[i >> 1];
    WriteAttr2(j + XY(1, 0), 0x5e5e);
    WriteAttr2(j + XY(1, 1), t);
    WriteAttr2(j + XY(1, 2), 0x5e5e);
    WriteAttr2(j + XY(1, 3), 0x5e5e);
  }
  for (; i != dung_num_wall_downnorth_spiral_stairs_2; i += 2, t += 0x101) {
    int j = dung_inter_starcases[i >> 1];
    WriteAttr2(j + XY(1, 0), 0x5f5f);
    WriteAttr2(j + XY(1, 1), t);
    WriteAttr2(j + XY(1, 2), 0x5f5f);
    WriteAttr2(j + XY(1, 3), 0x5f5f);
  }
  for (; i != dung_num_inter_room_downnorth_straight_stairs; i += 2, t += 0x101) {
    int j = dung_inter_starcases[i >> 1];
    WriteAttr2(j + XY(1, 0), 0x3838);
    WriteAttr2(j + XY(1, 1), t);
    WriteAttr2(j + XY(1, 2), 0);
    WriteAttr2(j + XY(1, 3), 0);
  }
  for (; i != dung_num_inter_room_downsouth_straight_stairs; i += 2, t += 0x101) {
    int j = dung_inter_starcases[i >> 1];
    WriteAttr2(j + XY(1, 0), 0);
    WriteAttr2(j + XY(1, 1), 0);
    WriteAttr2(j + XY(1, 2), t);
    WriteAttr2(j + XY(1, 3), 0x3939);
  }

  i = 0;
  int type = 0, iend = dung_num_inroom_upnorth_stairs;
  uint16 attr = 0x1f1f;
  if (iend == 0) {
    type = 1, attr = 0x1e1e;
    iend = dung_num_inroom_southdown_stairs;
    if (iend == 0) {
      type = 2, attr = 0x1d1d;
      iend = dung_num_interpseudo_upnorth_stairs;
      if (iend == 0)
        goto skip3;
    }
  }
  kind_of_in_room_staircase = type;
  for (; i != iend; i += 2) {
    int j = dung_stairs_table_1[i >> 1];
    WriteAttr2(j + XY(0, 0), 0x02);
    WriteAttr1(j + XY(0, 3), 0x02);
    WriteAttr2(j + XY(2, 0), 0x0200);
    WriteAttr1(j + XY(2, 3), 0x0200);
    WriteAttr2(j + XY(0, 1), 0x01);
    WriteAttr1(j + XY(0, 2), 0x01);
    WriteAttr2(j + XY(2, 1), 0x0100);  // todo: use 8-bit write?
    WriteAttr1(j + XY(2, 2), 0x0100);
    WriteAttr2(j + XY(1, 1), attr);
    WriteAttr1(j + XY(1, 1), attr);
    WriteAttr2(j + XY(1, 2), attr);
    WriteAttr1(j + XY(1, 2), attr);
  }
skip3:
  if (i != dung_some_stairs_unk4) {
    kind_of_in_room_staircase = 2;
    for (; i != dung_some_stairs_unk4; i += 2) {
      int j = dung_stairs_table_1[i >> 1];
      WriteAttr2(j + XY(0, 0), 0xa03);
      WriteAttr1(j + XY(0, 0), 0xa03);
      WriteAttr2(j + XY(2, 0), 0x30a);
      WriteAttr1(j + XY(2, 0), 0x30a);
      WriteAttr2(j + XY(0, 1), 0x803);
      WriteAttr2(j + XY(2, 1), 0x308);
    }
  }
  i = 0;
  if (i != dung_num_inroom_upnorth_stairs_water) {
    kind_of_in_room_staircase = 2;
    for (; i != dung_num_inroom_upnorth_stairs_water; i += 2) {
      int j = dung_stairs_table_1[i >> 1];
      WriteAttr2(j + XY(0, 0), 0x003);
      WriteAttr2(j + XY(2, 0), 0x300);
      WriteAttr1(j + XY(0, 0), 0xa03);
      WriteAttr1(j + XY(2, 0), 0x30a);
      WriteAttr2(j + XY(0, 1), 0x808);
      WriteAttr2(j + XY(2, 1), 0x808);
    }
  }
  if (i != dung_num_activated_water_ladders) {
    kind_of_in_room_staircase = 2;
    for (; i != dung_num_activated_water_ladders; i += 2) {
      int j = dung_stairs_table_1[i >> 1];
      WriteAttr2(j + XY(0, 0), 0x003);
      WriteAttr2(j + XY(2, 0), 0x300);
      WriteAttr1(j + XY(0, 0), 0xa03);
      WriteAttr1(j + XY(2, 0), 0x30a);
    }
  }

  for (i = 0, t = 0x7070; i != dung_misc_objs_index; i += 2, t += 0x101) {
    uint16 k = dung_replacement_tile_state[i >> 1];
    if ((k & 0xf0) != 0x30) {
      int j = (dung_object_tilemap_pos[i >> 1] & 0x3fff) >> 1;
      WriteAttr2(j + XY(0, 0), t);
      WriteAttr2(j + XY(0, 1), t);
    }
  }

  if (i != dung_index_of_torches) {
    for (t = 0xc0c0; i != dung_index_of_torches; i += 2, t = (t & 0xefef) + 0x101) {
      int j = (dung_object_tilemap_pos[i >> 1] & 0x3fff) >> 1;
      WriteAttr2(j + XY(0, 0), t);
      WriteAttr2(j + XY(0, 1), t);
    }
    dung_index_of_torches = 0;
  }

  t = 0x5858, i = 0;
  if (dung_num_chests_x2) {
    if (dung_hdr_tag[0] == 0x27 || dung_hdr_tag[0] == 0x3c || dung_hdr_tag[0] == 0x3e || dung_hdr_tag[0] >= 0x29 && dung_hdr_tag[0] < 0x33)
      goto no_big_key_locks;
    if (dung_hdr_tag[1] == 0x27 || dung_hdr_tag[1] == 0x3c || dung_hdr_tag[1] == 0x3e || dung_hdr_tag[1] >= 0x29 && dung_hdr_tag[1] < 0x33)
      goto no_big_key_locks;

    for (; i != dung_num_chests_x2; i += 2, t += 0x101) {
      int k = dung_chest_locations[i >> 1];
      if (k != 0) {
        int j = (k & 0x7fff) >> 1;
        WriteAttr2(j + XY(0, 0), t);
        WriteAttr2(j + XY(0, 1), t);
        if (k & 0x8000) {
          dung_chest_locations[i >> 1] = k & 0x7fff;
          WriteAttr2(j + XY(2, 1), t);
          WriteAttr2(j + XY(0, 2), t);
          WriteAttr2(j + XY(2, 2), t);
        }
      }
    }
  }
  for (; i != dung_num_bigkey_locks_x2; i += 2, t += 0x101) {
    int k = dung_chest_locations[i >> 1];
    dung_chest_locations[i >> 1] = k | 0x8000;
    int j = (k & 0x7fff) >> 1;
    WriteAttr2(j + XY(0, 0), t);
    WriteAttr2(j + XY(0, 1), t);
  }
no_big_key_locks:

  i = 0;
  type = 0, iend = dung_num_stairs_1;
  attr = 0x3f3f;
  if (iend == 0) {
    type = 1, attr = 0x3e3e;
    iend = dung_num_stairs_2;
    if (iend == 0) {
      type = 2, attr = 0x3d3d;
      iend = dung_num_stairs_wet;
      if (iend == 0)
        goto skip7;
    }
  }
  kind_of_in_room_staircase = type;
  for (i = 0; i != iend; i += 2) {
    int j = dung_stairs_table_2[i >> 1];
    WriteAttr1(j + XY(0, 0), 0x02);
    WriteAttr2(j + XY(0, 3), 0x02);
    WriteAttr1(j + XY(0, 1), 0x01);
    WriteAttr2(j + XY(0, 2), 0x01);
    WriteAttr1(j + XY(2, 0), 0x0200);
    WriteAttr2(j + XY(2, 3), 0x0200);
    WriteAttr1(j + XY(2, 1), 0x0100);  // todo: use 8-bit write?
    WriteAttr2(j + XY(2, 2), 0x0100);
    WriteAttr1(j + XY(1, 1), attr);
    WriteAttr2(j + XY(1, 1), attr);
    WriteAttr1(j + XY(1, 2), attr);
    WriteAttr2(j + XY(1, 2), attr);
  }
skip7:

  if (dung_num_inroom_upsouth_stairs_water) {
    kind_of_in_room_staircase = 2;
    for (i = 0; i != dung_num_inroom_upsouth_stairs_water; i += 2) {
      int j = dung_stairs_table_2[i >> 1];
      WriteAttr1(j + XY(0, 3), 0xa03);
      WriteAttr1(j + XY(2, 3), 0x30a);
      WriteAttr2(j + XY(0, 3), 0x003);
      WriteAttr2(j + XY(2, 3), 0x300);
      WriteAttr2(j + XY(0, 2), 0x808);
      WriteAttr2(j + XY(2, 2), 0x808);
    }
  }
  overworld_map_state += 1;
}

void Door_LoadBlastWallAttr(int k) {
  int j = dung_door_tilemap_address[k] >> 1;
  if (!(dung_door_direction[k] & 2)) {
    for (int n = 12; n; n--) {
      WriteAttr2(j + XY(0, 0), 0x102);
      for (int i = 2; i < 20; i += 2)
        WriteAttr2(j + XY(i, 0), 0x0);
      WriteAttr2(j + XY(20, 0), 0x201);
      j += XY(0, 1);
    }
  } else {
    for (int n = 5; n; n--) {
      WriteAttr2(j + XY(0, 0), 0x101);
      WriteAttr2(j + XY(0, 21), 0x101);
      WriteAttr2(j + XY(0, 1), 0x202);
      WriteAttr2(j + XY(0, 20), 0x202);
      for (int i = 2; i < 20; i++)
        WriteAttr2(j + XY(0, i), 0x0);
      j += XY(2, 0);
    }
  }
}

static const uint16 kTileAttrsByDoor[] = {
  0x8080, 0x8484, 0x0, 0x101, 0x8484, 0x8e8e, 0x0, 0x0, 0x8888, 0x8e8e, 0x8080, 0x8080, 0x8282, 0x8080, 0x8080, 0x8080,
  0x8080, 0x8080, 0x8080, 0x8080, 0x8282, 0x8e8e, 0x8080, 0x8282, 0x8080, 0x8080, 0x8080, 0x8282, 0x8282, 0x8080, 0x8080, 0x8080,
  0x8484, 0x8484, 0x8686, 0x8888, 0x8686, 0x8686, 0x8080, 0x8080,
};

void Dungeon_LoadSingleDoorAttr(int k) {
  assert(k >= 0 && k < 16);
  uint8 t = door_type_and_slot[k] & 0xfe, dir;
  uint16 attr;
  int i, j;

  if (t == kDoorType_Regular || t == kDoorType_EntranceDoor || t == kDoorType_ExitToOw || t == kDoorType_EntranceLarge || t == kDoorType_EntranceCave)
    goto alpha;

  if (t == kDoorType_EntranceLarge2 || t == kDoorType_EntranceCave2 || t == kDoorType_4 || t == kDoorType_Regular2 || t == kDoorType_WaterfallTunnel)
    goto beta;

  if (t == kDoorType_LgExplosion)
    return;

  if (t >= kDoorType_RegularDoor33) {
    if (t == kDoorType_RegularDoor33 || t == kDoorType_WarpRoomDoor)
      goto beta;
    if (dung_door_opened_incl_adjacent & kUpperBitmasks[k])
      goto beta;

    j = dung_door_tilemap_address[k] >> 1;
    attr = (0xf0 + k) * 0x101;
    WriteAttr2(j + XY(1, 1), attr);
    WriteAttr2(j + XY(1, 2), attr);
    return;

  }

  i = (t == kDoorType_ShuttersTwoWay || t == kDoorType_Shutter) ? k : k & 7;
  if (!(dung_door_opened_incl_adjacent & kUpperBitmasks[i])) {
    j = dung_door_tilemap_address[k] >> 1;
    attr = (0xf0 + k) * 0x101;
    WriteAttr2(j + XY(1, 1), attr);
    WriteAttr2(j + XY(1, 2), attr);
    return;
  }

alpha:
  if (t >= kDoorType_StairMaskLocked0 && t <= kDoorType_StairMaskLocked3)
    return;
  attr = kTileAttrsByDoor[t >> 1];
  dir = dung_door_direction[k] & 3;
  if (dir == 0) {
    uint16 a = dung_door_tilemap_address[k];
    if (a == dung_exit_door_addresses[0] || a == dung_exit_door_addresses[1] || a == dung_exit_door_addresses[2] || a == dung_exit_door_addresses[3])
      attr = 0x8e8e;
    j = (a >> 1) & ~0x7c0;
    WriteAttr2(j + XY(1, 0), attr);
    WriteAttr2(j + XY(1, 1), attr);
    WriteAttr2(j + XY(1, 2), attr);
    WriteAttr2(j + XY(1, 3), attr);
    WriteAttr2(j + XY(1, 4), attr);
    WriteAttr2(j + XY(1, 5), attr);
    WriteAttr2(j + XY(1, 6), attr);
    WriteAttr2(j + XY(1, 7), 0);
  } else if (dir == 1) {
    uint16 a = dung_door_tilemap_address[k];
    if (t == kDoorType_EntranceLarge || t == kDoorType_EntranceCave ||
        a == dung_exit_door_addresses[0] || a == dung_exit_door_addresses[1] || a == dung_exit_door_addresses[2] || a == dung_exit_door_addresses[3])
      attr = 0x8e8e;
    j = a >> 1;
    WriteAttr2(j + XY(1, 1), attr);
    WriteAttr2(j + XY(1, 2), attr);
    WriteAttr2(j + XY(1, 3), attr);
    WriteAttr2(j + XY(1, 4), attr);
    WriteAttr2(j + XY(1, 5), attr);
  } else if (dir == 2) {
    j = (dung_door_tilemap_address[k] >> 1) & ~0x1f;
    WriteAttr2(j + XY(0, 1), attr + 0x101);
    WriteAttr2(j + XY(2, 1), attr + 0x101);
    WriteAttr2(j + XY(0, 2), attr + 0x101);
    WriteAttr2(j + XY(2, 2), attr + 0x101);
    WriteAttr2(j + XY(4, 1), (attr + 0x101) & 0xff);
    WriteAttr2(j + XY(4, 2), (attr + 0x101) & 0xff);
  } else {
    j = (dung_door_tilemap_address[k] >> 1);
    WriteAttr2(j + XY(2, 1), attr + 0x101);
    WriteAttr2(j + XY(4, 1), attr + 0x101);
    WriteAttr2(j + XY(2, 2), attr + 0x101);
    WriteAttr2(j + XY(4, 2), attr + 0x101);
    WriteAttr2(j + XY(0, 1), (attr + 0x101) & 0xff00);
    WriteAttr2(j + XY(0, 2), (attr + 0x101) & 0xff00);
  }
  return;

beta:
  attr = kTileAttrsByDoor[t >> 1];
  dir = dung_door_direction[k] & 3;
  if (dir == 0) {
    j = (dung_door_tilemap_address[k] >> 1) & ~0x7c0;
    WriteAttr2(j + XY(1, 0), attr);
    WriteAttr2(j + XY(1, 1), attr);
    WriteAttr2(j + XY(1, 2), attr);
    WriteAttr2(j + XY(1, 3), attr);
    WriteAttr2(j + XY(1, 4), attr);
    WriteAttr2(j + XY(1, 5), attr);
    WriteAttr2(j + XY(1, 6), attr);
    WriteAttr2(j + XY(1, 7), attr);
    WriteAttr2(j + XY(1, 8), attr);
    WriteAttr2(j + XY(1, 9), attr);
  } else if (dir == 1) {
    uint16 a = dung_door_tilemap_address[k] & 0x1fff;
    if (t == kDoorType_EntranceLarge2 || t == kDoorType_EntranceCave2 || t == kDoorType_4 ||
        a == dung_exit_door_addresses[0] || a == dung_exit_door_addresses[1] || a == dung_exit_door_addresses[2] || a == dung_exit_door_addresses[3])
      attr = 0x8e8e;
    j = dung_door_tilemap_address[k] >> 1;
    WriteAttr2(j + XY(1, 1), attr);
    WriteAttr2(j + XY(1, 2), attr);
    WriteAttr2(j + XY(1, 3), attr);
    WriteAttr2(j + XY(1, 4), attr);
    WriteAttr2(j + XY(1, 5), attr);
    WriteAttr2(j + XY(1, 6), attr);
    WriteAttr2(j + XY(1, 7), attr);
    WriteAttr2(j + XY(1, 8), attr);
  } else if (dir == 2) {
    j = (dung_door_tilemap_address[k] >> 1) & ~0x1f;
    WriteAttr2(j + XY(0, 1), attr + 0x101);
    WriteAttr2(j + XY(2, 1), attr + 0x101);
    WriteAttr2(j + XY(4, 1), attr + 0x101);
    WriteAttr2(j + XY(6, 1), attr + 0x101);
    WriteAttr2(j + XY(0, 2), attr + 0x101);
    WriteAttr2(j + XY(2, 2), attr + 0x101);
    WriteAttr2(j + XY(4, 2), attr + 0x101);
    WriteAttr2(j + XY(6, 2), attr + 0x101);
  } else {
    j = ((dung_door_tilemap_address[k] >> 1) + 1);
    WriteAttr2(j + XY(0, 1), attr + 0x101);
    WriteAttr2(j + XY(2, 1), attr + 0x101);
    WriteAttr2(j + XY(4, 1), attr + 0x101);
    WriteAttr2(j + XY(6, 1), attr + 0x101);
    WriteAttr2(j + XY(0, 2), attr + 0x101);
    WriteAttr2(j + XY(2, 2), attr + 0x101);
    WriteAttr2(j + XY(4, 2), attr + 0x101);
    WriteAttr2(j + XY(6, 2), attr + 0x101);
  }
}

void Dungeon_LoadDoorAttrInner() {
  for (int i = 0; i != dung_num_toggle_floor; i += 2) {
    int j = dung_toggle_floor_pos[i >> 1];
    if ((dung_bg2_attr_table[j] & 0xf0) == 0x80) {
      uint16 attr = *(uint16 *)&dung_bg2_attr_table[j];
      WriteAttr2(j + XY(0, 0), attr | 0x1010);
      WriteAttr2(j + XY(0, 1), attr | 0x1010);
    } else {
      uint16 attr = *(uint16 *)&dung_bg1_attr_table[j];
      WriteAttr1(j + XY(0, 0), attr | 0x1010);
      WriteAttr1(j + XY(0, 1), attr | 0x1010);
    }
  }
  for (int i = 0; i != dung_num_toggle_palace; i += 2) {
    int j = dung_toggle_palace_pos[i >> 1];
    if ((dung_bg2_attr_table[j] & 0xf0) == 0x80) {
      uint16 attr = *(uint16 *)&dung_bg2_attr_table[j];
      WriteAttr2(j + XY(0, 0), attr | 0x2020);
      WriteAttr2(j + XY(0, 1), attr | 0x2020);
    } else {
      uint16 attr = *(uint16 *)&dung_bg1_attr_table[j];
      WriteAttr1(j + XY(0, 0), attr | 0x2020);
      WriteAttr1(j + XY(0, 1), attr | 0x2020);
    }
  }
}

void someExperimentalCrap() {
  assert(dung_unk5 == 0);
}

void Dungeon_LoadDoorAttr() {
  for (int i = 0; i != 16; i++) {
    if (dung_door_tilemap_address[i])
      Dungeon_LoadSingleDoorAttr(i);
  }
  Dungeon_LoadDoorAttrInner();
  someExperimentalCrap();
  overworld_map_state += 1;
}

void Dungeon_ToggleBarrierAttr() {
  for (int i = 0xfff; i >= 0; i--) {
    if ((dung_bg2_attr_table[i] & ~1) == 0x66)
      dung_bg2_attr_table[i] ^= 1;
    if ((dung_bg1_attr_table[i] & ~1) == 0x66)
      dung_bg1_attr_table[i] ^= 1;
  }
}


void Dungeon_LoadAttrTable() {
  dung_draw_width_indicator = dung_draw_height_indicator = 0;
  Dungeon_LoadBasicAttr_full(0x1000);
  Dungeon_LoadObjAttr();
  Dungeon_LoadDoorAttr();
  if (orange_blue_barrier_state)
    Dungeon_ToggleBarrierAttr();
  overworld_map_state = 0;
}

void Dungeon_LoadAttrIncremental() {
  switch (overworld_map_state) {
  case 0:  // Dungeon_LoadBasicAttr
    overworld_map_state = 1;
    dung_draw_width_indicator = dung_draw_height_indicator = 0;
  case 1:
    Dungeon_LoadBasicAttr_full(0x40);
    break;
  case 2:
    Dungeon_LoadObjAttr();
    break;
  case 3:
    Dungeon_LoadDoorAttr();
    break;
  case 4:
    overworld_map_state = 5;
    if (orange_blue_barrier_state)
      Dungeon_ToggleBarrierAttr();
    break;
  case 5:
    break;
  default:
    assert(0);
  }
}


void Dungeon_ExtinguishTorch() {
  int y = (byte_7E0333 & 0xf) * 2 + dung_index_of_torches_start;

  uint16 r8 = (dung_object_tilemap_pos[y >> 1] &= 0x7fff);

  dung_torch_data[(dung_object_pos_in_objdata[y >> 1] & 0xff) >> 1] = r8;

  r8 &= 0x3fff;
  Dungeon_PrepOverlayDma(r8, 0xec2, r8);
  nmi_copy_packets_flag = 1;

  if (dung_want_lights_out && dung_num_lit_torches != 0 && --dung_num_lit_torches < 3) {
    if (dung_num_lit_torches == 0)
      TS_copy = 1;
    overworld_fixed_color_plusminus = kLitTorchesColorPlus[dung_num_lit_torches];
    submodule_index = 10;
    subsubmodule_index = 0;
  }

  dung_torch_timers[byte_7E0333 & 0xf] = 0;
  byte_7E0333 = 0;
}

void Dungeon_ExtinguishFirstTorch() {
  Palette_AssertTranslucencySwap();
  byte_7E0333 = 0xc0;
  Dungeon_ExtinguishTorch();
}

void Dungeon_ExtinguishSecondTorch() {
  byte_7E0333 = 193;
  Dungeon_ExtinguishTorch();
}


void Dungeon_LoadToggleDoorAttr_OtherEntry(int door) {
  Dungeon_LoadSingleDoorAttr(door);
  Dungeon_LoadDoorAttrInner();
}

void Dungeon_ProcessTorchAndDoorInteractives() {
  static const int16 kDungLinkOffs1X[] = { 0, 0, -1, 17 };
  static const int16 kDungLinkOffs1Y[] = { 7, 24, 8, 8 };
  static const uint16 kDungLinkOffs1Pos[] = { 0x2, 0x2, 0x80, 0x80 };

  if ((frame_counter & 3) == 0 && !flag_custom_spell_anim_active) {
    for (int i = 0; i != 16; i++) {
      if (dung_torch_timers[i] && !--dung_torch_timers[i]) {
        byte_7E0333 = 0xc0 + i;
        Dungeon_ExtinguishTorch();
      }
    }
  }

  if (!flag_is_link_immobilized) {
    int dir = link_direction_facing >> 1;
    int pos = ((link_y_coord + kDungLinkOffs1Y[dir]) & 0x1f8) << 3;
    pos |= ((link_x_coord + kDungLinkOffs1X[dir]) & 0x1f8) >> 3;
    pos |= (link_is_on_lower_level ? 0x1000 : 0);

    if ((dung_bg2_attr_table[pos] & 0xf0) == 0xf0 ||
        (dung_bg2_attr_table[pos += kDungLinkOffs1Pos[dir]] & 0xf0) == 0xf0) {
      int k = dung_bg2_attr_table[pos] & 0xf;
      dung_which_key_x2 = 2 * k;

      if ((dung_door_direction[k] & 3) != dir)
        goto not_openable;

      uint8 door_type = door_type_and_slot[k] & 0xfe;
      if (door_type == kDoorType_BreakableWall) {
        if (link_is_running && link_dash_ctr < 63) {
          dung_cur_door_pos = pos;

          int db = AddDoorDebris();
          if (db >= 0) {
            door_debris_direction[db] = dung_door_direction[k] & 3;
            door_debris_x[db] = dung_loade_bgoffs_h_copy + (dung_door_tilemap_address[k] & 0x7e) * 4;
            door_debris_y[db] = dung_loade_bgoffs_v_copy + ((dung_door_tilemap_address[k] & 0x1f80) >> 4);
          }
          sound_effect_2 = 27;
          submodule_index = 9;
          Player_RepelDashAttack();
          return;
        }
      } else if (door_type == kDoorType_1E) {
        door_animation_step_indicator = 0;
        dung_cur_door_pos = pos;
        if (link_bigkey & kUpperBitmasks[cur_palace_index_x2 >> 1])
          goto has_key_for_door;
        if (!big_key_door_message_triggered) {
          big_key_door_message_triggered = 1;
          dialogue_message_index = 0x7a;
          Main_ShowTextMessage();
        }
      } else if (door_type >= kDoorType_SmallKeyDoor && door_type < 0x2c && door_type != 0x2a && link_num_keys != 0) {
        link_num_keys -= 1;
has_key_for_door:
        door_animation_step_indicator = 0;
        dung_cur_door_pos = pos;
        submodule_index = 4;
        static const uint8 kOpenDoorPanning[] = { 0x0, 0x0, 0x80, 0x40 };
        sound_effect_2 = 20 | kOpenDoorPanning[dung_door_direction[k] & 3];
        return;
      }
    } else {
not_openable:
      big_key_door_message_triggered = 0;
    }
  }

  if (!(invisible_door_dir_and_index_x2 & 0x80) && !is_standing_in_doorway && (link_x_coord >> 8) == 0xc) {
    uint8 dir = invisible_door_dir_and_index_x2;
    int j = (invisible_door_dir_and_index_x2 >> 8) >> 1;
    uint16 m = dung_door_opened_incl_adjacent;
    if (dir != link_direction_facing && (dir ^ 2) == link_direction_facing)
      m |= kUpperBitmasks[j];
    else
      m &= ~kUpperBitmasks[j];
    if (m != dung_door_opened_incl_adjacent) {
      dung_door_opened_incl_adjacent = m;
      // 81:D01F
      assert(0);
    }
  }

  if (!(button_mask_b_y & 0x80) || button_b_frames != 4)
    return;

  int pos = ((link_y_coord + (int8)player_oam_y_offset) & 0x1f8) << 3;
  pos |= ((link_x_coord + (int8)player_oam_x_offset) & 0x1f8) >> 3;
  uint8 attr, y;

#define is_6c_fx(yv,x) (y=yv, ((attr = (dung_bg2_attr_table[x] & 0xfc)) == 0x6c || (attr & 0xf0) == 0xf0))

  if (!(is_6c_fx(0x41, pos) || is_6c_fx(0x40, pos += 1) || is_6c_fx(1, pos += 63) || is_6c_fx(0, pos += 1)))
    return;

  int addr;

  if (attr == 0x6c) {
    if (y & 0x40 && (dung_bg2_attr_table[pos -= 64] & 0xfc) != 0x6c)
      pos += 64;
    if (y & 1 && (dung_bg2_attr_table[pos -= 1] & 0xfc) != 0x6c)
      pos += 1;
    attr = dung_bg2_attr_table[pos];
    WriteAttr2(pos + XY(0, 0), 0x202);
    WriteAttr2(pos + XY(0, 1), 0x202);
    static const uint16 kSrcTiles1[] = { 0x7ea, 0x80a, 0x80a, 0x82a };
    addr = (pos - XY(1, 1)) * 2;
    Object_Draw_Nx4(4, SrcPtr(kSrcTiles1[attr & 3]), &dung_bg2[addr >> 1]);
  } else {
    dung_cur_door_pos = pos;
    int k = attr & 0xf;

    uint8 door_type = door_type_and_slot[k];
    if (door_type != kDoorType_Slashable)
      return;
    sound_effect_2 = 27;
    addr = dung_door_tilemap_address[k];
    dung_door_opened_incl_adjacent |= kUpperBitmasks[k];
    dung_door_opened |= kUpperBitmasks[k];
    door_open_closed_counter = 0;
    dung_cur_door_idx = k * 2;
    dung_which_key_x2 = k * 2;
    Object_Draw_Nx4(4, SrcPtr(kDoorTypeSrcData[0x56 / 2]), &dung_bg2[addr >> 1]);
    Dungeon_LoadToggleDoorAttr_OtherEntry(k);
  }

  Dungeon_PrepOverlayDma_nextPrep(0, addr);
  sound_effect_1 = 30 | LightTorch_GetSfxPan((addr & 0x7f) * 2);
  nmi_copy_packets_flag = 1;
}

void Bomb_CheckForVulnerableTileObjects(uint16 x, uint16 y, uint8 r14) {
  if (main_module_index != 7) {
    Overworld_ApplyBombToTiles(x, y);
    return;
  }
  int k = ((y & 0x1f8) << 3 | (x & 0x1f8) >> 3) - 0x82;
  uint8 a;
  for (int i = 2; i >= 0; i--) {
    a = dung_bg2_attr_table[k];
    if (a == 0x62) {
handle_62:
      if (dungeon_room_index == 0x65)
        dung_savegame_state_bits |= 0x1000;
      Point16U pt;
      printf("Wtf is R6\n");
      Dungeon_CustomIndexedRevealCoveredTiles(0, 0, &pt);
      sound_effect_2 = 0x1b;
      return;
    }
    if ((a & 0xf0) == 0xf0) {
handle_f0:
      int j = a & 0xf;
      a = door_type_and_slot[j] & 0xfe;
      if (a != kDoorType_BreakableWall && a != 0x2A && a != 0x2E)
        return;
      dung_cur_door_pos = k;
      door_debris_x[r14] = ((dung_door_tilemap_address[j] & 0x7e) << 2) + dung_loade_bgoffs_h_copy;
      door_debris_y[r14] = ((dung_door_tilemap_address[j] & 0x1f80) >> 4) + dung_loade_bgoffs_v_copy;
      door_debris_direction[r14] = dung_door_direction[j] & 3;
      sound_effect_2 = 0x1b;
      submodule_index = 9;
      return;
    }

    a = dung_bg2_attr_table[k += 2];
    if (a == 0x62) goto handle_62;
    if ((a & 0xf0) == 0xf0) goto handle_f0;

    a = dung_bg2_attr_table[k += 2];
    if (a == 0x62) goto handle_62;
    if ((a & 0xf0) == 0xf0) goto handle_f0;

    k += 0x7c;
  }

}

void Effect_DoNothing() {
}
void Effect_MovingFloor() {
  if (dung_savegame_state_bits & 0x8000) {
    dung_hdr_collision_2 = 0;
    return;
  }
  dung_floor_x_vel = dung_floor_y_vel = 0;
  if (dung_floor_move_flags & 1)
    return;
  int t = dung_some_subpixel[1] + 0x80;
  dung_some_subpixel[1] = t;
  t >>= 8;
  if (dung_floor_move_flags & 2)
    t = -t;

  if (dung_floor_move_flags < 4) {
    dung_floor_x_vel = t;
    dung_floor_x_offs -= t;
    BG1HOFS_copy2 = BG2HOFS_copy2 + dung_floor_x_offs;
  } else {
    dung_floor_y_vel = t;
    dung_floor_y_offs -= t;
    BG1VOFS_copy2 = BG2VOFS_copy2 + dung_floor_y_offs;
  }
}
void Effect_MovingWater() {
  int t;
  dung_some_subpixel[1] = t = dung_some_subpixel[1] + 0x80;
  dung_floor_x_vel = -(t >> 8);
}

void Effect_MovingFloor2() {
  dung_floor_x_offs += dung_floor_x_vel;
  dung_floor_y_offs += dung_floor_y_vel;
  dung_floor_x_vel = 0;
  dung_floor_y_vel = 0;
}
void Effect_RedFlashes() {
  int j = frame_counter & 0x7f;
  if (j == 3 || j == 36) {
    main_palette_buffer[0x6d] = 0x1d59;
    main_palette_buffer[0x6e] = 0x25ff;
    main_palette_buffer[0x77] = main_palette_buffer[0x6f] = 0x1a;
    flag_update_cgram_in_nmi++;
  } else if (j == 5 || j == 38) {
    main_palette_buffer[0x6d] = aux_palette_buffer[0x6d];
    main_palette_buffer[0x6e] = aux_palette_buffer[0x6e];
    main_palette_buffer[0x77] = main_palette_buffer[0x6f] = aux_palette_buffer[0x6f];
    flag_update_cgram_in_nmi++;
  }
  TS_copy = 2;
}
void Effect_TorchHiddenTiles() {
  int count = 0;
  for (int i = 0; i < 16; i++)
    count += (dung_object_tilemap_pos[i] & 0x8000) != 0;

  uint16 x = 0x2940, y = 0x4e60;
  if (count == 0)
    x = y = 0;

  if (aux_palette_buffer[0x7b] != x) {
    main_palette_buffer[0x7b] = aux_palette_buffer[0x7b] = x;
    main_palette_buffer[0x7c] = aux_palette_buffer[0x7c] = y;
    flag_update_cgram_in_nmi++;
  }
  TS_copy = 2;
}
void Effect_TorchGanonRoom() {
  int count = 0;
  for (int i = 0; i < 16; i++)
    count += (dung_object_tilemap_pos[i] & 0x8000) != 0;

  byte_7E04C5 = count;
  if (count == 0) {
    TS_copy = 0;
    CGADSUB_copy = 0xb3;
  } else if (count == 1) {
    TS_copy = 2;
    CGADSUB_copy = 0x70;
  } else {
    TS_copy = 0;
    CGADSUB_copy = 0x70;
  }
}

static PlayerHandlerFunc *const kDungeon_Effect_Handler[28] = {
  &Effect_DoNothing,
  &Effect_DoNothing,
  &Effect_MovingFloor,
  &Effect_MovingWater,
  &Effect_MovingFloor2,
  &Effect_RedFlashes,
  &Effect_TorchHiddenTiles,
  &Effect_TorchGanonRoom,
};


void Dungeon_Effect_Handler() {
  kDungeon_Effect_Handler[dung_hdr_collision_2]();
}

static const int16 kPushBlockMoveDistances[] = { -0x100, 0x100, -0x4, 0x4 };

void ChangableDungeonObj_Func2C(uint8 i) {
  static const uint8 kPushedBlockDirMask[] = { 0x8, 0x4, 0x2, 0x1 };
  uint8 m = kPushedBlockDirMask[(uint8)pushedblock_facing[i] >> 1];
  uint32 o;
  link_actual_vel_x = link_actual_vel_y = 0;
  if (m & 3) {
    int8 vel = (m & 2) ? -12 : 12;
    link_actual_vel_x = vel;
    o = (pushedblocks_subpixel[i] | pushedblocks_x_lo[i] << 8 | pushedblocks_x_hi[i] << 16) + vel * 16;
    pushedblocks_subpixel[i] = (uint8)o;
    pushedblocks_x_lo[i] = (uint8)(o >> 8);
    pushedblocks_x_hi[i] = (uint8)(o >> 16);
  } else {
    int8 vel = (m & 8) ? -12 : 12;
    link_actual_vel_y = vel;
    o = (pushedblocks_subpixel[i] | pushedblocks_y_lo[i] << 8 | pushedblocks_y_hi[i] << 16);
    o += vel * 16;
    pushedblocks_subpixel[i] = (uint8)o;
    pushedblocks_y_lo[i] = (uint8)(o >> 8);
    pushedblocks_y_hi[i] = (uint8)(o >> 16);
  }
  if (((o >> 8) & 0xf) == (uint8)pushedblocks_target[i]) {
    int j = index_of_changable_dungeon_objs[i] - 1;
    dung_replacement_tile_state[j]++;
    link_cant_change_direction &= ~0x4;
    bitmask_of_dragstate &= ~0x4;
  }
  uint16 x = pushedblocks_x_lo[i] | pushedblocks_x_hi[i] << 8;
  uint16 y = pushedblocks_y_lo[i] | pushedblocks_y_hi[i] << 8;
  for (int j = 15; j >= 0; j--) {
    if (sprite_state[j] >= 9) {
      uint16 sx = sprite_x_lo[j] | sprite_x_hi[j] << 8;
      uint16 sy = sprite_y_lo[j] | sprite_y_hi[j] << 8;
      if ((uint16)(x - sx + 0x10) < 0x20 && (uint16)(y - sy + 0x10) < 0x20) {
        sprite_F[j] = 8;
        static const uint8 kPushBlockTab1[] = { 0x0, 0x0, 0xe0, 0x20 };
        static const uint8 kPushBlockTab2[] = { 0xe0, 0x20, 0x0, 0x0 };
        int k = (uint8)pushedblock_facing[i] >> 1;
        sprite_x_recoil[j] = kPushBlockTab1[k];
        sprite_y_recoil[j] = kPushBlockTab2[k];
      }
    }
  }
}

void ChangableDungeonObj_Func2B(uint8 i, uint16 x, uint16 y) {
  static const uint8 kPushBlock_A[] = { 0, 0, 8, 8 };
  static const uint8 kPushBlock_B[] = { 15, 15, 23, 23 };
  static const uint8 kPushBlock_D[] = { 15, 15, 15, 15 };
  static const uint8 kPushBlock_C[] = { 0x0, 0x0, 0x0, 0x0 };
  static const uint8 kPushBlock_E[] = { 8, 24, 0, 16 };
  static const uint8 kPushBlock_F[] = { 15, 0, 15, 0 };

  link_y_coord_safe_return_hi = link_y_coord >> 8;
  link_x_coord_safe_return_hi = link_x_coord >> 8;

  int dir = 3;
  uint8 m = link_direction & 0xf;
  while (!(m & 1)) {
    m >>= 1;
    if (--dir < 0)
      return;
  }
  int l = (dir < 2) ? link_x_coord : link_y_coord;
  int o = (dir < 2) ? x : y;

  uint16 r0 = l + kPushBlock_A[dir];
  uint16 r2 = l + kPushBlock_B[dir];
  uint16 r4 = o + kPushBlock_C[dir];
  uint16 r6 = o + kPushBlock_D[dir];

  uint16 *coord_p = (dir < 2) ? &link_y_coord : &link_x_coord;
  uint16 r8 = *coord_p + kPushBlock_E[dir];
  uint16 r10 = ((dir < 2) ? y : x) + kPushBlock_F[dir];

  bitmask_of_dragstate &= ~4;

  if (r0 >= r4 && r0 < r6 || r2 >= r4 && r2 < r6) {
    if (link_direction_facing == pushedblock_facing[i])
      bitmask_of_dragstate |= index_of_changable_dungeon_objs[i] ? 4 : 1;
    if (dir & 1 ? (r8 >= r10 && (uint16)(r8 - r10) < 8) : (uint16)(r8 - r10) >= 0xfff8) {
      *coord_p -= r8 - r10;
      (dir & 2 ? link_x_vel : link_y_vel) -= r8 - r10;
    }
  }
  Player_CheckDoorwayQuadrantMovement();
}

void ChangableDungeonObj_Func2(uint8 j) {
  if (submodule_index)
    return;
  int i = (index_of_changable_dungeon_objs[1] - 1) * 2 == j;
  pushedblocks_maybe_timeout = 9;
  pushedblocks_some_index = 0;
  ChangableDungeonObj_Func2C(i);
  int y = (uint8)pushedblocks_y_lo[i] | (uint8)pushedblocks_y_hi[i] << 8;
  int x = (uint8)pushedblocks_x_lo[i] | (uint8)pushedblocks_x_hi[i] << 8;
  ChangableDungeonObj_Func2B(i, x, y);
}

void PushBlock_StoppedMoving(uint8 y) {
  y >>= 1;
  if (!(dung_object_tilemap_pos[y] & 0x4000))
    dung_flag_movable_block_was_pushed ^= 1;

  int p = (dung_object_tilemap_pos[y] & 0x3fff) >> 1;
  uint8 attr = dung_bg2_attr_table[p];
  if (attr == 0x20) {  // fall into pit
    sound_effect_1 = 0x20;
    int k = dung_object_pos_in_objdata[y] >> 2;
    movable_block_datas[k].room = dung_hdr_travel_destinations[0];
    movable_block_datas[k].tilemap = dung_object_tilemap_pos[y];
    return;
  }

  int i = (index_of_changable_dungeon_objs[1] - 1) == y;
  index_of_changable_dungeon_objs[i] = 0;

  if (attr == 0x23) {
    related_to_trapdoors_somehow = dung_flag_trapdoors_down ^ 1;
    dung_replacement_tile_state[y] = 4;
  } else {
    dung_replacement_tile_state[y] = 0xffff;
  }
  Dungeon_Store2x2(p, 0x922, 0x932, 0x923, 0x933, 0x27);
}

void PushBlock_Handler_Step4(uint8 y) {
  y >>= 1;

  if (!sign8(--pushedblocks_maybe_timeout))
    return;

  pushedblocks_maybe_timeout = 9;

  if (++pushedblocks_some_index == 4) {
    BYTE(dung_replacement_tile_state[y]) = 0;
    pushedblocks_some_index = 0;
    int i = (index_of_changable_dungeon_objs[1] - 1) == y;
    index_of_changable_dungeon_objs[i] = 0;
  }
}


void Dungeon_PushBlock_Handler() {
  while (dung_misc_objs_index != dung_index_of_torches_start) {
    int k = dung_misc_objs_index >> 1;
    int st = dung_replacement_tile_state[k];
    if (st == 1) {
      Dungeon_EraseInteractive2x2(k * 2);
      dung_object_tilemap_pos[k] += kPushBlockMoveDistances[push_block_direction >> 1];
      dung_replacement_tile_state[k] = 2;
    } else if (st == 2) {
      ChangableDungeonObj_Func2(k * 2);

      if (dung_replacement_tile_state[dung_misc_objs_index >> 1] == 3) {
        PushBlock_StoppedMoving(dung_misc_objs_index);
        dung_replacement_tile_state[dung_misc_objs_index >> 1]++;
      }
    } else if (st == 4) {
      PushBlock_Handler_Step4(k * 2);
    }

    dung_misc_objs_index += 2;
  }
}

void Dungeon_HandleScreenScrolling() {
  if (link_y_vel) {
    int z = (allow_scroll_z && link_z_coord != 0xffff) ? link_z_coord : 0;
    int y = ((link_y_coord - z) & 0x1ff) + 12;
    int scrollamt = 1;
    int y_vel_abs = sign8(link_y_vel) ? (scrollamt = -1, -(int8)link_y_vel) : link_y_vel;
    do {
      int qm = quadrant_fullsize_y >> 1;
      if (sign8(link_y_vel)) {
        if (y > camera_y_coord_scroll_low)
          continue;
      } else {
        if (y < camera_y_coord_scroll_hi)
          continue;
        qm += 2;
      }
      if (BG2VOFS_copy2 == room_bounds_y.v[qm])
        continue;
      BG2VOFS_copy2 += scrollamt;
      if (dungeon_room_index == 0xffff)
        continue;

      BG1VOFS_subpixel += 0x8000;
      BG1VOFS_copy2 += (scrollamt >> 1) + ((BG1VOFS_subpixel & 0x8000) == 0);
      camera_y_coord_scroll_low += scrollamt;
      camera_y_coord_scroll_hi = camera_y_coord_scroll_low + 2;
    } while (--y_vel_abs);
  }
  if (link_x_vel) {
    int x = (link_x_coord & 0x1ff) + 8;
    int scrollamt = 1;
    int x_vel_abs = sign8(link_x_vel) ? (scrollamt = -1, -(int8)link_x_vel) : link_x_vel;
    do {
      int qm = quadrant_fullsize_x >> 1;
      if (sign8(link_x_vel)) {
        if (x > camera_x_coord_scroll_low)
          continue;
      } else {
        if (x < camera_x_coord_scroll_hi)
          continue;
        qm += 2;
      }
      if (BG2HOFS_copy2 == room_bounds_x.v[qm])
        continue;
      BG2HOFS_copy2 += scrollamt;
      if (dungeon_room_index == 0xffff)
        continue;
      BG1HOFS_subpixel += 0x8000;
      BG1HOFS_copy2 += (scrollamt >> 1) + ((BG1HOFS_subpixel & 0x8000) == 0);
      camera_x_coord_scroll_low += scrollamt;
      camera_x_coord_scroll_hi = camera_x_coord_scroll_low + 2;
    } while (--x_vel_abs);
  }
  if (dungeon_room_index != 0xffff) {
    if (dung_hdr_bg2_properties == 0 || dung_hdr_bg2_properties == 2 || dung_hdr_bg2_properties == 3 || dung_hdr_bg2_properties == 4 || dung_hdr_bg2_properties >= 6) {
      BG1HOFS_copy2 = BG2HOFS_copy2;
      BG1VOFS_copy2 = BG2VOFS_copy2;
    }
  }
}

void UsedForStraightInterRoomStaircase() {
  int i = 9;
  do {
    if (ancilla_type[i] == 13)
      ancilla_type[i] = 0;
  } while (--i >= 0);
  if (link_animation_steps >= 5)
    link_animation_steps = 0;
  link_subpixel_x = 0;
  link_subpixel_y = 0;
  some_animation_timer_steps = 0;
  link_timer_push_get_tired = 28;
  countdown_timer_for_staircases = 32;
  link_disable_sprite_damage = 1;
  Player_DoSfx2(which_staircase_index & 4 ? 0x18 : 0x16);

  tiledetect_which_y_pos[1] = link_x_coord + (which_staircase_index & 4 ? -15 : 16);
  tiledetect_which_y_pos[0] = link_y_coord;
}

void Dungeon_DetectStaircase() {
  int k = link_direction & 12;
  if (!k)
    return;

  static const int8 kBuggyLookup[] = { 7, 24, 8, 8, 0, 0, -1, 17 };
  int pos = ((link_y_coord + kBuggyLookup[k >> 1]) & 0x1f8) << 3;
  pos |= (link_x_coord & 0x1f8) >> 3;
  pos |= (link_is_on_lower_level ? 0x1000 : 0);

  uint8 at = dung_bg2_attr_table[pos + (k == 4 ? 0x80 : 0)];
  if (!(at == 0x26 || at == 0x38 || at == 0x39 || at == 0x5e || at == 0x5f))
    return;

  uint8 attr2 = dung_bg2_attr_table[pos + XY(0, 1)];
  if ((attr2 & 0xf8) != 0x30)
    return;

  if (link_state_bits & 0x80) {
    link_y_coord = link_y_coord_prev;
    return;
  }

  which_staircase_index = attr2;
  which_staircase_index_PADDING = pos >> 8; // residual
  dungeon_room_index_prev = dungeon_room_index;
  Dungeon_SaveRoomQuadrantData();

  if (at == 0x38 || at == 0x39) {
    staircase_var1 = 0x20;
    if (at == 0x38)
      Dungeon_StartInterRoomTrans_Up();
    else
      Dungeon_StartInterRoomTrans_Down();
  }

  int j = (which_staircase_index & 3);
  BYTE(dungeon_room_index) = dung_hdr_travel_destinations[j + 1];
  cur_staircase_plane = dung_hdr_staircase_plane[j];
  byte_7E0492 = (link_is_on_lower_level || link_is_on_lower_level_mirror) ? 2 : 0;
  subsubmodule_index = 0;
  bitmask_of_dragstate = 0;
  link_delay_timer_spin_attack = 0;
  button_mask_b_y = 0;
  button_b_frames = 0;
  link_cant_change_direction &= ~1;
  if (at == 0x26) {
    submodule_index = 6;
    sound_effect_1 = (cur_staircase_plane < 0x34 ? 22 : 24); // wtf?
  } else if (at == 0x38 || at == 0x39) {
    submodule_index = (at == 0x38) ? 18 : 19;
    link_timer_push_get_tired = 7;
  } else {
    UsedForStraightInterRoomStaircase();
    submodule_index = 14;
  }
}

void Dung_TagRoutine_0x31(int k);

void Dung_TagRoutine_0x00(int k) {
}
void Dung_TagRoutine_0x29(int k) {
  if (!(link_x_coord & 0x100) && !(link_y_coord & 0x100))
    Dung_TagRoutine_0x31(k);
}
void Dung_TagRoutine_0x2A(int k) {
  if ((link_x_coord & 0x100) && !(link_y_coord & 0x100))
    Dung_TagRoutine_0x31(k);
}
void Dung_TagRoutine_0x2B(int k) {
  if (!(link_x_coord & 0x100) && (link_y_coord & 0x100))
    Dung_TagRoutine_0x31(k);
}
void Dung_TagRoutine_0x2C(int k) {
  if ((link_x_coord & 0x100) && (link_y_coord & 0x100))
    Dung_TagRoutine_0x31(k);
}
void Dung_TagRoutine_0x2D(int k) {
  if (!(link_x_coord & 0x100))
    Dung_TagRoutine_0x31(k);
}
void Dung_TagRoutine_0x2E(int k) {
  if (link_x_coord & 0x100)
    Dung_TagRoutine_0x31(k);
}
void Dung_TagRoutine_0x2F(int k) {
  if (!(link_y_coord & 0x100))
    Dung_TagRoutine_0x31(k);
}
void Dung_TagRoutine_0x30(int k) {
  if (link_y_coord & 0x100)
    Dung_TagRoutine_0x31(k);
}

void Dung_TagRoutine_Helper(int k) {
  dung_hdr_tag[k] = 0;
  vram_upload_offset = 0;
  WORD(overworld_map_state) = 0;
  uint16 attr = 0x5858;
  do {
    int pos = dung_chest_locations[WORD(overworld_map_state) >> 1] >> 1 & 0x1fff;

    WORD(dung_bg2_attr_table[pos + XY(0, 0)]) = attr;
    WORD(dung_bg2_attr_table[pos + XY(0, 1)]) = attr;
    attr += 0x101;

    const uint16 *src = SrcPtr(0x149c);
    dung_bg2[pos + XY(0, 0)] = src[0];
    dung_bg2[pos + XY(0, 1)] = src[1];
    dung_bg2[pos + XY(1, 0)] = src[2];
    dung_bg2[pos + XY(1, 1)] = src[3];

    uint16 yy = WORD(overworld_map_state);

    uint16 *dst = &vram_upload_data[vram_upload_offset >> 1];
    dst[0] = Dungeon_GetKeyedObjectRelativeVramAddr(XY(0, 0) * 2, yy);
    dst[3] = Dungeon_GetKeyedObjectRelativeVramAddr(XY(0, 1) * 2, yy);
    dst[6] = Dungeon_GetKeyedObjectRelativeVramAddr(XY(1, 0) * 2, yy);
    dst[9] = Dungeon_GetKeyedObjectRelativeVramAddr(XY(1, 1) * 2, yy);

    dst[2] = src[0];
    dst[5] = src[1];
    dst[8] = src[2];
    dst[11] = src[3];

    dst[1] = 0x100;
    dst[4] = 0x100;
    dst[7] = 0x100;
    dst[10] = 0x100;

    dst[12] = 0xffff;

    vram_upload_offset += 24;
    WORD(overworld_map_state) += 2;
  } while (WORD(overworld_map_state) != dung_num_chests_x2);
  WORD(overworld_map_state) = 0;
  sound_effect_2 = 26;
  nmi_load_bg_from_vram = 1;
}

void Dung_TagRoutine_TrapdoorsUp() {
  if (dung_flag_trapdoors_down) {
    dung_flag_trapdoors_down = 0;
    dung_cur_door_pos = 0;
    door_animation_step_indicator = 0;
    sound_effect_2 = 0x1b;
    submodule_index = 5;
  }
}

void Dung_TagRoutine_0x31(int k) {
  uint8 tag = dung_hdr_tag[k];
  if (tag >= 0xb) {
    if (tag >= 0x29) {
      if (Sprite_VerifyAllOnScreenDefeated())
        Dung_TagRoutine_Helper(k);
    } else {
      uint8 a = (dung_flag_movable_block_was_pushed ^ 1);
      if (a != BYTE(dung_flag_trapdoors_down)) {
        BYTE(dung_flag_trapdoors_down) = a;
        sound_effect_2 = 37;
        submodule_index = 5;
        dung_cur_door_pos = 0;
        door_animation_step_indicator = 0;
      }
    }
  } else {
    if (Sprite_VerifyAllOnScreenDefeated())
      Dung_TagRoutine_TrapdoorsUp();
  }
}

void Dung_TagRoutine_0x32(int k) {
  if (dung_hdr_tag[k] == 10) {
    if (Sprite_VerifyAllOnScreenDefeatedB())
      Dung_TagRoutine_TrapdoorsUp();
  } else {
    if (Sprite_VerifyAllOnScreenDefeatedB())
      Dung_TagRoutine_Helper(k);
  }
}
void Dung_TagRoutine_0x14(int k) {
  if (dung_flag_statechange_waterpuzzle && dung_flag_trapdoors_down) {
    dung_flag_trapdoors_down = 0;
    dung_cur_door_pos = 0;
    door_animation_step_indicator = 0;
    submodule_index = 5;
  }
}
void Dung_TagRoutine_0x16(int k) {
  uint16 i = -2, v;
  uint8 tmp;
  for (;;) {
    i += 2;
    if (i == dung_index_of_torches_start)
      break;
    if (dung_replacement_tile_state[i >> 1] == 5) {
      v = related_to_trapdoors_somehow;
      if (v != 0xffff)
        goto shortcut;
      break;
    }
  }
  v = !dung_flag_somaria_block_switch && !dung_flag_statechange_waterpuzzle && !Dung_CheckStarTileSwitch(&tmp);
shortcut:
  if (v != dung_flag_trapdoors_down) {
    dung_flag_trapdoors_down = v;
    dung_cur_door_pos = 0;
    door_animation_step_indicator = 0;
    if (v == 0)
      sound_effect_2 = 0x25;
    submodule_index = 5;
  }
}

int Dung_DoorSwitch_GetPos() {
  return ((link_x_coord - 1) & 0x1f8) >> 3 | ((link_y_coord + 14) & 0x1f8) << 3 | (link_is_on_lower_level ? 0x1000 : 0);

}


bool Dung_DoorSwitch_Func1(uint8 *attr_out) {
  int p, t;
  word_7E04B6 = 0;
  if (flag_is_link_immobilized || link_auxiliary_state)
    return false;
  p = Dung_DoorSwitch_GetPos();
  t = WORD(dung_bg2_attr_table[p]);
  if (t == 0x2323 || t == 0x2424)
    goto done;
  t = WORD(dung_bg2_attr_table[p += 64]);
  if (t == 0x2323 || t == 0x2424)
    goto done;
  t = WORD(dung_bg2_attr_table[p -= 63]);
  if (t == 0x2323 || t == 0x2424)
    goto done;
  t = WORD(dung_bg2_attr_table[p += 64]);
  if (t == 0x2323 || t == 0x2424)
    goto done;
  return false;
done:
  if (t != WORD(dung_bg2_attr_table[p + 64]))
    return false;
  *attr_out = t;
  word_7E04B6 = p;
  return true;
}

bool Dung_CheckStarTileSwitch(uint8 *y_out) {
  int p, t;
  word_7E04B6 = 0;
  if (flag_is_link_immobilized || link_auxiliary_state)
    return false;
  p = Dung_DoorSwitch_GetPos();
  t = WORD(dung_bg2_attr_table[p]);
  if (t == 0x2323 || t == 0x3a3a || t == 0x3b3b)
    goto done;
  t = WORD(dung_bg2_attr_table[p += 64]);
  if (t == 0x2323 || t == 0x3a3a || t == 0x3b3b)
    goto done;
  t = WORD(dung_bg2_attr_table[p -= 63]);
  if (t == 0x2323 || t == 0x3a3a || t == 0x3b3b)
    goto done;
  t = WORD(dung_bg2_attr_table[p += 64]);
  if (t == 0x2323 || t == 0x3a3a || t == 0x3b3b)
    goto done;
  return false;
done:
  if (t != WORD(dung_bg2_attr_table[p + 64]))
    return false;
  *y_out = (t == 0x3b3b);
  word_7E04B6 = p;
  return true;
}

void Dung_DoorSwitch_Func2(uint8 attr) {
  submodule_index = 5;
  if (attr == 0x23 || !word_7E04B6)
    return;
  saved_module_for_menu = submodule_index;
  submodule_index = 23;
  subsubmodule_index = 32;
  link_y_coord += 2;
  if ((WORD(dung_bg2_attr_table[word_7E04B6]) & 0xfe00) != 0x2400)
    word_7E04B6++;
  Dungeon_SpriteInducedTilemapUpdate((word_7E04B6 & 0x3f) << 3, (word_7E04B6 >> 3) & 0x1f8, 0x10);
}

void Dungeon_Submodule_17() {
  if (--subsubmodule_index)
    return;
  link_y_coord -= 2;
  Dungeon_SpriteInducedTilemapUpdate((word_7E04B6 & 0x3f) << 3, (word_7E04B6 >> 3) & 0x1f8, 0xe);
  submodule_index = saved_module_for_menu;
}


void Dung_TagRoutine_0x17(int k) {
  uint8 attr;
  if (!dung_door_switch_triggered) {
    if (Dung_DoorSwitch_Func1(&attr)) {
      dung_cur_door_pos = 0;
      door_animation_step_indicator = 0;
      sound_effect_2 = 0x25;
      Dung_DoorSwitch_Func2(attr);
      dung_flag_trapdoors_down ^= 1;
      dung_door_switch_triggered = 1;
    }
  } else {
    if (!Dung_DoorSwitch_Func1(&attr))
      dung_door_switch_triggered = 0;
  }
}
void Dung_TagRoutine_0x18(int k) {
  if (dung_flag_statechange_waterpuzzle) {
    W12SEL_copy = 3;
    W34SEL_copy = 0;
    WOBJSEL_copy = 0;
    TMW_copy = 22;
    TSW_copy = 1;
    turn_on_off_water_ctr = 1;
    Hdma_ConfigureWaterTable();
    submodule_index = 11;
    palette_filter_countdown = 0;
    darkening_or_lightening_screen = 0;
    mosaic_target_level = 31;
    flag_update_cgram_in_nmi++;
    dung_hdr_tag[1] = 0;
    dung_savegame_state_bits |= 0x800;
    dung_flag_statechange_waterpuzzle = 0;
    int dsto = ((water_hdma_var1 & 0x1ff) - 0x10) << 3 | ((water_hdma_var0 & 0x1ff) - 0x10) >> 3;
    DrawWaterThing(&dung_bg2[dsto], SrcPtr(0x1438));
    Dungeon_PrepOverlayDma_nextPrep(0, dsto * 2);
    sound_effect_2 = 0x1b;
    sound_effect_1 = 0x2e;
    nmi_copy_packets_flag = 1;
  }
}

void Dung_TagRoutine_0x19(int k) {
  if (dung_flag_statechange_waterpuzzle) {
    sound_effect_2 = 0x1b;
    sound_effect_1 = 0x2f;
    submodule_index = 12;
    subsubmodule_index = 0;
    BYTE(dung_floor_y_offs) = 1;
    dung_hdr_tag[1] = 0;
    dung_savegame_state_bits |= 0x800;
    dung_flag_statechange_waterpuzzle = 0;
    dung_cur_quadrant_upload = 0;
  }
}
void Dung_TagRoutine_0x1A_Watergate(int k) {
  if (dung_savegame_state_bits & 0x800 || !dung_flag_statechange_waterpuzzle)
    return;
  submodule_index = 13;
  subsubmodule_index = 0;
  dung_hdr_tag[1] = 0;
  dung_savegame_state_bits |= 0x800;
  dung_flag_statechange_waterpuzzle = 0;
  BYTE(water_hdma_var2) = 0;
  BYTE(spotlight_var4) = 0;
  W12SEL_copy = 3;
  W34SEL_copy = 0;
  WOBJSEL_copy = 0;
  TMW_copy = 0x16;
  TSW_copy = 1;
  CGWSEL_copy = 2;
  CGADSUB_copy = 0x62;
  save_ow_event_info[0x3b] |= 32;
  save_ow_event_info[0x7b] |= 32;
  save_dung_info[0x28] |= 0x100;
  Object_WatergateChannelWater();
  water_hdma_var0 = ((watergate_pos & 0x7e) << 2) + (dung_draw_width_indicator * 16 + dung_loade_bgoffs_h_copy + 40);
  word_7E0678 = spotlight_y_upper = (watergate_pos & 0x1f80) >> 4;
  water_hdma_var1 = word_7E0678 + dung_loade_bgoffs_v_copy;
  water_hdma_var3 = 0;
  sound_effect_2 = 0x1b;
  sound_effect_1 = 0x2f;
}
void Dung_TagRoutine_0x1B(int k) {
  // empty
}

void Dung_TagRoutine_0x1F(int k) {
  if (!dung_flag_statechange_waterpuzzle) {
    int count = 0;
    for (int i = 0; i < 16; i++)
      count += (dung_object_tilemap_pos[i] & 0x8000) != 0;
    if (count < 4)
      return;
  }
  dung_floor_move_flags++;
  WORD(dung_flag_statechange_waterpuzzle) = 0;
  dung_savegame_state_bits |= 0x1000 >> k;
  sound_effect_ambient = 7;
  flag_is_link_immobilized = 1;
  flag_unk1 = 1;
}

void MovingWall_Func1(int k) {
  int i = frame_counter & 1;
  bg1_x_offset = i ? -1 : 1;
  bg1_y_offset = -bg1_x_offset;
  if (!dung_hdr_tag[k])
    bg1_x_offset = bg1_y_offset = 0;
}

int MovingWall_Func2() {
  int t = dung_some_subpixel[1] + 0x22;
  dung_some_subpixel[1] = t;
  return t >> 8;
}

int MovingWall_Func3(int k) {
  int i = moving_wall_var2;
  if (dung_hdr_tag[k] < 0x20) {
    dung_hdr_collision = 0;
    TM_copy = 0x16;
    i += 8;
  }
  return i;
}

void Dung_TagRoutine_0x1C(int k) {
  static const int16 kMovingWall_Tab1[8] = { -63, -127, -191, -255, -71, -135, -199, -263 };

  if (!dung_floor_move_flags) {
    Dung_TagRoutine_0x1F(k);
    dung_floor_x_vel = 0;
  } else {
    flag_unk1 = 1;
    MovingWall_Func1(k);
    dung_floor_x_vel = MovingWall_Func2();
  }
  dung_floor_x_offs -= dung_floor_x_vel;
  BG1HOFS_copy2 = BG2HOFS_copy2 + dung_floor_x_offs;

  if (dung_floor_x_vel) {
    if (dung_floor_x_offs < (uint16)kMovingWall_Tab1[moving_wall_var2 >> 1] &&
        dung_floor_x_offs < (uint16)kMovingWall_Tab1[MovingWall_Func3(k) >> 1]) {
      sound_effect_2 = 0x1b;
      sound_effect_ambient = 5;
      dung_hdr_tag[k] = 0;
      flag_is_link_immobilized = 0;
      flag_unk1 = 0;
      bg1_x_offset = bg1_y_offset = 0;
    }
    nmi_subroutine_index = 5;
    nmi_load_target_addr = (moving_wall_var1 - ((-dung_floor_x_offs & 0x1f8) >> 3)) & 0x141f;
  }
}




void Dung_TagRoutine_0x1D_SecretWallLeft(int k) {
  static const uint16 kMovingWall_Tab0[8] = { 0x42, 0x82, 0xc2, 0x102, 0x4a, 0x8a, 0xca, 0x10a };

  if (!dung_floor_move_flags) {
    Dung_TagRoutine_0x1F(k);
    dung_floor_x_vel = 0;
  } else {
    flag_unk1 = 1;
    MovingWall_Func1(k);
    dung_floor_x_vel = MovingWall_Func2();
  }
  dung_floor_x_offs += dung_floor_x_vel;
  BG1HOFS_copy2 = BG2HOFS_copy2 + dung_floor_x_offs;
  if (dung_floor_x_vel) {
    if (dung_floor_x_offs >= kMovingWall_Tab0[moving_wall_var2 >> 1] &&
        dung_floor_x_offs >= kMovingWall_Tab0[MovingWall_Func3(k) >> 1]) {
      sound_effect_2 = 0x1b;
      sound_effect_ambient = 5;
      dung_hdr_tag[k] = 0;
      flag_is_link_immobilized = 0;
      flag_unk1 = 0;
      bg1_x_offset = bg1_y_offset = 0;
    }
    nmi_subroutine_index = 5;
    nmi_load_target_addr = moving_wall_var1 + ((dung_floor_x_offs & 0x1f8) >> 3);
    if (nmi_load_target_addr & 0x1020)
      nmi_load_target_addr = (nmi_load_target_addr & 0x1020) ^ 0x420;
  }
}

void Dung_TagRoutine_Func2(uint8 av) {
  uint8 yv;
  if (!dung_overlay_to_load)
    dung_overlay_to_load = av;

  if (Dung_CheckStarTileSwitch(&yv) && (av += yv) != dung_overlay_to_load) {
    dung_overlay_to_load = av;
    dung_load_ptr_offs = 0;
    subsubmodule_index = 0;
    sound_effect_2 = 27;
    submodule_index = 3;
    byte_7E04BC ^= 1;
    Dungeon_RestoreStarTileChr();
  }
}
void Dung_TagRoutine_0x21(int k) {
  Dung_TagRoutine_Func2(1);
}


void Dung_TagRoutine_0x22_0x3B(int k, uint8 j) {
  if (dung_savegame_state_bits & 0x100) {
    dung_hdr_tag[k] = 0;
    dung_overlay_to_load = j;
    dung_load_ptr_offs = 0;
    subsubmodule_index = 0;
    sound_effect_2 = 0x1b;
    submodule_index = 3;
  }
}

void Dung_TagRoutine_0x3B(int k) {
  Dung_TagRoutine_0x22_0x3B(k, 0x12);
}

void Dung_TagRoutine_0x22(int k) {
  Dung_TagRoutine_0x22_0x3B(k, 0x0);
}

void Dung_TagRoutine_0x23(int k) {
  Dung_TagRoutine_Func2(3);
}
void Dung_TagRoutine_0x24(int k) {
  uint8 yv;

  if (!Dung_CheckStarTileSwitch(&yv))
    return;

  dung_hdr_tag[k] = 0;
  dung_overlay_to_load = 5;
  dung_load_ptr_offs = 0;
  subsubmodule_index = 0;
  sound_effect_2 = 0x1b;
  submodule_index = 3;
}

// Used for bosses
void Dung_TagRoutine_0x25(int k) {
  static const uint8 kBossFinishedFallingItem[13] = { 0, 0, 1, 2, 0, 6, 6, 6, 6, 6, 3, 6, 6 };
  if (!(dung_savegame_state_bits & 0x8000))
    return;
  int t = savegame_is_darkworld ? link_has_crystals : link_which_pendants;
  if (!(t & kDungeonCrystalPendantBit[BYTE(cur_palace_index_x2) >> 1])) {
    byte_7E04C2 = 128;
    Sprite_SpawnFallingItem(kBossFinishedFallingItem[BYTE(cur_palace_index_x2) >> 1]);
  }
  dung_hdr_tag[k] = 0;
}
// Used for bosses
void Dung_TagRoutine_0x15(int k) {
  int t = savegame_is_darkworld ? link_has_crystals : link_which_pendants;
  if (t & kDungeonCrystalPendantBit[BYTE(cur_palace_index_x2) >> 1]) {
    dung_flag_trapdoors_down = 0;
    dung_cur_door_pos = 0;
    door_animation_step_indicator = 0;
    submodule_index = 5;
    dung_hdr_tag[k] = 0;
  }
}


void Dung_TagRoutine_0x26(int k) {
  if (link_x_coord & 0x100 && link_y_coord & 0x100) {
    if (Sprite_VerifyAllOnScreenDefeated()) {
      sound_effect_2 = 0x1b;
      dung_hdr_tag[k] = 0;
    }
  }
}

void Dung_TagRoutine_0x27(int k) {
  uint8 attr;
  if (!countdown_for_blink && Dung_DoorSwitch_Func1(&attr))
    Dung_TagRoutine_Helper(k);
}

void Dung_TagRoutine_BlastWallStuff(int k) {
  static const uint8 kBlastWall_Tab0[5] = { 4, 6, 0, 0, 2 };
  static const uint16 kBlastWall_Tab1[5] = { 0, 0xa, 0, 0, 0x280 };

  dung_hdr_tag[k] = 0;

  int j = -1;
  do {
    j++;
  } while ((door_type_and_slot[j] & ~1) != 0x30);
  dung_unk_blast_walls_3 = j * 2;

  int i = ((link_y_coord >> 8 & 1) + 1) * 2;
  if (dung_door_direction[j] & 2)
    i = (link_x_coord >> 8 & 1);

  messaging_buf[0x1c / 2] = kBlastWall_Tab0[i];
  j = dung_door_tilemap_address[j] + kBlastWall_Tab1[i];

  messaging_buf[0x1a / 2] = (j & 0x7e) * 4 + dung_loade_bgoffs_h_copy;
  messaging_buf[0x18 / 2] = ((j & 0x1f80) >> 4) + dung_loade_bgoffs_v_copy;
  sound_effect_2 = 27;
  BYTE(dung_unk_blast_walls_2) = 1;
  AddBlastWall();
}

void Dung_TagRoutine_0x20(int k) {
  uint8 yv;
  if (!Dung_DoorSwitch_Func1(&yv))
    return;

  Dung_TagRoutine_BlastWallStuff(k);
}

void Dung_TagRoutine_0x28(int k) {
  if (!dung_flag_statechange_waterpuzzle)
    return;
  Dung_TagRoutine_BlastWallStuff(k);
}
void Dung_TagRoutine_0x33(int k) {
  int j = 0;
  for (int i = 0; i < 16; i++)
    if (dung_object_tilemap_pos[i] & 0x8000)
      j++;
  int down = (j < 4);
  if (down != dung_flag_trapdoors_down) {
    dung_flag_trapdoors_down = down;
    dung_cur_door_pos = 0;
    door_animation_step_indicator = 0;
    sound_effect_2 = 0x1b;
    submodule_index = 5;
  }
}
void Dung_TagRoutine_0x34(int k) {
  Dung_TagRoutine_Func2(6);
}
void Dung_TagRoutine_0x35(int k) {
  Dung_TagRoutine_Func2(8);
}
void Dung_TagRoutine_0x36(int k) {
  Dung_TagRoutine_Func2(10);
}
void Dung_TagRoutine_0x37(int k) {
  Dung_TagRoutine_Func2(12);
}
void Dung_TagRoutine_0x38(int k) {
  if (!(save_ow_event_info[0x5b] & 0x20) && dung_savegame_state_bits & 0x8000) {
    Palette_RevertTranslucencySwap();
    dung_hdr_tag[0] = 0;
    PrepDungeonBossExit();
  }
}
void Dung_TagRoutine_0x39(int k) {
  Dung_TagRoutine_Func2(14);
}
void Dung_TagRoutine_0x3A(int k) {
  Dung_TagRoutine_Func2(16);
}
void Dung_TagRoutine_0x3C(int k) {
  if (!nmi_load_bg_from_vram && dung_flag_movable_block_was_pushed)
    Dung_TagRoutine_Helper(k);
}
void Dung_TagRoutine_0x3D(int tagidx) {
  for (int k = 15; k >= 0; k--) {
    if (sprite_state[k] == 4 || !(sprite_flags4[k] & 64) && sprite_state[k])
      return;
  }
  if (link_player_handler_state != kPlayerState_FallingIntoHole) {
    flag_is_link_immobilized = 26;
    submodule_index = 26;
    subsubmodule_index = 0;
    dung_hdr_tag[0] = 0;
    link_force_hold_sword_up = 1;
    button_mask_b_y = 0;
    button_b_frames = 0;
    R16 = 0x364;
  }
}
void Dung_TagRoutine_0x3E(int k) {
  int j = 0;
  for (int i = 0; i < 16; i++)
    if (dung_object_tilemap_pos[i] & 0x8000)
      j++;
  if (j >= 4)
    Dung_TagRoutine_Helper(k);
}
void Dung_TagRoutine_0x3F(int k) {
  if (Sprite_VerifyAllOnScreenDefeatedB()) {
    flag_block_link_menu = 0;
    dung_hdr_tag[1] = 0;
  }
}
static HandlerFuncK *const kDungTagroutines[] = {
  &Dung_TagRoutine_0x00,
  &Dung_TagRoutine_0x29,
  &Dung_TagRoutine_0x2A,
  &Dung_TagRoutine_0x2B,
  &Dung_TagRoutine_0x2C,
  &Dung_TagRoutine_0x2D,
  &Dung_TagRoutine_0x2E,
  &Dung_TagRoutine_0x2F,
  &Dung_TagRoutine_0x30,
  &Dung_TagRoutine_0x31,
  &Dung_TagRoutine_0x32,
  &Dung_TagRoutine_0x29,
  &Dung_TagRoutine_0x2A,
  &Dung_TagRoutine_0x2B,
  &Dung_TagRoutine_0x2C,
  &Dung_TagRoutine_0x2D,
  &Dung_TagRoutine_0x2E,
  &Dung_TagRoutine_0x2F,
  &Dung_TagRoutine_0x30,
  &Dung_TagRoutine_0x31,
  &Dung_TagRoutine_0x14,
  &Dung_TagRoutine_0x15,
  &Dung_TagRoutine_0x16,
  &Dung_TagRoutine_0x17,
  &Dung_TagRoutine_0x18,
  &Dung_TagRoutine_0x19,
  &Dung_TagRoutine_0x1A_Watergate,
  &Dung_TagRoutine_0x1B,
  &Dung_TagRoutine_0x1C,
  &Dung_TagRoutine_0x1D_SecretWallLeft,
  &Dung_TagRoutine_0x1F,
  &Dung_TagRoutine_0x1F,
  &Dung_TagRoutine_0x20,
  &Dung_TagRoutine_0x21,
  &Dung_TagRoutine_0x22,
  &Dung_TagRoutine_0x23,
  &Dung_TagRoutine_0x24,
  &Dung_TagRoutine_0x25,
  &Dung_TagRoutine_0x26,
  &Dung_TagRoutine_0x27,
  &Dung_TagRoutine_0x28,
  &Dung_TagRoutine_0x29,
  &Dung_TagRoutine_0x2A,
  &Dung_TagRoutine_0x2B,
  &Dung_TagRoutine_0x2C,
  &Dung_TagRoutine_0x2D,
  &Dung_TagRoutine_0x2E,
  &Dung_TagRoutine_0x2F,
  &Dung_TagRoutine_0x30,
  &Dung_TagRoutine_0x31,
  &Dung_TagRoutine_0x32,
  &Dung_TagRoutine_0x33,
  &Dung_TagRoutine_0x34,
  &Dung_TagRoutine_0x35,
  &Dung_TagRoutine_0x36,
  &Dung_TagRoutine_0x37,
  &Dung_TagRoutine_0x38,
  &Dung_TagRoutine_0x39,
  &Dung_TagRoutine_0x3A,
  &Dung_TagRoutine_0x3B,
  &Dung_TagRoutine_0x3C,
  &Dung_TagRoutine_0x3D,
  &Dung_TagRoutine_0x3E,
  &Dung_TagRoutine_0x3F,
};

void Dungeon_CheckStairsAndRunScripts() {
  if (!flag_skip_call_tag_routines) {
    Dungeon_DetectStaircase();
    g_ram[14] = 0;
    kDungTagroutines[dung_hdr_tag[0]](0);
    g_ram[14] = 1;
    kDungTagroutines[dung_hdr_tag[1]](1);
  }
  flag_skip_call_tag_routines = 0;
}

void Door_BlastWallExploding_Helper4(uint16 *dst, const uint16 *src) {
  for (int i = 2; i != 0; i--) {
    for (int j = 0; j < 12; j++)
      dst[XY(0, j)] = src[j];
    dst++;
    src += 12;
  }
}

void Door_BlastWallExploding_Draw(int dsto) {
  uint16 *dst = &dung_bg2[dsto];
  const uint16 *src = SrcPtr(0x31ea);
  Door_BlastWallExploding_Helper4(dst, src);
  dst += 2;
  uint16 v = src[24];
  for (int n = dung_unk_blast_walls_2 - 1; n; n--) {
    for (int j = 0; j < 12; j++)
      dst[XY(0, j)] = v;
    dst++;
  }
  Door_BlastWallExploding_Helper4(dst, src + 25);
}

void Door_BlastWallExploding_CopyToVram(uint16 dsto) {
  static const uint16 kBlastWall_Tab2[16] = { 4, 8, 0xc, 0x10, 0x14, 0x18, 0x1c, 0x20, 0x100, 0x200, 0x300, 0x400, 0x500, 0x600, 0x700, 0x800 };

  uint16 r6 = 0x80;
  uint16 r14 = 0;
  uint16 r10 = dung_unk_blast_walls_2 + 3;
  uint16 r2 = 0;

  if (!sign16(r10 - 8)) {
    r2 = r10 - 6;
    r14 = 1;
    r10 = 3;
  }
  if (!(dung_door_direction[dung_cur_door_idx >> 1] & 2))
    r6++;

  uint16 *uvdata = &uvram.t3.data[0];
  for (;;) {
    const uint16 *bg2 = &dung_bg2[dsto];
    do {
      uint16 vram_addr = Dungeon_MapVramAddrNoSwap(dsto);
      uvdata[0] = vram_addr;
      uvdata[1] = r6 | 0xa00;
      uvdata[2] = bg2[XY(0, 0)];
      uvdata[3] = bg2[XY(0, 1)];
      uvdata[4] = bg2[XY(0, 2)];
      uvdata[5] = bg2[XY(0, 3)];
      uvdata[6] = bg2[XY(0, 4)];
      uvdata[7] = vram_addr + 0x4a0;
      uvdata[8] = r6 | 0xe00;
      uvdata[9] = bg2[XY(0, 5)];
      uvdata[10] = bg2[XY(0, 6)];
      uvdata[11] = bg2[XY(0, 7)];
      uvdata[12] = bg2[XY(0, 8)];
      uvdata[13] = bg2[XY(0, 9)];
      uvdata[14] = bg2[XY(0, 10)];
      uvdata[15] = bg2[XY(0, 11)];
      dsto++, bg2++, uvdata += 16;
    } while (--r10);
    if (!r14)
      break;
    r14--;
    dsto += kBlastWall_Tab2[(r2 >> 1) + ((r6 & 1) ? 0 : 8) - 1] >> 1;
    r10 = 3;
  }
  uvdata[0] = 0xffff;
}


void Door_BlastWallExploding() {
  flag_is_link_immobilized = 6;
  flag_unk1 = 6;
  if (BYTE(messaging_buf[0]) != 6)
    return;

  word_7E045E = 0;
  g_ram[12] = 0;
  door_animation_step_indicator = 0;
  dung_cur_door_idx = dung_unk_blast_walls_3;
  int dsto = (dung_door_tilemap_address[dung_unk_blast_walls_3 >> 1] -= 2) >> 1;

  Door_BlastWallExploding_Draw(dsto);
  Door_BlastWallExploding_CopyToVram(dsto);

  WORD(nmi_disable_core_updates) = 0xffff;
  dung_unk_blast_walls_2 += 2;

  if (dung_unk_blast_walls_2 == 21) {
    int m = kUpperBitmasks[dung_unk_blast_walls_3 >> 1];
    dung_door_opened_incl_adjacent |= m;
    dung_door_opened |= m;

    if (dung_door_direction[dung_unk_blast_walls_3 >> 1] & 2) {
      dung_blastwall_flag_x = 1;
      quadrant_fullsize_x = 2;
    } else {
      dung_blastwall_flag_y = 1;
      quadrant_fullsize_y = 2;
    }
    WORD(quadrant_fullsize_x_cached) = WORD(quadrant_fullsize_x);
    Door_LoadBlastWallAttr(dung_unk_blast_walls_3 >> 1);
    dung_unk_blast_walls_2 = 0;
    dung_unk_blast_walls_3 = 0;
    Dungeon_SaveRoomQuadrantData();
    flag_is_link_immobilized = 0;
    flag_unk1 = 0;
  }
  nmi_copy_packets_flag = 3;
}

void Dungeon_StartInterRoomTrans(int dir) {
  static const uint8 kLimitDirectionOnOneAxis[] = { 0x3, 0x3, 0xc, 0xc };
  link_direction &= kLimitDirectionOnOneAxis[dir];
  switch (dir) {
  case 0: Dungeon_StartInterRoomTrans_Right(); break;
  case 1: Dungeon_StartInterRoomTrans_Left(); break;
  case 2: Dungeon_StartInterRoomTrans_Down(); break;
  case 3: Dungeon_StartInterRoomTrans_Up(); break;
  default:
    assert(0);
  }
}

void Dung_CheckTriggerInterRoomTrans() {
  int dir;

  if (link_y_vel != 0) {
    int y = (link_y_coord & 0x1ff);
    if ((dir = 3, y < 4) || (dir = 2, y >= 476))
      goto trigger_trans;
  }

  if (link_x_vel != 0) {
    int y = (link_x_coord & 0x1ff);
    if ((dir = 1, y < 8) || (dir = 0, y >= 489))
      goto trigger_trans;
  }
  return;

trigger_trans:
  if (!Player_IsScreenTransitionBlocked() && main_module_index == 7) {
    Dungeon_StartInterRoomTrans(dir);
    if (main_module_index == 7)
      submodule_index = 2;
  }
}

void OrientLampBg() {
  static const uint16 kOrientLampBgTab0[] = { 0, 256, 0, 256 };
  static const uint16 kOrientLampBgTab1[] = { 0, 0, 256, 256 };
  static const int16 kOrientLampBgTab2[] = { 52, -2, 56, 6 };
  static const int16 kOrientLampBgTab3[] = { 64, 64, 82, -176 };
  static const int16 kOrientLampBgTab4[] = { 128, 384, 160, 160 };

  if (!hdr_dungeon_dark_with_lantern || submodule_index == 20)
    return;

  uint8 a = link_direction_facing >> 1, idx = a;
  if (is_standing_in_doorway) {
    idx = (is_standing_in_doorway & 0xfe);
    if (idx) {
      if (a < 2)
        idx += ((uint8)(link_x_coord + 8) >= 0x80);
      else
        idx = a;
    } else {
      if (a >= 2)
        idx += ((uint8)link_y_coord >= 0x80);
      else
        idx = a;
    }
  }

  if (idx < 2) {
    BG1HOFS_copy2 = BG2HOFS_copy2 - (link_x_coord - 0x77) + kOrientLampBgTab0[idx];
    uint16 t = BG2VOFS_copy2 - (link_y_coord - 0x58) + kOrientLampBgTab1[idx] + kOrientLampBgTab2[idx] + kOrientLampBgTab3[idx];
    if ((int16)t < 0) t = 0;
    if (t > kOrientLampBgTab4[idx]) t = kOrientLampBgTab4[idx];
    BG1VOFS_copy2 = t - kOrientLampBgTab3[idx];
  } else {
    BG1VOFS_copy2 = BG2VOFS_copy2 - (link_y_coord - 0x72) + kOrientLampBgTab1[idx];
    uint16 t = BG2HOFS_copy2 - (link_x_coord - 0x58) + kOrientLampBgTab0[idx] + kOrientLampBgTab2[idx] + kOrientLampBgTab3[idx];
    if ((int16)t < 0) t = 0;
    if (t > kOrientLampBgTab4[idx]) t = kOrientLampBgTab4[idx];
    BG1HOFS_copy2 = t - kOrientLampBgTab3[idx];
  }
}

void Sprite_HandlePushedBlocks_One(int i) {
  OAM_AllocateFromRegionB(4);

  int y = (uint8)pushedblocks_y_lo[i] | (uint8)pushedblocks_y_hi[i] << 8;
  int x = (uint8)pushedblocks_x_lo[i] | (uint8)pushedblocks_x_hi[i] << 8;

  y -= BG2VOFS_copy2 + 1;
  x -= BG2HOFS_copy2;

  if (pushedblocks_some_index < 3) {
    uint8 *oam = &g_ram[oam_cur_ptr];
    oam[0] = x;
    oam[1] = y;
    oam[2] = 12;
    oam[3] = 0x20;
    g_ram[oam_ext_cur_ptr] = 2;
  }
}

void Sprite_HandlePushedBlocks() {
  for (int i = 1; i >= 0; i--)
    if (index_of_changable_dungeon_objs[i])
      Sprite_HandlePushedBlocks_One(i);
}

void Dungeon_Normal() {
  if (!(flag_custom_spell_anim_active | flag_is_link_immobilized | flag_block_link_menu)) {
    if (filtered_joypad_H & 0x10) {
      overworld_map_state = 0;
      submodule_index = 1;
      saved_module_for_menu = main_module_index;
      main_module_index = 14;
      return;
    }
    if (filtered_joypad_L & 0x40 && (uint8)cur_palace_index_x2 != 0xff && (uint8)dungeon_room_index) {
      overworld_map_state = 0;
      submodule_index = 3;
      saved_module_for_menu = main_module_index;
      main_module_index = 14;
      return;
    }
    if (joypad1H_last & 0x20 && sram_progress_indicator) {
      choice_in_multiselect_box_bak = choice_in_multiselect_box;
      dialogue_message_index = 0x186;
      uint8 bak = main_module_index;
      Main_ShowTextMessage();
      main_module_index = bak;
      subsubmodule_index = 0;
      overworld_map_state = 0;
      submodule_index = 11;
      saved_module_for_menu = main_module_index;
      main_module_index = 14;
      return;
    }
  }
  Player_Main();
}

void Dung_AnimDoor_Up_Inner(int door, int r4_door);
void Dung_AnimDoor_Down_Inner(int door, int r4_door);
void Dung_AnimDoor_Left_Inner(int door, int r4_door);
void Dung_AnimDoor_Right_Inner(int door, int r4_door);

uint8 Dung_AnimDoorB_Remap(int door, int r4_door) {
  uint8 door_type = door_type_and_slot[door] & 0xfe;
  if (dung_door_opened_incl_adjacent & kUpperBitmasks[r4_door])
    door_type = kDoorTypeRemap[door_type >> 1];
  return door_type;
}



void Object_Draw_DoorUp_4x3(uint16 src, int door) {
  const uint16 *s = SrcPtr(src);
  uint16 *dst = &dung_bg2[dung_door_tilemap_address[door] >> 1];
  for (int i = 0; i < 4; i++) {
    dst[XY(0, 0)] = s[0];
    dst[XY(0, 1)] = s[1];
    dst[XY(0, 2)] = s[2];
    dst += 1, s += 3;
  }
}

void Object_Draw_DoorDown_4x3(uint16 src, int door) {
  const uint16 *s = SrcPtr(src);
  uint16 *dst = &dung_bg2[dung_door_tilemap_address[door] >> 1];
  for (int i = 0; i < 4; i++) {
    dst[XY(0, 1)] = s[0];
    dst[XY(0, 2)] = s[1];
    dst[XY(0, 3)] = s[2];
    dst += 1, s += 3;
  }
}

void Object_Draw_DoorLeft_3x4(uint16 src, int door) {
  const uint16 *s = SrcPtr(src);
  uint16 *dst = &dung_bg2[dung_door_tilemap_address[door] >> 1];
  for (int i = 0; i < 3; i++) {
    dst[XY(0, 0)] = s[0];
    dst[XY(0, 1)] = s[1];
    dst[XY(0, 2)] = s[2];
    dst[XY(0, 3)] = s[3];
    dst += 1, s += 4;
  }
}

void Object_Draw_DoorRight_3x4(uint16 src, int door) {
  const uint16 *s = SrcPtr(src);
  uint16 *dst = &dung_bg2[dung_door_tilemap_address[door] >> 1];
  for (int i = 0; i < 3; i++) {
    dst[XY(1, 0)] = s[0];
    dst[XY(1, 1)] = s[1];
    dst[XY(1, 2)] = s[2];
    dst[XY(1, 3)] = s[3];
    dst += 1, s += 4;
  }
}


void Dung_AnimDoorB_Up(int door, int r4_door) {
  Object_Draw_DoorUp_4x3(kDoorTypeSrcData[Dung_AnimDoorB_Remap(door, r4_door) >> 1], door);
}
void Dung_AnimDoorB_Down(int door, int r4_door) {
  Object_Draw_DoorDown_4x3(kDoorTypeSrcData2[Dung_AnimDoorB_Remap(door, r4_door) >> 1], door);
}
void Dung_AnimDoorB_Left(int door, int r4_door) {
  Object_Draw_DoorLeft_3x4(kDoorTypeSrcData3[Dung_AnimDoorB_Remap(door, r4_door) >> 1], door);
}
void Dung_AnimDoorB_Right(int door, int r4_door) {
  Object_Draw_DoorRight_3x4(kDoorTypeSrcData4[Dung_AnimDoorB_Remap(door, r4_door) >> 1], door);
}

static const uint16 kDoorAnimUpSrc[] = { 0x306a, 0x306a, 0x3082, 0x309a, 0x30b2 };
static const uint16 kDoorAnimDownSrc[] = { 0x30b2, 0x30ca, 0x30e2, 0x30fa, 0x3112 };
static const uint16 kDoorAnimLeftSrc[] = { 0x3112, 0x312a, 0x3142, 0x315a, 0x3172 };
static const uint16 kDoorAnimRightSrc[] = { 0x3172, 0x318a, 0x31a2, 0x31ba, 0x31D2 };

void Dung_AnimDoor_Up_Inner(int door, int r4_door) {
  uint8 door_type = door_type_and_slot[door] & 0xfe;
  int x = door_open_closed_counter;
  if (x == 0 || x == 4) {
    Dung_AnimDoorB_Up(door, r4_door);
    return;
  }
  x += (door_type == kDoorType_StairMaskLocked2 || door_type == kDoorType_StairMaskLocked3 || door_type >= 0x42) ? 4 : 0;
  x += (door_type == kDoorType_ShuttersTwoWay || door_type == kDoorType_Shutter) ? 2 : 0;
  //  assert(x < 8);
  Object_Draw_DoorUp_4x3(kDoorAnimUpSrc[x >> 1], door);
}

void Dung_AnimDoor_Down_Inner(int door, int r4_door) {
  uint8 door_type = door_type_and_slot[door] & 0xfe;
  int x = door_open_closed_counter;
  if (x == 0 || x == 4) {
    Dung_AnimDoorB_Down(door, r4_door);
    return;
  }
  x += (door_type >= 0x42) ? 4 : 0;
  x += (door_type == kDoorType_ShuttersTwoWay || door_type == kDoorType_Shutter) ? 2 : 0;
  //  assert(x < 8);
  Object_Draw_DoorDown_4x3(kDoorAnimDownSrc[x >> 1], door);
}

void Dung_AnimDoor_Left_Inner(int door, int r4_door) {
  uint8 door_type = door_type_and_slot[door] & 0xfe;
  int x = door_open_closed_counter;
  if (x == 0 || x == 4) {
    Dung_AnimDoorB_Left(door, r4_door);
    return;
  }
  x += (door_type >= 0x42) ? 4 : 0;
  x += (door_type == kDoorType_ShuttersTwoWay || door_type == kDoorType_Shutter) ? 2 : 0;
  Object_Draw_DoorLeft_3x4(kDoorAnimLeftSrc[x >> 1], door);
}

void Dung_AnimDoor_Right_Inner(int door, int r4_door) {
  uint8 door_type = door_type_and_slot[door] & 0xfe;
  int x = door_open_closed_counter;
  if (x == 0 || x == 4) {
    Dung_AnimDoorB_Right(door, r4_door);
    return;
  }
  x += (door_type >= 0x42) ? 4 : 0;
  x += (door_type == kDoorType_ShuttersTwoWay || door_type == kDoorType_Shutter) ? 2 : 0;
  Object_Draw_DoorRight_3x4(kDoorAnimRightSrc[x >> 1], door);
}

int Dung_AnimDoor_Up(int door, int dma_ptr) {
  int pos = dung_door_tilemap_address[door];
  if ((pos & 0x1fff) >= kDoorPositionToTilemapOffs_Up[6]) {
    pos -= 0x500;
    if ((door_type_and_slot[door] & 0xfe) >= 0x42)
      pos -= 0x300;
    Dung_AnimDoor_Down_Inner(door ^ 8, door & 7);
    dma_ptr = Dungeon_PrepOverlayDma_nextPrep(dma_ptr, pos);
    Dungeon_LoadSingleDoorAttr(door ^ 8);
  }
  Dung_AnimDoor_Up_Inner(door, door & 7);
  return dma_ptr;
}
int Dung_AnimDoor_Down(int door, int dma_ptr) {
  int pos = dung_door_tilemap_address[door];
  if ((pos & 0x1fff) < kDoorPositionToTilemapOffs_Down[9]) {
    pos += 0x500;
    if ((door_type_and_slot[door] & 0xfe) >= 0x42)
      pos += 0x300;
    Dung_AnimDoor_Up_Inner(door ^ 8, door & 7);
    dma_ptr = Dungeon_PrepOverlayDma_nextPrep(dma_ptr, pos);
    Dungeon_LoadSingleDoorAttr(door ^ 8);
  }
  Dung_AnimDoor_Down_Inner(door, door & 7);
  return dma_ptr;
}
int Dung_AnimDoor_Left(int door, int dma_ptr) {
  int pos = dung_door_tilemap_address[door];
  if ((pos & 0x7ff) >= kDoorPositionToTilemapOffs_Left[6]) {
    pos -= 16;
    if ((door_type_and_slot[door] & 0xfe) >= 0x42)
      pos -= 12;
    Dung_AnimDoor_Right_Inner(door ^ 8, door & 7);
    dma_ptr = Dungeon_PrepOverlayDma_nextPrep(dma_ptr, pos);
    Dungeon_LoadSingleDoorAttr(door ^ 8);
  }
  Dung_AnimDoor_Left_Inner(door, door & 7);
  return dma_ptr;
}
int Dung_AnimDoor_Right(int door, int dma_ptr) {
  int pos = dung_door_tilemap_address[door];
  if ((pos & 0x7ff) < kDoorPositionToTilemapOffs_Right[6]) {
    pos += 16;
    if ((door_type_and_slot[door] & 0xfe) >= 0x42)
      pos += 12;
    Dung_AnimDoor_Left_Inner(door ^ 8, door & 7);
    dma_ptr = Dungeon_PrepOverlayDma_nextPrep(dma_ptr, pos);
    Dungeon_LoadSingleDoorAttr(door ^ 8);
  }
  Dung_AnimDoor_Right_Inner(door, door & 7);
  return dma_ptr;
}

void Dung_AnimTrapDoor_Up(int door) {
  Dung_AnimDoor_Up_Inner(door, door);
}

void Dung_AnimTrapDoor_Down(int door) {
  Dung_AnimDoor_Down_Inner(door, door);
}

void Dung_AnimTrapDoor_Left(int door) {
  Dung_AnimDoor_Left_Inner(door, door);
}
void Dung_AnimTrapDoor_Right(int door) {
  Dung_AnimDoor_Right_Inner(door, door);
}

int Dung_AnimDoor(int door, int dma_ptr) {
  dung_cur_door_idx = door * 2;
  dung_which_key_x2 = door * 2;
  switch (dung_door_direction[door] & 3) {
  case 0: return Dung_AnimDoor_Up(door, dma_ptr);
  case 1: return Dung_AnimDoor_Down(door, dma_ptr);
  case 2: return Dung_AnimDoor_Left(door, dma_ptr);
  case 3: return Dung_AnimDoor_Right(door, dma_ptr);
  }
  return 0;
}

void Dung_AnimTrapDoor(int door) {
  dung_cur_door_idx = door * 2;
  dung_which_key_x2 = door * 2;
  switch (dung_door_direction[door] & 3) {
  case 0: Dung_AnimTrapDoor_Up(door); break;
  case 1: Dung_AnimTrapDoor_Down(door); break;
  case 2: Dung_AnimTrapDoor_Left(door); break;
  case 3: Dung_AnimTrapDoor_Right(door); break;
  }
}

void Dung_AnimDoorB(int door) {
  dung_cur_door_idx = door * 2;
  dung_which_key_x2 = door * 2;
  switch (dung_door_direction[door] & 3) {
  case 0: Dung_AnimDoorB_Up(door, door); break;
  case 1: Dung_AnimDoorB_Down(door, door); break;
  case 2: Dung_AnimDoorB_Left(door, door); break;
  case 3: Dung_AnimDoorB_Right(door, door); break;
  }
}

void Dungeon_AnimateTrapDoors() {
  int anim_dst = 0;
  uint8 y = 2;

  if (++door_animation_step_indicator != 4) {
    y = dung_flag_trapdoors_down ? 0 : 4;
    if (door_animation_step_indicator != 8)
      goto getout;
  }
  door_open_closed_counter = y;

  for (dung_cur_door_pos = 0; dung_cur_door_pos != 0x18; dung_cur_door_pos += 2) {
    int j = dung_cur_door_pos >> 1;
    uint8 door_type = door_type_and_slot[j] & 0xfe;
    if (door_type != kDoorType_Shutter && door_type != kDoorType_ShuttersTwoWay)
      continue;

    int mask = kUpperBitmasks[j];
    if (!dung_flag_trapdoors_down) {
      if (dung_door_opened_incl_adjacent & mask)
        continue;
      if (door_animation_step_indicator == 8) {
        sound_effect_2 = 21;
        dung_door_opened_incl_adjacent ^= mask;
      }
    } else {
      if (!(dung_door_opened_incl_adjacent & mask))
        continue;
      if (door_animation_step_indicator == 8) {
        sound_effect_2 = 22;
        dung_door_opened_incl_adjacent ^= mask;
      }
    }
    Dung_AnimTrapDoor(j);
    anim_dst = Dungeon_PrepOverlayDma_nextPrep(anim_dst, dung_door_tilemap_address[j]);
    if (door_animation_step_indicator == 8)
      Dungeon_LoadToggleDoorAttr_OtherEntry(j);
  }
  dung_cur_door_pos -= 2;

  if (anim_dst != 0) {
    nmi_disable_core_updates = nmi_copy_packets_flag = 1;
getout:
    if (BYTE(door_animation_step_indicator) != 0x10)
      return;
  }
  submodule_index = 0;
  nmi_copy_packets_flag = 0;
}

void Dungeon_IntraRoomTransInit() {
  darkening_or_lightening_screen = 0;
  palette_filter_countdown = 0;
  mosaic_target_level = 31;
  unused_config_gfx = 0;
  dung_flag_somaria_block_switch = 0;
  dung_flag_statechange_waterpuzzle = 0;
  subsubmodule_index++;
}
void Dungeon_IntraRoomTransFilter() {
  if (!dung_want_lights_out) {
    subsubmodule_index++;
    return;
  }
  PaletteFilter_doFiltering();
  if (BYTE(palette_filter_countdown))
    PaletteFilter_doFiltering();
}
void Dungeon_IntraRoomTransShutDoors() {
  BYTE(dung_flag_trapdoors_down) = 0;
  BYTE(door_animation_step_indicator) = 7;
  uint8 bak = submodule_index;
  Dungeon_AnimateTrapDoors();
  submodule_index = bak;
  BYTE(palette_filter_countdown) = 31;
  mosaic_target_level = 0;
  subsubmodule_index++;
}
void Dungeon_InterRoomTrans_State8() {
  transition_counter++;
  int i = overworld_screen_transition;
  bg1_y_offset = bg1_x_offset = 0;
  uint16 t;

  if (i >= 2) {
    t = BG1HOFS_copy2 = BG2HOFS_copy2 = (BG2HOFS_copy2 + kStaircaseTab3[i]) & ~1;
    if (transition_counter >= kStaircaseTab4[i])
      link_x_coord += kStaircaseTab3[i];
  } else {
    t = BG1VOFS_copy2 = BG2VOFS_copy2 = (BG2VOFS_copy2 + kStaircaseTab3[i]) & ~1;
    if (transition_counter >= kStaircaseTab4[i])
      link_y_coord += kStaircaseTab3[i];
  }

  if ((t & 0x1fc) == (&up_down_scroll_target)[i]) {
    Player_UpdateQuadrantsVisited();
    subsubmodule_index++;
    transition_counter = 0;
    if (submodule_index == 2)
      Dungeon_Upload_BG1_Outer();
  }

}
void Dungeon_IntraRoomTrans_State4() {
  Dungeon_ResetTorchBackgroundAndPlayerInner();
  Dungeon_Staircase_Func2();
  subsubmodule_index++;
  save_dung_info[dungeon_room_index] |= dung_quadrants_visited;
}

bool Dungeon_Staircase_Func5() {
  uint8 x = kStaircaseTab2[byte_7E004E + overworld_screen_transition * 5];
  int r0 = overworld_screen_transition & 1 ? -2 : 2;
  if ((overworld_screen_transition & 2) == 0) {
    link_y_coord += r0;
    return (BYTE(link_y_coord) & 0xfe) == x;
  } else {
    link_x_coord += r0;
    return (BYTE(link_x_coord) & 0xfe) == x;
  }
}
void Dungeon_IntraRoomTrans_State5() {
  Player_UpdateDirection();
  if (!Dungeon_Staircase_Func5())
    return;
  if (byte_7E004E == 2 || byte_7E004E == 4)
    is_standing_in_doorway = 0;
  // todo: write to tiledetect_diag_state
  BYTE(force_move_any_direction) = 0;
  byte_7E004E = 0;
  overworld_screen_transition = 0;
  subsubmodule_index++;
}

void Dungeon_IntraRoomTransOpenDoors() {
  Dungeon_InitAndCacheVars();
  if (!BYTE(dung_flag_trapdoors_down)) {
    BYTE(dung_flag_trapdoors_down)++;
    BYTE(dung_cur_door_pos) = 0;
    BYTE(door_animation_step_indicator) = 0;
    submodule_index = 5;
  }
}

void Dungeon_ResetTorchBackgroundAndPlayer() {
  uint8 ts = kSpiralTab1[dung_hdr_bg2_properties], tm = 0x16;
  if (sign8(ts))
    tm = 0x17, ts = 0;
  if (dung_hdr_bg2_properties == 2)
    ts = 3;
  TM_copy = tm;
  TS_copy = ts;
  Hud_RestoreTorchBackground();
  Dungeon_ResetTorchBackgroundAndPlayerInner();
}


static PlayerHandlerFunc *const kDungeon_IntraRoomTrans[8] = {
  &Dungeon_IntraRoomTransInit,
  &Dungeon_IntraRoomTransFilter,
  &Dungeon_IntraRoomTransShutDoors,
  &Dungeon_InterRoomTrans_State8,
  &Dungeon_IntraRoomTrans_State4,
  &Dungeon_IntraRoomTrans_State5,
  &Dungeon_IntraRoomTransFilter,
  &Dungeon_IntraRoomTransOpenDoors,
};


void Dungeon_IntraRoomTrans() {
  link_y_coord_prev = link_y_coord;
  link_x_coord_prev = link_x_coord;
  Player_UpdateDirection();
  kDungeon_IntraRoomTrans[subsubmodule_index]();
}

void Dungeon_InterRoomTrans_State0() {
  uint8 bak = hdr_dungeon_dark_with_lantern;
  Dungeon_Staircase_Func1();
  hdr_dungeon_dark_with_lantern = bak;
}
void MirrorBg1Bg2Offs() {
  BG1HOFS_copy2 = BG2HOFS_copy2;
  BG1VOFS_copy2 = BG2VOFS_copy2;
}
void Dungeon_InterRoomTrans_State1() {
  Dungeon_LoadRoom();
  Dungeon_InitStarTileChr();
  Dungeon_LoadSpriteSets();
  subsubmodule_index++;
  overworld_map_state = 0;
  BYTE(dungeon_room_index2) = BYTE(dungeon_room_index);
  Dungeon_ResetSprites();
  if (!hdr_dungeon_dark_with_lantern)
    MirrorBg1Bg2Offs();
  hdr_dungeon_dark_with_lantern = 0;
}
void Dungeon_InterRoomTrans_State2() {
  if (dung_want_lights_out | dung_want_lights_out_copy) {
    PaletteFilter_doFiltering();
    if (BYTE(palette_filter_countdown))
      PaletteFilter_doFiltering();
  } else {
    subsubmodule_index++;
  }
}
void Dungeon_InterRoomTrans_State3() {
  if (dung_want_lights_out | dung_want_lights_out_copy)
    TS_copy = 0;
  Dungeon_SpiralStaircase7_Inner3();
  LoadGfxFunc1();
  MirrorBg1Bg2Offs();
  Dungeon_Upload_BG1_Outer();
  subsubmodule_index++;
}
void Dungeon_InterRoomTrans_State4() {
  Dungeon_Upload_BG2();
  subsubmodule_index++;
}
void Dungeon_InterRoomTrans_State7() {
  BG1HOFS_copy2 = BG2HOFS_copy2;
  BG1VOFS_copy2 = BG2VOFS_copy2;

  if (dungeon_room_index != 54 && dungeon_room_index != 56) {
    uint16 y = kSpiralTab1[dung_hdr_bg2_properties] ? 0x116 : 0x16;
    if (y != (TM_copy | TS_copy << 8) && (TM_copy == 0x17 || (TM_copy | TS_copy) != 0x17))
      TM_copy = y, TS_copy = y >> 8;
  }
  Dung_UpdateLightsOutColor();
}
void Dungeon_InterRoomTrans_State10() {
  if (dung_want_lights_out | dung_want_lights_out_copy)
    PaletteFilter_doFiltering();
  Dungeon_InterRoomTrans_notDarkRoom();
}
void Dungeon_InterRoomTrans_State9() {
  if (dung_want_lights_out | dung_want_lights_out_copy)
    PaletteFilter_doFiltering();
  Dungeon_InterRoomTrans_State4();
}
void Dungeon_InterRoomTrans_State12() {
  if (submodule_index == 2) {
    if (overworld_map_state != 5)
      return;
    Dungeon_Staircase_Func2();
    if (dung_want_lights_out | dung_want_lights_out_copy)
      PaletteFilter_doFiltering();
  }
  subsubmodule_index++;
  Dungeon_ResetTorchBackgroundAndPlayer();
}
void Dungeon_InterRoomTrans_State13() {
  if (dung_want_lights_out | dung_want_lights_out_copy)
    PaletteFilter_doFiltering();
  Dungeon_IntraRoomTrans_State5();
}
void Dungeon_InterRoomTrans_State15() {
  Dungeon_InitAndCacheVars();
  if (!BYTE(dung_flag_trapdoors_down) && (BYTE(dungeon_room_index) != 172 || dung_savegame_state_bits & 0x3000)) {
    BYTE(dung_flag_trapdoors_down) = 1;
    BYTE(dung_cur_door_pos) = 0;
    BYTE(door_animation_step_indicator) = 0;
    submodule_index = 5;
  }
  Dungeon_PlayMusicIfDefeated();
}
static PlayerHandlerFunc *const kDungeon_InterRoomTrans[16] = {
  &Dungeon_InterRoomTrans_State0,
  &Dungeon_InterRoomTrans_State1,
  &Dungeon_InterRoomTrans_State2,
  &Dungeon_InterRoomTrans_State3,
  &Dungeon_InterRoomTrans_State4,
  &Dungeon_InterRoomTrans_notDarkRoom,
  &Dungeon_InterRoomTrans_State4,
  &Dungeon_InterRoomTrans_State7,
  &Dungeon_InterRoomTrans_State8,
  &Dungeon_InterRoomTrans_State9,
  &Dungeon_InterRoomTrans_State10,
  &Dungeon_InterRoomTrans_State9,
  &Dungeon_InterRoomTrans_State12,
  &Dungeon_InterRoomTrans_State13,
  &Dungeon_InterRoomTrans_State2,
  &Dungeon_InterRoomTrans_State15,
};

void Dungeon_InterRoomTrans() {
  link_y_coord_prev = link_y_coord;
  link_x_coord_prev = link_x_coord;
  if (subsubmodule_index != 0) {
    if (subsubmodule_index >= 7)
      Graphics_IncrementalVramUpload();
    Dungeon_LoadAttrIncremental();
  }
  Player_UpdateDirection();
  kDungeon_InterRoomTrans[subsubmodule_index]();
}

void Dungeon_DrawOverlay(const uint8 *src) {
  for (;;) {
    dung_draw_width_indicator = 0;
    dung_draw_height_indicator = 0;
    uint16 a = WORD(*src);
    if (a == 0xffff)
      break;
    uint16 *p = &dung_bg2[(src[0] >> 2) | (src[1] >> 2) << 6];
    uint8 type = src[2];
    if (type == 0xa4) {
      p[XY(0, 1)] = p[XY(1, 1)] = p[XY(2, 1)] = p[XY(3, 1)] =
          p[XY(0, 2)] = p[XY(1, 2)] = p[XY(2, 2)] = p[XY(3, 2)] = SrcPtr(0x5aa)[0];
      p[XY(0, 0)] = p[XY(1, 0)] = p[XY(2, 0)] = p[XY(3, 0)] = SrcPtr(0x63c)[1];
      p[XY(0, 3)] = p[XY(1, 3)] = p[XY(2, 3)] = p[XY(3, 3)] = SrcPtr(0x642)[1];
    } else {
      const uint16 *sp = SrcPtr(dung_floor_2_filler_tiles);
      p[XY(0, 0)] = p[XY(2, 0)] = p[XY(0, 2)] = p[XY(2, 2)] = sp[0];
      p[XY(1, 0)] = p[XY(3, 0)] = p[XY(1, 2)] = p[XY(3, 2)] = sp[1];
      p[XY(0, 1)] = p[XY(2, 1)] = p[XY(0, 3)] = p[XY(2, 3)] = sp[4];
      p[XY(1, 1)] = p[XY(3, 1)] = p[XY(1, 3)] = p[XY(3, 3)] = sp[5];
    }
    src += 3;
  }
}

void Dungeon_ApplyOverlayAttr(int p) {
  for (int j = 0; j < 4; j++, p += 64) {
    for (int i = 0; i < 4; i++) {
      uint16 t = dung_bg2[p + i] & 0x3fe;
      dung_bg2_attr_table[p + i] = (t == 0xee || t == 0xfe) ? 0 : 0x20;
    }
  }
}

void Dungeon_ApplyOverlay() {
  const uint8 *overlay_p = kDungeonRoomOverlay + kDungeonRoomOverlayOffs[dung_overlay_to_load];
  Dungeon_DrawOverlay(overlay_p);
  int dst_pos = 0;
  for (;;) {
    uint16 a = WORD(*overlay_p);
    if (a == 0xffff)
      break;
    int p = (overlay_p[0] >> 2) | (overlay_p[1] >> 2) << 6;
    dst_pos = Dungeon_PrepOverlayDma_nextPrep(dst_pos, p * 2);
    Dungeon_ApplyOverlayAttr(p);
    overlay_p += 3;
  }
  nmi_copy_packets_flag = 1;
  submodule_index = 0;
}

void Dungeon_OpeningLockedDoor_StairMaskLocked() {
  uint16 t;
  int i;

  for (i = 0, t = 0x3030; i != dung_num_inter_room_upnorth_stairs; i += 2, t += 0x101) {}

  for (; i != dung_num_wall_upnorth_spiral_stairs; i += 2, t += 0x101) {
    int pos = dung_inter_starcases[i >> 1];
    WriteAttr2(pos + XY(1, 0), 0x5e5e);
    WriteAttr2(pos + XY(1, 1), t);
    WriteAttr2(pos + XY(1, 2), 0);
    WriteAttr2(pos + XY(1, 3), 0);
  }

  for (; i != dung_num_wall_upnorth_spiral_stairs_2; i += 2, t += 0x101) {
    int pos = dung_inter_starcases[i >> 1];
    WriteAttr2(pos + XY(1, 0), 0x5f5f);
    WriteAttr2(pos + XY(1, 1), t);
    WriteAttr2(pos + XY(1, 2), 0);
    WriteAttr2(pos + XY(1, 3), 0);
  }

  for (; i != dung_num_inter_room_upnorth_straight_stairs; i += 2, t += 0x101) {}
  for (; i != dung_num_inter_room_upsouth_straight_stairs; i += 2, t += 0x101) {}

  t = (t & 0x707) | 0x3434;

  for (; i != dung_num_inter_room_southdown_stairs; i += 2, t += 0x101) {}

  for (; i != dung_num_wall_downnorth_spiral_stairs; i += 2, t += 0x101) {
    int pos = dung_inter_starcases[i >> 1];
    WriteAttr2(pos + XY(1, 0), 0x5e5e);
    WriteAttr2(pos + XY(1, 1), t);
    WriteAttr2(pos + XY(1, 2), 0);
    WriteAttr2(pos + XY(1, 3), 0);
  }

  for (; i != dung_num_wall_downnorth_spiral_stairs_2; i += 2, t += 0x101) {
    int pos = dung_inter_starcases[i >> 1];
    WriteAttr2(pos + XY(1, 0), 0x5f5f);
    WriteAttr2(pos + XY(1, 1), t);
    WriteAttr2(pos + XY(1, 2), 0);
    WriteAttr2(pos + XY(1, 3), 0);
  }
}

void Dungeon_OpeningLockedDoor_Combined(bool skip_anim) {
  uint8 ctr = 2;
  int k, dma_ptr;

  if (skip_anim) {
    door_animation_step_indicator = 16;
    goto step12;
  }

  door_animation_step_indicator++;
  if (door_animation_step_indicator != 4) {
    if (door_animation_step_indicator != 12)
      goto middle;
step12:
    int m = kUpperBitmasks[dung_bg2_attr_table[dung_cur_door_pos] & 7];
    dung_door_opened_incl_adjacent |= m;
    dung_door_opened |= m;
    ctr = 4;
  }
  door_open_closed_counter = ctr;

  k = dung_bg2_attr_table[dung_cur_door_pos] & 0xf;
  dma_ptr = Dung_AnimDoor(k, 0);
  Dungeon_PrepOverlayDma_nextPrep(dma_ptr, dung_door_tilemap_address[k]);
  sound_effect_2 = 21;
  nmi_copy_packets_flag = 1;

middle:
  if (door_animation_step_indicator == 16) {
    Dungeon_LoadToggleDoorAttr_OtherEntry(dung_bg2_attr_table[dung_cur_door_pos] & 0xf);
    if (dung_bg2_attr_table[dung_cur_door_pos] >= 0xf0) {
      k = dung_bg2_attr_table[dung_cur_door_pos] & 0xf;
      uint8 door_type = door_type_and_slot[k];
      if (door_type >= kDoorType_StairMaskLocked0 && door_type <= kDoorType_StairMaskLocked3)
        Dungeon_OpeningLockedDoor_StairMaskLocked();
    }
    submodule_index = 0;
  }
}

void Dungeon_OpeningLockedDoor() {
  Dungeon_OpeningLockedDoor_Combined(false);
}

void Dungeon_AnimateDestroyingWeakDoor() {
  Dungeon_OpeningLockedDoor_Combined(true);
}

void Dungeon_Submodule_5_TriggerAnim() {
  Dungeon_AnimateTrapDoors();
}


void Dungeon_StraightStaircase0() {
  if (dungeon_room_index == 0x10 || dungeon_room_index == 7 || dungeon_room_index == 0x17)
    music_control = 0xf1;
  Dungeon_Teleport0();
}

void Dungeon_StraightStaircase6() {
  MirrorBg1Bg2Offs();
  Dungeon_SpiralStaircase7_Inner3();
  uint8 ts = kSpiralTab1[dung_hdr_bg2_properties];
  uint8 tm = 0x16;
  if (sign8(ts))
    tm = 0x17, ts = 0;
  TM_copy = tm;
  TS_copy = ts;
  Dungeon_Upload_BG1_Outer();
  subsubmodule_index++;
}
void Dungeon_Staircase14() {
  subsubmodule_index++;
  Dungeon_ResetTorchBackgroundAndPlayer();
}
void Dungeon_StraightStaircase15() {
  PaletteFilter_doFiltering();
  if (BYTE(darkening_or_lightening_screen))
    return;
  HIBYTE(tiledetect_which_y_pos[0]) = HIBYTE(link_y_coord) + (BYTE(link_y_coord) >= BYTE(tiledetect_which_y_pos[0]));
  Dungeon_Staircase_MusicFunc1();
  if (BYTE(dungeon_room_index) == 0x89 || BYTE(dungeon_room_index) == 0x4f)
    return;
  if (BYTE(dungeon_room_index) == 0xA7) {
    hud_floor_changed_timer = 0;
    dung_cur_floor = 1;
    return;
  }
  dung_cur_floor--;
  Dungeon_SpiralStaircase7_Inner4();
}
void Dungeon_StraightStaircase16() {
  HoleToDungeon_Helper1();
  if (submodule_index)
    return;
  submodule_index = 7;
  subsubmodule_index = 17;
  load_chr_halfslot_even_odd = 1;
  Graphics_MaybeLoadChrHalfSlot();
}
void Dungeon_StraightStaircase17() {
  if (overworld_map_state == 5) {
    Dungeon_InitAndCacheVars();
    Dungeon_PlayMusicIfDefeated();
    Graphics_MaybeLoadChrHalfSlot();
  }
}

void Dungeon_Staircase3();
void Dungeon_Staircase4();
void Dungeon_Staircase5();
void Dungeon_SpiralStaircase11();
void Dungeon_SpiralStaircase12();
void Dungeon_SpiralStaircase15();

void Dungeon_HoleStaircase() {
  MirrorBg1Bg2Offs();
  Dungeon_SpiralStaircase7_Inner3();
  uint8 ts = kSpiralTab1[dung_hdr_bg2_properties];
  uint8 tm = 0x16;
  if (sign8(ts))
    tm = 0x17, ts = 0;
  TM_copy = tm;
  TS_copy = ts;

  link_speed_modifier = 1;
  if (which_staircase_index & 4) {
    dung_cur_floor--;
    staircase_var1 = 32;
    sound_effect_1 = 0x19;
  } else {
    dung_cur_floor++;
    staircase_var1 = 48;
    sound_effect_1 = 0x17;
  }
  sound_effect_2 = 0x24;
  Dungeon_SpiralStaircase7_Inner4();
  Dungeon_InterRoomTrans_notDarkRoom();
}

void Dungeon_Submodule_6_UpFloorTrans() {
  int j;

  if (subsubmodule_index >= 3)
    Dungeon_LoadAttrIncremental();

  if (subsubmodule_index >= 13) {
    Graphics_IncrementalVramUpload();
    if (!staircase_var1)
      goto table;
    if (staircase_var1-- == 0x10)
      link_speed_modifier = 2;
    link_direction = which_staircase_index & 4 ? 4 : 8;
    Player_SomethingWithVelocity();
    Dungeon_HandleScreenScrolling();
  }
  Player_UpdateDirection();
table:
  switch (subsubmodule_index) {
  case 0: Dungeon_Teleport0(); break;
  case 1:
    PaletteFilter_doFiltering();
    if (BYTE(palette_filter_countdown))
      PaletteFilter_doFiltering();
    break;
  case 2: Dungeon_Staircase3(); break;
  case 3: Dungeon_Staircase4(); break;
  case 4: Dungeon_Staircase5(); break;
  case 5: Dungeon_Staircase6(); break;
  case 6: Dungeon_HoleStaircase(); break;
  case 7: Dungeon_InterRoomTrans_State4(); break;
  case 8: Dungeon_InterRoomTrans_notDarkRoom(); break;
  case 9: Dungeon_InterRoomTrans_State4(); break;
  case 10: Dungeon_SpiralStaircase11(); break;
  case 11: Dungeon_SpiralStaircase12(); break;
  case 12: Dungeon_SpiralStaircase11(); break;
  case 13: Dungeon_SpiralStaircase12(); break;
  case 14: Dungeon_SpiralStaircase15(); break;
  case 15: Dungeon_Staircase14(); break;
  case 16:
    if (!(BYTE(darkening_or_lightening_screen) | BYTE(palette_filter_countdown)) && overworld_map_state == 5)
      Dungeon_InitAndCacheVars();
    break;
  }
}


static PlayerHandlerFunc *const kDungeon_Submodule_7_DownFloorTrans[18] = {
  &Dungeon_StraightStaircase0,
  &PaletteFilter_doFiltering,
  &Dungeon_Staircase3,
  &Dungeon_Staircase4,
  &Dungeon_Staircase5,
  &Dungeon_Staircase6,
  &Dungeon_StraightStaircase6,
  &Dungeon_InterRoomTrans_State4,
  &Dungeon_InterRoomTrans_notDarkRoom,
  &Dungeon_InterRoomTrans_State4,
  &Dungeon_InterRoomTrans_notDarkRoom,
  &Dungeon_InterRoomTrans_State4,
  &Dungeon_InterRoomTrans_notDarkRoom,
  &Dungeon_InterRoomTrans_State4,
  &Dungeon_Staircase14,
  &Dungeon_StraightStaircase15,
  &Dungeon_StraightStaircase16,
  &Dungeon_StraightStaircase17,
};

void Dungeon_Submodule_7_DownFloorTrans() {
  if (subsubmodule_index >= 6) {
    Graphics_IncrementalVramUpload();
    Dungeon_LoadAttrIncremental();
    Dungeon_ApproachFixedColor();
  }
  kDungeon_Submodule_7_DownFloorTrans[subsubmodule_index]();
}

void Dungeon_DestroyingWeakDoor() {
  Dungeon_AnimateDestroyingWeakDoor();
}

// Used when lighting a lamp
void Dungeon_Submodule_A() {
  OrientLampBg();
  Dungeon_ApproachFixedColor();
  if ((COLDATA_copy0 & 0x1f) != overworld_fixed_color_plusminus)
    return;
  submodule_index = 0;
  subsubmodule_index = 0;
}

void Dungeon_TurnOnOffWater_Func0() {
  Dungeon_Upload_BG1_Outer();
  dung_cur_quadrant_upload += 4;
  if (++subsubmodule_index == 6) {
    dung_cur_quadrant_upload = 0;
    subsubmodule_index = 0;
    submodule_index = 0;
  }
}


void Dungeon_SetAttrForActivatedWater() {
  WORD(TMW_copy) = 0;
  for (int j = 0; j != dung_num_interpseudo_upnorth_stairs; j += 2) {
    int dsto = dung_stairs_table_1[j >> 1];
    WriteAttr2(dsto + 0, 0x003);
    WriteAttr2(dsto + 2, 0x300);
    WriteAttr1(dsto + 0, 0xa03);
    WriteAttr1(dsto + 2, 0x30a);
    WriteAttr2(dsto + XY(0, 1), 0x808);
    WriteAttr2(dsto + XY(2, 1), 0x808);
    WriteAttr1(dsto + XY(0, 1), 0x808);
    WriteAttr1(dsto + XY(2, 1), 0x808);
    WriteAttr1(dsto + XY(0, 2), 0x808);
    WriteAttr1(dsto + XY(2, 2), 0x808);
    WriteAttr1(dsto + XY(0, 3), 0x808);
    WriteAttr1(dsto + XY(2, 3), 0x808);
  }

  for (int j = 0; j != dung_num_stairs_wet; j += 2) {
    int dsto = dung_stairs_table_2[j >> 1];
    WriteAttr2(dsto + XY(0, 3), 0x003);
    WriteAttr2(dsto + XY(2, 3), 0x300);
    WriteAttr1(dsto + XY(0, 3), 0xa03);
    WriteAttr1(dsto + XY(2, 3), 0x30a);
    WriteAttr2(dsto + XY(0, 2), 0x808);
    WriteAttr2(dsto + XY(2, 2), 0x808);
    WriteAttr1(dsto + XY(0, 0), 0x808);
    WriteAttr1(dsto + XY(2, 0), 0x808);
    WriteAttr1(dsto + XY(0, 1), 0x808);
    WriteAttr1(dsto + XY(2, 1), 0x808);
    WriteAttr1(dsto + XY(0, 2), 0x808);
    WriteAttr1(dsto + XY(2, 2), 0x808);
  }
  submodule_index = 0;
  nmi_boolean = 0; // wtf
  subsubmodule_index = 0;
}

void Dungeon_SetAttrForActivatedWaterOff() {
  CGWSEL_copy = 2;
  CGADSUB_copy = 0x32;
  zelda_ppu_write(TS, 0);
  TS_copy = 0;
  W12SEL_copy = 0;
  dung_hdr_collision = 0;
  WORD(TMW_copy) = 0;
  for (int j = 0; j != dung_num_inroom_upnorth_stairs_water; j += 2) {
    int dsto = dung_stairs_table_1[j >> 1];
    WriteAttr2(dsto + XY(1, 1), 0x1d1d);
    WriteAttr2(dsto + XY(1, 2), 0x1d1d);
  }
  for (int j = 0; j != dung_num_inroom_upsouth_stairs_water; j += 2) {
    int dsto = dung_stairs_table_2[j >> 1];
    WriteAttr2(dsto + XY(1, 1), 0x1d1d);
    WriteAttr2(dsto + XY(1, 2), 0x1d1d);
  }
  flag_update_cgram_in_nmi++;
  subsubmodule_index++;
}

void Dungeon_TurnOffWater() {
  static const int8 kTurnOffWater_Tab0[16] = { -1, -1, -1, 1, -1, -1, -1, 1, -1, -1, -1, 1, -1, -1, -1, 1 };
  switch (subsubmodule_index) {
  case 0: {
    if (!(turn_on_off_water_ctr & 7)) {
      int k = (turn_on_off_water_ctr >> 2) & 3;
      if (water_hdma_var2 == water_hdma_var4) {
        Dungeon_SetAttrForActivatedWaterOff();
        return;
      }
      water_hdma_var2 += kTurnOffWater_Tab0[k];
      water_hdma_var3 += kTurnOffWater_Tab0[k];
    }
    turn_on_off_water_ctr++;
    Hdma_ConfigureWaterTable();
    break;
  }
  case 1: {
    uint16 v = SrcPtr(0x1e0)[0];
    for (int i = 0; i < 0x1000; i++)
      dung_bg1[i] = v;
    dung_cur_quadrant_upload = 0;
    subsubmodule_index++;
    break;
  }
  case 2: case 3: case 4: case 5:
    Dungeon_TurnOnOffWater_Func0();
    break;
  }
}

void Dungeon_OnOffWater_Helper(const uint16 *src, int depth) {
  int dsto = (word_7E047C >> 1) + XY(0, 2);
  uint16 *dst = &dung_bg2[dsto];
  do {
    dst[0] = src[0];
    dst[1] = src[1];
    dst[2] = src[2];
    dst[3] = src[3];
    dst += XY(0, 1), src += 4;
  } while (--depth);
  uint16 *vram = vram_upload_data;
  for (int i = 0; i < 4; i++) {
    uint16 *dst = &dung_bg2[dsto];
    vram[0] = Dungeon_MapVramAddr(dsto);
    vram[1] = 0x980;
    vram[2] = dst[XY(0, 0)];
    vram[3] = dst[XY(0, 1)];
    vram[4] = dst[XY(0, 2)];
    vram[5] = dst[XY(0, 3)];
    vram[6] = dst[XY(0, 4)];
    vram += 7, dsto++;
  }
  vram[0] = 0xffff;
  nmi_load_bg_from_vram = 1;
}

void Dungeon_SetAttrForActivatedWater();

void Dungeon_TurnOnWater() {
  int k;
  static const int8 kTurnOnWater_Tab2[4] = { 1, 1, 1, -1 };
  static const int8 kTurnOnWater_Tab1[4] = { 1, 2, 1, -1 };
  static const int8 kTurnOnWater_Tab0[4] = { 1, -1, 1, -1 };

  switch (subsubmodule_index) {
  case 0: case 1: case 2: case 3:
    Dungeon_TurnOnOffWater_Func0();
    break;
  case 4: case 5: case 6: case 7: case 8:
    if (!--turn_on_off_water_ctr) {
      turn_on_off_water_ctr = 4;
      int depth = ++subsubmodule_index - 4;
      water_hdma_var3 = 8;
      water_hdma_var5 = 0;
      water_hdma_var2 = 0x30;
      Dungeon_OnOffWater_Helper(SrcPtr(0x1654 + 0x10), depth);
    }
    break;
  case 9:
    W12SEL_copy = 3;
    W34SEL_copy = 0;
    WOBJSEL_copy = 0;
    TMW_copy = 22;
    TSW_copy = 1;
    TS_copy = 1;
    CGWSEL_copy = 2;
    CGADSUB_copy = 98;
    turn_on_off_water_ctr = 0;
    subsubmodule_index++;
    // fall through
  case 10: {
    k = (turn_on_off_water_ctr & 3);
    uint16 r0 = 0x688 - BG2VOFS_copy2 - 0x24;
    water_hdma_var3 += kTurnOnWater_Tab0[k];
    water_hdma_var5 += kTurnOnWater_Tab1[k];
    if (water_hdma_var5 >= r0) {
      dung_hdr_bg2_properties = 7;
      subsubmodule_index++;
    }
    turn_on_off_water_ctr++;
    spotlight_y_lower = 0x688 - BG2VOFS_copy2 - water_hdma_var2;
    spotlight_y_upper = spotlight_y_lower + water_hdma_var5;
    Hdma_ConfigureWaterTable_Inner(spotlight_y_upper);
    break;
  }
  case 11: {
    if (!(turn_on_off_water_ctr & 7)) {
      k = (turn_on_off_water_ctr >> 2) & 3;
      if (water_hdma_var2 == water_hdma_var4) {
        Dungeon_SetAttrForActivatedWater();
        return;
      }
      water_hdma_var2 += kTurnOnWater_Tab2[k];
      water_hdma_var3 += kTurnOnWater_Tab2[k];

      uint16 a = water_hdma_var4 - water_hdma_var2;
      if (a == 0 || a == 8)
        Dungeon_OnOffWater_Helper(SrcPtr(a == 0 ? 0x16b4 : 0x168c), 5);
    }
    turn_on_off_water_ctr++;
    Hdma_ConfigureWaterTable();
    break;
  }
  }
}


void Watergate_Main_State0() {
  dung_cur_quadrant_upload = 0;
  overworld_screen_transition = 0;
  Dungeon_Upload_BG1_Outer();
  dung_cur_quadrant_upload += 4;
  subsubmodule_index++;
}

void Watergate_Main_State1() {
  overworld_screen_transition = 0;
  Dungeon_Upload_BG1_Outer();
  dung_cur_quadrant_upload += 4;
  subsubmodule_index++;
}

void Watergate_Main_State4() {
  watergate_var1++;
  water_hdma_var3 = watergate_var1 >> 1;
  uint8 r0 = water_hdma_var3 - 8;
  BYTE(spotlight_y_upper) = word_7E0678;
  BYTE(spotlight_var4) += 1;
  BYTE(water_hdma_var2) = spotlight_var4 + r0;

  if (watergate_var1 & 0xf)
    return;

  if (watergate_var1 == 64)
    subsubmodule_index++;

  static const uint16 kWatergateSrcs1[] = { 0x12f8, 0x1348, 0x1398, 0x13e8 };
  Object_Draw_Nx4(10, SrcPtr(kWatergateSrcs1[(watergate_var1 >> 4) - 1]), &dung_bg2[watergate_pos >> 1]);
  int pos = watergate_pos;
  int n = 3;
  int dma_ptr = 0;
  do {
    dma_ptr = Dungeon_PrepOverlayDma_watergate(dma_ptr, pos, 0x881, 4);
    pos += 6;
  } while (--n);
  nmi_copy_packets_flag = 1;
}

void Watergate_Main_State5() {
  BYTE(water_hdma_var2)++;
  uint8 t = water_hdma_var2 + spotlight_y_upper;
  if (t >= 225) {
    dung_cur_quadrant_upload = 0;
    submodule_index = 0;
    subsubmodule_index = 0;
    TMW_copy = 0;
    TSW_copy = 0;
    ResetSpotlightTable();
  }
}

static PlayerHandlerFunc *const kWatergateFuncs[6] = {
  &Watergate_Main_State0,
  &Watergate_Main_State1,
  &Watergate_Main_State1,
  &Watergate_Main_State1,
  &Watergate_Main_State4,
  &Watergate_Main_State5,
};

void Dungeon_Watergate() {
  Watergate_Helper();
  kWatergateFuncs[subsubmodule_index]();
}

void Dungeon_ElevateStaircasePriority() {
  int pos = dung_inter_starcases[which_staircase_index & 3] - 4;
  word_7E048C = pos * 2;
  uint16 *dst = &dung_bg2[pos];
  for (int i = 0; i < 5; i++) {
    dst[XY(0, 0)] |= 0x2000;
    dst[XY(0, 1)] |= 0x2000;
    dst[XY(0, 2)] |= 0x2000;
    dst[XY(0, 3)] |= 0x2000;
    dst += 1;
  }
  int dp = Dungeon_PrepOverlayDma_nextPrep(0, pos * 2);
  Dungeon_PrepOverlayDma_nextPrep(dp, pos * 2 + 8);
  nmi_copy_packets_flag = 1;
}

void Dungeon_DecreaseStaircasePriority() {
  int pos = word_7E048C >> 1;
  uint16 *dst = &dung_bg2[pos];
  for (int i = 0; i < 5; i++) {
    dst[XY(0, 0)] &= ~0x2000;
    dst[XY(0, 1)] &= ~0x2000;
    dst[XY(0, 2)] &= ~0x2000;
    dst[XY(0, 3)] &= ~0x2000;
    dst += 1;
  }
  int dp = Dungeon_PrepOverlayDma_nextPrep(0, pos * 2);
  Dungeon_PrepOverlayDma_nextPrep(dp, pos * 2 + 8);
  nmi_copy_packets_flag = 1;
}

void Dungeon_LoadCustomTileAttr() {
  memcpy(&attributes_for_tile[0x140], &kDungAttrsForTile[kDungAttrsForTile_Offs[aux_tile_theme_index]], 0x80);
}

void Dung_UpdateLightsOutColor() {
  if (dung_want_lights_out | dung_want_lights_out_copy) {
    overworld_fixed_color_plusminus = kLitTorchesColorPlus[dung_want_lights_out ? dung_num_lit_torches : 3];
    Dungeon_ApproachFixedColor_variable(overworld_fixed_color_plusminus);
    mosaic_target_level = 0;
  }
  Palette_Func1();
}

void Dungeon_SpiralStaircase0() {
  Dungeon_ElevateStaircasePriority();
  if (link_is_on_lower_level) {
    TM_copy &= 0xf;
    TS_copy |= 0x10;
    link_is_on_lower_level = 3;
  }
  subsubmodule_index++;
}
void Dungeon_Staircase_Func1() {
  WORD(mosaic_level) = 0;
  darkening_or_lightening_screen = 0;
  palette_filter_countdown = 0;
  mosaic_target_level = 31;
  unused_config_gfx = 0;
  dung_num_lit_torches = 0;
  if (hdr_dungeon_dark_with_lantern) {
    CGWSEL_copy = 0x02;
    CGADSUB_copy = 0xB3;
  }
  hdr_dungeon_dark_with_lantern = 0;
  Dungeon_ResetTorchBackgroundAndPlayerInner();
  Overworld_CgramAuxToMain();
  subsubmodule_index += 1;
}
void Dungeon_SpiralStaircase1() {
  if ((dungeon_room_index == 7 || dungeon_room_index == 23 && music_unk1 != 17) && !(link_which_pendants & 1))
    music_control = 0xf1;
  staircase_var1 = (which_staircase_index & 4) ? 106 : 88;
  overworld_map_state = 0;
  Dungeon_Staircase_Func1();
}
void Dungeon_SpiralStaircase2() {
  if (staircase_var1 < 9) {
    PaletteFilter_doFiltering();
    if (palette_filter_countdown)
      PaletteFilter_doFiltering();
  }
  if (staircase_var1 != 0) {
    staircase_var1--;
    return;
  }
  tagalong_var5 = link_visibility_status = 12;
}
void Dungeon_Staircase3() {
  Dung_AdjustCoordsForNewRoom();
  Dungeon_LoadRoom();
  Dungeon_InitStarTileChr();
  LoadTransAuxGfx();
  Dungeon_LoadCustomTileAttr();
  BYTE(dungeon_room_index2) = BYTE(dungeon_room_index);
  Tagalong_Init();
  subsubmodule_index += 1;
}
void Dungeon_Staircase4() {
  PrepTransAuxGfx();
  nmi_subroutine_index = nmi_disable_core_updates = 9;
  subsubmodule_index += 1;
}
void Dungeon_Staircase5() {
  nmi_subroutine_index = nmi_disable_core_updates = 10;
  subsubmodule_index += 1;
}
void Dungeon_Staircase6() {
  LoadGfxFunc1();
  Dungeon_ResetSprites();
  Dung_UpdateLightsOutColor();
}

void Dungeon_SpiralStaircase7_Inner() {
  if (which_staircase_index & 4)
    return;
  int lf = (word_7E048C + 8) & 0x7f;
  int x = 0, p;
  while ((((p = dung_inter_starcases[x]) * 2) & 0x7f) != lf)
    x++;
  p -= 4;
  word_7E048C = p * 2;
  uint16 *dst = &dung_bg2[p];
  for (int i = 0; i < 5; i++) {
    dst[XY(0, 0)] |= 0x2000;
    dst[XY(0, 1)] |= 0x2000;
    dst[XY(0, 2)] |= 0x2000;
    dst[XY(0, 3)] |= 0x2000;
    dst += 1;
  }
}
void Dungeon_SpiralStaircase7_Inner3() {
  UpdateCompositeOfLayoutAndQuadrant();
  quadrant_fullsize_x = (dung_blastwall_flag_x || (kLayoutQuadrantFlags[composite_of_layout_and_quadrant] & (link_quadrant_x ? 2 : 1)) == 0) ? 2 : 0;
  quadrant_fullsize_y = (dung_blastwall_flag_y || (kLayoutQuadrantFlags[composite_of_layout_and_quadrant] & (link_quadrant_y ? 8 : 4)) == 0) ? 2 : 0;
  if ((uint8)dung_unk2)
    quadrant_fullsize_x = (uint8)dung_unk2;
  if ((uint8)(dung_unk2 >> 8))
    quadrant_fullsize_y = (uint8)(dung_unk2 >> 8);
}
void Dungeon_SpiralStaircase7_Inner4() {
  hud_floor_changed_timer = 1;
  sound_effect_2 = 36;
  Player_UpdateQuadrantsVisited();
}

void Dungeon_ApproachFixedColor_variable(uint8 a) {
  COLDATA_copy0 = a | 0x20;
  COLDATA_copy1 = a | 0x40;
  COLDATA_copy2 = a | 0x80;
}

void Dungeon_ApproachFixedColor() {
  uint8 a = COLDATA_copy0 & 0x1f;
  if (a == overworld_fixed_color_plusminus)
    return;
  a += (a < overworld_fixed_color_plusminus) ? 1 : -1;
  Dungeon_ApproachFixedColor_variable(a);
}

void Dungeon_SpiralStaircase7() {
  if (savegame_tagalong == 6 && BYTE(dungeon_room_index) == 100)
    savegame_tagalong = 0;
  uint8 bak = link_is_on_lower_level;
  link_y_coord += which_staircase_index & 4 ? 48 : -48;
  link_is_on_lower_level = kTeleportPitLevel2[cur_staircase_plane];
  Dungeon_SpiralStaircase7_Inner();
  link_is_on_lower_level = bak;
  link_y_coord += which_staircase_index & 4 ? -48 : 48;
  BG1HOFS_copy2 = BG2HOFS_copy2;
  BG1VOFS_copy2 = BG2VOFS_copy2;
  Dungeon_SpiralStaircase7_Inner3();
  uint8 ts = kSpiralTab1[dung_hdr_bg2_properties], tm = 0x16;
  if (sign8(ts))
    tm = 0x17, ts = 0;
  if (dung_hdr_bg2_properties == 2)
    ts = 3;
  TM_copy = tm;
  TS_copy = ts;
  dung_cur_floor += (which_staircase_index & 4) ? -1 : 1;
  staircase_var1 = 24;
  Dungeon_SpiralStaircase7_Inner4();
  Hud_RestoreTorchBackground();
  Dungeon_InterRoomTrans_notDarkRoom();
}
void Dungeon_InterRoomTrans_notDarkRoom() {
  Dungeon_Upload_BG1_Outer();
  subsubmodule_index++;
}
void Dungeon_SpiralStaircase11() {
  PaletteFilter_doFiltering();
  Dungeon_Upload_BG1_Outer();
  subsubmodule_index++;
}

uint8 Dungeon_Staircase_Func3() {
  int pos = ((link_y_coord + 12) & 0x1f8) << 3;
  pos |= ((link_x_coord + 8) & 0x1f8) >> 3;
  pos |= (link_is_on_lower_level ? 0x1000 : 0);
  uint8 a = dung_bg2_attr_table[pos];
  uint8 r = (a == 0 || a == 9) ? 0 :
    ((a &= 0x8e) == 0x80) ? 1 :
    (a == 0x82) ? 2 :
    (a == 0x84 || a == 0x88) ? 3 :
    (a == 0x86) ? 4 : 2;
  return byte_7E004E = r;
}

void Dungeon_Staircase_Func2() {
  int st = overworld_screen_transition;
  int a = Dungeon_Staircase_Func3();
  if (a == 2)
    a = 1;
  else if (a == 4)
    a = 2;
  a += overworld_screen_transition * 5;

  int8 v = kStaircaseTab2[a];
  v -= (v < 0) ? -8 : 8;
  if (st & 2)
    BYTE(link_x_coord) = v;
  else
    BYTE(link_y_coord) = v;
  link_visibility_status = 0;
}
void Dungeon_Staircase_MusicFunc1() {
  uint8 x = 0x1c;
  if (dungeon_room_index != 16) {
    x = 0x15;
    if (dungeon_room_index != 7) {
      x = 0x11;
      if (dungeon_room_index != 23 || music_unk1 == 17)
        return;
    }
    if (music_unk1 != 0xf1 && (link_which_pendants & 1))
      return;
  }
  music_control = x;
}



void Dungeon_PlayMusicIfDefeated() {
  uint8 x = 0x14;
  if (dungeon_room_index != 18) {
    x = 0x10;
    if (dungeon_room_index != 2) {
      if (FindInWordArray(kBossRooms, dungeon_room_index, countof(kBossRooms)) < 0)
        return;
      if (Sprite_VerifyAllOnScreenDefeated())
        return;
      x = 0x15;
    }
  }
  music_control = x;
}

void Dungeon_InitAndCacheVars() {
  overworld_map_state = 0;
  subsubmodule_index = 0;
  overworld_screen_transition = 0;
  submodule_index = 0;
  dung_flag_statechange_waterpuzzle = 0;
  dung_flag_movable_block_was_pushed = 0;
  Player_CacheStatePriorToHandler();
}

void Dungeon_SpiralStaircase12() {
  PaletteFilter_doFiltering();
  Dungeon_Upload_BG2();
  subsubmodule_index++;
}

void Dungeon_SpiralStaircase15() {
  PaletteFilter_doFiltering();
  PaletteFilter_doFiltering();
  Dungeon_ApproachFixedColor();
}
void Dungeon_SpiralStaircase16() {
  Dungeon_ResetTorchBackgroundAndPlayerInner();
  staircase_var1 = 0x38;
  subsubmodule_index++;
  Dungeon_Staircase_MusicFunc1();
}
void UsedForSpiralStaircase2() {
  link_give_damage = 0;
  link_incapacitated_timer = 0;
  link_auxiliary_state = 0;
  link_disable_sprite_damage = 0;
  link_x_coord_prev = link_x_coord;
  link_y_coord_prev = link_y_coord;
  if (sign8(--countdown_timer_for_staircases)) {
    countdown_timer_for_staircases = 0;
    link_direction_facing = 2;
  }
  link_actual_vel_x = 4, link_actual_vel_y = 0;
  if (which_staircase_index & 4)
    link_actual_vel_x = -4, link_actual_vel_y = 2;
  if (some_animation_timer_steps == 2)
    link_actual_vel_x = 0, link_actual_vel_y = 16;
  Player_MovePosition1();
  Player_UpdateDirection_Part2();
  if ((uint8)link_x_coord == (uint8)tiledetect_which_y_pos[1])
    some_animation_timer_steps = 2;
}
void Dungeon_SpiralStaircase17() {
  UsedForSpiralStaircase2();
  if (!--staircase_var1) {
    staircase_var1 = which_staircase_index & 4 ? 10 : 24;
    subsubmodule_index++;
  }
}
void Dungeon_SpiralStaircase18() {
  UsedForSpiralStaircase2();
  if (!--staircase_var1) {
    subsubmodule_index++;
    overworld_map_state = 0;
  }
}
void Dungeon_SpiralStaircase19() {
  link_is_on_lower_level_mirror = kTeleportPitLevel1[cur_staircase_plane];
  link_is_on_lower_level = kTeleportPitLevel2[cur_staircase_plane];
  TM_copy |= 0x10;
  TS_copy &= 0xf;
  if (!(which_staircase_index & 4))
    Dungeon_DecreaseStaircasePriority();
  BYTE(dungeon_room_index2) = BYTE(dungeon_room_index);
  Dungeon_InitAndCacheVars();
}

static const int8 kSpiralStaircaseX[] = { -28, -28, 24, 24 };
static const int8 kSpiralStaircaseY[] = { 16, -10, -10, -32 };

void UsedForSpiralStaircase_Helper3() {
  link_visibility_status = 0;
  tagalong_var5 = 0;

  int i = (cur_staircase_plane == 0 && byte_7E0492 != 0) ? 1 : 0;
  i += (which_staircase_index & 4) ? 2 : 0;

  link_x_coord += kSpiralStaircaseX[i];
  link_y_coord += kSpiralStaircaseY[i];

  if (TM_copy & 0x10) {
    if (cur_staircase_plane == 2) {
      link_is_on_lower_level = 3;
      TM_copy &= 0xf;
      TS_copy |= 0x10;
      if (byte_7E0492 != 2)
        link_y_coord += 24;
    }
    Tagalong_Init();
  } else {
    if (cur_staircase_plane != 2) {
      TM_copy |= 0x10;
      TS_copy &= 0xf;
      if (byte_7E0492 != 2)
        link_y_coord -= 24;
    }
    Tagalong_Init();
  }
}


void Player_UsedForSpiralStaircase() {
  link_x_coord_prev = link_x_coord;
  link_y_coord_prev = link_y_coord;
  if (some_animation_timer_steps)
    return;

  link_give_damage = 0;
  link_incapacitated_timer = 0;
  link_auxiliary_state = 0;

  if (which_staircase_index & 4) {
    link_actual_vel_y = -2;
    if (sign8(--link_timer_push_get_tired)) {
      link_timer_push_get_tired = 0;
      link_actual_vel_y = 0;
      link_actual_vel_x = -2;
    }
  } else {
    link_actual_vel_y = -2;
    if (sign8(--link_timer_push_get_tired)) {
      link_timer_push_get_tired = 0;
      link_actual_vel_y = -2;
      link_actual_vel_x = 2;
    }
  }
  Player_MovePosition1();
  Player_UpdateDirection_Part2();
  if (!link_timer_push_get_tired && sign8(--countdown_timer_for_staircases)) {
    countdown_timer_for_staircases = 0;
    link_direction_facing = (which_staircase_index & 4) ? 4 : 6;
  }

  int8 xd = link_x_coord - tiledetect_which_y_pos[1];
  if (xd < 0)
    xd = -xd;
  if (xd)
    return;

  UsedForSpiralStaircase_Helper3();
  if (savegame_tagalong)
    Tagalong_Init();

  tiledetect_which_y_pos[1] = link_x_coord + ((which_staircase_index & 4) ? -8 : 12);
  some_animation_timer_steps = 1;
  countdown_timer_for_staircases = 6;
  Player_DoSfx2(which_staircase_index & 4 ? 25 : 23);
}

static PlayerHandlerFunc *const kDungeon_SpiralStaircase[20] = {
  &Dungeon_SpiralStaircase0,
  &Dungeon_SpiralStaircase1,
  &Dungeon_SpiralStaircase2,
  &Dungeon_Staircase3,
  &Dungeon_Staircase4,
  &Dungeon_Staircase5,
  &Dungeon_Staircase6,
  &Dungeon_SpiralStaircase7,
  &Dungeon_InterRoomTrans_State4,
  &Dungeon_InterRoomTrans_notDarkRoom,
  &Dungeon_InterRoomTrans_State4,
  &Dungeon_SpiralStaircase11,
  &Dungeon_SpiralStaircase12,
  &Dungeon_SpiralStaircase11,
  &Dungeon_SpiralStaircase12,
  &Dungeon_SpiralStaircase15,
  &Dungeon_SpiralStaircase16,
  &Dungeon_SpiralStaircase17,
  &Dungeon_SpiralStaircase18,
  &Dungeon_SpiralStaircase19,
};

void Dungeon_SpiralStaircase() {
  if (subsubmodule_index >= 7) {
    Graphics_IncrementalVramUpload();
    Dungeon_LoadAttrIncremental();
  }
  Player_UsedForSpiralStaircase();
  kDungeon_SpiralStaircase[subsubmodule_index]();
}

void Dungeon_Submodule_F_State0() {
  Spotlight_open();
  subsubmodule_index++;
}

void Dungeon_Submodule_F_State1() {
  Sprite_Main();
  ConfigureSpotlightTable();
  if (!submodule_index) {
    W12SEL_copy = 0;
    W34SEL_copy = 0;
    WOBJSEL_copy = 0;
    TMW_copy = 0;
    TSW_copy = 0;
    subsubmodule_index = 0;
    if (buffer_for_playing_songs != 0xff)
      music_control = buffer_for_playing_songs;
  }
}

static PlayerHandlerFunc *const kDungeon_Submodule_F[2] = {
  &Dungeon_Submodule_F_State0,
  &Dungeon_Submodule_F_State1,
};

void Dungeon_Submodule_F() {
  kDungeon_Submodule_F[subsubmodule_index]();
  Player_UpdateDirection();
  PlayerOam_Main();
}

void Dungeon_Straight_StaircaseUp_State0() {
  uint8 v1 = 0x3c, sfx = 25;
  if (link_direction & 4) {
    v1 = 0x38, sfx = 23;
    link_is_on_lower_level_mirror ^= 1;
    if ((uint8)kind_of_in_room_staircase != 2)
      link_is_on_lower_level ^= 1;
  }
  staircase_var1 = v1;
  sound_effect_1 = sfx;
  link_speed_modifier = 1;
  subsubmodule_index++;
}

void Dungeon_Straight_StaircaseUp_State1() {
  if (staircase_var1)
    return;
  if (link_direction & 8) {
    link_is_on_lower_level_mirror ^= 1;
    if ((uint8)kind_of_in_room_staircase != 2)
      link_is_on_lower_level ^= 1;
  }
  subsubmodule_index = 0;
  overworld_screen_transition = 0;
  submodule_index = 0;
  Player_UpdateQuadrantsVisited();
}

static PlayerHandlerFunc *const kDungeon_StraightStaircase[2] = {
  &Dungeon_Straight_StaircaseUp_State0,
  &Dungeon_Straight_StaircaseUp_State1,
};


// straight staircase going up when walking south
void Dungeon_Straight_Staircase() {
  uint8 t = staircase_var1;
  if (t) {
    staircase_var1--;
    if (t == 20)
      link_speed_modifier = 2;
    Player_SomethingWithVelocity();
    Player_CheckCrossQuadrantBoundary();
    Dungeon_HandleScreenScrolling();
    Player_UpdateDirection();
  }
  kDungeon_StraightStaircase[subsubmodule_index]();
}

void Dungeon_Straight_StaircaseDown_State0() {
  draw_water_ripples_or_grass = 0;

  uint8 v1 = 0x3c, sfx = 25;
  if (link_direction & 8) {
    v1 = 0x38, sfx = 23;
    link_is_on_lower_level_mirror = 0;
    if ((uint8)kind_of_in_room_staircase != 2)
      link_is_on_lower_level = 0;
  }
  staircase_var1 = v1;
  sound_effect_1 = sfx;
  link_speed_modifier = 1;
  subsubmodule_index++;
}

void Dungeon_Straight_StaircaseDown_State1() {
  if (staircase_var1)
    return;
  if (link_direction & 4) {
    link_is_on_lower_level_mirror = 1;
    if ((uint8)kind_of_in_room_staircase != 2)
      link_is_on_lower_level = 1;
  }
  subsubmodule_index = 0;
  overworld_screen_transition = 0;
  submodule_index = 0;
  Player_UpdateQuadrantsVisited();
}

static PlayerHandlerFunc *const kDungeon_StraightStaircaseDown[2] = {
  &Dungeon_Straight_StaircaseDown_State0,
  &Dungeon_Straight_StaircaseDown_State1,
};


// straight staircase going down when walking south
void Dungeon_Straight_StaircaseDown() {
  uint8 t = staircase_var1;
  if (t) {
    staircase_var1--;
    if (t == 20)
      link_speed_modifier = 2;
    Player_SomethingWithVelocity();
    Player_CheckCrossQuadrantBoundary();
    Dungeon_HandleScreenScrolling();
    Player_UpdateDirection();
  }
  kDungeon_StraightStaircaseDown[subsubmodule_index]();
}

void Dungeon_Teleport0() {
  overworld_map_state = 0;
  Dungeon_Staircase_Func1();
}

void StraightStairs_0() {
  if (link_is_running) {
    link_is_running = 0;
    link_speed_setting = 2;
  }
  sound_effect_1 = (which_staircase_index & 4) ? 24 : 22;
  if (dungeon_room_index == 48 || dungeon_room_index == 64)
    music_control = 0xf1;
  Dungeon_Teleport0();
}
void StraightStairs_1() {
  if (staircase_var1 < 9) {
    PaletteFilter_doFiltering();
    if (BYTE(palette_filter_countdown) == 23)
      subsubmodule_index++;
  }
}
void StraightStairs_2() {
  PaletteFilter_doFiltering();
  Dungeon_LoadRoom();
  Dungeon_RestoreStarTileChr();
  LoadTransAuxGfx();
  Dungeon_LoadCustomTileAttr();
  Dungeon_SpiralStaircase7_Inner3();
  Tagalong_Init();
  subsubmodule_index++;
}
void StraightStairs_3() {
  PaletteFilter_doFiltering();
  Dungeon_Staircase4();
}
void StraightStairs_4() {
  PaletteFilter_doFiltering();
  Dungeon_Staircase5();
  BYTE(dungeon_room_index2) = BYTE(dungeon_room_index);
  Dungeon_ResetSprites();
}
void StraightStairs_9() {
  PaletteFilter_doFiltering();
  subsubmodule_index--;
  LoadGfxFunc1();
  Palette_Func1();
}
void StraightStairs_10() {
  link_visibility_status = tagalong_var5 = 12;
  int i = overworld_screen_transition;
  BG1VOFS_copy2 = BG2VOFS_copy2 = (BG2VOFS_copy2 + kStaircaseTab3[i]) & ~3;
  if ((BG1VOFS_copy2 & 0x1fc) == (&up_down_scroll_target)[i]) {
    if (submodule_index >= 18)
      i += 2;
    link_y_coord += kStaircaseTab5[i];
    link_visibility_status = tagalong_var5 = 0;
    subsubmodule_index++;
  }
}
void StraightStairs_11() {
  uint8 ts = kSpiralTab1[dung_hdr_bg2_properties], tm = 0x16;
  if (sign8(ts))
    tm = 0x17, ts = 0;
  TM_copy = tm;
  TS_copy = ts;

  link_speed_modifier = 1;
  dung_cur_floor += (which_staircase_index & 4) ? -1 : 1;
  staircase_var1 = (which_staircase_index & 4) ? 0x32 : 0x3c;
  sound_effect_1 = (which_staircase_index & 4) ? 25 : 23;

  uint8 r0 = 0;
  if (link_is_on_lower_level) {
    link_y_coord += (submodule_index == 18) ? -32 : 32;
    r0++;
  }
  link_is_on_lower_level_mirror = kTeleportPitLevel1[cur_staircase_plane];
  link_is_on_lower_level = kTeleportPitLevel2[cur_staircase_plane];
  if (link_is_on_lower_level) {
    link_y_coord += (submodule_index == 18) ? -32 : 32;
    r0++;
  }

  if (!r0) {
    if (submodule_index == 18) {
      link_y_coord += (which_staircase_index & 4) ? -24 : -8;
    } else {
      link_y_coord += 12;
    }
  }

  Dungeon_SpiralStaircase7_Inner4();
  Hud_RestoreTorchBackground();
  Dungeon_InterRoomTrans_notDarkRoom();
}

void StraightStairs_16() {
  if (overworld_map_state == 5 && !BYTE(darkening_or_lightening_screen)) {
    subsubmodule_index++;
    if (dungeon_room_index == 48)
      music_control = 0x1c;
    else if (dungeon_room_index == 64)
      music_control = 0x10;
  }
  Dungeon_ApproachFixedColor();
}
void StraightStairs_17() {
  if (staircase_var1 == 0)
    subsubmodule_index++;
  else
    Dungeon_ApproachFixedColor();
}

static PlayerHandlerFunc *const kDungeon_StraightStairs[19] = {
  &StraightStairs_0,
  &StraightStairs_1,
  &StraightStairs_2,
  &StraightStairs_3,
  &StraightStairs_4,
  &Dungeon_SpiralStaircase11,
  &Dungeon_SpiralStaircase12,
  &Dungeon_SpiralStaircase11,
  &Dungeon_SpiralStaircase12,
  &StraightStairs_9,
  &StraightStairs_10,
  &StraightStairs_11,
  &Dungeon_InterRoomTrans_State4,
  &Dungeon_InterRoomTrans_notDarkRoom,
  &Dungeon_InterRoomTrans_State4,
  &Dungeon_SpiralStaircase15,
  &StraightStairs_16,
  &StraightStairs_17,
  &Dungeon_InitAndCacheVars,
};

// This is used for straight inter room stairs for example stairs to throne room in first dung
void Dungeon_StraightStairs() {
  if (subsubmodule_index >= 3)
    Dungeon_LoadAttrIncremental();
  if (subsubmodule_index >= 13)
    Graphics_IncrementalVramUpload();
  if (staircase_var1) {
    if (staircase_var1-- == 16)
      link_speed_modifier = 2;
    link_direction = (submodule_index == 18) ? 8 : 4;
    Player_SomethingWithVelocity();
  }
  Player_UpdateDirection();
  kDungeon_StraightStairs[subsubmodule_index]();
}

void Dungeon_Submodule_14_FallGap_0();
void Overworld_Func2A_State1();

void Dungeon_Submodule_14_FallGap() {
  switch (subsubmodule_index) {
  case 0:
    Dungeon_Submodule_14_FallGap_0();
    break;
  case 1:
    Overworld_Func2A_State1();
    break;
  }
}

void Dungeon_Submodule_14_FallGap_0() {
  for (int i = 0; i < 2; i++) {
    if (BG2HOFS_copy2 != BG2HOFS_copy2_cached)
      BG2HOFS_copy2 += BG2HOFS_copy2 < BG2HOFS_copy2_cached ? 1 : -1;
    if (BG2VOFS_copy2 != BG2VOFS_copy2_cached)
      BG2VOFS_copy2 += BG2VOFS_copy2 < BG2VOFS_copy2_cached ? 1 : -1;
  }
  if (BG2HOFS_copy2 == BG2HOFS_copy2_cached && BG2VOFS_copy2 == BG2VOFS_copy2_cached)
    subsubmodule_index++;
  if (!hdr_dungeon_dark_with_lantern)
    MirrorBg1Bg2Offs();
}

void Dungeon_Teleport1() {
  Overworld_ResetMosaic();
  MOSAIC_copy = mosaic_level | 3;
  PaletteFilter_doFiltering();
}
void Dungeon_Teleport4() {
  Dungeon_ApproachFixedColor();
  if (dungeon_room_index == 0x17)
    dung_cur_floor = 4;
  MirrorBg1Bg2Offs();
  Dungeon_SpiralStaircase7_Inner3();
  uint8 ts = kSpiralTab1[dung_hdr_bg2_properties], tm = 0x16;
  if (sign8(ts))
    tm = 0x17, ts = 0;
  TM_copy = tm;
  TS_copy = ts;
  Dungeon_Upload_BG1_Outer();
  subsubmodule_index++;
}
void Dungeon_Teleport13() {
  if (palette_filter_countdown & 1 && mosaic_level != 0)
    mosaic_level -= 0x10;
  BGMODE_copy = 9;
  MOSAIC_copy = mosaic_level | 3;
  PaletteFilter_doFiltering();
}
void Dungeon_Teleport14() {
  if (overworld_map_state == 5) {
    Player_UpdateQuadrantsVisited();
    submodule_index = 0;
    Dungeon_InitAndCacheVars();
  }
}
static PlayerHandlerFunc *const kDungeon_Teleport[15] = {
  &Dungeon_Teleport0,
  &Dungeon_Teleport1,
  &Dungeon_Staircase3,
  &Dungeon_Staircase6,
  &Dungeon_Teleport4,
  &Dungeon_InterRoomTrans_State4,
  &Dungeon_InterRoomTrans_notDarkRoom,
  &Dungeon_InterRoomTrans_State4,
  &Dungeon_InterRoomTrans_notDarkRoom,
  &Dungeon_InterRoomTrans_State4,
  &Dungeon_InterRoomTrans_notDarkRoom,
  &Dungeon_InterRoomTrans_State4,
  &Dungeon_Staircase14,
  &Dungeon_Teleport13,
  &Dungeon_Teleport14,
};

void Dungeon_Teleport() {
  if (subsubmodule_index >= 3) {
    Graphics_IncrementalVramUpload();
    Dungeon_LoadAttrIncremental();
  }
  kDungeon_Teleport[subsubmodule_index]();
}
void Dungeon_Submodule_16() {
  if (++subsubmodule_index & 3)
    return;
  switch (subsubmodule_index >> 2) {
  case 0:
  case 1: Dungeon_OrangeBlueBarrierUpload_A(); break;
  case 2: Dungeon_OrangeBlueBarrierUpload_B(); break;
  case 3: Dungeon_OrangeBlueBarrierUpload_C(); break;
  case 4:
    Dungeon_ToggleBarrierAttr();
    subsubmodule_index = 0;
    submodule_index = 0;
    break;
  }
}

void CrystalMaiden_SpawnAndConfigMaiden() {
  memset(sprite_state, 0, 16);
  SpriteSpawnInfo info;
  int j = Sprite_SpawnDynamically(0, 0xab, &info);
  sprite_x_hi[j] = link_x_coord >> 8;
  sprite_y_hi[j] = link_y_coord >> 8;
  sprite_x_lo[j] = 0x78;
  sprite_y_lo[j] = 0x7c;
  sprite_D[j] = 1;
  sprite_oam_flags[j] = 0xb;
  sprite_subtype2[j] = 0;
  sprite_floor[j] = 0;
  sprite_A[j] = Ancilla_TerminateSelectInteractives(j);
  item_receipt_method = 0;
  if (BYTE(cur_palace_index_x2) == 24) {
    sprite_oam_flags[j] = 9;
    savegame_tagalong = 1;
  } else {
    savegame_tagalong = 6;
  }
  Tagalong_LoadGfx();
  savegame_tagalong = 0;
  dung_floor_x_offs = BG2HOFS_copy2 - link_x_coord + 0x79;
  dung_floor_y_offs = 0x30 - (uint8)BG1VOFS_copy2;
  dung_hdr_collision_2_mirror = 1;
}

void CrystalMaiden_Configure() {
  static const uint16 kCrystalMaiden_Pal[8] = { 0, 0x3821, 0x4463, 0x54a5, 0x5ce7, 0x6d29, 0x79ad, 0x7e10 };

  CGADSUB_copy = 0x33;
  BYTE(palette_filter_countdown) = 0;
  BYTE(darkening_or_lightening_screen) = 0;
  Palette_AssertTranslucencySwap();
  PaletteFilter_Crystal();
  for (int i = 0; i < 8; i++)
    main_palette_buffer[112 + i] = kCrystalMaiden_Pal[i];
  flag_update_cgram_in_nmi++;
  CrystalMaiden_SpawnAndConfigMaiden();
  CrystalMaiden_InitPolyhedral();
}

void Dungeon_Crystal() {
  switch (subsubmodule_index) {
  case 0:
    PaletteFilter_Restore_Strictly_Bg_Subtractive();
    main_palette_buffer[0] = main_palette_buffer[32];
    if (BYTE(darkening_or_lightening_screen) != 255)
      return;
    for (int i = 0; i < 0x1000; i++)
      dung_bg2[i] = dung_bg1[i] = 0x1ec;
    bg1_y_offset = 0;
    bg1_x_offset = 0;
    dung_floor_x_offs = 0;
    dung_floor_y_offs = 0;
    overworld_screen_transition = 0;
    dung_cur_quadrant_upload = 0;
    subsubmodule_index++;
    break;
  case 1: {
    static const uint16 kCrystal_Tab0[7] = { 0x1618, 0x1658, 0x1658, 0x1618, 0x658, 0x1618, 0x1658 };
    PaletteFilter_Crystal();
    TS_copy = 1;
    flag_is_link_immobilized = 2;
    int j = FindInWordArray(kBossRooms, dungeon_room_index, countof(kBossRooms)) - 4;
    uint16 *dst = &dung_bg1[kCrystal_Tab0[j] >> 1];
    for (int n = 0, t = 0; n != 4; n++) {
      for (int i = 0; i != 8; i++, t++) {
        dst[i + XY(0, 0)] = 0x1f80 | t;
        dst[i + XY(0, 4)] = 0x1f88 | t;
      }
      t += 8, dst += XY(0, 1);
    }
    subsubmodule_index++;
    break;
  }
  case 2: case 4: case 6: case 8:
    Dungeon_InterRoomTrans_notDarkRoom();
    break;
  case 3: case 5: case 7: case 9:
    Dungeon_InterRoomTrans_State4();
    break;
  case 10:
    is_nmi_thread_active++;
    Polyhedral_InitThread();
    CrystalMaiden_Configure();
    submodule_index = 0;
    subsubmodule_index = 0;
    break;
  }
}
void Dungeon_Submodule_19() {
  // When using mirror
  Overworld_ResetMosaic_alwaysIncrease();
  if (!--INIDISP_copy) {
    main_module_index = 5;
    submodule_index = 0;
    nmi_load_bg_from_vram = 0;
    last_music_control = music_unk1;
    if (overworld_palette_swap_flag)
      Palette_RevertTranslucencySwap();
  }
}

void Dungeon_OpenGanonDoor() {
  static const uint16 kOpenGanonDoor_Tab[4] = { 0x2556, 0x2596, 0x25d6, 0x2616 };

  flag_is_link_immobilized = 1;
  if (R16 != 0) {
    if (--BYTE(R16) || --HIBYTE(R16))
      return;
    sound_effect_ambient = 21;
    link_force_hold_sword_up = 0;
    link_cant_change_direction = 0;
  }
  flag_is_link_immobilized = 0;
  if (++subsubmodule_index & 3)
    return;

  const uint16 *src = SrcPtr(kOpenGanonDoor_Tab[(subsubmodule_index - 4) >> 2]);
  uint16 *dst = &dung_bg2[0];
  for (int i = 0; i < 8; i++) {
    dst[XY(44, 3)] = src[0];
    dst[XY(44, 4)] = src[1];
    dst[XY(44, 5)] = src[2];
    dst[XY(44, 6)] = src[3];
    dst += XY(1, 0), src += 4;
  }

  Dungeon_PrepOverlayDma_watergate(0, 0x1d8, 0x881, 8);
  if (subsubmodule_index == 16) {
    WriteAttr2(XY(44, 5), 0x202);
    WriteAttr2(XY(44, 6), 0x202);
    WriteAttr2(XY(50, 5), 0x200);
    WriteAttr2(XY(50, 6), 0x200);
    for (int i = 0; i != 6; i += 2) {
      WriteAttr2(XY(45 + i, 0), 0x0);
      WriteAttr2(XY(45 + i, 1), 0x0);
      WriteAttr2(XY(45 + i, 2), 0x0);
      WriteAttr2(XY(45 + i, 3), 0x0);
      WriteAttr2(XY(45 + i, 4), 0x0);
      WriteAttr2(XY(45 + i, 5), 0x0);
      WriteAttr2(XY(45 + i, 6), 0x0);
    }
    room_bounds_y.a0 = -64;
    submodule_index = 0;
    subsubmodule_index = 0;
  }
  nmi_copy_packets_flag = 1;
}

static PlayerHandlerFunc *const kDungeonSubmodules[31] = {
  &Dungeon_Normal,
  &Dungeon_IntraRoomTrans,
  &Dungeon_InterRoomTrans,
  &Dungeon_ApplyOverlay,
  &Dungeon_OpeningLockedDoor,
  &Dungeon_Submodule_5_TriggerAnim,
  &Dungeon_Submodule_6_UpFloorTrans,
  &Dungeon_Submodule_7_DownFloorTrans,
  &Dungeon_Straight_StaircaseDown,
  &Dungeon_DestroyingWeakDoor,
  &Dungeon_Submodule_A,
  &Dungeon_TurnOffWater,
  &Dungeon_TurnOnWater,
  &Dungeon_Watergate,
  &Dungeon_SpiralStaircase,
  &Dungeon_Submodule_F,
  &Dungeon_Straight_Staircase,
  &Dungeon_StraightStairs,
  &Dungeon_StraightStairs,
  &Dungeon_StraightStairs,
  &Dungeon_Submodule_14_FallGap,
  &Dungeon_Teleport,
  &Dungeon_Submodule_16,
  &Dungeon_Submodule_17,
  &Dungeon_Crystal,
  &Dungeon_Submodule_19,
  &Dungeon_OpenGanonDoor,
};

void Module_Dungeon() {
  Dungeon_Effect_Handler();
  kDungeonSubmodules[submodule_index]();
  dung_misc_objs_index = 0;
  Dungeon_PushBlock_Handler();
  if (submodule_index) goto skip;
  Graphics_MaybeLoadChrHalfSlot();
  Dungeon_HandleScreenScrolling();
  if (submodule_index) goto skip;
  Dungeon_CheckStairsAndRunScripts();
  if (submodule_index) goto skip;
  Dungeon_ProcessTorchAndDoorInteractives();
  if (dung_unk_blast_walls_2)
    Door_BlastWallExploding();
  if (!is_standing_in_doorway)
    Dung_CheckTriggerInterRoomTrans();
skip:
  OrientLampBg();

  int bg2x = BG2HOFS_copy2;
  int bg2y = BG2VOFS_copy2;
  int bg1x = BG1HOFS_copy2;
  int bg1y = BG1VOFS_copy2;

  BG2HOFS_copy2 = BG2HOFS_copy = bg2x + bg1_x_offset;
  BG2VOFS_copy2 = BG2VOFS_copy = bg2y + bg1_y_offset;
  BG1HOFS_copy2 = BG1HOFS_copy = bg1x + bg1_x_offset;
  BG1VOFS_copy2 = BG1VOFS_copy = bg1y + bg1_y_offset;

  if (dung_hdr_collision_2_mirror) {
    BG1HOFS_copy2 = BG1HOFS_copy = bg1x = BG2HOFS_copy2 + dung_floor_x_offs;
    BG1VOFS_copy2 = BG1VOFS_copy = bg1y = BG2VOFS_copy2 + dung_floor_y_offs;
  }

  Sprite_HandlePushedBlocks();
  Sprite_Main();

  BG2HOFS_copy2 = bg2x;
  BG2VOFS_copy2 = bg2y;
  BG1HOFS_copy2 = bg1x;
  BG1VOFS_copy2 = bg1y;

  PlayerOam_Main();
  Hud_RefillLogic();
  Hud_FloorIndicator();
}

const uint8 kDungAnimatedTiles[24] = {
  0x5d, 0x5d, 0x5d, 0x5d, 0x5d, 0x5d, 0x5d, 0x5f, 0x5d, 0x5f, 0x5f, 0x5e, 0x5f, 0x5e, 0x5e, 0x5d,
  0x5d, 0x5e, 0x5d, 0x5d, 0x5d, 0x5d, 0x5d, 0x5d,
};

void Link_CheckBunnyStatus() {
  if (link_player_handler_state == kPlayerState_RecoilWall) {
    link_player_handler_state =
      !link_is_bunny_mirror ? kPlayerState_Ground :
      link_item_moon_pearl ? kPlayerState_TempBunny : kPlayerState_PermaBunny;
  }
}

void Module_PreDungeon() {
  sound_effect_ambient = 5;
  sound_effect_1 = 0;
  dungeon_room_index = 0;
  dungeon_room_index_prev = 0;
  dung_savegame_state_bits = 0;

  agahnim_pal_setting[0] = agahnim_pal_setting[1] = agahnim_pal_setting[2] = 0;
  agahnim_pal_setting[3] = agahnim_pal_setting[4] = agahnim_pal_setting[5] = 0;

  Dungeon_LoadEntrance();
  uint8 d = cur_palace_index_x2;
  link_num_keys = (d != 0xff) ? link_keys_earned_per_dungeon[d == 2 ? 0 : (d >> 1)] : 0xff;
  Hud_Rebuild();
  dung_num_lit_torches = 0;
  hdr_dungeon_dark_with_lantern = 0;
  Dungeon_LoadAndUploadRoom();
  Dungeon_LoadCustomTileAttr();

  DecompDungAnimatedTiles(kDungAnimatedTiles[main_tile_theme_index]);
  Dungeon_LoadAttrTable();
  misc_sprites_graphics_index = 10;
  InitTilesets();
  palette_sp6 = 10;
  Dungeon_LoadPalettes();
  if (link_is_bunny_mirror | link_is_bunny)
    LoadGearPalettes_bunny();

  dung_loade_bgoffs_h_copy = (dungeon_room_index & 0xf) << 9;
  dung_loade_bgoffs_v_copy = swap16((dungeon_room_index & 0xff0) >> 3);

  if (dungeon_room_index == 0x104 && sram_progress_flags & 0x10)
    WORD(dung_want_lights_out) = 0;

  Player_UpdateQuadrantsVisited();
  CGWSEL_copy = 2;
  CGADSUB_copy = 0xb3;

  uint8 x = dung_num_lit_torches;
  if (!dung_want_lights_out) {
    x = 3;
    CGADSUB_copy = dung_hdr_bg2_properties == 7 ? 0x32 :
      dung_hdr_bg2_properties == 4 ? 0x62 : 0x20;
  }
  overworld_fixed_color_plusminus = kLitTorchesColorPlus[x];
  Dungeon_ApproachFixedColor_variable(overworld_fixed_color_plusminus);
  BYTE(palette_filter_countdown) = 0x1f;
  mosaic_target_level = 0;
  BYTE(darkening_or_lightening_screen) = 2;
  overworld_palette_aux_or_main = 0;
  link_speed_modifier = 0;
  button_mask_b_y = 0;
  button_b_frames = 0;
  Dungeon_ResetTorchBackgroundAndPlayer();
  Link_CheckBunnyStatus();
  Dungeon_InitAndCacheVars();
  if (savegame_tagalong == 13) {
    savegame_tagalong = 0;
    super_bomb_indicator_unk2 = 0;
    Hud_RemoveSuperBombIndicator();
  }
  BGMODE_copy = 9;
  Tagalong_Init();
  Sprite_ResetAll();
  Dungeon_ResetSprites();
  byte_7E02F0 = 0;
  flag_skip_call_tag_routines++;
  if (!sram_progress_indicator && !(sram_progress_flags & 0x10)) {
    COLDATA_copy0 = 0x30;
    COLDATA_copy1 = 0x50;
    COLDATA_copy2 = 0x80;
    dung_want_lights_out = dung_want_lights_out_copy = 0;
    Link_SetInBed();
  }
  saved_module_for_menu = 7;
  main_module_index = 7;
  submodule_index = 15;
  Dungeon_LoadSongBankIfNeeded();
  Module_PreDungeon_setAmbientSfx();
}

void Module_PreDungeon_setAmbientSfx() {
  if (sram_progress_indicator < 2) {
    sound_effect_ambient = 5;
    if (!sign8(dung_cur_floor) && dungeon_room_index != 2 && dungeon_room_index != 18)
      sound_effect_ambient = 3;
  }
}

void Dungeon_LoadSongBankIfNeeded() {
  if (buffer_for_playing_songs == 0xff || buffer_for_playing_songs == 0xf2)
    return;

  if (buffer_for_playing_songs == 3 || buffer_for_playing_songs == 7 || buffer_for_playing_songs == 14) {
    Overworld_LoadMusicIfNeeded();
  } else {
    if (flag_which_music_type)
      return;
    zelda_snes_dummy_write(NMITIMEN, 0);
    zelda_snes_dummy_write(HDMAEN, 0);
    flag_which_music_type = 1;
    Sound_LoadIndoorSongBank();
    zelda_snes_dummy_write(NMITIMEN, 0x81);
  }
}

void Overworld_LoadMusicIfNeeded() {
  if (!flag_which_music_type)
    return;
  zelda_snes_dummy_write(NMITIMEN, 0);
  zelda_snes_dummy_write(HDMAEN, 0);
  flag_which_music_type = 0;
  Sound_LoadLightWorldSongBank();
  zelda_snes_dummy_write(NMITIMEN, 0x81);
}

void Dungeon_LoadEntrance() {
  player_is_indoors = 1;

  if (death_var5) {
    death_var5 = 0;
  } else {
    overworld_area_index_exit = overworld_area_index;
    TM_copy_exit = WORD(TM_copy);
    BG2VOFS_copy2_exit = BG2VOFS_copy2;
    BG2HOFS_copy2_exit = BG2HOFS_copy2;
    link_y_coord_exit = link_y_coord;
    link_x_coord_exit = link_x_coord;
    camera_y_coord_scroll_low_exit = camera_y_coord_scroll_low;
    camera_x_coord_scroll_low_exit = camera_x_coord_scroll_low;
    overworld_screen_index_exit = overworld_screen_index;
    map16_load_src_off_exit = map16_load_src_off;
    overworld_screen_index = 0;
    overlay_index = 0;
    ow_scroll_vars0_exit = ow_scroll_vars0;
    up_down_scroll_target_exit = up_down_scroll_target;
    up_down_scroll_target_end_exit = up_down_scroll_target_end;
    left_right_scroll_target_exit = left_right_scroll_target;
    left_right_scroll_target_end_exit = left_right_scroll_target_end;
    overworld_unk1_exit = overworld_unk1;
    overworld_unk1_neg_exit = overworld_unk1_neg;
    overworld_unk3_exit = overworld_unk3;
    overworld_unk3_neg_exit = overworld_unk3_neg;
    byte_7EC164 = byte_7E0AA0;
    main_tile_theme_index_exit = main_tile_theme_index;
    aux_tile_theme_index_exit = aux_tile_theme_index;
    sprite_graphics_index_exit = sprite_graphics_index;
  }
  bg1_y_offset = bg1_x_offset = 0;
  WORD(death_var5) = 0;
  if (WORD(savegame_tagalong) == 4 || WORD(death_var4)) {
    int i = which_starting_point;
    WORD(which_entrance) = kStartingPoint_entrance[i];
    dungeon_room_index = dungeon_room_index2 = kStartingPoint_rooms[i];
    BG1VOFS_copy = BG2VOFS_copy = BG1VOFS_copy2 = BG2VOFS_copy2 = kStartingPoint_scrollY[i];
    BG1HOFS_copy = BG2HOFS_copy = BG1HOFS_copy2 = BG2HOFS_copy2 = kStartingPoint_scrollX[i];
    if (WORD(sram_progress_indicator)) {
      link_y_coord = kStartingPoint_playerY[i];
      link_x_coord = kStartingPoint_playerX[i];
    }
    camera_y_coord_scroll_low = kStartingPoint_cameraY[i];
    camera_y_coord_scroll_hi = camera_y_coord_scroll_low + 2;
    camera_x_coord_scroll_low = kStartingPoint_cameraX[i];
    camera_x_coord_scroll_hi = camera_x_coord_scroll_low + 2;
    tilemap_location_calc_mask = 0x1f8;
    ow_entrance_value = kStartingPoint_doorSettings[i];
    up_down_scroll_target = 0;
    up_down_scroll_target_end = 0x110;
    left_right_scroll_target = 0;
    left_right_scroll_target_end = 0x100;
    room_bounds_y.a0 = kStartingPoint_relativeCoords[i * 8 + 0] << 8;
    room_bounds_y.b0 = kStartingPoint_relativeCoords[i * 8 + 1] << 8;
    room_bounds_y.a1 = kStartingPoint_relativeCoords[i * 8 + 2] << 8 | 0x10;
    room_bounds_y.b1 = kStartingPoint_relativeCoords[i * 8 + 3] << 8 | 0x10;
    room_bounds_x.a0 = kStartingPoint_relativeCoords[i * 8 + 4] << 8;
    room_bounds_x.b0 = kStartingPoint_relativeCoords[i * 8 + 5] << 8;
    room_bounds_x.a1 = kStartingPoint_relativeCoords[i * 8 + 6] << 8;
    room_bounds_x.b1 = kStartingPoint_relativeCoords[i * 8 + 7] << 8;

    link_direction_facing = 2;
    main_tile_theme_index = kStartingPoint_blockset[i];
    dung_cur_floor = kStartingPoint_floor[i];
    BYTE(cur_palace_index_x2) = kStartingPoint_palace[i];
    is_standing_in_doorway = 0;
    link_is_on_lower_level = kStartingPoint_startingBg[i] >> 4;
    link_is_on_lower_level_mirror = kStartingPoint_startingBg[i] & 0xf;
    quadrant_fullsize_x = kStartingPoint_quadrant1[i] >> 4;
    quadrant_fullsize_y = kStartingPoint_quadrant1[i] & 0xf;
    link_quadrant_x = kStartingPoint_quadrant2[i] >> 4;
    link_quadrant_y = kStartingPoint_quadrant2[i] & 0xf;

    buffer_for_playing_songs = kStartingPoint_musicTrack[i];
    if (i == 0 && sram_progress_indicator == 0)
      buffer_for_playing_songs = 0xff;
    death_var4 = 0;
  } else {
    int i = which_entrance;
    dungeon_room_index = dungeon_room_index2 = kEntranceData_rooms[i];
    BG1VOFS_copy = BG2VOFS_copy = BG1VOFS_copy2 = BG2VOFS_copy2 = kEntranceData_scrollY[i];
    BG1HOFS_copy = BG2HOFS_copy = BG1HOFS_copy2 = BG2HOFS_copy2 = kEntranceData_scrollX[i];
    if (WORD(sram_progress_indicator)) {
      link_y_coord = kEntranceData_playerY[i];
      link_x_coord = kEntranceData_playerX[i];
    }
    camera_y_coord_scroll_low = kEntranceData_cameraY[i];
    camera_y_coord_scroll_hi = camera_y_coord_scroll_low + 2;
    camera_x_coord_scroll_low = kEntranceData_cameraX[i];
    camera_x_coord_scroll_hi = camera_x_coord_scroll_low + 2;
    tilemap_location_calc_mask = 0x1f8;
    ow_entrance_value = kEntranceData_doorSettings[i];
    big_rock_starting_address = 0;
    up_down_scroll_target = 0;
    up_down_scroll_target_end = 0x110;
    left_right_scroll_target = 0;
    left_right_scroll_target_end = 0x100;

    room_bounds_y.a0 = kEntranceData_relativeCoords[i * 8 + 0] << 8;
    room_bounds_y.b0 = kEntranceData_relativeCoords[i * 8 + 1] << 8;
    room_bounds_y.a1 = kEntranceData_relativeCoords[i * 8 + 2] << 8 | 0x10;
    room_bounds_y.b1 = kEntranceData_relativeCoords[i * 8 + 3] << 8 | 0x10;

    room_bounds_x.a0 = kEntranceData_relativeCoords[i * 8 + 4] << 8;
    room_bounds_x.b0 = kEntranceData_relativeCoords[i * 8 + 5] << 8;
    room_bounds_x.a1 = kEntranceData_relativeCoords[i * 8 + 6] << 8;
    room_bounds_x.b1 = kEntranceData_relativeCoords[i * 8 + 7] << 8;

    link_direction_facing = (i == 0 || i == 0x43) ? 2 : 0;
    main_tile_theme_index = kEntranceData_blockset[i];
    buffer_for_playing_songs = kEntranceData_musicTrack[i];
    if (buffer_for_playing_songs == 3 && sram_progress_indicator >= 2)
      buffer_for_playing_songs = 18;

    dung_cur_floor = kEntranceData_floor[i];
    BYTE(cur_palace_index_x2) = kEntranceData_palace[i];
    is_standing_in_doorway = kEntranceData_doorwayOrientation[i];
    link_is_on_lower_level = kEntranceData_startingBg[i] >> 4;
    link_is_on_lower_level_mirror = kEntranceData_startingBg[i] & 0xf;
    quadrant_fullsize_x = kEntranceData_quadrant1[i] >> 4;
    quadrant_fullsize_y = kEntranceData_quadrant1[i] & 0xf;
    link_quadrant_x = kEntranceData_quadrant2[i] >> 4;
    link_quadrant_y = kEntranceData_quadrant2[i] & 0xf;

    if (dungeon_room_index >= 0x100)
      dung_cur_floor = 0;
  }
  player_oam_x_offset = player_oam_y_offset = 0x80;
  link_direction_mask_a = link_direction_mask_b = 0xf;
  BYTE(link_z_coord) = link_actual_vel_z = 0xff;
  memcpy(movable_block_datas, kMovableBlockDataInit, sizeof(kMovableBlockDataInit));
  memcpy(&movable_block_datas[99], kTorchDataInit, 116); // junk
  memcpy(dung_torch_data, kTorchDataInit, sizeof(kTorchDataInit));
  memcpy(&dung_torch_data[144], kTorchDataJunk, sizeof(kTorchDataJunk));

  memset(memorized_tile_addr, 0, 0x100);
  memset(pots_revealed_in_room, 0, 0x280);
  orange_blue_barrier_state = 0;
  byte_7E04BC = 0;
}

void HoleToDungeon_LoadDungeon() {
  EnableForceBlank();
  CGWSEL_copy = 2;
  Dungeon_LoadEntrance();

  uint8 dung = BYTE(cur_palace_index_x2);
  link_num_keys = (dung != 255) ? link_keys_earned_per_dungeon[((dung == 2) ? 0 : dung) >> 1] : 255;
  Hud_Rebuild();
  link_this_controls_sprite_oam = 4;
  player_near_pit_state = 3;
  link_visibility_status = 12;
  link_speed_modifier = 16;

  uint8 y = link_y_coord - BG2VOFS_copy2;
  link_state_bits = 0;
  link_picking_throw_state = 0;
  some_animation_timer = 0;
  dungeon_room_index_prev = dungeon_room_index;
  tiledetect_which_y_pos[0] = link_y_coord;
  link_y_coord -= y + 16;

  uint8 bak = subsubmodule_index;
  dung_num_lit_torches = 0;
  hdr_dungeon_dark_with_lantern = 0;
  Dungeon_LoadAndUploadRoom();
  Dungeon_LoadCustomTileAttr();
  DecompDungAnimatedTiles(kDungAnimatedTiles[main_tile_theme_index]);
  Dungeon_LoadAttrTable();
  subsubmodule_index = bak + 1;
  misc_sprites_graphics_index = 10;
  zelda_ppu_write(OBSEL, 2);
  InitTilesets();
  palette_sp6 = 10;
  Dungeon_LoadPalettes();
  Hud_RestoreTorchBackground();
  button_mask_b_y = 0;
  button_b_frames = 0;
  Dungeon_ResetTorchBackgroundAndPlayer();
  if (link_is_bunny_mirror)
    LoadGearPalettes_bunny();
  HDMAEN_copy = 0x80;
  Hud_RefillLogic();
  Module_PreDungeon_setAmbientSfx();
  submodule_index = 7;
  Dungeon_LoadSongBankIfNeeded();
}


void Module_HoleToDungeon() {
  switch (subsubmodule_index) {
  case 0:  // HoleToDungeon_FadeMusic
    if (kEntranceData_musicTrack[which_entrance] != 3 || sram_progress_indicator >= 2)
      music_control = 0xf1;
    Dungeon_Teleport0();
    break;
  case 1:
    if (!(frame_counter & 1))
      PaletteFilter_doFiltering();
    break;
  case 2:
    HoleToDungeon_LoadDungeon();
    break;
  case 3:
    Dungeon_Staircase6();
    break;
  case 4:
    INIDISP_copy = (INIDISP_copy + 1) & 0xf;
    if (INIDISP_copy == 15)
      subsubmodule_index++;
  case 5:
    HoleToDungeon_Helper1();
    if (submodule_index)
      return;
    main_module_index = 7;
    flag_skip_call_tag_routines++;
    Dungeon_SpiralStaircase7_Inner4();
    Dungeon_InitAndCacheVars();
    music_control = buffer_for_playing_songs;
    last_music_control = music_unk1;
    break;
  }
}

void UpdateCompositeOfLayoutAndQuadrant() {
  composite_of_layout_and_quadrant = dung_layout_and_starting_quadrant | link_quadrant_y | link_quadrant_x;
}

const DungPalInfo *GetDungPalInfo(int idx) {
  return &kDungPalinfos[idx];
}

uint16 Dungeon_GetTeleMsg(int room) {
  return kDungeonRoomTeleMsg[room];
}

uint8 GetEntranceMusicTrack(int entrance) {
  return kEntranceData_musicTrack[entrance];
}

bool Dungeon_IsPitThatHurtsPlayer() {
  for (int i = countof(kDungeonPitsHurtPlayer) - 1; i >= 0; i--) {
    if (kDungeonPitsHurtPlayer[i] == dungeon_room_index)
      return true;
  }
  return false;

}
