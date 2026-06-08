#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL.h>
#ifdef _WIN32
#include "platform/win32/volume_control.h"
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include "snes/ppu.h"

#include "types.h"
#include "variables.h"

#include "zelda_rtl.h"
#include "zelda_cpu_infra.h"

#include "config.h"
#include "assets.h"
#include "load_gfx.h"
#include "util.h"
#include "audio.h"
#ifdef ZELDA3_MULTIPLAYER
#include "player_state.h"
#include "net_transport.h"
#include "net_udp.h"
#include "sprite.h"
#endif

static bool g_run_without_emu = 0;

// Forwards
static bool LoadRom(const char *filename);
static void LoadLinkGraphics();
static void RenderNumber(uint8 *dst, size_t pitch, int n, bool big);
static void HandleInput(int keyCode, int modCode, bool pressed);
static void HandleCommand(uint32 j, bool pressed);
static int RemapSdlButton(int button);
static void HandleGamepadInput(int button, bool pressed);
static void HandleGamepadAxisInput(int gamepad_id, int axis, int value);
static void OpenOneGamepad(int i);
#ifdef ZELDA3_MULTIPLAYER
static void HandleGamepadInput_Player(int player, int button, bool pressed);
static void CloseOneGamepad(SDL_JoystickID joy_id);
#endif
static void HandleVolumeAdjustment(int volume_adjustment);
static void LoadAssets();
static void SwitchDirectory();

enum {
  kDefaultFullscreen = 0,
  kMaxWindowScale = 10,
  kDefaultFreq = 44100,
  kDefaultChannels = 2,
  kDefaultSamples = 2048,
};

static const char kWindowTitle[] = "The Legend of Zelda: A Link to the Past";
static uint32 g_win_flags = SDL_WINDOW_RESIZABLE;
static SDL_Window *g_window;

static uint8 g_paused, g_turbo, g_replay_turbo = true, g_cursor = true;
static uint8 g_current_window_scale;
static uint8 g_gamepad_buttons;
static int g_input1_state;
static bool g_display_perf;
static int g_curr_fps;
static int g_ppu_render_flags = 0;
static int g_snes_width, g_snes_height;
static int g_sdl_audio_mixer_volume = SDL_MIX_MAXVOLUME;
static struct RendererFuncs g_renderer_funcs;
static uint32 g_gamepad_modifiers;
static uint16 g_gamepad_last_cmd[kGamepadBtn_Count];

#ifdef ZELDA3_MULTIPLAYER
// Per-controller state for multiplayer
#define MAX_SDL_CONTROLLERS 4
static SDL_GameController *g_controllers[MAX_SDL_CONTROLLERS];
static SDL_JoystickID g_controller_joy_ids[MAX_SDL_CONTROLLERS]; // joystick instance IDs
static int g_num_controllers = 0;

// Per-player input state
static int g_player_input_state[2];        // keyboard input per player
static uint8 g_player_gamepad_buttons[2];  // gamepad dpad/axis per player
static uint32 g_player_gamepad_modifiers[2];
static uint16 g_player_gamepad_last_cmd[2][kGamepadBtn_Count];
static UdpTransport *g_online_udp = NULL;   // set when online co-op is active
static int g_online_stall_frames = 0;       // consecutive frames the sim didn't advance

// Map a joystick instance ID to a player index (0 or 1)
static int GetPlayerForController(SDL_JoystickID joy_id) {
  // Controllers are assigned in order: first controller = P1, second = P2
  for (int i = 0; i < g_num_controllers && i < 2; i++) {
    if (g_controller_joy_ids[i] == joy_id)
      return i;
  }
  return 0; // default to P1
}

// P2 keyboard (numpad cluster; requires NumLock ON). These keys are disjoint
// from P1's defaults (arrows + Z X A S C V + Enter/RShift) so both players can
// share one keyboard. `bit` is the control index, matching P1's control order
// (0=Up,1=Down,2=Left,3=Right,4=Select,5=Start,6=A,7=B,8=X,9=Y,10=L,11=R); it is
// then run through the SAME kKbdRemap as P1 so the resulting input bits match.
static int HandleP2KeyInput(int keyCode, bool pressed) {
  int bit = -1;
  switch (keyCode) {
    case SDLK_KP_8:     bit = 0;  break; // Up
    case SDLK_KP_2:     bit = 1;  break; // Down
    case SDLK_KP_4:     bit = 2;  break; // Left
    case SDLK_KP_6:     bit = 3;  break; // Right
    case SDLK_KP_1:     bit = 4;  break; // Select
    case SDLK_KP_3:     bit = 5;  break; // Start
    case SDLK_KP_0:     bit = 6;  break; // A  (lift / run / talk)
    case SDLK_KP_5:     bit = 7;  break; // B  (sword)
    case SDLK_KP_9:     bit = 8;  break; // X  (map)
    case SDLK_KP_7:     bit = 9;  break; // Y  (use item)
    case SDLK_KP_MINUS: bit = 10; break; // L
    case SDLK_KP_PLUS:  bit = 11; break; // R
    default: return 0;
  }
  static const uint8 kKbdRemap[] = { 0, 4, 5, 6, 7, 2, 3, 8, 0, 9, 1, 10, 11 };
  int mapped = kKbdRemap[bit + 1];  // +1: kKbdRemap[0] is the null entry (kKeys_Null)
  if (pressed)
    g_player_input_state[1] |= 1 << mapped;
  else
    g_player_input_state[1] &= ~(1 << mapped);
  return 1;
}
#endif // ZELDA3_MULTIPLAYER

void NORETURN Die(const char *error) {
#if defined(NDEBUG) && defined(_WIN32)
  SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, kWindowTitle, error, NULL);
#endif
  fprintf(stderr, "Error: %s\n", error);
  exit(1);
}

void ChangeWindowScale(int scale_step) {
  if ((SDL_GetWindowFlags(g_window) & (SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_FULLSCREEN | SDL_WINDOW_MINIMIZED | SDL_WINDOW_MAXIMIZED)) != 0)
    return;
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
    int mw = (bounds.w - bl - br + g_snes_width / 4) / g_snes_width;
    int mh = (bounds.h - bt - bb + g_snes_height / 4) / g_snes_height;
    max_scale = IntMin(mw, mh);
  }
  int new_scale = IntMax(IntMin(g_current_window_scale + scale_step, max_scale), 1);
  g_current_window_scale = new_scale;
  int w = new_scale * g_snes_width;
  int h = new_scale * g_snes_height;

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

#define RESIZE_BORDER 20
static SDL_HitTestResult HitTestCallback(SDL_Window *win, const SDL_Point *pt, void *data) {
  uint32 flags = SDL_GetWindowFlags(win);
  if ((flags & SDL_WINDOW_FULLSCREEN_DESKTOP) != 0 || (flags & SDL_WINDOW_FULLSCREEN) != 0)
    return SDL_HITTEST_NORMAL;

  if ((SDL_GetModState() & KMOD_CTRL) != 0)
    return SDL_HITTEST_DRAGGABLE;

  int w, h;
  SDL_GetWindowSize(win, &w, &h);

  if (pt->y < RESIZE_BORDER) {
    return (pt->x < RESIZE_BORDER) ? SDL_HITTEST_RESIZE_TOPLEFT :
           (pt->x >= w - RESIZE_BORDER) ? SDL_HITTEST_RESIZE_TOPRIGHT : SDL_HITTEST_RESIZE_TOP;
  } else if (pt->y >= h - RESIZE_BORDER) {
    return (pt->x < RESIZE_BORDER) ? SDL_HITTEST_RESIZE_BOTTOMLEFT :
           (pt->x >= w - RESIZE_BORDER) ? SDL_HITTEST_RESIZE_BOTTOMRIGHT : SDL_HITTEST_RESIZE_BOTTOM;
  } else {
    if (pt->x < RESIZE_BORDER) {
      return SDL_HITTEST_RESIZE_LEFT;
    } else if (pt->x >= w - RESIZE_BORDER) {
      return SDL_HITTEST_RESIZE_RIGHT;
    }
  }
  return SDL_HITTEST_NORMAL;
}

