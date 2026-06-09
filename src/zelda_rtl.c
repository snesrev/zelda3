#include "zelda_rtl.h"
#include "variables.h"
#include "misc.h"
#include "nmi.h"
#include "poly.h"
#include "attract.h"
#include "snes/ppu.h"
#include "snes/snes_regs.h"
#include "snes/dma.h"
#include "spc_player.h"
#include "util.h"
#include "audio.h"
#include "assets.h"
#ifdef ZELDA3_MULTIPLAYER
#include "player.h"
#include "player_oam.h"
#include "sprite.h"
#include "net_transport.h"
#endif
ZeldaEnv g_zenv;
uint8 g_ram[131072];

uint32 g_wanted_zelda_features;

static void Startup_InitializeMemory();

typedef struct SimpleHdma {
  const uint8 *table;
  const uint8 *indir_ptr;
  uint8 rep_count;
  uint8 mode;
  uint8 ppu_addr;
  uint8 indir_bank;
} SimpleHdma;
static void SimpleHdma_Init(SimpleHdma *c, DmaChannel *dc);
static void SimpleHdma_DoLine(SimpleHdma *c);

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

void zelda_ppu_write(uint32_t adr, uint8_t val) {
  assert(adr >= INIDISP && adr <= STAT78);
  ppu_write(g_zenv.ppu, (uint8)adr, val);
}

void zelda_ppu_write_word(uint32_t adr, uint16_t val) {
  zelda_ppu_write(adr, val);
  zelda_ppu_write(adr + 1, val >> 8);
}

