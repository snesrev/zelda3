
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "spc.h"
#include "apu.h"

static const int cyclesPerOpcode[256] = {
  2, 8, 4, 5, 3, 4, 3, 6, 2, 6, 5, 4, 5, 4, 6, 8,
  2, 8, 4, 5, 4, 5, 5, 6, 5, 5, 6, 5, 2, 2, 4, 6,
  2, 8, 4, 5, 3, 4, 3, 6, 2, 6, 5, 4, 5, 4, 5, 4,
  2, 8, 4, 5, 4, 5, 5, 6, 5, 5, 6, 5, 2, 2, 3, 8,
  2, 8, 4, 5, 3, 4, 3, 6, 2, 6, 4, 4, 5, 4, 6, 6,
  2, 8, 4, 5, 4, 5, 5, 6, 5, 5, 4, 5, 2, 2, 4, 3,
  2, 8, 4, 5, 3, 4, 3, 6, 2, 6, 4, 4, 5, 4, 5, 5,
  2, 8, 4, 5, 4, 5, 5, 6, 5, 5, 5, 5, 2, 2, 3, 6,
  2, 8, 4, 5, 3, 4, 3, 6, 2, 6, 5, 4, 5, 2, 4, 5,
  2, 8, 4, 5, 4, 5, 5, 6, 5, 5, 5, 5, 2, 2, 12,5,
  2, 8, 4, 5, 3, 4, 3, 6, 2, 6, 4, 4, 5, 2, 4, 4,
  2, 8, 4, 5, 4, 5, 5, 6, 5, 5, 5, 5, 2, 2, 3, 4,
  2, 8, 4, 5, 4, 5, 4, 7, 2, 5, 6, 4, 5, 2, 4, 9,
  2, 8, 4, 5, 5, 6, 6, 7, 4, 5, 5, 5, 2, 2, 6, 3,
  2, 8, 4, 5, 3, 4, 3, 6, 2, 4, 5, 3, 4, 3, 4, 3,
  2, 8, 4, 5, 4, 5, 5, 6, 3, 4, 5, 4, 2, 2, 4, 3
};

static uint8_t spc_read(Spc* spc, uint16_t adr);
static void spc_write(Spc* spc, uint16_t adr, uint8_t val);
static uint8_t spc_readOpcode(Spc* spc);
static uint16_t spc_readOpcodeWord(Spc* spc);
static uint8_t spc_getFlags(Spc* spc);
static void spc_setFlags(Spc* spc, uint8_t value);
static void spc_setZN(Spc* spc, uint8_t value);
static void spc_doBranch(Spc* spc, uint8_t value, bool check);
static uint8_t spc_pullByte(Spc* spc);
static void spc_pushByte(Spc* spc, uint8_t value);
static uint16_t spc_pullWord(Spc* spc);
static void spc_pushWord(Spc* spc, uint16_t value);
static uint16_t spc_readWord(Spc* spc, uint16_t adrl, uint16_t adrh);
static void spc_writeWord(Spc* spc, uint16_t adrl, uint16_t adrh, uint16_t value);
static void spc_doOpcode(Spc* spc, uint8_t opcode);

// addressing modes and opcode functions not declared, only used after defintions

static uint8_t spc_read(Spc* spc, uint16_t adr) {
  return apu_cpuRead(spc->apu, adr);
}

static void spc_write(Spc* spc, uint16_t adr, uint8_t val) {
  if (0 && adr == 0x5e)
    printf("writing to vol_dirty %d\n", val);
  apu_cpuWrite(spc->apu, adr, val);
}

Spc* spc_init(Apu* apu) {
  Spc* spc = (Spc * )malloc(sizeof(Spc));
  spc->apu = apu;
  return spc;
}

void spc_free(Spc* spc) {
  free(spc);
}

void spc_reset(Spc* spc) {
  spc->a = 0;
  spc->x = 0;
  spc->y = 0;
  spc->sp = 0;
  spc->pc = spc_read(spc, 0xfffe) | (spc_read(spc, 0xffff) << 8);
  spc->c = false;
  spc->z = false;
  spc->v = false;
  spc->n = false;
  spc->i = false;
  spc->h = false;
  spc->p = false;
  spc->b = false;
  spc->stopped = false;
  spc->cyclesUsed = 0;
}

void spc_saveload(Spc *spc, SaveLoadFunc *func, void *ctx) {
  func(ctx, &spc->a, offsetof(Spc, cyclesUsed) - offsetof(Spc, a));
}

int spc_runOpcode(Spc* spc) {
  spc->cyclesUsed = 0;
  if(spc->stopped) return 1;
  uint8_t opcode = spc_readOpcode(spc);
  spc->cyclesUsed = cyclesPerOpcode[opcode];
  spc_doOpcode(spc, opcode);
  return spc->cyclesUsed;
}

static uint8_t spc_readOpcode(Spc* spc) {
  return spc_read(spc, spc->pc++);
}

static uint16_t spc_readOpcodeWord(Spc* spc) {
  uint8_t low = spc_readOpcode(spc);
  return low | (spc_readOpcode(spc) << 8);
}

static uint8_t spc_getFlags(Spc* spc) {
  uint8_t val = spc->n << 7;
  val |= spc->v << 6;
  val |= spc->p << 5;
  val |= spc->b << 4;
  val |= spc->h << 3;
  val |= spc->i << 2;
  val |= spc->z << 1;
  val |= (uint8_t)spc->c;
  return val;
}

static void spc_setFlags(Spc* spc, uint8_t val) {
  spc->n = val & 0x80;
  spc->v = val & 0x40;
  spc->p = val & 0x20;
  spc->b = val & 0x10;
  spc->h = val & 8;
  spc->i = val & 4;
  spc->z = val & 2;
  spc->c = val & 1;
}

static void spc_setZN(Spc* spc, uint8_t value) {
  spc->z = value == 0;
  spc->n = value & 0x80;
}

static void spc_doBranch(Spc* spc, uint8_t value, bool check) {
  if(check) {
    spc->cyclesUsed += 2; // taken branch: 2 extra cycles
    spc->pc += (int8_t) value;
  }
}

static uint8_t spc_pullByte(Spc* spc) {
  spc->sp++;
  return spc_read(spc, 0x100 | spc->sp);
}

