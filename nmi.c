#include "nmi.h"
#include "zelda_rtl.h"
#include "variables.h"
#include "messaging.h"
#include "snes/snes_regs.h"
#include "snes/ppu.h"
#include "tables/generated_bg_tilemaps.h"
#include "tables/generated_link_graphics.h"

static const uint8 kNmiVramAddrs[] = {
  0, 0, 4, 8, 12, 8, 12, 0, 4, 0, 8, 4, 12, 4, 12, 0,
  8, 16, 20, 24, 28, 24, 28, 16, 20, 16, 24, 20, 28, 20, 28, 16,
  24, 96, 104,
};
static PlayerHandlerFunc *const kNmiSubroutines[25] = {
  &NMI_UploadTilemap_doNothing,
  &NMI_UploadTilemap,
  &NMI_UploadBG3Text,
  &NMI_UpdateOWScroll,
  &NMI_UpdateSubscreenOverlay,
  &NMI_UpdateBG1Wall,
  &NMI_TileMapNothing,
  &NMI_UpdateLoadLightWorldMap,
  &NMI_UpdateBG2Left,
  &NMI_UpdateBGChar3and4,
  &NMI_UpdateBGChar5and6,
  &NMI_UpdateBGCharHalf,
  &NMI_UploadSubscreenOverlayLatter,
  &NMI_UploadSubscreenOverlayFormer,
  &NMI_UpdateBGChar0,
  &NMI_UpdateBGChar1,
  &NMI_UpdateBGChar2,
  &NMI_UpdateBGChar3,
  &NMI_UpdateObjChar0,
  &NMI_UpdateObjChar2,
  &NMI_UpdateObjChar3,
  &NMI_UploadDarkWorldMap,
  &NMI_UploadGameOverText,
  &NMI_UpdatePegTiles,
  &NMI_UpdateStarTiles,
};
void NMI_UploadSubscreenOverlayFormer() {
  NMI_HandleArbitraryTileMap(&g_ram[0x12000], 0, 0x40);
}

void NMI_UploadSubscreenOverlayLatter() {
  NMI_HandleArbitraryTileMap(&g_ram[0x13000], 0x40, 0x80);
}

void CopyToVram(uint32 dstv, const uint8 *src, int len) {
  memcpy(&g_zenv.vram[dstv], src, len);
}

void CopyToVramVertical(uint32 dstv, const uint8 *src, int len) {
  assert(!(len & 1));
  uint16 *dst = &g_zenv.vram[dstv];
  for (int i = 0, i_end = len >> 1; i < i_end; i++, dst += 32, src += 2)
    *dst = WORD(*src);
}

void CopyToVramLow(const uint8 *src, uint32 addr, int num) {
  zelda_ppu_write(VMAIN, 0);
  zelda_ppu_write_word(VMADDL, addr);
  for (int i = 0; i < num; i++) {
    zelda_ppu_write(VMDATAL, *src++);
  }
}

