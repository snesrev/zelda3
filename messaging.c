#include "messaging.h"
#include "zelda_rtl.h"
#include "variables.h"
#include "snes/snes_regs.h"
#include "dungeon.h"
#include "hud.h"
#include "load_gfx.h"
#include "dungeon.h"
#include "overworld.h"
#include "variables.h"
#include "ancilla.h"
#include "player.h"
#include "misc.h"
#include "sprite.h"
#include "player_oam.h"
#include "attract.h"
#include "nmi.h"
#include "tables/generated_dialogue.h"
#include "tables/generated_overworld_map.h"
#include "tables/generated_dungeon_map.h"

static const int8 kDungMap_Tab0[14] = {-1, -1, -1, -1, -1, 2, 0, 10, 4, 8, -1, 6, 12, 14};
static const uint16 kDungMap_Tab1[8] = {0x2108, 0x2109, 0x2109, 0x210a, 0x210b, 0x210c, 0x210d, 0x211d};
static const uint16 kDungMap_Tab2[8] = {0x2118, 0x2119, 0xa109, 0x211a, 0x211b, 0x211c, 0x2118, 0xa11d};
static const uint8 kDungMap_Tab3[14] = {0x60, 0x84, 0, 0xb, 0x32, 0x21, 0x33, 0x21, 0x38, 0x21, 0x3a, 0x21, 0x7f, 0x20};
static const uint8 kDungMap_Tab4[14] = {0x60, 0xa4, 0, 0xb, 0x42, 0x21, 0x43, 0x21, 0x49, 0x21, 0x4a, 0x21, 0x7f, 0x20};
static const uint16 kDungMap_Tab8[7] = {0x1b28, 0x1b29, 0x1b2a, 0x1b2b, 0x1b2c, 0x1b2d, 0x1b2e};
static const uint16 kDungMap_Tab6[21] = {0xaa10, 0x100, 0x1b2f, 0xc910, 0x300, 0x1b2f, 0x1b2e, 0xe510, 0xb00, 0x1b2f, 0x1b2e, 0x5b2f, 0x1b2f, 0x1b2e, 0x1b2e, 0x311, 0x100, 0x1b2f, 0x411, 0xc40, 0x1b2e};
static const uint16 kDungMap_Tab5[14] = {0x21, 0x23, 0x20, 0x21, 0x70, 0x12, 0x11, 0x212, 2, 0x217, 0x160, 0x12, 0x113, 0x171};
static const uint16 kDungMap_Tab7[9] = {0x1223, 0x1263, 0x12a3, 0x12e3, 0x1323, 0x11e3, 0x11a3, 0x1163, 0x1123};
static const uint16 kDungMap_Tab9[8] = {0xf26, 0xf27, 0x4f27, 0x4f26, 0x8f26, 0x8f27, 0xcf27, 0xcf26};
static const uint16 kDungMap_Tab10[4] = {0xe2, 0xf8, 0x3a2, 0x3b8};
static const uint16 kDungMap_Tab11[4] = {0x1f19, 0x5f19, 0x9f19, 0xdf19};
static const uint16 kDungMap_Tab12[2] = {0xe4, 0x3a4};
static const uint16 kDungMap_Tab13[2] = {0x1f1a, 0x9f1a};
static const uint16 kDungMap_Tab14[2] = {0x122, 0x138};
static const uint16 kDungMap_Tab15[2] = {0x1f1b, 0x5f1b};
static const uint16 kDungMap_Tab16[8] = {0x1f1e, 0x1f1f, 0x1f20, 0x1f21, 0x1f22, 0x1f23, 0x1f24, 0x1f25};
static const uint16 kDungMap_Tab23[744] = {
  0xb61, 0x5361, 0x8b61, 0x8b62, 0xb60, 0xb63, 0x8b60, 0xb64, 0xb00, 0xb00, 0xb65, 0xb66, 0xb67, 0x4b67, 0x9367, 0xd367, 0xb60, 0x5360, 0x8b60, 0xcb60, 0xb6a, 0x4b6a, 0x4b6d, 0xb6d, 0x1368, 0x1369, 0xb00, 0xb00, 0xb6a, 0x136b, 0xb6c, 0xb6d,
  0x136e, 0x4b6e, 0xb00, 0xb00, 0x136f, 0xb00, 0xb00, 0xb00, 0x1340, 0xb00, 0xb78, 0x1744, 0x536d, 0x136d, 0x4b76, 0xb76, 0xb70, 0xb71, 0xb72, 0x8b71, 0xb75, 0xb76, 0x8b75, 0x8b76, 0xb00, 0xb53, 0xb00, 0xb55, 0x1354, 0x5354, 0xb00, 0xb00,
  0x4b53, 0xb00, 0xb56, 0xb57, 0xb00, 0xb59, 0xb00, 0x135e, 0x135a, 0x135b, 0x135f, 0x535f, 0xb5c, 0xb5d, 0x535e, 0xcb58, 0xb50, 0x4b50, 0x1352, 0x5352, 0xb00, 0xb40, 0x1345, 0xb46, 0x8b42, 0xb47, 0xb42, 0xb49, 0x1348, 0x5348, 0x174a, 0x574a,
  0x4b47, 0xcb42, 0x4b49, 0x4b42, 0xb00, 0xb4b, 0xb00, 0xb4d, 0xb4c, 0x4b4c, 0xb4e, 0x4b4e, 0xb51, 0xb44, 0xb00, 0xb00, 0xb4f, 0x4b4f, 0x934f, 0xd34f, 0xb00, 0xb00, 0xb00, 0xb40, 0xb00, 0xb41, 0xb00, 0xb42, 0xb00, 0xb00, 0xb43, 0xb43,
  0xb00, 0xb00, 0x9344, 0xb00, 0x1340, 0xb00, 0x1341, 0xb00, 0x1740, 0xb40, 0xb42, 0xb7d, 0x4b7a, 0xb7a, 0xb7e, 0x4b7e, 0xb40, 0x8b4d, 0x4bba, 0xb55, 0xb40, 0x8b55, 0x1378, 0xcb53, 0x4b76, 0x4b75, 0x13bb, 0x53bb, 0x4b7f, 0x4b42, 0xb83, 0x13bc,
  0xb00, 0xb00, 0xb79, 0xb00, 0xb6e, 0x4b7c, 0xb00, 0xb41, 0x1340, 0x8b55, 0xb42, 0xb7b, 0x8b42, 0x9344, 0x1341, 0xb00, 0xb53, 0x9344, 0x8b53, 0x9344, 0x8b42, 0x9344, 0xb42, 0x9344, 0x934d, 0xb00, 0x8b53, 0x9344, 0xb00, 0xb00, 0xb40, 0xb00,
  0xb41, 0xb00, 0x1384, 0xb00, 0xbb8, 0x13b9, 0x4b85, 0xcb7c, 0xb87, 0x13b0, 0x4b7b, 0x9344, 0xb00, 0xb00, 0xb40, 0xb00, 0xb91, 0x5391, 0xb9c, 0x4b9c, 0x8b42, 0x1392, 0xb93, 0x1394, 0xb95, 0xb96, 0x9395, 0x8b96, 0xb97, 0xb98, 0x8b97, 0x8b98,
  0x1799, 0x5799, 0x9799, 0xd799, 0x4b98, 0x4b97, 0xcb98, 0xcb97, 0x937b, 0xb00, 0xb7b, 0xb00, 0xba6, 0x4ba6, 0xcb7a, 0x8b7a, 0xb8e, 0x4b8e, 0x938e, 0xcb8e, 0x934d, 0xb8f, 0x1390, 0x5390, 0xb00, 0xb00, 0xb00, 0x8b48, 0xb00, 0x934e, 0xb00, 0x8b4d,
  0x8b72, 0x1346, 0xb45, 0xb46, 0x5744, 0x1744, 0xb00, 0xb00, 0x134d, 0xb00, 0x8b54, 0xb00, 0x1349, 0x1349, 0xb00, 0xb00, 0xb4b, 0x8b48, 0xb72, 0x4b72, 0xb00, 0xb74, 0xb00, 0xbb0, 0xb71, 0x1747, 0x17af, 0xb4b, 0xb6f, 0x1370, 0xb4b, 0xb00,
  0xb6b, 0x8b6c, 0x8b6b, 0xbad, 0xb73, 0xb00, 0x13ae, 0xb46, 0x176b, 0x576b, 0xb6a, 0x4b6a, 0x1368, 0x5368, 0x1369, 0x5369, 0x8b4e, 0xb00, 0x9354, 0xb00, 0xb00, 0xb00, 0xb00, 0x5377, 0xb00, 0x974d, 0xb00, 0x4b7b, 0xb40, 0x8b4d, 0xb51, 0xb8d,
  0x537a, 0x137a, 0x4b42, 0x8b40, 0xb00, 0xb00, 0xb00, 0xb00, 0xb00, 0xb00, 0xb40, 0xb00, 0xcb7a, 0x576e, 0xb00, 0xb00, 0xb6e, 0xb9f, 0xb00, 0x4ba5, 0x13a0, 0x13a1, 0xba2, 0xba3, 0xba4, 0xb00, 0xba5, 0xb00, 0xb40, 0x8b55, 0xb42, 0xcb87,
  0x8b95, 0xba7, 0x8b42, 0xbaf, 0x4b78, 0xb00, 0x4b78, 0xb00, 0x8b42, 0xb51, 0xb78, 0x8b51, 0xba8, 0xba9, 0xbac, 0x8ba9, 0xbaa, 0x17ab, 0x13b4, 0x8bab, 0x17b1, 0xb41, 0x4b44, 0x4b42, 0xb00, 0xbad, 0xb00, 0x13ae, 0x1340, 0xbb7, 0xb42, 0xbb6,
  0xb00, 0xb00, 0x139d, 0x139e, 0xb00, 0xb00, 0xb00, 0xb79, 0xb00, 0xb00, 0x8b42, 0xb86, 0xb42, 0x8b7b, 0x8b42, 0xb7b, 0xb87, 0x8b7b, 0x9387, 0xb7b, 0xb40, 0x13b3, 0x1378, 0xb8d, 0x8b42, 0xb88, 0x5378, 0xb40, 0x4b44, 0xd342, 0x97b5, 0x4b78,
  0x13b3, 0x8b55, 0x4b7b, 0xb8d, 0xb89, 0x138a, 0xb8b, 0xb8c, 0xb00, 0xb7c, 0xb00, 0xb00, 0xb00, 0x9348, 0xb00, 0xb56, 0xb00, 0xb00, 0xb88, 0xb00, 0xb00, 0xb48, 0xb00, 0xb00, 0xb00, 0x9348, 0x1786, 0xb65, 0xb00, 0xb00, 0xcb5a, 0xb00,
  0xb00, 0x5388, 0xb00, 0xb00, 0x4b5a, 0xb00, 0xb00, 0xb00, 0xb00, 0xcb5b, 0x13ab, 0xbac, 0xcb5a, 0xb00, 0x137e, 0xb00, 0xb00, 0x137e, 0xb00, 0xb00, 0xb00, 0x8b48, 0x1783, 0x1384, 0xb00, 0xb00, 0x1385, 0xb00, 0xb00, 0x537e, 0xb00, 0xb00,
  0xb00, 0x8b48, 0xb43, 0xcb43, 0xb00, 0xb00, 0x1379, 0x137a, 0xb5a, 0x137b, 0xb00, 0xb00, 0xb00, 0x8b48, 0x137f, 0x1380, 0xb00, 0xb00, 0x1381, 0x1382, 0xb00, 0xb48, 0xb00, 0xb00, 0xb00, 0xb00, 0x1387, 0x1377, 0x5746, 0xb47, 0x1349, 0xb48,
  0x1375, 0x4b42, 0x174a, 0x574a, 0xb43, 0x1344, 0xb45, 0x1746, 0x1742, 0x5742, 0x8b42, 0xcb42, 0x1375, 0x5375, 0x8b42, 0xcb42, 0x4b40, 0x1340, 0xb41, 0x4b41, 0x4b46, 0xb71, 0x1786, 0x8b71, 0x1347, 0xb4d, 0xb65, 0xb5b, 0xb00, 0xb00, 0x9348, 0xb00,
  0xb00, 0xb00, 0xb00, 0x8b48, 0x4b66, 0x8b65, 0x4b5b, 0xb65, 0x9365, 0xb66, 0xb63, 0x8b66, 0x4b51, 0xb5f, 0xcb76, 0xb60, 0xb64, 0x4b4f, 0x4b60, 0x8b76, 0x4b76, 0xb61, 0xd376, 0x1362, 0x4b61, 0xb76, 0xcb58, 0x8b51, 0xb00, 0xb00, 0x5746, 0xb5e,
  0xb00, 0xb00, 0xb5e, 0xb46, 0xb00, 0xb00, 0x8b48, 0xb00, 0xb4f, 0xb51, 0xcb76, 0x8b76, 0x5351, 0xb51, 0x8b4f, 0x8b51, 0x4b76, 0xb76, 0xcb51, 0x8b58, 0xb54, 0xb00, 0x8b66, 0xb00, 0x9348, 0x8b48, 0xb56, 0x4b45, 0xb00, 0xb57, 0xb00, 0xb59,
  0x4b50, 0xb58, 0xcb50, 0x8b50, 0x5758, 0x1751, 0xcb58, 0x8b51, 0xb56, 0x4b56, 0xb65, 0x5756, 0x9348, 0x8b48, 0xb4c, 0xb4b, 0xb4d, 0xb00, 0x8b54, 0xb00, 0xb4f, 0xb50, 0x8b4f, 0x8b50, 0x4b50, 0xb51, 0xcb58, 0x8b51, 0xb52, 0xb54, 0xb53, 0x9354,
  0x9748, 0x9748, 0x138d, 0x138e, 0x1391, 0x1392, 0x138c, 0x138f, 0x1393, 0x1390, 0x9393, 0x138f, 0x1394, 0x1395, 0x138e, 0x138c, 0x175d, 0x1399, 0x975d, 0x538f, 0x1397, 0x1398, 0x179a, 0x138c, 0x1399, 0x1766, 0x138f, 0xd75d, 0x538e, 0x538f, 0x1391, 0x1392,
  0x139b, 0x539b, 0x139c, 0x539c, 0x138f, 0x138e, 0x5392, 0x5391, 0x138a, 0x538a, 0x138b, 0x538b, 0xb00, 0xcb5b, 0xb00, 0x8b54, 0x4b74, 0x13a6, 0xb00, 0x4b48, 0x13a0, 0x13a1, 0x538e, 0x138e, 0xd38e, 0x53a3, 0x13a4, 0xb00, 0x97aa, 0xb00, 0x538e, 0x1399,
  0x13a4, 0xb00, 0x138e, 0xb00, 0xb00, 0x5393, 0xb00, 0x574e, 0x4b7d, 0xb00, 0x8b7d, 0x139f, 0x97aa, 0x13a4, 0x13a9, 0x53a9, 0x13a5, 0x13a6, 0x93a5, 0xd3a5, 0xd38e, 0x938e, 0x13a4, 0x13aa, 0xb00, 0x13a6, 0xb00, 0x8b5f, 0x139b, 0x13a6, 0x139c, 0x53a2,
  0xb00, 0xb00, 0x138c, 0xb00, 0x9394, 0x139e, 0xb00, 0xb00,
};
static const uint16 kDungMap_Ptrs27[14] = {0xfc00, 0xfc08, 0xfc15, 0xfc21, 0xfc2b, 0xfc32, 0xfc3f, 0xfc4d, 0xfc5f, 0xfc68, 0xfc7d, 0xfc83, 0xfc8f, 0xfca0};
static const uint16 kDungMap_Tab21[3] = {137, 167, 79};
static const uint16 kDungMap_Tab22[3] = {169, 119, 190};
static const uint16 kDungMap_Tab24[2] = {0x1f, 0x7f};
static const uint16 kDungMap_Tab25[14] = {15, 15, 200, 51, 32, 6, 90, 144, 41, 222, 7, 172, 164, 13};
static const int16 kDungMap_Tab28[14] = {-1, -1, 1, 1, 6, 0xff, 0xff, 0xff, 0xfe, 0xf9, 5, 0xff, 0xfd, 6};
static PlayerHandlerFunc *const kDungMapInit[] = {
  &Module0E_03_01_00_PrepMapGraphics,
  &Module0E_03_01_01_DrawLEVEL,
  &Module0E_03_01_02_DrawFloorsBackdrop,
  &Module0E_03_01_03_DrawRooms,
  &DungeonMap_DrawRoomMarkers,
};
static const uint8 kDungMap_Tab38[4] = {0x39, 0x3b, 0x3d, 0x3b};
static const int8 kDungMap_Tab29[4] = {-9, 8, -9, 8};
static const int8 kDungMap_Tab30[4] = {-8, -8, 9, 9};
static const uint8 kDungMap_Tab31[4] = {0xf1, 0xb1, 0x71, 0x31};
static const uint8 kDungMap_Tab32[4] = {0xc, 0xc, 8, 0xa};
static const uint8 kDungMap_Tab33[8] = {187, 171, 155, 139, 123, 107, 91, 75};
static const uint8 kDungMap_Tab34[8] = {0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25};
static const uint8 kDungMap_Tab35[2] = {0, 8};
static const uint8 kDungMap_Tab36[4] = {0x37, 0x38, 0x38, 0x37};
static const int16 kDungMap_Tab37[14] = { -1, -1, 0x808, 8, 0, 8, 0x808, 8, 0x808, 0x800, 0x404, 0x808, 8, 8 };
static const int8 kDungMap_Tab39[2] = {-4, 4};
static const int8 kDungMap_Tab40[2] = {4, -4};
static const int16 kDungMap_Tab26[2] = {0x60, -0x60};
static PlayerHandlerFunc *const kDungMapSubmodules[] = {
  &DungMap_Backup,
  &Module0E_03_01_DrawMap,
  &DungMap_LightenUpMap,
  &DungeonMap_HandleInputAndSprites,
  &DungMap_4,
  &DungMap_FadeMapToBlack,
  &DungeonMap_RecoverGFX,
  &ToggleStarTilesAndAdvance,
  &DungMap_RestoreOld,
};
static const uint16 kText_Positions[2] = {0x6125, 0x6244};
static const uint16 kSrmOffsets[4] = {0, 0x500, 0xa00, 0xf00};
static const uint8 kTextDictionary[] = {
  0x59, 0x59, 0x59, 0x59,
  0x59, 0x59, 0x59,
  0x59, 0x59,
  0x51, 0x2c, 0x59,
  0x1a, 0x27, 0x1d, 0x59,
  0x1a, 0x2b, 0x1e, 0x59,
  0x1a, 0x25, 0x25, 0x59,
  0x1a, 0x22, 0x27,
  0x1a, 0x27, 0x1d,
  0x1a, 0x2d, 0x59,
  0x1a, 0x2c, 0x2d,
  0x1a, 0x27,
  0x1a, 0x2d,
  0x1b, 0x25, 0x1e,
  0x1b, 0x1a,
  0x1b, 0x1e,
  0x1b, 0x28,
  0x1c, 0x1a, 0x27, 0x59,
  0x1c, 0x21, 0x1e,
  0x1c, 0x28, 0x26,
  0x1c, 0x24,
  0x1d, 0x1e, 0x2c,
  0x1d, 0x22,
  0x1d, 0x28,
  0x1e, 0x27, 0x59,
  0x1e, 0x2b, 0x59,
  0x1e, 0x1a, 0x2b,
  0x1e, 0x27, 0x2d,
  0x1e, 0x1d, 0x59,
  0x1e, 0x27,
  0x1e, 0x2b,
  0x1e, 0x2f,
  0x1f, 0x28, 0x2b,
  0x1f, 0x2b, 0x28,
  0x20, 0x22, 0x2f, 0x1e, 0x59,
  0x20, 0x1e, 0x2d,
  0x20, 0x28,
  0x21, 0x1a, 0x2f, 0x1e,
  0x21, 0x1a, 0x2c,
  0x21, 0x1e, 0x2b,
  0x21, 0x22,
  0x21, 0x1a,
  0x22, 0x20, 0x21, 0x2d, 0x59,
  0x22, 0x27, 0x20, 0x59,
  0x22, 0x27,
  0x22, 0x2c,
  0x22, 0x2d,
  0x23, 0x2e, 0x2c, 0x2d,
  0x24, 0x27, 0x28, 0x30,
  0x25, 0x32, 0x59,
  0x25, 0x1a,
  0x25, 0x28,
  0x26, 0x1a, 0x27,
  0x26, 0x1a,
  0x26, 0x1e,
  0x26, 0x2e,
  0x27, 0x51, 0x2d, 0x59,
  0x27, 0x28, 0x27,
  0x27, 0x28, 0x2d,
  0x28, 0x29, 0x1e, 0x27,
  0x28, 0x2e, 0x27, 0x1d,
  0x28, 0x2e, 0x2d, 0x59,
  0x28, 0x1f,
  0x28, 0x27,
  0x28, 0x2b,
  0x29, 0x1e, 0x2b,
  0x29, 0x25, 0x1e,
  0x29, 0x28, 0x30,
  0x29, 0x2b, 0x28,
  0x2b, 0x1e, 0x59,
  0x2b, 0x1e,
  0x2c, 0x28, 0x26, 0x1e,
  0x2c, 0x1e,
  0x2c, 0x21,
  0x2c, 0x28,
  0x2c, 0x2d,
  0x2d, 0x1e, 0x2b, 0x59,
  0x2d, 0x21, 0x22, 0x27,
  0x2d, 0x1e, 0x2b,
  0x2d, 0x21, 0x1a,
  0x2d, 0x21, 0x1e,
  0x2d, 0x21, 0x22,
  0x2d, 0x28,
  0x2d, 0x2b,
  0x2e, 0x29,
  0x2f, 0x1e, 0x2b,
  0x30, 0x22, 0x2d, 0x21,
  0x30, 0x1a,
  0x30, 0x1e,
  0x30, 0x21,
  0x30, 0x22,
  0x32, 0x28, 0x2e,
  0x7, 0x1e, 0x2b,
  0x13, 0x21, 0x1a,
  0x13, 0x21, 0x1e,
  0x13, 0x21, 0x22,
  0x18, 0x28, 0x2e,
};
static const uint16 kTextDictionary_Idx[] = {
  0, 4, 7, 9, 12, 16, 20, 24, 27, 30, 33, 36, 38, 40, 43, 45, 47, 49, 53, 56, 59, 61, 64, 66, 68, 71, 74, 77, 80, 83, 85, 87, 89, 92, 95, 100, 103, 105, 109, 112, 115, 117, 119, 124, 128, 130, 132, 134, 138, 142, 145, 147, 149, 152, 154, 156, 158, 162, 165, 168, 172, 176, 180, 182, 184, 186, 189, 192, 195, 198, 201, 203, 207, 209, 211, 213, 215, 219, 223, 226, 229, 232, 235, 237, 239, 241, 244, 248, 250, 252, 254, 256, 259, 262, 265, 268, 271, 274
};
static const int8 kText_InitializationData[32] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0x39, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x1c, 4, 0, 0, 0, 0, 0};
static const uint16 kText_BorderTiles[9] = {0x28f3, 0x28f4, 0x68f3, 0x28c8, 0x387f, 0x68c8, 0xa8f3, 0xa8f4, 0xe8f3};
static const uint8 kText_CommandLengths[25] = {
  1, 1, 1, 1, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1,
  2, 2, 2, 2, 1, 1, 1, 1, 1,
};
static const uint8 kVWF_RenderCharacter_setMasks[8] = {0x80, 0x40, 0x20, 0x10, 8, 4, 2, 1};
static const uint16 kVWF_RenderCharacter_renderPos[3] = {0, 0x2a0, 0x540};
static const uint16 kVWF_RenderCharacter_linePositions[3] = {0, 0x40, 0x80};
static const uint8 kVWF_RenderCharacter_widths[99] = {
  6, 6, 6, 6, 6, 6, 6, 6, 3, 6, 6, 6, 7, 6, 6, 6, 6, 6, 6, 7, 6, 7, 7, 7, 7, 6, 6, 6, 6, 6, 6, 6,
  6, 6, 3, 5, 6, 3, 7, 6, 6, 6, 6, 5, 6, 6, 6, 7, 7, 7, 7, 6, 6, 4, 6, 6, 6, 6, 6, 6, 6, 6, 3, 7,
  6, 4, 4, 6, 8, 6, 6, 6, 6, 6, 8, 8, 8, 7, 7, 7, 7, 4, 8, 8, 8, 8, 8, 8, 8, 4, 8, 8, 8, 8, 8, 8,
  8, 8, 4,
};
static const uint16 kVWF_RowPositions[3] = {0, 2, 4};
static const uint16 kVWF_LinePositions[3] = {0, 40, 80};
static const uint16 kVWF_Command7B[4] = {0x24b8, 0x24ba, 0x24bc, 0x24be};
static const uint16 kVWF_Command7C[8] = {0x24b8, 0x24ba, 0x24bc, 0x24be, 0x24b8, 0x24ba, 0x24bc, 0x24be};
static const uint16 kText_WaitDurations[16] = {31, 63, 94, 125, 156, 188, 219, 250, 281, 313, 344, 375, 406, 438, 469, 500};
static PlayerHandlerFunc *const kText_Render[] = {
  &RenderText_Draw_Border,
  &RenderText_Draw_BorderIncremental,
  &RenderText_Draw_CharacterTilemap,
  &RenderText_Draw_MessageCharacters,
  &RenderText_Draw_Finish,
};
static PlayerHandlerFunc *const kMessaging_Text[] = {
  &Text_Initialize,
  &Text_Render,
  &RenderText_PostDeathSaveOptions,
};
static const uint16 kOverworldMapPaletteData[256] = {
  0, 0x94b, 0x1563, 0x1203, 0x2995, 0x5bdf, 0x2191, 0x2e37, 0x7c1f, 0x6f37, 0x7359, 0x777a, 0x7b9b, 0x7fbd, 0, 0,
  0, 0x100, 0, 0, 0x7b9b, 0x11b6, 0x1a9b, 0x5fff, 0x2995, 0x6e94, 0x76d6, 0x7f39, 0x7f7b, 0x7fbd, 0, 0,
  0, 0x100, 0x1d74, 0x67f9, 0x1ee9, 0x338e, 0x6144, 0x7e6a, 0xa44, 0x7c1f, 0x6144, 0x22eb, 0x3dca, 0x5ed2, 0x7fda, 0x316a,
  0, 0x100, 0x14cc, 0x1910, 0x2995, 0x3e3a, 0x1963, 0x15e3, 0x25f5, 0x2e37, 0x15e3, 0x22eb, 0x6144, 0x7e33, 0x5d99, 0x771d,
  0, 0xcec, 0x22eb, 0x2fb1, 0x1d70, 0x2e37, 0x25f5, 0x3e77, 0x473a, 0x6144, 0x7e6a, 0x15e3, 0x2e0b, 0x5354, 0x7fff, 0x16a6,
  0, 0x100, 0x15c5, 0x16a6, 0x1ee9, 0x2f4d, 0x25f5, 0x3e77, 0x473a, 0x5354, 0x15e3, 0x22eb, 0x2918, 0x4a1f, 0x3f7f, 0x7c1f,
  0, 0x100, 0x1563, 0x1203, 0x1ee9, 0x2fb0, 0x1d70, 0x2e37, 0x473a, 0x6144, 0x15e3, 0x22eb, 0x1d70, 0x2e37, 0x4f3f, 0x7fbd,
  0, 0, 0, 0, 0, 0, 0, 0x25f5, 0x316a, 0x5ed2, 0x7fff, 0x15e3, 0x473a, 0x2918, 0x771d, 0,
  0, 0x18c6, 0x948, 0x118a, 0x25cf, 0x57bf, 0x1971, 0x2a18, 0x7c1f, 0x52d8, 0x5af9, 0x5f1a, 0x633b, 0x6b5c, 0, 0,
  0, 0x18c6, 5, 0x45fc, 0x633b, 0x1dce, 0x3694, 0x4718, 0x25cf, 0x1d40, 0x34ea, 0x616f, 0x771b, 0x26d6, 0x2b18, 0x2f5a,
  0, 0x18c6, 0x2571, 0x63da, 0x2a32, 0x3a94, 0x1d40, 0x2580, 0x7c1f, 0x7c1f, 0xcc0, 0x1ecc, 0x3135, 0x1dce, 0x4718, 0x3694,
  0, 0x18c6, 0x14e7, 0x216c, 0x25d0, 0x3a75, 0x2169, 0x2e0e, 0x21d6, 0x2a18, 0x1971, 0x2a32, 0x1d40, 0x2580, 0x597a, 0x72fe,
  0, 0x18c6, 0x2a32, 0x3a94, 0x2171, 0x3238, 0x29f6, 0x4278, 0x4edb, 0x1d40, 0x35cd, 0x15ab, 0x198e, 0x3254, 0x731f, 0x1ed4,
  0, 0x18c6, 0x16a, 0x21ce, 0x2a32, 0x3a94, 0x29f6, 0x4278, 0x4edb, 0x1d40, 0x1971, 0x2a32, 0x496c, 0x5a10, 0x3b5f, 0x7c1f,
  0, 0x18c6, 0x948, 0x118a, 0x222e, 0x32f2, 0x1951, 0x2a18, 0x431b, 0x1d40, 0x1971, 0x2a32, 0x21d4, 0x2a18, 0x4b1f, 0x7b9d,
  0, 0x7c1f, 0x7c1f, 0x7c1f, 0x7c1f, 0x2e31, 0xe4, 0x2169, 0x2e0e, 0x42f1, 0x7c1f, 0x7c1f, 0x7c1f, 0x4a1d, 0x4e3f, 0x5a5f,
};
static const uint8 kOverworldMap_tab1[333] = {
  0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xdf,
  0xde, 0xdd, 0xdc, 0xdb, 0xda, 0xd8, 0xd7, 0xd6, 0xd5, 0xd4, 0xd3, 0xd2, 0xd1, 0xd0, 0xcf, 0xce,
  0xcd, 0xcc, 0xcb, 0xca, 0xc9, 0xc7, 0xc6, 0xc5, 0xc4, 0xc3, 0xc2, 0xc1, 0xc0, 0xbf, 0xbe, 0xbd,
  0xbc, 0xbb, 0xba, 0xb9, 0xb8, 0xb7, 0xb6, 0xb5, 0xb4, 0xb3, 0xb2, 0xb1, 0xb0, 0xaf, 0xae, 0xad,
  0xac, 0xab, 0xaa, 0xa9, 0xa8, 0xa7, 0xa6, 0xa5, 0xa4, 0xa3, 0xa2, 0xa1, 0xa0, 0x9f, 0x9e, 0x9d,
  0x9c, 0x9b, 0x9b, 0x9a, 0x99, 0x98, 0x97, 0x96, 0x95, 0x94, 0x93, 0x92, 0x91, 0x90, 0x8f, 0x8e,
  0x8d, 0x8c, 0x8b, 0x8b, 0x8a, 0x89, 0x88, 0x87, 0x86, 0x85, 0x84, 0x83, 0x82, 0x81, 0x81, 0x80,
  0x7f, 0x7e, 0x7d, 0x7c, 0x7b, 0x7a, 0x79, 0x79, 0x78, 0x77, 0x76, 0x75, 0x74, 0x73, 0x72, 0x72,
  0x71, 0x70, 0x6f, 0x6e, 0x6d, 0x6c, 0x6c, 0x6b, 0x6a, 0x69, 0x68, 0x67, 0x67, 0x66, 0x65, 0x64,
  0x63, 0x62, 0x62, 0x61, 0x60, 0x5f, 0x5e, 0x5d, 0x5d, 0x5c, 0x5b, 0x5a, 0x59, 0x59, 0x58, 0x57,
  0x56, 0x55, 0x55, 0x54, 0x53, 0x52, 0x51, 0x51, 0x50, 0x4f, 0x4e, 0x4e, 0x4d, 0x4c, 0x4b, 0x4a,
  0x4a, 0x49, 0x48, 0x47, 0x47, 0x46, 0x45, 0x44, 0x44, 0x43, 0x42, 0x41, 0x41, 0x40, 0x3f, 0x3e,
  0x3e, 0x3d, 0x3c, 0x3c, 0x3b, 0x3a, 0x39, 0x39, 0x38, 0x37, 0x36, 0x36, 0x35, 0x34, 0x34, 0x33,
  0x32, 0x32, 0x31, 0x30, 0x2f, 0x2f, 0x2e, 0x2d, 0x2d, 0x2c, 0x2b, 0x2b, 0x2a, 0x29, 0x29, 0x28,
  0x27, 0x27, 0x26, 0x25, 0x25, 0x24, 0x23, 0x23, 0x22, 0x21, 0x21, 0x20, 0x1f, 0x1f, 0x1e, 0x1d,
  0x1d, 0x1c, 0x1c, 0x1b, 0x1a, 0x1a, 0x19, 0x18, 0x18, 0x17, 0x17, 0x16, 0x15, 0x15, 0x14, 0x14,
  0x13, 0x12, 0x12, 0x11, 0x10, 0x10,  0xf,  0xf,  0xe,  0xe,  0xd,  0xc,  0xc,  0xb,  0xb,  0xa,
     9,    9,    8,    8,    7,    7,    6,    5,    5,    4,    4,    3,    3,    2,    1,    1,
     0,    0,    0,    0, 0xff, 0xfe, 0xfe, 0xfd, 0xfc, 0xfc, 0xfb, 0xfb, 0xfa, 0xf9, 0xf9, 0xf8,
  0xf7, 0xf7, 0xf6, 0xf5, 0xf4, 0xf4, 0xf3, 0xf2, 0xf2, 0xf1, 0xf0, 0xef, 0xee, 0xee, 0xed, 0xec,
  0xeb, 0xea, 0xe9, 0xe8, 0xe8, 0xe7, 0xe6, 0xe5, 0xe4, 0xe3, 0xe2, 0xe1, 0xe0,
};
static const uint8 kOverworldMapData[7] = {0x79, 0x6e, 0x6f, 0x6d, 0x7c, 0x6c, 0x7f};
static const uint8 kBirdTravel_tab1[8] = {0x7f, 0x79, 0x6c, 0x6d, 0x6e, 0x6f, 0x7c, 0x7d};
static const uint8 kBirdTravel_x_lo[8] = {0x80, 0xcf, 0x10, 0xb8, 0x30, 0x70, 0x70, 0xf0};
static const uint8 kBirdTravel_x_hi[8] = {6, 0xc, 2, 8, 0xf, 0, 7, 0xe};
static const uint8 kBirdTravel_y_lo[8] = {0x5b, 0x98, 0xc0, 0x20, 0x50, 0xb0, 0x30, 0x80};
static const uint8 kBirdTravel_y_hi[8] = {3, 5, 7, 0xb, 0xb, 0xf, 0xf, 0xf};
static const uint8 kPendantBitMask[3] = {4, 1, 2};
static const uint8 kCrystalBitMask[7] = {2, 0x40, 8, 0x20, 1, 4, 0x10};
static const uint16 kOwMapCrystal0_x[9] = {0x7ff, 0x2c0, 0xd00, 0xf31, 0x6d, 0x7e0, 0xf40, 0xf40, 0x8dc};
static const uint16 kOwMapCrystal0_y[9] = {0x730, 0x6a0, 0x710, 0x620, 0x70, 0x640, 0x620, 0x620, 0x30};
static const uint16 kOwMapCrystal1_x[9] = {0xff00, 0xff00, 0xff00, 0x8d0, 0xff00, 0xff00, 0xff00, 0x82, 0xff00};
static const uint16 kOwMapCrystal1_y[9] = {0xff00, 0xff00, 0xff00, 0x80, 0xff00, 0xff00, 0xff00, 0xb0, 0xff00};
static const uint16 kOwMapCrystal2_x[9] = {0xff00, 0xff00, 0xff00, 0x108, 0xff00, 0xff00, 0xff00, 0xf11, 0xff00};
static const uint16 kOwMapCrystal2_y[9] = {0xff00, 0xff00, 0xff00, 0xd70, 0xff00, 0xff00, 0xff00, 0x103, 0xff00};
static const uint16 kOwMapCrystal3_x[9] = {0xff00, 0xff00, 0xff00, 0x6d, 0xff00, 0xff00, 0xff00, 0x1d0, 0xff00};
static const uint16 kOwMapCrystal3_y[9] = {0xff00, 0xff00, 0xff00, 0x70, 0xff00, 0xff00, 0xff00, 0x780, 0xff00};
static const uint16 kOwMapCrystal4_x[9] = {0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0x100, 0xff00};
static const uint16 kOwMapCrystal4_y[9] = {0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xca0, 0xff00};
static const uint16 kOwMapCrystal5_x[9] = {0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xca0, 0xff00};
static const uint16 kOwMapCrystal5_y[9] = {0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xda0, 0xff00};
static const uint16 kOwMapCrystal6_x[9] = {0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0x759, 0xff00};
static const uint16 kOwMapCrystal6_y[9] = {0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xed0, 0xff00};
static const uint16 kOwMapCrystal0_tab[9] = {0, 0, 0, 0x6038, 0x6234, 0x6632, 0x6434, 0x6434, 0x6632};
static const uint16 kOwMapCrystal1_tab[9] = {0, 0, 0, 0x6032, 0, 0, 0, 0x6434, 0};
static const uint16 kOwMapCrystal2_tab[9] = {0, 0, 0, 0x6034, 0, 0, 0, 0x6434, 0};
static const uint16 kOwMapCrystal3_tab[9] = {0, 0, 0, 0x6234, 0, 0, 0, 0x6434, 0};
static const uint16 kOwMapCrystal4_tab[9] = {0, 0, 0, 0, 0, 0, 0, 0x6434, 0};
static const uint16 kOwMapCrystal5_tab[9] = {0, 0, 0, 0, 0, 0, 0, 0x6434, 0};
static const uint16 kOwMapCrystal6_tab[9] = {0, 0, 0, 0, 0, 0, 0, 0x6434, 0};
static const uint8 kOwMap_tab2[4] = {0x68, 0x69, 0x78, 0x69};
static const uint8 kOverworldMap_Table4[4] = {0x34, 0x74, 0xf4, 0xb4};
static const uint8 kOverworldMap_Timer[2] = {33, 12};
static const int16 kOverworldMap_Table3[8] = {0, 0, 1, 2, -1, -2, 1, 2};
static const int16 kOverworldMap_Table2[6] = {0, 0, 224, 480, -72, -224};
static PlayerHandlerFunc *const kMessagingSubmodules[12] = {
  &Module_Messaging_0,
  &Hud_Module_Run,
  &RenderText,
  &Module0E_03_DungeonMap,
  &Module0E_04_RedPotion,
  &Module0E_05_DesertPrayer,
  &Module_Messaging_6,
  &Messaging_OverworldMap,
  &Module0E_08_GreenPotion,
  &Module0E_09_BluePotion,
  &Module0E_0A_FluteMenu,
  &Module0E_0B_SaveMenu,
};
static const uint8 kDeath_AnimCtr0[15] = {0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 4, 5, 5};
static const uint8 kDeath_AnimCtr1[15] = {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 3, 3, 0x62};
static const uint8 kDeath_SprFlags[2] = {0x20, 0x10};
static const uint8 kDeath_SprChar0[2] = {0xea, 0xec};
static const uint8 kDeath_SprY0[3] = {0x7f, 0x8f, 0x9f};
const uint8 kHealthAfterDeath[21] = {
  0x18, 0x18, 0x18, 0x18, 0x18, 0x20, 0x20, 0x28, 0x28, 0x30, 0x30, 0x38, 0x38, 0x38, 0x40, 0x40,
  0x40, 0x48, 0x48, 0x48, 0x50,
};
static PlayerHandlerFunc *const kModule_Death[16] = {
  &GameOver_AdvanceImmediately,
  &Death_Func1,
  &GameOver_DelayBeforeIris,
  &GameOver_IrisWipe,
  &Death_Func4,
  &GameOver_SplatAndFade,
  &Death_Func6,
  &Animate_GAMEOVER_Letters_bounce,
  &GameOver_Finalize_GAMEOVR,
  &GameOver_SaveAndOrContinue,
  &GameOver_InitializeRevivalFairy,
  &RevivalFairy_Main_bounce,
  &GameOver_RiseALittle,
  &GameOver_Restore0D,
  &GameOver_Restore0E,
  &GameOver_ResituateLink,
};
static const uint8 kLocationMenuStartPos[3] = {0, 1, 6};
static void RunInterface();
const uint8 *GetDungmapFloorLayout() {
  return kDungMap_FloorLayout[cur_palace_index_x2 >> 1];
}

