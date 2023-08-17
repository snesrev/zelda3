// This file is heavily influenced by Snes9x
#include "third_party/gl_core/gl_core_3_1.h"
#include "glsl_shader.h"
#include "util.h"
#include "config.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_THREAD_LOCALS
#define STBI_ONLY_PNG
#define STBI_MAX_DIMENSIONS 4096
#define STBI_NO_FAILURE_STRINGS
#include "third_party/stb/stb_image.h"

static GlslPass *ParseConfigKeyPass(GlslShader *gs, const char *key, const char *match) {
  char *endp;
  for (; *match; key++, match++) {
    if (*key != *match)
      return NULL;
  }
  if ((uint8)(*key - '0') >= 10)
    return NULL;
  uint pass = strtoul(key, &endp, 10);
  if (pass >= gs->n_pass || *endp != 0)
    return NULL;
  return gs->pass + pass + 1;
}

static uint8 ParseScaleType(const char *s) {
  return StringEqualsNoCase(s, "source") ? GLSL_SOURCE : 
         StringEqualsNoCase(s, "viewport") ? GLSL_VIEWPORT :
         StringEqualsNoCase(s, "absolute") ? GLSL_ABSOLUTE : GLSL_NONE;
}

static uint ParseWrapMode(const char *s) {
  return StringEqualsNoCase(s, "repeat") ? GL_REPEAT :
         StringEqualsNoCase(s, "clamp_to_edge") ? GL_CLAMP_TO_EDGE :
         StringEqualsNoCase(s, "clamp") ? GL_CLAMP : GL_CLAMP_TO_BORDER;
}

static void GlslPass_Initialize(GlslPass *pass) {
  pass->scale_x = 1.0f;
  pass->scale_y = 1.0f;
  pass->wrap_mode = GL_CLAMP_TO_BORDER;
}

static void ParseTextures(GlslShader *gs, char *value) {
  char *id;
  GlslTexture **nextp = &gs->first_texture;
  for (int num = 0; (id = NextDelim(&value, ';')) != NULL && num < kGlslMaxTextures; num++) {
    GlslTexture *t = calloc(sizeof(GlslTexture), 1);
    t->id = strdup(id);
    t->wrap_mode = GL_CLAMP_TO_BORDER;
    t->filter = GL_NEAREST;
    *nextp = t;
    nextp = &t->next;
  }
}

static bool ParseTextureKeyValue(GlslShader *gs, const char *key, const char *value) {
  for (GlslTexture *t = gs->first_texture; t != NULL; t = t->next) {
    const char *key2 = SkipPrefix(key, t->id);
    if (!key2) continue;
    if (*key2 == 0) {
      StrSet(&t->filename, value);
      return true;
    } else if (!strcmp(key2, "_wrap_mode")) {
      t->wrap_mode = ParseWrapMode(value);
      return true;
    } else if (!strcmp(key2, "_mipmap")) {
      t->mipmap = ParseBool(value, NULL);
      return true;
    } else if (!strcmp(key2, "_linear")) {
      t->filter = ParseBool(value, NULL) ? GL_LINEAR : GL_NEAREST;
      return true;
    }
  }
  return false;
}

static GlslParam *GlslShader_GetParam(GlslShader *gs, const char *id) {
  GlslParam **pp = &gs->first_param;
  for (; (*pp) != NULL; pp = &(*pp)->next)
    if (!strcmp((*pp)->id, id))
      return *pp;
  GlslParam *p = (GlslParam *)calloc(1, sizeof(GlslParam));
  *pp = p;
  p->id = strdup(id);
  return p;
}

static void ParseParameters(GlslShader *gs, char *value) {
  char *id;
  while ((id = NextDelim(&value, ';')) != NULL)
    GlslShader_GetParam(gs, id);
}

static bool ParseParameterKeyValue(GlslShader *gs, const char *key, const char *value) {
  for (GlslParam *p = gs->first_param; p != NULL; p = p->next) {
    if (strcmp(p->id, key) == 0) {
      p->value = atof(value);
      p->has_value = true;
      return true;
    }
  }
  return false;
}

