#pragma once

enum {
  kSrmOffs_Gloves = 0x354,
  kSrmOffs_Sword = 0x359,
  kSrmOffs_Shield = 0x35a,
  kSrmOffs_Armor = 0x35b,
  kSrmOffs_DiedCounter = 0x405,
  kSrmOffs_Name = 0x3d9,
  kSrmOffs_Health = 0x36c,
};



void Graphics_IncrementalVRAMUpload();
void LoadNewSpriteGFXSet();
void PrepTransAuxGfx();
void LoadTransAuxGFX();
void Graphics_LoadChrHalfSlot();
void Dungeon_HandleTranslucencyAndPalettes();
void Spotlight_open();
void IrisSpotlight_close();
void IrisSpotlight_ConfigureTable();
void IrisSpotlight_ResetTable();

void EraseTileMaps_dungeonmap();
void EraseTileMaps_normal();
void EraseTileMaps_triforce();
void InitializeTilesets();
void Palette_Load_DungeonMapBG();
void Palette_Load_DungeonMapSprite();
void Palette_Load_HUD();
void PaletteFilterHistory();
void Palette_Load_OWBG1();
void Palette_Load_OWBG2();
void Palette_Load_OWBG3();
void Palette_Load_SpriteAux1();
void Palette_Load_SpriteAux2();
void Palette_Load_SpritePal0Left();
void Palette_Load_DungeonSet();
void Palette_Load_LinkArmorAndGloves();

void Palette_ElectroThemedGear();
void Palette_SpecialOw();
void Palette_SetOwBgColor();
void Palette_Load_SpriteEnvironment_Dungeon();
void Overworld_LoadAllPalettes();
void Dungeon_LoadPalettes();
void Palette_Load_SpriteMain();
void LoadGearPalettes_bunny();
void SpecialOverworld_CopyPalettesToCache();
void Palette_Load_SpriteEnvironment();
void Palette_Load_OWBGMain();
void Palette_Load_Shield();
void Palette_Load_Sword();
void Palette_UpdateGlovesColor();
void Palette_AnimGetMasterSword();
void Palette_AnimGetMasterSword2();
void Palette_AnimGetMasterSword3();
void PaletteFilter_BlindingWhite();
void Palette_BgAndFixedColor_Black();
void Palette_LoadForFileSelect();

void Palette_AssertTranslucencySwap();
void PaletteFilter_SP5F();
void PaletteFilter_RestoreSP5F();
void Palette_RevertTranslucencySwap();

void Palette_Restore_BG_And_HUD();
void PaletteFilter_WishPonds();
void PaletteFilter_WishPonds_Inner();
void PaletteFilter_Crystal();
void AgahnimWarpShadowFilter(int k);
void KholdstareShell_PaletteFiltering();
void Filter_Majorly_Whiten_Bg();
void HandleScreenFlash();
void Palette_Restore_BG_From_Flash();

void Dungeon_UpdatePegGFXBuffer(int x, int y);
void Module07_16_UpdatePegs_Step1();
void Module07_16_UpdatePegs_Step2();
void Module07_16_UpdatePegs_Step3();

void NMI_UploadTilemap();
void Attract_DecompressStoryGFX();
void ResetHUDPalettes4and5();

void WriteTo4BPPBuffer_at_7F4000(uint8 a);
void DecodeAnimatedSpriteTile_variable(uint8 a);
void DecompressAnimatedOverworldTiles(uint8 a);
void DecompressAnimatedDungeonTiles(uint8 a);

void ReloadPreviouslyLoadedSheets();

void PaletteFilter_InitializeWhiteFilter();
void MirrorWarp_RunAnimationSubmodules();
void LoadFollowerGraphics();
void Sprite_ResetAll();
void Overworld_LoadAreaPalettes();
void Overworld_LoadAreaPalettesEx(uint8 x);
void Overworld_LoadPalettesInner();
void Overworld_LoadPalettes(uint8 bg, uint8 spr);
void Overworld_CopyPalettesToCache();
void SetBackdropcolorBlack();

void PaletteFilter_RestoreAdditive(int from, int to);
void PaletteFilter_RestoreSubtractive(uint16 from, uint16 to);

void EnableForceBlank();
void PaletteFilter_RestoreBGSubstractiveStrict();
void PaletteFilter_RestoreBGAdditiveStrict();

void LoadDefaultGraphics();
void DecompressSwordGraphics();
void DecompressShieldGraphics();

void LoadCommonSprites_2();
void PaletteFilter_BlindingWhiteTriforce();

void PaletteFilter_WhirlpoolBlue();
void PaletteFilter_IsolateWhirlpoolBlue();
void PaletteFilter_WhirlpoolRestoreRedGreen();
void PaletteFilter_WhirlpoolRestoreBlue();

void Trinexx_FlashShellPalette_Red();
void Trinexx_UnflashShellPalette_Red();
void Trinexx_FlashShellPalette_Blue();
void Trinexx_UnflashShellPalette_Blue();

void AdjustWaterHDMAWindow();
void AdjustWaterHDMAWindow_X(uint16 r10);
void FloodDam_PrepFloodHDMA();

void Palette_LoadAgahnim();
void LoadItemGFXIntoWRAM4BPPBuffer();

void Palette_FadeIntroOneStep();
void Palette_FadeIntro2();
void Attract_LoadBG3GFX();

int Decomp_spr(uint8 *dst, int gfx);
void Do3To4High(const uint8 *decomp_addr);
void TransferFontToVRAM();

void LinkZap_HandleMosaic();
void Player_SetCustomMosaicLevel(uint8 a);
void IrisSpotlight_ResetTable();
void LoadActualGearPalettes();
void LoadOverworldMapPalette();