#ifndef ZELDA_RTL_H
#define ZELDA_RTL_H
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include "types.h"

struct Snes;

typedef struct ZeldaEnv {
  uint8 *ram;
  uint8 *sram;
  uint16 *vram;
  struct Ppu *ppu;
  struct SpcPlayer *player;
  struct Dma *dma;
} ZeldaEnv;
extern ZeldaEnv g_zenv;

// it's here so that the variables.h can access it
extern uint8 g_ram[131072];
extern const uint16 kUpperBitmasks[];
extern const uint8 kLitTorchesColorPlus[];
extern const uint8 kDungeonCrystalPendantBit[];
extern const int8 kGetBestActionToPerformOnTile_x[];
extern const int8 kGetBestActionToPerformOnTile_y[];

static inline void zelda_snes_dummy_write(uint32 adr, uint8 val) {}

typedef struct MovableBlockData {
  uint16 room;
  uint16 tilemap;
} MovableBlockData;

typedef struct OamEntSigned {
  int8 x, y;
  uint8 charnum, flags;
} OamEntSigned;



#define movable_block_datas ((MovableBlockData*)(g_ram+0xf940))
#define oam_buf ((OamEnt*)(g_ram+0x800))


typedef struct RoomBounds {
  union {
    struct { uint16 a0, b0, a1, b1; };
    uint16 v[4];
  };
} RoomBounds;
#define room_bounds_y (*(RoomBounds*)(g_ram+0x600))
#define room_bounds_x (*(RoomBounds*)(g_ram+0x608))


typedef struct OwScrollVars {
  uint16 ystart, yend, xstart, xend;
} OwScrollVars;


#define ow_scroll_vars0 (*(OwScrollVars*)(g_ram+0x600))
#define ow_scroll_vars1 (*(OwScrollVars*)(g_ram+0x608))

#define ow_scroll_vars0_exit (*(OwScrollVars*)(g_ram+0xC154))

extern const uint8 kLayoutQuadrantFlags[];
extern const uint8 kVariousPacks[16];
extern const uint8 kMaxBombsForLevel[];
extern const uint8 kMaxArrowsForLevel[];
extern const uint8 kReceiveItem_Tab1[76];
extern const uint8 kHealthAfterDeath[21];
extern const uint8 kReceiveItemGfx[76];
extern const uint16 kOverworld_OffsetBaseY[64];
extern const uint16 kOverworld_OffsetBaseX[64];

// forwards


typedef struct MirrorHdmaVars {
  uint16 var0;
  uint16 var1[2];
  uint16 var3[2];
  uint16 var5;
  uint16 var6;
  uint16 var7;
  uint16 var8;
  uint16 var9;
  uint16 var10;
  uint16 var11;
  uint16 pad;
  uint8 ctr2, ctr;
} MirrorHdmaVars;


// Special RAM locations that are unused but I use for compat things.
enum {
  kRam_APUI00 = 0x648,
  kRam_CrystalRotateCounter = 0x649,
  kRam_BugsFixed = 0x64a,
  kRam_Features0 = 0x64c,
};

enum {
  // Poly rendered uses correct speed
  kBugFix_PolyRenderer = 1,
  kBugFix_AncillaOverwrites = 1,
  kBugFix_Latest = 1,
};

// Enum values for kRam_Features0
enum {
  kFeatures0_ExtendScreen64 = 1,
  kFeatures0_SwitchLR = 2,
  kFeatures0_TurnWhileDashing = 4,
  kFeatures0_MirrorToDarkworld = 8,
  kFeatures0_CollectItemsWithSword = 16,
  kFeatures0_BreakPotsWithSword = 32,
  kFeatures0_DisableLowHealthBeep = 64,
  kFeatures0_SkipIntroOnKeypress = 128,
  kFeatures0_ShowMaxItemsInYellow = 256,
  kFeatures0_MoreActiveBombs = 512,

  // This is set for visual fixes that don't affect game behavior but will affect ram compare.
  kFeatures0_WidescreenVisualFixes = 1024,
};

#define enhanced_features0 (*(uint32*)(g_ram+0x64c))
#define msu_curr_sample (*(uint32*)(g_ram+0x650))
#define msu_volume (*(uint8*)(g_ram+0x654))
#define msu_track (*(uint8*)(g_ram+0x655))
#define hud_cur_item_x (*(uint8*)(g_ram+0x656))
#define hud_inventory_order ((uint8*)(g_ram + 0x225)) // 4x6 bytes

extern uint32 g_wanted_zelda_features;
extern bool msu_enabled;

void zelda_apu_write(uint32_t adr, uint8_t val);
void zelda_apu_write_word(uint32_t adr, uint16_t val);
uint8_t zelda_read_apui00();
uint8_t zelda_apu_read(uint32_t adr);
uint16_t zelda_apu_read_word(uint32_t adr);
void zelda_ppu_write(uint32_t adr, uint8_t val);
void zelda_ppu_write_word(uint32_t adr, uint16_t val);
void zelda_apu_runcycles();
const uint8 *SimpleHdma_GetPtr(uint32 p);

// 512x480 32-bit pixels. Returns true if we instead draw 1024x960
bool ZeldaDrawPpuFrame(uint8 *pixel_buffer, size_t pitch, uint32 render_flags);
void HdmaSetup(uint32 addr6, uint32 addr7, uint8 transfer_unit, uint8 reg6, uint8 reg7, uint8 indirect_bank);
void ZeldaInitializationCode();
void ZeldaRunGameLoop();
void ZeldaInitialize();
void ZeldaRunFrame(uint16 input, int run_what);
void ClearOamBuffer();
void Startup_InitializeMemory();
void LoadSongBank(const uint8 *p);
void ZeldaWriteSram();
void ZeldaReadSram(struct Snes *snes);

void ZeldaPlayMsuAudioTrack();
void MixinMsuAudioData(int16 *audio_buffer, int audio_samples);
void ZeldaOpenMsuFile();
bool ZeldaIsMusicPlaying();


#endif  // ZELDA_RTL_H