static void GlslShader_InitializePasses(GlslShader *gs, int passes) {
  gs->n_pass = passes;
  gs->pass = (GlslPass *)calloc(gs->n_pass + 1, sizeof(GlslPass));
  for (int i = 0; i < gs->n_pass; i++)
    GlslPass_Initialize(gs->pass + i + 1);
}

static bool GlslShader_ReadPresetFile(GlslShader *gs, const char *filename) {
  char *data = (char *)ReadWholeFile(filename, NULL), *data_org = data, *line;
  GlslPass *pass;
  if (data == NULL)
    return false;
  for (int lineno = 1; (line = NextLineStripComments(&data)) != NULL; lineno++) {
    char *value = SplitKeyValue(line), *t;
    if (value == NULL) {
      if (*line)
        fprintf(stderr, "%s:%d: Expecting key=value\n", filename, lineno);
      continue;
    }
    if (*value == '"') {
      for (t = ++value; *t && *t != '"'; t++);
      if (*t) *t = 0;
    }

    if (gs->n_pass == 0) {
      if (strcmp(line, "shaders") != 0) {
        fprintf(stderr, "%s:%d: Expecting 'shaders'\n", filename, lineno);
        break;
      }
      int passes = strtoul(value, NULL, 10);
      if (passes < 1 || passes > kGlslMaxPasses)
        break;
      GlslShader_InitializePasses(gs, passes);
      continue;
    }
    if ((pass = ParseConfigKeyPass(gs, line, "filter_linear")) != NULL)
      pass->filter = ParseBool(value, NULL) ? GL_LINEAR : GL_NEAREST;
    else if ((pass = ParseConfigKeyPass(gs, line, "scale_type")) != NULL)
      pass->scale_type_x = pass->scale_type_y = ParseScaleType(value);
    else if ((pass = ParseConfigKeyPass(gs, line, "scale_type_x")) != NULL)
      pass->scale_type_x = ParseScaleType(value);
    else if ((pass = ParseConfigKeyPass(gs, line, "scale_type_y")) != NULL)
      pass->scale_type_y = ParseScaleType(value);
    else if ((pass = ParseConfigKeyPass(gs, line, "scale")) != NULL)
      pass->scale_x = pass->scale_y = atof(value);
    else if ((pass = ParseConfigKeyPass(gs, line, "scale_x")) != NULL)
      pass->scale_x = atof(value);
    else if ((pass = ParseConfigKeyPass(gs, line, "scale_y")) != NULL)
      pass->scale_y = atof(value);
    else if ((pass = ParseConfigKeyPass(gs, line, "shader")) != NULL)
      StrSet(&pass->filename, value);
    else if ((pass = ParseConfigKeyPass(gs, line, "wrap_mode")) != NULL)
      pass->wrap_mode = ParseWrapMode(value);
    else if ((pass = ParseConfigKeyPass(gs, line, "mipmap_input")) != NULL)
      pass->mipmap_input = ParseBool(value, NULL);
    else if ((pass = ParseConfigKeyPass(gs, line, "frame_count_mod")) != NULL)
      pass->frame_count_mod = atoi(value);
    else if ((pass = ParseConfigKeyPass(gs, line, "float_framebuffer")) != NULL)
      pass->float_framebuffer = ParseBool(value, NULL);
    else if ((pass = ParseConfigKeyPass(gs, line, "srgb_framebuffer")) != NULL)
      pass->srgb_framebuffer = ParseBool(value, NULL);
    else if ((pass = ParseConfigKeyPass(gs, line, "alias")) != NULL)
      ;
    else if (strcmp(line, "textures") == 0 && gs->first_texture == NULL)
      ParseTextures(gs, value);
    else if (strcmp(line, "parameters") == 0)
      ParseParameters(gs, value);
    else if (!ParseTextureKeyValue(gs, line, value) && !ParseParameterKeyValue(gs, line, value))
      fprintf(stderr, "%s:%d: Unknown key '%s'\n", filename, lineno, line);
  }
  free(data_org);
  return gs->n_pass != 0;
}

