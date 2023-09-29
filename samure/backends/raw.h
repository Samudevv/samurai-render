#pragma once

#include "../backend.h"
#include "../shared_memory.h"

struct samure_context;
struct samure_layer_surface;

struct samure_raw_surface {
  struct samure_shared_buffer shared_buffer;
};

struct samure_backend_raw {
  struct samure_backend base;

  char *error_string;
};

extern struct samure_backend_raw *
samure_init_backend_raw(struct samure_context *ctx);
extern void samure_destroy_backend_raw(struct samure_context *ctx,
                                       struct samure_backend *raw);
extern void samure_backend_raw_render_end(
    struct samure_output *output, struct samure_layer_surface *layer_surface,
    struct samure_context *ctx, struct samure_backend *raw);
extern void samure_backend_raw_associate_layer_surface(
    struct samure_context *ctx, struct samure_backend *raw,
    struct samure_output *output, struct samure_layer_surface *sfc);
extern void samure_backend_raw_unassociate_layer_surface(
    struct samure_context *ctx, struct samure_backend *raw,
    struct samure_output *output, struct samure_layer_surface *sfc);
extern struct samure_backend_raw *
samure_get_backend_raw(struct samure_context *ctx);
extern struct samure_raw_surface *
samure_get_raw_surface(struct samure_layer_surface *layer_surface);
