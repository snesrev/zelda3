#pragma once
#include "types.h"

void Hud_Module_Run();

void Hud_ClearTileMap();
void Hud_Init();
void Hud_BringMenuDown();
void Hud_ChooseNextMode();
void Hud_NormalMenu();
void Hud_UpdateHud();
void Hud_CloseMenu();
void Hud_GotoBottleMenu();
void Hud_InitBottleMenu();
void Hud_ExpandBottleMenu();
void Hud_BottleMenu();
void Hud_EraseBottleMenu();
void Hud_RestoreNormalMenu();

void Hud_SearchForEquippedItem();
void Hud_Rebuild();
uint16 Hud_GetPaletteMask(uint8 what);
void Hud_DrawYButtonItems(uint16 mask);
void Hud_DrawUnknownBox(uint16 palmask);
void Hud_DrawAbilityText(uint16 palmask);
void Hud_DrawAbilityIcons();
void Hud_DrawGlovesText(uint8 idx);
void Hud_DrawProgressIcons();
void Hud_DrawProgressIcons_Pendants();
void Hud_DrawProgressIcons_Crystals();
uint8 CheckPalaceItemPosession();

void Hud_DrawMoonPearl();
void Hud_DrawEquipment(uint16 palmask);
void Hud_DrawShield();
void Hud_DrawArmor();
void Hud_DrawMapAndBigKey();
void Hud_DrawCompass();

bool Hud_DoWeHaveThisItem();
void Hud_EquipNextItem();
void Hud_EquipPrevItem();
void Hud_EquipItemAbove();
void Hud_EquipItemBelow();

void Hud_DrawSelectedYButtonItem();
void Hud_DrawBottleMenu(uint16 palmask);
void Hud_Update_IgnoreHealth();
void Hud_AnimateHeartRefill();
bool Hud_RefillMagicPower();
void Hud_RestoreTorchBackground();
void Hud_Update_IgnoreItemBox();
bool Hud_RefillHealth();
void Hud_UpdateInternal();
void Hud_Update_IgnoreItemBox();
void Hud_UpdateOnly();
void Hud_UpdateEquippedItem();
void Hud_UpdateBottleMenu();

void Hud_RemoveSuperBombIndicator();
void Hud_SuperBombIndicator();
void Hud_FloorIndicator();
void Hud_RefillLogic();
void Hud_RefreshIcon();
void Hud_RebuildIndoor();

extern const uint16 *const kHudItemBoxGfxPtrs[];