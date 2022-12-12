#ifndef ZELDA3_GLSL_SHADER_H_
#define ZELDA3_GLSL_SHADER_H_

#include "types.h"

enum {
  kGlslMaxPasses = 20,
  kGlslMaxTextures = 10,
};

enum GLSLScaleType {
  GLSL_NONE,
  GLSL_SOURCE,
  GLSL_VIEWPORT,
  GLSL_ABSOLUTE
};

typedef struct GlslTextureUniform {
  int Texture;
  int InputSize;
  int TextureSize;
  int TexCoord;
} GlslTextureUniform;

typedef struct GlslUniforms {
  GlslTextureUniform Top;
  int OutputSize;
  int FrameCount, FrameDirection;
  int LUTTexCoord;
  int VertexCoord;
  GlslTextureUniform Orig;
  GlslTextureUniform Prev[7];
  GlslTextureUniform Pass[kGlslMaxPasses];
  GlslTextureUniform PassPrev[kGlslMaxPasses];
  int Texture[kGlslMaxTextures];
} GlslUniforms;

typedef struct GlslPass {
  char *filename;
  uint8 scale_type_x, scale_type_y;
  bool float_framebuffer;
  bool srgb_framebuffer;
  bool mipmap_input;
  float scale_x, scale_y;
  uint wrap_mode;
  uint frame_count_mod;
  uint frame_count;
  uint gl_program, gl_fbo;
  uint filter;
  uint gl_texture;
  uint16 width, height;
  GlslUniforms unif;
} GlslPass;

typedef struct GlTextureWithSize {
  uint gl_texture;
  uint16 width, height;
} GlTextureWithSize;

typedef struct GlslTexture {
  struct GlslTexture *next;
  char *id;
  char *filename;
  uint filter;
  uint gl_texture;
  uint wrap_mode;
  bool mipmap;
  int width;
  int height;
} GlslTexture;

typedef struct GlslParam {
  struct GlslParam *next;
  char *id;
  bool has_value;
  float value;
  float min;
  float max;
  uint uniform[kGlslMaxPasses];
} GlslParam;

typedef struct GlslShader {
  int n_pass;
  GlslPass *pass;
  GlslParam *first_param;
  GlslTexture *first_texture;
  uint *gl_vao;
  uint gl_vbo;
  uint frame_count;
  int max_prev_frame;
  GlTextureWithSize prev_frame[8];
} GlslShader;

GlslShader *GlslShader_CreateFromFile(const char *filename, bool opengl_es);
void GlslShader_Destroy(GlslShader *gs);
void GlslShader_Render(GlslShader *gs, GlTextureWithSize *tex, int viewport_x, int viewport_y, int viewport_width, int viewport_height);


#endif  // ZELDA3_GLSL_SHADER_H_
