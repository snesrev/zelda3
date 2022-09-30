
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <limits.h>
#include "dsp_regs.h"
#include "dsp.h"

#define MY_CHANGES 1

static const int rateValues[32] = {
  0, 2048, 1536, 1280, 1024, 768, 640, 512,
  384, 320, 256, 192, 160, 128, 96, 80,
  64, 48, 40, 32, 24, 20, 16, 12,
  10, 8, 6, 5, 4, 3, 2, 1
};

static const int gaussValues[512] = {
  0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
  0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x002, 0x002, 0x002, 0x002, 0x002,
  0x002, 0x002, 0x003, 0x003, 0x003, 0x003, 0x003, 0x004, 0x004, 0x004, 0x004, 0x004, 0x005, 0x005, 0x005, 0x005,
  0x006, 0x006, 0x006, 0x006, 0x007, 0x007, 0x007, 0x008, 0x008, 0x008, 0x009, 0x009, 0x009, 0x00A, 0x00A, 0x00A,
  0x00B, 0x00B, 0x00B, 0x00C, 0x00C, 0x00D, 0x00D, 0x00E, 0x00E, 0x00F, 0x00F, 0x00F, 0x010, 0x010, 0x011, 0x011,
  0x012, 0x013, 0x013, 0x014, 0x014, 0x015, 0x015, 0x016, 0x017, 0x017, 0x018, 0x018, 0x019, 0x01A, 0x01B, 0x01B,
  0x01C, 0x01D, 0x01D, 0x01E, 0x01F, 0x020, 0x020, 0x021, 0x022, 0x023, 0x024, 0x024, 0x025, 0x026, 0x027, 0x028,
  0x029, 0x02A, 0x02B, 0x02C, 0x02D, 0x02E, 0x02F, 0x030, 0x031, 0x032, 0x033, 0x034, 0x035, 0x036, 0x037, 0x038,
  0x03A, 0x03B, 0x03C, 0x03D, 0x03E, 0x040, 0x041, 0x042, 0x043, 0x045, 0x046, 0x047, 0x049, 0x04A, 0x04C, 0x04D,
  0x04E, 0x050, 0x051, 0x053, 0x054, 0x056, 0x057, 0x059, 0x05A, 0x05C, 0x05E, 0x05F, 0x061, 0x063, 0x064, 0x066,
  0x068, 0x06A, 0x06B, 0x06D, 0x06F, 0x071, 0x073, 0x075, 0x076, 0x078, 0x07A, 0x07C, 0x07E, 0x080, 0x082, 0x084,
  0x086, 0x089, 0x08B, 0x08D, 0x08F, 0x091, 0x093, 0x096, 0x098, 0x09A, 0x09C, 0x09F, 0x0A1, 0x0A3, 0x0A6, 0x0A8,
  0x0AB, 0x0AD, 0x0AF, 0x0B2, 0x0B4, 0x0B7, 0x0BA, 0x0BC, 0x0BF, 0x0C1, 0x0C4, 0x0C7, 0x0C9, 0x0CC, 0x0CF, 0x0D2,
  0x0D4, 0x0D7, 0x0DA, 0x0DD, 0x0E0, 0x0E3, 0x0E6, 0x0E9, 0x0EC, 0x0EF, 0x0F2, 0x0F5, 0x0F8, 0x0FB, 0x0FE, 0x101,
  0x104, 0x107, 0x10B, 0x10E, 0x111, 0x114, 0x118, 0x11B, 0x11E, 0x122, 0x125, 0x129, 0x12C, 0x130, 0x133, 0x137,
  0x13A, 0x13E, 0x141, 0x145, 0x148, 0x14C, 0x150, 0x153, 0x157, 0x15B, 0x15F, 0x162, 0x166, 0x16A, 0x16E, 0x172,
  0x176, 0x17A, 0x17D, 0x181, 0x185, 0x189, 0x18D, 0x191, 0x195, 0x19A, 0x19E, 0x1A2, 0x1A6, 0x1AA, 0x1AE, 0x1B2,
  0x1B7, 0x1BB, 0x1BF, 0x1C3, 0x1C8, 0x1CC, 0x1D0, 0x1D5, 0x1D9, 0x1DD, 0x1E2, 0x1E6, 0x1EB, 0x1EF, 0x1F3, 0x1F8,
  0x1FC, 0x201, 0x205, 0x20A, 0x20F, 0x213, 0x218, 0x21C, 0x221, 0x226, 0x22A, 0x22F, 0x233, 0x238, 0x23D, 0x241,
  0x246, 0x24B, 0x250, 0x254, 0x259, 0x25E, 0x263, 0x267, 0x26C, 0x271, 0x276, 0x27B, 0x280, 0x284, 0x289, 0x28E,
  0x293, 0x298, 0x29D, 0x2A2, 0x2A6, 0x2AB, 0x2B0, 0x2B5, 0x2BA, 0x2BF, 0x2C4, 0x2C9, 0x2CE, 0x2D3, 0x2D8, 0x2DC,
  0x2E1, 0x2E6, 0x2EB, 0x2F0, 0x2F5, 0x2FA, 0x2FF, 0x304, 0x309, 0x30E, 0x313, 0x318, 0x31D, 0x322, 0x326, 0x32B,
  0x330, 0x335, 0x33A, 0x33F, 0x344, 0x349, 0x34E, 0x353, 0x357, 0x35C, 0x361, 0x366, 0x36B, 0x370, 0x374, 0x379,
  0x37E, 0x383, 0x388, 0x38C, 0x391, 0x396, 0x39B, 0x39F, 0x3A4, 0x3A9, 0x3AD, 0x3B2, 0x3B7, 0x3BB, 0x3C0, 0x3C5,
  0x3C9, 0x3CE, 0x3D2, 0x3D7, 0x3DC, 0x3E0, 0x3E5, 0x3E9, 0x3ED, 0x3F2, 0x3F6, 0x3FB, 0x3FF, 0x403, 0x408, 0x40C,
  0x410, 0x415, 0x419, 0x41D, 0x421, 0x425, 0x42A, 0x42E, 0x432, 0x436, 0x43A, 0x43E, 0x442, 0x446, 0x44A, 0x44E,
  0x452, 0x455, 0x459, 0x45D, 0x461, 0x465, 0x468, 0x46C, 0x470, 0x473, 0x477, 0x47A, 0x47E, 0x481, 0x485, 0x488,
  0x48C, 0x48F, 0x492, 0x496, 0x499, 0x49C, 0x49F, 0x4A2, 0x4A6, 0x4A9, 0x4AC, 0x4AF, 0x4B2, 0x4B5, 0x4B7, 0x4BA,
  0x4BD, 0x4C0, 0x4C3, 0x4C5, 0x4C8, 0x4CB, 0x4CD, 0x4D0, 0x4D2, 0x4D5, 0x4D7, 0x4D9, 0x4DC, 0x4DE, 0x4E0, 0x4E3,
  0x4E5, 0x4E7, 0x4E9, 0x4EB, 0x4ED, 0x4EF, 0x4F1, 0x4F3, 0x4F5, 0x4F6, 0x4F8, 0x4FA, 0x4FB, 0x4FD, 0x4FF, 0x500,
  0x502, 0x503, 0x504, 0x506, 0x507, 0x508, 0x50A, 0x50B, 0x50C, 0x50D, 0x50E, 0x50F, 0x510, 0x511, 0x511, 0x512,
  0x513, 0x514, 0x514, 0x515, 0x516, 0x516, 0x517, 0x517, 0x517, 0x518, 0x518, 0x518, 0x518, 0x518, 0x519, 0x519
};

