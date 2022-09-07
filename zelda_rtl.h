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
};

enum {
  // Poly rendered uses correct speed
  kBugFix_PolyRenderer = 1,
  kBugFix_AncillaOverwrites = 1,
  kBugFix_Latest = 1,
};


#define scratch_0 (*(uint16*)(g_ram+0x72))
#define scratch_1 (*(uint16*)(g_ram+0x74))
#define srm_var1 (*(uint16*)(g_zenv.sram+0x1ffe))
#define messaging_buf ((uint16*)(g_ram+0x10000))
#define quake_arr1 ((uint8*)(g_ram+0x15800))
#define quake_arr2 ((uint8*)(g_ram+0x15805))
#define quake_var5 (*(uint8*)(g_ram+0x1580A))
#define quake_var1 (*(uint16*)(g_ram+0x1580B))
#define quake_var2 (*(uint16*)(g_ram+0x1580D))
#define quake_var4 (*(uint8*)(g_ram+0x1580F))
#define ether_y3 (*(uint16*)(g_ram+0x15810))
#define ether_var1 (*(uint8*)(g_ram+0x15812))
#define ether_y (*(uint16*)(g_ram+0x15813))
#define ether_x (*(uint16*)(g_ram+0x15815))
#define quake_var3 (*(uint16*)(g_ram+0x1581E))
#define bombos_arr7 ((uint8*)(g_ram+0x15820))
#define bombos_y_lo ((uint8*)(g_ram+0x15824))
#define bombos_y_hi ((uint8*)(g_ram+0x15864))
#define bombos_x_lo ((uint8*)(g_ram+0x158A4))
#define bombos_x_hi ((uint8*)(g_ram+0x158E4))
#define bombos_y_coord2 ((uint16*)(g_ram+0x15924))
#define bombos_x_coord2 ((uint16*)(g_ram+0x1592C))
#define bombos_var4 (*(uint8*)(g_ram+0x15934))
#define bombos_arr3 ((uint8*)(g_ram+0x15935))
#define bombos_arr4 ((uint8*)(g_ram+0x15945))
#define bombos_y_coord ((uint16*)(g_ram+0x15955))
#define bombos_x_coord ((uint16*)(g_ram+0x159D5))
#define bombos_var3 (*(uint8*)(g_ram+0x15A55))
#define bombos_var2 (*(uint8*)(g_ram+0x15A56))
#define bombos_var1 (*(uint8*)(g_ram+0x15A57))

#define happiness_pond_y_vel ((uint8*)(g_ram+0x15800))
#define happiness_pond_x_vel ((uint8*)(g_ram+0x1580C))
#define happiness_pond_z_vel ((uint8*)(g_ram+0x15818))
#define happiness_pond_y_lo ((uint8*)(g_ram+0x15824))
#define happiness_pond_y_hi ((uint8*)(g_ram+0x15830))
#define happiness_pond_x_lo ((uint8*)(g_ram+0x1583C))
#define happiness_pond_x_hi ((uint8*)(g_ram+0x15848))
#define happiness_pond_z ((uint8*)(g_ram+0x15854))
#define happiness_pond_timer ((uint8*)(g_ram+0x15860))
#define happiness_pond_arr1 ((uint8*)(g_ram+0x1586C))
#define happiness_pond_item_to_link ((uint8*)(g_ram+0x1587A))
#define happiness_pond_y_subpixel ((uint8*)(g_ram+0x15886))
#define happiness_pond_x_subpixel ((uint8*)(g_ram+0x15892))
#define happiness_pond_z_subpixel ((uint8*)(g_ram+0x1589E))
#define happiness_pond_step ((uint8*)(g_ram+0x158AA))


#define turn_on_off_water_ctr (*(uint8*)(g_ram+0x424))
#define mirror_vars (*(MirrorHdmaVars*)(g_ram+0x6A0))
#define sprite_N_word ((uint16*)(g_ram+0xBC0))
#define sprite_where_in_overworld ((uint8*)(g_ram+0x1DF80))
#define alt_sprite_B ((uint8*)(g_ram+0x1FA5C))
#define uvram_screen (*(UploadVram_32x32*)&g_ram[0x1000])
#define vram_upload_offset (*(uint16*)(g_ram+0x1000))
#define vram_upload_data ((uint16*)(g_ram+0x1002))
#define vram_upload_tile_buf ((uint16*)(g_ram+0x1100))
#define overworld_entrance_sequence_counter (*(uint8*)(g_ram+0xc8))

#ifndef overworld_tileattr
#define overworld_tileattr ((uint16*)(g_ram+0x2000))
#endif
#define dung_line_ptrs_row0 (*(uint16*)(g_ram+0xbf))
#define star_shaped_switches_tile ((uint16*)(g_ram+0x6A0))
#define dung_inter_starcases ((uint16*)(g_ram+0x6B0))
#define dung_stairs_table_1 ((uint16*)(g_ram+0x6B8))
#define selectfile_var8 (*(uint16*)(g_ram+0x630))

#define R10 (*(uint16*)(g_ram+10))
#define R12 (*(uint16*)(g_ram+12))
#define R14 (*(uint16*)(g_ram+14))
#define R16 (*(uint16*)(g_ram+0xc8))
#define R18 (*(uint16*)(g_ram+0xca))
#define R20 (*(uint16*)(g_ram+0xcc))

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

#endif  // ZELDA_RTL_H
