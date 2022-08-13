#include "types.h"
#include "variables.h"
#include "zelda_rtl.h"

#pragma once

void Main_PrepSpritesForNmi();
void Module_MainRouting();


void Sound_LoadLightWorldSongBank();
void Sound_LoadIntroSongBank();
void Sound_LoadIndoorSongBank();
void Sound_LoadEndingSongBank();

uint8 Sound_GetPanForPlayer();
void Sound_SetSfx3Pan(int k, uint8 sfx);
void Sound_SetSfx2Pan(int k, uint8 sfx);
void Sound_SetSfx1Pan(int k, uint8 sfx);
uint8 Sound_GetPanForX(uint16 x);


void AddReceivedItem(uint8 ain, uint8 yin, int chest_pos);

uint8 ToolAndTileInteraction(uint16 x, uint16 y);
uint8 Player_DoSfx2(uint8 a);
void Player_DoSfx3(uint8 a);

void Main_ShowTextMessage();
uint8 LightTorch_GetSfxPan(uint8 a);
void Dungeon_PrepOverlayDma(uint16 x, uint16 y, uint16 r8);
int Dungeon_PrepOverlayDma_nextPrep(int dst, uint16 r8);
int Dungeon_PrepOverlayDma_watergate(int dst, uint16 r8, uint16 r6, int loops);
void WallMaster_SendPlayerToLastEntrance();
void Module_LoadGame_indoors();

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

