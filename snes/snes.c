
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <stddef.h>

#include "saveload.h"
#include "snes.h"
#include "cpu.h"
#include "apu.h"
#include "dma.h"
#include "ppu.h"
#include "cart.h"
#include "input.h"
#include "../tracing.h"
#include "../snes_regs.h"

static const double apuCyclesPerMaster = (32040 * 32) / (1364 * 262 * 60.0);

static void snes_runCycle(Snes* snes);
static void snes_runCpu(Snes* snes);
static void snes_catchupApu(Snes* snes);
static uint8_t snes_readReg(Snes* snes, uint16_t adr);
static void snes_writeReg(Snes* snes, uint16_t adr, uint8_t val);
static uint8_t snes_rread(Snes* snes, uint32_t adr); // wrapped by read, to set open bus
static int snes_getAccessTime(Snes* snes, uint32_t adr);
void zelda_apu_runcycles();

Snes* snes_init(uint8_t *ram) {
  Snes* snes = (Snes * )malloc(sizeof(Snes));
  snes->ram = ram;
  snes->cpu = cpu_init(snes, 0);
  snes->apu = apu_init();
  snes->dma = dma_init(snes);
  snes->ppu = ppu_init(snes);
  snes->cart = cart_init(snes);
  snes->input1 = input_init(snes);
  snes->input2 = input_init(snes);
  snes->debug_cycles = false;
  snes->disableHpos = false;
  return snes;
}

void snes_free(Snes* snes) {
  cpu_free(snes->cpu);
  apu_free(snes->apu);
  dma_free(snes->dma);
  ppu_free(snes->ppu);
  cart_free(snes->cart);
  input_free(snes->input1);
  input_free(snes->input2);
  free(snes);
}

void snes_saveload(Snes *snes, SaveLoadFunc *func, void *ctx) {
  cpu_saveload(snes->cpu, func, ctx);
  apu_saveload(snes->apu, func, ctx);
  dma_saveload(snes->dma, func, ctx);
  ppu_saveload(snes->ppu, func, ctx);
  cart_saveload(snes->cart, func, ctx);

  func(ctx, &snes->hPos, offsetof(Snes, openBus) + 1 - offsetof(Snes, hPos));
  func(ctx, snes->ram, 0x20000);
  func(ctx, &snes->ramAdr, 4);
    

  snes->disableHpos = false;
}

void snes_reset(Snes* snes, bool hard) {
  cart_reset(snes->cart); // reset cart first, because resetting cpu will read from it (reset vector)
  cpu_reset(snes->cpu);
  apu_reset(snes->apu);
  dma_reset(snes->dma);
  ppu_reset(snes->ppu);
  input_reset(snes->input1);
  input_reset(snes->input2);
  if (hard) memset(snes->ram, 0, 0x20000);
  snes->ramAdr = 0;
  snes->hPos = 0;
  snes->vPos = 0;
  snes->frames = 0;
  snes->cpuCyclesLeft = 52; // 5 reads (8) + 2 IntOp (6)
  snes->cpuMemOps = 0;
  snes->apuCatchupCycles = 0.0;
  snes->hIrqEnabled = false;
  snes->vIrqEnabled = false;
  snes->nmiEnabled = false;
  snes->hTimer = 0x1ff;
  snes->vTimer = 0x1ff;
  snes->inNmi = false;
  snes->inIrq = false;
  snes->inVblank = false;
  memset(snes->portAutoRead, 0, sizeof(snes->portAutoRead));
  snes->autoJoyRead = false;
  snes->autoJoyTimer = 0;
  snes->ppuLatch = false;
  snes->multiplyA = 0xff;
  snes->multiplyResult = 0xfe01;
  snes->divideA = 0xffff;
  snes->divideResult = 0x101;
  snes->fastMem = false;
  snes->openBus = 0;
}

static uint8_t g_last_module;
bool didit;

void snes_runFrame(Snes* snes) {
  // runs a signle frame (run cycles until v/h pos is at 0,0)
  do {
    snes_runCycle(snes);
  } while(!(snes->hPos == 0 && snes->vPos == 0));
}


