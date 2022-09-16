#include "zelda_rtl.h"
#include "variables.h"
#include "misc.h"
#include "nmi.h"
#include "poly.h"
#include "attract.h"
#include "snes/ppu.h"
#include "snes/snes_regs.h"
#include "spc_player.h"

ZeldaEnv g_zenv;
// These point to the rewritten instance of the emu.
uint8 g_ram[131072];
typedef struct SimpleHdma {
  const uint8 *table;
  const uint8 *indir_ptr;
  uint8 rep_count;
  uint8 mode;
  uint8 ppu_addr;
  uint8 indir_bank;
} SimpleHdma;
void SimpleHdma_Init(SimpleHdma *c, DmaChannel *dc);
void SimpleHdma_DoLine(SimpleHdma *c);

static const uint8 bAdrOffsets[8][4] = {
  {0, 0, 0, 0},
  {0, 1, 0, 1},
  {0, 0, 0, 0},
  {0, 0, 1, 1},
  {0, 1, 2, 3},
  {0, 1, 0, 1},
  {0, 0, 0, 0},
  {0, 0, 1, 1}
};
static const uint8 transferLength[8] = {
  1, 2, 2, 4, 4, 4, 2, 4
};
const uint16 kUpperBitmasks[] = { 0x8000, 0x4000, 0x2000, 0x1000, 0x800, 0x400, 0x200, 0x100, 0x80, 0x40, 0x20, 0x10, 8, 4, 2, 1 };
const uint8 kLitTorchesColorPlus[] = {31, 8, 4, 0};
const uint8 kDungeonCrystalPendantBit[13] = {0, 0, 4, 2, 0, 16, 2, 1, 64, 4, 1, 32, 8};
const int8 kGetBestActionToPerformOnTile_x[4] = { 7, 7, -3, 16 };
const int8 kGetBestActionToPerformOnTile_y[4] = { 6, 24, 12, 12 };
#define AT_WORD(x) (uint8)(x), (x)>>8
// direct
static const uint8 kAttractDmaTable0[13] = {0x20, AT_WORD(0x00ff), 0x50, AT_WORD(0xe018), 0x50, AT_WORD(0xe018), 1, AT_WORD(0x00ff), 0};
static const uint8 kAttractDmaTable1[10] = {0x48, AT_WORD(0x00ff), 0x30, AT_WORD(0xd830), 1, AT_WORD(0x00ff), 0};
static const uint8 kHdmaTableForEnding[19] = {
  0x52, AT_WORD(0x600), 8, AT_WORD(0xe2), 8, AT_WORD(0x602), 5, AT_WORD(0x604), 0x10, AT_WORD(0x606), 0x81, AT_WORD(0xe2), 0,
};
static const uint8 kSpotlightIndirectHdma[7] = {0xf8, AT_WORD(0x1b00), 0xf8, AT_WORD(0x1bf0), 0};
static const uint8 kMapModeHdma0[7] = {0xf0, AT_WORD(0xdd27), 0xf0, AT_WORD(0xde07), 0};
static const uint8 kMapModeHdma1[7] = {0xf0, AT_WORD(0xdee7), 0xf0, AT_WORD(0xdfc7), 0};
static const uint8 kAttractIndirectHdmaTab[7] = {0xf0, AT_WORD(0x1b00), 0xf0, AT_WORD(0x1be0), 0};
static const uint8 kHdmaTableForPrayingScene[7] = {0xf8, AT_WORD(0x1b00), 0xf8, AT_WORD(0x1bf0), 0};
void zelda_apu_write(uint32_t adr, uint8_t val) {
  assert(adr >= APUI00 && adr <= APUI03);
  g_zenv.player->input_ports[adr & 0x3] = val;
}

void zelda_apu_write_word(uint32_t adr, uint16_t val) {
  zelda_apu_write(adr, val);
  zelda_apu_write(adr + 1, val >> 8);
}

