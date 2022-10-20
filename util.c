#include "util.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

char *NextDelim(char **s, int sep) {
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

bool StringEqualsNoCase(const char *a, const char *b) {
  for (;;) {
    int aa = ToLower(*a++), bb = ToLower(*b++);
    if (aa != bb)
      return false;
    if (aa == 0)
      return true;
  }
}

const char *StringStartsWithNoCase(const char *a, const char *b) {
  for (;; a++, b++) {
    int aa = ToLower(*a), bb = ToLower(*b);
    if (bb == 0)
      return a;
    if (aa != bb)
      return NULL;
  }
}

uint8 *ReadWholeFile(const char *name, size_t *length) {
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

char *NextLineStripComments(char **s) {
  char *p = *s;
  if (p == NULL)
    return NULL;
  // find end of line
  char *eol = strchr(p, '\n');
  *s = eol ? eol + 1 : NULL;
  eol = eol ? eol : p + strlen(p);
  // strip comments
  char *comment = memchr(p, '#', eol - p);
  eol = comment ? comment : eol;
  // strip trailing whitespace
  while (eol > p && (eol[-1] == '\r' || eol[-1] == ' ' || eol[-1] == '\t'))
    eol--;
  *eol = 0;
  // strip leading whitespace
  while (p[0] == ' ' || p[0] == '\t')
    p++;
  return p;
}

// Return the next possibly quoted string, space separated, or the empty string
char *NextPossiblyQuotedString(char **s) {
  char *r = *s, *t;
  while (*r == ' ' || *r == '\t')
    r++;
  if (*r == '"') {
    for (t = ++r; *t && *t != '"'; t++);
  } else {
    for (t = r; *t && *t != ' ' && *t != '\t'; t++);
  }
  if (*t) *t++ = 0;
  while (*t == ' ' || *t == '\t')
    t++;
  *s = t;
  return r;
}

char *ReplaceFilenameWithNewPath(const char *old_path, const char *new_path) {
  size_t olen = strlen(old_path);
  size_t nlen = strlen(new_path) + 1;
  while (olen && old_path[olen - 1] != '/' && old_path[olen - 1] != '\\')
    olen--;
  char *result = malloc(olen + nlen);
  memcpy(result, old_path, olen);
  memcpy(result + olen, new_path, nlen);
  return result;
}

char *SplitKeyValue(char *p) {
  char *equals = strchr(p, '=');
  if (equals == NULL)
    return NULL;
  char *kr = equals;
  while (kr > p && (kr[-1] == ' ' || kr[-1] == '\t'))
    kr--;
  *kr = 0;
  char *v = equals + 1;
  while (v[0] == ' ' || v[0] == '\t')
    v++;
  return v;
}

const char *SkipPrefix(const char *big, const char *little) {
  for (; *little; big++, little++) {
    if (*little != *big)
      return NULL;
  }
  return big;
}

void StrSet(char **rv, const char *s) {
  char *news = strdup(s);
  char *old = *rv;
  *rv = news;
  free(old);
}

char *StrFmt(const char *fmt, ...) {
  char buf[4096];
  va_list va;
  va_start(va, fmt);
  int n = vsnprintf(buf, sizeof(buf), fmt, va);
  if (n < 0 || n >= sizeof(buf)) Die("vsnprintf failed");
  va_end(va);
  return strdup(buf);
}

void ByteArray_Resize(ByteArray *arr, size_t new_size) {
  arr->size = new_size;
  if (new_size > arr->capacity) {
    size_t minsize = arr->capacity + (arr->capacity >> 1) + 8;
    arr->capacity = new_size < minsize ? minsize : new_size;
    void *data = realloc(arr->data, arr->capacity);
    if (!data) Die("memory allocation failed");
    arr->data = data;
  }
}

void ByteArray_Destroy(ByteArray *arr) {
  void *data = arr->data;
  arr->data = NULL;
  free(data);
}

void ByteArray_AppendData(ByteArray *arr, const uint8 *data, size_t data_size) {
  ByteArray_Resize(arr, arr->size + data_size);
  memcpy(arr->data + arr->size - data_size, data, data_size);
}

void ByteArray_AppendByte(ByteArray *arr, uint8 v) {
  ByteArray_Resize(arr, arr->size + 1);
  arr->data[arr->size - 1] = v;
}
