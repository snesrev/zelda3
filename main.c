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
#include <math.h>
#include "snes/snes.h"
#include "tracing.h"

#include "types.h"
#include "variables.h"

#include "zelda_rtl.h"

extern Ppu *GetPpuForRendering();
extern Dsp *GetDspForRendering();

extern uint8 g_emulated_ram[0x20000];
bool g_run_without_emu = false;

void PatchRom(uint8 *rom);
void SetSnes(Snes *snes);
void RunAudioPlayer();
void CopyStateAfterSnapshotRestore(bool is_reset);
void SaveLoadSlot(int cmd, int which);
void PatchCommand(char cmd);
bool RunOneFrame(Snes *snes, int input_state, bool turbo);

static uint8 *ReadFile(char* name, size_t* length);
static bool LoadRom(char* name, Snes* snes);
static void PlayAudio(Snes *snes, SDL_AudioDeviceID device, int16 *audioBuffer);
static void RenderScreen(SDL_Window *window, SDL_Renderer *renderer, SDL_Texture *texture, bool fullscreen);
static void HandleInput(int keyCode, int modCode, bool pressed);
static void HandleGamepadInput(int button, bool pressed);
static void HandleGamepadAxisInput(int gamepad_id, int axis, int value);
static void OpenOneGamepad(int i);

static int input1_current_state;

void NORETURN Die(const char *error) {
  fprintf(stderr, "Error: %s\n", error);
  exit(1);
}

void SetButtonState(int button, bool pressed) {
  // set key in constroller
  if (pressed) {
    input1_current_state |= 1 << button;
  } else {
    input1_current_state &= ~(1 << button);
  }
}

