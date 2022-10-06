#include "poly.h"
#include "zelda_rtl.h"
#include "variables.h"

static const int8 kPolySinCos[320] = {
  0,   2,   3,   5,   6,   8,   9,  11,  12,  14,  16,  17,  19,  20,  22,  23,
  24,  26,  27,  29,  30,  32,  33,  34,  36,  37,  38,  39,  41,  42,  43,  44,
  45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  56,  57,  58,  59,
  59,  60,  60,  61,  61,  62,  62,  62,  63,  63,  63,  64,  64,  64,  64,  64,
  64,  64,  64,  64,  64,  64,  63,  63,  63,  62,  62,  62,  61,  61,  60,  60,
  59,  59,  58,  57,  56,  56,  55,  54,  53,  52,  51,  50,  49,  48,  47,  46,
  45,  44,  43,  42,  41,  39,  38,  37,  36,  34,  33,  32,  30,  29,  27,  26,
  24,  23,  22,  20,  19,  17,  16,  14,  12,  11,   9,   8,   6,   5,   3,   2,
  0,  -2,  -3,  -5,  -6,  -8,  -9, -11, -12, -14, -16, -17, -19, -20, -22, -23,
  -24, -26, -27, -29, -30, -32, -33, -34, -36, -37, -38, -39, -41, -42, -43, -44,
  -45, -46, -47, -48, -49, -50, -51, -52, -53, -54, -55, -56, -56, -57, -58, -59,
  -59, -60, -60, -61, -61, -62, -62, -62, -63, -63, -63, -64, -64, -64, -64, -64,
  -64, -64, -64, -64, -64, -64, -63, -63, -63, -62, -62, -62, -61, -61, -60, -60,
  -59, -59, -58, -57, -56, -56, -55, -54, -53, -52, -51, -50, -49, -48, -47, -46,
  -45, -44, -43, -42, -41, -39, -38, -37, -36, -34, -33, -32, -30, -29, -27, -26,
  -24, -23, -22, -20, -19, -17, -16, -14, -12, -11,  -9,  -8,  -6,  -5,  -3,  -2,
  0,   2,   3,   5,   6,   8,   9,  11,  12,  14,  16,  17,  19,  20,  22,  23,
  24,  26,  27,  29,  30,  32,  33,  34,  36,  37,  38,  39,  41,  42,  43,  44,
  45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  56,  57,  58,  59,
  59,  60,  60,  61,  61,  62,  62,  62,  63,  63,  63,  64,  64,  64,  64,  64,
};

typedef struct Vertex3 {
  int8 x, y, z;
} Vertex3;

static const Vertex3 kPoly0_Vtx[6] = {
  {  0,  65,   0},
  {  0, -65,   0},
  {  0,   0, -40},
  {-40,   0,   0},
  {  0,   0,  40},
  { 40,   0,   0},
};

static const uint8 kPoly0_Polys[40] = {
  3, 0, 5, 2, 4,
  3, 0, 2, 3, 1,
  3, 0, 3, 4, 2,
  3, 0, 4, 5, 3,
  3, 1, 2, 5, 4,
  3, 1, 3, 2, 1,
  3, 1, 4, 3, 2,
  3, 1, 5, 4, 3,
};

static const Vertex3 kPoly1_Vtx[6] = {
  {  0,  40,  10},
  { 40, -40,  10},
  {-40, -40,  10},
  {  0,  40, -10},
  {-40, -40, -10},
  { 40, -40, -10},
};

static const uint8 kPoly1_Polys[28] = {
  3, 0, 1, 2, 7,
  3, 3, 4, 5, 6,
  4, 0, 3, 5, 1, 5,
  4, 1, 5, 4, 2, 4,
  4, 3, 0, 2, 4, 3,
};

typedef struct PolyConfig {
  uint8 num_vtx, num_poly;
  uint16 vtx_val, polys_val;
  const Vertex3 *vertex;
  const uint8 *poly;
} PolyConfig;

static const PolyConfig kPolyConfigs[2] = {
  {6, 8, 0xff98, 0xffaa, kPoly0_Vtx, kPoly0_Polys},
  {6, 5, 0xffd2, 0xffe4, kPoly1_Vtx, kPoly1_Polys},
};

