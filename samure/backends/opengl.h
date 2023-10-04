#pragma once
#include <EGL/egl.h>
#include <wayland-client.h>
#include <wayland-egl.h>

#include "../backend.h"
#include "../error_handling.h"

#define EGL_PLATFORM_WAYLAND_KHR 0x31D8

typedef EGLDisplay (*eglGetPlatformDisplayEXT_t)(EGLenum, EGLNativeDisplayType,
                                                 const EGLint *);
typedef EGLSurface (*eglCreatePlatformWindowSurfaceEXT_t)(EGLDisplay, EGLConfig,
                                                          EGLNativeWindowType,
                                                          const EGLint *);
extern eglGetPlatformDisplayEXT_t eglGetPlatformDisplayEXT;
extern eglCreatePlatformWindowSurfaceEXT_t eglCreatePlatformWindowSurfaceEXT;

struct samure_opengl_config {
  int red_size;
  int green_size;
  int blue_size;
  int alpha_size;
  int samples;
  int depth_size;
  int major_version;
  int minor_version;
  int profile_mask;
  int debug;
  int color_space;
  int render_buffer;
};

extern struct samure_opengl_config *samure_default_opengl_config();

struct samure_opengl_surface {
  EGLSurface surface;
  struct wl_egl_window *egl_window;
};

struct samure_backend_opengl {
  struct samure_backend base;

  EGLDisplay display;
  EGLContext context;
  EGLConfig config;
  struct samure_opengl_config *cfg;
};

SAMURE_DEFINE_RESULT(backend_opengl);

extern SAMURE_RESULT(backend_opengl)
    samure_init_backend_opengl(struct samure_context *ctx,
                               struct samure_opengl_config *cfg);
extern void samure_destroy_backend_opengl(struct samure_context *ctx);
extern void
samure_backend_opengl_render_start(struct samure_context *ctx,
                                   struct samure_layer_surface *layer_surface);
extern void
samure_backend_opengl_render_end(struct samure_context *ctx,
                                 struct samure_layer_surface *layer_surface);
extern samure_error samure_backend_opengl_associate_layer_surface(
    struct samure_context *ctx, struct samure_layer_surface *layer_surface);
extern void samure_backend_opengl_on_layer_surface_configure(
    struct samure_context *ctx, struct samure_layer_surface *layer_surface,
    int32_t width, int32_t height);
extern void samure_backend_opengl_unassociate_layer_surface(
    struct samure_context *ctx, struct samure_layer_surface *layer_surface);
extern struct samure_backend_opengl *
samure_get_backend_opengl(struct samure_context *ctx);
extern struct samure_opengl_surface *
samure_get_opengl_surface(struct samure_layer_surface *layer_surface);

extern void samure_backend_opengl_make_context_current(
    struct samure_backend_opengl *gl,
    struct samure_layer_surface *layer_surface);
