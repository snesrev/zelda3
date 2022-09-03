
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>
#include "ppu.h"
#include "snes.h"
#include "../types.h"

static const int spriteSizes[8][2] = {
  {8, 16}, {8, 32}, {8, 64}, {16, 32},
  {16, 64}, {32, 64}, {16, 32}, {16, 32}
};

static void ppu_handlePixel(Ppu* ppu, int x, int y);
static int ppu_getPixel(Ppu* ppu, int x, int y, bool sub, int* r, int* g, int* b);
static void ppu_calculateMode7Starts(Ppu* ppu, int y);
static int ppu_getPixelForMode7(Ppu* ppu, int x, int layer, bool priority);
static inline bool ppu_getWindowState(Ppu* ppu, int layer, int x);
static void ppu_evaluateSprites(Ppu* ppu, int line);
static uint16_t ppu_getVramRemap(Ppu* ppu);

Ppu* ppu_init(Snes* snes) {
  Ppu* ppu = (Ppu * )malloc(sizeof(Ppu));
  ppu->snes = snes;
  return ppu;
}

void ppu_free(Ppu* ppu) {
  free(ppu);
}

void ppu_reset(Ppu* ppu) {
  memset(ppu->vram, 0, sizeof(ppu->vram));
  ppu->vramPointer = 0;
  ppu->vramIncrementOnHigh = false;
  ppu->vramIncrement = 1;
  ppu->vramRemapMode = 0;
  ppu->vramReadBuffer = 0;
  memset(ppu->cgram, 0, sizeof(ppu->cgram));
  ppu->cgramPointer = 0;
  ppu->cgramSecondWrite = false;
  ppu->cgramBuffer = 0;
  memset(ppu->oam, 0, sizeof(ppu->oam));
  memset(ppu->highOam, 0, sizeof(ppu->highOam));
  ppu->oamAdr = 0;
  ppu->oamAdrWritten = 0;
  ppu->oamInHigh = false;
  ppu->oamInHighWritten = false;
  ppu->oamSecondWrite = false;
  ppu->oamBuffer = 0;
  ppu->objPriority = false;
  ppu->objTileAdr1 = 0;
  ppu->objTileAdr2 = 0;
  ppu->objSize = 0;
  memset(ppu->objPixelBuffer, 0, sizeof(ppu->objPixelBuffer));
  memset(ppu->objPriorityBuffer, 0, sizeof(ppu->objPriorityBuffer));
  ppu->timeOver = false;
  ppu->rangeOver = false;
  ppu->objInterlace_always_zero = false;
  for(int i = 0; i < 4; i++) {
    ppu->bgLayer[i].hScroll = 0;
    ppu->bgLayer[i].vScroll = 0;
    ppu->bgLayer[i].tilemapWider = false;
    ppu->bgLayer[i].tilemapHigher = false;
    ppu->bgLayer[i].tilemapAdr = 0;
    ppu->bgLayer[i].tileAdr = 0;
    ppu->bgLayer[i].bigTiles_always_zero = false;
    ppu->bgLayer[i].mosaicEnabled = false;
  }
  ppu->scrollPrev = 0;
  ppu->scrollPrev2 = 0;
  ppu->mosaicSize = 1;
  ppu->mosaicStartLine = 1;
  for(int i = 0; i < 5; i++) {
    ppu->layer[i].screenEnabled[0] = false;
    ppu->layer[i].screenEnabled[1] = false;
    ppu->layer[i].screenWindowed[0] = false;
    ppu->layer[i].screenWindowed[1] = false;
  }
  memset(ppu->m7matrix, 0, sizeof(ppu->m7matrix));
  ppu->m7prev = 0;
  ppu->m7largeField = false;
  ppu->m7charFill = false;
  ppu->m7xFlip = false;
  ppu->m7yFlip = false;
  ppu->m7extBg_always_zero = false;
  ppu->m7startX = 0;
  ppu->m7startY = 0;
  for(int i = 0; i < 6; i++) {
    ppu->windowLayer[i].window1enabled = false;
    ppu->windowLayer[i].window2enabled = false;
    ppu->windowLayer[i].window1inversed = false;
    ppu->windowLayer[i].window2inversed = false;
    ppu->windowLayer[i].maskLogic_always_zero = 0;
  }
  ppu->window1left = 0;
  ppu->window1right = 0;
  ppu->window2left = 0;
  ppu->window2right = 0;
  ppu->clipMode = 0;
  ppu->preventMathMode = 0;
  ppu->addSubscreen = false;
  ppu->subtractColor = false;
  ppu->halfColor = false;
  memset(ppu->mathEnabled, 0, sizeof(ppu->mathEnabled));
  ppu->fixedColorR = 0;
  ppu->fixedColorG = 0;
  ppu->fixedColorB = 0;
  ppu->forcedBlank = true;
  ppu->brightness = 0;
  ppu->mode = 0;
  ppu->bg3priority = false;
  ppu->evenFrame = false;
  ppu->pseudoHires_always_zero = false;
  ppu->overscan_always_zero = false;
  ppu->frameOverscan_always_zero = false;
  ppu->interlace_always_zero = false;
  ppu->frameInterlace_always_zero = false;
  ppu->directColor_always_zero = false;
  ppu->hCount = 0;
  ppu->vCount = 0;
  ppu->hCountSecond = false;
  ppu->vCountSecond = false;
  ppu->countersLatched = false;
  ppu->ppu1openBus = 0;
  ppu->ppu2openBus = 0;
  memset(ppu->pixelBuffer, 0, sizeof(ppu->pixelBuffer));
}

