/***********************************************************************************
 *                         This file is part of samurai-render
 *                    https://github.com/Samudevv/samurai-render
 ***********************************************************************************
 * Copyright (c) 2023 Jonas Pucher
 *
 * This software is provided ‘as-is’, without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 ************************************************************************************/

#include "opengl.h"
#include "../context.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wayland-egl.h>

#define EGL_PLATFORM_WAYLAND_KHR 0x31D8

typedef EGLDisplay (*eglGetPlatformDisplayEXT_t)(EGLenum, EGLNativeDisplayType,
                                                 const EGLint *);
typedef EGLSurface (*eglCreatePlatformWindowSurfaceEXT_t)(EGLDisplay, EGLConfig,
                                                          EGLNativeWindowType,
                                                          const EGLint *);
extern eglGetPlatformDisplayEXT_t eglGetPlatformDisplayEXT;
extern eglCreatePlatformWindowSurfaceEXT_t eglCreatePlatformWindowSurfaceEXT;

eglGetPlatformDisplayEXT_t eglGetPlatformDisplayEXT = NULL;
eglCreatePlatformWindowSurfaceEXT_t eglCreatePlatformWindowSurfaceEXT = NULL;

struct samure_backend_opengl {
  struct samure_backend base;

  EGLDisplay display;
  EGLContext context;
  EGLConfig config;
  struct samure_opengl_config *cfg;
};

#define SAMURE_BACKEND_OPENGL_DESTROY_ERROR(error_code)                        \
  {                                                                            \
    ctx->backend = (struct samure_backend *)gl;                                \
    opengl_destroy(ctx);                                                       \
    ctx->backend = NULL;                                                       \
    SAMURE_RETURN_ERROR(backend, error_code);                                  \
  }

void opengl_destroy(struct samure_context *ctx) {
  struct samure_backend_opengl *gl =
      (struct samure_backend_opengl *)ctx->backend;

  if (gl->display && gl->context)
    eglDestroyContext(gl->display, gl->context);

  if (gl->display)
    eglTerminate(gl->display);
  if (gl)
    free(gl->cfg);
  free(gl);
  ctx->backend = NULL;
}

void make_context_current(struct samure_backend *_gl,
                          struct samure_layer_surface *layer_surface) {
  struct samure_backend_opengl *gl = (struct samure_backend_opengl *)_gl;
  if (layer_surface) {
    struct samure_opengl_surface *s =
        (struct samure_opengl_surface *)layer_surface->backend_data;
    eglMakeCurrent(gl->display, s->surface, s->surface, gl->context);
  } else {
    eglMakeCurrent(gl->display, EGL_NO_SURFACE, EGL_NO_SURFACE, gl->context);
  }
}

void opengl_render_start(struct samure_context *ctx,
                         struct samure_layer_surface *layer_surface) {
  make_context_current(ctx->backend, layer_surface);
}

void opengl_render_end(struct samure_context *ctx,
                       struct samure_layer_surface *sfc) {
  struct samure_backend_opengl *gl =
      (struct samure_backend_opengl *)ctx->backend;
  struct samure_opengl_surface *s =
      (struct samure_opengl_surface *)sfc->backend_data;

  if (sfc->viewport) {
    wp_viewport_set_destination(sfc->viewport, sfc->w, sfc->h);
    wp_viewport_set_source(
        sfc->viewport, 0, 0,
        wl_fixed_from_double(ceil(GLOBAL_TO_LOCAL_SCALE(sfc, sfc->w))),
        wl_fixed_from_double(ceil(GLOBAL_TO_LOCAL_SCALE(sfc, sfc->h))));
  }

  eglSwapBuffers(gl->display, s->surface);
}

samure_error opengl_associate_layer_surface(struct samure_context *ctx,
                                            struct samure_layer_surface *sfc) {
  struct samure_backend_opengl *gl =
      (struct samure_backend_opengl *)ctx->backend;
  struct samure_opengl_surface *s =
      malloc(sizeof(struct samure_opengl_surface));
  if (!s) {
    return SAMURE_ERROR_MEMORY;
  }

  memset(s, 0, sizeof(struct samure_opengl_surface));

  const uint32_t scaled_width = ceil(RENDER_SCALE(sfc->w));
  const uint32_t scaled_height = ceil(RENDER_SCALE(sfc->h));

  s->egl_window =
      wl_egl_window_create(sfc->surface, scaled_width == 0 ? 1 : scaled_width,
                           scaled_height == 0 ? 1 : scaled_height);
  if (!s->egl_window) {
    free(s);
    return SAMURE_ERROR_OPENGL_WL_EGL_WINDOW_INIT;
  }

  // clang-format off
  const EGLint surface_attributes[] = {
    EGL_GL_COLORSPACE, gl->cfg->color_space,
    EGL_RENDER_BUFFER, gl->cfg->render_buffer,
    EGL_NONE,          EGL_NONE,
  };
  // clang-format on

  s->surface = eglCreatePlatformWindowSurfaceEXT(
      gl->display, gl->config, (EGLNativeWindowType)s->egl_window,
      surface_attributes);
  if (s->surface == EGL_NO_SURFACE) {
    wl_egl_window_destroy(s->egl_window);
    free(s);
    return SAMURE_ERROR_OPENGL_SURFACE_INIT;
  }

  sfc->backend_data = s;
  return SAMURE_ERROR_NONE;
}

void opengl_on_layer_surface_configure(
    struct samure_context *ctx, struct samure_layer_surface *layer_surface,
    int32_t width, int32_t height) {
  if (layer_surface->backend_data) {
    return;
  }

  const int32_t scaled_width =
      ceil(GLOBAL_TO_LOCAL_SCALE(layer_surface, width));
  const int32_t scaled_height =
      ceil(GLOBAL_TO_LOCAL_SCALE(layer_surface, height));

  struct samure_opengl_surface *s =
      (struct samure_opengl_surface *)layer_surface->backend_data;

  wl_egl_window_resize(s->egl_window, scaled_width, scaled_height, 0, 0);
}

