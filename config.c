#include "config.h"
#include "types.h"
#include <stdio.h>
#include <string.h>
#include <SDL.h>
#include "features.h"
#include "util.h"

enum {
  kKeyMod_ScanCode = 0x200,
  kKeyMod_Alt = 0x400,
  kKeyMod_Shift = 0x800,
  kKeyMod_Ctrl = 0x1000,
};

Config g_config;

#define REMAP_SDL_KEYCODE(key) ((key) & SDLK_SCANCODE_MASK ? kKeyMod_ScanCode : 0) | (key) & (kKeyMod_ScanCode - 1)
#define _(x) REMAP_SDL_KEYCODE(x)
#define S(x) REMAP_SDL_KEYCODE(x) | kKeyMod_Shift
#define A(x) REMAP_SDL_KEYCODE(x) | kKeyMod_Alt
#define C(x) REMAP_SDL_KEYCODE(x) | kKeyMod_Ctrl
#define N 0
static const uint16 kDefaultKbdControls[kKeys_Total] = {
  // Controls
  _(SDLK_UP), _(SDLK_DOWN), _(SDLK_LEFT), _(SDLK_RIGHT), _(SDLK_RSHIFT), _(SDLK_RETURN), _(SDLK_x), _(SDLK_z), _(SDLK_s), _(SDLK_a), _(SDLK_c), _(SDLK_v),
  // LoadState
  _(SDLK_F1), _(SDLK_F2), _(SDLK_F3), _(SDLK_F4), _(SDLK_F5), _(SDLK_F6), _(SDLK_F7), _(SDLK_F8), _(SDLK_F9), _(SDLK_F10), N, N, N, N, N, N, N, N, N, N,
  // SaveState
  S(SDLK_F1), S(SDLK_F2), S(SDLK_F3), S(SDLK_F4), S(SDLK_F5), S(SDLK_F6), S(SDLK_F7), S(SDLK_F8), S(SDLK_F9), S(SDLK_F10), N, N, N, N, N, N, N, N, N, N,
  // Replay State
  C(SDLK_F1), C(SDLK_F2), C(SDLK_F3), C(SDLK_F4), C(SDLK_F5), C(SDLK_F6), C(SDLK_F7), C(SDLK_F8), C(SDLK_F9), C(SDLK_F10), N, N, N, N, N, N, N, N, N, N,
  // Load Ref State
  _(SDLK_1), _(SDLK_2), _(SDLK_3), _(SDLK_4), _(SDLK_5), _(SDLK_6), _(SDLK_7), _(SDLK_8), _(SDLK_9), _(SDLK_0), _(SDLK_MINUS), _(SDLK_EQUALS), _(SDLK_BACKSPACE), N, N, N, N, N, N, N,
  // Replay Ref State
  C(SDLK_1), C(SDLK_2), C(SDLK_3), C(SDLK_4), C(SDLK_5), C(SDLK_6), C(SDLK_7), C(SDLK_8), C(SDLK_9), C(SDLK_0), C(SDLK_MINUS), C(SDLK_EQUALS), C(SDLK_BACKSPACE), N, N, N, N, N, N, N,
  // CheatLife, CheatKeys, CheatEquipment, CheatWalkThroughWalls
  _(SDLK_w), _(SDLK_o), S(SDLK_w), C(SDLK_e),
  // ClearKeyLog, StopReplay, Fullscreen, Reset, Pause, PauseDimmed, Turbo, ReplayTurbo, WindowBigger, WindowSmaller, DisplayPerf, ToggleRenderer
  _(SDLK_k), _(SDLK_l), A(SDLK_RETURN), C(SDLK_r), S(SDLK_p), _(SDLK_p), _(SDLK_TAB), _(SDLK_t), N, N, _(SDLK_f), _(SDLK_r),
};
#undef _
#undef A
#undef C
#undef S
#undef N

typedef struct KeyNameId {
  const char *name;
  uint16 id, size;
} KeyNameId;

#define M(n) {#n, kKeys_##n, kKeys_##n##_Last - kKeys_##n + 1}
#define S(n) {#n, kKeys_##n, 1}
static const KeyNameId kKeyNameId[] = {
  M(Controls), M(Load), M(Save), M(Replay), M(LoadRef), M(ReplayRef),
  S(CheatLife), S(CheatKeys), S(CheatEquipment), S(CheatWalkThroughWalls),
  S(ClearKeyLog), S(StopReplay), S(Fullscreen), S(Reset),
  S(Pause), S(PauseDimmed), S(Turbo), S(ReplayTurbo), S(WindowBigger), S(WindowSmaller), S(VolumeUp), S(VolumeDown), S(DisplayPerf), S(ToggleRenderer),
};
#undef S
#undef M
typedef struct KeyMapHashEnt {
  uint16 key, cmd, next;
} KeyMapHashEnt;

