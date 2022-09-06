#include "misc.h"
#include "variables.h"
#include "hud.h"
#include "dungeon.h"
#include "overworld.h"
#include "load_gfx.h"
#include "sprite.h"
#include "poly.h"
#include "ancilla.h"
#include "select_file.h"
#include "tile_detect.h"
#include "player.h"
#include "player_oam.h"
#include "messaging.h"
#include "ending.h"
#include "attract.h"
#include "snes/snes_regs.h"
#include "tables/generated_predefined_tiles.h"
#include "tables/generated_sound_banks.h"

static void KillAgahnim_LoadMusic();
static void KillAghanim_Init();
static void KillAghanim_Func2();
static void KillAghanim_Func3();
static void KillAghanim_Func4();
static void KillAghanim_Func5();
static void KillAghanim_Func6();
static void KillAghanim_Func7();
static void KillAghanim_Func8();
static void KillAghanim_Func12();
static uint8 PlaySfx_SetPan(uint8 a);

const uint8 kReceiveItem_Tab1[76] = {
  0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 2, 2, 2,
  2, 2, 2, 0, 2, 0, 2, 2, 0, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 0, 2, 2, 2, 2, 2, 0, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 0, 0, 2, 0, 2, 2, 2, 0, 2, 2,
};
static const int8 kReceiveItem_Tab2[76] = {
  -5, -5, -5, -5, -5, -4, -4, -5, -5, -4, -4, -4, -2, -4, -4, -4,
  -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4,
  -4, -4, -4, -5, -4, -4, -4, -4, -4, -4, -2, -4, -4, -4, -4, -4,
  -4, -4, -4, -4, -2, -2, -2, -4, -4, -4, -4, -4, -4, -4, -4, -4,
  -4, -4, -2, -2, -4, -2, -4, -4, -4, -5, -4, -4,
};
static const uint8 kReceiveItem_Tab3[76] = {
  4, 4, 4, 4, 4, 0, 0, 4, 4, 4, 4, 4, 5, 0, 0, 0,
  0, 0, 0, 4, 0, 4, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 4, 4, 0, 4, 0, 0, 0, 4, 0, 0,
};
const uint8 kReceiveItemGfx[76] = {
  6, 0x18, 0x18, 0x18, 0x2d, 0x20, 0x2e,    9,    9,  0xa,    8,    5, 0x10,  0xb, 0x2c, 0x1b,
  0x1a, 0x1c, 0x14, 0x19,  0xc,    7, 0x1d, 0x2f,    7, 0x15, 0x12,  0xd,  0xd,  0xe, 0x11, 0x17,
  0x28, 0x27,    4,    4,  0xf, 0x16,    3, 0x13,    1, 0x1e, 0x10,    0,    0,    0,    0,    0,
  0, 0x30, 0x22, 0x21, 0x24, 0x24, 0x24, 0x23, 0x23, 0x23, 0x29, 0x2a, 0x2c, 0x2b,    3,    3,
  0x34, 0x35, 0x31, 0x33,    2, 0x32, 0x36, 0x37, 0x2c,    6,  0xc, 0x38,
};
const uint16 kMemoryLocationToGiveItemTo[76] = {
  0xf359, 0xf359, 0xf359, 0xf359,
  0xf35a, 0xf35a, 0xf35a, 0xf345,
  0xf346, 0xf34b, 0xf342, 0xf340,
  0xf341, 0xf344, 0xf35c, 0xf347,
  0xf348, 0xf349, 0xf34a, 0xf34c,
  0xf34c, 0xf350, 0xf35c, 0xf36b,
  0xf351, 0xf352, 0xf353, 0xf354,
  0xf354, 0xf34e, 0xf356, 0xf357,
  0xf37a, 0xf34d, 0xf35b, 0xf35b,
  0xf36f, 0xf364, 0xf36c, 0xf375,
  0xf375, 0xf344, 0xf341, 0xf35c,
  0xf35c, 0xf35c, 0xf36d, 0xf36e,
  0xf36e, 0xf375, 0xf366, 0xf368,
  0xf360, 0xf360, 0xf360, 0xf374,
  0xf374, 0xf374, 0xf340, 0xf340,
  0xf35c, 0xf35c, 0xf36c, 0xf36c,
  0xf360, 0xf360, 0xf372, 0xf376,
  0xf376, 0xf373, 0xf360, 0xf360,
  0xf35c, 0xf359, 0xf34c, 0xf355,
};
static const int8 kValueToGiveItemTo[76] = {
     1,   2,   3,  4,
     1,   2,   3,  1,
     1,   1,   1,  1,
     1,   2,  -1,  1,
     1,   1,   1,  1,
     2,   1,  -1, -1,
     1,   1,   2,  1,
     2,   1,   1,  1,
    -1,   1,  -1,  2,
    -1,  -1,  -1, -1,
    -1,  -1,   2, -1,
    -1,  -1,  -1, -1,
    -1,  -1,  -1, -1,
    -1,  -5, -20, -1,
    -1,  -1,   1,  3,
    -1,  -1,  -1, -1,
  -100, -50,  -1,  1,
    10,  -1,  -1, -1,
    -1,   1,   3,  1,
};
static const uint8 kDungeon_DefaultAttr[384] = {
  1, 1, 1, 0, 2, 1, 2, 0, 1, 1, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 0, 0, 1, 0, 0, 2, 0, 0, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 1, 1, 1, 2, 2, 2, 2, 2, 1, 1, 0, 0,
  2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 1, 1, 0, 0,
  0, 0, 0, 0x2a, 1, 0x20, 1, 1, 4, 1, 1, 0x18, 1, 2, 0x1c, 1,
  0x28, 0x28, 0x2a, 0x2a, 1, 2, 1, 1, 4, 0, 0, 0, 0x28, 1, 0xa, 0,
  1, 1, 0xc, 0xc, 2, 2, 2, 2, 0x28, 0x2a, 0x20, 0x20, 0x20, 2, 8, 0,
  4, 4, 1, 1, 1, 2, 2, 2, 0, 0, 0x20, 0x20, 0, 2, 0, 0,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0x18, 0x10, 0x10, 1, 1, 1,
  1, 1, 4, 4, 4, 4, 4, 4, 1, 2, 2, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x62, 0x62,
  0, 0, 0x24, 0x24, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x62, 0x62,
  0x27, 2, 2, 2, 0x27, 0x27, 1, 0, 0, 0, 0, 0x24, 0, 0, 0, 0,
  0x27, 0x27, 0x27, 0x27, 0x27, 0x10, 2, 1, 0, 0, 0, 0x24, 0, 0, 0, 0,
  0x27, 2, 2, 2, 0x27, 0x27, 0x27, 0x27, 2, 2, 2, 0x24, 0, 0, 0, 0,
  0x27, 0x27, 0x27, 0x27, 0x27, 0x20, 2, 2, 1, 2, 2, 0x23, 2, 0, 0, 0,
  0x27, 0x27, 0x27, 0x27, 0x27, 0x20, 2, 0x27, 2, 0x54, 0, 0, 0x27, 2, 2, 2,
  0x27, 0x27, 0x27, 0x27, 0x27, 0x27, 2, 0x27, 2, 0x54, 0, 0, 0x27, 2, 2, 2,
  0x27, 0x27, 0, 0x27, 0x60, 0x60, 1, 1, 1, 1, 2, 2, 0xd, 0, 0, 0x4b,
  0x67, 0x67, 0x67, 0x67, 0x66, 0x66, 0x66, 0x66, 0, 0, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x27, 0x63, 0x27, 0x55, 0x55, 1, 0x44, 0, 1, 0x20, 2, 2, 0x1c, 0x3a, 0x3b, 0,
  0x27, 0x63, 0x27, 0x53, 0x53, 1, 0x44, 1, 0xd, 0, 0, 0, 9, 9, 9, 9,
};
static PlayerHandlerFunc *const kModule_BossVictory[6] = {
  &BossVictory_Heal,
  &Dungeon_StartVictorySpin,
  &Dungeon_RunVictorySpin,
  &Dungeon_CloseVictorySpin,
  &Dungeon_PrepExitWithSpotlight,
  &Spotlight_ConfigureTableAndControl,
};
static PlayerHandlerFunc *const kModule_KillAgahnim[13] = {
  &KillAgahnim_LoadMusic,
  &KillAghanim_Init,
  &KillAghanim_Func2,
  &KillAghanim_Func3,
  &KillAghanim_Func4,
  &KillAghanim_Func5,
  &KillAghanim_Func6,
  &KillAghanim_Func7,
  &KillAghanim_Func8,
  &BossVictory_Heal,
  &Dungeon_StartVictorySpin,
  &Dungeon_RunVictorySpin,
  &KillAghanim_Func12,
};
static PlayerHandlerFunc *const kMainRouting[28] = {
  &Module00_Intro,
  &Module01_FileSelect,
  &Module02_CopyFile,
  &Module03_KILLFile,
  &Module04_NameFile,
  &Module05_LoadFile,
  &Module_PreDungeon,
  &Module07_Dungeon,
  &Module08_OverworldLoad,
  &Module09_Overworld,
  &Module08_OverworldLoad,
  &Module09_Overworld,
  &Module_Unknown0,
  &Module_Unknown1,
  &Module0E_Interface,
  &Module0F_SpotlightClose,
  &Module10_SpotlightOpen,
  &Module11_DungeonFallingEntrance,
  &Module12_GameOver,
  &Module13_BossVictory_Pendant,
  &Module14_Attract,
  &Module15_MirrorWarpFromAga,
  &Module16_BossVictory_Crystal,
  &Module17_SaveAndQuit,
  &Module18_GanonEmerges,
  &Module19_TriforceRoom,
  &Module1A_Credits,
  &Module1B_SpawnSelect,
};