void opengl_unassociate_layer_surface(
    struct samure_context *ctx, struct samure_layer_surface *layer_surface) {
  if (!layer_surface->backend_data) {
    return;
  }

  struct samure_backend_opengl *gl =
      (struct samure_backend_opengl *)ctx->backend;
  struct samure_opengl_surface *s =
      (struct samure_opengl_surface *)layer_surface->backend_data;

  if (s->surface)
    eglDestroySurface(gl->display, s->surface);
  if (s->egl_window)
    wl_egl_window_destroy(s->egl_window);
  free(s);
  layer_surface->backend_data = NULL;
}

SAMURE_RESULT(backend)
init(struct samure_context *ctx) {
  struct samure_opengl_config *cfg = ctx->config.gl;

  if (cfg == NULL) {
    cfg = samure_default_opengl_config();
  }

  struct samure_backend_opengl *gl =
      malloc(sizeof(struct samure_backend_opengl));
  if (!gl) {
    SAMURE_RETURN_ERROR(backend, SAMURE_ERROR_MEMORY);
  }
  memset(gl, 0, sizeof(struct samure_backend_opengl));

  eglGetPlatformDisplayEXT =
      (eglGetPlatformDisplayEXT_t)eglGetProcAddress("eglGetPlatformDisplayEXT");
  if (!eglGetPlatformDisplayEXT) {
    SAMURE_BACKEND_OPENGL_DESTROY_ERROR(SAMURE_ERROR_OPENGL_LOAD_PROC);
  }

  eglCreatePlatformWindowSurfaceEXT =
      (eglCreatePlatformWindowSurfaceEXT_t)eglGetProcAddress(
          "eglCreatePlatformWindowSurfaceEXT");
  if (!eglCreatePlatformWindowSurfaceEXT) {
    SAMURE_BACKEND_OPENGL_DESTROY_ERROR(SAMURE_ERROR_OPENGL_LOAD_PROC);
  }

  gl->display =
      eglGetPlatformDisplayEXT(EGL_PLATFORM_WAYLAND_KHR, ctx->display, NULL);
  if (gl->display == EGL_NO_DISPLAY) {
    SAMURE_BACKEND_OPENGL_DESTROY_ERROR(SAMURE_ERROR_OPENGL_DISPLAY_CONNECT);
  }

  if (eglInitialize(gl->display, NULL, NULL) != EGL_TRUE) {
    SAMURE_BACKEND_OPENGL_DESTROY_ERROR(SAMURE_ERROR_OPENGL_INITIALIZE);
  }

  // clang-format off
  const EGLint config_attributes[] = {
      EGL_RED_SIZE,   cfg->red_size,
      EGL_BLUE_SIZE,  cfg->blue_size,
      EGL_GREEN_SIZE, cfg->green_size,
      EGL_ALPHA_SIZE, cfg->alpha_size,
      EGL_DEPTH_SIZE, cfg->depth_size,
      EGL_SAMPLES,    cfg->samples,
      EGL_CONFORMANT, EGL_OPENGL_BIT,
      EGL_NONE,       EGL_NONE,
  };

  EGLint context_attributes[] = {
    EGL_CONTEXT_MAJOR_VERSION,       cfg->major_version,
    EGL_CONTEXT_MINOR_VERSION,       cfg->minor_version,
    EGL_CONTEXT_OPENGL_PROFILE_MASK, cfg->profile_mask,
    EGL_CONTEXT_OPENGL_DEBUG,        cfg->debug,
    EGL_NONE,                        EGL_NONE,
  };

  // clang-format on

  EGLint num_config;
  EGLConfig config;
  if (eglChooseConfig(gl->display, config_attributes, &config, 1,
                      &num_config) != EGL_TRUE) {
    SAMURE_BACKEND_OPENGL_DESTROY_ERROR(SAMURE_ERROR_OPENGL_CONFIG);
  }
  if (num_config == 0) {
    SAMURE_BACKEND_OPENGL_DESTROY_ERROR(SAMURE_ERROR_OPENGL_CONFIG);
  }

  DEBUG_PRINTF("EGL found %d configs\n", num_config);

  if (eglBindAPI(EGL_OPENGL_API) != EGL_TRUE) {
    SAMURE_BACKEND_OPENGL_DESTROY_ERROR(SAMURE_ERROR_OPENGL_BIND_API);
  }

  for (size_t attrib = 6;; attrib -= 2) {
    gl->context = eglCreateContext(gl->display, config, EGL_NO_CONTEXT,
                                   context_attributes);
    if (gl->context || attrib > 6) {
      break;
    }

    context_attributes[attrib + 0] = EGL_NONE;
    context_attributes[attrib + 1] = EGL_NONE;
  }
  if (gl->context == EGL_NO_CONTEXT) {
    SAMURE_BACKEND_OPENGL_DESTROY_ERROR(SAMURE_ERROR_OPENGL_CONTEXT_INIT);
  }

  gl->config = config;
  gl->cfg = cfg;

  gl->base.destroy = opengl_destroy;
  gl->base.render_start = opengl_render_start;
  gl->base.render_end = opengl_render_end;
  gl->base.associate_layer_surface = opengl_associate_layer_surface;
  gl->base.on_layer_surface_configure = opengl_on_layer_surface_configure;
  gl->base.unassociate_layer_surface = opengl_unassociate_layer_surface;

  ctx->config.not_request_frame = 1;

  SAMURE_RETURN_RESULT(backend, (struct samure_backend *)gl);
}
