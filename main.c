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
#include <unistd.h>
#endif
#include <math.h>
#include "snes/snes.h"
#include "tracing.h"

#include "types.h"
#include "variables.h"

#include "zelda_rtl.h"
#include "config.h"

extern Dsp *GetDspForRendering();
extern Snes *g_snes;
extern uint8 g_emulated_ram[0x20000];

void PatchRom(uint8 *rom);
void SetSnes(Snes *snes);
void RunAudioPlayer();
void CopyStateAfterSnapshotRestore(bool is_reset);
void SaveLoadSlot(int cmd, int which);
void PatchCommand(char cmd);
bool RunOneFrame(Snes *snes, int input_state, bool turbo);

static bool LoadRom(const char *name, Snes *snes);
static void PlayAudio(Snes *snes, SDL_AudioDeviceID device, int channels, int16 *audioBuffer);
static void RenderScreen(SDL_Window *window, SDL_Renderer *renderer, SDL_Texture *texture, bool fullscreen);
static void HandleInput(int keyCode, int modCode, bool pressed);
static void HandleGamepadInput(int button, bool pressed);
static void HandleGamepadAxisInput(int gamepad_id, int axis, int value);
static void OpenOneGamepad(int i);


enum {
  kRenderWidth = 256 * 2,
  kRenderHeight = 224 * 2,
  kDefaultFullscreen = 0,
  kDefaultWindowScale = 2,
  kMaxWindowScale = 10,
  kDefaultFreq = 44100,
  kDefaultChannels = 2,
  kDefaultSamples = 2048,
};

static const char kWindowTitle[] = "The Legend of Zelda: A Link to the Past";

static uint32 g_win_flags = SDL_WINDOW_RESIZABLE;
static SDL_Window *g_window;
static SDL_Renderer *g_renderer;
static uint8 g_paused, g_turbo = true, g_cursor = true;
static uint8 g_current_window_scale;
static int g_samples_per_block;
static uint8 g_gamepad_buttons;
static int g_input1_state;
static bool g_display_perf;
static int g_curr_fps;
static int g_ppu_render_flags = 0;
bool g_run_without_emu = false;


void NORETURN Die(const char *error) {
  fprintf(stderr, "Error: %s\n", error);
  exit(1);
}

void SetButtonState(int button, bool pressed) {
  // set key in constroller
  if (pressed) {
    g_input1_state |= 1 << button;
  } else {
    g_input1_state &= ~(1 << button);
  }
}


