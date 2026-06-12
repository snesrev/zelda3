// headless_test.c — Headless driver for the game core (no SDL window/audio).
// Links against all of src/*.c except main.c. Drives ZeldaRunFrame with a
// scripted input sequence: boots from power-on, creates a save file, advances
// the intro text, then runs movement/rendering experiments on both players.
// Used to verify two-player independence without a display.
//
// Build (from repo root):
//   gcc -O1 -w -I. $(sdl2-config --cflags) -DZELDA3_MULTIPLAYER=1 \
//     -DSYSTEM_VOLUME_MIXER_AVAILABLE=0 tools/headless_test.c \
//     $(ls src/*.c | grep -v 'src/main.c') snes/*.c \
//     third_party/gl_core/gl_core_3_1.c \
//     third_party/opus-1.3.1-stripped/opus_decoder_amalgam.c \
//     -o headless_coop $(sdl2-config --libs) -lm
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/types.h"
#include "../src/zelda_rtl.h"
#include "../snes/ppu.h"
#include "../src/assets.h"
#include "../src/util.h"
#ifdef ZELDA3_MULTIPLAYER
#include "../src/player_state.h"
#endif

extern uint8 g_ram[131072];

// ---- Stubs for symbols normally provided by main.c ----
void NORETURN Die(const char *error) {
  fprintf(stderr, "DIE: %s\n", error);
  exit(1);
}
void ZeldaApuLock(void) {}
void ZeldaApuUnlock(void) {}

// From sprite.c — used for the enemy HP scaling unit checks.
void SpritePrep_LoadProperties(int k);

const uint8 *g_asset_ptrs[kNumberOfAssets];
uint32 g_asset_sizes[kNumberOfAssets];

static void LoadAssets(void) {
  size_t length = 0;
  uint8 *data = ReadWholeFile("zelda3_assets.dat", &length);
  if (!data)
    Die("Failed to read zelda3_assets.dat");
  static const char kAssetsSig[] = { kAssets_Sig };
  if (length < 16 + 32 + 32 + 8 + kNumberOfAssets * 4 ||
      memcmp(data, kAssetsSig, 48) != 0 ||
      *(uint32 *)(data + 80) != kNumberOfAssets)
    Die("Invalid assets file");
  uint32 offset = 88 + kNumberOfAssets * 4 + *(uint32 *)(data + 84);
  for (size_t i = 0; i < kNumberOfAssets; i++) {
    uint32 size = *(uint32 *)(data + 88 + i * 4);
    offset = (offset + 3) & ~3;
    if ((uint64)offset + size > length)
      Die("Assets file corruption");
    g_asset_sizes[i] = size;
    g_asset_ptrs[i] = data + offset;
    offset += size;
  }
}

MemBlk FindInAssetArray(int asset, int idx) {
  return FindIndexInMemblk((MemBlk){ g_asset_ptrs[asset], g_asset_sizes[asset] }, idx);
}

// ---- Input bits for ZeldaRunFrame ----
// bit0=B 1=Y 2=Select 3=Start 4=Up 5=Down 6=Left 7=Right 8=A 9=X 10=L 11=R
enum { B_B = 1, B_Y = 2, B_SEL = 4, B_START = 8, B_UP = 16, B_DOWN = 32,
       B_LEFT = 64, B_RIGHT = 128, B_A = 256 };

#define MMI g_ram[0x10]   // main_module_index
#define SMI g_ram[0x11]   // submodule_index

static int g_failures;

#ifdef ZELDA3_MULTIPLAYER
// Track chars used by non-P2 OAM entries inside P2's reserved char region
// (0x26-0x2f/0x36-0x3f minus the shadow chars 0x28/0x38), and frames where
// P2's pose tiles in VRAM differ from P1's.
static uint32 g_char_conflicts;
static uint32 g_conflict_by_char[0x40];
static uint32 g_p2_pose_differs_frames;
// Frames where P2's body piece (char 0x26) was visible but its packed
// 2-bit OAM extension (size/x-high) was NOT the 16x16 value (2). The PPU
// reads only the packed table, so a wrong value renders P2 as 8x8 garbage.
static uint32 g_p2_extbits_bad_frames;
// Histogram of OBJ palettes used by visible non-player OAM entries.
static uint32 g_palette_usage[8];

