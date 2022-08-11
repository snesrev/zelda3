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
void Dungeon_LoadAndUploadRoom();
void Dungeon_Upload_BG1_Outer();
void Dungeon_LoadAttrTable();
void Dungeon_LoadAttrIncremental();
void Dungeon_ProcessTorchAndDoorInteractives();
void Dungeon_InterRoomTrans_notDarkRoom();
void PaletteFilter_doFiltering();
void Dungeon_Staircase_Func1();
void Dungeon_Staircase_Func2();
void Dungeon_InitAndCacheVars();

void Dungeon_ResetSprites();
void Dungeon_InitStarTileChr();
void Dungeon_RestoreStarTileChr();
void Dungeon_ApproachFixedColor_variable(uint8 a);
void Dungeon_ApproachFixedColor();

void Dungeon_LoadSpriteSets();
void Dungeon_SpiralStaircase7_Inner3();
void Dung_UpdateLightsOutColor();
void Dungeon_PlayMusicIfDefeated();
void Dungeon_AnimateDestroyingWeakDoor();

void OrientLampBg();

void Dungeon_PushBlock_Handler();
void Dungeon_Upload_BG2();
void Dungeon_Upload_BG1_Outer();

void Dungeon_Teleport0();
void Tagalong_Init();
void Dungeon_SpiralStaircase7_Inner4();

void Module_Dungeon();
void Module_PreDungeon();
void Module_PreDungeon_setAmbientSfx();

void Module_LoadFile();

void Dungeon_LoadEntrance();
void Dungeon_LoadSongBankIfNeeded();
void Dungeon_Staircase6();
void Dungeon_LoadCustomTileAttr();
void Dungeon_ResetTorchBackgroundAndPlayer();

void Dungeon_SpriteInducedTilemapUpdate(int x, int y, uint8 v);

void SavePalaceDeaths();

void Door_LoadBlastWallAttr(int k);
void Dungeon_Effect_Handler();

void Dungeon_ExtinguishFirstTorch();
void Dungeon_ExtinguishSecondTorch();

void Module_HoleToDungeon();
void Ending_CinemaSequencesIndoorsInit();
void EndingSequenceCode_0();

void Player_CrossQuadrantBoundary_Left();
void Player_CrossQuadrantBoundary_Right();
void Player_CrossQuadrantBoundary_Up();
void Player_CrossQuadrantBoundary_Down();
void Dungeon_SaveRoomQuadrantData();
void Dungeon_ResetTorchBackgroundAndPlayerInner();
void Dung_AdjustCoordsForNewRoom();
void UpdateCompositeOfLayoutAndQuadrant();
void Dungeon_SaveRoomQuadrantData();
void Overworld_LoadMusicIfNeeded();
void Dungeon_EraseInteractive2x2(uint8 index);
void Dungeon_Store2x2(uint16 pos, uint16 t0, uint16 t1, uint16 t2, uint16 t3, uint8 attr);
uint16 Dungeon_GetKeyedObjectRelativeVramAddr(uint16 pos, uint16 y);
void Dungeon_ClearRupeeTile(uint16 x, uint16 y);
void Dungeon_StartInterRoomTrans_Left();
void Dung_StartInterRoomTrans_Left_Plus();
void Dung_StartInterRoomTrans_RightPlus();
void Dung_StartInterRoomTrans_DownPlus();
void Dungeon_StartInterRoomTrans_Right();
void Dungeon_StartInterRoomTrans_Up();
void Dungeon_StartInterRoomTrans_Down();

uint16 Dungeon_MapVramAddr(uint16 pos);
uint16 Dungeon_MapVramAddrNoSwap(uint16 pos);
void Bomb_CheckForVulnerableTileObjects(uint16 x, uint16 y, uint8 r14);
uint8 Dungeon_CustomIndexedRevealCoveredTiles(uint16 pos6, uint16 a, Point16U *pt);
uint8 Dungeon_RevealCoveredTiles(Point16U *pt);
uint8 Dungeon_OpenKeyedObject(uint8 tile, int *chest_position);
uint16 Dungeon_QueryIfTileLiftable();
void Dungeon_SaveRoomData();
void Dungeon_SaveRoomData_justKeys();
void SaveQuadrantsToSram();
uint8 Dungeon_ToolAndTileInteraction(uint16 x, uint16 y);

uint16 Dungeon_GetTeleMsg(int room);
uint8 GetEntranceMusicTrack(int entrance);
bool Dungeon_IsPitThatHurtsPlayer();

extern const uint8 kDungAnimatedTiles[24];