static uint16 keymap_hash_first[255];
static KeyMapHashEnt *keymap_hash;
static int keymap_hash_size;
static bool has_keynameid[countof(kKeyNameId)];

bool KeyMapHash_Add(uint16 key, uint16 cmd) {
  if ((keymap_hash_size & 0xff) == 0) {
    if (keymap_hash_size > 10000)
      Die("Too many keys");
    keymap_hash = realloc(keymap_hash, sizeof(KeyMapHashEnt) * (keymap_hash_size + 256));
  }
  int i = keymap_hash_size++;
  KeyMapHashEnt *ent = &keymap_hash[i];
  ent->key = key;
  ent->cmd = cmd;
  ent->next = 0;
  int j = (uint32)key % 255;

  uint16 *cur = &keymap_hash_first[j];
  while (*cur) {
    KeyMapHashEnt *ent = &keymap_hash[*cur - 1];
    if (ent->key == key)
      return false;
    cur = &ent->next;
  }
  *cur = i + 1;
  return true;
}

static int KeyMapHash_Find(uint16 key) {
  int i = keymap_hash_first[key % 255];
  while (i) {
    KeyMapHashEnt *ent = &keymap_hash[i - 1];
    if (ent->key == key)
      return ent->cmd;
    i = ent->next;
  }
  return -1;
}

int FindCmdForSdlKey(SDL_Keycode code, SDL_Keymod mod) {
  if (code & ~(SDLK_SCANCODE_MASK | 0x1ff))
    return -1;
  int key = mod & KMOD_ALT ? kKeyMod_Alt : 0;
  key |= mod & KMOD_CTRL ? kKeyMod_Ctrl : 0;
  key |= mod & KMOD_SHIFT ? kKeyMod_Shift : 0;
  key |= REMAP_SDL_KEYCODE(code);
  return KeyMapHash_Find(key);
}

static void ParseKeyArray(char *value, int cmd, int size) {
  char *s;
  int i = 0;
  for (; i < size && (s = NextDelim(&value, ',')) != NULL; i++, cmd++) {
    if (*s == 0)
      continue;
    int key_with_mod = 0;
    for (;;) {
      if (StringStartsWithNoCase(s, "Shift+")) {
        key_with_mod |= kKeyMod_Shift, s += 6;
      } else if (StringStartsWithNoCase(s, "Ctrl+")) {
        key_with_mod |= kKeyMod_Ctrl, s += 5;
      } else if (StringStartsWithNoCase(s, "Alt+")) {
        key_with_mod |= kKeyMod_Alt, s += 4;
      } else {
        break;
      }
    }
    SDL_Keycode key = SDL_GetKeyFromName(s);
    if (key == SDLK_UNKNOWN) {
      fprintf(stderr, "Unknown key: '%s'\n", s);
      continue;
    }
    if (!KeyMapHash_Add(key_with_mod | REMAP_SDL_KEYCODE(key), cmd))
      fprintf(stderr, "Duplicate key: '%s'\n", s);
  }
}

static void RegisterDefaultKeys() {
  for (int i = 0; i < countof(kKeyNameId); i++) {
    if (!has_keynameid[i]) {
      int size = kKeyNameId[i].size, k = kKeyNameId[i].id;
      for (int j = 0; j < size; j++, k++)
        KeyMapHash_Add(kDefaultKbdControls[k], k);
    }
  }
}

static int GetIniSection(const char *s) {
  if (StringEqualsNoCase(s, "[KeyMap]"))
    return 0;
  if (StringEqualsNoCase(s, "[Graphics]"))
    return 1;
  if (StringEqualsNoCase(s, "[Sound]"))
    return 2;
  if (StringEqualsNoCase(s, "[General]"))
    return 3;
  if (StringEqualsNoCase(s, "[Features]"))
    return 4;
  return -1;
}

bool ParseBool(const char *value, bool *result) {
  bool rv = false;
  switch (*value++ | 32) {
  case '0': if (*value == 0) break; return false;
  case 'f': if (StringEqualsNoCase(value, "alse")) break; return false;
  case 'n': if (StringEqualsNoCase(value, "o")) break; return false;
  case 'o':
    rv = (*value | 32) == 'n';
    if (StringEqualsNoCase(value, rv ? "n" : "ff")) break;
    return false;
  case '1': rv = true; if (*value == 0) break; return false;
  case 'y': rv = true; if (StringEqualsNoCase(value, "es")) break; return false;
  case 't': rv = true; if (StringEqualsNoCase(value, "rue")) break; return false;
  default: return false;
  }
  if (result) {
    *result = rv;
    return true;
  }
  return rv;
}

static bool ParseBoolBit(const char *value, uint32 *data, uint32 mask) {
  bool tmp;
  if (!ParseBool(value, &tmp))
    return false;
  *data = *data & ~mask | (tmp ? mask : 0);
  return true;
}

