#include "zelda_rtl.h"
#include "snes/snes_regs.h"
#include "variables.h"
#include "load_gfx.h"
#include "dungeon.h"
#include "sprite.h"
#include "ending.h"
#include "overworld.h"
#include "player.h"
#include "misc.h"
#include "messaging.h"
#include "player_oam.h"
#include "tables/generated_ending.h"
#include "sprite_main.h"
#include "ancilla.h"
#include "hud.h"

static const uint16 kPolyhedralPalette[8] = { 0, 0x14d, 0x1b0, 0x1f3, 0x256, 0x279, 0x2fd, 0x35f };

#define ending_which_dung (*(uint16*)(g_ram+0xcc))
#define kPolyThreadRam (g_ram + 0x1f00)
static const int8 kIntroSprite0_Xvel[3] = { 1, 0, -1 };
static const int8 kIntroSprite0_Yvel[3] = { -1, 1, -1 };
static const uint8 kIntroSprite3_X[4] = { 0xc2, 0x98, 0x6f, 0x34 };
static const uint8 kIntroSprite3_Y[4] = { 0x7c, 0x54, 0x7c, 0x57 };
static const uint8 kIntroSprite3_State[8] = { 0, 1, 2, 3, 2, 1, 0xff, 0xff };
static const uint8 kTriforce_Xfinal[3] = { 0x59, 0x5f, 0x67 };
static const uint8 kTriforce_Yfinal[3] = { 0x74, 0x68, 0x74 };
static const uint16 kEndingSprites_X[] = {
  0x1e0, 0x200, 0x1ed, 0x203, 0x1da, 0x216, 0x1c8, 0x228, 0x1c0, 0x1e0, 0x208, 0x228,
  0xf8, 0xf0,
  0x278, 0x298, 0x1e0, 0x200, 0x220, 0x288, 0x1e2,
  0xe0, 0x150, 0xe8, 0x168, 0x128, 0x170, 0x170,
  0x335, 0x335, 0x300,
  0xb8, 0xce, 0xac, 0xc4,
  0x3b0, 0x390, 0x3d0,
  0xf8, 0xc8,
  0x80,
  0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xe8, 0xf8, 0xd8, 0xf8, 0xc8, 0x108,
  0x70, 0x70, 0x70, 0x68, 0x88, 0x70,
  0x40, 0x70, 0x4f, 0x61, 0x37, 0x79,
  0xc8, 0x278, 0x258, 0x1d8, 0x1c8, 0x188, 0x270,
  0x180,
  0x2e8, 0x270, 0x270, 0x2a0, 0x2a0, 0x2a4, 0x2fc,
  0x76, 0x73, 0x76, 0x0, 0xd0, 0x80,
};
static const uint16 kEndingSprites_Y[] = {
  0x158, 0x158, 0x138, 0x138, 0x140, 0x140, 0x150, 0x150, 0x120, 0x120, 0x120, 0x120,
  0x60, 0x37,
  0xc2, 0xc2, 0x16b, 0x16c, 0x16b, 0xb8, 0x16b,
  0x80, 0x60, 0x146, 0x146, 0x1c6, 0x70, 0x70,
  0x128, 0x128, 0x16f,
  0xf5, 0xfc, 0x10d, 0x10d,
  0x40, 0x40, 0x40,
  0x150, 0x158,
  0xf4,
  0x120, 0x120, 0x120, 0x120, 0x120, 0x108, 0x100, 0xd8, 0xd8, 0xf0, 0xf0,
  0x3c, 0x3c, 0x3c, 0x90, 0x80, 0x3c,
  0x16c, 0x16c, 0x174, 0x174, 0x175, 0x175,
  0x250, 0x2b0, 0x2b0, 0x2a0, 0x2b0, 0x2b0, 0x2b8,
  0xd8,
  0x24b, 0x1b0, 0x1c8, 0x1c8, 0x1b0, 0x230, 0x230,
  0x8b, 0x83, 0x85, 0x2c, 0xf8, 0x100,
};
static const uint8 kEndingSprites_Idx[17] = {
  0, 12, 14, 21, 28, 31, 35, 38, 40, 41, 52, 58, 64, 71, 72, 79, 85
};
static PlayerHandlerFunc *const kEndSequence0_Funcs[3] = {
&Credits_LoadScene_Overworld_PrepGFX,
&Credits_LoadScene_Overworld_Overlay,
&Credits_LoadScene_Overworld_LoadMap,
};
static PrepOamCoordsRet g_ending_coords;
static const uint16 kEnding1_TargetScrollY[16] = { 0x6f2, 0x210, 0x72c, 0xc00, 0x10c, 0xa9b, 0x10, 0x510, 0x89, 0xa8e, 0x222c, 0x2510, 0x826, 0x5c, 0x20a, 0x30 };
static const uint16 kEnding1_TargetScrollX[16] = { 0x77f, 0x480, 0x193, 0xaa, 0x878, 0x847, 0x4fd, 0xc57, 0x40f, 0x478, 0xa00, 0x200, 0x201, 0xaa1, 0x26f, 0 };
static const int8 kEnding1_Yvel[16] = { -1, -1, 1, -1, 1, 1, 0, 1, 0, -1, -1, 0, 0, 0, 1, -1 };
static const int8 kEnding1_Xvel[16] = { 0, 0, -1, 0, 0, -1, 1, 0, -1, 0, 0, 0, 1, -1, 1, 0 };
static PlayerHandlerFunc *const kEndSequence_Funcs[39] = {
&Credits_LoadNextScene_Overworld,
&Credits_ScrollScene_Overworld,
&Credits_LoadNextScene_Dungeon,
&Credits_ScrollScene_Dungeon,
&Credits_LoadNextScene_Overworld,
&Credits_ScrollScene_Overworld,
&Credits_LoadNextScene_Overworld,
&Credits_ScrollScene_Overworld,
&Credits_LoadNextScene_Overworld,
&Credits_ScrollScene_Overworld,
&Credits_LoadNextScene_Overworld,
&Credits_ScrollScene_Overworld,
&Credits_LoadNextScene_Overworld,
&Credits_ScrollScene_Overworld,
&Credits_LoadNextScene_Overworld,
&Credits_ScrollScene_Overworld,
&Credits_LoadNextScene_Overworld,
&Credits_ScrollScene_Overworld,
&Credits_LoadNextScene_Overworld,
&Credits_ScrollScene_Overworld,
&Credits_LoadNextScene_Dungeon,
&Credits_ScrollScene_Dungeon,
&Credits_LoadNextScene_Dungeon,
&Credits_ScrollScene_Dungeon,
&Credits_LoadNextScene_Overworld,
&Credits_ScrollScene_Overworld,
&Credits_LoadNextScene_Overworld,
&Credits_ScrollScene_Overworld,
&Credits_LoadNextScene_Overworld,
&Credits_ScrollScene_Overworld,
&Credits_LoadNextScene_Overworld,
&Credits_ScrollScene_Overworld,
&EndSequence_32,
&Credits_BrightenTriangles,
&Credits_FadeColorAndBeginAnimating,
&Credits_StopCreditsScroll,
&Credits_FadeAndDisperseTriangles,
&Credits_FadeInTheEnd,
&Credits_HangForever,
};
#define intro_sword_ypos WORD(g_ram[0xc8])
#define intro_sword_18 g_ram[0xca]
#define intro_sword_19 g_ram[0xcb]
#define intro_sword_20 g_ram[0xcc]
#define intro_sword_21 g_ram[0xcd]
#define intro_sword_24 g_ram[0xd0]
static const uint16 kEnding_Tab1[16] = {
  0x1000, 2, 0x1002, 0x1012, 0x1004, 0x1006, 0x1010, 0x1014, 0x100a,
  0x1016, 0x5d, 0x64, 0x100e, 0x1008, 0x1018, 0x180 };
static const uint8 kEnding_SpritePack[17] = {
  0x28, 0x46, 0x27, 0x2e, 0x2b, 0x2b, 0xe, 0x2c, 0x1a, 0x29, 0x47, 0x28, 0x27, 0x28, 0x2a, 0x28, 0x2d,
};
static const uint8 kEnding_SpritePal[17] = {
  1, 0x40, 1, 4, 1, 1, 1, 0x11, 1, 1, 0x47, 0x40, 1, 1, 1, 1, 1,
};
void Intro_SetupScreen() {  // 828000
  nmi_disable_core_updates = 0x80;
  EnableForceBlank();
  TM_copy = 16;
  TS_copy = 0;
  Intro_InitializeBackgroundSettings();
  CGWSEL_copy = 0x20;
  zelda_ppu_write(OBSEL, 2);
  load_chr_halfslot_even_odd = 20;
  Graphics_LoadChrHalfSlot();
  load_chr_halfslot_even_odd = 0;
  LoadOWMusicIfNeeded();

  zelda_ppu_write(VMAIN, 0x80);
  zelda_ppu_write_word(VMADDL, 0x27f0);
  int i = 16;
  do {
    zelda_ppu_write_word(VMDATAL, 0);
    main_palette_buffer[144 + i] = 0x7fff;
  } while (--i >= 0);
  R16 = 0x1ffe;
  R18 = 0x1bfe;
}

void Intro_LoadTextPointersAndPalettes() {  // 828116
  Text_GenerateMessagePointers();
  Overworld_LoadAllPalettes();
}

void Credits_LoadScene_Overworld_PrepGFX() {  // 828604
  EnableForceBlank();
  EraseTileMaps_normal();
  CGWSEL_copy = 0x82;
  int k = submodule_index >> 1;
  dungeon_room_index = kEnding_Tab1[k];

  if (k != 6 && k != 15)
    LoadOverworldFromDungeon();
  else
    Overworld_EnterSpecialArea();
  music_control = 0;
  sound_effect_ambient = 0;

  int t = BYTE(overworld_screen_index) & ~0x40;
  DecompressAnimatedOverworldTiles((t == 3 || t == 5 || t == 7) ? 0x58 : 0x5a);

  k = submodule_index >> 1;
  sprite_graphics_index = kEnding_SpritePack[k];
  uint8 sprpal = kEnding_SpritePal[k];
  InitializeTilesets();
  OverworldLoadScreensPaletteSet();
  Overworld_LoadPalettes(GetOverworldBgPalette(BYTE(overworld_screen_index)), sprpal);

  hud_palette = 1;
  Palette_Load_HUD();
  if (!submodule_index)
    TransferFontToVRAM();
  Overworld_LoadPalettesInner();
  Overworld_SetFixedColAndScroll();
  if (BYTE(overworld_screen_index) >= 128)
    Palette_SetOwBgColor();
  BGMODE_copy = 9;
  subsubmodule_index++;
}

void Credits_LoadScene_Overworld_Overlay() {  // 828697
  Overworld_LoadOverlays2();
  music_control = 0;
  sound_effect_ambient = 0;
  submodule_index--;
  subsubmodule_index++;
}

void Credits_LoadScene_Overworld_LoadMap() {  // 8286a5
  Overworld_LoadAndBuildScreen();
  Credits_PrepAndLoadSprites();
  R16 = 0;
  subsubmodule_index = 0;
}

void Credits_OperateScrollingAndTileMap() {  // 8286b3
  Credits_HandleCameraScrollControl();
  if (BYTE(overworld_screen_trans_dir_bits2))
    OverworldHandleMapScroll();
}

void Credits_LoadCoolBackground() {  // 8286c0
  main_tile_theme_index = 33;
  aux_tile_theme_index = 59;
  sprite_graphics_index = 45;
  InitializeTilesets();
  BYTE(overworld_screen_index) = 0x5b;
  Overworld_LoadPalettes(GetOverworldBgPalette(BYTE(overworld_screen_index)), 0x13);
  overworld_palette_aux2_bp5to7_hi = 3;
  Palette_Load_OWBG2();
  Overworld_CopyPalettesToCache();
  Overworld_LoadOverlays2();
  BG1VOFS_copy2 = 0;
  BG1HOFS_copy2 = 0;
  submodule_index--;
}

void Credits_LoadScene_Dungeon() {  // 8286fd
  EnableForceBlank();
  EraseTileMaps_normal();
  WORD(which_entrance) = kEnding_Tab1[submodule_index >> 1];

  Dungeon_LoadEntrance();
  dung_num_lit_torches = 0;
  hdr_dungeon_dark_with_lantern = 0;
  Dungeon_LoadAndDrawRoom();
  DecompressAnimatedDungeonTiles(kDungAnimatedTiles[main_tile_theme_index]);

  int i = submodule_index >> 1;
  sprite_graphics_index = kEnding_SpritePack[i];
  const DungPalInfo *dpi = GetDungPalInfo(kEnding_SpritePal[i] & 0x3f);
  sprite_aux1_palette = dpi->pal2;
  sprite_aux2_palette = dpi->pal3;
  misc_sprites_graphics_index = 10;
  InitializeTilesets();
  palette_sp6 = 10;
  Dungeon_LoadPalettes();
  BGMODE_copy = 9;
  R16 = 0;
  INIDISP_copy = 0;
  submodule_index++;
  Credits_PrepAndLoadSprites();
}

