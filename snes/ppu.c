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

static const uint8 kSpriteSizes[8][2] = {
  {8, 16}, {8, 32}, {8, 64}, {16, 32},
  {16, 64}, {32, 64}, {16, 32}, {16, 32}
};

static void ppu_handlePixel(Ppu* ppu, int x, int y);
static int ppu_getPixel(Ppu* ppu, int x, int y, bool sub, int* r, int* g, int* b);
static int ppu_getPixelForBgLayer(Ppu *ppu, int x, int y, int layer, bool priority);
static void ppu_calculateMode7Starts(Ppu* ppu, int y);
static int ppu_getPixelForMode7(Ppu* ppu, int x, int layer, bool priority);
static bool ppu_getWindowState(Ppu* ppu, int layer, int x);
static bool ppu_evaluateSprites(Ppu* ppu, int line);
static uint16_t ppu_getVramRemap(Ppu* ppu);
static void PpuDrawWholeLine(Ppu *ppu, uint y);

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
  ppu->lastBrightnessMult = 0xff;
  ppu->lastMosaicModulo = 0xff;
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
  memset(&ppu->objBuffer, 0, sizeof(ppu->objBuffer));
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
}

void ppu_saveload(Ppu *ppu, SaveLoadFunc *func, void *ctx) {
  func(ctx, &ppu->vram, offsetof(Ppu, mosaicModulo) - offsetof(Ppu, vram));
}

bool PpuBeginDrawing(Ppu *ppu, uint8_t *pixels, size_t pitch, uint32_t render_flags) {
  ppu->renderFlags = render_flags;
  bool hq = ppu->mode == 7 && !ppu->forcedBlank && 
      (ppu->renderFlags & (kPpuRenderFlags_4x4Mode7 | kPpuRenderFlags_NewRenderer)) == (kPpuRenderFlags_4x4Mode7 | kPpuRenderFlags_NewRenderer);
  ppu->renderPitch = (uint)pitch;
  ppu->renderBuffer = pixels;

  // Cache the brightness computation
  if (ppu->brightness != ppu->lastBrightnessMult) {
    uint8_t ppu_brightness = ppu->brightness;
    ppu->lastBrightnessMult = ppu_brightness;
    for (int i = 0; i < 32; i++)
      ppu->brightnessMultHalf[i * 2] = ppu->brightnessMultHalf[i * 2 + 1] = ppu->brightnessMult[i] =
      ((i << 3) | (i >> 2)) * ppu_brightness / 15;
    // Store 31 extra entries to remove the need for clamping to 31.
    memset(&ppu->brightnessMult[32], ppu->brightnessMult[31], 31);
  }

  if (hq) {
    for (int i = 0; i < 256; i++) {
      uint32 color = ppu->cgram[i];
      ppu->colorMapRgb[i] = ppu->brightnessMult[color & 0x1f] << 16 | ppu->brightnessMult[(color >> 5) & 0x1f] << 8 | ppu->brightnessMult[(color >> 10) & 0x1f];
    }
  }

  
  return hq;
}

void ppu_runLine(Ppu *ppu, int line) {
  if(line == 0) {
    ppu->rangeOver = false;
    ppu->timeOver = false;
    ppu->evenFrame = !ppu->evenFrame;
  } else {
    if (ppu->mosaicSize != ppu->lastMosaicModulo) {
      int mod = ppu->mosaicSize;
      ppu->lastMosaicModulo = mod;
      for (int i = 0, j = 0; i < 256; i++) {
        ppu->mosaicModulo[i] = i - j;
        j = (j + 1 == mod ? 0 : j + 1);
      }
    }
    // evaluate sprites
    memset(&ppu->objBuffer.pixel, 0, sizeof(ppu->objBuffer.pixel));
    memset(&ppu->objBuffer.prio, 0x05, sizeof(ppu->objBuffer.prio));
    ppu->lineHasSprites = !ppu->forcedBlank && ppu_evaluateSprites(ppu, line - 1);

    // actual line
    if (ppu->renderFlags & kPpuRenderFlags_NewRenderer) {
      PpuDrawWholeLine(ppu, line);
    } else {
      if (ppu->mode == 7)
        ppu_calculateMode7Starts(ppu, line);
      for (int x = 0; x < 256; x++)
        ppu_handlePixel(ppu, x, line);
      uint8 *dst = ppu->renderBuffer + ((line - 1) * 2 * ppu->renderPitch);
      memcpy(dst + ppu->renderPitch, dst, 512 * 4);
    }

  }
}

typedef struct PpuWindows {
  uint16 edges[6];
  uint8 nr;
  uint8 bits;
} PpuWindows;

static void PpuWindows_Clear(PpuWindows *win) {
  win->edges[0] = 0;
  win->edges[1] = 256;
  win->nr = 1;
  win->bits = 0;
}

static void PpuWindows_Calc(PpuWindows *win, Ppu *ppu, uint layer) {
  WindowLayer *wl = &ppu->windowLayer[layer];
  // Evaluate which spans to render based on the window settings.
  // There are at most 5 windows.
  // Algorithm from Snes9x
  uint nr = 1;
  win->edges[0] = 0;
  win->edges[1] = 256;
  uint8 window_bits = 0;
  uint i, j, t;
  bool w1_ena = wl->window1enabled && ppu->window1left <= ppu->window1right;
  if (w1_ena) {
    if (ppu->window1left) {
      win->edges[nr] = ppu->window1left;
      win->edges[++nr] = 256;
    }
    if (ppu->window1right < 255) {
      win->edges[nr] = ppu->window1right + 1;
      win->edges[++nr] = 256;
    }
  }
  bool w2_ena = wl->window2enabled && ppu->window2left <= ppu->window2right;
  if (w2_ena) {
    for (i = 0; i <= nr && (t = ppu->window2left) != win->edges[i]; i++) {
      if (t < win->edges[i]) {
        for (j = nr++; j >= i; j--)
          win->edges[j + 1] = win->edges[j];
        win->edges[i] = t;
        break;
      }
    }
    for (; i <= nr && (t = ppu->window2right + 1) != win->edges[i]; i++) {
      if (t < win->edges[i]) {
        for (j = nr++; j >= i; j--)
          win->edges[j + 1] = win->edges[j];
        win->edges[i] = t;
        break;
      }
    }
  }
  win->nr = nr;
  // get a bitmap of how regions map to windows
  uint8 w1_bits = 0, w2_bits = 0;
  if (w1_ena) {
    for (i = 0; win->edges[i] != ppu->window1left; i++);
    for (j = i; win->edges[j] != ppu->window1right + 1; j++);
    w1_bits = ((1 << (j - i)) - 1) << i;
  }
  if (wl->window1enabled & wl->window1inversed)
    w1_bits = ~w1_bits;
  if (w2_ena) {
    for (i = 0; win->edges[i] != ppu->window2left; i++);
    for (j = i; win->edges[j] != ppu->window2right + 1; j++);
    w2_bits = ((1 << (j - i)) - 1) << i;
  }
  if (wl->window2enabled & wl->window2inversed)
    w2_bits = ~w2_bits;
  win->bits = w1_bits | w2_bits;
}

// Draw a whole line of a 4bpp background layer into bgBuffers
static void PpuDrawBackground_4bpp(Ppu *ppu, uint y, bool sub, uint layer, uint8 zhi, uint8 zlo) {
#define DO_PIXEL(i) do { \
  pixel = (bits >> i) & 1 | (bits >> (7 + i)) & 2 | (bits >> (14 + i)) & 4 | (bits >> (21 + i)) & 8; \
  if (pixel && z > dst[256 + i]) dst[i] = paletteBase + pixel, dst[256 + i] = z; } while (0)
#define DO_PIXEL_HFLIP(i) do { \
  pixel = (bits >> (7 - i)) & 1 | (bits >> (14 - i)) & 2 | (bits >> (21 - i)) & 4 | (bits >> (28 - i)) & 8; \
  if (pixel && z > dst[256 + i]) dst[i] = paletteBase + pixel, dst[256 + i] = z; } while (0)
