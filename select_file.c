#include "zelda_rtl.h"
#include "variables.h"
#include "load_gfx.h"
#include "select_file.h"
#include "snes/snes_regs.h"
#include "overworld.h"
#include "messaging.h"

#define selectfile_R16 g_ram[0xc8]
#define selectfile_R17 g_ram[0xc9]
#define selectfile_R18 WORD(g_ram[0xca])
#define selectfile_R20 WORD(g_ram[0xcc])
static const uint8 kSelectFile_Draw_Y[3] = {0x43, 0x63, 0x83};
bool Intro_CheckCksum(const uint8 *s) {
  const uint16 *src = (const uint16 *)s;
  uint16 sum = 0;
  for (int i = 0; i < 0x280; i++)
    sum += src[i];
  return sum == 0x5a5a;

}

uint16 *SelectFile_Func1() {
  static const uint16 kSelectFile_Func1_Tab[4] = {0x3581, 0x3582, 0x3591, 0x3592};
  uint16 *dst = (uint16 *)&g_ram[0x1002];
  *dst++ = 0x10;
  *dst++ = 0xff07;
  for (int i = 0; i < 1024; i++)
    *dst++ = kSelectFile_Func1_Tab[((i & 0x20) >> 4) + (i & 1)];
  return dst;
}

void SelectFile_Func5_DrawOams(int k) {
  static const uint8 kSelectFile_Draw_OamIdx[3] = {0x28, 0x3c, 0x50};
  static const uint8 kSelectFile_Draw_SwordChar[4] = {0x85, 0xa1, 0xa1, 0xa1};
  static const uint8 kSelectFile_Draw_ShieldChar[3] = {0xc4, 0xca, 0xe0};
  static const uint8 kSelectFile_Draw_Flags[3] = {0x72, 0x76, 0x7a};
  static const uint8 kSelectFile_Draw_Flags2[3] = {0x32, 0x36, 0x3a};
  static const uint8 kSelectFile_Draw_Flags3[3] = {0x30, 0x34, 0x38};

  link_dma_graphics_index = 0x116 * 2;
  uint8 *sram = g_zenv.sram + 0x500 * k;

  OamEnt *oam = oam_buf + kSelectFile_Draw_OamIdx[k] / 4;
  uint8 x = 0x34;
  uint8 y = kSelectFile_Draw_Y[k];

  oam[0].x = oam[1].x = x + 0xc;
  oam[0].y = y - 5;
  oam[1].y = y + 3;
  oam[0].flags = oam[1].flags = kSelectFile_Draw_Flags[k];
  uint8 sword = sram[kSrmOffs_Sword] - 1;
  if (sign8(sword))
    oam[1].y = oam[0].y = 0xf0, sword = 0;
  oam[0].charnum = kSelectFile_Draw_SwordChar[sword];
  oam[1].charnum = oam[0].charnum + 16;
  bytewise_extended_oam[oam - oam_buf] = bytewise_extended_oam[oam - oam_buf + 1] = 0;

  oam += 2;
  oam[0].x = x - 5;
  oam[0].y = y + 10;
  uint8 shield = sram[kSrmOffs_Shield] - 1;
  if (sign8(shield))
    oam[0].y = 0xf0, shield = 0;
  oam[0].charnum = kSelectFile_Draw_ShieldChar[shield];
  oam[0].flags = kSelectFile_Draw_Flags2[k];
  bytewise_extended_oam[oam - oam_buf] = 2;
  oam++;

  oam[0].x = oam[1].x = x;
  oam[0].y = y;
  oam[1].y = y + 8;
  oam[0].charnum = 0;
  oam[1].charnum = 2;
  oam[0].flags = kSelectFile_Draw_Flags3[k];
  oam[1].flags = oam[0].flags | 0x40;
  bytewise_extended_oam[oam - oam_buf] = bytewise_extended_oam[oam - oam_buf + 1] = 2;
}

void SelectFile_Func6_DrawOams2(int k) {
  static const uint8 kSelectFile_DrawDigit_Char[10] = {0xd0, 0xac, 0xad, 0xbc, 0xbd, 0xae, 0xaf, 0xbe, 0xbf, 0xc0};
  static const int8 kSelectFile_DrawDigit_OamIdx[3] = {4, 16, 28};
  static const int8 kSelectFile_DrawDigit_X[3] = {12, 4, -4};

  uint8 *sram = g_zenv.sram + 0x500 * k;
  uint8 x = 0x34;
  uint8 y = kSelectFile_Draw_Y[k];

  int died_ctr = WORD(sram[kSrmOffs_DiedCounter]);
  if (died_ctr == 0xffff)
    return;

  if (died_ctr > 999)
    died_ctr = 999;

  uint8 digits[3];
  digits[2] = died_ctr / 100;
  died_ctr %= 100;
  digits[1] = died_ctr / 10;
  digits[0] = died_ctr % 10;

  int i = (digits[2] != 0) ? 2 : (digits[1] != 0) ? 1 : 0;
  OamEnt *oam = oam_buf + kSelectFile_DrawDigit_OamIdx[k] / 4;
  do {
    oam->charnum = kSelectFile_DrawDigit_Char[digits[i]];
    oam->x = x + kSelectFile_DrawDigit_X[i];
    oam->y = y + 0x10;
    oam->flags = 0x3c;
    bytewise_extended_oam[oam - oam_buf] = 0;
  } while (oam++, --i >= 0);
}