static bool InP2OamBlock(int i) {
  // P2 renders into the OAM band opposite P1's: entries 56-67 or 100-111.
  return (i >= 0xe0 / 4 && i < 0xe0 / 4 + 12) ||
         (i >= 0x190 / 4 && i < 0x190 / 4 + 12);
}

static void ScanOam(void) {
  OamEnt *oam = (OamEnt *)(g_ram + 0x800);
  for (int i = 0; i < 128; i++) {
    if (oam[i].y == 0xf0)  // offscreen/hidden
      continue;
    if (InP2OamBlock(i))
      continue;
    uint8 c = oam[i].charnum;
    if (!(oam[i].flags & 1) && c != 0x28 && c != 0x38 &&
        ((c >= 0x26 && c <= 0x2f) || (c >= 0x36 && c <= 0x3f))) {
      g_char_conflicts++;
      g_conflict_by_char[c]++;
    }
    g_palette_usage[(oam[i].flags >> 1) & 7]++;
  }
  // Compare P1's body row 1 tiles (chars 0x00-0x01, vram word 0x4000) with
  // P2's (chars 0x26-0x27, vram word 0x4260): when the poses differ, the
  // tile data must differ too — that is the fix for "P2 mirrors P1".
  if (g_p2_draw_active &&
      memcmp(&g_zenv.vram[0x4000], &g_zenv.vram[0x4260], 0x40) != 0)
    g_p2_pose_differs_frames++;
  // Verify the PACKED extension bits the PPU actually uses for P2's
  // body-main piece (remapped char 0x26). Legitimate pair values:
  //   2 = 16x16 on-screen, 3 = 16x16 riding the right screen edge (x-high),
  //   1 = the engine's hide-Link trick (small + x-high parks it off-screen,
  //       used for doorway crossings and damage blinking).
  // Only 0 — a small sprite drawn ON screen — would be the old garbled-P2
  // rendering bug.
  if (g_p2_draw_active) {
    bool bad = false;
    for (int e = 0; e < 128; e++) {
      if (oam[e].y == 0xf0 || oam[e].charnum != 0x26 || (oam[e].flags & 1))
        continue;
      if (!InP2OamBlock(e))
        continue;
      uint8 packed = g_ram[0xA00 + (e >> 2)];
      if (((packed >> ((e & 3) * 2)) & 3) == 0)
        bad = true;
    }
    if (bad)
      g_p2_extbits_bad_frames++;
  }
}
#endif

static void RunFrames(int n, int p1, int p2) {
  for (int i = 0; i < n; i++) {
#ifdef ZELDA3_MULTIPLAYER
    ZeldaRunFrame(p1, p2);
    ScanOam();
#else
    ZeldaRunFrame(p1);
#endif
  }
}

#ifdef ZELDA3_MULTIPLAYER
static void Report(const char *tag) {
  printf("%-26s module=%02x.%02x P1=(%5d,%5d) P2=(%5d,%5d) p2drawn=%d p2hp=%d\n",
         tag, MMI, SMI,
         g_players[0].x_coord, g_players[0].y_coord,
         g_players[1].x_coord, g_players[1].y_coord,
         g_p2_draw_active, g_players[1].health_current);
}
static uint16 P1X(void) { return g_players[0].x_coord; }
static uint16 P1Y(void) { return g_players[0].y_coord; }
static uint16 P2X(void) { return g_players[1].x_coord; }
static uint16 P2Y(void) { return g_players[1].y_coord; }
#else
static void Report(const char *tag) {
  printf("%-26s module=%02x.%02x ram=(%5d,%5d)\n", tag, MMI, SMI,
         *(uint16 *)(g_ram + 0x22), *(uint16 *)(g_ram + 0x20));
}
#endif

// Render the current frame through the real PPU and write a raw RGBA dump.
static void DumpPpuFrame(const char *path) {
  static uint8 pixels[256 * 4 * 240];
  memset(pixels, 0, sizeof(pixels));
  ZeldaDrawPpuFrame(pixels, 256 * 4, 0);
  FILE *f = fopen(path, "wb");
  if (f) { fwrite(pixels, 1, 256 * 4 * 224, f); fclose(f); }
  printf("ppu frame dumped to %s\n", path);
}