void ChangeWindowScale(int scale_step) {
  int screen = SDL_GetWindowDisplayIndex(g_window);
  if (screen < 0) screen = 0;
  int max_scale = kMaxWindowScale;
  SDL_Rect bounds;
  int bt = -1, bl, bb, br;
  // note this takes into effect Windows display scaling, i.e., resolution is divided by scale factor
  if (SDL_GetDisplayUsableBounds(screen, &bounds) == 0) {
    // this call may take a while before it is reported by Windows (or not at all in my testing)
    if (SDL_GetWindowBordersSize(g_window, &bt, &bl, &bb, &br) != 0) {
      // guess based on Windows 10/11 defaults
      bl = br = bb = 1;
      bt = 31;
    }
    // Allow a scale level slightly above the max that fits on screen
    int mw = (bounds.w - bl - br + (kRenderWidth / kDefaultWindowScale) / 4) / (kRenderWidth / kDefaultWindowScale);
    int mh = (bounds.h - bt - bb + (kRenderHeight / kDefaultWindowScale) / 4) / (kRenderHeight / kDefaultWindowScale);
    max_scale = IntMin(mw, mh);
  }
  int new_scale = IntMax(IntMin(g_current_window_scale + scale_step, max_scale), 1);
  g_current_window_scale = new_scale;
  int w = new_scale * (kRenderWidth / kDefaultWindowScale);
  int h = new_scale * (kRenderHeight / kDefaultWindowScale);
  
  //SDL_RenderSetLogicalSize(g_renderer, w, h);
  SDL_SetWindowSize(g_window, w, h);
  if (bt >= 0) {
    // Center the window on top of the mouse
    int mx, my;
    SDL_GetGlobalMouseState(&mx, &my);
    int wx = IntMax(IntMin(mx - w / 2, bounds.x + bounds.w - bl - br - w), bounds.x + bl);
    int wy = IntMax(IntMin(my - h / 2, bounds.y + bounds.h - bt - bb - h), bounds.y + bt);
    SDL_SetWindowPosition(g_window, wx, wy);
  } else {
    SDL_SetWindowPosition(g_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
  }
}

static SDL_HitTestResult HitTestCallback(SDL_Window *win, const SDL_Point *area, void *data) {
  uint32 flags = SDL_GetWindowFlags(win);
  return ((flags & SDL_WINDOW_FULLSCREEN_DESKTOP) == 0 || (flags & SDL_WINDOW_FULLSCREEN) == 0) && 
         (SDL_GetModState() & KMOD_CTRL) != 0 ? SDL_HITTEST_DRAGGABLE : SDL_HITTEST_NORMAL;
}

static bool RenderScreenWithPerf(uint8 *pixel_buffer, size_t pitch, uint32 render_flags) {
  bool rv;
  if (g_display_perf || g_config.display_perf_title) {
    static float history[64], average;
    static int history_pos;
    uint64 before = SDL_GetPerformanceCounter();
    rv = ZeldaDrawPpuFrame(pixel_buffer, pitch, render_flags);
    uint64 after = SDL_GetPerformanceCounter();
    float v = (double)SDL_GetPerformanceFrequency() / (after - before);
    average += v - history[history_pos];
    history[history_pos] = v;
    history_pos = (history_pos + 1) & 63;
    g_curr_fps = average * (1.0f / 64);
  } else {
    rv = ZeldaDrawPpuFrame(pixel_buffer, pitch, render_flags);
  }
  return rv;
}

// Go some steps up and find zelda3.ini
static void SwitchDirectory() {
  char buf[4096];
  if (!getcwd(buf, sizeof(buf) - 32))
    return;
  size_t pos = strlen(buf);

  for (int step = 0; pos != 0 && step < 3; step++) {
    memcpy(buf + pos, "/zelda3.ini", 12);
    FILE *f = fopen(buf, "rb");
    if (f) {
      fclose(f);
      buf[pos] = 0;
      if (step != 0) {
        printf("Found zelda3.ini in %s\n", buf);
        chdir(buf);
      }
      return;
    }
    pos--;
    while (pos != 0 && buf[pos] != '/' && buf[pos] != '\\')
      pos--;
  }
}


#undef main
int main(int argc, char** argv) {
  SwitchDirectory();
  ParseConfigFile();
  AfterConfigParse();

  g_ppu_render_flags = g_config.new_renderer * kPpuRenderFlags_NewRenderer | g_config.enhanced_mode7 * kPpuRenderFlags_4x4Mode7;
  
  if (g_config.fullscreen == 1)
    g_win_flags ^= SDL_WINDOW_FULLSCREEN_DESKTOP;
  else if (g_config.fullscreen == 2)
    g_win_flags ^= SDL_WINDOW_FULLSCREEN;

  // Window scale (0=50%, 1=100%, 2=200%, 3=300%, etc.)
  g_current_window_scale = g_config.window_scale == 0 ? 1 : IntMin(g_config.window_scale * 2, kMaxWindowScale);

  // audio_freq: Use common sampling rates (see user config file. values higher than 48000 are not supported.)
  if (g_config.audio_freq < 11025 || g_config.audio_freq > 48000)
    g_config.audio_freq = kDefaultFreq;
  
  // Currently, the SPC/DSP implementation åonly supports up to stereo.
  if (g_config.audio_channels < 1 || g_config.audio_channels > 2)
    g_config.audio_channels = kDefaultChannels;
  
  // audio_samples: power of 2
  if (g_config.audio_samples <= 0 || ((g_config.audio_samples & (g_config.audio_samples - 1)) != 0))
    g_config.audio_samples = kDefaultSamples;

  // set up SDL
  if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) != 0) {
    printf("Failed to init SDL: %s\n", SDL_GetError());
    return 1;
  }

  int window_width = g_current_window_scale * (kRenderWidth / kDefaultWindowScale);
  int window_height = g_current_window_scale * (kRenderHeight / kDefaultWindowScale);
  SDL_Window* window = SDL_CreateWindow(kWindowTitle, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, window_width, window_height, g_win_flags);
  if(window == NULL) {
    printf("Failed to create window: %s\n", SDL_GetError());
    return 1;
  }
  g_window = window;
  SDL_SetWindowHitTest(window, HitTestCallback, NULL);
  SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if(renderer == NULL) {
    printf("Failed to create renderer: %s\n", SDL_GetError());
    return 1;
  }

  SDL_RendererInfo renderer_info;
  SDL_GetRendererInfo(renderer, &renderer_info);
  printf("Supported texture formats:");
  for (int i = 0; i < renderer_info.num_texture_formats; i++)
    printf(" %s", SDL_GetPixelFormatName(renderer_info.texture_formats[i]));
  printf("\n");

  g_renderer = renderer;
  if (!g_config.ignore_aspect_ratio)
    SDL_RenderSetLogicalSize(renderer, kRenderWidth, kRenderHeight);
  SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, kRenderWidth * 2, kRenderHeight * 2);
  if(texture == NULL) {
    printf("Failed to create texture: %s\n", SDL_GetError());
    return 1;
  }
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best");

  SDL_AudioSpec want = { 0 }, have;
  SDL_AudioDeviceID device;
  want.freq = g_config.audio_freq;
  want.format = AUDIO_S16;
  want.channels = g_config.audio_channels;
  want.samples = g_config.audio_samples;
  device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
  if(device == 0) {
    printf("Failed to open audio device: %s\n", SDL_GetError());
    return 1;
  }
  g_samples_per_block = (534 * have.freq) / 32000;
  int16_t *audioBuffer = (int16_t * )malloc(g_samples_per_block * have.channels * sizeof(int16));
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
  mkdir("saves", 0755);
