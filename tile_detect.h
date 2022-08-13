#pragma once
#include "types.h"

void DetectTiles_3_UpDown(uint16_t direction);
void DetectTiles_4_LeftRight(uint16_t direction);
void DetectTiles_MoveX(uint16 direction);
void DetectTiles_MoveY(uint16 direction);
void DetectTiles_7(int8 speed);
void DetectTiles_9(uint8 dw);
void DetectTiles_8();
void TileDetect_ResetState();
void TileDetect_Execute(uint16 x, uint16 y, uint16 bits);
void TileDetect_ExecuteInner(uint8 tile, uint16 offs, uint16 bits, bool is_indoors);
void Player_TileDetectNearby();
uint8 Overworld_GetTileAttrAtLocation(uint16 x, uint16 y);
void Hookshot_CheckTileCollison(int k);

