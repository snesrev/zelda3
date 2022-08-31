
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include "snes/snes.h"
#include "tracing.h"

#include "types.h"
#include "variables.h"

#include "zelda_rtl.h"

extern uint8 g_emulated_ram[0x20000];
bool g_run_without_emu = false;

void PatchRom(uint8_t *rom);
void SetSnes(Snes *snes);
void RunAudioPlayer();
void CopyStateAfterSnapshotRestore(bool is_reset);
void SaveLoadSlot(int cmd, int which);
void PatchCommand(char cmd);
bool RunOneFrame(Snes *snes, int input_state, bool turbo);

static uint8_t* readFile(char* name, size_t* length);
static bool loadRom(char* name, Snes* snes);
static bool checkExtention(const char* name, bool forZip);
static void playAudio(Snes *snes, SDL_AudioDeviceID device, int16_t* audioBuffer);
static void renderScreen(SDL_Renderer* renderer, SDL_Texture* texture);
static void handleInput(int keyCode, int modCode, bool pressed);

int input1_current_state;

void setButtonState(int button, bool pressed) {
  // set key in constroller
  if (pressed) {
    input1_current_state |= 1 << button;
  } else {
    input1_current_state &= ~(1 << button);
  }
}


void ZeldaReadSram(Snes *snes) {
  FILE *f = fopen("saves/sram.dat", "rb");
  if (f) {
    fread(g_zenv.sram, 1, 8192, f);
    memcpy(snes->cart->ram, g_zenv.sram, 8192);
    fclose(f);
  }
}

void ZeldaWriteSram() {
  rename("saves/sram.dat", "saves/sram.bak");
  FILE *f = fopen("saves/sram.dat", "wb");
  if (f) {
    fwrite(g_zenv.sram, 1, 8192, f);
    fclose(f);
  } else {
    fprintf(stderr, "Unable to write saves/sram.dat\n");
  }
}

const int NATIVE_WIDTH = 512;
const int NATIVE_HEIGHT = 480;
int zoom = -1;

static void doZoom(SDL_Window* window, SDL_Renderer* renderer, int zoomIn) {
  float w = NATIVE_WIDTH;
  float h = NATIVE_HEIGHT;

  SDL_SetWindowFullscreen(window, SDL_WINDOW_SHOWN);  // disable fullscreen
  int screen = SDL_GetWindowDisplayIndex(window);
  if (screen < 0) screen = 0;

  if (zoomIn > 0)
    zoom++;
  else if (zoomIn == 0)
    zoom--;

  if (zoom < 0) {  // autosize
    SDL_Rect bounds;
    if (SDL_GetDisplayUsableBounds(screen, &bounds) == 0) {  // note this takes into effect Windows display scaling, i.e., resolution is divided by scale factor
      int bw = bounds.w;
      int bh = bounds.h;
      int t, l, b, r;
      if (SDL_GetWindowBordersSize(window, &t, &l, &b, &r) == 0) {  // this call may take a while before it is reported by Windows (or not at all in my testing)
        bw = bw - l - r;
        bh = bh - t - b;
      } else {  // guess based on Windows 10/11 defaults
        bw -= 2;
        bh -= 32;
      }
      w = bw;
      h = bh;
      float nratio = (float)NATIVE_HEIGHT / (float)NATIVE_WIDTH;  // 480/512 = 0.9375
      float ratio = h / w;  // 2160/3840 = 0.5625  // 1200/1920 = 0.625

      // determine maximum window size for screen
      int i;
      for (i = 1; i <= 20; i++) {
        if (bw > NATIVE_WIDTH * i && bh > NATIVE_HEIGHT * i) {
          w = NATIVE_WIDTH * i;
          h = NATIVE_HEIGHT * i;
        } else {
          i--;
          break;
        }
      }

      if (i == 0) {
        w = NATIVE_WIDTH / 2;
        h = NATIVE_HEIGHT / 2;
      }
      zoom = -1;
    } else {
      zoom = 1;
    }
  } else if (zoom == 0) {
    w = NATIVE_WIDTH / 2;
    h = NATIVE_HEIGHT / 2;
  } else {
    if (zoom > 20) zoom = 20;
    w = NATIVE_WIDTH * zoom;
    h = NATIVE_HEIGHT * zoom;
  }
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best");
  SDL_RenderSetLogicalSize(renderer, (int)w, (int)h);
  SDL_SetWindowSize(window, (int)w, (int)h);
  SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
}