static void DrawPpuFrameWithPerf() {
  int render_scale = PpuGetCurrentRenderScale(g_zenv.ppu, g_ppu_render_flags);
  uint8 *pixel_buffer = 0;
  int pitch = 0;

  g_renderer_funcs.BeginDraw(g_snes_width * render_scale,
                             g_snes_height * render_scale,
                             &pixel_buffer, &pitch);
  if (g_display_perf || g_config.display_perf_title) {
    static float history[64], average;
    static int history_pos;
    uint64 before = SDL_GetPerformanceCounter();
    ZeldaDrawPpuFrame(pixel_buffer, pitch, g_ppu_render_flags);
    uint64 after = SDL_GetPerformanceCounter();
    float v = (double)SDL_GetPerformanceFrequency() / (after - before);
    average += v - history[history_pos];
    history[history_pos] = v;
    history_pos = (history_pos + 1) & 63;
    g_curr_fps = average * (1.0f / 64);
  } else {
    ZeldaDrawPpuFrame(pixel_buffer, pitch, g_ppu_render_flags);
  }
  if (g_display_perf)
    RenderNumber(pixel_buffer + pitch * render_scale, pitch, g_curr_fps, render_scale == 4);
  g_renderer_funcs.EndDraw();
}

static SDL_mutex *g_audio_mutex;
static uint8 *g_audiobuffer, *g_audiobuffer_cur, *g_audiobuffer_end;
static int g_frames_per_block;
static uint8 g_audio_channels;

static void SDLCALL AudioCallback(void *userdata, Uint8 *stream, int len) {
  if (SDL_LockMutex(g_audio_mutex)) Die("Mutex lock failed!");
  while (len != 0) {
    if (g_audiobuffer_end - g_audiobuffer_cur == 0) {
      ZeldaRenderAudio((int16*)g_audiobuffer, g_frames_per_block, g_audio_channels);
      g_audiobuffer_cur = g_audiobuffer;
      g_audiobuffer_end = g_audiobuffer + g_frames_per_block * g_audio_channels * sizeof(int16);
    }
    int n = IntMin(len, g_audiobuffer_end - g_audiobuffer_cur);
    if (g_sdl_audio_mixer_volume == SDL_MIX_MAXVOLUME) {
      memcpy(stream, g_audiobuffer_cur, n);
    } else {
      SDL_memset(stream, 0, n);
      SDL_MixAudioFormat(stream, g_audiobuffer_cur, AUDIO_S16, n, g_sdl_audio_mixer_volume);
    }
    g_audiobuffer_cur += n;
    stream += n;
    len -= n;
  }

  ZeldaDiscardUnusedAudioFrames();
  SDL_UnlockMutex(g_audio_mutex);
}

// State for sdl renderer
static SDL_Renderer *g_renderer;
static SDL_Texture *g_texture;
static SDL_Rect g_sdl_renderer_rect;

static bool SdlRenderer_Init(SDL_Window *window) {

  if (g_config.shader)
    fprintf(stderr, "Warning: Shaders are supported only with the OpenGL backend\n");

  SDL_Renderer *renderer = SDL_CreateRenderer(g_window, -1,
                                              g_config.output_method == kOutputMethod_SDLSoftware ? SDL_RENDERER_SOFTWARE :
                                              SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (renderer == NULL) {
    printf("Failed to create renderer: %s\n", SDL_GetError());
    return false;
  }
  SDL_RendererInfo renderer_info;
  SDL_GetRendererInfo(renderer, &renderer_info);
  if (kDebugFlag) {
    printf("Supported texture formats:");
    for (int i = 0; i < renderer_info.num_texture_formats; i++)
      printf(" %s", SDL_GetPixelFormatName(renderer_info.texture_formats[i]));
    printf("\n");
  }
  g_renderer = renderer;
  if (!g_config.ignore_aspect_ratio)
    SDL_RenderSetLogicalSize(renderer, g_snes_width, g_snes_height);
  if (g_config.linear_filtering)
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best");

  int tex_mult = (g_ppu_render_flags & kPpuRenderFlags_4x4Mode7) ? 4 : 1;
  g_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
                                g_snes_width * tex_mult, g_snes_height * tex_mult);
  if (g_texture == NULL) {
    printf("Failed to create texture: %s\n", SDL_GetError());
    return false;
  }
  return true;
}

static void SdlRenderer_Destroy() {
  SDL_DestroyTexture(g_texture);
  SDL_DestroyRenderer(g_renderer);
}

static void SdlRenderer_BeginDraw(int width, int height, uint8 **pixels, int *pitch) {
  g_sdl_renderer_rect.w = width;
  g_sdl_renderer_rect.h = height;
  if (SDL_LockTexture(g_texture, &g_sdl_renderer_rect, (void **)pixels, pitch) != 0) {
    printf("Failed to lock texture: %s\n", SDL_GetError());
    return;
  }
}

static void SdlRenderer_EndDraw() {

//  uint64 before = SDL_GetPerformanceCounter();
  SDL_UnlockTexture(g_texture);
//  uint64 after = SDL_GetPerformanceCounter();
//  float v = (double)(after - before) / SDL_GetPerformanceFrequency();
//  printf("%f ms\n", v * 1000);
  SDL_RenderClear(g_renderer);
  SDL_RenderCopy(g_renderer, g_texture, &g_sdl_renderer_rect, NULL);
  SDL_RenderPresent(g_renderer); // vsyncs to 60 FPS?
}

static const struct RendererFuncs kSdlRendererFuncs  = {
  &SdlRenderer_Init,
  &SdlRenderer_Destroy,
  &SdlRenderer_BeginDraw,
  &SdlRenderer_EndDraw,
};

void OpenGLRenderer_Create(struct RendererFuncs *funcs, bool use_opengl_es);

#ifdef ZELDA3_HEADLESS_TEST
// ---------------------------------------------------------------------------
// Headless verification harness (built only with -DZELDA3_HEADLESS_TEST).
// Reaches live gameplay by replaying a reference save, then drives divergent
// scripted P1/P2 inputs to prove independent control / shared camera / combat,
// dumping framebuffers to /tmp/zharness and logging per-player coords + CRC.
// No SDL window, renderer, or audio device is created.
// ---------------------------------------------------------------------------

// Self-contained 32bpp BMP writer (PPU emits ARGB8888 == BGRA bytes in memory,
// which matches BMP's BGRA pixel order; rows written bottom-up).
static void HeadlessWriteBmp(const char *path, const uint8 *px, int w, int h, size_t pitch) {
  FILE *f = fopen(path, "wb");
  if (!f) return;
  uint32 imgsize = (uint32)w * h * 4, filesize = 54 + imgsize;
  uint8 hdr[54] = {0};
  hdr[0] = 'B'; hdr[1] = 'M';
  memcpy(hdr + 2, &filesize, 4);
  uint32 dataoffs = 54, dibsize = 40, n_imgsize = imgsize; uint16 planes = 1, bpp = 32;
  memcpy(hdr + 10, &dataoffs, 4); memcpy(hdr + 14, &dibsize, 4);
  memcpy(hdr + 18, &w, 4); memcpy(hdr + 22, &h, 4);
  memcpy(hdr + 26, &planes, 2); memcpy(hdr + 28, &bpp, 2);
  memcpy(hdr + 34, &n_imgsize, 4);
  fwrite(hdr, 1, 54, f);
  for (int y = h - 1; y >= 0; y--)
    fwrite(px + (size_t)y * pitch, 1, (size_t)w * 4, f);
  fclose(f);
}

static void HeadlessCapture(const char *path) {
  int w = 256, h = 224;            // basic renderer, scale 1
  size_t pitch = (size_t)w * 4;
  uint8 *buf = calloc((size_t)h, pitch);
  if (!buf) return;
  ZeldaDrawPpuFrame(buf, pitch, 0);
  HeadlessWriteBmp(path, buf, w, h, pitch);
  free(buf);
}