uint8_t zelda_read_apui00() {
  // This needs to be here because the ancilla code reads
  // from the apu and we don't want to make the core code
  // dependent on the apu timings, so relocated this value
  // to 0x648.
  return g_ram[kRam_APUI00];
}

uint8_t zelda_apu_read(uint32_t adr) {
  return g_zenv.player->port_to_snes[adr & 0x3];
}

uint16_t zelda_apu_read_word(uint32_t adr) {
  uint16_t rv = zelda_apu_read(adr);
  rv |= zelda_apu_read(adr + 1) << 8;
  return rv;
}

void zelda_ppu_write(uint32_t adr, uint8_t val) {
  assert(adr >= INIDISP && adr <= STAT78);
  ppu_write(g_zenv.ppu, (uint8)adr, val);
}

void zelda_ppu_write_word(uint32_t adr, uint16_t val) {
  zelda_ppu_write(adr, val);
  zelda_ppu_write(adr + 1, val >> 8);
}

void zelda_apu_runcycles() {
//  apu_cycle(g_zenv.apu);
}

const uint8 *SimpleHdma_GetPtr(uint32 p) {
  switch (p) {

  case 0xCFA87: return kAttractDmaTable0;
  case 0xCFA94: return kAttractDmaTable1;
  case 0xebd53: return kHdmaTableForEnding;
  case 0x0F2FB: return kSpotlightIndirectHdma;
  case 0xabdcf: return kMapModeHdma0;             // mode7
  case 0xabdd6: return kMapModeHdma1;             // mode7
  case 0xABDDD: return kAttractIndirectHdmaTab;   // mode7
  case 0x2c80c: return kHdmaTableForPrayingScene;

  case 0x1b00: return (uint8 *)mode7_hdma_table;
  case 0x1be0: return (uint8 *)mode7_hdma_table + 0xe0;
  case 0x1bf0: return (uint8 *)mode7_hdma_table + 0xf0;
  case 0xadd27: return (uint8*)kMapMode_Zooms1;
  case 0xade07: return (uint8*)kMapMode_Zooms1 + 0xe0;
  case 0xadee7: return (uint8*)kMapMode_Zooms2;
  case 0xadfc7: return (uint8*)kMapMode_Zooms2 + 0xe0;
  case 0x600: return &g_ram[0x600];
  case 0x602: return &g_ram[0x602];
  case 0x604: return &g_ram[0x604];
  case 0x606: return &g_ram[0x606];
  case 0xe2: return &g_ram[0xe2];
  default:
    assert(0);
    return NULL;
  }
}

void SimpleHdma_Init(SimpleHdma *c, DmaChannel *dc) {
  if (!dc->hdmaActive) {
    c->table = 0;
    return;
  }
  c->table = SimpleHdma_GetPtr(dc->aAdr | dc->aBank << 16);
  c->rep_count = 0;
  c->mode = dc->mode | dc->indirect << 6;
  c->ppu_addr = dc->bAdr;
  c->indir_bank = dc->indBank;
}

void SimpleHdma_DoLine(SimpleHdma *c) {
  if (c->table == NULL)
    return;
  bool do_transfer = false;
  if ((c->rep_count & 0x7f) == 0) {
    c->rep_count = *c->table++;
    if (c->rep_count == 0) {
      c->table = NULL;
      return;
    }
    if(c->mode & 0x40) {
      c->indir_ptr = SimpleHdma_GetPtr(c->indir_bank << 16 | c->table[0] | c->table[1] * 256);
      c->table += 2;
    }
    do_transfer = true;
  }
  if(do_transfer || c->rep_count & 0x80) {
    for(int j = 0, j_end = transferLength[c->mode & 7]; j < j_end; j++) {
      uint8 v = c->mode & 0x40 ? *c->indir_ptr++ : *c->table++;
      zelda_ppu_write(0x2100 + c->ppu_addr + bAdrOffsets[c->mode & 7][j], v);
    }
  }
  c->rep_count--;
}