uint8 GetOtherDungmapInfo(int count) {
  return kDungMap_Tiles[cur_palace_index_x2 >> 1][count];
}

void DungMap_4() {
  BG2VOFS_copy2 += dungmap_var4;
  dungmap_var5 -= dungmap_var4;
  if (!--byte_7E0205)
    overworld_map_state--;
}

const uint8 *GetCurrentTextPtr() {
  return kDialogueText + kDialogueOffs[dialogue_message_index];
}

void Module_Messaging_6() {
  assert(0);
}

void OverworldMap_SetupHdma() {
  static const uint32 kOverworldMap_TableLow[2] = {0xabdcf, 0xabdd6};
  uint32 a = kOverworldMap_TableLow[overworld_map_flags];
  HdmaSetup(a, a, 0x42, (uint8)M7A, (uint8)M7D, 10);
}

const uint8 *GetLightOverworldTilemap() {
  return kLightOverworldTilemap;
}

void SaveGameFile() {  // 80894a
  int offs = ((srm_var1 >> 1) - 1) * 0x500;
  memcpy(g_zenv.sram + offs, save_dung_info, 0x500);
  memcpy(g_zenv.sram + offs + 0xf00, save_dung_info, 0x500);
  uint16 t = 0x5a5a;
  for (int i = 0; i < 0x4fe; i += 2)
    t -= *(uint16 *)((char *)save_dung_info + i);
  word_7EF4FE = t;
  WORD(g_zenv.sram[offs + 0x4fe]) = t;
  WORD(g_zenv.sram[offs + 0x4fe + 0xf00]) = t;
  ZeldaWriteSram();
}

