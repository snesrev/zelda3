// This file defines various things related to the runtime environment
// of the code
#ifndef ZELDA3_ZELDA_RTL_H_
#define ZELDA3_ZELDA_RTL_H_

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include "types.h"
#include "features.h"

struct Snes;
struct Dsp;

typedef struct ZeldaEnv {
  uint8 *ram;
  uint8 *sram;
  uint16 *vram;
  struct Ppu *ppu;
  struct SpcPlayer *player;
  struct Dma *dma;
} ZeldaEnv;
extern ZeldaEnv g_zenv;
extern int frame_ctr_dbg;

typedef void PlayerHandlerFunc();
typedef void HandlerFuncK(int k);

static inline void zelda_snes_dummy_write(uint32 adr, uint8 val) {}

void zelda_apu_write(uint32_t adr, uint8_t val);
uint8_t zelda_read_apui00();
uint8_t zelda_apu_read(uint32_t adr);
void zelda_ppu_write(uint32_t adr, uint8_t val);
void zelda_ppu_write_word(uint32_t adr, uint16_t val);


// 512x480 32-bit pixels. Returns true if we instead draw 1024x960
void HdmaSetup(uint32 addr6, uint32 addr7, uint8 transfer_unit, uint8 reg6, uint8 reg7, uint8 indirect_bank);

void ZeldaInitialize();
void ZeldaReset(bool preserve_sram);
bool ZeldaDrawPpuFrame(uint8 *pixel_buffer, size_t pitch, uint32 render_flags);
void ZeldaRunFrameInternal(uint16 input, int run_what);
bool ZeldaRunFrame(int input_state);
void LoadSongBank(const uint8 *p);

void PatchCommand(char cmd);

// Things for msu
void ZeldaPlayMsuAudioTrack();
void MixinMsuAudioData(int16 *audio_buffer, int audio_samples);
void ZeldaOpenMsuFile();
bool ZeldaIsMusicPlaying();


// Things for state management

enum {
  kSaveLoad_Save = 0,
  kSaveLoad_Load = 1,
  kSaveLoad_Replay = 2,
};

void SaveLoadSlot(int cmd, int which);
void ZeldaWriteSram();
void ZeldaReadSram();

void ZeldaRenderAudio(int16 *audio_buffer, int samples, int channels);
void ZeldaDiscardUnusedAudioFrames();

typedef void ZeldaRunFrameFunc(uint16 input, int run_what);
typedef void ZeldaSyncAllFunc();

void ZeldaSetupEmuCallbacks(uint8 *emu_ram, ZeldaRunFrameFunc *func, ZeldaSyncAllFunc *sync_all);

void Convert2bppToNewFormat(const uint16 *src, uint32 dst_addr, size_t count);

#endif  // ZELDA3_ZELDA_RTL_H_
