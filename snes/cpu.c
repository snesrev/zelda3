
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <stddef.h>
#include "cpu.h"
#include "snes.h"

static const int cyclesPerOpcode[256] = {
  7, 6, 7, 4, 5, 3, 5, 6, 3, 2, 2, 4, 6, 4, 6, 5,
  2, 5, 5, 7, 5, 4, 6, 6, 2, 4, 2, 2, 6, 4, 7, 5,
  6, 6, 8, 4, 3, 3, 5, 6, 4, 2, 2, 5, 4, 4, 6, 5,
  2, 5, 5, 7, 4, 4, 6, 6, 2, 4, 2, 2, 4, 4, 7, 5,
  6, 6, 2, 4, 7, 3, 5, 6, 3, 2, 2, 3, 3, 4, 6, 5,
  2, 5, 5, 7, 7, 4, 6, 6, 2, 4, 3, 2, 4, 4, 7, 5,
  6, 6, 6, 4, 3, 3, 5, 6, 4, 2, 2, 6, 5, 4, 6, 5,
  2, 5, 5, 7, 4, 4, 6, 6, 2, 4, 4, 2, 6, 4, 7, 5,
  3, 6, 4, 4, 3, 3, 3, 6, 2, 2, 2, 3, 4, 4, 4, 5,
  2, 6, 5, 7, 4, 4, 4, 6, 2, 5, 2, 2, 4, 5, 5, 5,
  2, 6, 2, 4, 3, 3, 3, 6, 2, 2, 2, 4, 4, 4, 4, 5,
  2, 5, 5, 7, 4, 4, 4, 6, 2, 4, 2, 2, 4, 4, 4, 5,
  2, 6, 3, 4, 3, 3, 5, 6, 2, 2, 2, 3, 4, 4, 6, 5,
  2, 5, 5, 7, 6, 4, 6, 6, 2, 4, 3, 3, 6, 4, 7, 5,
  2, 6, 3, 4, 3, 3, 5, 6, 2, 2, 2, 3, 4, 4, 6, 5,
  2, 5, 5, 7, 5, 4, 6, 6, 2, 4, 4, 2, 8, 4, 7, 5
};

static uint8_t cpu_read(Cpu* cpu, uint32_t adr);
static void cpu_write(Cpu* cpu, uint32_t adr, uint8_t val);
static uint8_t cpu_readOpcode(Cpu* cpu);
static uint16_t cpu_readOpcodeWord(Cpu* cpu);
void cpu_setFlags(Cpu* cpu, uint8_t value);
static void cpu_setZN(Cpu* cpu, uint16_t value, bool byte);
static void cpu_doBranch(Cpu* cpu, uint8_t value, bool check);
static uint8_t cpu_pullByte(Cpu* cpu);
static void cpu_pushByte(Cpu* cpu, uint8_t value);
static uint16_t cpu_pullWord(Cpu* cpu);
static void cpu_pushWord(Cpu* cpu, uint16_t value);
static uint16_t cpu_readWord(Cpu* cpu, uint32_t adrl, uint32_t adrh);
static void cpu_writeWord(Cpu* cpu, uint32_t adrl, uint32_t adrh, uint16_t value, bool reversed);
static void cpu_doInterrupt(Cpu* cpu, bool irq);
static void cpu_doOpcode(Cpu* cpu, uint8_t opcode);

// addressing modes and opcode functions not declared, only used after defintions

static uint8_t cpu_read(Cpu* cpu, uint32_t adr) {
  // assume mem is a pointer to a Snes
  return snes_cpuRead((Snes*) cpu->mem, adr);
}

static void cpu_write(Cpu* cpu, uint32_t adr, uint8_t val) {
  // assume mem is a pointer to a Snes
  snes_cpuWrite((Snes*) cpu->mem, adr, val);
}

Cpu* cpu_init(void* mem, int memType) {
  Cpu* cpu = (Cpu * )malloc(sizeof(Cpu));
  cpu->mem = mem;
  cpu->memType = memType;
  return cpu;
}

void cpu_free(Cpu* cpu) {
  free(cpu);
}

void cpu_reset(Cpu* cpu) {
  cpu->a = 0;
  cpu->x = 0;
  cpu->y = 0;
  cpu->sp = 0x100;
  cpu->pc = cpu_read(cpu, 0xfffc) | (cpu_read(cpu, 0xfffd) << 8);
  cpu->dp = 0;
  cpu->k = 0;
  cpu->db = 0;
  cpu->c = false;
  cpu->z = false;
  cpu->v = false;
  cpu->n = false;
  cpu->i = true;
  cpu->d = false;
  cpu->xf = true;
  cpu->mf = true;
  cpu->e = true;
  cpu->irqWanted = false;
  cpu->nmiWanted = false;
  cpu->waiting = false;
  cpu->stopped = false;
  cpu->cyclesUsed = 0;
  cpu->spBreakpoint = 0x0;
  cpu->in_emu = 0;
}

void cpu_saveload(Cpu *cpu, SaveLoadFunc *func, void *ctx) {
  func(ctx, &cpu->a, offsetof(Cpu, cyclesUsed) - offsetof(Cpu, a));
  cpu->spBreakpoint = 0x0;
}

int cpu_runOpcode(Cpu* cpu) {
  cpu->cyclesUsed = 0;
  if(cpu->stopped) return 1;
  if(cpu->waiting) {
    if(cpu->irqWanted || cpu->nmiWanted) {
      cpu->waiting = false;
    }
    return 1;
  }
  // not stopped or waiting, execute a opcode or go to interrupt
  if((!cpu->i && cpu->irqWanted) || cpu->nmiWanted) {
    if (cpu->in_emu) {
      printf("nmi while in emu!\n");
    }

    cpu->cyclesUsed = 7; // interrupt: at least 7 cycles
    if(cpu->nmiWanted) {
      cpu->nmiWanted = false;
      cpu_doInterrupt(cpu, false);
    } else {
      // must be irq
      cpu_doInterrupt(cpu, true);
    }
  } else {
    uint8_t opcode = cpu_readOpcode(cpu);
    cpu->cyclesUsed = cyclesPerOpcode[opcode];
    cpu_doOpcode(cpu, opcode);
  }
  return cpu->cyclesUsed;
}

static uint8_t cpu_readOpcode(Cpu* cpu) {
  return cpu_read(cpu, (cpu->k << 16) | cpu->pc++);
}

static uint16_t cpu_readOpcodeWord(Cpu* cpu) {
  uint8_t low = cpu_readOpcode(cpu);
  return low | (cpu_readOpcode(cpu) << 8);
}

uint8_t cpu_getFlags(Cpu* cpu) {
  uint8_t val = cpu->n << 7;
  val |= cpu->v << 6;
  val |= cpu->mf << 5;
  val |= cpu->xf << 4;
  val |= cpu->d << 3;
  val |= cpu->i << 2;
  val |= cpu->z << 1;
  val |= (uint8_t)cpu->c;
  return val;
}

void cpu_setFlags(Cpu* cpu, uint8_t val) {
  cpu->n = val & 0x80;
  cpu->v = val & 0x40;
  cpu->mf = val & 0x20;
  cpu->xf = val & 0x10;
  cpu->d = val & 8;
  cpu->i = val & 4;
  cpu->z = val & 2;
  cpu->c = val & 1;
  if(cpu->e) {
    cpu->mf = true;
    cpu->xf = true;
    cpu->sp = (cpu->sp & 0xff) | 0x100;
  }
  if(cpu->xf) {
    cpu->x &= 0xff;
    cpu->y &= 0xff;
  }
}

static void cpu_setZN(Cpu* cpu, uint16_t value, bool byte) {
  if(byte) {
    cpu->z = (value & 0xff) == 0;
    cpu->n = value & 0x80;
  } else {
    cpu->z = value == 0;
    cpu->n = value & 0x8000;
  }
}

static void cpu_doBranch(Cpu* cpu, uint8_t value, bool check) {
  if(check) {
    cpu->cyclesUsed++; // taken branch: 1 extra cycle
    cpu->pc += (int8_t) value;
  }
}

static uint8_t cpu_pullByte(Cpu* cpu) {
  cpu->sp++;
  if(cpu->e) cpu->sp = (cpu->sp & 0xff) | 0x100;
  return cpu_read(cpu, cpu->sp);
}

static void cpu_pushByte(Cpu* cpu, uint8_t value) {
  cpu_write(cpu, cpu->sp, value);
  cpu->sp--;
  if(cpu->e) cpu->sp = (cpu->sp & 0xff) | 0x100;
}

static uint16_t cpu_pullWord(Cpu* cpu) {
  uint8_t value = cpu_pullByte(cpu);
  return value | (cpu_pullByte(cpu) << 8);
}

static void cpu_pushWord(Cpu* cpu, uint16_t value) {
  cpu_pushByte(cpu, value >> 8);
  cpu_pushByte(cpu, value & 0xff);
}

static uint16_t cpu_readWord(Cpu* cpu, uint32_t adrl, uint32_t adrh) {
  uint8_t value = cpu_read(cpu, adrl);
  return value | (cpu_read(cpu, adrh) << 8);
}

static void cpu_writeWord(Cpu* cpu, uint32_t adrl, uint32_t adrh, uint16_t value, bool reversed) {
  if(reversed) {
    cpu_write(cpu, adrh, value >> 8);
    cpu_write(cpu, adrl, value & 0xff);
  } else {
    cpu_write(cpu, adrl, value & 0xff);
    cpu_write(cpu, adrh, value >> 8);
  }
}

static void cpu_doInterrupt(Cpu* cpu, bool irq) {
  cpu_pushByte(cpu, cpu->k);
  cpu_pushWord(cpu, cpu->pc);
  cpu_pushByte(cpu, cpu_getFlags(cpu));
  cpu->cyclesUsed++; // native mode: 1 extra cycle
  cpu->i = true;
  cpu->d = false;
  cpu->k = 0;
  if(irq) {
    cpu->pc = cpu_readWord(cpu, 0xffee, 0xffef);
  } else {
    // nmi
    cpu->pc = cpu_readWord(cpu, 0xffea, 0xffeb);
  }
}

// addressing modes

static uint32_t cpu_adrImm(Cpu* cpu, uint32_t* low, bool xFlag) {
  if((xFlag && cpu->xf) || (!xFlag && cpu->mf)) {
    *low = (cpu->k << 16) | cpu->pc++;
    return 0;
  } else {
    *low = (cpu->k << 16) | cpu->pc++;
    return (cpu->k << 16) | cpu->pc++;
  }
}

static uint32_t cpu_adrDp(Cpu* cpu, uint32_t* low) {
  uint8_t adr = cpu_readOpcode(cpu);
  if(cpu->dp & 0xff) cpu->cyclesUsed++; // dpr not 0: 1 extra cycle
  *low = (cpu->dp + adr) & 0xffff;
  return (cpu->dp + adr + 1) & 0xffff;
}

static uint32_t cpu_adrDpx(Cpu* cpu, uint32_t* low) {
  uint8_t adr = cpu_readOpcode(cpu);
  if(cpu->dp & 0xff) cpu->cyclesUsed++; // dpr not 0: 1 extra cycle
  *low = (cpu->dp + adr + cpu->x) & 0xffff;
  return (cpu->dp + adr + cpu->x + 1) & 0xffff;
}