void GlslShader_ReadShaderFile(GlslShader *gs, const char *filename, ByteArray *result) {
  char *data = (char *)ReadWholeFile(filename, NULL), *data_org = data, *line;
  if (data == NULL) {
    fprintf(stderr, "Unable to read file '%s'\n", filename);
    return;
  }
  while ((line = NextDelim(&data, '\n')) != NULL) {
    size_t linelen = strlen(line);
    if (linelen >= 8 && memcmp(line, "#include", 8) == 0) {
      char *tt = line + 8;
      char *new_filename = ReplaceFilenameWithNewPath(filename, NextPossiblyQuotedString(&tt));
      GlslShader_ReadShaderFile(gs, new_filename, result);
      free(new_filename);
    } else if (linelen >= 17 && memcmp(line, "#pragma parameter", 17) == 0) {
      char *tt = line + 17;
      GlslParam *param = GlslShader_GetParam(gs, NextPossiblyQuotedString(&tt));
      NextPossiblyQuotedString(&tt); // skip name
      float value = atof(NextPossiblyQuotedString(&tt));
      if (!param->has_value)
        param->value = value;
      param->min = atof(NextPossiblyQuotedString(&tt));
      param->max = atof(NextPossiblyQuotedString(&tt));
      // skip step
    } else {
      line[linelen] = '\n';
      ByteArray_AppendData(result, (uint8 *)line, linelen + 1);
    }
  }
  free(data_org);
}

size_t LengthOfInitialComments(const uint8 *data, size_t size) {
  const uint8 *data_org = data, *data_end = data + size;
  while (data != data_end) {
    uint8 c = *data++;
    if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
      continue;
    if (c != '/' || data == data_end) {
      data--;
      break;
    }
    c = *data++;
    if (c == '/') {
      while (data != data_end && *data++ != '\n') {}
    } else if (c == '*') {
      do {
        if (data_end - data < 2)
          return 0;
      } while (*data++ != '*' || data[0] != '/');
      data++;
    } else {
      data--;
      break;
    }
  }
  return data - data_org;
}

static bool GlslPass_Compile(GlslPass *p, uint type, const uint8 *data, size_t size, bool use_opengl_es) {
  static const char kVertexPrefix[] =   "#define VERTEX\n#define PARAMETER_UNIFORM\n";
  static const char kFragmentPrefixCore[] = "#define FRAGMENT\n#define PARAMETER_UNIFORM\n";
  static const char kFragmentPrefixEs[] = "#define FRAGMENT\n#define PARAMETER_UNIFORM\n" \
					"precision mediump float;";
  const GLchar *strings[3];
  GLint lengths[3];
  char buffer[256];
  GLint compile_status = 0;
  size_t skip = 0;

  size_t commsize = LengthOfInitialComments(data, size);
  data += commsize, size -= commsize;

  if (size < 8 || memcmp(data, "#version", 8) != 0) {
    if (!use_opengl_es) {
      strings[0] = "#version 330\n";
      lengths[0] = sizeof("#version 330\n") - 1;
    } else {
      strings[0] = "#version 300 es\n";
      lengths[0] = sizeof("#version 300 es\n") - 1;
    }
  } else {
    while (skip < size && data[skip++] != '\n') {}
    strings[0] = (char*)data;
    lengths[0] = (int)skip;
  }
  if (type == GL_VERTEX_SHADER) {
    strings[1] = (char *)kVertexPrefix;
    lengths[1] = sizeof(kVertexPrefix) - 1;
  } else {
    strings[1] = (use_opengl_es) ? (char *)kFragmentPrefixEs : kFragmentPrefixCore;
    lengths[1] = (use_opengl_es) ? sizeof(kFragmentPrefixEs) - 1 : sizeof(kFragmentPrefixCore) - 1;
  }
  strings[2] = (GLchar *)data + skip;
  lengths[2] = (int)(size - skip);
  uint shader = glCreateShader(type);
  glShaderSource(shader, 3, strings, lengths);
  glCompileShader(shader);
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
  buffer[0] = 0;
  glGetShaderInfoLog(shader, sizeof(buffer), NULL, buffer);
  if (compile_status != GL_TRUE || buffer[0]) {
    fprintf(stderr, "%s compiling %s shader in file '%s':\n%s\n",
            compile_status != GL_TRUE ? "Error" : "While",
            type == GL_VERTEX_SHADER ? "vertex" : "fragment", p->filename, buffer);
  }
  if (compile_status == GL_TRUE)
    glAttachShader(p->gl_program, shader);
  glDeleteShader(shader);
  return (compile_status == GL_TRUE);
}