void Module18_GanonEmerges() {  // 829edc
  uint16 hofs2 = BG2HOFS_copy2;
  uint16 vofs2 = BG2VOFS_copy2;
  uint16 hofs1 = BG1HOFS_copy2;
  uint16 vofs1 = BG1VOFS_copy2;

  BG2HOFS_copy2 = BG2HOFS_copy = hofs2 + bg1_x_offset;
  BG2VOFS_copy2 = BG2VOFS_copy = vofs2 + bg1_y_offset;
  BG1HOFS_copy2 = BG1HOFS_copy = hofs1 + bg1_x_offset;
  BG1VOFS_copy2 = BG1VOFS_copy = vofs1 + bg1_y_offset;
  Sprite_Main();
  BG1VOFS_copy2 = vofs1;
  BG1HOFS_copy2 = hofs1;
  BG2VOFS_copy2 = vofs2;
  BG2HOFS_copy2 = hofs2;

  switch (overworld_map_state) {
  case 0:  // GetBirdForPursuit
    Dungeon_HandleLayerEffect();
    CallForDuckIndoors();
    SaveDungeonKeys();
    overworld_map_state++;
    flag_is_link_immobilized++;
    break;
  case 1:  // PrepForPyramidLocation
    Dungeon_HandleLayerEffect();
    if (submodule_index == 10) {
      overworld_screen_index = 91;
      player_is_indoors = 0;
      main_module_index = 24;
      submodule_index = 0;
      overworld_map_state = 2;
    }
    break;
  case 2:  // FadeOutDungeonScreen
    Dungeon_HandleLayerEffect();
    if (--INIDISP_copy)
      break;
    EnableForceBlank();
    overworld_map_state++;
    Hud_RebuildIndoor();
    link_x_vel = link_y_vel = 0;
    break;
  case 3:  // LOadPyramidArea
    birdtravel_var1[0] = 8;
    birdtravel_var1[1] = 0;
    FluteMenu_LoadSelectedScreen();
    LoadOWMusicIfNeeded();
    music_control = 9;
    break;
  case 4:  // LoadAmbientOverlay
    Overworld_LoadOverlayAndMap();
    subsubmodule_index = 0;
    break;
  case 5:  // BrightenScreenThenSpawnBat
    if (++INIDISP_copy == 15) {
      dung_savegame_state_bits = 0;
      flag_unk1 = 0;
      Sprite_SpawnBatCrashCutscene();
      link_direction_facing = 2;
      saved_module_for_menu = 9;
      player_is_indoors = 0;
      overworld_map_state++;
      subsubmodule_index = 128;
      BYTE(cur_palace_index_x2) = 255;
    }
    break;
  case 6:  // DelayForBatSmashIntoPyramid
    break;
  case 7:  // DelayPlayerDropOff
    if (!--subsubmodule_index)
      overworld_map_state++;
    break;
  case 8:  // DropOffPlayerAtPyramid
    BirdTravel_Finish_Doit();
    break;
  }

  LinkOam_Main();
}

void Module19_TriforceRoom() {  // 829fec
  switch (subsubmodule_index) {
  case 0:  //
    Link_ResetProperties_A();
    link_last_direction_moved_towards = 0;
    music_control = 0xf1;
    ResetTransitionPropsAndAdvance_ResetInterface();
    break;
  case 1:  //
    ConditionalMosaicControl();
    ApplyPaletteFilter_bounce();
    break;
  case 2:  //
    EnableForceBlank();
    zelda_snes_dummy_write(NMITIMEN, 0);
    LoadCreditsSongs();
    zelda_snes_dummy_write(NMITIMEN, 0x81);
    dungeon_room_index = 0x189;
    EraseTileMaps_normal();
    Palette_RevertTranslucencySwap();
    Overworld_EnterSpecialArea();
    Overworld_LoadOverlays2();
    subsubmodule_index++;
    main_module_index = 25;
    submodule_index = 0;
    break;
  case 3:  //
    main_tile_theme_index = 36;
    sprite_graphics_index = 125;
    aux_tile_theme_index = 81;
    InitializeTilesets();
    Overworld_LoadAreaPalettesEx(4);
    Overworld_LoadPalettes(14, 0);
    SpecialOverworld_CopyPalettesToCache();
    subsubmodule_index++;
    break;
  case 4: { //
    uint8 bak0 = subsubmodule_index;
    Module08_02_LoadAndAdvance();
    subsubmodule_index = bak0 + 1;
    INIDISP_copy = 15;
    palette_filter_countdown = 31;
    mosaic_target_level = 0;
    HIBYTE(BG1HOFS_copy2) = 1;
    CGWSEL_copy = 2;
    CGADSUB_copy = 50;
    mosaic_level = 240;
    BYTE(link_y_coord) = 236;
    BYTE(link_x_coord) = 120;
    link_is_on_lower_level = 2;
    music_control = 32;
    main_module_index = 25;
    submodule_index = 0;
    break;
  }
  case 5:  //
    link_direction = 8;
    link_direction_last = 8;
    link_direction_facing = 0;
    if (BYTE(link_y_coord) < 192) {
      link_direction = 0;
      link_direction_last = 0;
      link_animation_steps = 0;
      subsubmodule_index++;
    }
    break;
  case 6:  //
    if (!(palette_filter_countdown & 1) && mosaic_level != 0)
      mosaic_level -= 0x10;
    BGMODE_copy = 9;
    MOSAIC_copy = mosaic_level | 7;
    ApplyPaletteFilter_bounce();
    break;
  case 7:  //
    TriforceRoom_PrepGFXSlotForPoly();
    dialogue_message_index = 0x173;
    Main_ShowTextMessage();
    RenderText();
    BYTE(R16) = 0x80;
    main_module_index = 25;
    subsubmodule_index++;
    break;
  case 8:  //
  case 10:  //
    AdvancePolyhedral();
    if (subsubmodule_index == 11) {
      music_control = 33;
      main_module_index = 25;
      link_direction = 0;
      link_direction_last = 0;
      submodule_index++;
    }
    break;
  case 9:  //
    AdvancePolyhedral();
    RenderText();
    if (!submodule_index) {
      overworld_map_state = 0;
      main_module_index = 25;
      subsubmodule_index++;
    }
    break;
  case 11:  //
    AdvancePolyhedral();
    TriforceRoom_LinkApproachTriforce();
    if (subsubmodule_index == 12) {
      link_direction = 0;
      link_direction_last = 0;
    }
    break;
  case 12:  //
    AdvancePolyhedral();
    if (!--BYTE(R16)) {
      Palette_AnimGetMasterSword2();
      submodule_index++;
    }
    break;
  case 13:  //
    AdvancePolyhedral();
    PaletteFilter_BlindingWhiteTriforce();
    if (BYTE(darkening_or_lightening_screen) == 255)
      subsubmodule_index++;
    break;
  case 14:  //
    if (!--INIDISP_copy) {
      main_module_index = 26;
      submodule_index = 0;
      subsubmodule_index = 0;
      irq_flag = 255;
      is_nmi_thread_active = 0;
      nmi_flag_update_polyhedral = 0;
      savegame_is_darkworld = 0;
    }
    break;
  }
  BG1HOFS_copy = BG1HOFS_copy2;
  BG1VOFS_copy = BG1VOFS_copy2;
  BG2HOFS_copy = BG2HOFS_copy2;
  BG2VOFS_copy = BG2VOFS_copy2;
  if (subsubmodule_index < 7 || subsubmodule_index >= 11) {
    Link_HandleVelocity();
    Link_HandleMovingAnimation_FullLongEntry();
  }
  LinkOam_Main();
}

void Intro_InitializeBackgroundSettings() {  // 82c500
  zelda_ppu_write(SETINI, 0);
  BGMODE_copy = 9;
  MOSAIC_copy = 0;
  zelda_ppu_write(BG1SC, 0x13);
  zelda_ppu_write(BG2SC, 3);
  zelda_ppu_write(BG3SC, 0x63);
  zelda_ppu_write(BG12NBA, 0x22);
  zelda_ppu_write(BG34NBA, 7);
  CGADSUB_copy = 32;
  COLDATA_copy0 = 32;
  COLDATA_copy1 = 64;
  COLDATA_copy2 = 128;
}

void Polyhedral_InitializeThread() {  // 89f7de
  static const uint8 kPolyThreadInit[13] = { 9, 0, 0x1f, 0, 0, 0, 0, 0, 0, 0x30, 0x1d, 0xf8, 9 };
  memset(kPolyThreadRam, 0, 256);
  thread_other_stack = 0x1f31;
  memcpy(&g_ram[0x1f32], kPolyThreadInit, 13);
}

void Module00_Intro() {  // 8cc120
  if (submodule_index >= 8 && ((filtered_joypad_L & 0xc0 | filtered_joypad_H) & 0xd0)) {
    FadeMusicAndResetSRAMMirror();
    return;
  }
  switch (submodule_index) {
  case 0: Intro_Init(); break;
  case 1: Intro_Init_Continue(); break;
  case 10:
  case 2: Intro_InitializeTriforcePolyThread(); break;
  case 3:
  case 4:
  case 9:
  case 11: Intro_HandleAllTriforceAnimations(); break;
  case 5: IntroZeldaFadein(); break;
  case 6: Intro_SwordComingDown(); break;
  case 7: Intro_FadeInBg(); break;
  case 8: Intro_WaitPlayer(); break;
  }
}

void Intro_Init() {  // 8cc15d
  Intro_SetupScreen();
  INIDISP_copy = 15;
  subsubmodule_index = 0;
  flag_update_cgram_in_nmi++;
  submodule_index++;
  sound_effect_2 = 10;
  Intro_Init_Continue();
}

void Intro_Init_Continue() {  // 8cc170
  Intro_DisplayLogo();
  int t = subsubmodule_index++;
  if (t >= 11) {
    if (--INIDISP_copy)
      return;
    Intro_InitializeMemory_darken();
    return;
  }
  switch (t) {
  case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7:
    Intro_Clear1kbBlocksOfWRAM();
    break;
  case 8: Intro_LoadTextPointersAndPalettes(); break;
  case 9: LoadItemGFXIntoWRAM4BPPBuffer(); break;
  case 10:LoadFollowerGraphics(); break;
  }
}

void Intro_Clear1kbBlocksOfWRAM() {  // 8cc1a0
  uint16 i = R16;
  uint8 *dst = (uint8 *)&g_ram[0x2000];
  do {
    for (int j = 0; j < 15; j++)
      WORD(dst[i + j * 0x2000]) = 0;
  } while ((i -= 2) != R18);
  R16 = i;
  R18 = i - 0x400;
}

void Intro_InitializeMemory_darken() {  // 8cc1f5
  EnableForceBlank();
  EraseTileMaps_normal();
  zelda_ppu_write(OBSEL, 2);
  main_tile_theme_index = 35;
  sprite_graphics_index = 125;
  aux_tile_theme_index = 81;
  misc_sprites_graphics_index = 8;
  LoadDefaultGraphics();
  InitializeTilesets();
  DecompressAnimatedDungeonTiles(0x5d);
  bg_tile_animation_countdown = 2;
  BYTE(overworld_screen_index) = 0;
  dung_hdr_palette_1 = 0;
  overworld_palette_aux3_bp7_lo = 0;
  R16 = 0;
  R18 = 0;
  darkening_or_lightening_screen = 2;
  palette_filter_countdown = 31;
  mosaic_target_level = 0;
  submodule_index++;
}

void IntroZeldaFadein() {  // 8cc25c
  Intro_HandleAllTriforceAnimations();
  if (!(frame_counter & 1))
    return;
  Palette_FadeIntroOneStep();
  if (BYTE(palette_filter_countdown) == 0) {
    subsubmodule_index = 42;
    submodule_index++;
    Intro_SetupSwordAndIntroFlash();
  } else if (BYTE(palette_filter_countdown) == 13) {
    TM_copy = 0x15;
    TS_copy = 0;
  }
}

void Intro_FadeInBg() {  // 8cc284
  Intro_PeriodicSwordAndIntroFlash();
  Intro_HandleAllTriforceAnimations();
  if (BYTE(palette_filter_countdown)) {
    if (frame_counter & 1)
      Palette_FadeIntro2();
  } else {
    if ((filtered_joypad_L & 0xc0 | filtered_joypad_H) & 0xd0)
      FadeMusicAndResetSRAMMirror();
    else {
      if (!--subsubmodule_index)
        submodule_index++;
    }
  }
}

void Intro_SwordComingDown() {  // 8cc2ae
  Intro_HandleAllTriforceAnimations();
  intro_did_run_step = 0;
  is_nmi_thread_active = 0;
  Intro_PeriodicSwordAndIntroFlash();
  if (!--subsubmodule_index) {
    submodule_index++;
    CGWSEL_copy = 2;
    CGADSUB_copy = 0x22;
    palette_filter_countdown = 31;
    TS_copy = 2;
  }
}

void Intro_WaitPlayer() {  // 8cc2d4
  Intro_HandleAllTriforceAnimations();
  intro_did_run_step = 0;
  is_nmi_thread_active = 0;
  Intro_PeriodicSwordAndIntroFlash();
  if (!--subsubmodule_index) {
    submodule_index++;
    main_module_index = 20;
    submodule_index = 0;
    BYTE(link_x_coord) = 0;
  }
}

void FadeMusicAndResetSRAMMirror() {  // 8cc2f0
  irq_flag = 255;
  TM_copy = 0x15;
  TS_copy = 0;
  player_is_indoors = 0;
  music_control = 0xf1;
  SetBackdropcolorBlack();

  memset(&link_y_coord, 0, 0x70);
  memset(save_dung_info, 0, 256 * 5);

  main_module_index = 1;
  death_var4 = 1;
  submodule_index = 0;
}

void Intro_InitializeTriforcePolyThread() {  // 8cc33c
  misc_sprites_graphics_index = 8;
  LoadCommonSprites_2();
  Intro_InitGfx_Helper();
  intro_sprite_isinited[0] = 1;
  intro_sprite_isinited[1] = 1;
  intro_sprite_isinited[2] = 1;
  intro_sprite_subtype[0] = 0;
  intro_sprite_subtype[1] = 0;
  intro_sprite_subtype[2] = 0;
  intro_sprite_isinited[4] = 1;
  intro_sprite_subtype[4] = 2;
  INIDISP_copy = 15;
  submodule_index++;
}

void Intro_InitGfx_Helper() {  // 8cc36f
  Polyhedral_InitializeThread();
  LoadTriforceSpritePalette();
  virq_trigger = 0x90;
  poly_config1 = 255;
  poly_base_x = 32;
  poly_base_y = 32;
  BYTE(poly_var1) = 32;
  poly_a = 0xA0;
  poly_b = 0x60;
  poly_config_color_mode = 1;
  poly_which_model = 1;
  is_nmi_thread_active = 1;
  intro_did_run_step = 1;
  memset(&intro_step_index, 0, 7 * 16);
}

void LoadTriforceSpritePalette() {  // 8cc3bd
  memcpy(main_palette_buffer + 0xd0, kPolyhedralPalette, 16);
  flag_update_cgram_in_nmi++;
}

void Intro_HandleAllTriforceAnimations() {  // 8cc404
  intro_frame_ctr++;
  Intro_AnimateTriforce();
  Scene_AnimateEverySprite();
}

