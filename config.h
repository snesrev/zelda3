#pragma once
#include "types.h"
#include <SDL_keycode.h>

enum {
  kKeys_Controls = 0,
  kKeys_Controls_Last = kKeys_Controls + 11,
  kKeys_Load,
  kKeys_Load_Last = kKeys_Load + 19,
  kKeys_Save,
  kKeys_Save_Last = kKeys_Save + 19,
  kKeys_Replay,
  kKeys_Replay_Last = kKeys_Replay + 19,
  kKeys_LoadRef,
  kKeys_LoadRef_Last = kKeys_LoadRef + 19,
  kKeys_ReplayRef,
  kKeys_ReplayRef_Last = kKeys_ReplayRef + 19,
  kKeys_CheatLife,
  kKeys_CheatKeys,
  kKeys_CheatEquipment,
  kKeys_ClearKeyLog,
  kKeys_StopReplay,
  kKeys_Fullscreen,
  kKeys_Reset,
  kKeys_Pause,
  kKeys_PauseDimmed,
  kKeys_Turbo,
  kKeys_ZoomIn,
  kKeys_ZoomOut,
  kKeys_DisplayPerf,
  kKeys_ToggleRenderer,
  kKeys_Total,
};

typedef struct Config {
  bool enhanced_mode7;
  bool new_renderer;
  bool ignore_aspect_ratio;
} Config;

extern Config g_config;

uint8 *ReadFile(const char *name, size_t *length);
void ParseConfigFile();
void AfterConfigParse();

int FindCmdForSdlKey(SDL_Keycode code, SDL_Keymod mod);