void ppu_saveload(Ppu *ppu, SaveLoadFunc *func, void *ctx) {
  func(ctx, &ppu->vram, offsetof(Ppu, pixelBuffer) - offsetof(Ppu, vram));
}

void ppu_handleVblank(Ppu* ppu) {
  // called either right after ppu_checkOverscan at (0,225), or at (0,240)
  if(!ppu->forcedBlank) {
    ppu->oamAdr = ppu->oamAdrWritten;
    ppu->oamInHigh = ppu->oamInHighWritten;
    ppu->oamSecondWrite = false;
  }
  ppu->frameInterlace_always_zero = ppu->interlace_always_zero; // set if we have a interlaced frame
}

void ppu_runLine(Ppu* ppu, int line) {
  if(line == 0) {

    // Ensure all window layer fields are just 0 or 1
    for (int i = 0; i < 6; i++) {
      WindowLayer *wl = &ppu->windowLayer[i];
      wl->window1enabled = (wl->window1enabled != 0);
      wl->window2enabled = (wl->window2enabled != 0);
      wl->window1inversed = (wl->window1inversed != 0);
      wl->window2inversed = (wl->window2inversed != 0);
    }

    ppu->mosaicStartLine = 1;
    ppu->rangeOver = false;
    ppu->timeOver = false;
    ppu->evenFrame = !ppu->evenFrame;
  } else {
    // evaluate sprites
    memset(ppu->objPixelBuffer, 0, sizeof(ppu->objPixelBuffer));
    memset(ppu->objPriorityBuffer, 0xff, sizeof(ppu->objPriorityBuffer));

    if(!ppu->forcedBlank) ppu_evaluateSprites(ppu, line - 1);
    // actual line
    if(ppu->mode == 7) ppu_calculateMode7Starts(ppu, line);
    for(int x = 0; x < 256; x++) {
      ppu_handlePixel(ppu, x, line);
    }
  }
}

static FORCEINLINE bool ppu_checkmodecondition(uint8_t mode, bool state) {
  return mode & (state + 1);
  return mode == 3 || mode == 2 && state || mode == 1 && !state;
  return (mode >> 1) & state | mode & (state ^ 1);
}

static FORCEINLINE int ppu_clamp0_31(int v) {
  if (v < 0) v = 0;
  if (v > 31) v = 31;
  return v;
}

static void ppu_handlePixel(Ppu* ppu, int x, int y) {
  int r = 0, g = 0, b = 0;
  
  if(!ppu->forcedBlank) {
    int mainLayer = ppu_getPixel(ppu, x, y, false, &r, &g, &b);

    bool colorWindowState = ppu_getWindowState(ppu, 5, x);
    if(ppu_checkmodecondition(ppu->clipMode, colorWindowState)) {
      r = g = b = 0;
    }
    int secondLayer = 5; // backdrop
    bool mathEnabled = mainLayer < 6 && ppu->mathEnabled[mainLayer] && !ppu_checkmodecondition(ppu->preventMathMode, colorWindowState);
    // TODO: subscreen pixels can be clipped to black as well
    // TODO: math for subscreen pixels (add/sub sub to main)
    if(mathEnabled) {
      int r2 = 0, g2 = 0, b2 = 0;
      if (ppu->addSubscreen && (secondLayer = ppu_getPixel(ppu, x, y, true, &r2, &g2, &b2)) != 5) {
        if (ppu->subtractColor) {
          r -= r2;
          g -= g2;
          b -= b2;
        } else {
          r += r2;
          g += g2;
          b += b2;
        }
      } else {
        if (ppu->subtractColor) {
          r -= ppu->fixedColorR;
          g -= ppu->fixedColorG;
          b -= ppu->fixedColorB;
        } else {
          r += ppu->fixedColorR;
          g += ppu->fixedColorG;
          b += ppu->fixedColorB;
        }
      }
      if(ppu->halfColor && (secondLayer != 5 || !ppu->addSubscreen)) {
        r >>= 1;
        g >>= 1;
        b >>= 1;
      }
      r = ppu_clamp0_31(r);
      g = ppu_clamp0_31(g);
      b = ppu_clamp0_31(b);
    }
  }
  int row = (y - 1) + (ppu->evenFrame ? 0 : 239);
  uint8_t *dst = &ppu->pixelBuffer[row * 2048 + x * 8];
  uint8_t ppu_brightness = ppu->brightness;
  dst[1] = dst[5] = ((b << 3) | (b >> 2)) * ppu_brightness / 15;
  dst[2] = dst[6] = ((g << 3) | (g >> 2)) * ppu_brightness / 15;
  dst[3] = dst[7] = ((r << 3) | (r >> 2)) * ppu_brightness / 15;
}


static bool FORCEINLINE ppu_islayeractive(Ppu *ppu, int curLayer, bool sub, int x) {
  Layer *layerp = &ppu->layer[curLayer];
  return layerp->screenEnabled[sub] && (!layerp->screenWindowed[sub] || !ppu_getWindowState(ppu, curLayer, x));
}

typedef struct GetPixelRV {
  uint8_t pixel;
  uint8_t layer;
} GetPixelRV;

typedef struct TileAndXY {
  uint16_t tile;
  uint8_t x, y;
} TileAndXY;

static GetPixelRV ppu_getPixel_Mode7(Ppu *ppu, int x, int y, bool sub);
static GetPixelRV ppu_getPixel_Mode1(Ppu *ppu, int x, int y, bool sub);

