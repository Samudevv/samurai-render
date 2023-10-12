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

#include "cairo.h"
#include "../context.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

SAMURE_DEFINE_RESULT_UNWRAP(backend_cairo);

SAMURE_RESULT(backend_cairo)
samure_init_backend_cairo(struct samure_context *ctx) {
  SAMURE_RESULT_ALLOC(backend_cairo, c);

  c->base.destroy = samure_destroy_backend_cairo;
  c->base.render_end = samure_backend_cairo_render_end;
  c->base.associate_layer_surface =
      samure_backend_cairo_associate_layer_surface;
  c->base.on_layer_surface_configure =
      samure_backend_cairo_on_layer_surface_configure;
  c->base.unassociate_layer_surface =
      samure_backend_cairo_unassociate_layer_surface;

  SAMURE_RETURN_RESULT(backend_cairo, c);
}

void samure_destroy_backend_cairo(struct samure_context *ctx) {
  free(ctx->backend);
  ctx->backend = NULL;
}

void samure_backend_cairo_render_end(struct samure_context *ctx,
                                     struct samure_layer_surface *s) {
  struct samure_cairo_surface *c =
      (struct samure_cairo_surface *)s->backend_data;
  samure_layer_surface_draw_buffer(s, c->buffer);
}

samure_error
samure_backend_cairo_associate_layer_surface(struct samure_context *ctx,
                                             struct samure_layer_surface *sfc) {
  struct samure_cairo_surface *c = malloc(sizeof(struct samure_cairo_surface));
  if (!c) {
    return SAMURE_ERROR_MEMORY;
  }
  memset(c, 0, sizeof(struct samure_cairo_surface));

  SAMURE_RESULT(shared_buffer)
  b_rs = samure_create_shared_buffer(ctx->shm, SAMURE_BUFFER_FORMAT,
                                     sfc->w == 0 ? 1 : sfc->w,
                                     sfc->h == 0 ? 1 : sfc->h);
  if (SAMURE_HAS_ERROR(b_rs)) {
    free(c);
    return SAMURE_ERROR_SHARED_BUFFER_INIT | b_rs.error;
  }

  c->buffer = SAMURE_UNWRAP(shared_buffer, b_rs);

  if (sfc->w != 0 && sfc->h != 0) {
    const samure_error err = _samure_cairo_surface_create_cairo(c);
    if (SAMURE_IS_ERROR(err)) {
      samure_destroy_shared_buffer(c->buffer);
      free(c);
      return err;
    }
  }

  sfc->backend_data = c;
  samure_backend_cairo_render_end(ctx, sfc);

  return SAMURE_ERROR_NONE;
}

void samure_backend_cairo_on_layer_surface_configure(
    struct samure_context *ctx, struct samure_layer_surface *layer_surface,
    int32_t width, int32_t height) {
  if (!layer_surface->backend_data) {
    return;
  }

  struct samure_cairo_surface *c =
      (struct samure_cairo_surface *)layer_surface->backend_data;

  if (c->buffer->width == width && c->buffer->height == height) {
    return;
  }

  if (c->buffer) {
    samure_destroy_shared_buffer(c->buffer);
  }
  if (c->cairo) {
    cairo_destroy(c->cairo);
    c->cairo = NULL;
  }
  if (c->cairo_surface) {
    cairo_surface_destroy(c->cairo_surface);
    c->cairo = NULL;
  }

  SAMURE_RESULT(shared_buffer)
  b_rs = samure_create_shared_buffer(ctx->shm, SAMURE_BUFFER_FORMAT, width,
                                     height);
  if (SAMURE_HAS_ERROR(b_rs)) {
    c->buffer = NULL;
  } else {
    c->buffer = SAMURE_UNWRAP(shared_buffer, b_rs);

    const samure_error err = _samure_cairo_surface_create_cairo(c);
    if (SAMURE_IS_ERROR(err)) {
      samure_destroy_shared_buffer(c->buffer);
      c->buffer = NULL;
    }
  }
}

void samure_backend_cairo_unassociate_layer_surface(
    struct samure_context *ctx, struct samure_layer_surface *layer_surface) {
  if (!layer_surface->backend_data) {
    return;
  }

  struct samure_cairo_surface *c =
      (struct samure_cairo_surface *)layer_surface->backend_data;

  if (c->cairo)
    cairo_destroy(c->cairo);
  if (c->cairo_surface)
    cairo_surface_destroy(c->cairo_surface);
  if (c->buffer)
    samure_destroy_shared_buffer(c->buffer);
  free(c);
  layer_surface->backend_data = NULL;
}

struct samure_backend_cairo *
samure_get_backend_cairo(struct samure_context *ctx) {
  return (struct samure_backend_cairo *)ctx->backend;
}

struct samure_cairo_surface *
samure_get_cairo_surface(struct samure_layer_surface *layer_surface) {
  return (struct samure_cairo_surface *)layer_surface->backend_data;
}

samure_error
_samure_cairo_surface_create_cairo(struct samure_cairo_surface *c) {
  c->cairo_surface = cairo_image_surface_create_for_data(
      (unsigned char *)c->buffer->data, CAIRO_FORMAT_ARGB32, c->buffer->width,
      c->buffer->height,
      cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, c->buffer->width));
  if (cairo_surface_status(c->cairo_surface) != CAIRO_STATUS_SUCCESS) {
    cairo_surface_destroy(c->cairo_surface);
    return SAMURE_ERROR_CAIRO_SURFACE_INIT;
  }
  c->cairo = cairo_create(c->cairo_surface);
  if (cairo_status(c->cairo) != CAIRO_STATUS_SUCCESS) {
    cairo_surface_destroy(c->cairo_surface);
    cairo_destroy(c->cairo);
    return SAMURE_ERROR_CAIRO_INIT;
  }

  return SAMURE_ERROR_NONE;
}