#endif

  SetSnes(snes);
  ZeldaInitialize();
  ZeldaReadSram(snes);

  for (int i = 0; i < SDL_NumJoysticks(); i++)
    OpenOneGamepad(i);

  bool running = true;
  SDL_Event event;
  uint32 lastTick = SDL_GetTicks();
  uint32 curTick = 0;
  uint32 frameCtr = 0;

  if (g_config.autosave)
    SaveLoadSlot(kSaveLoad_Load, 0);

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
      case SDL_MOUSEWHEEL:
        if (((g_win_flags & SDL_WINDOW_FULLSCREEN_DESKTOP) == 0 || (g_win_flags & SDL_WINDOW_FULLSCREEN) == 0) && event.wheel.y != 0 && SDL_GetModState() & KMOD_CTRL)
          ChangeWindowScale(event.wheel.y > 0 ? 1 : -1);
        break;
      case SDL_MOUSEBUTTONDOWN:
        if (event.button.button == SDL_BUTTON_LEFT && event.button.state == SDL_PRESSED && event.button.clicks == 2) {
          if (((g_win_flags & SDL_WINDOW_FULLSCREEN_DESKTOP) == 0 || (g_win_flags & SDL_WINDOW_FULLSCREEN) == 0) && SDL_GetModState() & KMOD_SHIFT) {
            g_win_flags ^= SDL_WINDOW_BORDERLESS;
            SDL_SetWindowBordered(g_window, (g_win_flags & SDL_WINDOW_BORDERLESS) == 0);
          }
        }
        break;
      case SDL_KEYDOWN:
        HandleInput(event.key.keysym.sym, event.key.keysym.mod, true);
        break;
      case SDL_KEYUP:
        HandleInput(event.key.keysym.sym, event.key.keysym.mod, false);
        break;
      case SDL_QUIT:
        running = false;
        break;
      }
    }

    if (g_paused) {
      SDL_Delay(16);
      continue;
    }

    // Clear gamepad inputs when joypad directional inputs to avoid wonkiness
    int inputs = g_input1_state;
    if (g_input1_state & 0xf0)
      g_gamepad_buttons = 0;

    uint64 t0 = SDL_GetPerformanceCounter();

    bool is_turbo = RunOneFrame(snes_run, g_input1_state | g_gamepad_buttons, (frameCtr++ & 0x7f) != 0 && g_turbo);

    if (is_turbo)
      continue;


    uint64 t1 = SDL_GetPerformanceCounter();
    PlayAudio(snes_run, device, have.channels, audioBuffer);
    uint64 t2 = SDL_GetPerformanceCounter();

    RenderScreen(window, renderer, texture, (g_win_flags & SDL_WINDOW_FULLSCREEN_DESKTOP) != 0);
    uint64 t3 = SDL_GetPerformanceCounter();
    SDL_RenderPresent(renderer); // vsyncs to 60 FPS?
    uint64 t4 = SDL_GetPerformanceCounter();

    double f = 1e3 / (double)SDL_GetPerformanceFrequency();
    if (0) printf("Perf %6.2f %6.2f %6.2f %6.2f\n", 
      (t1 - t0) * f,
      (t2 - t1) * f,
      (t3 - t2) * f,
      (t4 - t3) * f
      );


    // if vsync isn't working, delay manually
    curTick = SDL_GetTicks();

    static const uint8 delays[3] = { 17, 17, 16 }; // 60 fps
