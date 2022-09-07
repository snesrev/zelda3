#include "config.h"
#include "types.h"
#include <stdio.h>
#include <string.h>
#include <SDL.h>


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
  _(SDLK_UP), _(SDLK_DOWN), _(SDLK_LEFT), _(SDLK_RIGHT), _(SDLK_RSHIFT), _(SDLK_RETURN), _(SDLK_x), _(SDLK_z), _(SDLK_s), _(SDLK_a), _(SDLK_d), _(SDLK_c),
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
  // CheatLife, CheatKeys, CheatEquipment, ClearKeyLog, StopReplay, Fullscreen, Reset, Pause, PauseDimmed, Turbo, ZoomIn, ZoomOut, DisplayPerf, ToggleRenderer
  _(SDLK_w), _(SDLK_o), S(SDLK_w), _(SDLK_k), _(SDLK_l), A(SDLK_RETURN), _(SDLK_e), S(SDLK_p), _(SDLK_p), _(SDLK_t), N, N, _(SDLK_f), _(SDLK_r),
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
  S(CheatLife), S(CheatKeys), S(CheatEquipment), S(ClearKeyLog), S(StopReplay), S(Fullscreen), S(Reset),
  S(Pause), S(PauseDimmed), S(Turbo), S(ZoomIn), S(ZoomOut), S(DisplayPerf), S(ToggleRenderer),
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

static char *NextDelim(char **s, int sep) {
  char *r = *s;
  if (r) {
    while (r[0] == ' ' || r[0] == '\t')
      r++;
    char *t = strchr(r, sep);
    *s = t ? (*t++ = 0, t) : NULL;
  }
  return r;
}

static inline int ToLower(int a) {
  return a + (a >= 'A' && a <= 'Z') * 32;
}

static bool StringEqualsNoCase(const char *a, const char *b) {
  for (;;) {
    int aa = ToLower(*a++), bb = ToLower(*b++);
    if (aa != bb)
      return false;
    if (aa == 0)
      return true;
  }
}

static bool StringStartsWithNoCase(const char *a, const char *b) {
  for (;;) {
    int aa = ToLower(*a++), bb = ToLower(*b++);
    if (bb == 0)
      return true;
    if (aa != bb)
      return false;
  }
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
  return -1;
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
    if (StringEqualsNoCase(key, "EnhancedMode7")) {
      g_config.enhanced_mode7 = atoi(value);
      return true;
    } else if (StringEqualsNoCase(key, "NewRenderer")) {
      g_config.new_renderer = atoi(value);
      return true;
    } else if (StringEqualsNoCase(key, "IgnoreAspectRatio")) {
      g_config.ignore_aspect_ratio = atoi(value);
      return true;
    }

  }
  return false;
}


uint8 *ReadFile(const char *name, size_t *length) {
  FILE *f = fopen(name, "rb");
  if (f == NULL)
    return NULL;
  fseek(f, 0, SEEK_END);
  size_t size = ftell(f);
  rewind(f);
  uint8 *buffer = (uint8 *)malloc(size + 1);
  if (!buffer) Die("malloc failed");
  // Always zero terminate so this function can be used also for strings.
  buffer[size] = 0;
  if (fread(buffer, 1, size, f) != size)
    Die("fread failed");
  fclose(f);
  if (length) *length = size;
  return buffer;
}

void ParseConfigFile() {
  uint8 *file = ReadFile("zelda3.user.ini", NULL);
  if (!file) {
    file = ReadFile("zelda3.ini", NULL);
    if (!file)
      return;
  }
  fprintf(stderr, "Loading zelda3.ini\n");
  int section = -2;
  int lineno = 1;
  char *p, *next_p = (char*)file;
  for (; (p = next_p) != NULL; lineno++) {
    // find end of line
    char *eol = strchr(p, '\n');
    next_p = eol ? eol + 1 : NULL;
    eol = eol ? eol : p + strlen(p);
    // strip comments
    char *comment = memchr(p, '#', eol - p);
    eol = (comment != 0) ? comment : eol;
    // strip trailing whitespace
    while (eol > p && (eol[-1] == '\r' || eol[-1] == ' ' || eol[-1] == '\t'))
      eol--;
    *eol = 0;
    if (p == eol)
      continue; // empty line
    // strip leading whitespace
    while (p[0] == ' ' || p[0] == '\t')
      p++;
    if (*p == '[') {
      section = GetIniSection(p);
      if (section < 0)
        fprintf(stderr, "zelda3.ini:%d: Invalid .ini section %s\n", lineno, p);
    } else if (section == -2) {
      fprintf(stderr, "zelda3.ini:%d: Expecting [section]\n", lineno);
    } else {
      char *equals = memchr(p, '=', eol - p);
      if (equals == NULL) {
        fprintf(stderr, "zelda3.ini:%d: Expecting 'key=value'\n", lineno);
      } else {
        char *kr = equals;
        while (kr > p && (kr[-1] == ' ' || kr[-1] == '\t'))
          kr--;
        *kr = 0;
        char *v = equals + 1;
        while (v[0] == ' ' || v[0] == '\t')
          v++;
        if (section >= 0 && !HandleIniConfig(section, p, v))
          fprintf(stderr, "zelda3.ini:%d: Can't parse '%s'\n", lineno, p);
      }
    }
  }
  free(file);
}

void AfterConfigParse() {
  RegisterDefaultKeys();
}