static int RunHeadlessTest(void) {
  setvbuf(stdout, NULL, _IONBF, 0);  // unbuffered so logs survive a kill/timeout

  // UDP transport self-test (ZELDA3_TEST_UDP): stand up a host + client on
  // localhost in-process and verify InputFrames round-trip over real sockets,
  // including the loss-tolerance redundancy + frame de-dup. Exercises the actual
  // wire path the online lockstep uses.
  if (getenv("ZELDA3_TEST_UDP")) {
    // C1 guard: InputRing_Push must refuse to overwrite UNCONSUMED input once
    // full. A stalled online peer used to wrap the ring (INPUT_RING_SIZE) and
    // clobber the oldest unconsumed frame, desyncing on resume. Push past
    // capacity into a never-drained ring and verify it caps and keeps frame 0.
    InputFrame ring_seed = { 0, 0xABCD, 0, 0 };
    int ring_pushed = InputRing_Push(0, &ring_seed) ? 1 : 0;
    for (uint32 f = 1; f < INPUT_RING_SIZE + 50; f++) {
      InputFrame x = { f, (uint16)f, 0, 0 };
      if (InputRing_Push(0, &x)) ring_pushed++;
    }
    InputFrame ring_peek;
    int ring_ok = (ring_pushed == INPUT_RING_SIZE) &&
                  InputRing_Peek(0, 0, &ring_peek) && ring_peek.joypad == 0xABCD;

    UdpTransport host, client;
    if (!Udp_InitHost(&host, 38891, 2) || !Udp_InitClient(&client, "127.0.0.1", 38891, 2)) {
      printf("[harness] UDP TEST: socket init failed (sandbox may block UDP)\n");
      return 1;
    }
    InputFrame rf;
    // 1) Input round-trip + handshake (send() also emits HELLO until handshaked).
    for (uint32 f = 0; f < 5; f++) { InputFrame lf = { f, (uint16)(0x40 + f), 1, 0 }; client.iface.send(&client.iface, &lf); }
    int got = 0; uint32 expect = 0;
    for (int t = 0; t < 1000 && got < 5; t++) { while (host.iface.recv(&host.iface, &rf)) { if (rf.frame_number == expect) { got++; expect++; } } SDL_Delay(1); }
    for (uint32 f = 0; f < 5; f++) { InputFrame hf = { f, (uint16)(0x80 + f), 0, 0 }; host.iface.send(&host.iface, &hf); }
    int g2 = 0; uint32 e2 = 0;
    for (int t = 0; t < 1000 && g2 < 5; t++) { while (client.iface.recv(&client.iface, &rf)) { if (rf.frame_number == e2) { g2++; e2++; } } SDL_Delay(1); }
    // pump a few more recvs so HELLOs are processed both ways
    for (int t = 0; t < 50; t++) { host.iface.send(&host.iface, &rf); client.iface.send(&client.iface, &rf); while (host.iface.recv(&host.iface, &rf)) {} while (client.iface.recv(&client.iface, &rf)) {} SDL_Delay(1); }
    int hs = host.handshaked && client.handshaked && !host.version_mismatch && !client.version_mismatch;
    // 2) Desync detection: send mismatched checksums for the same frame.
    host.iface.send_sync(&host.iface, 600, 0xAAAA1111);
    client.iface.send_sync(&client.iface, 600, 0xBBBB2222);
    for (int t = 0; t < 200 && !(host.desynced && client.desynced); t++) { while (host.iface.recv(&host.iface, &rf)) {} while (client.iface.recv(&client.iface, &rf)) {} SDL_Delay(1); }
    int desync_ok = host.desynced && client.desynced;
    // 3) Clean disconnect: BYE -> peer_lost.
    Udp_SendBye(&host);
    int bye_ok = 0;
    for (int t = 0; t < 200 && !bye_ok; t++) { while (client.iface.recv(&client.iface, &rf)) {} bye_ok = client.peer_lost; SDL_Delay(1); }
    // 4) Packet hardening: a validly-formatted INPUT from the WRONG source plus
    //    several malformed datagrams must all be dropped — the sentinel frame
    //    must never surface, and the session's status must be untouched.
    InputFrame sentinel = { 0x7777u, 0x1234, 1, 0 };
    uint8 spoof[2 + INPUT_FRAME_WIRE_SIZE];
    spoof[0] = NETPKT_INPUT; spoof[1] = 1; InputFrame_Serialize(&sentinel, &spoof[2]);
    Udp_TestRawSendLocal(38891, spoof, sizeof(spoof));         // good format, wrong source
    uint8 junk_type[4]  = { 99, 1, 2, 3 };                     Udp_TestRawSendLocal(38891, junk_type, sizeof(junk_type));    // unknown type
    uint8 junk_count[2] = { NETPKT_INPUT, 200 };               Udp_TestRawSendLocal(38891, junk_count, sizeof(junk_count));  // count past cap
    uint8 junk_trunc[2] = { NETPKT_INPUT, 5 };                 Udp_TestRawSendLocal(38891, junk_trunc, sizeof(junk_trunc));  // claims 5, carries 0
    int spoof_seen = 0;
    for (int t = 0; t < 100; t++) { while (host.iface.recv(&host.iface, &rf)) { if (rf.frame_number == 0x7777u) spoof_seen = 1; } SDL_Delay(1); }
    int hard_ok = !spoof_seen && host.handshaked && !host.version_mismatch;
    int all = (got == 5 && g2 == 5 && hs && desync_ok && bye_ok && hard_ok && ring_ok);
    printf("[harness] UDP TEST: input %d/5,%d/5  handshake=%d  desync_detect=%d  disconnect=%d  hardening=%d  ringcap=%d -> %s\n",
           got, g2, hs, desync_ok, bye_ok, hard_ok, ring_ok, all ? "PASS" : "FAIL");
    return all ? 0 : 1;
  }

  // Self-test the P2 keyboard mapping (this can't be exercised any other way
  // headless — the harness injects inputs directly, bypassing SDL handlers).
  // Expected bits are the input layout: B=0x01,Y=0x02,Sel=0x04,St=0x08,
  // Up=0x10,Dn=0x20,Lt=0x40,Rt=0x80,A=0x100,X=0x200,L=0x400,R=0x800.
  {
    struct { int key, want; const char *name; } t[] = {
      { SDLK_KP_8, 0x10, "Up" }, { SDLK_KP_2, 0x20, "Down" },
      { SDLK_KP_4, 0x40, "Left" }, { SDLK_KP_6, 0x80, "Right" },
      { SDLK_KP_5, 0x01, "B" }, { SDLK_KP_0, 0x100, "A" },
      { SDLK_KP_7, 0x02, "Y" }, { SDLK_KP_9, 0x200, "X" },
      { SDLK_KP_1, 0x04, "Select" }, { SDLK_KP_3, 0x08, "Start" },
      { SDLK_KP_MINUS, 0x400, "L" }, { SDLK_KP_PLUS, 0x800, "R" },
    };
    int fails = 0;
    for (int i = 0; i < 12; i++) {
      g_player_input_state[1] = 0;
      HandleP2KeyInput(t[i].key, true);
      if (g_player_input_state[1] != t[i].want) {
        printf("[harness] P2 KEY MAP FAIL: %-6s got 0x%03x want 0x%03x\n",
               t[i].name, g_player_input_state[1], t[i].want);
        fails++;
      }
      HandleP2KeyInput(t[i].key, false);
    }
    g_player_input_state[1] = 0;
    printf("[harness] P2 keyboard mapping self-test: %s (%d/12 correct)\n",
           fails ? "FAIL" : "PASS", 12 - fails);
  }

  // Force the basic renderer (256x224, scale 1) for deterministic capture.
  g_ppu_render_flags = 0;
  g_snes_width = 256; g_snes_height = 224;
  g_zenv.ppu->extraLeftRight = 0;

  LoadRom("zelda3.sfc");

  // Local co-op mode (same as the normal coop binary configures).
  g_mp_config.mode = MP_MODE_LOCAL;
  g_mp_config.num_players = 2;
  g_mp_config.local_player_index = 0;
  g_mp_config.input_delay_frames = 0;

  if (getenv("ZELDA3_NO_P2")) { g_mp_p2_enabled = false; printf("[harness] P2 DISABLED (single-player isolation)\n"); }
  int slot = 0; const char *e = getenv("ZELDA3_TEST_SAVE"); if (e) slot = atoi(e);
  printf("[harness] loading ref save %d, fast-forwarding replay...\n", slot);
  SaveLoadSlot(kSaveLoad_Load, 256 + slot);

  int guard = 0;
  while (ZeldaRunFrame(0, 0) && ++guard < 1000000) {
    if (guard % 20000 == 0)
      printf("[harness] ...replaying frame %d (module=%d sub=%d)\n",
             guard, main_module_index, submodule_index);
  }
  printf("[harness] replay ended after %d frames: module=%d sub=%d\n",
         guard, main_module_index, submodule_index);
  printf("[harness] after replay: P1=(%d,%d) act=%d | P2=(%d,%d) act=%d\n",
         g_players[0].x_coord, g_players[0].y_coord, g_players[0].is_active,
         g_players[1].x_coord, g_players[1].y_coord, g_players[1].is_active);

  int total = 360; e = getenv("ZELDA3_TEST_FRAMES"); if (e) total = atoi(e);
  // Constant per-player inputs, hex, overridable for probing (e.g. 0x80=right).
  int in1 = 0x80; e = getenv("ZELDA3_P1_INPUT"); if (e) in1 = (int)strtol(e, NULL, 0);
  int in2 = 0x40; e = getenv("ZELDA3_P2_INPUT"); if (e) in2 = (int)strtol(e, NULL, 0);
  // Pickup refill test: knock P2 to 1 heart and seed its heart filler (as a
  // collected heart would), then verify the per-player refill (Multiplayer_RefillP2)
  // converts it to P2 health. Enable with ZELDA3_TEST_PICKUP=1. (The collection
  // itself reuses the engine's proven Sprite_CheckAbsorptionByPlayer; spawning a
  // valid collectible sprite headless needs the full drop init, so we verify the
  // added refill path here directly.)
  if (getenv("ZELDA3_TEST_PICKUP")) {
    g_players[1].health_current = 8;    // 1 heart
    g_players[1].hearts_filler = 24;    // 3 hearts pending (as if collected)
    printf("[harness] PICKUP TEST: P2 hp=8, hearts_filler=24 (expect P2 hp -> 32)\n");
    in1 = 0; in2 = 0;
  }
  if (getenv("ZELDA3_TEST_INVSHARE")) {
    // Give P1 distinctive shared inventory + different health; expect P2 to
    // inherit the items/rupees/keys but KEEP its own (different) health.
    g_players[0].item_bow = 3; g_players[0].rupees_goal = 150; g_players[0].num_keys = 5;
    g_players[0].health_current = 40; g_players[1].health_current = 8;
    printf("[harness] INVSHARE TEST: P1 bow=3 rupees=150 keys=5 hp=40; P2 hp=8\n");
    printf("[harness]   (expect P2 bow=3 rupees=150 keys=5, P2 hp stays ~8)\n");
    in1 = 0; in2 = 0;
  }
  if (getenv("ZELDA3_TEST_LIFT")) {
    // Verify a P2-carried object follows P2 (the carry-owner fix). Separate the
    // players, spawn a carried sprite tagged to P2 AT P1's position, then run a
    // couple frames: the carried-sprite handler should reposition it to P2's
    // hands (near P2), not P1's. Without the fix it would track P1.
    g_players[1].x_coord = g_players[0].x_coord + 80;  // P2 to the right of P1 (within leash)
    g_players[1].y_coord = g_players[0].y_coord;
    PlayerState_SetCurrent(1);                          // spawn as P2 -> owner = P2
    Sprite_SpawnThrowableTerrain(0, g_players[0].x_coord, g_players[0].y_coord);
    PlayerState_SetCurrent(0);
    for (int fr = 0; fr < 2; fr++) ZeldaRunFrame(0, 0);
    int kc = -1; for (int k = 0; k < 16; k++) if (sprite_state[k] == 10) { kc = k; break; }
    if (kc < 0) { printf("[harness] LIFT TEST: no carried sprite spawned -> FAIL\n"); return 1; }
    int sx = sprite_x_lo[kc] | (sprite_x_hi[kc] << 8);
    int sy = sprite_y_lo[kc] | (sprite_y_hi[kc] << 8);
    int dP1 = abs(sx - (int)g_players[0].x_coord) + abs(sy - (int)g_players[0].y_coord);
    int dP2 = abs(sx - (int)g_players[1].x_coord) + abs(sy - (int)g_players[1].y_coord);
    printf("[harness] LIFT TEST: P1=(%d,%d) P2=(%d,%d) carried@(%d,%d) distP1=%d distP2=%d -> %s\n",
           g_players[0].x_coord, g_players[0].y_coord, g_players[1].x_coord, g_players[1].y_coord,
           sx, sy, dP1, dP2, (dP2 < dP1) ? "FOLLOWS P2 (PASS)" : "FOLLOWS P1 (FAIL)");
    return (dP2 < dP1) ? 0 : 1;
  }
  if (getenv("ZELDA3_TEST_DEATH")) {
    // Down P2 and verify: PreventGameOver suppresses game-over + ghosts P2; the
    // ghost stays invulnerable/0-HP/frozen; then auto-respawns beside P1 once the
    // timer elapses. Place P2 far enough that revive-on-touch won't fire first.
    PlayerState_SetCurrent(1);
    cur_player->health_current = 0;
    bool suppressed = Multiplayer_PreventGameOver();
    PlayerState_SetCurrent(0);
    g_players[1].x_coord = g_players[0].x_coord + 80;
    g_players[1].y_coord = g_players[0].y_coord + 80;
    printf("[harness] DEATH TEST: PreventGameOver=%d (expect 1) P2 is_dead=%d rt=%d (expect 1, ~240)\n",
           suppressed, g_players[1].is_dead, g_players[1].respawn_timer);
    in1 = 0; in2 = 0;
  }
  printf("[harness] driving P1=0x%x P2=0x%x for %d frames\n", in1, in2, total);
  for (int k = 0; k < 16; k++) if (sprite_state[k] == 9)
    printf("[harness]   sprite[%d] type=0x%02x pos=(%d,%d) hp=%d\n", k, sprite_type[k],
           sprite_x_lo[k] | (sprite_x_hi[k] << 8), sprite_y_lo[k] | (sprite_y_hi[k] << 8), sprite_health[k]);
  // Optional: pulse the B button (tap) every 16 frames so the sword actually
  // swings instead of charging a spin attack. Enable with ZELDA3_PULSE_B=1.
  int pulse_b = getenv("ZELDA3_PULSE_B") ? 1 : 0;

  if (getenv("ZELDA3_TEST_NET")) {
    // Drive the sim through the ONLINE input-lockstep pipeline (loopback
    // transport) instead of the direct path. Proves the pipeline is
    // behavior-neutral: at input_delay=0 the final CRC must equal the direct
    // path's CRC for the same inputs (run a normal harness to compare). delay>0
    // must stay deterministic run-to-run. This exercises the real wire format
    // (frames are serialized/deserialized through the loopback FIFO).
    int delay = getenv("ZELDA3_NET_DELAY") ? atoi(getenv("ZELDA3_NET_DELAY")) : 0;
    static LoopbackTransport lt;
    Loopback_Init(&lt);
    g_mp_config.mode = MP_MODE_HOST;                  // online co-op, host = P1 local
    Multiplayer_LockstepInit(&lt.iface, 0, delay);
    printf("[harness] NET TEST: lockstep via loopback, local=P1, input_delay=%d, P1=0x%x P2=0x%x\n",
           delay, in1, in2);
    for (int t = 0; t < total + delay; t++) {
      // The remote peer (P2) transmits its input for this tick over the "wire".
      InputFrame rf; rf.frame_number = (uint32)t; rf.joypad = (uint16)in2;
      rf.player_index = 1; rf.flags = 0;
      Loopback_Inject(&lt, &rf);
      Multiplayer_LockstepTick((uint16)in1);
    }
    HeadlessCapture("/tmp/zharness/net_final.bmp");  // render once (matches the direct
                                                     // path's final capture) so render-only
                                                     // side effects line up for comparison.
    SyncChecksum nc = Multiplayer_ComputeChecksum();
    // At input_delay=0 this CRC must equal the direct-path "done" CRC for the
    // same inputs (proves the lockstep pipeline is behavior-neutral). delay>0
    // must be identical run-to-run (proves the input buffer is deterministic).
    printf("[harness] NET RESULT: sim_frames=%u CRC=%08x P1=(%d,%d) P2=(%d,%d)\n",
           g_sim_frame, nc.checksum, g_players[0].x_coord, g_players[0].y_coord,
           g_players[1].x_coord, g_players[1].y_coord);
    return 0;
  }

  mkdir("/tmp/zharness", 0755);
  for (int fr = 0; fr < total; fr++) {
    int a1 = in1, a2 = in2;
    if (pulse_b) {  // tap B (sword) for 2 of every 16 frames
      int b = ((fr % 16) < 2) ? 0x01 : 0;
      a1 = (in1 & ~0x01) | b;
      a2 = (in2 & ~0x01) | b;
    }
    ZeldaRunFrame(a1, a2);
    if (fr % 10 == 0) {
      char path[160]; sprintf(path, "/tmp/zharness/frame_%03d.bmp", fr);
      HeadlessCapture(path);
      PlayerState *p1 = &g_players[0], *p2 = &g_players[1];
      printf("[harness] f=%3d mod=%d sub=%d | P1=(%d,%d hp=%d dead=%d rt=%d) P2=(%d,%d hp=%d dead=%d rt=%d)\n", fr,
             main_module_index, submodule_index,
             p1->x_coord, p1->y_coord, p1->health_current, p1->is_dead, p1->respawn_timer,
             p2->x_coord, p2->y_coord, p2->health_current, p2->is_dead, p2->respawn_timer);
    }
  }
  HeadlessCapture("/tmp/zharness/final.bmp");
  SyncChecksum fc = Multiplayer_ComputeChecksum();
  printf("[harness] done (%d scripted frames). final g_ram CRC=%08x P1=(%d,%d) P2=(%d,%d)\n",
         total, fc.checksum, g_players[0].x_coord, g_players[0].y_coord,
         g_players[1].x_coord, g_players[1].y_coord);
  if (getenv("ZELDA3_TEST_INVSHARE"))
    printf("[harness] INVSHARE RESULT: P2 bow=%d rupees=%d keys=%d hp=%d | P1 hp=%d\n",
           g_players[1].item_bow, g_players[1].rupees_goal, g_players[1].num_keys,
           g_players[1].health_current, g_players[0].health_current);
  return 0;
}
#endif  // ZELDA3_HEADLESS_TEST

