#pragma once

#include "wayland/wlr-layer-shell-unstable-v1.h"
#include <wayland-client.h>

#include "error_handling.h"
#include "shared_memory.h"

#define SAMURE_LAYER_SURFACE_ANCHOR_FILL                                       \
  (ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP | ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT |      \
   ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT | ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM)
#define SAMURE_LAYER_OVERLAY ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY

struct samure_context;
struct samure_output;

struct samure_layer_surface {
  struct wl_surface *surface;
  struct zwlr_layer_surface_v1 *layer_surface;
  void *backend_data;
  uint32_t w;
  uint32_t h;

  struct samure_callback_data *callback_data;
};

SAMURE_DEFINE_RESULT(layer_surface);

extern SAMURE_RESULT(layer_surface)
    samure_create_layer_surface(struct samure_context *ctx,
                                struct samure_output *output, uint32_t layer,
                                uint32_t anchor, int keyboard_interaction,
                                int pointer_interaction,
                                int backend_association);

extern void samure_destroy_layer_surface(struct samure_context *ctx,
                                         struct samure_layer_surface *sfc);

extern void samure_layer_surface_draw_buffer(struct samure_layer_surface *sfc,
                                             struct samure_shared_buffer *buf);
