#include "types.h"
#include "variables.h"
#include "zelda_rtl.h"

#pragma once

void NMI_PrepareSprites();
void Module_MainRouting();


void LoadOverworldSongs();
void Sound_LoadIntroSongBank();
void LoadDungeonSongs();
void LoadCreditsSongs();

uint8 Link_CalculateSfxPan();
void SpriteSfx_QueueSfx3WithPan(int k, uint8 sfx);
void SpriteSfx_QueueSfx2WithPan(int k, uint8 sfx);
void SpriteSfx_QueueSfx1WithPan(int k, uint8 sfx);
uint8 CalculateSfxPan(uint16 x);


void AncillaAdd_ItemReceipt(uint8 ain, uint8 yin, int chest_pos);

uint8 HandleItemTileAction_Overworld(uint16 x, uint16 y);
uint8 Ancilla_Sfx2_Near(uint8 a);
void Ancilla_Sfx3_Near(uint8 a);

void Main_ShowTextMessage();
uint8 CalculateSfxPan_Arbitrary(uint8 a);
void RoomDraw_AdjustTorchLightingChange(uint16 x, uint16 y, uint16 r8);
int Dungeon_PrepOverlayDma_nextPrep(int dst, uint16 r8);
int Dungeon_PrepOverlayDma_watergate(int dst, uint16 r8, uint16 r6, int loops);
void WallMaster_SendPlayerToLastEntrance();
void LoadDungeonRoomRebuildHUD();

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

const uint16 *SrcPtr(uint16 src);

extern const uint16 kMemoryLocationToGiveItemTo[76];