static void GlslTextureUniform_Read(uint program, const char *prefix, int i, GlslTextureUniform *result) {
  char buf[40];
  char *e = &buf[snprintf(buf, sizeof(buf), i >= 0 ? "%s%u" : "%s", prefix, i)];
  memcpy(e, "Texture", 8);
  result->Texture = glGetUniformLocation(program, buf);
  memcpy(e, "InputSize", 10);
  result->InputSize = glGetUniformLocation(program, buf);
  memcpy(e, "TextureSize", 12);
  result->TextureSize = glGetUniformLocation(program, buf);
  memcpy(e, "TexCoord", 9);
  result->TexCoord = glGetAttribLocation(program, buf);
}

static const float kMvpMatrixOrtho[16] = {
   2.0f,  0.0f,  0.0f,  0.0f,
   0.0f,  2.0f,  0.0f,  0.0f,
   0.0f,  0.0f, -1.0f,  0.0f,
  -1.0f, -1.0f,  0.0f,  1.0f
};

static void GlslShader_GetUniforms(GlslShader *gs) {
  int pass_idx = 1;
  gs->max_prev_frame = 0;
  for (GlslPass *p = gs->pass + 1, *p_end = p + gs->n_pass; p < p_end; p++, pass_idx++) {
    uint program = p->gl_program;
    glUseProgram(program);

    GLint MVPMatrix = glGetUniformLocation(program, "MVPMatrix");
    if (MVPMatrix >= 0)
      glUniformMatrix4fv(MVPMatrix, 1, GL_FALSE, kMvpMatrixOrtho);

    GlslTextureUniform_Read(program, "", -1, &p->unif.Top);
    p->unif.OutputSize = glGetUniformLocation(program, "OutputSize");
    p->unif.FrameCount = glGetUniformLocation(program, "FrameCount");
    p->unif.FrameDirection = glGetUniformLocation(program, "FrameDirection");
    p->unif.LUTTexCoord = glGetAttribLocation(program, "LUTTexCoord");
    p->unif.VertexCoord = glGetAttribLocation(program, "VertexCoord");
    GlslTextureUniform_Read(program, "Orig", -1, &p->unif.Orig);
    for (int j = 0; j < 7; j++) {
      GlslTextureUniform_Read(program, "Prev", j ? j : -1, &p->unif.Prev[j]);
      if (p->unif.Prev[j].Texture >= 0)
        gs->max_prev_frame = j + 1;
    }
    for (int j = 0; j < gs->n_pass; j++) {
      GlslTextureUniform_Read(program, "Pass", j, &p->unif.Pass[j]);
      GlslTextureUniform_Read(program, "PassPrev", j, &p->unif.PassPrev[j]);
    }
    GlslTexture *t = gs->first_texture;
    for (int j = 0; t != NULL; t = t->next, j++)
      p->unif.Texture[j] = glGetUniformLocation(program, t->id);
    for (GlslParam *pa = gs->first_param; pa != NULL; pa = pa->next)
      pa->uniform[pass_idx] = glGetUniformLocation(program, pa->id);
  }
  glUseProgram(0);
}

static bool IsGlslFilename(const char *filename) {
  size_t len = strlen(filename);
  return len >= 5 && memcmp(filename + len - 5, ".glsl", 5) == 0;
}

