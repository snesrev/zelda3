#pragma once
#include "types.h"

extern const uint8 kSwimmingTab1[4];
extern const uint8 kSwimmingTab2[2];


bool Link_CheckForEdgeScreenTransition();
void Link_Initialize();
void Link_CancelDash();

void Link_Main();
void LinkHop_FindArbitraryLandingSpot();
void Link_HandleMovingAnimation_StartWithDash();
void Link_HandleMovingAnimation_FullLongEntry();
void ApplyLinksMovementToCamera();
void Link_HandleVelocity();
void Sprite_RepelDash();
void HandleFollowersAfterMirroring();
void Link_ItemReset_FromOverworldThings();
void RepelDash();
void Link_ResetSwimmingState();
void Link_ResetStateAfterDamagingPit();
void ResetAllAcceleration();

void Link_ResetBoomerangYStuff();
void Link_ResetProperties_A();
void Link_ResetProperties_B();
void Link_ResetProperties_C();
void ResetSomeThingsAfterDeath(uint8 a);
void SetAndSaveVisitedQuadrantFlags();
void Link_ForceUnequipCape();
void LinkState_Bunny_recache();
void Link_TempBunny_Func2();

void Link_TuckIntoBed();
void Link_HandleSwimAccels();
void Link_FlagMaxAccels();
void Link_SetIceMaxAccel();
void Link_SetMomentum();
void Link_SetTheMaxAccel();
void Link_HandleSwimMovements();
void Link_ApplyMovingFloorVelocity();
void LinkApplyTileRebound();
void Link_ReceiveItem(uint8 item, int chest_position);

void Link_ReceiveItem(uint8 item, int chest_position);
void HandleIndoorCameraAndDoors();
void TileDetect_MainHandler(uint8 item);
void Link_HandleRecoilAndTimer(bool jump_into_middle);
void HandleDungeonLandingFromPit();

void BombosTablet_StartCutscene();
void EtherTablet_StartCutscene();
void CheckAbilityToSwim();
void Link_SetSpinAttacking();
void Refund_Magic(uint8 x);
void CacheCameraProperties();

enum {
  kPlayerState_Ground = 0,
  kPlayerState_FallingIntoHole = 1,
  kPlayerState_RecoilWall = 2,
  kPlayerState_SpinAttacking = 3,
  kPlayerState_Swimming = 4,
  kPlayerState_TurtleRock = 5,
  kPlayerState_RecoilOther = 6,
  kPlayerState_Electrocution = 7,
  kPlayerState_Ether = 8,
  kPlayerState_Bombos = 9,
  kPlayerState_Quake = 10,
  kPlayerState_FallOfLeftRightLedge = 12,
  kPlayerState_JumpOffLedgeDiag = 14,
  kPlayerState_StartDash = 17,
  kPlayerState_StopDash = 18,
  kPlayerState_Hookshot = 19,
  kPlayerState_Mirror = 20,
  kPlayerState_HoldUpItem = 21,
  kPlayerState_AsleepInBed = 22,
  kPlayerState_PermaBunny = 23,
  kPlayerState_ReceivingEther = 25,
  kPlayerState_ReceivingBombos = 26,
  kPlayerState_OpeningDesertPalace = 27,
  kPlayerState_TempBunny = 28,
  kPlayerState_PullForRupees = 29,
  kPlayerState_SpinAttackMotion = 30,
};