void Interrupt_NMI(uint16 joypad_input) {  // 8080c9
  if (music_control == 0) {
    if (zelda_apu_read(APUI00) == last_music_control)
      zelda_apu_write(APUI00, 0);
  } else if (music_control != last_music_control) {
    last_music_control = music_control;
    zelda_apu_write(APUI00, music_control);
    if (music_control < 0xf2)
      music_unk1 = music_control;
    music_control = 0;
  }

  if (sound_effect_ambient == 0) {
    if (zelda_apu_read(APUI01) == sound_effect_ambient)
      zelda_apu_write(APUI01, 0);
  } else {
    sound_effect_ambient_last = sound_effect_ambient;
    zelda_apu_write(APUI01, sound_effect_ambient);
    sound_effect_ambient = 0;
  }
  zelda_apu_write(APUI02, sound_effect_1);
  zelda_apu_write(APUI03, sound_effect_2);
  sound_effect_1 = 0;
  sound_effect_2 = 0;

  zelda_ppu_write(INIDISP, 0x80);
  zelda_snes_dummy_write(HDMAEN, 0);
  if (!nmi_boolean) {
    nmi_boolean = true;
    NMI_DoUpdates();
    NMI_ReadJoypads(joypad_input);
  }

  if (is_nmi_thread_active) {
    NMI_SwitchThread();
  } else {
    zelda_ppu_write(W12SEL, W12SEL_copy);
    zelda_ppu_write(W34SEL, W34SEL_copy);
    zelda_ppu_write(WOBJSEL, WOBJSEL_copy);
    zelda_ppu_write(CGWSEL, CGWSEL_copy);
    zelda_ppu_write(CGADSUB, CGADSUB_copy);
    zelda_ppu_write(COLDATA, COLDATA_copy0);
    zelda_ppu_write(COLDATA, COLDATA_copy1);
    zelda_ppu_write(COLDATA, COLDATA_copy2);
    zelda_ppu_write(TM, TM_copy);
    zelda_ppu_write(TS, TS_copy);
    zelda_ppu_write(TMW, TMW_copy);
    zelda_ppu_write(TSW, TSW_copy);
    zelda_ppu_write(BG1HOFS, BG1HOFS_copy);
    zelda_ppu_write(BG1HOFS, BG1HOFS_copy >> 8);
    zelda_ppu_write(BG1VOFS, BG1VOFS_copy);
    zelda_ppu_write(BG1VOFS, BG1VOFS_copy >> 8);
    zelda_ppu_write(BG2HOFS, BG2HOFS_copy);
    zelda_ppu_write(BG2HOFS, BG2HOFS_copy >> 8);
    zelda_ppu_write(BG2VOFS, BG2VOFS_copy);
    zelda_ppu_write(BG2VOFS, BG2VOFS_copy >> 8);
    zelda_ppu_write(BG3HOFS, BG3HOFS_copy2);
    zelda_ppu_write(BG3HOFS, BG3HOFS_copy2 >> 8);
    zelda_ppu_write(BG3VOFS, BG3VOFS_copy2);
    zelda_ppu_write(BG3VOFS, BG3VOFS_copy2 >> 8);
    zelda_ppu_write(MOSAIC, MOSAIC_copy);
    zelda_ppu_write(BGMODE, BGMODE_copy);
    if ((BGMODE_copy & 7) == 7) {
      zelda_ppu_write(M7B, 0);
      zelda_ppu_write(M7B, 0);
      zelda_ppu_write(M7C, 0);
      zelda_ppu_write(M7C, 0);
      zelda_ppu_write(M7X, M7X_copy);
      zelda_ppu_write(M7X, M7X_copy >> 8);
      zelda_ppu_write(M7Y, M7Y_copy);
      zelda_ppu_write(M7Y, M7Y_copy >> 8);
    }
    //if (irq_flag) {
    //  snes_dummy_read(g_snes, TIMEUP);
    //  snes_dummy_write(g_snes, VTIMEL, 0x80);
    //  snes_dummy_write(g_snes, VTIMEH, 0);
    //  snes_dummy_write(g_snes, HTIMEL, 0);
    //  snes_dummy_write(g_snes, HTIMEH, 0);
    //  snes_dummy_write(g_snes, NMITIMEN, 0xa1);
    //}
    zelda_ppu_write(INIDISP, INIDISP_copy);
    zelda_snes_dummy_write(HDMAEN, HDMAEN_copy);
  }
}

void NMI_SwitchThread() {  // 80822d
  NMI_UpdateIRQGFX();
  //zelda_snes_dummy_write(VTIMEL, virq_trigger);
  //zelda_snes_dummy_write(VTIMEH, 0);
  //zelda_snes_dummy_write(NMITIMEN, 0xa1);
  zelda_ppu_write(W12SEL, W12SEL_copy);
  zelda_ppu_write(W34SEL, W34SEL_copy);
  zelda_ppu_write(WOBJSEL, WOBJSEL_copy);
  zelda_ppu_write(CGWSEL, CGWSEL_copy);
  zelda_ppu_write(CGADSUB, CGADSUB_copy);
  zelda_ppu_write(COLDATA, COLDATA_copy0);
  zelda_ppu_write(COLDATA, COLDATA_copy1);
  zelda_ppu_write(COLDATA, COLDATA_copy2);
  zelda_ppu_write(TM, TM_copy);
  zelda_ppu_write(TS, TS_copy);
  zelda_ppu_write(TMW, TMW_copy);
  zelda_ppu_write(TSW, TSW_copy);
  zelda_ppu_write(BG1HOFS, BG1HOFS_copy);
  zelda_ppu_write(BG1HOFS, BG1HOFS_copy >> 8);
  zelda_ppu_write(BG1VOFS, BG1VOFS_copy);
  zelda_ppu_write(BG1VOFS, BG1VOFS_copy >> 8);
  zelda_ppu_write(BG2HOFS, BG2HOFS_copy);
  zelda_ppu_write(BG2HOFS, BG2HOFS_copy >> 8);
  zelda_ppu_write(BG2VOFS, BG2VOFS_copy);
  zelda_ppu_write(BG2VOFS, BG2VOFS_copy >> 8);
  zelda_ppu_write(BG3HOFS, BG3HOFS_copy2);
  zelda_ppu_write(BG3HOFS, BG3HOFS_copy2 >> 8);
  zelda_ppu_write(BG3VOFS, BG3VOFS_copy2);
  zelda_ppu_write(BG3VOFS, BG3VOFS_copy2 >> 8);
  zelda_ppu_write(INIDISP, INIDISP_copy);
  zelda_snes_dummy_write(HDMAEN, HDMAEN_copy);
  thread_other_stack = (thread_other_stack != 0x1f31) ? 0x1f31 : 0x1f2;
}