void snes_printCpuLine(Snes *snes) {
  if (snes->debug_cycles) {
    static FILE *fout;

    if (!fout)
      fout = stdout;
    char line[80];
    getProcessorStateCpu(snes, line);
    fputs(line, fout);
    fprintf(fout, " 0x%x", snes->ppu->vram[0]);
    fputs("\n", fout);
    fflush(fout);
  }
}

void snes_runGfxCycles(Snes *snes) {
  // check for h/v timer irq's
  if(snes->vIrqEnabled && snes->hIrqEnabled) {
    if(snes->vPos == snes->vTimer && snes->hPos == (4 * snes->hTimer)) {
      snes->inIrq = true;
      snes->cpu->irqWanted = true; // request IRQ on CPU
    }
  } else if(snes->vIrqEnabled && !snes->hIrqEnabled) {
    if(snes->vPos == snes->vTimer && snes->hPos == 0) {
      snes->inIrq = true;
      snes->cpu->irqWanted = true; // request IRQ on CPU
    }
  } else if(!snes->vIrqEnabled && snes->hIrqEnabled) {
    if(snes->hPos == (4 * snes->hTimer)) {
      snes->inIrq = true;
      snes->cpu->irqWanted = true; // request IRQ on CPU
    }
  }
  // handle positional stuff
  // TODO: better timing? (especially Hpos)
  if(snes->hPos == 0) {
    // end of hblank, do most vPos-tests
    bool startingVblank = false;
    if(snes->vPos == 0) {
      // end of vblank
      snes->inVblank = false;
      snes->inNmi = false;
      dma_initHdma(snes->dma);
    } else if(snes->vPos == 225) {
      // ask the ppu if we start vblank now or at vPos 240 (overscan)
      startingVblank = !ppu_checkOverscan(snes->ppu);
    } else if(snes->vPos == 240){
      // if we are not yet in vblank, we had an overscan frame, set startingVblank
      if(!snes->inVblank) startingVblank = true;
    }
    if(startingVblank) {
      // if we are starting vblank
      ppu_handleVblank(snes->ppu);
      snes->inVblank = true;
      snes->inNmi = true;
      if(snes->autoJoyRead) {
        // TODO: this starts a little after start of vblank
        snes->autoJoyTimer = 4224;
        snes_doAutoJoypad(snes);
      }
      if(snes->nmiEnabled) {
        snes->cpu->nmiWanted = true; // request NMI on CPU
      }
    }
  } else if(snes->hPos == 512) {
    // render the line halfway of the screen for better compatibility
    if(!snes->inVblank) ppu_runLine(snes->ppu, snes->vPos);
  } else if(snes->hPos == 1024) {
    // start of hblank
    if(!snes->inVblank) dma_doHdma(snes->dma);
  }
  // handle autoJoyRead-timer
  if(snes->autoJoyTimer > 0) snes->autoJoyTimer -= 2;
  // increment position
  // TODO: exact frame timing (line 240 on odd frame is 4 cycles shorter,
  //   even frames in interlace is 1 extra line)
  if (!snes->disableHpos || !(snes->hPos < 536 || snes->hPos >= 576) || snes->hPos == 0 || snes->vPos == 0)
    snes->hPos += 2;
  if(snes->hPos == 1364) {
    snes->hPos = 0;
    snes->vPos++;
    if(snes->vPos == 262) {
      snes->vPos = 0;
      snes->frames++;
      snes_catchupApu(snes); // catch up the apu at the end of the frame
    }
  }
}

static void snes_runCycle(Snes* snes) {
  snes->apuCatchupCycles += apuCyclesPerMaster * 2.0;
  input_cycle(snes->input1);
  input_cycle(snes->input2);
  // if not in dram refresh, if we are busy with hdma/dma, do that, else do cpu cycle
  if(snes->hPos < 536 || snes->hPos >= 576) {
    if(!dma_cycle(snes->dma)) {
      snes_runCpu(snes);
    }
  }
 snes_runGfxCycles(snes);
}

static void snes_runCpu(Snes* snes) {
  if(snes->cpuCyclesLeft == 0) {
    snes->cpuMemOps = 0;
    uint32_t pc = snes->cpu->pc | snes->cpu->k << 16;
    if (snes->debug_cycles) {
      char line[80];
      getProcessorStateCpu(snes, line);
      puts(line);
    }
    int cycles = cpu_runOpcode(snes->cpu);
    snes->cpuCyclesLeft += (cycles - snes->cpuMemOps) * 6;
  }
  snes->cpuCyclesLeft -= 2;
}