static int FORCEINLINE ppu_getPixel(Ppu *ppu, int x, int y, bool sub, int *r, int *g, int *b) {
  GetPixelRV rv;
  if (ppu->mode == 1) {
    rv = ppu_getPixel_Mode1(ppu, x, y, sub);
  } else {
    rv = ppu_getPixel_Mode7(ppu, x, y, sub);
  }
  uint16_t color = ppu->cgram[rv.pixel];
  *r = color & 0x1f;
  *g = (color >> 5) & 0x1f;
  *b = (color >> 10) & 0x1f;
  return rv.layer;
}

static GetPixelRV ppu_getPixel_Mode7(Ppu *ppu, int x, int y, bool sub) {
  // figure out which color is on this location on main- or subscreen, sets it in r, g, b
  // returns which layer it is: 0-3 for bg layer, 4 or 6 for sprites (depending on palette), 5 for backdrop
  int actMode = ppu->mode == 1 ? 8 : ppu->mode;
  static const uint8_t kLayersMode7[] = { 4, 4, 4, 0, 4 };
  static const uint8_t kPrioritysMode7[] = { 3, 2, 1, 0, 0, };
  for (int i = 0; i < 5; i++) {
    int curLayer = kLayersMode7[i];
    int curPriority = kPrioritysMode7[i];
    int pixel;
    if (ppu_islayeractive(ppu, curLayer, sub, x)) {
      if (curLayer < 4) {
        pixel = ppu_getPixelForMode7(ppu, x, curLayer, curPriority);
      } else {
        pixel = (ppu->objPriorityBuffer[x] == curPriority) ? ppu->objPixelBuffer[x] : 0;
        if (pixel < 0xc0)
          curLayer = 6;
      }
      if (pixel != 0)
        return (GetPixelRV) { pixel, curLayer };
    }
  }
  return (GetPixelRV) { 0, 5 };
}

static FORCEINLINE TileAndXY ppu_GetBgTileAndXy(Ppu *ppu, int x, int y, int layer);
static FORCEINLINE int ppu_GetPixelFromTileAndXY_2bpp(Ppu *ppu, TileAndXY txy, int tileAdr);
static FORCEINLINE int ppu_GetPixelFromTileAndXY_4bpp(Ppu *ppu, TileAndXY txy, int tileAdr);

static FORCEINLINE GetPixelRV ppu_getSpritePixelRV(uint8_t pixel) {
  // sprites with palette color < 0xc0 are layer 6 instead of 4.
  return (GetPixelRV) { pixel, pixel < 0xc0 ? 6 : 4 };
}

static GetPixelRV ppu_getPixel_Mode1(Ppu *ppu, int x, int y, bool sub) {
  // figure out which color is on this location on main- or subscreen
  // returns which layer it is: 0-3 for bg layer, 4 or 6 for sprites (depending on palette), 5 for backdrop
  TileAndXY BG1, BG2, BG3;
  uint8_t pixel;

  // BG3 tiles with priority 1, 2bpp
  bool layer3active = ppu_islayeractive(ppu, 2, sub, x);
  if (layer3active) {
    BG3 = ppu_GetBgTileAndXy(ppu, x, y, 2);
    if (BG3.tile & 0x2000) {
      if ((pixel = ppu_GetPixelFromTileAndXY_2bpp(ppu, BG3, ppu->bgLayer[2].tileAdr)) != 0)
        return (GetPixelRV) { pixel, 2 };
      layer3active = false;
    }
  }
  // Sprites with priority 3
  uint8_t obj_prio = ppu->objPriorityBuffer[x];
  if (obj_prio == 3 && ppu_islayeractive(ppu, 4, sub, x))
    return ppu_getSpritePixelRV(ppu->objPixelBuffer[x]);
  
  // BG1 tiles with priority 1
  bool layer1active = ppu_islayeractive(ppu, 0, sub, x);
  if (layer1active) {
    BG1 = ppu_GetBgTileAndXy(ppu, x, y, 0);
    if (BG1.tile & 0x2000) {
      if ((pixel = ppu_GetPixelFromTileAndXY_4bpp(ppu, BG1, ppu->bgLayer[0].tileAdr)) != 0)
        return (GetPixelRV) { pixel, 0 };
      layer1active = false;
    }
  }
  // BG2 tiles with priority 1
  bool layer2active = ppu_islayeractive(ppu, 1, sub, x);
  if (layer2active) {
    BG2 = ppu_GetBgTileAndXy(ppu, x, y, 1);
    if (BG2.tile & 0x2000) {
      if ((pixel = ppu_GetPixelFromTileAndXY_4bpp(ppu, BG2, ppu->bgLayer[1].tileAdr)) != 0)
        return (GetPixelRV) { pixel, 1 };
      layer2active = false;
    }
  }
  // Sprites with priority 2
  if (obj_prio == 2 && ppu_islayeractive(ppu, 4, sub, x))
    return ppu_getSpritePixelRV(ppu->objPixelBuffer[x]);
  // BG1 tiles with priority 0
  if (layer1active && (pixel = ppu_GetPixelFromTileAndXY_4bpp(ppu, BG1, ppu->bgLayer[0].tileAdr)))
    return (GetPixelRV) { pixel, 0 };
  // BG2 tiles with priority 0
  if (layer2active && (pixel = ppu_GetPixelFromTileAndXY_4bpp(ppu, BG2, ppu->bgLayer[1].tileAdr)))
    return (GetPixelRV) { pixel, 1 };
  // Sprites with priority 1
  // Sprites with priority 0
  if (obj_prio <= 1 && ppu_islayeractive(ppu, 4, sub, x))
    return ppu_getSpritePixelRV(ppu->objPixelBuffer[x]);
  // BG3 tiles with priority 0
  if (layer3active && (pixel = ppu_GetPixelFromTileAndXY_2bpp(ppu, BG3, ppu->bgLayer[2].tileAdr)))
    return (GetPixelRV) { pixel, 2 };
  // backdrop
  return (GetPixelRV) { 0, 5 };
}