GlslShader *GlslShader_CreateFromFile(const char *filename, bool opengl_es) {
  char buffer[256];
  GLint link_status;
  ByteArray shader_code = { 0 };
  bool success = false;
  GlslShader *gs = (GlslShader *)calloc(sizeof(GlslShader), 1);
  if (!gs)
    return gs;

  if (IsGlslFilename(filename)) {
    GlslShader_InitializePasses(gs, 1);
    gs->pass[1].filename = strdup(filename);
    filename = "";
  } else {
    if (!GlslShader_ReadPresetFile(gs, filename)) {
      fprintf(stderr, "Unable to read file '%s'\n", filename);
      goto FAIL;
    }
  }
  for (int i = 1; i <= gs->n_pass; i++) {
    GlslPass *p = gs->pass + i;
    shader_code.size = 0;

    if (p->filename == NULL) {
      fprintf(stderr, "shader%d attribute missing\n", i - 1);
      goto FAIL;
    }

    char *new_filename = ReplaceFilenameWithNewPath(filename, p->filename);
    GlslShader_ReadShaderFile(gs, new_filename, &shader_code);
    free(new_filename);

    if (shader_code.size == 0) {
      fprintf(stderr, "Couldn't read shader in file '%s'\n", p->filename);
      goto FAIL;
    }
    p->gl_program = glCreateProgram();
    if (!GlslPass_Compile(p, GL_VERTEX_SHADER, shader_code.data, shader_code.size, opengl_es) ||
        !GlslPass_Compile(p, GL_FRAGMENT_SHADER, shader_code.data, shader_code.size, opengl_es)) {
      goto FAIL;
    }
    glLinkProgram(p->gl_program);
    glGetProgramiv(p->gl_program, GL_LINK_STATUS, &link_status);
    buffer[0] = 0;
    glGetProgramInfoLog(p->gl_program, sizeof(buffer), NULL, buffer);
    if (link_status != GL_TRUE || buffer[0])
      fprintf(stderr, "%s linking shader in file '%s':\n%s\n",
              link_status != GL_TRUE ? "Error" : "While", p->filename, buffer);
    if (link_status != GL_TRUE)
      goto FAIL;
    glGenFramebuffers(1, &p->gl_fbo);
    glGenTextures(1, &p->gl_texture);
  }
  for (GlslTexture *t = gs->first_texture; t; t = t->next) {
    glGenTextures(1, &t->gl_texture);
    glBindTexture(GL_TEXTURE_2D, t->gl_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, t->wrap_mode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, t->wrap_mode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, t->filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, t->mipmap ? 
                    (t->filter == GL_LINEAR ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST) : t->filter);
    if (t->filename) {
      char *new_filename = ReplaceFilenameWithNewPath(filename, t->filename);
      int imw, imh, imn;
      unsigned char *data = stbi_load(new_filename, &imw, &imh, &imn, 0);
      if (!data) {
        fprintf(stderr, "Unable to read PNG '%s'\n", new_filename);
      } else {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imw, imh, 0, 
                     (imn == 4) ? GL_RGBA : (imn == 3) ? GL_RGB : GL_LUMINANCE, GL_UNSIGNED_BYTE, data);
      }
      free(data);
      free(new_filename);
    }
    if (t->mipmap)
      glGenerateMipmap(GL_TEXTURE_2D);
  }
  for (GlslParam *p = gs->first_param; p; p = p->next)
    p->value = (p->value < p->min) ? p->min : (p->value > p->max) ? p->max : p->value;

  GlslShader_GetUniforms(gs);

  for (int i = 0; i < gs->max_prev_frame; i++)
    glGenTextures(1, &gs->prev_frame[-i - 1 & 7].gl_texture);

  static const GLfloat kTexCoords[16] = {
    0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
    0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f
  };
  glGenBuffers(1, &gs->gl_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, gs->gl_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(kTexCoords), kTexCoords, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  success = true;
FAIL:
  if (!success) {
    GlslShader_Destroy(gs);
    gs = NULL;
  }
  ByteArray_Destroy(&shader_code);
  return gs;
}

void GlslShader_Destroy(GlslShader *gs) {
  for (GlslPass *p = gs->pass + 1, *p_end = p + gs->n_pass; p < p_end; p++) {
    glDeleteProgram(p->gl_program);
    glDeleteTextures(1, &p->gl_texture);
    glDeleteFramebuffers(1, &p->gl_fbo);
    free(p->filename);
  }
  free(gs->pass);
  GlslTexture *t;
  while ((t = gs->first_texture) != NULL) {
    gs->first_texture = t->next;
    glDeleteTextures(1, &t->gl_texture);
    free(t->id);
    free(t->filename);
    free(t);
  }
  GlslParam *pp;
  while ((pp = gs->first_param) != NULL) {
    gs->first_param = pp->next;
    free(pp->id);
    free(pp);
  }
  for (int i = 0; i < 8; i++)
    glDeleteTextures(1, &gs->prev_frame[i].gl_texture);
  glDeleteBuffers(1, &gs->gl_vbo);
  free(gs);
}