void Scene_AnimateEverySprite() {  // 8cc412
  intro_sprite_alloc = 0x800;
  for (int k = 7; k >= 0; k--)
    Intro_AnimOneObj(k);
}

void Intro_AnimateTriforce() {  // 8cc435
  is_nmi_thread_active = 1;
  if (!intro_did_run_step) {
    Intro_RunStep();
    intro_did_run_step = 1;
  }
}

void Intro_RunStep() {  // 8cc448
  switch (intro_step_index) {
  case 0:
    if (++intro_step_timer == 64)
      intro_step_index++;
    poly_b += 5, poly_a += 3;
    break;
  case 1:
    if (poly_config1 < 2) {
      poly_config1 = 0;
      intro_step_index++;
      intro_step_timer = 64;
      return;
    }
    poly_config1 -= 2;
    poly_b += 5;
    poly_a += 3;
    if (poly_config1 < 225)
      submodule_index = 4;
    if (poly_config1 == 113)
      music_control = 1;
    break;
  case 2:
    if (!--intro_step_timer) {
      intro_step_index++;
    } else {
      poly_b += 5, poly_a += 3;
    }
    break;
  case 3:
    if (poly_b >= 250 && poly_a >= 252) {
      intro_step_index++;
      intro_step_timer = 32;
    } else {
      poly_b += 5, poly_a += 3;
    }
    break;
  case 4:
    poly_b = 0;
    poly_a = 0;
    if (!--intro_step_timer) {
      intro_step_index++;
      intro_sprite_isinited[5] = 1;
      intro_sprite_subtype[5] = 3;
      TM_copy = 0x10;
      TS_copy = 5;
      CGWSEL_copy = 2;
      CGADSUB_copy = 0x31;
      subsubmodule_index = 0;
      flag_update_cgram_in_nmi++;
      nmi_load_bg_from_vram = 3;
      submodule_index++;
    }
    break;
  }

}

void Intro_AnimOneObj(int k) {  // 8cc534
  switch (intro_sprite_isinited[k]) {
  case 0:
    break;
  case 1:
    switch (intro_sprite_subtype[k]) {
    case 0: Intro_SpriteType_A_0(k); break;
    case 1: EXIT_0CCA90(k); break;
    case 2: InitializeSceneSprite_Copyright(k); break;
    case 3: InitializeSceneSprite_Sparkle(k); break;
    case 4:
    case 5:
    case 6: InitializeSceneSprite_TriforceRoomTriangle(k); break;
    case 7: InitializeSceneSprite_CreditsTriangle(k); break;
    }
    break;
  case 2:
    switch (intro_sprite_subtype[k]) {
    case 0: Intro_SpriteType_B_0(k); break;
    case 1: EXIT_0CCA90(k); break;
    case 2: AnimateSceneSprite_Copyright(k); break;
    case 3: AnimateSceneSprite_Sparkle(k); break;
    case 4:
    case 5:
    case 6: Intro_SpriteType_B_456(k); break;
    case 7: AnimateSceneSprite_CreditsTriangle(k); break;
    }
    break;
  }
}

void Intro_SpriteType_A_0(int k) {  // 8cc57e
  static const int16 kIntroSprite0_X[3] = { -38, 95, 230 };
  static const int16 kIntroSprite0_Y[3] = { 200, -67, 200 };
  intro_x_lo[k] = kIntroSprite0_X[k];
  intro_x_hi[k] = kIntroSprite0_X[k] >> 8;
  intro_y_lo[k] = kIntroSprite0_Y[k];
  intro_y_hi[k] = kIntroSprite0_Y[k] >> 8;
  intro_x_vel[k] = kIntroSprite0_Xvel[k];
  intro_y_vel[k] = kIntroSprite0_Yvel[k];
  intro_sprite_isinited[k]++;
}

void Intro_SpriteType_B_0(int k) {  // 8cc5b1
  static const uint8 kIntroSprite0_XLimit[3] = { 75, 95, 117 };
  static const uint8 kIntroSprite0_YLimit[3] = { 88, 48, 88 };

  AnimateSceneSprite_DrawTriangle(k);
  AnimateSceneSprite_MoveTriangle(k);
  if (intro_step_index != 5) {
    if (!(intro_frame_ctr & 31)) {
      intro_x_vel[k] += kIntroSprite0_Xvel[k];
      intro_y_vel[k] += kIntroSprite0_Yvel[k];
    }
    if (intro_x_lo[k] == kIntroSprite0_XLimit[k])
      intro_x_vel[k] = 0;
    if (intro_y_lo[k] == kIntroSprite0_YLimit[k])
      intro_y_vel[k] = 0;
  } else {
    intro_x_vel[k] = 0;
    intro_y_vel[k] = 0;
  }
}

void AnimateSceneSprite_DrawTriangle(int k) {  // 8cc70f
  static const IntroSpriteEnt kIntroSprite0_Left_Ents[16] = {
    { 0,  0, 0x80, 0x1b, 2},
    {16,  0, 0x82, 0x1b, 2},
    {32,  0, 0x84, 0x1b, 2},
    {48,  0, 0x86, 0x1b, 2},
    { 0, 16, 0xa0, 0x1b, 2},
    {16, 16, 0xa2, 0x1b, 2},
    {32, 16, 0xa4, 0x1b, 2},
    {48, 16, 0xa6, 0x1b, 2},
    { 0, 32, 0x88, 0x1b, 2},
    {16, 32, 0x8a, 0x1b, 2},
    {32, 32, 0x8c, 0x1b, 2},
    {48, 32, 0x8e, 0x1b, 2},
    { 0, 48, 0xa8, 0x1b, 2},
    {16, 48, 0xaa, 0x1b, 2},
    {32, 48, 0xac, 0x1b, 2},
    {48, 48, 0xae, 0x1b, 2},
  };
  static const IntroSpriteEnt kIntroSprite0_Right_Ents[16] = {
    {48,  0, 0x80, 0x5b, 2},
    {32,  0, 0x82, 0x5b, 2},
    {16,  0, 0x84, 0x5b, 2},
    { 0,  0, 0x86, 0x5b, 2},
    {48, 16, 0xa0, 0x5b, 2},
    {32, 16, 0xa2, 0x5b, 2},
    {16, 16, 0xa4, 0x5b, 2},
    { 0, 16, 0xa6, 0x5b, 2},
    {48, 32, 0x88, 0x5b, 2},
    {32, 32, 0x8a, 0x5b, 2},
    {16, 32, 0x8c, 0x5b, 2},
    { 0, 32, 0x8e, 0x5b, 2},
    {48, 48, 0xa8, 0x5b, 2},
    {32, 48, 0xaa, 0x5b, 2},
    {16, 48, 0xac, 0x5b, 2},
    { 0, 48, 0xae, 0x5b, 2},
  };
  AnimateSceneSprite_AddObjectsToOamBuffer(k, k == 2 ? kIntroSprite0_Right_Ents : kIntroSprite0_Left_Ents, 16);
}

void Intro_CopySpriteType4ToOam(int k) {  // 8cc82f
  static const IntroSpriteEnt kIntroTriforceOam_Left[16] = {
    { 0,  0, 0x80, 0x2b, 2},
    {16,  0, 0x82, 0x2b, 2},
    {32,  0, 0x84, 0x2b, 2},
    {48,  0, 0x86, 0x2b, 2},
    { 0, 16, 0xa0, 0x2b, 2},
    {16, 16, 0xa2, 0x2b, 2},
    {32, 16, 0xa4, 0x2b, 2},
    {48, 16, 0xa6, 0x2b, 2},
    { 0, 32, 0x88, 0x2b, 2},
    {16, 32, 0x8a, 0x2b, 2},
    {32, 32, 0x8c, 0x2b, 2},
    {48, 32, 0x8e, 0x2b, 2},
    { 0, 48, 0xa8, 0x2b, 2},
    {16, 48, 0xaa, 0x2b, 2},
    {32, 48, 0xac, 0x2b, 2},
    {48, 48, 0xae, 0x2b, 2},
  };
  static const IntroSpriteEnt kIntroTriforceOam_Right[16] = {
    {48,  0, 0x80, 0x6b, 2},
    {32,  0, 0x82, 0x6b, 2},
    {16,  0, 0x84, 0x6b, 2},
    { 0,  0, 0x86, 0x6b, 2},
    {48, 16, 0xa0, 0x6b, 2},
    {32, 16, 0xa2, 0x6b, 2},
    {16, 16, 0xa4, 0x6b, 2},
    { 0, 16, 0xa6, 0x6b, 2},
    {48, 32, 0x88, 0x6b, 2},
    {32, 32, 0x8a, 0x6b, 2},
    {16, 32, 0x8c, 0x6b, 2},
    { 0, 32, 0x8e, 0x6b, 2},
    {48, 48, 0xa8, 0x6b, 2},
    {32, 48, 0xaa, 0x6b, 2},
    {16, 48, 0xac, 0x6b, 2},
    { 0, 48, 0xae, 0x6b, 2},
  };
  AnimateSceneSprite_AddObjectsToOamBuffer(k, k == 2 ? kIntroTriforceOam_Right : kIntroTriforceOam_Left, 16);
}

void EXIT_0CCA90(int k) {  // 8cc84f
  // empty
}

void InitializeSceneSprite_Copyright(int k) {  // 8cc850
  intro_x_lo[k] = 76;
  intro_x_hi[k] = 0;
  intro_y_lo[k] = 184;
  intro_y_hi[k] = 0;
  intro_sprite_isinited[k]++;
}

void AnimateSceneSprite_Copyright(int k) {  // 8cc864
  static const IntroSpriteEnt kIntroSprite2_Ents[13] = {
    { 0, 0, 0x40, 0x0a, 0},
    { 8, 0, 0x41, 0x0a, 0},
    {16, 0, 0x42, 0x0a, 0},
    {24, 0, 0x68, 0x0a, 0},
    {32, 0, 0x41, 0x0a, 0},
    {40, 0, 0x42, 0x0a, 0},
    {48, 0, 0x43, 0x0a, 0},
    {56, 0, 0x44, 0x0a, 0},
    {64, 0, 0x50, 0x0a, 0},
    {72, 0, 0x51, 0x0a, 0},
    {80, 0, 0x52, 0x0a, 0},
    {88, 0, 0x53, 0x0a, 0},
    {96, 0, 0x54, 0x0a, 0},
  };
  AnimateSceneSprite_AddObjectsToOamBuffer(k, kIntroSprite2_Ents, 13);
}

void InitializeSceneSprite_Sparkle(int k) {  // 8cc8e2
  int j = intro_frame_ctr >> 5 & 3;
  intro_x_lo[k] = kIntroSprite3_X[j];
  intro_x_hi[k] = 0;
  intro_y_lo[k] = kIntroSprite3_Y[j];
  intro_y_hi[k] = 0;
  intro_sprite_isinited[k]++;
}

void AnimateSceneSprite_Sparkle(int k) {  // 8cc90d
  static const IntroSpriteEnt kIntroSprite3_Ents[4] = {
    { 0,  0, 0x80, 0x34, 0},
    { 0,  0, 0xb7, 0x34, 0},
    {-4, -3, 0x64, 0x38, 2},
    {-4, -3, 0x62, 0x34, 2},
  };
  if (intro_sprite_state[k] < 4)
    AnimateSceneSprite_AddObjectsToOamBuffer(k, kIntroSprite3_Ents + intro_sprite_state[k], 1);

  intro_sprite_state[k] = kIntroSprite3_State[intro_frame_ctr >> 2 & 7];
  int j = intro_frame_ctr >> 5 & 3;
  intro_x_lo[k] = kIntroSprite3_X[j];
  intro_y_lo[k] = kIntroSprite3_Y[j];
}

void AnimateSceneSprite_AddObjectsToOamBuffer(int k, const IntroSpriteEnt *src, int num) {  // 8cc972
  uint16 x = intro_x_hi[k] << 8 | intro_x_lo[k];
  uint16 y = intro_y_hi[k] << 8 | intro_y_lo[k];
  OamEnt *oam = (OamEnt *)&g_ram[intro_sprite_alloc];
  intro_sprite_alloc += num * 4;
  do {
    uint16 xcur = x + src->x;
    uint16 ycur = y + src->y;
    oam->x = xcur;
    oam->y = ClampYForOam(ycur);
    oam->charnum = src->charnum;
    oam->flags = src->flags;
    bytewise_extended_oam[oam - oam_buf] = src->ext | (xcur >> 8 & 1);
  } while (oam++, src++, --num);
}

void AnimateSceneSprite_MoveTriangle(int k) {  // 8cc9f1
  if (intro_x_vel[k] != 0) {
    uint32 t = intro_x_subpixel[k] + (intro_x_lo[k] << 8) + (intro_x_hi[k] << 16) + ((int8)intro_x_vel[k] << 4);
    intro_x_subpixel[k] = t, intro_x_lo[k] = t >> 8, intro_x_hi[k] = t >> 16;
  }
  if (intro_y_vel[k] != 0) {
    uint32 t = intro_y_subpixel[k] + (intro_y_lo[k] << 8) + (intro_y_hi[k] << 16) + ((int8)intro_y_vel[k] << 4);
    intro_y_subpixel[k] = t, intro_y_lo[k] = t >> 8, intro_y_hi[k] = t >> 16;
  }
}

void TriforceRoom_PrepGFXSlotForPoly() {  // 8cca54
  misc_sprites_graphics_index = 8;
  LoadCommonSprites_2();
  Intro_InitGfx_Helper();
  intro_sprite_isinited[0] = 1;
  intro_sprite_isinited[1] = 1;
  intro_sprite_isinited[2] = 1;
  intro_sprite_subtype[0] = 4;
  intro_sprite_subtype[1] = 5;
  intro_sprite_subtype[2] = 6;
  INIDISP_copy = 15;
  submodule_index++;
}

void Credits_InitializePolyhedral() {  // 8cca81
  misc_sprites_graphics_index = 8;
  LoadCommonSprites_2();
  Intro_InitGfx_Helper();
  poly_config1 = 0;
  intro_sprite_isinited[0] = 1;
  intro_sprite_isinited[1] = 1;
  intro_sprite_isinited[2] = 1;
  intro_sprite_subtype[0] = 7;
  intro_sprite_subtype[1] = 7;
  intro_sprite_subtype[2] = 7;
  INIDISP_copy = 15;
  submodule_index++;
}