void SelectFile_Func17(int k) {
  static const uint16 kSelectFile_DrawName_VramOffs[3] = {8, 0x5c, 0xb0};
  static const uint16 kSelectFile_DrawName_HealthVramOffs[3] = {0x16, 0x6a, 0xbe};
  uint8 *sram = g_zenv.sram + 0x500 * k;
  uint16 *name = (uint16 *)(sram + kSrmOffs_Name);
  uint16 *dst = vram_upload_data + kSelectFile_DrawName_VramOffs[k] / 2;
  for (int i = 5; i >= 0; i--) {
    uint16 t = *name++ + 0x1800;
    dst[0] = t;
    dst[21] = t + 0x10;
    dst++;
  }
  int health = sram[kSrmOffs_Health] >> 3;
  dst = vram_upload_data + kSelectFile_DrawName_HealthVramOffs[k] / 2;
  uint16 *dst_org = dst;
  int row = 10;
  do {
    *dst++ = 0x520;
    if (--row == 0)
      dst = dst_org + 21;
  } while (--health);
}

void SelectFile_Func16() {
  static const uint8 kSelectFile_Func16_FaerieY[2] = {175, 191};
  FileSelect_DrawFairy(0x1c, kSelectFile_Func16_FaerieY[selectfile_R16]);

  int k = selectfile_R16;
  if (filtered_joypad_H & 0x2c) {
    k += (filtered_joypad_H & 0x24) ? 1 : -1;
    selectfile_R16 = k & 1;
    sound_effect_2 = 0x20;
  }

  uint8 a = (filtered_joypad_L & 0xc0 | filtered_joypad_H) & 0xd0;
  if (a != 0) {
    sound_effect_1 = 0x2c;
    if (selectfile_R16 == 0) {
      sound_effect_2 = 0x22;
      sound_effect_1 = 0x0;
      int k = subsubmodule_index;
      selectfile_arr1[k] = 0;
      memset(g_zenv.sram + k * 0x500, 0, 0x500);
      memset(g_zenv.sram + k * 0x500 + 0xf00, 0, 0x500);
      ZeldaWriteSram();
    }
    ReturnToFileSelect();
    subsubmodule_index = 0;
  }
}

void Module_NamePlayer_1() {
  uint16 *dst = SelectFile_Func1();
  dst[0] = 0xffff;
  nmi_load_bg_from_vram = 1;
  submodule_index++;
}

void Module_NamePlayer_2() {
  nmi_load_bg_from_vram = 5;
  submodule_index++;
  INIDISP_copy = 15;
  nmi_disable_core_updates = 0;
}

void Intro_FixCksum(uint8 *s) {
  uint16 *src = (uint16 *)s;
  uint16 sum = 0;
  for (int i = 0; i < 0x27f; i++)
    sum += src[i];
  src[0x27f] = 0x5a5a - sum;
}

void LoadFileSelectGraphics() {  // 80e4e9
  zelda_ppu_write(OBSEL, 2);
  zelda_ppu_write(VMAIN, 0x80);
  zelda_ppu_write_word(VMADDL, 0x5000);

  Decomp_spr(&g_ram[0x14000], 0x5e);
  Do3To4High(&g_ram[0x14000]);

  Decomp_spr(&g_ram[0x14000], 0x5f);
  Do3To4High(&g_ram[0x14000]);

  zelda_ppu_write_word(VMADDL, 0x7000);

  const uint16 *src = GetFontPtr();
  for (int i = 0; i < 0x800; i++)
    zelda_ppu_write_word(VMDATAL, *src++);

  Decomp_spr(&g_ram[0x14000], 0x6b);
  src = (const uint16 *)&g_ram[0x14000];
  for (int i = 0; i < 0x300; i++)
    zelda_ppu_write_word(VMDATAL, *src++);
}

void Intro_ValidateSram() {  // 828054
  uint8 *cart = g_zenv.sram;
  for (int i = 0; i < 3; i++) {
    uint8 *c = cart + i * 0x500;
    if (!Intro_CheckCksum(c)) {
      if (Intro_CheckCksum(c + 0xf00)) {
        memcpy(c, c + 0xf00, 0x500);
      } else {
        memset(c, 0, 0x500);
        memset(c + 0xf00, 0, 0x500);
      }
    }
  }
  memset(&g_ram[0xd00], 0, 256 * 3);
}

void Module01_FileSelect() {  // 8ccd7d
  BG3HOFS_copy2 = 0;
  BG3VOFS_copy2 = 0;
  switch (submodule_index) {
  case 0: Module_SelectFile_0(); break;
  case 1: FileSelect_ReInitSaveFlagsAndEraseTriforce(); break;
  case 2: Module_EraseFile_1(); break;
  case 3: FileSelect_TriggerStripesAndAdvance(); break;
  case 4: FileSelect_TriggerNameStripesAndAdvance(); break;
  case 5: FileSelect_Main(); break;
  }
}

void Module_SelectFile_0() {  // 8ccd9d
  EnableForceBlank();
  is_nmi_thread_active = 0;
  nmi_flag_update_polyhedral = 0;
  music_control = 11;
  submodule_index++;
  overworld_palette_aux_or_main = 0x200;
  dung_hdr_palette_1 = 6;
  nmi_disable_core_updates = 6;
  Palette_Load_DungeonSet();
  Palette_Load_OWBG3();
  hud_palette = 0;
  Palette_Load_HUD();
  hud_cur_item = 0;
  misc_sprites_graphics_index = 1;
  main_tile_theme_index = 35;
  aux_tile_theme_index = 81;
  LoadDefaultGraphics();
  InitializeTilesets();
  LoadFileSelectGraphics();
  Intro_ValidateSram();
  DecompressEnemyDamageSubclasses();
}

void FileSelect_ReInitSaveFlagsAndEraseTriforce() {  // 8ccdf2
  memset(selectfile_arr1, 0, 6);
  FileSelect_EraseTriforce();
}

void FileSelect_EraseTriforce() {  // 8ccdf9
  nmi_disable_core_updates = 128;
  EnableForceBlank();
  EraseTileMaps_triforce();
  Palette_LoadForFileSelect();
  flag_update_cgram_in_nmi++;
  submodule_index++;
}

