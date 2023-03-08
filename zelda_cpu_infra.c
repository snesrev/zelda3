// This file handles running zelda through the emulated cpu.
// It defines the runtime environment for the emulated side-by-side state.
// It should be possible to build and run the game without this file
#include "zelda_cpu_infra.h"
#include "zelda_rtl.h"
#include "variables.h"
#include "spc_player.h"
#include "snes/snes.h"
#include "snes/snes_regs.h"
#include "snes/cpu.h"
#include "snes/cart.h"
#include "snes/tracing.h"

Snes *g_snes;
Cpu *g_cpu;
uint8 g_emulated_ram[0x20000];

static void PatchRom(uint8 *rom);

uint8 *GetPtr(uint32 addr) {
  Cart *cart = g_snes->cart;
  return &cart->rom[(((addr >> 16) << 15) | (addr & 0x7fff)) & (cart->romSize - 1)];
}

uint8 *GetCartRamPtr(uint32 addr) {
  Cart *cart = g_snes->cart;
  return &cart->ram[addr];
}

typedef struct Snapshot {
  uint16 a, x, y, sp, dp, pc;
  uint8 k, db, flags;
  uint8 ram[0x20000];
  uint16 vram[0x8000];
  uint16 sram[0x2000];
} Snapshot;

static Snapshot g_snapshot_mine, g_snapshot_theirs, g_snapshot_before;

static void MakeSnapshot(Snapshot *s) {
  Cpu *c = g_cpu;
  s->a = c->a, s->x = c->x, s->y = c->y;
  s->sp = c->sp, s->dp = c->dp, s->db = c->db;
  s->pc = c->pc, s->k = c->k;
  s->flags = cpu_getFlags(c);
  memcpy(s->ram, g_snes->ram, 0x20000);
  memcpy(s->sram, g_snes->cart->ram, g_snes->cart->ramSize);
  memcpy(s->vram, g_snes->ppu->vram, sizeof(uint16) * 0x8000);
  memcpy(s->ram + 0x1DBA0, s->ram + 0x1B00, 224 * 2);  // hdma_table (partial)
}

static void MakeMySnapshot(Snapshot *s) {
  memcpy(s->ram, g_zenv.ram, 0x20000);
  memcpy(s->sram, g_zenv.sram, 0x2000);
  memcpy(s->vram, g_zenv.ppu->vram, sizeof(uint16) * 0x8000);
  memcpy(s->ram + 0x1B00, s->ram + 0x1DBA0, 224 * 2);  // hdma_table (partial)
}

static void RestoreMySnapshot(Snapshot *s) {
  memcpy(g_zenv.ram, s->ram, 0x20000);
  memcpy(g_zenv.sram, s->sram, 0x2000);
  memcpy(g_zenv.ppu->vram, s->vram, sizeof(uint16) * 0x8000);
}

static void RestoreSnapshot(Snapshot *s) {
  Cpu *c = g_cpu;

  c->a = s->a, c->x = s->x, c->y = s->y;
  c->sp = s->sp, c->dp = s->dp, c->db = s->db;
  c->pc = s->pc, c->k = s->k;
  cpu_setFlags(c, s->flags);
  memcpy(g_snes->ram, s->ram, 0x20000);
  memcpy(g_snes->cart->ram, s->sram, g_snes->cart->ramSize);
  memcpy(g_snes->ppu->vram, s->vram, sizeof(uint16) * 0x8000);
}

static bool g_fail;