void AdvancePolyhedral() {  // 8ccab1
  TriforceRoom_HandlePoly();
  Scene_AnimateEverySprite();
}

void TriforceRoom_HandlePoly() {  // 8ccabc
  is_nmi_thread_active = 1;
  intro_want_double_ret = 1;
  if (intro_did_run_step)
    return;
  switch (intro_step_index) {
  case 0:
    poly_config1 -= 2;
    if (poly_config1 < 2) {
      poly_config1 = 0;
      intro_step_index++;
      subsubmodule_index++;
    }
    // fall through
  case 1:
    if (subsubmodule_index >= 10) {
      intro_step_index++;
      intro_y_vel[1] = 5;
    }
    poly_b += 2, poly_a += 1;
    break;
  case 2:
    triforce_ctr = 0x1c0;
    if (poly_config1 < 128) {
      poly_config1 += 1;
    } else {
      if ((poly_b - 10 & 0x7f) >= 92 &&
          (uint8)(poly_a - 11) >= 220) {
        poly_a = 0;
        poly_b = 0;
        subsubmodule_index++;
        intro_step_index++;
        sound_effect_1 = 44;
        main_palette_buffer[0xd7] = 0x7fff;
        flag_update_cgram_in_nmi++;
        intro_step_timer = 6;
        break;
      }
    }
    poly_b += 5, poly_a += 3;
    break;
  case 3:
    if (!--intro_step_timer) {
      main_palette_buffer[0xd7] = kPolyhedralPalette[7];
      flag_update_cgram_in_nmi++;
      intro_step_index++;
    }
    break;
  case 4:
    break;
  }
  intro_did_run_step = 1;
  intro_want_double_ret = 0;
  intro_frame_ctr++;
}

void Credits_AnimateTheTriangles() {  // 8ccba2
  intro_frame_ctr++;
  is_nmi_thread_active = 1;
  if (!intro_did_run_step) {
    poly_b += 3;
    poly_a += 1;
    intro_did_run_step = 1;
  }
  Scene_AnimateEverySprite();
}

void InitializeSceneSprite_TriforceRoomTriangle(int k) {  // 8ccbe8
  static const int16 kIntroTriforce_X[3] = { 0x4e, 0x5f, 0x72 };
  static const int16 kIntroTriforce_Y[3] = { 0x9c, 0x9c, 0x9c };
  static const int8 kIntroTriforce_Xvel[3] = { -2, 0, 2 };
  static const int8 kIntroTriforce_Yvel[3] = { 4, -4, 4 };

  intro_x_lo[k] = kIntroTriforce_X[k];
  intro_x_hi[k] = 0;
  intro_y_lo[k] = kIntroTriforce_Y[k];
  intro_y_hi[k] = 0;
  intro_x_vel[k] = kIntroTriforce_Xvel[k];
  intro_y_vel[k] = kIntroTriforce_Yvel[k];
  intro_sprite_isinited[k]++;
}

void Intro_SpriteType_B_456(int k) {  // 8ccc13
  static const int8 kTriforce_Xacc[3] = { -1, 0, 1 };
  static const int8 kTriforce_Yacc[3] = { -1, -1, -1 };
  static const uint8 kTriforce_Yfinal2[3] = { 0x72, 0x66, 0x72 };

  Intro_CopySpriteType4ToOam(k);
  if (intro_want_double_ret)
    return;
  AnimateSceneSprite_MoveTriangle(k);
  switch (intro_step_index) {
  case 0:
    if (!(intro_frame_ctr & 7))
      intro_x_vel[k] += kTriforce_Xacc[k];
    if (!(intro_frame_ctr & 3))
      intro_y_vel[k] += kTriforce_Yacc[k];
    break;
  case 1:
    intro_x_vel[k] = 0;
    intro_y_vel[k] = 0;
    break;
  case 2:
    if (!(intro_frame_ctr & 3))
      AnimateTriforceRoomTriangle_HandleContracting(k);
    if (kTriforce_Xfinal[k] == intro_x_lo[k])
      intro_x_vel[k] = 0;
    if (kTriforce_Yfinal[k] == intro_y_lo[k])
      intro_y_vel[k] = 0;
    break;
  case 3:
  case 4:
    if (triforce_ctr == 0) {
      intro_y_lo[k] = kTriforce_Yfinal2[k];
    } else {
      triforce_ctr -= 1;
    }
    break;
  }
}

void AnimateTriforceRoomTriangle_HandleContracting(int k) {  // 8cccb0
  uint8 new_vel = intro_x_vel[k] + (intro_x_lo[k] <= kTriforce_Xfinal[k] ? 1 : -1);
  intro_x_vel[k] = (new_vel == 0x11) ? 0x10 : (new_vel == 0xef) ? 0xf0 : new_vel;
  new_vel = intro_y_vel[k] + (intro_y_lo[k] <= kTriforce_Yfinal[k] ? 1 : -1);
  intro_y_vel[k] = (new_vel == 0x11) ? 0x10 : (new_vel == 0xef) ? 0xf0 : new_vel;
}

void InitializeSceneSprite_CreditsTriangle(int k) {  // 8ccd19
  static const uint8 kIntroSprite7_X[3] = { 0x29, 0x5f, 0x97 };
  static const uint8 kIntroSprite7_Y[3] = { 0x70, 0x20, 0x70 };
  intro_x_lo[k] = kIntroSprite7_X[k];
  intro_x_hi[k] = 0;
  intro_y_lo[k] = kIntroSprite7_Y[k];
  intro_y_hi[k] = 0;
  intro_sprite_isinited[k]++;
}

void AnimateSceneSprite_CreditsTriangle(int k) {  // 8ccd3e
  static const int8 kIntroSprite7_XAcc[3] = { -1, 0, 1 };
  static const int8 kIntroSprite7_YAcc[3] = { 1, -1, 1 };

  LoadTriforceSpritePalette();
  Intro_CopySpriteType4ToOam(k);
  AnimateSceneSprite_MoveTriangle(k);
  if (submodule_index != 36) {
    intro_sprite_state[k] = 0;
    return;
  }
  if (intro_sprite_state[k] != 80) {
    intro_sprite_state[k]++;
    intro_x_vel[k] += kIntroSprite7_XAcc[k];
    intro_y_vel[k] += kIntroSprite7_YAcc[k];
  }
}

void Intro_DisplayLogo() {  // 8ced82
  static const uint8 kIntroLogo_X[4] = { 0x60, 0x70, 0x80, 0x88 };
  static const uint8 kIntroLogo_Tile[4] = { 0x69, 0x6b, 0x6d, 0x6e };
  OamEnt *oam = oam_buf;
  for (int i = 0; i < 4; i++) {
    oam[i].x = kIntroLogo_X[i];
    oam[i].y = 0x68;
    oam[i].charnum = kIntroLogo_Tile[i];
    oam[i].flags = 0x32;
    bytewise_extended_oam[i] = 2;
  }
}

void Intro_SetupSwordAndIntroFlash() {  // 8cfe45
  intro_sword_19 = 7;
  intro_sword_20 = 0;
  intro_sword_21 = 0;
  intro_sword_ypos = -130;

  Intro_PeriodicSwordAndIntroFlash();
}

void Intro_PeriodicSwordAndIntroFlash() {  // 8cfe56
  if (intro_sword_18)
    intro_sword_18--;
  SetBackdropcolorBlack();
  if (intro_times_pal_flash) {
    if ((intro_times_pal_flash & 3) != 0) {
      (&COLDATA_copy0)[intro_sword_24] |= 0x1f;
      intro_sword_24 = (intro_sword_24 == 2) ? 0 : intro_sword_24 + 1;
    }
    intro_times_pal_flash--;
  }
  OamEnt *oam = oam_buf + 0x52;
  for (int j = 9; j >= 0; j--) {
    static const uint8 kIntroSword_Char[10] = { 0, 2, 0x20, 0x22, 4, 6, 8, 0xa, 0xc, 0xe };
    static const uint8 kIntroSword_X[10] = { 0x40, 0x40, 0x30, 0x50, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40 };
    static const uint16 kIntroSword_Y[10] = { 0x10, 0x20, 0x28, 0x28, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80 };
    bytewise_extended_oam[0x52 + j] = 2;
    oam[j].charnum = kIntroSword_Char[j];
    oam[j].flags = 0x21;
    oam[j].x = kIntroSword_X[j];
    uint16 y = intro_sword_ypos + kIntroSword_Y[j];
    oam[j].y = ((y & 0xff00) ? 0xf8 : y) - 8;
  }

  if (intro_sword_ypos != 30) {
    if (intro_sword_ypos == 0xffbe) {
      sound_effect_1 = 1;
    } else if (intro_sword_ypos == 14) {
      WORD(intro_sword_24) = 0;
      intro_times_pal_flash = 0x20;
      sound_effect_1 = 0x2c;
    }
    intro_sword_ypos += 16;
  }

  switch (intro_sword_20 >> 1) {
  case 0:
    if (!intro_times_pal_flash && intro_sword_ypos == 30)
      intro_sword_20 += 2;
    break;
  case 1: {
    static const uint8 kSwordSparkle_Tab[8] = { 4, 4, 6, 6, 6, 4, 4 };

    if (!intro_sword_18) {
      intro_sword_19 -= 1;
      if (sign8(intro_sword_19)) {
        intro_sword_19 = 0;
        intro_sword_18 = 2;
        intro_sword_20 += 2;
        return;
      }
      intro_sword_18 = kSwordSparkle_Tab[intro_sword_19];
    }
    static const uint8 kSwordSparkle_Char[7] = { 0x28, 0x37, 0x27, 0x36, 0x27, 0x37, 0x28 };
    bytewise_extended_oam[0x50] = 0;
    oam_buf[0x50].x = 0x44;
    oam_buf[0x50].y = 0x43;
    oam_buf[0x50].flags = 0x25;
    oam_buf[0x50].charnum = kSwordSparkle_Char[intro_sword_19];
    break;
  }
  case 2: {
    static const uint8 kIntroSwordSparkle_Char[8] = { 0x26, 0x20, 0x24, 0x34, 0x25, 0x20, 0x35, 0x20 };
    int k = intro_sword_19;
    if (k >= 7)
      return;
    bytewise_extended_oam[0x50] = 0;
    bytewise_extended_oam[0x51] = 0;
    oam_buf[0x51].x = oam_buf[0x50].x = 0x42;

    uint8 y = (intro_sword_21 < 0x50 ? intro_sword_21 : 0x4f) + intro_sword_ypos + 0x31;
    oam_buf[0x50].y = y;
    oam_buf[0x51].y = y + 8;
    oam_buf[0x50].charnum = kIntroSwordSparkle_Char[k];
    oam_buf[0x51].charnum = kIntroSwordSparkle_Char[k + 1];
    oam_buf[0x51].flags = oam_buf[0x50].flags = 0x23;
    if (intro_sword_18 == 0) {
      intro_sword_21 += 4;
      if (intro_sword_21 == 0x4 || intro_sword_21 == 0x48 || intro_sword_21 == 0x4c || intro_sword_21 == 0x58)
        intro_sword_19 += 2;
    }
    break;
  }
  }
}

void Module1A_Credits() {  // 8e986e
  oam_region_base[0] = 0x30;
  oam_region_base[1] = 0x1d0;
  oam_region_base[2] = 0x0;

  kEndSequence_Funcs[submodule_index]();
}

void Credits_LoadNextScene_Overworld() {  // 8e9889
  kEndSequence0_Funcs[subsubmodule_index]();
  Credits_AddEndingSequenceText();
}

void Credits_LoadNextScene_Dungeon() {  // 8e9891
  Credits_LoadScene_Dungeon();
  Credits_AddEndingSequenceText();
}

void Credits_PrepAndLoadSprites() {  // 8e98b9
  for (int k = 15; k >= 0; k--) {
    SpritePrep_ResetProperties(k);
    sprite_state[k] = 0;
    sprite_flags5[k] = 0;
    sprite_defl_bits[k] = 0;
  }
  int k = submodule_index >> 1;
  switch (k) {
init_sprites_0:
  case 0: case 4: case 5: case 8: case 13: {
    int idx = kEndingSprites_Idx[k];
    int num = kEndingSprites_Idx[k + 1] - idx;
    const uint16 *px = kEndingSprites_X + idx;
    const uint16 *py = kEndingSprites_Y + idx;
    for (k = num - 1; k >= 0; k--) {
      sprcoll_x_size = sprcoll_y_size = 0xffff;
      uint16 x = (swap16(overworld_area_index << 1) & 0xf00) + px[k];
      uint16 y = (swap16(overworld_area_index >> 2) & 0xe00) + py[k];
      Sprite_SetX(k, x);
      Sprite_SetY(k, y);
    }
    break;
  }
init_sprites_1:
  case 1: {
    int idx = kEndingSprites_Idx[k];
    int num = kEndingSprites_Idx[k + 1] - idx;
    const uint16 *px = kEndingSprites_X + idx;
    const uint16 *py = kEndingSprites_Y + idx;
    byte_7E0FB1 = dungeon_room_index2 >> 3 & 254;
    byte_7E0FB0 = (dungeon_room_index2 & 15) << 1;
    for (k = num - 1; k >= 0; k--) {
      sprcoll_x_size = sprcoll_y_size = 0xffff;
      uint16 x = byte_7E0FB0 * 256 + px[k];
      uint16 y = byte_7E0FB1 * 256 + py[k];
      Sprite_SetX(k, x);
      Sprite_SetY(k, y);
    }
    break;
  }
  case 2:
    sprite_y_vel[6] = -16;
    goto init_sprites_0;
  case 3:
    sprite_A[5] = 22;
    sprite_y_vel[0] = -16;
    sprite_y_vel[1] = 16;
    sprite_head_dir[1] = 1;
    for (int j = 2; j >= 0; j--) {
      sprite_type[2 + j] = 0x57;
      sprite_oam_flags[2 + j] = 0x31;
    }
    goto init_sprites_0;
  case 6:
    sprite_delay_main[0] = 255;
    sprite_delay_main[1] = 255;
    sprite_delay_main[2] = 255;
    goto init_sprites_0;
  case 7:
    sprite_delay_main[1] = 255;
    goto init_sprites_0;
  case 9:
    for (int j = 4; j >= 0; j--) {
      sprite_delay_main[j] = j * 19;
      sprite_state[j] = 0;
    }
    sprite_type[5] = 0x2e;
    for (int j = 1; j >= 0; j--) {
      sprite_type[7 + j] = 0x9f;
      sprite_type[9 + j] = 0xa0;
      sprite_flags2[7 + j] = 1;
      sprite_flags2[9 + j] = 2;
      sprite_flags3[7 + j] = 0x10;
      sprite_flags3[9 + j] = 0x10;
    }
    goto init_sprites_0;
  case 10:
    sprite_delay_main[1] = 0x10;
    sprite_delay_main[2] = 0x20;
    sprite_oam_flags[3] = 8;
    sprite_oam_flags[4] = 8;
    goto init_sprites_1;
  case 11:
    sprite_oam_flags[4] = 0x79;
    sprite_oam_flags[5] = 0x39;
    sprite_D[1] = 1;
    sprite_A[1] = 4;
    goto init_sprites_1;
  case 12:
    for (int j = 1; j >= 0; j--) {
      sprite_oam_flags[j + 3] = 0x39;
      sprite_type[j + 3] = 0xb;
      sprite_flags3[j + 3] = 0x10;
      sprite_flags2[j + 3] = 1;
    }
    sprite_type[5] = 0x2a;
    sprite_type[6] = 0x79;
    sprite_ai_state[6] = 1;
    sprite_z[6] = 5;
    goto init_sprites_0;
  case 14:
    sprite_y_vel[5] = -16;
    sprite_y_vel[6] = 16;
    sprite_head_dir[6] = 1;
    sprite_A[0] = 8;
    for (int j = 3; j >= 0; j--)
      sprite_y_vel[1 + j] = 4;
    goto init_sprites_0;
  case 15:
    sprite_C[4] = 2;
    sprite_y_vel[5] = 8;
    sprite_delay_main[1] = 0x13;
    sprite_delay_main[4] = 0x40;
    goto init_sprites_0;
  }
}

