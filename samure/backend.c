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