const uint16 *SrcPtr(uint16 src) {
  return &kPredefinedTileData[src >> 1];
}

uint8 Ancilla_Sfx2_Near(uint8 a) {
  return sound_effect_1 = PlaySfx_SetPan(a);
}

void Ancilla_Sfx3_Near(uint8 a) {
  sound_effect_2 = PlaySfx_SetPan(a);
}

void LoadDungeonRoomRebuildHUD() {
  mosaic_level = 0;
  MOSAIC_copy = 7;
  Hud_SearchForEquippedItem();
  Hud_Rebuild();
  Hud_UpdateEquippedItem();
  Module_PreDungeon();
}

void Module_Unknown0() {
  assert(0);
}

void Module_Unknown1() {
  assert(0);
}

static void KillAgahnim_LoadMusic() {
  nmi_disable_core_updates = 0;
  overworld_map_state++;
  submodule_index++;
  LoadOWMusicIfNeeded();
}

static void KillAghanim_Init() {
  music_control = 8;
  BYTE(overworld_screen_trans_dir_bits) = 8;
  InitializeMirrorHDMA();
  overworld_map_state = 0;
  PaletteFilter_InitializeWhiteFilter();
  Overworld_LoadGFXAndScreenSize();
  submodule_index++;
  link_player_handler_state = kPlayerState_Mirror;
  bg1_x_offset = 0;
  bg1_y_offset = 0;
  dung_savegame_state_bits = 0;
  WORD(link_y_vel) = 0;
  main_palette_buffer[0] = 0x7fff;
  main_palette_buffer[32] = 0x7fff;
  Ancilla_TerminateSelectInteractives(0);
  Link_ResetProperties_A();
}

static void KillAghanim_Func2() {
  HDMAEN_copy = 192;
  MirrorWarp_BuildWavingHDMATable();
  submodule_index++;
  subsubmodule_index = 0;
}

static void KillAghanim_Func3() {
  MirrorWarp_BuildWavingHDMATable();
  if (subsubmodule_index) {
    subsubmodule_index = 0;
    submodule_index++;
  }
}

static void KillAghanim_Func4() {
  MirrorWarp_BuildDewavingHDMATable();
  if (subsubmodule_index) {
    subsubmodule_index = 0;
    submodule_index++;
  }
}