void ConfigurePpuSideSpace() {
  // Let PPU impl know about the maximum allowed extra space on the sides
  int extra_right = 0, extra_left = 0;
//  printf("main %d, sub %d  (%d, %d, %d)\n", main_module_index, submodule_index, BG2HOFS_copy2, room_bounds_x.v[2 | (quadrant_fullsize_x >> 1)], quadrant_fullsize_x >> 1);
  int mod = main_module_index;
  if (mod == 14)
    mod = saved_module_for_menu;
  if (mod == 9) {
    extra_left = BG2HOFS_copy2 - ow_scroll_vars0.xstart;
    extra_right = ow_scroll_vars0.xend - BG2HOFS_copy2;
  } else if (mod == 7) {
    int qm = quadrant_fullsize_x >> 1;
    extra_left = IntMax(BG2HOFS_copy2 - room_bounds_x.v[qm], 0);
    extra_right = IntMax(room_bounds_x.v[qm + 2] - BG2HOFS_copy2, 0);
  } else if (mod == 20) {
    extra_left = kPpuExtraLeftRight, extra_right = kPpuExtraLeftRight;
  }
  PpuSetExtraSideSpace(g_zenv.ppu, extra_left, extra_right);
}

bool ZeldaDrawPpuFrame(uint8 *pixel_buffer, size_t pitch, uint32 render_flags) {
  SimpleHdma hdma_chans[2];

  bool rv = PpuBeginDrawing(g_zenv.ppu, pixel_buffer, pitch, render_flags);

  dma_startDma(g_zenv.dma, HDMAEN_copy, true);

  SimpleHdma_Init(&hdma_chans[0], &g_zenv.dma->channel[6]);
  SimpleHdma_Init(&hdma_chans[1], &g_zenv.dma->channel[7]);

  // Cheat: Let the PPU impl know about the hdma perspective correction so it can avoid guessing.
  if ((render_flags & kPpuRenderFlags_4x4Mode7) && g_zenv.ppu->mode == 7) {
    if (hdma_chans[0].table == kMapModeHdma0)
      PpuSetMode7PerspectiveCorrection(g_zenv.ppu, kMapMode_Zooms1[0], kMapMode_Zooms1[223]);
    else if (hdma_chans[0].table == kMapModeHdma1)
      PpuSetMode7PerspectiveCorrection(g_zenv.ppu, kMapMode_Zooms2[0], kMapMode_Zooms2[223]);
    else if (hdma_chans[0].table == kAttractIndirectHdmaTab)
      PpuSetMode7PerspectiveCorrection(g_zenv.ppu, mode7_hdma_table[0], mode7_hdma_table[223]);
    else
      PpuSetMode7PerspectiveCorrection(g_zenv.ppu, 0, 0);
  }

  if (g_zenv.ppu->extraLeftRight != 0)
    ConfigurePpuSideSpace();

  for (int i = 0; i < 225; i++) {
    if (i == 128 && irq_flag) {
      zelda_ppu_write(BG3HOFS, selectfile_var8);
      zelda_ppu_write(BG3HOFS, selectfile_var8 >> 8);
      zelda_ppu_write(BG3VOFS, 0);
      zelda_ppu_write(BG3VOFS, 0);
      if (irq_flag & 0x80) {
        irq_flag = 0;
        zelda_snes_dummy_write(NMITIMEN, 0x81);
      }
    }
    ppu_runLine(g_zenv.ppu, i);
    SimpleHdma_DoLine(&hdma_chans[0]);
    SimpleHdma_DoLine(&hdma_chans[1]);
  }

  return rv;
}

void HdmaSetup(uint32 addr6, uint32 addr7, uint8 transfer_unit, uint8 reg6, uint8 reg7, uint8 indirect_bank) {
  Dma *dma = g_zenv.dma;
  if (addr6) {
    dma_write(dma, DMAP6, transfer_unit);
    dma_write(dma, BBAD6, reg6);
    dma_write(dma, A1T6L, addr6);
    dma_write(dma, A1T6H, addr6 >> 8);
    dma_write(dma, A1B6, addr6 >> 16);
    dma_write(dma, DAS60, indirect_bank);
  }
  dma_write(dma, DMAP7, transfer_unit);
  dma_write(dma, BBAD7, reg7);
  dma_write(dma, A1T7L, addr7);
  dma_write(dma, A1T7H, addr7 >> 8);
  dma_write(dma, A1B7, addr7 >> 16);
  dma_write(dma, DAS70, indirect_bank);
}

