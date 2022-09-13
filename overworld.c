#include "overworld.h"
#include "hud.h"
#include "load_gfx.h"
#include "dungeon.h"
#include "tagalong.h"
#include "sprite.h"
#include "ancilla.h"
#include "player.h"
#include "misc.h"
#include "messaging.h"
#include "player_oam.h"
#include "snes/snes_regs.h"
#include "tables/generated_map32_to_map16.h"
#include "tables/generated_map16_to_map8.h"
#include "tables/generated_overworld_tables.h"
#include "tables/generated_overworld.h"
#include "tables/generated_enemy_damage_data.h"

const uint16 kOverworld_OffsetBaseX[64] = {
  0,     0, 0x400, 0x600, 0x600, 0xa00, 0xa00, 0xe00,
  0,     0, 0x400, 0x600, 0x600, 0xa00, 0xa00, 0xe00,
  0, 0x200, 0x400, 0x600, 0x800, 0xa00, 0xc00, 0xe00,
  0,     0, 0x400, 0x600, 0x600, 0xa00, 0xc00, 0xc00,
  0,     0, 0x400, 0x600, 0x600, 0xa00, 0xc00, 0xc00,
  0, 0x200, 0x400, 0x600, 0x800, 0xa00, 0xc00, 0xe00,
  0,     0, 0x400, 0x600, 0x800, 0xa00, 0xa00, 0xe00,
  0,     0, 0x400, 0x600, 0x800, 0xa00, 0xa00, 0xe00,
};
const uint16 kOverworld_OffsetBaseY[64] = {
      0,     0,     0,     0,     0,     0,     0,     0,
      0,     0, 0x200,     0,     0,     0,     0, 0x200,
  0x400, 0x400, 0x400, 0x400, 0x400, 0x400, 0x400, 0x400,
  0x600, 0x600, 0x600, 0x600, 0x600, 0x600, 0x600, 0x600,
  0x600, 0x600, 0x800, 0x600, 0x600, 0x800, 0x600, 0x600,
  0xa00, 0xa00, 0xa00, 0xa00, 0xa00, 0xa00, 0xa00, 0xa00,
  0xc00, 0xc00, 0xc00, 0xc00, 0xc00, 0xc00, 0xc00, 0xc00,
  0xc00, 0xc00, 0xe00, 0xe00, 0xe00, 0xc00, 0xc00, 0xe00,
};
static const uint16 kOverworld_UpDownScrollTarget[64] = {
  0xff20, 0xff20, 0xff20, 0xff20, 0xff20, 0xff20, 0xff20, 0xff20,
  0xff20, 0xff20,  0x120, 0xff20, 0xff20, 0xff20, 0xff20,  0x120,
   0x320,  0x320,  0x320,  0x320,  0x320,  0x320,  0x320,  0x320,
   0x520,  0x520,  0x520,  0x520,  0x520,  0x520,  0x520,  0x520,
   0x520,  0x520,  0x720,  0x520,  0x520,  0x720,  0x520,  0x520,
   0x920,  0x920,  0x920,  0x920,  0x920,  0x920,  0x920,  0x920,
   0xb20,  0xb20,  0xb20,  0xb20,  0xb20,  0xb20,  0xb20,  0xb20,
   0xb20,  0xb20,  0xd20,  0xd20,  0xd20,  0xb20,  0xb20,  0xd20,
};
static const uint16 kOverworld_LeftRightScrollTarget[64] = {
  0xff00, 0xff00, 0x300, 0x500, 0x500, 0x900, 0x900, 0xd00,
  0xff00, 0xff00, 0x300, 0x500, 0x500, 0x900, 0x900, 0xd00,
  0xff00,  0x100, 0x300, 0x500, 0x700, 0x900, 0xb00, 0xd00,
  0xff00, 0xff00, 0x300, 0x500, 0x500, 0x900, 0xb00, 0xb00,
  0xff00, 0xff00, 0x300, 0x500, 0x500, 0x900, 0xb00, 0xb00,
  0xff00,  0x100, 0x300, 0x500, 0x700, 0x900, 0xb00, 0xd00,
  0xff00, 0xff00, 0x300, 0x500, 0x700, 0x900, 0x900, 0xd00,
  0xff00, 0xff00, 0x300, 0x500, 0x700, 0x900, 0x900, 0xd00,
};
#if 0
static const uint16 kSpExit_Top[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0x200, 0x200, 0, 0, 0, 0, 0, 0 };
static const uint16 kSpExit_Bottom[16] = { 0x120, 0x20, 0x320, 0x20, 0, 0, 0x320, 0x320, 0x320, 0x220, 0, 0, 0, 0, 0x320, 0x320 };
static const uint16 kSpExit_Left[16] = { 0, 0x100, 0x200, 0x600, 0x600, 0xa00, 0xc00, 0xc00, 0, 0x100, 0x200, 0x600, 0x600, 0xa00, 0xc00, 0xc00 };
static const uint16 kSpExit_Right[16] = { 0, 0x100, 0x500, 0x600, 0x600, 0xa00, 0xc00, 0xc00, 0, 0x100, 0x400, 0x600, 0x600, 0xa00, 0xc00, 0xc00 };
static const uint16 kSpExit_Tab4[16] = { 0xff20, 0xff20, 0xff20, 0xff20, 0xff20, 0xff20, 0xff20, 0xff20, 0xff20, 0xff20, 0x120, 0xff20, 0xff20, 0xff20, 0xff20, 0x120 };
static const int16 kSpExit_Tab6[16] = { -4, 0x100, 0x300, 0x100, 0x500, 0x900, 0xb00, 0xb00, -4, 0x100, 0x300, 0x500, 0x500, 0x900, 0xb00, 0xb00 };
static const int16 kSpExit_Tab5[16] = { -0xe0, -0xe0, -0xe0, -0xe0, -0xe0, -0xe0, 0x400, 0x400, -0xe0, -0xe0, 0x120, -0xe0, -0xe0, -0xe0, 0x400, 0x400 };
static const uint16 kSpExit_Tab7[16] = { 4, 0x104, 0x300, 0x100, 0x500, 0x900, 0xb00, 0xb00, 4, 0x104, 0x300, 0x100, 0x500, 0x900, 0xb00, 0xb00 };
static const uint16 kSpExit_LeftEdgeOfMap[16] = { 0, 0, 0x200, 0x600, 0x600, 0xa00, 0xc00, 0xc00, 0, 0, 0x200, 0x600, 0x600, 0xa00, 0xc00, 0xc00 };
static const uint8 kSpExit_Dir[16] = { 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static const uint8 kSpExit_SprGfx[16] = { 0xc, 0xc, 0xe, 0xe, 0xe, 0x10, 0x10, 0x10, 0xe, 0xe, 0xe, 0xe, 0x10, 0x10, 0x10, 0x10 };
static const uint8 kSpExit_AuxGfx[16] = { 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f };
static const uint8 kSpExit_PalBg[16] = { 0xa, 0xa, 0xa, 0xa, 2, 2, 2, 0xa, 2, 2, 0xa, 2, 2, 2, 2, 0xa };
static const uint8 kSpExit_PalSpr[16] = { 1, 8, 8, 8, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 2 };
#endif
#define turtlerock_ctr (g_ram[0xc8])
#define ganonentrance_ctr (g_ram[0xc8])
static PlayerHandlerFunc *const kOverworld_EntranceSequence[5] = {
  &Overworld_AnimateEntrance_PoD,
  &Overworld_AnimateEntrance_Skull,
  &Overworld_AnimateEntrance_Mire,
  &Overworld_AnimateEntrance_TurtleRock,
  &Overworld_AnimateEntrance_GanonsTower,
};
#ifndef map16_decode_0
#define map16_decode_0 ((uint8*)(g_ram+0x14400))
#define map16_decode_1 ((uint8*)(g_ram+0x14410))
#define map16_decode_2 ((uint8*)(g_ram+0x14420))
#define map16_decode_3 ((uint8*)(g_ram+0x14430))
#define map16_decode_last (*(uint16*)(g_ram+0x14440))
#define map16_decode_tmp (*(uint16*)(g_ram+0x14442))
#endif
static const uint16 kSecondaryOverlayPerOw[128] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0x1c0c, 0x1c0c, 0, 0, 0, 0, 0, 0, 0x1c0c, 0x1c0c, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0x3b0, 0x180c, 0x180c, 0x288, 0, 0, 0, 0, 0, 0x180c, 0x180c, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x1ab6, 0x1ab6, 0, 0xe2e, 0xe2e, 0, 0, 0,
  0x1ab6, 0x1ab6, 0, 0xe2e, 0xe2e, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x3b0, 0, 0, 0x288,
  0, 0, 0, 0, 0, 0, 0, 0,
};
#define XY(x, y) ((y)*64+(x))
// this alternate entry point is for scrolling OW area loads
// b/c drawing a door only applies to when you transition from a dungeon to the OW
// the exceptioon is OW areas 0x80 and above which are handled similar to entrances
static const uint16 kOverworld_DrawStrip_Tab[3] = { 0x3d0, 0x410, 0xf410 };
static const uint16 kOverworld_Func2_Tab[4] = { 8, 4, 2, 1 };
static const uint16 kOverworld_Entrance_Tab0[44] = {
  0xfe, 0xc5, 0xfe, 0x114, 0x115, 0x175, 0x156, 0xf5, 0xe2, 0x1ef, 0x119, 0xfe, 0x172, 0x177, 0x13f, 0x172, 0x112, 0x161, 0x172, 0x14c, 0x156, 0x1ef, 0xfe, 0xfe, 0xfe, 0x10b, 0x173, 0x143, 0x149, 0x175, 0x103, 0x100,
  0x1cc, 0x15e, 0x167, 0x128, 0x131, 0x112, 0x16d, 0x163, 0x173, 0xfe, 0x113, 0x177,
};
static const uint16 kOverworld_Entrance_Tab1[44] = {
  0x14a, 0xc4, 0x14f, 0x115, 0x114, 0x174, 0x155, 0xf5, 0xee, 0x1eb, 0x118, 0x146, 0x171, 0x155, 0x137, 0x174, 0x173, 0x121, 0x164, 0x155, 0x157, 0x128, 0x114, 0x123, 0x113, 0x109, 0x118, 0x161, 0x149, 0x117, 0x174, 0x101,
  0x1cc, 0x131, 0x51, 0x14e, 0x131, 0x112, 0x17a, 0x163, 0x172, 0x1bd, 0x152, 0x167,
};
static const uint16 kDwPaletteAnim[35] = {
  0x884, 0xcc7, 0x150a, 0x154d, 0x7ff6, 0x5944, 0x7ad1,
  0x884, 0xcc7, 0x150a, 0x154d, 0x5bff, 0x7ad1, 0x21af,
  0x1084, 0x48c0, 0x6186, 0x7e6d, 0x7fe0, 0x5944, 0x7e20,
  0x1084, 0x000e, 0x1059, 0x291f, 0x7fe0, 0x5944, 0x7e20,
  0x1084, 0x1508, 0x196c, 0x21af, 0x7ff6, 0x1d4c, 0x7ad1,
};
static const uint16 kDwPaletteAnim2[40] = {
  0x7fff, 0x884, 0x1cc8, 0x1dce, 0x3694, 0x4718, 0x1d4a, 0x18ac,
  0x7fff, 0x1908, 0x2d2f, 0x3614, 0x4eda, 0x471f, 0x1d4a, 0x390f,
  0x7fff, 0x34cd, 0x5971, 0x5635, 0x7f1b, 0x7fff, 0x1d4a, 0x3d54,
  0x7fff, 0x1908, 0x2d2f, 0x3614, 0x4eda, 0x471f, 0x1d4a, 0x390f,
  0x7fff, 0x884, 0x52a, 0x21ef, 0x3ab5, 0x4b39, 0x1d4c, 0x18ac,
};
static const uint16 kSpecialSwitchArea_Map8[4] = { 0x105, 0x1e4, 0xad, 0xb9 };
static const uint16 kSpecialSwitchArea_Screen[4] = { 0, 45, 15, 129 };
static const uint8 kSpecialSwitchArea_Direction[4] = { 8, 2, 8, 8 };
static const uint16 kSpecialSwitchArea_Exit[4] = { 0x180, 0x181, 0x182, 0x189 };
const uint8 kVariousPacks[16] = {
  0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x5b, 0x01, 0x5a,
  0x42, 0x43, 0x44, 0x45, 0x3f, 0x59, 0x0b, 0x5a
};
static const uint16 kSpecialSwitchAreaB_Map8[3] = { 0x17c, 0x1e4, 0xad };
static const uint16 kSpecialSwitchAreaB_Screen[3] = { 0x80, 0x80, 0x81 };
static const uint16 kSpecialSwitchAreaB_Direction[3] = { 4, 1, 4 };
static const int16 kSwitchAreaTab0[4] = { 0xf80, 0xf80, 0x3f, 0x3f };
static const int16 kSwitchAreaTab1[256] = {
  0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x1060, 0x1060, 0x1060, 0x1060, 0x60,
  0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60,
  0x60, 0x60, 0x60, 0x1060, 0x1060, 0x60, 0x1060, 0x1060, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60,
  0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x1060, 0x1060, 0x60,
  0x80, 0x80, 0x40, 0x80, 0x80, 0x80, 0x80, 0x40, 0x1080, 0x1080, 0x40, 0x1080, 0x1080, 0x1080, 0x1080, 0x40,
  0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x80, 0x80, 0x40, 0x80, 0x80, 0x40, 0x80, 0x80,
  0x1080, 0x1080, 0x40, 0x1080, 0x1080, 0x40, 0x1080, 0x1080, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
  0x80, 0x80, 0x40, 0x40, 0x40, 0x80, 0x80, 0x40, 0x1080, 0x1080, 0x40, 0x40, 0x40, 0x1080, 0x1080, 0x40,
  0x1800, 0x1840, 0x1800, 0x1800, 0x1840, 0x1800, 0x1840, 0x1800, 0x1800, 0x1840, 0x1800, 0x1800, 0x1840, 0x1800, 0x1840, 0x1800,
  0x1800, 0x1800, 0x1800, 0x1800, 0x1800, 0x1800, 0x1800, 0x1800, 0x1800, 0x1840, 0x1800, 0x1800, 0x1840, 0x1800, 0x1800, 0x1840,
  0x1800, 0x1840, 0x1800, 0x1800, 0x1840, 0x1800, 0x1800, 0x1840, 0x1800, 0x1800, 0x1800, 0x1800, 0x1800, 0x1800, 0x1800, 0x1800,
  0x1800, 0x1840, 0x1800, 0x1800, 0x1800, 0x1800, 0x1840, 0x1800, 0x1800, 0x1840, 0x1800, 0x1800, 0x1800, 0x1800, 0x1840, 0x1800,
  0x2000, 0x2040, 0x1000, 0x2000, 0x2040, 0x2000, 0x2040, 0x1000, 0x2000, 0x2040, 0x1000, 0x2000, 0x2040, 0x2000, 0x2040, 0x1000,
  0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x2000, 0x2040, 0x1000, 0x2000, 0x2040, 0x1000, 0x2000, 0x2040,
  0x2000, 0x2040, 0x1000, 0x2000, 0x2040, 0x1000, 0x2000, 0x2040, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000,
  0x2000, 0x2040, 0x1000, 0x1000, 0x1000, 0x2000, 0x2040, 0x1000, 0x2000, 0x2040, 0x1000, 0x1000, 0x1000, 0x2000, 0x2040, 0x1000,
};
static const int16 kSwitchAreaTab3[4] = { 2, -2, 16, -16 };
// kOverworldAreaHeads[i] != i for subregions of a big area
static const uint8 kOverworldAreaHeads[64] = {
  0,  0,  2,  3,  3,  5,  5,  7,
  0,  0, 10,  3,  3,  5,  5, 15,
  16, 17, 18, 19, 20, 21, 22, 23,
  24, 24, 26, 27, 27, 29, 30, 30,
  24, 24, 34, 27, 27, 37, 30, 30,
  40, 41, 42, 43, 44, 45, 46, 47,
  48, 48, 50, 51, 52, 53, 53, 55,
  48, 48, 58, 59, 60, 53, 53, 63,
};
static const uint16 kOverworld_Size1[2] = { 0x11e, 0x31e };
static const uint16 kOverworld_Size2[2] = { 0x100, 0x300 };
static const uint16 kOverworld_UpDownScrollSize[2] = { 0x2e0, 0x4e0 };
static const uint16 kOverworld_LeftRightScrollSize[2] = { 0x300, 0x500 };
static const int16 kOverworld_Func6B_Tab1[4] = { -8, 8, -8, 8 };
static const int16 kOverworld_Func6B_Tab2[4] = { 27, 27, 30, 30 };
static const int16 kOverworld_Func6B_Tab3[4] = { -0x70, 0x70, -0x70, 0x70 };
static const int16 kOverworld_Func6B_AreaDelta[4] = { -8, 8, -1, 1 };
static const uint8 kOverworld_Func8_tab[4] = { 0xe0, 8, 0xe0, 0x10 };
static const uint16 kDoorAnimTiles[56] = {
  0xda8, 0xda9, 0xdaa, 0xdab,
  0xdac, 0xdad, 0xdae, 0xdaf,
  0xdb0, 0xdb1, 0xdb2, 0xdb3,
  0xdb6, 0xdb7, 0xdb8, 0xdb9,
  0xdba, 0xdbb, 0xdbc, 0xdbd,
  0xdcd, 0xdce, 0xdcf, 0xdd0,
  0xdd3, 0xdd4, 0xdd5, 0xdd6,
  0xdd7, 0xdd8, 0xdd9, 0xdda,
  0xdd1, 0xdd2, 0xdd3, 0xdd4,
  0xdd1, 0xdd2, 0xdd7, 0xdd8,
  0x918, 0x919, 0x91a, 0x91b,
  0xddb, 0xddc, 0xddd, 0xdde,
  0xdd1, 0xdd2, 0xddb, 0xddc,
  0xe21, 0xe22, 0xe23, 0xe24,
};
static PlayerHandlerFunc *const kOverworldSubmodules[48] = {
  &Module09_00_PlayerControl,
  &Module09_LoadAuxGFX,
  &Overworld_FinishTransGfx,
  &Module09_LoadNewMapAndGFX,
  &Module09_LoadNewSprites,
  &Overworld_StartScrollTransition,
  &Overworld_RunScrollTransition,
  &Overworld_EaseOffScrollTransition,
  &Overworld_FinalizeEntryOntoScreen,
  &Module09_09_OpenBigDoorFromExiting,
  &Module09_0A_WalkFromExiting_FacingDown,
  &Module09_0B_WalkFromExiting_FacingUp,
  &Module09_0C_OpenBigDoor,
  &Overworld_StartMosaicTransition,
  &PreOverworld_LoadOverlays,
  &Module09_LoadAuxGFX,
  &Overworld_FinishTransGfx,
  &Module09_LoadNewMapAndGFX,
  &Module09_LoadNewSprites,
  &Overworld_StartScrollTransition,
  &Overworld_RunScrollTransition,
  &Overworld_EaseOffScrollTransition,
  &Module09_FadeBackInFromMosaic,
  &Overworld_StartMosaicTransition,
  &Overworld_Func18,
  &Overworld_Func19,
  &Module09_LoadAuxGFX,
  &Overworld_FinishTransGfx,
  &Overworld_Func1C,
  &Overworld_Func1D,
  &Overworld_Func1E,
  &Overworld_Func1F,
  &Overworld_LoadOverlays2,
  &Overworld_LoadAmbientOverlayFalse,
  &Overworld_Func22,
  &Module09_MirrorWarp,
  &Overworld_StartMosaicTransition,
  &Overworld_LoadOverlays,
  &Module09_LoadAuxGFX,
  &Overworld_FinishTransGfx,
  &Overworld_LoadAndBuildScreen,
  &Module09_FadeBackInFromMosaic,
  &Module09_2A_RecoverFromDrowning,
  &Overworld_Func2B,
  &Module09_MirrorWarp,
  &Overworld_WeathervaneExplosion,
  &Module09_2E_Whirlpool,
  &Overworld_Func2F,
};
static PlayerHandlerFunc *const kModule_PreOverworld[3] = {
  &PreOverworld_LoadProperties,
  &PreOverworld_LoadOverlays,
  &Module08_02_LoadAndAdvance,
};
const uint8 *GetMap8toTileAttr() {
  return kMap8DataToTileAttr;
}

const uint16 *GetMap16toMap8Table() {
  return kMap16ToMap8;
}

bool LookupInOwEntranceTab(uint16 r0, uint16 r2) {
  for (int i = countof(kOverworld_Entrance_Tab0) - 1; i >= 0; i--) {
    if (r0 == kOverworld_Entrance_Tab0[i] && r2 == kOverworld_Entrance_Tab1[i])
      return true;
  }
  return false;
}

int LookupInOwEntranceTab2(uint16 pos) {
  for (int i = 128; i >= 0; i--) {
    if (pos == kOverworld_Entrance_Pos[i] && overworld_area_index == kOverworld_Entrance_Area[i])
      return i;
  }
  return -1;
}

bool CanEnterWithTagalong(int e) {
  uint8 t = savegame_tagalong;
  return t == 0 || t == 5 || t == 14 || t == 1 || (t == 7 || t == 8) && e >= 59;
}

int DirToEnum(int dir) {
  int xx = 3;
  while (!(dir & 1))
    xx--, dir >>= 1;
  return xx;
}

void Overworld_ResetMosaicDown() {
  if (palette_filter_countdown & 1)
    mosaic_level -= 0x10;
  BGMODE_copy = 9;
  MOSAIC_copy = mosaic_level | 7;
}

void Overworld_Func1D() {
  assert(0);
}

void Overworld_Func1E() {
  assert(0);
}

uint16 Overworld_GetSignText(int area) {
  return kOverworld_SignText[area];
}

const uint8 *GetOverworldSpritePtr(int area) {
  int base = (sram_progress_indicator == 3) ? 2 :
    (sram_progress_indicator == 2) ? 1 : 0;
  return kOverworldSprites + kOverworldSpriteOffs[area + base * 144];
}

uint8 GetOverworldBgPalette(int idx) {
  return kOverworldBgPalettes[idx];
}

void Sprite_LoadGraphicsProperties() {  // 80fc41
  memcpy(overworld_sprite_gfx + 64, kOverworldSpriteGfx + 0xc0, 64);
  memcpy(overworld_sprite_palettes + 64, kOverworldSpritePalettes + 0xc0, 64);
  Sprite_LoadGraphicsProperties_light_world_only();
}