static void KillAghanim_Func5() {
  HdmaSetup(0, 0xf2fb, 0x41, 0, (uint8)WH0, 0);
  for (int i = 0; i < 224; i++)
    mode7_hdma_table[i] = 0xff00;
  palette_filter_countdown = 0;
  darkening_or_lightening_screen = 0;
  dialogue_message_index = 0x35;
  Main_ShowTextMessage();
  ReloadPreviouslyLoadedSheets();
  Hud_RebuildIndoor();
  HDMAEN_copy = 0x80;
  main_module_index = 21;
  submodule_index = 6;
  subsubmodule_index = 24;
}

static void KillAghanim_Func6() {
  if (!--subsubmodule_index) {
    submodule_index++;
    sound_effect_ambient = 9;
  }
}

static void KillAghanim_Func7() {
  RenderText();
  if (!submodule_index) {
    overworld_map_state = 0;
    sound_effect_ambient = 5;
    if (!link_item_moon_pearl) {
      dialogue_message_index = 0x36;
      Main_ShowTextMessage();
      sound_effect_ambient = 0;
      main_module_index = 21;
      submodule_index = 8;
    } else {
      submodule_index = 9;
    }
  }
}

static void KillAghanim_Func8() {
  RenderText();
  if (!submodule_index) {
    subsubmodule_index = 32;
    submodule_index = 12;
  }
}

static void KillAghanim_Func12() {
  if (--subsubmodule_index)
    return;
  ResetAncillaAndCutscene();
  Overworld_SetSongList();
  save_ow_event_info[0x1b] |= 32;
  BYTE(cur_palace_index_x2) = 255;
  submodule_index = 0;
  overworld_map_state = 0;
  nmi_disable_core_updates = 0;
  main_module_index = 9;
  BYTE(BG1VOFS_copy2) = 0;
  music_control = link_item_moon_pearl ? 9 : 4;
  savegame_map_icons_indicator = 6;
}

void Module_MainRouting() {  // 8080b5
  kMainRouting[main_module_index]();
}