#if 1
    lastTick += delays[frameCtr % 3];

    if (lastTick > curTick) {
      uint32 delta = lastTick - curTick;
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
  if (g_config.autosave)
    SaveLoadSlot(kSaveLoad_Save, 0);
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
  //SaveConfigFile();
  return 0;
}

static void PlayAudio(Snes *snes, SDL_AudioDeviceID device, int channels, int16 *audioBuffer) {
  // generate enough samples
  if (snes) {
    while (snes->apu->dsp->sampleOffset < 534)
      apu_cycle(snes->apu);
    snes->apu->dsp->sampleOffset = 0;
  }

  dsp_getSamples(GetDspForRendering(), audioBuffer, g_samples_per_block, channels);
  for (int i = 0; i < 10; i++) {
    if (SDL_GetQueuedAudioSize(device) <= g_samples_per_block * channels * sizeof(int16) * 6) {
      // don't queue audio if buffer is still filled
      SDL_QueueAudio(device, audioBuffer, g_samples_per_block * channels * sizeof(int16));
      return;
    }
    SDL_Delay(1);
  }
}

static void RenderDigit(uint8 *dst, size_t pitch, int digit, uint32 color, bool big) {
  static const uint8 kFont[] = {
    0x1c, 0x36, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x36, 0x1c,
    0x18, 0x1c, 0x1e, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x7e,
    0x3e, 0x63, 0x60, 0x30, 0x18, 0x0c, 0x06, 0x03, 0x63, 0x7f,
    0x3e, 0x63, 0x60, 0x60, 0x3c, 0x60, 0x60, 0x60, 0x63, 0x3e,
    0x30, 0x38, 0x3c, 0x36, 0x33, 0x7f, 0x30, 0x30, 0x30, 0x78,
    0x7f, 0x03, 0x03, 0x03, 0x3f, 0x60, 0x60, 0x60, 0x63, 0x3e,
    0x1c, 0x06, 0x03, 0x03, 0x3f, 0x63, 0x63, 0x63, 0x63, 0x3e,
    0x7f, 0x63, 0x60, 0x60, 0x30, 0x18, 0x0c, 0x0c, 0x0c, 0x0c,
    0x3e, 0x63, 0x63, 0x63, 0x3e, 0x63, 0x63, 0x63, 0x63, 0x3e,
    0x3e, 0x63, 0x63, 0x63, 0x7e, 0x60, 0x60, 0x60, 0x30, 0x1e,
  };
  const uint8 *p = kFont + digit * 10;
  if (!big) {
    for (int y = 0; y < 10; y++, dst += pitch) {
      int v = *p++;
      for (int x = 0; v; x++, v >>= 1) {
        if (v & 1)
          ((uint32 *)dst)[x] = color;
      }
    }
  } else {
    for (int y = 0; y < 10; y++, dst += pitch * 2) {
      int v = *p++;
      for (int x = 0; v; x++, v >>= 1) {
        if (v & 1) {
          ((uint32 *)dst)[x * 2 + 1] = ((uint32 *)dst)[x * 2] = color;
          ((uint32 *)(dst+pitch))[x * 2 + 1] = ((uint32 *)(dst + pitch))[x * 2] = color;
        }
      }
    }
  }
}

