#pragma once

#include "wayland/wlr-layer-shell-unstable-v1.h"
#include <wayland-client.h>

struct samure_context;
struct samure_output;
struct samure_layer_surface_callback_data;

struct samure_layer_surface {
  struct wl_surface *surface;
  struct zwlr_layer_surface_v1 *layer_surface;
  void *backend_data;

  char *error_string;
  struct samure_layer_surface_callback_data *callback_data;
};

extern struct samure_layer_surface *
samure_create_layer_surface(struct samure_context *ctx,
                            struct samure_output *output, uint32_t layer,
                            uint32_t anchor, uint32_t keyboard_interaction,
                            int pointer_interaction, int backend_association);

extern void samure_destroy_layer_surface(struct samure_context *ctx,
                                         struct samure_output *output,
                                         struct samure_layer_surface *sfc);

struct samure_layer_surface_callback_data {
  struct samure_context *ctx;
  struct samure_output *output;
  struct samure_layer_surface *surface;
};

extern struct samure_layer_surface_callback_data *
samure_create_layer_surface_callback_data(struct samure_context *ctx,
                                          struct samure_output *output,
                                          struct samure_layer_surface *surface);
