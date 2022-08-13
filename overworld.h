#pragma once
#include "zelda_rtl.h"
#include "variables.h"
#include "snes_regs.h"

void Module_Overworld();
void Overworld_DrawBadWeather();
void Overworld_DrawMap16(uint16 tile, uint16 value);
void Overworld_DrawPersistentMap16(uint16 tile, uint16 value);
void Overworld_CgramAuxToMain();
void Overworld_CheckSwitchArea();
void Overworld_ScrollMap();
void Overworld_LoadOverlays();
void Overworld_LoadExitData_alternate();
void Overworld_SimpleExit();
void Overworld_LoadMapData();
void Overworld_LoadExitData();
void MirrorWarp_Helper();
void MirrorWarp_LoadNext_7();
void MirrorWarp_LoadNext_8();
void Overworld_SetSongList();
void Module_PreOverworld();
void PreOverworld_LoadBunnyStuff();
void Player_RemoveBunny();
void Module_OpenSpotlight();
void Module_CloseSpotlight();
void Overworld_LoadOverlays2();
void BirdTravel_LoadTargetAreaData();
void BirdTravel_LoadTargetAreaPalettes();
void Overworld_SetFixedColorsAndScroll();
void Overworld_LoadAmbientOverlayAndMapData();
void Overworld_LoadBirdTravelPos(int k);
void Whirlpool_LookUpAndLoadTargetArea();
void Overworld_ResetMosaic();
void Overworld_ResetMosaic_alwaysIncrease();
void OpenSpotlight_Next();
void OpenSpotlight_Next2();
void CloseSpotlight_Init();
void Mirror_InitHdmaSettings();
void Overworld_LoadMapProperties();
void Overworld_MirrorWarp_State3();
void Overworld_MirrorWarp_State4();
void Mirror_Func30();
void Overworld_AlterGargoyleEntrance();
uint8 Overworld_ReadSomeTileAttr(uint16 x, uint16 y);
void Overworld_DwDeathMountainPaletteAnimation();
void Overworld_CreatePyramidHole();
void Overworld_EnterSpecialArea();
void PreOverworld_LoadLevelData();

void Overworld_ApplyBombToTiles(uint16 x, uint16 y);
void Overworld_AlterWeathervane();
void Overworld_PitDamage();

uint16 Overworld_RevealSecret(uint16 pos);
void Overworld_Memorize_Map16_Change(uint16 pos, uint16 value);
void Overworld_DrawMap16(uint16 tile, uint16 value);
void PreOverworld_LoadOverlays();
void DoorAnim_DoWork();
int Overworld_SmashRockPile(bool down_one_tile, Point16U *pt);
void DoorAnim_DoWork2();
void LoadEnemyDamageData();
uint8 Overworld_LiftableTiles(Point16U *pt_arg);
uint16 Overworld_ToolAndTileInteraction(uint16 x, uint16 y);
void Overworld_Hole();
void Overworld_LoadGfxProperties();
void Overworld_LoadGfxProperties_justLightWorld();
uint16 Overworld_GetSignText(int area);
const uint8 *GetOverworldSpritePtr(int area);

uint8 GetOverworldBgPalette(int idx);