static void RenderNumber(uint8 *dst, size_t pitch, int n, bool big) {
  char buf[32], *s;
  int i;
  sprintf(buf, "%d", n);
  for (s = buf, i = 2 * 4; *s; s++, i += 8 * 4)
    RenderDigit(dst + ((pitch + i + 4) << big), pitch, *s - '0', 0x404040, big);
  for (s = buf, i = 2 * 4; *s; s++, i += 8 * 4)
    RenderDigit(dst + (i << big), pitch, *s - '0', 0xffffff, big);
}

static void RenderScreen(SDL_Window *window, SDL_Renderer *renderer, SDL_Texture *texture, bool fullscreen) {
  uint8* pixels = NULL;
  int pitch = 0;
  uint64 t0 = SDL_GetPerformanceCounter();
  if(SDL_LockTexture(texture, NULL, (void**)&pixels, &pitch) != 0) {
    printf("Failed to lock texture: %s\n", SDL_GetError());
    return;
  }
  uint64 t1 = SDL_GetPerformanceCounter();
  bool hq = RenderScreenWithPerf(pixels, pitch, g_ppu_render_flags);
  if (g_display_perf) {
    RenderNumber(pixels + (pitch * 2 << hq), pitch, g_curr_fps, hq);
  }
  if (g_config.display_perf_title) {
    char title[60];
    snprintf(title, sizeof(title), "%s | FPS: %d", kWindowTitle, g_curr_fps);
    SDL_SetWindowTitle(window, title);
  }
  uint64 t2 = SDL_GetPerformanceCounter();
  SDL_UnlockTexture(texture);
  uint64 t3 = SDL_GetPerformanceCounter();
  SDL_RenderClear(renderer);
  uint64 t4 = SDL_GetPerformanceCounter();
  SDL_Rect src_rect = { 0, 0, kRenderWidth, kRenderHeight };
  SDL_RenderCopy(renderer, texture, hq ? NULL : &src_rect, NULL);
  uint64 t5 = SDL_GetPerformanceCounter();

  double f = 1e3 / (double)SDL_GetPerformanceFrequency();
  if (0) printf("RenderPerf %6.2f %6.2f %6.2f %6.2f %6.2f\n",
    (t1 - t0) * f,
    (t2 - t1) * f,
    (t3 - t2) * f,
    (t4 - t3) * f,
    (t5 - t4) * f
  );


}