void Sprite_LoadGraphicsProperties_light_world_only() {  // 80fc62
  int i = sram_progress_indicator < 2 ? 0 :
    sram_progress_indicator != 3 ? 1 : 2;
  memcpy(overworld_sprite_gfx, kOverworldSpriteGfx + i * 64, 64);
  memcpy(overworld_sprite_palettes, kOverworldSpritePalettes + i * 64, 64);
}

void InitializeMirrorHDMA() {  // 80fdee
  HDMAEN_copy = 0;

  mirror_vars.var0 = 0;
  mirror_vars.var6 = 0;
  mirror_vars.var5 = 0;
  mirror_vars.var7 = 0;
  mirror_vars.var8 = 0;

  mirror_vars.var10 = mirror_vars.var11 = 8;
  mirror_vars.var9 = 21;
  mirror_vars.var1[0] = -0x200;
  mirror_vars.var1[1] = 0x200;
  mirror_vars.var3[0] = -0x40;
  mirror_vars.var3[1] = 0x40;

  HdmaSetup(0xF2FB, 0xF2FB, 0x42, (uint8)BG1HOFS, (uint8)BG2HOFS, 0);

  uint16 v = BG2HOFS_copy2;
  for (int i = 0; i < 32 * 7; i++)
    mode7_hdma_table[i] = v;
  HDMAEN_copy = 0xc0;
}

void MirrorWarp_BuildWavingHDMATable() {  // 80fe64
  MirrorWarp_RunAnimationSubmodules();
  if (frame_counter & 1)
    return;

  int x = 0x1a0 / 2, y = 0x1b0 / 2;
  do {
    mode7_hdma_table[y] = mode7_hdma_table[y + 2] = mode7_hdma_table[y + 4] = mode7_hdma_table[y + 6] = mode7_hdma_table[x];
    x -= 8, y -= 8;
  } while (y != 0);
  int i = mirror_vars.var0 >> 1;
  int t = mirror_vars.var6 + mirror_vars.var3[i];
  if (!sign16(t - mirror_vars.var1[i] ^ mirror_vars.var1[i])) {
    t = mirror_vars.var1[i];
    mirror_vars.var5 = 0;
    mirror_vars.var7 = 0;
    mirror_vars.var0 ^= 2;
  }
  mirror_vars.var6 = t;
  t += mirror_vars.var7;
  mirror_vars.var7 = t & 0xff;
  if (sign16(t))
    t |= 0xff;
  else
    t &= ~0xff;
  t = mirror_vars.var5 + swap16(t);
  mirror_vars.var5 = t;
  if (palette_filter_countdown >= 0x30 && !(t & ~7)) {
    mirror_vars.var1[0] = -0x100;
    mirror_vars.var1[1] = 0x100;
    subsubmodule_index++;
    t = 0;
  }
  mode7_hdma_table[0] = mode7_hdma_table[2] = mode7_hdma_table[4] = mode7_hdma_table[6] = t + BG2HOFS_copy2;
}

void MirrorWarp_BuildDewavingHDMATable() {  // 80ff2f
  MirrorWarp_RunAnimationSubmodules();
  if (frame_counter & 1)
    return;
  int x = 0x1a0 / 2, y = 0x1b0 / 2;
  do {
    mode7_hdma_table[y] = mode7_hdma_table[y + 2] = mode7_hdma_table[y + 4] = mode7_hdma_table[y + 6] = mode7_hdma_table[x];
    x -= 8, y -= 8;
  } while (y != 0);

  uint16 t = mode7_hdma_table[0xc0] | mode7_hdma_table[0xc8] | mode7_hdma_table[0xd0] | mode7_hdma_table[0xd8];
  if (t == BG2HOFS_copy2) {
    HDMAEN_copy = 0;
    subsubmodule_index++;
    Overworld_SetFixedColAndScroll();
    if ((overworld_screen_index & 0x3f) != 0x1b) {
      BG1HOFS_copy2 = BG1HOFS_copy = BG2HOFS_copy = BG2HOFS_copy2;
      BG1VOFS_copy2 = BG1VOFS_copy = BG2VOFS_copy = BG2VOFS_copy2;
    }
  }
}

void TakeDamageFromPit() {  // 81ffd9
  link_visibility_status = 12;
  submodule_index = player_is_indoors ? 20 : 42;
  link_health_current -= 8;
  if (link_health_current >= 0xa8)
    link_health_current = 0;
}

void Module08_OverworldLoad() {  // 8283bf
  kModule_PreOverworld[submodule_index]();
}

void PreOverworld_LoadProperties() {  // 8283c7
  CGWSEL_copy = 0x82;
  dung_unk6 = 0;
  AdjustLinkBunnyStatus();
  if (main_module_index == 8)
    LoadOverworldFromDungeon();
  else
    LoadOverworldFromSpecialOverworld();
  Overworld_SetSongList();
  link_num_keys = 0xff;
  Hud_RefillLogic();

  uint8 sc = overworld_screen_index, dr = dungeon_room_index;
  uint8 ow_anim_tiles = 0x58;
  uint8 xt = 2;

  if (sc == 3 || sc == 5 || sc == 7) {
    xt = 2;
  } else if (sc == 0x43 || sc == 0x45 || sc == 0x47) {
    xt = 9;
  } else if (ow_anim_tiles = 0x5a, sc >= 0x40) {
    goto dark;
  } else if (dr == 0xe3 || dr == 0x18 || dr == 0x2f || dr == 0x1f && sc == 0x18) {
    xt = sram_progress_indicator < 3 ? 7 : 2;
  } else {
    xt = savegame_has_master_sword_flags & 0x40 ? 2 : 5;
    if (dr != 0 && dr != 0xe1) {
dark:
      xt = 0xf3;
      if (buffer_for_playing_songs == 0xf2)
        goto setsong;
      xt = sram_progress_indicator < 2 ? 3 : 2;
    }
  }
  if (savegame_is_darkworld) {
    xt = sc == 0x40 || sc == 0x43 || sc == 0x45 || sc == 0x47 ? 13 : 9;
    if (!link_item_moon_pearl)
      xt = 4;
  }
setsong:
  buffer_for_playing_songs = xt;
  DecompressAnimatedOverworldTiles(ow_anim_tiles);
  InitializeTilesets();
  OverworldLoadScreensPaletteSet();
  Overworld_LoadPalettes(kOverworldBgPalettes[sc], overworld_sprite_palettes[sc]);
  Palette_SetOwBgColor();
  if (main_module_index == 8) {
    Overworld_LoadPalettesInner();
  } else {
    SpecialOverworld_CopyPalettesToCache();
  }
  Overworld_SetFixedColAndScroll();
  overworld_fixed_color_plusminus = 0;
  Follower_Initialize();

  if (!(BYTE(overworld_screen_index) & 0x3f))
    DecodeAnimatedSpriteTile_variable(0x1e);
  saved_module_for_menu = 9;
  Sprite_ReloadAll_Overworld();
  if (!(overworld_screen_index & 0x40))
    Sprite_InitializeMirrorPortal();
  sound_effect_ambient = sram_progress_indicator < 2 ? 1 : 5;
  if (savegame_tagalong == 6)
    savegame_tagalong = 0;

  is_standing_in_doorway = 0;
  button_mask_b_y = 0;
  button_b_frames = 0;
  link_cant_change_direction = 0;
  link_speed_setting = 0;
  draw_water_ripples_or_grass = 0;
  Dungeon_ResetTorchBackgroundAndPlayerInner();
  if (!link_item_moon_pearl && savegame_is_darkworld) {
    link_is_bunny = link_is_bunny_mirror = 1;
    link_player_handler_state = kPlayerState_PermaBunny;
    LoadGearPalettes_bunny();
  }
  BGMODE_copy = 9;
  dung_want_lights_out = 0;
  dung_hdr_collision = 0;
  link_is_on_lower_level = 0;
  link_is_on_lower_level_mirror = 0;
  submodule_index++;
  flag_update_hud_in_nmi++;
  dung_savegame_state_bits = 0;
  LoadOWMusicIfNeeded();
}

void AdjustLinkBunnyStatus() {  // 82856a
  if (link_item_moon_pearl)
    ForceNonbunnyStatus();
}

void ForceNonbunnyStatus() {  // 828570
  link_player_handler_state = kPlayerState_Ground;
  link_timer_tempbunny = 0;
  link_need_for_poof_for_transform = 0;
  link_is_bunny = 0;
  link_is_bunny_mirror = 0;
}

void RecoverPositionAfterDrowning() {  // 829583
  link_x_coord = link_x_coord_cached;
  link_y_coord = link_y_coord_cached;
  ow_scroll_vars0.ystart = room_scroll_vars_y_vofs1_cached;
  ow_scroll_vars0.xstart = room_scroll_vars_y_vofs2_cached;
  ow_scroll_vars1.ystart = room_scroll_vars_x_vofs1_cached;
  ow_scroll_vars1.xstart = room_scroll_vars_x_vofs2_cached;

  up_down_scroll_target = up_down_scroll_target_cached;
  up_down_scroll_target_end = up_down_scroll_target_end_cached;
  left_right_scroll_target = left_right_scroll_target_cached;
  left_right_scroll_target_end = left_right_scroll_target_end_cached;

  if (player_is_indoors) {
    camera_y_coord_scroll_low = camera_y_coord_scroll_low_cached;
    camera_y_coord_scroll_hi = camera_y_coord_scroll_low + 2;
    camera_x_coord_scroll_low = camera_x_coord_scroll_low_cached;
    camera_x_coord_scroll_hi = camera_x_coord_scroll_low + 2;
  }
  WORD(quadrant_fullsize_x) = WORD(quadrant_fullsize_x_cached);
  WORD(link_quadrant_x) = WORD(link_quadrant_x_cached);
  if (!player_is_indoors) {
    camera_y_coord_scroll_hi = camera_y_coord_scroll_low - 2;
    camera_x_coord_scroll_hi = camera_x_coord_scroll_low - 2;
  }

  link_direction_facing = link_direction_facing_cached;
  link_is_on_lower_level = link_is_on_lower_level_cached;
  link_is_on_lower_level_mirror = link_is_on_lower_level_mirror_cached;
  is_standing_in_doorway = is_standing_in_doorway_cahed;
  dung_cur_floor = dung_cur_floor_cached;
  link_visibility_status = 0;
  countdown_for_blink = 0x90;
  Dungeon_PlayBlipAndCacheQuadrantVisits();
  link_disable_sprite_damage = 0;
  Link_ResetStateAfterDamagingPit();
  tagalong_var5 = 0;
  Follower_Initialize();
  dung_flag_statechange_waterpuzzle = 0;
  overworld_map_state = 0;
  subsubmodule_index = 0;
  overworld_screen_transition = 0;
  submodule_index = 0;
  if (!link_health_current) {
    mapbak_TM = TM_copy;
    mapbak_TS = TS_copy;
    saved_module_for_menu = main_module_index;
    main_module_index = 18;
    submodule_index = 1;
    countdown_for_blink = 0;
  }
}

void Module0F_SpotlightClose() {  // 829982
  static const uint8 kTab[4] = { 8, 4, 2, 1 };
  Sprite_Main();
  if (submodule_index == 0)
    Dungeon_PrepExitWithSpotlight();
  else
    Spotlight_ConfigureTableAndControl();

  if (!player_is_indoors) {
    if (BYTE(overworld_screen_index) == 0xf)
      draw_water_ripples_or_grass = 1;
    link_speed_setting = 6;
    Link_HandleVelocity();
    link_x_vel = link_y_vel = 0;
  }

  int i = link_direction_facing >> 1;
  if (!player_is_indoors)
    i = (which_entrance == 0x43) ? 1 : 0;

  link_direction = link_direction_last = kTab[i];
  Link_HandleMovingAnimation_FullLongEntry();
  LinkOam_Main();
}

void Dungeon_PrepExitWithSpotlight() {  // 8299ca
  is_nmi_thread_active = 0;
  nmi_flag_update_polyhedral = 0;
  if (!player_is_indoors) {
    Ancilla_TerminateWaterfallSplashes();
    link_y_coord_exit = link_y_coord;
  }
  uint8 m = GetEntranceMusicTrack(which_entrance);
  if (m != 3 || (m = sram_progress_indicator) >= 2) {
    if (m != 0xf2)
      m = 0xf1;
    else if (music_unk1 == 0xc)
      m = 7;
    music_control = m;
  }
  hud_floor_changed_timer = 0;
  Hud_FloorIndicator();
  flag_update_hud_in_nmi++;
  IrisSpotlight_close();
  submodule_index++;
}

void Spotlight_ConfigureTableAndControl() {  // 829a19
  IrisSpotlight_ConfigureTable();
  is_nmi_thread_active = 0;
  nmi_flag_update_polyhedral = 0;
  if (submodule_index)
    return;
  if (main_module_index == 6)
    link_y_coord = link_y_coord_exit;
  OpenSpotlight_Next2();
}

void OpenSpotlight_Next2() {  // 829a37
  if (main_module_index != 9) {
    EnableForceBlank();
    Link_ItemReset_FromOverworldThings();
  }

  if (main_module_index == 9) {
    if (dungeon_room_index != 0x20)
      submodule_index = link_direction_facing ? 0xa : 0xb;

    ow_countdown_transition = 16;
    if ((BYTE(ow_entrance_value) | BYTE(big_rock_starting_address)) && HIBYTE(big_rock_starting_address)) { // wtf
      BYTE(door_open_closed_counter) = (big_rock_starting_address & 0x8000) ? 0x18 : 0;
      big_rock_starting_address &= 0x7fff;
      BYTE(door_animation_step_indicator) = 0;
      submodule_index = 9;
      subsubmodule_index = 0;
      sound_effect_2 = 21;
    }
  }
  W12SEL_copy = 0;
  W34SEL_copy = 0;
  WOBJSEL_copy = 0;
  TMW_copy = 0;
  TSW_copy = 0;
  link_force_hold_sword_up = 0;

  uint8 sc = overworld_screen_index;
  if (sc == 3 || sc == 5 || sc == 7) {
    COLDATA_copy0 = 0x26;
    COLDATA_copy1 = 0x4c;
    COLDATA_copy2 = 0x8c;
  } else if (sc == 0x43 || sc == 0x45 || sc == 0x47) {
    COLDATA_copy0 = 0x26;
    COLDATA_copy1 = 0x4a;
    COLDATA_copy2 = 0x87;
  }
}

void Module10_SpotlightOpen() {  // 829ad7
  Sprite_Main();
  if (submodule_index == 0)
    Module10_00_OpenIris();
  else
    Spotlight_ConfigureTableAndControl();
  LinkOam_Main();
}

void Module10_00_OpenIris() {  // 829ae6
  Spotlight_open();
  submodule_index++;
}

void SetTargetOverworldWarpToPyramid() {  // 829e5f
  if (main_module_index != 21)
    return;
  LoadOverworldFromDungeon();
  DecompressAnimatedOverworldTiles(0x5a);
  ResetAncillaAndCutscene();
}

void ResetAncillaAndCutscene() {  // 829e6e
  Ancilla_TerminateSelectInteractives(0);
  link_disable_sprite_damage = 0;
  button_b_frames = 0;
  button_mask_b_y = 0;
  link_force_hold_sword_up = 0;
  flag_is_link_immobilized = 0;
}

void Module09_Overworld() {  // 82a475
  kOverworldSubmodules[submodule_index]();

  int bg2x = BG2HOFS_copy2;
  int bg2y = BG2VOFS_copy2;
  int bg1x = BG1HOFS_copy2;
  int bg1y = BG1VOFS_copy2;

  BG2HOFS_copy2 = BG2HOFS_copy = bg2x + bg1_x_offset;
  BG2VOFS_copy2 = BG2VOFS_copy = bg2y + bg1_y_offset;
  BG1HOFS_copy2 = BG1HOFS_copy = bg1x + bg1_x_offset;
  BG1VOFS_copy2 = BG1VOFS_copy = bg1y + bg1_y_offset;

  Sprite_Main();

  BG2HOFS_copy2 = bg2x;
  BG2VOFS_copy2 = bg2y;
  BG1HOFS_copy2 = bg1x;
  BG1VOFS_copy2 = bg1y;

  LinkOam_Main();
  Hud_RefillLogic();
  OverworldOverlay_HandleRain();
}

void OverworldOverlay_HandleRain() {  // 82a4cd
  static const uint8 kOverworld_DrawBadWeather_X[4] = { 1, 0, 1, 0 };
  static const uint8 kOverworld_DrawBadWeather_Y[4] = { 0, 17, 0, 17 };
  if (BYTE(overworld_screen_index) != 0x70 && sram_progress_indicator >= 2 || (save_ow_event_info[0x70] & 0x20))
    return;
  if (frame_counter == 3 || frame_counter == 88) {
    CGADSUB_copy = 0x32;
  } else if (frame_counter == 5 || frame_counter == 44 || frame_counter == 90) {
    CGADSUB_copy = 0x72;
  } else if (frame_counter == 36) {
    sound_effect_1 = 54;
    CGADSUB_copy = 0x32;
  }
  if (frame_counter & 3)
    return;
  int i = (move_overlay_ctr + 1) & 3;
  move_overlay_ctr = i;
  BG1HOFS_copy2 += kOverworld_DrawBadWeather_X[i] << 8;
  BG1VOFS_copy2 += kOverworld_DrawBadWeather_Y[i] << 8;
}

void Module09_00_PlayerControl() {  // 82a53c
  if (!(flag_custom_spell_anim_active | flag_is_link_immobilized | flag_block_link_menu | trigger_special_entrance)) {
    if (filtered_joypad_H & 0x10) {
      overworld_map_state = 0;
      submodule_index = 1;
      saved_module_for_menu = main_module_index;
      main_module_index = 14;
      return;
    }
    if (filtered_joypad_L & 0x40) {
      overworld_map_state = 0;
      submodule_index = 7;
      saved_module_for_menu = main_module_index;
      main_module_index = 14;
      return;
    }
    if (joypad1H_last & 0x20) {
      choice_in_multiselect_box_bak = choice_in_multiselect_box;
      dialogue_message_index = 0x186;
      int bak = main_module_index;
      Main_ShowTextMessage();
      main_module_index = bak;
      subsubmodule_index = 0;
      submodule_index = 11;
      saved_module_for_menu = main_module_index;
      main_module_index = 14;
      return;
    }
  }
  if (trigger_special_entrance)
    Overworld_AnimateEntrance();
  Link_Main();
  if (super_bomb_indicator_unk2 != 0xff)
    Hud_SuperBombIndicator();
  current_area_of_player = (link_y_coord & 0x1e00) >> 5 | (link_x_coord & 0x1e00) >> 8;
  Graphics_LoadChrHalfSlot();
  Overworld_OperateCameraScroll();
  if (main_module_index != 11) {
    Overworld_UseEntrance();
    Overworld_DwDeathMountainPaletteAnimation();
    OverworldHandleTransitions();
  } else {
    ScrollAndCheckForSOWExit();
  }
}

void OverworldHandleTransitions() {  // 82a9c4
  if (BYTE(overworld_screen_trans_dir_bits2))
    OverworldHandleMapScroll();
  int dir;
  uint16 x, y, t;
  if (link_y_vel != 0) {
    dir = link_direction & 12;
    t = link_y_coord - kOverworld_OffsetBaseY[BYTE(current_area_of_player) >> 1];
    if ((y = 6, x = 8, t < 4) || (y = 4, x = 4, t >= overworld_right_bottom_bound_for_scroll))
      goto compare;
  }
  if (link_x_vel != 0) {
    dir = link_direction & 3;
    t = link_x_coord - kOverworld_OffsetBaseX[BYTE(current_area_of_player) >> 1];
    if ((y = 2, x = 2, t < 6) || (y = 0, x = 1, t >= (uint16)(overworld_right_bottom_bound_for_scroll + 4))) {
compare:
      if (x == dir && !Link_CheckForEdgeScreenTransition())
        goto after;
    }
  }
  Overworld_CheckSpecialSwitchArea();
  return;
after:
  y >>= 1;
  Dungeon_ResetTorchBackgroundAndPlayerInner();
  map16_load_src_off &= kSwitchAreaTab0[y];
  uint16 pushed = current_area_of_player + kSwitchAreaTab3[y];
  map16_load_src_off += kSwitchAreaTab1[(y * 128 | pushed) >> 1];

  uint8 old_screen = overworld_screen_index;
  if (old_screen == 0x2a)
    sound_effect_ambient = 0x80;

  uint8 new_area = kOverworldAreaHeads[pushed >> 1] | savegame_is_darkworld;
  BYTE(overworld_screen_index) = new_area;
  BYTE(overworld_area_index) = new_area;
  if (!savegame_is_darkworld || link_item_moon_pearl) {
    uint8 music = overworld_music[new_area];
    if ((music & 0xf0) == 0)
      sound_effect_ambient = 5;
    if ((music & 0xf) != music_unk1)
      music_control = 0xf1;
  }
  Overworld_LoadGFXAndScreenSize();
  submodule_index = 1;
  BYTE(overworld_screen_trans_dir_bits) = dir;
  BYTE(overworld_screen_trans_dir_bits2) = dir;
  byte_7E069C = overworld_screen_transition = DirToEnum(dir);
  BYTE(ow_entrance_value) = 0;
  BYTE(big_rock_starting_address) = 0;
  transition_counter = 0;

  if (!(old_screen & 0x3f) || !(overworld_screen_index & 0xbf)) {
    subsubmodule_index = 0;
    submodule_index = 13;
    MOSAIC_copy = 0;
    mosaic_level = 0;
  } else {
    uint8 sc = overworld_screen_index;
    Overworld_LoadPalettes(kOverworldBgPalettes[sc], overworld_sprite_palettes[sc]);
    Overworld_CopyPalettesToCache();
  }
}

void Overworld_LoadGFXAndScreenSize() {  // 82ab08
  int i = BYTE(overworld_screen_index);
  incremental_counter_for_vram = 0;
  sprite_graphics_index = overworld_sprite_gfx[i];
  aux_tile_theme_index = kOverworldAuxTileThemeIndexes[i];

  overworld_area_is_big_backup = overworld_area_is_big;
  BYTE(overworld_area_is_big) = kOverworldMapIsSmall[i & 0x3f] ? 0 : 0x20;
  ((uint8 *)&overworld_right_bottom_bound_for_scroll)[1] = kOverworldMapIsSmall[i & 0x3f] ? 1 : 3;
  main_tile_theme_index = overworld_screen_index & 0x40 ? 0x21 : 0x20;
  misc_sprites_graphics_index = kVariousPacks[6 + (overworld_screen_index & 0x40 ? 8 : 0)];

  int j = overworld_screen_index & 0xbf;
  overworld_offset_base_y = kOverworld_OffsetBaseY[j];
  overworld_offset_base_x = kOverworld_OffsetBaseX[j] >> 3;

  int m = overworld_area_is_big ? 0x3f0 : 0x1f0;
  overworld_offset_mask_y = m;
  overworld_offset_mask_x = m >> 3;
}