void TransferMode7Characters() {  // 80e399
  uint16 *dst = g_zenv.vram;
  const uint8 *src = kOverworldMapGfx;
  for (int i = 0; i != 0x4000; i++)
    HIBYTE(dst[i]) = src[i];
}

void Module0E_Interface() {  // 80f800
  bool skip_run = false;
  if (player_is_indoors) {
    if (submodule_index == 3) {
      skip_run = (overworld_map_state != 0 && overworld_map_state != 7);
    } else {
      Dungeon_PushBlock_Handler();
    }
  } else {
    skip_run = ((submodule_index == 7 || submodule_index == 10) && overworld_map_state);
  }
  if (!skip_run) {
    Sprite_Main();
    LinkOam_Main();
    if (!player_is_indoors)
      OverworldOverlay_HandleRain();
    Hud_RefillLogic();
    if (submodule_index != 2)
      OrientLampLightCone();
  }
  RunInterface();
  BG2HOFS_copy = BG2HOFS_copy2 + bg1_x_offset;
  BG2VOFS_copy = BG2VOFS_copy2 + bg1_y_offset;
  BG1HOFS_copy = BG1HOFS_copy2 + bg1_x_offset;
  BG1VOFS_copy = BG1VOFS_copy2 + bg1_y_offset;
}

void Module_Messaging_0() {  // 80f875
  assert(0);
}

static void RunInterface() {  // 80f89a
  kMessagingSubmodules[submodule_index]();
}

void Module0E_05_DesertPrayer() {  // 80f8b1
  switch (subsubmodule_index) {
  case 0: ResetTransitionPropsAndAdvance_ResetInterface(); break;
  case 1: ApplyPaletteFilter_bounce(); break;
  case 2:
    DesertPrayer_InitializeIrisHDMA();
    BYTE(palette_filter_countdown) = mosaic_target_level - 1;
    mosaic_target_level = 0;
    BYTE(darkening_or_lightening_screen) = 2;
    break;
  case 3:
    ApplyPaletteFilter_bounce();
    // fall through
  case 4:
    DesertPrayer_BuildIrisHDMATable();
    break;
  }
}

void Module0E_04_RedPotion() {  // 80f8fb
  if (Hud_RefillHealth()) {
    button_mask_b_y &= ~0x40;
    flag_update_hud_in_nmi++;
    submodule_index = 0;
    main_module_index = saved_module_for_menu;
  }
}

void Module0E_08_GreenPotion() {  // 80f911
  if (Hud_RefillMagicPower()) {
    button_mask_b_y &= ~0x40;
    flag_update_hud_in_nmi++;
    submodule_index = 0;
    main_module_index = saved_module_for_menu;
  }
}

void Module0E_09_BluePotion() {  // 80f918
  if (Hud_RefillHealth())
    submodule_index = 8;
  if (Hud_RefillMagicPower())
    submodule_index = 4;
}

void Module0E_0B_SaveMenu() {  // 80f9fa
  // This is the continue / save and quit menu
  if (!player_is_indoors)
    Overworld_DwDeathMountainPaletteAnimation();
  RenderText();
  flag_update_hud_in_nmi = 0;
  nmi_disable_core_updates = 0;
  if (subsubmodule_index < 3)
    subsubmodule_index++;
  else
    nmi_load_bg_from_vram = 0;
  if (!submodule_index) {
    subsubmodule_index = 0;
    nmi_load_bg_from_vram = 1;
    if (choice_in_multiselect_box) {
      sound_effect_ambient = 15;
      main_module_index = 23;
      submodule_index = 1;
      index_of_changable_dungeon_objs[0] = 0;
      index_of_changable_dungeon_objs[1] = 0;
    } else {
      choice_in_multiselect_box = choice_in_multiselect_box_bak;
    }
  }
}

void Module1B_SpawnSelect() {  // 828586
  RenderText();
  if (submodule_index)
    return;
  nmi_load_bg_from_vram = 0;
  EnableForceBlank();
  EraseTileMaps_normal();
  uint8 bak = which_starting_point;
  which_starting_point = kLocationMenuStartPos[choice_in_multiselect_box];
  subsubmodule_index = 0;
  LoadDungeonRoomRebuildHUD();
  which_starting_point = bak;
}

void CleanUpAndPrepDesertPrayerHDMA() {  // 82c7b8
  HdmaSetup(0, 0x2c80c, 0x41, 0, (uint8)WH0, 0);

  W12SEL_copy = 0x33;
  W34SEL_copy = 3;
  WOBJSEL_copy = 0x33;
  TMW_copy = TM_copy;
  TSW_copy = TS_copy;
  HDMAEN_copy = 0x80;
  memset(mode7_hdma_table, 0, 0x1e0);
}

void DesertPrayer_InitializeIrisHDMA() {  // 87ea06
  CleanUpAndPrepDesertPrayerHDMA();
  spotlight_var1 = 0x26;
  BYTE(spotlight_var2) = 0;
  DesertPrayer_BuildIrisHDMATable();
  subsubmodule_index++;
}

void DesertPrayer_BuildIrisHDMATable() {  // 87ea27
  uint16 r14 = link_y_coord - BG2VOFS_copy2 + 12;
  spotlight_y_lower = r14 - spotlight_var1;
  uint16 r4 = sign16(spotlight_y_lower) ? spotlight_y_lower : 0;
  uint16 k;
  spotlight_y_upper = spotlight_y_lower + spotlight_var1 * 2;
  spotlight_var3 = link_x_coord - BG2HOFS_copy2 + 8;
  spotlight_var4 = 1;
  do {
    uint16 r0 = 0x100, r2 = 0x100;
    if (!(sign16(spotlight_y_lower) || (r4 >= spotlight_y_lower && r4 < spotlight_y_upper))) {
      k = (r4 - 1);
    } else if (spotlight_var1 < spotlight_var4) {
      spotlight_var4 = 1;
      spotlight_y_lower = 0;
      r4 = spotlight_y_upper;
      if (r4 >= 225)
        break;
      k = (r4 - 1);
    } else {
      Pair16U pair = DesertHDMA_CalculateIrisShapeLine();
      if (pair.a == 0) {
        spotlight_y_lower = 0;
      } else {
        r2 = spotlight_var3 + pair.b;
        r0 = spotlight_var3 - pair.b;
      }
      k = (r14 - BYTE(spotlight_var4) - 1);
    }
    uint8 t6 = (r0 < 256) ? r0 : (r0 < 512) ? 255 : 0;
    uint8 t7 = (r2 < 256) ? r2 : 255;
    uint16 r6 = t7 << 8 | t6;
    if (k < 224)
     mode7_hdma_table[k] = (r6 == 0xffff) ? 0xff : r6;
    if (sign16(spotlight_y_lower) || (r4 >= spotlight_y_lower && r4 < spotlight_y_upper)) {
      k = BYTE(spotlight_var4) - 2 + r14;
      if (k < 224)
        mode7_hdma_table[k] = (r6 == 0xffff) ? 0xff : r6;
      spotlight_var4++;
    }
    r4++;
  } while (sign16(r4) || r4 < 225);

  if (subsubmodule_index != 4)
    return;
  if (BYTE(spotlight_var2) != 1 && (filtered_joypad_H | filtered_joypad_L) & 0xc0) {
    BYTE(spotlight_var2) = 1;
    BYTE(spotlight_var1) >>= 1;
  }
  if (BYTE(spotlight_var2) && (BYTE(spotlight_var1) += 8) >= 0xc0) {
    byte_7E02F0 ^= 1;
    music_control = 0xf3;
    sound_effect_ambient = 0;
    flag_unk1 = 0;
    some_animation_timer_steps = 0;
    button_mask_b_y = 0;
    link_state_bits = 0;
    link_cant_change_direction &= ~1;
    subsubmodule_index = 0;
    submodule_index = 0;
    main_module_index = saved_module_for_menu;
    TMW_copy = 0;
    TSW_copy = 0;
    W12SEL_copy = 0;
    W34SEL_copy = 0;
    WOBJSEL_copy = 0;
    IrisSpotlight_ResetTable();
  } else {
    static const uint8 kPrayingScene_Delays[5] = {22, 22, 22, 64, 1};
    if (sign8(--link_delay_timer_spin_attack)) {
      int i = some_animation_timer_steps + 1;
      if (i != 4)
        some_animation_timer_steps = i;
      link_delay_timer_spin_attack = kPrayingScene_Delays[i];
    }
  }
}

Pair16U DesertHDMA_CalculateIrisShapeLine() {  // 87ecdc
  static const uint8 kPrayingScene_Tab1[129] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xfe, 0xfe, 0xfe,
    0xfd, 0xfd, 0xfd, 0xfd, 0xfc, 0xfc, 0xfc, 0xfb, 0xfb, 0xfb, 0xfa, 0xfa, 0xf9, 0xf9, 0xf8, 0xf8,
    0xf7, 0xf7, 0xf6, 0xf6, 0xf5, 0xf5, 0xf4, 0xf3, 0xf3, 0xf2, 0xf1, 0xf1, 0xf0, 0xef, 0xee, 0xee,
    0xed, 0xec, 0xeb, 0xea, 0xe9, 0xe9, 0xe8, 0xe7, 0xe6, 0xe5, 0xe4, 0xe3, 0xe2, 0xe1, 0xdf, 0xde,
    0xdd, 0xdc, 0xdb, 0xda, 0xd8, 0xd7, 0xd6, 0xd5, 0xd3, 0xd2, 0xd0, 0xcf, 0xcd, 0xcc, 0xca, 0xc9,
    0xc7, 0xc6, 0xc4, 0xc2, 0xc1, 0xbf, 0xbd, 0xbb, 0xb9, 0xb7, 0xb6, 0xb4, 0xb1, 0xaf, 0xad, 0xab,
    0xa9, 0xa7, 0xa4, 0xa2, 0x9f, 0x9d, 0x9a, 0x97, 0x95, 0x92, 0x8f, 0x8c, 0x89, 0x86, 0x82, 0x7f,
    0x7b, 0x78, 0x74, 0x70, 0x6c, 0x67, 0x63, 0x5e, 0x59, 0x53, 0x4d, 0x46, 0x3f, 0x37, 0x2d, 0x1f,
    0,
  };
  static const uint8 kPrayingScene_Tab0[129] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xfe, 0xfd, 0xfd, 0xfc, 0xfc, 0xfb, 0xfa, 0xf9, 0xf8,
    0xf7, 0xf6, 0xf5, 0xf4, 0xf3, 0xf1, 0xf0, 0xee, 0xed, 0xeb, 0xe9, 0xe8, 0xe6, 0xe4, 0xe2, 0xdf,
    0xdd, 0xdb, 0xd8, 0xd6, 0xd3, 0xd0, 0xcd, 0xca, 0xc7, 0xc4, 0xc1, 0xbd, 0xb9, 0xb6, 0xb1, 0xad,
    0xa9, 0xa4, 0x9f, 0x9a, 0x95, 0x8f, 0x89, 0x82, 0x7b, 0x74, 0x6c, 0x63, 0x59, 0x4d, 0x3f, 0x2d,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,
  };
  uint8 t = snes_divide(BYTE(spotlight_var4) << 8, BYTE(spotlight_var1)) >> 1;
  uint8 r6 = BYTE(spotlight_var2) ? kPrayingScene_Tab1[t] : kPrayingScene_Tab0[t];
  uint16 r8 = r6 * BYTE(spotlight_var1) >> 8;
  if (BYTE(spotlight_var2))
    r8 <<= 1;
  Pair16U ret = { r6, r8 };
  return ret;
}

void Animate_GAMEOVER_Letters() {  // 88f4ca
  switch (ancilla_type[0]) {
  case 0:
    submodule_index++;
    break;
  case 1:
    GameOverText_SweepLeft();
    break;
  case 2:
    GameOverText_UnfurlRight();
    break;
  case 3:
    GameOverText_Draw();
    break;
  }
}

void GameOverText_SweepLeft() {  // 88f4f6
  static const uint8 kGameOverText_Tab1[8] = {0x40, 0x50, 0x60, 0x70, 0x88, 0x98, 0xa8, 0x40};

  int k = flag_for_boomerang_in_place;
  cur_object_index = k;
  ancilla_x_vel[k] = 0x80;
  Ancilla_MoveX(k);
  if (Ancilla_GetX(k) < kGameOverText_Tab1[k]) {
    ancilla_x_lo[k] = kGameOverText_Tab1[k];
    flag_for_boomerang_in_place = ++k;
    if (k == 8) {
      flag_for_boomerang_in_place = 7;
      ancilla_type[0]++;
      hookshot_effect_index = 0;
      sound_effect_2 = 38;
      goto draw;
    }
  }
  if (k == 7) {
    int j = 6;
    while (j != hookshot_effect_index)
      ancilla_x_lo[j--] = ancilla_x_lo[k];
    if (Ancilla_GetX(k) < kGameOverText_Tab1[hookshot_effect_index])
      hookshot_effect_index--;
  }
draw:
  GameOverText_Draw();
}

void GameOverText_UnfurlRight() {  // 88f56d
  static const uint8 kGameOverText_Tab2[8] = {0x58, 0x60, 0x68, 0x70, 0x88, 0x90, 0x98, 0xa0};

  int k = flag_for_boomerang_in_place, end;
  cur_object_index = k;
  ancilla_x_vel[k] = 0x60;
  Ancilla_MoveX(k);
  int j = hookshot_effect_index;
  if (ancilla_x_lo[k] >= kGameOverText_Tab2[j]) {
    ancilla_x_lo[j] = kGameOverText_Tab2[j];
    if (++hookshot_effect_index == 8) {
      submodule_index++;
      ancilla_type[0]++;
      goto draw;
    }
  }
  end = hookshot_effect_index - 1;
  k = flag_for_boomerang_in_place;
  j = k;
  do {
    ancilla_x_lo[j] = ancilla_x_lo[k];
  } while (--j != end);
draw:
  GameOverText_Draw();
}