void ZeldaInitializationCode() {
  zelda_snes_dummy_write(NMITIMEN, 0);
  zelda_snes_dummy_write(HDMAEN, 0);
  zelda_snes_dummy_write(MDMAEN, 0);
  zelda_apu_write(APUI00, 0);
  zelda_apu_write(APUI01, 0);
  zelda_apu_write(APUI02, 0);
  zelda_apu_write(APUI03, 0);
  zelda_ppu_write(INIDISP, 0x80);

  Sound_LoadIntroSongBank();
  Startup_InitializeMemory();

  animated_tile_data_src = 0xa680;
  dma_source_addr_9 = 0xb280;
  dma_source_addr_14 = 0xb280 + 0x60;
  zelda_snes_dummy_write(NMITIMEN, 0x81);
}

void ZeldaRunGameLoop() {
  frame_counter++;
  ClearOamBuffer();
  Module_MainRouting();
  NMI_PrepareSprites();
  nmi_boolean = 0;
}

void ZeldaInitialize() {
  g_zenv.dma = dma_init(NULL);
  g_zenv.ppu = ppu_init(NULL);
  g_zenv.ram = g_ram;
  g_zenv.sram = (uint8*)calloc(8192, 1);
  g_zenv.vram = g_zenv.ppu->vram;
  g_zenv.player = SpcPlayer_Create();
  SpcPlayer_Initialize(g_zenv.player);
  dma_reset(g_zenv.dma);
  ppu_reset(g_zenv.ppu);
}

void ZeldaRunPolyLoop() {
  if (intro_did_run_step && !nmi_flag_update_polyhedral) {
    Poly_RunFrame();
    intro_did_run_step = 0;
    nmi_flag_update_polyhedral = 0xff;
  }
}

void ZeldaRunFrame(uint16 input, int run_what) {
  if (animated_tile_data_src == 0)
    ZeldaInitializationCode();

  if (run_what & 2)
    ZeldaRunPolyLoop();
  if (run_what & 1)
    ZeldaRunGameLoop();
  Interrupt_NMI(input);
}

void ClearOamBuffer() {  // 80841e
  for (int i = 0; i < 128; i++)
    oam_buf[i].y = 0xf0;
}

void Startup_InitializeMemory() {  // 8087c0
  memset(g_ram + 0x0, 0, 0x2000);
  main_palette_buffer[0] = 0;
  srm_var1 = 0;
  uint8 *sram = g_zenv.sram;
  if (WORD(sram[0x3e5]) != 0x55aa)
    WORD(sram[0x3e5]) = 0;
  if (WORD(sram[0x8e5]) != 0x55aa)
    WORD(sram[0x8e5]) = 0;
  if (WORD(sram[0xde5]) != 0x55aa)
    WORD(sram[0xde5]) = 0;
  zelda_ppu_write(TMW, 0);
  INIDISP_copy = 0x80;
  flag_update_cgram_in_nmi++;
}

void LoadSongBank(const uint8 *p) {  // 808888
  SpcPlayer_Upload(g_zenv.player, p);
}


bool msu_enabled;
static FILE *msu_file;
static uint32 msu_loop_start;
static uint32 msu_buffer_size, msu_buffer_pos;
static uint8 msu_buffer[65536];

static const uint8 kMsuTracksWithRepeat[48] = {
  1,0,1,1,1,1,1,1,0,1,0,1,1,1,1,0,
  1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,
  1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,
};

#define msu_curr_sample (*(uint32*)(g_ram+0x650))
#define msu_volume (*(uint8*)(g_ram+0x654))
#define msu_track (*(uint8*)(g_ram+0x655))

