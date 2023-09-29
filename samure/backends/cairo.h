#pragma once

#include <cairo/cairo.h>

#include "../backend.h"
#include "../shared_memory.h"

struct samure_cairo_surface {
  struct samure_shared_buffer buffer;
  cairo_surface_t *cairo_surface;
  cairo_t *cairo;
};

struct samure_backend_cairo {
  struct samure_backend base;

  char *error_string;
};

extern struct samure_backend_cairo *
samure_init_backend_cairo(struct samure_context *ctx);
extern void samure_destroy_backend_cairo(struct samure_context *ctx,
                                         struct samure_backend *c);
extern void samure_backend_cairo_render_end(
    struct samure_output *output, struct samure_layer_surface *layer_surface,
    struct samure_context *ctx, struct samure_backend *c);
extern void samure_backend_cairo_associate_layer_surface(
    struct samure_context *ctx, struct samure_backend *backend,
    struct samure_output *output, struct samure_layer_surface *layer_surface);
extern void samure_backend_cairo_unassociate_layer_surface(
    struct samure_context *ctx, struct samure_backend *backend,
    struct samure_output *output, struct samure_layer_surface *layer_surface);
extern struct samure_backend_cairo *
samure_get_backend_cairo(struct samure_context *ctx);
