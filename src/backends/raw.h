#pragma once

#include "../backend.h"
#include "../shared_memory.h"

struct samure_context;

struct samure_raw_surface {
  struct samure_shared_buffer shared_buffer;
};

struct samure_backend_raw {
  struct samure_backend base;

  struct samure_raw_surface *surfaces;
  size_t num_outputs;

  char *error_string;
};

extern struct samure_backend_raw
samure_init_backend_raw(struct samure_context *ctx);
extern void samure_destroy_backend_raw(struct samure_backend_raw raw);
extern void samure_backend_raw_frame_end(struct samure_context *ctx,
                                         struct samure_backend_raw raw);
extern void samure_backend_raw_on_layer_surface_configure(
    void *backend, struct samure_context *ctx, struct samure_output *output,
    int32_t width, int32_t height);