#undef main
int main(int argc, char** argv) {
  // set up SDL
  if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
    printf("Failed to init SDL: %s\n", SDL_GetError());
    return 1;
  }
  uint32 win_flags = SDL_WINDOWPOS_UNDEFINED;
  SDL_Window* window = SDL_CreateWindow("Zelda3", SDL_WINDOWPOS_UNDEFINED, win_flags, NATIVE_WIDTH, NATIVE_HEIGHT, 0);
  if(window == NULL) {
    printf("Failed to create window: %s\n", SDL_GetError());
    return 1;
  }
  SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if(renderer == NULL) {
    printf("Failed to create renderer: %s\n", SDL_GetError());
    return 1;
  }

  if (zoom < 0) {  // autosize
    doZoom(window, renderer, zoom);
  }

  SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBX8888, SDL_TEXTUREACCESS_STREAMING, NATIVE_WIDTH, NATIVE_HEIGHT);
  if(texture == NULL) {
    printf("Failed to create texture: %s\n", SDL_GetError());
    return 1;
  }
  SDL_AudioSpec want, have;
  SDL_AudioDeviceID device;
  SDL_memset(&want, 0, sizeof(want));
  want.freq = 44100;
  want.format = AUDIO_S16;
  want.channels = 2;
  want.samples = 2048;
  want.callback = NULL; // use queue
  device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
  if(device == 0) {
    printf("Failed to open audio device: %s\n", SDL_GetError());
    return 1;
  }
  int16_t* audioBuffer = (int16_t * )malloc(735 * 4); // *2 for stereo, *2 for sizeof(int16)
  SDL_PauseAudioDevice(device, 0);

  Snes *snes = snes_init(g_emulated_ram), *snes_run = NULL;
  if (argc >= 2 && !g_run_without_emu) {
    // init snes, load rom
    bool loaded = loadRom(argv[1], snes);
    if (!loaded) {
      puts("No rom loaded");
      return 1;
    }
    snes_run = snes;
  } else {
    snes_reset(snes, true);
  }

#if defined(_WIN32)
  _mkdir("saves");
#else
  mkdir("saves", 755);
#endif

  SetSnes(snes);
  ZeldaInitialize();
  ZeldaReadSram(snes);

  bool hooks = true;
  // sdl loop
  bool running = true;
  SDL_Event event;
  uint32 lastTick = SDL_GetTicks();
  uint32 curTick = 0;
  uint32 delta = 0;
  int numFrames = 0;
  bool cpuNext = false;
  bool spcNext = false;
  int counter = 0;
  bool paused = false;
  bool turbo = true;
  uint32 frameCtr = 0;

  while(running) {
    while(SDL_PollEvent(&event)) {
      switch(event.type) {
      case SDL_KEYDOWN: {
        bool skip_default = false;
        switch(event.key.keysym.sym) {
        case SDLK_e:
          if (snes) {
            snes_reset(snes, event.key.keysym.sym == SDLK_e);
            CopyStateAfterSnapshotRestore(true);
          }
          break;
        case SDLK_p: paused ^= true; break;
        case SDLK_w:
          PatchCommand('w');
          break;
        case SDLK_o:
          PatchCommand('o');
          break;
        case SDLK_k:
          PatchCommand('k');
          break;
        case SDLK_t:
          turbo = !turbo;
          break;
        case SDLK_KP_MINUS:
            doZoom(window, renderer, false);
            break;
        case SDLK_KP_PLUS:
            doZoom(window, renderer, true);
            break;
        case SDLK_RETURN:
          if (event.key.keysym.mod & KMOD_ALT) {
            win_flags ^= SDL_WINDOW_FULLSCREEN_DESKTOP;
            SDL_SetWindowFullscreen(window, win_flags);
            skip_default = true;
          }
          break;
        }
        if (!skip_default)
          handleInput(event.key.keysym.sym, event.key.keysym.mod, true);
        break;
      }
      case SDL_KEYUP: {
        handleInput(event.key.keysym.sym, event.key.keysym.mod, false);
        break;
      }
      case SDL_QUIT: {
        running = false;
        break;
      }
    }
  }

    if (paused) {
      SDL_Delay(16);
      continue;
    }

    bool is_turbo = RunOneFrame(snes_run, input1_current_state, (counter++ & 0x7f) != 0 && turbo);

    if (is_turbo)
      continue;

    ZeldaDrawPpuFrame();

    playAudio(snes_run, device, audioBuffer);
    renderScreen(renderer, texture);

    SDL_RenderPresent(renderer); // vsyncs to 60 FPS
    // if vsync isn't working, delay manually
    curTick = SDL_GetTicks();

    static const uint8 delays[3] = { 17, 17, 16 }; // 60 fps
#if 1
    lastTick += delays[frameCtr++ % 3];

    if (lastTick > curTick) {
      delta = lastTick - curTick;
      if (delta > 500) {
        lastTick = curTick - 500;
        delta = 500;
      }
      SDL_Delay(delta);
    } else if (curTick - lastTick > 500) {
      lastTick = curTick;
    }
#endif
  }
  // clean snes
  snes_free(snes);
  // clean sdl
  SDL_PauseAudioDevice(device, 1);
  SDL_CloseAudioDevice(device);
  free(audioBuffer);
  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}

extern struct Ppu *GetPpuForRendering();
extern struct Dsp *GetDspForRendering();