// b is mine, a is theirs
static void VerifySnapshotsEq(Snapshot *b, Snapshot *a, Snapshot *prev) {
  memcpy(b->ram, a->ram, 16);
  b->ram[0xfa1] = a->ram[0xfa1];
  b->ram[0x72] = a->ram[0x72];
  b->ram[0x73] = a->ram[0x73];
  b->ram[0x74] = a->ram[0x74];
  b->ram[0x75] = a->ram[0x75];
  b->ram[0xb7] = a->ram[0xb7];
  b->ram[0xb8] = a->ram[0xb8];
  b->ram[0xb9] = a->ram[0xb9];
  b->ram[0xba] = a->ram[0xba];
  b->ram[0xbb] = a->ram[0xbb];
  b->ram[0xbd] = a->ram[0xbd];
  b->ram[0xbe] = a->ram[0xbe];
  b->ram[0xc8] = a->ram[0xc8];
  b->ram[0xc9] = a->ram[0xc9];
  b->ram[0xca] = a->ram[0xca];
  b->ram[0xcb] = a->ram[0xcb];
  b->ram[0xcc] = a->ram[0xcc];
  b->ram[0xcd] = a->ram[0xcd];
  b->ram[0xa0] = a->ram[0xa0];
  b->ram[0x128] = a->ram[0x128];  // irq_flag
  b->ram[0x463] = a->ram[0x463];  // which_staircase_index_padding

  // c code is authoritative
  WORD(a->ram[0x1f0a]) = WORD(b->ram[0x1f0a]);

  memcpy(&b->ram[0x1f0d], &a->ram[0x1f0d], 0x3f - 0xd);
  memcpy(b->ram + 0x138, a->ram + 0x138, 256 - 0x38); // copy the stack over

  memcpy(a->ram + 0x1cc0, b->ram + 0x1cc0, 2);  // some leftover stuff in hdma table
  memcpy(a->ram + 0x1dd60, b->ram + 0x1dd60, 16 * 2);  // some leftover stuff in hdma table

  memcpy(a->ram + 0x1db20, b->ram + 0x1db20, 64 * 2);  // msu
  a->ram[0x654] = b->ram[0x654];  // msu_volume

  memcpy(a->ram + 0x1CDD, b->ram + 0x1CDD, 2);  // dialogue_msg_src_offs
  
  if (memcmp(b->ram, a->ram, 0x20000)) {
    fprintf(stderr, "@%d: Memory compare failed (mine != theirs, prev):\n", frame_counter);
    int j = 0;
    for (size_t i = 0; i < 0x20000; i++) {
      if (a->ram[i] != b->ram[i]) {
        if (++j < 128) {
          if ((i&1) == 0 && a->ram[i + 1] != b->ram[i + 1]) {
            fprintf(stderr, "0x%.6X: %.4X != %.4X (%.4X)\n", (int)i, 
              WORD(b->ram[i]), WORD(a->ram[i]), WORD(prev->ram[i]));
            i++, j++;
          } else {
            fprintf(stderr, "0x%.6X: %.2X != %.2X (%.2X)\n", (int)i, b->ram[i], a->ram[i], prev->ram[i]);
          }
        }
      }
    }
    if (j)
      g_fail = true;
    fprintf(stderr, "  total of %d failed bytes\n", (int)j);
  }

  if (memcmp(b->sram, a->sram, 0x2000)) {
    fprintf(stderr, "@%d: SRAM compare failed (mine != theirs, prev):\n", frame_counter);
    int j = 0;
    for (size_t i = 0; i < 0x2000; i++) {
      if (a->sram[i] != b->sram[i]) {
        if (++j < 128) {
          if ((i&1) == 0 && a->sram[i + 1] != b->sram[i + 1]) {
            fprintf(stderr, "0x%.6X: %.4X != %.4X (%.4X)\n", (int)i, 
              WORD(b->sram[i]), WORD(a->sram[i]), WORD(prev->sram[i]));
            i++, j++;
          } else {
            fprintf(stderr, "0x%.6X: %.2X != %.2X (%.2X)\n", (int)i, b->sram[i], a->sram[i], prev->sram[i]);
          }
        }
      }
    }
    if (j)
      g_fail = true;
    fprintf(stderr, "  total of %d failed bytes\n", (int)j);
  }

  if (memcmp(b->vram, a->vram, sizeof(uint16) * 0x8000)) {
    fprintf(stderr, "@%d: VRAM compare failed (mine != theirs, prev):\n", frame_counter);
    for (size_t i = 0, j = 0; i < 0x8000; i++) {
      if (a->vram[i] != b->vram[i]) {
        fprintf(stderr, "0x%.6X: %.4X != %.4X (%.4X)\n", (int)i, b->vram[i], a->vram[i], prev->vram[i]);
        g_fail = true;
        if (++j >= 16)
          break;
      }
    }
  }
}