static FORCEINLINE TileAndXY ppu_GetBgTileAndXy(Ppu *ppu, int x, int y, int layer) {
  BgLayer *layerp = &ppu->bgLayer[layer];
  if (layerp->mosaicEnabled && ppu->mosaicSize > 1) {
    x -= x % ppu->mosaicSize;
    y -= (y - ppu->mosaicStartLine) % ppu->mosaicSize;
  }
  x += layerp->hScroll;
  y += layerp->vScroll;
  // figure out address of tilemap word and read it
  int tilemapAdr = layerp->tilemapAdr + (((y >> 3) & 0x1f) << 5 | ((x >> 3) & 0x1f));
  if (layerp->tilemapWider)
    tilemapAdr += (x & 0x100) << 2;
  if ((y & 0x100) && layerp->tilemapHigher) tilemapAdr += layerp->tilemapWider ? 0x800 : 0x400;
  return (TileAndXY) { ppu->vram[tilemapAdr & 0x7fff], x, y };
}

static FORCEINLINE int ppu_GetPixelFromTileAndXY_2bpp(Ppu *ppu, TileAndXY txy, int tileAdr) {
  int paletteNum = (txy.tile & 0x1c00) >> 10;
  int row = (txy.tile & 0x8000) ? 7 - (txy.y & 0x7) : (txy.y & 0x7);
  int col = (txy.tile & 0x4000) ? (txy.x & 0x7) : 7 - (txy.x & 0x7);
  int plane1 = ppu->vram[(tileAdr + ((txy.tile & 0x3ff) * 4 * 2) + row) & 0x7fff] >> col;
  int pixel = plane1 & 1 | (plane1 >> 7) & 2;
  return pixel == 0 ? 0 : 4 * paletteNum + pixel;
}

static FORCEINLINE int ppu_GetPixelFromTileAndXY_4bpp(Ppu *ppu, TileAndXY txy, int tileAdr) {
  int paletteNum = (txy.tile & 0x1c00) >> 10;
  int row = (txy.tile & 0x8000) ? 7 - (txy.y & 0x7) : (txy.y & 0x7);
  int col = (txy.tile & 0x4000) ? (txy.x & 0x7) : 7 - (txy.x & 0x7);
  uint16_t *addr = &ppu->vram[(tileAdr + ((txy.tile & 0x3ff) * 4 * 4) + row) & 0x7fff];
  uint16_t plane1 = addr[0] >> col, plane2 = addr[8] >> col;
  int pixel = plane1 & 1 | (plane1 >> 7) & 2 | (plane2 << 2) & 4 | (plane2 >> 5) & 8;
  return pixel == 0 ? 0 : 16 * paletteNum + pixel;
}

static void ppu_calculateMode7Starts(Ppu* ppu, int y) {
  // expand 13-bit values to signed values
  int hScroll = ((int16_t) (ppu->m7matrix[6] << 3)) >> 3;
  int vScroll = ((int16_t) (ppu->m7matrix[7] << 3)) >> 3;
  int xCenter = ((int16_t) (ppu->m7matrix[4] << 3)) >> 3;
  int yCenter = ((int16_t) (ppu->m7matrix[5] << 3)) >> 3;
  // do calculation
  int clippedH = hScroll - xCenter;
  int clippedV = vScroll - yCenter;
  clippedH = (clippedH & 0x2000) ? (clippedH | ~1023) : (clippedH & 1023);
  clippedV = (clippedV & 0x2000) ? (clippedV | ~1023) : (clippedV & 1023);
  if(ppu->bgLayer[0].mosaicEnabled && ppu->mosaicSize > 1) {
    y -= (y - ppu->mosaicStartLine) % ppu->mosaicSize;
  }
  uint8_t ry = ppu->m7yFlip ? 255 - y : y;
  ppu->m7startX = (
    ((ppu->m7matrix[0] * clippedH) & ~63) +
    ((ppu->m7matrix[1] * ry) & ~63) +
    ((ppu->m7matrix[1] * clippedV) & ~63) +
    (xCenter << 8)
  );
  ppu->m7startY = (
    ((ppu->m7matrix[2] * clippedH) & ~63) +
    ((ppu->m7matrix[3] * ry) & ~63) +
    ((ppu->m7matrix[3] * clippedV) & ~63) +
    (yCenter << 8)
  );
}

static int ppu_getPixelForMode7(Ppu* ppu, int x, int layer, bool priority) {
  if (ppu->bgLayer[layer].mosaicEnabled && ppu->mosaicSize > 1)
    x -= x % ppu->mosaicSize;
  uint8_t rx = ppu->m7xFlip ? 255 - x : x;
  int xPos = (ppu->m7startX + ppu->m7matrix[0] * rx) >> 8;
  int yPos = (ppu->m7startY + ppu->m7matrix[2] * rx) >> 8;
  bool outsideMap = xPos < 0 || xPos >= 1024 || yPos < 0 || yPos >= 1024;
  xPos &= 0x3ff;
  yPos &= 0x3ff;
  if(!ppu->m7largeField) outsideMap = false;
  uint8_t tile = outsideMap ? 0 : ppu->vram[(yPos >> 3) * 128 + (xPos >> 3)] & 0xff;
  uint8_t pixel = outsideMap && !ppu->m7charFill ? 0 : ppu->vram[tile * 64 + (yPos & 7) * 8 + (xPos & 7)] >> 8;
  if(layer == 1) {
    if(((bool) (pixel & 0x80)) != priority) return 0;
    return pixel & 0x7f;
  }
  return pixel;
}

