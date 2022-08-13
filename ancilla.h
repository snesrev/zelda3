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

PairU8 Ancilla_IsBelowLink(int k);
PairU8 Ancilla_IsRightOfLink(int k);
ProjectSpeedRet Ancilla_ProjectSpeedTowardsPlayer(int k, uint8 vel);

struct CheckPlayerCollOut {
  uint16 r4, r6;
  uint16 r8, r10;
};

bool Ancilla_CheckLinkCollision(int k, int j, CheckPlayerCollOut *coll_out);
bool Ancilla_CheckTileCollision_Class2(int k);
bool Ancilla_CheckInitialTileCollision_Class2(int k);

uint8 Ancilla_CheckTileCollision(int k);
bool Ancilla_CheckTileCollisionOneFloor(int k);
bool Ancilla_CheckTileCollision_targeted(int k, uint16 x, uint16 y);

void Ancilla_Sfx2_Pan(int k, uint8 v);
void Ancilla_Sfx3_Pan(int k, uint8 v);

int Ancilla_AllocInit(uint8 type, uint8 y);
int AncillaAdd_AddAncilla_Bank08(uint8 type, uint8 y);

void AncillaAdd_ExplodingSomariaBlock(int k);
void Ancilla_Main();
int Ancilla_AddAncilla(uint8 a, uint8 y);
void AncillaAdd_SwordSwingSparkle(uint8 a, uint8 y);


void AncillaSpawn_SwordChargeSparkle();
void AncillaAdd_ChargedSpinAttackSparkle();
void AncillaAdd_SpinAttackInitSpark(uint8 a, uint8 x, uint8 y);
void AncillaAdd_WallTapSpark(uint8 a, uint8 y);
void AncillaAdd_Blanket(uint8 a);
void AncillaAdd_GTCutscene();
void AncillaAdd_WaterfallSplash();
void AddBirdTravelSomething(uint8 a, uint8 y);
void AncillaAdd_BlastWall();
void AddHappinessPondRupees(uint8 a);
void AncillaAdd_TossedPondItem(uint8 a, uint8 x, uint8 y);
void AncillaAdd_SwordChargeSparkle(int k);
void AncillaAdd_DashDust_charging(uint8 a, uint8 y);
void AncillaAdd_DashDust(uint8 a, uint8 y);
void AncillaAdd_DashTremor(uint8 a, uint8 y);
void Prepare_ApplyRumbleToSprites();
void AncillaAdd_DwarfPoof(uint8 a, uint8 y);
void AncillaAdd_BunnyPoof(uint8 a, uint8 y);
void AncillaAdd_LampFlame(uint8 a, uint8 y);
void AncillaAdd_MagicPowder(uint8 a, uint8 y);
void AncillaAdd_MSCutscene(uint8 a, uint8 y);
void AncillaAdd_Snoring(uint8 a, uint8 y);
void AddSwordBeam(uint8 y);
void AncillaAdd_VictorySpin();
int AncillaAdd_DoorDebris();

void AncillaAdd_QuakeSpell(uint8 a, uint8 y);
void AncillaAdd_EtherSpell(uint8 a, uint8 y);
void AncillaAdd_BombosSpell(uint8 a, uint8 y);
bool AncillaAdd_Splash(uint8 A, uint8 Y);
void AncillaAdd_GraveStone(uint8 a, uint8 y);
void AncillaAdd_BushPoof(uint16 x, uint16 y);
uint8 AncillaAdd_Boomerang(uint8 a, uint8 y);
int AncillaAdd_Arrow(uint8 a, uint8 ax, uint8 ay, uint16 xcoord, uint16 ycoord);
void AncillaAdd_Bomb(uint8 a, uint8 y);

void AncillaAdd_FireRodShot(uint8 type, uint8 y);
void RodItem_CreateIceShot(uint8 a, uint8 y);
void Ancilla_AddHitStars(uint8 a, uint8 y);
void AncillaAdd_ExplodingWeatherVane(uint8 a, uint8 y);
void AncillaAdd_Duck_take_off(uint8 a, uint8 y);

void AncillaAdd_SuperBombExplosion(uint8 a, uint8 y);
void AncillaAdd_SomariaBlock(uint8 type, uint8 y);
void AncillaAdd_CapePoof(uint8 a, uint8 y);
void CallForDuckIndoors();
void AncillaAdd_CapePoof(uint8 a, uint8 y);

uint8 Ancilla_TerminateSelectInteractives(uint8 y);
void AncillaAdd_FallingPrize(uint8 a, uint8 x, uint8 y);
void AncillaAdd_SomariaPlatformPoof(int k);
void RevivalFairy_Main();
void Ancilla_TerminateSparkleObjects();
void GameOverText_Draw();