void Credits_ScrollScene_Overworld() {  // 8e9958

  for (int k = 15; k >= 0; k--)
    if (sprite_delay_main[k])
      sprite_delay_main[k]--;

  int i = submodule_index >> 1;

  link_x_vel = link_y_vel = 0;
  if (R16 >= 0x40 && !(R16 & 1)) {
    if (BG2VOFS_copy2 != kEnding1_TargetScrollY[i])
      link_y_vel = kEnding1_Yvel[i];
    if (BG2HOFS_copy2 != kEnding1_TargetScrollX[i])
      link_x_vel = kEnding1_Xvel[i];
  }

  Credits_OperateScrollingAndTileMap();
  Credits_HandleSceneFade();
}

void Credits_ScrollScene_Dungeon() {  // 8e99c5
  for (int k = 15; k >= 0; k--)
    if (sprite_delay_main[k])
      sprite_delay_main[k]--;

  int i = submodule_index >> 1;
  if (R16 >= 0x40 && !(R16 & 1)) {
    if (BG2VOFS_copy2 != kEnding1_TargetScrollY[i])
      BG2VOFS_copy2 += kEnding1_Yvel[i];
    if (BG2HOFS_copy2 != kEnding1_TargetScrollX[i])
      BG2HOFS_copy2 += kEnding1_Xvel[i];
  }
  Credits_HandleSceneFade();
}

