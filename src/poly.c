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
static const uint32 kPoly_RasterColors[16] = {
  0x00, 0xff, 0xff00, 0xffff,
  0xff0000, 0xff00ff, 0xffff00, 0xffffff,
  0xff000000, 0xff0000ff, 0xff00ff00, 0xff00ffff,
  0xffff0000, 0xffff00ff, 0xffffff00, 0xffffffff,
};
static const uint16 kPoly_LeftSideMask[8] = {0xffff, 0x7f7f, 0x3f3f, 0x1f1f, 0xf0f, 0x707, 0x303, 0x101};
static const uint16 kPoly_RightSideMask[8] = {0x8080, 0xc0c0, 0xe0e0, 0xf0f0, 0xf8f8, 0xfcfc, 0xfefe, 0xffff};
uint16 Poly_Divide(uint16 a, uint16 b) {
  poly_tmp1 = sign16(a) ? -a : a;
  poly_tmp0 = b;
  while (poly_tmp0 >= 256)
    poly_tmp0 >>= 1, poly_tmp1 >>= 1;
  int q = poly_tmp1 / poly_tmp0;
  return sign16(a) ? -q : q;
}

void Poly_RunFrame() {
  Polyhedral_EmptyBitMapBuffer();
  Polyhedral_SetShapePointer();
  Polyhedral_SetRotationMatrix();
  Polyhedral_OperateRotation();
  Polyhedral_DrawPolyhedron();
}

void Polyhedral_SetShapePointer() {  // 89f83d
  poly_var1 = poly_config1 * 2 + 0x80;
  poly_tmp0 = poly_which_model * 2;

  const PolyConfig *poly_config = &kPolyConfigs[poly_which_model];
  poly_config_num_vertex = poly_config->num_vtx;
  poly_config_num_polys = poly_config->num_poly;
  poly_fromlut_ptr2 = poly_config->vtx_val;
  poly_fromlut_ptr4 = poly_config->polys_val;
}

void Polyhedral_SetRotationMatrix() {  // 89f864
  poly_sin_a = kPolySinCos[poly_a];
  poly_cos_a = kPolySinCos[poly_a + 64];
  poly_sin_b = kPolySinCos[poly_b];
  poly_cos_b = kPolySinCos[poly_b + 64];
  poly_e0 = (int16)poly_sin_b * (int8)poly_sin_a >> 8 << 2;
  poly_e1 = (int16)poly_cos_b * (int8)poly_cos_a >> 8 << 2;
  poly_e2 = (int16)poly_cos_b * (int8)poly_sin_a >> 8 << 2;
  poly_e3 = (int16)poly_sin_b * (int8)poly_cos_a >> 8 << 2;
}