void Module_EraseFile_1() {  // 8cce53
  static const uint8 kSelectFile_Gfx0[224] = {
    0x10, 0x42,    0, 0x27, 0x89, 0x35, 0x8a, 0x35, 0x8b, 0x35, 0x8c, 0x35, 0x8b, 0x35, 0x8c, 0x35,
    0x8b, 0x35, 0x8c, 0x35, 0x8b, 0x35, 0x8c, 0x35, 0x8b, 0x35, 0x8c, 0x35, 0x8b, 0x35, 0x8c, 0x35,
    0x8b, 0x35, 0x8c, 0x35, 0x8b, 0x35, 0x8c, 0x35, 0x8a, 0x75, 0x89, 0x75, 0x10, 0x62,    0,    3,
    0x99, 0x35, 0x9a, 0x35, 0x10, 0x64, 0x40, 0x1e, 0x7f, 0x34, 0x10, 0x74,    0,    3, 0x9a, 0x75,
    0x99, 0x75, 0x10, 0x82,    0,    3, 0xa9, 0x35, 0xaa, 0x35, 0x10, 0x84, 0x40, 0x1e, 0x7f, 0x34,
    0x10, 0x94,    0,    3, 0xaa, 0x75, 0xa9, 0x75, 0x10, 0xa2,    0, 0x27, 0x9d, 0x35, 0xad, 0x35,
    0x9b, 0x35, 0x9c, 0x35, 0x9b, 0x35, 0x9c, 0x35, 0x9b, 0x35, 0x9c, 0x35, 0x9b, 0x35, 0x9c, 0x35,
    0x9b, 0x35, 0x9c, 0x35, 0x9b, 0x35, 0x9c, 0x35, 0x9b, 0x35, 0x9c, 0x35, 0x9b, 0x35, 0x9c, 0x35,
    0xad, 0x75, 0x9d, 0x75, 0x10, 0xc2,    0, 0x27, 0xab, 0x35, 0xac, 0x35, 0xab, 0x35, 0xac, 0x35,
    0xab, 0x35, 0xac, 0x35, 0xab, 0x35, 0xac, 0x35, 0xab, 0x35, 0xac, 0x35, 0xab, 0x35, 0xac, 0x35,
    0xab, 0x35, 0xac, 0x35, 0xab, 0x35, 0xac, 0x35, 0xab, 0x35, 0xac, 0x35, 0xab, 0x75, 0xac, 0x75,
    0x10, 0xe2,    0,    1, 0x83, 0x35, 0x10, 0xe3, 0x40, 0x32, 0x85, 0x35, 0x10, 0xfd,    0,    1,
    0x84, 0x35, 0x11,    2, 0xc0, 0x22, 0x86, 0x35, 0x11, 0x1d, 0xc0, 0x22, 0x96, 0x35, 0x13, 0x42,
       0,    1, 0x93, 0x35, 0x13, 0x43, 0x40, 0x32, 0x95, 0x35, 0x13, 0x5d,    0,    1, 0x94, 0x35,
  };
  uint16 *dst = SelectFile_Func1();
  memcpy(dst, kSelectFile_Gfx0, 224);
  dst += 224 / 2;
  uint16 t = 0x1103;
  for (int i = 17; i >= 0; i--) {
    *dst++ = swap16(t);
    t += 0x20;
    *dst++ = 0x3240;
    *dst++ = 0x347f;
  }
  *(uint8 *)dst = 0xff;
  submodule_index++;
  nmi_load_bg_from_vram = 1;
}

void FileSelect_TriggerStripesAndAdvance() {  // 8ccea5
  selectfile_R16 = selectfile_var2;
  submodule_index++;
  nmi_load_bg_from_vram = 6;
}

void FileSelect_TriggerNameStripesAndAdvance() {  // 8cceb1
  static const uint8 kSelectFile_Func3_Data[253] = {
    0x61, 0x29,    0, 0x25, 0xe7, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18,
    0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18,
    0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0x61, 0x49,    0, 0x25, 0xf7, 0x18,
    0x91, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18,
    0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18,
    0xa9, 0x18, 0xa9, 0x18, 0x61, 0xa9,    0, 0x25, 0xe8, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18,
    0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18,
    0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0x61, 0xc9,
       0, 0x25, 0xf8, 0x18, 0x91, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18,
    0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18,
    0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0x62, 0x29,    0, 0x25, 0xe9, 0x18, 0xa9, 0x18,
    0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18,
    0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18,
    0xa9, 0x18, 0x62, 0x49,    0, 0x25, 0xf9, 0x18, 0x91, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18,
    0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18,
    0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xff,
  };
  memcpy(vram_upload_data, kSelectFile_Func3_Data, 253);
  INIDISP_copy = 0xf;
  nmi_disable_core_updates = 0;
  submodule_index++;
  nmi_load_bg_from_vram = 6;
}