void Module12_GameOver() {  // 89f290
  kModule_Death[submodule_index]();
  if (submodule_index != 9)
    LinkOam_Main();
}

void GameOver_AdvanceImmediately() {  // 89f2a2
  submodule_index++;
  Death_Func1();
}

void Death_Func1() {  // 89f2a4
  music_unk1_death = music_unk1;
  sound_effect_ambient_last_death = sound_effect_ambient_last;
  music_control = 241;
  sound_effect_ambient = 5;
  overworld_map_state = 5;
  byte_7E03F3 = 0;
  byte_7E0322 = 0;
  link_cape_mode = 0;
  mapbak_bg1_x_offset = palette_filter_countdown;
  mapbak_bg1_y_offset = darkening_or_lightening_screen;
  memcpy(mapbak_palette, aux_palette_buffer, 256);
  memset(aux_palette_buffer + 32, 0, 192);
  palette_filter_countdown = 0;
  darkening_or_lightening_screen = 0;
  bg1_x_offset = 0;
  bg1_y_offset = 0;
  mapbak_CGWSEL = WORD(CGWSEL_copy);
  g_ram[0xc8] = 32;
  hud_floor_changed_timer = 0;
  Hud_FloorIndicator();
  flag_update_hud_in_nmi++;
  sound_effect_ambient = 5;
  submodule_index++;
}

void GameOver_DelayBeforeIris() {  // 89f33b
  if (--g_ram[0xc8])
    return;
  Death_InitializeGameOverLetters();
  IrisSpotlight_close();
  WOBJSEL_copy = 0x30;
  W34SEL_copy = 0;
  submodule_index++;
}

void GameOver_IrisWipe() {  // 89f350
  PaletteFilter_RestoreBGSubstractiveStrict();
  main_palette_buffer[0] = main_palette_buffer[32];
  uint8 bak = main_module_index;
  IrisSpotlight_ConfigureTable();
  main_module_index = bak;
  if (submodule_index)
    return;
  for (int i = 0; i < 16; i++) {
    main_palette_buffer[0x20 + i] = 0x18;
    main_palette_buffer[0x30 + i] = 0x18;
    main_palette_buffer[0x40 + i] = 0x18;
    main_palette_buffer[0x50 + i] = 0x18;
    main_palette_buffer[0x60 + i] = 0x18;
    main_palette_buffer[0x70 + i] = 0x18;
  }
  main_palette_buffer[0] = main_palette_buffer[32] = 0x18;

  IrisSpotlight_ResetTable();
  COLDATA_copy0 = 32;
  COLDATA_copy1 = 64;
  COLDATA_copy2 = 128;
  W12SEL_copy = 0;
  W34SEL_copy = 0;
  WOBJSEL_copy = 0;
  submodule_index = 4;
  flag_update_cgram_in_nmi++;
  INIDISP_copy = 15;
  TM_copy = 20;
  TS_copy = 0;
  CGADSUB_copy = 32;
  g_ram[0xc8] = 64;
  BYTE(palette_filter_countdown) = 0;
  BYTE(darkening_or_lightening_screen) = 0;
  Death_PrepFaint();
}

void GameOver_SplatAndFade() {  // 89f3de
  if (g_ram[0xc8]) {
    g_ram[0xc8]--;
    return;
  }
  PaletteFilter_RestoreBGSubstractiveStrict();
  main_palette_buffer[0] = main_palette_buffer[32];
  if (BYTE(darkening_or_lightening_screen) != 0xff)
    return;
  mosaic_level = 0;
  mosaic_inc_or_dec = 0;
  MOSAIC_copy = 3;

  for (int i = 0; i != 4; i++) {
    if (link_bottle_info[i] == 6) {
      link_bottle_info[i] = 2;
      g_ram[0xc8] = 12;
      load_chr_halfslot_even_odd = 15;
      Graphics_LoadChrHalfSlot();
      load_chr_halfslot_even_odd = 0;
      submodule_index = 10;
      return;
    }
  }
  index_of_changable_dungeon_objs[0] = 0;
  index_of_changable_dungeon_objs[1] = 0;
  nmi_subroutine_index = 22;
  nmi_disable_core_updates = 22;
  submodule_index++;
}

void Death_Func6() {  // 89f458
  g_ram[0xc8] = 12;
  load_chr_halfslot_even_odd = 15;
  Graphics_LoadChrHalfSlot();
  load_chr_halfslot_even_odd = 0;
  palette_sp6 = 5;
  overworld_palette_aux_or_main = 0x200;
  Palette_Load_SpriteEnvironment_Dungeon();
  Palette_Load_SpriteMain();
  flag_update_cgram_in_nmi++;
  submodule_index++;
  Death_PlayerSwoon();
}

void Death_Func4() {  // 89f47e
  Death_PlayerSwoon();
}

void Animate_GAMEOVER_Letters_bounce() {  // 89f483
  Animate_GAMEOVER_Letters();
}

void GameOver_Finalize_GAMEOVR() {  // 89f488
  Animate_GAMEOVER_Letters();
  uint8 bak1 = main_module_index;
  uint8 bak2 = submodule_index;
  messaging_module = 2;
  RenderText();
  submodule_index = bak2 + 1;
  main_module_index = bak1;
  g_ram[0xc8] = 2;
  music_control = 11;
}

void GameOver_SaveAndOrContinue() {  // 89f4c1
  GameOver_AnimateChoiceFairy();
  if (ancilla_type)
    Animate_GAMEOVER_Letters();

  if (filtered_joypad_H & 0x20)
    goto do_inc;

  if (!--g_ram[0xc8]) {
    g_ram[0xc8] = 1;
    if (joypad1H_last & 12) {
      if (joypad1H_last & 4) {
do_inc:
        if (++subsubmodule_index >= 3)
          subsubmodule_index = 0;
      } else {
        if (sign8(--subsubmodule_index))
          subsubmodule_index = 2;
      }
      g_ram[0xc8] = 12;
      sound_effect_2 = 32;
    }
  }
  if (!((filtered_joypad_L & 0xc0 | filtered_joypad_H) & 0xd0))
    return;
  sound_effect_1 = 44;
  Death_Func15();
}

void Death_Func15() {  // 89f50f
  music_control = 0xf1;
  if (player_is_indoors)
    Dungeon_FlagRoomData_Quadrants();
  AdjustLinkBunnyStatus();
  if (sram_progress_indicator < 3) {
    savegame_is_darkworld = 0;
    if (!link_item_moon_pearl)
      ForceNonbunnyStatus();
  }
  if (dungeon_room_index == 0)
    player_is_indoors = 0;

  ResetSomeThingsAfterDeath((uint8)dungeon_room_index);
  if (savegame_tagalong == 6 || savegame_tagalong == 9 || savegame_tagalong == 10 || savegame_tagalong == 13)
    savegame_tagalong = 0;

  death_var4 = link_health_current = kHealthAfterDeath[link_health_capacity >> 3];
  uint8 i = BYTE(cur_palace_index_x2);
  if (i != 0xff)
    link_keys_earned_per_dungeon[(i == 2 ? 0 : i) >> 1] = link_num_keys;
  Sprite_ResetAll();
  if (death_var2 == 0xffff)
    death_save_counter++;
  death_var5++;
  if (subsubmodule_index != 1) {
    if (!player_is_indoors)
      goto outdoors;

    if (savegame_tagalong != 1 && BYTE(cur_palace_index_x2) != 255) {
      death_var4 = 0;
    } else {
      buffer_for_playing_songs = 0;
      player_is_indoors = 0;
    outdoors:
      if (savegame_is_darkworld)
        dungeon_room_index = 32;
    }

    if (sram_progress_indicator) {
      if (subsubmodule_index == 0)
        SaveGameFile();
      main_module_index = 5;
      submodule_index = 0;
      nmi_load_bg_from_vram = 0;
    } else {
      uint8 slot = srm_var1;
      int offs = kSrmOffsets[(slot >> 1) - 1];
      WORD(g_ram[0]) = offs;
      death_var5 = 0;
      CopySaveToWRAM();
    }
  } else {
    if (sram_progress_indicator)
      SaveGameFile();
    TM_copy = 16;
    player_is_indoors = 0;
    Death_Func31();
    death_var4 = 0;
    death_var5 = 0;
    buffer_for_playing_songs = 0;
    zelda_snes_dummy_write(NMITIMEN, 0);
    zelda_snes_dummy_write(HDMAEN, 0);
    BG1HOFS_copy2 = 0;
    BG2HOFS_copy2 = 0;
    BG3HOFS_copy2 = 0;
    BG1VOFS_copy2 = 0;
    BG2VOFS_copy2 = 0;
    BG3VOFS_copy2 = 0;
    BG1HOFS_copy = 0;
    BG2HOFS_copy = 0;
    BG1VOFS_copy = 0;
    BG2VOFS_copy = 0;
    memset(save_dung_info, 0, 256 * 5);
    flag_which_music_type = 0;
    LoadOverworldSongs();
    zelda_snes_dummy_write(NMITIMEN, 0x81);
  }
}

void GameOver_AnimateChoiceFairy() {  // 89f67a
  int spr = 0x14;
  bytewise_extended_oam[spr] = 2;
  oam_buf[spr].x = 0x34;
  oam_buf[spr].y = kDeath_SprY0[subsubmodule_index];
  oam_buf[spr].charnum = kDeath_SprChar0[frame_counter >> 3 & 1];
  oam_buf[spr].flags = 0x78;
}

void GameOver_InitializeRevivalFairy() {  // 89f6a4
  ConfigureRevivalAncillae();
  link_hearts_filler = 56;
  submodule_index += 1;
  overworld_map_state = 0;
}

void RevivalFairy_Main_bounce() {  // 89f6b4
  RevivalFairy_Main();
}

void GameOver_RiseALittle() {  // 89f6b9
  if (link_hearts_filler == 0) {
    memcpy(aux_palette_buffer, mapbak_palette, 256);
    memset(main_palette_buffer + 32, 0, 192);
    main_palette_buffer[0] = 0;
    palette_filter_countdown = 0;
    darkening_or_lightening_screen = 2;
    WORD(CGWSEL_copy) = mapbak_CGWSEL;
    submodule_index++;
  }
  RevivalFairy_Main();
  Hud_RefillLogic();
}

void GameOver_Restore0D() {  // 89f71d
  if (!is_doing_heart_animation) {
    load_chr_halfslot_even_odd = 1;
    Graphics_LoadChrHalfSlot();
    Dungeon_ApproachFixedColor_variable(overworld_fixed_color_plusminus);
    submodule_index++;
  }
  RevivalFairy_Main();
  Hud_RefillLogic();
}

void GameOver_Restore0E() {  // 89f735
  Graphics_LoadChrHalfSlot();
  TS_copy = mapbak_TS;
  submodule_index++;
}

void GameOver_ResituateLink() {  // 89f742
  PaletteFilter_RestoreBGAdditiveStrict();
  main_palette_buffer[0] = main_palette_buffer[32];
  if (BYTE(palette_filter_countdown) != 32)
    return;
  if (!player_is_indoors)
    Overworld_SetFixedColAndScroll();
  TS_copy = mapbak_TS;
  main_module_index = saved_module_for_menu;
  submodule_index = 0;
  countdown_for_blink = 144;
  music_control = music_unk1_death;
  sound_effect_ambient = sound_effect_ambient_last_death;
  palette_filter_countdown = mapbak_bg1_x_offset;
  darkening_or_lightening_screen = mapbak_bg1_y_offset;
}

void Module0E_0A_FluteMenu() {  // 8ab730
  switch (overworld_map_state) {
  case 0:
    WorldMap_FadeOut();
    break;
  case 1:
    birdtravel_var1[0] = 0;
    WorldMap_LoadLightWorldMap();
    break;
  case 2:
    WorldMap_LoadSpriteGFX();
    break;
  case 3:
    WorldMap_Brighten();
    break;
  case 4:
    g_ram[0xc8] = 0x10;
    overworld_map_state++;
    break;
  case 5:
    FluteMenu_HandleSelection();
    break;
  case 6:
    WorldMap_RestoreGraphics();
    break;
  case 7:
    FluteMenu_LoadSelectedScreen();
    break;
  case 8:
    Overworld_LoadOverlayAndMap();
    break;
  case 9:
    FluteMenu_FadeInAndQuack();
    break;
  default:
    assert(0);
  }
}

void FluteMenu_HandleSelection() {  // 8ab78b
  PointU8 pt;

  if (g_ram[0xc8] == 0) {
    if ((joypad1L_last | joypad1H_last) & 0xc0) {
      overworld_map_state++;
      return;
    }
  } else {
    g_ram[0xc8]--;
  }
  if (filtered_joypad_H & 10) {
    birdtravel_var1[0]--;
    sound_effect_2 = 32;
  }
  if (filtered_joypad_H & 5) {
    birdtravel_var1[0]++;
    sound_effect_2 = 32;
  }
  birdtravel_var1[0] = birdtravel_var1[0] & 7;
  if (frame_counter & 0x10 && WorldMap_CalculateOamCoordinates(&pt))
    WorldMap_HandleSpriteBlink(16, 2, 0x3e, 0, pt.x - 4, pt.y - 4);

  uint16 ybak = link_y_coord_spexit;
  uint16 xbak = link_x_coord_spexit;
  for (int i = 7; i >= 0; i--) {
    bird_travel_x_lo[i] = kBirdTravel_x_lo[i];
    bird_travel_x_hi[i] = kBirdTravel_x_hi[i];
    link_x_coord_spexit = kBirdTravel_x_hi[i] << 8 | kBirdTravel_x_lo[i];

    bird_travel_y_lo[i] = kBirdTravel_y_lo[i];
    bird_travel_y_hi[i] = kBirdTravel_y_hi[i];
    link_y_coord_spexit = kBirdTravel_y_hi[i] << 8 | kBirdTravel_y_lo[i];

    if (WorldMap_CalculateOamCoordinates(&pt))
      WorldMap_HandleSpriteBlink(i, 0, (i == birdtravel_var1[0]) ? 0x30 + (frame_counter & 6) : 0x32, kBirdTravel_tab1[i], pt.x, pt.y);
  }
  link_x_coord_spexit = xbak;
  link_y_coord_spexit = ybak;
}

void FluteMenu_LoadSelectedScreen() {  // 8ab8c5
  save_ow_event_info[0x3b] &= ~0x20;
  save_ow_event_info[0x7b] &= ~0x20;
  save_dung_info[267] &= ~0x80;
  save_dung_info[40] &= ~0x100;
  FluteMenu_LoadTransport();
  FluteMenu_LoadSelectedScreenPalettes();
  uint8 t = overworld_screen_index & 0xbf;
  DecompressAnimatedOverworldTiles((t == 3 || t == 5 || t == 7) ? 0x58 : 0x5a);
  Overworld_SetFixedColAndScroll();
  overworld_palette_aux_or_main = 0;
  hud_palette = 0;
  InitializeTilesets();
  overworld_map_state++;
  BYTE(dung_draw_width_indicator) = 0;
  Overworld_LoadOverlays2();
  submodule_index--;
  sound_effect_2 = 16;
  uint8 m = overworld_music[BYTE(overworld_screen_index)];
  sound_effect_ambient = m >> 4;
  music_control = (m & 0xf) != music_unk1 ? (m & 0xf) : 0xf3;
}

void Overworld_LoadOverlayAndMap() {  // 8ab948
  uint16 bak1 = WORD(main_module_index);
  uint16 bak2 = WORD(overworld_map_state);
  Overworld_LoadAndBuildScreen();
  WORD(overworld_map_state) = bak2 + 1;
  WORD(main_module_index) = bak1;
}

void FluteMenu_FadeInAndQuack() {  // 8ab964
  if (++INIDISP_copy == 15) {
    BirdTravel_Finish_Doit();
  } else {
    Sprite_Main();
  }
}

void BirdTravel_Finish_Doit() {  // 8ab96c
  overworld_map_state = 0;
  subsubmodule_index = 0;
  main_module_index = saved_module_for_menu;
  submodule_index = 0;
  HDMAEN_copy = mapbak_HDMAEN;
  AddBirdTravelSomething(0x27, 4);
  Sprite_Main();
}

void Messaging_OverworldMap() {  // 8ab98b
  switch (overworld_map_state) {
  case 0:
    WorldMap_FadeOut();
    break;
  case 1:
    WorldMap_LoadLightWorldMap();
    break;
  case 2:
    WorldMap_LoadDarkWorldMap();
    break;
  case 3:
    WorldMap_LoadSpriteGFX();
    break;
  case 4:
    WorldMap_Brighten();
    break;
  case 5:
    WorldMap_PlayerControl();
    break;
  case 6:
    WorldMap_RestoreGraphics();
    break;
  case 7:
    WorldMap_ExitMap();
    break;
  }
}

void WorldMap_FadeOut() {  // 8ab9a3
  if (--INIDISP_copy)
    return;
  mapbak_HDMAEN = HDMAEN_copy;
  EnableForceBlank();
  MOSAIC_copy = 3;
  overworld_map_state++;
  WORD(mapbak_TM) = WORD(TM_copy);
  mapbak_BG1HOFS_copy2 = BG1HOFS_copy2;
  mapbak_BG2HOFS_copy2 = BG2HOFS_copy2;
  mapbak_BG1VOFS_copy2 = BG1VOFS_copy2;
  mapbak_BG2VOFS_copy2 = BG2VOFS_copy2;
  BG1HOFS_copy2 = BG2HOFS_copy2 = BG3HOFS_copy2 = 0;
  BG1VOFS_copy2 = BG2VOFS_copy2 = BG3VOFS_copy2 = 0;
  WORD(mapbak_CGWSEL) = WORD(CGWSEL_copy);
  link_dma_graphics_index = 0x1fc;
  if (BYTE(overworld_screen_index) < 0x80) {
    link_y_coord_spexit = link_y_coord;
    link_x_coord_spexit = link_x_coord;
  }
  if (sram_progress_indicator < 2) {
    CGWSEL_copy = 0x80;
    CGADSUB_copy = 0x61;
  }
  sound_effect_2 = 16;
  sound_effect_ambient = 5;
  music_control = 0xf2;
  zelda_ppu_write(BGMODE, 7);
  BGMODE_copy = 7;
  zelda_ppu_write(M7SEL, 0x80);
}

void WorldMap_LoadLightWorldMap() {  // 8aba30
  WorldMap_FillTilemapWithEF();
  TM_copy = 0x11;
  TS_copy = 0;
  TransferMode7Characters();
  WorldMap_SetUpHDMA();
  LoadOverworldMapPalette();
  LoadActualGearPalettes();
  flag_update_cgram_in_nmi++;
  nmi_subroutine_index = 7;
  INIDISP_copy = 0;
  nmi_disable_core_updates++;
  overworld_map_state++;
}