enum {
  kMaxVaosInRenderCtx = 11 + kGlslMaxPasses * 2
};

typedef struct RenderCtx {
  uint texture_unit;
  uint offset;
  uint num_vaos;
  uint vaos[kMaxVaosInRenderCtx];
} RenderCtx;

static void RenderCtx_SetTexture(RenderCtx *ctx, int textureu, uint texture_id) {
  if (textureu >= 0) {
    glActiveTexture(GL_TEXTURE0 + ctx->texture_unit);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glUniform1i(textureu, ctx->texture_unit++);
  }
}

static void RenderCtx_SetTexCoords(RenderCtx *ctx, int tex_coord, uintptr_t offset) {
  if (tex_coord >= 0) {
    assert(ctx->num_vaos < kMaxVaosInRenderCtx);
    ctx->vaos[ctx->num_vaos++] = tex_coord;
    glVertexAttribPointer(tex_coord, 2, GL_FLOAT, GL_FALSE, 0, (void *)offset);
    glEnableVertexAttribArray(tex_coord);
  }
}

static void RenderCtx_SetGlslTextureUniform(RenderCtx *ctx, GlslTextureUniform *u,
                                            int width, int height, uint texture) {
  float size[2] = { width, height };
  RenderCtx_SetTexture(ctx, u->Texture, texture);
  if (u->InputSize >= 0)
    glUniform2fv(u->InputSize, 1, size);
  if (u->TextureSize >= 0)
    glUniform2fv(u->TextureSize, 1, size);
  RenderCtx_SetTexCoords(ctx, u->TexCoord, ctx->offset);
}

static void GlslShader_SetShaderVars(GlslShader *gs, RenderCtx *ctx, int pass) {
  GlslPass *p = &gs->pass[pass];
    
  RenderCtx_SetGlslTextureUniform(ctx, &p->unif.Top, p[-1].width, p[-1].height, p[-1].gl_texture);
  if (p->unif.OutputSize >= 0) {
    float output_size[2] = { (float)p[0].width, (float)p[0].height };
    glUniform2fv(p->unif.OutputSize, 1, output_size);
  }
  if (p->unif.FrameCount >= 0)
    glUniform1i(p->unif.FrameCount, p->frame_count_mod ? 
                gs->frame_count % p->frame_count_mod : gs->frame_count);
  if (p->unif.FrameDirection >= 0)
    glUniform1i(p->unif.FrameDirection, 1);
  RenderCtx_SetTexCoords(ctx, p->unif.LUTTexCoord, ctx->offset);
  RenderCtx_SetTexCoords(ctx, p->unif.VertexCoord, 0);
  RenderCtx_SetGlslTextureUniform(ctx, &p->unif.Orig, gs->pass[0].width, gs->pass[0].height, gs->pass[0].gl_texture);
  // Prev, Prev1-Prev6 uniforms
  for (int i = 0; i < gs->max_prev_frame; i++) {
    GlTextureWithSize *t = &gs->prev_frame[(gs->frame_count - 1 - i) & 7];
    assert(t->gl_texture != 0);
    if (t->width)
      RenderCtx_SetGlslTextureUniform(ctx, &p->unif.Prev[i], t->width, t->height, t->gl_texture);
  }
  // Texture uniforms
  int tctr = 0;
  for (GlslTexture *t = gs->first_texture; t; t = t->next, tctr++)
    RenderCtx_SetTexture(ctx, p->unif.Texture[tctr], t->gl_texture);
  // PassX uniforms
  for (int i = 1; i < pass; i++)
    RenderCtx_SetGlslTextureUniform(ctx, &p->unif.Pass[i], gs->pass[i].width, gs->pass[i].height, gs->pass[i].gl_texture);
  // PassPrevX uniforms
  for (int i = 1; i < pass; i++)
    RenderCtx_SetGlslTextureUniform(ctx, &p->unif.PassPrev[pass - i], gs->pass[i].width, gs->pass[i].height, gs->pass[i].gl_texture);
  // #parameter uniforms
  for (GlslParam *pa = gs->first_param; pa != NULL; pa = pa->next)
    if (pa->uniform[pass] >= 0)
      glUniform1f(pa->uniform[pass], pa->value);

  glActiveTexture(GL_TEXTURE0);
}