void ScrollAndCheckForSOWExit() {  // 82ab7b
  if (BYTE(overworld_screen_trans_dir_bits2))
    OverworldHandleMapScroll();

  const uint16 *map8 = Overworld_GetMap16OfLink_Mult8();
  int a = map8[0] & 0x1ff;
  for (int i = 2; i >= 0; i--) {
    if (kSpecialSwitchAreaB_Map8[i] == a && kSpecialSwitchAreaB_Screen[i] == overworld_screen_index) {
      link_direction = kSpecialSwitchAreaB_Direction[i];
      byte_7E069C = overworld_screen_transition = DirToEnum(link_direction);
      submodule_index = 36;
      subsubmodule_index = 0;
      BYTE(dungeon_room_index) = 0;
      break;
    }
  }
}

void Module09_LoadAuxGFX() {  // 82ab88
  save_ow_event_info[0x3b] &= ~0x20;
  save_ow_event_info[0x7b] &= ~0x20;
  save_dung_info[267] &= ~0x80;
  save_dung_info[40] &= ~0x100;
  LoadTransAuxGFX();
  PrepTransAuxGfx();
  nmi_disable_core_updates = nmi_subroutine_index = 9;
  submodule_index++;
}

void Overworld_FinishTransGfx() {  // 82abbc
  nmi_disable_core_updates = nmi_subroutine_index = 10;
  submodule_index++;
}

void Module09_LoadNewMapAndGFX() {  // 82abc6
  word_7E04C8 = 0;
  SomeTileMapChange();
  nmi_disable_core_updates++;
  CreateInitialNewScreenMapToScroll();
  LoadNewSpriteGFXSet();
}

void Overworld_RunScrollTransition() {  // 82abda
  Link_HandleMovingAnimation_FullLongEntry();
  Graphics_IncrementalVRAMUpload();
  uint8 rv = OverworldScrollTransition();
  if (!(rv & 0xf)) {
    BYTE(overworld_screen_trans_dir_bits2) = BYTE(overworld_screen_trans_dir_bits);
    OverworldTransitionScrollAndLoadMap();
    BYTE(overworld_screen_trans_dir_bits2) = 0;
  }
}

void Module09_LoadNewSprites() {  // 82abed
  if (overworld_screen_transition == 1) {
    BG2VOFS_copy2 += 2;
    link_y_coord += 2;
  }
  Sprite_OverworldReloadAll_justLoad();
  num_memorized_tiles = 0;
  if (sram_progress_indicator >= 2 && submodule_index != 18)
    Overworld_SetFixedColAndScroll();
  Overworld_StartScrollTransition();
}

void Overworld_StartScrollTransition() {  // 82ac27
  submodule_index++;
  if (BYTE(overworld_screen_trans_dir_bits) >= 4) {
    BYTE(overworld_screen_trans_dir_bits2) = BYTE(overworld_screen_trans_dir_bits);
    OverworldTransitionScrollAndLoadMap();
    BYTE(overworld_screen_trans_dir_bits2) = 0;
  }
}

void Overworld_EaseOffScrollTransition() {  // 82ac3a
  if (kOverworldMapIsSmall[BYTE(overworld_screen_index)]) {
    BYTE(overworld_screen_trans_dir_bits2) = BYTE(overworld_screen_trans_dir_bits);
    OverworldTransitionScrollAndLoadMap();
    BYTE(overworld_screen_trans_dir_bits2) = 0;
  }
  if (++subsubmodule_index < 8)
    return;
  if ((BYTE(overworld_screen_trans_dir_bits) == 8 || BYTE(overworld_screen_trans_dir_bits) == 2) && subsubmodule_index < 9)
    return;

  subsubmodule_index = 0;
  BYTE(overworld_screen_trans_dir_bits) = 0;

  if (kOverworldMapIsSmall[BYTE(overworld_screen_index)]) {
    map16_load_src_off = orange_blue_barrier_state;
    map16_load_dst_off = word_7EC174;
    map16_load_var2 = word_7EC176;
  }
  submodule_index++;
  Follower_Disable();
}

void Module09_0A_WalkFromExiting_FacingDown() {  // 82ac8f
  link_direction_last = 4;
  Link_HandleMovingAnimation_FullLongEntry();
  link_y_coord += 1;
  if (--ow_countdown_transition)
    return;
  submodule_index = 0;
  link_y_coord += 3;
  link_y_vel = 3;
  Overworld_OperateCameraScroll();
  if (BYTE(overworld_screen_trans_dir_bits2))
    OverworldHandleMapScroll();
}

void Module09_0B_WalkFromExiting_FacingUp() {  // 82acc2
  Link_HandleMovingAnimation_FullLongEntry();
  link_y_coord -= 1;
  if (--ow_countdown_transition)
    return;
  submodule_index = 0;
}

void Module09_09_OpenBigDoorFromExiting() {  // 82ad4a
  if (BYTE(door_animation_step_indicator) != 3) {
    Overworld_DoMapUpdate32x32_conditional();
    return;
  }
  ow_countdown_transition = 36;
  BYTE(overworld_screen_trans_dir_bits2) = 0;
  submodule_index++;
}

void Overworld_DoMapUpdate32x32_B() {  // 82ad5c
  Overworld_DoMapUpdate32x32();
  BYTE(door_open_closed_counter) = 0;
}

void Module09_0C_OpenBigDoor() {  // 82ad6c
  if (BYTE(door_animation_step_indicator) != 3) {
    Overworld_DoMapUpdate32x32_conditional();
    return;
  }
  submodule_index = 0;
  subsubmodule_index = 0;
  BYTE(overworld_screen_trans_dir_bits2) = 0;
}

void Overworld_DoMapUpdate32x32_conditional() {  // 82ad7b
  if (door_open_closed_counter & 7)
    BYTE(door_open_closed_counter)++;
  else
    Overworld_DoMapUpdate32x32();
}

void Overworld_DoMapUpdate32x32() {  // 82ad85
  int i = num_memorized_tiles >> 1;
  int j = door_open_closed_counter >> 1;
  uint16 pos, tile;

  memorized_tile_addr[i] = pos = big_rock_starting_address;
  memorized_tile_value[i] = tile = kDoorAnimTiles[j + 0];
  Overworld_DrawMap16_Persist(pos, tile);

  memorized_tile_addr[i + 1] = pos = big_rock_starting_address + 2;
  memorized_tile_value[i + 1] = tile = kDoorAnimTiles[j + 1];
  Overworld_DrawMap16_Persist(pos, tile);

  memorized_tile_addr[i + 2] = pos = big_rock_starting_address + 0x80;
  memorized_tile_value[i + 2] = tile = kDoorAnimTiles[j + 2];
  Overworld_DrawMap16_Persist(pos, tile);

  memorized_tile_addr[i + 3] = pos = big_rock_starting_address + 0x82;
  memorized_tile_value[i + 3] = tile = kDoorAnimTiles[j + 3];
  Overworld_DrawMap16_Persist(pos, tile);
  vram_upload_data[vram_upload_offset >> 1] = 0xffff;
  num_memorized_tiles += 8;
  door_animation_step_indicator += (door_open_closed_counter == 32) ? 2 : 1;
  nmi_load_bg_from_vram = 1;
  BYTE(door_open_closed_counter)++;
}

void Overworld_StartMosaicTransition() {  // 82ae5e
  ConditionalMosaicControl();
  switch (subsubmodule_index) {
  case 0:
    if (BYTE(overworld_screen_index) != 0x80) {
      if ((overworld_music[BYTE(overworld_screen_index)] & 0xf) != music_unk1)
        music_control = 0xf1;
    }
    ResetTransitionPropsAndAdvance_ResetInterface();
    break;
  case 1:
    ApplyPaletteFilter_bounce();
    break;
  default:
    INIDISP_copy = 0x80;
    subsubmodule_index = 0;
    if (!(overworld_screen_index & 0x3f))
      DecodeAnimatedSpriteTile_variable(0x1e);
    if (BYTE(overworld_area_index) != 0 && main_module_index != 11) {
      TM_copy = 0x16;
      TS_copy = 1;
      CGWSEL_copy = 0x82;
      CGADSUB_copy = 0x20;
      submodule_index++;
      return;
    }
    if (submodule_index == 36) {
      LoadOverworldFromSpecialOverworld();
      if (!(overworld_screen_index & 0x3f))
        DecodeAnimatedSpriteTile_variable(0x1e);
    }
    submodule_index++;
    break;
  }
}

void Overworld_LoadOverlays() {  // 82af0b
  Sprite_InitializeSlots();
  Sprite_ReloadAll_Overworld();
  link_state_bits = 0;
  link_picking_throw_state = 0;
  sound_effect_ambient = 5;
  Overworld_LoadOverlays2();

}

void PreOverworld_LoadOverlays() {  // 82af19
  sound_effect_ambient = 5;
  Overworld_LoadOverlays2();
}

void Overworld_LoadOverlays2() {  // 82af1e
  uint16 xv;

  overworld_screen_index_prev = overworld_screen_index;
  map16_load_src_off_prev = map16_load_src_off;
  map16_load_var2_prev = map16_load_var2;
  map16_load_dst_off_prev = map16_load_dst_off;
  overworld_screen_transition_prev = overworld_screen_transition;
  overworld_screen_trans_dir_bits_prev = overworld_screen_trans_dir_bits;
  overworld_screen_trans_dir_bits2_prev = overworld_screen_trans_dir_bits2;

  overlay_index = 0;
  BG1VOFS_subpixel = 0;
  BG1HOFS_subpixel = 0;

  int si = overworld_screen_index;
  if (si >= 0x80) {
    xv = 0x97;
    if (dungeon_room_index == 0x180) {
      if (save_ow_event_info[0x80] & 0x40)
        goto getout; // master sword retrieved?
      goto load_overlay;
    }
    if ((xv = 0x94, dungeon_room_index == 0x181) ||
        (xv = 0x93, dungeon_room_index == 0x189))
      goto load_overlay;

    if (dungeon_room_index == 0x182 || dungeon_room_index == 0x183)
      sound_effect_ambient = 1;  // zora falls
getout:
    TS_copy = 0;
    submodule_index++;
    return;
  }

  if ((si & 0x3f) == 0) {
    xv = (!(si & 0x40) && save_ow_event_info[0x80] & 0x40) ? 0x9e : 0x9d;  // forest
    goto load_overlay;
  }

  if ((xv = 0x95, si == 0x3 || si == 0x5 || si == 0x7) ||
      (xv = 0x9c, si == 0x43 || si == 0x45 || si == 0x47))
    goto load_overlay;
  if (si == 0x70) {
    if (!(save_ow_event_info[0x70] & 0x20))
      xv = 0x9f; // rain
  } else {
    xv = (sram_progress_indicator < 2) ? 0x9f : 0x96;
  }
load_overlay:
  map16_load_src_off = 0x390;
  overlay_index = overworld_screen_index = xv;
  map16_load_var2 = (map16_load_src_off - 0x400 & 0xf80) >> 7;
  map16_load_dst_off = (map16_load_src_off - 0x10 & 0x3e) >> 1;
  overworld_screen_transition = 0;
  overworld_screen_trans_dir_bits = 0;
  overworld_screen_trans_dir_bits2 = 0;
  CGWSEL_copy = 0x82;
  TM_copy = 0x16;
  TS_copy = 1;
  sound_effect_ambient = overworld_music[BYTE(overworld_screen_index)] >> 4;

  if (xv == 0x97 || xv == 0x94 || xv == 0x93 || xv == 0x9d || xv == 0x9e || xv == 0x9f)
    CGADSUB_copy = 0x72;
  else if (xv == 0x95 || xv == 0x9c || BYTE(overworld_screen_index_prev) == 0x5b ||
           BYTE(overworld_screen_index_prev) == 0x1b && (submodule_index == 35 || submodule_index == 44))
    CGADSUB_copy = 0x20;
  else
    TS_copy = 0, CGADSUB_copy = 0x20;

  LoadOverworldOverlay();
  if (BYTE(overlay_index) == 0x94)
    BG1VOFS_copy2 |= 0x100;

  overworld_screen_index = overworld_screen_index_prev;
  map16_load_src_off = map16_load_src_off_prev;
  map16_load_var2 = map16_load_var2_prev;
  map16_load_dst_off = map16_load_dst_off_prev;
  overworld_screen_transition = overworld_screen_transition_prev;
  overworld_screen_trans_dir_bits = overworld_screen_trans_dir_bits_prev;
  overworld_screen_trans_dir_bits2 = overworld_screen_trans_dir_bits2_prev;
}

void Module09_FadeBackInFromMosaic() {  // 82b0d2
  Overworld_ResetMosaicDown();
  switch (subsubmodule_index) {
  case 0: {
    uint8 sc = overworld_screen_index;
    Overworld_LoadPalettes(kOverworldBgPalettes[sc], overworld_sprite_palettes[sc]);
    OverworldMosaicTransition_LoadSpriteGraphicsAndSetMosaic();
    break;
  }
  case 1:
    Graphics_IncrementalVRAMUpload();
    ApplyPaletteFilter_bounce();
    break;
  default:
    last_music_control = music_unk1;
    if (BYTE(overworld_screen_index) != 0x80 && BYTE(overworld_screen_index) != 0x2a) {
      uint8 m = overworld_music[BYTE(overworld_screen_index)];
      sound_effect_ambient = (m >> 4) ? (m >> 4) : 5;
      if ((m & 0xf) != music_unk1)
        music_control = (m & 0xf);
    }
    submodule_index = 8;
    subsubmodule_index = 0;
    if (main_module_index == 11) {
      main_module_index = 9;
      submodule_index = 31;
      ow_countdown_transition = 12;
    }
  }
}

void Overworld_Func1C() {  // 82b150
  Overworld_ResetMosaicDown();
  switch (subsubmodule_index) {
  case 0:
    OverworldMosaicTransition_LoadSpriteGraphicsAndSetMosaic();
    break;
  case 1:
    Graphics_IncrementalVRAMUpload();
    ApplyPaletteFilter_bounce();
    break;
  default:
    if (BYTE(overworld_screen_index) < 0x80)
      music_control = (overworld_screen_index & 0x3f) ? 2 : 5;
    submodule_index = 8;
    subsubmodule_index = 0;
    break;
  }
}

void OverworldMosaicTransition_LoadSpriteGraphicsAndSetMosaic() {  // 82b171
  LoadNewSpriteGFXSet();
  INIDISP_copy = 0xf;
  HDMAEN_copy = 0x80;
  BYTE(palette_filter_countdown) = mosaic_target_level - 1;
  mosaic_target_level = 0;
  BYTE(darkening_or_lightening_screen) = 2;
  subsubmodule_index++;
}

void Overworld_Func22() {  // 82b1bb
  if (++INIDISP_copy == 15) {
    submodule_index = 0;
    subsubmodule_index = 0;
  }
}

void Overworld_Func18() {  // 82b1c8
  link_maybe_swim_faster = 0;
  uint8 m = main_module_index;
  uint8 sub = submodule_index;
  Overworld_EnterSpecialArea();
  Overworld_LoadOverlays();
  submodule_index = sub + 1;
  main_module_index = m;
}

void Overworld_Func19() {  // 82b1df
  uint8 m = main_module_index;
  uint8 sub = submodule_index;
  Module08_02_LoadAndAdvance();
  submodule_index = sub + 1;
  main_module_index = m;
}

void Module09_MirrorWarp() {  // 82b1fa
  nmi_disable_core_updates++;
  switch (subsubmodule_index) {
  case 0:
    if (BYTE(overworld_screen_index) >= 0x80) {
      submodule_index = 0;
      subsubmodule_index = 0;
      overworld_map_state = 0;
      return;
    }
    music_control = 8;
    flag_overworld_area_did_change = 8;
    countdown_for_blink = 0x90;
    InitializeMirrorHDMA();
    savegame_is_darkworld ^= 0x40;
    word_7E04C8 = 0;
    BYTE(overworld_screen_index) = BYTE(overworld_area_index) = (overworld_screen_index & 0x3f) | savegame_is_darkworld;
    overworld_map_state = 0;
    PaletteFilter_InitializeWhiteFilter();
    Overworld_LoadGFXAndScreenSize();
    subsubmodule_index++;
    break;
  case 1:
    subsubmodule_index++;
    HDMAEN_copy = 0xc0;
  case 2:
    MirrorWarp_BuildWavingHDMATable();
    break;
  case 3:
    MirrorWarp_BuildDewavingHDMATable();
    break;
  default:
    MirrorWarp_FinalizeAndLoadDestination();
    break;
  }
}

void MirrorWarp_FinalizeAndLoadDestination() {  // 82b260
  HdmaSetup(0, 0xf2fb, 0x41, 0, (uint8)WH0, 0);
  IrisSpotlight_ResetTable();
  palette_filter_countdown = 0;
  darkening_or_lightening_screen = 0;
  ReloadPreviouslyLoadedSheets();
  Overworld_SetSongList();
  HDMAEN_copy = 0x80;
  uint8 m = overworld_music[BYTE(overworld_screen_index)];
  music_control = m & 0xf;
  sound_effect_ambient = m >> 4;
  if (BYTE(overworld_screen_index) >= 0x40 && !link_item_moon_pearl)
    music_control = 4;

  saved_module_for_menu = submodule_index;
  submodule_index = 0;
  subsubmodule_index = 0;
  overworld_map_state = 0;
  nmi_disable_core_updates = 0;
}

void Overworld_DrawScreenAtCurrentMirrorPosition() {  // 82b2e6
  uint16 bak1 = map16_load_src_off;
  uint16 bak2 = map16_load_dst_off;
  uint16 bak3 = map16_load_var2;
  if (kOverworldMapIsSmall[BYTE(overworld_screen_index)]) {
    map16_load_src_off = 0x390;
    map16_load_var2 = (0x390 - 0x400 & 0xf80) >> 7;
    map16_load_dst_off = (0x390 - 0x10 & 0x3e) >> 1;
  }
  Overworld_DrawQuadrantsAndOverlays();
  if (submodule_index == 44)
    MirrorBonk_RecoverChangedTiles();
  map16_load_var2 = bak3;
  map16_load_dst_off = bak2;
  map16_load_src_off = bak1;
}

void MirrorWarp_LoadSpritesAndColors() {  // 82b334
  countdown_for_blink = 0x90;
  uint16 bak1 = map16_load_src_off;
  uint16 bak2 = map16_load_dst_off;
  uint16 bak3 = map16_load_var2;
  if (kOverworldMapIsSmall[BYTE(overworld_screen_index)]) {
    map16_load_src_off = 0x390;
    map16_load_var2 = (0x390 - 0x400 & 0xf80) >> 7;
    map16_load_dst_off = (0x390 - 0x10 & 0x3e) >> 1;
  }
  Map16ToMap8(&g_ram[0x2000], 0);
  map16_load_var2 = bak3;
  map16_load_dst_off = bak2;
  map16_load_src_off = bak1;
  OverworldLoadScreensPaletteSet();
  uint8 sc = overworld_screen_index;
  Overworld_LoadPalettes(kOverworldBgPalettes[sc], overworld_sprite_palettes[sc]);
  Palette_SpecialOw();
  Overworld_SetFixedColAndScroll();
  if (BYTE(overworld_screen_index) == 0x1b || BYTE(overworld_screen_index) == 0x5b)
    TS_copy = 1;
  for (int i = 0; i < 16 * 6; i++)
    main_palette_buffer[32 + i] = 0x7fff;
  main_palette_buffer[0] = 0x7fff;
  if (overworld_screen_index == 0x5b) {
    main_palette_buffer[0] = 0;
    main_palette_buffer[32] = 0;
  }
  Sprite_ResetAll();
  Sprite_ReloadAll_Overworld();
  Link_ItemReset_FromOverworldThings();
  Dungeon_ResetTorchBackgroundAndPlayerInner();
  link_player_handler_state = kPlayerState_Mirror;
  if (!(overworld_screen_index & 0x40))
    Sprite_InitializeMirrorPortal();
}

void Overworld_Func2B() {  // 82b40a
  Palette_AnimGetMasterSword();
}

void Overworld_WeathervaneExplosion() {  // 82b40e
  // empty
}

void Module09_2E_Whirlpool() {  // 82b40f
  // this is called when entering the whirlpool
  nmi_disable_core_updates++;
  switch (subsubmodule_index) {
  case 0:
    sound_effect_1 = 0x34;
    sound_effect_ambient = 5;
    overworld_map_state = 0;
    palette_filter_countdown = 0;
    subsubmodule_index++;
    break;
  case 1:
    PaletteFilter_WhirlpoolBlue();
    break;
  case 2:
    PaletteFilter_IsolateWhirlpoolBlue();
    break;
  case 3:
    COLDATA_copy2 = 0x9f;
    overworld_palette_aux_or_main = 0;
    hud_palette = 0;
    FindPartnerWhirlpoolExit();
    BYTE(dung_draw_width_indicator) = 0;
    Overworld_LoadOverlays2();
    submodule_index--;
    nmi_subroutine_index = 12;
    flag_update_cgram_in_nmi = 0;
    COLDATA_copy2 = 0x80;
    INIDISP_copy = 0xf;
    nmi_disable_core_updates++;
    subsubmodule_index++;
    break;
  case 4: case 6:
    nmi_subroutine_index = 13;
    nmi_disable_core_updates++;
    subsubmodule_index++;
    break;
  case 5:
    Overworld_LoadOverlayAndMap();
    nmi_subroutine_index = 12;
    INIDISP_copy = 0xf;
    nmi_disable_core_updates++;
    subsubmodule_index++;
    break;
  case 7:
    Module09_LoadAuxGFX();
    submodule_index--;
    subsubmodule_index++;
    break;
  case 8:
    Overworld_FinishTransGfx();
    INIDISP_copy = 0xf;
    nmi_disable_core_updates++;
    submodule_index--;
    subsubmodule_index++;
    break;
  case 9: {
    overworld_palette_aux_or_main = 0;
    Palette_Load_SpriteMain();
    Palette_Load_SpriteEnvironment();
    Palette_Load_SpritePal0Left();
    Palette_Load_HUD();
    Palette_Load_OWBGMain();
    uint8 sc = overworld_screen_index;
    Overworld_LoadPalettes(kOverworldBgPalettes[sc], overworld_sprite_palettes[sc]);
    Palette_SetOwBgColor();
    Overworld_SetFixedColAndScroll();
    LoadNewSpriteGFXSet();
    COLDATA_copy2 = 0x80;
    INIDISP_copy = 0xf;
    nmi_disable_core_updates++;
    subsubmodule_index++;
    break;
  }
  case 10:
    PaletteFilter_WhirlpoolRestoreRedGreen();
    if (BYTE(palette_filter_countdown))
      PaletteFilter_WhirlpoolRestoreRedGreen();
    break;
  case 11:
    Graphics_IncrementalVRAMUpload();
    PaletteFilter_WhirlpoolRestoreBlue();
    break;
  case 12:
    countdown_for_blink = 144;
    ReloadPreviouslyLoadedSheets();
    HDMAEN_copy = 0x80;
    sound_effect_ambient = overworld_music[BYTE(overworld_screen_index)] >> 4;
    music_control = savegame_is_darkworld ? 9 : 2;
    submodule_index = 0;
    subsubmodule_index = 0;
    overworld_map_state = 0;
    nmi_disable_core_updates = 0;
    break;
  }
}

