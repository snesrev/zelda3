#pragma once
#include "types.h"

enum kHudItems {

  kHudItem_Bombs = 4,
  kHudItem_Mushroom = 5,
  kHudItem_Hammer = 12,
  kHudItem_Flute = 13,
  kHudItem_BookMudora = 15,
  kHudItem_BottleOld = 16,
  
  kHudItem_Shovel = 16,
  kHudItem_Bottle1 = 21,
  kHudItem_Bottle2 = 22,
  kHudItem_Bottle3 = 23,
  kHudItem_Bottle4 = 24,
};

void Hud_RefreshIcon();
uint8 CheckPalaceItemPosession();
void Hud_FloorIndicator();
void Hud_RemoveSuperBombIndicator();
void Hud_SuperBombIndicator();
void Hud_RefillLogic();
void Hud_Module_Run();
void Hud_ClearTileMap();
void Hud_Init();
void Hud_BringMenuDown();
void Hud_ChooseNextMode();
void Hud_NormalMenu();
void Hud_UpdateHud();
uint8 Hud_LookupInventoryItem(uint8 item);
void Hud_UpdateEquippedItem();
void Hud_CloseMenu();
void Hud_GotoBottleMenu();
void Hud_InitBottleMenu();
void Hud_ExpandBottleMenu();
void Hud_BottleMenu();
void Hud_DrawBottleMenu_Update();
void Hud_EraseBottleMenu();
void Hud_RestoreNormalMenu();
void Hud_SearchForEquippedItem();
void Hud_DrawYButtonItems();
void Hud_DrawAbilityBox();
void Hud_DrawProgressIcons();
void Hud_DrawProgressIcons_Pendants();
void Hud_DrawProgressIcons_Crystals();
void Hud_DrawSelectedYButtonItem();
void Hud_DrawEquipmentBox();
void Hud_DrawBottleMenu();
bool Hud_RefillHealth();
void Hud_AnimateHeartRefill();
bool Hud_RefillMagicPower();
void Hud_RestoreTorchBackground();
void Hud_RebuildIndoor();
void Hud_Rebuild();
const uint16 *Hud_GetItemBoxPtr(int item);
int GetCurrentItemButtonIndex();
uint8 *GetCurrentItemButtonPtr(int i);

void Hud_HandleItemSwitchInputs();
