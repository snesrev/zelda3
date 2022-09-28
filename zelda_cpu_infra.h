#ifndef ZELDA3_ZELDA_CPU_INFRA_H_
#define ZELDA3_ZELDA_CPU_INFRA_H_
#include "types.h"

extern uint8 g_emulated_ram[0x20000];

uint8 *GetPtr(uint32 addr);

void RunEmulatedFuncSilent(uint32 pc, uint16 a, uint16 x, uint16 y, bool mf, bool xf, int b, int whatflags);
void RunEmulatedFunc(uint32 pc, uint16 a, uint16 x, uint16 y, bool mf, bool xf, int b, int whatflags);

bool EmuInitialize(uint8 *data, size_t size);

#endif  // ZELDA3_ZELDA_CPU_INFRA_H_