#define READ_BITS(ta, tile) (addr = &ppu->vram[((ta) + (tile) * 16) & 0x7fff], addr[0] | addr[8] << 16)
  enum { kPaletteShift = 6 };
  Layer *layerp = &ppu->layer[layer];
  if (!layerp->screenEnabled[sub])
    return;  // layer is completely hidden
  PpuWindows win;
  layerp->screenWindowed[sub] ? PpuWindows_Calc(&win, ppu, layer) : PpuWindows_Clear(&win);
  BgLayer *bglayer = &ppu->bgLayer[layer];
  y += bglayer->vScroll;
  int sc_offs = bglayer->tilemapAdr + (((y >> 3) & 0x1f) << 5);
  if ((y & 0x100) && bglayer->tilemapHigher)
    sc_offs += bglayer->tilemapWider ? 0x800 : 0x400;
  const uint16 *tps[2] = {
    &ppu->vram[sc_offs & 0x7fff],
    &ppu->vram[sc_offs + (bglayer->tilemapWider ? 0x400 : 0) & 0x7fff]
  };
  int tileadr = ppu->bgLayer[layer].tileAdr, pixel;
  int tileadr1 = tileadr + 7 - (y & 0x7), tileadr0 = tileadr + (y & 0x7);
  const uint16 *addr;
  for (size_t windex = 0; windex < win.nr; windex++) {
    if (win.bits & (1 << windex))
      continue;  // layer is disabled for this window part
    uint x = win.edges[windex] + bglayer->hScroll;
    uint w = win.edges[windex + 1] - win.edges[windex];
    uint8 *dst = ppu->bgBuffers[sub].pixel + win.edges[windex];
    const uint16 *tp = tps[x >> 8 & 1] + ((x >> 3) & 0x1f);
    const uint16 *tp_last = tps[x >> 8 & 1] + 31;
    const uint16 *tp_next = tps[(x >> 8 & 1) ^ 1];
    // Handle clipped pixels on left side
    if (x & 7) {
      int curw = IntMin(8 - (x & 7), w);
      w -= curw;
      uint32 tile = *tp;
      tp = (tp != tp_last) ? tp + 1 : tp_next;
      int ta = (tile & 0x8000) ? tileadr1 : tileadr0;
      uint8 z = (tile & 0x2000) ? zhi : zlo;
      uint32 bits = READ_BITS(ta, tile & 0x3ff);
      if (bits) {
        int paletteBase = (tile & 0x1c00) >> kPaletteShift;
        if (tile & 0x4000) {
          bits >>= (x & 7), x += curw;
          do DO_PIXEL(0); while (bits >>= 1, dst++, --curw);
        } else {
          bits <<= (x & 7), x += curw;
          do DO_PIXEL_HFLIP(0); while (bits <<= 1, dst++, --curw);
        }
      } else {
        dst += curw;
      }
    }
    // Handle full tiles in the middle
    while (w >= 8) {
      uint32 tile = *tp;
      tp = (tp != tp_last) ? tp + 1 : tp_next;
      int ta = (tile & 0x8000) ? tileadr1 : tileadr0;
      uint8 z = (tile & 0x2000) ? zhi : zlo;
      uint32 bits = READ_BITS(ta, tile & 0x3ff);
      if (bits) {
        int paletteBase = (tile & 0x1c00) >> kPaletteShift;
        if (tile & 0x4000) {
          DO_PIXEL(0); DO_PIXEL(1); DO_PIXEL(2); DO_PIXEL(3);
          DO_PIXEL(4); DO_PIXEL(5); DO_PIXEL(6); DO_PIXEL(7);
        } else {
          DO_PIXEL_HFLIP(0); DO_PIXEL_HFLIP(1); DO_PIXEL_HFLIP(2); DO_PIXEL_HFLIP(3);
          DO_PIXEL_HFLIP(4); DO_PIXEL_HFLIP(5); DO_PIXEL_HFLIP(6); DO_PIXEL_HFLIP(7);
        }
      }
      dst += 8, w -= 8;
    }
    // Handle remaining clipped part
    if (w) {
      uint32 tile = *tp;
      int ta = (tile & 0x8000) ? tileadr1 : tileadr0;
      uint8 z = (tile & 0x2000) ? zhi : zlo;
      uint32 bits = READ_BITS(ta, tile & 0x3ff);
      if (bits) {
        int paletteBase = (tile & 0x1c00) >> kPaletteShift;
        if (tile & 0x4000) {
          do DO_PIXEL(0); while (bits >>= 1, dst++, --w);
        } else {
          do DO_PIXEL_HFLIP(0); while (bits <<= 1, dst++, --w);
        }
      }
    }
  }
#undef READ_BITS
#undef DO_PIXEL
#undef DO_PIXEL_HFLIP
}

// Draw a whole line of a 2bpp background layer into bgBuffers
static void PpuDrawBackground_2bpp(Ppu *ppu, uint y, bool sub, uint layer, uint8 zhi, uint8 zlo) {
#define DO_PIXEL(i) do { \
  pixel = (bits >> i) & 1 | (bits >> (7 + i)) & 2; \
  if (pixel && z > dst[256 + i]) dst[i] = paletteBase + pixel, dst[256 + i] = z; } while (0)
#define DO_PIXEL_HFLIP(i) do { \
  pixel = (bits >> (7 - i)) & 1 | (bits >> (14 - i)) & 2; \
  if (pixel && z > dst[256 + i]) dst[i] = paletteBase + pixel, dst[256 + i] = z; } while (0)