static void snes_catchupApu(Snes* snes) {
  int catchupCycles = (int) snes->apuCatchupCycles;
  for(int i = 0; i < catchupCycles; i++) {
    apu_cycle(snes->apu);
    zelda_apu_runcycles();
  }
  snes->apuCatchupCycles -= (double) catchupCycles;
}

void snes_doAutoJoypad(Snes* snes) {
  // TODO: improve? (now calls input_cycle)
  memset(snes->portAutoRead, 0, sizeof(snes->portAutoRead));
  snes->input1->latchLine = true;
  snes->input2->latchLine = true;
  input_cycle(snes->input1); // latches the controllers
  input_cycle(snes->input2);
  snes->input1->latchLine = false;
  snes->input2->latchLine = false;
  for(int i = 0; i < 16; i++) {
    uint8_t val = input_read(snes->input1);
    snes->portAutoRead[0] |= ((val & 1) << (15 - i));
    snes->portAutoRead[2] |= (((val >> 1) & 1) << (15 - i));
    val = input_read(snes->input2);
    snes->portAutoRead[1] |= ((val & 1) << (15 - i));
    snes->portAutoRead[3] |= (((val >> 1) & 1) << (15 - i));
  }
}

uint8_t snes_readBBus(Snes* snes, uint8_t adr) {
  if(adr < 0x40) {
    return ppu_read(snes->ppu, adr);
  }
  if(adr < 0x80) {
    if (kIsOrigEmu)
      snes_catchupApu(snes); // catch up the apu before reading
    else
      apu_cycle(snes->apu);//spc_runOpcode(snes->apu->spc);
    return snes->apu->outPorts[adr & 0x3];
  }
  if(adr == 0x80) {
    uint8_t ret = snes->ram[snes->ramAdr++];
    snes->ramAdr &= 0x1ffff;
    return ret;
  }
  return snes->openBus;
}

void snes_writeBBus(Snes* snes, uint8_t adr, uint8_t val) {
  if(adr < 0x40) {
    ppu_write(snes->ppu, adr, val);
    return;
  }
  if(adr < 0x80) {
    snes_catchupApu(snes); // catch up the apu before writing
    snes->apu->inPorts[adr & 0x3] = val;
    return;
  }
  switch(adr) {
    case 0x80: {
      snes->ram[snes->ramAdr++] = val;
      snes->ramAdr &= 0x1ffff;
      break;
    }
    case 0x81: {
      snes->ramAdr = (snes->ramAdr & 0x1ff00) | val;
      break;
    }
    case 0x82: {
      snes->ramAdr = (snes->ramAdr & 0x100ff) | (val << 8);
      break;
    }
    case 0x83: {
      snes->ramAdr = (snes->ramAdr & 0x0ffff) | ((val & 1) << 16);
      break;
    }
  }
}

static uint8_t snes_readReg(Snes* snes, uint16_t adr) {
  switch(adr) {
    case 0x4210: {
      uint8_t val = 0x2; // CPU version (4 bit)
      val |= snes->inNmi << 7;
      snes->inNmi = false;
      return val | (snes->openBus & 0x70);
    }
    case 0x4211: {
      uint8_t val = snes->inIrq << 7;
      snes->inIrq = false;
      snes->cpu->irqWanted = false;
      return val | (snes->openBus & 0x7f);
    }
    case 0x4212: {
      uint8_t val = (snes->autoJoyTimer > 0);
      val |= (snes->hPos >= 1024) << 6;
      val |= snes->inVblank << 7;
      return val | (snes->openBus & 0x3e);
    }
    case 0x4213: {
      return snes->ppuLatch << 7; // IO-port
    }
    case 0x4214: {
      return snes->divideResult & 0xff;
    }
    case 0x4215: {
      return snes->divideResult >> 8;
    }
    case 0x4216: {
      return snes->multiplyResult & 0xff;
    }
    case 0x4217: {
      return snes->multiplyResult >> 8;
    }
    case 0x4218:
    case 0x421a:
    case 0x421c:
    case 0x421e: {
      return snes->portAutoRead[(adr - 0x4218) / 2] & 0xff;
    }
    case 0x4219:
    case 0x421b:
    case 0x421d:
    case 0x421f: {
      return snes->portAutoRead[(adr - 0x4219) / 2] >> 8;
    }
    default: {
      return snes->openBus;
    }
  }
}

