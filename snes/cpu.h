
#ifndef CPU_H
#define CPU_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "saveload.h"

typedef struct Cpu Cpu;

struct Cpu {
  // reference to memory handler, for reading//writing
  void* mem;
  uint8_t memType; // used to define which type mem is
  // registers
  uint16_t a;
  uint16_t x;
  uint16_t y;
  uint16_t sp;
  uint16_t pc;
  uint16_t dp; // direct page (D)
  uint8_t k; // program bank (PB)
  uint8_t db; // data bank (B)
  // flags
  bool c;
  bool z;
  bool v;
  bool n;
  bool i;
  bool d;
  bool xf;
  bool mf;
  bool e;
  // interrupts
  bool irqWanted;
  bool nmiWanted;
  // power state (WAI/STP)
  bool waiting;
  bool stopped;
  // internal use
  uint8_t cyclesUsed; // indicates how many cycles an opcode used
  uint16_t spBreakpoint;
  bool in_emu;
};

Cpu* cpu_init(void* mem, int memType);
void cpu_free(Cpu* cpu);
void cpu_reset(Cpu* cpu);
int cpu_runOpcode(Cpu* cpu);
void cpu_saveload(Cpu *cpu, SaveLoadFunc *func, void *ctx);
uint8_t cpu_getFlags(Cpu *cpu);
void cpu_setFlags(Cpu *cpu, uint8_t val);

#endif