static void spc_pushByte(Spc* spc, uint8_t value) {
  spc_write(spc, 0x100 | spc->sp, value);
  spc->sp--;
}

static uint16_t spc_pullWord(Spc* spc) {
  uint8_t value = spc_pullByte(spc);
  return value | (spc_pullByte(spc) << 8);
}

static void spc_pushWord(Spc* spc, uint16_t value) {
  spc_pushByte(spc, value >> 8);
  spc_pushByte(spc, value & 0xff);
}

static uint16_t spc_readWord(Spc* spc, uint16_t adrl, uint16_t adrh) {
  uint8_t value = spc_read(spc, adrl);
  return value | (spc_read(spc, adrh) << 8);
}

static void spc_writeWord(Spc* spc, uint16_t adrl, uint16_t adrh, uint16_t value) {
  spc_write(spc, adrl, value & 0xff);
  spc_write(spc, adrh, value >> 8);
}

// adressing modes

static uint16_t spc_adrDp(Spc* spc) {
  return spc_readOpcode(spc) | (spc->p << 8);
}

static uint16_t spc_adrAbs(Spc* spc) {
  return spc_readOpcodeWord(spc);
}

static uint16_t spc_adrInd(Spc* spc) {
  return spc->x | (spc->p << 8);
}

static uint16_t spc_adrIdx(Spc* spc) {
  uint8_t pointer = spc_readOpcode(spc);
  return spc_readWord(spc, ((pointer + spc->x) & 0xff) | (spc->p << 8), ((pointer + spc->x + 1) & 0xff) | (spc->p << 8));
}

static uint16_t spc_adrImm(Spc* spc) {
  return spc->pc++;
}

static uint16_t spc_adrDpx(Spc* spc) {
  return ((spc_readOpcode(spc) + spc->x) & 0xff) | (spc->p << 8);
}

static uint16_t spc_adrDpy(Spc* spc) {
  return ((spc_readOpcode(spc) + spc->y) & 0xff) | (spc->p << 8);
}

static uint16_t spc_adrAbx(Spc* spc) {
  return (spc_readOpcodeWord(spc) + spc->x) & 0xffff;
}

static uint16_t spc_adrAby(Spc* spc) {
  return (spc_readOpcodeWord(spc) + spc->y) & 0xffff;
}

static uint16_t spc_adrIdy(Spc* spc) {
  uint8_t pointer = spc_readOpcode(spc);
  uint16_t adr = spc_readWord(spc, pointer | (spc->p << 8), ((pointer + 1) & 0xff) | (spc->p << 8));
  return (adr + spc->y) & 0xffff;
}

static uint16_t spc_adrDpDp(Spc* spc, uint16_t* src) {
  *src = spc_readOpcode(spc) | (spc->p << 8);
  return spc_readOpcode(spc) | (spc->p << 8);
}

static uint16_t spc_adrDpImm(Spc* spc, uint16_t* src) {
  *src = spc->pc++;
  return spc_readOpcode(spc) | (spc->p << 8);
}

static uint16_t spc_adrIndInd(Spc* spc, uint16_t* src) {
  *src = spc->y | (spc->p << 8);
  return spc->x | (spc->p << 8);
}

static uint8_t spc_adrAbsBit(Spc* spc, uint16_t* adr) {
  uint16_t adrBit = spc_readOpcodeWord(spc);
  *adr = adrBit & 0x1fff;
  return adrBit >> 13;
}

static uint16_t spc_adrDpWord(Spc* spc, uint16_t* low) {
  uint8_t adr = spc_readOpcode(spc);
  *low = adr | (spc->p << 8);
  return ((adr + 1) & 0xff) | (spc->p << 8);
}

static uint16_t spc_adrIndP(Spc* spc) {
  return spc->x++ | (spc->p << 8);
}

// opcode functions

static void spc_and(Spc* spc, uint16_t adr) {
  spc->a &= spc_read(spc, adr);
  spc_setZN(spc, spc->a);
}

static void spc_andm(Spc* spc, uint16_t dst, uint16_t src) {
  uint8_t value = spc_read(spc, src);
  uint8_t result = spc_read(spc, dst) & value;
  spc_write(spc, dst, result);
  spc_setZN(spc, result);
}

static void spc_or(Spc* spc, uint16_t adr) {
  spc->a |= spc_read(spc, adr);
  spc_setZN(spc, spc->a);
}

static void spc_orm(Spc* spc, uint16_t dst, uint16_t src) {
  uint8_t value = spc_read(spc, src);
  uint8_t result = spc_read(spc, dst) | value;
  spc_write(spc, dst, result);
  spc_setZN(spc, result);
}

static void spc_eor(Spc* spc, uint16_t adr) {
  spc->a ^= spc_read(spc, adr);
  spc_setZN(spc, spc->a);
}

static void spc_eorm(Spc* spc, uint16_t dst, uint16_t src) {
  uint8_t value = spc_read(spc, src);
  uint8_t result = spc_read(spc, dst) ^ value;
  spc_write(spc, dst, result);
  spc_setZN(spc, result);
}

static void spc_adc(Spc* spc, uint16_t adr) {
  uint8_t value = spc_read(spc, adr);
  int result = spc->a + value + spc->c;
  spc->v = (spc->a & 0x80) == (value & 0x80) && (value & 0x80) != (result & 0x80);
  spc->h = ((spc->a & 0xf) + (value & 0xf) + spc->c) > 0xf;
  spc->c = result > 0xff;
  spc->a = result;
  spc_setZN(spc, spc->a);
}

static void spc_adcm(Spc* spc, uint16_t dst, uint16_t src) {
  uint8_t value = spc_read(spc, src);
  uint8_t applyOn = spc_read(spc, dst);
  int result = applyOn + value + spc->c;
  spc->v = (applyOn & 0x80) == (value & 0x80) && (value & 0x80) != (result & 0x80);
  spc->h = ((applyOn & 0xf) + (value & 0xf) + spc->c) > 0xf;
  spc->c = result > 0xff;
  spc_write(spc, dst, result);
  spc_setZN(spc, result);
}

static void spc_sbc(Spc* spc, uint16_t adr) {
  uint8_t value = spc_read(spc, adr) ^ 0xff;
  int result = spc->a + value + spc->c;
  spc->v = (spc->a & 0x80) == (value & 0x80) && (value & 0x80) != (result & 0x80);
  spc->h = ((spc->a & 0xf) + (value & 0xf) + spc->c) > 0xf;
  spc->c = result > 0xff;
  spc->a = result;
  spc_setZN(spc, spc->a);
}

