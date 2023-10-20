#ifndef ZELDA3_UTIL_H_
#define ZELDA3_UTIL_H_

#include "types.h"

typedef struct SDL_Window SDL_Window;

struct RendererFuncs {
  bool (*Initialize)(SDL_Window *window);
  void (*Destroy)();
  void (*BeginDraw)(int width, int height, uint8 **pixels, int *pitch);
  void (*EndDraw)();
};


typedef struct ByteArray {
  uint8 *data;
  size_t size, capacity;
} ByteArray;

void ByteArray_Resize(ByteArray *arr, size_t new_size);
void ByteArray_Destroy(ByteArray *arr);
void ByteArray_AppendData(ByteArray *arr, const uint8 *data, size_t data_size);
void ByteArray_AppendByte(ByteArray *arr, uint8 v);

uint8 *ReadWholeFile(const char *name, size_t *length);
char *NextDelim(char **s, int sep);
char *NextLineStripComments(char **s);
char *NextPossiblyQuotedString(char **s);
char *SplitKeyValue(char *p);
bool StringEqualsNoCase(const char *a, const char *b);
const char *StringStartsWithNoCase(const char *a, const char *b);
bool ParseBool(const char *value, bool *result);
const char *SkipPrefix(const char *big, const char *little);
void StrSet(char **rv, const char *s);
char *StrFmt(const char *fmt, ...);
char *ReplaceFilenameWithNewPath(const char *old_path, const char *new_path);
uint8 *ApplyBps(const uint8 *src, size_t src_size_in,
  const uint8 *bps, size_t bps_size, size_t *length_out);

#endif  // ZELDA3_UTIL_H_