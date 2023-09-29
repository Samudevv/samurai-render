#pragma once

#include <stdint.h>

struct samure_context;
struct samure_output;

struct samure_backend {
  void (*on_layer_surface_configure)(struct samure_backend *backend,
                                     struct samure_context *ctx,
                                     struct samure_output *output,
                                     int32_t width, int32_t height);
  void (*render_start)(struct samure_output *output, struct samure_context *ctx,
                       struct samure_backend *backend);
  void (*render_end)(struct samure_output *output, struct samure_context *ctx,
                     struct samure_backend *backend);
  void (*destroy)(struct samure_context *ctx, struct samure_backend *backend);
};
