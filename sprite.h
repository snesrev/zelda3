#pragma once
#include "types.h"
#include "variables.h"

void Sprite_Main();
void SpriteModule_Initialize(int k);
void SpriteActive_Main(int k);
void SpritePrep_LoadProperties(int k);
void SpritePrep_ResetProperties(int k);
void SpritePrep_LoadPalette(int k);

struct PrepOamCoordsRet {
  uint16 x, y;
  uint8 r4;
  uint8 flags;
};

struct SpriteHitBox {
  uint8 r0_xlo;
  uint8 r8_xhi;
  uint8 r1_ylo;
  uint8 r9_yhi;
  uint8 r2, r3;

  uint8 r4_spr_xlo;
  uint8 r10_spr_xhi;

  uint8 r5_spr_ylo;
  uint8 r11_spr_yhi;
  uint8 r6_spr_xsize;
  uint8 r7_spr_ysize;
};

struct SpriteSpawnInfo {
  uint16 r0_x;
  uint16 r2_y;
  uint8 r4_z;
  uint16 r5_overlord_x;
  uint16 r7_overlord_y;
};

extern const uint8 kAbsorbBigKey[2];

uint16 Sprite_GetX(int k);
uint16 Sprite_GetY(int k);
void Sprite_SetX(int k, uint16 x);
void Sprite_SetY(int k, uint16 y);

uint8 Oam_AllocateFromRegionA(uint8 num);
uint8 Oam_AllocateFromRegionB(uint8 num);
uint8 Oam_AllocateFromRegionC(uint8 num);
uint8 Oam_AllocateFromRegionD(uint8 num);
uint8 Oam_AllocateFromRegionE(uint8 num);
uint8 Oam_AllocateFromRegionF(uint8 num);
void OAM_AllocateDeferToPlayer(int k);

struct DrawMultipleData {
  int8 x, y;
  uint16 char_flags;
  uint8 ext;
};

void Sprite_DrawMultiple(int k, const DrawMultipleData *src, int n, PrepOamCoordsRet *info);
void Sprite_DrawMultiplePlayerDeferred(int k, const DrawMultipleData *src, int n, PrepOamCoordsRet *info);

int Sprite_SpawnDynamically(int k, uint8 what, SpriteSpawnInfo *info);
int Sprite_SpawnDynamicallyEx(int k, uint8 what, SpriteSpawnInfo *info, int last);

bool Sprite_PrepOamCoordOrDoubleRet(int k, PrepOamCoordsRet *ret);
void Sprite_PrepOamCoord(int k, PrepOamCoordsRet *ret);
bool Sprite_ReturnIfInactive(int k);
bool Sprite_ReturnIfPaused(int k);
void Sprite_Move(int k);
void Sprite_MoveZ(int k);
void Sprite_MoveXyz(int k);
void Sprite_MoveY(int k);

bool Sprite_ReturnIfRecoiling(int k);
bool Sprite_CheckDamageToPlayer(int k);
bool Sprite_CheckDamageToPlayerSameLayer(int k);
bool Sprite_CheckDamageToPlayerIgnoreLayer(int k);
bool Sprite_CheckDamageToPlayer_1(int k);

uint8 Sprite_CheckTileCollision(int k);
void Sprite_CheckTileCollision2(int k);

void Sprite_ZeroVelocity_XY(int k);
uint8 Sprite_DirectionToFacePlayer(int k, PointU8 *coords_out);
uint8 GetRandomNumber();
void SpriteDraw_Shadow_custom(int k, PrepOamCoordsRet *oam, uint8 a);
void Sprite_DrawShadow(int k, PrepOamCoordsRet *oam);



ProjectSpeedRet Sprite_ProjectSpeedTowardsLink(int k, uint8 vel);
ProjectSpeedRet Sprite_ProjectSpeedTowardsEntity(int k, uint16 x, uint16 y, uint8 vel);
void Sprite_ApplySpeedTowardsLink(int k, uint8 vel);

ProjectSpeedRet Ancilla_ProjectSpeedTowardsPlayer(int k, uint8 vel);

uint8 Sprite_ConvertVelocityToAngle(uint8 x, uint8 y);