#define READ_BITS(ta, tile) (addr = &ppu->vram[(ta) + (tile) * 8 & 0x7fff], addr[0])
  enum { kPaletteShift = 8 };
  Layer *layerp = &ppu->layer[layer];
  if (!layerp->screenEnabled[sub])
    return;  // layer is completely hidden
  PpuWindows win;
  layerp->screenWindowed[sub] ? PpuWindows_Calc(&win, ppu, layer) : PpuWindows_Clear(&win);
  BgLayer *bglayer = &ppu->bgLayer[layer];
  y += bglayer->vScroll;
  int sc_offs = bglayer->tilemapAdr + (((y >> 3) & 0x1f) << 5);
  if ((y & 0x100) && bglayer->tilemapHigher)
    sc_offs += bglayer->tilemapWider ? 0x800 : 0x400;
  const uint16 *tps[2] = {
    &ppu->vram[sc_offs & 0x7fff],
    &ppu->vram[sc_offs + (bglayer->tilemapWider ? 0x400 : 0) & 0x7fff]
  };
  int tileadr = ppu->bgLayer[layer].tileAdr, pixel;
  int tileadr1 = tileadr + 7 - (y & 0x7), tileadr0 = tileadr + (y & 0x7);
  const uint16 *addr;
  for (size_t windex = 0; windex < win.nr; windex++) {
    if (win.bits & (1 << windex))
      continue;  // layer is disabled for this window part
    uint x = win.edges[windex] + bglayer->hScroll;
    uint w = win.edges[windex + 1] - win.edges[windex];
    uint8 *dst = ppu->bgBuffers[sub].pixel + win.edges[windex];
    const uint16 *tp = tps[x >> 8 & 1] + ((x >> 3) & 0x1f);
    const uint16 *tp_last = tps[x >> 8 & 1] + 31;
    const uint16 *tp_next = tps[(x >> 8 & 1) ^ 1];
    // Handle clipped pixels on left side
    if (x & 7) {
      int curw = IntMin(8 - (x & 7), w);
      w -= curw;
      uint32 tile = *tp;
      tp = (tp != tp_last) ? tp + 1 : tp_next;
      int ta = (tile & 0x8000) ? tileadr1 : tileadr0;
      uint8 z = (tile & 0x2000) ? zhi : zlo;
      uint32 bits = READ_BITS(ta, tile & 0x3ff);
      if (bits) {
        int paletteBase = (tile & 0x1c00) >> kPaletteShift;
        if (tile & 0x4000) {
          bits >>= (x & 7), x += curw;
          do DO_PIXEL(0); while (bits >>= 1, dst++, --curw);
        } else {
          bits <<= (x & 7), x += curw;
          do DO_PIXEL_HFLIP(0); while (bits <<= 1, dst++, --curw);
        }
      } else {
        dst += curw;
      }
    }
    // Handle full tiles in the middle
    while (w >= 8) {
      uint32 tile = *tp;
      tp = (tp != tp_last) ? tp + 1 : tp_next;
      int ta = (tile & 0x8000) ? tileadr1 : tileadr0;
      uint8 z = (tile & 0x2000) ? zhi : zlo;
      uint32 bits = READ_BITS(ta, tile & 0x3ff);
      if (bits) {
        int paletteBase = (tile & 0x1c00) >> kPaletteShift;
        if (tile & 0x4000) {
          DO_PIXEL(0); DO_PIXEL(1); DO_PIXEL(2); DO_PIXEL(3);
          DO_PIXEL(4); DO_PIXEL(5); DO_PIXEL(6); DO_PIXEL(7);
        } else {
          DO_PIXEL_HFLIP(0); DO_PIXEL_HFLIP(1); DO_PIXEL_HFLIP(2); DO_PIXEL_HFLIP(3);
          DO_PIXEL_HFLIP(4); DO_PIXEL_HFLIP(5); DO_PIXEL_HFLIP(6); DO_PIXEL_HFLIP(7);
        }
      }
      dst += 8, w -= 8;
    }
    // Handle remaining clipped part
    if (w) {
      uint32 tile = *tp;
      int ta = (tile & 0x8000) ? tileadr1 : tileadr0;
      uint8 z = (tile & 0x2000) ? zhi : zlo;
      uint32 bits = READ_BITS(ta, tile & 0x3ff);
      if (bits) {
        int paletteBase = (tile & 0x1c00) >> kPaletteShift;
        if (tile & 0x4000) {
          do DO_PIXEL(0); while (bits >>= 1, dst++, --w);
        } else {
          do DO_PIXEL_HFLIP(0); while (bits <<= 1, dst++, --w);
        }
      }
    }
  }
#undef READ_BITS
#undef DO_PIXEL
#undef DO_PIXEL_HFLIP
}

// Draw a whole line of a 4bpp background layer into bgBuffers, with mosaic applied
static void PpuDrawBackground_4bpp_mosaic(Ppu *ppu, uint y, bool sub, uint layer, uint8 zhi, uint8 zlo) {
#define GET_PIXEL(i) pixel = (bits) & 1 | (bits >> 7) & 2 | (bits >> 14) & 4 | (bits >> 21) & 8
#define GET_PIXEL_HFLIP(i) pixel = (bits >> 7) & 1 | (bits >> 14) & 2 | (bits >> 21) & 4 | (bits >> 28) & 8
#define READ_BITS(ta, tile) (addr = &ppu->vram[((ta) + (tile) * 16) & 0x7fff], addr[0] | addr[8] << 16)
  enum { kPaletteShift = 6 };
  Layer *layerp = &ppu->layer[layer];
  if (!layerp->screenEnabled[sub])
    return;  // layer is completely hidden
  PpuWindows win;
  layerp->screenWindowed[sub] ? PpuWindows_Calc(&win, ppu, layer) : PpuWindows_Clear(&win);
  BgLayer *bglayer = &ppu->bgLayer[layer];
  y = ppu->mosaicModulo[y] + bglayer->vScroll;
  int sc_offs = bglayer->tilemapAdr + (((y >> 3) & 0x1f) << 5);
  if ((y & 0x100) && bglayer->tilemapHigher)
    sc_offs += bglayer->tilemapWider ? 0x800 : 0x400;
  const uint16 *tps[2] = {
    &ppu->vram[sc_offs & 0x7fff],
    &ppu->vram[sc_offs + (bglayer->tilemapWider ? 0x400 : 0) & 0x7fff]
  };
  int tileadr = ppu->bgLayer[layer].tileAdr, pixel;
  int tileadr1 = tileadr + 7 - (y & 0x7), tileadr0 = tileadr + (y & 0x7);
  const uint16 *addr;
  for (size_t windex = 0; windex < win.nr; windex++) {
    if (win.bits & (1 << windex))
      continue;  // layer is disabled for this window part
    int sx = win.edges[windex];
    uint8 *dst = ppu->bgBuffers[sub].pixel + sx;
    uint8 *dst_end = ppu->bgBuffers[sub].pixel + win.edges[windex + 1];
    uint x = sx + bglayer->hScroll;
    const uint16 *tp = tps[x >> 8 & 1] + ((x >> 3) & 0x1f);
    const uint16 *tp_last = tps[x >> 8 & 1] + 31, *tp_next = tps[(x >> 8 & 1) ^ 1];
    x &= 7;
    int w = ppu->mosaicSize - (sx - ppu->mosaicModulo[sx]);
    do {
      w = IntMin(w, dst_end - dst);
      uint32 tile = *tp;
      int ta = (tile & 0x8000) ? tileadr1 : tileadr0;
      uint8 z = (tile & 0x2000) ? zhi : zlo;
      uint32 bits = READ_BITS(ta, tile & 0x3ff);
      if (tile & 0x4000) bits >>= x, GET_PIXEL(0); else bits <<= x, GET_PIXEL_HFLIP(0);
      if (pixel) {
        pixel += (tile & 0x1c00) >> kPaletteShift;
        int i = 0;
        do {
          if (z > dst[i + 256])
            dst[i] = pixel, dst[i + 256] = z;
        } while (++i != w);
      }
      dst += w, x += w;
      for (; x >= 8; x -= 8)
        tp = (tp != tp_last) ? tp + 1 : tp_next;
      w = ppu->mosaicSize;
    } while (dst_end - dst != 0);
  }
#undef READ_BITS
#undef GET_PIXEL
#undef GET_PIXEL_HFLIP
}