#undef main
int main(int argc, char** argv) {
  argc--, argv++;
  const char *config_file = NULL;
  if (argc >= 2 && strcmp(argv[0], "--config") == 0) {
    config_file = argv[1];
    argc -= 2, argv += 2;
  } else {
    SwitchDirectory();
  }
  ParseConfigFile(config_file);
  LoadAssets();
  LoadLinkGraphics();

  ZeldaInitialize();
  g_zenv.ppu->extraLeftRight = UintMin(g_config.extended_aspect_ratio, kPpuExtraLeftRight);
  g_snes_width = (g_config.extended_aspect_ratio * 2 + 256);
  g_snes_height = (g_config.extend_y ? 240 : 224);


  // Delay actually setting those features in ram until any snapshots finish playing.
  g_wanted_zelda_features = g_config.features0;

  g_ppu_render_flags = g_config.new_renderer * kPpuRenderFlags_NewRenderer |
                       g_config.enhanced_mode7 * kPpuRenderFlags_4x4Mode7 |
                       g_config.extend_y * kPpuRenderFlags_Height240 |
                       g_config.no_sprite_limits * kPpuRenderFlags_NoSpriteLimits;
  ZeldaEnableMsu(g_config.enable_msu);
  ZeldaSetLanguage(g_config.language);

#ifdef ZELDA3_HEADLESS_TEST
  // Engine is initialized; run the headless verification harness and exit
  // without ever creating an SDL window, renderer, or audio device.
  return RunHeadlessTest();
#endif

  if (g_config.fullscreen == 1)
    g_win_flags ^= SDL_WINDOW_FULLSCREEN_DESKTOP;
  else if (g_config.fullscreen == 2)
    g_win_flags ^= SDL_WINDOW_FULLSCREEN;

  // Window scale (1=100%, 2=200%, 3=300%, etc.)
  g_current_window_scale = (g_config.window_scale == 0) ? 2 : IntMin(g_config.window_scale, kMaxWindowScale);

  // audio_freq: Use common sampling rates (see user config file. values higher than 48000 are not supported.)
  if (g_config.audio_freq < 11025 || g_config.audio_freq > 48000)
    g_config.audio_freq = kDefaultFreq;

  // Currently, the SPC/DSP implementation only supports up to stereo.
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

  bool custom_size  = g_config.window_width != 0 && g_config.window_height != 0;
  int window_width  = custom_size ? g_config.window_width  : g_current_window_scale * g_snes_width;
  int window_height = custom_size ? g_config.window_height : g_current_window_scale * g_snes_height;

  if (g_config.output_method == kOutputMethod_OpenGL ||
      g_config.output_method == kOutputMethod_OpenGL_ES) {
    g_win_flags |= SDL_WINDOW_OPENGL;
    OpenGLRenderer_Create(&g_renderer_funcs, (g_config.output_method == kOutputMethod_OpenGL_ES));
  } else {
    g_renderer_funcs = kSdlRendererFuncs;
  }

  SDL_Window* window = SDL_CreateWindow(kWindowTitle, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, window_width, window_height, g_win_flags);
  if(window == NULL) {
    printf("Failed to create window: %s\n", SDL_GetError());
    return 1;
  }
  g_window = window;
  SDL_SetWindowHitTest(window, HitTestCallback, NULL);

  if (!g_renderer_funcs.Initialize(window))
    return 1;

  SDL_AudioDeviceID device = 0;
  SDL_AudioSpec want = { 0 }, have;
  g_audio_mutex = SDL_CreateMutex();
  if (!g_audio_mutex) Die("No mutex");

  if (g_config.enable_audio) {
    want.freq = g_config.audio_freq;
    want.format = AUDIO_S16;
    want.channels = g_config.audio_channels;
    want.samples = g_config.audio_samples;
    want.callback = &AudioCallback;
    device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if (device == 0) {
      printf("Failed to open audio device: %s\n", SDL_GetError());
      return 1;
    }
    g_audio_channels = have.channels;
    g_frames_per_block = (534 * have.freq) / 32000;
    g_audiobuffer = malloc(g_frames_per_block * have.channels * sizeof(int16));
  }

  if (argc >= 1 && !g_run_without_emu)
    LoadRom(argv[0]);

#if defined(_WIN32)
  _mkdir("saves");
#else
  mkdir("saves", 0755);
#endif

  ZeldaReadSram();

#ifdef ZELDA3_MULTIPLAYER
  // Initialize multiplayer in local co-op mode
  g_mp_config.mode = MP_MODE_LOCAL;
  g_mp_config.num_players = 2;
  g_mp_config.local_player_index = 0;
  g_mp_config.input_delay_frames = 0;
  memset(g_player_input_state, 0, sizeof(g_player_input_state));
  memset(g_player_gamepad_buttons, 0, sizeof(g_player_gamepad_buttons));
  memset(g_player_gamepad_modifiers, 0, sizeof(g_player_gamepad_modifiers));
  memset(g_player_gamepad_last_cmd, 0, sizeof(g_player_gamepad_last_cmd));

  // Online co-op (optional): "--host [port]" or "--connect <ip> [port]", plus
  // "--net-delay N" (input-delay frames; default 2). Without these flags the
  // game stays in local co-op. Both peers run the identical deterministic sim;
  // only 8-byte InputFrames are exchanged (see NET_ONLINE.md).
  {
    static UdpTransport s_udp;
    const char *connect_ip = NULL;
    int want_host = 0, port = 0, delay = 2;
    for (int i = 0; i < argc; i++) {
      if (strcmp(argv[i], "--host") == 0) {
        want_host = 1;
        if (i + 1 < argc && argv[i + 1][0] != '-') port = atoi(argv[++i]);
      } else if (strcmp(argv[i], "--connect") == 0 && i + 1 < argc) {
        connect_ip = argv[++i];
        if (i + 1 < argc && argv[i + 1][0] != '-') port = atoi(argv[++i]);
      } else if (strcmp(argv[i], "--net-delay") == 0 && i + 1 < argc) {
        delay = atoi(argv[++i]);
      }
    }
    if (port <= 0 || port > 65535) port = 7777;
    int d = (delay < 0) ? 0 : (delay > 10 ? 10 : delay);
    bool online_ok = false;
    if (want_host)          online_ok = Udp_InitHost(&s_udp, (unsigned short)port, d);
    else if (connect_ip)    online_ok = Udp_InitClient(&s_udp, connect_ip, (unsigned short)port, d);
    if (online_ok) {
      g_mp_config.mode = want_host ? MP_MODE_HOST : MP_MODE_CLIENT;
      g_mp_config.local_player_index = want_host ? 0 : 1;  // host drives P1, client P2
      g_mp_config.num_players = 2;
      g_mp_config.input_delay_frames = (uint8)d;
      g_online_udp = &s_udp;
      Multiplayer_LockstepInit(&s_udp.iface, g_mp_config.local_player_index,
                               g_mp_config.input_delay_frames);
      printf("[net] ONLINE co-op: %s, you are Player %d, input_delay=%d frames\n",
             want_host ? "HOST" : "CLIENT", g_mp_config.local_player_index + 1,
             g_mp_config.input_delay_frames);
    } else if (want_host || connect_ip) {
      printf("[net] online init FAILED — staying in local co-op\n");
    }
  }
#endif

  for (int i = 0; i < SDL_NumJoysticks(); i++)
    OpenOneGamepad(i);

  bool running = true;
  SDL_Event event;
  uint32 lastTick = SDL_GetTicks();
  uint32 curTick = 0;
  uint32 frameCtr = 0;
  bool audiopaused = true;

  // Online co-op must start both peers from identical state; an autosave-load
  // here would seed host/client from possibly-different save files and desync
  // the lockstep on frame 1. Skip it when online (both peers boot the same).
  bool mp_online = false;
#ifdef ZELDA3_MULTIPLAYER
  mp_online = (g_mp_config.mode == MP_MODE_HOST || g_mp_config.mode == MP_MODE_CLIENT);
#endif
  if (g_config.autosave && !mp_online)
    HandleCommand(kKeys_Load + 0, true);

  while(running) {
    while(SDL_PollEvent(&event)) {
      switch(event.type) {
      case SDL_CONTROLLERDEVICEADDED:
        OpenOneGamepad(event.cdevice.which);
        break;
#ifdef ZELDA3_MULTIPLAYER
      case SDL_CONTROLLERDEVICEREMOVED:
        CloseOneGamepad(event.cdevice.which);
        break;
#endif
      case SDL_CONTROLLERAXISMOTION:
        HandleGamepadAxisInput(event.caxis.which, event.caxis.axis, event.caxis.value);
        break;
      case SDL_CONTROLLERBUTTONDOWN:
      case SDL_CONTROLLERBUTTONUP: {
        int b = RemapSdlButton(event.cbutton.button);
        if (b >= 0) {
#ifdef ZELDA3_MULTIPLAYER
          if (g_mp_config.mode == MP_MODE_LOCAL) {
            int player = GetPlayerForController(event.cbutton.which);
            HandleGamepadInput_Player(player, b, event.type == SDL_CONTROLLERBUTTONDOWN);
          } else
#endif
          HandleGamepadInput(b, event.type == SDL_CONTROLLERBUTTONDOWN);
        }
        break;
      }
      case SDL_MOUSEWHEEL:
        if (SDL_GetModState() & KMOD_CTRL && event.wheel.y != 0)
          ChangeWindowScale(event.wheel.y > 0 ? 1 : -1);
        break;
      case SDL_MOUSEBUTTONDOWN:
        if (event.button.button == SDL_BUTTON_LEFT && event.button.state == SDL_PRESSED && event.button.clicks == 2) {
          if ((g_win_flags & SDL_WINDOW_FULLSCREEN_DESKTOP) == 0 && (g_win_flags & SDL_WINDOW_FULLSCREEN) == 0 && SDL_GetModState() & KMOD_SHIFT) {
            g_win_flags ^= SDL_WINDOW_BORDERLESS;
            SDL_SetWindowBordered(g_window, (g_win_flags & SDL_WINDOW_BORDERLESS) == 0);
          }
        }
        break;
      case SDL_KEYDOWN:
#ifdef ZELDA3_MULTIPLAYER
        if (g_mp_config.mode == MP_MODE_LOCAL && HandleP2KeyInput(event.key.keysym.sym, true))
          break;
#endif
        HandleInput(event.key.keysym.sym, event.key.keysym.mod, true);
        break;
      case SDL_KEYUP:
#ifdef ZELDA3_MULTIPLAYER
        if (g_mp_config.mode == MP_MODE_LOCAL && HandleP2KeyInput(event.key.keysym.sym, false))
          break;
#endif
        HandleInput(event.key.keysym.sym, event.key.keysym.mod, false);
        break;
      case SDL_QUIT:
        running = false;
        break;
      }
    }

    if (g_paused != audiopaused) {
      audiopaused = g_paused;
      if (device)
        SDL_PauseAudioDevice(device, audiopaused);
    }

    if (g_paused) {
      SDL_Delay(16);
      continue;
    }

    // Clear gamepad inputs when joypad directional inputs to avoid wonkiness
    int inputs = g_input1_state;
    if (g_input1_state & 0xf0)
      g_gamepad_buttons = 0;
    inputs |= g_gamepad_buttons;

#ifdef ZELDA3_MULTIPLAYER
    int inputs_p2 = 0;
    if (g_mp_config.mode == MP_MODE_LOCAL) {
      inputs_p2 = g_player_input_state[1];
      if (g_player_input_state[1] & 0xf0)
        g_player_gamepad_buttons[1] = 0;
      inputs_p2 |= g_player_gamepad_buttons[1];
      // Avoid conflicting directions for P2
      if ((inputs_p2 & 0x30) == 0x30) inputs_p2 ^= 0x30;
      if ((inputs_p2 & 0xc0) == 0xc0) inputs_p2 ^= 0xc0;
    }
#endif

    SDL_LockMutex(g_audio_mutex);
#ifdef ZELDA3_MULTIPLAYER
    bool is_replay = false;
    if (g_mp_config.mode == MP_MODE_HOST || g_mp_config.mode == MP_MODE_CLIENT) {
      // Online lockstep: our local controls drive whichever player we are; the
      // driver sends them, ingests the peer's input, and advances the sim only
      // for frames both players' inputs are ready. If it returns 0 it's waiting
      // on the peer — we just render the last frame again this tick.
      int li = inputs;
      if ((li & 0x30) == 0x30) li ^= 0x30;
      if ((li & 0xc0) == 0xc0) li ^= 0xc0;
      int adv = Multiplayer_LockstepTick((uint16)li);
      g_online_stall_frames = (adv > 0) ? 0 : (g_online_stall_frames + 1);
    } else {
      is_replay = ZeldaRunFrame(inputs, inputs_p2);
    }
#else
    bool is_replay = ZeldaRunFrame(inputs);
#endif
    SDL_UnlockMutex(g_audio_mutex);

    frameCtr++;

    if ((g_turbo ^ (is_replay & g_replay_turbo)) && (frameCtr & (g_turbo ? 0xf : 0x7f)) != 0) {
      continue;
    }

    DrawPpuFrameWithPerf();

    if (g_config.display_perf_title) {
      char title[60];
      snprintf(title, sizeof(title), "%s | FPS: %d", kWindowTitle, g_curr_fps);
      SDL_SetWindowTitle(g_window, title);
    }

#ifdef ZELDA3_MULTIPLAYER
    // Surface online status in the window title (most severe first).
    if (g_online_udp) {
      const char *st = NULL;
      if (g_online_udp->version_mismatch) st = "online: INCOMPATIBLE VERSION - cannot play";
      else if (g_online_udp->desynced)    st = "online: DESYNC DETECTED (states diverged)";
      else if (g_online_udp->peer_lost)   st = "online: player disconnected";
      else if (!g_online_udp->handshaked) st = "online: connecting to peer...";
      else if (g_online_stall_frames > 30) st = "online: waiting for player...";
      char t[96];
      if (st) snprintf(t, sizeof(t), "%s - %s", kWindowTitle, st);
      else    snprintf(t, sizeof(t), "%s - online co-op (you are Player %d)",
                       kWindowTitle, g_mp_config.local_player_index + 1);
      SDL_SetWindowTitle(g_window, t);
    }
#endif

    // if vsync isn't working, delay manually
    curTick = SDL_GetTicks();

    if (!g_config.disable_frame_delay) {
      static const uint8 delays[3] = { 17, 17, 16 }; // 60 fps
      lastTick += delays[frameCtr % 3];

      if (lastTick > curTick) {
        uint32 delta = lastTick - curTick;
        if (delta > 500) {
          lastTick = curTick - 500;
          delta = 500;
        }
//        printf("Sleeping %d\n", delta);
        SDL_Delay(delta);
      } else if (curTick - lastTick > 500) {
        lastTick = curTick;
      }
    }
  }
  if (g_config.autosave)
    HandleCommand(kKeys_Save + 0, true);

#ifdef ZELDA3_MULTIPLAYER
  // Tell the peer we're leaving so it shows "player disconnected" promptly.
  if (g_online_udp) Udp_SendBye(g_online_udp);
#endif

  // clean sdl
  if (g_config.enable_audio) {
    SDL_PauseAudioDevice(device, 1);
    SDL_CloseAudioDevice(device);
  }

  SDL_DestroyMutex(g_audio_mutex);
  free(g_audiobuffer);

  g_renderer_funcs.Destroy();

  SDL_DestroyWindow(window);
  SDL_Quit();
  //SaveConfigFile();
  return 0;
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

static void HandleCommand_Locked(uint32 j, bool pressed);

static void HandleCommand(uint32 j, bool pressed) {
  if (j <= kKeys_Controls_Last) {
    static const uint8 kKbdRemap[] = { 0, 4, 5, 6, 7, 2, 3, 8, 0, 9, 1, 10, 11 };
    if (pressed)
      g_input1_state |= 1 << kKbdRemap[j];
    else
      g_input1_state &= ~(1 << kKbdRemap[j]);
    return;
  }

  if (j == kKeys_Turbo) {
    g_turbo = pressed;
    return;
  }

  // Everything that might access audio state
  // (like SaveLoad and Reset) must have the lock.
  SDL_LockMutex(g_audio_mutex);
  HandleCommand_Locked(j, pressed);
  SDL_UnlockMutex(g_audio_mutex);
}

void ZeldaApuLock() {
  SDL_LockMutex(g_audio_mutex);
}

void ZeldaApuUnlock() {
  SDL_UnlockMutex(g_audio_mutex);
}


static void HandleCommand_Locked(uint32 j, bool pressed) {
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
    case kKeys_CheatWalkThroughWalls: PatchCommand('E'); break;
    case kKeys_ClearKeyLog: PatchCommand('k'); break;
    case kKeys_StopReplay: PatchCommand('l'); break;
    case kKeys_Fullscreen:
      g_win_flags ^= SDL_WINDOW_FULLSCREEN_DESKTOP;
      SDL_SetWindowFullscreen(g_window, g_win_flags & SDL_WINDOW_FULLSCREEN_DESKTOP);
      g_cursor = !g_cursor;
      SDL_ShowCursor(g_cursor);
      break;
    case kKeys_Reset:
      ZeldaReset(true);
      break;
    case kKeys_Pause: g_paused = !g_paused; break;
    case kKeys_PauseDimmed:
      g_paused = !g_paused;
      // SDL_RenderPresent may not be called more than once per frame.
      // Seems to work on Windows still. Temporary measure until it's fixed.
#ifdef _WIN32
      if (g_paused) {
        SDL_SetRenderDrawBlendMode(g_renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 159);
        SDL_RenderFillRect(g_renderer, NULL);
        SDL_RenderPresent(g_renderer);
      }
#endif
      break;
    case kKeys_ReplayTurbo: g_replay_turbo = !g_replay_turbo; break;
    case kKeys_WindowBigger: ChangeWindowScale(1); break;
    case kKeys_WindowSmaller: ChangeWindowScale(-1); break;
    case kKeys_DisplayPerf: g_display_perf ^= 1; break;
    case kKeys_ToggleRenderer: g_ppu_render_flags ^= kPpuRenderFlags_NewRenderer; break;
    case kKeys_VolumeUp:
    case kKeys_VolumeDown: HandleVolumeAdjustment(j == kKeys_VolumeUp ? 1 : -1); break;
    default: assert(0);
    }
  }
}

