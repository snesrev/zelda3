#pragma once
#include "types.h"

uint8 *GetPtr(uint32 addr);

void RunEmulatedFuncSilent(uint32 pc, uint16 a, uint16 x, uint16 y, bool mf, bool xf, int b = -1, int whatflags = 0);
void RunEmulatedFunc(uint32 pc, uint16 a, uint16 x, uint16 y, bool mf, bool xf, int b = -1, int whatflags = 0);