uint8_t *RomByte(Cart *cart, uint32_t addr) {
  return &cart->rom[(((addr >> 16) << 15) | (addr & 0x7fff)) & (cart->romSize - 1)];
}

bool g_calling_asm_from_c;

void HookedFunctionRts(int is_long) {
  if (g_calling_asm_from_c) {
    g_calling_asm_from_c = false;
    return;
  }
  assert(0);
}

void RunEmulatedFunc(uint32 pc, uint16 a, uint16 x, uint16 y, bool mf, bool xf, int b, int whatflags) {
  g_snes->debug_cycles = 1;
  RunEmulatedFuncSilent(pc, a, x, y, mf, xf, b, whatflags | 2);
  g_snes->debug_cycles = 0;
}

void RunEmulatedFuncSilent(uint32 pc, uint16 a, uint16 x, uint16 y, bool mf, bool xf, int b, int whatflags) {
  uint16 org_sp = g_cpu->sp;
  uint16 org_pc = g_cpu->pc;
  uint8 org_b = g_cpu->db;
  uint8 org_dp = g_cpu->dp;
  if (b != -1)
    g_cpu->db = b >= 0 ? b : pc >> 16;
  if (b == -3)
    g_cpu->dp = 0x1f00;

  static uint8 *rambak;
  if (rambak == 0) rambak = (uint8 *)malloc(0x20000);
  memcpy(rambak, g_emulated_ram, 0x20000);
  memcpy(g_emulated_ram, g_ram, 0x20000);

  if (whatflags & 2)
    g_emulated_ram[0x1ffff] = 0x67;

  g_cpu->a = a;
  g_cpu->x = x;
  g_cpu->y = y;
  g_cpu->spBreakpoint = g_cpu->sp;
  g_cpu->k = (pc >> 16);
  g_cpu->pc = (pc & 0xffff);
  g_cpu->mf = mf;
  g_cpu->xf = xf;
  g_calling_asm_from_c = true;
  while (g_calling_asm_from_c) {
    if (g_snes->debug_cycles) {
      char line[80];
      getProcessorStateCpu(g_snes, line);
      puts(line);
    }
    cpu_runOpcode(g_cpu);
    while (g_snes->dma->dmaBusy)
      dma_doDma(g_snes->dma);

    if (whatflags & 1) {
/*      if (apu_debugging == 2 && g_snes->apu->cpuCyclesLeft == 0) {
        char line[80];
        getProcessorStateSpc(g_snes->apu, line);
        puts(line);
      }*/
//      apu_cycle(g_snes->apu);
    }
  }
  g_cpu->dp = org_dp;
  g_cpu->sp = org_sp;
  g_cpu->db = org_b;
  g_cpu->pc = org_pc;

  memcpy(g_ram, g_emulated_ram, 0x20000);
  memcpy(g_emulated_ram, rambak, 0x20000);
}

void RunOrigAsmCodeOneLoop(Snes *snes) {
  Cpu *cpu = snes->cpu;
  cpu->a = cpu->x = cpu->y = 0;
  cpu->e = false;
  cpu->irqWanted = cpu->nmiWanted = cpu->waiting = cpu->stopped = 0;
  cpu_setFlags(cpu, 0x30);

  // Run until the wait loop in Interrupt_Reset,
  // Or the polyhedral main function.
  for(int loops = 0;;loops++) {
    snes_printCpuLine(snes);
    cpu_runOpcode(snes->cpu);
    while (snes->dma->dmaBusy)
      dma_doDma(snes->dma);

    uint32_t pc = snes->cpu->k << 16 | snes->cpu->pc;
    if (pc == 0x8034 || pc == 0x9f81d && loops >= 10 || pc == 0x8225 || pc == 0x82D2)
      break;
  }
}

