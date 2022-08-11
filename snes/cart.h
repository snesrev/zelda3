
#ifndef CART_H
#define CART_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct Cart Cart;

#include "snes.h"

struct Cart {
  Snes* snes;
  uint8_t type;

  uint8_t* rom;
  uint32_t romSize;
  uint8_t* ram;
  uint32_t ramSize;
};

// TODO: how to handle reset & load? (especially where to init ram)

Cart* cart_init(Snes* snes);
void cart_free(Cart* cart);
void cart_reset(Cart* cart); // will reset special chips etc, general reading is set up in load
void cart_load(Cart* cart, int type, uint8_t* rom, int romSize, int ramSize); // TODO: figure out how to handle (battery, cart-chips etc)
uint8_t cart_read(Cart* cart, uint8_t bank, uint16_t adr);
void cart_write(Cart* cart, uint8_t bank, uint16_t adr, uint8_t val);
void cart_saveload(Cart *cart, SaveLoadFunc *func, void *ctx);

#endif