void Credits_HandleSceneFade() {  // 8e9a2a
  static const uint16 kEnding1_3_Tab0[16] = { 0x300, 0x280, 0x250, 0x2e0, 0x280, 0x250, 0x2c0, 0x2c0, 0x250, 0x250, 0x280, 0x250, 0x480, 0x400, 0x250, 0x500 };
  static const uint8 kEndSequence_Case0_Tab1[12] = { 0x1e, 0x20, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x16, 0x16, 0x16, 0x16 };
  static const uint8 kEndSequence_Case0_Tab0[12] = { 6, 3, 2, 2, 2, 2, 2, 2, 6, 6, 6, 6 };
  static const uint8 kEndSequence_Case0_OamFlags[12] = { 0x3b, 0x31, 0x3d, 0x3f, 0x39, 0x3b, 0x37, 0x3d, 0x39, 0x37, 0x37, 0x39 };
  int i = submodule_index >> 1, j, k;

  switch (i) {
  case 0:
    for (int k = 11; k != 7; k--) {
      sprite_oam_flags[k] = kEndSequence_Case0_OamFlags[k];
      Credits_SpriteDraw_Single(k, kEndSequence_Case0_Tab0[k], kEndSequence_Case0_Tab1[k]);
    }
    for (int k = 7; k != 1; k--) {
      sprite_oam_flags[k] = kEndSequence_Case0_OamFlags[k] | (frame_counter << 2 & 0x40);
      Credits_SpriteDraw_Single(k, kEndSequence_Case0_Tab0[k], kEndSequence_Case0_Tab1[k]);
    }
    for (int k = 1; k >= 0; k--) {
      sprite_oam_flags[k] = kEndSequence_Case0_OamFlags[k];
      Credits_SpriteDraw_Single(k, kEndSequence_Case0_Tab0[k], kEndSequence_Case0_Tab1[k]);
    }
    break;
  case 1:
    Credits_SpriteDraw_Single(0, 3, 12);
    Credits_SpriteDraw_DrawShadow(0);
    k = 1;
    sprite_type[k] = 0x73;
    sprite_oam_flags[k] = 0x27;
    sprite_E[k] = 2;
    Credits_SpriteDraw_PreexistingSpriteDraw(k, 16);
    break;
  case 2: {
    static const uint8 kEnding_Case2_Tab0[2] = { 0x20, 0x40 };
    static const int8 kEnding_Case2_Tab1[2] = { 16, -16 };
    static const int8 kEnding_Case2_Tab2[5] = { 0x28, 0x2a, 0x2c, 0x2e, 0x2c };
    static const int8 kEnding_Case2_Tab3[5] = { 3, 3, 3, 3, 3 };
    static const uint8 kEnding_Case2_Delay[2] = { 0x30, 0x10 };

    BYTE(flag_travel_bird) = kEnding_Case2_Tab0[frame_counter >> 2 & 1];
    k = 6;
    j = sprite_x_vel[k] >> 7 & 1;
    sprite_oam_flags[k] = (sprite_x_vel[k] + kEnding_Case2_Tab1[j]) >> 1 & 0x40 | 0x32;
    Credits_SpriteDraw_Single(k, 2, 0x24);
    Credits_SpriteDraw_CirclingBirds(k);
    k -= 1;
    sprite_oam_flags[k] = 0x31;
    if (!sprite_delay_main[k]) {
      j = sprite_A[k];
      sprite_A[k] ^= 1;
      sprite_delay_main[k] = kEnding_Case2_Delay[j];
      sprite_graphics[k] = sprite_graphics[k] + 1 & 3;
    }
    Credits_SpriteDraw_Single(k, 2, 0x26);
    k -= 1;
    do {
      if (!(frame_counter & 15))
        sprite_graphics[k] ^= 1;
      sprite_oam_flags[k] = 0x31;
      Credits_SpriteDraw_Single(k, kEnding_Case2_Tab3[k], kEnding_Case2_Tab2[k]);
      EndSequence_DrawShadow2(k);
    } while (--k >= 0);
    break;
  }
  case 3: {
    static const uint8 kEnding_Case3_Gfx[4] = { 1, 2, 3, 2 };
    for (k = 0; k < 5; k++) {
      if (k < 2) {
        sprite_type[k] = 1;
        sprite_oam_flags[k] = 0xb;
        Credits_SpriteDraw_SetShadowProp(k, 2);
        sprite_z[k] = 48;
        j = (frame_counter + (k ? 0x5f : 0x7d)) >> 2 & 3;
        sprite_graphics[k] = kEnding_Case3_Gfx[j];
        Credits_SpriteDraw_CirclingBirds(k);
        Credits_SpriteDraw_PreexistingSpriteDraw(k, 12);
      } else {
        Credits_SpriteDraw_PreexistingSpriteDraw(k, 16);
      }
    }
    Credits_SpriteDraw_Single(k, 2, 0x38);
    Ending_Func2(k, 0x30);
    k++;
    Credits_SpriteDraw_Single(k, 3, 0x3a);
    break;
  }
  case 4: {
    static const uint8 kEnding_Case4_Tab1[2] = { 0x30, 0x32 };
    static const uint8 kEnding_Case4_Tab0[2] = { 2, 2 };
    static const uint8 kEnding_Case4_Ctr[2] = { 0x20, 0 };
    static const int8 kEnding_Case4_XYvel[10] = { 0, -12, -16, -12, 0, 12, 16, 12, 0, -12 };
    static const uint8 kEnding_Case4_DelayVel[24] = {
      0x3b, 0x14, 0x1e, 0x1d, 0x2c, 0x2b, 0x42, 0x20, 0x27, 0x28, 0x2e, 0x38, 0x3a, 0x4c, 0x32, 0x44,
      0x2e, 0x2f, 0x1e, 0x28, 0x47, 0x35, 0x32, 0x30,
    };
    k = 2;
    sprite_oam_flags[k] = 0x35;
    Credits_SpriteDraw_Single(k, 1, 0x3c);
    k--;
    do {
      sprite_oam_flags[k] = (sprite_x_vel[k] - 1) >> 1 & 0x40 ^ 0x71;
      sprite_graphics[k] = frame_counter >> 3 & 1;
      if (R16 >= kEnding_Case4_Ctr[k] && !sprite_delay_main[k]) {
        uint8 a = kEnding_Case4_DelayVel[sprite_A[k]];
        sprite_delay_main[k] = a & 0xf8;
        sprite_y_vel[k] = kEnding_Case4_XYvel[(a & 7) + 2];
        sprite_x_vel[k] = kEnding_Case4_XYvel[a & 7];
        sprite_A[k]++;
      }
      Credits_SpriteDraw_Single(k, kEnding_Case4_Tab0[k], kEnding_Case4_Tab1[k]);
      EndSequence_DrawShadow2(k);
      Sprite_MoveXY(k);
    } while (--k >= 0);
    break;
  }
  case 5: {
    static const uint8 kEnding_Case5_Tab0[2] = { 0, 4 };
    static const uint16 kEnding_Case5_Tab1[2] = { 0xa, 0x224 };
    static const uint8 kEnding_Case5_Tab2[2] = { 10, 14 };
    if (R16 == 0x200)
      sound_effect_1 = 1;
    else if (R16 == 0x208)
      sound_effect_1 = 0x2c;
    if ((uint16)(R16 - 0x208) < 0x30)
      Credits_SpriteDraw_AddSparkle(2, 10, R16 - 0x208); // wtf x,y
    k = 3;
    if (R16 >= 0x200)
      sprite_graphics[k] = 1;
    sprite_oam_flags[k] = 0x31;
    Credits_SpriteDraw_Single(k, 4, 8);
    EndSequence_DrawShadow2(k);
    int j = sprite_graphics[k];
    sprite_graphics[--k] = j;
    link_dma_var3 = 0;
    link_dma_var4 = kEnding_Case5_Tab0[j];
    sprite_oam_flags[k] = 0x30;

    link_dma_graphics_index = kEnding_Case5_Tab1[j];
    Credits_SpriteDraw_Single(k, 5, kEnding_Case5_Tab2[j]);
    EndSequence_DrawShadow2(k);
    break;
  }
  case 6: {
    static const uint8 kEnding_Case6_SprType[3] = { 0x52, 0x55, 0x55 };
    static const uint8 kEnding_Case6_OamSize[3] = { 0x20, 8, 8 };
    static const uint8 kEnding_Case6_State[3] = { 3, 1, 1 };
    static const uint8 kEnding_Case6_Gfx[6] = { 0, 5, 5, 1, 6, 6 };

    int idx = kEndingSprites_Idx[i];
    int num = kEndingSprites_Idx[i + 1] - idx;

    for (int k = num - 1; k >= 0; k--) {
      cur_object_index = k;
      sprite_type[k] = kEnding_Case6_SprType[k];
      Oam_AllocateFromRegionA(kEnding_Case6_OamSize[k]);
      sprite_ai_state[k] = kEnding_Case6_State[k];
      j = (R16 >= 0x26f) ? k + 3 : k;
      if (R16 == 0x26f)
        sound_effect_2 = 0x21;
      sprite_graphics[k] = kEnding_Case6_Gfx[j];
      sprite_oam_flags[k] = 0x33;
      Sprite_Get16BitCoords(k);
      SpriteActive_Main(k);
    }
    break;
  }
  case 7:
    k = 1;
    Credits_SpriteDraw_SetShadowProp(k, 2);
    sprite_type[k] = 0xe9;
    Oam_AllocateFromRegionA(0xc);
    sprite_oam_flags[k] = 0x37;
    Sprite_Get16BitCoords(k);
    if (!(frame_counter & 15))
      sprite_graphics[k] ^= 1;
    SpriteActive_Main(k);
    if (R16 >= 0x180) {
      sprite_y_vel[k] = 4;
      if (sprite_y_lo[k] != 0x7c)
        Sprite_MoveXY(k);
    }
    k--;
    sprite_type[k] = 0x36;
    Oam_AllocateFromRegionA(0x18);
    sprite_oam_flags[k] = 0x39;
    Sprite_Get16BitCoords(k);
    if (!sprite_delay_main[k]) {
      static const int8 kEnding_Case7_Gfx[2] = { 1, -1 };
      sprite_delay_main[k] = 4;
      sprite_graphics[k] = sprite_graphics[k] + kEnding_Case7_Gfx[R16 >> 9 & 1] & 7;
    }
    SpriteActive_Main(k);
    break;
  case 8:
    k = 0;
    sprite_type[k] = 0x2c;
    Oam_AllocateFromRegionA(0x2c);
    sprite_oam_flags[k] = 0x3b;
    Sprite_Get16BitCoords(k);
    sprite_graphics[k] = R16 < 0x1c0 ? R16 >> 5 & 1 : 2;
    SpriteActive_Main(k);
    break;
  case 9:
    for (k = 0; k < 5; k++) {
      if (!sprite_delay_main[k]) {
        sprite_delay_main[k] = 96;
        sprite_state[k] = 96;
        sprite_x_vel[k] = 0;
        sprite_x_lo[k] = 238;
        sprite_x_hi[k] = 4;
        sprite_y_lo[k] = 24;
        sprite_y_hi[k] = 11;
      }
      if (sprite_state[k]) {
        sprite_y_vel[k] = -8;
        Sprite_MoveXY(k);
        if (!(frame_counter & 1))
          sprite_x_vel[k] += ((frame_counter >> 5) ^ k) & 1 ? -1 : 1;
        Credits_SpriteDraw_Single(k, 1, 0x10);
      }
    }
    for (;;) {
      if (!sprite_delay_main[k]) {
        static const uint8 kEnding_Case8_Delay1[4] = { 16, 14, 16, 18 };
        static const uint8 kEnding_Case8_Delay2[4] = { 20, 48, 20, 20 };
        sprite_delay_main[k] = (k == 5) ? kEnding_Case8_Delay1[sprite_A[k]] : kEnding_Case8_Delay2[sprite_A[k]];
        sprite_A[k] = sprite_A[k] + 1 & 3;
        sprite_graphics[k] ^= 1;
      }
      if (k == 5) {
        sprite_oam_flags[k] = 0x31;
        Credits_SpriteDraw_PreexistingSpriteDraw(k, 0x10);
        k++;
      } else {
        Credits_SpriteDraw_Single(k, 2, 0x12);
        k++;
        break;
      }
    }
    do {
      static const uint8 kEnding_Case8_D[4] = { 0, 1, 0, 1 };
      static const uint8 kEnding_Case8_OamFlags[4] = { 55, 55, 59, 61 };
      static const uint8 kEnding_Case8_Tab0[4] = { 8, 8, 12, 12 };
      sprite_oam_flags[k] = kEnding_Case8_OamFlags[k - 7];
      sprite_D[k] = kEnding_Case8_D[k - 7];
      Credits_SpriteDraw_ActivateAndRunSprite(k, kEnding_Case8_Tab0[k - 7]);
    } while (++k != 11);
    break;
  case 10: {
    static const uint8 kWishPond_X[8] = { 0, 4, 8, 12, 16, 20, 24, 0 };
    static const uint8 kWishPond_Y[8] = { 0, 8, 16, 24, 32, 40, 4, 36 };
    k = 5;
    Sprite_Get16BitCoords(k);
    if (!sprite_pause[k]) {
      uint8 xb = kWishPond_X[GetRandomNumber() & 7] + cur_sprite_x;
      uint8 yb = kWishPond_Y[GetRandomNumber() & 7] + cur_sprite_y;
      Credits_SpriteDraw_AddSparkle(3, xb, yb);
    }
    for (int k = 3; k < 5; k++) {
      if (sprite_delay_aux1[k])
        sprite_delay_aux1[k]--;
      sprite_type[k] = 0xe3;
      Credits_SpriteDraw_SetShadowProp(k, 1);
      Credits_SpriteDraw_ActivateAndRunSprite(k, 8);
    }
    sprite_type[k] = 0x72;
    sprite_oam_flags[k] = 0x3b;
    sprite_state[k] = 9;
    sprite_B[k] = 9;
    Credits_SpriteDraw_PreexistingSpriteDraw(k, 0x30);
    break;
  }
  case 11:
    if (R16 >= 0x170) {
      for (int k = 4; k != 6; k++) {
        Credits_SpriteDraw_Single(k, 1, 0x3e);
      }
      k = 0;
      sprite_oam_flags[k] = 0x39;
      if (R16 < 0x1c0) {
        sprite_graphics[k] = 2;
      } else if (sprite_delay_main[k] == 0) {
        sprite_delay_main[k] = 0x20;
        sprite_graphics[k] = (sprite_graphics[k] ^ 1) & 1;
      }
      Credits_SpriteDraw_Single(k, 4, 6);
    } else {
      static const uint8 kEnding_Case11_Gfx[16] = { 1, 1, 2, 2, 1, 1, 1, 1, 2, 2, 2, 2, 0, 0, 0, 0 };
      for (int k = 0; k < 2; k++) {
        sprite_type[k] = 0x1a;
        sprite_oam_flags[k] = 0x39;
        Credits_SpriteDraw_SetShadowProp(k, 2);
        uint8 bak0 = main_module_index;
        Credits_SpriteDraw_ActivateAndRunSprite(k, 0xc);
        main_module_index = bak0;
        if (sprite_B[k] == 15 && sprite_A[k] == 4)
          sprite_delay_main[k + 2] = 15;
        int j = sprite_delay_main[k + 2];
        if (j != 0) {
          sprite_oam_flags[k + 2] = 2;
          sprite_graphics[k + 2] = kEnding_Case11_Gfx[j];
          Credits_SpriteDraw_Single(k + 2, 2, 0x36);
        }
      }
    }
    break;
  case 12:
    k = 6;
    sprite_graphics[k] = frame_counter & 1;
    if (!sprite_graphics[k]) {
      sprite_x_vel[k] += sign8(sprite_x_lo[k] - 0x80) ? 1 : -1;
      sprite_y_vel[k] += sign8(sprite_y_lo[k] - 0xb0) ? 1 : -1;
      Sprite_MoveXY(k);
    }

    sprite_oam_flags[k] = sprite_x_vel[k] >> 1 & 0x40 ^ 0x7e;
    sprite_flags2[k] = 1;
    sprite_flags3[k] = 0x30;
    sprite_z[k] = 16;
    Credits_SpriteDraw_PreexistingSpriteDraw(k, 8);
    k--;
    sprite_oam_flags[k] = 0x37;
    Credits_SpriteDraw_SetShadowProp(k, 2);
    Credits_SpriteDraw_ActivateAndRunSprite(k, 12);
    k--;
    Credits_SpriteDraw_ActivateAndRunSprite(k, 8);
    k--;
    Credits_SpriteDraw_ActivateAndRunSprite(k, 8);
    k--;
    do {
      static const uint8 kEnding_Case12_Tab[3] = { 3, 3, 8 };
      static const uint8 kEnding_Case12_Z[15] = { 2, 4, 5, 6, 6, 7, 7, 7, 7, 6, 6, 5, 4, 2, 0 };

      Credits_SpriteDraw_Single(k, kEnding_Case12_Tab[k], k * 2);
      if (k == 0) {
        Ending_Func2(k, 0x30);
      } else if (k & ~1) {
        sprite_graphics[k] = frame_counter >> 3 & 1;
      } else {
        int j = frame_counter & 0x1f;
        if (j < 0xf) {
          sprite_z[k] = kEnding_Case12_Z[j];
        }
        sprite_graphics[k] = (j < 0xf) ? 1 : 0;
        Credits_SpriteDraw_DrawShadow(k);
      }
    } while (--k >= 0);
    break;
  case 13:
    k = 0;
    if (R16 == 0x200)
      sprite_x_vel[k] = -4;
    sprite_graphics[k] = frame_counter >> 4 & 1;
    if (sprite_x_lo[k] == 56) {
      sprite_x_vel[k] = 0;
      sprite_graphics[k] += 2;
    }
    Credits_SpriteDraw_Single(k, 3, 0x34);
    Sprite_MoveXY(k);
    break;
  case 14: {
    static const int8 kEnding_Case14_Tab1[4] = { 0, 1, 0, 2 };
    static const int8 kEnding_Case14_Tab0[5] = { 2, 8, 32, 32, 8 };
    for (k = 6; k; k--) {
      if (k >= 5) {
        sprite_type[k] = 0;
        Credits_SpriteDraw_SetShadowProp(k, 1);
        sprite_graphics[k] = (frame_counter + 0x4a & 8) >> 3;
        sprite_z[k] = 32;
        Credits_SpriteDraw_CirclingBirds(k);
        sprite_oam_flags[k] = (sprite_x_vel[k] >> 1 & 0x40) ^ 0xf;
        Credits_SpriteDraw_PreexistingSpriteDraw(k, 8);
      } else {
        sprite_type[k] = 0xd;
        if (k == 1)
          sprite_head_dir[k] = 0xd;
        Credits_SpriteDraw_SetShadowProp(k, 3);
        sprite_oam_flags[k] = 0x2b;
        uint8 a = sprite_delay_main[k];
        if (!a)
          sprite_delay_main[k] = a = 0xc0;
        a >>= 1;
        if (a == 0) {
          sprite_y_vel[k] = sprite_x_vel[k] = 0;
        } else {
          if (a < kEnding_Case14_Tab0[k] && !(frame_counter & 3) && (a = sprite_y_vel[k]) != 0) {
            sprite_y_vel[k] = --a;
            a -= 4;
            if (k < 3)
              a = -a;
            sprite_x_vel[k] = a;
          }
        }
        Sprite_MoveXY(k);
        sprite_graphics[k] = kEnding_Case14_Tab1[frame_counter >> 3 & 3];
        Credits_SpriteDraw_PreexistingSpriteDraw(k, 16);
      }
    }
    Credits_SpriteDraw_Single(k, 3, 0x18);
    Ending_Func2(k, 0x20);
    break;
  }
  case 15: {
    static const uint8 kEnding_Case15_X[4] = { 0x76, 0x73, 0x71, 0x78 };
    static const uint8 kEnding_Case15_Y[4] = { 0x8b, 0x83, 0x8d, 0x85 };
    static const uint8 kEnding_Case15_Delay[8] = { 6, 6, 6, 6, 6, 6, 10, 8 };
    static const uint8 kEnding_Case15_OamFlags[4] = { 0x61, 0x61, 0x3b, 0x39 };
    j = kGeneratedEndSequence15[frame_counter] & 3;
    Credits_SpriteDraw_AddSparkle(2, kEnding_Case15_X[j], kEnding_Case15_Y[j]);
    k = 2;
    sprite_type[k] = 0x62;
    sprite_oam_flags[k] = 0x39;
    Credits_SpriteDraw_PreexistingSpriteDraw(k, 0x18);
    for (j = 1; j >= 0; j--) {
      k++;
      if (sprite_delay_aux1[k])
        sprite_delay_aux1[k]--;
      sprite_oam_flags[k] = (sprite_x_vel[k] >> 1 & 0x40) ^ kEnding_Case15_OamFlags[j];
      if (!sprite_delay_main[k]) {
        sprite_delay_main[k] = 128;
        sprite_A[k] = 0;
      }
      if (!sprite_A[k]) {
        sprite_graphics[k] = (frame_counter >> 2 & 1) + 2;
        Credits_SpriteDraw_MoveSquirrel(k);
      } else if (!sprite_delay_aux1[k]) {
        if (sprite_B[k] == 8)
          sprite_B[k] = 0;
        sprite_delay_aux1[k] = kEnding_Case15_Delay[sprite_B[k] & 7];
        sprite_graphics[k] = sprite_graphics[k] & 1 ^ 1;
        sprite_B[k]++;
      }
      Credits_SpriteDraw_Single(k, 1, 20);
      EndSequence_DrawShadow2(k);
    }
    Credits_SpriteDraw_WalkLinkAwayFromPedestal(k + 1);
    break;
  }
  }

  k = submodule_index >> 1;
  if (R16 >= kEnding1_3_Tab0[k]) {
    if (!(R16 & 1) && !--INIDISP_copy)
      submodule_index++;
    else
      R16++;
  } else {
    if (!(R16 & 1) && INIDISP_copy != 15)
      INIDISP_copy++;
    R16++;
  }
  BG2HOFS_copy = BG2HOFS_copy2;
  BG2VOFS_copy = BG2VOFS_copy2;
  BG1HOFS_copy = BG1HOFS_copy2;
  BG1VOFS_copy = BG1VOFS_copy2;
}

void Credits_SpriteDraw_DrawShadow(int k) {  // 8ea5f8
  sprite_oam_flags[k] = 0x30;
  Credits_SpriteDraw_SetShadowProp(k, 0);
  Oam_AllocateFromRegionA(4);
  SpriteDraw_Shadow(k, &g_ending_coords);
}

void EndSequence_DrawShadow2(int k) {  // 8ea5fd
  Credits_SpriteDraw_SetShadowProp(k, 0);
  Oam_AllocateFromRegionA(4);
  SpriteDraw_Shadow(k, &g_ending_coords);
}

void Ending_Func2(int k, uint8 ain) {  // 8ea645
  static const uint8 kEnding_Func2_Delay[27] = {
  10, 10, 10, 10, 20, 8,   8,   0, 255, 12, 12, 12, 12, 12, 12, 30,
   8,  4,  4,  4,  0, 0, 255, 255, 144,  4, 0,
  };
  static const int8 kEnding_Func2_Tab0[28] = {
    0, 0, 1, 0, 1, 0, 2,  3,  0,  2, 0, 1, 0, 1, 0, 1,
    2, 3, 4, 5, 6, 3, 0, -1, -1, -1, 2, 3,
  };
  sprite_oam_flags[k] = ain;
  EndSequence_DrawShadow2(k);
  int j = sprite_A[k];
  if (!sprite_delay_main[k]) {
    j++;
    if (j == 8)
      j = 6;
    else if (j == 22)
      j = 21;
    else if (j == 28)
      j = 27;
    sprite_A[k] = j;
    sprite_delay_main[k] = kEnding_Func2_Delay[j - 1];
  }
  uint8 a = kEnding_Func2_Tab0[j];
  sprite_graphics[k] = (a == 255) ? frame_counter >> 3 & 1 : a;
  if ((j < 5 || j >= 10 && j < 15) && !(frame_counter & 1))
    sprite_y_lo[k]++;
}

void Credits_SpriteDraw_ActivateAndRunSprite(int k, uint8 a) {  // 8ea694
  cur_object_index = k;
  Oam_AllocateFromRegionA(a);
  Sprite_Get16BitCoords(k);
  uint8 bak0 = submodule_index;
  submodule_index = 0;
  sprite_state[k] = 9;
  SpriteActive_Main(k);
  submodule_index = bak0;
}