void WorldMap_LoadDarkWorldMap() {  // 8aba7a
  if (overworld_screen_index & 0x40) {
    memcpy(&uvram, kDarkOverworldTilemap, 1024);
    nmi_subroutine_index = 21;
  }
  overworld_map_state++;
}

void WorldMap_LoadSpriteGFX() {  // 8aba9a
  load_chr_halfslot_even_odd = 0x10;
  Graphics_LoadChrHalfSlot();
  load_chr_halfslot_even_odd = 0;
  overworld_map_state++;
}

void WorldMap_Brighten() {  // 8abaaa
  if (++INIDISP_copy == 15)
    overworld_map_state++;
}

void WorldMap_PlayerControl() {  // 8abae6
  if (overworld_map_flags & 0x80) {
    overworld_map_flags &= ~0x80;
    OverworldMap_SetupHdma();
  }

  if (!overworld_map_flags && filtered_joypad_L & 0x40) { // X
    overworld_map_state++;
    return;
  }
  if (BYTE(dung_draw_width_indicator)) {
    BYTE(dung_draw_width_indicator)--;
  } else if (filtered_joypad_L & 0x70) {
    sound_effect_2 = 36;
    BYTE(dung_draw_width_indicator) = 8;

    int t = overworld_map_flags ^ 1;
    overworld_map_flags = t | 0x80;
    timer_for_mode7_zoom = kOverworldMap_Timer[t];
    if (timer_for_mode7_zoom == 12) {
      BG1VOFS_copy2 = ((link_y_coord_spexit >> 4) - 0x48 & ~1);
      M7Y_copy = BG1VOFS_copy2 + 0x100;
      uint16 t0 = (link_x_coord_spexit >> 4) - 0x80;
      uint16 t1 = (uint16)(5 * (sign16(t0) ? -t0 : t0)) >> 1;
      uint16 t2 = sign16(t0) ? -t1 : t1;
      BG1HOFS_copy2 = t2 + 0x80 & ~1;
    } else {
      BG1VOFS_copy2 = 200;
      M7Y_copy = 200 + 256;
      BG1HOFS_copy2 = 128;
    }
  }

  if (overworld_map_flags) {
    int k = (joypad1H_last & 12) >> 1;
    if (BG1VOFS_copy2 != kOverworldMap_Table2[k]) {
      BG1VOFS_copy2 += kOverworldMap_Table3[k];
      M7Y_copy = BG1VOFS_copy2 + 0x100;
    }
    k = (joypad1H_last & 3) * 2 + 1;
    if (BG1HOFS_copy2 != kOverworldMap_Table2[k])
      BG1HOFS_copy2 += kOverworldMap_Table3[k];
  }
  WorldMap_HandleSprites();
}

void WorldMap_RestoreGraphics() {  // 8abbd6
  if (--INIDISP_copy)
    return;
  EnableForceBlank();
  overworld_map_state++;
  memcpy(main_palette_buffer, aux_palette_buffer, 512);
  WORD(CGWSEL_copy) = WORD(mapbak_CGWSEL);
  BG3HOFS_copy2 = BG3VOFS_copy2 = 0;
  BG1HOFS_copy2 = mapbak_BG1HOFS_copy2;
  BG2HOFS_copy2 = mapbak_BG2HOFS_copy2;
  BG1VOFS_copy2 = mapbak_BG1VOFS_copy2;
  BG2VOFS_copy2 = mapbak_BG2VOFS_copy2;
  WORD(TM_copy) = WORD(mapbak_TM);
  Attract_SetUpConclusionHDMA();
}

void Attract_SetUpConclusionHDMA() {  // 8abc33
  HdmaSetup(0xABDDD, 0xABDDD, 0x42, (uint8)M7A, (uint8)M7D, 0);
  HDMAEN_copy = 0x80;
  zelda_ppu_write(BGMODE, 9);
  BGMODE_copy = 9;
  nmi_disable_core_updates = 0;
}

void WorldMap_ExitMap() {  // 8abc54
  overworld_palette_aux_or_main = 0;
  hud_palette = 0;
  InitializeTilesets();
  flag_update_cgram_in_nmi++;
  BYTE(dung_draw_width_indicator) = 0;
  overworld_map_state = 0;
  subsubmodule_index = 0;
  main_module_index = saved_module_for_menu;
  submodule_index = 32;
  vram_upload_offset = 0;
  HDMAEN_copy = mapbak_HDMAEN;
  sound_effect_ambient = overworld_music[BYTE(overworld_screen_index)] >> 4;
  sound_effect_2 = 0x10;
  music_control = 0xf3;
}

void WorldMap_SetUpHDMA() {  // 8abc96
  BG1HOFS_copy2 = 0x80;
  BG1VOFS_copy2 = 0xc8;
  M7Y_copy = 0x1c9;
  M7X_copy = 0x100;
  W12SEL_copy = 0;
  W34SEL_copy = 0;
  WOBJSEL_copy = 0;
  TMW_copy = 0;
  TSW_copy = 0;
  zelda_ppu_write(M7B, 0);
  zelda_ppu_write(M7B, 0);
  zelda_ppu_write(M7C, 0);
  zelda_ppu_write(M7C, 0);
  zelda_ppu_write(M7X, 0);
  zelda_ppu_write(M7X, 1);
  zelda_ppu_write(M7Y, 0);
  zelda_ppu_write(M7Y, 1);

  if (main_module_index == 20) {
    HdmaSetup(0xABDDD, 0xABDDD, 0x42, (uint8)M7A, (uint8)M7D, 0);
    HDMAEN_copy = 0xc0;
  } else if (submodule_index != 10) {
    byte_7E0635 = 4;
    timer_for_mode7_zoom = 12;
    overworld_map_flags = 1;
    BG1VOFS_copy2 = ((link_y_coord_spexit >> 4) - 0x48 & ~1);
    M7Y_copy = BG1VOFS_copy2 + 0x100;
    uint16 t0 = (link_x_coord_spexit >> 4) - 0x80;
    uint16 t1 = (uint16)(5 * (sign16(t0) ? -t0 : t0)) >> 1;
    uint16 t2 = sign16(t0) ? -t1 : t1;
    BG1HOFS_copy2 = t2 + 0x80 & ~1;
    OverworldMap_SetupHdma();
    HDMAEN_copy = 0xc0;
  } else {
    byte_7E0635 = 4;
    timer_for_mode7_zoom = 33;
    overworld_map_flags = 0;
    HdmaSetup(0xABDCF, 0xABDCF, 0x42, (uint8)M7A, (uint8)M7D, 10);
    HDMAEN_copy = 0xc0;
  }
}

void WorldMap_FillTilemapWithEF() {  // 8abda5
  uint16 *dst = g_zenv.vram;
  for (int i = 0; i != 0x4000; i++)
    BYTE(dst[i]) = 0xef;
}

void WorldMap_HandleSprites() {  // 8abf66
  PointU8 pt;

  if (frame_counter & 0x10 && WorldMap_CalculateOamCoordinates(&pt))
    WorldMap_HandleSpriteBlink(0, 2, 0x3e, 0, pt.x - 4, pt.y - 4);

  uint16 ybak = link_y_coord_spexit;
  uint16 xbak = link_x_coord_spexit;

  int k = 15;
  if (BYTE(overworld_screen_index) < 0x40 && (bird_travel_x_lo[k] | bird_travel_x_hi[k] | bird_travel_y_lo[k] | bird_travel_y_hi[k])) {
    if (!frame_counter)
      birdtravel_var1[k]++;
    link_x_coord_spexit = bird_travel_x_hi[k] << 8 | bird_travel_x_lo[k];
    link_y_coord_spexit = bird_travel_y_hi[k] << 8 | bird_travel_y_lo[k];
    if (WorldMap_CalculateOamCoordinates(&pt))
      WorldMap_HandleSpriteBlink(15, 2, kOverworldMap_Table4[frame_counter >> 1 & 3], 0x6a, pt.x, pt.y);
  }

  if (save_ow_event_info[0x5b] & 0x20 || (((savegame_map_icons_indicator >= 6) ^ is_in_dark_world) & 1))
    goto out;

  k = savegame_map_icons_indicator;

  if (!OverworldMap_CheckForPendant(0) && !OverworldMap_CheckForCrystal(0) && !sign16(kOwMapCrystal0_x[k])) {
    link_x_coord_spexit = kOwMapCrystal0_x[k];
    link_y_coord_spexit = kOwMapCrystal0_y[k];
    uint8 t = kOwMapCrystal0_tab[k] >> 8;
    if (t != 0) {
      if (t != 100 && frame_counter & 0x10)
        goto endif_crystal0;
      link_x_coord_spexit -= 4, link_y_coord_spexit -= 4;
    }
    if (WorldMap_CalculateOamCoordinates(&pt)) {
      uint16 info = kOwMapCrystal0_tab[k];
      uint8 ext = 2;
      if (!(info >> 8))
        info = kOwMap_tab2[frame_counter >> 3 & 3] << 8 | 0x32, ext = 0;
      WorldMap_HandleSpriteBlink(14, ext, (uint8)info, (uint8)(info >> 8), pt.x, pt.y);
    }
  endif_crystal0:;
  }

  if (!OverworldMap_CheckForPendant(1) && !OverworldMap_CheckForCrystal(1) && !sign16(kOwMapCrystal1_x[k])) {
    link_x_coord_spexit = kOwMapCrystal1_x[k];
    link_y_coord_spexit = kOwMapCrystal1_y[k];
    uint8 t = kOwMapCrystal1_tab[k] >> 8;
    if (t != 0) {
      if (t != 100 && frame_counter & 0x10)
        goto endif_crystal1;
      link_x_coord_spexit -= 4, link_y_coord_spexit -= 4;
    }
    if (WorldMap_CalculateOamCoordinates(&pt)) {
      uint16 info = kOwMapCrystal1_tab[k];
      uint8 ext = 2;
      if (!(info >> 8))
        info = kOwMap_tab2[frame_counter >> 3 & 3] << 8 | 0x32, ext = 0;
      WorldMap_HandleSpriteBlink(13, ext, (uint8)info, (uint8)(info >> 8), pt.x, pt.y);
    }
  endif_crystal1:;
  }

  if (!OverworldMap_CheckForPendant(2) && !OverworldMap_CheckForCrystal(2) && !sign16(kOwMapCrystal2_x[k])) {
    link_x_coord_spexit = kOwMapCrystal2_x[k];
    link_y_coord_spexit = kOwMapCrystal2_y[k];
    uint8 t = kOwMapCrystal2_tab[k] >> 8;
    if (t != 0) {
      if (t != 100 && frame_counter & 0x10)
        goto endif_crystal2;
      link_x_coord_spexit -= 4, link_y_coord_spexit -= 4;
    }
    if (WorldMap_CalculateOamCoordinates(&pt)) {
      uint16 info = kOwMapCrystal2_tab[k];
      uint8 ext = 2;
      if (!(info >> 8))
        info = kOwMap_tab2[frame_counter >> 3 & 3] << 8 | 0x32, ext = 0;
      WorldMap_HandleSpriteBlink(12, ext, (uint8)info, (uint8)(info >> 8), pt.x, pt.y);
    }
  endif_crystal2:;
  }

  if (!OverworldMap_CheckForCrystal(3) && !sign16(kOwMapCrystal3_x[k])) {
    link_x_coord_spexit = kOwMapCrystal3_x[k];
    link_y_coord_spexit = kOwMapCrystal3_y[k];
    uint8 t = kOwMapCrystal3_tab[k] >> 8;
    if (t != 0) {
      if (t != 100 && frame_counter & 0x10)
        goto endif_crystal3;
      link_x_coord_spexit -= 4, link_y_coord_spexit -= 4;
    }
    if (WorldMap_CalculateOamCoordinates(&pt)) {
      uint16 info = kOwMapCrystal3_tab[k];
      uint8 ext = 2;
      if (!(info >> 8))
        info = kOwMap_tab2[frame_counter >> 3 & 3] << 8 | 0x32, ext = 0;
      WorldMap_HandleSpriteBlink(11, ext, (uint8)info, (uint8)(info >> 8), pt.x, pt.y);
    }
  endif_crystal3:;
  }

  if (!OverworldMap_CheckForCrystal(4) && !sign16(kOwMapCrystal4_x[k])) {
    link_x_coord_spexit = kOwMapCrystal4_x[k];
    link_y_coord_spexit = kOwMapCrystal4_y[k];
    uint8 t = kOwMapCrystal4_tab[k] >> 8;
    if (t != 0) {
      if (t != 100 && frame_counter & 0x10)
        goto endif_crystal4;
      link_x_coord_spexit -= 4, link_y_coord_spexit -= 4;
    }
    if (WorldMap_CalculateOamCoordinates(&pt)) {
      uint16 info = kOwMapCrystal4_tab[k];
      uint8 ext = 2;
      if (!(info >> 8))
        info = kOwMap_tab2[frame_counter >> 3 & 3] << 8 | 0x32, ext = 0;
      WorldMap_HandleSpriteBlink(10, ext, (uint8)info, (uint8)(info >> 8), pt.x, pt.y);
    }
  endif_crystal4:;
  }

  if (!OverworldMap_CheckForCrystal(5) && !sign16(kOwMapCrystal5_x[k])) {
    link_x_coord_spexit = kOwMapCrystal5_x[k];
    link_y_coord_spexit = kOwMapCrystal5_y[k];
    uint8 t = kOwMapCrystal5_tab[k] >> 8;
    if (t != 0) {
      if (t != 100 && frame_counter & 0x10)
        goto endif_crystal5;
      link_x_coord_spexit -= 4, link_y_coord_spexit -= 4;
    }
    if (WorldMap_CalculateOamCoordinates(&pt)) {
      uint16 info = kOwMapCrystal5_tab[k];
      uint8 ext = 2;
      if (!(info >> 8))
        info = kOwMap_tab2[frame_counter >> 3 & 3] << 8 | 0x32, ext = 0;
      WorldMap_HandleSpriteBlink(9, ext, (uint8)info, (uint8)(info >> 8), pt.x, pt.y);
    }
  endif_crystal5:;
  }

  if (!OverworldMap_CheckForCrystal(6) && !sign16(kOwMapCrystal6_x[k])) {
    link_x_coord_spexit = kOwMapCrystal6_x[k];
    link_y_coord_spexit = kOwMapCrystal6_y[k];
    uint8 t = kOwMapCrystal6_tab[k] >> 8;
    if (t != 0) {
      if (t != 100 && frame_counter & 0x10)
        goto endif_crystal6;
      link_x_coord_spexit -= 4, link_y_coord_spexit -= 4;
    }
    if (WorldMap_CalculateOamCoordinates(&pt)) {
      uint16 info = kOwMapCrystal6_tab[k];
      uint8 ext = 2;
      if (!(info >> 8))
        info = kOwMap_tab2[frame_counter >> 3 & 3] << 8 | 0x32, ext = 0;
      WorldMap_HandleSpriteBlink(8, ext, (uint8)info, (uint8)(info >> 8), pt.x, pt.y);
    }
  endif_crystal6:;
  }

out:
  link_x_coord_spexit = xbak;
  link_y_coord_spexit = ybak;
}

bool WorldMap_CalculateOamCoordinates(PointU8 *pt) {  // 8ac39f
  uint8 r14, r15;

  if (overworld_map_flags == 0) {
    int j = -(link_y_coord_spexit >> 4) + M7Y_copy + (link_y_coord_spexit >> 3 & 1) - 0xc0;
    uint8 t0 = kOverworldMap_tab1[j];
    r15 = 13 * t0 >> 4;

    uint8 at = link_x_coord_spexit >> 4;
    bool below = at < 0x80;
    at -= 0x80;
    if (sign8(at)) at = ~at;

    uint8 t1 = ((r15 < 224 ? r15 : 0) * 0x54 >> 8) + 0xb2;
    uint8 t2 = at * t1 >> 8;
    uint8 t3 = (below) ? 0x80 - t2 : t2 + 0x80;

    pt->x = t3 - BG1HOFS_copy2 + 0x80;
    pt->y = r15 + 12;
    return true;
  } else {
    uint16 t0 = -(link_y_coord_spexit >> 4) + M7Y_copy - 0x80;
    if (t0 >= 0x100)
      return false;
    uint16 t1 = t0 * 37 >> 4;
    if (t1 >= 333)
      return false;
    r15 = kOverworldMap_tab1[t1];
    uint16 t2 = link_x_coord_spexit;
    bool below = t2 < 0x7F8;
    t2 -= 0x7f8;
    if (sign16(t2))
      t2 = -t2;
    uint8 t3 = r15 < 226 ? r15 : 0;
    uint8 t4 = (t3 * 84 >> 8) + 178;  // r0
    uint8 t5 = (uint8)t2 * t4 >> 8; // r1
    uint16 t6 = (uint8)(t2 >> 8) * t4 + t5;
    uint16 t7 = (below) ? 0x800 - t6 : t6 + 0x800;
    bool below2 = t7 < 0x800;
    t7 -= 0x800;
    uint16 t8 = below2 ? -t7 : t7;
    uint8 t9 = (uint8)t8 * 45 >> 8;
    uint16 t10 = ((t8 >> 8) * 45) + t9;
    uint16 t11 = below2 ? 0x80 - t10 : t10 + 0x80;
    r14 = t11 - BG1HOFS_copy2;
    uint16 t12 = t11 - 0xFF80 - BG1HOFS_copy2;
    if (t12 >= 0x100)
      return false;
    pt->x = r14 + 0x81;
    pt->y = r15 + 16;
    return true;
  }
}

void WorldMap_HandleSpriteBlink(int spr, uint8 r11_ext, uint8 r12_flags, uint8 r13_char, uint8 r14_x, uint8 r15_y) {  // 8ac51c
  if (!(frame_counter & 0x10) && r13_char == 100) {
    assert(spr >= 8);
    r13_char = kOverworldMapData[spr - 8];
    r12_flags = 0x32;
    r11_ext = 0;
  } else {
    r14_x -= 4;
    r15_y -= 4;
  }
  bytewise_extended_oam[spr] = r11_ext;
  oam_buf[spr].x = r14_x;
  oam_buf[spr].y = r15_y;
  oam_buf[spr].charnum = r13_char;
  oam_buf[spr].flags = r12_flags;
}

bool OverworldMap_CheckForPendant(int k) {  // 8ac5a9
  return (savegame_map_icons_indicator == 3) && (link_which_pendants & kPendantBitMask[k]) != 0;
}

bool OverworldMap_CheckForCrystal(int k) {  // 8ac5c6
  return (savegame_map_icons_indicator == 7) && (link_has_crystals & kCrystalBitMask[k]) != 0;
}

void Module0E_03_DungeonMap() {  // 8ae0b0
  kDungMapSubmodules[overworld_map_state]();
}

void Module0E_03_01_DrawMap() {  // 8ae0dc
  kDungMapInit[dungmap_init_state]();
}