void FileSelect_Main() {  // 8ccebd
  static const uint8 kSelectFile_Faerie_Y[5] = {0x4a, 0x6a, 0x8a, 0xaf, 0xbf};

  const uint8 *cart = g_zenv.sram;

  if (selectfile_R16 < 3)
    selectfile_var2 = selectfile_R16;

  for (int k = 0; k < 3; k++) {
    if (*(uint16 *)(cart + k * 0x500 + 0x3E5) == 0x55AA) {
      selectfile_arr1[k] = 1;
      SelectFile_Func5_DrawOams(k);
      SelectFile_Func6_DrawOams2(k);
      SelectFile_Func17(k);
    }
  }

  FileSelect_DrawFairy(0x1c, kSelectFile_Faerie_Y[selectfile_R16]);
  nmi_load_bg_from_vram = 1;

  uint8 a = (filtered_joypad_L & 0xc0 | filtered_joypad_H) & 0xfc;
  if (a & 0x2c) {
    if (a & 8) {
      sound_effect_2 = 0x20;
      if (sign8(--selectfile_R16))
        selectfile_R16 = 4;
    } else {
      sound_effect_2 = 0x20;
      if (++selectfile_R16 == 5)
        selectfile_R16 = 0;
    }
  } else if (a != 0) {
    sound_effect_1 = 0x2c;
    if (selectfile_R16 < 3) {
      selectfile_R17 = 0;
      if (!selectfile_arr1[selectfile_R16]) {
        main_module_index = 4;
        submodule_index = 0;
        subsubmodule_index = 0;
      } else {
        music_control = 0xf1;
        srm_var1 = selectfile_R16 * 2 + 2;
        WORD(g_ram[0]) = selectfile_R16 * 0x500;
        CopySaveToWRAM();
      }
    } else if (selectfile_arr1[0] | selectfile_arr1[1] | selectfile_arr1[2]) {
      main_module_index = (selectfile_R16 == 3) ? 2 : 3;
      selectfile_R16 = 0;
      submodule_index = 0;
      subsubmodule_index = 0;
    } else {
      sound_effect_1 = 0x3c;
    }
  }
}

void Module02_CopyFile() {  // 8cd053
  selectfile_var2 = 0;
  switch (submodule_index) {
  case 0: FileSelect_EraseTriforce(); break;
  case 1: Module_EraseFile_1(); break;
  case 2: Module_CopyFile_2(); break;
  case 3: CopyFile_ChooseSelection(); break;
  case 4: CopyFile_ChooseTarget(); break;
  case 5: CopyFile_ConfirmSelection(); break;
  }
}

void Module_CopyFile_2() {  // 8cd06e
  nmi_load_bg_from_vram = 7;
  submodule_index++;
  INIDISP_copy = 0xf;
  nmi_disable_core_updates = 0;
  int i = 0;
  for (; selectfile_arr1[i] == 0; i++) {}
  selectfile_R16 = i;
}

void CopyFile_ChooseSelection() {  // 8cd087
  CopyFile_SelectionAndBlinker();
  if (submodule_index == 3 && !(frame_counter & 0x30))
    FilePicker_DeleteHeaderStripe();
  nmi_load_bg_from_vram = 1;
}

void CopyFile_ChooseTarget() {  // 8cd0a2
  CopyFile_TargetSelectionAndBlink();
  if (submodule_index == 4 && !(frame_counter & 0x30))
    FilePicker_DeleteHeaderStripe();
  nmi_load_bg_from_vram = 1;
}

void CopyFile_ConfirmSelection() {  // 8cd0b9
  CopyFile_HandleConfirmation();
  nmi_load_bg_from_vram = 1;
}

void FilePicker_DeleteHeaderStripe() {  // 8cd0c6
  static const uint16 kFilePicker_DeleteHeaderStripe_Dst[2] = {4, 0x1e};
  for (int j = 1; j >= 0; j--) {
    uint16 *dst = vram_upload_data + kFilePicker_DeleteHeaderStripe_Dst[j] / 2;
    for (int i = 0; i != 11; i++)
      dst[i] = 0xa9;
  }
}

void CopyFile_SelectionAndBlinker() {  // 8cd13f
  static const uint8 kCopyFile_SelectionAndBlinker_Tab[173] = {
    0x61,    4,    0, 0x15, 0x85, 0x18, 0x26, 0x18,    7, 0x18, 0xaf, 0x18,    2, 0x18,    7, 0x18,
    0x6f, 0x18, 0x86, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0x61, 0x24,    0, 0x15, 0x95, 0x18,
    0x36, 0x18, 0x17, 0x18, 0xbf, 0x18, 0x12, 0x18, 0x17, 0x18, 0x7f, 0x18, 0x96, 0x18, 0xa9, 0x18,
    0xa9, 0x18, 0xa9, 0x18, 0x61, 0x67,    0,  0xf, 0xe7, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18,
    0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0x61, 0x87,    0,  0xf, 0xf7, 0x18, 0x91, 0x18,
    0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0x61, 0xc7,    0,  0xf,
    0xe8, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18,
    0x61, 0xe7,    0,  0xf, 0xf8, 0x18, 0x91, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18,
    0xa9, 0x18, 0xa9, 0x18, 0x62, 0x27,    0,  0xf, 0xe9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18,
    0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0x62, 0x47,    0,  0xf, 0xf9, 0x18, 0x91, 0x18,
    0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xff,
  };
  static const uint8 kCopyFile_SelectionAndBlinker_Tab1[73] = {
    0x61, 0x67, 0x40,  0xe, 0xa9,    0, 0x61, 0x87, 0x40,  0xe, 0xa9,    0, 0x61, 0xc7, 0x40,  0xe,
    0xa9,    0, 0x61, 0xe7, 0x40,  0xe, 0xa9,    0, 0x11, 0x30,    0,    1, 0x83, 0x35, 0x11, 0x31,
    0x40, 0x14, 0x85, 0x35, 0x11, 0x3c,    0,    1, 0x84, 0x35, 0x11, 0x50, 0xc0,  0xe, 0x86, 0x35,
    0x11, 0x5c, 0xc0,  0xe, 0x96, 0x35, 0x12, 0x50,    0,    1, 0x93, 0x35, 0x12, 0x51, 0x40, 0x14,
    0x95, 0x35, 0x12, 0x5c,    0,    1, 0x94, 0x35, 0xff,
  };
  static const uint16 kCopyFile_SelectionAndBlinker_Dst[3] = {0x3c, 0x64, 0x8c};
  static const uint8 kCopyFile_SelectionAndBlinker_FaerieX[4] = {36, 36, 36, 28};
  static const uint8 kCopyFile_SelectionAndBlinker_FaerieY[4] = {87, 111, 135, 191};

  vram_upload_offset = 0xac;
  memcpy(vram_upload_data, kCopyFile_SelectionAndBlinker_Tab, 173);

  for (int k = 0; k != 3; k++) {
    if (selectfile_arr1[k] & 1) {
      const uint16 *name = (uint16 *)(g_zenv.sram + 0x500 * k + kSrmOffs_Name);
      uint16 *dst = vram_upload_data + kCopyFile_SelectionAndBlinker_Dst[k] / 2;
      for (int i = 0; i != 6; i++) {
        uint16 t = *name++ + 0x1800;
        dst[0] = t;
        dst[10] = t + 0x10;
        dst++;
      }
    }
  }
  FileSelect_DrawFairy(kCopyFile_SelectionAndBlinker_FaerieX[selectfile_R16], kCopyFile_SelectionAndBlinker_FaerieY[selectfile_R16]);

  uint8 a = (filtered_joypad_L & 0xc0 | filtered_joypad_H) & 0xfc;
  if (a & 0x2c) {
    uint8 k = selectfile_R16;
    if (a & 8) {
      do {
        if (--k < 0) {
          k = 3;
          break;
        }
      } while (!selectfile_arr1[k]);
    } else {
      do {
        k++;
        if (k >= 4)
          k = 0;
      } while (k != 3 && !selectfile_arr1[k]);
    }
    selectfile_R16 = k;
    sound_effect_2 = 0x20;
  } else if (a != 0) {
    sound_effect_1 = 0x2c;
    if (selectfile_R16 == 3) {
      ReturnToFileSelect();
      return;
    }
    selectfile_R20 = selectfile_R16 * 2;
    memcpy(vram_upload_data + 26, kCopyFile_SelectionAndBlinker_Tab1, 73);
    if (selectfile_R16 != 2) {
      uint16 *dst = vram_upload_data + selectfile_R16 * 6;
      dst[26] = 0x2762;
      dst[29] = 0x4762;
    }
    submodule_index++;
    selectfile_R16 = 0;
  }
}

