#pragma once
#include <stdint.h>

#include "error_handling.h"

struct samure_context;
struct samure_output;
struct samure_layer_surface;

struct samure_backend {
  void (*on_layer_surface_configure)(struct samure_backend *backend,
                                     struct samure_context *ctx,
                                     struct samure_layer_surface *layer_surface,
                                     int32_t width, int32_t height);
  void (*render_start)(struct samure_layer_surface *layer_surface,
                       struct samure_context *ctx,
                       struct samure_backend *backend);
  void (*render_end)(struct samure_layer_surface *layer_surface,
                     struct samure_context *ctx,
                     struct samure_backend *backend);
  void (*destroy)(struct samure_context *ctx, struct samure_backend *backend);
  samure_error (*associate_layer_surface)(
      struct samure_context *ctx, struct samure_backend *backend,
      struct samure_layer_surface *layer_surface);
  void (*unassociate_layer_surface)(struct samure_context *ctx,
                                    struct samure_backend *backend,
                                    struct samure_layer_surface *layer_surface);
};