static inline bool ppu_getWindowState(Ppu* ppu, int layer, int x) {
  WindowLayer *wl = &ppu->windowLayer[layer];
  if(!(wl->window1enabled | wl->window2enabled))
    return false;
  bool test1 = x >= ppu->window1left && x <= ppu->window1right;
  bool test2 = x >= ppu->window2left && x <= ppu->window2right;
  return ((test1 ^ wl->window1inversed) & wl->window1enabled) | ((test2 ^ wl->window2inversed) & wl->window2enabled);
}

static void ppu_evaluateSprites(Ppu* ppu, int line) {
  // TODO: iterate over oam normally to determine in-range sprites,
  //   then iterate those in-range sprites in reverse for tile-fetching
  // TODO: rectangular sprites, wierdness with sprites at -256
  uint8_t index = ppu->objPriority ? (ppu->oamAdr & 0xfe) : 0;
  int spritesFound = 0;
  int tilesFound = 0;
  for(int i = 0; i < 128; i++) {
    uint8_t y = ppu->oam[index] >> 8;
    // check if the sprite is on this line and get the sprite size
    uint8_t row = line - y;
    int spriteSize = spriteSizes[ppu->objSize][(ppu->highOam[index >> 3] >> ((index & 7) + 1)) & 1];
    int spriteHeight = spriteSize;
    if(row < spriteHeight) {
      // in y-range, get the x location, using the high bit as well
      int x = ppu->oam[index] & 0xff;
      x |= ((ppu->highOam[index >> 3] >> (index & 7)) & 1) << 8;
      if(x > 255) x -= 512;
      // if in x-range
      if(x > -spriteSize) {
        // break if we found 32 sprites already
        spritesFound++;
        if(spritesFound > 32) {
          ppu->rangeOver = true;
          break;
        }
        // get some data for the sprite and y-flip row if needed
        int tile = ppu->oam[index + 1] & 0xff;
        int palette = (ppu->oam[index + 1] & 0xe00) >> 9;
        bool hFlipped = ppu->oam[index + 1] & 0x4000;
        if(ppu->oam[index + 1] & 0x8000) row = spriteSize - 1 - row;
        // fetch all tiles in x-range
        for(int col = 0; col < spriteSize; col += 8) {
          if(col + x > -8 && col + x < 256) {
            // break if we found 34 8*1 slivers already
            tilesFound++;
            if(tilesFound > 34) {
              ppu->timeOver = true;
              break;
            }
            // figure out which tile this uses, looping within 16x16 pages, and get it's data
            int usedCol = hFlipped ? spriteSize - 1 - col : col;
            uint8_t usedTile = (((tile >> 4) + (row / 8)) << 4) | (((tile & 0xf) + (usedCol / 8)) & 0xf);
            uint16_t objAdr = (ppu->oam[index + 1] & 0x100) ? ppu->objTileAdr2 : ppu->objTileAdr1;
            uint16_t plane1 = ppu->vram[(objAdr + usedTile * 16 + (row & 0x7)) & 0x7fff];
            uint16_t plane2 = ppu->vram[(objAdr + usedTile * 16 + 8 + (row & 0x7)) & 0x7fff];
            // go over each pixel
            for(int px = 0; px < 8; px++) {
              int shift = hFlipped ? px : 7 - px;
              int pixel = (plane1 >> shift) & 1;
              pixel |= ((plane1 >> (8 + shift)) & 1) << 1;
              pixel |= ((plane2 >> shift) & 1) << 2;
              pixel |= ((plane2 >> (8 + shift)) & 1) << 3;
              // draw it in the buffer if there is a pixel here, and the buffer there is still empty
              int screenCol = col + x + px;
              if(pixel != 0 && screenCol >= 0 && screenCol < 256 && ppu->objPixelBuffer[screenCol] == 0) {
                ppu->objPixelBuffer[screenCol] = 0x80 + 16 * palette + pixel;
                ppu->objPriorityBuffer[screenCol] = (ppu->oam[index + 1] & 0x3000) >> 12;
              }
            }
          }
        }
        if(tilesFound > 34)
          break; // break out of sprite-loop if max tiles found
      }
    }
    index += 2;
  }
}

static uint16_t ppu_getVramRemap(Ppu* ppu) {
  uint16_t adr = ppu->vramPointer;
  switch(ppu->vramRemapMode) {
    case 0: return adr;
    case 1: return (adr & 0xff00) | ((adr & 0xe0) >> 5) | ((adr & 0x1f) << 3);
    case 2: return (adr & 0xfe00) | ((adr & 0x1c0) >> 6) | ((adr & 0x3f) << 3);
    case 3: return (adr & 0xfc00) | ((adr & 0x380) >> 7) | ((adr & 0x7f) << 3);
  }
  return adr;
}