// Draw a whole line of a 2bpp background layer into bgBuffers, with mosaic applied
static void PpuDrawBackground_2bpp_mosaic(Ppu *ppu, int y, bool sub, uint layer, uint8 zhi, uint8 zlo) {
#define GET_PIXEL(i) pixel = (bits) & 1 | (bits >> 7) & 2
#define GET_PIXEL_HFLIP(i) pixel = (bits >> 7) & 1 | (bits >> 14) & 2
#define READ_BITS(ta, tile) (addr = &ppu->vram[((ta) + (tile) * 8) & 0x7fff], addr[0])
  enum { kPaletteShift = 8 };
  Layer *layerp = &ppu->layer[layer];
  if (!layerp->screenEnabled[sub])
    return;  // layer is completely hidden
  PpuWindows win;
  layerp->screenWindowed[sub] ? PpuWindows_Calc(&win, ppu, layer) : PpuWindows_Clear(&win);
  BgLayer *bglayer = &ppu->bgLayer[layer];
  y = ppu->mosaicModulo[y] + bglayer->vScroll;
  int sc_offs = bglayer->tilemapAdr + (((y >> 3) & 0x1f) << 5);
  if ((y & 0x100) && bglayer->tilemapHigher)
    sc_offs += bglayer->tilemapWider ? 0x800 : 0x400;
  const uint16 *tps[2] = {
    &ppu->vram[sc_offs & 0x7fff],
    &ppu->vram[sc_offs + (bglayer->tilemapWider ? 0x400 : 0) & 0x7fff]
  };
  int tileadr = ppu->bgLayer[layer].tileAdr, pixel;
  int tileadr1 = tileadr + 7 - (y & 0x7), tileadr0 = tileadr + (y & 0x7);
  const uint16 *addr;
  for (size_t windex = 0; windex < win.nr; windex++) {
    if (win.bits & (1 << windex))
      continue;  // layer is disabled for this window part
    int sx = win.edges[windex];
    uint8 *dst = ppu->bgBuffers[sub].pixel + sx;
    uint8 *dst_end = ppu->bgBuffers[sub].pixel + win.edges[windex + 1];
    uint x = sx + bglayer->hScroll;
    const uint16 *tp = tps[x >> 8 & 1] + ((x >> 3) & 0x1f);
    const uint16 *tp_last = tps[x >> 8 & 1] + 31, *tp_next = tps[(x >> 8 & 1) ^ 1];
    x &= 7;
    int w = ppu->mosaicSize - (sx - ppu->mosaicModulo[sx]);
    do {
      w = IntMin(w, dst_end - dst);
      uint32 tile = *tp;
      int ta = (tile & 0x8000) ? tileadr1 : tileadr0;
      uint8 z = (tile & 0x2000) ? zhi : zlo;
      uint32 bits = READ_BITS(ta, tile & 0x3ff);
      if (tile & 0x4000) bits >>= x, GET_PIXEL(0); else bits <<= x, GET_PIXEL_HFLIP(0);
      if (pixel) {
        pixel += (tile & 0x1c00) >> kPaletteShift;
        uint i = 0;
        do {
          if (z > dst[i + 256])
            dst[i] = pixel, dst[i + 256] = z;
        } while (++i != w);
      }
      dst += w, x += w;
      for (; x >= 8; x -= 8)
        tp = (tp != tp_last) ? tp + 1 : tp_next;
      w = ppu->mosaicSize;
    } while (dst_end - dst != 0);
  }
#undef READ_BITS
#undef GET_PIXEL
#undef GET_PIXEL_HFLIP
}


// level6 should be set if it's from palette 0xc0 which means color math is not applied
#define SPRITE_PRIO_TO_PRIO(prio, level6) (((prio) * 4 + 2) * 16 + 4 + (level6 ? 2 : 0))
#define SPRITE_PRIO_TO_PRIO_HI(prio) ((prio) * 4 + 2)

static void PpuDrawSprites(Ppu *ppu, uint y, uint sub, bool clear_backdrop) {
  Layer *layerp = &ppu->layer[4];
  if (!layerp->screenEnabled[sub])
    return;  // layer is completely hidden
  PpuWindows win;
  layerp->screenWindowed[sub] ? PpuWindows_Calc(&win, ppu, 4) : PpuWindows_Clear(&win);
  for (size_t windex = 0; windex < win.nr; windex++) {
    if (win.bits & (1 << windex))
      continue;  // layer is disabled for this window part
    int left = win.edges[windex];
    int width = win.edges[windex + 1] - left;
    uint8 *src = ppu->objBuffer.pixel + left;
    uint8 *dst = ppu->bgBuffers[sub].pixel + left;
    if (clear_backdrop) {
      memcpy(dst, src, width);
      memcpy(dst + 256, src + 256, width);
    } else {
      do {
        if (src[256] > dst[256])
          dst[0] = src[0], dst[256] = src[256];
      } while (src++, dst++, --width);
    }
  }
}

// Assumes it's drawn on an empty backdrop
static void PpuDrawBackground_mode7(Ppu *ppu, uint y, bool sub, uint8 z) {
  int layer = 0;
  Layer *layerp = &ppu->layer[layer];
  if (!layerp->screenEnabled[sub])
    return;  // layer is completely hidden
  PpuWindows win;
  layerp->screenWindowed[sub] ? PpuWindows_Calc(&win, ppu, layer) : PpuWindows_Clear(&win);

  // expand 13-bit values to signed values
  int hScroll = ((int16_t)(ppu->m7matrix[6] << 3)) >> 3;
  int vScroll = ((int16_t)(ppu->m7matrix[7] << 3)) >> 3;
  int xCenter = ((int16_t)(ppu->m7matrix[4] << 3)) >> 3;
  int yCenter = ((int16_t)(ppu->m7matrix[5] << 3)) >> 3;
  int clippedH = hScroll - xCenter;
  int clippedV = vScroll - yCenter;
  clippedH = (clippedH & 0x2000) ? (clippedH | ~1023) : (clippedH & 1023);
  clippedV = (clippedV & 0x2000) ? (clippedV | ~1023) : (clippedV & 1023);
  bool mosaic_enabled = ppu->bgLayer[0].mosaicEnabled && ppu->mosaicSize > 1;
  if (mosaic_enabled)
    y = ppu->mosaicModulo[y];
  uint32 ry = ppu->m7yFlip ? 255 - y : y;
  uint32 m7startX = (ppu->m7matrix[0] * clippedH & ~63) + (ppu->m7matrix[1] * ry & ~63) +
    (ppu->m7matrix[1] * clippedV & ~63) + (xCenter << 8);
  uint32 m7startY = (ppu->m7matrix[2] * clippedH & ~63) + (ppu->m7matrix[3] * ry & ~63) +
    (ppu->m7matrix[3] * clippedV & ~63) + (yCenter << 8);
  for (size_t windex = 0; windex < win.nr; windex++) {
    if (win.bits & (1 << windex))
      continue;  // layer is disabled for this window part
    int x = win.edges[windex], x2 = win.edges[windex + 1], tile;
    uint8 *dst = ppu->bgBuffers[sub].pixel + x, *dst_end = ppu->bgBuffers[sub].pixel + x2;
    uint32 rx = ppu->m7xFlip ? 255 - x : x;
    uint32 xpos = m7startX + ppu->m7matrix[0] * rx;
    uint32 ypos = m7startY + ppu->m7matrix[2] * rx;
    uint32 dx = ppu->m7xFlip ? -ppu->m7matrix[0] : ppu->m7matrix[0];
    uint32 dy = ppu->m7xFlip ? -ppu->m7matrix[2] : ppu->m7matrix[2];
    uint32 outside_value = ppu->m7largeField ? 0x3ffff : 0xffffffff;
    bool char_fill = ppu->m7charFill;
    if (mosaic_enabled) {
      int w = ppu->mosaicSize - (x - ppu->mosaicModulo[x]);
      do {
        w = IntMin(w, dst_end - dst);
        if ((uint32)(xpos | ypos) > outside_value) {
          if (!char_fill)
            continue;
          tile = 0;
        } else {
          tile = ppu->vram[(ypos >> 11 & 0x7f) * 128 + (xpos >> 11 & 0x7f)] & 0xff;
        }
        uint8 pixel = ppu->vram[tile * 64 + (ypos >> 8 & 7) * 8 + (xpos >> 8 & 7)] >> 8;
        if (pixel) {
          int i = 0;
          do dst[i] = pixel, dst[i + 256] = z; while (++i != w);
        }
      } while (xpos += dx * w, ypos += dy * w, dst += w, w = ppu->mosaicSize, dst_end - dst != 0);
    } else {
      do {
        if ((uint32)(xpos | ypos) > outside_value) {
          if (!char_fill)
            continue;
          tile = 0;
        } else {
          tile = ppu->vram[(ypos >> 11 & 0x7f) * 128 + (xpos >> 11 & 0x7f)] & 0xff;
        }
        uint8 pixel = ppu->vram[tile * 64 + (ypos >> 8 & 7) * 8 + (xpos >> 8 & 7)] >> 8;
        if (pixel)
          dst[0] = pixel, dst[256] = z;
      } while (xpos += dx, ypos += dy, ++dst != dst_end);
    }
  }
}

uint16 g_mode7_lo, g_mode7_hi;
void PpuSetMode7PerspectiveCorrection(Ppu *ppu, int low, int high) {
  ppu->mode7PerspectiveLow = low ? 1.0f / low : 0.0f;
  ppu->mode7PerspectiveHigh = 1.0f / high;
}