void Overworld_Func2F() {  // 82b521
  dung_bg2[0x720 / 2] = 0x212;
  Overworld_Memorize_Map16_Change(0x720, 0x212);
  Overworld_DrawMap16(0x720, 0x212);
  nmi_load_bg_from_vram = 1;
  submodule_index = 0;
}

void Module09_2A_RecoverFromDrowning() {  // 82b528
  // this is called for example when entering water without swim capability
  switch (subsubmodule_index) {
  case 0:  Module09_2A_00_ScrollToLand(); break;
  default: RecoverPositionAfterDrowning(); break;
  }
}

void Module09_2A_00_ScrollToLand() {  // 82b532
  uint16 x = link_x_coord, xd = 0;
  if (x != link_x_coord_cached) {
    int16 d = (x > link_x_coord_cached) ? -1 : 1;
    ((x += d) != link_x_coord_cached) && (x += d);
    xd = x - link_x_coord;
    link_x_coord = x;
  }
  uint16 y = link_y_coord, yd = 0;
  if (y != link_y_coord_cached) {
    int16 d = (y > link_y_coord_cached) ? -1 : 1;
    ((y += d) != link_y_coord_cached) && (y += d);
    yd = y - link_y_coord;
    link_y_coord = y;
  }
  link_y_vel = yd;
  link_x_vel = xd;
  if (y == link_y_coord_cached && x == link_x_coord_cached) {
    subsubmodule_index++;
    link_incapacitated_timer = 0;
    set_when_damaging_enemies = 0;
  }
  Overworld_OperateCameraScroll();
  if (BYTE(overworld_screen_trans_dir_bits2))
    OverworldHandleMapScroll();
}

void Overworld_OperateCameraScroll() {  // 82bb90
  int z = (allow_scroll_z && link_z_coord != 0xffff) ? link_z_coord : 0;
  uint16 y = link_y_coord - z + 12;

  if (link_y_vel != 0) {
    int vy = sign8(link_y_vel) ? -1 : 1;
    int av = sign8(link_y_vel) ? (link_y_vel ^ 0xff) + 1 : link_y_vel;
    uint16 r4 = 0, subp;
    do {
      if (sign8(link_y_vel)) {
        if (y <= camera_y_coord_scroll_low)
          r4 += OverworldCameraBoundaryCheck(6, 0, vy, 0);
      } else {
        if (y >= camera_y_coord_scroll_hi)
          r4 += OverworldCameraBoundaryCheck(6, 2, vy, 0);
      }
    } while (--av);
    WORD(byte_7E069E[0]) = r4;
    uint8 oi = BYTE(overlay_index);
    if (oi != 0x97 && oi != 0x9d && r4 != 0) {
      if (oi == 0xb5 || oi == 0xbe) {
        subp = (r4 & 3) << 14;
        r4 >>= 2;
        if (r4 >= 0x3000)
          r4 |= 0xf000;
      } else {
        subp = (r4 & 1) << 15;
        r4 >>= 1;
        if (r4 >= 0x7000)
          r4 |= 0xf000;
      }
      uint32 tmp = BG1VOFS_subpixel | BG1VOFS_copy2 << 16;
      tmp += subp | r4 << 16;
      BG1VOFS_subpixel = (uint16)(tmp);
      BG1VOFS_copy2 = (uint16)(tmp >> 16);
      if ((overworld_screen_index & 0x3f) == 0x1b) {
        if (BG1VOFS_copy2 <= 0x600)
          BG1VOFS_copy2 = 0x600;
        else if (BG1VOFS_copy2 >= 0x6c0)
          BG1VOFS_copy2 = 0x6c0;
      }
    }
  }

  uint16 x = link_x_coord + 8;
  if (link_x_vel != 0) {
    int vx = sign8(link_x_vel) ? -1 : 1;
    int ax = sign8(link_x_vel) ? (link_x_vel ^ 0xff) + 1 : link_x_vel;
    uint16 r4 = 0, subp;
    do {
      if (sign8(link_x_vel)) {
        if (x <= camera_x_coord_scroll_low)
          r4 += OverworldCameraBoundaryCheck(0, 4, vx, 4);
      } else {
        if (x >= camera_x_coord_scroll_hi)
          r4 += OverworldCameraBoundaryCheck(0, 6, vx, 4);
      }
    } while (--ax);
    WORD(byte_7E069E[1]) = r4;
    uint8 oi = BYTE(overlay_index);
    if (oi != 0x97 && oi != 0x9d && r4 != 0) {
      if (oi == 0x95 || oi == 0x9e) {
        subp = (r4 & 3) << 14;
        r4 >>= 2;
        if (r4 >= 0x3000)
          r4 |= 0xf000;
      } else {
        subp = (r4 & 1) << 15;
        r4 >>= 1;
        if (r4 >= 0x7000)
          r4 |= 0xf000;
      }
      uint32 tmp = BG1HOFS_subpixel | BG1HOFS_copy2 << 16;
      tmp += subp | r4 << 16;
      BG1HOFS_subpixel = (uint16)(tmp), BG1HOFS_copy2 = (uint16)(tmp >> 16);
    }
  }
  if (BYTE(overworld_screen_index) != 0x47) {
    if (BYTE(overlay_index) == 0x9c) {
      uint32 tmp = BG1VOFS_subpixel | BG1VOFS_copy2 << 16;
      tmp -= 0x2000;
      BG1VOFS_subpixel = (uint16)(tmp), BG1VOFS_copy2 = (uint16)(tmp >> 16) + WORD(byte_7E069E[0]);
      BG1HOFS_copy2 = BG2HOFS_copy2;
    } else if (BYTE(overlay_index) == 0x97 || BYTE(overlay_index) == 0x9d) {
      uint32 tmp = BG1VOFS_subpixel | BG1VOFS_copy2 << 16;
      tmp += 0x2000;
      BG1VOFS_subpixel = (uint16)(tmp), BG1VOFS_copy2 = (uint16)(tmp >> 16);
      tmp = BG1HOFS_subpixel | BG1HOFS_copy2 << 16;
      tmp += 0x2000;
      BG1HOFS_subpixel = (uint16)(tmp), BG1HOFS_copy2 = (uint16)(tmp >> 16);
    }
  }

  if (dungeon_room_index == 0x181) {
    BG1VOFS_copy2 = BG2VOFS_copy2 | 0x100;
    BG1HOFS_copy2 = BG2HOFS_copy2;
  }
}

int OverworldCameraBoundaryCheck(int xa, int ya, int vd, int r8) {  // 82bd62
  ya >>= 1, r8 >>= 1;

  uint16 *xp = (xa ? &BG2VOFS_copy2 : &BG2HOFS_copy2);
  uint16 *yp = &(&ow_scroll_vars0.ystart)[ya];
  if (*xp == *yp) {
    (&overworld_unk1)[ya] = 0;
    (&overworld_unk1)[ya ^ 1] = 0;
    return 0;
  }
  *xp += vd;

  int tt = vd + (&camera_y_coord_scroll_hi)[r8];
  (&camera_y_coord_scroll_hi)[r8] = tt;
  (&camera_y_coord_scroll_low)[r8] = tt + 2;
  uint16 *op = (&overworld_unk1) + ya;
  if (!sign16(++(*op) - 0x10)) {
    (*op) -= 0x10;
    overworld_screen_trans_dir_bits2 |= kOverworld_Func2_Tab[ya];
  }
  (&overworld_unk1)[ya ^ 1] = -(&overworld_unk1)[ya];
  return vd;
}

int OverworldScrollTransition() {  // 82c001
  transition_counter++;
  int y = overworld_screen_transition;
  int d = kOverworld_Func6B_Tab1[y], rv;
  if (y < 2) {
    byte_7E069E[0] = d;
    rv = (BG2VOFS_copy2 += d);
    if (BYTE(overworld_screen_index) != 0x1b && BYTE(overworld_screen_index) != 0x5b)
      BG1VOFS_copy2 = BG2VOFS_copy2;
    if (transition_counter >= kOverworld_Func6B_Tab2[y])
      link_y_coord += d;
    if (rv != (&up_down_scroll_target)[y])
      return rv;
    if (y == 0)
      BG2VOFS_copy2 -= 2;
    link_y_coord &= ~7;
    camera_y_coord_scroll_hi = link_y_coord + kOverworld_Func6B_Tab3[y] + 11;
    camera_y_coord_scroll_low = camera_y_coord_scroll_hi + 2;
    overworld_unk1 = overworld_unk1_neg = 0;
  } else {
    byte_7E069E[1] = d;
    rv = (BG2HOFS_copy2 += d);
    if (BYTE(overworld_screen_index) != 0x1b && BYTE(overworld_screen_index) != 0x5b)
      BG1HOFS_copy2 = BG2HOFS_copy2;
    if (transition_counter >= kOverworld_Func6B_Tab2[y])
      link_x_coord += d;
    if (rv != (&up_down_scroll_target)[y])
      return rv;
    link_x_coord &= ~7;
    camera_x_coord_scroll_hi = link_x_coord + kOverworld_Func6B_Tab3[y] + 11;
    camera_x_coord_scroll_low = camera_x_coord_scroll_hi + 2;
    overworld_unk3 = overworld_unk3_neg = 0;
  }
  Overworld_SetCameraBoundaries(overworld_area_is_big != 0, (current_area_of_player >> 1) + kOverworld_Func6B_AreaDelta[y]);

  flag_overworld_area_did_change = 1;
  submodule_index += 1;
  subsubmodule_index = 0;
  transition_counter = 0;
  Sprite_InitializeSlots();
  return rv;
}

void Overworld_SetCameraBoundaries(int big, int area) {  // 82c0c3
  ow_scroll_vars0.ystart = kOverworld_OffsetBaseY[area];
  ow_scroll_vars0.yend = ow_scroll_vars0.ystart + kOverworld_Size1[big];
  ow_scroll_vars0.xstart = kOverworld_OffsetBaseX[area];
  ow_scroll_vars0.xend = ow_scroll_vars0.xstart + kOverworld_Size2[big];
  up_down_scroll_target = kOverworld_UpDownScrollTarget[area];
  up_down_scroll_target_end = up_down_scroll_target + kOverworld_UpDownScrollSize[big];
  left_right_scroll_target = kOverworld_LeftRightScrollTarget[area];
  left_right_scroll_target_end = left_right_scroll_target + kOverworld_LeftRightScrollSize[big];
}

void Overworld_FinalizeEntryOntoScreen() {  // 82c242
  Link_HandleMovingAnimation_FullLongEntry();
  int d = (byte_7E069C & 1) ? 2 : -2;
  if (byte_7E069C & 2)
    link_x_coord = (d += link_x_coord);
  else
    link_y_coord = (d += link_y_coord);
  if ((d & 0xfe) == kOverworld_Func8_tab[byte_7E069C]) {
    submodule_index = 0;
    subsubmodule_index = 0;
    uint8 m = overworld_music[BYTE(overworld_screen_index)];
    sound_effect_ambient = m >> 4;
    if (music_unk1 == 0xf1)
      music_control = m & 0xf;
  }
  Overworld_OperateCameraScroll();
  if (BYTE(overworld_screen_trans_dir_bits2))
    OverworldHandleMapScroll();
}

void Overworld_Func1F() {  // 82c2a4
  Link_HandleMovingAnimation_FullLongEntry();
  int8 vel = byte_7E069C & 1 ? 1 : -1;
  if (byte_7E069C & 2) {
    link_x_coord += vel;
    link_x_vel = vel;
  } else {
    link_y_coord += vel;
    link_y_vel = vel;
  }
  if (!--ow_countdown_transition) {
    main_module_index = 9;
    subsubmodule_index = submodule_index = 0;
  }
  Overworld_OperateCameraScroll();
}

void ConditionalMosaicControl() {  // 82c2e4
  if (palette_filter_countdown & 1)
    mosaic_level += 0x10;
  BGMODE_copy = 9;
  MOSAIC_copy = mosaic_level | 7;
}

void Overworld_ResetMosaic_alwaysIncrease() {  // 82c2eb
  mosaic_level += 0x10;
  BGMODE_copy = 9;
  MOSAIC_copy = mosaic_level | 7;
}

void Overworld_SetSongList() {  // 82c463
  uint8 r0 = 2, y = 0xc0;
  if (sram_progress_indicator < 3) {
    y = 0x80;
    if (link_sword_type < 2) {
      r0 = 5;
      y = 0x40;
      if (sram_progress_indicator < 2)
        y = 0;
    }
  }
  memcpy(overworld_music, &kOwMusicSets[y], 64);
  memcpy(overworld_music + 64, kOwMusicSets2, 96);
  overworld_music[128] = r0;
}

void LoadOverworldFromDungeon() {  // 82e4a3
  player_is_indoors = 0;
  hdr_dungeon_dark_with_lantern = 0;
  WORD(overworld_fixed_color_plusminus) = 0;
  cur_palace_index_x2 = 0xff;
  num_memorized_tiles = 0;

  if (dungeon_room_index != 0x104 && dungeon_room_index < 0x180 && dungeon_room_index >= 0x100) {
    LoadCachedEntranceProperties();
  } else {

    int k = 79;
    do k--; while (kExitDataRooms[k] != dungeon_room_index);
    BG1VOFS_copy2 = BG2VOFS_copy2 = BG1VOFS_copy = BG2VOFS_copy = kExitData_ScrollY[k];
    BG1HOFS_copy2 = BG2HOFS_copy2 = BG1HOFS_copy = BG2HOFS_copy = kExitData_ScrollX[k];
    link_y_coord = kExitData_YCoord[k];
    link_x_coord = kExitData_XCoord[k];
    map16_load_src_off = kExitData_Map16LoadSrcOff[k];
    map16_load_var2 = (map16_load_src_off - 0x400 & 0xf80) >> 7;
    map16_load_dst_off = (map16_load_src_off - 0x10 & 0x3e) >> 1;
    camera_y_coord_scroll_low = kExitData_CameraYScroll[k];
    camera_y_coord_scroll_hi = camera_y_coord_scroll_low - 2;
    camera_x_coord_scroll_low = kExitData_CameraXScroll[k];
    camera_x_coord_scroll_hi = camera_x_coord_scroll_low - 2;
    WORD(link_direction_facing) = 2;
    ow_entrance_value = kExitData_NormalDoor[k];
    big_rock_starting_address = kExitData_FancyDoor[k];
    overworld_area_index = overworld_screen_index = kExitData_ScreenIndex[k];
    overworld_unk1 = kExitData_Unk1[k];
    overworld_unk3 = kExitData_Unk3[k];
    overworld_unk1_neg = -overworld_unk1;
    overworld_unk3_neg = -overworld_unk3;
  }
  Overworld_LoadNewScreenProperties();
}

void Overworld_LoadNewScreenProperties() {  // 82e58b
  tilemap_location_calc_mask = ~7;
  Overworld_LoadGFXAndScreenSize();
  BYTE(overworld_right_bottom_bound_for_scroll) = 0xe4;
  overworld_area_is_big &= 0xff;
  Overworld_SetCameraBoundaries(overworld_area_is_big != 0, overworld_screen_index & 0x3f);
  link_quadrant_x = 0;
  link_quadrant_y = 2;
  quadrant_fullsize_x = 2;
  quadrant_fullsize_y = 2;
  player_oam_x_offset = player_oam_y_offset = 0x80;
  link_direction_mask_a = link_direction_mask_b = 0xf;
  BYTE(link_z_coord) = 0xff;
  link_actual_vel_z = 0xff;
}

void LoadCachedEntranceProperties() {  // 82e5d4
  overworld_area_index = overworld_area_index_exit;
  WORD(TM_copy) = WORD(TM_copy_exit);
  BG2VOFS_copy2 = BG2VOFS_copy = BG1VOFS_copy2 = BG1VOFS_copy = BG2VOFS_copy2_exit;
  BG2HOFS_copy2 = BG2HOFS_copy = BG1HOFS_copy2 = BG1HOFS_copy = BG2HOFS_copy2_exit;
  link_x_coord = link_x_coord_exit;
  link_y_coord = link_y_coord_exit;
  if (dungeon_room_index < 0x124)
    link_y_coord -= 0x10;
  WORD(link_direction_facing) = 2;
  if (ow_entrance_value == 0xffff) {
    link_y_coord += 0x20;
    WORD(link_direction_facing) = 0;
  }
  overworld_screen_index = overworld_screen_index_exit;
  map16_load_src_off = map16_load_src_off_exit;
  map16_load_var2 = (map16_load_src_off - 0x400 & 0xf80) >> 7;
  map16_load_dst_off = (map16_load_src_off - 0x10 & 0x3e) >> 1;
  camera_y_coord_scroll_low = camera_y_coord_scroll_low_exit;
  camera_y_coord_scroll_hi = camera_y_coord_scroll_low - 2;
  camera_x_coord_scroll_low = camera_x_coord_scroll_low_exit;
  camera_x_coord_scroll_hi = camera_x_coord_scroll_low - 2;
  ow_scroll_vars0 = ow_scroll_vars0_exit;
  up_down_scroll_target = up_down_scroll_target_exit;
  up_down_scroll_target_end = up_down_scroll_target_end_exit;
  left_right_scroll_target = left_right_scroll_target_exit;
  left_right_scroll_target_end = left_right_scroll_target_end_exit;
  overworld_unk1 = overworld_unk1_exit;
  overworld_unk1_neg = overworld_unk1_neg_exit;
  overworld_unk3 = overworld_unk3_exit;
  overworld_unk3_neg = overworld_unk3_neg_exit;
  byte_7E0AA0 = byte_7EC164;
  main_tile_theme_index = main_tile_theme_index_exit;
  aux_tile_theme_index = aux_tile_theme_index_exit;
  sprite_graphics_index = sprite_graphics_index_exit;

}

void Overworld_EnterSpecialArea() {  // 82e851
  num_memorized_tiles = 0;
  overworld_area_index_spexit = overworld_area_index;
  WORD(TM_copy_spexit) = WORD(TM_copy);
  BG2VOFS_copy2_spexit = BG2VOFS_copy2;
  BG2HOFS_copy2_spexit = BG2HOFS_copy2;

  link_x_coord_spexit = link_x_coord;
  link_y_coord_spexit = link_y_coord;

  camera_y_coord_scroll_low_spexit = camera_y_coord_scroll_low;
  camera_x_coord_scroll_low_spexit = camera_x_coord_scroll_low;
  overworld_screen_index_spexit = overworld_screen_index;
  map16_load_src_off_spexit = map16_load_src_off;
  room_scroll_vars0_ystart_spexit = ow_scroll_vars0.ystart;
  room_scroll_vars0_yend_spexit = ow_scroll_vars0.yend;
  room_scroll_vars0_xstart_spexit = ow_scroll_vars0.xstart;
  room_scroll_vars0_xend_spexit = ow_scroll_vars0.xend;

  up_down_scroll_target_spexit = up_down_scroll_target;
  up_down_scroll_target_end_spexit = up_down_scroll_target_end;
  left_right_scroll_target_spexit = left_right_scroll_target;
  left_right_scroll_target_end_spexit = left_right_scroll_target_end;
  overworld_unk1_spexit = overworld_unk1;
  overworld_unk1_neg_spexit = overworld_unk1_neg;
  overworld_unk3_spexit = overworld_unk3;
  overworld_unk3_neg_spexit = overworld_unk3_neg;
  byte_7EC124 = byte_7E0AA0;
  main_tile_theme_index_spexit = main_tile_theme_index;
  aux_tile_theme_index_spexit = aux_tile_theme_index;
  sprite_graphics_index_spexit = sprite_graphics_index;
  LoadOverworldFromDungeon();
  if (dungeon_room_index == 0x1010)
    dungeon_room_index = 0x182;

  uint8 roombak = dungeon_room_index;
  int i = (BYTE(dungeon_room_index) -= 0x80);
  link_direction_facing = kSpExit_Dir[i];
  incremental_counter_for_vram = 0;
  sprite_graphics_index = kSpExit_SprGfx[i];
  aux_tile_theme_index = kSpExit_AuxGfx[i];
  Overworld_LoadPalettes(kSpExit_PalBg[i], kSpExit_PalSpr[i]);

  int j = dungeon_room_index & 0x3f;
  overworld_offset_base_y = kSpExit_Top[j];
  overworld_offset_base_x = kSpExit_LeftEdgeOfMap[j] >> 3;
  overworld_offset_mask_y = 0x3f0;
  overworld_offset_mask_x = 0x3f0 >> 3;

  int k = dungeon_room_index & 0x7f;
  ow_scroll_vars0.ystart = kSpExit_Top[k];
  ow_scroll_vars0.yend = kSpExit_Bottom[k];
  ow_scroll_vars0.xstart = kSpExit_Left[k];
  ow_scroll_vars0.xend = kSpExit_Right[k];
  up_down_scroll_target = kSpExit_Tab4[k];
  up_down_scroll_target_end = kSpExit_Tab5[k];
  left_right_scroll_target = kSpExit_Tab6[k];
  left_right_scroll_target_end = kSpExit_Tab7[k];

  BYTE(dungeon_room_index) = roombak;
  Palette_SpecialOw();
}