static void RunEmulatedSnesFrame(Snes *snes, int run_what) {
  // First call runs until init
  if (snes->cpu->pc == 0x8000 && snes->cpu->k == 0) {
    RunOrigAsmCodeOneLoop(snes);
    g_emulated_ram[0x12] = 1;
    // Fixup uninitialized variable
    *(uint16*)(g_emulated_ram+0xAE0) = 0xb280;
    *(uint16*)(g_emulated_ram+0xAE2) = 0xb280 + 0x60;
  }

  // Run poly code
  if (run_what & 2) {
    Cpu *cpu = snes->cpu;
    cpu->sp = 0x1f3e;
    cpu->pc = 0xf81d;
    cpu->db = cpu->k = 9;
    cpu->dp = 0x1f00;
    RunOrigAsmCodeOneLoop(snes);
  }
    
  // Run main code
  if (run_what & 1) {
    Cpu *cpu = g_snes->cpu;
    cpu->sp = 0x1ff;
    cpu->pc = 0x8034;
    cpu->k = cpu->dp = cpu->db = 0;
    RunOrigAsmCodeOneLoop(snes);
  }

  snes_doAutoJoypad(snes);

  // animated_tile_vram_addr uninited
  if (snes->ram[0xadd] == 0)
    *(uint16_t*)&snes->ram[0xadc] = 0xa680;

  // In one code path flag_update_hud_in_nmi uses an undefined value
  snes_write(snes, DMAP0, 0x01);
  snes_write(snes, BBAD0, 0x18);

  // Run NMI handler
  Cpu *cpu = g_snes->cpu;
  cpu->sp = 0x1ff;
  cpu->pc = 0x80D9;
  cpu->k = cpu->dp = cpu->db = 0;
  RunOrigAsmCodeOneLoop(snes);
}


// Copy state into the emulator, we can skip dsp/apu because 
// we're not emulating that.
static void EmuSynchronizeWholeState() {
  *g_snes->ppu = *g_zenv.ppu;
  memcpy(g_snes->ram, g_zenv.ram, 0x20000);
  memcpy(g_snes->cart->ram, g_zenv.sram, 0x2000);
  memcpy(g_snes->dma->channel, g_zenv.dma->channel, sizeof(Dma) - offsetof(Dma, channel));

  // todo: this is hacky
  if (animated_tile_data_src == 0)
    cpu_reset(g_snes->cpu);
}

void EmuRunFrameWithCompare(uint16 input_state, int run_what) {
  MakeSnapshot(&g_snapshot_before);
  MakeMySnapshot(&g_snapshot_mine);
  MakeSnapshot(&g_snapshot_theirs);

  // Compare both snapshots before we run the frame, to see they match
  VerifySnapshotsEq(&g_snapshot_mine, &g_snapshot_theirs, &g_snapshot_before);
  if (g_fail) {
    printf("early fail\n");
    assert(0);
    //return turbo;
  }

  // Run orig version then snapshot
again_theirs:
  g_snes->input1->currentState = input_state;
  RunEmulatedSnesFrame(g_snes, run_what);
  MakeSnapshot(&g_snapshot_theirs);

  // Run my version and snapshot
again_mine:
  ZeldaRunFrameInternal(input_state, run_what);

  MakeMySnapshot(&g_snapshot_mine);

  // Compare both snapshots
  VerifySnapshotsEq(&g_snapshot_mine, &g_snapshot_theirs, &g_snapshot_before);

  if (g_fail) {
    g_fail = false;
    if (1) {
      RestoreMySnapshot(&g_snapshot_before);
      //SaveLoadSlot(kSaveLoad_Save, 0);
      if (0)
        goto again_mine;
      RestoreSnapshot(&g_snapshot_before);
      goto again_theirs;
    }
    if (1) {
      MakeSnapshot(&g_snapshot_theirs);
      RestoreMySnapshot(&g_snapshot_theirs);
    }
  }
}


static void PatchRomBP(uint8_t *rom, uint32_t addr) {
  rom[(addr >> 16) << 15 | (addr & 0x7fff)] = 0;
}

