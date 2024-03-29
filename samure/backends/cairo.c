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

#include "cairo.h"
#include "../context.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void destroy(struct samure_context *ctx) {
  free(ctx->backend);
  ctx->backend = NULL;
}

void render_end(struct samure_context *ctx, struct samure_layer_surface *s) {
  struct samure_cairo_surface *c =
      (struct samure_cairo_surface *)s->backend_data;
  samure_layer_surface_draw_buffer(s, c->buffer);
}

samure_error associate_layer_surface(struct samure_context *ctx,
                                     struct samure_layer_surface *sfc) {
  struct samure_cairo_surface *c = malloc(sizeof(struct samure_cairo_surface));
  if (!c) {
    return SAMURE_ERROR_MEMORY;
  }
  memset(c, 0, sizeof(struct samure_cairo_surface));

  SAMURE_RESULT(shared_buffer)
  b_rs = samure_create_shared_buffer_for_layer_surface(ctx, sfc, c->buffer);
  if (SAMURE_HAS_ERROR(b_rs)) {
    free(c);
    return SAMURE_ERROR_SHARED_BUFFER_INIT | b_rs.error;
  }

  c->buffer = SAMURE_UNWRAP(shared_buffer, b_rs);

  if (c->buffer->width != 0 && c->buffer->height != 0) {
    const samure_error err = _samure_cairo_surface_create_cairo(c);
    if (SAMURE_IS_ERROR(err)) {
      samure_destroy_shared_buffer(c->buffer);
      free(c);
      return err;
    }
  }

  sfc->backend_data = c;
  render_end(ctx, sfc);

  return SAMURE_ERROR_NONE;
}

void on_layer_surface_configure(struct samure_context *ctx,
                                struct samure_layer_surface *layer_surface,
                                int32_t width, int32_t height) {
  if (!layer_surface->backend_data) {
    return;
  }

  struct samure_cairo_surface *c =
      (struct samure_cairo_surface *)layer_surface->backend_data;

  SAMURE_RESULT(shared_buffer)
  b_rs = samure_create_shared_buffer_for_layer_surface(ctx, layer_surface,
                                                       c->buffer);
  if (SAMURE_HAS_ERROR(b_rs)) {
    c->buffer = NULL;
  } else {
    struct samure_shared_buffer *b = SAMURE_UNWRAP(shared_buffer, b_rs);
    if (c->buffer != b) {
      if (c->cairo) {
        cairo_destroy(c->cairo);
        c->cairo = NULL;
      }
      if (c->cairo_surface) {
        cairo_surface_destroy(c->cairo_surface);
        c->cairo_surface = NULL;
      }
      c->buffer = b;

      const samure_error err = _samure_cairo_surface_create_cairo(c);
      if (SAMURE_IS_ERROR(err)) {
        samure_destroy_shared_buffer(c->buffer);
        c->buffer = NULL;
      }
    }
  }
}

void unassociate_layer_surface(struct samure_context *ctx,
                               struct samure_layer_surface *layer_surface) {
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
