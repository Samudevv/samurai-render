#pragma once

#include <cairo/cairo.h>

#include "../backend.h"
#include "../error_handling.h"
#include "../shared_memory.h"

struct samure_cairo_surface {
  struct samure_shared_buffer *buffer;
  cairo_surface_t *cairo_surface;
  cairo_t *cairo;
};

struct samure_backend_cairo {
  struct samure_backend base;
};

SAMURE_DEFINE_RESULT(backend_cairo);

extern SAMURE_RESULT(backend_cairo)
    samure_init_backend_cairo(struct samure_context *ctx);
extern void samure_destroy_backend_cairo(struct samure_context *ctx);
extern void
samure_backend_cairo_render_end(struct samure_context *ctx,
                                struct samure_layer_surface *layer_surface);
extern samure_error samure_backend_cairo_associate_layer_surface(
    struct samure_context *ctx, struct samure_layer_surface *layer_surface);
extern void samure_backend_cairo_on_layer_surface_configure(
    struct samure_context *ctx, struct samure_layer_surface *layer_surface,
    int32_t width, int32_t height);
extern void samure_backend_cairo_unassociate_layer_surface(
    struct samure_context *ctx, struct samure_layer_surface *layer_surface);
extern struct samure_backend_cairo *
samure_get_backend_cairo(struct samure_context *ctx);
extern struct samure_cairo_surface *
samure_get_cairo_surface(struct samure_layer_surface *layer_surface);

extern samure_error
_samure_cairo_surface_create_cairo(struct samure_cairo_surface *c);
