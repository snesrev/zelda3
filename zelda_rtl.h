#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include "types.h"
#include "snes_regs.h"


struct ZeldaEnv {
  uint8 *ram;
  uint8 *sram;
  uint16 *vram;
  struct Ppu *ppu;
  struct SpcPlayer *player;
  struct Dma *dma;
};
extern ZeldaEnv g_zenv;
// it's here so that the variables.h can access it
extern uint8 g_ram[131072];



static inline void zelda_snes_dummy_write(uint32_t adr, uint8_t val) {}

const uint16 kUpperBitmasks[] = { 0x8000, 0x4000, 0x2000, 0x1000, 0x800, 0x400, 0x200, 0x100, 0x80, 0x40, 0x20, 0x10, 8, 4, 2, 1 };
const uint8 kLitTorchesColorPlus[] = {31, 8, 4, 0};
const uint8 kDungeonCrystalPendantBit[13] = {0, 0, 4, 2, 0, 16, 2, 1, 64, 4, 1, 32, 8};
const int8 kGetBestActionToPerformOnTile_x[4] = { 7, 7, -3, 16 };
const int8 kGetBestActionToPerformOnTile_y[4] = { 6, 24, 12, 12 };

struct MovableBlockData {
  uint16 room;
  uint16 tilemap;
};

struct OamEntSigned {
  int8 x, y;
  uint8 charnum, flags;
};



#define movable_block_datas ((MovableBlockData*)(g_ram+0xf940))
#define oam_buf ((OamEnt*)(g_ram+0x800))




struct OwScrollVars {
  uint16 ystart, yend, xstart, xend;
};

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


struct MirrorHdmaVars {
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
};


// Various level tables


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
void ZeldaDrawPpuFrame();
void HdmaSetup(uint32 addr6, uint32 addr7, uint8 transfer_unit, uint8 reg6, uint8 reg7, uint8 indirect_bank);
void ZeldaInitializationCode();
void ZeldaRunGameLoop();
void ZeldaInitialize();
void ZeldaRunFrame(uint16 input);
void ClearOamBuffer();
void Startup_InitializeMemory();
void LoadSongBank(const uint8 *p);
void ZeldaWriteSram();
