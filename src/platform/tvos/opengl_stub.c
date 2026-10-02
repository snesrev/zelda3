#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "util.h"

struct RendererFuncs;

// tvOS uses SDL's UIKit-backed renderer. Keep the desktop OpenGL entry point
// available to the shared main.c without pulling desktop OpenGL into the app.
void OpenGLRenderer_Create(struct RendererFuncs *funcs, bool use_opengl_es) {
  (void)funcs;
  (void)use_opengl_es;
  fputs("OpenGL output is not available in the tvOS build; use OutputMethod = SDL\n", stderr);
  abort();
}