void NMI_ReadJoypads(uint16 joypad_input) {  // 8083d1
  uint16 both = joypad_input;
  uint16 reversed = 0;
  for (int i = 0; i < 16; i++, both >>= 1)
    reversed = reversed * 2 + (both & 1);
  uint8 r0 = reversed;
  uint8 r1 = reversed >> 8;

  joypad1L_last = r0;
  filtered_joypad_L = (r0 ^ joypad1L_last2) & r0;
  joypad1L_last2 = r0;

  joypad1H_last = r1;
  filtered_joypad_H = (r1 ^ joypad1H_last2) & r1;
  joypad1H_last2 = r1;
}

void NMI_DoUpdates() {  // 8089e0
  if (!nmi_disable_core_updates) {
    memcpy(&g_zenv.vram[0x4100], &kLinkGraphics[dma_source_addr_0 - 0x8000], 0x40);
    memcpy(&g_zenv.vram[0x4120], &kLinkGraphics[dma_source_addr_1 - 0x8000], 0x40);
    memcpy(&g_zenv.vram[0x4140], &kLinkGraphics[dma_source_addr_2 - 0x8000], 0x20);

    memcpy(&g_zenv.vram[0x4000], &kLinkGraphics[dma_source_addr_3 - 0x8000], 0x40);
    memcpy(&g_zenv.vram[0x4020], &kLinkGraphics[dma_source_addr_4 - 0x8000], 0x40);
    memcpy(&g_zenv.vram[0x4040], &kLinkGraphics[dma_source_addr_5 - 0x8000], 0x20);

    memcpy(&g_zenv.vram[0x4050], &g_ram[dma_source_addr_6], 0x40);
    memcpy(&g_zenv.vram[0x4070], &g_ram[dma_source_addr_7], 0x40);
    memcpy(&g_zenv.vram[0x4090], &g_ram[dma_source_addr_8], 0x40);
    memcpy(&g_zenv.vram[0x40b0], &g_ram[dma_source_addr_9], 0x20);
    memcpy(&g_zenv.vram[0x40c0], &g_ram[dma_source_addr_10], 0x40);
    memcpy(&g_zenv.vram[0x4150], &g_ram[dma_source_addr_11], 0x40);
    memcpy(&g_zenv.vram[0x4170], &g_ram[dma_source_addr_12], 0x40);
    memcpy(&g_zenv.vram[0x4190], &g_ram[dma_source_addr_13], 0x40);
    memcpy(&g_zenv.vram[0x41b0], &g_ram[dma_source_addr_14], 0x20);
    memcpy(&g_zenv.vram[0x41c0], &g_ram[dma_source_addr_15], 0x40);
    memcpy(&g_zenv.vram[0x4200], &g_ram[dma_source_addr_16], 0x40);
    memcpy(&g_zenv.vram[0x4220], &g_ram[dma_source_addr_17], 0x40);
    memcpy(&g_zenv.vram[0x4240], &g_ram[0xbd40], 0x40);
    memcpy(&g_zenv.vram[0x4300], &g_ram[dma_source_addr_18], 0x40);
    memcpy(&g_zenv.vram[0x4320], &g_ram[dma_source_addr_19], 0x40);
    memcpy(&g_zenv.vram[0x4340], &g_ram[0xbd80], 0x40);

    if (BYTE(flag_travel_bird)) {
      memcpy(&g_zenv.vram[0x40e0], &g_ram[dma_source_addr_20], 0x40);
      memcpy(&g_zenv.vram[0x41e0], &g_ram[dma_source_addr_21], 0x40);
    }

    memcpy(&g_zenv.vram[animated_tile_vram_addr], &g_ram[animated_tile_data_src], 0x400);
  }

  if (flag_update_hud_in_nmi) {
    memcpy(&g_zenv.vram[word_7E0219], hud_tile_indices_buffer, 0x14a);
  }

  if (flag_update_cgram_in_nmi) {
    memcpy(g_zenv.ppu->cgram, main_palette_buffer, 0x200);
  }

  flag_update_hud_in_nmi = 0;
  flag_update_cgram_in_nmi = 0;

  memcpy(g_zenv.ppu->oam, &g_ram[0x800], 0x200);
  memcpy(g_zenv.ppu->highOam, &g_ram[0xa00], 0x20);

  if (nmi_load_bg_from_vram) {
    const uint8 *p;
    switch (nmi_load_bg_from_vram) {
    case 1: p = g_ram + 0x1002; break;
    case 2: p = g_ram + 0x1000; break;
    case 3: p = kBgTilemap_0; break;
    case 4: p = g_ram + 0x21b; break;
    case 5: p = kBgTilemap_1; break;
    case 6: p = kBgTilemap_2; break;
    case 7: p = kBgTilemap_3; break;
    case 8: p = kBgTilemap_4; break;
    case 9: p = kBgTilemap_5; break;
    default: assert(0);
    }
    HandleStripes14(p);
    if (nmi_load_bg_from_vram == 1)
      vram_upload_offset = 0;
    nmi_load_bg_from_vram = 0;
  }

  if (nmi_update_tilemap_dst) {
    memcpy(&g_zenv.vram[nmi_update_tilemap_dst * 256], &g_ram[0x10000 + nmi_update_tilemap_src], 0x200);
    nmi_update_tilemap_dst = 0;
  }

  if (nmi_copy_packets_flag) {
    uint8 *p = (uint8 *)uvram.data;
    do {
      int dst = WORD(p[0]);
      int vmain = p[2];
      int len = p[3];
      p += 4;
      if (vmain == 0x80) {
        // plain copy
        memcpy(&g_zenv.vram[dst], p, len);
      } else if (vmain == 0x81) {
        // copy with other increment
        assert((len & 1) == 0);
        uint16 *dp = &g_zenv.vram[dst];
        for (int i = 0; i < len; i += 2, dp += 32)
          *dp = WORD(p[i]);
      } else {
        assert(0);
      }
      p += len;
    } while (WORD(p[0]) != 0xffff);
    nmi_copy_packets_flag = 0;
    nmi_disable_core_updates = 0;
  }

  int idx = nmi_subroutine_index;
  nmi_subroutine_index = 0;
  kNmiSubroutines[idx]();
}

