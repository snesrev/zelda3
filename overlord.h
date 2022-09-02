#pragma once
#include "types.h"

void Overlord_StalfosFactory(int k);
void Overlord_SetX(int k, uint16 v);
void Overlord_SetY(int k, uint16 v);
void Overlord_SpawnBoulder();
void Overlord_Main();
void Overlord_ExecuteAll();
void Overlord_ExecuteSingle(int k);
void Overlord19_ArmosCoordinator_bounce(int k);
void Overlord18_InvisibleStalfos(int k);
void Overlord17_PotTrap(int k);
void Overlord16_ZoroSpawner(int k);
void Overlord15_WizzrobeSpawner(int k);
void Overlord14_TileRoom(int k);
int TileRoom_SpawnTile(int k);
void Overlord10_PirogusuSpawner_left(int k);
void Overlord0A_FallingSquare(int k);
void SpawnFallingTile(int k);
void Overlord09_WallmasterSpawner(int k);
void Overlord08_BlobSpawner(int k);
void Overlord07_MovingFloor(int k);
void Sprite_Overlord_PlayFallingSfx(int k);
void Overlord05_FallingStalfos(int k);
void Overlord06_BadSwitchSnake(int k);
void Overlord02_FullRoomCannons(int k);
void Overlord03_VerticalCannon(int k);
void Overlord_SpawnCannonBall(int k, int xd);
void Overlord01_PositionTarget(int k);
void Overlord_CheckIfActive(int k);
void ArmosCoordinator_RotateKnights(int k);
void ArmosCoordinator_Rotate(int k);
bool ArmosCoordinator_CheckKnights();
void ArmosCoordinator_DisableCoercion(int k);