static void PatchRomByte(uint8_t *rom, uint32_t addr, uint8 old_value, uint8 value) {
  assert(rom[(addr >> 16) << 15 | (addr & 0x7fff)] == old_value);
  rom[(addr >> 16) << 15 | (addr & 0x7fff)] = value;
}

static void PatchRomWord(uint8_t *rom, uint32_t addr, uint16 old_value, uint16 value) {
  assert(WORD(rom[(addr >> 16) << 15 | (addr & 0x7fff)]) == old_value);
  WORD(rom[(addr >> 16) << 15 | (addr & 0x7fff)]) = value;
}

static void PatchRomArray(uint8_t *rom, uint32_t addr, const uint8 *values, int n) {
  for (int i = 0; i < n; i++) {
    rom[(addr >> 16) << 15 | (addr & 0x7fff)] = values[i];
    addr += 1;
  }
}



static void PatchRom(uint8_t *rom) {
  //  fix a bug with unitialized memory
  {
    uint8_t *p = rom + 0x36434;
    memmove(p, p + 2, 7);
    p[7] = 0xb0;
    p[8] = 0x40 - 7;
  }

  // BufferAndBuildMap16Stripes_Y can read bad memory if int is negative
  if (1) {
    uint8_t *p = rom + 0x10000 - 0x8000;
    int thunk = 0xFF6E;
    uint8_t *tp = p + thunk;

    *tp++ = 0xc0; *tp++ = 0x00; *tp++ = 0x20;
    *tp++ = 0x90; *tp++ = 0x03;
    *tp++ = 0xa9; *tp++ = 0x00; *tp++ = 0x00;
    *tp++ = 0x9d; *tp++ = 0x00; *tp++ = 0x05;
    *tp++ = 0x60;

    p[0xf4a7] = 0x20; p[0xf4a8] = thunk; p[0xf4a9] = thunk >> 8;
    p[0xf4b5] = 0x20; p[0xf4b6] = thunk; p[0xf4b7] = thunk >> 8;

    p[0xf3dd] = 0x20; p[0xf3de] = thunk; p[0xf3df] = thunk >> 8;
    p[0xf3ef] = 0x20; p[0xf3f0] = thunk; p[0xf3f1] = thunk >> 8;
  }

  // Better random numbers
  if (1) {
    // 8D:FFC1                      new_random_gen:
    int new_routine = 0xffc1;
    uint8_t *p = rom + 0x60000, *tp = p + new_routine;

    *tp++ = 0xad; *tp++ = 0xa1; *tp++ = 0x0f;  // mov.b   A, byte_7E0FA1
    *tp++ = 0x18; *tp++ = 0x65; *tp++ = 0x1a;  // add.b   A, frame_counter
    *tp++ = 0x4a;                              // lsr     A
    *tp++ = 0xb0; *tp++ = 0x02;                // jnb     loc_8DFFCC
    *tp++ = 0x49; *tp++ = 0xb8;                // eor.b   A, #0xB8
    *tp++ = 0x8d; *tp++ = 0xa1; *tp++ = 0x0f;  // byte_7E0FA1, A
    *tp++ = 0x18;                              // clc
    *tp++ = 0x6b;                              // retf

    p[0xBA71] = 0x4c; p[0xBA72] = new_routine; p[0xBA73] = new_routine >> 8;
  }

  {

  }

  // Fix so SmashRockPile_fromLift / Overworld_DoMapUpdate32x32_B preserves R2/R0 destroyed
  {
    /*
    .9B:BFA2 A5 00                 mov.w   A, R0
    .9B:BFA4 48                    push    A
    .9B:BFA5 A5 02                 mov.w   A, R2
    .9B:BFA7 48                    push    A
    .9B:C0F1 22 5C AD 02           callf   Overworld_DoMapUpdate32x32_B
    .9B:C048 68                    pop     A
    .9B:C049 85 00                 mov.w   R0, A
    .9B:C04B 68                    pop     A
    .9B:C04C 85 02                 mov.w   R2, A
    */
    uint8_t *tp = rom + 0x6ffd8;
    *tp++ = 0xa5; *tp++ = 0x00; *tp++ = 0x48;
    *tp++ = 0xa5; *tp++ = 0x02; *tp++ = 0x48;
    *tp++ = 0x22; *tp++ = 0x5c; *tp++ = 0xad; *tp++ = 0x02;
    *tp++ = 0xc2; *tp++ = 0x30;
    *tp++ = 0x68; *tp++ = 0x85; *tp++ = 0x02;
    *tp++ = 0x68; *tp++ = 0x85; *tp++ = 0x00;
    *tp++ = 0x6b;

    int target = 0xDFFD8;  // DoorAnim_DoWork2_Preserving

    rom[0xdc0f2] = target;
    rom[0xdc0f3] = target >> 8;
    rom[0xdc0f4] = target >> 16;
  }

  rom[0x2dec7] = 0;  // Fix Uncle_Embark reading bad ram

  rom[0x4be5e] = 0;  // Overlord05_FallingStalfos doesn't initialize the sprite_D memory location

  rom[0xD79A4] = 0;  // 0x1AF9A4: // Lanmola_SpawnShrapnel uses undefined carry value

  rom[0xF0A46] = 0;  // 0x1E8A46 Helmasaur Carry Junk
  rom[0xF0A52] = 0;  // 0x1E8A52 Helmasaur Carry Junk

  rom[0xef9b9] = 0xb9; // TalkingTree_SpitBomb

  rom[0xdf107] = 0xa2;
  rom[0xdf108] = 0x03;
  rom[0xdf109] = 0x6b;  // Palette_AgahnimClone destoys X


  rom[0x4a966] = 0;  // Follower_AnimateMovement_preserved

  PatchRomBP(rom, 0x1de0e5);
  PatchRomBP(rom, 0x6d0b6);
  PatchRomBP(rom, 0x6d0c6);

  PatchRomBP(rom, 0x1d8f29); // adc instead of add
  PatchRomBP(rom, 0x1DDBD3); // adc instead of add
  PatchRomBP(rom, 0x1DF856); // adc instead of add
  PatchRomBP(rom, 0x1E88DA); // adc instead of add

  PatchRomBP(rom, 0x06ED0B);

  PatchRomBP(rom, 0x1dc812); // adc instead of add

  PatchRomBP(rom, 0x9b46c); // adc instead of add
  PatchRomBP(rom, 0x9b478); // adc instead of add

  PatchRomBP(rom, 0x9B468);  // sbc
  PatchRomBP(rom, 0x9B46A);
  PatchRomBP(rom, 0x9B474);
  PatchRomBP(rom, 0x9B476);
  PatchRomBP(rom, 0x9B60C);

  PatchRomBP(rom, 0x8f708); // don't init scratch_c

  PatchRomBP(rom, 0x1DCDEB); // y is destroyed earlier, restore it..

  PatchRomBP(rom, 0x7B269);  // Link_APress_LiftCarryThrow oob

  // Smithy_Frog doesn't save X
  memmove(rom + 0x332b8, rom + 0x332b7, 4); rom[0x332b7] = 0xfa;

  // This needs to be here because the ancilla code reads
  // from the apu and we don't want to make the core code
  // dependent on the apu timings, so relocated this value
  // to 0x648.
  rom[0x443fe] = 0x48; rom[0x443ff] = 0x6;
  rom[0x44607] = 0x48; rom[0x44608] = 0x6;

  // AncillaAdd_AddAncilla_Bank09 destroys R14
  rom[0x49d0c] = 0xda; rom[0x49d0d] = 0xfa; 
  rom[0x49d0f] = 0xda; rom[0x49d10] = 0xfa; 

  // Prevent LoadSongBank from executing in the rom because it hangs
  rom[0x888] = 0x60;

  // CleanUpAndPrepDesertPrayerHDMA clearing too much
  PatchRomWord(rom, 0x2C7E5 + 1, 0x1df, 0x1cf);

  // Merge ancilla_arr23 with boomerang_arr1 because they're only 3 bytes long,
  // and boomerang might get allocated in slot 4.
  PatchRomByte(rom, 0x9816C, 0xd2, 0xCF);
  PatchRomByte(rom, 0xffdeb, 0xd2, 0xCF);
  PatchRomByte(rom, 0xffdee, 0xd2, 0xCF);
  PatchRomByte(rom, 0xffdf7, 0xd2, 0xCF);
  PatchRomByte(rom, 0xffdfa, 0xd2, 0xCF);

  // Relocate the door debris variables so they become 5 entries each (they were 2 before).
  static const int kDoorDebrisX_Uses[] = {0x1CFC6, 0x1d29d, 0x89794, 0x897a3, 0x8a0a1, 0x8edca, 0x99aa6};
  for (int i = 0; i < countof(kDoorDebrisX_Uses); i++) PatchRomWord(rom, kDoorDebrisX_Uses[i] + 1, 0x3b6, 0x728);
  static const int kDoorDebrisX1_Uses[] = { 0x89797, 0x897A6 };
  for (int i = 0; i < countof(kDoorDebrisX1_Uses); i++) PatchRomWord(rom, kDoorDebrisX1_Uses[i] + 1, 0x3b7, 0x729);
  static const int kDoorDebrisY_Uses[] = { 0x1CFD7, 0x1D2AE, 0x8A099, 0x8EDC5, 0x99AA1 };
  for (int i = 0; i < countof(kDoorDebrisY_Uses); i++) PatchRomWord(rom, kDoorDebrisY_Uses[i] + 1, 0x3ba, 0x732);
  static const int kDoorDebrisDir_Uses[] = { 0x1CFB2, 0x1D2BA, 0x8A0B7 };
  for (int i = 0; i < countof(kDoorDebrisDir_Uses); i++) PatchRomWord(rom, kDoorDebrisDir_Uses[i] + 1, 0x3be, 0x73c);
  static const int ancilla_arr26_Uses[] = { 0x89fb9, 0x89fc0, 0x98157, 0x99c49 };
  for (int i = 0; i < countof(ancilla_arr26_Uses); i++) PatchRomWord(rom, ancilla_arr26_Uses[i] + 1, 0x3c0, 0x741);
  static const int ancilla_arr25_Uses[] = { 0x89fc3, 0x89fc6, 0x8a0ae, 0x8ab7c, 0x8aba7, 0x8abb6, 0x8ae92, 0x8bae2, 0x8baff, 0x8f429, 0x98148, 0x98e0a, 0x98ebc, 0x9920a, 0x9931e, 0x9987f, 0x99c44 };
  for (int i = 0; i < countof(ancilla_arr25_Uses); i++) PatchRomWord(rom, ancilla_arr25_Uses[i] + 1, 0x3c2, 0x746);
  static const int ancilla_arr22_Uses[] = { 0x9816e, 0xffde0, 0xffde7 };
  for (int i = 0; i < countof(ancilla_arr22_Uses); i++) PatchRomWord(rom, ancilla_arr22_Uses[i] + 1, 0x3e1, 0x74b);

  PatchRomWord(rom, 0xddfac + 1, 0xfa85, 0xfa70); // call Hud_Rebuild instead of Hud_UpdateOnly

  // Make sure it's not calling Decomp_spr on tilesheets less than 12
  PatchRomWord(rom, 0xe589, 0xe772, 0xe852);  // call New addr
  static const uint8 kFixSoItWontDecodeSheetLessThan12[] = { 0xc0, 0x0c, 0xb0, 0x02, 0xa0, 0x0c, 0x4c, 0x72, 0xe7 };
  PatchRomArray(rom, 0xe852, kFixSoItWontDecodeSheetLessThan12, sizeof(kFixSoItWontDecodeSheetLessThan12));
}



bool EmuInitialize(uint8 *data, size_t size) {
  PatchRom(data);
  g_snes = snes_init(g_emulated_ram);
  g_cpu = g_snes->cpu;

  ZeldaSetupEmuCallbacks(g_emulated_ram, &EmuRunFrameWithCompare, &EmuSynchronizeWholeState);
  return snes_loadRom(g_snes, data, (int)size);
}