static void snes_writeReg(Snes* snes, uint16_t adr, uint8_t val) {
  switch(adr) {
    case 0x4200: {
      snes->autoJoyRead = val & 0x1;
      if(!snes->autoJoyRead) snes->autoJoyTimer = 0;
      snes->hIrqEnabled = val & 0x10;
      snes->vIrqEnabled = val & 0x20;
      snes->nmiEnabled = val & 0x80;
      if(!snes->hIrqEnabled && !snes->vIrqEnabled) {
        snes->inIrq = false;
        snes->cpu->irqWanted = false;
      }
      // TODO: enabling nmi during vblank with inNmi still set generates nmi
      //   enabling virq (and not h) on the vPos that vTimer is at generates irq (?)
      break;
    }
    case 0x4201: {
      if(!(val & 0x80) && snes->ppuLatch) {
        // latch the ppu
        ppu_read(snes->ppu, 0x37);
      }
      snes->ppuLatch = val & 0x80;
      break;
    }
    case 0x4202: {
      snes->multiplyA = val;
      break;
    }
    case 0x4203: {
      snes->multiplyResult = snes->multiplyA * val;
      break;
    }
    case 0x4204: {
      snes->divideA = (snes->divideA & 0xff00) | val;
      break;
    }
    case 0x4205: {
      snes->divideA = (snes->divideA & 0x00ff) | (val << 8);
      break;
    }
    case 0x4206: {
      if(val == 0) {
        snes->divideResult = 0xffff;
        snes->multiplyResult = snes->divideA;
      } else {
        snes->divideResult = snes->divideA / val;
        snes->multiplyResult = snes->divideA % val;
      }
      break;
    }
    case 0x4207: {
      snes->hTimer = (snes->hTimer & 0x100) | val;
      break;
    }
    case 0x4208: {
      snes->hTimer = (snes->hTimer & 0x0ff) | ((val & 1) << 8);
      break;
    }
    case 0x4209: {
      snes->vTimer = (snes->vTimer & 0x100) | val;
      break;
    }
    case 0x420a: {
      snes->vTimer = (snes->vTimer & 0x0ff) | ((val & 1) << 8);
      break;
    }
    case 0x420b: {
      dma_startDma(snes->dma, val, false);
      break;
    }
    case 0x420c: {
      dma_startDma(snes->dma, val, true);
      break;
    }
    case 0x420d: {
      snes->fastMem = val & 0x1;
      break;
    }
    default: {
      break;
    }
  }
}

static uint8_t snes_rread(Snes* snes, uint32_t adr) {
  uint8_t bank = adr >> 16;
  adr &= 0xffff;
  if((bank & 0x7f) < 0x40 && adr < 0x4380) {
    if(adr < 0x2000) {
      return snes->ram[adr]; // ram mirror
    }
    if(adr >= 0x2100 && adr < 0x2200) {
      return snes_readBBus(snes, adr & 0xff); // B-bus
    }
    if(adr == 0x4016) {
      return input_read(snes->input1) | (snes->openBus & 0xfc);
    }
    if(adr == 0x4017) {
      return input_read(snes->input2) | (snes->openBus & 0xe0) | 0x1c;
    }
    if(adr >= 0x4200 && adr < 0x4220) {
      return snes_readReg(snes, adr); // internal registers
    }
    if(adr >= 0x4300 && adr < 0x4380) {
      return dma_read(snes->dma, adr); // dma registers
    }
  } else if ((bank & ~1) == 0x7e) {
    return snes->ram[((bank & 1) << 16) | adr]; // ram
  }

  // read from cart
  return cart_read(snes->cart, bank, adr);
}

int g_bp_addr = 0;