static FORCEINLINE float FloatInterpolate(float x, float xmin, float xmax, float ymin, float ymax) {
  return ymin + (ymax - ymin) * (x - xmin) * (1.0f / (xmax - xmin));
}

// Upsampled version of mode7 rendering. Draws everything in 4x the normal resolution.
// Draws directly to the pixel buffer and bypasses any math, and supports only
// a subset of the normal features (all that zelda needs)
static void PpuDrawMode7Upsampled(Ppu *ppu, uint y) {
  // expand 13-bit values to signed values
  uint32 xCenter = ((int16_t)(ppu->m7matrix[4] << 3)) >> 3, yCenter = ((int16_t)(ppu->m7matrix[5] << 3)) >> 3;
  uint32 clippedH = (((int16_t)(ppu->m7matrix[6] << 3)) >> 3) - xCenter;
  uint32 clippedV = (((int16_t)(ppu->m7matrix[7] << 3)) >> 3) - yCenter;
  uint32 m0 = ppu->m7matrix[0];  // xpos increment per horiz movement
  uint32 m3 = ppu->m7matrix[3];  // ypos increment per vert movement
  uint8 *dst_start = &ppu->renderBuffer[(y - 1) * 4 * ppu->renderPitch], *dst_end, *dst = dst_start;
  int32 m0v[4];
  if (*(uint32*)&ppu->mode7PerspectiveLow == 0) {
    m0v[0] = m0v[1] = m0v[2] = m0v[3] = ppu->m7matrix[0] << 12;
  } else {
    static const float kInterpolateOffsets[4] = { -1, -1 + 0.25f, -1 + 0.5f, -1 + 0.75f };
    for (int i = 0; i < 4; i++)
      m0v[i] = 4096.0f / FloatInterpolate((int)y + kInterpolateOffsets[i], 0, 223, ppu->mode7PerspectiveLow, ppu->mode7PerspectiveHigh);
  }

  for (int j = 0; j < 4; j++) {
    m0 = m3 = m0v[j];
    uint32 m1 = ppu->m7matrix[1] << 12;  // xpos increment per vert movement
    uint32 m2 = ppu->m7matrix[2] << 12;  // ypos increment per horiz movement
    uint32 xpos = m0 * clippedH + m1 * (clippedV + y) + (xCenter << 20), xcur;
    uint32 ypos = m2 * clippedH + m3 * (clippedV + y) + (yCenter << 20), ycur;

    uint32 tile, pixel;
    xpos -= (m0 + m1) >> 1;
    ypos -= (m2 + m3) >> 1;
    xcur = (xpos << 2) + j * m1;
    ycur = (ypos << 2) + j * m3;
    dst_end = dst + 4096;

#define DRAW_PIXEL() \
    tile = ppu->vram[(ycur >> 25 & 0x7f) * 128 + (xcur >> 25 & 0x7f)] & 0xff;  \
    pixel = ppu->vram[tile * 64 + (ycur >> 22 & 7) * 8 + (xcur >> 22 & 7)] >> 8; \
    *(uint32*)dst = ppu->colorMapRgb[pixel]; \
    xcur += m0, ycur += m2, dst += 4;

    do {
      DRAW_PIXEL();
      DRAW_PIXEL();
      DRAW_PIXEL();
      DRAW_PIXEL();
    } while (dst != dst_end);
#undef DRAW_PIXEL
    dst += (ppu->renderPitch - 4096);
  }

  if (ppu->lineHasSprites) {
    uint8 *pixels = ppu->objBuffer.pixel;
    size_t pitch = ppu->renderPitch;
    for (size_t i = 0; i < 256; i++) {
      uint32 pixel = pixels[i];
      if (pixel) {
        uint32 color = ppu->colorMapRgb[pixel];
        uint8 *dst = dst_start + i * 16;

        ((uint32 *)dst)[3] = ((uint32 *)dst)[2] = ((uint32 *)dst)[1] = ((uint32 *)dst)[0] = color;
        ((uint32 *)(dst + pitch * 1))[3] = ((uint32 *)(dst + pitch * 1))[2] = ((uint32 *)(dst + pitch * 1))[1] = ((uint32 *)(dst + pitch * 1))[0] = color;
        ((uint32 *)(dst + pitch * 2))[3] = ((uint32 *)(dst + pitch * 2))[2] = ((uint32 *)(dst + pitch * 2))[1] = ((uint32 *)(dst + pitch * 2))[0] = color;
        ((uint32 *)(dst + pitch * 3))[3] = ((uint32 *)(dst + pitch * 3))[2] = ((uint32 *)(dst + pitch * 3))[1] = ((uint32 *)(dst + pitch * 3))[0] = color;
      }
    }
  }

#undef DRAW_PIXEL
}

static void PpuDrawBackgrounds(Ppu *ppu, int y, bool sub) {
// Top 4 bits contain the prio level, and bottom 4 bits the layer type.
// SPRITE_PRIO_TO_PRIO can be used to convert from obj prio to this prio.
//  15: BG3 tiles with priority 1 if bit 3 of $2105 is set
//  14: Sprites with priority 3 (4 * sprite_prio + 2)
//  12: BG1 tiles with priority 1
//  11: BG2 tiles with priority 1
//  10: Sprites with priority 2 (4 * sprite_prio + 2)
//  8: BG1 tiles with priority 0
//  7: BG2 tiles with priority 0
//  6: Sprites with priority 1 (4 * sprite_prio + 2)
//  3: BG3 tiles with priority 1 if bit 3 of $2105 is clear
//  2: Sprites with priority 0 (4 * sprite_prio + 2)
//  1: BG3 tiles with priority 0
//  0: backdrop

  if (ppu->mode == 1) {
    if (ppu->lineHasSprites)
      PpuDrawSprites(ppu, y, sub, true);

    if (ppu->bgLayer[0].mosaicEnabled && ppu->mosaicSize > 1)
      PpuDrawBackground_4bpp_mosaic(ppu, y, sub, 0, 0xc0, 0x80);
    else
      PpuDrawBackground_4bpp(ppu, y, sub, 0, 0xc0, 0x80);

    if (ppu->bgLayer[1].mosaicEnabled && ppu->mosaicSize > 1)
      PpuDrawBackground_4bpp_mosaic(ppu, y, sub, 1, 0xb1, 0x71);
    else
      PpuDrawBackground_4bpp(ppu, y, sub, 1, 0xb1, 0x71);

    if (ppu->bgLayer[2].mosaicEnabled && ppu->mosaicSize > 1)
      PpuDrawBackground_2bpp_mosaic(ppu, y, sub, 2, 0xf2, 0x12);
    else
      PpuDrawBackground_2bpp(ppu, y, sub, 2, 0xf2, 0x12);
  } else {
    // mode 7
    PpuDrawBackground_mode7(ppu, y, sub, 0xc0);
    if (ppu->lineHasSprites)
      PpuDrawSprites(ppu, y, sub, false);
  }
}

