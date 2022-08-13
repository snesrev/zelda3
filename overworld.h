#pragma once
#include "zelda_rtl.h"
#include "variables.h"
#include "snes_regs.h"

void Module_Overworld();
void OverworldOverlay_HandleRain();
void Overworld_DrawMap16(uint16 tile, uint16 value);
void Overworld_DrawMap16_Persist(uint16 tile, uint16 value);
void Overworld_CopyPalettesToCache();
void OverworldHandleTransitions();
void OverworldHandleMapScroll();
void Overworld_LoadOverlays();
void Overworld_LoadNewScreenProperties();
void LoadCachedEntranceProperties();
void Overworld_DrawQuadrantsAndOverlays();
void LoadOverworldFromDungeon();
void MirrorWarp_Helper();
void Overworld_DrawScreenAtCurrentMirrorPosition();
void MirrorWarp_LoadSpritesAndColors();
void Overworld_SetSongList();
void Module_PreOverworld();
void AdjustLinkBunnyStatus();
void ForceNonbunnyStatus();
void Module10_SpotlightOpen();
void Module0F_SpotlightClose();
void Overworld_LoadOverlays2();
void FluteMenu_LoadTransport();
void FluteMenu_LoadSelectedScreenPalettes();
void Overworld_SetFixedColAndScroll();
void Overworld_LoadAmbientOverlayAndMapData();
void Overworld_LoadBirdTravelPos(int k);
void FindPartnerWhirlpoolExit();
void ConditionalMosaicControl();
void Overworld_ResetMosaic_alwaysIncrease();
void Spotlight_ConfigureTableAndControl();
void OpenSpotlight_Next2();
void Dungeon_PrepExitWithSpotlight();
void InitializeMirrorHDMA();
void Overworld_LoadGFXAndScreenSize();
void MirrorWarp_BuildWavingHDMATable();
void MirrorWarp_BuildDewavingHDMATable();
void ResetAncillaAndCutscene();
void OpenGargoylesDomain();
uint8 Overworld_ReadTileAttribute(uint16 x, uint16 y);
void Overworld_DwDeathMountainPaletteAnimation();
void CreatePyramidHole();
void Overworld_EnterSpecialArea();
void Module08_02_LoadAndAdvance();

void Overworld_BombTiles32x32(uint16 x, uint16 y);
void Overworld_AlterWeathervane();
void TakeDamageFromPit();

uint16 Overworld_RevealSecret(uint16 pos);
void Overworld_Memorize_Map16_Change(uint16 pos, uint16 value);
void Overworld_DrawMap16(uint16 tile, uint16 value);
void PreOverworld_LoadOverlays();
void Overworld_DoMapUpdate32x32();
int Overworld_SmashRockPile(bool down_one_tile, Point16U *pt);
void Overworld_DoMapUpdate32x32_B();
void LoadEnemyDamageData();
uint8 Overworld_LiftableTiles(Point16U *pt_arg);
uint16 Overworld_ToolAndTileInteraction(uint16 x, uint16 y);
void Overworld_GetPitDestination();
void Sprite_LoadGraphicsProperties();
void Sprite_LoadGraphicsProperties_light_world_only();
uint16 Overworld_GetSignText(int area);
const uint8 *GetOverworldSpritePtr(int area);

uint8 GetOverworldBgPalette(int idx);