static void dsp_cycleChannel(Dsp* dsp, int ch);
static void dsp_handleEcho(Dsp* dsp, int* outputL, int* outputR);
static void dsp_handleGain(Dsp* dsp, int ch);
static void dsp_decodeBrr(Dsp* dsp, int ch);
static int16_t dsp_getSample(Dsp* dsp, int ch, int sampleNum, int offset);
static void dsp_handleNoise(Dsp* dsp);

Dsp* dsp_init(uint8_t *apu_ram) {
  Dsp* dsp = (Dsp*)malloc(sizeof(Dsp));
  dsp->apu_ram = apu_ram;
  return dsp;
}

void dsp_free(Dsp* dsp) {
  free(dsp);
}

void dsp_reset(Dsp* dsp) {
  memset(dsp->ram, 0, sizeof(dsp->ram));
  dsp->ram[ENDX] = 0xff; // set ENDX bit for all channels
  for(int i = 0; i < 8; i++) {
    dsp->channel[i].pitch = 0;
    dsp->channel[i].pitchCounter = 0;
    dsp->channel[i].pitchModulation = false;
    memset(dsp->channel[i].decodeBuffer, 0, sizeof(dsp->channel[i].decodeBuffer));
    dsp->channel[i].srcn = 0;
    dsp->channel[i].decodeOffset = 0;
    dsp->channel[i].previousFlags = 0;
    dsp->channel[i].old = 0;
    dsp->channel[i].older = 0;
    dsp->channel[i].useNoise = false;
    memset(dsp->channel[i].adsrRates, 0, sizeof(dsp->channel[i].adsrRates));
    dsp->channel[i].rateCounter = 0;
    dsp->channel[i].adsrState = 0;
    dsp->channel[i].sustainLevel = 0;
    dsp->channel[i].useGain = false;
    dsp->channel[i].gainMode = 0;
    dsp->channel[i].directGain = false;
    dsp->channel[i].gainValue = 0;
    dsp->channel[i].gain = 0;
    dsp->channel[i].keyOn = false;
    dsp->channel[i].keyOff = false;
    dsp->channel[i].sampleOut = 0;
    dsp->channel[i].volumeL = 0;
    dsp->channel[i].volumeR = 0;
    dsp->channel[i].echoEnable = false;
  }
  dsp->dirPage = 0;
  dsp->evenCycle = false;
  dsp->mute = true;
  dsp->reset = true;
  dsp->masterVolumeL = 0;
  dsp->masterVolumeR = 0;
  dsp->noiseSample = -0x4000;
  dsp->noiseRate = 0;
  dsp->noiseCounter = 0;
  dsp->echoWrites = false;
  dsp->echoVolumeL = 0;
  dsp->echoVolumeR = 0;
  dsp->feedbackVolume = 0;
  dsp->echoBufferAdr = 0;
  dsp->echoDelay = 1;
  dsp->echoRemain = 1;
  dsp->echoBufferIndex = 0;
  dsp->firBufferIndex = 0;
  memset(dsp->firValues, 0, sizeof(dsp->firValues));
  memset(dsp->firBufferL, 0, sizeof(dsp->firBufferL));
  memset(dsp->firBufferR, 0, sizeof(dsp->firBufferR));
  memset(dsp->sampleBuffer, 0, sizeof(dsp->sampleBuffer));
  dsp->sampleOffset = 0;
}