void GlslShader_Render(GlslShader *gs, GlTextureWithSize *tex, int viewport_x, int viewport_y, int viewport_width, int viewport_height) {
  gs->pass[0].gl_texture = tex->gl_texture;
  gs->pass[0].width = tex->width;
  gs->pass[0].height = tex->height;

  GLint previous_framebuffer;
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previous_framebuffer);
  glBindBuffer(GL_ARRAY_BUFFER, gs->gl_vbo);

  for (int pass = 1; pass <= gs->n_pass; pass++) {
    bool last_pass = pass == gs->n_pass;
    GlslPass *p = gs->pass + pass;

    switch (p->scale_type_x) {
    case GLSL_ABSOLUTE: p->width = (uint16)p->scale_x; break;
    case GLSL_SOURCE: p->width = (uint16)(p[-1].width * p->scale_x); break;
    case GLSL_VIEWPORT: p->width = (uint16)(viewport_width * p->scale_x); break;
    default: p->width = (uint16)(last_pass ? viewport_width : (p[-1].width * p->scale_x)); break;
    }

    switch (p->scale_type_y) {
    case GLSL_ABSOLUTE: p->height = (uint16)p->scale_y; break;
    case GLSL_SOURCE: p->height = (uint16)(p[-1].height * p->scale_y); break;
    case GLSL_VIEWPORT: p->height = (uint16)(viewport_height * p->scale_y); break;
    default: p->height = (uint16)(last_pass ? viewport_height : (p[-1].height * p->scale_y)); break;
    }

    if (!last_pass) {
      // output to a texture
      glBindTexture(GL_TEXTURE_2D, p->gl_texture);
      if (p->srgb_framebuffer) {
        glEnable(GL_FRAMEBUFFER_SRGB);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, p->width, p->height, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, NULL);
      } else {
        glTexImage2D(GL_TEXTURE_2D, 0, p->float_framebuffer ? GL_RGBA32F : GL_RGBA,
                     p->width, p->height, 0, GL_RGBA,
                     p->float_framebuffer ? GL_FLOAT : GL_UNSIGNED_INT_8_8_8_8, NULL);
      }
      glViewport(0, 0, p->width, p->height);
      glBindFramebuffer(GL_FRAMEBUFFER, p->gl_fbo);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, p->gl_texture, 0);
    } else {
      // output to screen
      glBindFramebuffer(GL_FRAMEBUFFER, previous_framebuffer);
      glViewport(viewport_x, viewport_y, viewport_width, viewport_height);
    }

    glBindTexture(GL_TEXTURE_2D, p[-1].gl_texture);

    uint filter = p->filter ? p->filter : (last_pass && g_config.linear_filtering) ? GL_LINEAR : GL_NEAREST;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, p->mipmap_input ? 
                    (filter == GL_LINEAR ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST) : filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, p->wrap_mode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, p->wrap_mode);
    if (p->mipmap_input)
      glGenerateMipmap(GL_TEXTURE_2D);
    glUseProgram(p->gl_program);

    RenderCtx ctx;
    ctx.texture_unit = ctx.num_vaos = 0;
    ctx.offset = last_pass ? sizeof(float) * 8 : 0;
    GlslShader_SetShaderVars(gs, &ctx, pass);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    for(int i = 0; i < ctx.num_vaos; i++)
      glDisableVertexAttribArray(ctx.vaos[i]);
    if (p->srgb_framebuffer)
      glDisable(GL_FRAMEBUFFER_SRGB);
  }

  glBindFramebuffer(GL_FRAMEBUFFER, previous_framebuffer);
  glUseProgram(0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Store the input frame in the prev array, and extract the next one.
  if (gs->max_prev_frame != 0) {
    // 01234567
    //    43210
    // ^-- store pos
    //    ^-- load pos
    GlTextureWithSize *store_pos = &gs->prev_frame[gs->frame_count & 7];
    GlTextureWithSize *load_pos = &gs->prev_frame[gs->frame_count - gs->max_prev_frame & 7];
    assert(store_pos->gl_texture == 0);
    *store_pos = *tex;
    *tex = *load_pos;
    memset(load_pos, 0, sizeof(GlTextureWithSize));
  }

  gs->frame_count++;
}