#undef main
int main(int argc, char** argv) {
  // set up SDL
  if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) != 0) {
    printf("Failed to init SDL: %s\n", SDL_GetError());
    return 1;
  }
  uint32 win_flags = SDL_WINDOWPOS_UNDEFINED;
  SDL_Window* window = SDL_CreateWindow("Zelda3", SDL_WINDOWPOS_UNDEFINED, win_flags, 512, 480, 0);
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
    bool loaded = LoadRom(argv[1], snes);
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

  for (int i = 0; i < SDL_NumJoysticks(); i++)
    OpenOneGamepad(i);

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
      case SDL_CONTROLLERDEVICEADDED:
        OpenOneGamepad(event.cdevice.which);
        break;
      case SDL_CONTROLLERAXISMOTION:
        HandleGamepadAxisInput(event.caxis.which, event.caxis.axis, event.caxis.value);
        break;
      case SDL_CONTROLLERBUTTONDOWN:
        HandleGamepadInput(event.cbutton.button, event.cbutton.state == SDL_PRESSED);
        break;
      case SDL_CONTROLLERBUTTONUP:
        HandleGamepadInput(event.cbutton.button, event.cbutton.state == SDL_PRESSED);
        break;
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
        case SDLK_RETURN:
          if (event.key.keysym.mod & KMOD_ALT) {
            win_flags ^= SDL_WINDOW_FULLSCREEN_DESKTOP;
            SDL_SetWindowFullscreen(window, win_flags);
            skip_default = true;
          }
          break;
        }
        if (!skip_default)
          HandleInput(event.key.keysym.sym, event.key.keysym.mod, true);
        break;
      }
      case SDL_KEYUP: {
        HandleInput(event.key.keysym.sym, event.key.keysym.mod, false);
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

    PlayAudio(snes_run, device, audioBuffer);
    RenderScreen(window, renderer, texture, (win_flags & SDL_WINDOW_FULLSCREEN_DESKTOP) != 0);

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

static void PlayAudio(Snes *snes, SDL_AudioDeviceID device, int16 *audioBuffer) {
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

static void RenderScreen(SDL_Window *window, SDL_Renderer *renderer, SDL_Texture *texture, bool fullscreen) {
  void* pixels = NULL;
  int pitch = 0;
  if(SDL_LockTexture(texture, NULL, &pixels, &pitch) != 0) {
    printf("Failed to lock texture: %s\n", SDL_GetError());
    return;
  }

  ppu_putPixels(GetPpuForRendering(), (uint8_t*) pixels);
  SDL_UnlockTexture(texture);

  SDL_DisplayMode display_mode;
  if (fullscreen && SDL_GetWindowDisplayMode(window, &display_mode) == 0) {
    uint32 w = display_mode.w, h = display_mode.h;
    if (w * 15 < h * 16)
      h = w * 15 / 16;  // limit height
    else
      w = h * 16 / 15;  // limit width
    SDL_Rect r = { (display_mode.w - w) / 2, (display_mode.h - h) / 2, w, h };
    SDL_RenderCopy(renderer, texture, NULL, &r);
  } else {
    SDL_RenderCopy(renderer, texture, NULL, NULL);
  }
}


static void HandleInput(int keyCode, int keyMod, bool pressed) {
  switch(keyCode) {
    case SDLK_z: SetButtonState(0, pressed); break;
    case SDLK_a: SetButtonState(1, pressed); break;
    case SDLK_RSHIFT: SetButtonState(2, pressed); break;
    case SDLK_RETURN: SetButtonState(3, pressed); break;
    case SDLK_UP: SetButtonState(4, pressed); break;
    case SDLK_DOWN: SetButtonState(5, pressed); break;
    case SDLK_LEFT: SetButtonState(6, pressed); break;
    case SDLK_RIGHT: SetButtonState(7, pressed); break;
    case SDLK_x: SetButtonState(8, pressed); break;
    case SDLK_s: SetButtonState(9, pressed); break;
    case SDLK_d: SetButtonState(10, pressed); break;
    case SDLK_c: SetButtonState(11, pressed); break;
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

static void OpenOneGamepad(int i) {
  if (SDL_IsGameController(i)) {
    SDL_GameController *controller = SDL_GameControllerOpen(i);
    if (!controller)
      fprintf(stderr, "Could not open gamepad %d: %s\n", i, SDL_GetError());
  }
}

static void HandleGamepadInput(int button, bool pressed) {
  switch (button) {
  case SDL_CONTROLLER_BUTTON_A: SetButtonState(0, pressed); break;
  case SDL_CONTROLLER_BUTTON_X: SetButtonState(1, pressed); break;
  case SDL_CONTROLLER_BUTTON_BACK: SetButtonState(2, pressed); break;
  case SDL_CONTROLLER_BUTTON_START: SetButtonState(3, pressed); break;
  case SDL_CONTROLLER_BUTTON_DPAD_UP: SetButtonState(4, pressed); break;
  case SDL_CONTROLLER_BUTTON_DPAD_DOWN: SetButtonState(5, pressed); break;
  case SDL_CONTROLLER_BUTTON_DPAD_LEFT: SetButtonState(6, pressed); break;
  case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: SetButtonState(7, pressed); break;
  case SDL_CONTROLLER_BUTTON_B: SetButtonState(8, pressed); break;
  case SDL_CONTROLLER_BUTTON_Y: SetButtonState(9, pressed); break;
  case SDL_CONTROLLER_BUTTON_LEFTSHOULDER: SetButtonState(10, pressed); break;
  case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: SetButtonState(11, pressed); break;
  }
}

static void HandleGamepadAxisInput(int gamepad_id, int axis, int value) {
  static int last_gamepad_id, last_x, last_y;
  if (axis == SDL_CONTROLLER_AXIS_LEFTX || axis == SDL_CONTROLLER_AXIS_LEFTY) {
    // try to be smart if there's more than one gamepad
    if (last_gamepad_id != gamepad_id && (value < -10000 || value > 10000)) {
      last_gamepad_id = gamepad_id;
      last_x = last_y = 0;
    }
    *(axis == SDL_CONTROLLER_AXIS_LEFTX ? &last_x : &last_y) = value;
    int buttons = 0;
    if (last_x * last_x + last_y * last_y >= 10000 * 10000) {
      // in the non deadzone part, divide the circle into eight 45 degree
      // segments rotated by 22.5 degrees that control which direction to move.
      // todo: do this without floats?
      static const uint8 kSegmentToButtons[8] = {
        1 << 4,           // 0 = up
        1 << 4 | 1 << 7,  // 1 = up, right
        1 << 7,           // 2 = right
        1 << 7 | 1 << 5,  // 3 = right, down
        1 << 5,           // 4 = down
        1 << 5 | 1 << 6,  // 5 = down, left
        1 << 6,           // 6 = left
        1 << 6 | 1 << 4,  // 7 = left, up
      };
      int angle = (int)(atan2f(last_y, last_x) * (float)(128 / M_PI) + 0.5f);
      buttons = kSegmentToButtons[(uint8)(angle + 16 + 64) >> 5];
    }
    input1_current_state = input1_current_state & ~0xf0 | buttons;
  }
}

static bool LoadRom(char* name, Snes* snes) {
  size_t length = 0;
  uint8_t* file = NULL;
  file = ReadFile(name, &length);
  if(!file) Die("Failed to read file");

  PatchRom(file);

  bool result = snes_loadRom(snes, file, length);
  free(file);
  return result;
}


static uint8_t* ReadFile(char* name, size_t* length) {
  FILE* f = fopen(name, "rb");
  if(f == NULL) {
    return NULL;
  }
  fseek(f, 0, SEEK_END);
  int size = ftell(f);
  rewind(f);
  uint8_t* buffer = (uint8_t *)malloc(size);
  if (!buffer) Die("malloc failed");
  fread(buffer, size, 1, f);
  fclose(f);
  *length = size;
  return buffer;
}