void Polyhedral_OperateRotation() {  // 89f8fb
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

void Polyhedral_RotatePoint() {  // 89f931
  int x = (int8)poly_fromlut_x;
  int y = (int8)poly_fromlut_y;
  int z = (int8)poly_fromlut_z;

  poly_f0 =   (int16)poly_cos_b * z                         - (int16)poly_sin_b * x;
  poly_f1 =   (int16)poly_e0    * z + (int16)poly_cos_a * y + (int16)poly_e2    * x;
  poly_f2 = ((int16)poly_e3 * z >> 8) - ((int16)poly_sin_a * y >> 8) + ((int16)poly_e1 * x >> 8) + poly_var1;
}

void Polyhedral_ProjectPoint() {  // 89f9d6
  poly_f0 = Poly_Divide(poly_f0, poly_f2);
  poly_f1 = Poly_Divide(poly_f1, poly_f2);
}

void Polyhedral_DrawPolyhedron() {  // 89fa4f
  const PolyConfig *poly_config = &kPolyConfigs[poly_which_model];
  const uint8 *src = poly_config->poly;
  do {
    poly_num_vertex_in_poly = *src++;
    BYTE(poly_tmp0) = poly_num_vertex_in_poly;
    poly_xy_coords[0] = poly_num_vertex_in_poly * 2;

    int i = 1;
    do {
      int j = *src++;
      poly_xy_coords[i + 0] = poly_arr_x[j];
      poly_xy_coords[i + 1] = poly_arr_y[j];
      i += 2;
    } while (--BYTE(poly_tmp0));

    poly_raster_color_config = *src++;
    int order = Polyhedral_CalculateCrossProduct();
    if (order > 0) {
      Polyhedral_SetForegroundColor();
      Polyhedral_DrawFace();
    }
  } while (--poly_config_num_polys);
}

void Polyhedral_SetForegroundColor() {  // 89faca
  uint8 t = poly_which_model ? (poly_config1 >> 5) : 0;
  uint8 a = (poly_tmp0 << (t + 1)) >> 8;
  Polyhedral_SetColorMask(a <= 1 ? 1 : a >= 7 ? 7 : a);
}

int16 Polyhedral_CalculateCrossProduct() {  // 89fb24
  int16 a = poly_xy_coords[3] - poly_xy_coords[1];
  poly_tmp0 = a * (int8)(poly_xy_coords[6] - poly_xy_coords[4]);
  a = poly_xy_coords[5] - poly_xy_coords[3];
  poly_tmp0 -= a * (int8)(poly_xy_coords[4] - poly_xy_coords[2]);
  return poly_tmp0;
}

void Polyhedral_SetColorMask(int c) {  // 89fcae
  uint32 v = kPoly_RasterColors[c];
  poly_raster_color0 = v;
  poly_raster_color1 = v >> 16;
}

void Polyhedral_EmptyBitMapBuffer() {  // 89fd04
  memset(polyhedral_buffer, 0, 0x800);
}

void Polyhedral_DrawFace() {  // 89fd1e
  int n = poly_xy_coords[0];
  uint8 min_y = poly_xy_coords[n];
  int min_idx = n;
  while (n -= 2) {
    if (poly_xy_coords[n] < min_y)
      min_y = poly_xy_coords[n], min_idx = n;
  }
  poly_raster_dst_ptr = 0xe800 + (((min_y & 0x38) ^ (min_y & 0x20 ? 0x24 : 0)) << 6) + (min_y & 7) * 2;
  poly_cur_vertex_idx0 = poly_cur_vertex_idx1 = min_idx;
  poly_total_num_steps = poly_xy_coords[0] >> 1;
  poly_y0_cur = poly_y1_cur = poly_xy_coords[min_idx];
  poly_x0_cur = poly_x1_cur = poly_xy_coords[min_idx - 1];
  if (Polyhedral_SetLeft() || Polyhedral_SetRight())
    return;
  for (;;) {
    Polyhedral_FillLine();
    if (BYTE(poly_raster_dst_ptr) != 0xe) {
      poly_raster_dst_ptr += 2;
    } else {
      uint8 a = HIBYTE(poly_raster_dst_ptr) + 2;
      poly_raster_dst_ptr = (a ^ ((a & 8) ? 0 : 0x19)) << 8;
    }
    if (poly_y0_cur == poly_y0_trig) {
      poly_x0_cur = poly_x0_target;
      if (Polyhedral_SetLeft())
        return;
    }
    poly_y0_cur++;
    if (poly_y1_cur == poly_y1_trig) {
      poly_x1_cur = poly_x1_target;
      if (Polyhedral_SetRight())
        return;
    }
    poly_y1_cur++;
    poly_x0_frac += poly_x0_step;
    poly_x1_frac += poly_x1_step;
  }
}

void Polyhedral_FillLine() {  // 89fdcf
  uint16 left = kPoly_LeftSideMask[(poly_x0_frac >> 8) & 7];
  uint16 right = kPoly_RightSideMask[(poly_x1_frac >> 8) & 7];
  poly_tmp2 = (poly_x0_frac >> 8) & 0x38;
  int d0 = ((poly_x1_frac >> 8) & 0x38);
  uint16 *ptr = (uint16*)&g_ram[poly_raster_dst_ptr + d0 * 4];
  if ((d0 -= poly_tmp2) == 0) {
    poly_tmp1 = left & right;
    ptr[0] ^= (ptr[0] ^ poly_raster_color0) & poly_tmp1;
    ptr[8] ^= (ptr[8] ^ poly_raster_color1) & poly_tmp1;
    return;
  }
  if (d0 < 0)
    return;
  int n = d0 >> 3;
  ptr[0] ^= (ptr[0] ^ poly_raster_color0) & right;
  ptr[8] ^= (ptr[8] ^ poly_raster_color1) & right;
  ptr -= 0x10;
  while (--n) {
    ptr[0] = poly_raster_color0;
    ptr[8] = poly_raster_color1;
    ptr -= 0x10;
  }
  ptr[0] ^= (ptr[0] ^ poly_raster_color0) & left;
  ptr[8] ^= (ptr[8] ^ poly_raster_color1) & left;
  poly_tmp1 = left, poly_raster_numfull = 0;
}

bool Polyhedral_SetLeft() {  // 89feb4
  int i;
  for (;;) {
    if (sign8(--poly_total_num_steps))
      return true;
    i = poly_cur_vertex_idx0 - 2;
    if (i == 0)
      i = poly_xy_coords[0];
    if (poly_xy_coords[i] < poly_y0_cur)
      return true;
    if (poly_xy_coords[i] != poly_y0_cur)
      break;
    poly_x0_cur = poly_xy_coords[i - 1];
    poly_cur_vertex_idx0 = i;
  }
  poly_y0_trig = poly_xy_coords[i];
  poly_x0_target = poly_xy_coords[i - 1];
  poly_cur_vertex_idx0 = i;
  int t = poly_x0_target - poly_x0_cur, u = t;
  if (t < 0)
    t = -t;
  t = ((t & 0xff) << 8) / (uint8)(poly_y0_trig - poly_y0_cur);
  poly_x0_frac = (poly_x0_cur << 8) | 0x80;
  poly_x0_step = (u < 0) ? -t : t;
  return false;
}

bool Polyhedral_SetRight() {  // 89ff1e
  int i;
  for (;;) {
    if (sign8(--poly_total_num_steps))
      return true;
    i = poly_cur_vertex_idx1;
    if (i == poly_xy_coords[0])
      i = 0;
    i += 2;
    if (poly_xy_coords[i] < poly_y1_cur)
      return true;
    if (poly_xy_coords[i] != poly_y1_cur)
      break;
    poly_x1_cur = poly_xy_coords[i - 1];
    poly_cur_vertex_idx1 = i;
  }
  poly_y1_trig = poly_xy_coords[i];
  poly_x1_target = poly_xy_coords[i - 1];
  poly_cur_vertex_idx1 = i;
  int t = poly_x1_target - poly_x1_cur, u = t;
  if (t < 0)
    t = -t;
  t = ((t & 0xff) << 8) / (uint8)(poly_y1_trig - poly_y1_cur);
  poly_x1_frac = (poly_x1_cur << 8) | 0x80;
  poly_x1_step = (u < 0) ? -t : t;
  return false;
}