void Sprite_PlaceRupulseSpark_2(int k);
void Sprite_ScheduleForBreakage(int k);
void Sprite_HalveSpeed_XY(int k);
void Sprite_Func3(int k);
void Sprite_Sprite_AttemptDamageToLinkPlusRecoil(int k);
void Sprite_Invert_XY_Speeds(int k);
bool Sprite_SetupHitBox00(int k);
void Sprite_InvertSpeed_XY(int k);

void Guard_ParrySwordAttacks(int k);
bool Sprite_CheckTileProperty(int k, int yy);
void Sprite_CheckForTileInDirection_vertical(int k, int yy);
void Sprite_CheckForTileInDirection_horizontal(int k, int yy);
void Sprite_Func8(int k);
void SpriteFall_AdjustPosition(int k);
void Ancilla_CheckDamageToSprite_preset(int k, int a);
void Sprite_AttemptZapDamage(int k);
void Sprite_CalculateSwordDamage(int k);
void Sprite_Func15(int k, int a);
void Sprite_ApplyCalculatedDamage(int k, int a);
void AgahnimBalls_DamageAgahnim(int k, uint8 dmg, uint8 r0_hit_timer);
void Sprite_Func18(int k, uint8 new_type);

void Sprite_PlaceWeaponTink(int k);
uint8 Sprite_CalculateSfxPan(int k);
void Sprite_ApplyConveyor(int k, int yy);
void SpriteAddXY(int k, int xv, int yv);
bool Sprite_CheckTileInDirection(int k, int yy);
bool Entity_CheckSlopedTileCollision(uint16 x, uint16 y);
uint8 Sprite_GetTileAttribute(int k, uint16 *x, uint16 y);
uint8 GetTileAttribute(uint8 floor, uint16 *x, uint16 y);

PairU8 Sprite_IsBelowPlayer(int k);
PairU8 Sprite_IsToRightOfPlayer(int k);

PairU8 Sprite_IsBelowLocation(int k, uint16 y);
PairU8 Sprite_IsRightOfLocation(int k, uint16 x);
uint8 Sprite_DirectionToFaceLocation(int k, uint16 x, uint16 y);
void Sprite_DoHitBoxesFast(int k, SpriteHitBox *hb);

int Sprite_ShowSolicitedMessage(int k, uint16 msg);
int Sprite_ShowMessageOnContact(int k, uint16 msg);
void Sprite_ShowMessageUnconditional(uint16 msg);
bool Sprite_TutorialGuard_ShowMessageOnContact(int k, uint16 msg);
void Sprite_ShowMessageMinimal();

void Link_PlaceWeaponTink();
void Sprite_ApplyRecoilToLink(int k, uint8 vel);
void Sprite_AttemptDamageToLinkWithCollisionCheck(int k);
void Player_SetupActionHitBox(SpriteHitBox *hb);
void Sprite_PrepAndDrawSingleLarge(int k);
void Sprite_PrepAndDrawSingleLargeNoPrep(int k, PrepOamCoordsRet *info);

void Link_SetupHitBox_conditional(SpriteHitBox *hb);
void Link_SetupHitBox(SpriteHitBox *hb);
bool Sprite_CheckIfLinkIsBusy();
bool Sprite_CheckDamageToAndFromLink(int k);

enum {
  kCheckDamageFromPlayer_Carry = 1,
  kCheckDamageFromPlayer_Ne = 2,
};
uint8 Sprite_CheckDamageFromPlayer(int k);
void Sprite_NullifyHookshotDrag();
void ThrowableScenery_InteractWithSpritesAndTiles(int k);
void ThrownSprite_TileAndPeerInteraction(int k);
void ThrownSprite_CheckDamageToSprites(int k);
void ThrownSprite_CheckDamageToSingleSprite(int k, int j);
void ThrowableScenery_ScatterIntoDebris(int k);

void SpriteStunned_Main_Func1(int k);
void Sprite_BounceOffWall(int k);
void Sprite_ApplyRicochet(int k);
void ThrowableScenery_TransmuteIfValid(int k);
void ThrowableScenery_TransmuteToDebris(int k);
void Sprite_Func22(int k);
void Sprite_SpawnSecret(int k);
void Sprite_SpawnLeapingFish(int k);
bool Sprite_ReturnIfLifted(int k);
bool Sprite_ReturnIfLiftedPermissive(int k);
void Sprite_CheckIfLifted_permissive(int k);
uint8 Sprite_BounceFromTileCollision(int k);
void Sprite_Sprite_SetSpawnedCoordinates(int k, SpriteSpawnInfo *info);