void Module0E_03_01_00_PrepMapGraphics() {  // 8ae0e4
  uint8 hdmaen_bak = HDMAEN_copy;
  zelda_snes_dummy_write(HDMAEN, 0);
  HDMAEN_copy = 0;
  mapbak_main_tile_theme_index = main_tile_theme_index;
  mapbak_sprite_graphics_index = sprite_graphics_index;
  mapbak_aux_tile_theme_index = aux_tile_theme_index;
  mapbak_TM = TM_copy;
  mapbak_TS = TS_copy;
  main_tile_theme_index = 32;
  sprite_graphics_index = 0x80 | BYTE(cur_palace_index_x2) >> 1;
  aux_tile_theme_index = 64;
  TM_copy = 0x16;
  TS_copy = 1;
  EraseTileMaps_dungeonmap();
  InitializeTilesets();
  overworld_palette_aux_or_main = 0x200;
  Palette_Load_DungeonMapBG();
  Palette_Load_DungeonMapSprite();
  hud_palette = 1;
  Palette_Load_HUD();
  LoadActualGearPalettes();
  flag_update_cgram_in_nmi++;
  dungmap_init_state++;
  HDMAEN_copy = hdmaen_bak;
  nmi_load_bg_from_vram = 9;
  nmi_disable_core_updates = 9;
}

void Module0E_03_01_01_DrawLEVEL() {  // 8ae1a4
  // Display FLOOR instead of MAP
  int i = kDungMap_Tab0[cur_palace_index_x2 >> 1] >> 1;
  if (i >= 0) {
    uint8 *dst = (uint8 *)&vram_upload_data[0];
    dst[32] = 0xff;
    WORD(dst[14   ]) = kDungMap_Tab1[i];
    WORD(dst[14+16]) = kDungMap_Tab2[i];
    for (int i = 13; i >= 0; i--) {
      dst[i] = kDungMap_Tab3[i];
      dst[i+16] = kDungMap_Tab4[i];
    }
    nmi_load_bg_from_vram = 1;
  }
  dungmap_init_state++;
}

void Module0E_03_01_02_DrawFloorsBackdrop() {  // 8ae1f3
  int offs = 0;
  uint16 t5 = kDungMap_Tab5[cur_palace_index_x2 >> 1];
  if (t5 & 0x100) {
    for (int i = 0; i < 21; i++)
      vram_upload_data[offs++] = kDungMap_Tab6[i];
    uint16 t = 0x1123;
    for (int i = 0; i < 16; i++, t += 0x20, offs += 3) {
      vram_upload_data[offs + 0] = swap16(t);
      vram_upload_data[offs + 1] = 0xE40;
      vram_upload_data[offs + 2] = 0x1B2E;
    }
  }
  int t7 = kDungMap_Tab7[(uint8)t5 >= 0x50 ? (((uint8)t5 >> 4) - 4) : (t5 & 0xf) >= 5 ? (t5 & 0xf) : 0], t7_org = t7;
  int j = 0;
  do {
    vram_upload_data[offs++] = swap16(t7);
    vram_upload_data[offs++] = 0xe40;
    vram_upload_data[offs++] = kDungMap_Tab8[j] + (t5 & 0x200 ? 0x400 : 0);
    j += (j != 6);
  } while (t7 += 0x20, t7 < 0x1360);
  vram_upload_offset = offs * 2;
  DungeonMap_BuildFloorListBoxes(t5, t7_org);
  ((uint8 *)vram_upload_data)[vram_upload_offset] = 0xff;
  dungmap_init_state++;
  nmi_load_bg_from_vram = 1;
}

void DungeonMap_BuildFloorListBoxes(uint8 t5, uint16 r14) {  // 8ae2f5
  int n = (t5 & 0xf) + (t5 >> 4);
  uint8 r12 = dung_cur_floor + (t5 & 0xf);
  r14 -= 0x40 - 2;
  r14 += (t5 & 0xf) * 0x40;
  int offs = vram_upload_offset >> 1;
  int i = 0;
  do {
    int x = 0;
loop2:
    vram_upload_data[offs++] = swap16(r14);
    vram_upload_data[offs++] = 0x700;
    do {
      vram_upload_data[offs++] = kDungMap_Tab9[x++];
      if (x == 4) {
        r14 += 0x20;
        goto loop2;
      }
    } while (x != 8);

    r14 -= 0x40 + 0x20;
  } while (++i < n);
  vram_upload_offset = offs * 2;
}

void Module0E_03_01_03_DrawRooms() {  // 8ae384
  dungmap_var2 = 0;
  dungmap_idx = 0;
  uint8 t = -(kDungMap_Tab5[cur_palace_index_x2 >> 1] & 0xf);
  if (WORD(dung_cur_floor) != t) {
    dungmap_cur_floor = dung_cur_floor;
  } else {
    dungmap_cur_floor = WORD(dung_cur_floor) + 1;
    dungmap_idx += 2;
  }
  DungeonMap_DrawFloorNumbersByRoom(0, ~0x1000);
  DungeonMap_DrawBorderForRooms(0, ~0x1000);
  DungeonMap_DrawDungeonLayout(0);
  BYTE(dungmap_cur_floor)--;
  DungeonMap_DrawFloorNumbersByRoom(0x300, ~0x1000);
  DungeonMap_DrawBorderForRooms(0x300, ~0x1000);
  DungeonMap_DrawDungeonLayout(0x300);
  dungmap_cur_floor++;
  WORD(g_ram[6]) = 0;
  WORD(g_ram[10]) = 0;
  nmi_subroutine_index = 8;
  BYTE(nmi_load_target_addr) = 0x22;
  dungmap_init_state++;
}

void DungeonMap_DrawBorderForRooms(uint16 pd, uint16 mask) {  // 8ae449
  for (int i = 0; i != 4; i++)
    messaging_buf[((kDungMap_Tab10[i] + pd) & 0xfff) >> 1] = kDungMap_Tab11[i] & mask;
  for (int i = 0; i != 2; i++) {
    int r4 = kDungMap_Tab12[i] + pd;
    for (int j = 0; j != 20; j+=2)
      messaging_buf[((r4 + j) & 0xfff) >> 1] = kDungMap_Tab13[i] & mask;
  }

  for (int i = 0; i != 2; i++) {
    int r4 = kDungMap_Tab14[i] + pd;
    for (int j = 0; j != 0x280; j+=0x40)
      messaging_buf[((r4 + j) & 0xfff) >> 1] = kDungMap_Tab15[i] & mask;
  }
}

void DungeonMap_DrawFloorNumbersByRoom(uint16 pd, uint16 r8) {  // 8ae4f9
  uint16 p = 0xDE;
  do {
    int t = ((p + pd) & 0xfff) >> 1;
    messaging_buf[t] = 0xf00;
    messaging_buf[t+1] = 0xf00;
  } while (p += 0x40, p != 0x39e);
  int t = ((0x35e + pd) & 0xfff) >> 1;
  uint16 q1 = (dungmap_cur_floor & 0x80) ? 0x1F1C : kDungMap_Tab16[dungmap_cur_floor & 0xf];
  uint16 q2 = (dungmap_cur_floor & 0x80) ? kDungMap_Tab16[(uint8)~dungmap_cur_floor] : 0x1F1D;
  messaging_buf[t+0] = q1 & r8;
  messaging_buf[t+1] = q2 & r8;
}

void DungeonMap_DrawDungeonLayout(int pd) {  // 8ae579
  for (int i = 0; i < 5; i++)
    DungeonMap_DrawSingleRowOfRooms(i, ((292 + 128 * i + pd) & 0xfff) >> 1);
}

void DungeonMap_DrawSingleRowOfRooms(int i, int arg_x) {  // 8ae5bc
  uint16 t5 = kDungMap_Tab5[cur_palace_index_x2 >> 1];
  int dungmask = kUpperBitmasks[cur_palace_index_x2 >> 1];

  for (int j = 0; j < 5; j++, arg_x += 2) {
    int r14 = (uint8)(dungmap_cur_floor + (t5 & 0xf));
    const uint8 *curp = GetDungmapFloorLayout();
    uint8 v = curp[r14 * 25 + i * 5 + j];
    uint16 yv, av;
    if (v == 0xf) {
      yv = 0x51;
    } else {
      r14 = save_dung_info[v] & 0xf;
      int k = 0, count = 0;
      for(; curp[k] != v; k++)
        count += (curp[k] != 0xf);
      yv = GetOtherDungmapInfo(count);
    }

    uint16 r12 = kDungMap_Tab23[yv * 4 + 0], r12_org = r12;
    if (r12 != 0xB00 && (r14 & 8) == 0) {
      if (!(r12 & 0x1000)) {
        r12 = 0x400;
      } else if (link_dungeon_map & dungmask) {
        av = (r12 & ~0x1c00) | 0xc00;
        goto write_3;
      } else {
        r12 = 0;
      }
    } else {
      r12 = 0;
    }
    av = ((link_dungeon_map & dungmask) || (r14 & 8)) ? r12 + r12_org : 0xb00;
  write_3:
    messaging_buf[arg_x] = av;

    r12 = kDungMap_Tab23[yv * 4 + 1], r12_org = r12;
    if (r12 != 0xB00 && (r14 & 4) == 0) {
      if (!(r12 & 0x1000)) {
        r12 = 0x400;
      } else if (link_dungeon_map & dungmask) {
        av = (r12 & ~0x1c00) | 0xc00;
        goto write_4;
      } else {
        r12 = 0;
      }
    } else {
      r12 = 0;
    }
    av = ((link_dungeon_map & dungmask) || (r14 & 4)) ? r12 + r12_org : 0xb00;
  write_4:
    messaging_buf[arg_x + 1] = av;

    r12 = kDungMap_Tab23[yv * 4 + 2], r12_org = r12;
    if (r12 != 0xB00 && (r14 & 2) == 0) {
      if (!(r12 & 0x1000)) {
        r12 = 0x400;
      } else if (link_dungeon_map & dungmask) {
        av = (r12 & ~0x1c00) | 0xc00;
        goto write_5;
      } else {
        r12 = 0;
      }
    } else {
      r12 = 0;
    }
    av = ((link_dungeon_map & dungmask) || (r14 & 2)) ? r12 + r12_org : 0xb00;
  write_5:
    messaging_buf[arg_x + 32] = av;

    r12 = kDungMap_Tab23[yv * 4 + 3], r12_org = r12;
    if (r12 != 0xB00 && (r14 & 1) == 0) {
      if (!(r12 & 0x1000)) {
        r12 = 0x400;
      } else if (link_dungeon_map & dungmask) {
        av = (r12 & ~0x1c00) | 0xc00;
        goto write_6;
      } else {
        r12 = 0;
      }
    } else {
      r12 = 0;
    }
    av = ((link_dungeon_map & dungmask) || (r14 & 1)) ? r12 + r12_org : 0xb00;
  write_6:
    messaging_buf[arg_x + 33] = av;
  }
}

void DungeonMap_DrawRoomMarkers() {  // 8ae823
  int dung = cur_palace_index_x2 >> 1;
  uint8 t5 = (kDungMap_Tab5[dung] & 0xf);
  uint8 floor1 = t5 + dung_cur_floor;

  uint16 room = dungeon_room_index;
  for (int i = 0; i != 3; i++) {
    if (room == kDungMap_Tab21[i])
      room = kDungMap_Tab22[i];
  }
  const uint8 *roomp = GetDungmapFloorLayout();
  const uint8 *curp = &roomp[floor1 * 25];
  int i;

  uint8 xcoord = 0, ycoord = 0;
  for(i = 0; i < 25 && *curp++ != (uint8)room; i++) {
    if (xcoord < 64)
      xcoord += 16;
    else
      xcoord = 0, ycoord += 16;
  }
  dungmap_var3 = xcoord + 0x90;
  dungmap_var3 += (link_x_coord & 0x1e0) >> 5;

  dungmap_var6 = ycoord;

  dungmap_var5 = ycoord + kDungMap_Tab24[dungmap_idx >> 1];
  dungmap_var5 += (link_y_coord & 0x1e0) >> 5;

  uint8 floor2 = t5 + kDungMap_Tab28[dung];
  curp = &roomp[floor2 * 25];

  dungmap_var8 = dungmap_var7 = 0x40;

  uint8 lookfor = kDungMap_Tab25[dung];
  for (int j = 24; j >= 0; j--) {
    if (curp[j] != 0xf && curp[j] == lookfor)
      break;
    if ((int16)(dungmap_var7 -= 0x10) < 0) {
      dungmap_var7 = 0x40;
      BYTE(dungmap_var8) -= 0x10;
    }
  }

  int8 floor3 = dungmap_cur_floor - kDungMap_Tab28[dung];
  dungmap_var8 += 0x60 * floor3;
  dungmap_var8 += kDungMap_Tab24[0];
  overworld_map_state++;
  INIDISP_copy = 0;
  dungmap_init_state = 0;
}

void DungeonMap_HandleInputAndSprites() {  // 8ae954
  DungeonMap_HandleInput();
  DungeonMap_DrawSprites();
}

void DungeonMap_HandleInput() {  // 8ae95b
  if (filtered_joypad_L & 0x40) {
    overworld_map_state += 2;
    dungmap_init_state = 0;
  } else {
    DungeonMap_HandleMovementInput();
  }
}

void DungeonMap_HandleMovementInput() {  // 8ae979
  DungeonMap_HandleFloorSelect();
  if (dungmap_var2)
    DungeonMap_ScrollFloors();
}

void DungeonMap_HandleFloorSelect() {  // 8ae986
  uint8 r2 = (kDungMap_Tab5[cur_palace_index_x2 >> 1] >> 4 & 0xf);
  uint8 r3 = (kDungMap_Tab5[cur_palace_index_x2 >> 1] & 0xf);
  uint8 yv = 7;
  if (r2 + r3 < 3 || dungmap_var2 || !(joypad1H_last & 0xc))
    return;
  dungmap_cur_floor &= 0xff;
  uint16 r6 = WORD(g_ram[6]);
  if (joypad1H_last & 8) {
    if (r2 - 1 == dungmap_cur_floor)
      return;
    dungmap_cur_floor++;
    r6 = (r6 - 0x300) & 0xfff;
  } else {
    if ((uint8)(-r3 + 1) == dungmap_cur_floor)
      return;
    dungmap_cur_floor -= 2;
    r6 = (r6 + 0x600) & 0xfff;
  }
  DungeonMap_DrawFloorNumbersByRoom(r6, ~0x1000);
  DungeonMap_DrawBorderForRooms(r6, ~0x1000);
  DungeonMap_DrawDungeonLayout(r6);
  dungmap_var2++;
  WORD(g_ram[10]) = joypad1H_last;
  int x = joypad1H_last >> 3 & 1;
  dungmap_var4 = BG2VOFS_copy2 + kDungMap_Tab26[x];
  if (!x) {
    r6 = (r6 - 0x300) & 0xfff;
    dungmap_cur_floor++;
  }
  WORD(g_ram[6]) = r6;
  nmi_subroutine_index = 8;
}

void DungeonMap_ScrollFloors() {  // 8aea7f
  int x = WORD(g_ram[10]) >> 3 & 1;
  dungmap_var5 += kDungMap_Tab39[x];
  dungmap_var8 += kDungMap_Tab39[x];
  BG2VOFS_copy2 += kDungMap_Tab40[x];
  if (BG2VOFS_copy2 == dungmap_var4)
    dungmap_var2 = 0;
}

void DungeonMap_DrawSprites() {  // 8aeab2
  int dung = cur_palace_index_x2 >> 1;
  uint8 r2 = (kDungMap_Tab5[dung] & 0xf);
  uint8 floor = r2 + dung_cur_floor;

  int spr_pos = 0;
  uint8 r14 = 0;
  DungeonMap_DrawLinkPointing(spr_pos++, r2, floor);
  do {
    spr_pos = DungeonMap_DrawLocationMarker(spr_pos, r14);
    r14 += 1;
  } while (spr_pos != 9);
  spr_pos = DungeonMap_DrawBlinkingIndicator(spr_pos);
  spr_pos = DungeonMap_DrawBossIcon(spr_pos);
  spr_pos = DungeonMap_DrawFloorNumberObjects(spr_pos);
  DungeonMap_DrawFloorBlinker();
}

void DungeonMap_DrawLinkPointing(int spr_pos, uint8 r2, uint8 r3) {  // 8aeaf0
  int dung = cur_palace_index_x2 >> 1;
  uint8 t5 = kDungMap_Tab5[dung];
  if (4 - r2 >= 0) {
    r3 += 4 - r2;
    int8 a = (t5 >> 4) - 4;
    if (a >= 0)
      r3 -= a;
  }
  bytewise_extended_oam[spr_pos] = 2;
  oam_buf[spr_pos].x = 0x19;
  oam_buf[spr_pos].y = kDungMap_Tab33[r3] - 4;
  oam_buf[spr_pos].charnum = 0;
  oam_buf[spr_pos].flags = overworld_palette_swap_flag ? 0x30 : 0x3e;
}

int DungeonMap_DrawBlinkingIndicator(int spr_pos) {  // 8aeb50
  bytewise_extended_oam[spr_pos] = 0;
  oam_buf[spr_pos].x = dungmap_var3 - 3;
  oam_buf[spr_pos].y = ((dungmap_var5 < 256) ? dungmap_var5 : 0xf0) - 3;
  oam_buf[spr_pos].charnum = 0x34;
  oam_buf[spr_pos].flags = kDungMap_Tab38[frame_counter >> 2 & 3];
  return spr_pos + 1;
}

int DungeonMap_DrawLocationMarker(int spr_pos, uint16 r14) {  // 8aeba8
  for (int i = 3; i >= 0; i--, spr_pos++) {
    bytewise_extended_oam[spr_pos] = 2;
    oam_buf[spr_pos].x = kDungMap_Tab29[i] + (dungmap_var3 & 0xf0);
    uint8 r15 = dungmap_var6 + kDungMap_Tab24[r14];
    oam_buf[spr_pos].y = r15 + kDungMap_Tab30[i];
    oam_buf[spr_pos].charnum = 0;
    int fr = (frame_counter >> 2) & 1;
    if ((dungmap_var5 + 1 & 0xf0) == ++r15 && dungmap_var5 < 256)
      fr += 2;
    oam_buf[spr_pos].flags = kDungMap_Tab32[fr] | kDungMap_Tab31[i];
  }
  return spr_pos;
}