void NMI_PrepareSprites() {  // 8085fc
  static const uint16 kLinkDmaSources1[303] = {
    0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x8040, 0x8040, 0x8040, 0x8040, 0x8040, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000,
    0x9440, 0x8080, 0x8080, 0x8080, 0x9400, 0x8040, 0x80c0, 0x80c0, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000,
    0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x8040, 0x8040, 0x8040, 0x8040, 0x8040, 0x8000, 0xa8c0, 0xa900, 0x8000, 0xa8c0, 0xa900,
    0x9100, 0x8080, 0x8080, 0x90c0, 0x8040, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x9a00, 0x9140, 0x9180, 0x8000, 0x9500,
    0x9480, 0x94c0, 0x94c0, 0x9ae0, 0x8080, 0x8080, 0x9a60, 0x80c0, 0x80c0, 0x9aa0, 0x8000, 0x8000, 0x9aa0, 0x8000, 0x8000, 0x8080,
    0x8080, 0x8100, 0x8100, 0x85c0, 0x8000, 0x8000, 0x85c0, 0x8000, 0x8000, 0xadc0, 0xadc0, 0xadc0, 0xadc0, 0xadc0, 0xad40, 0xad40,
    0xad40, 0xad40, 0xad40, 0xad80, 0xad80, 0xad80, 0xad80, 0xad80, 0xad80, 0x8040, 0x9400, 0x8040, 0x8000, 0x8080, 0x8080, 0x9440,
    0x8000, 0x8000, 0x8000, 0x8000, 0x8080, 0x8040, 0x8040, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0xc440, 0x8140, 0x8140,
    0xca40, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8040, 0x85c0, 0x8040, 0x85c0, 0x8100, 0x80c0, 0x91c0, 0x8080, 0x8080,
    0x8040, 0x8040, 0x8000, 0x8000, 0x8000, 0x8000, 0x8080, 0x8080, 0x9100, 0xa0c0, 0xa100, 0xa100, 0xa1c0, 0xa400, 0xa440, 0xa1c0,
    0xa400, 0xa440, 0x8080, 0xc480, 0x8080, 0x8040, 0x8040, 0xca80, 0xca80, 0xca00, 0xc400, 0xca00, 0xc400, 0x81c0, 0x8080, 0x8080,
    0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x8040, 0x8040, 0x8040, 0x8040, 0x8040, 0x8040, 0x8040, 0x8000, 0xa8c0, 0xa900,
    0x8000, 0x8000, 0xa8c0, 0xa900, 0x8000, 0xa8c0, 0xa900, 0x8000, 0x8000, 0xa8c0, 0xa900, 0x8040, 0x8040, 0x8040, 0x8080, 0x8080,
    0x8040, 0x8040, 0x8040, 0x8040, 0x8000, 0x8000, 0x8000, 0x8000, 0xd080, 0x8080, 0x90c0, 0xd000, 0x9080, 0xd040, 0x9080, 0xd040,
    0xd080, 0xd080, 0xd080, 0xd080, 0xd080, 0xd000, 0xd000, 0xd000, 0xd000, 0xd000, 0xd040, 0xd040, 0xd040, 0xd040, 0xd040, 0xd040,
    0x8040, 0xd000, 0x85c0, 0x85c0, 0x85c0, 0xdc40, 0xdc40, 0xdc40, 0x85c0, 0x85c0, 0x85c0, 0xdc40, 0xdc40, 0xdc40, 0xe1c0, 0xd000,
    0x8000, 0xe400, 0xe400, 0xe440, 0x90c0, 0x90c0, 0xd000, 0x8000, 0x8000, 0xd040, 0x8000, 0x8000, 0xd040, 0xe400, 0xe400, 0xe400,
    0x9080, 0xa5c0, 0xac40, 0xe480, 0x8180, 0x90c0, 0x80c0, 0xe180, 0xd000, 0xe4c0, 0xe4c0, 0xe840, 0xe840, 0xe840, 0xe540, 0xe540,
    0xe540, 0xe900, 0xe900, 0xe900, 0xe900, 0x8080, 0x8080, 0x8000, 0xa9c0, 0x8080, 0x8140, 0x91c0, 0x8040, 0xa800, 0xa840,
  };
  static const uint16 kLinkDmaSources2[303] = {
    0x8840, 0x8800, 0x8580, 0x8800, 0x8580, 0x84c0, 0x8500, 0x8540, 0x8500, 0x8540, 0x8400, 0x8440, 0x8480, 0x8400, 0x8440, 0x8480,
    0x9640, 0x8c40, 0x8c80, 0xad00, 0x9600, 0x8980, 0x8c00, 0xacc0, 0x8880, 0x88c0, 0x8900, 0x8940, 0x8880, 0x88c0, 0x8900, 0x8940,
    0xb0c0, 0xb100, 0xb140, 0xb100, 0xb140, 0xb000, 0xb040, 0xb080, 0xec80, 0xecc0, 0xb180, 0xd440, 0xb1c0, 0xb180, 0xd440, 0xb1c0,
    0x8c80, 0xad00, 0x95c0, 0x99c0, 0xb440, 0x9580, 0xb480, 0xb4c0, 0x9580, 0xb480, 0xb4c0, 0x9c20, 0x8000, 0x8000, 0x8000, 0x9700,
    0x9680, 0x96c0, 0x96c0, 0x9ce0, 0x8c80, 0xb540, 0x9c60, 0xb580, 0x8c00, 0x9ca0, 0x8900, 0xb500, 0x9ca0, 0x8900, 0xb500, 0x8c40,
    0xec40, 0x8c00, 0xec00, 0x8dc0, 0x9540, 0x89c0, 0x8dc0, 0x9540, 0x89c0, 0xb940, 0xb980, 0xb9c0, 0xb980, 0xb9c0, 0xb5c0, 0xb800,
    0xb840, 0xb800, 0xb840, 0xb880, 0xb8c0, 0xb900, 0xb880, 0xb8c0, 0xb900, 0x8980, 0x9600, 0xbcc0, 0x8400, 0xbc80, 0x8c40, 0x9640,
    0xa040, 0xa080, 0xa000, 0xbc40, 0xbd40, 0x8500, 0xbd00, 0xbd80, 0xbd80, 0x88c0, 0x8900, 0xe9c0, 0x8900, 0xc640, 0xc040, 0xc000,
    0xcc40, 0x8940, 0x88c0, 0x8900, 0xe9c0, 0x8900, 0x8940, 0x8d40, 0x8d80, 0x8d40, 0x8d80, 0xbd00, 0xb000, 0xb000, 0xa480, 0xa480,
    0xa480, 0xa480, 0xac00, 0xac00, 0xac00, 0xac00, 0xa140, 0xa180, 0xa180, 0xa4c0, 0xa4c0, 0xa500, 0x9d40, 0x9d80, 0x9dc0, 0x9d40,
    0x9d80, 0x9dc0, 0x8d00, 0xc680, 0xc180, 0xc140, 0x8c00, 0xcc80, 0xcc80, 0xcc00, 0xc600, 0xcc00, 0xc600, 0xbd00, 0x8580, 0x8800,
    0xc9c0, 0xccc0, 0xcdc0, 0xcd00, 0xcd40, 0xcd80, 0x8500, 0x8540, 0xc940, 0xc980, 0x8540, 0xc940, 0xc980, 0x8440, 0x8480, 0xc1c0,
    0xc900, 0xc580, 0xc5c0, 0xc8c0, 0x8440, 0x8480, 0xc1c0, 0xc900, 0xc580, 0xc5c0, 0xc8c0, 0xbd00, 0xacc0, 0xc040, 0xd540, 0xd580,
    0xd4c0, 0xd500, 0xd4c0, 0xd500, 0xd440, 0xd480, 0xd440, 0xd480, 0xd1c0, 0xd400, 0xd100, 0xd100, 0xd140, 0xd180, 0xd140, 0xd180,
    0xb0c0, 0xb100, 0xb140, 0xb100, 0xb140, 0xdd40, 0xdd80, 0xddc0, 0xdd80, 0xddc0, 0xdc80, 0xdcc0, 0xdd00, 0xdc80, 0xdcc0, 0xdd00,
    0xd100, 0xd100, 0xe000, 0xe040, 0xe080, 0xe0c0, 0xe100, 0xe140, 0xe000, 0xe040, 0xe080, 0xe0c0, 0xe100, 0xe140, 0x8000, 0xd0c0,
    0x8000, 0xb940, 0xb980, 0xb940, 0xdd40, 0xdd80, 0xdd40, 0xdc80, 0xdcc0, 0xc0c0, 0xdc80, 0xdcc0, 0xc0c0, 0xb9c0, 0xb980, 0xb9c0,
    0xa560, 0xa5a0, 0xac80, 0xed00, 0x8000, 0x8cc0, 0xbd00, 0xe380, 0xbdc0, 0xe500, 0xe500, 0xe880, 0xe8c0, 0xe8c0, 0xe800, 0xe5c0,
    0xe5c0, 0xe940, 0xe980, 0xe940, 0xe980, 0xbd40, 0x8c80, 0xa080, 0x8000, 0xa980, 0xbd00, 0xbdc0, 0xb400, 0xa880, 0xedc0,
  };
  static const uint16 kLinkDmaSources3[27] = {
    0x9a40, 0x9e00, 0x9d20, 0x9f20, 0x9b20, 0xbc20, 0xbc20, 0xbe20, 0xbe20, 0xbe00, 0xbe00, 0xbe00, 0xbe00, 0xa540, 0xa540, 0xa540,
    0xa540, 0xbc00, 0xbc00, 0xbc00, 0xbc00, 0xa740, 0xa740, 0xa740, 0xa740, 0xe780, 0xe780,
  };
  static const uint16 kLinkDmaSources4[8] = { 0x9000, 0x9020, 0x9060, 0x91e0, 0x90a0, 0x90c0, 0x9100, 0x9140 };
  static const uint16 kLinkDmaSources5[3] = { 0x9300, 0x9340, 0x9380 };
  static const uint16 kLinkDmaSources6[128] = {
    0x9480, 0x94c0, 0x94e0, 0x95c0, 0x9500, 0x9520, 0x9540, 0x9480, 0x9640, 0x9680, 0x96a0, 0x9780, 0x96c0, 0x96e0, 0x9700, 0x9480,
    0x9800, 0x9840, 0x98a0, 0x9480, 0x9480, 0x9480, 0x9480, 0x9480, 0x9ac0, 0x9b00, 0x9480, 0x9480, 0x9480, 0x9480, 0x9480, 0x9480,
    0x9bc0, 0x9c00, 0x9c40, 0x9c80, 0x9cc0, 0x9d00, 0x9d40, 0x9480, 0x9f40, 0x9f80, 0x9fc0, 0x9fe0, 0xa000, 0x9480, 0x9480, 0x9480,
    0xa100, 0x9480, 0x9480, 0x9480, 0x9480, 0x9480, 0x9480, 0x9480, 0x9480, 0x9480, 0x9480, 0x9480, 0x9480, 0x9480, 0x9480, 0x9480,
    0x98c0, 0x9900, 0x99c0, 0x99e0, 0x9a00, 0x9a20, 0x9a40, 0x9a60, 0x9480, 0x9480, 0x9480, 0x9480, 0x9480, 0x9480, 0x9480, 0x9480,
    0x9a80, 0x9480, 0x9480, 0x9480, 0x9480, 0x9480, 0x9480, 0x9480, 0x9480, 0x9480, 0x9480, 0x9480, 0x9480, 0x9480, 0x9480, 0x9480,
    0x9480, 0x9480, 0x9480, 0x9480, 0x9480, 0x9480, 0x9480, 0x9480, 0x9480, 0x9480, 0x9480, 0x9480, 0x9480, 0x9480, 0x9480, 0x9480,
    0x9480, 0x9480, 0x9480, 0x9480, 0x9480, 0x9480, 0x9480, 0x9480, 0x9480, 0x9480, 0x9480, 0x9480, 0x9480, 0x9480, 0x9480, 0x9480,
  };
  static const uint16 kLinkDmaSources7[16] = { 0xe0, 0xe0, 0x60, 0x80, 0x1c0, 0xe0, 0x40, 0, 0x80, 0, 0x40, 0, 0, 0, 0, 0 };
  static const uint16 kLinkDmaCtrs0[6] = { 14, 4, 6, 16, 6, 8 };
  static const uint16 kLinkDmaSources9[15] = { 0, 0x20, 0x40, 0, 0x20, 0x40, 0, 0x40, 0x80, 0, 0x40, 0x80, 0xb340, 0xb400, 0xb4c0 };
  static const uint16 kLinkDmaSources8[4] = { 0xa480, 0xa4c0, 0xa500, 0xa540 };

  for (int i = 0; i < 32; i++) {
    extended_oam[i] = bytewise_extended_oam[3 + 4 * i] << 6 |
      bytewise_extended_oam[2 + 4 * i] << 4 |
      bytewise_extended_oam[1 + 4 * i] << 2 |
      bytewise_extended_oam[0 + 4 * i] << 0;
  }

  dma_source_addr_3 = kLinkDmaSources1[link_dma_graphics_index >> 1];
  dma_source_addr_0 = dma_source_addr_3 + 0x200;
  dma_source_addr_4 = kLinkDmaSources2[link_dma_graphics_index >> 1];
  dma_source_addr_1 = dma_source_addr_4 + 0x200;
  dma_source_addr_5 = kLinkDmaSources3[link_dma_var1 >> 1];
  dma_source_addr_2 = kLinkDmaSources3[link_dma_var2 >> 1];


  dma_source_addr_6 = kLinkDmaSources4[link_dma_var3 >> 1];
  dma_source_addr_11 = dma_source_addr_6 + 0x180;

  if (link_dma_var4 == 0x8b) {
    dma_source_addr_7 = 0xe099;
  } else {
    dma_source_addr_7 = kLinkDmaSources5[link_dma_var4 >> 1];
  }
  dma_source_addr_12 = dma_source_addr_7 + 0xc0;


  int j = (link_dma_var5 & 0xf8) >> 3;
  dma_source_addr_8 = kLinkDmaSources6[link_dma_var5];
  dma_source_addr_13 = dma_source_addr_8 + kLinkDmaSources7[j];
  dma_source_addr_10 = kLinkDmaSources8[pushedblocks_some_index & 3];
  dma_source_addr_15 = dma_source_addr_10 + 0x100;

  if (--bg_tile_animation_countdown == 0) {
    bg_tile_animation_countdown = (BYTE(overlay_index) == 0xb5 || BYTE(overlay_index) == 0xbc) ? 0x17 : 9;

    uint16 t = word_7EC00F + 0x400;
    if (t == 0xc00)
      t = 0;
    word_7EC00F = t;
    animated_tile_data_src = 0xa680 + word_7EC00F;
  }

  if (--word_7EC013 == 0) {
    int t = word_7EC015 + 2;
    if (t == 12)
      t = 0;
    word_7EC015 = t;
    word_7EC013 = kLinkDmaCtrs0[t >> 1];
    dma_source_addr_9 = kLinkDmaSources9[t >> 1] + 0xb280;
    dma_source_addr_14 = dma_source_addr_9 + 0x60;
  }

  dma_source_addr_16 = 0xB940 + dma_var6 * 2;
  dma_source_addr_18 = dma_source_addr_16 + 0x200;

  dma_source_addr_17 = 0xB940 + dma_var7 * 2;
  dma_source_addr_19 = dma_source_addr_17 + 0x200;

  dma_source_addr_20 = 0xB540 + flag_travel_bird * 2;
  dma_source_addr_21 = dma_source_addr_20 + 0x200;
}

