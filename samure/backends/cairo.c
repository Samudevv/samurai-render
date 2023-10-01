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
  c->base.unassociate_layer_surface =
      samure_backend_cairo_unassociate_layer_surface;

  SAMURE_RETURN_RESULT(backend_cairo, c);
}

void samure_destroy_backend_cairo(struct samure_context *ctx,
                                  struct samure_backend *backend) {
  struct samure_backend_cairo *c = (struct samure_backend_cairo *)backend;
  free(c);
}

void samure_backend_cairo_render_end(struct samure_output *output,
                                     struct samure_layer_surface *s,
                                     struct samure_context *ctx,
                                     struct samure_backend *backend) {
  struct samure_cairo_surface *c =
      (struct samure_cairo_surface *)s->backend_data;
  samure_layer_surface_draw_buffer(s, c->buffer);
}

samure_error samure_backend_cairo_associate_layer_surface(
    struct samure_context *ctx, struct samure_backend *backend,
    struct samure_output *output, struct samure_layer_surface *sfc) {
  struct samure_backend_cairo *c = (struct samure_backend_cairo *)backend;
  struct samure_cairo_surface *cairo_sfc =
      malloc(sizeof(struct samure_cairo_surface));
  if (!cairo_sfc) {
    return SAMURE_ERROR_MEMORY;
  }
  memset(cairo_sfc, 0, sizeof(struct samure_cairo_surface));

  SAMURE_RESULT(shared_buffer)
  b_rs = samure_create_shared_buffer(ctx->shm, SAMURE_BUFFER_FORMAT,
                                     output->geo.w, output->geo.h);
  if (SAMURE_HAS_ERROR(b_rs)) {
    free(cairo_sfc);
    return SAMURE_ERROR_SHARED_BUFFER_INIT | b_rs.error;
  }

  cairo_sfc->buffer = SAMURE_UNWRAP(shared_buffer, b_rs);

  cairo_sfc->cairo_surface = cairo_image_surface_create_for_data(
      (unsigned char *)cairo_sfc->buffer->data, CAIRO_FORMAT_ARGB32,
      cairo_sfc->buffer->width, cairo_sfc->buffer->height,
      cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32,
                                    cairo_sfc->buffer->width));
  if (cairo_surface_status(cairo_sfc->cairo_surface) != CAIRO_STATUS_SUCCESS) {
    samure_destroy_shared_buffer(cairo_sfc->buffer);
    cairo_surface_destroy(cairo_sfc->cairo_surface);
    free(cairo_sfc);
    return SAMURE_ERROR_CAIRO_SURFACE_INIT;
  }
  cairo_sfc->cairo = cairo_create(cairo_sfc->cairo_surface);
  if (cairo_status(cairo_sfc->cairo) != CAIRO_STATUS_SUCCESS) {
    samure_destroy_shared_buffer(cairo_sfc->buffer);
    cairo_surface_destroy(cairo_sfc->cairo_surface);
    cairo_destroy(cairo_sfc->cairo);
    free(cairo_sfc);
    return SAMURE_ERROR_CAIRO_INIT;
  }

  sfc->backend_data = cairo_sfc;
  samure_backend_cairo_render_end(output, sfc, ctx, &c->base);

  return SAMURE_ERROR_NONE;
}

void samure_backend_cairo_unassociate_layer_surface(
    struct samure_context *ctx, struct samure_backend *backend,
    struct samure_output *output, struct samure_layer_surface *layer_surface) {
  if (!layer_surface->backend_data) {
    return;
  }

  struct samure_cairo_surface *c =
      (struct samure_cairo_surface *)layer_surface->backend_data;

  if (c->cairo)
    cairo_destroy(c->cairo);
  if (c->cairo_surface)
    cairo_surface_destroy(c->cairo_surface);
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