static uint32_t cpu_adrDpy(Cpu* cpu, uint32_t* low) {
  uint8_t adr = cpu_readOpcode(cpu);
  if(cpu->dp & 0xff) cpu->cyclesUsed++; // dpr not 0: 1 extra cycle
  *low = (cpu->dp + adr + cpu->y) & 0xffff;
  return (cpu->dp + adr + cpu->y + 1) & 0xffff;
}

static uint32_t cpu_adrIdp(Cpu* cpu, uint32_t* low) {
  uint8_t adr = cpu_readOpcode(cpu);
  if(cpu->dp & 0xff) cpu->cyclesUsed++; // dpr not 0: 1 extra cycle
  uint16_t pointer = cpu_readWord(cpu, (cpu->dp + adr) & 0xffff, (cpu->dp + adr + 1) & 0xffff);
  *low = (cpu->db << 16) + pointer;
  return ((cpu->db << 16) + pointer + 1) & 0xffffff;
}

static uint32_t cpu_adrIdx(Cpu* cpu, uint32_t* low) {
  uint8_t adr = cpu_readOpcode(cpu);
  if(cpu->dp & 0xff) cpu->cyclesUsed++; // dpr not 0: 1 extra cycle
  uint16_t pointer = cpu_readWord(cpu, (cpu->dp + adr + cpu->x) & 0xffff, (cpu->dp + adr + cpu->x + 1) & 0xffff);
  *low = (cpu->db << 16) + pointer;
  return ((cpu->db << 16) + pointer + 1) & 0xffffff;
}

static uint32_t cpu_adrIdy(Cpu* cpu, uint32_t* low, bool write) {
  uint8_t adr = cpu_readOpcode(cpu);
  if(cpu->dp & 0xff) cpu->cyclesUsed++; // dpr not 0: 1 extra cycle
  uint16_t pointer = cpu_readWord(cpu, (cpu->dp + adr) & 0xffff, (cpu->dp + adr + 1) & 0xffff);
  if(write && (!cpu->xf || ((pointer >> 8) != ((pointer + cpu->y) >> 8)))) cpu->cyclesUsed++;
  // x = 0 or page crossed, with writing opcode: 1 extra cycle
  *low = ((cpu->db << 16) + pointer + cpu->y) & 0xffffff;
  return ((cpu->db << 16) + pointer + cpu->y + 1) & 0xffffff;
}

static uint32_t cpu_adrIdl(Cpu* cpu, uint32_t* low) {
  uint8_t adr = cpu_readOpcode(cpu);
  if(cpu->dp & 0xff) cpu->cyclesUsed++; // dpr not 0: 1 extra cycle
  uint32_t pointer = cpu_readWord(cpu, (cpu->dp + adr) & 0xffff, (cpu->dp + adr + 1) & 0xffff);
  pointer |= cpu_read(cpu, (cpu->dp + adr + 2) & 0xffff) << 16;
  *low = pointer;
  return (pointer + 1) & 0xffffff;
}

static uint32_t cpu_adrIly(Cpu* cpu, uint32_t* low) {
  uint8_t adr = cpu_readOpcode(cpu);
  if(cpu->dp & 0xff) cpu->cyclesUsed++; // dpr not 0: 1 extra cycle
  uint32_t pointer = cpu_readWord(cpu, (cpu->dp + adr) & 0xffff, (cpu->dp + adr + 1) & 0xffff);
  pointer |= cpu_read(cpu, (cpu->dp + adr + 2) & 0xffff) << 16;
  *low = (pointer + cpu->y) & 0xffffff;
  return (pointer + cpu->y + 1) & 0xffffff;
}

static uint32_t cpu_adrSr(Cpu* cpu, uint32_t* low) {
  uint8_t adr = cpu_readOpcode(cpu);
  *low = (cpu->sp + adr) & 0xffff;
  return (cpu->sp + adr + 1) & 0xffff;
}

static uint32_t cpu_adrIsy(Cpu* cpu, uint32_t* low) {
  uint8_t adr = cpu_readOpcode(cpu);
  uint16_t pointer = cpu_readWord(cpu, (cpu->sp + adr) & 0xffff, (cpu->sp + adr + 1) & 0xffff);
  *low = ((cpu->db << 16) + pointer + cpu->y) & 0xffffff;
  return ((cpu->db << 16) + pointer + cpu->y + 1) & 0xffffff;
}

static uint32_t cpu_adrAbs(Cpu* cpu, uint32_t* low) {
  uint16_t adr = cpu_readOpcodeWord(cpu);
  *low = (cpu->db << 16) + adr;
  return ((cpu->db << 16) + adr + 1) & 0xffffff;
}

static uint32_t cpu_adrAbx(Cpu* cpu, uint32_t* low, bool write) {
  uint16_t adr = cpu_readOpcodeWord(cpu);
  if(write && (!cpu->xf || ((adr >> 8) != ((adr + cpu->x) >> 8)))) cpu->cyclesUsed++;
  // x = 0 or page crossed, with writing opcode: 1 extra cycle
  *low = ((cpu->db << 16) + adr + cpu->x) & 0xffffff;
  return ((cpu->db << 16) + adr + cpu->x + 1) & 0xffffff;
}

static uint32_t cpu_adrAby(Cpu* cpu, uint32_t* low, bool write) {
  uint16_t adr = cpu_readOpcodeWord(cpu);
  if(write && (!cpu->xf || ((adr >> 8) != ((adr + cpu->y) >> 8)))) cpu->cyclesUsed++;
  // x = 0 or page crossed, with writing opcode: 1 extra cycle
  *low = ((cpu->db << 16) + adr + cpu->y) & 0xffffff;
  return ((cpu->db << 16) + adr + cpu->y + 1) & 0xffffff;
}

static uint32_t cpu_adrAbl(Cpu* cpu, uint32_t* low) {
  uint32_t adr = cpu_readOpcodeWord(cpu);
  adr |= cpu_readOpcode(cpu) << 16;
  *low = adr;
  return (adr + 1) & 0xffffff;
}

static uint32_t cpu_adrAlx(Cpu* cpu, uint32_t* low) {
  uint32_t adr = cpu_readOpcodeWord(cpu);
  adr |= cpu_readOpcode(cpu) << 16;
  *low = (adr + cpu->x) & 0xffffff;
  return (adr + cpu->x + 1) & 0xffffff;
}

static uint16_t cpu_adrIax(Cpu* cpu) {
  uint16_t adr = cpu_readOpcodeWord(cpu);
  return cpu_readWord(cpu, (cpu->k << 16) | ((adr + cpu->x) & 0xffff), (cpu->k << 16) | ((adr + cpu->x + 1) & 0xffff));
}

// opcode functions

static void cpu_and(Cpu* cpu, uint32_t low, uint32_t high) {
  if(cpu->mf) {
    uint8_t value = cpu_read(cpu, low);
    cpu->a = (cpu->a & 0xff00) | ((cpu->a & value) & 0xff);
  } else {
    cpu->cyclesUsed++; // m = 0: 1 extra cycle
    uint16_t value = cpu_readWord(cpu, low, high);
    cpu->a &= value;
  }
  cpu_setZN(cpu, cpu->a, cpu->mf);
}

static void cpu_ora(Cpu* cpu, uint32_t low, uint32_t high) {
  if(cpu->mf) {
    uint8_t value = cpu_read(cpu, low);
    cpu->a = (cpu->a & 0xff00) | ((cpu->a | value) & 0xff);
  } else {
    cpu->cyclesUsed++; // m = 0: 1 extra cycle
    uint16_t value = cpu_readWord(cpu, low, high);
    cpu->a |= value;
  }
  cpu_setZN(cpu, cpu->a, cpu->mf);
}

static void cpu_eor(Cpu* cpu, uint32_t low, uint32_t high) {
  if(cpu->mf) {
    uint8_t value = cpu_read(cpu, low);
    cpu->a = (cpu->a & 0xff00) | ((cpu->a ^ value) & 0xff);
  } else {
    cpu->cyclesUsed++; // m = 0: 1 extra cycle
    uint16_t value = cpu_readWord(cpu, low, high);
    cpu->a ^= value;
  }
  cpu_setZN(cpu, cpu->a, cpu->mf);
}

static void cpu_adc(Cpu* cpu, uint32_t low, uint32_t high) {
  if(cpu->mf) {
    uint8_t value = cpu_read(cpu, low);
    int result = 0;
    if(cpu->d) {
      result = (cpu->a & 0xf) + (value & 0xf) + cpu->c;
      if(result > 0x9) result = ((result + 0x6) & 0xf) + 0x10;
      result = (cpu->a & 0xf0) + (value & 0xf0) + result;
    } else {
      result = (cpu->a & 0xff) + value + cpu->c;
    }
    cpu->v = (cpu->a & 0x80) == (value & 0x80) && (value & 0x80) != (result & 0x80);
    if(cpu->d && result > 0x9f) result += 0x60;
    cpu->c = result > 0xff;
    cpu->a = (cpu->a & 0xff00) | (result & 0xff);
  } else {
    cpu->cyclesUsed++; // m = 0: 1 extra cycle
    uint16_t value = cpu_readWord(cpu, low, high);
    int result = 0;
    if(cpu->d) {
      result = (cpu->a & 0xf) + (value & 0xf) + cpu->c;
      if(result > 0x9) result = ((result + 0x6) & 0xf) + 0x10;
      result = (cpu->a & 0xf0) + (value & 0xf0) + result;
      if(result > 0x9f) result = ((result + 0x60) & 0xff) + 0x100;
      result = (cpu->a & 0xf00) + (value & 0xf00) + result;
      if(result > 0x9ff) result = ((result + 0x600) & 0xfff) + 0x1000;
      result = (cpu->a & 0xf000) + (value & 0xf000) + result;
    } else {
      result = cpu->a + value + cpu->c;
    }
    cpu->v = (cpu->a & 0x8000) == (value & 0x8000) && (value & 0x8000) != (result & 0x8000);
    if(cpu->d && result > 0x9fff) result += 0x6000;
    cpu->c = result > 0xffff;
    cpu->a = result;
  }
  cpu_setZN(cpu, cpu->a, cpu->mf);
}

