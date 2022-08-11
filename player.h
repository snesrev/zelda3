#pragma once
#include "types.h"

extern const uint8 kSwimmingTab1[4];
extern const uint8 kSwimmingTab2[2];


bool Player_IsScreenTransitionBlocked();
void Init_Player();
void Player_HaltDashAttack();

void Player_Main();
void Player_MovePosition1();
void Player_UpdateDirection_Part2();
void Player_UpdateDirection();
void Player_CheckCrossQuadrantBoundary();
void Player_SomethingWithVelocity();
void Player_RepelDashAttack();
void Player_AfterMirrorWarp();
void Player_ResetSomeCrap();
void Link_Move_Helper5();
void Player_ResetSwimState();
void Link_ResetSomething1();
void Player_ResetSwimCollision();

void LinkItem_RemoveFromHand();
void Player_ResetState();
void Player_ResetState2();
void Player_ResetState3();
void Player_ResetSomeStuff2(uint8 a);
void Player_UpdateQuadrantsVisited();
void Item_Cape_StopWithAnim();
void Link_TempBunny_Func1();
void Link_TempBunny_Func2();

void Link_SetInBed();
void Link_Swim_Helper1();
void Link_Swim_Helper2();
void Link_Swim_Helper3();
void Link_Swim_Helper4();
void Link_Swim_Helper5();
void PlayerHandler_04_Swimming_Inner();
void Player_ApplyFloorVel();
void Link_MoveXY_Helper();
void Link_ReceiveItem(uint8 item, int chest_position);

void Link_ReceiveItem(uint8 item, int chest_position);
void Player_CheckDoorwayQuadrantMovement();
void DoSwordInteractionWithTiles(uint8 item);
void Player_HandleIncapacitated(bool jump_into_middle);
void HoleToDungeon_Helper1();

void Player_InitiateFirstBombosSpell();
void Player_InitiateFirstEtherSpell();
void Link_CheckSwimCapability();
void Link_SetSpinAttacking();
void LinkItem_ReturnUnusedMagic(uint8 x);
void Player_CacheStatePriorToHandler();

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