void Sound_LoadIntroSongBank() {  // 808901
  LoadSongBank(kSoundBank_intro);
}

void LoadOverworldSongs() {  // 808913
  LoadSongBank(kSoundBank_intro);
}

void LoadDungeonSongs() {  // 808925
  LoadSongBank(kSoundBank_indoor);
}

void LoadCreditsSongs() {  // 808931
  LoadSongBank(kSoundBank_ending);
}

void Dungeon_LightTorch() {  // 81f3ec
  if ((byte_7E0333 & 0xf0) != 0xc0) {
    byte_7E0333 = 0;
    return;
  }
  uint8 r8 = (uint8)dungeon_room_index == 0 ? 0x80 : 0xc0;

  int i = (byte_7E0333 & 0xf) + (dung_index_of_torches_start >> 1);
  int opos = dung_object_pos_in_objdata[i];
  if (dung_object_tilemap_pos[i] & 0x8000)
    return;
  dung_object_tilemap_pos[i] |= 0x8000;
  if (r8 == 0)
    dung_torch_data[opos] = dung_object_tilemap_pos[i];

  uint16 x = dung_object_tilemap_pos[i] & 0x3fff;
  RoomDraw_AdjustTorchLightingChange(x, 0xeca, x);

  sound_effect_1 = 42 | CalculateSfxPan_Arbitrary((x & 0x7f) * 2);

  nmi_copy_packets_flag = 1;
  if (dung_want_lights_out) {
    if (dung_num_lit_torches++ < 3) {
      TS_copy = 0;
      overworld_fixed_color_plusminus = kLitTorchesColorPlus[dung_num_lit_torches];
      submodule_index = 10;
      subsubmodule_index = 0;
    }
  }

  dung_torch_timers[byte_7E0333 & 0xf] = r8;
  byte_7E0333 = 0;
}

