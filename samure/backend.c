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

#include "backend.h"
#include "backends/cairo.h"
#include "backends/opengl.h"
#include "context.h"
#include <dlfcn.h>

#define SAMURE_DLSYM(func_name)                                                \
  void *func_name = dlsym(lib, #func_name);                                    \
  if (func_name) {                                                             \
    DEBUG_PRINTF("Loaded symbol %s\n", #func_name);                            \
  } else {                                                                     \
    DEBUG_PRINTF("Failed to load symbol %s\n", #func_name);                    \
  }

// public
typedef SAMURE_RESULT(backend) (*backend_init_t)(struct samure_context *ctx);

SAMURE_DEFINE_RESULT_UNWRAP(backend);

extern SAMURE_RESULT(backend) samure_create_backend(
    samure_on_layer_surface_configure_t on_layer_surface_configure,
    samure_render_start_t render_start, samure_render_end_t render_end,
    samure_destroy_t destroy,
    samure_associate_layer_surface_t associate_layer_surface,
    samure_unassociate_layer_surface_t unassociate_layer_surface) {
  SAMURE_RESULT_ALLOC(backend, b);

  b->on_layer_surface_configure = on_layer_surface_configure;
  b->render_start = render_start;
  b->render_end = render_end;
  b->destroy = destroy;
  b->associate_layer_surface = associate_layer_surface;
  b->unassociate_layer_surface = unassociate_layer_surface;

  SAMURE_RETURN_RESULT(backend, b);
}

extern SAMURE_RESULT(backend)
    samure_create_backend_from_lib(struct samure_context *ctx,
                                   const char *lib_name,
                                   const char *depend_lib_name) {
  DEBUG_PRINTF("Creating Backend from %s with depend %s\n", lib_name,
               depend_lib_name ? depend_lib_name : "no depend");

  if (depend_lib_name) {
    // Check if dependlib exists
    void *depend_lib = dlopen(depend_lib_name, RTLD_LAZY);
    if (!depend_lib) {
      SAMURE_RETURN_ERROR(backend, SAMURE_ERROR_NO_DEPEND_LIB);
    }
    dlclose(depend_lib);
  }

  // Open lib
  void *lib = dlopen(lib_name, RTLD_NOW);
  if (!lib) {
    SAMURE_RETURN_ERROR(backend, SAMURE_ERROR_NO_LIB);
  }
  ctx->backend_lib_handle = lib;

  SAMURE_DLSYM(init);

  if (init) {
    backend_init_t init_func = init;
    return init_func(ctx);
  }

  SAMURE_DLSYM(on_layer_surface_configure);
  SAMURE_DLSYM(render_start);
  SAMURE_DLSYM(render_end);
  SAMURE_DLSYM(destroy);
  SAMURE_DLSYM(associate_layer_surface);
  SAMURE_DLSYM(unassociate_layer_surface);

  return samure_create_backend(on_layer_surface_configure, render_start,
                               render_end, destroy, associate_layer_surface,
                               unassociate_layer_surface);
}

struct samure_cairo_surface *
samure_get_cairo_surface(struct samure_layer_surface *layer_surface) {
  return (struct samure_cairo_surface *)layer_surface->backend_data;
}

struct samure_opengl_config *samure_default_opengl_config() {
  struct samure_opengl_config *cfg =
      malloc(sizeof(struct samure_opengl_config));
  assert(cfg != NULL);
  memset(cfg, 0, sizeof(struct samure_opengl_config));

  cfg->red_size = 8;
  cfg->green_size = 8;
  cfg->blue_size = 8;
  cfg->alpha_size = 8;
  cfg->major_version = 1;
  cfg->profile_mask = EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT;
  cfg->color_space = EGL_GL_COLORSPACE_LINEAR;
  cfg->render_buffer = EGL_BACK_BUFFER;
  cfg->debug = EGL_FALSE;

  return cfg;
}

struct samure_opengl_surface *
samure_get_opengl_surface(struct samure_layer_surface *layer_surface) {
  return (struct samure_opengl_surface *)layer_surface->backend_data;
}

// Backwards compatiblity
struct samure_backend *cairo_backend = NULL;
struct samure_backend *opengl_backend = NULL;

SAMURE_RESULT(backend) samure_init_backend_cairo(struct samure_context *ctx) {
  SAMURE_RESULT(backend)
  b_rs = samure_create_backend_from_lib(
      ctx, "libsamurai-render-backend-cairo.so", "libcairo.so");
  cairo_backend = b_rs.result;
  return b_rs;
}

SAMURE_RESULT(backend) samure_init_backend_opengl(struct samure_context *ctx) {
  SAMURE_RESULT(backend)
  b_rs = samure_create_backend_from_lib(
      ctx, "libsamurai-render-backend-opengl.so", "libEGL.so");
  opengl_backend = b_rs.result;
  return b_rs;
}

void samure_destroy_backend_cairo(struct samure_context *ctx) {
  cairo_backend->destroy(ctx);
}

void samure_backend_cairo_render_end(
    struct samure_context *ctx, struct samure_layer_surface *layer_surface) {
  cairo_backend->render_end(ctx, layer_surface);
}

samure_error samure_backend_cairo_associate_layer_surface(
    struct samure_context *ctx, struct samure_layer_surface *layer_surface) {
  return cairo_backend->associate_layer_surface(ctx, layer_surface);
}

void samure_backend_cairo_on_layer_surface_configure(
    struct samure_context *ctx, struct samure_layer_surface *layer_surface,
    int32_t width, int32_t height) {
  cairo_backend->on_layer_surface_configure(ctx, layer_surface, width, height);
}

void samure_backend_cairo_unassociate_layer_surface(
    struct samure_context *ctx, struct samure_layer_surface *layer_surface) {
  cairo_backend->unassociate_layer_surface(ctx, layer_surface);
}

void samure_destroy_backend_opengl(struct samure_context *ctx) {
  opengl_backend->destroy(ctx);
}

void samure_backend_opengl_render_start(
    struct samure_context *ctx, struct samure_layer_surface *layer_surface) {
  opengl_backend->render_start(ctx, layer_surface);
}

void samure_backend_opengl_render_end(
    struct samure_context *ctx, struct samure_layer_surface *layer_surface) {
  opengl_backend->render_end(ctx, layer_surface);
}

samure_error samure_backend_opengl_associate_layer_surface(
    struct samure_context *ctx, struct samure_layer_surface *layer_surface) {
  return opengl_backend->associate_layer_surface(ctx, layer_surface);
}

void samure_backend_opengl_on_layer_surface_configure(
    struct samure_context *ctx, struct samure_layer_surface *layer_surface,
    int32_t width, int32_t height) {
  opengl_backend->on_layer_surface_configure(ctx, layer_surface, width, height);
}

void samure_backend_opengl_unassociate_layer_surface(
    struct samure_context *ctx, struct samure_layer_surface *layer_surface) {
  opengl_backend->unassociate_layer_surface(ctx, layer_surface);
}

void samure_backend_opengl_make_context_current(
    struct samure_backend *gl, struct samure_layer_surface *layer_surface) {
  struct samure_context ctx;
  ctx.backend = gl;
  gl->render_start(&ctx, layer_surface);
}