void Sprite_DrawRippleIfInWater(int k);
int Sprite_GarnishSpawn_Sparkle(int k, uint16 x, uint16 y);
void Sprite_GarnishSpawn_Sparkle_limited(int k, uint16 x, uint16 y);
bool SpriteDraw_AbsorbableTransient(int k, bool transient);

bool Sprite_ReturnIfPhasingOut(int k);
void Sprite_DrawKey(int k);
void Sprite_DrawNumberedAbsorbable(int k, int a);
void Sprite_PrepAndDrawSingleSmall(int k);

void Sprite_CheckAbsorptionByPlayer(int k);
void Sprite_HandleAbsorptionByPlayer(int k);
void Sprite_ManuallySetDeathFlagUW(int k);
void Sprite_Absorbable_Main(int k);
bool Sprite_HandleDraggingByAncilla(int k);

void SpriteDeath_Func1(int k);
void ForcePrizeDrop(int k, uint8 prize, uint8 slot);
void PrepareEnemyDrop(int k, uint8 item);
void SpriteDeath_Func4(int k);
void SpriteDeath_DrawPoof(int k);
void Sprite_CorrectOamEntries(int k, int n, uint8 islarge);


void SpritePrep_BigKey_load_graphics(int k);
void SpritePrep_KeySetItemDrop(int k);

bool Sprite_CheckIfScreenIsClear();
bool Sprite_CheckIfRoomIsClear();
bool Sprite_CheckIfOverlordsClear();
void Sprite_InitializeMirrorPortal();
void Sprite_InitializeSlots();

void Sprite_DrawDistress_custom(uint16 x, uint16 y, uint8 time);
void SpriteDraw_FallingHelmaBeetle(int k);
void SpriteDraw_FallingHumanoid(int k);

void Sprite_BehaveAsBarrier(int k);
bool Sprite_TrackBodyToHead(int k);

void Sprite_KillFriends();
void SpriteStunned_MainEx(int k, bool second_entry);

void Sprite_OverworldReloadAll_justLoad();
void Sprite_ReloadAll_Overworld();

static inline uint8 ClampYForOam(uint16 y) {
  return (uint16)(y + 0x10) < 0x100 ? y : 0xf0;
}
void Sprite_Get_16_bit_Coords(int k);

void Sprite_SetupHitBox(int k, SpriteHitBox *hb);
bool Utility_CheckIfHitBoxesOverlap(SpriteHitBox *hb);

void Sprite_SpawnPoofGarnish(int j);
void Sprite_MoveX(int k);
void Ancilla_SpawnFallingPrize(uint8 item);
void Sprite_TransmuteToBomb(int k);

void Sprite_KillSelf(int k);

bool Sprite_ReturnIfBossFinished(int k);
void Flame_Draw(int k);

void PrepareDungeonExitFromBossFight();
extern const uint8 kAbsorptionSfx[15];
extern const uint8 kSpriteInit_BumpDamage[243];
extern const uint16 kSinusLookupTable[256];
extern const uint8 kThrowableScenery_Flags[9];
extern const uint8 kWishPond2_OamFlags[76];

int GarnishAllocForce();
int GarnishAlloc();
int GarnishAllocLow();
int GarnishAllocLimit(int k);

int GarnishAllocOverwriteOld();
int GarnishAllocOverwriteOldLow();
void Garnish_SetX(int k, uint16 x);
void Garnish_SetY(int k, uint16 y);
int Sprite_SpawnSmallSplash(int k);
void SpriteDraw_WaterRipple(int k);

void Sprite_ApproachTargetSpeed(int k, uint8 x, uint8 y);
void Sprite_HaltAllMovement();
int ReleaseBeeFromBottle();
void Sprite_InitializedSegmented(int k);
void Sprite_MakeBossDeathExplosion_NoSound(int k);
int SpawnBossPoof(int k);
void Soldier_Draw(int k);
void Sprite_SpawnBatCrashCutscene();
void Sprite_Main();
void Sprite_SpawnImmediatelySmashedTerrain(uint8 what, uint16 x, uint16 y);
void Sprite_SpawnThrowableTerrain(uint8 what, uint16 x, uint16 y);
int ReleaseFairy();

void Sprite_Main();