int DungeonMap_DrawFloorNumberObjects(int spr_pos) {  // 8aec0a
  uint8 r2 = (kDungMap_Tab5[cur_palace_index_x2 >> 1] >> 4 & 0xf);
  uint8 r3 = (kDungMap_Tab5[cur_palace_index_x2 >> 1] & 0xf);
  uint8 yv = 7;
  if (r2 + r3 != 8 && r2 < 4) {
    yv = 6;
    for (int i = 3; i != 0 && i != r2; i--)
      yv--;
    if (r3 >= 5) {
      for (int i = 5; i != r3 && r3 != 8; i++)
        yv++;
    }
  }

  uint8 r4 = kDungMap_Tab33[yv] + 1;
  r2--;
  r3 = -r3;
  do {
    bytewise_extended_oam[spr_pos+0] = 0;
    bytewise_extended_oam[spr_pos+1] = 0;
    oam_buf[spr_pos + 0].x = 0x30;
    oam_buf[spr_pos + 1].x = 0x38;
    oam_buf[spr_pos + 0].y = r4;
    oam_buf[spr_pos + 1].y = r4;
    r4 += 16;
    oam_buf[spr_pos + 0].flags = 0x3d;
    oam_buf[spr_pos + 1].flags = 0x3d;
    oam_buf[spr_pos + 0].charnum = sign8(r2) ? 0x1c : kDungMap_Tab34[r2];
    oam_buf[spr_pos + 1].charnum = sign8(r2) ? kDungMap_Tab34[r2 ^ 0xff] : 0x1d;
  } while (spr_pos += 2, r2-- != r3);
  return spr_pos;
}

void DungeonMap_DrawFloorBlinker() {  // 8aeccf
  uint8 floor = dungmap_cur_floor;
  uint8 t5 = kDungMap_Tab5[cur_palace_index_x2 >> 1];
  uint8 flag = ((t5 >> 4 & 0xf) + (t5 & 0xf) != 1);
  floor -= flag;
  uint8 r0;
  uint8 i = flag;
  do {
    r0 = floor + (t5 & 0xf);
    int8 a = 4 - (t5 & 0xf);
    if (a >= 0) {
      r0 += a;
      a = (t5 >> 4) - 4;
      if (a >= 0)
        r0 -= a;
    }
    floor += 1;
  } while (i--);
  if (!(frame_counter & 0x10))
    return;
  uint8 y = kDungMap_Tab33[r0] - 4;
  do {
    uint8 x = 40;
    int spr_pos = 0x40 + kDungMap_Tab35[flag];
    for (int i = 3; i >= 0; i--, spr_pos++) {
      bytewise_extended_oam[spr_pos+0] = 0;
      bytewise_extended_oam[spr_pos+4] = 0;
      oam_buf[spr_pos + 0].x = x;
      oam_buf[spr_pos + 4].x = x;
      oam_buf[spr_pos + 0].y = y + flag * 16;
      oam_buf[spr_pos + 4].y = y + flag * 16 + 8;
      oam_buf[spr_pos + 0].charnum = kDungMap_Tab36[i];
      oam_buf[spr_pos + 4].charnum = kDungMap_Tab36[i];
      uint8 t = 0x3d | (i ? 0 : 0x40);
      oam_buf[spr_pos + 0].flags = t;
      oam_buf[spr_pos + 4].flags = t | 0x80;
      x += 8;
    }
  } while (flag--);
}

int DungeonMap_DrawBossIcon(int spr_pos) {  // 8aede4
  int dung = cur_palace_index_x2 >> 1;
  if (save_dung_info[kDungMap_Tab25[dung]] & 0x800 || !(link_compass & kUpperBitmasks[dung]) || kDungMap_Tab28[dung] < 0)
    return spr_pos;
  spr_pos = DungeonMap_DrawBossIconByFloor(spr_pos);
  if ((frame_counter & 0xf) >= 10)
    return spr_pos;
  bytewise_extended_oam[spr_pos] = 0;
  uint16 xy = kDungMap_Tab37[dung];
  oam_buf[spr_pos].x = (xy >> 8) + dungmap_var7 + 0x90;
  oam_buf[spr_pos].y = (dungmap_var8 < 256) ? xy + dungmap_var8 : 0xf0;
  oam_buf[spr_pos].charnum = 0x31;
  oam_buf[spr_pos].flags = 0x33;
  return spr_pos + 1;
}

int DungeonMap_DrawBossIconByFloor(int spr_pos) {  // 8aee95
  int dung = cur_palace_index_x2 >> 1;
  uint8 t5 = kDungMap_Tab5[dung];
  uint8 r2 = t5 & 0xf;
  uint8 r3 = r2 + kDungMap_Tab28[dung];
  if (4 - r2 >= 0) {
    r3 += 4 - r2;
    int8 a = (t5 >> 4) - 4;
    if (a >= 0)
      r3 -= a;
  }
  if ((frame_counter & 0xf) >= 10)
    return spr_pos;
  bytewise_extended_oam[spr_pos] = 0;
  uint16 xy = kDungMap_Tab37[dung];
  oam_buf[spr_pos].x = 0x4C;
  oam_buf[spr_pos].y = kDungMap_Tab33[r3];
  oam_buf[spr_pos].charnum = 0x31;
  oam_buf[spr_pos].flags = 0x33;
  return spr_pos + 1;
}

void DungeonMap_RecoverGFX() {  // 8aef19
  uint8 hdmaen_bak = HDMAEN_copy;
  zelda_snes_dummy_write(HDMAEN, 0);
  HDMAEN_copy = 0;
  EraseTileMaps_normal();

  TM_copy = mapbak_TM;
  TS_copy = mapbak_TS;
  main_tile_theme_index = mapbak_main_tile_theme_index;
  sprite_graphics_index = mapbak_sprite_graphics_index;
  aux_tile_theme_index = mapbak_aux_tile_theme_index;
  InitializeTilesets();
  overworld_palette_aux_or_main = 0;
  hud_palette = 0;
  Hud_Rebuild();

  overworld_screen_transition = 0;
  dung_cur_quadrant_upload = 0;
  do {
    WaterFlood_BuildOneQuadrantForVRAM();
    NMI_UploadTilemap();
    Dungeon_PrepareNextRoomQuadrantUpload();
    NMI_UploadTilemap();
  } while (dung_cur_quadrant_upload != 0x10);

  nmi_subroutine_index = 0;
  subsubmodule_index = 0;
  HDMAEN_copy = hdmaen_bak;

  memcpy(main_palette_buffer, mapbak_palette, sizeof(uint16) * 256);
  COLDATA_copy0 |= overworld_fixed_color_plusminus;
  COLDATA_copy1 |= overworld_fixed_color_plusminus;
  COLDATA_copy2 |= overworld_fixed_color_plusminus;

  sound_effect_2 = 16;
  music_control = 0xf3;
  RecoverPegGFXFromMapping();
  flag_update_cgram_in_nmi++;
  overworld_map_state++;
  INIDISP_copy = 0;
  nmi_disable_core_updates = 0;
}

void ToggleStarTilesAndAdvance() {  // 8aefc9
  Dungeon_RestoreStarTileChr();
  overworld_map_state++;
}

void Death_InitializeGameOverLetters() {  // 8afe20
  flag_for_boomerang_in_place = 0;
  for (int i = 0; i < 8; i++) {
    ancilla_x_lo[i] = 0xb0;
    ancilla_x_hi[i] = 0;
  }
  ancilla_type[0] = 1;
  hookshot_effect_index = 6;
}

void CopySaveToWRAM() {  // 8ccfbb
  int k = 0xf;
  bird_travel_x_hi[k] = 0;
  bird_travel_y_hi[k] = 0;
  bird_travel_x_lo[k] = 0;
  bird_travel_y_lo[k] = 0;
  birdtravel_var1[k] = 0;

  memcpy(save_dung_info, &g_zenv.sram[WORD(g_ram[0])], 0x500);

  bg_tile_animation_countdown = 7;
  word_7EC013 = 7;
  word_7EC00F = 0;
  word_7EC015 = 0;
  word_7E0219 = 0x6040;
  word_7E021D = 0x4841;
  word_7E021F = 0x7f;
  word_7E0221 = 0xffff;

  hud_var1 = 128;
  main_module_index = 5;
  submodule_index = 0;
  which_entrance = 0;
  nmi_disable_core_updates = 0;
  hud_palette = 0;
}

void RenderText() {  // 8ec440
  kMessaging_Text[messaging_module]();
}

void RenderText_PostDeathSaveOptions() {  // 8ec455
  dialogue_message_index = 3;
  Text_Initialize_initModuleStateLoop();
  text_msgbox_topleft = 0x61e8;
  text_render_state = 2;
  for (int i = 0; i < 5; i++)
    Text_Render();
}

void Text_Initialize() {  // 8ec483
  if (main_module_index == 20)
    ResetHUDPalettes4and5();
  Attract_DecompressStoryGFX();
  Text_Initialize_initModuleStateLoop();
}

void Text_Initialize_initModuleStateLoop() {  // 8ec493
  memcpy(&text_msgbox_topleft_copy, kText_InitializationData, 32);
  Text_InitVwfState();
  RenderText_SetDefaultWindowPosition();
  text_tilemap_cur = 0x3980;
  Text_LoadCharacterBuffer();
  RenderText_Draw_EmptyBuffer();
  dialogue_msg_dst_offs = 0;
  nmi_subroutine_index = 2;
  nmi_disable_core_updates = 2;
}

void Text_InitVwfState() {  // 8ec4c9
  vwf_curline = 0;
  vwf_flag_next_line = 0;
  vwf_var1 = 0;
  vwf_line_ptr = 0;
}

void Text_LoadCharacterBuffer() {  // 8ec4e2
  const uint8 *src = GetCurrentTextPtr(), *src_org = src;
  uint8 *dst = messaging_text_buffer;
  dst[0] = dst[1] = 0x7f;
  dialogue_msg_dst_offs = 0;
  dialogue_msg_src_offs = 0;
  for (;;) {
    uint8 c = *src++;
    if (!(c & 0x80)) {
      switch (c) {
      case 0x67 + 3: dst = Text_WritePlayerName(dst); break;
      case 0x67 + 4:  // RenderText_ExtendedCommand_SetWindowType
        text_render_state = *src++;
        break;
      case 0x67 + 5: {  // Text_WritePreloadedNumber
        uint8 t = *src++;
        uint8 v = byte_7E1CF2[t >> 1];
        *dst++ = 0x34 + ((t & 1) ? v >> 4 : v & 0xf);
        break;
      }
      case 0x67 + 6:
        text_msgbox_topleft = kText_Positions[*src++];
        break;
      case 0x67 + 16:
        text_tilemap_cur = ((0x387F & 0xe300) | 0x180) | (*src++ << 10) & 0x3c00;
        break;
      case 0x67 + 7:
      case 0x67 + 17:
      case 0x67 + 18:
      case 0x67 + 19:
        *dst++ = c;
        *dst++ = *src++;
        break;
      case 0x7f:
        dialogue_msg_dst_offs = dst - messaging_text_buffer;
        dialogue_msg_src_offs = src - src_org - 1;
        *dst = 0x7f;
        return; // done
      default:
        *dst++ = c;
        break;
      }
    } else {
      // dictionary
      c -= 0x88;
      int idx = kTextDictionary_Idx[c], num = kTextDictionary_Idx[c + 1] - idx;
      memcpy(dst, &kTextDictionary[idx], num);
      dst += num;
    }
  }
}

uint8 *Text_WritePlayerName(uint8 *p) {  // 8ec5b3
  uint8 slot = srm_var1;
  int offs = ((slot>>1) - 1) * 0x500;
  for (int i = 0; i < 6; i++) {
    uint8 *pp = &g_zenv.sram[0x3d9 + offs + i * 2];
    uint16 a = WORD(*pp);
    p[i] = Text_FilterPlayerNameCharacters(a & 0xf | (a >> 1) & 0xf0);
  }
  int i = 6;
  while (i && p[i - 1] == 0x59)
    i--;
  return p + i;
}

uint8 Text_FilterPlayerNameCharacters(uint8 a) {  // 8ec639
  if (a >= 0x5f) {
    if (a >= 0x76)
      a -= 0x42;
    else if (a == 0x5f)
      a = 8;
    else if (a == 0x60)
      a = 0x22;
    else if (a == 0x61)
      a = 0x3e;
  }
  return a;
}

void Text_Render() {  // 8ec8d9
  kText_Render[text_render_state]();
}

void RenderText_Draw_Border() {  // 8ec8ea
  RenderText_DrawBorderInitialize();
  uint16 *d = RenderText_DrawBorderRow(vram_upload_data, 0);
  for(int i = 0; i != 6; i++)
    d = RenderText_DrawBorderRow(d, 6);
  d = RenderText_DrawBorderRow(d, 12);
  nmi_load_bg_from_vram = 1;
  text_render_state = 2;
}

void RenderText_Draw_BorderIncremental() {  // 8ec919
  nmi_load_bg_from_vram = 1;
  uint8 a = text_incremental_state;
  uint16 *d = vram_upload_data;
  if (a)
    a = (a < 7) ? 1 : 2;
  switch (a) {
  case 0:
    RenderText_DrawBorderInitialize();
    d = RenderText_DrawBorderRow(d, 0);
    text_incremental_state++;
    break;
  case 1:
    d = RenderText_DrawBorderRow(d, 6);
    text_incremental_state++;
    break;
  case 2:
    text_render_state = 2;
    d = RenderText_DrawBorderRow(d, 12);
    text_incremental_state++;
    break;
  }
}

void RenderText_Draw_CharacterTilemap() {  // 8ec97d
  Text_BuildCharacterTilemap();
  text_render_state++;
}

void RenderText_Draw_MessageCharacters() {  // 8ec984
restart:
  if (dialogue_msg_src_offs >= 99) {
    dialogue_msg_src_offs = 0;
    text_next_position = 0;
  } else if (dialogue_msg_src_offs >= 59 && dialogue_msg_src_offs < 80) {
    dialogue_msg_src_offs = 0x50;
    text_next_position = 0;
  } else if (dialogue_msg_src_offs >= 19 && dialogue_msg_src_offs < 40) {
    dialogue_msg_src_offs = 0x28;
    text_next_position = 0;
  }
  if ((dialogue_msg_src_offs == 18 || dialogue_msg_src_offs == 58 || dialogue_msg_src_offs == 98) && (text_next_position & 7) >= 6) {
    dialogue_msg_src_offs++;
    goto restart;
  }
  int t = (messaging_text_buffer[dialogue_msg_dst_offs] & 0x7f) - 0x66;
  if (t < 0)
    t = 0;
  switch (t) {
  case 0:  // RenderText_Draw_RenderCharacter
    switch (vwf_line_mode < 2 ? vwf_line_mode : 2) {
    case 0:  // RenderText_Draw_RenderCharacter_All
      RenderText_Draw_RenderCharacter_All();
      break;
    case 1:  // VWF_RenderSingle
      VWF_RenderSingle();
      break;
    default:
      vwf_line_mode--;
      break;
    }
    break;
  case 1:  // RenderText_Draw_NextImage
    if (main_module_index == 20) {
      PaletteFilterHistory();
      if (!BYTE(palette_filter_countdown))
        dialogue_msg_dst_offs++;
    } else {
      dialogue_msg_dst_offs++;
    }
    break;
  case 2:  // RenderText_Draw_Choose2LowOr3
    RenderText_Draw_Choose2LowOr3();
    break;
  case 3:  // RenderText_Draw_ChooseItem
    RenderText_Draw_ChooseItem();
    break;
  case 4:  //
  case 5:  //
  case 6:  //
  case 7:  //
  case 8:  // RenderText_Draw_Ignore
    byte_7E1CEA = messaging_text_buffer[dialogue_msg_dst_offs + 1];
    dialogue_msg_dst_offs += 2;
    break;
  case 9:   // RenderText_Draw_Choose2HiOr3
    RenderText_Draw_Choose2HiOr3();
    break;
  case 10:  //
    assert(0);
    break;
  case 11:  // RenderText_Draw_Choose3
    RenderText_Draw_Choose3();
    break;
  case 12:  // RenderText_Draw_Choose1Or2
    RenderText_Draw_Choose1Or2();
    break;
  case 13:  // RenderText_Draw_Scroll
    RenderText_Draw_Scroll();
    break;
  case 14:  //
  case 15:  //
  case 16:  // VWF_SetLine
    dialogue_msg_src_offs = kVWF_LinePositions[(t + 2) & 3];
    vwf_curline = kVWF_RowPositions[(t + 2) & 3];
    vwf_flag_next_line = 1;
    dialogue_msg_dst_offs++;
    text_next_position = 0;
    break;
  case 17:  // RenderText_Draw_SetColor
    byte_7E1CDC &= ~0x1c;
    byte_7E1CDC |= (messaging_text_buffer[dialogue_msg_dst_offs + 1] & 7) << 2;
    dialogue_msg_dst_offs += 2;
    break;
  case 18:  // RenderText_Draw_Wait
    switch (joypad1L_last & 0x80 ? 1 : text_wait_countdown >= 2 ? 2 : text_wait_countdown) {
    case 0:
      text_wait_countdown = kText_WaitDurations[messaging_text_buffer[dialogue_msg_dst_offs + 1] & 0xf] - 1;
      break;
    case 1:
      dialogue_msg_dst_offs += 2;
      BYTE(text_wait_countdown) = 0;
      break;
    case 2:
      text_wait_countdown--;
      break;
    }
    break;
  case 19:  // RenderText_Draw_PlaySfx
    sound_effect_2 = messaging_text_buffer[dialogue_msg_dst_offs + 1];
    dialogue_msg_dst_offs += 2;
    break;
  case 20:  // RenderText_Draw_SetSpeed
    vwf_line_speed = vwf_line_mode = messaging_text_buffer[dialogue_msg_dst_offs + 1];
    dialogue_msg_dst_offs += 2;
    break;
  case 21:  // RenderText_Draw_Command7B
    RenderText_Draw_Command7B();
    break;
  case 22:  // RenderText_Draw_ABunchOfSpaces
    RenderText_Draw_ABunchOfSpaces();
    break;
  case 23:  // RenderText_Draw_EmptyBuffer
    RenderText_Draw_EmptyBuffer();
    break;
  case 24:  // RenderText_Draw_PauseForInput
    if (text_wait_countdown2 != 0) {
      if (--text_wait_countdown2 == 1)
        sound_effect_2 = 36;
    } else {
      if ((filtered_joypad_H | filtered_joypad_L) & 0xc0) {
        dialogue_msg_dst_offs++;
        text_wait_countdown2 = 28;
      }
    }
    break;
  case 25:  // RenderText_Draw_Terminate
    if (text_wait_countdown2 != 0) {
      if (--text_wait_countdown2 == 1)
        sound_effect_2 = 36;
    } else {
      if ((filtered_joypad_H | filtered_joypad_L)) {
        text_render_state = 4;
        text_wait_countdown2 = 28;
      }
    }
    break;
  }
  nmi_subroutine_index = 2;
  nmi_disable_core_updates = 2;
}