static NOINLINE void PpuDrawWholeLine(Ppu *ppu, uint y) {
  if (ppu->forcedBlank) {
    uint8 *dst = &ppu->renderBuffer[(y - 1) * 2 * ppu->renderPitch];
    size_t pitch = ppu->renderPitch;
    for (int i = 0; i < 256; i++, dst += 8) {
      ((uint32*)dst)[1] = ((uint32 *)dst)[0] = 0;
      ((uint32*)(dst + pitch))[1] = ((uint32*)(dst + pitch))[0] = 0;
    }
    return;
  }

  if (ppu->mode == 7 && (ppu->renderFlags & kPpuRenderFlags_4x4Mode7)) {
    PpuDrawMode7Upsampled(ppu, y);
    return;
  }


  // Default background is backdrop
  memset(&ppu->bgBuffers[0].pixel, 0, sizeof(ppu->bgBuffers[0].pixel));
  memset(&ppu->bgBuffers[0].prio, 0x05, sizeof(ppu->bgBuffers[0].prio));

  // Render main screen
  PpuDrawBackgrounds(ppu, y, false);

  // The 6:th bit is automatically zero, math is never applied to the first half of the sprites.
  uint32 math_enabled = ppu->mathEnabled[0] << 0 | ppu->mathEnabled[1] << 1 | ppu->mathEnabled[2] << 2 |
                        ppu->mathEnabled[3] << 3 | ppu->mathEnabled[4] << 4 | ppu->mathEnabled[5] << 5;

  // Render also the subscreen?
  bool rendered_subscreen = false;
  if (ppu->preventMathMode != 3 && ppu->addSubscreen && math_enabled) {
    memset(&ppu->bgBuffers[1].pixel, 0, sizeof(ppu->bgBuffers[1].pixel));
    if (ppu->layer[0].screenEnabled[1] | ppu->layer[1].screenEnabled[1] | ppu->layer[2].screenEnabled[1] |
        ppu->layer[3].screenEnabled[1] | ppu->layer[4].screenEnabled[1]) {
      memset(&ppu->bgBuffers[1].prio, 0x05, sizeof(ppu->bgBuffers[1].prio));
      PpuDrawBackgrounds(ppu, y, true);
      rendered_subscreen = true;
    }
  }

  // Color window affects the drawing mode in each region
  PpuWindows cwin;
  PpuWindows_Calc(&cwin, ppu, 5);
  static const uint8 kCwBitsMod[8] = {
    0x00, 0xff, 0xff, 0x00,
    0xff, 0x00, 0xff, 0x00,
  };
  uint32 cw_clip_math = ((cwin.bits & kCwBitsMod[ppu->clipMode]) ^ kCwBitsMod[ppu->clipMode + 4]) |
                        ((cwin.bits & kCwBitsMod[ppu->preventMathMode]) ^ kCwBitsMod[ppu->preventMathMode + 4]) << 8;

  uint32 *dst = (uint32*)&ppu->renderBuffer[(y - 1) * 2 * ppu->renderPitch];
  
  uint32 windex = 0;
  do {
    uint32 left = cwin.edges[windex], right = cwin.edges[windex + 1];
    // If clip is set, then zero out the rgb values from the main screen.
    uint32 clip_color_mask = (cw_clip_math & 1) ? 0x1f : 0;
    uint32 math_enabled_cur = (cw_clip_math & 0x100) ? math_enabled : 0;
    uint32 fixed_color = ppu->fixedColorR | ppu->fixedColorG << 5 | ppu->fixedColorB << 10;
    if (math_enabled_cur == 0 || fixed_color == 0 && !ppu->halfColor && !rendered_subscreen) {
      // Math is disabled (or has no effect), so can avoid the per-pixel maths check
      uint32 i = left;
      do {
        uint32 color = ppu->cgram[ppu->bgBuffers[0].pixel[i]];
        dst[1] = dst[0] = ppu->brightnessMult[color & clip_color_mask] << 16 |
                          ppu->brightnessMult[(color >> 5) & clip_color_mask] << 8 |
                          ppu->brightnessMult[(color >> 10) & clip_color_mask];
      } while (dst += 2, ++i < right);
    } else {
      uint8 *half_color_map = ppu->halfColor ? ppu->brightnessMultHalf : ppu->brightnessMult;
      // Store this in locals
      math_enabled_cur |= ppu->addSubscreen << 8 | ppu->subtractColor << 9;
      // Need to check for each pixel whether to use math or not based on the main screen layer.
      uint32 i = left;
      do {
        uint32 color = ppu->cgram[ppu->bgBuffers[0].pixel[i]], color2;
        uint8 main_layer = ppu->bgBuffers[0].prio[i] & 0xf;
        uint32 r = color & clip_color_mask;
        uint32 g = (color >> 5) & clip_color_mask;
        uint32 b = (color >> 10) & clip_color_mask;
        uint8 *color_map = ppu->brightnessMult;
        if (math_enabled_cur & (1 << main_layer)) {
          if (math_enabled_cur & 0x100) {  // addSubscreen ?
            if (ppu->bgBuffers[1].pixel[i] != 0)
              color2 = ppu->cgram[ppu->bgBuffers[1].pixel[i]], color_map = half_color_map;
            else  // Don't halve if ppu->addSubscreen && backdrop
              color2 = fixed_color;
          } else {
            color2 = fixed_color, color_map = half_color_map;
          }
          uint32 r2 = (color2 & 0x1f), g2 = ((color2 >> 5) & 0x1f), b2 = ((color2 >> 10) & 0x1f);
          if (math_enabled_cur & 0x200) {  // subtractColor?
            r = (r >= r2) ? r - r2 : 0;
            g = (g >= g2) ? g - g2 : 0;
            b = (b >= b2) ? b - b2 : 0;
          } else {
            r += r2;
            g += g2;
            b += b2;
          }
        }
        dst[0] = dst[1] = color_map[b] | color_map[g] << 8 | color_map[r] << 16;
      } while (dst += 2, ++i < right);
    }
  } while (cw_clip_math >>= 1, ++windex < cwin.nr);

  // Duplicate one line
  memcpy((uint8*)(dst - 512) + ppu->renderPitch, dst - 512, 512 * 4);

}

static void ppu_handlePixel(Ppu* ppu, int x, int y) {
  int r = 0, r2 = 0;
  int g = 0, g2 = 0;
  int b = 0, b2 = 0;
  if (!ppu->forcedBlank) {
    int mainLayer = ppu_getPixel(ppu, x, y, false, &r, &g, &b);

    bool colorWindowState = ppu_getWindowState(ppu, 5, x);
    if (
      ppu->clipMode == 3 ||
      (ppu->clipMode == 2 && colorWindowState) ||
      (ppu->clipMode == 1 && !colorWindowState)
      ) {
      r = g = b = 0;
    }
    int secondLayer = 5; // backdrop
    bool mathEnabled = mainLayer < 6 && ppu->mathEnabled[mainLayer] && !(
      ppu->preventMathMode == 3 ||
      (ppu->preventMathMode == 2 && colorWindowState) ||
      (ppu->preventMathMode == 1 && !colorWindowState)
      );
    if ((mathEnabled && ppu->addSubscreen) || ppu->pseudoHires_always_zero || ppu->mode == 5 || ppu->mode == 6) {
      secondLayer = ppu_getPixel(ppu, x, y, true, &r2, &g2, &b2);
    }
    // TODO: subscreen pixels can be clipped to black as well
    // TODO: math for subscreen pixels (add/sub sub to main)
    if (mathEnabled) {
      if (ppu->subtractColor) {
        r -= (ppu->addSubscreen && secondLayer != 5) ? r2 : ppu->fixedColorR;
        g -= (ppu->addSubscreen && secondLayer != 5) ? g2 : ppu->fixedColorG;
        b -= (ppu->addSubscreen && secondLayer != 5) ? b2 : ppu->fixedColorB;
      } else {
        r += (ppu->addSubscreen && secondLayer != 5) ? r2 : ppu->fixedColorR;
        g += (ppu->addSubscreen && secondLayer != 5) ? g2 : ppu->fixedColorG;
        b += (ppu->addSubscreen && secondLayer != 5) ? b2 : ppu->fixedColorB;
      }
      if (ppu->halfColor && (secondLayer != 5 || !ppu->addSubscreen)) {
        r >>= 1;
        g >>= 1;
        b >>= 1;
      }
      if (r > 31) r = 31;
      if (g > 31) g = 31;
      if (b > 31) b = 31;
      if (r < 0) r = 0;
      if (g < 0) g = 0;
      if (b < 0) b = 0;
    }
    if (!(ppu->pseudoHires_always_zero || ppu->mode == 5 || ppu->mode == 6)) {
      r2 = r; g2 = g; b2 = b;
    }
  }
  int row = y - 1;
  uint8 *pixelBuffer = (uint8*) &ppu->renderBuffer[row * 2 * ppu->renderPitch + x * 8];
  pixelBuffer[0] = ((b2 << 3) | (b2 >> 2)) * ppu->brightness / 15;
  pixelBuffer[1] = ((g2 << 3) | (g2 >> 2)) * ppu->brightness / 15;
  pixelBuffer[2] = ((r2 << 3) | (r2 >> 2)) * ppu->brightness / 15;
  pixelBuffer[3] = 0;
  pixelBuffer[4] = ((b << 3) | (b >> 2)) * ppu->brightness / 15;
  pixelBuffer[5] = ((g << 3) | (g >> 2)) * ppu->brightness / 15;
  pixelBuffer[6] = ((r << 3) | (r >> 2)) * ppu->brightness / 15;
  pixelBuffer[7] = 0;
}