uint8_t ppu_read(Ppu* ppu, uint8_t adr) {
  switch(adr) {
    case 0x04: case 0x14: case 0x24:
    case 0x05: case 0x15: case 0x25:
    case 0x06: case 0x16: case 0x26:
    case 0x08: case 0x18: case 0x28:
    case 0x09: case 0x19: case 0x29:
    case 0x0a: case 0x1a: case 0x2a: {
      return ppu->ppu1openBus;
    }
    case 0x34:
    case 0x35:
    case 0x36: {
      int result = ppu->m7matrix[0] * (ppu->m7matrix[1] >> 8);
      ppu->ppu1openBus = (result >> (8 * (adr - 0x34))) & 0xff;
      return ppu->ppu1openBus;
    }
    case 0x37: {
      // TODO: only when ppulatch is set
      ppu->hCount = ppu->snes->hPos / 4;
      ppu->vCount = ppu->snes->vPos;
      ppu->countersLatched = true;
      if (ppu->snes->disableHpos)
        ppu->vCount = 192;

      return ppu->snes->openBus;
    }
    case 0x38: {
      uint8_t ret = 0;
      if(ppu->oamInHigh) {
        ret = ppu->highOam[((ppu->oamAdr & 0xf) << 1) | (uint8_t)ppu->oamSecondWrite];
        if(ppu->oamSecondWrite) {
          ppu->oamAdr++;
          if(ppu->oamAdr == 0) ppu->oamInHigh = false;
        }
      } else {
        if(!ppu->oamSecondWrite) {
          ret = ppu->oam[ppu->oamAdr] & 0xff;
        } else {
          ret = ppu->oam[ppu->oamAdr++] >> 8;
          if(ppu->oamAdr == 0) ppu->oamInHigh = true;
        }
      }
      ppu->oamSecondWrite = !ppu->oamSecondWrite;
      ppu->ppu1openBus = ret;
      return ret;
    }
    case 0x39: {
      uint16_t val = ppu->vramReadBuffer;
      if(!ppu->vramIncrementOnHigh) {
        ppu->vramReadBuffer = ppu->vram[ppu_getVramRemap(ppu) & 0x7fff];
        ppu->vramPointer += ppu->vramIncrement;
      }
      ppu->ppu1openBus = val & 0xff;
      return val & 0xff;
    }
    case 0x3a: {
      uint16_t val = ppu->vramReadBuffer;
      if(ppu->vramIncrementOnHigh) {
        ppu->vramReadBuffer = ppu->vram[ppu_getVramRemap(ppu) & 0x7fff];
        ppu->vramPointer += ppu->vramIncrement;
      }
      ppu->ppu1openBus = val >> 8;
      return val >> 8;
    }
    case 0x3b: {
      uint8_t ret = 0;
      if(!ppu->cgramSecondWrite) {
        ret = ppu->cgram[ppu->cgramPointer] & 0xff;
      } else {
        ret = ((ppu->cgram[ppu->cgramPointer++] >> 8) & 0x7f) | (ppu->ppu2openBus & 0x80);
      }
      ppu->cgramSecondWrite = !ppu->cgramSecondWrite;
      ppu->ppu2openBus = ret;
      return ret;
    }
    case 0x3c: {
      uint8_t val = 0x17;// (ppu->ppu2openBus + ppu->cgramPointer * 7) * 0x31337 >> 8;
      ppu->hCountSecond = !ppu->hCountSecond;
      ppu->ppu2openBus = val;
      return val;
    }
    case 0x3d: {
      uint8_t val = 0;
      uint16_t vCount = 192;// ppu->vCount
      if(ppu->vCountSecond) {
        val = ((vCount >> 8) & 1) | (ppu->ppu2openBus & 0xfe);
      } else {
        val = vCount & 0xff;
      }
      ppu->vCountSecond = !ppu->vCountSecond;
      ppu->ppu2openBus = val;
      return val;
    }
    case 0x3e: {
      uint8_t val = 0x1; // ppu1 version (4 bit)
      val |= ppu->ppu1openBus & 0x10;
      val |= ppu->rangeOver << 6;
      val |= ppu->timeOver << 7;
      ppu->ppu1openBus = val;
      return val;
    }
    case 0x3f: {
      uint8_t val = 0x3; // ppu2 version (4 bit), bit 4: ntsc/pal
      val |= ppu->ppu2openBus & 0x20;
      val |= ppu->countersLatched << 6;
      val |= ppu->evenFrame << 7;
      ppu->countersLatched = false; // TODO: only when ppulatch is set
      ppu->hCountSecond = false;
      ppu->vCountSecond = false;
      ppu->ppu2openBus = val;
      return val;
    }
    default: {
      return ppu->snes->openBus;
    }
  }
}