void dsp_saveload(Dsp *dsp, SaveLoadFunc *func, void *ctx) {
  func(ctx, &dsp->ram, sizeof(Dsp) - offsetof(Dsp, ram));
}

void dsp_cycle(Dsp* dsp) {
  int totalL = 0;
  int totalR = 0;
  for(int i = 0; i < 8; i++) {
    dsp_cycleChannel(dsp, i);
    totalL += (dsp->channel[i].sampleOut * dsp->channel[i].volumeL) >> 6;
    totalR += (dsp->channel[i].sampleOut * dsp->channel[i].volumeR) >> 6;
    totalL = totalL < -0x8000 ? -0x8000 : (totalL > 0x7fff ? 0x7fff : totalL); // clamp 16-bit
    totalR = totalR < -0x8000 ? -0x8000 : (totalR > 0x7fff ? 0x7fff : totalR); // clamp 16-bit
  }
  totalL = (totalL * dsp->masterVolumeL) >> 7;
  totalR = (totalR * dsp->masterVolumeR) >> 7;
  totalL = totalL < -0x8000 ? -0x8000 : (totalL > 0x7fff ? 0x7fff : totalL); // clamp 16-bit
  totalR = totalR < -0x8000 ? -0x8000 : (totalR > 0x7fff ? 0x7fff : totalR); // clamp 16-bit
  dsp_handleEcho(dsp, &totalL, &totalR);
  if(dsp->mute) {
    totalL = 0;
    totalR = 0;
  }
  dsp_handleNoise(dsp);
  // put it in the samplebuffer, if space
  if (dsp->sampleOffset < 534) {
    dsp->sampleBuffer[dsp->sampleOffset * 2] = totalL;
    dsp->sampleBuffer[dsp->sampleOffset * 2 + 1] = totalR;
    dsp->sampleOffset++;
  }
  dsp->evenCycle = !dsp->evenCycle;
}

