
#ifndef TRACING_H
#define TRACING_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "snes.h"

void getProcessorStateCpu(Snes* snes, char* line);
void getProcessorStateSpc(Apu* apu, char* line);

#endif
