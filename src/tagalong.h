#pragma once
#include "types.h"

typedef struct TagalongMessageInfo {
  uint16 y, x, bit, msg, tagalong;
} TagalongMessageInfo;

bool Tagalong_IsFollowing();
bool Follower_ValidateMessageFreedom();
void Follower_MoveTowardsLink();
bool Follower_CheckBlindTrigger();
void Follower_Initialize();
void Sprite_BecomeFollower(int k);
void Follower_Main();
void Follower_NoTimedMessage();
void Follower_CheckGameMode();
void Follower_BasicMover();
void Follower_NotFollowing();
void Follower_OldMan();
void Follower_OldManUnused();
void Follower_DoLayers();
bool Follower_CheckProximityToLink();
void Follower_HandleTrigger();
void Tagalong_Draw();
void Follower_AnimateMovement_preserved(uint8 ain, uint16 xin, uint16 yin);
bool Follower_CheckForTrigger(const TagalongMessageInfo *info);
void Follower_Disable();
void Blind_SpawnFromMaiden(uint16 x, uint16 y);
void Kiki_RevertToSprite(int k);
int Kiki_SpawnHandlerMonke(int k);
void Kiki_SpawnHandler_A(int k);
void Kiki_SpawnHandler_B(int k);
