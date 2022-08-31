// This file handles running zelda through the emulated cpu.

#include "zelda_cpu_infra.h"
#include "zelda_rtl.h"
#include "variables.h"
#include "misc.h"
#include "nmi.h"
#include "poly.h"
#include "attract.h"

#include "snes/snes.h"
#include "snes/cpu.h"
#include "snes/cart.h"
#include "tracing.h"

//#include <vector>

Snes *g_snes;
Cpu *g_cpu;
uint8 g_emulated_ram[0x20000];


void SaveLoadSlot(int cmd, int which);

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
}

static void MakeMySnapshot(Snapshot *s) {
  memcpy(s->ram, g_zenv.ram, 0x20000);
  memcpy(s->sram, g_zenv.sram, 0x2000);
  memcpy(s->vram, g_zenv.ppu->vram, sizeof(uint16) * 0x8000);
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
  memcpy(&b->ram[0x1f0d], &a->ram[0x1f0d], 0x3f - 0xd);
  memcpy(b->ram + 0x138, a->ram + 0x138, 256 - 0x38); // copy the stack over

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

static uint8_t *RomByte(Cart *cart, uint32_t addr) {
  return &cart->rom[(((addr >> 16) << 15) | (addr & 0x7fff)) & (cart->romSize - 1)];
}

void SetSnes(Snes *snes) {
  g_snes = snes;
  g_cpu = snes->cpu;
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
  // Run until the wait loop in Interrupt_Reset,
  // Or the polyhedral main function.
  for(int loops = 0;;loops++) {
    snes_printCpuLine(snes);
    cpu_runOpcode(snes->cpu);
    while (snes->dma->dmaBusy)
      dma_doDma(snes->dma);

    uint32_t pc = snes->cpu->k << 16 | snes->cpu->pc;
    if (pc == 0x8034 || pc == 0x9f81d && loops >= 10)
      break;
  }
}

void RunEmulatedSnesFrame(Snes *snes) {
  // First call runs until init
  if (snes->cpu->pc == 0x8000 && snes->cpu->k == 0) {
    RunOrigAsmCodeOneLoop(snes);
    g_emulated_ram[0x12] = 1;

    // Fixup uninitialized variable
    *(uint16*)(g_emulated_ram+0xAE0) = 0xb280;
    *(uint16*)(g_emulated_ram+0xAE2) = 0xb280 + 0x60;
  }
  RunOrigAsmCodeOneLoop(snes);

  snes_doAutoJoypad(snes);

  // animated_tile_vram_addr uninited
  if (snes->ram[0xadd] == 0)
    *(uint16_t*)&snes->ram[0xadc] = 0xa680;

  // In one code path flag_update_hud_in_nmi uses an undefined value
  snes_write(snes, DMAP0, 0x01);
  snes_write(snes, BBAD0, 0x18);
  snes->cpu->nmiWanted = true;
  for (;;) {
    snes_printCpuLine(snes);
    cpu_runOpcode(snes->cpu);
    while (snes->dma->dmaBusy)
      dma_doDma(snes->dma);

    uint32_t pc = snes->cpu->k << 16 | snes->cpu->pc;
    if (pc == 0x8039 || pc == 0x9f81d)
      break;
  }
}

struct Ppu *GetPpuForRendering() {
  return g_zenv.ppu;
}

Dsp *GetDspForRendering() {
  SpcPlayer_GenerateSamples(g_zenv.player);
  return g_zenv.player->dsp;
}

typedef struct AllocContext {
  uint8* data;
  uint32 size;
} AllocContext;

void saveFunc(void *_ctx, void *data, size_t data_size) {
  AllocContext *ctx = (AllocContext*)_ctx;
  ctx->data = realloc(ctx->data, ctx->size + data_size);
  memcpy(ctx->data + ctx->size, data, data_size);
  ctx->size += data_size;
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


void CopyStateAfterSnapshotRestore(bool is_reset) {
  memcpy(g_zenv.ram, g_snes->ram, 0x20000);
  memcpy(g_zenv.sram, g_snes->cart->ram, g_snes->cart->ramSize);
  memcpy(g_zenv.ppu->vram, &g_snes->ppu->vram, offsetof(Ppu, pixelBuffer) - offsetof(Ppu, vram));
  memcpy(g_zenv.player->ram, g_snes->apu->ram, sizeof(g_snes->apu->ram));

  if (!is_reset) {
    memcpy(g_zenv.player->dsp->ram, g_snes->apu->dsp->ram, sizeof(Dsp) - offsetof(Dsp, ram));
    SpcPlayer_CopyVariablesFromRam(g_zenv.player);
  }

  memcpy(g_zenv.dma->channel, g_snes->dma->channel, sizeof(Dma) - offsetof(Dma, channel));
  
  g_zenv.player->timer_cycles = 0;

  if (!is_reset) {
    // Setup some fake cpu state cause we can't depend on the savegame's
    Cpu *cpu = g_snes->cpu;
    cpu->a = cpu->x = cpu->y = 0;
    cpu->pc = 0x8034;
    cpu->sp = 0x1ff;
    cpu->k = cpu->dp = cpu->db = 0;
    cpu_setFlags(cpu, 0x30);
    cpu->irqWanted = cpu->nmiWanted = cpu->waiting = cpu->stopped = 0;
    cpu->e = false;

    if (thread_other_stack == 0x1f2) {
      cpu->sp = 0x1f3e;
      cpu->pc = 0xf81d;
      cpu->db = cpu->k = 9;
      cpu->dp = 0x1f00;
      static const uint8 kStackInit[] = { 0x82, 0, 0, 0, 0, 0, 0, 0, 0x40, 0xb7, 0xb0, 0x34, 0x80, 0 };
      memcpy(g_snes->ram + 0x1f2, kStackInit, sizeof(kStackInit));
    }
  }
}
void SaveSnesState(AllocContext *ctx) {
  MakeSnapshot(&g_snapshot_before);

  // Copy from my state into the emulator
  memcpy(&g_snes->ppu->vram, g_zenv.ppu->vram, offsetof(Ppu, pixelBuffer) - offsetof(Ppu, vram));
  memcpy(g_snes->ram, g_zenv.ram, 0x20000);
  memcpy(g_snes->cart->ram, g_zenv.sram, 0x2000);
  SpcPlayer_CopyVariablesToRam(g_zenv.player);
  memcpy(g_snes->apu->ram, g_zenv.player->ram, 0x10000);
  memcpy(g_snes->apu->dsp->ram, g_zenv.player->dsp->ram, sizeof(Dsp) - offsetof(Dsp, ram));
  memcpy(g_snes->dma->channel, g_zenv.dma->channel, sizeof(Dma) - offsetof(Dma, channel));

  snes_saveload(g_snes, &saveFunc, ctx);

  RestoreSnapshot(&g_snapshot_before);
}

uint16 last_inputs_ = 0;
uint32 frames_since_last_ = 0;
uint32 total_frames_ = 0;

// For replay
uint32 replay_pos_;
uint32 replay_frame_counter_;
uint32 replay_next_cmd_at_;
uint8 replay_cmd_;
bool replay_mode_ = false;

uint8* log_ = NULL;
uint32 log_size = 0;
uint8* base_snapshot_ = NULL;
uint32 base_snapshot_size = 0;

uint32 RamChecksum() {
  uint64_t cksum = 0, cksum2 = 0;
  for (int i = 0; i < 0x20000; i += 4) {
    cksum += *(uint32 *)&g_ram[i];
    cksum2 += cksum;
  }
  return cksum ^ (cksum >> 32) ^ cksum2 ^ (cksum2 >> 32);
}

void StateRecorderAppendByte(uint8 v) {
  log_ = realloc(log_, log_size + sizeof(v));
  log_[log_size] = v;
  printf("%.2x ", v);
}

void StateRecorderAppendVl(uint32 v) {
  for (; v >= 255; v -= 255)
    StateRecorderAppendByte(255);
  StateRecorderAppendByte(v);
}

void StateRecorderRecordJoypadBit(int command) {
  int frames = frames_since_last_;
  StateRecorderAppendByte(command << 4 | (frames < 15 ? frames : 15));
  if (frames >= 15)
    StateRecorderAppendVl(frames - 15);
  frames_since_last_ = 0;
}

void StateRecorderRecord(uint16 inputs) {
  uint16 diff = inputs ^ last_inputs_;
  if (diff != 0) {
    last_inputs_ = inputs;
    printf("0x%.4x %d: ", diff, frames_since_last_);
    for (int i = 0; i < 12; i++) {
      if ((diff >> i) & 1)
        StateRecorderRecordJoypadBit(i);
    }
    printf("\n");
  }
  frames_since_last_++;
  total_frames_++;
}

void StateRecorderRecordPatchByte(uint32 addr, const uint8 *value, int num) {
  assert(addr < 0x20000);
  printf("%d: PatchByte(0x%x, 0x%x. %d): ", frames_since_last_, addr, *value, num);

  int frames = frames_since_last_;

  int lq = (num - 1) <= 3 ? (num - 1) : 3;

  StateRecorderAppendByte(0xc0 | (frames != 0 ? 1 : 0) | (addr & 0x10000 ? 2 : 0) | lq << 2);
  if (frames != 0)
    StateRecorderAppendVl(frames - 1);
  if (lq == 3)
    StateRecorderAppendVl(num - 1 - 3);

  frames_since_last_ = 0;
  StateRecorderAppendByte(addr >> 8);
  StateRecorderAppendByte(addr);
  for(int i = 0; i < num; i++)
    StateRecorderAppendByte(value[i]);
  printf("\n");
}

void StateRecorderLoad(FILE *f, bool replay_mode) {
  uint32 hdr[8] = { 0 };
  fread(hdr, 8, 4, f);

  assert(hdr[0] == 1);

  total_frames_ = hdr[1];
  log_size = hdr[2];
  log_ = realloc(log_, log_size);
  fread(log_, 1, log_size, f);
  last_inputs_ = hdr[3];
  frames_since_last_ = hdr[4];

  base_snapshot_size = (hdr[5] & 1) ? hdr[6] : 0;
  base_snapshot_ = realloc(base_snapshot_, base_snapshot_size);
  fread(base_snapshot_, 1, base_snapshot_size, f);

  bool is_reset = false;
  replay_mode_ = replay_mode;
  if (replay_mode) {
    replay_next_cmd_at_ = frames_since_last_ = 0;
    last_inputs_ = 0;
    replay_pos_ = 0;
    replay_frame_counter_ = 0;
    replay_cmd_ = 0xff;
    // Load snapshot from |base_snapshot_|, or reset if empty.

    if (base_snapshot_size) {
      LoadFuncState state = { base_snapshot_, base_snapshot_ + base_snapshot_size };
      snes_saveload(g_snes, &loadFunc, &state);
      assert(state.p == state.pend);
    } else {
      snes_reset(g_snes, true);
      SpcPlayer_Initialize(g_zenv.player);
      is_reset = true;
    }
  } else {
    AllocContext context = {malloc(hdr[6]), hdr[6]};
    fread(context.data, 1, context.size, f);

    LoadFuncState state = { context.data, context.data + context.size };
    snes_saveload(g_snes, &loadFunc, &state);
    assert(state.p == state.pend);
  }
  CopyStateAfterSnapshotRestore(is_reset);
}

void StateRecorderSave(FILE *f) {
  uint32 hdr[8] = { 0 };
  AllocContext context = {};
  SaveSnesState(&context);
  assert(base_snapshot_size == 0 || base_snapshot_size == context.size);

  hdr[0] = 1;
  hdr[1] = total_frames_;
  hdr[2] = log_size;
  hdr[3] = last_inputs_;
  hdr[4] = frames_since_last_;
  hdr[5] = base_snapshot_size ? 1 : 0;
  hdr[6] = context.size;

  fwrite(hdr, 8, 4, f);
  fwrite(log_, 1, log_size, f);
  fwrite(base_snapshot_, 1, base_snapshot_size, f);
  fwrite(context.data, 1, context.size, f);
}

void StateRecorderMigrateToBaseSnapshot() {
  printf("Migrating to base snapshot!\n");
  AllocContext context;
  SaveSnesState(&context);
  base_snapshot_ = context.data;
  base_snapshot_size = context.size;

  replay_mode_ = false;
  frames_since_last_ = 0;
  last_inputs_ = 0;
  total_frames_ = 0;
  log_size = 0;
  log_ = realloc(log_, log_size);
}

uint16 StateRecorderReadNextReplayState() {
  assert(replay_mode_);
  while (frames_since_last_ >= replay_next_cmd_at_) {
    frames_since_last_ = 0;
    // Apply next command
    if (replay_cmd_ != 0xff) {
      if (replay_cmd_ < 0xc0) {
        last_inputs_ ^= 1 << (replay_cmd_ >> 4);
      } else if (replay_cmd_ < 0xd0) {
        int nb = 1 + ((replay_cmd_ >> 2) & 3);
        uint8 t;
        if (nb == 4) do {
          nb += t = log_[replay_pos_++];
        } while (t == 255);
        uint32 addr = ((replay_cmd_ >> 1) & 1) << 16;
        addr |= log_[replay_pos_++] << 8;
        addr |= log_[replay_pos_++];
        do {
          g_emulated_ram[addr & 0x1ffff] = g_ram[addr & 0x1ffff] = log_[replay_pos_++];
        } while (addr++, --nb);
      } else {
        assert(0);
      }
    }
    if (replay_pos_ >= log_size) {
      replay_cmd_ = 0xff;
      replay_next_cmd_at_ = 0xffffffff;
      break;
    }
    // Read the next one
    uint8 cmd = log_[replay_pos_++], t;
    int mask = (cmd < 0xc0) ? 0xf : 0x1;
    int frames = cmd & mask;
    if (frames == mask) do {
      frames += t = log_[replay_pos_++];
    } while (t == 255);
    replay_next_cmd_at_ = frames;
    replay_cmd_ = cmd;
  }
  frames_since_last_++;
  // Turn off replay mode after we reached the final frame position
  if (++replay_frame_counter_ >= total_frames_) {
    replay_mode_ = false;
  }
  return last_inputs_;
}

static int frame_ctr;

bool RunOneFrame(Snes *snes, int input_state, bool turbo) {
  frame_ctr++;

  if (kIsOrigEmu) {
    snes_runFrame(snes);
    return false;
  }

  // Either copy state or apply state
  if (replay_mode_) {
    input_state = StateRecorderReadNextReplayState();
  } else {
    StateRecorderRecord(input_state);
    turbo = false;

    // This is whether APUI00 is true or false, this is used by the ancilla code.
    uint8 apui00 = g_zenv.player->port_to_snes[0] != 0;
    if (apui00 != g_ram[0x648]) {
      g_emulated_ram[0x648] = g_ram[0x648] = apui00;
      StateRecorderRecordPatchByte(0x648, &apui00, 1);
    }
  }

  if (snes == NULL) {
    ZeldaRunFrame(input_state);
    return turbo;
  }

  MakeSnapshot(&g_snapshot_before);
  MakeMySnapshot(&g_snapshot_mine);
  MakeSnapshot(&g_snapshot_theirs);

  // Compare both snapshots
  VerifySnapshotsEq(&g_snapshot_mine, &g_snapshot_theirs, &g_snapshot_before);
  if (g_fail) {
    printf("early fail\n");
    return turbo;
  }

again:
  // Run orig version then snapshot
  snes->input1->currentState = input_state;
  RunEmulatedSnesFrame(snes);
  MakeSnapshot(&g_snapshot_theirs);

  // Run my version and snapshot
again_mine:
  ZeldaRunFrame(input_state);
  
  MakeMySnapshot(&g_snapshot_mine);

  // Compare both snapshots
  VerifySnapshotsEq(&g_snapshot_mine, &g_snapshot_theirs, &g_snapshot_before);
  
  if (g_fail) {
    g_fail = false;
    RestoreMySnapshot(&g_snapshot_before);
    // RestoreSnapshot(&g_snapshot_before);
    //SaveLoadSlot(kSaveLoad_Save, 0);
    goto again_mine;
  }

  return turbo;
}

void PatchRomBP(uint8_t *rom, uint32_t addr) {
  rom[(addr >> 16) << 15 | (addr & 0x7fff)] = 0;
}

void PatchRom(uint8_t *rom) {
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
      cmd==kSaveLoad_Save ? "Saving" : cmd==kSaveLoad_Load ? "Loading" : "Replaying", which);

    if (cmd != kSaveLoad_Save)
      StateRecorderLoad(f, cmd == kSaveLoad_Replay);
    else
      StateRecorderSave(f);

    fclose(f);
  }
}
uint32 count_ = 0;
uint32 addr_ = 0;
uint8 vals_[256];
void patch(uint32 addr, uint8 value) {
  if (count_ >= 256 || addr != addr_ + count_) {
    if (count_)
      StateRecorderRecordPatchByte(addr_, vals_, count_);
    addr_ = addr;
    count_ = 0;
  }
  vals_[count_++] = value;

  g_emulated_ram[addr] = g_ram[addr] = value;
}
void PatchCommand(char c) {
  if (c == 'w') {
    patch(0xf372, 80);  // health filler
    patch(0xf373, 80);  // magic filler
    //    b.Patch(0x1FE01, 25);
  } else if (c == 'k') {
    StateRecorderMigrateToBaseSnapshot();
  } else if (c == 'o') {
    patch(0xf36f, 1);
  }
  if (count_)
    StateRecorderRecordPatchByte(addr_, vals_, count_);
}
