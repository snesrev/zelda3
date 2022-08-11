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



void Graphics_IncrementalVramUpload();
void LoadGfxFunc1();
void PrepTransAuxGfx();
void LoadTransAuxGfx();
void Graphics_MaybeLoadChrHalfSlot();
void Palette_Func1();
void Spotlight_open();
void Spotlight_close();
void ConfigureSpotlightTable();
void ResetSpotlightTable();

void Vram_EraseTilemaps_PalaceMap();
void Vram_EraseTilemaps_normal();
void Vram_EraseTilemaps_Triforce();
void InitTilesets();
void Palette_PalaceMapBg();
void Palette_PalaceMapSpr();
void Palette_Hud();
void PaletteFilterHistory();
void Palette_OverworldBgAux1();
void Palette_OverworldBgAux2();
void Palette_OverworldBgAux3();
void Palette_SpriteAux1();
void Palette_SpriteAux2();
void Palette_SpriteAux3();
void Palette_DungBgMain();
void Palette_ArmorAndGloves();

void Palette_ElectroThemedGear();
void Palette_SpecialOw();
void Palette_SetOwBgColor();
void Palette_MiscSprite_Indoors();
void Intro_LoadPalettes();
void Dungeon_LoadPalettes();
void Palette_MainSpr();
void LoadGearPalettes_bunny();
void Palette_ZeroPalettesAndCopyFirst();
void Palette_MiscSprite();
void Palette_OverworldBgMain();
void Palette_Shield();
void Palette_Sword();
void Palette_UpdateGlovesColor();
void Palette_AnimGetMasterSword();
void Palette_AnimGetMasterSword2();
void Palette_AnimGetMasterSword3();
void Palette_DarkenOrLighten_Step();
void Palette_BgAndFixedColor_Black();
void Palette_SelectScreen();

void Palette_AssertTranslucencySwap();
void Palette_Filter_SP5F();
void Palette_Restore_SP5F();
void Palette_RevertTranslucencySwap();

void Palette_Restore_BG_And_HUD();
void PaletteFilter_WishPonds();
void PaletteFilter_WishPonds_Inner();
void PaletteFilter_Crystal();
void PaletteFilter_Agahnim(int k);
void KholdstareShell_PaletteFiltering();
void Filter_Majorly_Whiten_Bg();
void Filter_MajorWhitenMain();
void Palette_Restore_BG_From_Flash();

void Dungeon_OrangeBlueBarrierUpload(int x, int y);
void Dungeon_OrangeBlueBarrierUpload_A();
void Dungeon_OrangeBlueBarrierUpload_B();
void Dungeon_OrangeBlueBarrierUpload_C();

void NMI_UploadTilemap();
void Text_DecompressStoryGfx();
void Palette_ResetHud45ForText();

void DecodeAnimatedSpriteTile(uint8 a);
void DecodeAnimatedSpriteTile_variable(uint8 a);
void DecompOwAnimatedTiles(uint8 a);
void DecompDungAnimatedTiles(uint8 a);

void DecompAuxAndSprites();

void Palette_InitWhiteFilter();
void Palette_MirrorWarp_Step();
void Tagalong_LoadGfx();
void Sprite_ResetAll();
void Overworld_LoadAreaPalettes();
void Overworld_LoadAreaPalettesEx(uint8 x);
void Overworld_LoadPalettesInner();
void Overworld_LoadPalettes(uint8 bg, uint8 spr);
void Overworld_CgramAuxToMain();
void SetBackdropcolorBlack();

void RestorePaletteAdditive_FadeIn(int from, int to);
void RestorePaletteSubtractive(uint16 from, uint16 to);

void EnableForceBlank();
void PaletteFilter_Restore_Strictly_Bg_Subtractive();
void PaletteFilter_Restore_Strictly_Bg_Additive();

void LoadDefaultGfx();
void DecompSwordGfx();
void DecompShieldGfx();

void Graphics_LoadCommonSpr();
void TriforceRoom_AnimPalette();

void WhirlpoolSaturateBlue();
void WhirlpoolIsolateBlue();
void WhirlpoolRestoreRedGreen();
void WhirlpoolRestoreBlue();

void PaletteFilter_IncreaseTrinexxRed();
void PaletteFilter_RestoreTrinexxRed();
void PaletteFilter_IncreaseTrinexxBlue();
void PaletteFilter_RestoreTrinexxBlue();

void Hdma_ConfigureWaterTable();
void Hdma_ConfigureWaterTable_Inner(uint16 r10);
void Watergate_Helper();

void Palette_AgahnimClones();
void LoadItemAnimationGfx();

void Palette_FadeIntroOneStep();
void Palette_FadeIntro2();
void Attract_InitGraphics_Helper1();

int Decomp_spr(uint8 *dst, int gfx);
void Upload3To4High(const uint8 *decomp_addr);
void CopyFontToVram();

void Player_SetElectrocutionMosaicLevel();
void Player_SetCustomMosaicLevel(uint8 a);
void ResetSpotlightTable();
void LoadActualGearPalettes();
void LoadOverworldMapPalette();