void RoomDraw_AdjustTorchLightingChange(uint16 x, uint16 y, uint16 r8) {  // 81f746
  const uint16 *ptr = SrcPtr(y);
  x >>= 1;
  overworld_tileattr[x + 0] = ptr[0];
  overworld_tileattr[x + 64] = ptr[1];
  overworld_tileattr[x + 1] = ptr[2];
  overworld_tileattr[x + 65] = ptr[3];
  Dungeon_PrepOverlayDma_nextPrep(0, r8);
}

int Dungeon_PrepOverlayDma_nextPrep(int dst, uint16 r8) {  // 81f764
  uint16 r6 = 0x880 + ((r8 & 0x3f) >= 0x3a);
  return Dungeon_PrepOverlayDma_watergate(dst, r8, r6, 4);
}

int Dungeon_PrepOverlayDma_watergate(int dst, uint16 r8, uint16 r6, int loops) {  // 81f77c
  for (int k = 0; k < loops; k++) {
    int x = r8 >> 1;
    vram_upload_tile_buf[dst + 0] = ((r8 & 0x40) << 4) | ((r8 & 0x303f) >> 1) | ((r8 & 0xf80) >> 2);
    vram_upload_tile_buf[dst + 1] = r6;
    vram_upload_tile_buf[dst + 2] = overworld_tileattr[x + 0];
    if (!(r6 & 1)) {
      vram_upload_tile_buf[dst + 3] = overworld_tileattr[x + 1];
      vram_upload_tile_buf[dst + 4] = overworld_tileattr[x + 2];
      vram_upload_tile_buf[dst + 5] = overworld_tileattr[x + 3];
      r8 += 128;
    } else {
      vram_upload_tile_buf[dst + 3] = overworld_tileattr[x + 64];
      vram_upload_tile_buf[dst + 4] = overworld_tileattr[x + 128];
      vram_upload_tile_buf[dst + 5] = overworld_tileattr[x + 192];
      r8 += 2;
    }
    dst += 6;
  }
  vram_upload_tile_buf[dst] = 0xffff;
  return dst;
}

void Module05_LoadFile() {  // 828136
  EnableForceBlank();
  overworld_map_state = 0;
  dung_unk6 = 0;
  byte_7E02D4 = 0;
  byte_7E02D7 = 0;
  tagalong_var5 = 0;
  byte_7E0379 = 0;
  byte_7E03FD = 0;
  EraseTileMaps_normal();
  zelda_ppu_write(OBSEL, 2);
  LoadDefaultGraphics();
  Sprite_LoadGraphicsProperties();
  Init_LoadDefaultTileAttr();
  DecompressSwordGraphics();
  DecompressShieldGraphics();
  Link_Initialize();
  LoadFollowerGraphics();
  sprite_gfx_subset_0 = 70;
  sprite_gfx_subset_1 = 70;
  sprite_gfx_subset_2 = 70;
  sprite_gfx_subset_3 = 70;
  word_7E02CD = 0x200;
  virq_trigger = 48;
  if (savegame_is_darkworld) {
    if (player_is_indoors) {
      LoadDungeonRoomRebuildHUD();
      return;
    }
    Hud_SearchForEquippedItem();
    Hud_Rebuild();
    Hud_UpdateEquippedItem();
    death_var5 = 0;
    dungeon_room_index = 32;
    main_module_index = 8;
    submodule_index = 0;
    subsubmodule_index = 0;
    death_var4 = 0;
  } else {
    if (mosaic_level || death_var5 != 0 && !death_var4 || sram_progress_indicator < 2 || which_starting_point == 5) {
      LoadDungeonRoomRebuildHUD();
      return;
    }
    dialogue_message_index = (link_item_mirror == 2) ? 0x185 : 0x184;
    Main_ShowTextMessage();
    Dungeon_LoadPalettes();
    INIDISP_copy = 15;
    TM_copy = 4;
    TS_copy = 0;
    main_module_index = 27;
  }
}

void Module13_BossVictory_Pendant() {  // 829c4a
  kModule_BossVictory[submodule_index]();
  Sprite_Main();
  LinkOam_Main();
}

void BossVictory_Heal() {  // 829c59
  if (!Hud_RefillMagicPower())
    overworld_map_state++;
  if (!Hud_RefillHealth())
    overworld_map_state++;
  if (!overworld_map_state) {
    button_mask_b_y &= ~0x40;
    Dungeon_ResetTorchBackgroundAndPlayerInner();
    link_direction_facing = 2;
    link_direction_last = 2 << 1;
    flag_update_hud_in_nmi++;
    submodule_index++;
    subsubmodule_index = 16;
    flag_is_link_immobilized++;
  }
  overworld_map_state = 0;
  Hud_RefillLogic();
}

void Dungeon_StartVictorySpin() {  // 829c93
  if (--subsubmodule_index)
    return;
  flag_is_link_immobilized = 0;
  link_direction_facing = 2;
  Link_AnimateVictorySpin();
  Ancilla_TerminateSelectInteractives(0);
  AncillaAdd_VictorySpin();
  submodule_index++;
}