void ReturnToFileSelect() {  // 8cd22d
  main_module_index = 1;
  submodule_index = 1;
  subsubmodule_index = 0;
  selectfile_R16 = 0;
}

void CopyFile_TargetSelectionAndBlink() {  // 8cd27b
  {
    int k = 1, t = 4;
    do {
      if (t != selectfile_R20)
        selectfile_arr2[k--] = t;
    } while ((t -= 2) >= 0);
  }

  static const uint8 kCopyFile_TargetSelectionAndBlink_Tab0[133] = {
    0x61, 0x51,    0, 0x15, 0x85, 0x18, 0x23, 0x18,  0xe, 0x18, 0xa9, 0x18, 0x26, 0x18,    7, 0x18,
    0xaf, 0x18,    2, 0x18,    7, 0x18, 0x6f, 0x18, 0x86, 0x18, 0x61, 0x71,    0, 0x15, 0x95, 0x18,
    0x33, 0x18, 0x1e, 0x18, 0xb9, 0x18, 0x36, 0x18, 0x17, 0x18, 0xbf, 0x18, 0x12, 0x18, 0x17, 0x18,
    0x7f, 0x18, 0x96, 0x18, 0x61, 0xb4,    0,  0xf, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18,
    0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0x61, 0xd4,    0,  0xf, 0xa9, 0x18, 0x91, 0x18,
    0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0x62, 0x14,    0,  0xf,
    0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18,
    0x62, 0x34,    0,  0xf, 0xa9, 0x18, 0x91, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18,
    0xa9, 0x18, 0xa9, 0x18, 0xff,
  };
  static const uint8 kCopyFile_TargetSelectionAndBlink_Tab2[49] = {
    0x61, 0xb4, 0x40,  0xe, 0xa9,    0, 0x61, 0xd4, 0x40,  0xe, 0xa9,    0, 0x62, 0xc6,    0,  0xd,
       2, 0x18,  0xe, 0x18,  0xf, 0x18, 0x28, 0x18, 0xa9, 0x18,  0xe, 0x18,  0xa, 0x18, 0x62, 0xe6,
       0,  0xd, 0x12, 0x18, 0x1e, 0x18, 0x1f, 0x18, 0x38, 0x18, 0xa9, 0x18, 0x1e, 0x18, 0x1a, 0x18,
    0xff,
  };
  static const uint8 kCopyFile_TargetSelectionAndBlink_FaerieX[3] = {0x8c, 0x8c, 0x1c};
  static const uint8 kCopyFile_TargetSelectionAndBlink_FaerieY[3] = {0x67, 0x7f, 0xbf};
  static const uint16 kCopyFile_TargetSelectionAndBlink_Dst[2] = {0x38, 0x60};
  static const uint16 kCopyFile_TargetSelectionAndBlink_Tab1[3] = {0x18e7, 0x18e8, 0x18e9};
  memcpy(vram_upload_data, kCopyFile_TargetSelectionAndBlink_Tab0, 133);

  for (int k = 0, j = 0; k != 3; k++) {
    if (k * 2 == selectfile_R20)
      continue;

    uint16 *dst = vram_upload_data + kCopyFile_TargetSelectionAndBlink_Dst[j++] / 2;
    uint16 t = kCopyFile_TargetSelectionAndBlink_Tab1[k];
    dst[0] = t;
    dst[10] = t + 0x10;
    dst += 2;
    if (selectfile_arr1[k]) {
      const uint16 *name = (uint16 *)(g_zenv.sram + 0x500 * k + kSrmOffs_Name);
      for (int i = 0; i != 6; i++) {
        uint16 t = *name++ + 0x1800;
        dst[0] = t;
        dst[10] = t + 0x10;
        dst++;
      }
    }
  }

  vram_upload_offset = 132;

  FileSelect_DrawFairy(kCopyFile_TargetSelectionAndBlink_FaerieX[selectfile_R16], kCopyFile_TargetSelectionAndBlink_FaerieY[selectfile_R16]);

  uint8 a = (filtered_joypad_L & 0xc0 | filtered_joypad_H) & 0xfc;
  if (a & 0x2c) {
    uint8 k = selectfile_R16;
    if (a & 8) {
      if (sign8(--k))
        k = 2;
    } else {
      if (++k >= 3)
        k = 0;
    }
    selectfile_R16 = k;
    sound_effect_2 = 0x20;
  } else if (a) {
    sound_effect_1 = 0x2c;
    if (selectfile_R16 == 2) {
      ReturnToFileSelect();
      selectfile_R16 = 0;
      return;
    }
    selectfile_R18 = selectfile_arr2[selectfile_R16];
    memcpy(vram_upload_data + 26, kCopyFile_TargetSelectionAndBlink_Tab2, 49);
    if (selectfile_R16 == 0) {
      uint16 *dst = vram_upload_data;
      dst[26] = 0x1462;
      dst[29] = 0x3462;
    }
    submodule_index++;
    selectfile_R16 = 0;
  }
}