bool ZeldaIsMusicPlaying() {
  if (msu_track) {
    return msu_file != NULL;
  } else {
    return g_zenv.player->port_to_snes[0] != 0;
  }
}

void ZeldaOpenMsuFile() {
  if (msu_file) fclose(msu_file), msu_file = NULL;
  if (msu_track == 0)
    return;
  char buf[40], hdr[8];
  sprintf(buf, "msu/alttp_msu-%d.pcm", msu_track);
  msu_file = fopen(buf, "rb");
  if (msu_file == NULL || fread(hdr, 1, 8, msu_file) != 8 || *(uint32 *)(hdr + 0) != '1USM') {
    if (msu_file != NULL) fclose(msu_file), msu_file = NULL;
    zelda_apu_write(APUI00, msu_track);
    msu_track = 0;
    return;
  }
  if (msu_curr_sample != 0)
    fseek(msu_file, msu_curr_sample * 4 + 8, SEEK_SET);
  printf("Loading MSU PCM file: %s\n", buf);
  msu_loop_start = *(uint32 *)(hdr + 4);
  msu_buffer_size = msu_buffer_pos = 0;
}

void ZeldaPlayMsuAudioTrack() {
  if (!msu_enabled) normal_playback: {
    msu_track = 0;
    zelda_apu_write(APUI00, music_control);
    return;
  }
  if ((music_control & 0xf0) != 0xf0) {
    msu_track = music_control;
    msu_volume = 255;
    msu_curr_sample = 0;
    ZeldaOpenMsuFile();
  } else if (msu_file == NULL) {
    goto normal_playback;
  }
  zelda_apu_write(APUI00, 0xf1);  // pause spc player
}

void MixinMsuAudioData(int16 *audio_buffer, int audio_samples) {  
  if (msu_file == NULL)
    return;  // msu inactive
  // handle volume fade
  if (last_music_control >= 0xf1) {
    if (last_music_control == 0xf1)
      msu_volume = IntMax(msu_volume - 3, 0);
    else if (last_music_control == 0xf2)
      msu_volume = IntMax(msu_volume - 3, 0x40);
    else if (last_music_control == 0xf3)
      msu_volume = IntMin(msu_volume + 3, 0xff);
  }
  if (msu_volume == 0)
    return;
  int last_audio_samples = 0;
  for (;;) {
    if (msu_buffer_pos >= msu_buffer_size) {
      msu_buffer_size = (int)fread(msu_buffer, 4, sizeof(msu_buffer) / 4, msu_file);
      msu_buffer_pos = 0;
    }
    int nr = IntMin(audio_samples, msu_buffer_size - msu_buffer_pos);
    uint8 *buf = msu_buffer + msu_buffer_pos * 4;
    msu_buffer_pos += nr;
    msu_curr_sample += nr;
    int volume = msu_volume + 1;
    if (volume == 256) {
      for (int i = 0; i < nr; i++) {
        audio_buffer[i * 2 + 0] += ((int16 *)buf)[i * 2 + 0];
        audio_buffer[i * 2 + 1] += ((int16 *)buf)[i * 2 + 1];
      }
    } else {
      for (int i = 0; i < nr; i++) {
        audio_buffer[i * 2 + 0] += ((int16 *)buf)[i * 2 + 0] * volume >> 8;
        audio_buffer[i * 2 + 1] += ((int16 *)buf)[i * 2 + 1] * volume >> 8;
      }
    }
    audio_samples -= nr, audio_buffer += nr * 2;
    if (audio_samples == 0)
      break;
    if (nr != 0)
      continue;

    if (last_audio_samples == audio_samples) {  // error?
      zelda_apu_write(APUI00, msu_track);
      fclose(msu_file), msu_file = NULL;
      return;
    }
    last_audio_samples = audio_samples;

    if (!kMsuTracksWithRepeat[msu_track]) {
      fclose(msu_file), msu_file = NULL;
      return;
    }
    fseek(msu_file, msu_loop_start * 4 + 8, SEEK_SET);
    msu_curr_sample = msu_loop_start;
  }
}
