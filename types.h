#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#pragma once

typedef uint8_t uint8;
typedef int8_t int8;
typedef uint16_t uint16;
typedef int16_t int16;
typedef uint32_t uint32;
typedef int32_t int32;
typedef uint64_t uint64;
typedef unsigned int uint;

#define arraysize(x) sizeof(x)/sizeof(x[0])
#define sign8(x) ((x) & 0x80)
#define sign16(x) ((x) & 0x8000)
#define load24(x) ((*(uint32*)&(x))&0xffffff)

#ifdef _MSC_VER
#define countof _countof
#define NORETURN __declspec(noreturn)
#define FORCEINLINE __forceinline
#define NOINLINE __declspec(noinline)
#else
#define countof(a) (sizeof(a)/sizeof(*(a)))
#define NORETURN
#define FORCEINLINE inline
#define NOINLINE
#endif

static FORCEINLINE uint16 abs16(uint16 t) { return sign16(t) ? -t : t; }
static FORCEINLINE uint8 abs8(uint8 t) { return sign8(t) ? -t : t; }
static FORCEINLINE int IntMin(int a, int b) { return a < b ? a : b; }
static FORCEINLINE int IntMax(int a, int b) { return a > b ? a : b; }

#define BYTE(x) (*(uint8*)&(x))
#define HIBYTE(x) (((uint8*)&(x))[1])
#define WORD(x) (*(uint16*)&(x))
#define XY(x, y) ((y)*64+(x))

static inline uint16 swap16(uint16 v) { return (v << 8) | (v >> 8); }

typedef struct Point16U {
  uint16 x, y;
} Point16U;
typedef struct PointU8 {
  uint8 x, y;
} PointU8;
typedef struct Pair16U {
  uint16 a, b;
} Pair16U;

typedef struct PairU8 {
  uint8 a, b;
} PairU8;

typedef struct ProjectSpeedRet {
  uint8 x, y;
  uint8 xdiff, ydiff;
} ProjectSpeedRet;

typedef struct OamEnt {
  uint8 x, y, charnum, flags;
} OamEnt;

typedef struct UploadVram_Row {
  uint16 col[32];
} UploadVram_Row;

typedef struct UploadVram_32x32 {
  UploadVram_Row row[32];
} UploadVram_32x32;

typedef struct UploadVram_3 {
  uint8 pad[256];
  uint16 data[4];
} UploadVram_3;

#define uvram (*(UploadVram_3*)(&g_ram[0x1000]))

typedef void PlayerHandlerFunc();
typedef void HandlerFuncK(int k);

void NORETURN Die(const char *error);
