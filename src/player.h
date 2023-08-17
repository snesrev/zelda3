#pragma once
#include "types.h"

extern const uint8 kSwimmingTab1[4];
extern const uint8 kSwimmingTab2[2];

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






void Dungeon_HandleLayerChange();
void CacheCameraProperties();
void CheckAbilityToSwim();
void Link_Main();
void Link_ControlHandler();
void LinkState_Default();
void HandleLink_From1D();
void PlayerHandler_00_Ground_3();
bool Link_HandleBunnyTransformation();
void LinkState_TemporaryBunny();
void PlayerHandler_17_Bunny();
void LinkState_Bunny_recache();
void Link_TempBunny_Func2();
void LinkState_HoldingBigRock();
void EtherTablet_StartCutscene();
void LinkState_ReceivingEther();
void BombosTablet_StartCutscene();
void LinkState_ReceivingBombos();
void LinkState_ReadingDesertTablet();
void HandleSomariaAndGraves();
void LinkState_Recoil();
void Link_HandleRecoilAndTimer(bool jump_into_middle);
void LinkState_OnIce();
void Link_HandleChangeInZVelocity();
void Player_ChangeZ(uint8 zd);
void LinkHop_HoppingSouthOW();
void LinkState_HandlingJump();
void LinkHop_FindTileToLandOnSouth();
void LinkState_HoppingHorizontallyOW();
void Link_HoppingHorizontally_FindTile_Y();
void Link_SetToDeepWater();
void LinkState_0F();
uint8 Link_HoppingHorizontally_FindTile_X(uint8 o);
void LinkState_HoppingDiagonallyUpOW();
void LinkState_HoppingDiagonallyDownOW();
void LinkHop_FindLandingSpotDiagonallyDown();
void Link_SplashUponLanding();
void LinkState_Dashing();
void LinkState_ExitingDash();
void Link_CancelDash();
void RepelDash();
void LinkApplyTileRebound();
void Sprite_RepelDash();
void Flag67WithDirections();
void LinkState_Pits();
void HandleLayerOfDestination();
void DungeonPitDoDamage();
void HandleDungeonLandingFromPit();
void PlayerHandler_04_Swimming();
void Link_HandleSwimMovements();
void Link_FlagMaxAccels();
void Link_SetIceMaxAccel();
void Link_SetMomentum();
void Link_ResetSwimmingState();
void Link_ResetStateAfterDamagingPit();
void ResetAllAcceleration();
void Link_HandleSwimAccels();
void Link_SetTheMaxAccel();
void LinkState_Zapped();
void PlayerHandler_15_HoldItem();
void Link_ReceiveItem(uint8 item, int chest_position);
void Link_TuckIntoBed();
void LinkState_Sleeping();
void Link_HandleSwordCooldown();
void Link_HandleYItem();
void Link_HandleAPress();
void Link_APress_PerformBasic(uint8 action_x2);
void HandleSwordSfxAndBeam();
void Link_CheckForSwordSwing();
void HandleSwordControls();
void Link_ResetSwordAndItemUsage();
void Player_Sword_SpinAttackJerks_HoldDown();
void LinkItem_Rod();
void LinkItem_Hammer();
void LinkItem_Bow();
void LinkItem_Boomerang();
void Link_ResetBoomerangYStuff();
void LinkItem_Bombs();
void LinkItem_Bottle();
void LinkItem_Lamp();
void LinkItem_Powder();
void LinkItem_ShovelAndFlute();
void LinkItem_Shovel();
void LinkItem_Flute();
void LinkItem_Book();
void LinkItem_Ether();
void LinkState_UsingEther();
void LinkItem_Bombos();
void LinkState_UsingBombos();
void LinkItem_Quake();
void LinkState_UsingQuake();
void Link_ActivateSpinAttack();
void Link_AnimateVictorySpin();
void LinkState_SpinAttack();
void LinkItem_Mirror();
void DoSwordInteractionWithTiles_Mirror();
void LinkState_CrossingWorlds();
void Link_PerformDesertPrayer();
void HandleFollowersAfterMirroring();
void LinkItem_Hookshot();
void LinkState_Hookshotting();
void LinkItem_Cape();
void Link_ForceUnequipCape();
void Link_ForceUnequipCape_quietly();
void HaltLinkWhenUsingItems();
void Link_HandleCape_passive_LiftCheck();
void Player_CheckHandleCapeStuff();
void LinkItem_CaneOfSomaria();
void LinkItem_CaneOfByrna();
bool SearchForByrnaSpark();
void LinkItem_Net();
bool CheckYButtonPress();
bool LinkCheckMagicCost(uint8 x);
void Refund_Magic(uint8 x);
void Link_ItemReset_FromOverworldThings();
void Link_PerformThrow();
void Link_APress_LiftCarryThrow();
void Link_PerformDash();
void Link_PerformGrab();
void Link_APress_PullObject();
void Link_PerformStatueDrag();
void Link_APress_StatueDrag();
void Link_PerformRupeePull();
void LinkState_TreePull();
void Link_PerformRead();
void Link_PerformOpenChest();
bool Link_CheckNewAPress();
bool Link_HandleToss();
void Link_HandleDiagonalCollision();
void Player_LimitDirections_Inner();
void Link_HandleCardinalCollision();
void RunSlopeCollisionChecks_VerticalFirst();
void RunSlopeCollisionChecks_HorizontalFirst();
bool CheckIfRoomNeedsDoubleLayerCheck();
void CreateVelocityFromMovingBackground();
void StartMovementCollisionChecks_Y();
void StartMovementCollisionChecks_Y_HandleIndoors();
void HandlePushingBonkingSnaps_Y();
void StartMovementCollisionChecks_Y_HandleOutdoors();
bool RunLedgeHopTimer(); // carry
void Link_BonkAndSmash();
void Link_AddInVelocityYFalling();
void CalculateSnapScratch_Y();
void ChangeAxisOfPerpendicularDoorMovement_Y();
void Link_AddInVelocityY();
void Link_HopInOrOutOfWater_Y();
void Link_FindValidLandingTile_North();
void Link_FindValidLandingTile_DiagonalNorth();
void StartMovementCollisionChecks_X();
void StartMovementCollisionChecks_X_HandleIndoors();
void HandlePushingBonkingSnaps_X();
void StartMovementCollisionChecks_X_HandleOutdoors();
void SnapOnX();
void CalculateSnapScratch_X();
int8 ChangeAxisOfPerpendicularDoorMovement_X();
void Link_HopInOrOutOfWater_X();
void Link_HandleDiagonalKickback();
void TileDetect_MainHandler(uint8 item);
bool Link_PermissionForSloshSounds();
bool PushBlock_AttemptToPushTheBlock(uint8 what, uint16 x, uint16 y);
uint8 Link_HandleLiftables();
void HandleNudging(int8 arg_r0);
void TileBehavior_HandleItemAndExecute(uint16 x, uint16 y);
uint8 PushBlock_GetTargetTileFlag(uint16 x, uint16 y);
void FlagMovingIntoSlopes_Y();
void FlagMovingIntoSlopes_X();
void Link_HandleRecoiling();
void Player_HandleIncapacitated_Inner2();
void Link_HandleVelocity();
void Link_MovePosition();
void Link_HandleVelocityAndSandDrag(uint16 x, uint16 y);
void HandleSwimStrokeAndSubpixels();
void Player_SomethingWithVelocity_TiredOrSwim(uint16 xvel, uint16 yvel);
void Link_HandleMovingFloor();
void Link_ApplyMovingFloorVelocity();
void Link_ApplyConveyor();
void Link_HandleMovingAnimation_FullLongEntry();
void Link_HandleMovingAnimation_StartWithDash();
void Link_HandleMovingAnimationSwimming();
void Link_HandleMovingAnimation_Dash();
void HandleIndoorCameraAndDoors();
void HandleDoorTransitions();
void ApplyLinksMovementToCamera();
uint8 FindFreeMovingBlockSlot(uint8 x);
bool InitializePushBlock(uint8 r14, uint8 idx);
void Sprite_Dungeon_DrawSinglePushBlock(int j);
void Link_Initialize();
void Link_ResetProperties_A();
void Link_ResetProperties_B();
void Link_ResetProperties_C();
bool Link_CheckForEdgeScreenTransition();
void CacheCameraPropertiesIfOutdoors();
void SomariaBlock_HandlePlayerInteraction(int k);
void Gravestone_Move(int k);
void Gravestone_ActAsBarrier(int k);
void AncillaAdd_DugUpFlute(uint8 a, uint8 y);
void AncillaAdd_CaneOfByrnaInitSpark(uint8 a, uint8 y);
void AncillaAdd_ShovelDirt(uint8 a, uint8 y);
void AncillaAdd_Hookshot(uint8 a, uint8 y);
void ResetSomeThingsAfterDeath(uint8 a);
void SpawnHammerWaterSplash();
void DiggingGameGuy_AttemptPrizeSpawn();