static void HandleInput(int keyCode, int keyMod, bool pressed) {
  int j = FindCmdForSdlKey(keyCode, keyMod);
  if (j != 0)
    HandleCommand(j, pressed);
}

static void OpenOneGamepad(int i) {
  if (SDL_IsGameController(i)) {
    SDL_GameController *controller = SDL_GameControllerOpen(i);
    if (!controller) {
      fprintf(stderr, "Could not open gamepad %d: %s\n", i, SDL_GetError());
      return;
    }
#ifdef ZELDA3_MULTIPLAYER
    if (g_num_controllers < MAX_SDL_CONTROLLERS) {
      SDL_Joystick *joy = SDL_GameControllerGetJoystick(controller);
      g_controllers[g_num_controllers] = controller;
      g_controller_joy_ids[g_num_controllers] = SDL_JoystickInstanceID(joy);
      g_num_controllers++;
      printf("Controller %d assigned to Player %d\n", i, g_num_controllers <= 2 ? g_num_controllers : 0);
    }
#endif
  }
}

#ifdef ZELDA3_MULTIPLAYER
// Handle a controller disconnect: close it, compact the controller table so slot
// order (0=P1, 1=P2) stays contiguous, and clear per-player gamepad state so a
// button held at the moment of disconnect doesn't stay logically pressed (stuck
// input) and stale modifiers from a reassigned slot don't bleed in.
static void CloseOneGamepad(SDL_JoystickID joy_id) {
  int idx = -1;
  for (int i = 0; i < g_num_controllers; i++)
    if (g_controller_joy_ids[i] == joy_id) { idx = i; break; }
  if (idx < 0) return;
  if (g_controllers[idx])
    SDL_GameControllerClose(g_controllers[idx]);
  for (int i = idx; i < g_num_controllers - 1; i++) {
    g_controllers[i] = g_controllers[i + 1];
    g_controller_joy_ids[i] = g_controller_joy_ids[i + 1];
  }
  g_num_controllers--;
  g_controllers[g_num_controllers] = NULL;
  g_controller_joy_ids[g_num_controllers] = 0;
  for (int p = 0; p < 2; p++) {
    g_player_gamepad_buttons[p] = 0;
    g_player_gamepad_modifiers[p] = 0;
    for (int c = 0; c < kGamepadBtn_Count; c++)
      g_player_gamepad_last_cmd[p][c] = 0;
  }
  printf("Controller removed (id %d); %d controller(s) remain\n",
         (int)joy_id, g_num_controllers);
}
#endif

