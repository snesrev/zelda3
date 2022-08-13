#pragma once
#include "zelda_rtl.h"

void Ancilla_MoveX(int k);
void Ancilla_MoveY(int k);
void Ancilla_MoveZ(int k);
uint16 Ancilla_GetX(int k);
uint16 Ancilla_GetY(int k);
void Ancilla_SetX(int k, uint16 x);
void Ancilla_SetY(int k, uint16 y);
void Ancilla_SetXY(int k, uint16 x, uint16 y);

PairU8 Ancilla_IsBelowPlayer(int k);
PairU8 Ancilla_IsToRightOfPlayer(int k);
ProjectSpeedRet Ancilla_ProjectSpeedTowardsPlayer(int k, uint8 vel);

struct CheckPlayerCollOut {
  uint16 r4, r6;
  uint16 r8, r10;
};

bool Ancilla_CheckPlayerCollision(int k, int j, CheckPlayerCollOut *coll_out);
bool Ancilla_CheckTileCollision_Class2(int k);
bool Ancilla_CheckInitialTileCollision_Class2(int k);

uint8 Ancilla_CheckTileCollision(int k);
bool Ancilla_CheckTileCollisionOneFloor(int k);
bool Ancilla_CheckTileCollisionOneFloorEx(int k, uint16 x, uint16 y);

void Ancilla_DoSfx2(int k, uint8 v);
void Ancilla_DoSfx3(int k, uint8 v);

int Ancilla_AllocInit(uint8 type, uint8 y);
int Ancilla_Func1(uint8 type, uint8 y);

void AddSomarianBlockDivide(int k);
void Ancilla_Main();
int AddAncilla(uint8 a, uint8 y);
void AddLinksSleepZs(uint8 a, uint8 y);


void AncillaSpawn_SwordChargeSparkle();
void AddChargedSpinAttackSparkle();
void AddSpinAttackStartSparkle(uint8 a, uint8 x, uint8 y);
void AddWallTapSpark(uint8 a, uint8 y);
void AddLinksBedSpread(uint8 a);
void AddBreakTowerSeal();
void AddWaterfallSplash();
void AddBirdTravelSomething(uint8 a, uint8 y);
void AddBlastWall();
void AddHappinessPondRupees(uint8 a);
void AddWishPondItem(uint8 a, uint8 x, uint8 y);
void AddSwordChargeSpark(int k);
void AddDashingDust_notYetMoving(uint8 a, uint8 y);
void AddDashingDust(uint8 a, uint8 y);
void AddDashTremor(uint8 a, uint8 y);
void ApplyRumbleToSprites();
void AddDwarfTransformationCloud(uint8 a, uint8 y);
void AddWarpTransformationCloud(uint8 a, uint8 y);
void AddLampFlame(uint8 a, uint8 y);
void AddMagicPowder(uint8 a, uint8 y);
void AddSwordCeremony(uint8 a, uint8 y);
void AddSleepInBedAncilla(uint8 a, uint8 y);
void AddSwordBeam(uint8 y);
void AddVictorySpinEffect();
int AddDoorDebris();

void Quake_StartAnim(uint8 a, uint8 y);
void Ether_StartAnim(uint8 a, uint8 y);
void Bombos_StartAnim(uint8 a, uint8 y);
bool AddTransitionSplash(uint8 A, uint8 Y);
void Link_MoveGravestone(uint8 a, uint8 y);
void AddDisintegratingBushPoof(uint16 x, uint16 y);
uint8 Item_Boomerang_Shoot(uint8 a, uint8 y);
int Item_Bow_Shoot(uint8 a, uint8 ax, uint8 ay, uint16 xcoord, uint16 ycoord);
void Item_Bomb_Place(uint8 a, uint8 y);

void Item_Rod_Shoot_1(uint8 type, uint8 y);
void Item_Rod_Shoot_2(uint8 a, uint8 y);
void AddShovelHitStars(uint8 a, uint8 y);
void Item_Flute_DoAnim1(uint8 a, uint8 y);
void Item_Flute_DoAnim2(uint8 a, uint8 y);

void AddSuperBombExplosion(uint8 a, uint8 y);
void Item_CaneOfSomaria_Shoot(uint8 type, uint8 y);
void Cape_DoAnim(uint8 a, uint8 y);
void GanonEmerges_SpawnTravelBird();
void Cape_DoAnim(uint8 a, uint8 y);

uint8 Ancilla_TerminateSelectInteractives(uint8 y);
void AddPendantOrCrystal(uint8 a, uint8 x, uint8 y);
void AddSomarianPlatformPoof(int k);
void Ancilla_RevivalFaerie();
void Ancilla_TerminateSparkleObjects();
void GameOverText_Draw();