static const uint8 *SimpleHdma_GetPtr(uint32 p) {
  switch (p) {

  case 0xCFA87: return kAttractDmaTable0;
  case 0xCFA94: return kAttractDmaTable1;
  case 0xebd53: return kHdmaTableForEnding;
  case 0x0F2FB: return kSpotlightIndirectHdma;
  case 0xabdcf: return kMapModeHdma0;             // mode7
  case 0xabdd6: return kMapModeHdma1;             // mode7
  case 0xABDDD: return kAttractIndirectHdmaTab;   // mode7
  case 0x2c80c: return kHdmaTableForPrayingScene;

  case 0x1b00: return (uint8 *)hdma_table_dynamic;
  case 0x1be0: return (uint8 *)hdma_table_dynamic + 0xe0;
  case 0x1bf0: return (uint8 *)hdma_table_dynamic + 0xf0;
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

static void SimpleHdma_Init(SimpleHdma *c, DmaChannel *dc) {
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

static void SimpleHdma_DoLine(SimpleHdma *c) {
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

static void ConfigurePpuSideSpace() {
  // Let PPU impl know about the maximum allowed extra space on the sides and bottom
  int extra_right = 0, extra_left = 0, extra_bottom = 0;
//  printf("main %d, sub %d  (%d, %d, %d)\n", main_module_index, submodule_index, BG2HOFS_copy2, room_bounds_x.v[2 | (quadrant_fullsize_x >> 1)], quadrant_fullsize_x >> 1);
  int mod = main_module_index;
  if (mod == 14)
    mod = saved_module_for_menu;
  if (mod == 9) {
    if (main_module_index == 14 && submodule_index == 7 && overworld_map_state >= 4) {
      // World map
      extra_left = kPpuExtraLeftRight, extra_right = kPpuExtraLeftRight;
      extra_bottom = 16;
    } else {
      // outdoors
      extra_left = BG2HOFS_copy2 - ow_scroll_vars0.xstart;
      extra_right = ow_scroll_vars0.xend - BG2HOFS_copy2;
      extra_bottom = ow_scroll_vars0.yend - BG2VOFS_copy2;
    }
  } else if (mod == 7) {
    // indoors, except when the light cone is in use
    if (!(hdr_dungeon_dark_with_lantern && TS_copy != 0)) {
      int qm = quadrant_fullsize_x >> 1;
      extra_left = IntMax(BG2HOFS_copy2 - room_bounds_x.v[qm], 0);
      extra_right = IntMax(room_bounds_x.v[qm + 2] - BG2HOFS_copy2, 0);
    }

    int qy = quadrant_fullsize_y >> 1;
    extra_bottom = IntMax(room_bounds_y.v[qy + 2] - BG2VOFS_copy2, 0);
  } else if (mod == 20 || mod == 0 || mod == 1) {
    extra_left = kPpuExtraLeftRight, extra_right = kPpuExtraLeftRight;
    extra_bottom = 16;
  }
  PpuSetExtraSideSpace(g_zenv.ppu, extra_left, extra_right, extra_bottom);
}

void ZeldaDrawPpuFrame(uint8 *pixel_buffer, size_t pitch, uint32 render_flags) {
  SimpleHdma hdma_chans[2];

  PpuBeginDrawing(g_zenv.ppu, pixel_buffer, pitch, render_flags);

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
      PpuSetMode7PerspectiveCorrection(g_zenv.ppu, hdma_table_dynamic[0], hdma_table_dynamic[223]);
    else
      PpuSetMode7PerspectiveCorrection(g_zenv.ppu, 0, 0);
  }

  if (g_zenv.ppu->extraLeftRight != 0 || render_flags & kPpuRenderFlags_Height240)
    ConfigurePpuSideSpace();

  int height = render_flags & kPpuRenderFlags_Height240 ? 240 : 224;

  for (int i = 0; i <= height; i++) {
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

static void ZeldaInitializationCode() {
  zelda_snes_dummy_write(NMITIMEN, 0);
  zelda_snes_dummy_write(HDMAEN, 0);
  zelda_snes_dummy_write(MDMAEN, 0);

  Sound_LoadIntroSongBank();

  Startup_InitializeMemory();

  animated_tile_data_src = 0xa680;
  dma_source_addr_9 = 0xb280;
  dma_source_addr_14 = 0xb280 + 0x60;
  zelda_snes_dummy_write(NMITIMEN, 0x81);
}

static void ClearOamBuffer() {  // 80841e
  for (int i = 0; i < 128; i++)
    oam_buf[i].y = 0xf0;
}

static void ZeldaRunGameLoop() {
  frame_counter++;
  ClearOamBuffer();
  Module_MainRouting();
  NMI_PrepareSprites();
  nmi_boolean = 0;
}

#ifdef ZELDA3_MULTIPLAYER
static bool g_mp_initialized = false;
static uint16 g_p2_input_this_frame;
// Master co-op toggle: when false, the second player is never spawned/updated and
// the game runs as ordinary single-player through the same loop. Default on.
bool g_mp_p2_enabled = true;
static void Multiplayer_UpdateCamera(void);
static void Multiplayer_WarpP2OnTransition(void);
static void Multiplayer_PlaceP2Beside(PlayerState *p1, PlayerState *p2);
static void Multiplayer_SeparatePlayers(void);

// Process P2's NMI input (writes to per-player joypad globals via cur_player macros)
static void Multiplayer_ProcessP2Input(uint16 joypad_input) {
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

// Initialize P2 when first entering gameplay (module 7 = dungeon, 9 = overworld)
static bool g_p1_seeded = false;
static void Multiplayer_InitIfNeeded(void) {
  // Seed P1 across the whole load + gameplay span (5=LoadFile, 6=PreDungeon,
  // 7=Dungeon, 8/10=OverworldLoad, 9/11=Overworld), NOT just gameplay (7/9).
  // CopySaveToWRAM memcpys the SRAM save (inventory at 0xF340+) straight into
  // g_ram and sets main_module_index=5, so by module 5 the loaded inventory is
  // already present. Seeding here captures it BEFORE the spawn-position setup in
  // modules 6/8 (which writes Link's x/y through the macros into the now-seeded,
  // authoritative struct). Seeding only at 7/9 was too late: the per-frame
  // SyncToRam (gated below) would otherwise have wiped the inventory first.
  if (main_module_index < 5 || main_module_index > 11) return;

  // One-time: seed P1's PlayerState struct from the live g_ram (which holds the
  // loaded save / boot state). After this the struct is authoritative for P1's
  // link state and is shadowed back into g_ram via SyncToRam each frame. This
  // must happen whether or not co-op is enabled, otherwise the empty struct
  // would be synced over g_ram and wipe the game state.
  if (!g_p1_seeded) {
    PlayerState_SetCurrent(0);
    PlayerState_SyncFromRam();
    // Clear any co-op downed state carried in the struct. is_dead/is_ghost/etc.
    // are co-op-only fields (NOT g_ram-backed), so SyncFromRam won't reset them;
    // without this, re-seeding after a game-over+Continue would leave P1 a frozen
    // 0-HP ghost. (P2 is re-cloned fresh by PlayerState_Init below.)
    g_players[0].is_dead = 0;
    g_players[0].is_ghost = 0;
    g_players[0].respawn_timer = 0;
    g_players[0].disable_sprite_damage = 0;
    flag_is_link_immobilized = 0;   // macro -> cur_player (P1) here; field name collides
    g_p1_seeded = true;
  }

  // Spawn P2 from P1's state once, only when co-op is enabled.
  if (g_mp_p2_enabled && !g_mp_initialized) {
    PlayerState_Init(1);
    g_mp_initialized = true;
    printf("Multiplayer: P2 initialized at (%d, %d)\n",
           g_players[1].x_coord, g_players[1].y_coord);
  }
}

// Called when a save is (re)loaded into WRAM (file-select load, or the post-death
// Continue path) — both route through CopySaveToWRAM. Force a re-seed of P1 from
// the freshly loaded g_ram and a re-spawn of P2, so the loaded save's inventory/
// position isn't overwritten by stale struct state and P2 matches the loaded
// file. The re-seed also clears leftover co-op downed state, which fixes the
// game-over -> Continue soft-lock (both players stuck as frozen 0-HP ghosts).
void Multiplayer_OnSaveLoaded(void) {
  g_p1_seeded = false;
  g_mp_initialized = false;
}

// Per-player health/magic refill for P2. The engine's Hud_RefillLogic only runs
// for P1 (and also redraws the HUD), so we replicate just the per-player part
// here — converting P2's just-collected hearts/magic fillers into P2's actual
// health/magic. cur_player must be P2.
static void Multiplayer_RefillP2(void) {
  if (link_magic_filler) {
    if (link_magic_power >= 128) { link_magic_power = 128; link_magic_filler = 0; }
    else { link_magic_filler--; link_magic_power++; }
  }
  if (link_hearts_filler) {
    if (link_health_current < link_health_capacity) {
      link_health_current += 8;
      if (link_health_current >= link_health_capacity)
        link_health_current = link_health_capacity;
      link_hearts_filler -= 8;
    } else {
      link_health_current = link_health_capacity;
      link_hearts_filler = 0;
    }
  }
}

// Run the multiplayer game loop: P1 full update, then P2 Link_Main only
static void ZeldaRunGameLoop_Multiplayer(uint16 p2_input) {
  frame_counter++;
  ClearOamBuffer();

  // Ensure P2 is initialized when gameplay starts
  Multiplayer_InitIfNeeded();

  // === P1 update (full game loop) ===
  PlayerState_SetCurrent(0);
  // Only shadow the struct into g_ram once P1 has been seeded from the loaded
  // save. Before seeding, the engine is the sole owner of g_ram (the SRAM->WRAM
  // inventory load, Link_Initialize, Dungeon_LoadEntrance); writing the still
  // empty P1 struct back over it would wipe the just-loaded inventory.
  if (g_p1_seeded)
    PlayerState_SyncToRam();

  Module_MainRouting();
  NMI_PrepareSprites();

  // NOTE: do NOT SyncFromRam here. In the multiplayer build the link_ macros
  // resolve to cur_player->field (the PlayerState struct), so Module_MainRouting
  // already wrote P1's new position/state into g_players[0]. Copying g_ram back
  // over the struct would clobber that movement with the stale pre-frame link
  // bytes (this is what froze both players). The struct is authoritative for
  // link state; the SyncToRam above only shadows it into g_ram for the few
  // subsystems that still read raw g_ram at link offsets.

  // === P2 update (Link_Main only — world state already updated) ===
  // Only update P2 during NORMAL free movement: gameplay module (7=dungeon,
  // 9=overworld) AND submodule_index == 0. When submodule_index != 0 a screen
  // transition / special state is in progress: P1's Module_MainRouting drives
  // the scroll and moves Link itself, so P2 must NOT move independently (it
  // would wander out of the transitioning screen and desync the scroll). P2 is
  // frozen during the transition and snapped back to P1 when it completes (see
  // Multiplayer_WarpP2OnTransition below).
  // Capture P1's scripted-control-lock flag while cur_player is still P1 (the
  // link_ macros resolve to P1 here; we can't read it once we switch to P2,
  // and the field name collides with the macro so g_players[0].<field> won't
  // compile). Used below to freeze P2 during P1-driven cutscenes.
  // A dead/ghost P1 sets its own immobilize flag (to stay frozen), but that must
  // NOT freeze P2 — P2 needs to move to revive P1. Only mirror P1's lock to P2
  // for real cutscenes (P1 alive and immobilized).
  uint8 p1_immobilized = flag_is_link_immobilized && !g_players[0].is_dead;

  bool p2_normal_play = (main_module_index == 7 || main_module_index == 9) &&
                        submodule_index == 0;
  if (g_mp_p2_enabled && g_mp_initialized && g_players[1].is_active && p2_normal_play) {
    PlayerState_SetCurrent(1);
    // Give P2 the current shared inventory before it acts (so it can use the
    // team's items and its pickups add to the team total).
    Multiplayer_ShareInventory(&g_players[0], &g_players[1]);
    PlayerState_SyncToRam();

    // Co-op cutscene / scripted control lock. flag_is_link_immobilized is the
    // engine's authoritative "Link can't move this frame" gate (player.c:
    // Link_Main skips Link_ControlHandler when it's set) and is now per-player.
    // P1's scripted sequences that stay in normal gameplay (item-get overhead
    // pose, Ether/Bombos/Quake spell animations, tree pull, scripted dungeon
    // events) set only P1's copy, so without this P2 would keep walking/acting
    // through the cutscene. Module-level cutscenes, the menu/map and text boxes
    // are module 14 (or submodule != 0) and already freeze P2 via the gate
    // above; this covers the in-module case by feeding P2 no input while P1 is
    // locked, so both players are held together.
    uint16 p2_eff_input = p1_immobilized ? 0 : p2_input;

    // Process P2 input into the per-player joypad globals
    Multiplayer_ProcessP2Input(p2_eff_input);

    // P2's Link_Main can hit a hazard (drowning / pit damage / pit fall) whose
    // handler sets the SHARED main_module_index/submodule_index to a recovery or
    // transition state. That state machine then runs in P1's module pass against
    // P1 — yanking P1 to its safe-return spot and stranding P2. Co-op: a P2
    // hazard must affect only P2. Snapshot the shared transition state; if P2
    // changed it, revert it and resolve the hazard locally for P2 below.
    uint8 mp_hazard_module = main_module_index;
    uint8 mp_hazard_submodule = submodule_index;

    Link_Main();

    if (main_module_index != mp_hazard_module || submodule_index != mp_hazard_submodule) {
      main_module_index = mp_hazard_module;
      submodule_index = mp_hazard_submodule;
      if (link_health_current == 0) {
        // Hazard damage was lethal to P2 -> drop P2 into the ghost/respawn state
        // (cur_player == P2 here). On a true double-KO let game-over run.
        if (!Multiplayer_PreventGameOver()) { main_module_index = 18; submodule_index = 0; }
      } else {
        // P2 survived -> pull it back beside P1 (out of the water/pit) and clear
        // any half-started fall/swim handler state.
        Multiplayer_PlaceP2Beside(&g_players[0], &g_players[1]);
      }
    }

    // Render P2's sprite. Use the OAM band OPPOSITE P1's so the two Links never
    // share slots (the player offset table is {0x190, 0xe0}; LinkOam_Main selects
    // it from sort_sprites_setting). LinkOam_Main also sets player_oam_y_offset,
    // which Sprite_CheckDamageFromLink relies on below, so it must run first.
    uint8 p1_oam_setting = (uint8)sort_sprites_setting;
    sort_sprites_setting = p1_oam_setting ? 0 : 1;
    LinkOam_Main();
    sort_sprites_setting = p1_oam_setting;

    // Real two-way combat for P2. Sprites already ran their AI against P1 during
    // Module_MainRouting; re-run the engine's actual hitbox checks with
    // cur_player == P2 so enemies can damage P2 (full recoil / shield / sfx) and
    // P2's sword and items can damage enemies. No friendly fire: players never
    // appear in the sprite arrays, so neither player's weapons can hit the other.
    for (int k = 0; k < 16; k++) {
      if (sprite_state[k] == 9) {
        // Wallmaster (0x90) grabs Link and warps them to the dungeon entrance,
        // but its carry/warp runs during P1's pass against P1's state. If it
        // latched onto P2 here, it would teleport the WRONG player (P1) and kick
        // off a global room reload from inside this P2 update -> desync. Keep the
        // wallmaster P1-only by skipping its grab check for P2 (it still grabs P1
        // via its own AI). P2's weapons can still hit it (CheckDamageFromLink).
        if (sprite_type[k] != 0x90)
          Sprite_CheckDamageToLink(k);
        Sprite_CheckDamageFromLink(k);
        // Let P2 collect drops too (prize sprites are types 0xD8..0xE6). With
        // cur_player == P2 this routes hearts/magic into P2's own pools (so P2
        // heals/refills) and shared items into P2's inventory copy. The sprite
        // is despawned on collect, so P1's pass next frame won't double-collect.
        if (sprite_type[k] >= 0xd8 && sprite_type[k] <= 0xe6)
          Sprite_CheckAbsorptionByPlayer(k);
      }
    }

    // Turn P2's just-collected hearts/magic into actual health/magic (P1's
    // Hud_RefillLogic doesn't run for P2).
    Multiplayer_RefillP2();

    // Push any shared-inventory changes P2 just made (pickups/item use) back to
    // the canonical pool so P1 sees them next frame.
    Multiplayer_ShareInventory(&g_players[1], &g_players[0]);

    // Do NOT SyncFromRam: Link_Main wrote P2's new state into g_players[1] via
    // the cur_player macros, so the struct is already current. Pulling g_ram
    // back would clobber P2's movement (same bug as P1 above).

    // Restore P1 as current so g_ram + cur_player reflect P1 for the subsequent
    // Interrupt_NMI (joypad read) and ZeldaDrawPpuFrame.
    PlayerState_SetCurrent(0);
    PlayerState_SyncToRam();

    // === Shared camera: center between both players ===
    Multiplayer_UpdateCamera();
  }

  // Per-frame co-op bookkeeping (runs even during transitions / when a player is
  // downed): tick respawn timers + revive, and warp P2 to P1 after a transition.
  if (g_mp_initialized) {
    // After a real double-KO + Continue (or bottle-fairy revival), the engine
    // returns to gameplay but those paths bypass CopySaveToWRAM (hence
    // Multiplayer_OnSaveLoaded), so BOTH players are still flagged is_dead here.
    // UpdateDeathRespawn's "both down -> stay down" rule would then keep them
    // frozen 0-HP ghosts forever. Detect that (both dead, but back in a normal
    // gameplay module rather than the game-over module) and re-seed: next frame
    // InitIfNeeded clears P1's downed state and re-clones P2, so both come back.
    // (Both-dead can't legitimately persist in module 7/9 — a fatal double-KO
    // switches to the game-over module the same frame.)
    if (g_players[0].is_dead && g_players[1].is_dead &&
        (main_module_index == 7 || main_module_index == 9)) {
      Multiplayer_OnSaveLoaded();
    } else {
      Multiplayer_UpdateDeathRespawn();
      Multiplayer_WarpP2OnTransition();
      Multiplayer_SeparatePlayers();
    }
  }

  nmi_boolean = 0;
}

// (P2 HUD intentionally deferred for this slice: the previous heart-OAM draw
// used guessed tile/palette numbers. P2 health is tracked per-player and will
// get a proper HUD in a later pass.)

// (Death/respawn is handled symmetrically for both players by
// Multiplayer_PreventGameOver + Multiplayer_UpdateDeathRespawn in player_state.c.)

// Snap P2 to P1 when a screen transition COMPLETES (or the module changes), so
// P2 ends up beside P1 in the new room/area. Warping on completion (rather than
// at the start) is what lets P2 follow P1 through doors, screen-edge scrolls and
// dungeon entrances: during the transition P2 is frozen (see the p2_normal_play
// gate above), and the engine has just moved P1 to the destination, so copying
// P1's fresh position + room/level state to P2 places it correctly.
// Place P2 right beside P1 and clear ALL transient per-frame state, so P2
// resumes cleanly at P1's location. Used by the transition warp and to recover
// P2 from a hazard. Resetting the handler/aux/z/deep-water/incap state matters
// when P2 was mid-action (swimming, recoiling, falling) at the instant it's
// relocated; otherwise it would resume that action on incompatible terrain.
static void Multiplayer_PlaceP2Beside(PlayerState *p1, PlayerState *p2) {
  p2->x_coord = p1->x_coord + 16;
  p2->y_coord = p1->y_coord;
  p2->is_on_lower_level = p1->is_on_lower_level;
  p2->is_on_lower_level_mirror = p1->is_on_lower_level_mirror;  // keep OAM floor priority correct
  p2->quadrant_x = p1->quadrant_x;
  p2->quadrant_y = p1->quadrant_y;
  p2->x_vel = p2->y_vel = 0;
  p2->flag_moving = 0;
  p2->player_handler_state = 0;
  p2->auxiliary_state = 0;
  p2->z_coord = 0;
  p2->is_in_deep_water = 0;
  p2->incapacitated_timer = 0;
  p2->visibility_status = 0;
}

static uint8 g_last_module_index = 0;
static uint8 g_last_submodule_index = 0;
static void Multiplayer_WarpP2OnTransition(void) {
  uint8 cur_module = main_module_index;
  uint8 cur_submodule = submodule_index;

  bool warp = false;
  // Module changed (e.g. overworld<->dungeon entrance/exit): always re-place P2.
  if (cur_module != g_last_module_index)
    warp = true;
  // Same-module transition just finished: submodule returned to 0 (normal play)
  // from a nonzero transition state, in dungeon or overworld.
  if ((cur_module == 7 || cur_module == 9) &&
      cur_submodule == 0 && g_last_submodule_index != 0)
    warp = true;

  if (warp && g_mp_p2_enabled && g_players[1].is_active) {
    PlayerState *p1 = &g_players[0];
    PlayerState *p2 = &g_players[1];
    Multiplayer_PlaceP2Beside(p1, p2);
    // FUTURE: Multiplayer_HandleIndependentTransition() — load a second room
    // for split-screen instead of warping P2 to P1.
  }

  g_last_module_index = cur_module;
  g_last_submodule_index = cur_submodule;
}

// Shared-screen camera. The engine's camera/scroll already follows P1 (it reads
// link_x/y_coord, which resolve to g_players[0] during P1's Module_MainRouting).
// Rather than fight that by writing the BG scroll registers afterwards (the old
// version accumulated shifts into BG1/BG2 H/V OFS every frame, which both
// corrupted the display AND broke P1's own movement, since the engine couples
// Link's walking to the camera-scroll boundary), we simply keep P2 leashed
// inside P1's viewport so both Links stay on screen.
// FUTURE: a true shared-midpoint camera (would require hooking the scroll
// computation itself in overworld.c / dungeon.c rather than post-processing it).
#define MP_LEASH_X 112   // ~half of the 256px screen width
#define MP_LEASH_Y 96    // ~half of the 224px screen height
static void Multiplayer_UpdateCamera(void) {
  PlayerState *p1 = &g_players[0];
  PlayerState *p2 = &g_players[1];
  if (p2->is_dead)
    return;   // don't shove a frozen ghost around against the leash
  int dx = (int)p2->x_coord - (int)p1->x_coord;
  int dy = (int)p2->y_coord - (int)p1->y_coord;
  // Clamp P2 into P1's viewport, and kill P2's velocity into the boundary so it
  // doesn't jitter/stutter while pushing against the leash at a screen edge.
  // Signed intermediates on the low side: coords are uint16 and P1 can be within
  // a leash of the map origin (0), where p1->coord - MP_LEASH would underflow.
  if (dx >  MP_LEASH_X) { p2->x_coord = p1->x_coord + MP_LEASH_X; p2->x_vel = 0; }
  if (dx < -MP_LEASH_X) { int v = (int)p1->x_coord - MP_LEASH_X; p2->x_coord = v < 0 ? 0 : (uint16)v; p2->x_vel = 0; }
  if (dy >  MP_LEASH_Y) { p2->y_coord = p1->y_coord + MP_LEASH_Y; p2->y_vel = 0; }
  if (dy < -MP_LEASH_Y) { int v = (int)p1->y_coord - MP_LEASH_Y; p2->y_coord = v < 0 ? 0 : (uint16)v; p2->y_vel = 0; }
}

// Soft co-op body collision (Phase 3): keep the two Links from fully stacking.
// Only P2 is nudged (P1 stays put as the camera/scroll anchor), 1px/frame along
// the shortest escape axis — gentle enough that P2's normal tile collision next
// frame keeps it out of walls, and "soft" enough that a player can still shove
// the other around. Integer-only (deterministic). Skipped for ghosts, different
// floors, and non-normal-play states (menus/transitions freeze P2 anyway).
#define MP_BODY_HALF 12   // soft half-extent; the Link sprite is ~16px wide
static void Multiplayer_SeparatePlayers(void) {
  PlayerState *p1 = &g_players[0], *p2 = &g_players[1];
  if (!g_mp_p2_enabled || !p1->is_active || !p2->is_active) return;
  if (p1->is_dead || p2->is_dead) return;                    // ghosts are intangible
  if (!((main_module_index == 7 || main_module_index == 9) && submodule_index == 0)) return;
  if (p1->is_on_lower_level != p2->is_on_lower_level) return; // different floors don't collide
  int dx = (int)p2->x_coord - (int)p1->x_coord;
  int dy = (int)p2->y_coord - (int)p1->y_coord;
  int adx = dx < 0 ? -dx : dx, ady = dy < 0 ? -dy : dy;
  if (adx >= MP_BODY_HALF || ady >= MP_BODY_HALF) return;     // not overlapping
  // Push P2 out 1px along the axis of least penetration (shortest way out).
  if ((MP_BODY_HALF - adx) <= (MP_BODY_HALF - ady))
    p2->x_coord += (dx >= 0) ? 1 : -1;
  else
    p2->y_coord += (dy >= 0) ? 1 : -1;
}
#endif // ZELDA3_MULTIPLAYER

void ZeldaInitialize() {
  g_zenv.dma = dma_init(NULL);
  g_zenv.ppu = ppu_init();
  g_zenv.ram = g_ram;
  g_zenv.sram = (uint8*)calloc(8192, 1);
  g_zenv.vram = g_zenv.ppu->vram;
  g_zenv.player = SpcPlayer_Create();
  SpcPlayer_Initialize(g_zenv.player);
  dma_reset(g_zenv.dma);
  ppu_reset(g_zenv.ppu);
#ifdef ZELDA3_MULTIPLAYER
  // Initialize P1 player state
  PlayerState_Init(0);
  g_mp_initialized = false;
#endif
}

static void ZeldaRunPolyLoop() {
  if (intro_did_run_step && !nmi_flag_update_polyhedral) {
    Poly_RunFrame();
    intro_did_run_step = 0;
    nmi_flag_update_polyhedral = 0xff;
  }
}

void ZeldaRunFrameInternal(uint16 input, int run_what) {
  if (animated_tile_data_src == 0)
    ZeldaInitializationCode();

  if (run_what & 2)
    ZeldaRunPolyLoop();
  if (run_what & 1) {
#ifdef ZELDA3_MULTIPLAYER
    // Run the dual-player loop for every co-op mode (local OR online host/client).
    // Online lockstep feeds the remote player's input in through the same
    // g_p2_input_this_frame path, so the loop is identical to local co-op.
    if (g_mp_config.mode == MP_MODE_LOCAL ||
        g_mp_config.mode == MP_MODE_HOST ||
        g_mp_config.mode == MP_MODE_CLIENT)
      ZeldaRunGameLoop_Multiplayer(g_p2_input_this_frame);
    else
#endif
    ZeldaRunGameLoop();
  }
  Interrupt_NMI(input);
}


static int IncrementCrystalCountdown(uint8 *a, int v) {
  int t = *a + v;
  *a = t;
  return t >> 8;
}

int frame_ctr_dbg;
static uint8 *g_emu_memory_ptr;
static ZeldaRunFrameFunc *g_emu_runframe;
static ZeldaSyncAllFunc *g_emu_syncall;

void ZeldaSetupEmuCallbacks(uint8 *emu_ram, ZeldaRunFrameFunc *func, ZeldaSyncAllFunc *sync_all) {
  g_emu_memory_ptr = emu_ram;
  g_emu_runframe = func;
  g_emu_syncall = sync_all;
}

static void EmuSynchronizeWholeState() {
  if (g_emu_syncall)
    g_emu_syncall();
}

// |ptr| must be a pointer into g_ram, will synchronize the RAM memory with the
// emulator.
static void EmuSyncMemoryRegion(void *ptr, size_t n) {
  uint8 *data = (uint8 *)ptr;
  assert(data >= g_ram && data < g_ram + 0x20000);
  if (g_emu_memory_ptr)
    memcpy(g_emu_memory_ptr + (data - g_ram), data, n);
}

static void Startup_InitializeMemory() {  // 8087c0
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
  INIDISP_copy = 0x80;
  flag_update_cgram_in_nmi++;
}

void ByteArray_AppendVl(ByteArray *arr, uint32 v) {
  for (; v >= 255; v -= 255)
    ByteArray_AppendByte(arr, 255);
  ByteArray_AppendByte(arr, v);
}

void saveFunc(void *ctx_in, void *data, size_t data_size) {
  ByteArray_AppendData((ByteArray *)ctx_in, data, data_size);
}

typedef struct LoadFuncState {
  uint8 *p, *pend;
} LoadFuncState;

void loadFunc(void *ctx, void *data, size_t data_size) {
  LoadFuncState *st = (LoadFuncState *)ctx;
  assert(st->pend - st->p >= data_size);
  memcpy(data, st->p, data_size);
  st->p += data_size;
}

static void InternalSaveLoad(SaveLoadFunc *func, void *ctx) {
  uint8 junk[58] = { 0 };
  func(ctx, junk, 27);
  func(ctx, g_zenv.player->ram, 0x10000);  // apu ram
  func(ctx, junk, 40); // junk
  dsp_saveload(g_zenv.player->dsp, func, ctx); // 3024 bytes of dsp
  func(ctx, junk, 15); // spc junk
  dma_saveload(g_zenv.dma, func, ctx); // 192 bytes of dma state
  ppu_saveload(g_zenv.ppu, func, ctx); // 66619 + 512 + 174
  func(ctx, g_zenv.sram, 0x2000);  // 8192 bytes of sram
  func(ctx, junk, 58); // snes junk
  func(ctx, g_zenv.ram, 0x20000);  // 0x20000 bytes of ram
  func(ctx, junk, 4); // snes junk
}

void ZeldaReset(bool preserve_sram) {
  frame_ctr_dbg = 0;
  dma_reset(g_zenv.dma);
  ppu_reset(g_zenv.ppu);
  memset(g_zenv.ram, 0, 0x20000);
  if (!preserve_sram)
    memset(g_zenv.sram, 0, 0x2000);
  ZeldaApuLock();
  ZeldaRestoreMusicAfterLoad_Locked(true);
  ZeldaApuUnlock();
  EmuSynchronizeWholeState();

}

static void LoadSnesState(SaveLoadFunc *func, void *ctx) {
  // Do the actual loading
  ZeldaApuLock();
  InternalSaveLoad(func, ctx);
  memcpy(g_zenv.ram + 0x1DBA0, g_zenv.ram + 0x1b00, 224 * 2); // hdma table was moved

  ZeldaRestoreMusicAfterLoad_Locked(false);
  ZeldaApuUnlock();
  EmuSynchronizeWholeState();
}

static void SaveSnesState(SaveLoadFunc *func, void *ctx) {
  memcpy(g_zenv.ram + 0x1b00, g_zenv.ram + 0x1DBA0, 224 * 2); // hdma table was moved
  ZeldaApuLock();
  ZeldaSaveMusicStateToRam_Locked();
  InternalSaveLoad(func, ctx);
  ZeldaApuUnlock();
}

typedef struct StateRecorder {
  uint16 last_inputs;
  uint32 frames_since_last;
  uint32 total_frames;

  // For replay
  uint32 replay_pos, replay_pos_last_complete;
  uint32 replay_frame_counter;
  uint32 replay_next_cmd_at;
  uint8 replay_cmd;
  bool replay_mode;

  ByteArray log;
  ByteArray base_snapshot;
} StateRecorder;

static StateRecorder state_recorder;

void StateRecorder_Init(StateRecorder *sr) {
  memset(sr, 0, sizeof(*sr));
}

void StateRecorder_RecordCmd(StateRecorder *sr, uint8 cmd) {
  int frames = sr->frames_since_last;
  sr->frames_since_last = 0;
  int x = (cmd < 0xc0) ? 0xf : 0x1;
  ByteArray_AppendByte(&sr->log, cmd | (frames < x ? frames : x));
  if (frames >= x)
    ByteArray_AppendVl(&sr->log, frames - x);
}

void StateRecorder_Record(StateRecorder *sr, uint16 inputs) {
  uint16 diff = inputs ^ sr->last_inputs;
  if (diff != 0) {
    sr->last_inputs = inputs;
    //    printf("0x%.4x %d: ", diff, sr->frames_since_last);
    //    size_t lb = sr->log.size;
    for (int i = 0; i < 12; i++) {
      if ((diff >> i) & 1)
        StateRecorder_RecordCmd(sr, i << 4);
    }
    //    while (lb < sr->log.size)
    //      printf("%.2x ", sr->log.data[lb++]);
    //    printf("\n");
  }
  sr->frames_since_last++;
  sr->total_frames++;
}

void StateRecorder_RecordPatchByte(StateRecorder *sr, uint32 addr, const uint8 *value, int num) {
  assert(addr < 0x20000);

  //  printf("%d: PatchByte(0x%x, 0x%x. %d): ", sr->frames_since_last, addr, *value, num);
  //  size_t lb = sr->log.size;
  int lq = (num - 1) <= 3 ? (num - 1) : 3;
  StateRecorder_RecordCmd(sr, 0xc0 | (addr & 0x10000 ? 2 : 0) | lq << 2);
  if (lq == 3)
    ByteArray_AppendVl(&sr->log, num - 1 - 3);
  ByteArray_AppendByte(&sr->log, addr >> 8);
  ByteArray_AppendByte(&sr->log, addr);
  for (int i = 0; i < num; i++)
    ByteArray_AppendByte(&sr->log, value[i]);
  //  while (lb < sr->log.size)
  //    printf("%.2x ", sr->log.data[lb++]);
  //  printf("\n");
}

void ReadFromFile(FILE *f, void *data, size_t n) {
  if (fread(data, 1, n, f) != n)
    Die("fread failed\n");
}

void StateRecorder_Load(StateRecorder *sr, FILE *f, bool replay_mode) {
  // todo: fix robustness on invalid data.
  uint32 hdr[8] = { 0 };
  ReadFromFile(f, hdr, sizeof(hdr));

  assert(hdr[0] == 1);

  sr->total_frames = hdr[1];
  ByteArray_Resize(&sr->log, hdr[2]);
  ReadFromFile(f, sr->log.data, sr->log.size);
  sr->last_inputs = hdr[3];
  sr->frames_since_last = hdr[4];

  ByteArray_Resize(&sr->base_snapshot, (hdr[5] & 1) ? hdr[6] : 0);
  ReadFromFile(f, sr->base_snapshot.data, sr->base_snapshot.size);

  sr->replay_next_cmd_at = 0;

  sr->replay_mode = replay_mode;
  if (replay_mode) {
    sr->frames_since_last = 0;
    sr->last_inputs = 0;
    sr->replay_pos = sr->replay_pos_last_complete = 0;
    sr->replay_frame_counter = 0;
    // Load snapshot from |base_snapshot_|, or reset if empty.

    if (sr->base_snapshot.size) {
      LoadFuncState state = { sr->base_snapshot.data, sr->base_snapshot.data + sr->base_snapshot.size };
      LoadSnesState(&loadFunc, &state);
      assert(state.p == state.pend);
    } else {
      ZeldaReset(false);
    }
  } else {
    // Resume replay from the saved position?
    sr->replay_pos = sr->replay_pos_last_complete = hdr[5] >> 1;
    sr->replay_frame_counter = hdr[7];
    sr->replay_mode = (sr->replay_frame_counter != 0);

    ByteArray arr = { 0 };
    ByteArray_Resize(&arr, hdr[6]);
    ReadFromFile(f, arr.data, arr.size);
    LoadFuncState state = { arr.data, arr.data + arr.size };
    LoadSnesState(&loadFunc, &state);
    ByteArray_Destroy(&arr);
    assert(state.p == state.pend);
  }
}

void StateRecorder_Save(StateRecorder *sr, FILE *f) {
  uint32 hdr[8] = { 0 };
  ByteArray arr = { 0 };
  SaveSnesState(&saveFunc, &arr);
  assert(sr->base_snapshot.size == 0 || sr->base_snapshot.size == arr.size);

  hdr[0] = 1;
  hdr[1] = sr->total_frames;
  hdr[2] = (uint32)sr->log.size;
  hdr[3] = sr->last_inputs;
  hdr[4] = sr->frames_since_last;
  hdr[5] = sr->base_snapshot.size ? 1 : 0;
  hdr[6] = (uint32)arr.size;
  // If saving while in replay mode, also need to persist
  // sr->replay_pos_last_complete and sr->replay_frame_counter
  // so the replaying can be resumed.
  if (sr->replay_mode) {
    hdr[5] |= sr->replay_pos_last_complete << 1;
    hdr[7] = sr->replay_frame_counter;
  }
  fwrite(hdr, 1, sizeof(hdr), f);
  fwrite(sr->log.data, 1, hdr[2], f);
  fwrite(sr->base_snapshot.data, 1, sr->base_snapshot.size, f);
  fwrite(arr.data, 1, arr.size, f);

  ByteArray_Destroy(&arr);
}

void StateRecorder_ClearKeyLog(StateRecorder *sr) {
  printf("Clearing key log!\n");
  sr->base_snapshot.size = 0;
  SaveSnesState(&saveFunc, &sr->base_snapshot);
  ByteArray old_log = sr->log;
  int old_frames_since_last = sr->frames_since_last;
  memset(&sr->log, 0, sizeof(sr->log));
  // If there are currently any active inputs, record them initially at timestamp 0.
  sr->frames_since_last = 0;
  if (sr->last_inputs) {
    for (int i = 0; i < 12; i++) {
      if ((sr->last_inputs >> i) & 1)
        StateRecorder_RecordCmd(sr, i << 4);
    }
  }
  if (sr->replay_mode) {
    // When clearing the key log while in replay mode, we want to keep
    // replaying but discarding all key history up until this point.
    if (sr->replay_next_cmd_at != 0xffffffff) {
      sr->replay_next_cmd_at -= old_frames_since_last;
      sr->frames_since_last = sr->replay_next_cmd_at;
      sr->replay_pos_last_complete = (uint32)sr->log.size;
      StateRecorder_RecordCmd(sr, sr->replay_cmd);
      int old_replay_pos = sr->replay_pos;
      sr->replay_pos = (uint32)sr->log.size;
      ByteArray_AppendData(&sr->log, old_log.data + old_replay_pos, old_log.size - old_replay_pos);
    }
    sr->total_frames -= sr->replay_frame_counter;
    sr->replay_frame_counter = 0;
  } else {
    sr->total_frames = 0;
  }
  ByteArray_Destroy(&old_log);
  sr->frames_since_last = 0;
}

uint16 StateRecorder_ReadNextReplayState(StateRecorder *sr) {
  assert(sr->replay_mode);
  while (sr->frames_since_last >= sr->replay_next_cmd_at) {
    int replay_pos = sr->replay_pos;
    if (replay_pos != sr->replay_pos_last_complete) {
      // Apply next command
      sr->frames_since_last = 0;
      if (sr->replay_cmd < 0xc0) {
        sr->last_inputs ^= 1 << (sr->replay_cmd >> 4);
      } else if (sr->replay_cmd < 0xd0) {
        int nb = 1 + ((sr->replay_cmd >> 2) & 3);
        uint8 t;
        if (nb == 4) do {
          nb += t = sr->log.data[replay_pos++];
        } while (t == 255);
        uint32 addr = ((sr->replay_cmd >> 1) & 1) << 16;
        addr |= sr->log.data[replay_pos++] << 8;
        addr |= sr->log.data[replay_pos++];
        do {
          g_ram[addr & 0x1ffff] = sr->log.data[replay_pos++];
          EmuSyncMemoryRegion(&g_ram[addr & 0x1ffff], 1);
        } while (addr++, --nb);
      } else {
        assert(0);
      }
    }
    sr->replay_pos_last_complete = replay_pos;
    if (replay_pos >= sr->log.size) {
      sr->replay_pos = replay_pos;
      sr->replay_next_cmd_at = 0xffffffff;
      break;
    }
    // Read the next one
    uint8 cmd = sr->log.data[replay_pos++], t;
    int mask = (cmd < 0xc0) ? 0xf : 0x1;
    int frames = cmd & mask;
    if (frames == mask) do {
      frames += t = sr->log.data[replay_pos++];
    } while (t == 255);
    sr->replay_next_cmd_at = frames;
    sr->replay_cmd = cmd;
    sr->replay_pos = replay_pos;
  }
  sr->frames_since_last++;
  // Turn off replay mode after we reached the final frame position
  if (++sr->replay_frame_counter >= sr->total_frames) {
    sr->replay_mode = false;
  }
  return sr->last_inputs;
}

void StateRecorder_StopReplay(StateRecorder *sr) {
  if (!sr->replay_mode)
    return;
  sr->replay_mode = false;
  sr->total_frames = sr->replay_frame_counter;
  sr->log.size = sr->replay_pos_last_complete;
}

#ifdef _DEBUG
// This can be used to read inputs from a text file for easier debugging
int InputStateReadFromFile() {
  static FILE *f;
  static uint32 next_ts, next_keys, cur_keys;
  char buf[64];
  char keys[64];

  while (state_recorder.total_frames == next_ts) {
    cur_keys = next_keys;
    if (!f)
      f = fopen("boss_bug.txt", "r");
    if (fgets(buf, sizeof(buf), f)) {
      if (sscanf(buf, "%d: %s", &next_ts, keys) == 1) keys[0] = 0;
      int i = 0;
      for (const char *s = keys; *s; s++) {
        static const char kKeys[] = "AXsSUDLRBY";
        const char *t = strchr(kKeys, *s);
        assert(t);
        i |= 1 << (t - kKeys);
      }
      next_keys = i;
    } else {
      next_ts = 0xffffffff;
    }
  }

  return cur_keys;
}
#endif

#ifdef ZELDA3_MULTIPLAYER
bool ZeldaRunFrame(int inputs, int inputs_p2) {
#else
bool ZeldaRunFrame(int inputs) {
#endif

  // Avoid up/down and left/right from being pressed at the same time
  if ((inputs & 0x30) == 0x30) inputs ^= 0x30;
  if ((inputs & 0xc0) == 0xc0) inputs ^= 0xc0;

#ifdef ZELDA3_MULTIPLAYER
  // Same opposing-direction filter P1 gets above (the SNES pad can't report
  // up+down / left+right at once); keep P2 symmetric and deterministic.
  if ((inputs_p2 & 0x30) == 0x30) inputs_p2 ^= 0x30;
  if ((inputs_p2 & 0xc0) == 0xc0) inputs_p2 ^= 0xc0;
  // Store P2 input for use in the multiplayer game loop
  g_p2_input_this_frame = (uint16)inputs_p2;
#endif

  frame_ctr_dbg++;

  bool is_replay = state_recorder.replay_mode;

  // Either copy state or apply state
  if (is_replay) {
    inputs = StateRecorder_ReadNextReplayState(&state_recorder);
  } else {
    //    input_state = InputStateReadFromFile();
    // Co-op caveat: the StateRecorder captures only P1's joypad, so the built-in
    // rewind/replay is P1-only — a recorded co-op session would desync P2 on
    // playback (P2 input isn't in the stream). This matches the design that P2
    // state is ephemeral/not saved; co-op just doesn't support rewind. A future
    // fix would record g_p2_input_this_frame alongside P1's.
    StateRecorder_Record(&state_recorder, inputs);

    // This is whether APUI00 is true or false, this is used by the ancilla code.
    uint8 apui00 = ZeldaIsMusicPlaying();
    if (apui00 != g_ram[kRam_APUI00]) {
      g_ram[kRam_APUI00] = apui00;
      EmuSyncMemoryRegion(&g_ram[kRam_APUI00], 1);
      StateRecorder_RecordPatchByte(&state_recorder, 0x648, &apui00, 1);
    }

    if (animated_tile_data_src != 0) {
      // Whenever we're no longer replaying, we'll remember what bugs were fixed,
      // but only if game is initialized.
      if (g_ram[kRam_BugsFixed] < kBugFix_Latest) {
        g_ram[kRam_BugsFixed] = kBugFix_Latest;
        EmuSyncMemoryRegion(&g_ram[kRam_BugsFixed], 1);
        StateRecorder_RecordPatchByte(&state_recorder, kRam_BugsFixed, &g_ram[kRam_BugsFixed], 1);
      }

      if (enhanced_features0 != g_wanted_zelda_features) {
        enhanced_features0 = g_wanted_zelda_features;
        EmuSyncMemoryRegion(&enhanced_features0, sizeof(enhanced_features0));
        StateRecorder_RecordPatchByte(&state_recorder, kRam_Features0, (uint8 *)&enhanced_features0, 4);
      }
    }
  }

  int run_what;
  if (g_ram[kRam_BugsFixed] < kBugFix_PolyRenderer) {
    // A previous version of this code alternated the game loop with
    // the poly renderer.
    run_what = (is_nmi_thread_active && thread_other_stack != 0x1f31) ? 2 : 1;
  } else {
    // The snes seems to let poly rendering run for a little
    // while each fram until it eventually completes a frame.
    // Simulate this by rendering the poly every n:th frame.
    run_what = (is_nmi_thread_active && IncrementCrystalCountdown(&g_ram[kRam_CrystalRotateCounter], virq_trigger)) ? 3 : 1;
    EmuSyncMemoryRegion(&g_ram[kRam_CrystalRotateCounter], 1);
  }

  if (g_emu_runframe == NULL || enhanced_features0 != 0 || g_zenv.dialogue_flags
#ifdef ZELDA3_MULTIPLAYER
      // Co-op intentionally diverges from a vanilla single-player SNES (a second
      // Link, shared camera, etc.), so the reference-emulator memory compare is
      // meaningless here. Always run the C implementation directly in co-op —
      // local OR online (host/client).
      || g_mp_config.mode == MP_MODE_LOCAL
      || g_mp_config.mode == MP_MODE_HOST
      || g_mp_config.mode == MP_MODE_CLIENT
#endif
      ) {
    // can't compare against real impl when running with extra features.
    ZeldaRunFrameInternal(inputs, run_what);
  } else {
    g_emu_runframe(inputs, run_what);
  }

  ZeldaPushApuState();

  return is_replay;
}

#ifdef ZELDA3_MULTIPLAYER
// ============================================================================
// Online input-lockstep driver (transport-agnostic; see net_transport.h).
// The deterministic sim is the same on both peers, so only InputFrames cross
// the wire. This is the verified foundation; a real UDP transport plugs into
// the NetTransport passed to Multiplayer_LockstepInit with no changes here.
// ============================================================================
static NetTransport *g_net_transport;
static int g_net_local_player;     // which player's input is captured locally (0/1)
static int g_net_input_delay;      // sim trails input capture by this many frames
static uint32 g_net_send_frame;    // frame number to tag the next local input with
static uint32 g_net_remote_next;   // next remote frame# expected (de-dups UDP resends)
static uint32 g_net_last_sync_frame; // last frame we sent a SYNC checksum for

void Multiplayer_LockstepInit(NetTransport *t, int local_player_index, int input_delay) {
  g_net_transport = t;
  g_net_local_player = (local_player_index != 0);
  g_net_input_delay = input_delay < 0 ? 0 : input_delay;
  g_net_send_frame = 0;
  g_net_remote_next = 0;
  g_net_last_sync_frame = 0xFFFFFFFF;
  Multiplayer_ResetLockstep();   // clears the input rings + g_sim_frame
}

int Multiplayer_LockstepTick(uint16 local_joypad) {
  if (!g_net_transport)
    return 0;

  // 1. Tag this tick's local input, queue it locally, and transmit it — but only
  //    while the local ring has room. A stalled peer (paused, or one-way packet
  //    loss) freezes the sim so the ring never drains; without this gate the ring
  //    would wrap at INPUT_RING_SIZE and overwrite unconsumed input, desyncing on
  //    resume. When full we hold local capture (standard lockstep wait-for-peer).
  InputFrame lf;
  lf.frame_number = g_net_send_frame;
  lf.joypad = local_joypad;
  lf.player_index = (uint8)g_net_local_player;
  lf.flags = INPUT_FLAG_NONE;
  if (InputRing_Push(g_net_local_player, &lf)) {
    g_net_transport->send(g_net_transport, &lf);
    g_net_send_frame++;
  }

  // 2. Ingest remote input into the remote player's ring, in order, de-duping
  //    the redundant copies a UDP transport resends for loss tolerance: accept a
  //    frame only when it's exactly the next one expected (older = duplicate,
  //    newer = a gap that a later resend will fill). Stop if the ring fills; the
  //    dropped frame's redundant resend is re-accepted next tick once it drains.
  int remote = g_net_local_player ^ 1;
  InputFrame rf;
  while (g_net_transport->recv(g_net_transport, &rf)) {
    if ((int)rf.player_index == remote && rf.frame_number == g_net_remote_next) {
      if (!InputRing_Push(remote, &rf))
        break;
      g_net_remote_next++;
    }
  }

  // 3. Advance the sim for every frame whose inputs (both players) are present,
  //    keeping it input_delay frames behind capture so the remote frame has time
  //    to arrive. Both peers run the identical pair -> they stay in lockstep.
  int advanced = 0;
  while ((int)(g_net_send_frame - g_sim_frame) > g_net_input_delay &&
         Multiplayer_InputsReady()) {
    FrameInputPair pair = Multiplayer_ConsumeInputs();
    ZeldaRunFrame(pair.joypad[0], pair.joypad[1]);
    advanced++;
  }

  // Periodically hand the transport our state checksum for this frame; it
  // exchanges them with the peer and flags a desync if they ever disagree.
  if (g_net_transport->send_sync && g_sim_frame != g_net_last_sync_frame &&
      (g_sim_frame % SYNC_CHECKSUM_INTERVAL) == 0) {
    g_net_last_sync_frame = g_sim_frame;
    g_net_transport->send_sync(g_net_transport, g_sim_frame,
                               Multiplayer_ComputeChecksum().checksum);
  }
  return advanced;
}
#endif  // ZELDA3_MULTIPLAYER

void ZeldaSetLanguage(const char *language) {
  static const uint8 kDefaultConf[3] = { 0, 0, 0 };
  MemBlk found = { kDefaultConf, 3 };
  if (language) {
    size_t n = strlen(language);
    for (int i = 0; ; i++) {
      MemBlk mb = kDialogueMap(i);
      if (mb.ptr == 0) {
        fprintf(stderr, "Unable to find language '%s'\n", language);
        break;
      }
      MemBlk name = FindIndexInMemblk(mb, 0);
      if (name.size == n && !memcmp(name.ptr, language, n)) {
        found = FindIndexInMemblk(mb, 1);
        break;
      }
    }
  }
  g_zenv.dialogue_blk = kDialogue(found.ptr[0]);
  g_zenv.dialogue_font_blk = kDialogueFont(found.ptr[1]);
  g_zenv.dialogue_flags = found.ptr[2];
}


static const char *const kReferenceSaves[] = {
  "Chapter 1 - Zelda's Rescue.sav",
  "Chapter 2 - After Eastern Palace.sav",
  "Chapter 3 - After Desert Palace.sav",
  "Chapter 4 - After Tower of Hera.sav",
  "Chapter 5 - After Hyrule Castle Tower.sav",
  "Chapter 6 - After Dark Palace.sav",
  "Chapter 7 - After Swamp Palace.sav",
  "Chapter 8 - After Skull Woods.sav",
  "Chapter 9 - After Gargoyle's Domain.sav",
  "Chapter 10 - After Ice Palace.sav",
  "Chapter 11 - After Misery Mire.sav",
  "Chapter 12 - After Turtle Rock.sav",
  "Chapter 13 - After Ganon's Tower.sav",
};

void SaveLoadSlot(int cmd, int which) {
  char name[128];
  if (which & 256) {
    if (cmd == kSaveLoad_Save)
      return;
    sprintf(name, "saves/ref/%s", kReferenceSaves[which - 256]);
  } else {
    sprintf(name, "saves/save%d.sav", which);
  }
  FILE *f = fopen(name, cmd != kSaveLoad_Save ? "rb" : "wb");
  if (f) {
    printf("*** %s slot %d\n",
      cmd == kSaveLoad_Save ? "Saving" : cmd == kSaveLoad_Load ? "Loading" : "Replaying", which);

    if (cmd != kSaveLoad_Save)
      StateRecorder_Load(&state_recorder, f, cmd == kSaveLoad_Replay);
    else
      StateRecorder_Save(&state_recorder, f);

    fclose(f);
  }
}

typedef struct StateRecoderMultiPatch {
  uint32 count;
  uint32 addr;
  uint8 vals[256];
} StateRecoderMultiPatch;


void StateRecoderMultiPatch_Init(StateRecoderMultiPatch *mp) {
  mp->count = mp->addr = 0;
}

void StateRecoderMultiPatch_Commit(StateRecoderMultiPatch *mp) {
  if (mp->count)
    StateRecorder_RecordPatchByte(&state_recorder, mp->addr, mp->vals, mp->count);
}

void StateRecoderMultiPatch_Patch(StateRecoderMultiPatch *mp, uint32 addr, uint8 value) {
  if (mp->count >= 256 || addr != mp->addr + mp->count) {
    StateRecoderMultiPatch_Commit(mp);
    mp->addr = addr;
    mp->count = 0;
  }
  mp->vals[mp->count++] = value;
  g_ram[addr] = value;
  EmuSyncMemoryRegion(&g_ram[addr], 1);
}

void PatchCommand(char c) {
  StateRecoderMultiPatch mp;

  StateRecoderMultiPatch_Init(&mp);
  if (c == 'w') {
    StateRecoderMultiPatch_Patch(&mp, 0xf372, 80);  // health filler
    StateRecoderMultiPatch_Patch(&mp, 0xf373, 80);  // magic filler
    //    b.Patch(0x1FE01, 25);
  } else if (c == 'W') {
    StateRecoderMultiPatch_Patch(&mp, 0xf375, 10);  // link_bomb_filler
    StateRecoderMultiPatch_Patch(&mp, 0xf376, 10);  // link_arrow_filler
    uint16 rupees = link_rupees_goal + 100;
    StateRecoderMultiPatch_Patch(&mp, 0xf360, rupees);  // link_rupees_goal
    StateRecoderMultiPatch_Patch(&mp, 0xf361, rupees >> 8);  // link_rupees_goal
  } else if (c == 'k') {
    StateRecorder_ClearKeyLog(&state_recorder);
  } else if (c == 'o') {
    StateRecoderMultiPatch_Patch(&mp, 0xf36f, 1);
  } else if (c == 'l') {
    StateRecorder_StopReplay(&state_recorder);
  } else if (c == 'E') {
    StateRecoderMultiPatch_Patch(&mp, 0x37f, g_ram[0x37f] ^ 1);
  }
  StateRecoderMultiPatch_Commit(&mp);
}


void ZeldaReadSram() {
  FILE *f = fopen("saves/sram.dat", "rb");
  if (f) {
    if (fread(g_zenv.sram, 1, 8192, f) != 8192)
      fprintf(stderr, "Error reading saves/sram.dat\n");
    fclose(f);
    EmuSynchronizeWholeState();
  }
}

void ZeldaWriteSram() {
  rename("saves/sram.dat", "saves/sram.bak");
  FILE *f = fopen("saves/sram.dat", "wb");
  if (f) {
    fwrite(g_zenv.sram, 1, 8192, f);
    fclose(f);
  } else {
    fprintf(stderr, "Unable to write saves/sram.dat\n");
  }
}