void NMI_UploadTilemap() {  // 808cb0
  memcpy(&g_zenv.vram[kNmiVramAddrs[BYTE(nmi_load_target_addr)] << 8], &g_ram[0x1000], 0x800);

  *(uint16 *)&g_ram[0x1000] = 0;
  nmi_disable_core_updates = 0;
}

void NMI_UploadTilemap_doNothing() {  // 808ce3
}

void NMI_UploadBG3Text() {  // 808ce4
  memcpy(&g_zenv.vram[0x7c00], &g_ram[0x10000], 0x7e0);
  nmi_disable_core_updates = 0;
}

void NMI_UpdateOWScroll() {  // 808d13
  uint8 *src = (uint8 *)uvram.data, *src_org = src;
  int f = WORD(src[0]);
  int step = (f & 0x8000) ? 32 : 1;
  int len = f & 0x3fff;
  src += 2;
  do {
    uint16 *dst = &g_zenv.vram[WORD(src[0])];
    src += 2;
    for (int i = 0, i_end = len >> 1; i < i_end; i++, dst += step, src += 2)
      *dst = WORD(*src);
  } while (!(src[1] & 0x80));
  nmi_disable_core_updates = 0;
}

void NMI_UpdateSubscreenOverlay() {  // 808d62
  NMI_HandleArbitraryTileMap(&g_ram[0x12000], 0, 0x80);
}

void NMI_HandleArbitraryTileMap(const uint8 *src, int i, int i_end) {  // 808dae
  uint16 *r10 = &word_7F4000;
  do {
    memcpy(&g_zenv.vram[r10[i >> 1]], src, 0x80);
    src += 0x80;
  } while ((i += 2) != i_end);
  nmi_disable_core_updates = 0;
}

void NMI_UpdateBG1Wall() {  // 808e09
  // Secret Wall Right
  CopyToVramVertical(nmi_load_target_addr, &g_ram[0xc880], 0x40);
  CopyToVramVertical(nmi_load_target_addr + 0x800, &g_ram[0xc8c0], 0x40);
}

void NMI_TileMapNothing() {  // 808e4b
}

void NMI_UpdateLoadLightWorldMap() {  // 808e54
  static const uint16 kLightWorldTileMapDsts[4] = { 0, 0x20, 0x1000, 0x1020 };
  const uint8 *src = GetLightOverworldTilemap();
  for (int j = 0; j != 4; j++) {
    int t = kLightWorldTileMapDsts[j];
    for (int i = 0x20; i; i--) {
      CopyToVramLow(src, t, 0x20);
      src += 32;
      t += 0x80;
    }
  }
}

