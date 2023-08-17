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

// Automatically selects between 16 or 32 bit indexes. Can hold up to 8192 elements in 16-bit mode.
MemBlk FindIndexInMemblk(MemBlk data, size_t i) {
  if (data.size < 2)
    return (MemBlk) { 0, 0 };
  size_t end = data.size - 2, left_off, right_off;
  size_t mx = *(uint16 *)(data.ptr + end);
  if (mx < 8192) {
    if (i > mx || mx * 2 > end)
      return (MemBlk) { 0, 0 };
    left_off = ((i == 0) ? mx * 2 : mx * 2 + *(uint16 *)(data.ptr + i * 2 - 2));
    right_off = (i == mx) ? end : mx * 2 + *(uint16 *)(data.ptr + i * 2);
  } else {
    mx -= 8192;
    if (i > mx || mx * 4 > end)
      return (MemBlk) { 0, 0 };
    left_off = ((i == 0) ? mx * 4 : mx * 4 + *(uint32 *)(data.ptr + i * 4 - 4));
    right_off = (i == mx) ? end : mx * 4 + *(uint32 *)(data.ptr + i * 4);
  }
  if (left_off > right_off || right_off > end)
    return (MemBlk) { 0, 0 };
  return (MemBlk) { data.ptr + left_off, right_off - left_off };
}


static uint64 BpsDecodeInt(const uint8 **src) {
  uint64 data = 0, shift = 1;
  while(true) {
    uint8 x = *(*src)++;
    data += (x & 0x7f) * shift;
    if(x & 0x80) break;
    shift <<= 7;
    data += shift;
  }
  return data;
}

#define CRC32_POLYNOMIAL 0xEDB88320

static uint32 crc32(const void *data, size_t length) {
  uint32 crc = 0xFFFFFFFF;
  const uint8 *byteData = (const uint8 *)data;
  for (size_t i = 0; i < length; i++) {
    crc ^= byteData[i];
    for (int j = 0; j < 8; j++)
      crc = (crc >> 1) ^ ((crc & 1) * CRC32_POLYNOMIAL);
  }
  return crc ^ 0xFFFFFFFF;
}

uint8 *ApplyBps(const uint8 *src, size_t src_size_in,
  const uint8 *bps, size_t bps_size, size_t *length_out) {
  const uint8 *bps_end = bps + bps_size - 12;

  if (memcmp(bps, "BPS1", 4))
    return NULL;
  if (crc32(src, src_size_in) != *(uint32 *)(bps_end))
    return NULL;
  if (crc32(bps, bps_size - 4) != *(uint32 *)(bps_end + 8))
    return NULL;

  bps += 4;
  uint32 src_size = BpsDecodeInt(&bps);
  uint32 dst_size = BpsDecodeInt(&bps);
  uint32 meta_size = BpsDecodeInt(&bps);
  uint32 outputOffset = 0;
  uint32 sourceRelativeOffset = 0;
  uint32 targetRelativeOffset = 0;
  if (src_size != src_size_in)
    return NULL;
  *length_out = dst_size;
  uint8 *dst = malloc(dst_size);
  if (!dst)
    return NULL;
  while (bps < bps_end) {
    uint32 cmd = BpsDecodeInt(&bps);
    uint32 length = (cmd >> 2) + 1;
    switch (cmd & 3) {
    case 0:
      while(length--) {
        dst[outputOffset] = src[outputOffset];
        outputOffset++;
      }
      break;
    case 1:
      while (length--)
        dst[outputOffset++] = *bps++;
      break;
    case 2:
      cmd = BpsDecodeInt(&bps);
      sourceRelativeOffset += (cmd & 1 ? -1 : +1) * (cmd >> 1);
      while (length--)
        dst[outputOffset++] = src[sourceRelativeOffset++];
      break;
    default:
      cmd = BpsDecodeInt(&bps);
      targetRelativeOffset += (cmd & 1 ? -1 : +1) * (cmd >> 1);
      while(length--)
        dst[outputOffset++] = dst[targetRelativeOffset++];
      break;
    }
  }
  if (dst_size != outputOffset)
    return NULL;
  if (crc32(dst, dst_size) != *(uint32 *)(bps_end + 4))
    return NULL;
  return dst;
}