static void HandleCommand(uint32 j, bool pressed) {
  if (j <= kKeys_Controls_Last) {
    static const uint8 kKbdRemap[12] = { 4, 5, 6, 7, 2, 3, 8, 0, 9, 1, 10, 11 };
    SetButtonState(kKbdRemap[j], pressed);
    return;
  }
  if (!pressed)
    return;
  if (j <= kKeys_Load_Last) {
    SaveLoadSlot(kSaveLoad_Load, j - kKeys_Load);
  } else if (j <= kKeys_Save_Last) {
    SaveLoadSlot(kSaveLoad_Save, j - kKeys_Save);
  } else if (j <= kKeys_Replay_Last) {
    SaveLoadSlot(kSaveLoad_Replay, j - kKeys_Replay);
  } else if (j <= kKeys_LoadRef_Last) {
    SaveLoadSlot(kSaveLoad_Load, 256 + j - kKeys_LoadRef);
  } else if (j <= kKeys_ReplayRef_Last) {
    SaveLoadSlot(kSaveLoad_Replay, 256 + j - kKeys_ReplayRef);
  } else {
    switch (j) {
    case kKeys_CheatLife: PatchCommand('w'); break;
    case kKeys_CheatEquipment: PatchCommand('W'); break;
    case kKeys_CheatKeys: PatchCommand('o'); break;
    case kKeys_ClearKeyLog: PatchCommand('k'); break;
    case kKeys_StopReplay: PatchCommand('l'); break;
    case kKeys_Fullscreen:
      g_win_flags ^= SDL_WINDOW_FULLSCREEN_DESKTOP;
      SDL_SetWindowFullscreen(g_window, g_win_flags & SDL_WINDOW_FULLSCREEN_DESKTOP);
      g_cursor = !g_cursor;
      SDL_ShowCursor(g_cursor);
      break;
    case kKeys_Reset:
      snes_reset(g_snes, true);
      ZeldaReadSram(g_snes);
      CopyStateAfterSnapshotRestore(true);
      break;
    case kKeys_Pause: g_paused = !g_paused; break;
    case kKeys_PauseDimmed: 
      g_paused = !g_paused;
      if (g_paused) {
        SDL_SetRenderDrawBlendMode(g_renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 159);
        SDL_RenderFillRect(g_renderer, NULL);
        SDL_RenderPresent(g_renderer);
      }
      break;
    case kKeys_Turbo: g_turbo = !g_turbo; break;
    case kKeys_WindowBigger: ChangeWindowScale(1); break;
    case kKeys_WindowSmaller: ChangeWindowScale(-1); break;
    case kKeys_DisplayPerf: g_display_perf ^= 1; break;
    case kKeys_ToggleRenderer: g_ppu_render_flags ^= kPpuRenderFlags_NewRenderer; break;
    default: assert(0);
    }
  }
}

static void HandleInput(int keyCode, int keyMod, bool pressed) {
  int j = FindCmdForSdlKey(keyCode, keyMod);
  if (j >= 0)
    HandleCommand(j, pressed);
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

// Approximates atan2(y, x) normalized to the [0,4) range
// with a maximum error of 0.1620 degrees
// normalized_atan(x) ~ (b x + x^2) / (1 + 2 b x + x^2)
static float ApproximateAtan2(float y, float x) {
  uint32 sign_mask = 0x80000000;
  float b = 0.596227f;
  // Extract the sign bits
  uint32 ux_s = sign_mask & *(uint32 *)&x;
  uint32 uy_s = sign_mask & *(uint32 *)&y;
  // Determine the quadrant offset
  float q = (float)((~ux_s & uy_s) >> 29 | ux_s >> 30);
  // Calculate the arctangent in the first quadrant
  float bxy_a = fabs(b * x * y);
  float num = bxy_a + y * y;
  float atan_1q = num / (x * x + bxy_a + num + 0.000001f);
  // Translate it to the proper quadrant
  uint32_t uatan_2q = (ux_s ^ uy_s) | *(uint32 *)&atan_1q;
  return q + *(float *)&uatan_2q;
}

static void HandleGamepadAxisInput(int gamepad_id, int axis, int value) {
  static int last_gamepad_id, last_x, last_y;
  if (axis == SDL_CONTROLLER_AXIS_LEFTX || axis == SDL_CONTROLLER_AXIS_LEFTY) {
    // ignore other gamepads unless they have a big input
    if (last_gamepad_id != gamepad_id) {
      if (value > -16000 && value < 16000)
        return;
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
      uint8 angle = (uint8)(int)(ApproximateAtan2(last_y, last_x) * 64.0f + 0.5f);
      buttons = kSegmentToButtons[(uint8)(angle + 16 + 64) >> 5];
    }
    g_gamepad_buttons = buttons;
  }
}

static bool LoadRom(const char *name, Snes *snes) {
  size_t length = 0;
  uint8 *file = ReadFile(name, &length);
  if(!file) Die("Failed to read file");

  PatchRom(file);

  bool result = snes_loadRom(snes, file, (int)length);
  free(file);
  return result;
}