static void cpu_sbc(Cpu* cpu, uint32_t low, uint32_t high) {
  if(cpu->mf) {
    uint8_t value = cpu_read(cpu, low) ^ 0xff;
    int result = 0;
    if(cpu->d) {
      result = (cpu->a & 0xf) + (value & 0xf) + cpu->c;
      if(result < 0x10) result = (result - 0x6) & ((result - 0x6 < 0) ? 0xf : 0x1f);
      result = (cpu->a & 0xf0) + (value & 0xf0) + result;
    } else {
      result = (cpu->a & 0xff) + value + cpu->c;
    }
    cpu->v = (cpu->a & 0x80) == (value & 0x80) && (value & 0x80) != (result & 0x80);
    if(cpu->d && result < 0x100) result -= 0x60;
    cpu->c = result > 0xff;
    cpu->a = (cpu->a & 0xff00) | (result & 0xff);
  } else {
    cpu->cyclesUsed++; // m = 0: 1 extra cycle
    uint16_t value = cpu_readWord(cpu, low, high) ^ 0xffff;
    int result = 0;
    if(cpu->d) {
      result = (cpu->a & 0xf) + (value & 0xf) + cpu->c;
      if(result < 0x10) result = (result - 0x6) & ((result - 0x6 < 0) ? 0xf : 0x1f);
      result = (cpu->a & 0xf0) + (value & 0xf0) + result;
      if(result < 0x100) result = (result - 0x60) & ((result - 0x60 < 0) ? 0xff : 0x1ff);
      result = (cpu->a & 0xf00) + (value & 0xf00) + result;
      if(result < 0x1000) result = (result - 0x600) & ((result - 0x600 < 0) ? 0xfff : 0x1fff);
      result = (cpu->a & 0xf000) + (value & 0xf000) + result;
    } else {
      result = cpu->a + value + cpu->c;
    }
    cpu->v = (cpu->a & 0x8000) == (value & 0x8000) && (value & 0x8000) != (result & 0x8000);
    if(cpu->d && result < 0x10000) result -= 0x6000;
    cpu->c = result > 0xffff;
    cpu->a = result;
  }
  cpu_setZN(cpu, cpu->a, cpu->mf);
}

static void cpu_cmp(Cpu* cpu, uint32_t low, uint32_t high) {
  int result = 0;
  if(cpu->mf) {
    uint8_t value = cpu_read(cpu, low) ^ 0xff;
    result = (cpu->a & 0xff) + value + 1;
    cpu->c = result > 0xff;
  } else {
    cpu->cyclesUsed++; // m = 0: 1 extra cycle
    uint16_t value = cpu_readWord(cpu, low, high) ^ 0xffff;
    result = cpu->a + value + 1;
    cpu->c = result > 0xffff;
  }
  cpu_setZN(cpu, result, cpu->mf);
}

static void cpu_cpx(Cpu* cpu, uint32_t low, uint32_t high) {
  int result = 0;
  if(cpu->xf) {
    uint8_t value = cpu_read(cpu, low) ^ 0xff;
    result = (cpu->x & 0xff) + value + 1;
    cpu->c = result > 0xff;
  } else {
    cpu->cyclesUsed++; // x = 0: 1 extra cycle
    uint16_t value = cpu_readWord(cpu, low, high) ^ 0xffff;
    result = cpu->x + value + 1;
    cpu->c = result > 0xffff;
  }
  cpu_setZN(cpu, result, cpu->xf);
}

static void cpu_cpy(Cpu* cpu, uint32_t low, uint32_t high) {
  int result = 0;
  if(cpu->xf) {
    uint8_t value = cpu_read(cpu, low) ^ 0xff;
    result = (cpu->y & 0xff) + value + 1;
    cpu->c = result > 0xff;
  } else {
    cpu->cyclesUsed++; // x = 0: 1 extra cycle
    uint16_t value = cpu_readWord(cpu, low, high) ^ 0xffff;
    result = cpu->y + value + 1;
    cpu->c = result > 0xffff;
  }
  cpu_setZN(cpu, result, cpu->xf);
}

static void cpu_bit(Cpu* cpu, uint32_t low, uint32_t high) {
  if(cpu->mf) {
    uint8_t value = cpu_read(cpu, low);
    uint8_t result = (cpu->a & 0xff) & value;
    cpu->z = result == 0;
    cpu->n = value & 0x80;
    cpu->v = value & 0x40;
  } else {
    cpu->cyclesUsed++; // m = 0: 1 extra cycle
    uint16_t value = cpu_readWord(cpu, low, high);
    uint16_t result = cpu->a & value;
    cpu->z = result == 0;
    cpu->n = value & 0x8000;
    cpu->v = value & 0x4000;
  }
}

static void cpu_lda(Cpu* cpu, uint32_t low, uint32_t high) {
  if(cpu->mf) {
    cpu->a = (cpu->a & 0xff00) | cpu_read(cpu, low);
  } else {
    cpu->cyclesUsed++; // m = 0: 1 extra cycle
    cpu->a = cpu_readWord(cpu, low, high);
  }
  cpu_setZN(cpu, cpu->a, cpu->mf);
}

static void cpu_ldx(Cpu* cpu, uint32_t low, uint32_t high) {
  if(cpu->xf) {
    cpu->x = cpu_read(cpu, low);
  } else {
    cpu->cyclesUsed++; // x = 0: 1 extra cycle
    cpu->x = cpu_readWord(cpu, low, high);
  }
  cpu_setZN(cpu, cpu->x, cpu->xf);
}

static void cpu_ldy(Cpu* cpu, uint32_t low, uint32_t high) {
  if(cpu->xf) {
    cpu->y = cpu_read(cpu, low);
  } else {
    cpu->cyclesUsed++; // x = 0: 1 extra cycle
    cpu->y = cpu_readWord(cpu, low, high);
  }
  cpu_setZN(cpu, cpu->y, cpu->xf);
}

static void cpu_sta(Cpu* cpu, uint32_t low, uint32_t high) {
  if(cpu->mf) {
    cpu_write(cpu, low, cpu->a);
  } else {
    cpu->cyclesUsed++; // m = 0: 1 extra cycle
    cpu_writeWord(cpu, low, high, cpu->a, false);
  }
}

static void cpu_stx(Cpu* cpu, uint32_t low, uint32_t high) {
  if(cpu->xf) {
    cpu_write(cpu, low, cpu->x);
  } else {
    cpu->cyclesUsed++; // x = 0: 1 extra cycle
    cpu_writeWord(cpu, low, high, cpu->x, false);
  }
}

static void cpu_sty(Cpu* cpu, uint32_t low, uint32_t high) {
  if(cpu->xf) {
    cpu_write(cpu, low, cpu->y);
  } else {
    cpu->cyclesUsed++; // x = 0: 1 extra cycle
    cpu_writeWord(cpu, low, high, cpu->y, false);
  }
}

static void cpu_stz(Cpu* cpu, uint32_t low, uint32_t high) {
  if(cpu->mf) {
    cpu_write(cpu, low, 0);
  } else {
    cpu->cyclesUsed++; // m = 0: 1 extra cycle
    cpu_writeWord(cpu, low, high, 0, false);
  }
}

static void cpu_ror(Cpu* cpu, uint32_t low, uint32_t high) {
  bool carry = false;
  int result = 0;
  if(cpu->mf) {
    uint8_t value = cpu_read(cpu, low);
    carry = value & 1;
    result = (value >> 1) | (cpu->c << 7);
    cpu_write(cpu, low, result);
  } else {
    cpu->cyclesUsed += 2; // m = 0: 2 extra cycles
    uint16_t value = cpu_readWord(cpu, low, high);
    carry = value & 1;
    result = (value >> 1) | (cpu->c << 15);
    cpu_writeWord(cpu, low, high, result, true);
  }
  cpu_setZN(cpu, result, cpu->mf);
  cpu->c = carry;
}

static void cpu_rol(Cpu* cpu, uint32_t low, uint32_t high) {
  int result = 0;
  if(cpu->mf) {
    result = (cpu_read(cpu, low) << 1) | (uint8_t)cpu->c;
    cpu->c = result & 0x100;
    cpu_write(cpu, low, result);
  } else {
    cpu->cyclesUsed += 2; // m = 0: 2 extra cycles
    result = (cpu_readWord(cpu, low, high) << 1) | (uint8_t)cpu->c;
    cpu->c = result & 0x10000;
    cpu_writeWord(cpu, low, high, result, true);
  }
  cpu_setZN(cpu, result, cpu->mf);
}

static void cpu_lsr(Cpu* cpu, uint32_t low, uint32_t high) {
  int result = 0;
  if(cpu->mf) {
    uint8_t value = cpu_read(cpu, low);
    cpu->c = value & 1;
    result = value >> 1;
    cpu_write(cpu, low, result);
  } else {
    cpu->cyclesUsed += 2; // m = 0: 2 extra cycles
    uint16_t value = cpu_readWord(cpu, low, high);
    cpu->c = value & 1;
    result = value >> 1;
    cpu_writeWord(cpu, low, high, result, true);
  }
  cpu_setZN(cpu, result, cpu->mf);
}

static void cpu_asl(Cpu* cpu, uint32_t low, uint32_t high) {
  int result = 0;
  if(cpu->mf) {
    result = cpu_read(cpu, low) << 1;
    cpu->c = result & 0x100;
    cpu_write(cpu, low, result);
  } else {
    cpu->cyclesUsed += 2; // m = 0: 2 extra cycles
    result = cpu_readWord(cpu, low, high) << 1;
    cpu->c = result & 0x10000;
    cpu_writeWord(cpu, low, high, result, true);
  }
  cpu_setZN(cpu, result, cpu->mf);
}

static void cpu_inc(Cpu* cpu, uint32_t low, uint32_t high) {
  int result = 0;
  if(cpu->mf) {
    result = cpu_read(cpu, low) + 1;
    cpu_write(cpu, low, result);
  } else {
    cpu->cyclesUsed += 2; // m = 0: 2 extra cycles
    result = cpu_readWord(cpu, low, high) + 1;
    cpu_writeWord(cpu, low, high, result, true);
  }
  cpu_setZN(cpu, result, cpu->mf);
}

static void cpu_dec(Cpu* cpu, uint32_t low, uint32_t high) {
  int result = 0;
  if(cpu->mf) {
    result = cpu_read(cpu, low) - 1;
    cpu_write(cpu, low, result);
  } else {
    cpu->cyclesUsed += 2; // m = 0: 2 extra cycles
    result = cpu_readWord(cpu, low, high) - 1;
    cpu_writeWord(cpu, low, high, result, true);
  }
  cpu_setZN(cpu, result, cpu->mf);
}

static void cpu_tsb(Cpu* cpu, uint32_t low, uint32_t high) {
  if(cpu->mf) {
    uint8_t value = cpu_read(cpu, low);
    cpu->z = ((cpu->a & 0xff) & value) == 0;
    cpu_write(cpu, low, value | (cpu->a & 0xff));
  } else {
    cpu->cyclesUsed += 2; // m = 0: 2 extra cycles
    uint16_t value = cpu_readWord(cpu, low, high);
    cpu->z = (cpu->a & value) == 0;
    cpu_writeWord(cpu, low, high, value | cpu->a, true);
  }
}

static void cpu_trb(Cpu* cpu, uint32_t low, uint32_t high) {
  if(cpu->mf) {
    uint8_t value = cpu_read(cpu, low);
    cpu->z = ((cpu->a & 0xff) & value) == 0;
    cpu_write(cpu, low, value & ~(cpu->a & 0xff));
  } else {
    cpu->cyclesUsed += 2; // m = 0: 2 extra cycles
    uint16_t value = cpu_readWord(cpu, low, high);
    cpu->z = (cpu->a & value) == 0;
    cpu_writeWord(cpu, low, high, value & ~cpu->a, true);
  }
}

void HookedFunctionRts(int is_long);

