
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <SDL.h>

#include "snes/snes.h"
#include "tracing.h"

#include "types.h"
#include "variables.h"

#include "zelda_rtl.h"

extern uint8 g_emulated_ram[0x20000];
bool g_run_without_emu = false;

bool fast_forward = false;
int fast_forward_counter = 0;

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

#define SNES_BTN_B        0
#define SNES_BTN_Y        1
#define SNES_BTN_SELECT   2
#define SNES_BTN_START    3
#define SNES_BTN_UP       4
#define SNES_BTN_DOWN     5
#define SNES_BTN_LEFT     6
#define SNES_BTN_RIGHT    7
#define SNES_BTN_A        8
#define SNES_BTN_X        9
#define SNES_BTN_L        10
#define SNES_BTN_R        11

SDL_Joystick* gGameController = NULL;

#undef main
int main(int argc, char** argv) {
  // set up SDL
  if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) != 0) {
    printf("Failed to init SDL: %s\n", SDL_GetError());
    return 1;
  }

  // check for joysticks
  if (SDL_NumJoysticks() < 1) {
    printf("Warning: No joysticks connected!\n");
  } else {
    // Load joystick
    gGameController = SDL_JoystickOpen(0);
    if (gGameController == NULL) {
      printf("Warning: Unable to open game controller! SDL Error: %s\n", SDL_GetError());
    }
  }

  SDL_Window* window = SDL_CreateWindow("Zelda3", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 512*2, 480*2, 0);
  if(window == NULL) {
    printf("Failed to create window: %s\n", SDL_GetError());
    return 1;
  }
  SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if(renderer == NULL) {
    printf("Failed to create renderer: %s\n", SDL_GetError());
    return 1;
  }
  SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBX8888, SDL_TEXTUREACCESS_STREAMING, 512, 480);
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
  SetSnes(snes);
  ZeldaInitialize();
  bool hooks = true;
  // sdl loop
  bool running = true;
  SDL_Event event;
  uint32_t lastTick = SDL_GetTicks();
  uint32_t curTick = 0;
  uint32_t delta = 0;
  int numFrames = 0;
  bool cpuNext = false;
  bool spcNext = false;
  int counter = 0;
  bool paused = false;
  bool turbo = true;
  uint32_t frameCtr = 0;

  printf("%d\n", *(int *)snes->cart->ram);

  while(running) {
    while(SDL_PollEvent(&event)) {
      switch(event.type) {
        case SDL_KEYDOWN: {
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
          }
          handleInput(event.key.keysym.sym, event.key.keysym.mod, true);
          break;
        }
        case SDL_KEYUP: {
          handleInput(event.key.keysym.sym, event.key.keysym.mod, false);
          break;
        }
        case SDL_JOYAXISMOTION: {
          // only first controller for now:
          if (event.jaxis.which != 0) {
            break;
          }

          //printf("Axis Moved : %d, %d\n", event.jaxis.axis, event.jaxis.value);
          if (event.jaxis.axis == 0) {
            if (event.jaxis.value <= -8000) {
              setButtonState(6, true);
              setButtonState(7, false);
            } else if (event.jaxis.value > 8000) {
              setButtonState(6, false);
              setButtonState(7, true);
            } else {
              setButtonState(6, false);
              setButtonState(7, false);
            }
          }
          if (event.jaxis.axis == 1) {
            if (event.jaxis.value <= -8000) {
              setButtonState(4, true);
              setButtonState(5, false);
            } else if (event.jaxis.value > 8000) {
              setButtonState(4, false);
              setButtonState(5, true);
            } else {
              setButtonState(4, false);
              setButtonState(5, false);
            }
          }
          break;
        }
        case SDL_JOYHATMOTION: {
          //printf("Hat Moved : %d\n", event.jhat.value);
          setButtonState(4, (event.jhat.value & 1) == 1);
          setButtonState(5, (event.jhat.value & 4) == 4);
          setButtonState(6, (event.jhat.value & 8) == 8);
          setButtonState(7, (event.jhat.value & 2) == 2);
          break;
        }
        case SDL_JOYBUTTONDOWN: {
          printf("Button Pressed : %d\n", event.jbutton.button);
          if (event.jbutton.button == 0) {
            setButtonState(SNES_BTN_A, true);
          } else if (event.jbutton.button == 1) {
            setButtonState(SNES_BTN_B, true);
          } else if (event.jbutton.button == 2) {
            setButtonState(SNES_BTN_X, true);
          } else if (event.jbutton.button == 3) {
            setButtonState(SNES_BTN_Y, true);
          } else if (event.jbutton.button == 4) {
            setButtonState(SNES_BTN_L, true);
          } else if (event.jbutton.button == 5) {
            setButtonState(SNES_BTN_R, true);
          } else if (event.jbutton.button == 9) {
            setButtonState(SNES_BTN_START, true);
          } else if (event.jbutton.button == 8) {
            setButtonState(SNES_BTN_SELECT, true);
          }
          break;
        }
        case SDL_JOYBUTTONUP: {
          printf("Button Released : %d\n", event.jbutton.button);
          if (event.jbutton.button == 0) {
            setButtonState(SNES_BTN_A, false);
          } else if (event.jbutton.button == 1) {
            setButtonState(SNES_BTN_B, false);
          } else if (event.jbutton.button == 2) {
            setButtonState(SNES_BTN_X, false);
          } else if (event.jbutton.button == 3) {
            setButtonState(SNES_BTN_Y, false);
          } else if (event.jbutton.button == 4) {
            setButtonState(SNES_BTN_L, false);
          } else if (event.jbutton.button == 5) {
            setButtonState(SNES_BTN_R, false);
          } else if (event.jbutton.button == 9) {
            setButtonState(SNES_BTN_START, false);
          } else if (event.jbutton.button == 8) {
            setButtonState(SNES_BTN_SELECT, false);
          }
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

    if (fast_forward) {
      if ((fast_forward_counter++ & 3) != 0) {
        continue;
      }
    }

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

    case SDLK_TAB:
      fast_forward = pressed;
      printf("Fast Forward: %d\n", fast_forward);
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