void RenderText_Draw_Finish() {  // 8eca35
  RenderText_DrawBorderInitialize();
  uint16 *d = vram_upload_data;
  d[0] = swap16(text_msgbox_topleft_copy);
  d[1] = 0x2E42;
  d[2] = 0x387F;
  d[3] = 0xffff;
  nmi_load_bg_from_vram = 1;
  messaging_module = 0;
  submodule_index = 0;
  main_module_index = saved_module_for_menu;
}

void RenderText_Draw_RenderCharacter_All() {  // 8eca99
  VWF_RenderSingle();
  if (dialogue_msg_src_offs != 19 && dialogue_msg_src_offs != 59 && dialogue_msg_src_offs != 99)
    RenderText_Draw_MessageCharacters();
}

void VWF_RenderSingle() {  // 8ecab8
  uint8 t = messaging_text_buffer[dialogue_msg_dst_offs];
  if (t != 0x59)
    sound_effect_2 = 12;
  VWF_RenderCharacter();
  vwf_line_mode = vwf_line_speed;
}

void VWF_RenderCharacter() {  // 8ecb5e
  if (vwf_flag_next_line) {
    vwf_line_ptr = kVWF_RenderCharacter_renderPos[vwf_curline>>1];
    vwf_var1 = kVWF_RenderCharacter_linePositions[vwf_curline>>1];
    vwf_flag_next_line = 0;
  }
  uint8 c = messaging_text_buffer[dialogue_msg_dst_offs];
  uint8 width = kVWF_RenderCharacter_widths[c];
  int i = vwf_var1++;
  uint8 arrval = vwf_arr[i];
  vwf_arr[i + 1] = arrval + width;
  uint16 r10 = (c & 0x70) * 2 + (c & 0xf);
  uint16 r0 = arrval * 2;
  const uint16 *const kTextBits = GetFontPtr();
  const uint16 *src2 = kTextBits + r10 * 8;
  uint8 *mbuf = (uint8 *)messaging_buf;
  for (int i = 0; i != 16; i += 2) {
    uint16 r4 = *src2++;
    int y = r0 + vwf_line_ptr;
    int x = (y & 0xff0) + i;
    y = (y >> 1) & 7;
    uint8 r3 = width;
    do {
      if (r4 & 0x0080)
        mbuf[x + 0] ^= kVWF_RenderCharacter_setMasks[y];
      else
        mbuf[x + 0] &= ~kVWF_RenderCharacter_setMasks[y];
      if (r4 & 0x8000)
        mbuf[x + 1] ^= kVWF_RenderCharacter_setMasks[y];
      else
        mbuf[x + 1] &= ~kVWF_RenderCharacter_setMasks[y];
      r4 = (r4 & ~0x8080) << 1;
      //r4 <<= 1;
    } while (--r3 && ++y != 8);
    x += 16;
    if (r4 != 0)
      WORD(mbuf[x + 0]) = r4;
  }
  uint16 r8 = vwf_line_ptr + 0x150;
  const uint16 *src3 = kTextBits + (r10 + 16) * 8;
  for (int i = 0; i != 16; i += 2) {
    uint16 r4 = *src3++;
    int y = r8 + r0;
    int x = (y & 0xff0) + i;
    y = (y >> 1) & 7;
    uint8 r3 = width;
    do {
      if (r4 & 0x0080)
        mbuf[x + 0] ^= kVWF_RenderCharacter_setMasks[y];
      else
        mbuf[x + 0] &= ~kVWF_RenderCharacter_setMasks[y];
      if (r4 & 0x8000)
        mbuf[x + 1] ^= kVWF_RenderCharacter_setMasks[y];
      else
        mbuf[x + 1] &= ~kVWF_RenderCharacter_setMasks[y];
      //r4 <<= 1;
      r4 = (r4 & ~0x8080) << 1;
    } while (--r3 && ++y != 8);
    x += 16;
    if (r4 != 0)
      WORD(mbuf[x + 0]) = r4;
  }
  dialogue_msg_dst_offs++;
}

void RenderText_Draw_Choose2LowOr3() {  // 8ecd1a
  if (text_wait_countdown2 != 0) {
    if (--text_wait_countdown2 == 1)
      sound_effect_2 = 36;
  } else if ((filtered_joypad_H | filtered_joypad_L) & 0xc0) {
    sound_effect_1 = 43;
    text_render_state = 4;
  } else if (filtered_joypad_H & 12) {
    int t = filtered_joypad_H & 8 ? 0 : 1;
    if (choice_in_multiselect_box == t)
      return;
    choice_in_multiselect_box = t;
    sound_effect_2 = 32;
    dialogue_message_index = t + 1;
    Text_LoadCharacterBuffer();
    text_next_position = 0;
    dialogue_msg_dst_offs = 0;
    Text_InitVwfState();
  }
}

void RenderText_Draw_ChooseItem() {  // 8ecd88
  if (text_wait_countdown2 != 0) {
    if (--text_wait_countdown2 == 1)
      RenderText_FindYItem_Next();
  } else if ((filtered_joypad_H | filtered_joypad_L) & 0xc0) {
    text_render_state = 4;
  } else {
    if (filtered_joypad_H & 5) {
      choice_in_multiselect_box++;
    } else if (filtered_joypad_H & 10) {
      choice_in_multiselect_box--;
      RenderText_FindYItem_Previous();
      RenderText_Refresh();
      return;
    }
    RenderText_FindYItem_Next();
    RenderText_Refresh();
  }
}

void RenderText_FindYItem_Previous() {  // 8ecdc8
  for (;;) {
    uint8 x = choice_in_multiselect_box;
    if (sign8(x))
      choice_in_multiselect_box = x = 31;
    if (x != 15 && ((&link_item_bow)[x] || x == 32 && (&link_item_bow)[x + 1]))
      break;
    choice_in_multiselect_box--;
  }
  RenderText_DrawSelectedYItem();
}

void RenderText_FindYItem_Next() {  // 8ecded
  for (;;) {
    uint8 x = choice_in_multiselect_box;
    if (x >= 32)
      choice_in_multiselect_box = x = 0;
    if (x != 15 && ((&link_item_bow)[x] || x == 32 && (&link_item_bow)[x + 1]))
      break;
    choice_in_multiselect_box++;
  }
  RenderText_DrawSelectedYItem();
}

void RenderText_DrawSelectedYItem() {  // 8ece14
  int item = choice_in_multiselect_box;
  const uint16 *p = Hud_GetItemBoxPtr(item);
  p += ((item == 3 || item == 32) ? 1 : (&link_item_bow)[item]) * 4;
  uint8 *vwf300 = &g_ram[0x1300];
  memcpy(vwf300 + 0xc2, p, 4);
  memcpy(vwf300 + 0xec, p + 2, 4);
}

void RenderText_Draw_Choose2HiOr3() {  // 8ece83
  if (text_wait_countdown2 != 0) {
    if (--text_wait_countdown2 == 1)
      sound_effect_2 = 36;
  } else if ((filtered_joypad_H | filtered_joypad_L) & 0xc0) {
    sound_effect_1 = 43;
    text_render_state = 4;
  } else if (filtered_joypad_H & 12) {
    int t = filtered_joypad_H & 8 ? 0 : 1;
    if (choice_in_multiselect_box == t)
      return;
    choice_in_multiselect_box = t;
    sound_effect_2 = 32;
    dialogue_message_index = t + 11;
    Text_LoadCharacterBuffer();
    text_next_position = 0;
    dialogue_msg_dst_offs = 0;
    Text_InitVwfState();
  }
}

void RenderText_Draw_Choose3() {  // 8ecef7
  uint8 y;
  if (text_wait_countdown2 != 0) {
    if (--text_wait_countdown2 == 1)
      sound_effect_2 = 36;
  } else if ((y = filtered_joypad_L & 0xc0 | filtered_joypad_H) & 0xd0) {
    sound_effect_1 = 43;
    text_render_state = 4;
  } else if (y & 12) {
    int choice = choice_in_multiselect_box;
    if (y & 8)
      choice = (choice == 0) ? 2 : choice - 1;
    else
      choice = (choice == 2) ? 0 : choice + 1;
    choice_in_multiselect_box = choice;
    sound_effect_2 = 32;
    dialogue_message_index = choice + 6;
    Text_LoadCharacterBuffer();
    text_next_position = 0;
    dialogue_msg_dst_offs = 0;
    Text_InitVwfState();
  }
}

void RenderText_Draw_Choose1Or2() {  // 8ecf72
  uint8 y;
  if (text_wait_countdown2 != 0) {
    if (--text_wait_countdown2 == 1)
      sound_effect_2 = 36;
  } else if ((y = filtered_joypad_L & 0xc0 | filtered_joypad_H) & 0xd0) {
    sound_effect_1 = 43;
    text_render_state = 4;
  } else if (y & 12) {
    int t = y & 8 ? 0 : 1;
    if (choice_in_multiselect_box == t)
      return;
    choice_in_multiselect_box = t;
    sound_effect_2 = 32;
    dialogue_message_index = t + 9;
    Text_LoadCharacterBuffer();
    text_next_position = 0;
    dialogue_msg_dst_offs = 0;
    Text_InitVwfState();
  }
}

void RenderText_Draw_Scroll() {  // 8ecfe2
  uint8 r2 = byte_7E1CEA;
  do {
    for (int i = 0; i < 0x7e0; i += 16) {
      uint16 *p = (uint16 *)((uint8 *)messaging_buf + i);
      p[0] = p[1];
      p[1] = p[2];
      p[2] = p[3];
      p[3] = p[4];
      p[4] = p[5];
      p[5] = p[6];
      p[6] = p[7];
      p[7] = p[168];
    }
    uint16 *p = messaging_buf;
    for (int i = 0x34f; i <= 0x3ef; i += 8)
      p[i] = 0;

    if ((++byte_7E1CDF & 0xf) == 0) {
      dialogue_msg_dst_offs++;
      dialogue_msg_src_offs = 80;
      vwf_curline = 4;
      vwf_flag_next_line = 1;
      text_next_position = 0;
      break;
    }
  } while (r2--);
}

void RenderText_Draw_Command7B() {  // 8ed18d
  int i = (messaging_text_buffer[dialogue_msg_dst_offs + 1] & 0x7f);
  int j = dialogue_msg_src_offs;
  WORD(g_ram[0x2D8 + j]) = kVWF_Command7B[i * 2 + 0];
  WORD(g_ram[0x300 + j]) = kVWF_Command7B[i * 2 + 1];
  dialogue_msg_src_offs = j + 2;
  dialogue_msg_dst_offs += 2;
  RenderText_Draw_MessageCharacters();
}

void RenderText_Draw_ABunchOfSpaces() {  // 8ed1bd
  int i = (messaging_text_buffer[dialogue_msg_dst_offs + 1] & 0x7f);
  int j = dialogue_msg_src_offs;
  WORD(g_ram[0x2D8 + j]) = kVWF_Command7C[i * 4 + 0];
  WORD(g_ram[0x300 + j]) = kVWF_Command7C[i * 4 + 1];
  WORD(g_ram[0x2DA + j]) = kVWF_Command7C[i * 4 + 2];
  WORD(g_ram[0x302 + j]) = kVWF_Command7C[i * 4 + 3];
  dialogue_msg_src_offs = j + 4;
  dialogue_msg_dst_offs += 2;
  RenderText_Draw_MessageCharacters();
}

void RenderText_Draw_EmptyBuffer() {  // 8ed1f9
  memset(messaging_buf, 0, 0x7e0);
  dialogue_msg_src_offs = 0;
  dialogue_msg_dst_offs++;
  text_next_position = 0;
}

void RenderText_SetDefaultWindowPosition() {  // 8ed280
  uint16 y = link_y_coord - BG2VOFS_copy2;
  int flag = (y < 0x78);
  text_msgbox_topleft = kText_Positions[flag];
}

void RenderText_DrawBorderInitialize() {  // 8ed29c
  text_msgbox_topleft_copy = text_msgbox_topleft;
}

uint16 *RenderText_DrawBorderRow(uint16 *d, int y) {  // 8ed2ab
  y >>= 1;
  *d++ = swap16(text_msgbox_topleft_copy);
  text_msgbox_topleft_copy += 0x20;
  *d++ = 0x2F00;
  *d++ = kText_BorderTiles[y];
  for(int i = 0; i < 22; i++)
    *d++ = kText_BorderTiles[y+1];
  *d++ = kText_BorderTiles[y+2];
  *d = 0xffff;
  return d;
}

void Text_BuildCharacterTilemap() {  // 8ed2ec
  uint16 *vwf300 = (uint16 *)&g_ram[0x1300];
  for (int i = 0; i < 126; i++)
    vwf300[i] = text_tilemap_cur++;
  RenderText_Refresh();
}

void RenderText_Refresh() {  // 8ed307
  RenderText_DrawBorderInitialize();
  text_msgbox_topleft_copy += 0x21;
  uint16 *d = vram_upload_data;
  uint16 *s = (uint16 *)&g_ram[0x1300];
  for (int j = 0; j != 6; j++) {
    *d++ = swap16(text_msgbox_topleft_copy);
    text_msgbox_topleft_copy += 0x20;
    *d++ = 0x2900;
    for (int i = 0; i != 21; i++)
      *d++ = *s++;
  }
  *d = 0xffff;
  nmi_load_bg_from_vram = 1;
}

void Text_GenerateMessagePointers() {  // 8ed3eb
  const uint8 *src = kDialogueText;
  uint32 p = 0x1c8000;
  uint8 *dst = kTextDialoguePointers;
  for (int i = 0;; i++) {
    if (i == 359)
      p = 0xedf40;
    WORD(dst[0]) = p;
    dst[2] = p >> 16;
    dst += 3;

    if (i == 397)
      break;

    for (;;) {
      int j = *src;
      int len = (j >= 0x67 && j < 0x80) ? kText_CommandLengths[j - 0x67] : 1;
      src += len;
      p += len;
      if (j == 0x7f)
        break;
    }
  }
}

void DungMap_LightenUpMap() {  // 8ed940
  if (++INIDISP_copy == 0xf)
    overworld_map_state++;
}

void DungMap_Backup() {  // 8ed94c
  if (--INIDISP_copy)
    return;
  MOSAIC_copy = 3;
  mapbak_HDMAEN = HDMAEN_copy;
  EnableForceBlank();
  overworld_map_state++;
  dungmap_init_state = 0;
  COLDATA_copy0 = 0x20;
  COLDATA_copy1 = 0x40;
  COLDATA_copy2 = 0x80;
  link_dma_graphics_index = 0x250;
  memcpy(mapbak_palette, main_palette_buffer, sizeof(uint16) * 256);
  mapbak_bg1_x_offset = bg1_x_offset;
  mapbak_bg1_y_offset = bg1_y_offset;
  bg1_x_offset = 0;
  bg1_y_offset = 0;
  mapbak_BG1HOFS_copy2 = BG1HOFS_copy2;
  mapbak_BG2HOFS_copy2 = BG2HOFS_copy2;
  mapbak_BG1VOFS_copy2 = BG1VOFS_copy2;
  mapbak_BG2VOFS_copy2 = BG2VOFS_copy2;
  BG1HOFS_copy2 = BG1VOFS_copy2 = 0;
  BG2HOFS_copy2 = BG2VOFS_copy2 = 0;
  BG3HOFS_copy2 = BG3VOFS_copy2 = 0;
  mapbak_CGWSEL = WORD(CGWSEL_copy);
  CGWSEL_copy = 0x02;
  CGADSUB_copy = 0x20;
  for (int i = 0; i < 2048; i++)
    messaging_buf[i] = 0x300;
  sound_effect_2 = 16;
  music_control = 0xf2;
}

void DungMap_FadeMapToBlack() {  // 8eda37
  if (--INIDISP_copy)
    return;
  EnableForceBlank();
  overworld_map_state++;
  WORD(CGWSEL_copy) = mapbak_CGWSEL;
  BG1HOFS_copy2 =  mapbak_BG1HOFS_copy2;
  BG2HOFS_copy2 =  mapbak_BG2HOFS_copy2;
  BG1VOFS_copy2 =  mapbak_BG1VOFS_copy2;
  BG2VOFS_copy2 =  mapbak_BG2VOFS_copy2;
  BG3VOFS_copy2 = BG3HOFS_copy2 = 0;
  bg1_x_offset = mapbak_bg1_x_offset;
  bg1_y_offset = mapbak_bg1_y_offset;
  flag_update_cgram_in_nmi++;
}

void DungMap_RestoreOld() {  // 8eda79
  OrientLampLightCone();
  if (++INIDISP_copy != 0xf)
    return;
  main_module_index = saved_module_for_menu;
  submodule_index = 0;
  overworld_map_state = 0;
  subsubmodule_index = 0;
  INIDISP_copy = 0xf;
  HDMAEN_copy = mapbak_HDMAEN;
}

void Death_PlayerSwoon() {  // 8ff5e3
  int k = link_var30d;
  if (sign8(--some_animation_timer)) {
    k++;
    if (k == 15)
      return;
    if (k == 14)
      submodule_index++;
    link_var30d = k;
    some_animation_timer_steps = kDeath_AnimCtr0[k];
    some_animation_timer = kDeath_AnimCtr1[k];
  }
  if (k != 13 || link_visibility_status == 12)
    return;
  uint8 y = link_y_coord + 16 - BG2VOFS_copy2;
  uint8 x = link_x_coord + 7 - BG2HOFS_copy2;

  int spr = 0x74;
  bytewise_extended_oam[spr] = 2;
  oam_buf[spr].x = x;
  oam_buf[spr].y = y;
  oam_buf[spr].charnum = 0xaa;
  oam_buf[spr].flags = kDeath_SprFlags[link_is_on_lower_level] | 2;
}

void Death_PrepFaint() {  // 8ffa6f
  link_direction_facing = 2;
  player_unk1 = 1;
  link_var30d = 0;
  some_animation_timer_steps = 0;
  some_animation_timer = 5;
  link_hearts_filler = 0;
  link_health_current = 0;
  Link_ResetProperties_C();
  player_on_somaria_platform = 0;
  draw_water_ripples_or_grass = 0;
  link_is_bunny_mirror = 0;
  bitmask_of_dragstate = 0;
  flag_is_ancilla_to_pick_up = 0;
  link_auxiliary_state = 0;
  link_incapacitated_timer = 0;
  link_give_damage = 0;
  link_is_transforming = 0;
  link_speed_setting = 0;
  link_need_for_poof_for_transform = 0;
  if (link_item_moon_pearl)
    link_is_bunny = 0;
  link_timer_tempbunny = 0;
  sound_effect_1 = 0x27 | Link_CalculateSfxPan();
  for (int i = 0; i != 4; i++) {
    if (link_bottle_info[i] == 6)
      return;
  }
  index_of_changable_dungeon_objs[0] = index_of_changable_dungeon_objs[1] = 0;
}