static const int bitDepthsPerMode[10][4] = {
  {2, 2, 2, 2},
  {4, 4, 2, 5},
  {4, 4, 5, 5},
  {8, 4, 5, 5},
  {8, 2, 5, 5},
  {4, 2, 5, 5},
  {4, 5, 5, 5},
  {8, 5, 5, 5},
  {4, 4, 2, 5},
  {8, 7, 5, 5}
};

static int ppu_getPixel(Ppu *ppu, int x, int y, bool sub, int *r, int *g, int *b) {
  // array for layer definitions per mode:
//   0-7: mode 0-7; 8: mode 1 + l3prio; 9: mode 7 + extbg

//   0-3; layers 1-4; 4: sprites; 5: nonexistent
  static const int layersPerMode[10][12] = {
    {4, 0, 1, 4, 0, 1, 4, 2, 3, 4, 2, 3},
    {4, 0, 1, 4, 0, 1, 4, 2, 4, 2, 5, 5},
    {4, 0, 4, 1, 4, 0, 4, 1, 5, 5, 5, 5},
    {4, 0, 4, 1, 4, 0, 4, 1, 5, 5, 5, 5},
    {4, 0, 4, 1, 4, 0, 4, 1, 5, 5, 5, 5},
    {4, 0, 4, 1, 4, 0, 4, 1, 5, 5, 5, 5},
    {4, 0, 4, 4, 0, 4, 5, 5, 5, 5, 5, 5},
    {4, 4, 4, 0, 4, 5, 5, 5, 5, 5, 5, 5},
    {2, 4, 0, 1, 4, 0, 1, 4, 4, 2, 5, 5},
    {4, 4, 1, 4, 0, 4, 1, 5, 5, 5, 5, 5}
  };

  static const int prioritysPerMode[10][12] = {
    {3, 1, 1, 2, 0, 0, 1, 1, 1, 0, 0, 0},
    {3, 1, 1, 2, 0, 0, 1, 1, 0, 0, 5, 5},
    {3, 1, 2, 1, 1, 0, 0, 0, 5, 5, 5, 5},
    {3, 1, 2, 1, 1, 0, 0, 0, 5, 5, 5, 5},
    {3, 1, 2, 1, 1, 0, 0, 0, 5, 5, 5, 5},
    {3, 1, 2, 1, 1, 0, 0, 0, 5, 5, 5, 5},
    {3, 1, 2, 1, 0, 0, 5, 5, 5, 5, 5, 5},
    {3, 2, 1, 0, 0, 5, 5, 5, 5, 5, 5, 5},
    {1, 3, 1, 1, 2, 0, 0, 1, 0, 0, 5, 5},
    {3, 2, 1, 1, 0, 0, 0, 5, 5, 5, 5, 5}
  };

  static const int layerCountPerMode[10] = {
    12, 10, 8, 8, 8, 8, 6, 5, 10, 7
  };

  
  // figure out which color is on this location on main- or subscreen, sets it in r, g, b
  // returns which layer it is: 0-3 for bg layer, 4 or 6 for sprites (depending on palette), 5 for backdrop
  int actMode = ppu->mode == 1 && ppu->bg3priority ? 8 : ppu->mode;
  actMode = ppu->mode == 7 && ppu->m7extBg_always_zero ? 9 : actMode;
  int layer = 5;
  int pixel = 0;
  for (int i = 0; i < layerCountPerMode[actMode]; i++) {
    int curLayer = layersPerMode[actMode][i];
    int curPriority = prioritysPerMode[actMode][i];
    bool layerActive = false;
    if (!sub) {
      layerActive = ppu->layer[curLayer].screenEnabled[0] && (
        !ppu->layer[curLayer].screenWindowed[0] || !ppu_getWindowState(ppu, curLayer, x)
        );
    } else {
      layerActive = ppu->layer[curLayer].screenEnabled[1] && (
        !ppu->layer[curLayer].screenWindowed[1] || !ppu_getWindowState(ppu, curLayer, x)
        );
    }
    if (layerActive) {
      if (curLayer < 4) {
        // bg layer
        int lx = x;
        int ly = y;
        if (ppu->bgLayer[curLayer].mosaicEnabled && ppu->mosaicSize > 1) {
          lx -= lx % ppu->mosaicSize;
          ly -= (ly - ppu->mosaicStartLine) % ppu->mosaicSize;
        }
        if (ppu->mode == 7) {
          pixel = ppu_getPixelForMode7(ppu, lx, curLayer, curPriority);
        } else {
          lx += ppu->bgLayer[curLayer].hScroll;
          if (ppu->mode == 5 || ppu->mode == 6) {
            lx *= 2;
            lx += (sub || ppu->bgLayer[curLayer].mosaicEnabled) ? 0 : 1;
            if (ppu->interlace_always_zero) {
              ly *= 2;
              ly += (ppu->evenFrame || ppu->bgLayer[curLayer].mosaicEnabled) ? 0 : 1;
            }
          }
          ly += ppu->bgLayer[curLayer].vScroll;
          pixel = ppu_getPixelForBgLayer(
            ppu, lx & 0x3ff, ly & 0x3ff,
            curLayer, curPriority
          );
        }
      } else {
        // get a pixel from the sprite buffer
        pixel = 0;
        if ((ppu->objBuffer.prio[x] >> 4) == SPRITE_PRIO_TO_PRIO_HI(curPriority)) pixel = ppu->objBuffer.pixel[x];
      }
    }
    if (pixel > 0) {
      layer = curLayer;
      break;
    }
  }
  if (ppu->directColor_always_zero && layer < 4 && bitDepthsPerMode[actMode][layer] == 8) {
    *r = ((pixel & 0x7) << 2) | ((pixel & 0x100) >> 7);
    *g = ((pixel & 0x38) >> 1) | ((pixel & 0x200) >> 8);
    *b = ((pixel & 0xc0) >> 3) | ((pixel & 0x400) >> 8);
  } else {
    uint16_t color = ppu->cgram[pixel & 0xff];
    *r = color & 0x1f;
    *g = (color >> 5) & 0x1f;
    *b = (color >> 10) & 0x1f;
  }
  if (layer == 4 && pixel < 0xc0) layer = 6; // sprites with palette color < 0xc0
  return layer;

}