static uint16 Poly_Divide(uint16 a, uint16 b);
static void Polyhedral_SetShapePointer();
static void Polyhedral_SetRotationMatrix();
static void Polyhedral_OperateRotation();
static void Polyhedral_RotatePoint();
static void Polyhedral_ProjectPoint();
static void Polyhedral_DrawPolyhedron();
static int Polyhedral_CalcForegroundColor(int16 facing);
static void Polyhedral_EmptyBitMapBuffer();

static uint16 Poly_Divide(uint16 a, uint16 b) {
  uint16 tmp1 = sign16(a) ? -a : a;
  uint16 tmp0 = b;
  while (tmp0 >= 256)
    tmp0 >>= 1, tmp1 >>= 1;
  int q = tmp1 / tmp0;
  return sign16(a) ? -q : q;
}

void Poly_RunFrame() {
  Polyhedral_EmptyBitMapBuffer();
  Polyhedral_SetShapePointer();
  Polyhedral_SetRotationMatrix();
  Polyhedral_OperateRotation();
  Polyhedral_DrawPolyhedron();
}

static void Polyhedral_SetShapePointer() {  // 89f83d
  poly_var1 = poly_config1 * 2 + 0x80;
  poly_tmp0 = poly_which_model * 2;

  const PolyConfig *poly_config = &kPolyConfigs[poly_which_model];
  poly_config_num_vertex = poly_config->num_vtx;
  poly_config_num_polys = poly_config->num_poly;
  poly_fromlut_ptr2 = poly_config->vtx_val;
  poly_fromlut_ptr4 = poly_config->polys_val;
}

static void Polyhedral_SetRotationMatrix() {  // 89f864
  poly_sin_a = kPolySinCos[poly_a];
  poly_cos_a = kPolySinCos[poly_a + 64];
  poly_sin_b = kPolySinCos[poly_b];
  poly_cos_b = kPolySinCos[poly_b + 64];
  poly_e0 = (int16)poly_sin_b * (int8)poly_sin_a >> 8 << 2;
  poly_e1 = (int16)poly_cos_b * (int8)poly_cos_a >> 8 << 2;
  poly_e2 = (int16)poly_cos_b * (int8)poly_sin_a >> 8 << 2;
  poly_e3 = (int16)poly_sin_b * (int8)poly_cos_a >> 8 << 2;
}

static void Polyhedral_OperateRotation() {  // 89f8fb
  const PolyConfig *poly_config = &kPolyConfigs[poly_which_model];
  const int8 *src = &poly_config->vertex[0].x;
  int i = poly_config_num_vertex;
  src += i * 3;
  do {
    src -= 3, i -= 1;
    poly_fromlut_x = src[2];
    poly_fromlut_y = src[1];
    poly_fromlut_z = src[0];
    Polyhedral_RotatePoint();
    Polyhedral_ProjectPoint();
    poly_arr_x[i] = poly_base_x + poly_f0;
    poly_arr_y[i] = poly_base_y - poly_f1;
  } while (i);
}

static void Polyhedral_RotatePoint() {  // 89f931
  int x = (int8)poly_fromlut_x;
  int y = (int8)poly_fromlut_y;
  int z = (int8)poly_fromlut_z;

  poly_f0 =   (int16)poly_cos_b * z                         - (int16)poly_sin_b * x;
  poly_f1 =   (int16)poly_e0    * z + (int16)poly_cos_a * y + (int16)poly_e2    * x;
  poly_f2 = ((int16)poly_e3 * z >> 8) - ((int16)poly_sin_a * y >> 8) + ((int16)poly_e1 * x >> 8) + poly_var1;
}

static void Polyhedral_ProjectPoint() {  // 89f9d6
  poly_f0 = Poly_Divide(poly_f0, poly_f2);
  poly_f1 = Poly_Divide(poly_f1, poly_f2);
}

struct PolyPoint {
  uint8 x, y;
};

static int16 Polyhedral_CalculateCrossProduct(struct PolyPoint *pt) {  // 89fb24
  int tmp0 = (pt[1].x - pt[0].x) * (pt[2].y - pt[1].y);
  tmp0 -= (pt[2].x - pt[1].x) * (pt[1].y - pt[0].y);
  return tmp0;
}

static void Polyhedral_DrawFace(int color, struct PolyPoint *pt, int npt, bool x2);