void LoadOverworldFromSpecialOverworld() {  // 82e9bc
  num_memorized_tiles = 0;
  overworld_area_index = overworld_area_index_spexit;
   WORD(TM_copy) = WORD(TM_copy_spexit);
  BG2VOFS_copy2 = BG2VOFS_copy = BG1VOFS_copy2 = BG1VOFS_copy = BG2VOFS_copy2_spexit;
  BG2HOFS_copy2 = BG2HOFS_copy = BG1HOFS_copy2 = BG1HOFS_copy = BG2HOFS_copy2_spexit;
  link_x_coord = link_x_coord_spexit;
  link_y_coord = link_y_coord_spexit;
  overworld_screen_index = overworld_screen_index_spexit;
  map16_load_src_off = map16_load_src_off_spexit;
  map16_load_var2 = (map16_load_src_off - 0x400 & 0xf80) >> 7;
  map16_load_dst_off = (map16_load_src_off - 0x10 & 0x3e) >> 1;
  camera_y_coord_scroll_low = camera_y_coord_scroll_low_spexit;
  camera_y_coord_scroll_hi = camera_y_coord_scroll_low - 2;
  camera_x_coord_scroll_low = camera_x_coord_scroll_low_spexit;
  camera_x_coord_scroll_hi = camera_x_coord_scroll_low - 2;
  ow_scroll_vars0.ystart = room_scroll_vars0_ystart_spexit;
  ow_scroll_vars0.yend = room_scroll_vars0_yend_spexit;
  ow_scroll_vars0.xstart = room_scroll_vars0_xstart_spexit;
  ow_scroll_vars0.xend = room_scroll_vars0_xend_spexit;
  up_down_scroll_target = up_down_scroll_target_spexit;
  up_down_scroll_target_end = up_down_scroll_target_end_spexit;
  left_right_scroll_target = left_right_scroll_target_spexit;
  left_right_scroll_target_end = left_right_scroll_target_end_spexit;
  overworld_unk1 = overworld_unk1_spexit;
  overworld_unk1_neg = overworld_unk1_neg_spexit;
  overworld_unk3 = overworld_unk3_spexit;
  overworld_unk3_neg = overworld_unk3_neg_spexit;
  byte_7E0AA0 = byte_7EC124;
  main_tile_theme_index = main_tile_theme_index_spexit;
  aux_tile_theme_index = aux_tile_theme_index_spexit;
  sprite_graphics_index = sprite_graphics_index_spexit;
  uint8 sc = overworld_screen_index;
  Overworld_LoadPalettes(kOverworldBgPalettes[sc], overworld_sprite_palettes[sc]);
  Palette_SpecialOw();
  link_quadrant_x = 0;
  link_quadrant_y = 2;
  quadrant_fullsize_x = 2;
  quadrant_fullsize_y = 2;
  player_oam_x_offset = player_oam_y_offset = 0x80;
  link_direction_mask_a = link_direction_mask_b = 0xf;
  BYTE(link_z_coord) = 0xff;
  link_actual_vel_z = 0xff;
  Link_ResetSwimmingState();
  Overworld_LoadGFXAndScreenSize();
  BYTE(overworld_right_bottom_bound_for_scroll) = 228;
  overworld_area_is_big &= 0xff;
}

void FluteMenu_LoadTransport() {  // 82ec39
  num_memorized_tiles = 0;
  int k = birdtravel_var1[0];
  WORD(birdtravel_var1[0]) <<= 1;
  Overworld_LoadBirdTravelPos(k);
}

void Overworld_LoadBirdTravelPos(int k) {  // 82ec47
  BG1VOFS_copy2 = BG2VOFS_copy2 = BG1VOFS_copy = BG2VOFS_copy = kBirdTravel_ScrollY[k];
  BG1HOFS_copy2 = BG2HOFS_copy2 = BG1HOFS_copy = BG2HOFS_copy = kBirdTravel_ScrollX[k];
  link_y_coord = kBirdTravel_LinkYCoord[k];
  link_x_coord = kBirdTravel_LinkXCoord[k];
  overworld_unk1 = kBirdTravel_Unk1[k];
  overworld_unk3 = kBirdTravel_Unk3[k];
  overworld_unk1_neg = -overworld_unk1;
  overworld_unk3_neg = -overworld_unk3;
  overworld_area_index = overworld_screen_index = kBirdTravel_ScreenIndex[k];

  map16_load_src_off = kBirdTravel_Map16LoadSrcOff[k];
  map16_load_var2 = (map16_load_src_off - 0x400 & 0xf80) >> 7;
  map16_load_dst_off = (map16_load_src_off - 0x10 & 0x3e) >> 1;
  camera_y_coord_scroll_low = kBirdTravel_CameraYScroll[k];
  camera_y_coord_scroll_hi = camera_y_coord_scroll_low - 2;
  camera_x_coord_scroll_low = kBirdTravel_CameraXScroll[k];
  camera_x_coord_scroll_hi = camera_x_coord_scroll_low - 2;
  ow_entrance_value = 0;
  big_rock_starting_address = 0;
  Overworld_LoadNewScreenProperties();
  Sprite_ResetAll();
  Sprite_ReloadAll_Overworld();
  is_standing_in_doorway = 0;
  Dungeon_ResetTorchBackgroundAndPlayerInner();
}

void FluteMenu_LoadSelectedScreenPalettes() {  // 82ecdd
  OverworldLoadScreensPaletteSet();
  uint8 sc = overworld_screen_index;
  Overworld_LoadPalettes(kOverworldBgPalettes[sc], overworld_sprite_palettes[sc]);
  Palette_SetOwBgColor();
  Overworld_LoadPalettesInner();
}

void FindPartnerWhirlpoolExit() {  // 82ed08
  int j = FindInWordArray(kWhirlpoolAreas, overworld_screen_index, countof(kWhirlpoolAreas));
  if (j >= 0) {
    num_memorized_tiles = 0;
    Overworld_LoadBirdTravelPos(j + 9);
  }
}

void Overworld_LoadAmbientOverlay(bool load_map_data) {  // 82ed25
  uint16 bak1 = map16_load_src_off;
  uint16 bak2 = map16_load_dst_off;
  uint16 bak3 = map16_load_var2;

  if (kOverworldMapIsSmall[BYTE(overworld_screen_index)]) {
    map16_load_src_off = 0x390;
    map16_load_var2 = (0x390 - 0x400 & 0xf80) >> 7;
    map16_load_dst_off = (0x390 - 0x10 & 0x3e) >> 1;
  }

  if (load_map_data)
    Overworld_DrawQuadrantsAndOverlays();

  Map16ToMap8(&g_ram[0x2000], 0);
  map16_load_var2 = bak3;
  map16_load_dst_off = bak2;
  map16_load_src_off = bak1;

  nmi_subroutine_index = 4;
  nmi_disable_core_updates = 4;
  submodule_index++;
  INIDISP_copy = 0;
}

void Overworld_LoadAmbientOverlayFalse() {  // 82ed24
  Overworld_LoadAmbientOverlay(false);
}

void Overworld_LoadAndBuildScreen() {  // 82ed59
  Overworld_LoadAmbientOverlay(true);
}

void Module08_02_LoadAndAdvance() {  // 82edb9
  Overworld_LoadAndBuildScreen();
  main_module_index = 16;
  submodule_index = 0;
  subsubmodule_index = 0;
}

void Overworld_DrawQuadrantsAndOverlays() {  // 82eec5
  Overworld_DecompressAndDrawAllQuadrants();
  for (int i = 0; i < 16 * 4; i++)
    dung_bg1[i] = 0xdc4;
  uint16 pos = ow_entrance_value;
  if (pos != 0 && pos != 0xffff) {
    if (pos < 0x8000) {
      dung_bg2[pos >> 1] = 0xDA4;
      Overworld_Memorize_Map16_Change(pos, 0xda4);
      dung_bg2[(pos + 2) >> 1] = 0xda6;
      Overworld_Memorize_Map16_Change(pos + 2, 0xda6);
    } else {
      pos &= 0x1fff;
      dung_bg2[pos >> 1] = 0xdb4;
      Overworld_Memorize_Map16_Change(pos, 0xdb4);
      dung_bg2[(pos + 2) >> 1] = 0xdb5;
      Overworld_Memorize_Map16_Change(pos + 2, 0xdb5);
    }
    ow_entrance_value = 0;
  }
  Overworld_HandleOverlaysAndBombDoors();
}

void Overworld_HandleOverlaysAndBombDoors() {  // 82ef29
  if (overworld_screen_index == 0x33)
    dung_bg2[340] = 0x20f;
  else if (overworld_screen_index == 0x2f)
    dung_bg2[1497] = 0x20f;
  if (BYTE(overworld_screen_index) < 0x80 && save_ow_event_info[BYTE(overworld_screen_index)] & 0x20)
    Overworld_LoadEventOverlay();
  if (save_ow_event_info[BYTE(overworld_screen_index)] & 2) {
    int pos = kSecondaryOverlayPerOw[overworld_screen_index] >> 1;
    dung_bg2[pos + 0] = 0xdb4;
    dung_bg2[pos + 1] = 0xdb5;
  }
}

void TriggerAndFinishMapLoadStripe_Y(int n) {  // 82ef7a
  BYTE(overworld_screen_trans_dir_bits2) = 8;
  nmi_subroutine_index = 3;
  uint16 *dst = uvram.data;
  *dst++ = 0x80;
  do {
    dst = BufferAndBuildMap16Stripes_Y(dst);
    map16_load_src_off -= 0x80;
    map16_load_var2 = (map16_load_var2 - 1) & 0x1f;
  } while (--n);
  *dst = 0xffff;
}

void TriggerAndFinishMapLoadStripe_X(int n) {  // 82efb3
  BYTE(overworld_screen_trans_dir_bits2) = 2;
  nmi_subroutine_index = 3;
  uint16 *dst = uvram.data;
  *dst++ = 0x8040;
  do {
    dst = BufferAndBuildMap16Stripes_X(dst);
    map16_load_src_off -= 2;
    map16_load_dst_off = (map16_load_dst_off - 1) & 0x1f;
  } while (--n);
  *dst = 0xffff;
}

void SomeTileMapChange() {  // 82efe8
  Overworld_DecompressAndDrawAllQuadrants();
  for (int i = 0; i < 64; i++)
    dung_bg1[i] = 0xdc4;
  Overworld_HandleOverlaysAndBombDoors();
  submodule_index++;
}

void CreateInitialNewScreenMapToScroll() {  // 82f031
  if (!kOverworldMapIsSmall[BYTE(overworld_screen_index)]) {
    switch (BYTE(overworld_screen_trans_dir_bits2)) {
    case 1: CreateInitialOWScreenView_Big_East(); break;
    case 2: CreateInitialOWScreenView_Big_West(); break;
    case 4: CreateInitialOWScreenView_Big_South(); break;
    case 8: CreateInitialOWScreenView_Big_North(); break;
    default:
      assert(0);
      submodule_index = 0;
    }
  } else {
    switch (BYTE(overworld_screen_trans_dir_bits2)) {
    case 1: CreateInitialOWScreenView_Small_East(); break;
    case 2: CreateInitialOWScreenView_Small_West(); break;
    case 4: CreateInitialOWScreenView_Small_South(); break;
    case 8: CreateInitialOWScreenView_Small_North(); break;
    default:
      assert(0);
      submodule_index = 0;
    }
  }
}

void CreateInitialOWScreenView_Big_North() {  // 82f06b
  map16_load_src_off += 0x380;
  map16_load_var2 = 31;
  TriggerAndFinishMapLoadStripe_Y(7);
}

void CreateInitialOWScreenView_Big_South() {  // 82f087
  uint16 pos = map16_load_src_off;
  while (pos >= 0x80)
    pos -= 0x80;
  map16_load_src_off = pos + 0x780;
  map16_load_var2 = 7;
  TriggerAndFinishMapLoadStripe_Y(8);
  map16_load_var2 = (map16_load_var2 + 9) & 0x1f;
  map16_load_src_off -= 0xB80;
}

void CreateInitialOWScreenView_Big_West() {  // 82f0c0
  map16_load_src_off += 14;
  map16_load_dst_off = 31;
  TriggerAndFinishMapLoadStripe_X(7);
}

void CreateInitialOWScreenView_Big_East() {  // 82f0dc
  map16_load_src_off = map16_load_src_off - 0x60 + 0x1e;
  map16_load_dst_off = 7;
  TriggerAndFinishMapLoadStripe_X(8);
  map16_load_dst_off = (map16_load_dst_off + 9) & 0x1f;
  map16_load_src_off -= 0x2e;
}

void CreateInitialOWScreenView_Small_North() {  // 82f10f
  orange_blue_barrier_state = map16_load_src_off - 0x700;
  word_7EC174 = map16_load_dst_off;
  word_7EC176 = 10;
  map16_load_src_off = 0x1390;
  map16_load_dst_off = 0;
  map16_load_var2 = 31;
  TriggerAndFinishMapLoadStripe_Y(7);
}

void CreateInitialOWScreenView_Small_South() {  // 82f141
  orange_blue_barrier_state = BYTE(map16_load_src_off);
  word_7EC174 = map16_load_dst_off;
  word_7EC176 = 24;
  map16_load_src_off = 0x790;
  map16_load_dst_off = 0;
  map16_load_var2 = 7;
  TriggerAndFinishMapLoadStripe_Y(8);
  map16_load_var2 = (map16_load_var2 + 9) & 0x1f;
  map16_load_src_off -= 0xB80;
}

void CreateInitialOWScreenView_Small_West() {  // 82f185
  orange_blue_barrier_state = map16_load_src_off - 0x20;
  word_7EC174 = 8;
  word_7EC176 = map16_load_var2;
  map16_load_src_off = 0x44e;
  map16_load_var2 = 0;
  map16_load_dst_off = 31;
  TriggerAndFinishMapLoadStripe_X(7);
}

void CreateInitialOWScreenView_Small_East() {  // 82f1b7
  orange_blue_barrier_state = map16_load_src_off - 0x60;
  word_7EC174 = 0x18;
  word_7EC176 = map16_load_var2;
  map16_load_src_off = 0x41e;
  map16_load_var2 = 0;
  map16_load_dst_off = 7;
  TriggerAndFinishMapLoadStripe_X(8);
  map16_load_dst_off = (map16_load_dst_off + 9) & 0x1f;
  map16_load_src_off -= 0x2e;
}

void OverworldTransitionScrollAndLoadMap() {  // 82f20e
  uint16 *dst = uvram.data;
  switch (BYTE(overworld_screen_trans_dir_bits2)) {
  case 1: dst = BuildFullStripeDuringTransition_East(dst); break;
  case 2: dst = BuildFullStripeDuringTransition_West(dst); break;
  case 4: dst = BuildFullStripeDuringTransition_South(dst); break;
  case 8: dst = BuildFullStripeDuringTransition_North(dst); break;
  default:
    assert(0);
    submodule_index = 0;
  }
  dst[0] = dst[1] = 0xffff;
  if (dst != uvram.data)
    nmi_subroutine_index = 3;
}

uint16 *BuildFullStripeDuringTransition_North(uint16 *dst) {  // 82f218
  *dst++ = 0x80;
  dst = BufferAndBuildMap16Stripes_Y(dst);
  map16_load_src_off -= 0x80;
  map16_load_var2 = (map16_load_var2 - 1) & 0x1f;
  return dst;
}

uint16 *BuildFullStripeDuringTransition_South(uint16 *dst) {  // 82f238
  *dst++ = 0x80;
  dst = BufferAndBuildMap16Stripes_Y(dst);
  map16_load_src_off += 0x80;
  map16_load_var2 = (map16_load_var2 + 1) & 0x1f;
  return dst;
}

uint16 *BuildFullStripeDuringTransition_West(uint16 *dst) {  // 82f241
  *dst++ = 0x8040;
  dst = BufferAndBuildMap16Stripes_X(dst);
  map16_load_src_off -= 2;
  map16_load_dst_off = (map16_load_dst_off - 1) & 0x1f;
  return dst;
}

uint16 *BuildFullStripeDuringTransition_East(uint16 *dst) {  // 82f24a
  *dst++ = 0x8040;
  dst = BufferAndBuildMap16Stripes_X(dst);
  map16_load_src_off += 2;
  map16_load_dst_off = (map16_load_dst_off + 1) & 0x1f;
  return dst;
}

void OverworldHandleMapScroll() {  // 82f273
  uint16 *dst = uvram.data;
  switch (BYTE(overworld_screen_trans_dir_bits2)) {
  case 1:
    dst = CheckForNewlyLoadedMapAreas_East(dst);
    BYTE(overworld_screen_trans_dir_bits2) = 0;
    break;
  case 2:
    dst = CheckForNewlyLoadedMapAreas_West(dst);
    BYTE(overworld_screen_trans_dir_bits2) = 0;
    break;
  case 4:
    dst = CheckForNewlyLoadedMapAreas_South(dst);
    BYTE(overworld_screen_trans_dir_bits2) = 0;
    break;
  case 5:
  case 6:
    dst = CheckForNewlyLoadedMapAreas_South(dst);
    BYTE(overworld_screen_trans_dir_bits2) &= 3;
    break;
  case 8:
    dst = CheckForNewlyLoadedMapAreas_North(dst);
    BYTE(overworld_screen_trans_dir_bits2) = 0;
    break;
  case 9:
  case 10:
    dst = CheckForNewlyLoadedMapAreas_North(dst);
    BYTE(overworld_screen_trans_dir_bits2) &= 3;
    break;
  default:
    assert(0);
    submodule_index = 0;
  }
  dst[0] = dst[1] = 0xffff;
  if (dst != uvram.data)
    nmi_subroutine_index = 3;
  overworld_screen_transition = overworld_screen_trans_dir_bits2;
}

uint16 *CheckForNewlyLoadedMapAreas_North(uint16 *dst) {  // 82f2dd
  if (sign16(map16_load_src_off - 0x80))
    return dst;
  if (!kOverworldMapIsSmall[overworld_screen_index]) {
    *dst++ = 0x80;
    dst = BufferAndBuildMap16Stripes_Y(dst);
  }
  map16_load_src_off -= 0x80;
  map16_load_var2 = (map16_load_var2 - 1) & 0x1f;
  return dst;
}

uint16 *CheckForNewlyLoadedMapAreas_South(uint16 *dst) {  // 82f311
  if (map16_load_src_off >= 0x1800)
    return dst;
  if (!kOverworldMapIsSmall[overworld_screen_index]) {
    *dst++ = 0x80;
    dst = BufferAndBuildMap16Stripes_Y(dst);
  }
  map16_load_src_off += 0x80;
  map16_load_var2 = (map16_load_var2 + 1) & 0x1f;
  return dst;
}

uint16 *CheckForNewlyLoadedMapAreas_West(uint16 *dst) {  // 82f345
  uint16 pos = map16_load_src_off;
  while (pos >= 0x80)
    pos -= 0x80;
  if (pos == 0)
    return dst;
  if (!kOverworldMapIsSmall[overworld_screen_index]) {
    *dst++ = 0x8040;
    dst = BufferAndBuildMap16Stripes_X(dst);
  }
  map16_load_src_off -= 2;
  map16_load_dst_off = (map16_load_dst_off - 1) & 0x1f;
  return dst;
}

uint16 *CheckForNewlyLoadedMapAreas_East(uint16 *dst) {  // 82f37f
  uint16 pos = map16_load_src_off;
  while (pos >= 0x80)
    pos -= 0x80;
  if (pos >= 0x60)
    return dst;
  if (!kOverworldMapIsSmall[overworld_screen_index]) {
    *dst++ = 0x8040;
    dst = BufferAndBuildMap16Stripes_X(dst);
  }
  map16_load_src_off += 2;
  map16_load_dst_off = (map16_load_dst_off + 1) & 0x1f;
  return dst;
}

uint16 *BufferAndBuildMap16Stripes_X(uint16 *dst) {  // 82f3b9
  uint16 pos = map16_load_src_off - kOverworld_DrawStrip_Tab[overworld_screen_trans_dir_bits2 >> 1 & 1];
  int d = map16_load_var2;
  uint16 *tmp = dung_replacement_tile_state;
  for (int i = 0; i < 32; i++) {
    tmp[d] = (pos >= 0x2000) ? 0 : dung_bg2[pos >> 1];
    d = (d + 1) & 0x1f, pos += 128;
  }
  const uint16 *map8 = GetMap16toMap8Table();
  uint16 r0 = 0, of = map16_load_dst_off;
  if (of >= 0x10)
    of &= 0xf, r0 = 0x400;
  r0 += of * 2;
  for (int i = 0; i < 2; i++, r0 += 0x800) {
    dst[0] = r0;
    dst[33] = r0 + 1;
    dst++;
    for (int j = 0; j < 16; j++) {
      int k = *tmp++;
      assert(k < 0xea8);
      const uint16 *s = map8 + k * 4;
      dst[0] = s[0];
      dst[33] = s[1];
      dst[1] = s[2];
      dst[34] = s[3];
      dst += 2;
    }
    dst += 33;
  }
  return dst;
}

uint16 *BufferAndBuildMap16Stripes_Y(uint16 *dst) {  // 82f482
  uint16 pos = map16_load_src_off - kOverworld_DrawStrip_Tab[1 + (overworld_screen_trans_dir_bits2 >> 2 & 1)];
  int d = map16_load_dst_off;
  uint16 *tmp = dung_replacement_tile_state;
  for (int i = 0; i < 32; i++) {
    tmp[d] = (pos >= 0x2000) ? 0 : dung_bg2[pos >> 1]; // fixed bug warning. can go negative
    pos += 2;
    d = (d + 1) & 0x1f;
  }
  const uint16 *map8 = GetMap16toMap8Table();
  uint16 r0 = 0, of = map16_load_var2;
  if (of >= 0x10)
    of &= 0xf, r0 = 0x800;
  r0 += of * 64;
  for (int i = 0; i < 2; i++, r0 += 0x400) {
    *dst++ = r0;
    for (int j = 0; j < 16; j++) {
      int k = *tmp++;
      assert(k < 0xea8);
      const uint16 *s = map8 + k * 4;
      dst[0] = s[0];
      dst[32] = s[2];
      dst[1] = s[1];
      dst[33] = s[3];
      dst += 2;
    }
    dst += 32;
  }
  return dst;
}

void Overworld_DecompressAndDrawAllQuadrants() {  // 82f54a
  int si = overworld_screen_index;
  Overworld_DecompressAndDrawOneQuadrant((uint16 *)&g_ram[0x2000], si + 0);
  Overworld_DecompressAndDrawOneQuadrant((uint16 *)&g_ram[0x2040], si + 1);
  Overworld_DecompressAndDrawOneQuadrant((uint16 *)&g_ram[0x3000], si + 8);
  Overworld_DecompressAndDrawOneQuadrant((uint16 *)&g_ram[0x3040], si + 9);
}