static int ppu_getPixelForBgLayer(Ppu *ppu, int x, int y, int layer, bool priority) {
  BgLayer *layerp = &ppu->bgLayer[layer];
  // figure out address of tilemap word and read it
  bool wideTiles = layerp->bigTiles_always_zero || ppu->mode == 5 || ppu->mode == 6;
  int tileBitsX = wideTiles ? 4 : 3;
  int tileHighBitX = wideTiles ? 0x200 : 0x100;
  int tileBitsY = layerp->bigTiles_always_zero ? 4 : 3;
  int tileHighBitY = layerp->bigTiles_always_zero ? 0x200 : 0x100;
  uint16_t tilemapAdr = layerp->tilemapAdr + (((y >> tileBitsY) & 0x1f) << 5 | ((x >> tileBitsX) & 0x1f));
  if ((x & tileHighBitX) && layerp->tilemapWider) tilemapAdr += 0x400;
  if ((y & tileHighBitY) && layerp->tilemapHigher) tilemapAdr += layerp->tilemapWider ? 0x800 : 0x400;
  uint16_t tile = ppu->vram[tilemapAdr & 0x7fff];
  // check priority, get palette
  if (((bool)(tile & 0x2000)) != priority) return 0; // wrong priority
  int paletteNum = (tile & 0x1c00) >> 10;
  // figure out position within tile
  int row = (tile & 0x8000) ? 7 - (y & 0x7) : (y & 0x7);
  int col = (tile & 0x4000) ? (x & 0x7) : 7 - (x & 0x7);
  int tileNum = tile & 0x3ff;
  if (wideTiles) {
    // if unflipped right half of tile, or flipped left half of tile
    if (((bool)(x & 8)) ^ ((bool)(tile & 0x4000))) tileNum += 1;
  }
  if (layerp->bigTiles_always_zero) {
    // if unflipped bottom half of tile, or flipped upper half of tile
    if (((bool)(y & 8)) ^ ((bool)(tile & 0x8000))) tileNum += 0x10;
  }
  // read tiledata, ajust palette for mode 0
  int bitDepth = bitDepthsPerMode[ppu->mode][layer];
  if (ppu->mode == 0) paletteNum += 8 * layer;
  // plane 1 (always)
  int paletteSize = 4;
  uint16_t plane1 = ppu->vram[(layerp->tileAdr + ((tileNum & 0x3ff) * 4 * bitDepth) + row) & 0x7fff];
  int pixel = (plane1 >> col) & 1;
  pixel |= ((plane1 >> (8 + col)) & 1) << 1;
  // plane 2 (for 4bpp, 8bpp)
  if (bitDepth > 2) {
    paletteSize = 16;
    uint16_t plane2 = ppu->vram[(layerp->tileAdr + ((tileNum & 0x3ff) * 4 * bitDepth) + 8 + row) & 0x7fff];
    pixel |= ((plane2 >> col) & 1) << 2;
    pixel |= ((plane2 >> (8 + col)) & 1) << 3;
  }
  // plane 3 & 4 (for 8bpp)
  if (bitDepth > 4) {
    paletteSize = 256;
    uint16_t plane3 = ppu->vram[(layerp->tileAdr + ((tileNum & 0x3ff) * 4 * bitDepth) + 16 + row) & 0x7fff];
    pixel |= ((plane3 >> col) & 1) << 4;
    pixel |= ((plane3 >> (8 + col)) & 1) << 5;
    uint16_t plane4 = ppu->vram[(layerp->tileAdr + ((tileNum & 0x3ff) * 4 * bitDepth) + 24 + row) & 0x7fff];
    pixel |= ((plane4 >> col) & 1) << 6;
    pixel |= ((plane4 >> (8 + col)) & 1) << 7;
  }
  // return cgram index, or 0 if transparent, palette number in bits 10-8 for 8-color layers
  return pixel == 0 ? 0 : paletteSize * paletteNum + pixel;
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

static bool ppu_getWindowState(Ppu* ppu, int layer, int x) {
  if (!ppu->windowLayer[layer].window1enabled && !ppu->windowLayer[layer].window2enabled) {
    return false;
  }
  if (ppu->windowLayer[layer].window1enabled && !ppu->windowLayer[layer].window2enabled) {
    bool test = x >= ppu->window1left && x <= ppu->window1right;
    return ppu->windowLayer[layer].window1inversed ? !test : test;
  }
  if (!ppu->windowLayer[layer].window1enabled && ppu->windowLayer[layer].window2enabled) {
    bool test = x >= ppu->window2left && x <= ppu->window2right;
    return ppu->windowLayer[layer].window2inversed ? !test : test;
  }
  bool test1 = x >= ppu->window1left && x <= ppu->window1right;
  bool test2 = x >= ppu->window2left && x <= ppu->window2right;
  if (ppu->windowLayer[layer].window1inversed) test1 = !test1;
  if (ppu->windowLayer[layer].window2inversed) test2 = !test2;
  switch (ppu->windowLayer[layer].maskLogic_always_zero) {
  case 0: return test1 || test2;
  case 1: return test1 && test2;
  case 2: return test1 != test2;
  case 3: return test1 == test2;
  }
  return false;
}

static bool ppu_evaluateSprites(Ppu* ppu, int line) {
  // TODO: iterate over oam normally to determine in-range sprites,
  //   then iterate those in-range sprites in reverse for tile-fetching
  // TODO: rectangular sprites, wierdness with sprites at -256
  int index = ppu->objPriority ? (ppu->oamAdr & 0xfe) : 0, index_end = index;
  int spritesFound = 0, tilesFound = 0;
  uint8 spriteSizes[2] = { kSpriteSizes[ppu->objSize][0], kSpriteSizes[ppu->objSize][1] };
  do {
    int yy = ppu->oam[index] >> 8;
    if (yy == 0xf0)
      continue;  // this works for zelda because sprites are always 8 or 16.
    // check if the sprite is on this line and get the sprite size
    int row = (line - yy) & 0xff;
    int highOam = ppu->highOam[index >> 3] >> (index & 7);
    int spriteSize = spriteSizes[(highOam >> 1) & 1];
    if (row >= spriteSize)
      continue;
    // in y-range, get the x location, using the high bit as well
    int x = (ppu->oam[index] & 0xff) - (highOam & 1) * 256;
    // if in x-range
    if (x <= -spriteSize)
      continue;
    // break if we found 32 sprites already
    if (++spritesFound > 32) {
      ppu->rangeOver = true;
      break;
    }
    // get some data for the sprite and y-flip row if needed
    int oam1 = ppu->oam[index + 1];
    int objAdr = (oam1 & 0x100) ? ppu->objTileAdr2 : ppu->objTileAdr1;
    if (oam1 & 0x8000)
      row = spriteSize - 1 - row;
    // fetch all tiles in x-range
    uint8 paletteBase = 0x80 + 16 * ((oam1 & 0xe00) >> 9);
    uint8 prio = SPRITE_PRIO_TO_PRIO((oam1 & 0x3000) >> 12, (oam1 & 0x800) == 0);
    for (int col = 0; col < spriteSize; col += 8) {
      if (col + x > -8 && col + x < 256) {
        // break if we found 34 8*1 slivers already
        if (++tilesFound > 34) {
          ppu->timeOver = true;
          return true;
        }
        // figure out which tile this uses, looping within 16x16 pages, and get it's data
        int usedCol = oam1 & 0x4000 ? spriteSize - 1 - col : col;
        int usedTile = ((((oam1 & 0xff) >> 4) + (row >> 3)) << 4) | (((oam1 & 0xf) + (usedCol >> 3)) & 0xf);
        uint16 *addr = &ppu->vram[(objAdr + usedTile * 16 + (row & 0x7)) & 0x7fff];
        uint32 plane = addr[0] | addr[8] << 16;
        // go over each pixel
        int px_left = IntMax(-(col + x), 0);
        int px_right = IntMin(256 - (col + x), 8);
        uint8 *dst = ppu->objBuffer.pixel + col + x + px_left;
        
        for (int px = px_left; px < px_right; px++, dst++) {
          int shift = oam1 & 0x4000 ? px : 7 - px;
          uint32 bits = plane >> shift;
          int pixel = (bits >> 0) & 1 | (bits >> 7) & 2 | (bits >> 14) & 4 | (bits >> 21) & 8;
          // draw it in the buffer if there is a pixel here, and the buffer there is still empty
          if (pixel != 0 && dst[0] == 0)
            dst[0] = paletteBase + pixel, dst[256] = prio;
        }
      }
    }
  } while ((index = (index + 2) & 0xff) != index_end);
  return (tilesFound != 0);
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