static void dsp_handleEcho(Dsp* dsp, int* outputL, int* outputR) {
  // get value out of ram
  uint16_t adr = dsp->echoBufferAdr + dsp->echoBufferIndex * 4;
  dsp->firBufferL[dsp->firBufferIndex] = (
    dsp->apu_ram[adr] + (dsp->apu_ram[(adr + 1) & 0xffff] << 8)
  );
  dsp->firBufferL[dsp->firBufferIndex] >>= 1;
  dsp->firBufferR[dsp->firBufferIndex] = (
    dsp->apu_ram[(adr + 2) & 0xffff] + (dsp->apu_ram[(adr + 3) & 0xffff] << 8)
  );
  dsp->firBufferR[dsp->firBufferIndex] >>= 1;
  // calculate FIR-sum
  int sumL = 0, sumR = 0;
  for(int i = 0; i < 8; i++) {
    sumL += (dsp->firBufferL[(dsp->firBufferIndex + i + 1) & 0x7] * dsp->firValues[i]) >> 6;
    sumR += (dsp->firBufferR[(dsp->firBufferIndex + i + 1) & 0x7] * dsp->firValues[i]) >> 6;
    if(i == 6) {
      // clip to 16-bit before last addition
      sumL = ((int16_t) (sumL & 0xffff)); // clip 16-bit
      sumR = ((int16_t) (sumR & 0xffff)); // clip 16-bit
    }
  }
  sumL = sumL < -0x8000 ? -0x8000 : (sumL > 0x7fff ? 0x7fff : sumL); // clamp 16-bit
  sumR = sumR < -0x8000 ? -0x8000 : (sumR > 0x7fff ? 0x7fff : sumR); // clamp 16-bit
  // modify output with sum
  int outL = *outputL + ((sumL * dsp->echoVolumeL) >> 7);
  int outR = *outputR + ((sumR * dsp->echoVolumeR) >> 7);
  *outputL = outL < -0x8000 ? -0x8000 : (outL > 0x7fff ? 0x7fff : outL); // clamp 16-bit
  *outputR = outR < -0x8000 ? -0x8000 : (outR > 0x7fff ? 0x7fff : outR); // clamp 16-bit
  // get echo input
  int inL = 0, inR = 0;
  for(int i = 0; i < 8; i++) {
    if(dsp->channel[i].echoEnable) {
      inL += (dsp->channel[i].sampleOut * dsp->channel[i].volumeL) >> 6;
      inR += (dsp->channel[i].sampleOut * dsp->channel[i].volumeR) >> 6;
      inL = inL < -0x8000 ? -0x8000 : (inL > 0x7fff ? 0x7fff : inL); // clamp 16-bit
      inR = inR < -0x8000 ? -0x8000 : (inR > 0x7fff ? 0x7fff : inR); // clamp 16-bit
    }
  }
  // write this to ram
  inL += (sumL * dsp->feedbackVolume) >> 7;
  inR += (sumR * dsp->feedbackVolume) >> 7;
  inL = inL < -0x8000 ? -0x8000 : (inL > 0x7fff ? 0x7fff : inL); // clamp 16-bit
  inR = inR < -0x8000 ? -0x8000 : (inR > 0x7fff ? 0x7fff : inR); // clamp 16-bit
  inL &= 0xfffe;
  inR &= 0xfffe;
  if(dsp->echoWrites) {
    dsp->apu_ram[adr] = inL & 0xff;
    dsp->apu_ram[(adr + 1) & 0xffff] = inL >> 8;
    dsp->apu_ram[(adr + 2) & 0xffff] = inR & 0xff;
    dsp->apu_ram[(adr + 3) & 0xffff] = inR >> 8;
  }
  // handle indexes
  dsp->firBufferIndex++;
  dsp->firBufferIndex &= 7;
  dsp->echoBufferIndex++;
  dsp->echoRemain--;
  if(dsp->echoRemain == 0) {
    dsp->echoRemain = dsp->echoDelay;
    dsp->echoBufferIndex = 0;
  }
}