static void Polyhedral_DrawPolyhedron() {  // 89fa4f
  const PolyConfig *poly_config = &kPolyConfigs[poly_which_model];
  const uint8 *src = poly_config->poly;
  struct PolyPoint points[16];
  do {
    int n = *src++;
    for(int i = 0; i < n; i++) {
      int j = *src++;
      points[i].x = poly_arr_x[j];
      points[i].y = poly_arr_y[j];
    }
    poly_raster_color_config = *src++;
    int16 facing = Polyhedral_CalculateCrossProduct(points);
    if (facing > 0) {
      int color = Polyhedral_CalcForegroundColor(facing);
      Polyhedral_DrawFace(color, points, n, false);
      if (kPpuUpsample2x2) {
        for (int i = 0; i < n; i++)
          points[i].x *= 2, points[i].y *= 2;
        Polyhedral_DrawFace(color, points, n, true);
      }
    }
  } while (--poly_config_num_polys);
}

static int Polyhedral_CalcForegroundColor(int16 facing) {  // 89faca
  uint8 t = poly_which_model ? (poly_config1 >> 5) : 0;
  uint8 a = (facing << (t + 1)) >> 8;
  return a <= 1 ? 1 : a >= 7 ? 7 : a;
}

static void Polyhedral_EmptyBitMapBuffer() {  // 89fd04
  memset(polyhedral_buffer, 0, 0x800);
  if (kPpuUpsample2x2)
    memset(&g_zenv.ext_vram[GET_SPRITE_ADDR_X2(0x5800)], 0, 0x800 * 4);
}

struct PolyFaceRender {
  struct PolyPoint *pt;
  uint8 npt;
  uint8 color;
  uint8 total_num_steps;

  struct PolyPoint target0, target1;
  struct PolyPoint cur0, cur1;

  uint16 x0_frac, x0_step;
  uint8 cur_vertex_idx0, cur_vertex_idx1;
  uint16 x1_frac, x1_step;
};

static void Polyhedral_FillLine(struct PolyFaceRender *poly, int y);
static void Polyhedral_FillLineX2(struct PolyFaceRender *poly, int y);
static bool Polyhedral_StepLeft(struct PolyFaceRender *poly);
static bool Polyhedral_StepRight(struct PolyFaceRender *poly);

static void Polyhedral_DrawFace(int color, struct PolyPoint *pt, int npt, bool x2) {  // 89fd1e
  struct PolyFaceRender poly;

  poly.pt = pt;
  poly.color = color;
  poly.total_num_steps = poly.npt = npt;

  uint8 min_y = pt[npt - 1].y;
  int min_idx = npt - 1;
  while (--npt) {
    if (pt[npt - 1].y < min_y)
      min_y = pt[npt - 1].y, min_idx = npt - 1;
  }
  int y = min_y;
  poly.cur_vertex_idx0 = poly.cur_vertex_idx1 = min_idx;
  poly.cur1 = poly.cur0 = pt[min_idx];
  if (Polyhedral_StepLeft(&poly) || Polyhedral_StepRight(&poly))
    return;
  for (;;) {
    x2 ? Polyhedral_FillLineX2(&poly, y) : Polyhedral_FillLine(&poly, y);
    y++;
    if (poly.cur0.y == poly.target0.y) {
      poly.cur0.x = poly.target0.x;
      if (Polyhedral_StepLeft(&poly))
        return;
    }
    poly.cur0.y++;
    if (poly.cur1.y == poly.target1.y) {
      poly.cur1.x = poly.target1.x;
      if (Polyhedral_StepRight(&poly))
        return;
    }
    poly.cur1.y++;
    poly.x0_frac += poly.x0_step;
    poly.x1_frac += poly.x1_step;
  }
}

