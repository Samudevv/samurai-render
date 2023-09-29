#pragma once
#include <EGL/egl.h>
#include <wayland-client.h>
#include <wayland-egl.h>

#include "../backend.h"

#define EGL_PLATFORM_WAYLAND_KHR 0x31D8

typedef EGLDisplay (*eglGetPlatformDisplayEXT_t)(EGLenum, EGLNativeDisplayType,
                                                 const EGLint *);
typedef EGLSurface (*eglCreatePlatformWindowSurfaceEXT_t)(EGLDisplay, EGLConfig,
                                                          EGLNativeWindowType,
                                                          const EGLint *);
extern eglGetPlatformDisplayEXT_t eglGetPlatformDisplayEXT;
extern eglCreatePlatformWindowSurfaceEXT_t eglCreatePlatformWindowSurfaceEXT;

struct samure_opengl_surface {
  EGLContext context;
  EGLSurface surface;
  struct wl_egl_window *egl_window;
};

struct samure_backend_opengl {
  struct samure_backend base;

  EGLDisplay display;
  struct samure_opengl_surface *surfaces;
  size_t num_outputs;

  char *error_string;
};

extern struct samure_backend_opengl *
samure_init_backend_opengl(struct samure_context *ctx);
extern void samure_destroy_backend_opengl(struct samure_context *ctx,
                                          struct samure_backend *gl);
extern void samure_backend_opengl_render_start(struct samure_output *output,
                                               struct samure_context *ctx,
                                               struct samure_backend *gl);
extern void samure_backend_opengl_render_end(struct samure_output *output,
                                             struct samure_context *ctx,
                                             struct samure_backend *gl);
extern struct samure_backend_opengl *
samure_get_backend_opengl(struct samure_context *ctx);