static bool HandleIniConfig(int section, const char *key, char *value) {
  if (section == 0) {
    for (int i = 0; i < countof(kKeyNameId); i++) {
      if (StringEqualsNoCase(key, kKeyNameId[i].name)) {
        has_keynameid[i] = true;
        ParseKeyArray(value, kKeyNameId[i].id, kKeyNameId[i].size);
        return true;
      }
    }
  } else if (section == 1) {
    if (StringEqualsNoCase(key, "WindowSize")) {
      char *s;
      if (StringEqualsNoCase(value, "Auto")){
        g_config.window_width  = 0;
        g_config.window_height = 0;
        return true;
      }
      while ((s = NextDelim(&value, 'x')) != NULL) {
        if(g_config.window_width == 0) {
          g_config.window_width = atoi(s);
        } else {
          g_config.window_height = atoi(s);
          return true;
        }
      }
    } else if (StringEqualsNoCase(key, "EnhancedMode7")) {
      return ParseBool(value, &g_config.enhanced_mode7);
    } else if (StringEqualsNoCase(key, "NewRenderer")) {
      return ParseBool(value, &g_config.new_renderer);
    } else if (StringEqualsNoCase(key, "IgnoreAspectRatio")) {
      return ParseBool(value, &g_config.ignore_aspect_ratio);
    } else if (StringEqualsNoCase(key, "Fullscreen")) {
      g_config.fullscreen = (uint8)strtol(value, (char**)NULL, 10);
      return true;
    } else if (StringEqualsNoCase(key, "WindowScale")) {
      g_config.window_scale = (uint8)strtol(value, (char**)NULL, 10);
      return true;
    } else if (StringEqualsNoCase(key, "OutputMethod")) {
      g_config.output_method = StringEqualsNoCase(value, "SDL-Software") ? kOutputMethod_SDLSoftware :
                               StringEqualsNoCase(value, "OpenGL") ? kOutputMethod_OpenGL : kOutputMethod_SDL;
      return true;
    } else if (StringEqualsNoCase(key, "LinearFiltering")) {
      return ParseBool(value, &g_config.linear_filtering);
    } else if (StringEqualsNoCase(key, "NoSpriteLimits")) {
      return ParseBool(value, &g_config.no_sprite_limits);
    } else if (StringEqualsNoCase(key, "LinkGraphics")) {
      g_config.link_graphics = value;
      return true;
    } else if (StringEqualsNoCase(key, "Shader")) {
      g_config.shader = *value ? value : NULL;
      return true;
    }
  } else if (section == 2) {
    if (StringEqualsNoCase(key, "EnableAudio")) {
      return ParseBool(value, &g_config.enable_audio);
    } else if (StringEqualsNoCase(key, "AudioFreq")) {
      g_config.audio_freq = (uint16)strtol(value, (char**)NULL, 10);
      return true;
    } else if (StringEqualsNoCase(key, "AudioChannels")) {
      g_config.audio_channels = (uint8)strtol(value, (char**)NULL, 10);
      return true;
    } else if (StringEqualsNoCase(key, "AudioSamples")) {
      g_config.audio_samples = (uint16)strtol(value, (char**)NULL, 10);
      return true;
    } else if (StringEqualsNoCase(key, "EnableMSU")) {
      return ParseBool(value, &g_config.enable_msu);
    }
  } else if (section == 3) {
    if (StringEqualsNoCase(key, "Autosave")) {
      g_config.autosave = (bool)strtol(value, (char**)NULL, 10);
      return true;
    } else if (StringEqualsNoCase(key, "ExtendedAspectRatio")) {
      const char* s;
      int h = 224;
      bool nospr = false, novis = false;
      // todo: make it not depend on the order
      while ((s = NextDelim(&value, ',')) != NULL) {
        if (strcmp(s, "extend_y") == 0)
          h = 240, g_config.extend_y = true;
        else if (strcmp(s, "16:9") == 0)
          g_config.extended_aspect_ratio = (h * 16 / 9 - 256) / 2;
        else if (strcmp(s, "16:10") == 0)
          g_config.extended_aspect_ratio = (h * 16 / 10 - 256) / 2;
        else if (strcmp(s, "18:9") == 0)
          g_config.extended_aspect_ratio = (h * 18 / 9 - 256) / 2;
        else if (strcmp(s, "4:3") == 0)
          g_config.extended_aspect_ratio = 0;
        else if (strcmp(s, "unchanged_sprites") == 0)
          nospr = true;
        else if (strcmp(s, "no_visual_fixes") == 0)
          novis = true;
        else
          return false;
      }
      if (g_config.extended_aspect_ratio && !nospr)
        g_config.features0 |= kFeatures0_ExtendScreen64;
      if (g_config.extended_aspect_ratio && !novis)
        g_config.features0 |= kFeatures0_WidescreenVisualFixes;
      return true;
    } else if (StringEqualsNoCase(key, "DisplayPerfInTitle")) {
      return ParseBool(value, &g_config.display_perf_title);
    } else if (StringEqualsNoCase(key, "DisableFrameDelay")) {
      return ParseBool(value, &g_config.disable_frame_delay);
    }
  } else if (section == 4) {
    if (StringEqualsNoCase(key, "ItemSwitchLR")) {
      return ParseBoolBit(value, &g_config.features0, kFeatures0_SwitchLR);
    } else if (StringEqualsNoCase(key, "TurnWhileDashing")) {
      return ParseBoolBit(value, &g_config.features0, kFeatures0_TurnWhileDashing);
    } else if (StringEqualsNoCase(key, "MirrorToDarkworld")) {
      return ParseBoolBit(value, &g_config.features0, kFeatures0_MirrorToDarkworld);
    } else if (StringEqualsNoCase(key, "CollectItemsWithSword")) {
      return ParseBoolBit(value, &g_config.features0, kFeatures0_CollectItemsWithSword);
    } else if (StringEqualsNoCase(key, "BreakPotsWithSword")) {
      return ParseBoolBit(value, &g_config.features0, kFeatures0_BreakPotsWithSword);
    } else if (StringEqualsNoCase(key, "DisableLowHealthBeep")) {
      return ParseBoolBit(value, &g_config.features0, kFeatures0_DisableLowHealthBeep);
    } else if (StringEqualsNoCase(key, "SkipIntroOnKeypress")) {
      return ParseBoolBit(value, &g_config.features0, kFeatures0_SkipIntroOnKeypress);
    } else if (StringEqualsNoCase(key, "ShowMaxItemsInYellow")) {
      return ParseBoolBit(value, &g_config.features0, kFeatures0_ShowMaxItemsInYellow);
    } else if (StringEqualsNoCase(key, "MoreActiveBombs")) {
      return ParseBoolBit(value, &g_config.features0, kFeatures0_MoreActiveBombs);
    } else if (StringEqualsNoCase(key, "CarryMoreRupees")) {
      return ParseBoolBit(value, &g_config.features0, kFeatures0_CarryMoreRupees);
    } else if (StringEqualsNoCase(key, "MiscBugFixes")) {
      return ParseBoolBit(value, &g_config.features0, kFeatures0_MiscBugFixes);
    } else if (StringEqualsNoCase(key, "GameChangingBugFixes")) {
      return ParseBoolBit(value, &g_config.features0, kFeatures0_GameChangingBugFixes);
    } else if (StringEqualsNoCase(key, "CancelBirdTravel")) {
      return ParseBoolBit(value, &g_config.features0, kFeatures0_CancelBirdTravel);
    }
  }
  return false;
}