static void playAudio(Snes *snes, SDL_AudioDeviceID device, int16_t* audioBuffer) {
  // generate enough samples
  if (!kIsOrigEmu && snes) {
    while (snes->apu->dsp->sampleOffset < 534)
      apu_cycle(snes->apu);
    snes->apu->dsp->sampleOffset = 0;
  }

  dsp_getSamples(GetDspForRendering(), audioBuffer, 735);
  if(SDL_GetQueuedAudioSize(device) <= 735 * 4 * 6) {
    // don't queue audio if buffer is still filled
    SDL_QueueAudio(device, audioBuffer, 735 * 4);
  } else {
    printf("Skipping audio!\n");
  }
}

static void renderScreen(SDL_Renderer* renderer, SDL_Texture* texture) {
  void* pixels = NULL;
  int pitch = 0;
  if(SDL_LockTexture(texture, NULL, &pixels, &pitch) != 0) {
    printf("Failed to lock texture: %s\n", SDL_GetError());
    return;
  }

  ppu_putPixels(GetPpuForRendering(), (uint8_t*) pixels);
  SDL_UnlockTexture(texture);
  SDL_RenderCopy(renderer, texture, NULL, NULL);
}


static void handleInput(int keyCode, int keyMod, bool pressed) {
  switch(keyCode) {
    case SDLK_z: setButtonState(0, pressed); break;
    case SDLK_a: setButtonState(1, pressed); break;
    case SDLK_RSHIFT: setButtonState(2, pressed); break;
    case SDLK_RETURN: setButtonState(3, pressed); break;
    case SDLK_UP: setButtonState(4, pressed); break;
    case SDLK_DOWN: setButtonState(5, pressed); break;
    case SDLK_LEFT: setButtonState(6, pressed); break;
    case SDLK_RIGHT: setButtonState(7, pressed); break;
    case SDLK_x: setButtonState(8, pressed); break;
    case SDLK_s: setButtonState(9, pressed); break;
    case SDLK_d: setButtonState(10, pressed); break;
    case SDLK_c: setButtonState(11, pressed); break;
    case SDLK_BACKSPACE:
    case SDLK_1:
    case SDLK_2:
    case SDLK_3:
    case SDLK_4:
    case SDLK_5:
    case SDLK_6:
    case SDLK_7:
    case SDLK_8:
    case SDLK_9:
    case SDLK_0:
    case SDLK_MINUS:
    case SDLK_EQUALS:
      if (pressed) {
        SaveLoadSlot(
          (keyMod & KMOD_CTRL) != 0 ? kSaveLoad_Replay : kSaveLoad_Load,
          256 + (keyCode == SDLK_0 ? 9 : 
                 keyCode == SDLK_MINUS ? 10 : 
                 keyCode == SDLK_EQUALS ? 11 :
                 keyCode == SDLK_BACKSPACE ? 12 :
                 keyCode - SDLK_1));
      }
      break;

    case SDLK_F1: 
    case SDLK_F2: 
    case SDLK_F3: 
    case SDLK_F4: 
    case SDLK_F5: 
    case SDLK_F6: 
    case SDLK_F7: 
    case SDLK_F8: 
    case SDLK_F9: 
    case SDLK_F10: 
      if (pressed) {
        SaveLoadSlot(
          (keyMod & KMOD_CTRL) != 0 ? kSaveLoad_Replay : 
          (keyMod & KMOD_SHIFT) != 0 ? kSaveLoad_Save : kSaveLoad_Load,
          keyCode - SDLK_F1);
      }
      break;
  }
}

static bool checkExtention(const char* name, bool forZip) {
  if(name == NULL) return false;
  int length = strlen(name);
  if(length < 4) return false;
  if(forZip) {
    if(strcmp(name + length - 4, ".zip") == 0) return true;
    if(strcmp(name + length - 4, ".ZIP") == 0) return true;
  } else {
    if(strcmp(name + length - 4, ".smc") == 0) return true;
    if(strcmp(name + length - 4, ".SMC") == 0) return true;
    if(strcmp(name + length - 4, ".sfc") == 0) return true;
    if(strcmp(name + length - 4, ".SFC") == 0) return true;
  }
  return false;
}

static bool loadRom(char* name, Snes* snes) {
  // zip library from https://github.com/kuba--/zip
  size_t length = 0;
  uint8_t* file = NULL;
  file = readFile(name, &length);
  if(file == NULL) {
    puts("Failed to read file");
    return false;
  }

  PatchRom(file);

  bool result = snes_loadRom(snes, file, length);
  free(file);
  return result;
}

static uint8_t* readFile(char* name, size_t* length) {
  FILE* f = fopen(name, "rb");
  if(f == NULL) {
    return NULL;
  }
  fseek(f, 0, SEEK_END);
  int size = ftell(f);
  rewind(f);
  uint8_t* buffer = (uint8_t *)malloc(size);
  fread(buffer, size, 1, f);
  fclose(f);
  *length = size;
  return buffer;
}