void ppu_write(Ppu* ppu, uint8_t adr, uint8_t val) {
  switch(adr) {
    case 0x00: {
      // TODO: oam address reset when written on first line of vblank, (and when forced blank is disabled?)
      ppu->brightness = val & 0xf;
      ppu->forcedBlank = val & 0x80;
      break;
    }
    case 0x01: {
      ppu->objSize = val >> 5;
      ppu->objTileAdr1 = (val & 7) << 13;
      ppu->objTileAdr2 = ppu->objTileAdr1 + (((val & 0x18) + 8) << 9);
      break;
    }
    case 0x02: {
      ppu->oamAdr = val;
      ppu->oamAdrWritten = ppu->oamAdr;
      ppu->oamInHigh = ppu->oamInHighWritten;
      ppu->oamSecondWrite = false;
      break;
    }
    case 0x03: {
      ppu->objPriority = val & 0x80;
      ppu->oamInHigh = val & 1;
      ppu->oamInHighWritten = ppu->oamInHigh;
      ppu->oamAdr = ppu->oamAdrWritten;
      ppu->oamSecondWrite = false;
      break;
    }
    case 0x04: {
      if(ppu->oamInHigh) {
        ppu->highOam[((ppu->oamAdr & 0xf) << 1) | (uint8_t)ppu->oamSecondWrite] = val;
        if(ppu->oamSecondWrite) {
          ppu->oamAdr++;
          if(ppu->oamAdr == 0) ppu->oamInHigh = false;
        }
      } else {
        if(!ppu->oamSecondWrite) {
          ppu->oamBuffer = val;
        } else {
          ppu->oam[ppu->oamAdr++] = (val << 8) | ppu->oamBuffer;
          if(ppu->oamAdr == 0) ppu->oamInHigh = true;
        }
      }
      ppu->oamSecondWrite = !ppu->oamSecondWrite;
      break;
    }
    case 0x05: {
      ppu->mode = val & 0x7;
      ppu->bg3priority = val & 0x8;
      assert(val == 7 || val == 9);
      assert(ppu->mode == 1 || ppu->mode == 7);
      // bigTiles are never used
      assert((val & 0xf0) == 0);
      break;
    }
    case 0x06: {
      // TODO: mosaic line reset specifics
      ppu->bgLayer[0].mosaicEnabled = val & 0x1;
      ppu->bgLayer[1].mosaicEnabled = val & 0x2;
      ppu->bgLayer[2].mosaicEnabled = val & 0x4;
      ppu->bgLayer[3].mosaicEnabled = val & 0x8;
      ppu->mosaicSize = (val >> 4) + 1;
      ppu->mosaicStartLine = 0;
      break;
    }
    case 0x07:
    case 0x08:
    case 0x09:
    case 0x0a: {
      // small tilemaps are used in attract intro
      ppu->bgLayer[adr - 7].tilemapWider = val & 0x1;
      ppu->bgLayer[adr - 7].tilemapHigher = val & 0x2;
      ppu->bgLayer[adr - 7].tilemapAdr = (val & 0xfc) << 8;
      break;
    }
    case 0x0b: {
      ppu->bgLayer[0].tileAdr = (val & 0xf) << 12;
      ppu->bgLayer[1].tileAdr = (val & 0xf0) << 8;
      break;
    }
    case 0x0c: {
      ppu->bgLayer[2].tileAdr = (val & 0xf) << 12;
      ppu->bgLayer[3].tileAdr = (val & 0xf0) << 8;
      break;
    }
    case 0x0d: {
      ppu->m7matrix[6] = ((val << 8) | ppu->m7prev) & 0x1fff;
      ppu->m7prev = val;
      // fallthrough to normal layer BG-HOFS
    }
    case 0x0f:
    case 0x11:
    case 0x13: {
      ppu->bgLayer[(adr - 0xd) / 2].hScroll = ((val << 8) | (ppu->scrollPrev & 0xf8) | (ppu->scrollPrev2 & 0x7)) & 0x3ff;
      ppu->scrollPrev = val;
      ppu->scrollPrev2 = val;
      break;
    }
    case 0x0e: {
      ppu->m7matrix[7] = ((val << 8) | ppu->m7prev) & 0x1fff;
      ppu->m7prev = val;
      // fallthrough to normal layer BG-VOFS
    }
    case 0x10:
    case 0x12:
    case 0x14: {
      ppu->bgLayer[(adr - 0xe) / 2].vScroll = ((val << 8) | ppu->scrollPrev) & 0x3ff;
      ppu->scrollPrev = val;
      break;
    }
    case 0x15: {
      if((val & 3) == 0) {
        ppu->vramIncrement = 1;
      } else if((val & 3) == 1) {
        ppu->vramIncrement = 32;
      } else {
        ppu->vramIncrement = 128;
      }
      ppu->vramRemapMode = (val & 0xc) >> 2;
      ppu->vramIncrementOnHigh = val & 0x80;
      break;
    }
    case 0x16: {
      ppu->vramPointer = (ppu->vramPointer & 0xff00) | val;
      ppu->vramReadBuffer = ppu->vram[ppu_getVramRemap(ppu) & 0x7fff];
      break;
    }
    case 0x17: {
      ppu->vramPointer = (ppu->vramPointer & 0x00ff) | (val << 8);
      ppu->vramReadBuffer = ppu->vram[ppu_getVramRemap(ppu) & 0x7fff];
      break;
    }
    case 0x18: {
      // TODO: vram access during rendering (also cgram and oam)
      uint16_t vramAdr = ppu_getVramRemap(ppu);
      if (val != 0xef) {
        val += 0;
      }
      ppu->vram[vramAdr & 0x7fff] = (ppu->vram[vramAdr & 0x7fff] & 0xff00) | val;
      if(!ppu->vramIncrementOnHigh) ppu->vramPointer += ppu->vramIncrement;
      break;
    }
    case 0x19: {
      uint16_t vramAdr = ppu_getVramRemap(ppu);
      ppu->vram[vramAdr & 0x7fff] = (ppu->vram[vramAdr & 0x7fff] & 0x00ff) | (val << 8);
      if(ppu->vramIncrementOnHigh) ppu->vramPointer += ppu->vramIncrement;
      break;
    }
    case 0x1a: {  
      ppu->m7largeField = val & 0x80;
      ppu->m7charFill = val & 0x40;
      ppu->m7yFlip = val & 0x2;
      ppu->m7xFlip = val & 0x1;
      break;
    }
    case 0x1b:
    case 0x1c:
    case 0x1d:
    case 0x1e: {
      ppu->m7matrix[adr - 0x1b] = (val << 8) | ppu->m7prev;
      ppu->m7prev = val;
      break;
    }
    case 0x1f:
    case 0x20: {
      ppu->m7matrix[adr - 0x1b] = ((val << 8) | ppu->m7prev) & 0x1fff;
      ppu->m7prev = val;
      break;
    }
    case 0x21: {
      ppu->cgramPointer = val;
      ppu->cgramSecondWrite = false;
      break;
    }
    case 0x22: {
      if(!ppu->cgramSecondWrite) {
        ppu->cgramBuffer = val;
      } else {
        ppu->cgram[ppu->cgramPointer++] = (val << 8) | ppu->cgramBuffer;
      }
      ppu->cgramSecondWrite = !ppu->cgramSecondWrite;
      break;
    }
    case 0x23:
    case 0x24:
    case 0x25: {
      ppu->windowLayer[(adr - 0x23) * 2].window1inversed = (val & 0x1) != 0;
      ppu->windowLayer[(adr - 0x23) * 2].window1enabled = (val & 0x2) != 0;
      ppu->windowLayer[(adr - 0x23) * 2].window2inversed = (val & 0x4) != 0;
      ppu->windowLayer[(adr - 0x23) * 2].window2enabled = (val & 0x8) != 0;
      ppu->windowLayer[(adr - 0x23) * 2 + 1].window1inversed = (val & 0x10) != 0;
      ppu->windowLayer[(adr - 0x23) * 2 + 1].window1enabled = (val & 0x20) != 0;
      ppu->windowLayer[(adr - 0x23) * 2 + 1].window2inversed = (val & 0x40) != 0;
      ppu->windowLayer[(adr - 0x23) * 2 + 1].window2enabled = (val & 0x80) != 0;
      break;
    }
    case 0x26: {
      ppu->window1left = val;
      break;
    }
    case 0x27: {
      ppu->window1right = val;
      break;
    }
    case 0x28: {
      ppu->window2left = val;
      break;
    }
    case 0x29: {
      ppu->window2right = val;
      break;
    }
    case 0x2a: {
      assert(val == 0);
      // maskLogic_always_zero
      break;
    }
    case 0x2b: {
      assert(val == 0);
      // maskLogic_always_zero
      break;
    }
    case 0x2c: {
      ppu->layer[0].screenEnabled[0] = val & 0x1;
      ppu->layer[1].screenEnabled[0] = val & 0x2;
      ppu->layer[2].screenEnabled[0] = val & 0x4;
      ppu->layer[3].screenEnabled[0] = val & 0x8;
      ppu->layer[4].screenEnabled[0] = val & 0x10;
      break;
    }
    case 0x2d: {
      ppu->layer[0].screenEnabled[1] = val & 0x1;
      ppu->layer[1].screenEnabled[1] = val & 0x2;
      ppu->layer[2].screenEnabled[1] = val & 0x4;
      ppu->layer[3].screenEnabled[1] = val & 0x8;
      ppu->layer[4].screenEnabled[1] = val & 0x10;
      break;
    }
    case 0x2e: {
      ppu->layer[0].screenWindowed[0] = val & 0x1;
      ppu->layer[1].screenWindowed[0] = val & 0x2;
      ppu->layer[2].screenWindowed[0] = val & 0x4;
      ppu->layer[3].screenWindowed[0] = val & 0x8;
      ppu->layer[4].screenWindowed[0] = val & 0x10;
      break;
    }
    case 0x2f: {
      ppu->layer[0].screenWindowed[1] = val & 0x1;
      ppu->layer[1].screenWindowed[1] = val & 0x2;
      ppu->layer[2].screenWindowed[1] = val & 0x4;
      ppu->layer[3].screenWindowed[1] = val & 0x8;
      ppu->layer[4].screenWindowed[1] = val & 0x10;
      break;
    }
    case 0x30: {
      assert((val & 1) == 0);  // directColor always zero
      ppu->addSubscreen = val & 0x2;
      ppu->preventMathMode = (val & 0x30) >> 4;
      ppu->clipMode = (val & 0xc0) >> 6;
      break;
    }
    case 0x31: {
      ppu->subtractColor = val & 0x80;
      ppu->halfColor = val & 0x40;
      for(int i = 0; i < 6; i++) {
        ppu->mathEnabled[i] = val & (1 << i);
      }
      break;
    }
    case 0x32: {
      if(val & 0x80) ppu->fixedColorB = val & 0x1f;
      if(val & 0x40) ppu->fixedColorG = val & 0x1f;
      if(val & 0x20) ppu->fixedColorR = val & 0x1f;
      break;
    }
    case 0x33: {
      assert(val == 0);
      ppu->interlace_always_zero = val & 0x1;
      ppu->objInterlace_always_zero = val & 0x2;
      ppu->overscan_always_zero = val & 0x4;
      ppu->pseudoHires_always_zero = val & 0x8;
      ppu->m7extBg_always_zero = val & 0x40;
      break;
    }
    default: {
      break;
    }
  }
}

void ppu_putPixels(Ppu* ppu, uint8_t* pixels) {
  for(int y = 0; y < 224; y++) {
    int dest = y * 2 + 16;
    int y1 = y + (ppu->evenFrame ? 0 : 239);
    memcpy(pixels + (dest * 2048), &ppu->pixelBuffer[y1 * 2048], 2048);
    memcpy(pixels + ((dest + 1) * 2048), &ppu->pixelBuffer[y1 * 2048], 2048);
  }
  // clear top 16 and last 16 lines
  memset(pixels, 0, 2048 * 16);
  memset(pixels + (464 * 2048), 0, 2048 * 16);
}