void Overworld_DecompressAndDrawOneQuadrant(uint16 *dst, int screen) {  // 82f595
  int rv;
  rv = Decompress_bank02(&g_ram[0x14400], kOverworld_Hibytes_Comp[screen]);
  for (int i = 0; i < 256; i++)
    g_ram[0x14001 + i * 2] = g_ram[0x14400 + i];

  rv = Decompress_bank02(&g_ram[0x14400], kOverworld_Lobytes_Comp[screen]);
  for (int i = 0; i < 256; i++)
    g_ram[0x14000 + i * 2] = g_ram[0x14400 + i];

  map16_decode_last = 0xffff;

  uint16 *src = (uint16 *)&g_ram[0x14000];
  for (int j = 0; j < 16; j++) {
    for (int i = 0; i < 16; i++) {
      Overworld_ParseMap32Definition(dst, *src++ * 2);
      dst += 2;
    }
    dst += 96;
  }
}

void Overworld_ParseMap32Definition(uint16 *dst, uint16 input) {  // 82f691
  uint16 a = input & ~7;
  if (a != map16_decode_last) {
    map16_decode_last = a;
    map16_decode_tmp = a >> 1;
    int x = (a >> 1) + (a >> 2);
    const uint8 *ov;
    ov = kMap32ToMap16_0 + x;
    map16_decode_0[0] = ov[0];
    map16_decode_0[2] = ov[1];
    map16_decode_0[4] = ov[2];
    map16_decode_0[6] = ov[3];
    map16_decode_0[1] = ov[4] >> 4;
    map16_decode_0[3] = ov[4] & 0xf;
    map16_decode_0[5] = ov[5] >> 4;
    map16_decode_0[7] = ov[5] & 0xf;
    ov = kMap32ToMap16_1 + x;
    map16_decode_1[0] = ov[0];
    map16_decode_1[2] = ov[1];
    map16_decode_1[4] = ov[2];
    map16_decode_1[6] = ov[3];
    map16_decode_1[1] = ov[4] >> 4;
    map16_decode_1[3] = ov[4] & 0xf;
    map16_decode_1[5] = ov[5] >> 4;
    map16_decode_1[7] = ov[5] & 0xf;
    ov = kMap32ToMap16_2 + x;
    map16_decode_2[0] = ov[0];
    map16_decode_2[2] = ov[1];
    map16_decode_2[4] = ov[2];
    map16_decode_2[6] = ov[3];
    map16_decode_2[1] = ov[4] >> 4;
    map16_decode_2[3] = ov[4] & 0xf;
    map16_decode_2[5] = ov[5] >> 4;
    map16_decode_2[7] = ov[5] & 0xf;
    ov = kMap32ToMap16_3 + x;
    map16_decode_3[0] = ov[0];
    map16_decode_3[2] = ov[1];
    map16_decode_3[4] = ov[2];
    map16_decode_3[6] = ov[3];
    map16_decode_3[1] = ov[4] >> 4;
    map16_decode_3[3] = ov[4] & 0xf;
    map16_decode_3[5] = ov[5] >> 4;
    map16_decode_3[7] = ov[5] & 0xf;
  }
  dst[0] = WORD(map16_decode_0[input & 7]);
  dst[64] = WORD(map16_decode_2[input & 7]);
  dst[1] = WORD(map16_decode_1[input & 7]);
  dst[65] = WORD(map16_decode_3[input & 7]);
}

void OverworldLoad_LoadSubOverlayMap32() {  // 82f7cb
  int si = overworld_screen_index;
  Overworld_DecompressAndDrawOneQuadrant((uint16 *)&g_ram[0x4000], si);
}

void LoadOverworldOverlay() {  // 82fd0d
  OverworldLoad_LoadSubOverlayMap32();
  Map16ToMap8(&g_ram[0x4000], 0x1000);
  nmi_subroutine_index = nmi_disable_core_updates = 4;
  submodule_index++;
}

void Map16ToMap8(const uint8 *src, int r20) {  // 82fd46
  map16_load_src_off += 0x1000;
  int n = 32;
  int r14 = 0;
  uint16 *r10 = &word_7F4000;
  do {
    OverworldCopyMap16ToBuffer(src, r20, r14, r10);
    r14 += 0x100, r10 += 2;
    map16_load_src_off -= 0x80;
    map16_load_var2 = (map16_load_var2 - 1) & 0x1f;
  } while (--n);
}

void OverworldCopyMap16ToBuffer(const uint8 *src, uint16 r20, int r14, uint16 *r10) {  // 82fd87
  const uint16 *map8 = GetMap16toMap8Table();

  int yr = map16_load_src_off - 0x410 & 0x1fff;
  int xr = map16_load_dst_off;
  uint16 *tmp = (uint16 *)(g_ram + 0x500);
  int n = 32;
  do {
    WORD(tmp[xr]) = WORD(src[yr]);
    xr = (xr + 1) & 0x1f;
    yr = (yr + 2) & 0x1fff;
  } while (--n);

  uint16 r0 = 0, of = map16_load_var2;
  if (of >= 0x10)
    of &= 0xf, r0 = 0x800;
  r0 += of * 64;

  for (int i = 0; i < 2; i++, r0 += 0x400, r14 += 0x40) {
    *r10++ = r0 | r20;
    for (int j = 0; j < 16; j++, r14 += 4) {
      const uint16 *m = map8 + 4 * *tmp++;
      WORD(dung_bg2_attr_table[r14]) = WORD(m[0]);
      WORD(dung_bg2_attr_table[r14 + 64]) = WORD(m[2]);
      WORD(dung_bg2_attr_table[r14 + 2]) = WORD(m[1]);
      WORD(dung_bg2_attr_table[r14 + 66]) = WORD(m[3]);
    }
  }

}

void MirrorBonk_RecoverChangedTiles() {  // 82fe47
  for (int i = 0, i_end = num_memorized_tiles >> 1; i != i_end; i++) {
    uint16 pos = memorized_tile_addr[i];
    dung_bg2[pos >> 1] = memorized_tile_value[i];
  }
}

void DecompressEnemyDamageSubclasses() {  // 82fe71
  uint8 *tmp = &g_ram[0x14000];
  memcpy(tmp, kEnemyDamageData, sizeof(kEnemyDamageData));
  for (int i = 0; i < 0x1000; i += 2) {
    uint8 t = *tmp++;
    enemy_damage_data[i + 0] = t >> 4;
    enemy_damage_data[i + 1] = t & 0xf;
  }
}

