#pragma once

#include <stdint.h>

struct samure_context;
struct samure_output;

struct samure_backend {
  void (*on_layer_surface_configure)(void *backend, struct samure_context *ctx,
                                     struct samure_output *output,
                                     int32_t width, int32_t height);
};
