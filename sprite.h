#pragma once
#include "types.h"
#include "variables.h"

void Sprite_Main();
void SpritePrep_Main(int k);
void SpriteActive_Main(int k);
void Sprite_LoadProperties(int k);
void Sprite_ResetProperties(int k);
void Sprite_LoadPalette(int k);

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

uint8 OAM_AllocateFromRegionA(uint8 num);
uint8 OAM_AllocateFromRegionB(uint8 num);
uint8 OAM_AllocateFromRegionC(uint8 num);
uint8 OAM_AllocateFromRegionD(uint8 num);
uint8 OAM_AllocateFromRegionE(uint8 num);
uint8 OAM_AllocateFromRegionF(uint8 num);
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
void Sprite_PrepOamCoordSafe(int k, PrepOamCoordsRet *ret);
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

void Sprite_ZeroVelocity(int k);
uint8 Sprite_DirectionToFacePlayer(int k, PointU8 *coords_out);
uint8 GetRandomInt();
void Sprite_DrawShadowEx(int k, PrepOamCoordsRet *oam, uint8 a);
void Sprite_DrawShadow(int k, PrepOamCoordsRet *oam);



ProjectSpeedRet Sprite_ProjectSpeedTowardsPlayer(int k, uint8 vel);
ProjectSpeedRet Sprite_ProjectSpeedTowardsEntity(int k, uint16 x, uint16 y, uint8 vel);
void Sprite_ApplySpeedTowardsPlayer(int k, uint8 vel);

ProjectSpeedRet Ancilla_ProjectSpeedTowardsPlayer(int k, uint8 vel);

uint8 ConvertVelocityToAngle(uint8 x, uint8 y);

void Sprite_PlaceRupulseSpark_2(int k);
void Sprite_Func2(int k);
void Sprite_HalveVelocity(int k);
void Sprite_Func3(int k);
void Sprite_AttemptDamageToPlayerPlusRecoil(int k);
void Sprite_Invert_XY_Speeds(int k);
bool Sprite_CheckDamageToPlayer_CheckCoord(int k);
void Sprite_NegateVel(int k);

void Sprite_Func1(int k);
bool Sprite_Func5(int k, int yy);
void Sprite_Func6(int k, int yy);
void Sprite_Func7(int k, int yy);
void Sprite_Func8(int k);
void Sprite_Func9(int k);
void Sprite_Func11(int k, int a);
void Sprite_Func13(int k);
void Sprite_Func14(int k);
void Sprite_Func15(int k, int a);
void Sprite_Func16(int k, int a);
void Sprite_Func17(int k, uint8 dmg, uint8 r0_hit_timer);
void Sprite_Func18(int k, uint8 new_type);

void Sprite_PlaceRupulseSpark(int k);
uint8 Sprite_GetSfxPan(int k);
void Sprite_ApplyConveyorAdjustment(int k, int yy);
void SpriteAddXY(int k, int xv, int yv);
bool Sprite_Func10(int k, int yy);
bool Entity_CheckSlopedTileCollision(uint16 x, uint16 y);
uint8 Sprite_GetTileAttrLocal(int k, uint16 *x, uint16 y);
uint8 Entity_GetTileAttr(uint8 floor, uint16 *x, uint16 y);

PairU8 Sprite_IsBelowPlayer(int k);
PairU8 Sprite_IsToRightOfPlayer(int k);

PairU8 Sprite_IsBelowEntity(int k, uint16 y);
PairU8 Sprite_IsToRightOfEntity(int k, uint16 x);
uint8 Sprite_DirectionToFaceEntity(int k, uint16 x, uint16 y);
void Sprite_Func12(int k, SpriteHitBox *hb);

int Sprite_ShowSolicitedMessageIfPlayerFacing(int k, uint16 msg);
int Sprite_ShowMessageFromPlayerContact(int k, uint16 msg);
void Sprite_ShowMessageUnconditional(uint16 msg);
bool Sprite_ShowMessageIfPlayerTouching(int k, uint16 msg);
void Sprite_ShowMessageMinimal();

void Player_PlaceRepulseSpark();
void Sprite_ApplyRecoilToPlayer(int k, uint8 vel);
void Sprite_StaggeredCheckDamageToPlayerPlusRecoil(int k);
void Player_SetupActionHitBox(SpriteHitBox *hb);
void Sprite_PrepAndDrawSingleLarge(int k);
void Sprite_PrepAndDrawSingleLargeNoPrep(int k, PrepOamCoordsRet *info);

void GetPlayerHitBoxUnlessDisabled(SpriteHitBox *hb);
void GetPlayerHitBox(SpriteHitBox *hb);
bool Sprite_CheckIfPlayerPreoccupied();
bool Sprite_CheckDamageBoth(int k);

