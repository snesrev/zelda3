#pragma once
#include "types.h"


uint8 Overworld_GetTileAttributeAtLocation(uint16 x, uint16 y);
void TileDetect_Movement_Y(uint16 direction);
void TileDetect_Movement_X(uint16 direction);
void TileDetect_Movement_VerticalSlopes(uint16_t direction);
void TileDetect_Movement_HorizontalSlopes(uint16_t direction);
void Player_TileDetectNearby();
void Hookshot_CheckTileCollision(int k);
void Hookshot_CheckSingleLayerTileCollision(uint16 x, uint16 y, int dir);
void HandleNudgingInADoor(int8 speed);
void TileCheckForMirrorBonk();
void TileDetect_SwordSwingDeepInDoor(uint8 dw);
void TileDetect_ResetState();
void TileDetection_Execute(uint16 x, uint16 y, uint16 bits);
void TileDetect_ExecuteInner(uint8 tile, uint16 offs, uint16 bits, bool is_indoors);