static int RemapSdlButton(int button) {
  switch (button) {
  case SDL_CONTROLLER_BUTTON_A: return kGamepadBtn_A;
  case SDL_CONTROLLER_BUTTON_B: return kGamepadBtn_B;
  case SDL_CONTROLLER_BUTTON_X: return kGamepadBtn_X;
  case SDL_CONTROLLER_BUTTON_Y: return kGamepadBtn_Y;
  case SDL_CONTROLLER_BUTTON_BACK: return kGamepadBtn_Back;
  case SDL_CONTROLLER_BUTTON_GUIDE: return kGamepadBtn_Guide;
  case SDL_CONTROLLER_BUTTON_START: return kGamepadBtn_Start;
  case SDL_CONTROLLER_BUTTON_LEFTSTICK: return kGamepadBtn_L3;
  case SDL_CONTROLLER_BUTTON_RIGHTSTICK: return kGamepadBtn_R3;
  case SDL_CONTROLLER_BUTTON_LEFTSHOULDER: return kGamepadBtn_L1;
  case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: return kGamepadBtn_R1;
  case SDL_CONTROLLER_BUTTON_DPAD_UP: return kGamepadBtn_DpadUp;
  case SDL_CONTROLLER_BUTTON_DPAD_DOWN: return kGamepadBtn_DpadDown;
  case SDL_CONTROLLER_BUTTON_DPAD_LEFT: return kGamepadBtn_DpadLeft;
  case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: return kGamepadBtn_DpadRight;
  default: return -1;
  }
}