static void cpu_doOpcode(Cpu* cpu, uint8_t opcode) {
RESTART:
  switch(opcode) {
    case 0x00: { // brk imp
      uint32_t addr = (cpu->k << 16) | cpu->pc;
      switch (addr - 1) {
      case 0x7B269:  // Link_APress_LiftCarryThrow reads OOB
        if ((cpu->x & 0xff) >= 28)
          cpu->pc = 0xB280; // RTS
        opcode = 0xE8;
        goto RESTART;

        // Uncle_AtHome case 3 will read random memory. 
      case 0x5DEC7:
        *(uint8_t *)&cpu->a = cpu_read(cpu, 0x5DEB0 + (cpu->y & 0xff));

        if (cpu_read(cpu, 0xD90 + (cpu->x & 0xff)) == 2) {
          cpu->pc = 0xdeea;
          return;
        }
        cpu->pc += 2;
        return;

      // Overlord_StalfosTrap doesn't initialize the sprite_D memory location
      case 0x9be5e:
        *(uint8_t *)&cpu->a = 224;
        cpu_write(cpu, 0xDE0 + (uint8_t)cpu->y, 0);
        cpu->pc++;
        return;

      case 0x1AF9A4: // Lanmola_SpawnShrapnel uses undefined carry value
        *(uint8_t *)&cpu->a += 4;
        cpu->c = 0;
        cpu->pc++;
        return;

        /*
.9E:8A46 E5 E2                 sbc.b   A, BYTE BG2HOFS_copy2
.9E:8A48 E5 08                 sbc.b   A, R8
.9E:8A4A 69 0C                 adc.b   A, #0xC
.        */
      case 0x1E8A46:  // carry junk
        cpu->a = cpu->a - cpu_read(cpu, 0xe2) - cpu_read(cpu, 8) + 12;
        cpu->pc += 5;
        return;
/*
.9E:8A52 E5 E8                 sbc.b   A, BYTE BG2VOFS_copy2
.9E:8A54 69 08                 adc.b   A, #8
.9E:8A56 E5 09                 sbc.b   A, R9
.9E:8A58 69 08                 adc.b   A, #8
*/
      case 0x1E8A52:  // carry junk
        cpu->a = cpu->a - cpu_read(cpu, 0xe8) + 8 - cpu_read(cpu, 9) + 8;
        cpu->pc += 7;
        return;

      case 0x9a966:  // TAgalong_DrawInner doesn't init scratch_0 / scratch_1
        for(int i = 0; i < 4; i++) cpu_write(cpu, 0x72 + i, 0);
        cpu->pc += 1;
        return;

      case 0x8f708:
        cpu->pc += 0;
        cpu_write(cpu, 0x75, 0);
        goto case_iny_c8;

      case 0x1de0e5:  // GreatCatfish_ConversateThenSubmerge - not carry preserving
        if ((uint8_t)cpu->a >= 160)
          cpu->pc = 0xe164;
        else
          cpu->pc += 1;
        return;

      case 0x6d0b6:
      case 0x6d0c6: { // Sprite_CommonItemPickup - wrong carry chain
        cpu->c = ((uint8_t)cpu->a >= 4);
        cpu->a = cpu->a - 4;
        cpu->pc += 1;
        return;
      }
    
      case 0x1d8f29:
      case 0x1dc812:
      case 0x1DDBD3:
      case 0x1DF856:
      case 0x6ED0B:
      case 0x9b478:
      case 0x9b46c:
        cpu->c = 0;
        goto adc_69;
      
      case 0x1E88DA:
        cpu->c = 0;
        goto adc_65;


      case 0x9B468:
      case 0x9B46A:
      case 0x9B474:
      case 0x9B476:
        cpu->c = 1;
        goto sbc_e5;

      case 0x9B60C:
        cpu->c = 1;
        goto sbc_e9;

      case 0x1DCDEB:
        cpu->y = cpu_read(cpu, 0x0eb0 + (cpu->x & 0xff));  // BC B0 0E              mov.b   Y, sprite_head_dir[X]
        cpu->a = cpu->x;
        return;
      }

      assert(0);
#if 0
      cpu_pushByte(cpu, cpu->k);
      cpu_pushWord(cpu, cpu->pc + 1);
      cpu_pushByte(cpu, cpu_getFlags(cpu));
      cpu->cyclesUsed++; // native mode: 1 extra cycle
      cpu->i = true;
      cpu->d = false;
      cpu->k = 0;
      cpu->pc = cpu_readWord(cpu, 0xffe6, 0xffe7);
#endif
      break;
    }
    case 0x01: { // ora idx
      uint32_t low = 0;
      uint32_t high = cpu_adrIdx(cpu, &low);
      cpu_ora(cpu, low, high);
      break;
    }
    case 0x02: { // cop imm(s)
      cpu_readOpcode(cpu);
      cpu_pushByte(cpu, cpu->k);
      cpu_pushWord(cpu, cpu->pc);
      cpu_pushByte(cpu, cpu_getFlags(cpu));
      cpu->cyclesUsed++; // native mode: 1 extra cycle
      cpu->i = true;
      cpu->d = false;
      cpu->k = 0;
      cpu->pc = cpu_readWord(cpu, 0xffe4, 0xffe5);
      break;
    }
    case 0x03: { // ora sr
      uint32_t low = 0;
      uint32_t high = cpu_adrSr(cpu, &low);
      cpu_ora(cpu, low, high);
      break;
    }
    case 0x04: { // tsb dp
      uint32_t low = 0;
      uint32_t high = cpu_adrDp(cpu, &low);
      cpu_tsb(cpu, low, high);
      break;
    }
    case 0x05: { // ora dp
      uint32_t low = 0;
      uint32_t high = cpu_adrDp(cpu, &low);
      cpu_ora(cpu, low, high);
      break;
    }
    case 0x06: { // asl dp
      uint32_t low = 0;
      uint32_t high = cpu_adrDp(cpu, &low);
      cpu_asl(cpu, low, high);
      break;
    }
    case 0x07: { // ora idl
      uint32_t low = 0;
      uint32_t high = cpu_adrIdl(cpu, &low);
      cpu_ora(cpu, low, high);
      break;
    }
    case 0x08: { // php imp
      cpu_pushByte(cpu, cpu_getFlags(cpu));
      break;
    }
    case 0x09: { // ora imm(m)
      uint32_t low = 0;
      uint32_t high = cpu_adrImm(cpu, &low, false);
      cpu_ora(cpu, low, high);
      break;
    }
    case 0x0a: { // asla imp
      if(cpu->mf) {
        cpu->c = cpu->a & 0x80;
        cpu->a = (cpu->a & 0xff00) | ((cpu->a << 1) & 0xff);
      } else {
        cpu->c = cpu->a & 0x8000;
        cpu->a <<= 1;
      }
      cpu_setZN(cpu, cpu->a, cpu->mf);
      break;
    }
    case 0x0b: { // phd imp
      cpu_pushWord(cpu, cpu->dp);
      break;
    }
    case 0x0c: { // tsb abs
      uint32_t low = 0;
      uint32_t high = cpu_adrAbs(cpu, &low);
      cpu_tsb(cpu, low, high);
      break;
    }
    case 0x0d: { // ora abs
      uint32_t low = 0;
      uint32_t high = cpu_adrAbs(cpu, &low);
      cpu_ora(cpu, low, high);
      break;
    }
    case 0x0e: { // asl abs
      uint32_t low = 0;
      uint32_t high = cpu_adrAbs(cpu, &low);
      cpu_asl(cpu, low, high);
      break;
    }
    case 0x0f: { // ora abl
      uint32_t low = 0;
      uint32_t high = cpu_adrAbl(cpu, &low);
      cpu_ora(cpu, low, high);
      break;
    }
    case 0x10: { // bpl rel
      cpu_doBranch(cpu, cpu_readOpcode(cpu), !cpu->n);
      break;
    }
    case 0x11: { // ora idy(r)
      uint32_t low = 0;
      uint32_t high = cpu_adrIdy(cpu, &low, false);
      cpu_ora(cpu, low, high);
      break;
    }
    case 0x12: { // ora idp
      uint32_t low = 0;
      uint32_t high = cpu_adrIdp(cpu, &low);
      cpu_ora(cpu, low, high);
      break;
    }
    case 0x13: { // ora isy
      uint32_t low = 0;
      uint32_t high = cpu_adrIsy(cpu, &low);
      cpu_ora(cpu, low, high);
      break;
    }
    case 0x14: { // trb dp
      uint32_t low = 0;
      uint32_t high = cpu_adrDp(cpu, &low);
      cpu_trb(cpu, low, high);
      break;
    }
    case 0x15: { // ora dpx
      uint32_t low = 0;
      uint32_t high = cpu_adrDpx(cpu, &low);
      cpu_ora(cpu, low, high);
      break;
    }
    case 0x16: { // asl dpx
      uint32_t low = 0;
      uint32_t high = cpu_adrDpx(cpu, &low);
      cpu_asl(cpu, low, high);
      break;
    }
    case 0x17: { // ora ily
      uint32_t low = 0;
      uint32_t high = cpu_adrIly(cpu, &low);
      cpu_ora(cpu, low, high);
      break;
    }
    case 0x18: { // clc imp
      cpu->c = false;
      break;
    }
    case 0x19: { // ora aby(r)
      uint32_t low = 0;
      uint32_t high = cpu_adrAby(cpu, &low, false);
      cpu_ora(cpu, low, high);
      break;
    }
    case 0x1a: { // inca imp
      if(cpu->mf) {
        cpu->a = (cpu->a & 0xff00) | ((cpu->a + 1) & 0xff);
      } else {
        cpu->a++;
      }
      cpu_setZN(cpu, cpu->a, cpu->mf);
      break;
    }
    case 0x1b: { // tcs imp
      cpu->sp = cpu->a;
      break;
    }
    case 0x1c: { // trb abs
      uint32_t low = 0;
      uint32_t high = cpu_adrAbs(cpu, &low);
      cpu_trb(cpu, low, high);
      break;
    }
    case 0x1d: { // ora abx(r)
      uint32_t low = 0;
      uint32_t high = cpu_adrAbx(cpu, &low, false);
      cpu_ora(cpu, low, high);
      break;
    }
    case 0x1e: { // asl abx
      uint32_t low = 0;
      uint32_t high = cpu_adrAbx(cpu, &low, true);
      cpu_asl(cpu, low, high);
      break;
    }
    case 0x1f: { // ora alx
      uint32_t low = 0;
      uint32_t high = cpu_adrAlx(cpu, &low);
      cpu_ora(cpu, low, high);
      break;
    }
    case 0x20: { // jsr abs
      uint16_t value = cpu_readOpcodeWord(cpu);
      cpu_pushWord(cpu, cpu->pc - 1);
      cpu->pc = value;
      break;
    }
    case 0x21: { // and idx
      uint32_t low = 0;
      uint32_t high = cpu_adrIdx(cpu, &low);
      cpu_and(cpu, low, high);
      break;
    }
    case 0x22: { // jsl abl
      uint16_t value = cpu_readOpcodeWord(cpu);
      uint8_t newK = cpu_readOpcode(cpu);
      cpu_pushByte(cpu, cpu->k);
      cpu_pushWord(cpu, cpu->pc - 1);
      cpu->pc = value;
      cpu->k = newK;
      break;
    }
    case 0x23: { // and sr
      uint32_t low = 0;
      uint32_t high = cpu_adrSr(cpu, &low);
      cpu_and(cpu, low, high);
      break;
    }
    case 0x24: { // bit dp
      uint32_t low = 0;
      uint32_t high = cpu_adrDp(cpu, &low);
      cpu_bit(cpu, low, high);
      break;
    }
    case 0x25: { // and dp
      uint32_t low = 0;
      uint32_t high = cpu_adrDp(cpu, &low);
      cpu_and(cpu, low, high);
      break;
    }
    case 0x26: { // rol dp
      uint32_t low = 0;
      uint32_t high = cpu_adrDp(cpu, &low);
      cpu_rol(cpu, low, high);
      break;
    }
    case 0x27: { // and idl
      uint32_t low = 0;
      uint32_t high = cpu_adrIdl(cpu, &low);
      cpu_and(cpu, low, high);
      break;
    }
    case 0x28: { // plp imp
      cpu_setFlags(cpu, cpu_pullByte(cpu));
      break;
    }
    case 0x29: { // and imm(m)
      uint32_t low = 0;
      uint32_t high = cpu_adrImm(cpu, &low, false);
      cpu_and(cpu, low, high);
      break;
    }
    case 0x2a: { // rola imp
      int result = (cpu->a << 1) | (uint8_t)cpu->c;
      if(cpu->mf) {
        cpu->c = result & 0x100;
        cpu->a = (cpu->a & 0xff00) | (result & 0xff);
      } else {
        cpu->c = result & 0x10000;
        cpu->a = result;
      }
      cpu_setZN(cpu, cpu->a, cpu->mf);
      break;
    }
    case 0x2b: { // pld imp
      cpu->dp = cpu_pullWord(cpu);
      cpu_setZN(cpu, cpu->dp, false);
      break;
    }
    case 0x2c: { // bit abs
      uint32_t low = 0;
      uint32_t high = cpu_adrAbs(cpu, &low);
      cpu_bit(cpu, low, high);
      break;
    }
    case 0x2d: { // and abs
      uint32_t low = 0;
      uint32_t high = cpu_adrAbs(cpu, &low);
      cpu_and(cpu, low, high);
      break;
    }
    case 0x2e: { // rol abs
      uint32_t low = 0;
      uint32_t high = cpu_adrAbs(cpu, &low);
      cpu_rol(cpu, low, high);
      break;
    }
    case 0x2f: { // and abl
      uint32_t low = 0;
      uint32_t high = cpu_adrAbl(cpu, &low);
      cpu_and(cpu, low, high);
      break;
    }
    case 0x30: { // bmi rel
      cpu_doBranch(cpu, cpu_readOpcode(cpu), cpu->n);
      break;
    }
    case 0x31: { // and idy(r)
      uint32_t low = 0;
      uint32_t high = cpu_adrIdy(cpu, &low, false);
      cpu_and(cpu, low, high);
      break;
    }
    case 0x32: { // and idp
      uint32_t low = 0;
      uint32_t high = cpu_adrIdp(cpu, &low);
      cpu_and(cpu, low, high);
      break;
    }
    case 0x33: { // and isy
      uint32_t low = 0;
      uint32_t high = cpu_adrIsy(cpu, &low);
      cpu_and(cpu, low, high);
      break;
    }
    case 0x34: { // bit dpx
      uint32_t low = 0;
      uint32_t high = cpu_adrDpx(cpu, &low);
      cpu_bit(cpu, low, high);
      break;
    }
    case 0x35: { // and dpx
      uint32_t low = 0;
      uint32_t high = cpu_adrDpx(cpu, &low);
      cpu_and(cpu, low, high);
      break;
    }
    case 0x36: { // rol dpx
      uint32_t low = 0;
      uint32_t high = cpu_adrDpx(cpu, &low);
      cpu_rol(cpu, low, high);
      break;
    }
    case 0x37: { // and ily
      uint32_t low = 0;
      uint32_t high = cpu_adrIly(cpu, &low);
      cpu_and(cpu, low, high);
      break;
    }
    case 0x38: { // sec imp
      cpu->c = true;
      break;
    }
    case 0x39: { // and aby(r)
      uint32_t low = 0;
      uint32_t high = cpu_adrAby(cpu, &low, false);
      cpu_and(cpu, low, high);
      break;
    }
    case 0x3a: { // deca imp
      if(cpu->mf) {
        cpu->a = (cpu->a & 0xff00) | ((cpu->a - 1) & 0xff);
      } else {
        cpu->a--;
      }
      cpu_setZN(cpu, cpu->a, cpu->mf);
      break;
    }
    case 0x3b: { // tsc imp
      cpu->a = cpu->sp;
      cpu_setZN(cpu, cpu->a, false);
      break;
    }
    case 0x3c: { // bit abx(r)
      uint32_t low = 0;
      uint32_t high = cpu_adrAbx(cpu, &low, false);
      cpu_bit(cpu, low, high);
      break;
    }
    case 0x3d: { // and abx(r)
      uint32_t low = 0;
      uint32_t high = cpu_adrAbx(cpu, &low, false);
      cpu_and(cpu, low, high);
      break;
    }
    case 0x3e: { // rol abx
      uint32_t low = 0;
      uint32_t high = cpu_adrAbx(cpu, &low, true);
      cpu_rol(cpu, low, high);
      break;
    }
    case 0x3f: { // and alx
      uint32_t low = 0;
      uint32_t high = cpu_adrAlx(cpu, &low);
      cpu_and(cpu, low, high);
      break;
    }
    case 0x40: { // rti imp
      cpu_setFlags(cpu, cpu_pullByte(cpu));
      cpu->cyclesUsed++; // native mode: 1 extra cycle
      cpu->pc = cpu_pullWord(cpu);
      cpu->k = cpu_pullByte(cpu);
      break;
    }
    case 0x41: { // eor idx
      uint32_t low = 0;
      uint32_t high = cpu_adrIdx(cpu, &low);
      cpu_eor(cpu, low, high);
      break;
    }
    case 0x42: { // wdm imm(s)
      cpu_readOpcode(cpu);
      break;
    }
    case 0x43: { // eor sr
      uint32_t low = 0;
      uint32_t high = cpu_adrSr(cpu, &low);
      cpu_eor(cpu, low, high);
      break;
    }
    case 0x44: { // mvp bm
      uint8_t dest = cpu_readOpcode(cpu);
      uint8_t src = cpu_readOpcode(cpu);
      cpu->db = dest;
      cpu_write(cpu, (dest << 16) | cpu->y, cpu_read(cpu, (src << 16) | cpu->x));
      cpu->a--;
      cpu->x--;
      cpu->y--;
      if(cpu->a != 0xffff) {
        cpu->pc -= 3;
      }
      if(cpu->xf) {
        cpu->x &= 0xff;
        cpu->y &= 0xff;
      }
      break;
    }
    case 0x45: { // eor dp
      uint32_t low = 0;
      uint32_t high = cpu_adrDp(cpu, &low);
      cpu_eor(cpu, low, high);
      break;
    }
    case 0x46: { // lsr dp
      uint32_t low = 0;
      uint32_t high = cpu_adrDp(cpu, &low);
      cpu_lsr(cpu, low, high);
      break;
    }
    case 0x47: { // eor idl
      uint32_t low = 0;
      uint32_t high = cpu_adrIdl(cpu, &low);
      cpu_eor(cpu, low, high);
      break;
    }
    case 0x48: { // pha imp
      if(cpu->mf) {
        cpu_pushByte(cpu, cpu->a);
      } else {
        cpu->cyclesUsed++; // m = 0: 1 extra cycle
        cpu_pushWord(cpu, cpu->a);
      }
      break;
    }
    case 0x49: { // eor imm(m)
      uint32_t low = 0;
      uint32_t high = cpu_adrImm(cpu, &low, false);
      cpu_eor(cpu, low, high);
      break;
    }
    case 0x4a: { // lsra imp
      cpu->c = cpu->a & 1;
      if(cpu->mf) {
        cpu->a = (cpu->a & 0xff00) | ((cpu->a >> 1) & 0x7f);
      } else {
        cpu->a >>= 1;
      }
      cpu_setZN(cpu, cpu->a, cpu->mf);
      break;
    }
    case 0x4b: { // phk imp
      cpu_pushByte(cpu, cpu->k);
      break;
    }
    case 0x4c: { // jmp abs
      cpu->pc = cpu_readOpcodeWord(cpu);
      break;
    }
    case 0x4d: { // eor abs
      uint32_t low = 0;
      uint32_t high = cpu_adrAbs(cpu, &low);
      cpu_eor(cpu, low, high);
      break;
    }
    case 0x4e: { // lsr abs
      uint32_t low = 0;
      uint32_t high = cpu_adrAbs(cpu, &low);
      cpu_lsr(cpu, low, high);
      break;
    }
    case 0x4f: { // eor abl
      uint32_t low = 0;
      uint32_t high = cpu_adrAbl(cpu, &low);
      cpu_eor(cpu, low, high);
      break;
    }
    case 0x50: { // bvc rel
      cpu_doBranch(cpu, cpu_readOpcode(cpu), !cpu->v);
      break;
    }
    case 0x51: { // eor idy(r)
      uint32_t low = 0;
      uint32_t high = cpu_adrIdy(cpu, &low, false);
      cpu_eor(cpu, low, high);
      break;
    }
    case 0x52: { // eor idp
      uint32_t low = 0;
      uint32_t high = cpu_adrIdp(cpu, &low);
      cpu_eor(cpu, low, high);
      break;
    }
    case 0x53: { // eor isy
      uint32_t low = 0;
      uint32_t high = cpu_adrIsy(cpu, &low);
      cpu_eor(cpu, low, high);
      break;
    }
    case 0x54: { // mvn bm
      uint8_t dest = cpu_readOpcode(cpu);
      uint8_t src = cpu_readOpcode(cpu);
      cpu->db = dest;
      cpu_write(cpu, (dest << 16) | cpu->y, cpu_read(cpu, (src << 16) | cpu->x));
      cpu->a--;
      cpu->x++;
      cpu->y++;
      if(cpu->a != 0xffff) {
        cpu->pc -= 3;
      }
      if(cpu->xf) {
        cpu->x &= 0xff;
        cpu->y &= 0xff;
      }
      break;
    }
    case 0x55: { // eor dpx
      uint32_t low = 0;
      uint32_t high = cpu_adrDpx(cpu, &low);
      cpu_eor(cpu, low, high);
      break;
    }
    case 0x56: { // lsr dpx
      uint32_t low = 0;
      uint32_t high = cpu_adrDpx(cpu, &low);
      cpu_lsr(cpu, low, high);
      break;
    }
    case 0x57: { // eor ily
      uint32_t low = 0;
      uint32_t high = cpu_adrIly(cpu, &low);
      cpu_eor(cpu, low, high);
      break;
    }
    case 0x58: { // cli imp
      cpu->i = false;
      break;
    }
    case 0x59: { // eor aby(r)
      uint32_t low = 0;
      uint32_t high = cpu_adrAby(cpu, &low, false);
      cpu_eor(cpu, low, high);
      break;
    }
    case 0x5a: { // phy imp
      if(cpu->xf) {
        cpu_pushByte(cpu, cpu->y);
      } else {
        cpu->cyclesUsed++; // m = 0: 1 extra cycle
        cpu_pushWord(cpu, cpu->y);
      }
      break;
    }
    case 0x5b: { // tcd imp
      cpu->dp = cpu->a;
      cpu_setZN(cpu, cpu->dp, false);
      break;
    }
    case 0x5c: { // jml abl
      uint16_t value = cpu_readOpcodeWord(cpu);
      cpu->k = cpu_readOpcode(cpu);
      cpu->pc = value;
      break;
    }
    case 0x5d: { // eor abx(r)
      uint32_t low = 0;
      uint32_t high = cpu_adrAbx(cpu, &low, false);
      cpu_eor(cpu, low, high);
      break;
    }
    case 0x5e: { // lsr abx
      uint32_t low = 0;
      uint32_t high = cpu_adrAbx(cpu, &low, true);
      cpu_lsr(cpu, low, high);
      break;
    }
    case 0x5f: { // eor alx
      uint32_t low = 0;
      uint32_t high = cpu_adrAlx(cpu, &low);
      cpu_eor(cpu, low, high);
      break;
    }
    case 0x60: { // rts imp
      //if (cpu->spBreakpoint)
      //  fprintf(stderr, "0x%x: rts 0x%x 0x%x\n", cpu->k<<16 | cpu->pc, cpu->spBreakpoint, cpu->sp);
      if (cpu->sp >= cpu->spBreakpoint && cpu->spBreakpoint) {
        assert(cpu->sp == cpu->spBreakpoint);
        cpu->spBreakpoint = 0;
        HookedFunctionRts(0);
      }
      cpu->pc = cpu_pullWord(cpu) + 1;
      break;
    }
    case 0x61: { // adc idx
      uint32_t low = 0;
      uint32_t high = cpu_adrIdx(cpu, &low);
      cpu_adc(cpu, low, high);
      break;
    }
    case 0x62: { // per rll
      uint16_t value = cpu_readOpcodeWord(cpu);
      cpu_pushWord(cpu, cpu->pc + (int16_t) value);
      break;
    }
    case 0x63: { // adc sr
      uint32_t low = 0;
      uint32_t high = cpu_adrSr(cpu, &low);
      cpu_adc(cpu, low, high);
      break;
    }
    case 0x64: { // stz dp
      uint32_t low = 0;
      uint32_t high = cpu_adrDp(cpu, &low);
      cpu_stz(cpu, low, high);
      break;
    }
    case 0x65: adc_65: { // adc dp
      uint32_t low = 0;
      uint32_t high = cpu_adrDp(cpu, &low);
      cpu_adc(cpu, low, high);
      break;
    }
    case 0x66: { // ror dp
      uint32_t low = 0;
      uint32_t high = cpu_adrDp(cpu, &low);
      cpu_ror(cpu, low, high);
      break;
    }
    case 0x67: { // adc idl
      uint32_t low = 0;
      uint32_t high = cpu_adrIdl(cpu, &low);
      cpu_adc(cpu, low, high);
      break;
    }
    case 0x68: { // pla imp
      if(cpu->mf) {
        cpu->a = (cpu->a & 0xff00) | cpu_pullByte(cpu);
      } else {
        cpu->cyclesUsed++; // 16-bit m: 1 extra cycle
        cpu->a = cpu_pullWord(cpu);
      }
      cpu_setZN(cpu, cpu->a, cpu->mf);
      break;
    }
    adc_69:
    case 0x69: { // adc imm(m)
      uint32_t low = 0;
      uint32_t high = cpu_adrImm(cpu, &low, false);
      cpu_adc(cpu, low, high);
      break;
    }
    case 0x6a: { // rora imp
      bool carry = cpu->a & 1;
      if(cpu->mf) {
        cpu->a = (cpu->a & 0xff00) | ((cpu->a >> 1) & 0x7f) | (cpu->c << 7);
      } else {
        cpu->a = (cpu->a >> 1) | (cpu->c << 15);
      }
      cpu->c = carry;
      cpu_setZN(cpu, cpu->a, cpu->mf);
      break;
    }
    case 0x6b: { // rtl imp
      //if (cpu->spBreakpoint)
      //  fprintf(stderr, "0x%x: rtl 0x%x 0x%x\n", cpu->k<<16 | cpu->pc, cpu->spBreakpoint, cpu->sp);

      if (cpu->sp >= cpu->spBreakpoint && cpu->spBreakpoint) {
        assert(cpu->sp == cpu->spBreakpoint);
        cpu->spBreakpoint = 0;
        HookedFunctionRts(1);
      }
      cpu->pc = cpu_pullWord(cpu) + 1;
      cpu->k = cpu_pullByte(cpu);
      break;
    }
    case 0x6c: { // jmp ind
      uint16_t adr = cpu_readOpcodeWord(cpu);
      cpu->pc = cpu_readWord(cpu, adr, (adr + 1) & 0xffff);
      break;
    }
    case 0x6d: { // adc abs
      uint32_t low = 0;
      uint32_t high = cpu_adrAbs(cpu, &low);
      cpu_adc(cpu, low, high);
      break;
    }
    case 0x6e: { // ror abs
      uint32_t low = 0;
      uint32_t high = cpu_adrAbs(cpu, &low);
      cpu_ror(cpu, low, high);
      break;
    }
    case 0x6f: { // adc abl
      uint32_t low = 0;
      uint32_t high = cpu_adrAbl(cpu, &low);
      cpu_adc(cpu, low, high);
      break;
    }
    case 0x70: { // bvs rel
      cpu_doBranch(cpu, cpu_readOpcode(cpu), cpu->v);
      break;
    }
    case 0x71: { // adc idy(r)
      uint32_t low = 0;
      uint32_t high = cpu_adrIdy(cpu, &low, false);
      cpu_adc(cpu, low, high);
      break;
    }
    case 0x72: { // adc idp
      uint32_t low = 0;
      uint32_t high = cpu_adrIdp(cpu, &low);
      cpu_adc(cpu, low, high);
      break;
    }
    case 0x73: { // adc isy
      uint32_t low = 0;
      uint32_t high = cpu_adrIsy(cpu, &low);
      cpu_adc(cpu, low, high);
      break;
    }
    case 0x74: { // stz dpx
      uint32_t low = 0;
      uint32_t high = cpu_adrDpx(cpu, &low);
      cpu_stz(cpu, low, high);
      break;
    }
    case 0x75: { // adc dpx
      uint32_t low = 0;
      uint32_t high = cpu_adrDpx(cpu, &low);
      cpu_adc(cpu, low, high);
      break;
    }
    case 0x76: { // ror dpx
      uint32_t low = 0;
      uint32_t high = cpu_adrDpx(cpu, &low);
      cpu_ror(cpu, low, high);
      break;
    }
    case 0x77: { // adc ily
      uint32_t low = 0;
      uint32_t high = cpu_adrIly(cpu, &low);
      cpu_adc(cpu, low, high);
      break;
    }
    case 0x78: { // sei imp
      cpu->i = true;
      break;
    }
    case 0x79: { // adc aby(r)
      uint32_t low = 0;
      uint32_t high = cpu_adrAby(cpu, &low, false);
      cpu_adc(cpu, low, high);
      break;
    }
    case 0x7a: { // ply imp
      if(cpu->xf) {
        cpu->y = cpu_pullByte(cpu);
      } else {
        cpu->cyclesUsed++; // 16-bit x: 1 extra cycle
        cpu->y = cpu_pullWord(cpu);
      }
      cpu_setZN(cpu, cpu->y, cpu->xf);
      break;
    }
    case 0x7b: { // tdc imp
      cpu->a = cpu->dp;
      cpu_setZN(cpu, cpu->a, false);
      break;
    }
    case 0x7c: { // jmp iax
      cpu->pc = cpu_adrIax(cpu);
      break;
    }
    case 0x7d: { // adc abx(r)
      uint32_t low = 0;
      uint32_t high = cpu_adrAbx(cpu, &low, false);
      cpu_adc(cpu, low, high);
      break;
    }
    case 0x7e: { // ror abx
      uint32_t low = 0;
      uint32_t high = cpu_adrAbx(cpu, &low, true);
      cpu_ror(cpu, low, high);
      break;
    }
    case 0x7f: { // adc alx
      uint32_t low = 0;
      uint32_t high = cpu_adrAlx(cpu, &low);
      cpu_adc(cpu, low, high);
      break;
    }
    case 0x80: { // bra rel
      cpu->pc += (int8_t) cpu_readOpcode(cpu);
      break;
    }
    case 0x81: { // sta idx
      uint32_t low = 0;
      uint32_t high = cpu_adrIdx(cpu, &low);
      cpu_sta(cpu, low, high);
      break;
    }
    case 0x82: { // brl rll
      cpu->pc += (int16_t) cpu_readOpcodeWord(cpu);
      break;
    }
    case 0x83: { // sta sr
      uint32_t low = 0;
      uint32_t high = cpu_adrSr(cpu, &low);
      cpu_sta(cpu, low, high);
      break;
    }
    case 0x84: { // sty dp
      uint32_t low = 0;
      uint32_t high = cpu_adrDp(cpu, &low);
      cpu_sty(cpu, low, high);
      break;
    }
    case 0x85: { // sta dp
      uint32_t low = 0;
      uint32_t high = cpu_adrDp(cpu, &low);
      cpu_sta(cpu, low, high);
      break;
    }
    case 0x86: { // stx dp
      uint32_t low = 0;
      uint32_t high = cpu_adrDp(cpu, &low);
      cpu_stx(cpu, low, high);
      break;
    }
    case 0x87: { // sta idl
      uint32_t low = 0;
      uint32_t high = cpu_adrIdl(cpu, &low);
      cpu_sta(cpu, low, high);
      break;
    }
    case 0x88: { // dey imp
      if(cpu->xf) {
        cpu->y = (cpu->y - 1) & 0xff;
      } else {
        cpu->y--;
      }
      cpu_setZN(cpu, cpu->y, cpu->xf);
      break;
    }
    case 0x89: { // biti imm(m)
      if(cpu->mf) {
        uint8_t result = (cpu->a & 0xff) & cpu_readOpcode(cpu);
        cpu->z = result == 0;
      } else {
        cpu->cyclesUsed++; // m = 0: 1 extra cycle
        uint16_t result = cpu->a & cpu_readOpcodeWord(cpu);
        cpu->z = result == 0;
      }
      break;
    }
    case 0x8a: { // txa imp
      if(cpu->mf) {
        cpu->a = (cpu->a & 0xff00) | (cpu->x & 0xff);
      } else {
        cpu->a = cpu->x;
      }
      cpu_setZN(cpu, cpu->a, cpu->mf);
      break;
    }
    case 0x8b: { // phb imp
      cpu_pushByte(cpu, cpu->db);
      break;
    }
    case 0x8c: { // sty abs
      uint32_t low = 0;
      uint32_t high = cpu_adrAbs(cpu, &low);
      cpu_sty(cpu, low, high);
      break;
    }
    case 0x8d: { // sta abs
      uint32_t low = 0;
      uint32_t high = cpu_adrAbs(cpu, &low);
      cpu_sta(cpu, low, high);
      break;
    }
    case 0x8e: { // stx abs
      uint32_t low = 0;
      uint32_t high = cpu_adrAbs(cpu, &low);
      cpu_stx(cpu, low, high);
      break;
    }
    case 0x8f: { // sta abl
      uint32_t low = 0;
      uint32_t high = cpu_adrAbl(cpu, &low);
      cpu_sta(cpu, low, high);
      break;
    }
    case 0x90: { // bcc rel
      cpu_doBranch(cpu, cpu_readOpcode(cpu), !cpu->c);
      break;
    }
    case 0x91: { // sta idy
      uint32_t low = 0;
      uint32_t high = cpu_adrIdy(cpu, &low, true);
      cpu_sta(cpu, low, high);
      break;
    }
    case 0x92: { // sta idp
      uint32_t low = 0;
      uint32_t high = cpu_adrIdp(cpu, &low);
      cpu_sta(cpu, low, high);
      break;
    }
    case 0x93: { // sta isy
      uint32_t low = 0;
      uint32_t high = cpu_adrIsy(cpu, &low);
      cpu_sta(cpu, low, high);
      break;
    }
    case 0x94: { // sty dpx
      uint32_t low = 0;
      uint32_t high = cpu_adrDpx(cpu, &low);
      cpu_sty(cpu, low, high);
      break;
    }
    case 0x95: { // sta dpx
      uint32_t low = 0;
      uint32_t high = cpu_adrDpx(cpu, &low);
      cpu_sta(cpu, low, high);
      break;
    }
    case 0x96: { // stx dpy
      uint32_t low = 0;
      uint32_t high = cpu_adrDpy(cpu, &low);
      cpu_stx(cpu, low, high);
      break;
    }
    case 0x97: { // sta ily
      uint32_t low = 0;
      uint32_t high = cpu_adrIly(cpu, &low);
      cpu_sta(cpu, low, high);
      break;
    }
    case 0x98: { // tya imp
      if(cpu->mf) {
        cpu->a = (cpu->a & 0xff00) | (cpu->y & 0xff);
      } else {
        cpu->a = cpu->y;
      }
      cpu_setZN(cpu, cpu->a, cpu->mf);
      break;
    }
    case 0x99: { // sta aby
      uint32_t low = 0;
      uint32_t high = cpu_adrAby(cpu, &low, true);
      cpu_sta(cpu, low, high);
      break;
    }
    case 0x9a: { // txs imp
      cpu->sp = cpu->x;
      break;
    }
    case 0x9b: { // txy imp
      if(cpu->xf) {
        cpu->y = cpu->x & 0xff;
      } else {
        cpu->y = cpu->x;
      }
      cpu_setZN(cpu, cpu->y, cpu->xf);
      break;
    }
    case 0x9c: { // stz abs
      uint32_t low = 0;
      uint32_t high = cpu_adrAbs(cpu, &low);
      cpu_stz(cpu, low, high);
      break;
    }
    case 0x9d: { // sta abx
      uint32_t low = 0;
      uint32_t high = cpu_adrAbx(cpu, &low, true);
      cpu_sta(cpu, low, high);
      break;
    }
    case 0x9e: { // stz abx
      uint32_t low = 0;
      uint32_t high = cpu_adrAbx(cpu, &low, true);
      cpu_stz(cpu, low, high);
      break;
    }
    case 0x9f: { // sta alx
      uint32_t low = 0;
      uint32_t high = cpu_adrAlx(cpu, &low);
      cpu_sta(cpu, low, high);
      break;
    }
    case 0xa0: { // ldy imm(x)
      uint32_t low = 0;
      uint32_t high = cpu_adrImm(cpu, &low, true);
      cpu_ldy(cpu, low, high);
      break;
    }
    case 0xa1: { // lda idx
      uint32_t low = 0;
      uint32_t high = cpu_adrIdx(cpu, &low);
      cpu_lda(cpu, low, high);
      break;
    }
    case 0xa2: { // ldx imm(x)
      uint32_t low = 0;
      uint32_t high = cpu_adrImm(cpu, &low, true);
      cpu_ldx(cpu, low, high);
      break;
    }
    case 0xa3: { // lda sr
      uint32_t low = 0;
      uint32_t high = cpu_adrSr(cpu, &low);
      cpu_lda(cpu, low, high);
      break;
    }
    case 0xa4: { // ldy dp
      uint32_t low = 0;
      uint32_t high = cpu_adrDp(cpu, &low);
      cpu_ldy(cpu, low, high);
      break;
    }
    case 0xa5: { // lda dp
      uint32_t low = 0;
      uint32_t high = cpu_adrDp(cpu, &low);
      cpu_lda(cpu, low, high);
      break;
    }
    case 0xa6: { // ldx dp
      uint32_t low = 0;
      uint32_t high = cpu_adrDp(cpu, &low);
      cpu_ldx(cpu, low, high);
      break;
    }
    case 0xa7: { // lda idl
      uint32_t low = 0;
      uint32_t high = cpu_adrIdl(cpu, &low);
      cpu_lda(cpu, low, high);
      break;
    }
    case 0xa8: { // tay imp
      if(cpu->xf) {
        cpu->y = cpu->a & 0xff;
      } else {
        cpu->y = cpu->a;
      }
      cpu_setZN(cpu, cpu->y, cpu->xf);
      break;
    }
    case 0xa9: { // lda imm(m)
      uint32_t low = 0;
      uint32_t high = cpu_adrImm(cpu, &low, false);
      cpu_lda(cpu, low, high);
      break;
    }
    case 0xaa: { // tax imp
      if(cpu->xf) {
        cpu->x = cpu->a & 0xff;
      } else {
        cpu->x = cpu->a;
      }
      cpu_setZN(cpu, cpu->x, cpu->xf);
      break;
    }
    case 0xab: { // plb imp
      cpu->db = cpu_pullByte(cpu);
      cpu_setZN(cpu, cpu->db, true);
      break;
    }
    case 0xac: { // ldy abs
      uint32_t low = 0;
      uint32_t high = cpu_adrAbs(cpu, &low);
      cpu_ldy(cpu, low, high);
      break;
    }
    case 0xad: { // lda abs
      uint32_t low = 0;
      uint32_t high = cpu_adrAbs(cpu, &low);
      cpu_lda(cpu, low, high);
      break;
    }
    case 0xae: { // ldx abs
      uint32_t low = 0;
      uint32_t high = cpu_adrAbs(cpu, &low);
      cpu_ldx(cpu, low, high);
      break;
    }
    case 0xaf: { // lda abl
      uint32_t low = 0;
      uint32_t high = cpu_adrAbl(cpu, &low);
      cpu_lda(cpu, low, high);
      break;
    }
    case 0xb0: { // bcs rel
      cpu_doBranch(cpu, cpu_readOpcode(cpu), cpu->c);
      break;
    }
    case 0xb1: { // lda idy(r)
      uint32_t low = 0;
      uint32_t high = cpu_adrIdy(cpu, &low, false);
      cpu_lda(cpu, low, high);
      break;
    }
    case 0xb2: { // lda idp
      uint32_t low = 0;
      uint32_t high = cpu_adrIdp(cpu, &low);
      cpu_lda(cpu, low, high);
      break;
    }
    case 0xb3: { // lda isy
      uint32_t low = 0;
      uint32_t high = cpu_adrIsy(cpu, &low);
      cpu_lda(cpu, low, high);
      break;
    }
    case 0xb4: { // ldy dpx
      uint32_t low = 0;
      uint32_t high = cpu_adrDpx(cpu, &low);
      cpu_ldy(cpu, low, high);
      break;
    }
    case 0xb5: { // lda dpx
      uint32_t low = 0;
      uint32_t high = cpu_adrDpx(cpu, &low);
      cpu_lda(cpu, low, high);
      break;
    }
    case 0xb6: { // ldx dpy
      uint32_t low = 0;
      uint32_t high = cpu_adrDpy(cpu, &low);
      cpu_ldx(cpu, low, high);
      break;
    }
    case 0xb7: { // lda ily
      uint32_t low = 0;
      uint32_t high = cpu_adrIly(cpu, &low);
      cpu_lda(cpu, low, high);
      break;
    }
    case 0xb8: { // clv imp
      cpu->v = false;
      break;
    }
    case 0xb9: { // lda aby(r)
      uint32_t low = 0;
      uint32_t high = cpu_adrAby(cpu, &low, false);
      cpu_lda(cpu, low, high);
      break;
    }
    case 0xba: { // tsx imp
      if(cpu->xf) {
        cpu->x = cpu->sp & 0xff;
      } else {
        cpu->x = cpu->sp;
      }
      cpu_setZN(cpu, cpu->x, cpu->xf);
      break;
    }
    case 0xbb: { // tyx imp
      if(cpu->xf) {
        cpu->x = cpu->y & 0xff;
      } else {
        cpu->x = cpu->y;
      }
      cpu_setZN(cpu, cpu->x, cpu->xf);
      break;
    }
    case 0xbc: { // ldy abx(r)
      uint32_t low = 0;
      uint32_t high = cpu_adrAbx(cpu, &low, false);
      cpu_ldy(cpu, low, high);
      break;
    }
    case 0xbd: { // lda abx(r)
      uint32_t low = 0;
      uint32_t high = cpu_adrAbx(cpu, &low, false);
      cpu_lda(cpu, low, high);
      break;
    }
    case 0xbe: { // ldx aby(r)
      uint32_t low = 0;
      uint32_t high = cpu_adrAby(cpu, &low, false);
      cpu_ldx(cpu, low, high);
      break;
    }
    case 0xbf: { // lda alx
      uint32_t low = 0;
      uint32_t high = cpu_adrAlx(cpu, &low);
      cpu_lda(cpu, low, high);
      break;
    }
    case 0xc0: { // cpy imm(x)
      uint32_t low = 0;
      uint32_t high = cpu_adrImm(cpu, &low, true);
      cpu_cpy(cpu, low, high);
      break;
    }
    case 0xc1: { // cmp idx
      uint32_t low = 0;
      uint32_t high = cpu_adrIdx(cpu, &low);
      cpu_cmp(cpu, low, high);
      break;
    }
    case 0xc2: { // rep imm(s)
      cpu_setFlags(cpu, cpu_getFlags(cpu) & ~cpu_readOpcode(cpu));
      break;
    }
    case 0xc3: { // cmp sr
      uint32_t low = 0;
      uint32_t high = cpu_adrSr(cpu, &low);
      cpu_cmp(cpu, low, high);
      break;
    }
    case 0xc4: { // cpy dp
      uint32_t low = 0;
      uint32_t high = cpu_adrDp(cpu, &low);
      cpu_cpy(cpu, low, high);
      break;
    }
    case 0xc5: { // cmp dp
      uint32_t low = 0;
      uint32_t high = cpu_adrDp(cpu, &low);
      cpu_cmp(cpu, low, high);
      break;
    }
    case 0xc6: { // dec dp
      uint32_t low = 0;
      uint32_t high = cpu_adrDp(cpu, &low);
      cpu_dec(cpu, low, high);
      break;
    }
    case 0xc7: { // cmp idl
      uint32_t low = 0;
      uint32_t high = cpu_adrIdl(cpu, &low);
      cpu_cmp(cpu, low, high);
      break;
    }
    case_iny_c8:
    case 0xc8: { // iny imp
      if(cpu->xf) {
        cpu->y = (cpu->y + 1) & 0xff;
      } else {
        cpu->y++;
      }
      cpu_setZN(cpu, cpu->y, cpu->xf);
      break;
    }
    case 0xc9: { // cmp imm(m)
      uint32_t low = 0;
      uint32_t high = cpu_adrImm(cpu, &low, false);
      cpu_cmp(cpu, low, high);
      break;
    }
    case 0xca: { // dex imp
      if(cpu->xf) {
        cpu->x = (cpu->x - 1) & 0xff;
      } else {
        cpu->x--;
      }
      cpu_setZN(cpu, cpu->x, cpu->xf);
      break;
    }
    case 0xcb: { // wai imp
      cpu->waiting = true;
      break;
    }
    case 0xcc: { // cpy abs
      uint32_t low = 0;
      uint32_t high = cpu_adrAbs(cpu, &low);
      cpu_cpy(cpu, low, high);
      break;
    }
    case 0xcd: { // cmp abs
      uint32_t low = 0;
      uint32_t high = cpu_adrAbs(cpu, &low);
      cpu_cmp(cpu, low, high);
      break;
    }
    case 0xce: { // dec abs
      uint32_t low = 0;
      uint32_t high = cpu_adrAbs(cpu, &low);
      cpu_dec(cpu, low, high);
      break;
    }
    case 0xcf: { // cmp abl
      uint32_t low = 0;
      uint32_t high = cpu_adrAbl(cpu, &low);
      cpu_cmp(cpu, low, high);
      break;
    }
    case 0xd0: { // bne rel
      cpu_doBranch(cpu, cpu_readOpcode(cpu), !cpu->z);
      break;
    }
    case 0xd1: { // cmp idy(r)
      uint32_t low = 0;
      uint32_t high = cpu_adrIdy(cpu, &low, false);
      cpu_cmp(cpu, low, high);
      break;
    }
    case 0xd2: { // cmp idp
      uint32_t low = 0;
      uint32_t high = cpu_adrIdp(cpu, &low);
      cpu_cmp(cpu, low, high);
      break;
    }
    case 0xd3: { // cmp isy
      uint32_t low = 0;
      uint32_t high = cpu_adrIsy(cpu, &low);
      cpu_cmp(cpu, low, high);
      break;
    }
    case 0xd4: { // pei dp
      uint32_t low = 0;
      uint32_t high = cpu_adrDp(cpu, &low);
      cpu_pushWord(cpu, cpu_readWord(cpu, low, high));
      break;
    }
    case 0xd5: { // cmp dpx
      uint32_t low = 0;
      uint32_t high = cpu_adrDpx(cpu, &low);
      cpu_cmp(cpu, low, high);
      break;
    }
    case 0xd6: { // dec dpx
      uint32_t low = 0;
      uint32_t high = cpu_adrDpx(cpu, &low);
      cpu_dec(cpu, low, high);
      break;
    }
    case 0xd7: { // cmp ily
      uint32_t low = 0;
      uint32_t high = cpu_adrIly(cpu, &low);
      cpu_cmp(cpu, low, high);
      break;
    }
    case 0xd8: { // cld imp
      cpu->d = false;
      break;
    }
    case 0xd9: { // cmp aby(r)
      uint32_t low = 0;
      uint32_t high = cpu_adrAby(cpu, &low, false);
      cpu_cmp(cpu, low, high);
      break;
    }
    case 0xda: { // phx imp
      if(cpu->xf) {
        cpu_pushByte(cpu, cpu->x);
      } else {
        cpu->cyclesUsed++; // m = 0: 1 extra cycle
        cpu_pushWord(cpu, cpu->x);
      }
      break;
    }
    case 0xdb: { // stp imp
      cpu->stopped = true;
      break;
    }
    case 0xdc: { // jml ial
      uint16_t adr = cpu_readOpcodeWord(cpu);
      cpu->pc = cpu_readWord(cpu, adr, (adr + 1) & 0xffff);
      cpu->k = cpu_read(cpu, (adr + 2) & 0xffff);
      break;
    }
    case 0xdd: { // cmp abx(r)
      uint32_t low = 0;
      uint32_t high = cpu_adrAbx(cpu, &low, false);
      cpu_cmp(cpu, low, high);
      break;
    }
    case 0xde: { // dec abx
      uint32_t low = 0;
      uint32_t high = cpu_adrAbx(cpu, &low, true);
      cpu_dec(cpu, low, high);
      break;
    }
    case 0xdf: { // cmp alx
      uint32_t low = 0;
      uint32_t high = cpu_adrAlx(cpu, &low);
      cpu_cmp(cpu, low, high);
      break;
    }
    case 0xe0: { // cpx imm(x)
      uint32_t low = 0;
      uint32_t high = cpu_adrImm(cpu, &low, true);
      cpu_cpx(cpu, low, high);
      break;
    }
    case 0xe1: { // sbc idx
      uint32_t low = 0;
      uint32_t high = cpu_adrIdx(cpu, &low);
      cpu_sbc(cpu, low, high);
      break;
    }
    case 0xe2: { // sep imm(s)
      cpu_setFlags(cpu, cpu_getFlags(cpu) | cpu_readOpcode(cpu));
      break;
    }
    case 0xe3: { // sbc sr
      uint32_t low = 0;
      uint32_t high = cpu_adrSr(cpu, &low);
      cpu_sbc(cpu, low, high);
      break;
    }
    case 0xe4: { // cpx dp
      uint32_t low = 0;
      uint32_t high = cpu_adrDp(cpu, &low);
      cpu_cpx(cpu, low, high);
      break;
    }
    sbc_e5:
    case 0xe5: { // sbc dp
      uint32_t low = 0;
      uint32_t high = cpu_adrDp(cpu, &low);
      cpu_sbc(cpu, low, high);
      break;
    }
    case 0xe6: { // inc dp
      uint32_t low = 0;
      uint32_t high = cpu_adrDp(cpu, &low);
      cpu_inc(cpu, low, high);
      break;
    }
    case 0xe7: { // sbc idl
      uint32_t low = 0;
      uint32_t high = cpu_adrIdl(cpu, &low);
      cpu_sbc(cpu, low, high);
      break;
    }
    case 0xe8: { // inx imp
      if(cpu->xf) {
        cpu->x = (cpu->x + 1) & 0xff;
      } else {
        cpu->x++;
      }
      cpu_setZN(cpu, cpu->x, cpu->xf);
      break;
    }
    sbc_e9:
    case 0xe9: { // sbc imm(m)
      uint32_t low = 0;
      uint32_t high = cpu_adrImm(cpu, &low, false);
      cpu_sbc(cpu, low, high);
      break;
    }
    case 0xea: { // nop imp
      // no operation
      break;
    }
    case 0xeb: { // xba imp
      uint8_t low = cpu->a & 0xff;
      uint8_t high = cpu->a >> 8;
      cpu->a = (low << 8) | high;
      cpu_setZN(cpu, high, true);
      break;
    }
    case 0xec: { // cpx abs
      uint32_t low = 0;
      uint32_t high = cpu_adrAbs(cpu, &low);
      cpu_cpx(cpu, low, high);
      break;
    }
    case 0xed: { // sbc abs
      uint32_t low = 0;
      uint32_t high = cpu_adrAbs(cpu, &low);
      cpu_sbc(cpu, low, high);
      break;
    }
    case 0xee: { // inc abs
      uint32_t low = 0;
      uint32_t high = cpu_adrAbs(cpu, &low);
      cpu_inc(cpu, low, high);
      break;
    }
    case 0xef: { // sbc abl
      uint32_t low = 0;
      uint32_t high = cpu_adrAbl(cpu, &low);
      cpu_sbc(cpu, low, high);
      break;
    }
    case 0xf0: { // beq rel
      cpu_doBranch(cpu, cpu_readOpcode(cpu), cpu->z);
      break;
    }
    case 0xf1: { // sbc idy(r)
      uint32_t low = 0;
      uint32_t high = cpu_adrIdy(cpu, &low, false);
      cpu_sbc(cpu, low, high);
      break;
    }
    case 0xf2: { // sbc idp
      uint32_t low = 0;
      uint32_t high = cpu_adrIdp(cpu, &low);
      cpu_sbc(cpu, low, high);
      break;
    }
    case 0xf3: { // sbc isy
      uint32_t low = 0;
      uint32_t high = cpu_adrIsy(cpu, &low);
      cpu_sbc(cpu, low, high);
      break;
    }
    case 0xf4: { // pea imm(l)
      cpu_pushWord(cpu, cpu_readOpcodeWord(cpu));
      break;
    }
    case 0xf5: { // sbc dpx
      uint32_t low = 0;
      uint32_t high = cpu_adrDpx(cpu, &low);
      cpu_sbc(cpu, low, high);
      break;
    }
    case 0xf6: { // inc dpx
      uint32_t low = 0;
      uint32_t high = cpu_adrDpx(cpu, &low);
      cpu_inc(cpu, low, high);
      break;
    }
    case 0xf7: { // sbc ily
      uint32_t low = 0;
      uint32_t high = cpu_adrIly(cpu, &low);
      cpu_sbc(cpu, low, high);
      break;
    }
    case 0xf8: { // sed imp
      cpu->d = true;
      break;
    }
    case 0xf9: { // sbc aby(r)
      uint32_t low = 0;
      uint32_t high = cpu_adrAby(cpu, &low, false);
      cpu_sbc(cpu, low, high);
      break;
    }
    case 0xfa: { // plx imp
      if(cpu->xf) {
        cpu->x = cpu_pullByte(cpu);
      } else {
        cpu->cyclesUsed++; // 16-bit x: 1 extra cycle
        cpu->x = cpu_pullWord(cpu);
      }
      cpu_setZN(cpu, cpu->x, cpu->xf);
      break;
    }
    case 0xfb: { // xce imp
      bool temp = cpu->c;
      cpu->c = cpu->e;
      cpu->e = temp;
      cpu_setFlags(cpu, cpu_getFlags(cpu)); // updates x and m flags, clears upper half of x and y if needed
      break;
    }
    case 0xfc: { // jsr iax
      uint16_t value = cpu_adrIax(cpu);
      cpu_pushWord(cpu, cpu->pc - 1);
      cpu->pc = value;
      break;
    }
    case 0xfd: { // sbc abx(r)
      uint32_t low = 0;
      uint32_t high = cpu_adrAbx(cpu, &low, false);
      cpu_sbc(cpu, low, high);
      break;
    }
    case 0xfe: { // inc abx
      uint32_t low = 0;
      uint32_t high = cpu_adrAbx(cpu, &low, true);
      cpu_inc(cpu, low, high);
      break;
    }
    case 0xff: { // sbc alx
      uint32_t low = 0;
      uint32_t high = cpu_adrAlx(cpu, &low);
      cpu_sbc(cpu, low, high);
      break;
    }
  }
}