static void dsp_cycleChannel(Dsp* dsp, int ch) {
  // handle pitch counter
  uint16_t pitch = dsp->channel[ch].pitch;
  if(ch > 0 && dsp->channel[ch].pitchModulation) {
    int factor = (dsp->channel[ch - 1].sampleOut >> 4) + 0x400;
    pitch = (pitch * factor) >> 10;
    if(pitch > 0x3fff) pitch = 0x3fff;
  }
  int newCounter = dsp->channel[ch].pitchCounter + pitch;
  if(newCounter > 0xffff) {
    // next sample
    dsp_decodeBrr(dsp, ch);
  }
  dsp->channel[ch].pitchCounter = newCounter;
  int16_t sample = 0;
  if(dsp->channel[ch].useNoise) {
    sample = dsp->noiseSample;
  } else {
    sample = dsp_getSample(dsp, ch, dsp->channel[ch].pitchCounter >> 12, (dsp->channel[ch].pitchCounter >> 4) & 0xff);
  }
#if !MY_CHANGES
  if(dsp->evenCycle) {
    // handle keyon/off (every other cycle)
    if(dsp->channel[ch].keyOff) {
      // go to release
      dsp->channel[ch].adsrState = 4;
    } else if(dsp->channel[ch].keyOn) {
      dsp->channel[ch].keyOn = false;
      // restart current sample
      dsp->channel[ch].previousFlags = 0;
      uint16_t samplePointer = dsp->dirPage + 4 * dsp->channel[ch].srcn;
      dsp->channel[ch].decodeOffset = dsp->apu_ram[samplePointer];
      dsp->channel[ch].decodeOffset |= dsp->apu_ram[(samplePointer + 1) & 0xffff] << 8;
      memset(dsp->channel[ch].decodeBuffer, 0, sizeof(dsp->channel[ch].decodeBuffer));
      dsp->channel[ch].gain = 0;
      dsp->channel[ch].adsrState = dsp->channel[ch].useGain ? 3 : 0;
    }
  }
#endif
  // handle reset
  if(dsp->reset) {
    dsp->channel[ch].adsrState = 4;
    dsp->channel[ch].gain = 0;
  }
  // handle envelope/adsr
  bool doingDirectGain = dsp->channel[ch].adsrState != 4 && dsp->channel[ch].useGain && dsp->channel[ch].directGain;
  uint16_t rate = dsp->channel[ch].adsrState == 4 ? 0 : dsp->channel[ch].adsrRates[dsp->channel[ch].adsrState];
  if(dsp->channel[ch].adsrState != 4 && !doingDirectGain && rate != 0) {
    dsp->channel[ch].rateCounter++;
  }
  if(dsp->channel[ch].adsrState == 4 || (!doingDirectGain && dsp->channel[ch].rateCounter >= rate && rate != 0)) {
    if(dsp->channel[ch].adsrState != 4) dsp->channel[ch].rateCounter = 0;
    dsp_handleGain(dsp, ch);
  }
  if(doingDirectGain) dsp->channel[ch].gain = dsp->channel[ch].gainValue;
  // set outputs
  dsp->ram[(ch << 4) | 8] = dsp->channel[ch].gain >> 4;
  sample = (sample * dsp->channel[ch].gain) >> 11;
  dsp->ram[(ch << 4) | 9] = sample >> 7;
  dsp->channel[ch].sampleOut = sample;
}