void Dungeon_RunVictorySpin() {  // 829cad
  Link_Main();
  if (link_player_handler_state != 0)
    return;
  if (link_sword_type + 1 & 0xfe)
    sound_effect_1 = 0x2C;
  link_force_hold_sword_up = 1;
  subsubmodule_index = 32;
  submodule_index++;
}

void Dungeon_CloseVictorySpin() {  // 829cd1
  if (--subsubmodule_index)
    return;
  submodule_index++;
  link_y_vel = 0;
  link_x_vel = 0;
  overworld_fixed_color_plusminus = 0;
}

void Module15_MirrorWarpFromAga() {  // 829cfc
  kModule_KillAgahnim[submodule_index]();
  if (submodule_index < 2 || submodule_index >= 5) {
    Sprite_Main();
    LinkOam_Main();
  }
}

void Module16_BossVictory_Crystal() {  // 829e8a
  switch (submodule_index) {
  case 0: BossVictory_Heal(); break;
  case 1: Dungeon_StartVictorySpin(); break;
  case 2: Dungeon_RunVictorySpin(); break;
  case 3: Dungeon_CloseVictorySpin(); break;
  case 4: Module16_04_FadeAndEnd(); break;
  }
  Sprite_Main();
  LinkOam_Main();
}

void Module16_04_FadeAndEnd() {  // 829e9a
  if (--INIDISP_copy)
    return;
  bg1_x_offset = 0;
  bg1_y_offset = 0;
  link_y_vel = 0;
  flag_is_link_immobilized = 0;
  Palette_RevertTranslucencySwap();
  link_player_handler_state = kPlayerState_Ground;
  link_receiveitem_index = 0;
  link_pose_for_item = 0;
  link_disable_sprite_damage = 0;
  main_module_index = saved_module_for_menu;
  submodule_index = 0;
  subsubmodule_index = 0;
  OpenSpotlight_Next2();
}

static uint8 PlaySfx_SetPan(uint8 a) {  // 878036
  byte_7E0CF8 = a;
  return a | Link_CalculateSfxPan();
}

void TriforceRoom_LinkApproachTriforce() {  // 87f49c
  uint8 y = link_y_coord;
  if (y < 152) {
    link_animation_steps = 0;
    link_direction = 0;
    link_direction_last = 0;
    if (!--link_delay_timer_spin_attack) {
      link_pose_for_item = 2;
      subsubmodule_index++;
    }
  } else {
    if (y < 169)
      link_speed_setting = 0x14;
    link_direction = 8;
    link_direction_last = 8;
    link_direction_facing = 0;
    link_delay_timer_spin_attack = 64;
  }
}

void AncillaAdd_ItemReceipt(uint8 ain, uint8 yin, int chest_pos) {  // 8985e8
  int ancilla = Ancilla_AddAncilla(ain, yin);
  if (ancilla < 0)
    return;

  flag_is_link_immobilized = (link_receiveitem_index == 0x20) ? 2 : 1;
  uint8 t;

  int j = link_receiveitem_index;
  if (j == 0) {
    g_ram[kMemoryLocationToGiveItemTo[4]] = kValueToGiveItemTo[0];
  }

  uint8 v = kValueToGiveItemTo[j];
  uint8 *p = &g_ram[kMemoryLocationToGiveItemTo[j]];
  if (!sign8(v))
    *p = v;

  if (j == 0x1f)
    link_is_bunny = 0;
  else if (j == 0x4b || j == 0x1e)
    link_ability_flags |= (j == 0x4b) ? 4 : 2;

  if (j == 0x1b || j == 0x1c) {
    Palette_UpdateGlovesColor();
  } else if ((t = 4, j == 0x37) || (t = 1, j == 0x38) || (t = 2, j == 0x39)) {
    *p |= t;
    if ((*p & 7) == 7)
      savegame_map_icons_indicator = 4;
    overworld_map_state++;
  } else if (j == 0x22) {
    if (*p == 0)
      *p = 1;
  } else if (j == 0x25 || j == 0x32 || j == 0x33) {
    WORD(*p) |= 0x8000 >> (BYTE(cur_palace_index_x2) >> 1);
  } else if (j == 0x3e) {
    if (link_state_bits & 0x80)
      link_picking_throw_state = 2;
  } else if (j == 0x20) {
    overworld_map_state++;
    for (int i = 4; i >= 0; i--) {
      if (ancilla_type[i] == 7 || ancilla_type[i] == 0x2c) {
        ancilla_type[i] = 0;
        link_state_bits = 0;
        link_picking_throw_state = 0;
      }
    }
    if (link_cape_mode) {
      link_bunny_transform_timer = 32;
      link_disable_sprite_damage = 0;
      link_cape_mode = 0;
      AncillaAdd_CapePoof(0x23, 4);
      sound_effect_1 = 0x15 | Link_CalculateSfxPan();
    }
  } else if (j == 0x29) {
    if (link_item_mushroom != 2) {
      *p = 1;
      Hud_RefreshIcon();
    }
  } else if ((t = 1, j == 0x24) || item_receipt_method != 2 && (j == 0x27 || (t = 3, j == 0x28) || (t = 10, j == 0x31))) {
    *p += t;
    if (*p > 99)
      *p = 99;
    Hud_RefreshIcon();
  } else if (j == 0x17) {
    *p = (*p + 1) & 3;
    sound_effect_2 = 0x2d | Link_CalculateSfxPan();
  } else if (j == 1) {
    Overworld_SetSongList();
  } else {
    ItemReceipt_GiveBottledItem(j);
  }

  uint8 gfx = kReceiveItemGfx[j];
  if (gfx == 0xff) {
    gfx = 0;
  } else if (gfx == 0x20 || gfx == 0x2d || gfx == 0x2e) {
    DecompressShieldGraphics();
    Palette_Load_Shield();
  }
  DecodeAnimatedSpriteTile_variable(gfx);

  if ((gfx == 6 || gfx == 0x18) && j != 0) {
    DecompressSwordGraphics();
    Palette_Load_Sword();
  }

  ancilla_item_to_link[ancilla] = j;
  ancilla_arr1[ancilla] = 0;

  if (j == 1 && item_receipt_method != 2) {
    ancilla_timer[ancilla] = 160;
    submodule_index = 43;
    BYTE(palette_filter_countdown) = 0;
    AncillaAdd_MSCutscene(0x35, 4);
    ancilla_arr3[ancilla] = 2;
  } else {
    ancilla_arr3[ancilla] = 9;
  }
  ancilla_arr4[ancilla] = 5;
  ancilla_step[ancilla] = item_receipt_method;

  ancilla_aux_timer[ancilla] =
    (j == 0x20 || j == 0x37 || j == 0x38 || j == 0x39) ? 0x68 :
    (j == 0x26) ? 0x2 : (item_receipt_method ? 0x38 : 0x60);

  int x, y;

  if (item_receipt_method == 1) {
    y = (chest_pos & 0x1f80) >> 4;
    x = (chest_pos & 0x7e) << 2;
    y += dung_loade_bgoffs_v_copy & ~0xff;
    x += dung_loade_bgoffs_h_copy & ~0xff;
    y += kReceiveItem_Tab2[j];
    x += kReceiveItem_Tab3[j];
  } else {
    if (ancilla_step[ancilla] == 0 && j == 1) {
      sound_effect_1 = Link_CalculateSfxPan() | 0x2c;
    } else if (j == 0x20 || j == 0x37 || j == 0x38 || j == 0x39) {
      music_control = Link_CalculateSfxPan() | 0x13;
    } else if (j != 0x3e && j != 0x17) {
      sound_effect_2 = Link_CalculateSfxPan() | 0xf;
    }
    int method = item_receipt_method == 3 ? 0 : item_receipt_method;
    x = (method != 0) ? kReceiveItem_Tab3[j] :
      (kReceiveItem_Tab1[j] == 0) ? 10 : (j == 0x20) ? 0 : 6;
    x += link_x_coord;
    y = method ? kReceiveItem_Tab2[j] : -14;
    y += link_y_coord + ((method == 2) ? -8 : 0);
  }
  Ancilla_SetXY(ancilla, x, y);
}