void CopyFile_HandleConfirmation() {  // 8cd371
  static const uint8 kCopyFile_HandleConfirmation_FaerieY[2] = {0xaf, 0xbf};
  FileSelect_DrawFairy(0x1c, kCopyFile_HandleConfirmation_FaerieY[selectfile_R16]);

  uint8 a = (filtered_joypad_L & 0xc0 | filtered_joypad_H) & 0xfc;
  if (a & 0x2c) {
    sound_effect_2 = 0x20;
    if (a & 0x24) {
      if (++selectfile_R16 >= 2)
        selectfile_R16 = 0;
    } else {
      if (sign8(--selectfile_R16))
        selectfile_R16 = 1;
    }
  } else if (a != 0) {
    sound_effect_1 = 0x2c;
    if (selectfile_R16 == 0) {
      memcpy(g_zenv.sram + (selectfile_R18 >> 1) * 0x500, g_zenv.sram + (selectfile_R20 >> 1) * 0x500, 0x500);
      selectfile_arr1[(selectfile_R18 >> 1)] = 1;
      ZeldaWriteSram();
    }
    ReturnToFileSelect();
    selectfile_R16 = 0;
  }
}

void Module03_KILLFile() {  // 8cd485
  switch (submodule_index) {
  case 0: FileSelect_EraseTriforce(); break;
  case 1: Module_EraseFile_1(); break;
  case 2: KILLFile_SetUp(); break;
  case 3: KILLFile_HandleSelection(); break;
  case 4: KILLFile_HandleConfirmation(); break;
  }
}

void KILLFile_SetUp() {  // 8cd49a
  nmi_load_bg_from_vram = 8;
  submodule_index++;
  INIDISP_copy = 0xf;
  nmi_disable_core_updates = 0;
  int i = 0;
  for (; selectfile_arr1[i] == 0; i++) {}
  selectfile_R16 = i;
}

void KILLFile_HandleSelection() {  // 8cd49f
  if (selectfile_R16 < 3)
    selectfile_var2 = selectfile_R16;
  KILLFile_ChooseTarget();
  nmi_load_bg_from_vram = 1;
}

void KILLFile_HandleConfirmation() {  // 8cd4b1
  SelectFile_Func16();
  nmi_load_bg_from_vram = 1;
}

void KILLFile_ChooseTarget() {  // 8cd4ba
  static const uint8 kKILLFile_ChooseTarget_Tab[253] = {
    0x61, 0xa7,    0, 0x25, 0xe7, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18,
    0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18,
    0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0x61, 0xc7,    0, 0x25, 0xf7, 0x18,
    0x91, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18,
    0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18,
    0xa9, 0x18, 0xa9, 0x18, 0x62,    7,    0, 0x25, 0xe8, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18,
    0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18,
    0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0x62, 0x27,
       0, 0x25, 0xf8, 0x18, 0x91, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18,
    0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18,
    0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0x62, 0x67,    0, 0x25, 0xe9, 0x18, 0xa9, 0x18,
    0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18,
    0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18,
    0xa9, 0x18, 0x62, 0x87,    0, 0x25, 0xf9, 0x18, 0x91, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18,
    0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18,
    0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xa9, 0x18, 0xff,
  };
  static const uint8 kKILLFile_ChooseTarget_Tab2[101] = {
    0x61, 0xa7, 0x40, 0x24, 0xa9,    0, 0x61, 0xc7, 0x40, 0x24, 0xa9,    0, 0x62,    7, 0x40, 0x24,
    0xa9,    0, 0x62, 0x27, 0x40, 0x24, 0xa9,    0, 0x62, 0xc6,    0, 0x21,    4, 0x18, 0x21, 0x18,
       0, 0x18, 0x22, 0x18,    4, 0x18, 0xa9, 0x18, 0x23, 0x18,    7, 0x18, 0xaf, 0x18, 0x22, 0x18,
    0xa9, 0x18,  0xf, 0x18,  0xb, 0x18,    0, 0x18, 0x28, 0x18,    4, 0x18, 0x21, 0x18, 0x62, 0xe6,
       0, 0x21, 0x14, 0x18, 0x31, 0x18, 0x10, 0x18, 0x32, 0x18, 0x14, 0x18, 0xa9, 0x18, 0x33, 0x18,
    0x17, 0x18, 0xbf, 0x18, 0x32, 0x18, 0xa9, 0x18, 0x1f, 0x18, 0x1b, 0x18, 0x10, 0x18, 0x38, 0x18,
    0x14, 0x18, 0x31, 0x18, 0xff,
  };
  static const uint8 kKILLFile_ChooseTarget_FaerieX[4] = {36, 36, 36, 28};
  static const uint8 kKILLFile_ChooseTarget_FaerieY[4] = {103, 127, 151, 191};
  memcpy(vram_upload_data, kKILLFile_ChooseTarget_Tab, 253);
  for (int k = 0; k < 3; k++) {
    if (selectfile_arr1[k])
      SelectFile_Func17(k);
  }

  FileSelect_DrawFairy(kKILLFile_ChooseTarget_FaerieX[selectfile_R16], kKILLFile_ChooseTarget_FaerieY[selectfile_R16]);

  int k = selectfile_R16;
  if (filtered_joypad_H & 0x2c) {
    if (!(filtered_joypad_H & 0x24)) {
      do {
        if (--k < 0) {
          k = 3;
          break;
        }
      } while (!selectfile_arr1[k]);
    } else {
      do {
        k++;
        if (k >= 4)
          k = 0;
      } while (k != 3 && !selectfile_arr1[k]);
    }
    sound_effect_2 = 0x20;
  }
  selectfile_R16 = k;

  uint8 a = (filtered_joypad_L & 0xc0 | filtered_joypad_H) & 0xd0;
  if (a) {
    sound_effect_1 = 0x2c;
    if (k == 3) {
      ReturnToFileSelect();
      return;
    }

    memcpy(vram_upload_data, kKILLFile_ChooseTarget_Tab2, 101);
    submodule_index++;
    if (selectfile_R16 != 2) {
      uint16 *dst = vram_upload_data + selectfile_R16 * 6;
      dst[0] = 0x6762;
      dst[3] = 0x8762;
    }
    subsubmodule_index = selectfile_R16;
    selectfile_R16 = 0;
  }
}

