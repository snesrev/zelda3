
#ifndef SNES_H
#define SNES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct Snes Snes;

#include "cpu.h"
#include "apu.h"
#include "dma.h"
#include "ppu.h"
#include "cart.h"
#include "input.h"

struct Snes {
  Cpu* cpu;
  Apu* apu;
  Ppu* ppu;
  Dma* dma;
  Cart* cart;
  // input
  bool debug_cycles;
  bool disableHpos;
  Input* input1;
  Input* input2;
  

  // frame timing
  

  uint16_t hPos;
  uint16_t vPos;
  uint32_t frames;
  // cpu handling
  uint8_t cpuCyclesLeft;
  uint8_t cpuMemOps;
  double apuCatchupCycles;
  // nmi / irq
  bool hIrqEnabled;
  bool vIrqEnabled;
  bool nmiEnabled;
  uint16_t hTimer;
  uint16_t vTimer;
  bool inNmi;
  bool inIrq;
  bool inVblank;
  // joypad handling
  uint16_t portAutoRead[4]; // as read by auto-joypad read
  bool autoJoyRead;
  uint16_t autoJoyTimer; // times how long until reading is done
  bool ppuLatch;
  // multiplication/division
  uint8_t multiplyA;
  uint16_t multiplyResult;
  uint16_t divideA;
  uint16_t divideResult;
  // misc
  bool fastMem;
  uint8_t openBus;
  // ram
  uint8_t *ram;
  uint32_t ramAdr;
};

Snes* snes_init(uint8_t *ram);
void snes_free(Snes* snes);
void snes_reset(Snes* snes, bool hard);
// used by dma, cpu
uint8_t snes_readBBus(Snes* snes, uint8_t adr);
void snes_writeBBus(Snes* snes, uint8_t adr, uint8_t val);
uint8_t snes_read(Snes* snes, uint32_t adr);
void snes_write(Snes* snes, uint32_t adr, uint8_t val);
uint8_t snes_cpuRead(Snes* snes, uint32_t adr);
void snes_cpuWrite(Snes* snes, uint32_t adr, uint8_t val);
// debugging
void snes_printCpuLine(Snes *snes);
void snes_doAutoJoypad(Snes *snes);
// snes_other.c functions:

bool snes_loadRom(Snes* snes, uint8_t* data, int length);
void snes_saveload(Snes *snes, SaveLoadFunc *func, void *ctx);

enum {
  kSaveLoad_Save = 0,
  kSaveLoad_Load = 1,
  kSaveLoad_Replay = 2,
};


#endif