void Credits_SpriteDraw_PreexistingSpriteDraw(int k, uint8 a) {  // 8ea6b3
  Oam_AllocateFromRegionA(a);
  cur_object_index = k;
  Sprite_Get16BitCoords(k);
  SpriteActive_Main(k);
}

void Credits_SpriteDraw_Single(int k, uint8 a, uint8 j) {  // 8ea703
  static const DrawMultipleData kEndSequence_Dmd0[12] = {
    { 0, -8, 0x072a, 2},
    { 0, -8, 0x072a, 2},
    { 0,  0, 0x4fca, 2},
    { 0, -8, 0x072a, 2},
    { 0, -8, 0x072a, 2},
    { 0,  0, 0x0fca, 2},
    {-2,  0, 0x0f77, 0},
    { 0, -8, 0x072a, 2},
    { 0,  0, 0x4fca, 2},
    {-3,  0, 0x0f66, 0},
    { 0, -8, 0x072a, 2},
    { 0,  0, 0x4fca, 2},
  };
  static const DrawMultipleData kEndSequence_Dmd1[6] = {
    {14,  -7, 0x0d48, 2},
    { 0,  -6, 0x0944, 2},
    { 0,   0, 0x094e, 2},
    {13, -14, 0x0d48, 2},
    { 0,  -8, 0x0944, 2},
    { 0,   0, 0x0946, 2},
  };
  static const DrawMultipleData kEndSequence_Dmd2[16] = {
    {-2, -16, 0x3d78, 0},
    { 0, -24, 0x3d24, 2},
    { 0, -16, 0x3dc2, 2},
    {61, -16, 0x3777, 0},
    {64, -24, 0x37c4, 2},
    {64, -16, 0x77ca, 2},
    { 0,  -6, 0x326c, 2},
    {64,  -6, 0x326c, 2},
    {-2, -16, 0x3d68, 0},
    { 0, -24, 0x3d24, 2},
    { 0, -16, 0x3dc2, 2},
    {61, -16, 0x3766, 0},
    {64, -24, 0x37c4, 2},
    {64, -16, 0x77ca, 2},
    { 0,  -6, 0x326c, 2},
    {64,  -6, 0x326c, 2},
  };
  static const DrawMultipleData kEndSequence_Dmd3[12] = {
    { 0,  0, 0x0022, 2},
    {48,  0, 0x0064, 2},
    { 0, 10, 0x016c, 2},
    {48, 10, 0x016c, 2},
    { 0,  0, 0x0064, 2},
    {48,  0, 0x0022, 2},
    { 0, 10, 0x016c, 2},
    {48, 10, 0x016c, 2},
    { 0,  0, 0x0064, 2},
    {48,  0, 0x0064, 2},
    { 0, 10, 0x016c, 2},
    {48, 10, 0x016c, 2},
  };
  static const DrawMultipleData kEndSequence_Dmd4[8] = {
    {10,   8, 0x8a32, 0},
    {10,  16, 0x8a22, 0},
    { 0, -10, 0x0800, 2},
    { 0,   0, 0x082c, 2},
    {10, -14, 0x0a22, 0},
    {10,  -6, 0x0a32, 0},
    {0, -10, 0x082a, 2},
    {0,   0, 0x0828, 2},
  };
  static const DrawMultipleData kEndSequence_Dmd5[10] = {
    {10,  16, 0x8a05, 0},
    {10,   8, 0x8a15, 0},
    {-4,   2, 0x0a07, 2},
    { 0,  -7, 0x0e00, 2},
    { 0,   1, 0x0e02, 2},
    {10, -20, 0x0a05, 0},
    {10, -12, 0x0a15, 0},
    {-7,   1, 0x4a07, 2},
    { 0,  -7, 0x0e00, 2},
    { 0,   1, 0x0e02, 2},
  };
  static const DrawMultipleData kEndSequence_Dmd6[3] = {
    {-6, -2, 0x0706, 2},
    { 0, -9, 0x090e, 2},
    { 0, -1, 0x0908, 2},
  };
  static const DrawMultipleData kEndSequence_Dmd7[10] = {
    {0, -10, 0x082a, 2},
    {0,   0, 0x0828, 2},
    {10,  16, 0x8a05, 0},
    {10,   8, 0x8a15, 0},
    {-4,   2, 0x0a07, 2},
    { 0,  -7, 0x0e00, 2},
    { 0,   1, 0x0e02, 2},
    {10, -20, 0x0a05, 0},
    {10, -12, 0x0a15, 0},
    {-7,   1, 0x4a07, 2},
  };
  static const DrawMultipleData kEndSequence_Dmd8[1] = {
    {0, -19, 0x39af, 0},
  };
  static const DrawMultipleData kEndSequence_Dmd9[4] = {
    {-16, -24, 0x3704, 2},
    {-16, -16, 0x3764, 2},
    {-16, -24, 0x3762, 2},
    {-16, -16, 0x3764, 2},
  };
  static const DrawMultipleData kEndSequence_Dmd10[4] = {
    {0, 0, 0x0c0c, 2},
    {0, 0, 0x0c0a, 2},
    {0, 0, 0x0cc5, 2},
    {0, 0, 0x0ce1, 2},
  };
  static const DrawMultipleData kEndSequence_Dmd11[6] = {
    {1,  4, 0x002a, 0},
    {1, 12, 0x003a, 0},
    {4,  0, 0x0026, 2},
    {0,  9, 0x0024, 2},
    {8,  9, 0x4024, 2},
    {4, 20, 0x016c, 2},
  };
  static const DrawMultipleData kEndSequence_Dmd12[21] = {
    { 0, -7, 0x0d00, 2},
    { 0, -7, 0x0d00, 2},
    { 0,  0, 0x0d06, 2},
    { 0, -7, 0x0d00, 2},
    { 0, -7, 0x0d00, 2},
    { 0,  0, 0x4d06, 2},
    { 0, -8, 0x0d00, 2},
    { 0, -8, 0x0d00, 2},
    { 0,  0, 0x0d20, 2},
    { 0, -8, 0x0d02, 2},
    { 0, -8, 0x0d02, 2},
    { 0,  0, 0x0d2c, 2},
    {-3,  0, 0x0d2f, 0},
    { 0, -7, 0x0d02, 2},
    { 0,  0, 0x0d2c, 2},
    {-5,  2, 0x0d2f, 0},
    { 0, -8, 0x0d02, 2},
    { 0,  0, 0x0d2c, 2},
    {-5,  2, 0x0d3f, 0},
    { 0, -8, 0x0d02, 2},
    { 0,  0, 0x0d2c, 2},
  };
  static const DrawMultipleData kEndSequence_Dmd13[16] = {
    {0, -7, 0x0e00, 2},
    {0,  1, 0x4e02, 2},
    {0, -8, 0x0e00, 2},
    {0,  1, 0x0e02, 2},
    {0, -9, 0x0e00, 2},
    {0,  1, 0x0e02, 2},
    {0, -7, 0x0e00, 2},
    {0,  1, 0x0e02, 2},
    {0, -7, 0x0e00, 2},
    {0,  1, 0x4e02, 2},
    {0, -8, 0x0e00, 2},
    {0,  1, 0x4e02, 2},
    {0, -9, 0x0e00, 2},
    {0,  1, 0x4e02, 2},
    {0, -7, 0x0e00, 2},
    {0,  1, 0x4e02, 2},
  };
  static const DrawMultipleData kEndSequence_Dmd14[6] = {
    {0, 0, 0, 0},
    {0, 0, 0x34c7, 0},
    {0, 0, 0x3480, 0},
    {0, 0, 0x34b6, 0},
    {0, 0, 0x34b7, 0},
    {0, 0, 0x34a6, 0},
  };
  static const DrawMultipleData kEndSequence_Dmd15[6] = {
    {-3, 17, 0x002b, 0},
    {-3, 25, 0x003b, 0},
    { 0,  0, 0x000e, 2},
    {16,  0, 0x400e, 2},
    { 0, 16, 0x002e, 2},
    {16, 16, 0x402e, 2},
  };
  static const DrawMultipleData kEndSequence_Dmd16[3] = {
    { 8,  5, 0x0a04, 2},
    { 0, 16, 0x0806, 2},
    {16, 16, 0x4806, 2},
  };
  static const DrawMultipleData kEndSequence_Dmd17[2] = {
    {0,  0, 0x0000, 2},
    {0, 11, 0x0002, 2},
  };
  static const DrawMultipleData kEndSequence_Dmd18[2] = {
    {0,  0, 0x000e, 2},
    {0, 64, 0x006c, 2},
  };
  static const DrawMultipleData kEndSequence_Dmd19[8] = {
    {0, 0, 0x0882, 2},
    {0, 7, 0x0a4e, 2},
    {0, 0, 0x4880, 2},
    {0, 7, 0x0a4e, 2},
    {0, 0, 0x0882, 2},
    {0, 7, 0x0a4e, 2},
    {0, 0, 0x0880, 2},
    {0, 7, 0x0a4e, 2},
  };
  static const DrawMultipleData kEndSequence_Dmd20[6] = {
    {-4,  1, 0x0c68, 0},
    { 0, -8, 0x0c40, 2},
    { 0,  1, 0x0c42, 2},
    {-4,  1, 0x0c78, 0},
    { 0, -8, 0x0c40, 2},
    { 0,  1, 0x0c42, 2},
  };
  static const DrawMultipleData kEndSequence_Dmd21[6] = {
    {8,   5, 0x0679, 0},
    {0, -10, 0x088e, 2},
    {0,   0, 0x066e, 2},
    {0, -10, 0x088e, 2},
    {0, -10, 0x088e, 2},
    {0,   0, 0x066e, 2},
  };
  static const DrawMultipleData kEndSequence_Dmd22[6] = {
    {11,  -3, 0x0869, 0},
    { 0, -12, 0x0804, 2},
    { 0,   0, 0x0860, 2},
    {10,  -3, 0x0867, 0},
    { 0, -12, 0x0804, 2},
    { 0,   0, 0x0860, 2},
  };
  static const DrawMultipleData kEndSequence_Dmd23[6] = {
    {-2,  1, 0x0868, 0},
    { 0, -8, 0x08c0, 2},
    { 0,  0, 0x08c2, 2},
    {-3,  1, 0x0878, 0},
    { 0, -8, 0x08c0, 2},
    { 0,  0, 0x08c2, 2},
  };
  static const DrawMultipleData kEndSequence_Dmd24[4] = {
    {0, -10, 0x084c, 2},
    {0,   0, 0x0a6c, 2},
    {0,  -9, 0x084c, 2},
    {0,   0, 0x0aa8, 2},
  };
  static const DrawMultipleData kEndSequence_Dmd25[4] = {
    {0, -7, 0x084a, 2},
    {0,  0, 0x0c6a, 2},
    {0, -7, 0x084a, 2},
    {0,  0, 0x0ca6, 2},
  };
  static const DrawMultipleData kEndSequence_Dmd26[12] = {
    {-18, -24, 0x39a4, 2},
    {-16, -16, 0x39a8, 2},
    {-18, -24, 0x39a4, 2},
    {-18, -24, 0x39a4, 2},
    {-16, -16, 0x39a6, 2},
    {-18, -24, 0x39a4, 2},
    { -6, -17, 0x392d, 0},
    {-16, -24, 0x39a0, 2},
    {-16, -16, 0x39aa, 2},
    { -5, -17, 0x392c, 0},
    {-16, -24, 0x39a0, 2},
    {-16, -16, 0x39aa, 2},
  };
  static const DrawMultipleData kEndSequence_Dmd27[6] = {
    { 0,  -4, 0x30aa, 2},
    { 0,  -4, 0x30aa, 2},
    {-4,  -8, 0x3090, 0},
    {12,  -8, 0x7090, 0},
    {-6, -10, 0x3091, 0},
    {14, -10, 0x7091, 0},
  };
  static const DrawMultipleData kEndSequence_Dmd28[8] = {
    {0,  0, 0x0722, 2},
    {0, -8, 0x09c2, 2},
    {0,  0, 0x4722, 2},
    {0, -8, 0x09c2, 2},
    {0, -9, 0x09c4, 2},
    {0,  0, 0x0722, 2},
    {0, -9, 0x0924, 2},
    {0,  0, 0x0722, 2},
  };
  static const DrawMultipleData kEndSequence_Dmd29[3] = {
    {-16, -12, 0x3f08, 2},
    {  0, -12, 0x3f20, 2},
    { 16, -12, 0x3f20, 2},
  };
  static const DrawMultipleData kEndSequence_Dmd30[1] = {
    {0, 0, 0x0086, 2},
  };
  static const DrawMultipleData kEndSequence_Dmd31[1] = {
    {0, 0, 0x8060, 2},
  };
  static const DrawMultipleData *const kEndSequence_Dmds[] = {
    kEndSequence_Dmd0, kEndSequence_Dmd1, kEndSequence_Dmd2, kEndSequence_Dmd3,
    kEndSequence_Dmd4, kEndSequence_Dmd5, kEndSequence_Dmd6, kEndSequence_Dmd7,
    kEndSequence_Dmd8, kEndSequence_Dmd9, kEndSequence_Dmd10, kEndSequence_Dmd11,
    kEndSequence_Dmd12, kEndSequence_Dmd13, kEndSequence_Dmd14, kEndSequence_Dmd15,
    kEndSequence_Dmd16, kEndSequence_Dmd17, kEndSequence_Dmd18, kEndSequence_Dmd19,
    kEndSequence_Dmd20, kEndSequence_Dmd21, kEndSequence_Dmd22, kEndSequence_Dmd23,
    kEndSequence_Dmd24, kEndSequence_Dmd25, kEndSequence_Dmd26, kEndSequence_Dmd27,
    kEndSequence_Dmd28, kEndSequence_Dmd29, kEndSequence_Dmd30, kEndSequence_Dmd31
  };

  Oam_AllocateFromRegionA(a * 4);
  Sprite_Get16BitCoords(k);
  Sprite_DrawMultiple(k, kEndSequence_Dmds[j >> 1] + a * sprite_graphics[k], a, &g_ending_coords);
}