void FileSelect_DrawFairy(uint8 x, uint8 y) {  // 8cd7a5
  oam_buf[0].x = x;
  oam_buf[0].y = y;
  oam_buf[0].charnum = frame_counter & 8 ? 0xaa : 0xa8;
  oam_buf[0].flags = 0x7e;
  bytewise_extended_oam[0] = 2;
}

void Module04_NameFile() {  // 8cd88a
  switch (submodule_index) {
  case 0: NameFile_EraseSave(); break;
  case 1: Module_NamePlayer_1(); break;
  case 2: Module_NamePlayer_2(); break;
  case 3: NameFile_DoTheNaming(); break;
  }
}

void NameFile_EraseSave() {  // 8cd89c
  FileSelect_EraseTriforce();
  irq_flag = 1;
  selectfile_var3 = 0;
  selectfile_var4 = 0;
  selectfile_var5 = 0;
  selectfile_arr2[0] = 0;
  selectfile_var6 = 0;
  selectfile_var7 = 0x83;
  selectfile_var8 = 0x1f0;
  BG3HOFS_copy2 = 0;
  int offs = selectfile_R16 * 0x500;
  attract_legend_ctr = offs;
  memset(g_zenv.sram + offs, 0, 0x500);
  uint16 *name = (uint16 *)(g_zenv.sram + offs + kSrmOffs_Name);
  name[0] = name[1] = name[2] = name[3] = name[4] = name[5] = 0xa9;
}