void PrintState();
void snes_write(Snes* snes, uint32_t adr, uint8_t val) {
  snes->openBus = val;
  uint8_t bank = adr >> 16;
  adr &= 0xffff;
  if(bank == 0x7e || bank == 0x7f) {
    if ((adr & 0xffff) == g_bp_addr && g_bp_addr) {
      printf("@0x%x: Writing1 0x%X to 0x%x (frame %d) %.2x %.2x %.2x\n", snes->cpu->k * 65536 + snes->cpu->pc, val, adr & 0xffff, snes->ram[0x1a],
        snes->ram[snes->cpu->sp+1], snes->ram[snes->cpu->sp+2], snes->ram[snes->cpu->sp+3]);
    }

    snes->ram[((bank & 1) << 16) | adr] = val; // ram
  } else if(bank < 0x40 || (bank >= 0x80 && bank < 0xc0)) {
    if (adr < 0x2000) {
      if ((adr & 0xffff) == g_bp_addr && g_bp_addr) {
        printf("@0x%x: Writing2 0x%X to 0x%x (frame %d) %.2x %.2x %.2x\n", snes->cpu->k * 65536 + snes->cpu->pc, val, adr & 0xffff, snes->ram[0x1a],
          snes->ram[snes->cpu->sp+1], snes->ram[snes->cpu->sp+2], snes->ram[snes->cpu->sp+3]);
      }

      snes->ram[adr] = val; // ram mirror
    } else if(adr >= 0x2100 && adr < 0x2200) {
      snes_writeBBus(snes, adr & 0xff, val); // B-bus
    } else if(adr == 0x4016) {
      snes->input1->latchLine = val & 1;
      snes->input2->latchLine = val & 1;
    } else if(adr >= 0x4200 && adr < 0x4220) {
      snes_writeReg(snes, adr, val); // internal registers
    } else if(adr >= 0x4300 && adr < 0x4380) {
      dma_write(snes->dma, adr, val); // dma registers
    }
  }
  cart_write(snes->cart, bank, adr, val);
  // write to cart
}

static int snes_getAccessTime(Snes* snes, uint32_t adr) {
  // optimization
  return 6;

  uint8_t bank = adr >> 16;
  adr &= 0xffff;
  if(bank >= 0x40 && bank < 0x80) {
    return 8; // slow
  }
  if(bank >= 0xc0) {
    return snes->fastMem ? 6 : 8; // depends on setting
  }
  // banks 0x00-0x3f and 0x80-0xcf
  if(adr < 0x2000) {
    return 8; // slow
  }
  if(adr < 0x4000) {
    return 6; // fast
  }
  if(adr < 0x4200) {
    return 12; // extra slow
  }
  if(adr < 0x6000) {
    return 6; // fast
  }
  if(adr < 0x8000) {
    return 8; // slow
  }
  // 0x8000-0xffff
  return (snes->fastMem && bank >= 0x80) ? 6 : 8; // depends on setting in banks 80+
}

uint8_t snes_read(Snes* snes, uint32_t adr) {
  uint8_t val = snes_rread(snes, adr);
  snes->openBus = val;
  return val;
}

uint8_t snes_cpuRead(Snes* snes, uint32_t adr) {
  snes->cpuMemOps++;
  snes->cpuCyclesLeft += snes_getAccessTime(snes, adr);
  return snes_read(snes, adr);
}

void snes_cpuWrite(Snes* snes, uint32_t adr, uint8_t val) {
  snes->cpuMemOps++;
  snes->cpuCyclesLeft += snes_getAccessTime(snes, adr);
  snes_write(snes, adr, val);
}

// debugging

void snes_debugCycle(Snes* snes, bool* cpuNext, bool* spcNext) {
  // runs a normal cycle, catches up the apu, then looks if the next cycle will execute a CPU and/or a SPC opcode
  snes_runCycle(snes);
  snes_catchupApu(snes);
  if(snes->dma->hdmaTimer > 0 || snes->dma->dmaBusy || (snes->hPos >= 536 && snes->hPos < 576)) {
    *cpuNext = false;
  } else {
    *cpuNext = snes->cpuCyclesLeft == 0;
  }
  if(snes->apuCatchupCycles + (apuCyclesPerMaster * 2.0) >= 1.0) {
    // we will run a apu cycle next call, see if it also starts a opcode
    *spcNext = snes->apu->cpuCyclesLeft == 0;
  } else {
    *spcNext = false;
  }
}
