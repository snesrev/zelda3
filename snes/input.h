
#ifndef INPUT_H
#define INPUT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct Input Input;

#include "snes.h"

struct Input {
  Snes* snes;
  uint8_t type;
  // latchline
  bool latchLine;
  // for controller
  uint16_t currentState; // actual state
  uint16_t latchedState;
};

Input* input_init(Snes* snes);
void input_free(Input* input);
void input_reset(Input* input);
void input_cycle(Input* input);
uint8_t input_read(Input* input);

#endif