void NameFile_DoTheNaming() {  // 8cda4d
  static const int16 kNamePlayer_Tab1[26] = {
    -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1,
    -2, 2, -2, 2, -2, 2, -2, 2, -4, 4,
  };
  static const uint8 kNamePlayer_Tab2[4] = {131, 147, 163, 179};
  static const int8 kNamePlayer_X[6] = {31, 47, 63, 79, 95, 111};
  static const int16 kNamePlayer_Tab0[32] = {
    0x1f0,     0,  0x10,  0x20,  0x30,  0x40,  0x50,  0x60,  0x70,  0x80,  0x90,  0xa0,  0xb0,  0xc0,  0xd0,  0xe0,
     0xf0, 0x100, 0x110, 0x120, 0x130, 0x140, 0x150, 0x160, 0x170, 0x180, 0x190, 0x1a0, 0x1b0, 0x1c0, 0x1d0, 0x1e0,
  };
  static const int8 kNamePlayer_Tab3[128] = {
       6,    7, 0x5f,    9, 0x59, 0x59, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x60, 0x23,
    0x59, 0x59, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x59, 0x59, 0x59,    0,    1,    2,    3,    4,    5,
    0x10, 0x11, 0x12, 0x13, 0x59, 0x59, 0x24, 0x5f, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d,
    0x59, 0x59, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f, 0x59, 0x59, 0x59,  0xa,  0xb,  0xc,  0xd,  0xe,  0xf,
    0x40, 0x41, 0x42, 0x59, 0x59, 0x59, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x40, 0x41, 0x42, 0x59,
    0x59, 0x59, 0x61, 0x3f, 0x45, 0x46, 0x59, 0x59, 0x59, 0x59, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
    0x44, 0x59, 0x6f, 0x6f, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x5a, 0x44, 0x59, 0x6f, 0x6f,
    0x59, 0x59, 0x5a, 0x44, 0x59, 0x6f, 0x6f, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x59, 0x5a,
  };
  for (;;) {
    int j = selectfile_var9;
    if (j == 0) {
      NameFile_CheckForScrollInputX();
      break;
    }
    if (j != 0x31)
      selectfile_var9 += 4;
    j--;
    if (kNamePlayer_Tab0[selectfile_var3] == selectfile_var8) {
      selectfile_var9 = (joypad1H_last & 3) ? 0x30 : 0;
      NameFile_CheckForScrollInputX();
      continue;
    }
    if (!selectfile_var10)
      j += 2;
    selectfile_var8 = (selectfile_var8 + WORD(((uint8*)&kNamePlayer_Tab1)[j])) & 0x1ff;
    break;
  }

  for (;;) {
    if (selectfile_var11 == 0) {
      NameFile_CheckForScrollInputY();
      break;
    }
    uint8 diff = selectfile_var7 - kNamePlayer_Tab2[selectfile_var5];
    if (diff != 0) {
      selectfile_var7 += sign8(diff) ? 2 : -2;
      break;
    }
    selectfile_var11 = 0;
    NameFile_CheckForScrollInputY();
  }

  OamEnt *oam = oam_buf;
  for (int i = 0; i != 26; i++) {
    oam->x = 0x18 + i * 8;
    oam->y = selectfile_var7;
    oam->charnum = 0x2e;
    oam->flags = 0x3c;
    bytewise_extended_oam[oam - oam_buf] = 0;
    oam++;
  }

  oam->x = kNamePlayer_X[selectfile_var4];
  oam->y = 0x58;
  oam->charnum = 0x29;
  oam->flags = 0xc;
  bytewise_extended_oam[oam - oam_buf] = 0;

  if (selectfile_var9 | selectfile_var11)
    return;

  if (!(filtered_joypad_H & 0x10)) {
    if (!(filtered_joypad_H & 0xc0 || filtered_joypad_L & 0xc0))
      return;

    sound_effect_1 = 0x2b;
    uint8 t = kNamePlayer_Tab3[selectfile_var3 + selectfile_var5 * 0x20];
    if (t == 0x5a) {
      if (!selectfile_var4)
        selectfile_var4 = 5;
      else
        selectfile_var4--;
      return;
    } else if (t == 0x44) {
      if (++selectfile_var4 == 6)
        selectfile_var4 = 0;
      return;
    } else if (t != 0x6f) {
      int p = selectfile_var4 * 2 + attract_legend_ctr;
      uint16 chr = (t & 0xfff0) * 2 + (t & 0xf);
      WORD(g_zenv.sram[p + kSrmOffs_Name]) = chr;
      NameFile_DrawSelectedCharacter(selectfile_var4, chr);
      if (++selectfile_var4 == 6)
        selectfile_var4 = 0;
      return;
    }
  }
  int i = 0;
  for(;;) {
    uint16 a = WORD(g_zenv.sram[i * 2 + attract_legend_ctr + kSrmOffs_Name]);
    if (a != 0xa9)
      break;
    if (++i == 6) {
      sound_effect_1 = 0x3c;
      return;
    }
  }
  srm_var1 = selectfile_R16 * 2 + 2;
  uint8 *sram = &g_zenv.sram[selectfile_R16 * 0x500];
  WORD(sram[0x3e5]) = 0x55aa;
  WORD(sram[0x20c]) = 0xf000;
  WORD(sram[0x20e]) = 0xf000;
  WORD(sram[kSrmOffs_DiedCounter]) = 0xffff;
  static const uint8 kSramInit_Normal[60] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0,    0, 0, 0,    0,    0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0,    0, 0, 0,    0,    0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0,    0, 0, 0, 0x18, 0x18, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0xf8, 0, 0,
  };
  memcpy(sram + 0x340, kSramInit_Normal, 60);
  Intro_FixCksum(sram);
  ZeldaWriteSram();
  ReturnToFileSelect();
  irq_flag = 0xff;
  sound_effect_1 = 0x2c;
}

void NameFile_CheckForScrollInputX() {  // 8cdc8c
  static const uint16 kNameFile_CheckForScrollInputX_Add[2] = {1, 0xff};
  static const int16 kNameFile_CheckForScrollInputX_Cmp[2] = {0x20, 0xff};
  static const int16 kNameFile_CheckForScrollInputX_Set[2] = {0, 0x1f};
  if (joypad1H_last & 3) {
    int k = (joypad1H_last & 3) - 1;
    selectfile_var10 = k;
    selectfile_var9++;
    uint8 t = selectfile_var3 + kNameFile_CheckForScrollInputX_Add[k];
    if (t == kNameFile_CheckForScrollInputX_Cmp[k])
      t = kNameFile_CheckForScrollInputX_Set[k];
    selectfile_var3 = t;
  }
}

void NameFile_CheckForScrollInputY() {  // 8cdcbf
  static const int8 kNameFile_CheckForScrollInputY_Add[2] = {1, -1};
  static const int8 kNameFile_CheckForScrollInputY_Cmp[2] = {4, -1};
  static const int8 kNameFile_CheckForScrollInputY_Set[2] = {0, 3};

  uint8 a = joypad1H_last & 0xc;
  if (a) {
    if ((a * 2 | selectfile_var5) == 0x10 || (a * 4 | selectfile_var5) == 0x13) {
      selectfile_arr2[1] = a;
      return;
    }
     a >>= 2;
    int t = selectfile_var5 + kNameFile_CheckForScrollInputY_Add[a-1];
    if (t == kNameFile_CheckForScrollInputY_Cmp[a-1])
      t = kNameFile_CheckForScrollInputY_Set[a-1];
    selectfile_var5 = t;

    selectfile_var11++;
    selectfile_arr2[1] = a;

  } else {
    selectfile_arr2[0] = 0;
  }
}

void NameFile_DrawSelectedCharacter(int k, uint16 chr) {  // 8cdd30
  static const uint16 kNameFile_DrawSelectedCharacter_Tab[6] = {0x84, 0x86, 0x88, 0x8a, 0x8c, 0x8e};
  uint16 *dst = vram_upload_data;
  uint16 a = kNameFile_DrawSelectedCharacter_Tab[k] | 0x6100;
  dst[0] = swap16(a);
  dst[1] = 0x100;
  dst[2] = 0x1800 | chr;
  dst[3] = swap16(a + 0x20);
  dst[4] = 0x100;
  dst[5] = (0x1800 | chr) + 0x10;
  BYTE(dst[6]) = 0xff;
  nmi_load_bg_from_vram = 1;
}