static bool ParseOneConfigFile(const char *filename, int depth) {
  char *filedata = (char*)ReadWholeFile(filename, NULL), *p;
  if (!filedata)
    return false;
  
  int section = -2;
  g_config.memory_buffer = filedata;

  for (int lineno = 1; (p = NextLineStripComments(&filedata)) != NULL; lineno++) {
    if (*p == 0)
      continue; // empty line
    if (*p == '[') {
      section = GetIniSection(p);
      if (section < 0)
        fprintf(stderr, "%s:%d: Invalid .ini section %s\n", filename, lineno, p);
    } else if (*p == '!' && SkipPrefix(p + 1, "include ")) {
      char *tt = p + 8;
      char *new_filename = ReplaceFilenameWithNewPath(filename, NextPossiblyQuotedString(&tt));
      if (depth > 10 || !ParseOneConfigFile(new_filename, depth + 1))
        fprintf(stderr, "Warning: Unable to read %s\n", new_filename);
      free(new_filename);
    } else if (section == -2) {
      fprintf(stderr, "%s:%d: Expecting [section]\n", filename, lineno);
    } else {
      char *v = SplitKeyValue(p);
      if (v == NULL) {
        fprintf(stderr, "%s:%d: Expecting 'key=value'\n", filename, lineno);
        continue;
      }
      if (section >= 0 && !HandleIniConfig(section, p, v))
        fprintf(stderr, "%s:%d: Can't parse '%s'\n", filename, lineno, p);
    }
  }
  return true;
}

void ParseConfigFile(const char *filename) {
  if (filename != NULL || !ParseOneConfigFile("zelda3.user.ini", 0)) {
    if (filename == NULL)
      filename = "zelda3.ini";
    if (!ParseOneConfigFile(filename, 0))
      fprintf(stderr, "Warning: Unable to read config file %s\n", filename);
  }
  RegisterDefaultKeys();
}