void NMI_UpdateBG2Left() {  // 808ea9
  CopyToVram(0, &g_ram[0x10000], 0x800);
  CopyToVram(0x800, &g_ram[0x10800], 0x800);
}

void NMI_UpdateBGChar3and4() {  // 808ee7
  memcpy(&g_zenv.vram[0x2c00], &g_ram[0x10000], 0x1000);
  nmi_disable_core_updates = 0;
}

void NMI_UpdateBGChar5and6() {  // 808f16
  memcpy(&g_zenv.vram[0x3400], &g_ram[0x11000], 0x1000);
  nmi_disable_core_updates = 0;
}

void NMI_UpdateBGCharHalf() {  // 808f45
  memcpy(&g_zenv.vram[BYTE(nmi_load_target_addr) * 256], &g_ram[0x11000], 0x400);
}

void NMI_UpdateBGChar0() {  // 808f72
  NMI_RunTileMapUpdateDMA(0x2000);
}

void NMI_UpdateBGChar1() {  // 808f79
  NMI_RunTileMapUpdateDMA(0x2800);
}

void NMI_UpdateBGChar2() {  // 808f80
  NMI_RunTileMapUpdateDMA(0x3000);
}

void NMI_UpdateBGChar3() {  // 808f87
  NMI_RunTileMapUpdateDMA(0x3800);
}

void NMI_UpdateObjChar0() {  // 808f8e
  CopyToVram(0x4400, &g_ram[0x10000], 0x800);
  nmi_disable_core_updates = 0;
}

void NMI_UpdateObjChar2() {  // 808fbd
  NMI_RunTileMapUpdateDMA(0x5000);
}

void NMI_UpdateObjChar3() {  // 808fc4
  NMI_RunTileMapUpdateDMA(0x5800);
}

void NMI_RunTileMapUpdateDMA(int dst) {  // 808fc9
  CopyToVram(dst, &g_ram[0x10000], 0x1000);
  nmi_disable_core_updates = 0;
}

void NMI_UploadDarkWorldMap() {  // 808ff3
  static const uint16 kLightWorldTileMapSrcs[4] = { 0, 0x20, 0x1000, 0x1020 };
  const uint8 *src = g_ram + 0x1000;
  int t = 0x810;
  for (int i = 0x20; i; i--) {
    CopyToVramLow(src, t, 0x20);
    src += 32;
    t += 0x80;
  }
}

void NMI_UploadGameOverText() {  // 809038
  CopyToVram(0x7800, &g_ram[0x2000], 0x800);
  CopyToVram(0x7d00, &g_ram[0x3400], 0x600);
}

void NMI_UpdatePegTiles() {  // 80908b
  CopyToVram(0x3d00, &g_ram[0x10000], 0x100);
}

void NMI_UpdateStarTiles() {  // 8090b7
  CopyToVram(0x3ed0, &g_ram[0x10000], 0x40);
}

void HandleStripes14(const uint8 *p) {  // 8092a1
  while (!(p[0] & 0x80)) {
    uint16 vmem_addr = swap16(WORD(p[0]));
    uint8 vram_incr_amount = (p[2] & 0x80) >> 7;
    uint8 is_memset = p[2] & 0x40;  // Cpu BUS Address Step  (0=Increment, 2=Decrement, 1/3=Fixed) (DMA only)
    int len = (swap16(WORD(p[2])) & 0x3fff) + 1;
    p += 4;

    if (vram_incr_amount == 0) {
      uint16 *dst = &g_zenv.vram[vmem_addr];
      if (is_memset) {
        uint16 v = p[0] | p[1] << 8;
        len = (len + 1) >> 1;
        for (int i = 0; i < len; i++)
          dst[i] = v;
        p += 2;
      } else {
        memcpy(dst, p, len);
        p += len;
      }
    } else {
      // increment vram by 32 instead of 1
      uint16 *dst = &g_zenv.vram[vmem_addr];
      if (is_memset) {
        uint16 v = p[0] | p[1] << 8;
        len = (len + 1) >> 1;
        for (int i = 0; i < len; i++, dst += 32)
          *dst = v;
        p += 2;
      } else {
        assert((len & 1) == 0);
        len >>= 1;
        for (int i = 0; i < len; i++, dst += 32, p += 2)
          WORD(*dst) = WORD(*p);
      }
    }
  }
}

void NMI_UpdateIRQGFX() {  // 809347
  if (nmi_flag_update_polyhedral) {
    memcpy(&g_zenv.vram[0x5800], &g_ram[0xe800], 0x800);
    nmi_flag_update_polyhedral = 0;
  }
}

