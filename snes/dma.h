
#ifndef DMA_H
#define DMA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct Dma Dma;

#include "snes.h"

typedef struct DmaChannel {
  uint8_t bAdr;
  uint8_t aBank;
  uint8_t indBank; // hdma
  uint8_t repCount; // hdma
  uint16_t aAdr;
  uint16_t size; // also indirect hdma adr
  uint16_t tableAdr; // hdma
  uint8_t unusedByte;
  bool dmaActive;
  bool hdmaActive;
  uint8_t mode;
  bool fixed;
  bool decrement;
  bool indirect; // hdma
  bool fromB;
  bool unusedBit;
  bool doTransfer; // hdma
  bool terminated; // hdma
  uint8_t offIndex;
} DmaChannel;

struct Dma {
  Snes* snes;
  DmaChannel channel[8];
  uint16_t hdmaTimer;
  uint32_t dmaTimer;
  bool dmaBusy;
};

Dma* dma_init(Snes* snes);
void dma_free(Dma* dma);
void dma_reset(Dma* dma);
uint8_t dma_read(Dma* dma, uint16_t adr); // 43x0-43xf
void dma_write(Dma* dma, uint16_t adr, uint8_t val); // 43x0-43xf
void dma_doDma(Dma* dma);
void dma_initHdma(Dma* dma);
void dma_doHdma(Dma* dma);
bool dma_cycle(Dma* dma);
void dma_startDma(Dma* dma, uint8_t val, bool hdma);
void dma_saveload(Dma *dma, SaveLoadFunc *func, void *ctx);

#endif