static void dsp_handleGain(Dsp* dsp, int ch) {
  switch(dsp->channel[ch].adsrState) {
    case 0: { // attack
      uint16_t rate = dsp->channel[ch].adsrRates[dsp->channel[ch].adsrState];
      dsp->channel[ch].gain += rate == 1 ? 1024 : 32;
      if(dsp->channel[ch].gain >= 0x7e0) dsp->channel[ch].adsrState = 1;
      if(dsp->channel[ch].gain > 0x7ff) dsp->channel[ch].gain = 0x7ff;
      break;
    }
    case 1: { // decay
      dsp->channel[ch].gain -= ((dsp->channel[ch].gain - 1) >> 8) + 1;
      if(dsp->channel[ch].gain < dsp->channel[ch].sustainLevel) dsp->channel[ch].adsrState = 2;
      break;
    }
    case 2: { // sustain
      dsp->channel[ch].gain -= ((dsp->channel[ch].gain - 1) >> 8) + 1;
      break;
    }
    case 3: { // gain
      switch(dsp->channel[ch].gainMode) {
        case 0: { // linear decrease
          dsp->channel[ch].gain -= 32;
          // decreasing below 0 will underflow to above 0x7ff
          if(dsp->channel[ch].gain > 0x7ff) dsp->channel[ch].gain = 0;
          break;
        }
        case 1: { // exponential decrease
          dsp->channel[ch].gain -= ((dsp->channel[ch].gain - 1) >> 8) + 1;
          break;
        }
        case 2: { // linear increase
          dsp->channel[ch].gain += 32;
          if(dsp->channel[ch].gain > 0x7ff) dsp->channel[ch].gain = 0;
          break;
        }
        case 3: { // bent increase
          dsp->channel[ch].gain += dsp->channel[ch].gain < 0x600 ? 32 : 8;
          if(dsp->channel[ch].gain > 0x7ff) dsp->channel[ch].gain = 0;
          break;
        }
      }
      break;
    }
    case 4: { // release
      dsp->channel[ch].gain -= 8;
      // decreasing below 0 will underflow to above 0x7ff
      if(dsp->channel[ch].gain > 0x7ff) dsp->channel[ch].gain = 0;
      break;
    }
  }
}

static int16_t dsp_getSample(Dsp* dsp, int ch, int sampleNum, int offset) {
  int16_t news = dsp->channel[ch].decodeBuffer[sampleNum + 3];
  int16_t olds = dsp->channel[ch].decodeBuffer[sampleNum + 2];
  int16_t olders = dsp->channel[ch].decodeBuffer[sampleNum + 1];
  int16_t oldests = dsp->channel[ch].decodeBuffer[sampleNum];
  int out = (gaussValues[0xff - offset] * oldests) >> 10;
  out += (gaussValues[0x1ff - offset] * olders) >> 10;
  out += (gaussValues[0x100 + offset] * olds) >> 10;
  out = ((int16_t) (out & 0xffff)); // clip 16-bit
  out += (gaussValues[offset] * news) >> 10;
  out = out < -0x8000 ? -0x8000 : (out > 0x7fff ? 0x7fff : out); // clamp 16-bit
  return out >> 1;
}

static void dsp_decodeBrr(Dsp* dsp, int ch) {
  // copy last 3 samples (16-18) to first 3 for interpolation
  dsp->channel[ch].decodeBuffer[0] = dsp->channel[ch].decodeBuffer[16];
  dsp->channel[ch].decodeBuffer[1] = dsp->channel[ch].decodeBuffer[17];
  dsp->channel[ch].decodeBuffer[2] = dsp->channel[ch].decodeBuffer[18];
  // handle flags from previous block
  if(dsp->channel[ch].previousFlags == 1 || dsp->channel[ch].previousFlags == 3) {
    // loop sample
    uint16_t samplePointer = dsp->dirPage + 4 * dsp->channel[ch].srcn;
    dsp->channel[ch].decodeOffset = dsp->apu_ram[(samplePointer + 2) & 0xffff];
    dsp->channel[ch].decodeOffset |= (dsp->apu_ram[(samplePointer + 3) & 0xffff]) << 8;
    if(dsp->channel[ch].previousFlags == 1) {
      // also release and clear gain
      dsp->channel[ch].adsrState = 4;
      dsp->channel[ch].gain = 0;
    }
    dsp->ram[ENDX] |= 1 << ch; // set ENDX bit for channel
  }
  uint8_t header = dsp->apu_ram[dsp->channel[ch].decodeOffset++];
  int shift = header >> 4;
  int filter = (header & 0xc) >> 2;
  dsp->channel[ch].previousFlags = header & 0x3;
  uint8_t curByte = 0;
  int old = dsp->channel[ch].old;
  int older = dsp->channel[ch].older;
  for(int i = 0; i < 16; i++) {
    int s = 0;
    if(i & 1) {
      s = curByte & 0xf;
    } else {
      curByte = dsp->apu_ram[dsp->channel[ch].decodeOffset++];
      s = curByte >> 4;
    }
    if(s > 7) s -= 16;
    if(shift <= 0xc) {
      s = (s << shift) >> 1;
    } else {
      s = (s >> 3) << 12;
    }
    switch(filter) {
      case 1: s += old + (-old >> 4); break;
      case 2: s += 2 * old + ((3 * -old) >> 5) - older + (older >> 4); break;
      case 3: s += 2 * old + ((13 * -old) >> 6) - older + ((3 * older) >> 4); break;
    }
    s = s < -0x8000 ? -0x8000 : (s > 0x7fff ? 0x7fff : s); // clamp 16-bit
    s = ((int16_t) ((s & 0x7fff) << 1)) >> 1; // clip 15-bit
    older = old;
    old = s;
    dsp->channel[ch].decodeBuffer[i + 3] = s;
  }
  dsp->channel[ch].older = older;
  dsp->channel[ch].old = old;
}

