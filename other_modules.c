#include "zelda_rtl.h"
#include "variables.h"
#include "dungeon.h"
#include "ancilla.h"
#include "load_gfx.h"
#include "hud.h"
#include "sprite.h"
#include "messaging.h"
#include "player_oam.h"
#include "sprite_main.h"

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