static void HandleGamepadInput(int button, bool pressed) {
  if (!!(g_gamepad_modifiers & (1 << button)) == pressed)
    return;
  g_gamepad_modifiers ^= 1 << button;
  if (pressed)
    g_gamepad_last_cmd[button] = FindCmdForGamepadButton(button, g_gamepad_modifiers);
  if (g_gamepad_last_cmd[button] != 0)
    HandleCommand(g_gamepad_last_cmd[button], pressed);
}

#ifdef ZELDA3_MULTIPLAYER
static void HandleGamepadInput_Player(int player, int button, bool pressed) {
  if (player < 0 || player > 1) return;
  if (!!(g_player_gamepad_modifiers[player] & (1 << button)) == pressed)
    return;
  g_player_gamepad_modifiers[player] ^= 1 << button;
  if (player == 0) {
    // P1: route through existing command system
    if (pressed)
      g_player_gamepad_last_cmd[player][button] = FindCmdForGamepadButton(button, g_player_gamepad_modifiers[player]);
    if (g_player_gamepad_last_cmd[player][button] != 0)
      HandleCommand(g_player_gamepad_last_cmd[player][button], pressed);
  } else {
    // P2: translate the gamepad command to an input bit exactly like P1 does in
    // HandleCommand (which indexes kKbdRemap by the FULL key id `cmd`). The old
    // code subtracted kKeys_Controls first, shifting every button by one (e.g.
    // D-pad Up registered as B). Use kKbdRemap[cmd] to match P1.
    if (pressed)
      g_player_gamepad_last_cmd[player][button] = FindCmdForGamepadButton(button, g_player_gamepad_modifiers[player]);
    uint16 cmd = g_player_gamepad_last_cmd[player][button];
    if (cmd >= kKeys_Controls && cmd <= kKeys_Controls_Last) {
      static const uint8 kKbdRemap[] = { 0, 4, 5, 6, 7, 2, 3, 8, 0, 9, 1, 10, 11 };
      if (pressed)
        g_player_input_state[1] |= 1 << kKbdRemap[cmd];
      else
        g_player_input_state[1] &= ~(1 << kKbdRemap[cmd]);
    }
  }
}
#endif

