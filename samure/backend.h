#pragma once
#include <stdint.h>

#include "error_handling.h"

struct samure_context;
struct samure_output;
struct samure_layer_surface;

typedef void (*samure_on_layer_surface_configure_t)(
    struct samure_context *ctx, struct samure_layer_surface *layer_surface,
    int32_t width, int32_t height);
typedef void (*samure_render_start_t)(
    struct samure_context *ctx, struct samure_layer_surface *layer_surface);
typedef void (*samure_render_end_t)(struct samure_context *ctx,
                                    struct samure_layer_surface *layer_surface);
typedef void (*samure_destroy_t)(struct samure_context *ctx);
typedef samure_error (*samure_associate_layer_surface_t)(
    struct samure_context *ctx, struct samure_layer_surface *layer_surface);
typedef void (*samure_unassociate_layer_surface_t)(
    struct samure_context *ctx, struct samure_layer_surface *layer_surface);

struct samure_backend {
  samure_on_layer_surface_configure_t on_layer_surface_configure;
  samure_render_start_t render_start;
  samure_render_end_t render_end;
  samure_destroy_t destroy;
  samure_associate_layer_surface_t associate_layer_surface;
  samure_unassociate_layer_surface_t unassociate_layer_surface;
};

SAMURE_DEFINE_RESULT(backend);

extern SAMURE_RESULT(backend) samure_create_backend(
    samure_on_layer_surface_configure_t on_layer_surface_configure,
    samure_render_start_t render_start, samure_render_end_t render_end,
    samure_destroy_t destroy,
    samure_associate_layer_surface_t associate_layer_surface,
    samure_unassociate_layer_surface_t unassociate_layer_surface);
