#include "cairo.h"
#include "../context.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CAI_ADD_ERR_F(format, ...)                                             \
  {                                                                            \
    const size_t error_string_len =                                            \
        c->error_string ? strlen(c->error_string) : 0;                         \
    c->error_string = realloc(c->error_string, error_string_len + 2048);       \
    snprintf(&c->error_string[error_string_len], 2048, "\n" format,            \
             __VA_ARGS__);                                                     \
  }

struct samure_backend_cairo *
samure_init_backend_cairo(struct samure_context *ctx) {
  struct samure_backend_cairo *c = malloc(sizeof(struct samure_backend_cairo));
  memset(c, 0, sizeof(struct samure_backend_cairo));

  c->base.destroy = samure_destroy_backend_cairo;
  c->base.render_end = samure_backend_cairo_render_end;
  c->base.associate_layer_surface =
      samure_backend_cairo_associate_layer_surface;
  c->base.unassociate_layer_surface =
      samure_backend_cairo_unassociate_layer_surface;

  return c;
}

void samure_destroy_backend_cairo(struct samure_context *ctx,
                                  struct samure_backend *backend) {
  struct samure_backend_cairo *c = (struct samure_backend_cairo *)backend;
  free(c->error_string);
  free(c);
}

void samure_backend_cairo_render_end(struct samure_output *output,
                                     struct samure_layer_surface *s,
                                     struct samure_context *ctx,
                                     struct samure_backend *backend) {
  struct samure_cairo_surface *c =
      (struct samure_cairo_surface *)s->backend_data;
  wl_surface_attach(s->surface, c->buffer.buffer, 0, 0);
  wl_surface_damage(s->surface, 0, 0, output->geo.w, output->geo.h);
  wl_surface_commit(s->surface);
}

void samure_backend_cairo_associate_layer_surface(
    struct samure_context *ctx, struct samure_backend *backend,
    struct samure_output *output, struct samure_layer_surface *sfc) {
  struct samure_backend_cairo *c = (struct samure_backend_cairo *)backend;
  struct samure_cairo_surface *cairo_sfc =
      malloc(sizeof(struct samure_cairo_surface));

  cairo_sfc->buffer =
      samure_create_shared_buffer(ctx->shm, output->geo.w, output->geo.h);
  if (cairo_sfc->buffer.buffer == NULL) {
    CAI_ADD_ERR_F("failed to create shared memory buffer for layer surface for "
                  "output: %s",
                  output->name);
  } else {
    cairo_sfc->cairo_surface = cairo_image_surface_create_for_data(
        (unsigned char *)cairo_sfc->buffer.data, CAIRO_FORMAT_ARGB32,
        cairo_sfc->buffer.width, cairo_sfc->buffer.height,
        cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32,
                                      cairo_sfc->buffer.width));
    cairo_sfc->cairo = cairo_create(cairo_sfc->cairo_surface);

    sfc->backend_data = cairo_sfc;
    samure_backend_cairo_render_end(output, sfc, ctx, &c->base);
  }
}

void samure_backend_cairo_unassociate_layer_surface(
    struct samure_context *ctx, struct samure_backend *backend,
    struct samure_output *output, struct samure_layer_surface *layer_surface) {
  struct samure_cairo_surface *c =
      (struct samure_cairo_surface *)layer_surface->backend_data;

  cairo_destroy(c->cairo);
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
