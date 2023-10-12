/***********************************************************************************
 *                         This file is part of samurai-render
 *                    https://github.com/PucklaJ/samurai-render
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

#include "raw.h"
#include "../context.h"
#include "../layer_surface.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

SAMURE_DEFINE_RESULT_UNWRAP(backend_raw);

SAMURE_RESULT(backend_raw)
samure_init_backend_raw(struct samure_context *ctx) {
  SAMURE_RESULT_ALLOC(backend_raw, r);

  r->base.render_end = samure_backend_raw_render_end;
  r->base.destroy = samure_destroy_backend_raw;
  r->base.associate_layer_surface = samure_backend_raw_associate_layer_surface;
  r->base.on_layer_surface_configure =
      samure_backend_raw_on_layer_surface_configure;
  r->base.unassociate_layer_surface =
      samure_backend_raw_unassociate_layer_surface;

  SAMURE_RETURN_RESULT(backend_raw, r);
}

void samure_destroy_backend_raw(struct samure_context *ctx) {
  free(ctx->backend);
  ctx->backend = NULL;
}

void samure_backend_raw_render_end(struct samure_context *ctx,
                                   struct samure_layer_surface *layer_surface) {
  struct samure_raw_surface *r =
      (struct samure_raw_surface *)layer_surface->backend_data;
  samure_layer_surface_draw_buffer(layer_surface, r->buffer);
}

samure_error
samure_backend_raw_associate_layer_surface(struct samure_context *ctx,
                                           struct samure_layer_surface *sfc) {
  struct samure_raw_surface *r = malloc(sizeof(struct samure_raw_surface));
  if (!r) {
    return SAMURE_ERROR_MEMORY;
  }
  memset(r, 0, sizeof(struct samure_raw_surface));

  SAMURE_RESULT(shared_buffer)
  b_rs = samure_create_shared_buffer(ctx->shm, SAMURE_BUFFER_FORMAT,
                                     sfc->w == 0 ? 1 : sfc->w,
                                     sfc->h == 0 ? 1 : sfc->h);
  if (SAMURE_HAS_ERROR(b_rs)) {
    free(r);
    return SAMURE_ERROR_SHARED_BUFFER_INIT | b_rs.error;
  }

  r->buffer = SAMURE_UNWRAP(shared_buffer, b_rs);

  sfc->backend_data = r;
  samure_backend_raw_render_end(ctx, sfc);

  return SAMURE_ERROR_NONE;
}

void samure_backend_raw_on_layer_surface_configure(
    struct samure_context *ctx, struct samure_layer_surface *layer_surface,
    int32_t width, int32_t height) {
  if (!layer_surface->backend_data) {
    return;
  }

  struct samure_raw_surface *r =
      (struct samure_raw_surface *)layer_surface->backend_data;

  if (r->buffer->width == width && r->buffer->height == height) {
    return;
  }

  if (r->buffer) {
    samure_destroy_shared_buffer(r->buffer);
  }

  SAMURE_RESULT(shared_buffer)
  b_rs = samure_create_shared_buffer(ctx->shm, SAMURE_BUFFER_FORMAT, width,
                                     height);
  if (SAMURE_HAS_ERROR(b_rs)) {
    r->buffer = NULL;
  } else {
    r->buffer = SAMURE_UNWRAP(shared_buffer, b_rs);
  }
}

void samure_backend_raw_unassociate_layer_surface(
    struct samure_context *ctx, struct samure_layer_surface *sfc) {
  if (!sfc->backend_data) {
    return;
  }

  struct samure_raw_surface *r = (struct samure_raw_surface *)sfc->backend_data;

  samure_destroy_shared_buffer(r->buffer);
  free(r);
  sfc->backend_data = NULL;
}

struct samure_backend_raw *samure_get_backend_raw(struct samure_context *ctx) {
  return (struct samure_backend_raw *)ctx->backend;
}

struct samure_raw_surface *
samure_get_raw_surface(struct samure_layer_surface *layer_surface) {
  return (struct samure_raw_surface *)layer_surface->backend_data;
}