int Decompress_bank02(uint8 *dst, const uint8 *src) {  // 82febb
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
      uint32 offs = *src++ << 8;
      offs |= *src++;
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

uint8 Overworld_ReadTileAttribute(uint16 x, uint16 y) {  // 85faa2
  int t = ((x - overworld_offset_base_x) & overworld_offset_mask_x);
  t |= ((y - overworld_offset_base_y) & overworld_offset_mask_y) << 3;
  return kSomeTileAttr[dung_bg2[t >> 1]];
}

void Overworld_SetFixedColAndScroll() {  // 8bfe70
  TS_copy = 0;
  uint16 p = 0x19C6;
  uint16 si = overworld_screen_index;
  if (si == 0x80) {
    if (dungeon_room_index == 0x181) {
      TS_copy = 1;
      p = si & 0x40 ? 0x2A32 : 0x2669;
    }
  } else if (si != 0x81) {
    p = 0;
    if (si != 0x5b && (si & 0xbf) != 3 && (si & 0xbf) != 5 && (si & 0xbf) != 7)
      p = si & 0x40 ? 0x2A32 : 0x2669;
  }
  main_palette_buffer[0] = p;
  aux_palette_buffer[0] = p;
  main_palette_buffer[32] = p;
  aux_palette_buffer[32] = p;

  COLDATA_copy0 = 0x20;
  COLDATA_copy1 = 0x40;
  COLDATA_copy2 = 0x80;

  uint32 cv;

  if (si != 0 && si != 0x40 && si != 0x5b) {
    if (si == 0x70)
      goto getout;
    cv = 0x8c4c26;
    if (si != 3 && si != 5 && si != 7) {
      cv = 0x874a26;
      if (si != 0x43 && si != 0x45) {
        flag_update_cgram_in_nmi += 1;
        return;
      }
    }
    COLDATA_copy0 = (uint8)(cv);
    COLDATA_copy1 = (uint8)(cv >> 8);
    COLDATA_copy2 = (uint8)(cv >> 16);
  }

  if (submodule_index != 4) {
    BG1VOFS_copy2 = BG2VOFS_copy2;
    BG1HOFS_copy2 = BG2HOFS_copy2;
    if ((si & 0x3f) == 0x1b) {
      int16 y = (int16)(BG2HOFS_copy2 - 0x778) >> 1;
      BG1HOFS_copy2 = BG2HOFS_copy2 - y;
      uint16 a = BG1VOFS_copy2;
      if (a >= 0x6C0) {
        a = (a - 0x600) & 0x3ff;
        BG1VOFS_copy2 = (a < 0x180) ? (a >> 1) | 0x600 : 0x6c0;
      } else {
        BG1VOFS_copy2 = (a & 0xff) >> 1 | 0x600;
      }
    }
  } else {
    if ((si & 0x3f) == 0x1b) {
      BG1HOFS_copy2 = (BYTE(overworld_screen_trans_dir_bits) != 8) ? 0x838 : BG2HOFS_copy2;
      BG1VOFS_copy2 = 0x6c0;
    }
  }
getout:
  TS_copy = 1;
  flag_update_cgram_in_nmi++;
}

void Overworld_Memorize_Map16_Change(uint16 pos, uint16 value) {  // 8edd40
  if (value == 0xdc5 || value == 0xdc9)
    return;

  int x = num_memorized_tiles;
  memorized_tile_value[x >> 1] = value;
  memorized_tile_addr[x >> 1] = pos;
  num_memorized_tiles = x + 2;
}

void HandlePegPuzzles(uint16 pos) {  // 8edd67
  static const uint16 kLwTurtleRockPegPositions[3] = { 0x826, 0x5a0, 0x81a };

  if (overworld_screen_index == 7) {
    if (save_ow_event_info[7] & 0x20)
      return;
    if (word_7E04C8 != 0xffff && kLwTurtleRockPegPositions[word_7E04C8 >> 1] == pos) {
      WORD(sound_effect_1) = 0x2d00;
      word_7E04C8 += 2;
      if (word_7E04C8 == 6) {
        WORD(sound_effect_1) = 0x1b00;
        save_ow_event_info[7] |= 0x20;
        submodule_index = 47;
      }
    } else {
      WORD(sound_effect_1) = 0x3c;
      word_7E04C8 = 0xffff;
    }
  } else if (overworld_screen_index == 98) {
    if (++word_7E04C8 == 22) {
      save_ow_event_info[0x62] |= 0x20;
      sound_effect_2 = 27;
      door_open_closed_counter = 0x50;
      big_rock_starting_address = 0xd20;
      Overworld_DoMapUpdate32x32_B();
    }
  }
  //assert(0);
}

void GanonTowerEntrance_Func1() {  // 8eddfc
  if (!subsubmodule_index) {
    sound_effect_1 = 0x2e;
    Palette_AnimGetMasterSword2();
  } else {
    PaletteFilter_BlindingWhite();
    if (darkening_or_lightening_screen == 255) {
      palette_filter_countdown = 255;
      subsubmodule_index++;
    } else {
      Palette_AnimGetMasterSword3();
    }
  }
}

void Overworld_CheckSpecialSwitchArea() {  // 8ede49
  const uint16 *map8 = Overworld_GetMap16OfLink_Mult8();
  int a = map8[0] & 0x1ff;
  for (int i = 3; i >= 0; i--) {
    if (kSpecialSwitchArea_Map8[i] == a && kSpecialSwitchArea_Screen[i] == overworld_screen_index) {
      dungeon_room_index = kSpecialSwitchArea_Exit[i];
      BYTE(overworld_screen_trans_dir_bits) = BYTE(overworld_screen_trans_dir_bits2) = link_direction = kSpecialSwitchArea_Direction[i];
      WORD(byte_7E069C) = WORD(overworld_screen_transition) = DirToEnum(link_direction);
      submodule_index = 23;
      main_module_index = 11;
      break;
    }
  }
}

const uint16 *Overworld_GetMap16OfLink_Mult8() {  // 8ede9a
  const uint16 *map8 = GetMap16toMap8Table();
  uint16 xc = (link_x_coord + 8) >> 3, yc = link_y_coord + 12;
  uint16 pos = ((yc - overworld_offset_base_y) & overworld_offset_mask_y) * 8 +
               ((xc - overworld_offset_base_x) & overworld_offset_mask_x);
  return map8 + dung_bg2[pos >> 1] * 4;
}

void Palette_AnimGetMasterSword() {  // 8ef400
  if (subsubmodule_index == 0) {
    Palette_AnimGetMasterSword2();
  } else {
    PaletteFilter_BlindingWhite();
    if (darkening_or_lightening_screen == 0xff) {
      for (int i = 0; i < 8; i++)
        main_palette_buffer[0x58 + i] = aux_palette_buffer[0x58 + i] = 0;
      palette_filter_countdown = 0;
      darkening_or_lightening_screen = 0;
      submodule_index = 0;
    } else {
      Palette_AnimGetMasterSword3();
    }
  }
}

void Palette_AnimGetMasterSword2() {  // 8ef404
  memcpy(mapbak_palette, aux_palette_buffer, 512);
  for (int i = 0; i < 256; i++)
    aux_palette_buffer[i] = 0x7fff;
  main_palette_buffer[32] = main_palette_buffer[0];
  palette_filter_countdown = 0;
  darkening_or_lightening_screen = 2;
  subsubmodule_index++;
}

void Palette_AnimGetMasterSword3() {  // 8ef48c
  if (darkening_or_lightening_screen != 0 || palette_filter_countdown != 31)
    return;
  memcpy(aux_palette_buffer, mapbak_palette, 512);
  TS_copy = 0;
}

void Overworld_DwDeathMountainPaletteAnimation() {  // 8ef582
  if (trigger_special_entrance)
    return;
  uint8 sc = overworld_screen_index;
  if (sc != 0x43 && sc != 0x45 && sc != 0x47)
    return;
  uint8 fc = frame_counter;
  if (fc == 5 || fc == 44 || fc == 90) {
    for (int i = 1; i < 8; i++) {
      main_palette_buffer[0x30 + i] = aux_palette_buffer[0x30 + i];
      main_palette_buffer[0x38 + i] = aux_palette_buffer[0x38 + i];
      main_palette_buffer[0x48 + i] = aux_palette_buffer[0x48 + i];
      main_palette_buffer[0x70 + i] = aux_palette_buffer[0x70 + i];
      main_palette_buffer[0x78 + i] = aux_palette_buffer[0x78 + i];
    }
  } else if (fc == 3 || fc == 36 || fc == 88) {
    if (fc == 36)
      sound_effect_1 = 54;
    for (int i = 1; i < 8; i++) {
      main_palette_buffer[0x30 + i] = kDwPaletteAnim[i - 1 + 0];
      main_palette_buffer[0x38 + i] = kDwPaletteAnim[i - 1 + 7];
      main_palette_buffer[0x48 + i] = kDwPaletteAnim[i - 1 + 14];
      main_palette_buffer[0x70 + i] = kDwPaletteAnim[i - 1 + 21];
      main_palette_buffer[0x78 + i] = kDwPaletteAnim[i - 1 + 28];
    }
  }
  flag_update_cgram_in_nmi++;
  int yy = 32;
  if (sc == 0x43 || sc == 0x45) {
    if (save_ow_event_info[0x43] & 0x20)
      return;
    yy = (frame_counter & 0xc) * 2;
  }
  for (int i = 0; i < 8; i++)
    main_palette_buffer[0x68 + i] = kDwPaletteAnim2[yy + i];
}

void Overworld_LoadEventOverlay() {  // 8ef652
  int x;
  uint16 *dst = dung_bg2;
  switch (overworld_screen_index) {
  case 0: case 1: case 2:
    dst[XY(11, 16)] = 0xe32;
    dst[XY(12, 16)] = 0xe32;
    dst[XY(13, 16)] = 0xe32;
    dst[XY(14, 16)] = 0xe32;
    dst[XY(11, 17)] = 0xe32;
    dst[XY(14, 17)] = 0xe32;
    dst[XY(12, 17)] = 0xe33;
    dst[XY(13, 17)] = 0xe34;
    dst[XY(11, 18)] = 0xe35;
    dst[XY(12, 18)] = 0xe36;
    dst[XY(13, 18)] = 0xe37;
    dst[XY(14, 18)] = 0xe38;
    dst[XY(11, 19)] = 0xe39;
    dst[XY(12, 19)] = 0xe3a;
    dst[XY(13, 19)] = 0xe3b;
    dst[XY(14, 19)] = 0xe3c;
    dst[XY(12, 20)] = 0xe3d;
    dst[XY(13, 20)] = 0xe3e;
    break;
  case 3: case 4: case 5: case 6: case 7:
    dst[XY(16, 14)] = 0x212;
    break;
  case 8: case 9: case 10: case 11: case 12: case 13: case 14: case 15: case 16: case 17: case 18: case 19:
    x = XY(3, 10);
loc_8EF7B4:
    dst[x + XY(0, 0)] = 0x918;
    dst[x + XY(1, 0)] = 0x919;
    dst[x + XY(0, 1)] = 0x91a;
    dst[x + XY(1, 1)] = 0x91b;
    break;
  case 20:
    dst[XY(25, 10)] = 0xdd1;
    dst[XY(26, 10)] = 0xdd2;
    dst[XY(25, 11)] = 0xdd7;
    dst[XY(26, 11)] = 0xdd8;
    dst[XY(25, 12)] = 0xdd9;
    dst[XY(26, 12)] = 0xdda;
    break;
  case 21: case 22: case 23: case 24: case 25: case 32: case 33:
    dst[XY(31, 24)] = 0xe21;
    dst[XY(33, 24)] = 0xe21;
    dst[XY(32, 24)] = 0xe22;
    dst[XY(31, 25)] = 0xe23;
    dst[XY(32, 25)] = 0xe24;
    dst[XY(33, 25)] = 0xe25;
    break;
  case 26: case 27: case 28: case 35: case 36:
    dst[XY(30, 39)] = 0xdc1;
    dst[XY(31, 39)] = 0xdc2;
    dst[XY(30, 40)] = 0xdbe;
    dst[XY(31, 40)] = 0xdbf;
    dst[XY(32, 39)] = 0xdc2;
    dst[XY(33, 39)] = 0xdc3;
    dst[XY(32, 40)] = 0xdbf;
    dst[XY(33, 40)] = 0xdc0;
    break;
  case 29: case 30: case 31: case 34: case 37: case 38: case 39: case 40: case 41: case 42: case 43: case 107:
    x = XY(24, 6);
    goto loc_8EF7B4;
  case 44: case 45: case 46: case 47: case 48: case 49: case 56: case 57:
    x = XY(44, 6);
    goto loc_8EF7B4;
  case 50: case 51: case 52: case 53: case 54: case 55: case 119:
    x = XY(6, 8);
    goto loc_8EF7B4;
  case 58:
    x = XY(15, 20);
    goto loc_8EF7B4;
  case 59: case 123:
    dst[XY(22, 7)] = 0xddf;
    dst[XY(18, 8)] = 0xddf;
    dst[XY(16, 9)] = 0xddf;
    dst[XY(15, 10)] = 0xddf;
    dst[XY(14, 12)] = 0xddf;
    dst[XY(26, 14)] = 0xddf;
    dst[XY(23, 7)] = 0xde0;
    dst[XY(17, 9)] = 0xde0;
    dst[XY(24, 7)] = 0xde1;
    dst[XY(28, 8)] = 0xde1;
    dst[XY(29, 9)] = 0xde1;
    dst[XY(21, 11)] = 0xde1;
    dst[XY(29, 14)] = 0xde1;
    dst[XY(19, 8)] = 0xde2;
    dst[XY(20, 8)] = 0xde2;
    dst[XY(21, 8)] = 0xde2;
    dst[XY(25, 8)] = 0xde2;
    dst[XY(26, 8)] = 0xde2;
    dst[XY(27, 8)] = 0xde2;
    dst[XY(22, 8)] = 0xde3;
    dst[XY(18, 9)] = 0xde3;
    dst[XY(16, 10)] = 0xde3;
    dst[XY(15, 12)] = 0xde3;
    dst[XY(23, 8)] = 0xde4;
    dst[XY(19, 9)] = 0xde4;
    dst[XY(20, 9)] = 0xde4;
    dst[XY(24, 9)] = 0xde4;
    dst[XY(27, 9)] = 0xde4;
    dst[XY(17, 10)] = 0xde4;
    dst[XY(18, 10)] = 0xde4;
    dst[XY(19, 10)] = 0xde4;
    dst[XY(28, 10)] = 0xde4;
    dst[XY(16, 11)] = 0xde4;
    dst[XY(17, 11)] = 0xde4;
    dst[XY(18, 11)] = 0xde4;
    dst[XY(19, 11)] = 0xde4;
    dst[XY(16, 12)] = 0xde4;
    dst[XY(17, 12)] = 0xde4;
    dst[XY(15, 13)] = 0xde4;
    dst[XY(16, 13)] = 0xde4;
    dst[XY(15, 14)] = 0xde4;
    dst[XY(16, 14)] = 0xde4;
    dst[XY(19, 16)] = 0xde4;
    dst[XY(19, 17)] = 0xde4;
    dst[XY(20, 17)] = 0xde4;
    dst[XY(19, 18)] = 0xde4;
    dst[XY(24, 8)] = 0xde5;
    dst[XY(28, 9)] = 0xde5;
    dst[XY(20, 11)] = 0xde5;
    dst[XY(21, 12)] = 0xde5;
    dst[XY(21, 9)] = 0xde6;
    dst[XY(25, 9)] = 0xde6;
    dst[XY(20, 10)] = 0xde6;
    dst[XY(28, 11)] = 0xde6;
    dst[XY(21, 17)] = 0xde6;
    dst[XY(20, 18)] = 0xde6;
    dst[XY(22, 9)] = 0xde7;
    dst[XY(24, 10)] = 0xde7;
    dst[XY(15, 15)] = 0xde7;
    dst[XY(16, 15)] = 0xde7;
    dst[XY(19, 19)] = 0xde7;
    dst[XY(28, 19)] = 0xde7;
    dst[XY(23, 9)] = 0xde8;
    dst[XY(26, 9)] = 0xde8;
    dst[XY(27, 10)] = 0xde8;
    dst[XY(17, 15)] = 0xde8;
    dst[XY(18, 16)] = 0xde8;
    dst[XY(23, 10)] = 0xde9;
    dst[XY(26, 10)] = 0xde9;
    dst[XY(14, 15)] = 0xde9;
    dst[XY(17, 16)] = 0xde9;
    dst[XY(26, 18)] = 0xde9;
    dst[XY(27, 19)] = 0xde9;
    dst[XY(29, 10)] = 0xdea;
    dst[XY(28, 12)] = 0xdea;
    dst[XY(28, 13)] = 0xdea;
    dst[XY(29, 18)] = 0xdea;
    dst[XY(15, 11)] = 0xdeb;
    dst[XY(27, 11)] = 0xdeb;
    dst[XY(27, 12)] = 0xdeb;
    dst[XY(14, 13)] = 0xdeb;
    dst[XY(27, 13)] = 0xdeb;
    dst[XY(14, 14)] = 0xdeb;
    dst[XY(18, 17)] = 0xdeb;
    dst[XY(18, 18)] = 0xdeb;
    dst[XY(18, 12)] = 0xdec;
    dst[XY(17, 13)] = 0xdec;
    dst[XY(19, 12)] = 0xded;
    dst[XY(20, 12)] = 0xdee;
    dst[XY(18, 13)] = 0xdef;
    dst[XY(27, 15)] = 0xdef;
    dst[XY(19, 13)] = 0xdf0;
    dst[XY(19, 14)] = 0xdf0;
    dst[XY(20, 14)] = 0xdf0;
    dst[XY(21, 14)] = 0xdf0;
    dst[XY(21, 15)] = 0xdf0;
    dst[XY(27, 16)] = 0xdf0;
    dst[XY(28, 16)] = 0xdf0;
    dst[XY(20, 13)] = 0xdf1;
    dst[XY(28, 15)] = 0xdf1;
    dst[XY(21, 13)] = 0xdf2;
    dst[XY(17, 14)] = 0xdf3;
    dst[XY(18, 15)] = 0xdf3;
    dst[XY(20, 16)] = 0xdf3;
    dst[XY(18, 14)] = 0xdf4;
    dst[XY(19, 15)] = 0xdf5;
    dst[XY(20, 15)] = 0xdf6;
    dst[XY(27, 17)] = 0xdf6;
    dst[XY(26, 15)] = 0xdf7;
    dst[XY(29, 15)] = 0xdf8;
    dst[XY(21, 16)] = 0xdf9;
    dst[XY(26, 16)] = 0xdfa;
    dst[XY(29, 16)] = 0xdfb;
    dst[XY(26, 17)] = 0xdfc;
    dst[XY(28, 17)] = 0xdfd;
    dst[XY(29, 17)] = 0xdfe;
    dst[XY(27, 18)] = 0xdff;
    dst[XY(28, 18)] = 0xe00;
    dst[XY(21, 10)] = 0xe01;
    dst[XY(25, 10)] = 0xe01;
    dst[XY(21, 18)] = 0xe01;
    dst[XY(29, 11)] = 0xe02;
    dst[XY(20, 19)] = 0xe02;
    dst[XY(29, 19)] = 0xe02;
    dst[XY(18, 19)] = 0xe03;
    dst[XY(27, 14)] = 0xe04;
    dst[XY(28, 14)] = 0xe05;
    break;
  case 60: case 61: case 62: case 63: case 64: case 65: case 72: case 73:
    dst[XY(8, 11)] = 0xe13;
    dst[XY(11, 11)] = 0xe14;
    dst[XY(8, 12)] = 0xe15;
    dst[XY(9, 12)] = 0xe16;
    dst[XY(10, 12)] = 0xe17;
    dst[XY(11, 12)] = 0xe18;
    dst[XY(9, 13)] = 0xe19;
    dst[XY(10, 13)] = 0xe1a;
    dst[XY(9, 16)] = 0xe06;
    dst[XY(10, 16)] = 0xe06;
    dst[XY(8, 14)] = 0xe07;
    dst[XY(8, 15)] = 0xe07;
    dst[XY(9, 14)] = 0xe08;
    dst[XY(9, 15)] = 0xe08;
    dst[XY(10, 14)] = 0xe09;
    dst[XY(10, 15)] = 0xe09;
    dst[XY(11, 14)] = 0xe0a;
    dst[XY(11, 15)] = 0xe0a;
    break;
  case 66: case 67: case 68: case 75: case 76:
    dst[XY(47, 8)] = 0xe96;
    dst[XY(48, 8)] = 0xe97;
    dst[XY(47, 9)] = 0xe9c;
    dst[XY(47, 10)] = 0xe9c;
    dst[XY(48, 9)] = 0xe9d;
    dst[XY(48, 10)] = 0xe9d;
    dst[XY(47, 11)] = 0xe9a;
    dst[XY(48, 11)] = 0xe9b;
    break;
  case 69: case 70: case 77: case 78:
    x = XY(52, 16);
    goto loc_8EF7B4;
  case 71:
    dst[XY(15, 19)] = 0xe78;
    dst[XY(16, 19)] = 0xe79;
    dst[XY(17, 19)] = 0xe7a;
    dst[XY(18, 19)] = 0xe7b;
    dst[XY(15, 20)] = 0xe7c;
    dst[XY(16, 20)] = 0xe7d;
    dst[XY(17, 20)] = 0xe7e;
    dst[XY(18, 20)] = 0xe7f;
    dst[XY(15, 21)] = 0xe80;
    dst[XY(16, 21)] = 0xe81;
    dst[XY(17, 21)] = 0xe82;
    dst[XY(18, 21)] = 0xe83;
    dst[XY(15, 22)] = 0xe84;
    dst[XY(16, 22)] = 0xe85;
    dst[XY(17, 22)] = 0xe86;
    dst[XY(18, 22)] = 0xe87;
    break;
  case 74: case 79: case 80: case 81: case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 96: case 97:
    dst[XY(31, 26)] = 0xe1b;
    dst[XY(32, 26)] = 0xe1c;
    dst[XY(31, 27)] = 0xe1d;
    dst[XY(32, 27)] = 0xe1e;
    dst[XY(31, 28)] = 0xe1f;
    dst[XY(32, 28)] = 0xe20;
    break;
  case 90: case 91: case 92: case 99: case 100:
    dst[XY(30, 7)] = 0xe3f;
    dst[XY(31, 7)] = 0xe40;
    dst[XY(32, 7)] = 0xe41;
    dst[XY(30, 8)] = 0xe42;
    dst[XY(31, 8)] = 0xe43;
    dst[XY(32, 8)] = 0xe44;
    dst[XY(30, 9)] = 0xe45;
    dst[XY(31, 9)] = 0xe46;
    dst[XY(32, 9)] = 0xe47;
    break;
  case 93: case 94: case 95: case 102: case 103:
    dst[XY(51, 3)] = 0xe31;
    dst[XY(53, 4)] = 0xe2d;
    dst[XY(53, 5)] = 0xe2e;
    dst[XY(53, 6)] = 0xe2f;
    break;
  case 98:
    x = XY(16, 26);
    goto loc_8EF7B4;
  case 101: case 104: case 105: case 106: case 108: case 109: case 110: case 111: case 112: case 113: case 120: case 121:
    dst[XY(17, 10)] = 0xe64;
    dst[XY(18, 10)] = 0xe65;
    dst[XY(19, 10)] = 0xe66;
    dst[XY(20, 10)] = 0xe67;
    dst[XY(17, 11)] = 0xe68;
    dst[XY(18, 11)] = 0xe69;
    dst[XY(19, 11)] = 0xe6a;
    dst[XY(20, 11)] = 0xe6b;
    dst[XY(17, 12)] = 0xe6c;
    dst[XY(18, 12)] = 0xe6d;
    dst[XY(19, 12)] = 0xe6e;
    dst[XY(20, 12)] = 0xe6f;
    dst[XY(17, 13)] = 0xe70;
    dst[XY(18, 13)] = 0xe71;
    dst[XY(19, 13)] = 0xe72;
    dst[XY(20, 13)] = 0xe73;
    dst[XY(17, 14)] = 0xe74;
    dst[XY(18, 14)] = 0xe75;
    dst[XY(19, 14)] = 0xe76;
    dst[XY(20, 14)] = 0xe77;
    break;
  case 114: case 115: case 116: case 117: case 118: case 122: case 124: case 125: case 126: case 127:
    assert(0);
  }
}

void Ancilla_TerminateWaterfallSplashes() {  // 8ffd3c
  uint8 t = BYTE(overworld_screen_index);
  if (t == 0xf) {
    for (int i = 4; i >= 0; i--) {
      if (ancilla_type[i] == 0x41)
        ancilla_type[i] = 0;
    }
  }
}

void Overworld_GetPitDestination() {  // 9bb860
  uint16 x = (link_x_coord & ~7);
  uint16 y = (link_y_coord & ~7);
  uint16 pos = ((y - overworld_offset_base_y) & overworld_offset_mask_y) << 3;
  pos += (((x >> 3) - overworld_offset_base_x) & overworld_offset_mask_x);


  int i = 36 / 2;
  for (;;) {
    if (kFallHole_Pos[i] == pos && kFallHole_Area[i] == overworld_area_index)
      break;
    if (--i < 0) {
      savegame_is_darkworld = 0;
      // Chris Houlihan's room
      which_entrance = 130;
      byte_7E010F = 0;
      return;
    }
  }
  which_entrance = kFallHole_Entrances[i];
  byte_7E010F = 0;
}

void Overworld_UseEntrance() {  // 9bbbf4
  uint16 xc = link_x_coord >> 3, yc = link_y_coord + 7;
  uint16 pos = ((yc - overworld_offset_base_y) & overworld_offset_mask_y) * 8 +
    ((xc - overworld_offset_base_x) & overworld_offset_mask_x);

  int x = dung_bg2[pos >> 1] * 4;
  const uint16 *map8p = GetMap16toMap8Table();

  if (!link_direction_facing) {
    uint16 a = map8p[x + 1] & 0x41ff;
    if (a == 0xe9)
      goto do_draw;

    if (a == 0x149 || a == 0x169)
      goto is_149_or_169;

    x = dung_bg2[(pos >> 1) + 1] * 4;
    a = map8p[x] & 0x41ff;
    if (a == 0x40e9) {
      pos -= 2;
do_draw:
      Overworld_DrawMap16_Persist(pos + 0, 0xDA4);
      Overworld_DrawMap16_Persist(pos + 2, 0xDA6);
      sound_effect_2 = 21;
      nmi_load_bg_from_vram = 1;
      return;
    }
    if (a == 0x4149 || a == 0x4169) {
      pos -= 2;
is_149_or_169:
      door_open_closed_counter = 0;
      if (a & 0x20) {
        // 0x169
        if ((sram_progress_indicator & 0xf) >= 3)
          goto after;
        door_open_closed_counter = 24;
      }
      big_rock_starting_address = pos - 0x80;
      sound_effect_2 = 21;
      subsubmodule_index = 0;
      BYTE(door_animation_step_indicator) = 0;
      submodule_index = 12;
      return;
    }
  }
after:
  if (!LookupInOwEntranceTab(map8p[x + 2] & 0x1ff, map8p[x + 3] & 0x1ff)) {
    big_key_door_message_triggered = 0;
    return;
  }

  int lx = LookupInOwEntranceTab2(pos);
  if (lx < 0)
    return;

  if (!super_bomb_going_off && (link_pose_for_item == 1 || !CanEnterWithTagalong(kOverworld_Entrance_Id[lx] - 1))) {
    if (!big_key_door_message_triggered) {
      big_key_door_message_triggered = 1;
      dialogue_message_index = 5;
      Main_ShowTextMessage();
    }
  } else {
    which_entrance = kOverworld_Entrance_Id[lx];
    link_auxiliary_state = 0;
    link_incapacitated_timer = 0;
    main_module_index = 15;
    saved_module_for_menu = 6;
    submodule_index = 0;
    subsubmodule_index = 0;
  }
}

uint16 Overworld_ToolAndTileInteraction(uint16 x, uint16 y) {  // 9bbd82
  word_7E04B2 = 0;
  index_of_interacting_tile = 0;
  uint16 pos = ((y - overworld_offset_base_y) & overworld_offset_mask_y) * 8 +
    ((x - overworld_offset_base_x) & overworld_offset_mask_x);
  uint16 attr = overworld_tileattr[pos >> 1], yv;

  if (!(link_item_in_hand & 2)) {
    if (!(link_item_in_hand & 0x40)) {
      if (attr == 0x34 || attr == 0x71 || attr == 0x35 || attr == 0x10d ||
          attr == 0x10f || attr == 0xe1 || attr == 0xe2 || attr == 0xda ||
          attr == 0xf8 || attr == 0x10e) {  // shovelable
        if (link_position_mode != 1)
          return attr;
        if (overworld_screen_index == 0x2a && pos == 0x492)
          word_7E04B2 = pos;
        yv = 0xdc9;
        goto check_secret;
      } else if (attr == 0x37e) {  // isThickGrass
        if (link_position_mode == 1)
          return attr;
        scratch_0 = x * 8 - 8;
        scratch_1 = (y - 8) & ~7;
        index_of_interacting_tile = 3;
        yv = 0xdc5;
        goto check_secret;
      }
    }
    if ((yv = 2, attr == 0x36) || (yv = 4, attr == 0x72a)) {
      // is_bush
      if (link_position_mode != 1) {
        scratch_0 = (x & ~1) * 8;
        scratch_1 = y & ~0xf;
        index_of_interacting_tile = yv;
        yv = (attr == 0x72a) ? 0xdc8 : 0xdc7;
        uint32 result;
check_secret:
        result = Overworld_RevealSecret(pos);
        if (result != 0)
          yv = result;
memoize_getout:
        overworld_tileattr[pos >> 1] = yv;
        Overworld_Memorize_Map16_Change(pos, yv);
        Overworld_DrawMap16(pos, yv);
        nmi_load_bg_from_vram = 1;
      }
      uint16 t;
      t = GetMap16toMap8Table()[attr * 4 + ((y & 8) >> 2) + (x & 1)];
      attr = kMap8DataToTileAttr[t & 0x1ff];
      if (index_of_interacting_tile) {
        Sprite_SpawnImmediatelySmashedTerrain(index_of_interacting_tile, scratch_0, scratch_1);
        AncillaAdd_BushPoof(scratch_0, scratch_1);
      }
      return attr;
    }
    return attr;
  } else {  // else_1
    if (attr == 0x21b) {
      sound_effect_1 = 17;
      HandlePegPuzzles(pos);
      yv = 0xdcb;
      goto memoize_getout;
    } else { // else_3
      Overworld_PickHammerSfx(attr);
      return attr;
    }
  }

  return 0;
}

void Overworld_PickHammerSfx(uint16 a) {  // 9bbf1e
  uint16 attr = kMap8DataToTileAttr[GetMap16toMap8Table()[a * 4] & 0x1ff];
  uint8 y;
  if (attr < 0x50) {
    return;
  } else if (attr < 0x52) {
    y = 26;
  } else if (attr < 0x54) {
    y = 17;
  } else if (attr < 0x58) {
    y = 5;
  } else {
    return;
  }
  sound_effect_1 = y;
}

uint16 Overworld_GetLinkMap16Coords(Point16U *xy) {  // 9bbf64
  uint16 x = (link_x_coord + kGetBestActionToPerformOnTile_x[link_direction_facing >> 1]) & ~0xf;
  uint16 y = (link_y_coord + kGetBestActionToPerformOnTile_y[link_direction_facing >> 1]) & ~0xf;
  xy->x = x;
  xy->y = y;
  uint16 rv = ((y - overworld_offset_base_y) & overworld_offset_mask_y) << 3;
  return rv + (((x >> 3) - overworld_offset_base_x) & overworld_offset_mask_x);
}

uint8 Overworld_HandleLiftableTiles(Point16U *pt_arg) {  // 9bbf9d
  uint16 pos = Overworld_GetLinkMap16Coords(pt_arg);
  Point16U pt = *pt_arg;
  uint16 a = overworld_tileattr[pos >> 1], y;
  if ((y = 0, a == 0x36d) || (y = 1, a == 0x36e) || (y = 2, a == 0x374) || (y = 3, a == 0x375) ||
      (y = 0, a == 0x23b) || (y = 1, a == 0x23c) || (y = 2, a == 0x23d) || (y = 3, a == 0x23e)) {
    return SmashRockPile_fromLift(a, pos, y, pt);
  } else if ((y = 0xdc7, a == 0x36) || (y = 0xdc8, a == 0x72a) || (y = 0xdca, a == 0x20f) || (y = 0xdca, a == 0x239) || (y = 0xdc6, a == 0x101)) {
    return Overworld_LiftingSmallObj(a, pos, y, pt);
  } else {
    uint16 t = a * 4 + (pt.x & 8 ? 2 : 0) + (pt.y & 8 ? 1 : 0);
    return kMap8DataToTileAttr[GetMap16toMap8Table()[t] & 0x1ff];
  }
}

uint8 Overworld_LiftingSmallObj(uint16 a, uint16 pos, uint16 y, Point16U pt) {  // 9bc008
  uint16 secret = Overworld_RevealSecret(pos);
  if (secret != 0)
    y = secret;
  overworld_tileattr[pos >> 1] = y;
  Overworld_Memorize_Map16_Change(pos, y);
  Overworld_DrawMap16(pos, y);
  nmi_load_bg_from_vram = 1;
  uint16 t = a * 4 + (pt.x & 8 ? 2 : 0) + (pt.y & 8 ? 1 : 0);
  return kMap8DataToTileAttr[GetMap16toMap8Table()[t] & 0x1ff];
}

int Overworld_SmashRockPile(bool down_one_tile, Point16U *pt) {  // 9bc055
  uint16 bak = link_y_coord;
  link_y_coord += down_one_tile ? 8 : 0;
  uint16 pos = Overworld_GetLinkMap16Coords(pt);
  link_y_coord = bak;
  uint16 a = dung_bg2[pos >> 1];
  uint8 y = 0;
  if ((y = 0, a == 0x226) || (y = 1, a == 0x227) || (y = 2, a == 0x228) || (y = 3, a == 0x229)) {
    return SmashRockPile_fromLift(a, pos, y, *pt);
  } else if (a == 0x36) {
    return Overworld_LiftingSmallObj(a, pos, 0xDC7, *pt);
  } else {
    return -1;
  }
}

uint8 SmashRockPile_fromLift(uint16 a, uint16 pos, uint16 y, Point16U pt) {  // 9bc09f
  static const int8 kBigRockTab1[] = { 0, -1, -64, -65 };
  static const int8 kBigRockTabY[] = { 0, 0, -64, -64 };
  static const int8 kBigRockTabX[] = { 0, -1, 0, -1 };
  pos = 2 * ((pos >> 1) + kBigRockTab1[y]);
  big_rock_starting_address = pos;
  door_open_closed_counter = 40;

  *(uint16 *)&g_ram[0] = pt.y;
  *(uint16 *)&g_ram[2] = pt.x;

  uint16 secret = Overworld_RevealSecret(pos);
  pt.y = *(uint16 *)&g_ram[0];
  pt.x = *(uint16 *)&g_ram[2];

  if (secret == 0xffff) {
    save_ow_event_info[overworld_screen_index] |= 0x20;
    sound_effect_2 = 27;
    door_open_closed_counter = 80;
  }
  pt.x += kBigRockTabX[y] * 2;
  pt.y += kBigRockTabY[y] * 2;

  Overworld_DoMapUpdate32x32_B(); // WARNING: The original destroys ram[0] and ram[2]

  uint16 t = a * 4 + (pt.x & 8 ? 2 : 0) + (pt.y & 8 ? 1 : 0);
  return kMap8DataToTileAttr[GetMap16toMap8Table()[t] & 0x1ff];
}

void Overworld_BombTiles32x32(uint16 x, uint16 y) {  // 9bc0f8
  x = (x - 23) & ~7;
  y = (y - 20) & ~7;

  for (int yy = 3; yy != 0; yy--, y += 16) {
    for (int xx = 3, xt = x; xx != 0; xx--, xt += 16) {
      Overworld_BombTile(xt, y);
    }
  }
  word_7E0486 = x, word_7E0488 = y;
}

void Overworld_BombTile(int x, int y) {  // 9bc155
  int a, j, k;

  int pos = ((y - overworld_offset_base_y & overworld_offset_mask_y) << 3) +
    ((x >> 3) - overworld_offset_base_x & overworld_offset_mask_x);

  if (savegame_tagalong == 13)
    goto label_a;

  a = dung_bg2[pos >> 1];

  if (a == 0x36) {
    k = 2, j = 0xdc7;
  } else if (a == 0x72a) {
    k = 4, j = 0xdc8;
  } else if (a == 0x37e) {
    k = 3, j = 0xdc5;
  } else {
    goto label_a;
  }
  a = Overworld_RevealSecret(pos);
  if (a == 0)
    a = j;
  dung_bg2[pos >> 1] = a;
  Overworld_Memorize_Map16_Change(pos, a);
  Overworld_DrawMap16(pos, a);

  Sprite_SpawnImmediatelySmashedTerrain(k, x & ~7, y & ~7);
  nmi_load_bg_from_vram = 1;
  return;

label_a:
  a = Overworld_RevealSecret(pos);
  if (a == 0xdb4) {
    dung_bg2[pos >> 1] = a;
    Overworld_Memorize_Map16_Change(pos, a);
    Overworld_DrawMap16(pos, a);

    dung_bg2[(pos >> 1) + 1] = 0xDB5;
    Overworld_Memorize_Map16_Change(pos, 0xDB5);  // wtf
    Overworld_DrawMap16(pos + 2, 0xDB5);
    nmi_load_bg_from_vram = 1;
    save_ow_event_info[overworld_screen_index] |= 2;
  }
}

void Overworld_AlterWeathervane() {  // 9bc21d
  door_open_closed_counter = 0x68;
  big_rock_starting_address = 0xc3e;
  Overworld_DoMapUpdate32x32_B();
  Overworld_DrawMap16_Persist(0xc42, 0xe21);
  Overworld_DrawMap16_Persist(0xcc2, 0xe25);

  save_ow_event_info[0x18] |= 0x20;
  nmi_load_bg_from_vram = 1;
}

void OpenGargoylesDomain() {  // 9bc264
  Overworld_DrawMap16_Persist(0xd3e, 0xe1b);
  Overworld_DrawMap16_Persist(0xd40, 0xe1c);
  Overworld_DrawMap16_Persist(0xdbe, 0xe1d);
  Overworld_DrawMap16_Persist(0xdc0, 0xe1e);
  Overworld_DrawMap16_Persist(0xe3e, 0xe1f);
  Overworld_DrawMap16_Persist(0xe40, 0xe20);
  save_ow_event_info[0x58] |= 0x20;
  sound_effect_2 = 0x1b;
  nmi_load_bg_from_vram = 1;
}

void CreatePyramidHole() {  // 9bc2a7
  Overworld_DrawMap16_Persist(0x3bc, 0xe3f);
  Overworld_DrawMap16_Persist(0x3be, 0xe40);
  Overworld_DrawMap16_Persist(0x3c0, 0xe41);
  Overworld_DrawMap16_Persist(0x43c, 0xe42);
  Overworld_DrawMap16_Persist(0x43e, 0xe43);
  Overworld_DrawMap16_Persist(0x440, 0xe44);
  Overworld_DrawMap16_Persist(0x4bc, 0xe45);
  Overworld_DrawMap16_Persist(0x4be, 0xe46);
  Overworld_DrawMap16_Persist(0x4c0, 0xe47);
  WORD(sound_effect_ambient) = 0x3515;
  save_ow_event_info[0x5b] |= 0x20;
  sound_effect_2 = 3;
  nmi_load_bg_from_vram = 1;
}

// Strange return value in Carry/R14
uint16 Overworld_RevealSecret(uint16 pos) {  // 9bc8a4
  BYTE(dung_secrets_unk1) = 0;

  if (overworld_screen_index >= 0x80) {
fail:
    AdjustSecretForPowder();
    return 0;
  }

  const uint8 *ptr = kOverworldSecrets + kOverworldSecrets_Offs[overworld_screen_index];
  for (;;) {
    uint16 x = *(uint16 *)ptr;
    if (x == 0xffff)
      goto fail;
    if ((x & 0x7fff) == pos)
      break;
    ptr += 3;
  }
  uint8 data = ptr[2];
  if (data && data < 0x80)
    BYTE(dung_secrets_unk1) |= data;
  if (data < 0x80) {
    AdjustSecretForPowder();
    return 0; // carry set
  }

  BYTE(dung_secrets_unk1) = 0xff;
  if (data != 0x84 && !(save_ow_event_info[overworld_screen_index] & 2)) {
    if (overworld_screen_index == 0x5b && savegame_tagalong != 13)
      goto fail;
    sound_effect_2 = 0x1b;
  }
  static const uint16 kTileBelow[4] = { 0xDCC, 0x212, 0xFFFF, 0xDB4 };
  AdjustSecretForPowder();
  return kTileBelow[(data & 0xf) >> 1];

}

void AdjustSecretForPowder() {  // 9bc943
  if (link_item_in_hand & 0x40)
    dung_secrets_unk1 = 4;
}

void Overworld_DrawMap16_Persist(uint16 pos, uint16 value) {  // 9bc97c
  dung_bg2[pos >> 1] = value;
  Overworld_DrawMap16(pos, value);
}

void Overworld_DrawMap16(uint16 pos, uint16 value) {  // 9bc980
  pos = Overworld_FindMap16VRAMAddress(pos);
  uint16 *dst = &vram_upload_data[vram_upload_offset >> 1];
  const uint16 *src = GetMap16toMap8Table() + value * 4;
  dst[0] = swap16(pos);
  dst[1] = 0x300;
  dst[2] = src[0];
  dst[3] = src[1];
  dst[4] = swap16(pos + 0x20);
  dst[5] = 0x300;
  dst[6] = src[2];
  dst[7] = src[3];
  dst[8] = 0xffff;
  vram_upload_offset += 16;
}

void Overworld_AlterTileHardcore(uint16 pos, uint16 value) {  // 9bc9de
  dung_bg2[pos >> 1] = value;
  pos = Overworld_FindMap16VRAMAddress(pos);
  uint16 *dst = &vram_upload_data[vram_upload_offset >> 1];
  const uint16 *src = GetMap16toMap8Table() + value * 4;
  dst[0] = swap16(pos);
  dst[1] = 0x300;
  dst[2] = src[0];
  dst[3] = src[1];
  dst[4] = swap16(pos + 0x20);
  dst[5] = 0x300;
  dst[6] = src[2];
  dst[7] = src[3];
  dst[8] = 0xffff;
  vram_upload_offset += 16;
}

uint16 Overworld_FindMap16VRAMAddress(uint16 addr) {  // 9bca69
  return (((addr & 0x3f) >= 0x20) ? 0x400 : 0) + (((addr & 0xfff) >= 0x800) ? 0x800 : 0) + (addr & 0x1f) + ((addr & 0x780) >> 1);
}

void Overworld_AnimateEntrance() {  // 9bcac4
  uint8 j = trigger_special_entrance;
  flag_is_link_immobilized = j;
  flag_unk1 = j;
  nmi_disable_core_updates = j;
  kOverworld_EntranceSequence[j - 1]();
}

void Overworld_AnimateEntrance_PoD() {  // 9bcade
  switch (subsubmodule_index) {
  case 0:
    if (++overworld_entrance_sequence_counter != 0x40)
      return;
    OverworldEntrance_AdvanceAndBoom();
    save_ow_event_info[0x5e] |= 0x20;
    Overworld_DrawMap16_Persist(0x1e6, 0xe31);
    Overworld_DrawMap16_Persist(0x2ea, 0xe30);
    Overworld_DrawMap16_Persist(0x26a, 0xe26);
    Overworld_DrawMap16_Persist(0x2ea, 0xe27);
    nmi_load_bg_from_vram = 1;
    break;
  case 1:
    if (++overworld_entrance_sequence_counter != 0x20)
      return;
    OverworldEntrance_AdvanceAndBoom();
    Overworld_DrawMap16_Persist(0x26a, 0xe28);
    Overworld_DrawMap16_Persist(0x2ea, 0xe29);
    nmi_load_bg_from_vram = 1;
    break;
  case 2:
    if (++overworld_entrance_sequence_counter != 0x20)
      return;
    OverworldEntrance_AdvanceAndBoom();
    Overworld_DrawMap16_Persist(0x26a, 0xe2a);
    Overworld_DrawMap16_Persist(0x2ea, 0xe2b);
    Overworld_DrawMap16_Persist(0x36a, 0xe2c);
    nmi_load_bg_from_vram = 1;
    break;
  case 3:
    if (++overworld_entrance_sequence_counter != 0x20)
      return;
    OverworldEntrance_AdvanceAndBoom();
    Overworld_DrawMap16_Persist(0x26a, 0xe2d);
    Overworld_DrawMap16_Persist(0x2ea, 0xe2e);
    Overworld_DrawMap16_Persist(0x36a, 0xe2f);
    nmi_load_bg_from_vram = 1;
    break;
  case 4:
    if (++overworld_entrance_sequence_counter != 0x20)
      return;
    OverworldEntrance_PlayJingle();
    break;
  }
}

// Dark Forest Palace
void Overworld_AnimateEntrance_Skull() {  // 9bcba6
  switch (subsubmodule_index) {
  case 0:
    if (++overworld_entrance_sequence_counter != 4)
      return;
    overworld_entrance_sequence_counter = 0;
    subsubmodule_index++;
    Overworld_DrawMap16_Persist(0x409 * 2, 0xe06);
    Overworld_DrawMap16_Persist(0x40a * 2, 0xe06);
    save_ow_event_info[BYTE(overworld_screen_index)] |= 0x20;
    nmi_load_bg_from_vram = 1;
    sound_effect_2 = 0x16;
    break;
  case 1:
    if (++overworld_entrance_sequence_counter != 12)
      return;
    overworld_entrance_sequence_counter = 0;
    subsubmodule_index++;
    Overworld_DrawMap16_Persist(0x3c8 * 2, 0xe07);
    Overworld_DrawMap16_Persist(0x3c9 * 2, 0xe08);
    Overworld_DrawMap16_Persist(0x3ca * 2, 0xe09);
    Overworld_DrawMap16_Persist(0x3cb * 2, 0xe0a);
    nmi_load_bg_from_vram = 1;
    sound_effect_2 = 0x16;
    break;
  case 2:
    if (++overworld_entrance_sequence_counter != 12)
      return;
    overworld_entrance_sequence_counter = 0;
    subsubmodule_index++;
    Overworld_DrawMap16_Persist(0x388 * 2, 0xe07);
    Overworld_DrawMap16_Persist(0x389 * 2, 0xe08);
    Overworld_DrawMap16_Persist(0x38a * 2, 0xe09);
    Overworld_DrawMap16_Persist(0x38b * 2, 0xe0a);
    nmi_load_bg_from_vram = 1;
    sound_effect_2 = 0x16;
    break;
  case 3:
    if (++overworld_entrance_sequence_counter != 12)
      return;
    overworld_entrance_sequence_counter = 0;
    subsubmodule_index++;
    Overworld_DrawMap16_Persist(0x2c8 * 2, 0xe11);
    Overworld_DrawMap16_Persist(0x2cb * 2, 0xe12);
    Overworld_DrawMap16_Persist(0x308 * 2, 0xe0d);
    Overworld_DrawMap16_Persist(0x309 * 2, 0xe0e);
    Overworld_DrawMap16_Persist(0x30a * 2, 0xe0f);
    Overworld_DrawMap16_Persist(0x30b * 2, 0xe10);
    Overworld_DrawMap16_Persist(0x349 * 2, 0xe0b);
    Overworld_DrawMap16_Persist(0x34a * 2, 0xe0c);
    nmi_load_bg_from_vram = 1;
    sound_effect_2 = 0x16;
    break;
  case 4:
    if (++overworld_entrance_sequence_counter != 12)
      return;
    overworld_entrance_sequence_counter = 0;
    subsubmodule_index++;
    Overworld_DrawMap16_Persist(0x2c8 * 2, 0xe13);
    Overworld_DrawMap16_Persist(0x2cb * 2, 0xe14);
    Overworld_DrawMap16_Persist(0x308 * 2, 0xe15);
    Overworld_DrawMap16_Persist(0x309 * 2, 0xe16);
    Overworld_DrawMap16_Persist(0x30a * 2, 0xe17);
    Overworld_DrawMap16_Persist(0x30b * 2, 0xe18);
    Overworld_DrawMap16_Persist(0x349 * 2, 0xe19);
    Overworld_DrawMap16_Persist(0x34a * 2, 0xe1a);
    nmi_load_bg_from_vram = 1;
    sound_effect_2 = 0x16;
    OverworldEntrance_PlayJingle();
    break;
  }
}

void Overworld_AnimateEntrance_Mire() {  // 9bccd4
  static const uint8 kMiseryMireEntranceBits[26] = {
    0xff, 0xf7, 0xf7, 0xfb, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
    0xaa, 0x88, 0x88, 0x88, 0x88, 0x80, 0x80, 0x80, 0x80, 0x80,
  };

  int j;
  if (subsubmodule_index >= 2) {
    bg1_x_offset = frame_counter & 1 ? -1 : 1;
    bg1_y_offset = -bg1_x_offset;
  }
  switch (subsubmodule_index) {
  case 0:
    if ((j = ++overworld_entrance_sequence_counter) < 32)
      break;
    j -= 32;
    if (j == 207)
      subsubmodule_index = 1, overworld_entrance_sequence_counter = 0;
    TS_copy = (kMiseryMireEntranceBits[j >> 3] & (0x80 >> (j & 7))) != 0;
    break;
  case 1: case 2:
    if ((j = ++overworld_entrance_sequence_counter) == 16) {
      subsubmodule_index++;
      sound_effect_ambient = 7;
    }
    if (j != 72)
      break;
    OverworldEntrance_AdvanceAndBoom();
    save_ow_event_info[BYTE(overworld_screen_index)] |= 0x20;
    j = 0xe48;
draw_misery_2:
    Overworld_DrawMap16_Persist(0x622, j++);
    Overworld_DrawMap16_Persist(0x624, j++);
    Overworld_DrawMap16_Persist(0x626, j++);
    Overworld_DrawMap16_Persist(0x628, j++);
    Overworld_DrawMap16_Persist(0x6a2, j++);
    Overworld_DrawMap16_Persist(0x6a4, j++);
    Overworld_DrawMap16_Persist(0x6a6, j++);
    Overworld_DrawMap16_Persist(0x6a8, j++);
    Overworld_DrawMap16_Persist(0x722, j++);
    Overworld_DrawMap16_Persist(0x724, j++);
    Overworld_DrawMap16_Persist(0x726, j++);
    Overworld_DrawMap16_Persist(0x728, j++);
    nmi_load_bg_from_vram = 1;
    break;
  case 3:
    if ((j = ++overworld_entrance_sequence_counter) != 72)
      break;
    OverworldEntrance_AdvanceAndBoom();
    j = 0xe54;
draw_misery_3:
    Overworld_DrawMap16_Persist(0x5a2, j++);
    Overworld_DrawMap16_Persist(0x5a4, j++);
    Overworld_DrawMap16_Persist(0x5a6, j++);
    Overworld_DrawMap16_Persist(0x5a8, j++);
    goto draw_misery_2;
  case 4:
    if ((j = ++overworld_entrance_sequence_counter) != 80)
      break;
    OverworldEntrance_AdvanceAndBoom();
    j = 0xe64;
    Overworld_DrawMap16_Persist(0x522, j++);
    Overworld_DrawMap16_Persist(0x524, j++);
    Overworld_DrawMap16_Persist(0x526, j++);
    Overworld_DrawMap16_Persist(0x528, j++);
    goto draw_misery_3;
  case 5:
    if ((j = ++overworld_entrance_sequence_counter) != 128)
      break;
    OverworldEntrance_PlayJingle();
    sound_effect_ambient = 5;
    break;
  }
}

void Overworld_AnimateEntrance_TurtleRock() {  // 9bce28
  bg1_x_offset = frame_counter & 1 ? -1 : 1;
  bg1_y_offset = -bg1_x_offset;

  switch (subsubmodule_index) {
  case 0:
    save_ow_event_info[overworld_screen_index] |= 0x20;
    Dungeon_ApproachFixedColor_variable(0);
    vram_upload_data[0] = 0x10;
common:
    vram_upload_data[1] = 0xfe47;
    vram_upload_data[2] = 0x1e3;
    BYTE(vram_upload_data[3]) = 0xff;
    subsubmodule_index++;
    nmi_load_bg_from_vram = 1;
    break;
  case 1:
    vram_upload_data[0] = 0x14;
    goto common;
  case 2:
    vram_upload_data[0] = 0x18;
    goto common;
  case 3:
    vram_upload_data[0] = 0x1c;
    goto common;
  case 4:
    for (int i = 0; i < 8; i++)
      main_palette_buffer[0x58 + i] = aux_palette_buffer[0x68 + i] = 0;
    BG1VOFS_copy2 = BG2VOFS_copy2;
    BG1HOFS_copy2 = BG2HOFS_copy2;
    subsubmodule_index++;
    flag_update_cgram_in_nmi++;
    break;
  case 5: {
    OverworldEntrance_DrawManyTR();
    TS_copy = 1;
    CGWSEL_copy = 2;
    CGADSUB_copy = 0x22;
    uint16 *vram = vram_upload_data, *vram_end = vram + (vram_upload_offset >> 1);
    do {
      vram[0] |= 0x10;
      if (vram[2] == 0x8aa)
        vram[2] = 0x1e3;
      if (vram[3] == 0x8aa)
        vram[3] = 0x1e3;
    } while ((vram += 4) != vram_end);
    turtlerock_ctr = 0;
    subsubmodule_index++;
    break;
  }
  case 6:
    if (!(frame_counter & 1)) {
      if (!(turtlerock_ctr & 7)) {
        PaletteFilter_RestoreAdditive(0xb0, 0xc0);
        PaletteFilter_RestoreSubtractive(0xd0, 0xe0);
        flag_update_cgram_in_nmi++;
        sound_effect_2 = 2;
      }
      if (!--turtlerock_ctr) {
        turtlerock_ctr = 0x30;
        subsubmodule_index++;
      }
    }
    break;
  case 7:
    if (!(frame_counter & 1) && !(turtlerock_ctr & 7))
      sound_effect_2 = 2;
    if (!--turtlerock_ctr) {
      OverworldEntrance_DrawManyTR();
      TS_copy = 0;
      CGWSEL_copy = 0x82;
      CGADSUB_copy = 0x20;
      subsubmodule_index++;
      sound_effect_ambient = 5;
    }
    break;
  case 8:
    OverworldEntrance_PlayJingle();
    break;
  }
}

void OverworldEntrance_PlayJingle() {  // 9bcf40
  sound_effect_2 = 27;
  trigger_special_entrance = 0;
  subsubmodule_index = 0;
  nmi_disable_core_updates = 0;
  flag_is_link_immobilized = 0;
  flag_unk1 = 0;
  bg1_x_offset = 0;
  bg1_y_offset = 0;
}

void OverworldEntrance_DrawManyTR() {  // 9bcf60
  int j = 0xe78;
  Overworld_DrawMap16_Persist(0x99e, j++);
  Overworld_DrawMap16_Persist(0x9a0, j++);
  Overworld_DrawMap16_Persist(0x9a2, j++);
  Overworld_DrawMap16_Persist(0x9a4, j++);

  Overworld_DrawMap16_Persist(0xa1e, j++);
  Overworld_DrawMap16_Persist(0xa20, j++);
  Overworld_DrawMap16_Persist(0xa22, j++);
  Overworld_DrawMap16_Persist(0xa24, j++);

  Overworld_DrawMap16_Persist(0xa9e, j++);
  Overworld_DrawMap16_Persist(0xaa0, j++);
  Overworld_DrawMap16_Persist(0xaa2, j++);
  Overworld_DrawMap16_Persist(0xaa4, j++);

  Overworld_DrawMap16_Persist(0xb1e, j++);
  Overworld_DrawMap16_Persist(0xb20, j++);
  Overworld_DrawMap16_Persist(0xb22, j++);
  Overworld_DrawMap16_Persist(0xb24, j++);
  nmi_load_bg_from_vram = 1;
  nmi_disable_core_updates = 1;
}

void Overworld_AnimateEntrance_GanonsTower() {  // 9bcfd9
  switch (subsubmodule_index) {
  case 0:
  case 1:
    save_ow_event_info[BYTE(overworld_screen_index)] |= 0x20;
    GanonTowerEntrance_Func1();
    break;
  case 2:
    GanonTowerEntrance_Func1();
    if (!TS_copy) {
      TS_copy = 1;
      if (++ganonentrance_ctr == 3) {
        ganonentrance_ctr = 0;
        sound_effect_ambient = 7;
      } else {
        subsubmodule_index = 0;
      }
    }
    break;
  case 3:
    if (++ganonentrance_ctr != 48)
      return;
    OverworldEntrance_AdvanceAndBoom();
    Overworld_DrawMap16_Persist(0x45e, 0xe88);
    Overworld_DrawMap16_Persist(0x460, 0xe89);
    Overworld_DrawMap16_Persist(0x4de, 0xea2);
    Overworld_DrawMap16_Persist(0x4e0, 0xea3);
    Overworld_DrawMap16_Persist(0x55e, 0xe8a);
    Overworld_DrawMap16_Persist(0x560, 0xe8b);
    nmi_load_bg_from_vram = 1;
    break;
  case 4:
    if (++ganonentrance_ctr != 48)
      return;
    OverworldEntrance_AdvanceAndBoom();
    Overworld_DrawMap16_Persist(0x45e, 0xe8c);
    Overworld_DrawMap16_Persist(0x460, 0xe8d);
    Overworld_DrawMap16_Persist(0x4de, 0xe8e);
    Overworld_DrawMap16_Persist(0x4e0, 0xe8f);
    Overworld_DrawMap16_Persist(0x55e, 0xe90);
    Overworld_DrawMap16_Persist(0x560, 0xe91);
    nmi_load_bg_from_vram = 1;
    break;
  case 5:
    if (++ganonentrance_ctr != 52)
      return;
    OverworldEntrance_AdvanceAndBoom();
    Overworld_DrawMap16_Persist(0x45e, 0xe92);
    Overworld_DrawMap16_Persist(0x460, 0xe93);
    Overworld_DrawMap16_Persist(0x4de, 0xe94);
    Overworld_DrawMap16_Persist(0x4e0, 0xe94);
    Overworld_DrawMap16_Persist(0x55e, 0xe95);
    Overworld_DrawMap16_Persist(0x560, 0xe95);
    nmi_load_bg_from_vram = 1;
    break;
  case 6:
    if (++ganonentrance_ctr != 32)
      return;
    OverworldEntrance_AdvanceAndBoom();
    Overworld_DrawMap16_Persist(0x45e, 0xe96);
    Overworld_DrawMap16_Persist(0x460, 0xe97);
    Overworld_DrawMap16_Persist(0x4de, 0xe98);
    Overworld_DrawMap16_Persist(0x4e0, 0xe99);
    nmi_load_bg_from_vram = 1;
    break;
  case 7:
    if (++ganonentrance_ctr != 32)
      return;
    OverworldEntrance_AdvanceAndBoom();
    Overworld_DrawMap16_Persist(0x4de, 0xe9a);
    Overworld_DrawMap16_Persist(0x4e0, 0xe9b);
    nmi_load_bg_from_vram = 1;
    break;
  case 8:
    if (++ganonentrance_ctr != 32)
      return;
    OverworldEntrance_AdvanceAndBoom();
    Overworld_DrawMap16_Persist(0x4de, 0xe9c);
    Overworld_DrawMap16_Persist(0x4e0, 0xe9d);
    Overworld_DrawMap16_Persist(0x55e, 0xe9e);
    Overworld_DrawMap16_Persist(0x560, 0xe9f);
    nmi_load_bg_from_vram = 1;
    break;
  case 9:
    if (++ganonentrance_ctr != 32)
      return;
    OverworldEntrance_AdvanceAndBoom();
    Overworld_DrawMap16_Persist(0x55e, 0xe9a);
    Overworld_DrawMap16_Persist(0x560, 0xe9b);
    nmi_load_bg_from_vram = 1;
    break;
  case 10:
    if (++ganonentrance_ctr != 32)
      return;
    OverworldEntrance_AdvanceAndBoom();
    Overworld_DrawMap16_Persist(0x55e, 0xe9c);
    Overworld_DrawMap16_Persist(0x560, 0xe9d);
    Overworld_DrawMap16_Persist(0x5de, 0xea0);
    Overworld_DrawMap16_Persist(0x5e0, 0xea1);
    nmi_load_bg_from_vram = 1;
    break;
  case 11:
    if (++ganonentrance_ctr != 32)
      return;
    sound_effect_ambient = 5;
    OverworldEntrance_AdvanceAndBoom();
    Overworld_DrawMap16_Persist(0x5de, 0xe9a);
    Overworld_DrawMap16_Persist(0x5e0, 0xe9b);
    nmi_load_bg_from_vram = 1;
    break;
  case 12:
    if (++ganonentrance_ctr != 72)
      return;
    OverworldEntrance_PlayJingle();
    ganonentrance_ctr = 0;
    music_control = 13;
    sound_effect_ambient = 9;
    break;
  }
}

void OverworldEntrance_AdvanceAndBoom() {  // 9bd00e
  subsubmodule_index++;
  overworld_entrance_sequence_counter = 0;
  sound_effect_1 = 12;
  sound_effect_2 = 7;
}