static void HandleVolumeAdjustment(int volume_adjustment) {
#if SYSTEM_VOLUME_MIXER_AVAILABLE
  int current_volume = GetApplicationVolume();
  int new_volume = IntMin(IntMax(0, current_volume + volume_adjustment * 5), 100);
  SetApplicationVolume(new_volume);
  printf("[System Volume]=%i\n", new_volume);
#else
  g_sdl_audio_mixer_volume = IntMin(IntMax(0, g_sdl_audio_mixer_volume + volume_adjustment * (SDL_MIX_MAXVOLUME >> 4)), SDL_MIX_MAXVOLUME);
  printf("[SDL mixer volume]=%i\n", g_sdl_audio_mixer_volume);
#endif
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
  float bxy_a = b * x * y;
  if (bxy_a < 0.0f) bxy_a = -bxy_a;  // avoid fabs
  float num = bxy_a + y * y;
  float atan_1q = num / (x * x + bxy_a + num + 0.000001f);
  // Translate it to the proper quadrant
  uint32_t uatan_2q = (ux_s ^ uy_s) | *(uint32 *)&atan_1q;
  return q + *(float *)&uatan_2q;
}

static void HandleGamepadAxisInput(int gamepad_id, int axis, int value) {
  static int last_gamepad_id, last_x, last_y;
#ifdef ZELDA3_MULTIPLAYER
  if (g_mp_config.mode == MP_MODE_LOCAL) {
    int player = GetPlayerForController(gamepad_id);
    static int mp_last_x[2], mp_last_y[2];
    if (axis == SDL_CONTROLLER_AXIS_LEFTX || axis == SDL_CONTROLLER_AXIS_LEFTY) {
      *(axis == SDL_CONTROLLER_AXIS_LEFTX ? &mp_last_x[player] : &mp_last_y[player]) = value;
      int buttons = 0;
      int lx = mp_last_x[player], ly = mp_last_y[player];
      if (lx * lx + ly * ly >= 10000 * 10000) {
        static const uint8 kSegmentToButtons[8] = {
          1 << 4, 1 << 4 | 1 << 7, 1 << 7, 1 << 7 | 1 << 5,
          1 << 5, 1 << 5 | 1 << 6, 1 << 6, 1 << 6 | 1 << 4,
        };
        uint8 angle = (uint8)(int)(ApproximateAtan2(ly, lx) * 64.0f + 0.5f);
        buttons = kSegmentToButtons[(uint8)(angle + 16 + 64) >> 5];
      }
      g_player_gamepad_buttons[player] = buttons;
      if (player == 0)
        g_gamepad_buttons = buttons;
    } else if (axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT || axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT) {
      if (value < 12000 || value >= 16000)
        HandleGamepadInput_Player(player, axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT ? kGamepadBtn_L2 : kGamepadBtn_R2, value >= 12000);
    }
    return;
  }
#endif
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
  } else if ((axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT || axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT)) {
    if (value < 12000 || value >= 16000)  // hysteresis
      HandleGamepadInput(axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT ? kGamepadBtn_L2 : kGamepadBtn_R2, value >= 12000);
  }
}

static bool LoadRom(const char *filename) {
  size_t length = 0;
  uint8 *file = ReadWholeFile(filename, &length);
  if(!file) Die("Failed to read file");
  bool result = EmuInitialize(file, length);
  free(file);
  return result;
}

static bool ParseLinkGraphics(uint8 *file, size_t length) {
  if (length < 27 || memcmp(file, "ZSPR", 4) != 0)
    return false;
  uint32 pixel_offs = DWORD(file[9]);
  uint32 pixel_length = WORD(file[13]);
  uint32 palette_offs = DWORD(file[15]);
  uint32 palette_length = WORD(file[19]);
  if ((uint64)pixel_offs + pixel_length > length ||
      (uint64)palette_offs + palette_length > length ||
      pixel_length != 0x7000)
    return false;
  if (kPalette_ArmorAndGloves_SIZE != 150 || kLinkGraphics_SIZE != 0x7000)
    Die("ParseLinkGraphics: Invalid asset sizes");
  memcpy(kLinkGraphics, file + pixel_offs, 0x7000);
  if (palette_length >= 120)
    memcpy(kPalette_ArmorAndGloves, file + palette_offs, 120);
  if (palette_length >= 124)
    memcpy(kGlovesColor, file + palette_offs + 120, 4);
  return true;
}

static void LoadLinkGraphics() {
  if (g_config.link_graphics) {
    fprintf(stderr, "Loading Link Graphics: %s\n", g_config.link_graphics);
    size_t length = 0;
    uint8 *file = ReadWholeFile(g_config.link_graphics, &length);
    if (file == NULL || !ParseLinkGraphics(file, length))
      Die("Unable to load file");
    free(file);
  }
}


const uint8 *g_asset_ptrs[kNumberOfAssets];
uint32 g_asset_sizes[kNumberOfAssets];

static void LoadAssets() {
  size_t length = 0;
  uint8 *data = ReadWholeFile("zelda3_assets.dat", &length);
  if (!data) {
    size_t bps_length, bps_src_length;
    uint8 *bps, *bps_src;
    bps = ReadWholeFile("zelda3_assets.bps", &bps_length);
    if (!bps)
      Die("Failed to read zelda3_assets.dat. Please see the README for information about how you get this file.");
    bps_src = ReadWholeFile("zelda3.sfc", &bps_src_length);
    if (!bps_src)
      Die("Missing file: zelda3.sfc");
    data = ApplyBps(bps_src, bps_src_length, bps, bps_length, &length);
    if (!data)
      Die("Unable to apply zelda3_assets.bps. Please make sure you got the right version of 'zelda3.sfc'");
  }

  static const char kAssetsSig[] = { kAssets_Sig };

  if (length < 16 + 32 + 32 + 8 + kNumberOfAssets * 4 ||
      memcmp(data, kAssetsSig, 48) != 0 ||
      *(uint32*)(data + 80) != kNumberOfAssets)
    Die("Invalid assets file");

  uint32 offset = 88 + kNumberOfAssets * 4 + *(uint32 *)(data + 84);

  for (size_t i = 0; i < kNumberOfAssets; i++) {
    uint32 size = *(uint32 *)(data + 88 + i * 4);
    offset = (offset + 3) & ~3;
    if ((uint64)offset + size > length)
      Die("Assets file corruption");
    g_asset_sizes[i] = size;
    g_asset_ptrs[i] = data + offset;
    offset += size;
  }

  if (g_config.features0 & kFeatures0_DimFlashes) { // patch dungeon floor palettes
    kPalette_DungBgMain[0x484] = 0x70;
    kPalette_DungBgMain[0x485] = 0x95;
    kPalette_DungBgMain[0x486] = 0x57;
  }
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
        int err = chdir(buf);
        (void)err;
      }
      return;
    }
    pos--;
    while (pos != 0 && buf[pos] != '/' && buf[pos] != '\\')
      pos--;
  }
}

MemBlk FindInAssetArray(int asset, int idx) {
  return FindIndexInMemblk((MemBlk) { g_asset_ptrs[asset], g_asset_sizes[asset] }, idx);
}
