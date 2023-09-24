#include "opengl.h"
#include "../context.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

eglGetPlatformDisplayEXT_t eglGetPlatformDisplayEXT = NULL;
eglCreatePlatformWindowSurfaceEXT_t eglCreatePlatformWindowSurfaceEXT = NULL;

struct samure_backend_opengl *
samure_init_backend_opengl(struct samure_context *ctx) {
  struct samure_backend_opengl *gl =
      malloc(sizeof(struct samure_backend_opengl));
  memset(gl, 0, sizeof(struct samure_backend_opengl));

  char *wl_egl_err = open_libwayland_egl();
  if (wl_egl_err) {
    gl->error_string = wl_egl_err;
    return gl;
  }

  eglGetPlatformDisplayEXT =
      (eglGetPlatformDisplayEXT_t)eglGetProcAddress("eglGetPlatformDisplayEXT");
  if (!eglGetPlatformDisplayEXT) {
    gl->error_string = malloc(1024);
    snprintf(gl->error_string, 1024, "failed to load eglGetPlatformDisplayEXT");
    return gl;
  }

  eglCreatePlatformWindowSurfaceEXT =
      (eglCreatePlatformWindowSurfaceEXT_t)eglGetProcAddress(
          "eglCreatePlatformWindowSurfaceEXT");
  if (!eglCreatePlatformWindowSurfaceEXT) {
    gl->error_string = malloc(1024);
    snprintf(gl->error_string, 1024,
             "failed to load eglCreatePlatformWindowSurfaceEXT");
    return gl;
  }

  gl->display =
      eglGetPlatformDisplayEXT(EGL_PLATFORM_WAYLAND_KHR, ctx->display, NULL);
  if (gl->display == EGL_NO_DISPLAY) {
    gl->error_string = malloc(1024);
    snprintf(gl->error_string, 1024, "failed to get egl display connection");
    return gl;
  }

  if (eglInitialize(gl->display, NULL, NULL) != EGL_TRUE) {
    gl->error_string = malloc(1024);
    snprintf(gl->error_string, 1024, "failed to initialize egl");
    return gl;
  }

  EGLint num_config;
  EGLConfig config;
  if (eglGetConfigs(gl->display, &config, 1, &num_config) != EGL_TRUE) {
    gl->error_string = malloc(1024);
    snprintf(gl->error_string, 1024, "failed to get egl config");
    eglTerminate(gl->display);
    return gl;
  }
  if (num_config == 0) {
    gl->error_string = malloc(1024);
    snprintf(gl->error_string, 1024, "did not find any egl configs");
    eglTerminate(gl->display);
    return gl;
  }

  if (eglBindAPI(EGL_OPENGL_API) != EGL_TRUE) {
    gl->error_string = malloc(1024);
    snprintf(gl->error_string, 1024, "failed to bind opengl api");
    eglTerminate(gl->display);
    return gl;
  }

  gl->surfaces =
      malloc(ctx->num_outputs * sizeof(struct samure_opengl_surface));
  gl->num_outputs = ctx->num_outputs;

  const EGLint context_attributes[] = {EGL_NONE, EGL_NONE};

  for (size_t i = 0; i < gl->num_outputs; i++) {
    gl->surfaces[i].context = eglCreateContext(
        gl->display, config, EGL_NO_CONTEXT, context_attributes);
    if (gl->surfaces[i].context == EGL_NO_CONTEXT) {
      gl->error_string = malloc(1024);
      snprintf(gl->error_string, 1024,
               "failed to create egl context for output %zu", i);
      eglTerminate(gl->display);
      return gl;
    }

    gl->surfaces[i].egl_window = wl_egl_window_create(
        ctx->outputs[i].surface, ctx->outputs[i].geo.w, ctx->outputs[i].geo.h);
    if (!gl->surfaces[i].egl_window) {
      gl->error_string = malloc(1024);
      snprintf(gl->error_string, 1024,
               "failed to create wl egl window for output %zu", i);
      eglDestroyContext(gl->display, gl->surfaces[i].context);
      eglTerminate(gl->display);
      return gl;
    }

    gl->surfaces[i].surface = eglCreatePlatformWindowSurfaceEXT(
        gl->display, config, (EGLNativeWindowType)gl->surfaces[i].egl_window,
        context_attributes);
    if (gl->surfaces[i].surface == EGL_NO_SURFACE) {
      gl->error_string = malloc(1024);
      snprintf(gl->error_string, 1024,
               "failed to create egl surface for output %zu", i);
      eglDestroyContext(gl->display, gl->surfaces[i].context);
      wl_egl_window_destroy(gl->surfaces[i].egl_window);
      eglTerminate(gl->display);
      return gl;
    }
  }

  gl->base.destroy = samure_destroy_backend_opengl;
  gl->base.render_start = samure_backend_opengl_render_start;
  gl->base.render_end = samure_backend_opengl_render_end;

  return gl;
}

void samure_destroy_backend_opengl(struct samure_context *ctx,
                                   struct samure_backend *backend) {
  struct samure_backend_opengl *gl = (struct samure_backend_opengl *)backend;
  for (size_t i = 0; i < gl->num_outputs; i++) {
    eglDestroySurface(gl->display, gl->surfaces[i].surface);
    eglDestroyContext(gl->display, gl->surfaces[i].context);
    wl_egl_window_destroy(gl->surfaces[i].egl_window);
  }
  free(gl->surfaces);

  eglTerminate(gl->display);
  free(gl->error_string);
  free(gl);
}

void samure_backend_opengl_render_start(struct samure_output *output,
                                        struct samure_context *ctx,
                                        struct samure_backend *backend) {
  struct samure_backend_opengl *gl = (struct samure_backend_opengl *)backend;
  const uintptr_t i = OUT_IDX();
  eglMakeCurrent(gl->display, gl->surfaces[i].surface, gl->surfaces[i].surface,
                 gl->surfaces[i].context);
}

void samure_backend_opengl_render_end(struct samure_output *output,
                                      struct samure_context *ctx,
                                      struct samure_backend *backend) {
  struct samure_backend_opengl *gl = (struct samure_backend_opengl *)backend;
  const uintptr_t i = OUT_IDX();
  eglSwapBuffers(gl->display, gl->surfaces[i].surface);
}

struct samure_backend_opengl *
samure_get_backend_opengl(struct samure_context *ctx) {
  return (struct samure_backend_opengl *)ctx->backend;
}