void Credits_SpriteDraw_SetShadowProp(int k, uint8 a) {  // 8eaca2
  sprite_flags2[k] = a;
  sprite_flags3[k] = 16;
}

void Credits_SpriteDraw_AddSparkle(int j_count, uint8 xb, uint8 yb) {  // 8eace5
  static const uint8 kEnding_Func3_Delay[6] = { 32, 4, 4, 4, 5, 6 };
  sprite_C[0] = j_count;
  for (int k = 0; k < j_count; k++) {
    int j = sprite_graphics[k];
    if (!sprite_delay_main[k]) {
      if (++j >= 6) {
        sprite_x_lo[k] = xb;
        sprite_y_lo[k] = yb;
        j = 0;
      }
      sprite_graphics[k] = j;
      sprite_delay_main[k] = kEnding_Func3_Delay[j];
    }
    if (j)
      Credits_SpriteDraw_Single(k, 1, 0x1c);
  }
}

void Credits_SpriteDraw_WalkLinkAwayFromPedestal(int k) {  // 8eadf7
  static const uint16 kEnding_Func6_Dma[8] = { 0x16c, 0x16e, 0x170, 0x172, 0x16c, 0x174, 0x176, 0x178 };
  if (!sprite_delay_main[k]) {
    sprite_graphics[k] = sprite_graphics[k] + 1 & 7;
    sprite_delay_main[k] = 4;
  }
  link_dma_graphics_index = kEnding_Func6_Dma[sprite_graphics[k]];
  sprite_oam_flags[k] = 32;
  Credits_SpriteDraw_Single(k, 2, 26);
  EndSequence_DrawShadow2(k);
  Sprite_MoveXY(k);
}

void Credits_SpriteDraw_MoveSquirrel(int k) {  // 8eae35
  static const int8 kEnding_Func5_Xvel[4] = { 32, 24, -32, -24 };
  static const int8 kEnding_Func5_Yvel[4] = { 8, -8, -8, 8 };
  if (sprite_delay_main[k] < 64) {
    sprite_C[k] = sprite_C[k] + 1 & 3;
    sprite_A[k]++;
  } else {
    int j = sprite_C[k];
    sprite_x_vel[k] = kEnding_Func5_Xvel[j];
    sprite_y_vel[k] = kEnding_Func5_Yvel[j];
    Sprite_MoveXY(k);
  }
}

void Credits_SpriteDraw_CirclingBirds(int k) {  // 8eae63
  static const int8 kEnding_MoveSprite_Func1_TargetX[2] = { 0x20, -0x20 };
  static const int8 kEnding_MoveSprite_Func1_TargetY[2] = { 0x10, -0x10 };

  int j = sprite_D[k] & 1;
  sprite_x_vel[k] += j ? -1 : 1;
  if (sprite_x_vel[k] == (uint8)kEnding_MoveSprite_Func1_TargetX[j])
    sprite_D[k]++;
  if (!(frame_counter & 1)) {
    j = sprite_head_dir[k] & 1;
    sprite_y_vel[k] += j ? -1 : 1;
    if (sprite_y_vel[k] == (uint8)kEnding_MoveSprite_Func1_TargetY[j])
      sprite_head_dir[k]++;
  }
  Sprite_MoveXY(k);
}

void Credits_HandleCameraScrollControl() {  // 8eaea6
  if (link_y_vel != 0) {
    uint8 yvel = link_y_vel;
    BG2VOFS_copy2 += (int8)yvel;
    uint16 *which = sign8(yvel) ? &overworld_unk1 : &overworld_unk1_neg;
    *which += abs8(yvel);
    if (!sign16(*which - 0x10)) {
      *which -= 0x10;
      overworld_screen_trans_dir_bits2 |= sign8(yvel) ? 8 : 4;
    }
    *(sign8(yvel) ? &overworld_unk1_neg : &overworld_unk1) = -*which;
    uint16 r4 = (int8)yvel, subp;
    WORD(byte_7E069E[0]) = r4;
    uint8 oi = BYTE(overlay_index);
    if (oi != 0x97 && oi != 0x9d) {
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
    }
  }

  if (link_x_vel != 0) {
    uint8 xvel = link_x_vel;
    BG2HOFS_copy2 += (int8)xvel;
    uint16 *which = sign8(xvel) ? &overworld_unk3 : &overworld_unk3_neg;
    *which += abs8(xvel);
    if (!sign16(*which - 0x10)) {
      *which -= 0x10;
      overworld_screen_trans_dir_bits2 |= sign8(xvel) ? 2 : 1;
    }
    *(sign8(xvel) ? &overworld_unk3_neg : &overworld_unk3) = -*which;

    uint16 r4 = (int8)xvel, subp;
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

  if (dungeon_room_index == 0x181) {
    BG1VOFS_copy2 = BG2VOFS_copy2 | 0x100;
    BG1HOFS_copy2 = BG2HOFS_copy2;
  }
}

void EndSequence_32() {  // 8ebc6d
  EnableForceBlank();
  EraseTileMaps_triforce();
  TransferFontToVRAM();
  Credits_LoadCoolBackground();
  Credits_InitializePolyhedral();
  INIDISP_copy = 128;
  overworld_palette_aux_or_main = 0x200;
  hud_palette = 1;
  Palette_Load_HUD();
  flag_update_cgram_in_nmi++;
  deaths_per_palace[4] = 0;
  deaths_per_palace[13] += death_save_counter;
  int sum = deaths_per_palace[13];
  for (int i = 12; i >= 0; i--)
    sum += deaths_per_palace[i];
  death_var2 = sum;
  death_save_counter = 0;
  link_health_current = kHealthAfterDeath[link_health_capacity >> 3];
  savegame_is_darkworld = 0x40;
  SaveGameFile();
  aux_palette_buffer[38] = 0;
  main_palette_buffer[38] = 0;
  aux_palette_buffer[0] = 0;
  main_palette_buffer[0] = 0;
  TM_copy = 0x16;
  TS_copy = 0;
  R16 = 0x6800;
  R18 = 0;
  ending_which_dung = 0;
  BG2VOFS_copy2 = -0x48;
  BG2HOFS_copy2 = 0x90;
  BG3VOFS_copy2 = 0;
  BG3HOFS_copy2 = 0;
  Credits_AddNextAttribution();
  music_control = 0x22;
  CGWSEL_copy = 0;
  CGADSUB_copy = 162;
  // real zelda does 0x12 here but this seems to work too
  zelda_ppu_write(BG2SC, 0x13);
  COLDATA_copy0 = 0x3f;
  COLDATA_copy1 = 0x5f;
  COLDATA_copy2 = 0x9f;
  subsubmodule_index = 64;
  INIDISP_copy = 0;

  HdmaSetup(0, 0xebd53, 0x42, 0, (uint8)BG2HOFS, 0);
  HDMAEN_copy = 0x80;

  BG2HOFS_copy = BG2HOFS_copy2;
  BG2VOFS_copy = BG2VOFS_copy2;
  BG1HOFS_copy = BG1HOFS_copy2;
  BG1VOFS_copy = BG1VOFS_copy2;
}

void Credits_FadeOutFixedCol() {  // 8ebd66
  if (--subsubmodule_index == 0) {
    subsubmodule_index = 16;
    if (COLDATA_copy0 != 32) {
      COLDATA_copy0--;
    } else if (COLDATA_copy1 != 64) {
      COLDATA_copy1--;
    } else if (COLDATA_copy2 != 128) {
      COLDATA_copy2--;
    }
  }
}

void Credits_FadeColorAndBeginAnimating() {  // 8ebd8b
  Credits_FadeOutFixedCol();
  nmi_disable_core_updates = 1;
  Credits_AnimateTheTriangles();
  if (!(frame_counter & 3)) {
    if (++BG2HOFS_copy2 == 0xc00) {
      // real zelda writes 0x00 to BG1SC here but that doesn't seem needed
      zelda_ppu_write(BG2SC, 0x13);
    }
    room_bounds_y.a1 = BG2HOFS_copy2 >> 1;
    room_bounds_y.a0 = room_bounds_y.a1 + BG2HOFS_copy2;
    room_bounds_y.b0 = room_bounds_y.a0 >> 1;
    room_bounds_y.b1 = room_bounds_y.a1 >> 1;
    if (BG3VOFS_copy2 == 3288) {
      R16 = 0x80;
      submodule_index++;
    } else {
      BG3VOFS_copy2++;
      if ((BG3VOFS_copy2 & 7) == 0) {
        R18 = BG3VOFS_copy2 >> 3;
        Credits_AddNextAttribution();
      }
    }
  }
  BG2HOFS_copy = BG2HOFS_copy2;
  BG2VOFS_copy = BG2VOFS_copy2;
  BG1HOFS_copy = BG1HOFS_copy2;
  BG1VOFS_copy = BG1VOFS_copy2;
}

void Credits_AddNextAttribution() {  // 8ebe24
  static const uint8 kEnding_Func9_Tab2[14] = { 1, 0, 2, 3, 10, 6, 5, 8, 11, 9, 7, 12, 13, 15 };
  static const uint16 kEnding_Digits_ScrollY[14] = { 0x290, 0x298, 0x2a0, 0x2a8, 0x2b0, 0x2ba, 0x2c2, 0x2ca, 0x2d2, 0x2da, 0x2e2, 0x2ea, 0x2f2, 0x310 };
  static const uint16 kEnding_Credits_DigitChar[2] = { 0x3ce6, 0x3cf6 };

  uint16 *dst = vram_upload_data + (vram_upload_offset >> 1);

  dst[0] = swap16(R16);
  dst[1] = 0x3e40;
  dst[2] = kEnding_MapData[159];
  dst += 3;

  if (R18 < 394) {
    const uint8 *src = &kEnding_Credits_Text[kEnding_Credits_Offs[R18]];
    if (*src != 0xff) {
      *dst++ = swap16(R16 + *src++);
      int n = *src++;
      *dst++ = swap16(n);
      n = (n + 1) >> 1;
      do {
        *dst++ = kEnding_MapData[*src++];
      } while (--n);
    }

    if ((ending_which_dung & 1) || R18 * 2 == kEnding_Digits_ScrollY[ending_which_dung >> 1]) {
      int t = kEnding_Credits_DigitChar[ending_which_dung & 1];
      WORD(g_ram[0xce]) = t;

      dst[0] = swap16(R16 + 0x19);
      dst[1] = 0x500;

      uint16 deaths = deaths_per_palace[kEnding_Func9_Tab2[ending_which_dung >> 1]];
      if (deaths >= 1000)
        deaths = 999;

      dst[4] = t + deaths % 10, deaths /= 10;
      dst[3] = t + deaths % 10, deaths /= 10;
      dst[2] = t + deaths;
      dst += 5;
      ending_which_dung++;
    }
  }

  R16 += 0x20;
  if (!(R16 & 0x3ff))
    R16 = (R16 & 0x6800) ^ 0x800;
  vram_upload_offset = (char *)dst - (char *)vram_upload_data;
  BYTE(*dst) = 0xff;
  nmi_load_bg_from_vram = 1;
}

void Credits_AddEndingSequenceText() {  // 8ec303

  uint16 *dst = vram_upload_data;
  dst[0] = 0x60;
  dst[1] = 0xfe47;
  dst[2] = kEnding_MapData[159];
  dst += 3;

  const uint8 *curo = &kEnding0_Data[kEnding0_Offs[submodule_index >> 1]];
  const uint8 *endo = &kEnding0_Data[kEnding0_Offs[(submodule_index >> 1) + 1]];
  do {
    dst[0] = WORD(curo[0]);
    dst[1] = WORD(curo[2]);
    int m = (dst[1] >> 9) & 0x7f;
    dst += 2, curo += 4;
    do {
      *dst++ = kEnding_MapData[*curo++];
    } while (--m >= 0);
  } while (curo != endo);

  vram_upload_offset = (char *)dst - (char *)vram_upload_data;
  BYTE(*dst) = 0xff;
  nmi_load_bg_from_vram = 1;
}

void Credits_BrightenTriangles() {  // 8ec37c
  if (!(frame_counter & 15) && ++INIDISP_copy == 15)
    submodule_index++;
  Credits_AnimateTheTriangles();
}

void Credits_StopCreditsScroll() {  // 8ec391
  if (!--BYTE(R16)) {
    darkening_or_lightening_screen = 0;
    palette_filter_countdown = 0;
    WORD(mosaic_target_level) = 0x1f;
    submodule_index++;
    R16 = 0xc0;
    R18 = 0;
  }
  Credits_AnimateTheTriangles();
}

void Credits_FadeAndDisperseTriangles() {  // 8ec3b8
  BYTE(R16)--;
  if (!BYTE(R18)) {
    ApplyPaletteFilter_bounce();
    if (BYTE(palette_filter_countdown)) {
      Credits_AnimateTheTriangles();
      return;
    }
    BYTE(R18)++;
  }
  if (BYTE(R16)) {
    Credits_AnimateTheTriangles();
    return;
  }
  submodule_index++;
  PaletteFilter_WishPonds_Inner();
}

void Credits_FadeInTheEnd() {  // 8ec3d5
  if (!(frame_counter & 7)) {
    PaletteFilter_SP5F();
    if (!BYTE(palette_filter_countdown))
      submodule_index++;;
  }
  Credits_HangForever();
}

void Credits_HangForever() {  // 8ec41a
  static const OamEntSigned kEndSequence37_Oams[4] = {
    {-96, -72, 0x00, 0x3b},
    {-80, -72, 0x02, 0x3b},
    {-64, -72, 0x04, 0x3b},
    {-48, -72, 0x06, 0x3b},
  };
  memcpy(oam_buf, kEndSequence37_Oams, 4 * 4);
  bytewise_extended_oam[0] = bytewise_extended_oam[1] = bytewise_extended_oam[2] = bytewise_extended_oam[3] = 2;
}

void CrystalCutscene_InitializePolyhedral() {  // 9ecdd9
  poly_config1 = 156;
  poly_config_color_mode = 1;
  is_nmi_thread_active = 1;
  intro_did_run_step = 1;
  poly_base_x = 32;
  poly_base_y = 32;
  BYTE(poly_var1) = 32;
  poly_which_model = 0;
  poly_a = 16;
  TS_copy = 0;
  TM_copy = 0x16;
}