static void spc_sbcm(Spc* spc, uint16_t dst, uint16_t src) {
  uint8_t value = spc_read(spc, src) ^ 0xff;
  uint8_t applyOn = spc_read(spc, dst);
  int result = applyOn + value + spc->c;
  spc->v = (applyOn & 0x80) == (value & 0x80) && (value & 0x80) != (result & 0x80);
  spc->h = ((applyOn & 0xf) + (value & 0xf) + spc->c) > 0xf;
  spc->c = result > 0xff;
  spc_write(spc, dst, result);
  spc_setZN(spc, result);
}

static void spc_cmp(Spc* spc, uint16_t adr) {
  uint8_t value = spc_read(spc, adr) ^ 0xff;
  int result = spc->a + value + 1;
  spc->c = result > 0xff;
  spc_setZN(spc, result);
}

static void spc_cmpx(Spc* spc, uint16_t adr) {
  uint8_t value = spc_read(spc, adr) ^ 0xff;
  int result = spc->x + value + 1;
  spc->c = result > 0xff;
  spc_setZN(spc, result);
}

static void spc_cmpy(Spc* spc, uint16_t adr) {
  uint8_t value = spc_read(spc, adr) ^ 0xff;
  int result = spc->y + value + 1;
  spc->c = result > 0xff;
  spc_setZN(spc, result);
}

static void spc_cmpm(Spc* spc, uint16_t dst, uint16_t src) {
  uint8_t value = spc_read(spc, src) ^ 0xff;
  int result = spc_read(spc, dst) + value + 1;
  spc->c = result > 0xff;
  spc_setZN(spc, result);
}

static void spc_mov(Spc* spc, uint16_t adr) {
  spc->a = spc_read(spc, adr);
  spc_setZN(spc, spc->a);
}

static void spc_movx(Spc* spc, uint16_t adr) {
  spc->x = spc_read(spc, adr);
  spc_setZN(spc, spc->x);
}

static void spc_movy(Spc* spc, uint16_t adr) {
  spc->y = spc_read(spc, adr);
  spc_setZN(spc, spc->y);
}

static void spc_movs(Spc* spc, uint16_t adr) {
  spc_read(spc, adr);
  spc_write(spc, adr, spc->a);
}

static void spc_movsx(Spc* spc, uint16_t adr) {
  spc_read(spc, adr);
  spc_write(spc, adr, spc->x);
}

static void spc_movsy(Spc* spc, uint16_t adr) {
  spc_read(spc, adr);
  spc_write(spc, adr, spc->y);
}

static void spc_asl(Spc* spc, uint16_t adr) {
  uint8_t val = spc_read(spc, adr);
  spc->c = val & 0x80;
  val <<= 1;
  spc_write(spc, adr, val);
  spc_setZN(spc, val);
}

static void spc_lsr(Spc* spc, uint16_t adr) {
  uint8_t val = spc_read(spc, adr);
  spc->c = val & 1;
  val >>= 1;
  spc_write(spc, adr, val);
  spc_setZN(spc, val);
}

static void spc_rol(Spc* spc, uint16_t adr) {
  uint8_t val = spc_read(spc, adr);
  bool newC = val & 0x80;
  val = (val << 1) | (uint8_t)spc->c;
  spc->c = newC;
  spc_write(spc, adr, val);
  spc_setZN(spc, val);
}

static void spc_ror(Spc* spc, uint16_t adr) {
  uint8_t val = spc_read(spc, adr);
  bool newC = val & 1;
  val = (val >> 1) | (spc->c << 7);
  spc->c = newC;
  spc_write(spc, adr, val);
  spc_setZN(spc, val);
}

static void spc_inc(Spc* spc, uint16_t adr) {
  uint8_t val = spc_read(spc, adr) + 1;
  spc_write(spc, adr, val);
  spc_setZN(spc, val);
}

static void spc_dec(Spc* spc, uint16_t adr) {
  uint8_t val = spc_read(spc, adr) - 1;
  spc_write(spc, adr, val);
  spc_setZN(spc, val);
}