enum {
  kCheckDamageFromPlayer_Carry = 1,
  kCheckDamageFromPlayer_Ne = 2,
};
uint8 Sprite_CheckDamageFromPlayer(int k);
void Sprite_NullifyHookshotDrag();
void ThrowableScenery_InteractWithSpritesAndTiles(int k);
void ThrownSprite_TileAndPeerInteraction(int k);
void ThrownSprite_CheckDamageToPeers(int k);
void ThrownSprite_CheckDamageToSinglePeer(int k, int j);
void ThrowableScenery_ScatterIntoDebris(int k);

void SpriteStunned_Main_Func1(int k);
void Sprite_WallInducedSpeedInversion(int k);
void Sprite_NegateHalveSpeedEtc(int k);
void Sprite_Func20(int k);
void Sprite_Func21(int k);
void Sprite_Func22(int k);
void Sprite_SpawnSecret(int k);
void Fish_SpawnLeapingFish(int k);
bool Sprite_ReturnIfLifted(int k);
bool Sprite_ReturnIfLiftedPermissive(int k);
void Sprite_CheckIfLiftedPermissive(int k);
uint8 Sprite_BounceFromTileCollision(int k);
void Sprite_InitFromInfo(int k, SpriteSpawnInfo *info);

void Sprite_DrawRippleIfInWater(int k);
int Sprite_SpawnSimpleSparkleGarnish(int k, uint16 x, uint16 y);
void Sprite_SpawnSimpleSparkleGarnish_SlotRestricted(int k, uint16 x, uint16 y);
bool Sprite_DrawAbsorbableOrReturn(int k, bool transient);

bool Sprite_ReturnIfPhasingOut(int k);
void Sprite_DrawKey(int k);
void Sprite_DrawNumberedAbsorbable(int k, int a);
void Sprite_PrepAndDrawSingleSmall(int k);

void Sprite_CheckAbsorptionByPlayer(int k);
void Sprite_HandleAbsorptionByPlayer(int k);
void Sprite_SetDungeonSpriteDeathFlag(int k);
void Sprite_CommonItemPickup(int k);
bool Sprite_ReturnHandleDraggingByAncilla(int k);

void SpriteDeath_Func1(int k);
void SpriteDeath_Func2(int k, uint8 prize, uint8 slot);
void SpriteDeath_Func3(int k, uint8 item);
void SpriteDeath_Func4(int k);
void SpriteDeath_DrawPerishingOverlay(int k);
void Sprite_CorrectOamEntries(int k, int n, uint8 islarge);


void SpritePrep_LoadBigKeyGfx(int k);
void SpritePrep_KeySetItemDrop(int k);

bool Sprite_VerifyAllOnScreenDefeated();
bool Sprite_VerifyAllOnScreenDefeatedB();
bool Sprite_VerifyOverlordDefeated();
void Sprite_ReinitWarpVortex();
void InitSpriteSlots();

void Sprite_CustomTimedDrawDistressMarker(uint16 x, uint16 y, uint8 time);
void Sprite_DrawFall_Type0(int k);
void Sprite_DrawFall_Type1(int k);

void Sprite_PlayerCantPassThrough(int k);
bool Sprite_MakeBodyTrackHeadDirection(int k);

void Sprite_SchedulePeersForDeath();
void SpriteStunned_MainEx(int k, bool second_entry);

void Sprite_OverworldReloadAll_justLoad();
void Sprite_OverworldReloadAll();

static inline uint8 ClampYForOam(uint16 y) {
  return (uint16)(y + 0x10) < 0x100 ? y : 0xf0;
}
void Sprite_Get_16_bit_Coords(int k);

void Sprite_SetupHitBox(int k, SpriteHitBox *hb);
bool Utility_CheckIfHitBoxesOverlap(SpriteHitBox *hb);

void Sprite_SpawnPoofGarnish(int j);
void Sprite_MoveX(int k);
void Sprite_SpawnFallingItem(uint8 item);
void Sprite_TransmuteToEnemyBomb(int k);

void Sprite_SelfTerminate(int k);

bool Sprite_ReturnIfBossFinished(int k);
void Flame_Draw(int k);

void PrepDungeonBossExit();
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
int Sprite_SpawnSmallWaterSplash(int k);
void Sprite_DrawWaterRipple(int k);

void Sprite_ApproachTargetSpeed(int k, uint8 x, uint8 y);
void Player_HaltSpecialPlayerMovement();
int SpawnBee();
void Sprite_InitializedSegmented(int k);
void Sprite_MakeBossDeathExplosion_NoSound(int k);
int Blind_SpawnPoof(int k);
void Soldier_Draw(int k);
void GanonEmerges_SpawnRetreatBat();
void Sprite_Main();
void Sprite_SpawnImmediatelySmashedTerrain(uint8 what, uint16 x, uint16 y);
void Sprite_SpawnThrowableTerrain(uint8 what, uint16 x, uint16 y);
int SpawnFairy();

void Sprite_Main();