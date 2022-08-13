#pragma once

enum {
  kDoorType_Regular = 0,
  kDoorType_Regular2 = 2,
  kDoorType_4 = 4,
  kDoorType_EntranceDoor = 6,
  kDoorType_WaterfallTunnel = 8,
  kDoorType_EntranceLarge = 10,
  kDoorType_EntranceLarge2 = 12,
  kDoorType_EntranceCave = 14,
  kDoorType_EntranceCave2 = 16,
  kDoorType_ExitToOw = 18,
  kDoorType_ThroneRoom = 20,
  kDoorType_PlayerBgChange = 22,
  kDoorType_ShuttersTwoWay = 24,
  kDoorType_InvisibleDoor = 26,
  kDoorType_SmallKeyDoor = 0x1c,
  kDoorType_1E = 0x1e,
  kDoorType_StairMaskLocked0 = 32,
  kDoorType_StairMaskLocked1 = 34,
  kDoorType_StairMaskLocked2 = 36,
  kDoorType_StairMaskLocked3 = 38,
  kDoorType_BreakableWall = 0x28,
  kDoorType_LgExplosion = 48,
  kDoorType_Slashable = 50,
  kDoorType_36 = 0x36,
  kDoorType_38 = 0x38,

  kDoorType_RegularDoor33 = 64,
  kDoorType_Shutter = 68,
  kDoorType_WarpRoomDoor = 70,
  kDoorType_ShutterTrapUR = 72,
  kDoorType_ShutterTrapDL = 74,
};

struct DungPalInfo {
  uint8 pal0;
  uint8 pal1;
  uint8 pal2;
  uint8 pal3;
};

const DungPalInfo *GetDungPalInfo(int idx);

void Dungeon_LoadRoom();
void Dungeon_LoadAndDrawRoom();
void WaterFlood_BuildOneQuadrantForVRAM();
void Dungeon_LoadAttributeTable();
void Dungeon_LoadAttribute_Selectable();
void Dungeon_ProcessTorchesAndDoors();
void Dungeon_InterRoomTrans_notDarkRoom();
void ApplyPaletteFilter();
void ResetTransitionPropsAndAdvanceSubmodule();
void SubtileTransitionCalculateLanding();
void ResetThenCacheRoomEntryProperties();

void Dungeon_ResetSprites();
void ResetStarTileGraphics();
void Dungeon_RestoreStarTileChr();
void Dungeon_ApproachFixedColor_variable(uint8 a);
void ApplyGrayscaleFixed_Incremental();

void LoadTransAuxGFX_sprite();
void Dungeon_AdjustForRoomLayout();
void DungeonTransition_RunFiltering();
void Dungeon_PlayMusicIfDefeated();
void OpenCrackedDoor();

void OrientLampLightCone();

void Dungeon_PushBlock_Handler();
void Dungeon_PrepareNextRoomQuadrantUpload();
void WaterFlood_BuildOneQuadrantForVRAM();

void ResetTransitionPropsAndAdvance_ResetInterface();
void Follower_Initialize();
void Dungeon_PlayBlipAndCacheQuadrantVisits();

void Module07_Dungeon();
void Module_PreDungeon();
void Module_PreDungeon_setAmbientSfx();

void Module05_LoadFile();

void Dungeon_LoadEntrance();
void Dungeon_LoadSongBankIfNeeded();
void DungeonTransition_LoadSpriteGFX();
void Dungeon_LoadCustomTileAttr();
void Dungeon_ResetTorchBackgroundAndPlayer();

void Dungeon_UpdateTileMapWithCommonTile(int x, int y, uint8 v);

void SavePalaceDeaths();

void Door_LoadBlastWallAttr(int k);
void Dungeon_Effect_Handler();

void Ganon_ExtinguishTorch_adjust_translucency();
void Ganon_ExtinguishTorch();

void Module11_DungeonFallingEntrance();
void Credits_LoadScene_Dungeon();
void Credits_LoadScene_Overworld_PrepGFX();

void AdjustQuadrantAndCamera_left();
void AdjustQuadrantAndCamera_right();
void AdjustQuadrantAndCamera_up();
void AdjustQuadrantAndCamera_down();
void Dungeon_FlagRoomData_Quadrants();
void Dungeon_ResetTorchBackgroundAndPlayerInner();
void Dungeon_AdjustAfterSpiralStairs();
void Dungeon_AdjustQuadrant();
void Dungeon_FlagRoomData_Quadrants();
void LoadOWMusicIfNeeded();
void RoomDraw_16x16Single(uint8 index);
void Dungeon_Store2x2(uint16 pos, uint16 t0, uint16 t1, uint16 t2, uint16 t3, uint8 attr);
uint16 RoomTag_BuildChestStripes(uint16 pos, uint16 y);
void Dungeon_DeleteRupeeTile(uint16 x, uint16 y);
void Dungeon_StartInterRoomTrans_Left();
void Dung_StartInterRoomTrans_Left_Plus();
void HandleEdgeTransitionMovementEast_RightBy8();
void HandleEdgeTransitionMovementSouth_DownBy16();
void Dungeon_StartInterRoomTrans_Right();
void Dungeon_StartInterRoomTrans_Up();
void Dungeon_StartInterRoomTrans_Down();

uint16 Dungeon_MapVramAddr(uint16 pos);
uint16 Dungeon_MapVramAddrNoSwap(uint16 pos);
void Bomb_CheckForDestructibles(uint16 x, uint16 y, uint8 r14);
uint8 ThievesAttic_DrawLightenedHole(uint16 pos6, uint16 a, Point16U *pt);
uint8 Dungeon_LiftAndReplaceLiftable(Point16U *pt);
uint8 OpenChestForItem(uint8 tile, int *chest_position);
uint16 Dungeon_CheckForAndIDLiftableTile();
void Mirror_SaveRoomData();
void SaveDungeonKeys();
void SaveQuadrantsToSram();
uint8 HandleItemTileAction_Dungeon(uint16 x, uint16 y);

uint16 Dungeon_GetTeleMsg(int room);
uint8 GetEntranceMusicTrack(int entrance);
bool Dungeon_IsPitThatHurtsPlayer();

extern const uint8 kDungAnimatedTiles[24];