void ItemReceipt_GiveBottledItem(uint8 item) {  // 89893e
  static const uint8 kBottleList[7] = { 0x16, 0x2b, 0x2c, 0x2d, 0x3d, 0x3c, 0x48 };
  static const uint8 kPotionList[5] = { 0x2e, 0x2f, 0x30, 0xff, 0xe };
  int j;
  if ((j = FindInByteArray(kBottleList, item, 7)) >= 0) {
    for (int i = 0; i != 4; i++) {
      if (link_bottle_info[i] < 2) {
        link_bottle_info[i] = j + 2;
        return;
      }
    }
  }
  if ((j = FindInByteArray(kPotionList, item, 5)) >= 0) {
    for (int i = 0; i != 4; i++) {
      if (link_bottle_info[i] == 2) {
        link_bottle_info[i] = j + 3;
        return;
      }
    }
  }
}

void Module17_SaveAndQuit() {  // 89f79f
  switch (submodule_index) {
  case 0:
    submodule_index++;
  case 1:
    if (!--INIDISP_copy) {
      MOSAIC_copy = 15;
      subsubmodule_index = 1;
      Death_Func15();
    }
    break;
  }
  Sprite_Main();
  LinkOam_Main();
}

void WallMaster_SendPlayerToLastEntrance() {  // 8bffa8
  SaveDungeonKeys();
  Dungeon_FlagRoomData_Quadrants();
  Sprite_ResetAll();
  death_var4 = 0;
  main_module_index = 17;
  submodule_index = 0;
  nmi_load_bg_from_vram = 0;
  ResetSomeThingsAfterDeath(17);  // wtf: argument?
}

uint8 GetRandomNumber() {  // 8dba71
  uint8 t = byte_7E0FA1 + frame_counter;
  t = (t & 1) ? (t >> 1) : (t >> 1) ^ 0xb8;
  byte_7E0FA1 = t;
  return t;
}

uint8 Link_CalculateSfxPan() {  // 8dbb67
  return CalculateSfxPan(link_x_coord);
}

void SpriteSfx_QueueSfx1WithPan(int k, uint8 a) {  // 8dbb6e
  if (sound_effect_ambient == 0)
    sound_effect_ambient = a | Sprite_CalculateSfxPan(k);
}

void SpriteSfx_QueueSfx2WithPan(int k, uint8 a) {  // 8dbb7c
  if (sound_effect_1 == 0)
    sound_effect_1 = a | Sprite_CalculateSfxPan(k);
}

void SpriteSfx_QueueSfx3WithPan(int k, uint8 a) {  // 8dbb8a
  if (sound_effect_2 == 0)
    sound_effect_2 = a | Sprite_CalculateSfxPan(k);
}

uint8 Sprite_CalculateSfxPan(int k) {  // 8dbba1
  return CalculateSfxPan(Sprite_GetX(k));
}

uint8 CalculateSfxPan(uint16 x) {  // 8dbba8
  static const uint8 kPanTable[] = { 0, 0x80, 0x40 };
  int o = 0;
  x -= BG2HOFS_copy2 + 80;
  if (x >= 80)
    o = 1 + ((int16)x >= 0);
  return kPanTable[o];
}

uint8 CalculateSfxPan_Arbitrary(uint8 a) {  // 8dbbd0
  static const uint8 kTorchPans[] = { 0x80, 0x80, 0x80, 0, 0, 0x40, 0x40, 0x40 };
  return kTorchPans[((a - BG2HOFS_copy2) >> 5) & 7];
}

void Init_LoadDefaultTileAttr() {  // 8e97d9
  memcpy(attributes_for_tile, kDungeon_DefaultAttr, 0x140);
  memcpy(attributes_for_tile + 0x1c0, kDungeon_DefaultAttr + 0x140, 64);
}

void Main_ShowTextMessage() {  // 8ffdaa
  if (main_module_index != 14) {
    byte_7E0223 = 0;
    messaging_module = 0;
    submodule_index = 2;
    saved_module_for_menu = main_module_index;
    main_module_index = 14;
  }
}

uint8 HandleItemTileAction_Overworld(uint16 x, uint16 y) {  // 9bbd7a
  if (player_is_indoors)
    return HandleItemTileAction_Dungeon(x, y);
  else
    return Overworld_ToolAndTileInteraction(x, y);
}

