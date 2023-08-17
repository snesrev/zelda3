#include "types.h"
#include "variables.h"
#include "zelda_rtl.h"

#pragma once









static inline OamEnt *GetOamCurPtr() {
  return (OamEnt *)&g_ram[oam_cur_ptr];
}

static inline int FindInByteArray(const uint8 *data, uint8 lookfor, size_t size) {
  for (size_t i = size; i--;)
    if (data[i] == lookfor)
      return (int)i;
  return -1;
}

static inline int FindInWordArray(const uint16 *data, uint16 lookfor, size_t size) {
  for (size_t i = size; i--;)
    if (data[i] == lookfor)
      return (int)i;
  return -1;
}


extern const uint16 kMemoryLocationToGiveItemTo[76];

const uint16 *SrcPtr(uint16 src);
uint8 Ancilla_Sfx2_Near(uint8 a);
void Ancilla_Sfx3_Near(uint8 a);
void LoadDungeonRoomRebuildHUD();
void Module_Unknown0();
void Module_Unknown1();
void Module_MainRouting();
void NMI_PrepareSprites();
void Sound_LoadIntroSongBank();
void LoadOverworldSongs();
void LoadDungeonSongs();
void LoadCreditsSongs();
void Dungeon_LightTorch();
void RoomDraw_AdjustTorchLightingChange(uint16 x, uint16 y, uint16 r8);
int Dungeon_PrepOverlayDma_nextPrep(int dst, uint16 r8);
int Dungeon_PrepOverlayDma_watergate(int dst, uint16 r8, uint16 r6, int loops);
void Module05_LoadFile();
void Module13_BossVictory_Pendant();
void BossVictory_Heal();
void Dungeon_StartVictorySpin();
void Dungeon_RunVictorySpin();
void Dungeon_CloseVictorySpin();
void Module15_MirrorWarpFromAga();
void Module16_BossVictory_Crystal();
void Module16_04_FadeAndEnd();
void TriforceRoom_LinkApproachTriforce();
void AncillaAdd_ItemReceipt(uint8 ain, uint8 yin, int chest_pos);
void ItemReceipt_GiveBottledItem(uint8 item);
void Module17_SaveAndQuit();
void WallMaster_SendPlayerToLastEntrance();
uint8 GetRandomNumber();
uint8 Link_CalculateSfxPan();
void SpriteSfx_QueueSfx1WithPan(int k, uint8 a);
void SpriteSfx_QueueSfx2WithPan(int k, uint8 a);
void SpriteSfx_QueueSfx3WithPan(int k, uint8 a);
uint8 Sprite_CalculateSfxPan(int k);
uint8 CalculateSfxPan(uint16 x);
uint8 CalculateSfxPan_Arbitrary(uint8 a);
void Init_LoadDefaultTileAttr();
void Main_ShowTextMessage();
uint8 HandleItemTileAction_Overworld(uint16 x, uint16 y);