static void dsp_handleNoise(Dsp* dsp) {
  if(dsp->noiseRate != 0) {
    dsp->noiseCounter++;
  }
  if(dsp->noiseCounter >= dsp->noiseRate && dsp->noiseRate != 0) {
    int bit = (dsp->noiseSample & 1) ^ ((dsp->noiseSample >> 1) & 1);
    dsp->noiseSample = ((dsp->noiseSample >> 1) & 0x3fff) | (bit << 14);
    dsp->noiseSample = ((int16_t) ((dsp->noiseSample & 0x7fff) << 1)) >> 1;
    dsp->noiseCounter = 0;
  }
}

uint8_t dsp_read(Dsp* dsp, uint8_t adr) {
  return dsp->ram[adr];
}

void dsp_write(Dsp *dsp, uint8_t adr, uint8_t val) {
  int ch = adr >> 4;
  switch (adr) {
  case V0VOLL:
  case V1VOLL:
  case V2VOLL:
  case V3VOLL:
  case V4VOLL:
  case V5VOLL:
  case V6VOLL:
  case V7VOLL: {
    dsp->channel[ch].volumeL = val;
    break;
  }
  case V0VOLR:
  case V1VOLR:
  case V2VOLR:
  case V3VOLR:
  case V4VOLR:
  case V5VOLR:
  case V6VOLR:
  case V7VOLR: {
    dsp->channel[ch].volumeR = val;
    break;
  }
  case V0PITCHL:
  case V1PL:
  case V2PL:
  case V3PL:
  case V4PL:
  case V5PL:
  case V6PL:
  case V7PL: {
    dsp->channel[ch].pitch = (dsp->channel[ch].pitch & 0x3f00) | val;
    break;
  }
  case V0PITCHH:
  case V1PH:
  case V2PH:
  case V3PH:
  case V4PH:
  case V5PH:
  case V6PH:
  case V7PH: {
    dsp->channel[ch].pitch =
        ((dsp->channel[ch].pitch & 0x00ff) | (val << 8)) & 0x3fff;
    break;
  }
  case V0SRCN:
  case V1SRCN:
  case V2SRCN:
  case V3SRCN:
  case V4SRCN:
  case V5SRCN:
  case V6SRCN:
  case V7SRCN: {
    dsp->channel[ch].srcn = val;
    break;
  }
  case V0ADSR1:
  case V1ADSR1:
  case V2ADSR1:
  case V3ADSR1:
  case V4ADSR1:
  case V5ADSR1:
  case V6ADSR1:
  case V7ADSR1: {
    dsp->channel[ch].adsrRates[0] = rateValues[(val & 0xf) * 2 + 1];
    dsp->channel[ch].adsrRates[1] = rateValues[((val & 0x70) >> 4) * 2 + 16];
    dsp->channel[ch].useGain = (val & 0x80) == 0;
    break;
  }
  case V0ADSR2:
  case V1ADSR2:
  case V2ADSR2:
  case V3ADSR2:
  case V4ADSR2:
  case V5ADSR2:
  case V6ADSR2:
  case V7ADSR2: {
    dsp->channel[ch].adsrRates[2] = rateValues[val & 0x1f];
    dsp->channel[ch].sustainLevel = (((val & 0xe0) >> 5) + 1) * 0x100;
    break;
  }
  case V0GAIN:
  case V1GAIN:
  case V2GAIN:
  case V3GAIN:
  case V4GAIN:
  case V5GAIN:
  case V6GAIN:
  case V7GAIN: {
    dsp->channel[ch].directGain = (val & 0x80) == 0;
    if (val & 0x80) {
      dsp->channel[ch].gainMode = (val & 0x60) >> 5;
      dsp->channel[ch].adsrRates[3] = rateValues[val & 0x1f];
    } else {
      dsp->channel[ch].gainValue = (val & 0x7f) * 16;
    }
    break;
  }
  case MVOLL: {
    dsp->masterVolumeL = val;
    break;
  }
  case MVOLR: {
    dsp->masterVolumeR = val;
    break;
  }
  case EVOLL: {
    dsp->echoVolumeL = val;
    break;
  }
  case EVOLR: {
    dsp->echoVolumeR = val;
    break;
  }
  case KON: {
    for (int ch = 0; ch < 8; ch++) {
      dsp->channel[ch].keyOn = val & (1 << ch);
#if MY_CHANGES

      if (dsp->channel[ch].keyOn) {
        dsp->channel[ch].keyOn = false;
        // restart current sample
        dsp->channel[ch].previousFlags = 0;
        uint16_t samplePointer = dsp->dirPage + 4 * dsp->channel[ch].srcn;
        dsp->channel[ch].decodeOffset = dsp->apu_ram[samplePointer];
        dsp->channel[ch].decodeOffset |=
            dsp->apu_ram[(samplePointer + 1) & 0xffff] << 8;
        memset(dsp->channel[ch].decodeBuffer, 0,
               sizeof(dsp->channel[ch].decodeBuffer));
        dsp->channel[ch].gain = 0;
        dsp->channel[ch].adsrState = dsp->channel[ch].useGain ? 3 : 0;
      }
#endif
    }
    break;
  }
  case KOF: {
    for (int ch = 0; ch < 8; ch++) {
      dsp->channel[ch].keyOff = val & (1 << ch);
#if MY_CHANGES
      if (dsp->channel[ch].keyOff) {
        // go to release
        dsp->channel[ch].adsrState = 4;
      }
#endif
    }

    break;
  }
  case FLG: {
    dsp->reset = val & 0x80;
    dsp->mute = val & 0x40;
    dsp->echoWrites = (val & 0x20) == 0;
    dsp->noiseRate = rateValues[val & 0x1f];
    break;
  }
  case ENDX: {
    val = 0; // any write clears ENDx
    break;
  }
  case EFB: {
    dsp->feedbackVolume = val;
    break;
  }
  case PMON: {
    for (int i = 0; i < 8; i++) {
      dsp->channel[i].pitchModulation = val & (1 << i);
    }
    break;
  }
  case NON: {
    for (int i = 0; i < 8; i++) {
      dsp->channel[i].useNoise = val & (1 << i);
    }
    break;
  }
  case EON: {
    for (int i = 0; i < 8; i++) {
      dsp->channel[i].echoEnable = val & (1 << i);
    }
    break;
  }
  case DIR: {
    dsp->dirPage = val << 8;
    break;
  }
  case ESA: {
    dsp->echoBufferAdr = val << 8;
    break;
  }
  case EDL: {
    dsp->echoDelay =
        (val & 0xf) * 512; // 2048-byte steps, stereo sample is 4 bytes
    if (dsp->echoDelay == 0)
      dsp->echoDelay = 1;
    break;
  }
  case FIR0:
  case FIR1:
  case FIR2:
  case FIR3:
  case FIR4:
  case FIR5:
  case FIR6:
  case FIR7: {
    dsp->firValues[ch] = val;
    break;
  }
  }
  dsp->ram[adr] = val;
}

void dsp_getSamples(Dsp* dsp, int16_t* sampleData, int samplesPerFrame, int numChannels) {
  // resample from 534 samples per frame to wanted value
  float adder = 534.0f / samplesPerFrame;
  float location = 0.0f;

  if (numChannels == 1) {
    for (int i = 0; i < samplesPerFrame; i++) {
      int sampleL = dsp->sampleBuffer[((int)location) * 2];
      int sampleR = dsp->sampleBuffer[((int)location) * 2 + 1];
      sampleData[i] = (sampleL + sampleR) >> 1;
      location += adder;
    }
  } else {
    for (int i = 0; i < samplesPerFrame; i++) {
      int sampleL = dsp->sampleBuffer[((int)location) * 2];
      int sampleR = dsp->sampleBuffer[((int)location) * 2 + 1];
      sampleData[i * 2] = sampleL;
      sampleData[i * 2 + 1] = sampleR;
      location += adder;
    }
  }
  dsp->sampleOffset = 0;
}