static void Polyhedral_FillLine(struct PolyFaceRender *poly, int y) {  // 89fdcf
  static const uint16 kPoly_LeftSideMask[8] = { 0xffff, 0x7f7f, 0x3f3f, 0x1f1f, 0xf0f, 0x707, 0x303, 0x101 };
  static const uint16 kPoly_RightSideMask[8] = { 0x8080, 0xc0c0, 0xe0e0, 0xf0f0, 0xf8f8, 0xfcfc, 0xfefe, 0xffff };
  static const uint32 kPoly_RasterColors[16] = {
    0x00, 0xff, 0xff00, 0xffff,
    0xff0000, 0xff00ff, 0xffff00, 0xffffff,
    0xff000000, 0xff0000ff, 0xff00ff00, 0xff00ffff,
    0xffff0000, 0xffff00ff, 0xffffff00, 0xffffffff,
  };
  uint32 v = kPoly_RasterColors[poly->color];
  uint16 color0 = v, color1 = v >> 16;
  uint16 leftmask = kPoly_LeftSideMask[(poly->x0_frac >> 8) & 7];
  uint16 rightmask = kPoly_RightSideMask[(poly->x1_frac >> 8) & 7];
  int left = (poly->x0_frac >> 11) & 7;
  int right = (poly->x1_frac >> 11) & 7;
  int tile_index = (y >> 3 & 3) * 16 + (y & 0x20 ? 8 : 0) + right;
  uint16 *ptr = (uint16*)&g_ram[0xe800 + tile_index * 32 + (y & 7) * 2];
  if ((right -= left) == 0) {
    ptr[0] ^= (ptr[0] ^ color0) & (leftmask & rightmask);
    ptr[8] ^= (ptr[8] ^ color1) & (leftmask & rightmask);
  } else if (right >= 0) {
    ptr[0] ^= (ptr[0] ^ color0) & rightmask;
    ptr[8] ^= (ptr[8] ^ color1) & rightmask;
    ptr -= 32 / 2;
    while (--right) {
      ptr[0] = color0;
      ptr[8] = color1;
      ptr -= 32 / 2;
    }
    ptr[0] ^= (ptr[0] ^ color0) & leftmask;
    ptr[8] ^= (ptr[8] ^ color1) & leftmask;
  }
}

static void Polyhedral_FillLineX2(struct PolyFaceRender *poly, int y) {  // 89fdcf
  uint32 x0_frac = poly->x0_frac, x1_frac = poly->x1_frac;
  int tile_index = (y >> 4 & 3) * 16 + (y & 0x40 ? 8 : 0);
  uint64 *ptr = (uint64 *)(&g_zenv.ext_vram[GET_SPRITE_ADDR_X2(tile_index * 16 + 0x5800)]) + (y & 15);
  uint64 color = poly->color * 0x1111111111111111ull;
  uint64 lmask = 0xffffffffffffffffull << (((x0_frac >> 8) & 15) * 4);
  uint64 rmask = 0xffffffffffffffffull >> ((15 - ((x1_frac >> 8) & 15)) * 4);
  int left = (x0_frac >> 12) & 7;
  int right = (x1_frac >> 12) & 7;
  ptr += left * 16;
  if ((right -= left) == 0) {
    ptr[0] ^= (ptr[0] ^ color) & (lmask & rmask);
  } else if (right >= 0) {
    ptr[0] ^= (ptr[0] ^ color) & lmask;
    ptr += 128 / 8;
    while (--right) {
      ptr[0] = color;
      ptr += 128 / 8;
    }
    ptr[0] ^= (ptr[0] ^ color) & rmask;
  }
}

static uint16 PolyDivide2(int num, uint8 denom) {
  int t = num, u = t;
  if (t < 0)
    t = -t;
  t = ((t & 0xff) << 8) / denom;
  return (u < 0) ? -t : t;
}

static bool Polyhedral_StepLeft(struct PolyFaceRender *poly) {  // 89feb4
  int i;
  for (;;) {
    if (sign8(--poly->total_num_steps))
      return true;
    i = (poly->cur_vertex_idx0 == 0 ? poly->npt : poly->cur_vertex_idx0) - 1;
    poly->cur_vertex_idx0 = i;
    if (poly->pt[i].y < poly->cur0.y)
      return true;
    if (poly->pt[i].y != poly->cur0.y)
      break;
    poly->cur0.x = poly->pt[i].x;
  }
  poly->x0_frac = (poly->cur0.x << 8) | 0x80;
  poly->target0 = poly->pt[i];
  poly->x0_step = PolyDivide2(poly->target0.x - poly->cur0.x, poly->target0.y - poly->cur0.y);
  return false;
}

static bool Polyhedral_StepRight(struct PolyFaceRender *poly) {  // 89ff1e
  int i;
  for (;;) {
    if (sign8(--poly->total_num_steps))
      return true;
    i = (poly->cur_vertex_idx1 + 1 == poly->npt) ? 0 : poly->cur_vertex_idx1 + 1;
    poly->cur_vertex_idx1 = i;
    if (poly->pt[i].y < poly->cur1.y)
      return true;
    if (poly->pt[i].y != poly->cur1.y)
      break;
    poly->cur1.x = poly->pt[i].x;
  }
  poly->x1_frac = (poly->cur1.x << 8) | 0x80;
  poly->target1 = poly->pt[i];
  poly->x1_step = PolyDivide2(poly->target1.x - poly->cur1.x, poly->target1.y - poly->cur1.y);
  return false;
}