static void Check(bool cond, const char *what) {
  printf("  %-52s %s\n", what, cond ? "PASS" : "FAIL");
  if (!cond)
    g_failures++;
}

int main(int argc, char **argv) {
  LoadAssets();
  ZeldaInitialize();
  ZeldaSetLanguage(NULL);
#ifdef ZELDA3_MULTIPLAYER
  // Mirror main.c: enable local co-op mode
  g_mp_config.mode = MP_MODE_LOCAL;
  g_mp_config.num_players = 2;
  g_mp_config.local_player_index = 0;
  g_mp_config.input_delay_frames = 0;
#endif

  // ---- Boot to gameplay: menu-mash Start/A through intro & file select. ----
  int frame = 0, stable = 0;
  for (; frame < 36000; frame++) {
    int p1 = 0;
    bool press = (frame & 7) < 2;
    if (MMI < 7) {
      p1 = press ? (((frame >> 3) & 1) ? B_START : B_A) : 0;
    } else if (MMI == 14 || ((MMI == 7 || MMI == 9) && SMI != 0)) {
      p1 = press ? B_A : 0;
    }
    RunFrames(1, p1, 0);
    if ((MMI == 7 || MMI == 9) && SMI == 0)
      stable++;
    else
      stable = 0;
    if (stable >= 180)
      break;
  }
  Report("gameplay reached");
  if (stable < 180) {
    printf("FAILED to reach gameplay within %d frames\n", frame);
    return 1;
  }

  // Reset conflict counters: only steady gameplay frames matter.
#ifdef ZELDA3_MULTIPLAYER
  g_char_conflicts = 0;
  memset(g_conflict_by_char, 0, sizeof(g_conflict_by_char));
  g_p2_pose_differs_frames = 0;
#endif

  // Get out of bed: advance any leftover text, then walk down (a direction
  // press makes Link hop off the bed), then settle.
  RunFrames(2, B_A, 0);
  RunFrames(120, 0, 0);
  RunFrames(60, B_DOWN, 0);
  RunFrames(180, 0, 0);
  Report("settled");

#ifdef ZELDA3_MULTIPLAYER
  DumpPpuFrame("/tmp/frame_settled.rgba");
  // Palette experiment: force P2's OAM pieces to Link's palette (7). If P2
  // then renders pixel-identical to P1, the tiles are perfect and palette 5
  // is the only problem.
  {
    OamEnt *oam = (OamEnt *)(g_ram + 0x800);
    for (int e = 0; e < 128; e++) {
      if (oam[e].y == 0xf0 || !InP2OamBlock(e)) continue;
      oam[e].flags = (oam[e].flags & ~0x0e) | 0x0e;  // palette bits 1-3 -> 7
    }
    memcpy(g_zenv.ppu->oam, &g_ram[0x800], 0x220);
    DumpPpuFrame("/tmp/frame_pal7.rgba");
  }
  Check(g_players[1].is_active, "setup: P2 spawned");

  // ---- Experiment 0: P2 tile CORRECTNESS (not just difference). ----
  // Both players are idle facing down right now (P2 spawned in P1's pose,
  // neither has moved). Same pose => P2's uploaded chars must be IDENTICAL
  // to P1's streamed chars. A wrong DMA source produces garbage that would
  // still pass a mere "tiles differ" check.
  {
    printf("p2 dma addrs:");
    for (int i = 0; i < 10; i++) printf(" %04x", g_p2_dma_addrs[i]);
    printf("\n");
    printf("p1 dma addrs (g_ram): a0=%04x a1=%04x a2=%04x a3=%04x a4=%04x a5=%04x a6=%04x a7=%04x a11=%04x a12=%04x\n",
           *(uint16*)(g_ram+0xACE), *(uint16*)(g_ram+0xAD2), *(uint16*)(g_ram+0xAD6),
           *(uint16*)(g_ram+0xACC), *(uint16*)(g_ram+0xAD0), *(uint16*)(g_ram+0xAD4),
           *(uint16*)(g_ram+0xAC0), *(uint16*)(g_ram+0xAC4),
           *(uint16*)(g_ram+0xAC2), *(uint16*)(g_ram+0xAC6));
    Check(memcmp(&g_zenv.vram[0x4000], &g_zenv.vram[0x4260], 0x40) == 0,
          "exp0: body row1 lo identical (same pose)");
    Check(memcmp(&g_zenv.vram[0x4020], &g_zenv.vram[0x4290], 0x40) == 0,
          "exp0: body row1 hi identical (same pose)");
    Check(memcmp(&g_zenv.vram[0x4040], &g_zenv.vram[0x42b0], 0x20) == 0,
          "exp0: head 8x8 identical (same pose)");
    Check(memcmp(&g_zenv.vram[0x4100], &g_zenv.vram[0x4360], 0x40) == 0,
          "exp0: body row2 lo identical (same pose)");
    Check(memcmp(&g_zenv.vram[0x4120], &g_zenv.vram[0x4390], 0x40) == 0,
          "exp0: body row2 hi identical (same pose)");
    Check(memcmp(&g_zenv.vram[0x4140], &g_zenv.vram[0x43b0], 0x20) == 0,
          "exp0: head row2 8x8 identical (same pose)");
    // Dump the OBJ char region + P2's OAM band for offline tile inspection.
    FILE *f = fopen("/tmp/vram_dump.bin", "wb");
    if (f) { fwrite(g_zenv.vram, 2, 0x8000, f); fclose(f); }
    OamEnt *oam = (OamEnt *)(g_ram + 0x800);
    for (int i = 0; i < 128; i++) {
      if (oam[i].y == 0xf0) continue;
      printf("  oam[%3d] x=%3d y=%3d char=%02x flags=%02x\n",
             i, oam[i].x, oam[i].y, oam[i].charnum, oam[i].flags);
    }
  }

  // ---- Experiment 1: only P1 walks (short, inside any leash radius).
  // P2 must not move at all. ----
  uint16 p1x0 = P1X(), p1y0 = P1Y(), p2x0 = P2X(), p2y0 = P2Y();
  RunFrames(15, B_DOWN, 0);
  RunFrames(15, B_RIGHT, 0);
  Report("exp1 P1 walked");
  Check(P1X() != p1x0 || P1Y() != p1y0, "exp1: P1 moved with P1 input");
  Check(P2X() == p2x0 && P2Y() == p2y0, "exp1: P2 stayed put");
  Check(g_p2_pose_differs_frames > 0, "exp1: P2 tiles differ from P1's (own pose)");

  // ---- Experiment 2: only P2 walks. P1 must not move. ----
  RunFrames(30, 0, 0);
  p1x0 = P1X(); p1y0 = P1Y(); p2x0 = P2X(); p2y0 = P2Y();
  RunFrames(45, 0, B_DOWN);
  RunFrames(45, 0, B_LEFT);
  Report("exp2 P2 walked");
  Check(P2X() != p2x0 || P2Y() != p2y0, "exp2: P2 moved with P2 input");
  Check(P1X() == p1x0 && P1Y() == p1y0, "exp2: P1 stayed put");

  // ---- Experiment 3: both walk in opposite directions. ----
  RunFrames(30, 0, 0);
  p1x0 = P1X(); p2x0 = P2X();
  RunFrames(45, B_LEFT, B_RIGHT);
  Report("exp3 both walked");
  Check((int16)(P1X() - p1x0) < 0, "exp3: P1 went left");
  Check((int16)(P2X() - p2x0) > 0, "exp3: P2 went right");
  Check(g_p2_draw_active, "exp3: P2 draw pass active");

  // ---- Experiment 4: both players leave the house; world transitions. ----
  // With the mutual leash, an idle P2 pins P1 to the shared screen, so both
  // players walk out together (P2 gets the same inputs P1 does — like two
  // people playing side by side).
  RunFrames(30, 0, B_UP);  // move P2 clear of the bottom wall
  // Find the exit: walk down, and when blocked by the wall, sweep sideways
  // along it until the door lets P1 through.
  int walked = 0, stuck = 0, dir_right = 1;
  uint16 last_y = P1Y();
  while (MMI == 7 && walked < 4000) {
    RunFrames(1, B_DOWN, B_DOWN);
    walked++;
    if (P1Y() == last_y) {
      if (++stuck > 20) {
        RunFrames(16, dir_right ? B_RIGHT : B_LEFT, dir_right ? B_RIGHT : B_LEFT);
        walked += 16;
        stuck = 0;
        if (P1X() > 2520) dir_right = 0;
        if (P1X() < 2330) dir_right = 1;
      }
    } else {
      stuck = 0;
      last_y = P1Y();
    }
  }
  RunFrames(300, 0, 0);  // let the outdoor transition finish
  RunFrames(60, B_DOWN, B_DOWN);  // step clear of the doorway corridor
  RunFrames(30, 0, 0);
  Report("exp4 left house");
  Check(MMI == 9, "exp4: world reached overworld");
  Check(abs((int)P2X() - (int)P1X()) <= 160 &&
        abs((int)P2Y() - (int)P1Y()) <= 160, "exp4: P2 came along");
  DumpPpuFrame("/tmp/frame_exit.rgba");
  // Both walk west across the open yard.
  p1x0 = P1X(); p2x0 = P2X();
  RunFrames(40, B_LEFT, B_LEFT);
  Report("exp4 both walked outside");
  DumpPpuFrame("/tmp/frame_outside.rgba");
  Check(P1X() != p1x0, "exp4: P1 moves outside");
  Check(P2X() != p2x0, "exp4: P2 moves outside");
  Check(g_p2_draw_active, "exp4: P2 still drawn outside");

  // ---- Experiment 6: mutual leash — the MOVER is blocked, the other
  // player is NEVER dragged (the old clamp teleported P2 along with P1,
  // pulling it through walls into buildings). Run on the Y axis: the field
  // south of Link's house is open ground. ----
  RunFrames(30, 0, 0);
  {
    // P1 runs south alone: P1 must stop at the boundary; P2 must not move.
    uint16 p2x_pin = P2X(), p2y_pin = P2Y();
    RunFrames(420, B_DOWN, 0);
    int d_after = (int)P1Y() - (int)P2Y();
    Check(d_after <= 96, "exp6: P1 blocked at the leash boundary");
    Check(d_after >= 80, "exp6: P1 actually reached the boundary");
    Check(P2X() == p2x_pin && P2Y() == p2y_pin,
          "exp6: P2 never dragged while P1 runs away");
    // P2 runs south alone past P1: P2 blocked, P1 not dragged.
    RunFrames(30, 0, 0);
    uint16 p1x_pin = P1X(), p1y_pin = P1Y();
    RunFrames(420, 0, B_DOWN);
    int d2 = (int)P2Y() - (int)P1Y();
    Check(d2 <= 96, "exp6: P2 blocked at the leash boundary");
    Check(P1X() == p1x_pin && P1Y() == p1y_pin,
          "exp6: P1 never dragged while P2 runs away");
  }

  // ---- Experiment 6b: leash clamp unit checks (terrain-independent). ----
  // Drive the exported clamp directly with synthetic positions: the mover is
  // clamped to the boundary, the anchor never moves, and a boundary that the
  // anchor has advanced no longer clamps (release). Restores positions after.
  {
    uint16 sx1 = P1X(), sy1 = P1Y(), sx2 = P2X(), sy2 = P2Y();
    // P1 moved beyond the boundary -> clamped back to anchor + 112.
    g_players[0].x_coord = 1130; g_players[0].y_coord = 1000;
    g_players[1].x_coord = 1000; g_players[1].y_coord = 1000;
    PlayerState_SetCurrent(0);
    Multiplayer_LeashConstrainCurrentPlayer();
    Check(g_players[0].x_coord == 1112, "exp6b: mover clamped to boundary");
    Check(g_players[1].x_coord == 1000, "exp6b: anchor untouched by clamp");
    // Anchor advanced -> the same mover position is inside the box (release).
    g_players[0].x_coord = 1130;
    g_players[1].x_coord = 1050;
    Multiplayer_LeashConstrainCurrentPlayer();
    Check(g_players[0].x_coord == 1130, "exp6b: leash releases as anchor advances");
    // P2 as mover, symmetric.
    g_players[1].y_coord = 1100; g_players[1].x_coord = 1050;
    g_players[0].y_coord = 1000; g_players[0].x_coord = 1050;
    g_players[1].y_coord = 1100;
    PlayerState_SetCurrent(1);
    Multiplayer_LeashConstrainCurrentPlayer();
    Check(g_players[1].y_coord == 1096, "exp6b: P2 mover clamped symmetrically");
    PlayerState_SetCurrent(0);
    g_players[0].x_coord = sx1; g_players[0].y_coord = sy1;
    g_players[1].x_coord = sx2; g_players[1].y_coord = sy2;
  }

  // ---- Experiment 7: the P2 magic-gauge black bar is gone. ----
  // The bare liquid column was drawn into HUD cells at column 18, rows 1-4.
  // With it removed, none of those cells may hold magic-liquid tiles.
  {
    uint16 *hud = (uint16 *)(g_ram + 0xC700);  // hud_tile_indices_buffer
    bool clean = true;
    for (int row = 1; row <= 4; row++) {
      uint16 v = hud[18 + row * 32];
      if (v == 0x3cf5 || v == 0x3c5e || v == 0x3c5f ||
          v == 0x3c4c || v == 0x3c4d || v == 0x3c4e)
        clean = false;
    }
    Check(clean, "exp7: no P2 magic bar tiles in the HUD gap");
  }

  // ---- Experiment 5: co-op enemy HP scaling at spawn. ----
  // Run the real spawn-prep for a few species with co-op off vs on and
  // compare. Done after the gameplay experiments since it scribbles over
  // sprite slot 0. (3/2 must match MP_ENEMY_HEALTH_NUM/DEN.)
  {
    uint8 *sprite_type0 = g_ram + 0xE20, *sprite_health0 = g_ram + 0xE50;
    uint8 solo, coop;

    *sprite_type0 = 0x41;  // soldier (regular enemy)
    g_mp_p2_enabled = false; SpritePrep_LoadProperties(0); solo = *sprite_health0;
    g_mp_p2_enabled = true;  SpritePrep_LoadProperties(0); coop = *sprite_health0;
    Check(solo > 0 && coop == solo * 3 / 2 && coop > solo,
          "exp5: regular enemy HP scaled 1.5x in co-op");

    *sprite_type0 = 0x02;  // health 255: unkillable sentinel tier
    g_mp_p2_enabled = false; SpritePrep_LoadProperties(0); solo = *sprite_health0;
    g_mp_p2_enabled = true;  SpritePrep_LoadProperties(0); coop = *sprite_health0;
    Check(solo == 255 && coop == 255, "exp5: 255-HP sentinel untouched");

    *sprite_type0 = 0x0C;  // health 0: one-hit by design
    g_mp_p2_enabled = false; SpritePrep_LoadProperties(0); solo = *sprite_health0;
    g_mp_p2_enabled = true;  SpritePrep_LoadProperties(0); coop = *sprite_health0;
    Check(solo == 0 && coop == 0, "exp5: zero-HP one-hit enemy untouched");

    g_mp_p2_enabled = true;
  }

  printf("char conflicts in P2 region from other systems: %u\n", g_char_conflicts);
  for (int c = 0; c < 0x40; c++)
    if (g_conflict_by_char[c])
      printf("  conflict char %02x: %u times\n", c, g_conflict_by_char[c]);
  printf("frames where P2 pose tiles differed from P1's: %u\n", g_p2_pose_differs_frames);
  printf("frames where P2 packed ext bits were WRONG: %u\n", g_p2_extbits_bad_frames);
  printf("OBJ palette usage by non-player sprites:");
  for (int p = 0; p < 8; p++) printf(" p%d=%u", p, g_palette_usage[p]);
  printf("\n");
  Check(g_p2_extbits_bad_frames == 0, "P2 packed OAM ext bits always correct");
  printf("%s after %d frames total\n", g_failures ? "FAILURES" : "ALL PASS", frame);
  return g_failures != 0;
#else
  printf("vanilla smoke test done after %d frames\n", frame);
  return 0;
#endif
}