static void spc_doOpcode(Spc* spc, uint8_t opcode) {
  switch(opcode) {
    case 0x00: { // nop imp
      // no operation
      break;
    }
    case 0x01:
    case 0x11:
    case 0x21:
    case 0x31:
    case 0x41:
    case 0x51:
    case 0x61:
    case 0x71:
    case 0x81:
    case 0x91:
    case 0xa1:
    case 0xb1:
    case 0xc1:
    case 0xd1:
    case 0xe1:
    case 0xf1: { // tcall imp
      spc_pushWord(spc, spc->pc);
      uint16_t adr = 0xffde - (2 * (opcode >> 4));
      spc->pc = spc_readWord(spc, adr, adr + 1);
      break;
    }
    case 0x02:
    case 0x22:
    case 0x42:
    case 0x62:
    case 0x82:
    case 0xa2:
    case 0xc2:
    case 0xe2: { // set1 dp
      uint16_t adr = spc_adrDp(spc);
      spc_write(spc, adr, spc_read(spc, adr) | (1 << (opcode >> 5)));
      break;
    }
    case 0x12:
    case 0x32:
    case 0x52:
    case 0x72:
    case 0x92:
    case 0xb2:
    case 0xd2:
    case 0xf2: { // clr1 dp
      uint16_t adr = spc_adrDp(spc);
      spc_write(spc, adr, spc_read(spc, adr) & ~(1 << (opcode >> 5)));
      break;
    }
    case 0x03:
    case 0x23:
    case 0x43:
    case 0x63:
    case 0x83:
    case 0xa3:
    case 0xc3:
    case 0xe3: { // bbs dp, rel
      uint8_t val = spc_read(spc, spc_adrDp(spc));
      spc_doBranch(spc, spc_readOpcode(spc), val & (1 << (opcode >> 5)));
      break;
    }
    case 0x13:
    case 0x33:
    case 0x53:
    case 0x73:
    case 0x93:
    case 0xb3:
    case 0xd3:
    case 0xf3: { // bbc dp, rel
      uint8_t val = spc_read(spc, spc_adrDp(spc));
      spc_doBranch(spc, spc_readOpcode(spc), (val & (1 << (opcode >> 5))) == 0);
      break;
    }
    case 0x04: { // or  dp
      spc_or(spc, spc_adrDp(spc));
      break;
    }
    case 0x05: { // or  abs
      spc_or(spc, spc_adrAbs(spc));
      break;
    }
    case 0x06: { // or  ind
      spc_or(spc, spc_adrInd(spc));
      break;
    }
    case 0x07: { // or  idx
      spc_or(spc, spc_adrIdx(spc));
      break;
    }
    case 0x08: { // or  imm
      spc_or(spc, spc_adrImm(spc));
      break;
    }
    case 0x09: { // orm dp, dp
      uint16_t src = 0;
      uint16_t dst = spc_adrDpDp(spc, &src);
      spc_orm(spc, dst, src);
      break;
    }
    case 0x0a: { // or1 abs.bit
      uint16_t adr = 0;
      uint8_t bit = spc_adrAbsBit(spc, &adr);
      spc->c = spc->c | ((spc_read(spc, adr) >> bit) & 1);
      break;
    }
    case 0x0b: { // asl dp
      spc_asl(spc, spc_adrDp(spc));
      break;
    }
    case 0x0c: { // asl abs
      spc_asl(spc, spc_adrAbs(spc));
      break;
    }
    case 0x0d: { // pushp imp
      spc_pushByte(spc, spc_getFlags(spc));
      break;
    }
    case 0x0e: { // tset1 abs
      uint16_t adr = spc_adrAbs(spc);
      uint8_t val = spc_read(spc, adr);
      uint8_t result = spc->a + (val ^ 0xff) + 1;
      spc_setZN(spc, result);
      spc_write(spc, adr, val | spc->a);
      break;
    }
    case 0x0f: { // brk imp
      spc_pushWord(spc, spc->pc);
      spc_pushByte(spc, spc_getFlags(spc));
      spc->i = false;
      spc->b = true;
      spc->pc = spc_readWord(spc, 0xffde, 0xffdf);
      break;
    }
    case 0x10: { // bpl rel
      spc_doBranch(spc, spc_readOpcode(spc), !spc->n);
      break;
    }
    case 0x14: { // or  dpx
      spc_or(spc, spc_adrDpx(spc));
      break;
    }
    case 0x15: { // or  abx
      spc_or(spc, spc_adrAbx(spc));
      break;
    }
    case 0x16: { // or  aby
      spc_or(spc, spc_adrAby(spc));
      break;
    }
    case 0x17: { // or  idy
      spc_or(spc, spc_adrIdy(spc));
      break;
    }
    case 0x18: { // orm dp, imm
      uint16_t src = 0;
      uint16_t dst = spc_adrDpImm(spc, &src);
      spc_orm(spc, dst, src);
      break;
    }
    case 0x19: { // orm ind, ind
      uint16_t src = 0;
      uint16_t dst = spc_adrIndInd(spc, &src);
      spc_orm(spc, dst, src);
      break;
    }
    case 0x1a: { // decw dp
      uint16_t low = 0;
      uint16_t high = spc_adrDpWord(spc, &low);
      uint16_t value = spc_readWord(spc, low, high) - 1;
      spc->z = value == 0;
      spc->n = value & 0x8000;
      spc_writeWord(spc, low, high, value);
      break;
    }
    case 0x1b: { // asl dpx
      spc_asl(spc, spc_adrDpx(spc));
      break;
    }
    case 0x1c: { // asla imp
      spc->c = spc->a & 0x80;
      spc->a <<= 1;
      spc_setZN(spc, spc->a);
      break;
    }
    case 0x1d: { // decx imp
      spc->x--;
      spc_setZN(spc, spc->x);
      break;
    }
    case 0x1e: { // cmpx abs
      spc_cmpx(spc, spc_adrAbs(spc));
      break;
    }
    case 0x1f: { // jmp iax
      uint16_t pointer = spc_readOpcodeWord(spc);
      spc->pc = spc_readWord(spc, (pointer + spc->x) & 0xffff, (pointer + spc->x + 1) & 0xffff);
      break;
    }
    case 0x20: { // clrp imp
      spc->p = false;
      break;
    }
    case 0x24: { // and dp
      spc_and(spc, spc_adrDp(spc));
      break;
    }
    case 0x25: { // and abs
      spc_and(spc, spc_adrAbs(spc));
      break;
    }
    case 0x26: { // and ind
      spc_and(spc, spc_adrInd(spc));
      break;
    }
    case 0x27: { // and idx
      spc_and(spc, spc_adrIdx(spc));
      break;
    }
    case 0x28: { // and imm
      spc_and(spc, spc_adrImm(spc));
      break;
    }
    case 0x29: { // andm dp, dp
      uint16_t src = 0;
      uint16_t dst = spc_adrDpDp(spc, &src);
      spc_andm(spc, dst, src);
      break;
    }
    case 0x2a: { // or1n abs.bit
      uint16_t adr = 0;
      uint8_t bit = spc_adrAbsBit(spc, &adr);
      spc->c = spc->c | (~(spc_read(spc, adr) >> bit) & 1);
      break;
    }
    case 0x2b: { // rol dp
      spc_rol(spc, spc_adrDp(spc));
      break;
    }
    case 0x2c: { // rol abs
      spc_rol(spc, spc_adrAbs(spc));
      break;
    }
    case 0x2d: { // pusha imp
      spc_pushByte(spc, spc->a);
      break;
    }
    case 0x2e: { // cbne dp, rel
      uint8_t val = spc_read(spc, spc_adrDp(spc)) ^ 0xff;
      uint8_t result = spc->a + val + 1;
      spc_doBranch(spc, spc_readOpcode(spc), result != 0);
      break;
    }
    case 0x2f: { // bra rel
      spc->pc += (int8_t) spc_readOpcode(spc);
      break;
    }
    case 0x30: { // bmi rel
      spc_doBranch(spc, spc_readOpcode(spc), spc->n);
      break;
    }
    case 0x34: { // and dpx
      spc_and(spc, spc_adrDpx(spc));
      break;
    }
    case 0x35: { // and abx
      spc_and(spc, spc_adrAbx(spc));
      break;
    }
    case 0x36: { // and aby
      spc_and(spc, spc_adrAby(spc));
      break;
    }
    case 0x37: { // and idy
      spc_and(spc, spc_adrIdy(spc));
      break;
    }
    case 0x38: { // andm dp, imm
      uint16_t src = 0;
      uint16_t dst = spc_adrDpImm(spc, &src);
      spc_andm(spc, dst, src);
      break;
    }
    case 0x39: { // andm ind, ind
      uint16_t src = 0;
      uint16_t dst = spc_adrIndInd(spc, &src);
      spc_andm(spc, dst, src);
      break;
    }
    case 0x3a: { // incw dp
      uint16_t low = 0;
      uint16_t high = spc_adrDpWord(spc, &low);
      uint16_t value = spc_readWord(spc, low, high) + 1;
      spc->z = value == 0;
      spc->n = value & 0x8000;
      spc_writeWord(spc, low, high, value);
      break;
    }
    case 0x3b: { // rol dpx
      spc_rol(spc, spc_adrDpx(spc));
      break;
    }
    case 0x3c: { // rola imp
      bool newC = spc->a & 0x80;
      spc->a = (spc->a << 1) | (uint8_t)spc->c;
      spc->c = newC;
      spc_setZN(spc, spc->a);
      break;
    }
    case 0x3d: { // incx imp
      spc->x++;
      spc_setZN(spc, spc->x);
      break;
    }
    case 0x3e: { // cmpx dp
      spc_cmpx(spc, spc_adrDp(spc));
      break;
    }
    case 0x3f: { // call abs
      uint16_t dst = spc_readOpcodeWord(spc);
      spc_pushWord(spc, spc->pc);
      spc->pc = dst;
      break;
    }
    case 0x40: { // setp imp
      spc->p = true;
      break;
    }
    case 0x44: { // eor dp
      spc_eor(spc, spc_adrDp(spc));
      break;
    }
    case 0x45: { // eor abs
      spc_eor(spc, spc_adrAbs(spc));
      break;
    }
    case 0x46: { // eor ind
      spc_eor(spc, spc_adrInd(spc));
      break;
    }
    case 0x47: { // eor idx
      spc_eor(spc, spc_adrIdx(spc));
      break;
    }
    case 0x48: { // eor imm
      spc_eor(spc, spc_adrImm(spc));
      break;
    }
    case 0x49: { // eorm dp, dp
      uint16_t src = 0;
      uint16_t dst = spc_adrDpDp(spc, &src);
      spc_eorm(spc, dst, src);
      break;
    }
    case 0x4a: { // and1 abs.bit
      uint16_t adr = 0;
      uint8_t bit = spc_adrAbsBit(spc, &adr);
      spc->c = spc->c & ((spc_read(spc, adr) >> bit) & 1);
      break;
    }
    case 0x4b: { // lsr dp
      spc_lsr(spc, spc_adrDp(spc));
      break;
    }
    case 0x4c: { // lsr abs
      spc_lsr(spc, spc_adrAbs(spc));
      break;
    }
    case 0x4d: { // pushx imp
      spc_pushByte(spc, spc->x);
      break;
    }
    case 0x4e: { // tclr1 abs
      uint16_t adr = spc_adrAbs(spc);
      uint8_t val = spc_read(spc, adr);
      uint8_t result = spc->a + (val ^ 0xff) + 1;
      spc_setZN(spc, result);
      spc_write(spc, adr, val & ~spc->a);
      break;
    }
    case 0x4f: { // pcall dp
      uint8_t dst = spc_readOpcode(spc);
      spc_pushWord(spc, spc->pc);
      spc->pc = 0xff00 | dst;
      break;
    }
    case 0x50: { // bvc rel
      spc_doBranch(spc, spc_readOpcode(spc), !spc->v);
      break;
    }
    case 0x54: { // eor dpx
      spc_eor(spc, spc_adrDpx(spc));
      break;
    }
    case 0x55: { // eor abx
      spc_eor(spc, spc_adrAbx(spc));
      break;
    }
    case 0x56: { // eor aby
      spc_eor(spc, spc_adrAby(spc));
      break;
    }
    case 0x57: { // eor idy
      spc_eor(spc, spc_adrIdy(spc));
      break;
    }
    case 0x58: { // eorm dp, imm
      uint16_t src = 0;
      uint16_t dst = spc_adrDpImm(spc, &src);
      spc_eorm(spc, dst, src);
      break;
    }
    case 0x59: { // eorm ind, ind
      uint16_t src = 0;
      uint16_t dst = spc_adrIndInd(spc, &src);
      spc_eorm(spc, dst, src);
      break;
    }
    case 0x5a: { // cmpw dp
      uint16_t low = 0;
      uint16_t high = spc_adrDpWord(spc, &low);
      uint16_t value = spc_readWord(spc, low, high) ^ 0xffff;
      uint16_t ya = spc->a | (spc->y << 8);
      int result = ya + value + 1;
      spc->c = result > 0xffff;
      spc->z = (result & 0xffff) == 0;
      spc->n = result & 0x8000;
      break;
    }
    case 0x5b: { // lsr dpx
      spc_lsr(spc, spc_adrDpx(spc));
      break;
    }
    case 0x5c: { // lsra imp
      spc->c = spc->a & 1;
      spc->a >>= 1;
      spc_setZN(spc, spc->a);
      break;
    }
    case 0x5d: { // movxa imp
      spc->x = spc->a;
      spc_setZN(spc, spc->x);
      break;
    }
    case 0x5e: { // cmpy abs
      spc_cmpy(spc, spc_adrAbs(spc));
      break;
    }
    case 0x5f: { // jmp abs
      spc->pc = spc_readOpcodeWord(spc);
      break;
    }
    case 0x60: { // clrc imp
      spc->c = false;
      break;
    }
    case 0x64: { // cmp dp
      spc_cmp(spc, spc_adrDp(spc));
      break;
    }
    case 0x65: { // cmp abs
      spc_cmp(spc, spc_adrAbs(spc));
      break;
    }
    case 0x66: { // cmp ind
      spc_cmp(spc, spc_adrInd(spc));
      break;
    }
    case 0x67: { // cmp idx
      spc_cmp(spc, spc_adrIdx(spc));
      break;
    }
    case 0x68: { // cmp imm
      spc_cmp(spc, spc_adrImm(spc));
      break;
    }
    case 0x69: { // cmpm dp, dp
      uint16_t src = 0;
      uint16_t dst = spc_adrDpDp(spc, &src);
      spc_cmpm(spc, dst, src);
      break;
    }
    case 0x6a: { // and1n abs.bit
      uint16_t adr = 0;
      uint8_t bit = spc_adrAbsBit(spc, &adr);
      spc->c = spc->c & (~(spc_read(spc, adr) >> bit) & 1);
      break;
    }
    case 0x6b: { // ror dp
      spc_ror(spc, spc_adrDp(spc));
      break;
    }
    case 0x6c: { // ror abs
      spc_ror(spc, spc_adrAbs(spc));
      break;
    }
    case 0x6d: { // pushy imp
      spc_pushByte(spc, spc->y);
      break;
    }
    case 0x6e: { // dbnz dp, rel
      uint16_t adr = spc_adrDp(spc);
      uint8_t result = spc_read(spc, adr) - 1;
      spc_write(spc, adr, result);
      spc_doBranch(spc, spc_readOpcode(spc), result != 0);
      break;
    }
    case 0x6f: { // ret imp
      spc->pc = spc_pullWord(spc);
      break;
    }
    case 0x70: { // bvs rel
      spc_doBranch(spc, spc_readOpcode(spc), spc->v);
      break;
    }
    case 0x74: { // cmp dpx
      spc_cmp(spc, spc_adrDpx(spc));
      break;
    }
    case 0x75: { // cmp abx
      spc_cmp(spc, spc_adrAbx(spc));
      break;
    }
    case 0x76: { // cmp aby
      spc_cmp(spc, spc_adrAby(spc));
      break;
    }
    case 0x77: { // cmp idy
      spc_cmp(spc, spc_adrIdy(spc));
      break;
    }
    case 0x78: { // cmpm dp, imm
      uint16_t src = 0;
      uint16_t dst = spc_adrDpImm(spc, &src);
      spc_cmpm(spc, dst, src);
      break;
    }
    case 0x79: { // cmpm ind, ind
      uint16_t src = 0;
      uint16_t dst = spc_adrIndInd(spc, &src);
      spc_cmpm(spc, dst, src);
      break;
    }
    case 0x7a: { // addw dp
      uint16_t low = 0;
      uint16_t high = spc_adrDpWord(spc, &low);
      uint16_t value = spc_readWord(spc, low, high);
      uint16_t ya = spc->a | (spc->y << 8);
      int result = ya + value;
      spc->v = (ya & 0x8000) == (value & 0x8000) && (value & 0x8000) != (result & 0x8000);
      spc->h = ((ya & 0xfff) + (value & 0xfff) + 1) > 0xfff;
      spc->c = result > 0xffff;
      spc->z = (result & 0xffff) == 0;
      spc->n = result & 0x8000;
      spc->a = result & 0xff;
      spc->y = result >> 8;
      break;
    }
    case 0x7b: { // ror dpx
      spc_ror(spc, spc_adrDpx(spc));
      break;
    }
    case 0x7c: { // rora imp
      bool newC = spc->a & 1;
      spc->a = (spc->a >> 1) | (spc->c << 7);
      spc->c = newC;
      spc_setZN(spc, spc->a);
      break;
    }
    case 0x7d: { // movax imp
      spc->a = spc->x;
      spc_setZN(spc, spc->a);
      break;
    }
    case 0x7e: { // cmpy dp
      spc_cmpy(spc, spc_adrDp(spc));
      break;
    }
    case 0x7f: { // reti imp
      spc_setFlags(spc, spc_pullByte(spc));
      spc->pc = spc_pullWord(spc);
      break;
    }
    case 0x80: { // setc imp
      spc->c = true;
      break;
    }
    case 0x84: { // adc dp
      spc_adc(spc, spc_adrDp(spc));
      break;
    }
    case 0x85: { // adc abs
      spc_adc(spc, spc_adrAbs(spc));
      break;
    }
    case 0x86: { // adc ind
      spc_adc(spc, spc_adrInd(spc));
      break;
    }
    case 0x87: { // adc idx
      spc_adc(spc, spc_adrIdx(spc));
      break;
    }
    case 0x88: { // adc imm
      spc_adc(spc, spc_adrImm(spc));
      break;
    }
    case 0x89: { // adcm dp, dp
      uint16_t src = 0;
      uint16_t dst = spc_adrDpDp(spc, &src);
      spc_adcm(spc, dst, src);
      break;
    }
    case 0x8a: { // eor1 abs.bit
      uint16_t adr = 0;
      uint8_t bit = spc_adrAbsBit(spc, &adr);
      spc->c = spc->c ^ ((spc_read(spc, adr) >> bit) & 1);
      break;
    }
    case 0x8b: { // dec dp
      spc_dec(spc, spc_adrDp(spc));
      break;
    }
    case 0x8c: { // dec abs
      spc_dec(spc, spc_adrAbs(spc));
      break;
    }
    case 0x8d: { // movy imm
      spc_movy(spc, spc_adrImm(spc));
      break;
    }
    case 0x8e: { // popp imp
      spc_setFlags(spc, spc_pullByte(spc));
      break;
    }
    case 0x8f: { // movm dp, imm
      uint16_t src = 0;
      uint16_t dst = spc_adrDpImm(spc, &src);
      uint8_t val = spc_read(spc, src);
      spc_read(spc, dst);
      spc_write(spc, dst, val);
      break;
    }
    case 0x90: { // bcc rel
      spc_doBranch(spc, spc_readOpcode(spc), !spc->c);
      break;
    }
    case 0x94: { // adc dpx
      spc_adc(spc, spc_adrDpx(spc));
      break;
    }
    case 0x95: { // adc abx
      spc_adc(spc, spc_adrAbx(spc));
      break;
    }
    case 0x96: { // adc aby
      spc_adc(spc, spc_adrAby(spc));
      break;
    }
    case 0x97: { // adc idy
      spc_adc(spc, spc_adrIdy(spc));
      break;
    }
    case 0x98: { // adcm dp, imm
      uint16_t src = 0;
      uint16_t dst = spc_adrDpImm(spc, &src);
      spc_adcm(spc, dst, src);
      break;
    }
    case 0x99: { // adcm ind, ind
      uint16_t src = 0;
      uint16_t dst = spc_adrIndInd(spc, &src);
      spc_adcm(spc, dst, src);
      break;
    }
    case 0x9a: { // subw dp
      uint16_t low = 0;
      uint16_t high = spc_adrDpWord(spc, &low);
      uint16_t value = spc_readWord(spc, low, high) ^ 0xffff;
      uint16_t ya = spc->a | (spc->y << 8);
      int result = ya + value + 1;
      spc->v = (ya & 0x8000) == (value & 0x8000) && (value & 0x8000) != (result & 0x8000);
      spc->h = ((ya & 0xfff) + (value & 0xfff) + 1) > 0xfff;
      spc->c = result > 0xffff;
      spc->z = (result & 0xffff) == 0;
      spc->n = result & 0x8000;
      spc->a = result & 0xff;
      spc->y = result >> 8;
      break;
    }
    case 0x9b: { // dec dpx
      spc_dec(spc, spc_adrDpx(spc));
      break;
    }
    case 0x9c: { // deca imp
      spc->a--;
      spc_setZN(spc, spc->a);
      break;
    }
    case 0x9d: { // movxp imp
      spc->x = spc->sp;
      spc_setZN(spc, spc->x);
      break;
    }
    case 0x9e: { // div imp
      // TODO: proper division algorithm
      uint16_t value = spc->a | (spc->y << 8);
      int result = 0xffff;
      int mod = spc->a;
      if(spc->x != 0) {
        result = value / spc->x;
        mod = value % spc->x;
      }
      spc->v = result > 0xff;
      spc->h = (spc->x & 0xf) <= (spc->y & 0xf);
      spc->a = result;
      spc->y = mod;
      spc_setZN(spc, spc->a);
      break;
    }
    case 0x9f: { // xcn imp
      spc->a = (spc->a >> 4) | (spc->a << 4);
      spc_setZN(spc, spc->a);
      break;
    }
    case 0xa0: { // ei  imp
      spc->i = true;
      break;
    }
    case 0xa4: { // sbc dp
      spc_sbc(spc, spc_adrDp(spc));
      break;
    }
    case 0xa5: { // sbc abs
      spc_sbc(spc, spc_adrAbs(spc));
      break;
    }
    case 0xa6: { // sbc ind
      spc_sbc(spc, spc_adrInd(spc));
      break;
    }
    case 0xa7: { // sbc idx
      spc_sbc(spc, spc_adrIdx(spc));
      break;
    }
    case 0xa8: { // sbc imm
      spc_sbc(spc, spc_adrImm(spc));
      break;
    }
    case 0xa9: { // sbcm dp, dp
      uint16_t src = 0;
      uint16_t dst = spc_adrDpDp(spc, &src);
      spc_sbcm(spc, dst, src);
      break;
    }
    case 0xaa: { // mov1 abs.bit
      uint16_t adr = 0;
      uint8_t bit = spc_adrAbsBit(spc, &adr);
      spc->c = (spc_read(spc, adr) >> bit) & 1;
      break;
    }
    case 0xab: { // inc dp
      spc_inc(spc, spc_adrDp(spc));
      break;
    }
    case 0xac: { // inc abs
      spc_inc(spc, spc_adrAbs(spc));
      break;
    }
    case 0xad: { // cmpy imm
      spc_cmpy(spc, spc_adrImm(spc));
      break;
    }
    case 0xae: { // popa imp
      spc->a = spc_pullByte(spc);
      break;
    }
    case 0xaf: { // movs ind+
      uint16_t adr = spc_adrIndP(spc);
      spc_write(spc, adr, spc->a);
      break;
    }
    case 0xb0: { // bcs rel
      spc_doBranch(spc, spc_readOpcode(spc), spc->c);
      break;
    }
    case 0xb4: { // sbc dpx
      spc_sbc(spc, spc_adrDpx(spc));
      break;
    }
    case 0xb5: { // sbc abx
      spc_sbc(spc, spc_adrAbx(spc));
      break;
    }
    case 0xb6: { // sbc aby
      spc_sbc(spc, spc_adrAby(spc));
      break;
    }
    case 0xb7: { // sbc idy
      spc_sbc(spc, spc_adrIdy(spc));
      break;
    }
    case 0xb8: { // sbcm dp, imm
      uint16_t src = 0;
      uint16_t dst = spc_adrDpImm(spc, &src);
      spc_sbcm(spc, dst, src);
      break;
    }
    case 0xb9: { // sbcm ind, ind
      uint16_t src = 0;
      uint16_t dst = spc_adrIndInd(spc, &src);
      spc_sbcm(spc, dst, src);
      break;
    }
    case 0xba: { // movw dp
      uint16_t low = 0;
      uint16_t high = spc_adrDpWord(spc, &low);
      uint16_t val = spc_readWord(spc, low, high);
      spc->a = val & 0xff;
      spc->y = val >> 8;
      spc->z = val == 0;
      spc->n = val & 0x8000;
      break;
    }
    case 0xbb: { // inc dpx
      spc_inc(spc, spc_adrDpx(spc));
      break;
    }
    case 0xbc: { // inca imp
      spc->a++;
      spc_setZN(spc, spc->a);
      break;
    }
    case 0xbd: { // movpx imp
      spc->sp = spc->x;
      break;
    }
    case 0xbe: { // das imp
      if(spc->a > 0x99 || !spc->c) {
        spc->a -= 0x60;
        spc->c = false;
      }
      if((spc->a & 0xf) > 9 || !spc->h) {
        spc->a -= 6;
      }
      spc_setZN(spc, spc->a);
      break;
    }
    case 0xbf: { // mov ind+
      uint16_t adr = spc_adrIndP(spc);
      spc->a = spc_read(spc, adr);
      spc_setZN(spc, spc->a);
      break;
    }
    case 0xc0: { // di  imp
      spc->i = false;
      break;
    }
    case 0xc4: { // movs dp
      spc_movs(spc, spc_adrDp(spc));
      break;
    }
    case 0xc5: { // movs abs
      spc_movs(spc, spc_adrAbs(spc));
      break;
    }
    case 0xc6: { // movs ind
      spc_movs(spc, spc_adrInd(spc));
      break;
    }
    case 0xc7: { // movs idx
      spc_movs(spc, spc_adrIdx(spc));
      break;
    }
    case 0xc8: { // cmpx imm
      spc_cmpx(spc, spc_adrImm(spc));
      break;
    }
    case 0xc9: { // movsx abs
      spc_movsx(spc, spc_adrAbs(spc));
      break;
    }
    case 0xca: { // mov1s abs.bit
      uint16_t adr = 0;
      uint8_t bit = spc_adrAbsBit(spc, &adr);
      uint8_t result = (spc_read(spc, adr) & (~(1 << bit))) | (spc->c << bit);
      spc_write(spc, adr, result);
      break;
    }
    case 0xcb: { // movsy dp
      spc_movsy(spc, spc_adrDp(spc));
      break;
    }
    case 0xcc: { // movsy abs
      spc_movsy(spc, spc_adrAbs(spc));
      break;
    }
    case 0xcd: { // movx imm
      spc_movx(spc, spc_adrImm(spc));
      break;
    }
    case 0xce: { // popx imp
      spc->x = spc_pullByte(spc);
      break;
    }
    case 0xcf: { // mul imp
      uint16_t result = spc->a * spc->y;
      spc->a = result & 0xff;
      spc->y = result >> 8;
      spc_setZN(spc, spc->y);
      break;
    }
    case 0xd0: { // bne rel
      spc_doBranch(spc, spc_readOpcode(spc), !spc->z);
      break;
    }
    case 0xd4: { // movs dpx
      spc_movs(spc, spc_adrDpx(spc));
      break;
    }
    case 0xd5: { // movs abx
      spc_movs(spc, spc_adrAbx(spc));
      break;
    }
    case 0xd6: { // movs aby
      spc_movs(spc, spc_adrAby(spc));
      break;
    }
    case 0xd7: { // movs idy
      spc_movs(spc, spc_adrIdy(spc));
      break;
    }
    case 0xd8: { // movsx dp
      spc_movsx(spc, spc_adrDp(spc));
      break;
    }
    case 0xd9: { // movsx dpy
      spc_movsx(spc, spc_adrDpy(spc));
      break;
    }
    case 0xda: { // movws dp
      uint16_t low = 0;
      uint16_t high = spc_adrDpWord(spc, &low);
      spc_read(spc, low);
      spc_write(spc, low, spc->a);
      spc_write(spc, high, spc->y);
      break;
    }
    case 0xdb: { // movsy dpx
      spc_movsy(spc, spc_adrDpx(spc));
      break;
    }
    case 0xdc: { // decy imp
      spc->y--;
      spc_setZN(spc, spc->y);
      break;
    }
    case 0xdd: { // movay imp
      spc->a = spc->y;
      spc_setZN(spc, spc->a);
      break;
    }
    case 0xde: { // cbne dpx, rel
      uint8_t val = spc_read(spc, spc_adrDpx(spc)) ^ 0xff;
      uint8_t result = spc->a + val + 1;
      spc_doBranch(spc, spc_readOpcode(spc), result != 0);
      break;
    }
    case 0xdf: { // daa imp
      if(spc->a > 0x99 || spc->c) {
        spc->a += 0x60;
        spc->c = true;
      }
      if((spc->a & 0xf) > 9 || spc->h) {
        spc->a += 6;
      }
      spc_setZN(spc, spc->a);
      break;
    }
    case 0xe0: { // clrv imp
      spc->v = false;
      spc->h = false;
      break;
    }
    case 0xe4: { // mov dp
      spc_mov(spc, spc_adrDp(spc));
      break;
    }
    case 0xe5: { // mov abs
      spc_mov(spc, spc_adrAbs(spc));
      break;
    }
    case 0xe6: { // mov ind
      spc_mov(spc, spc_adrInd(spc));
      break;
    }
    case 0xe7: { // mov idx
      spc_mov(spc, spc_adrIdx(spc));
      break;
    }
    case 0xe8: { // mov imm
      spc_mov(spc, spc_adrImm(spc));
      break;
    }
    case 0xe9: { // movx abs
      spc_movx(spc, spc_adrAbs(spc));
      break;
    }
    case 0xea: { // not1 abs.bit
      uint16_t adr = 0;
      uint8_t bit = spc_adrAbsBit(spc, &adr);
      uint8_t result = spc_read(spc, adr) ^ (1 << bit);
      spc_write(spc, adr, result);
      break;
    }
    case 0xeb: { // movy dp
      spc_movy(spc, spc_adrDp(spc));
      break;
    }
    case 0xec: { // movy abs
      spc_movy(spc, spc_adrAbs(spc));
      break;
    }
    case 0xed: { // notc imp
      spc->c = !spc->c;
      break;
    }
    case 0xee: { // popy imp
      spc->y = spc_pullByte(spc);
      break;
    }
    case 0xef: { // sleep imp
      spc->stopped = true; // no interrupts, so sleeping stops as well
      break;
    }
    case 0xf0: { // beq rel
      spc_doBranch(spc, spc_readOpcode(spc), spc->z);
      break;
    }
    case 0xf4: { // mov dpx
      spc_mov(spc, spc_adrDpx(spc));
      break;
    }
    case 0xf5: { // mov abx
      spc_mov(spc, spc_adrAbx(spc));
      break;
    }
    case 0xf6: { // mov aby
      spc_mov(spc, spc_adrAby(spc));
      break;
    }
    case 0xf7: { // mov idy
      spc_mov(spc, spc_adrIdy(spc));
      break;
    }
    case 0xf8: { // movx dp
      spc_movx(spc, spc_adrDp(spc));
      break;
    }
    case 0xf9: { // movx dpy
      spc_movx(spc, spc_adrDpy(spc));
      break;
    }
    case 0xfa: { // movm dp, dp
      uint16_t src = 0;
      uint16_t dst = spc_adrDpDp(spc, &src);
      uint8_t val = spc_read(spc, src);
      spc_write(spc, dst, val);
      break;
    }
    case 0xfb: { // movy dpx
      spc_movy(spc, spc_adrDpx(spc));
      break;
    }
    case 0xfc: { // incy imp
      spc->y++;
      spc_setZN(spc, spc->y);
      break;
    }
    case 0xfd: { // movya imp
      spc->y = spc->a;
      spc_setZN(spc, spc->y);
      break;
    }
    case 0xfe: { // dbnzy rel
      spc->y--;
      spc_doBranch(spc, spc_readOpcode(spc), spc->y != 0);
      break;
    }
    case 0xff: { // stop imp
      spc->stopped = true;
      break;
    }
  }
}
