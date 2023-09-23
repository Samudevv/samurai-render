#pragma once

#include "wayland/wlr-layer-shell-unstable-v1.h"
#include "wayland/xdg-output-unstable-v1.h"
#include <wayland-client.h>

#include "output.h"
#include "seat.h"

#define SAMURE_NO_CONTEXT_CONFIG NULL

struct samure_context_config {
  // TODO: Fill out different options for creating the context
};

extern struct samure_context_config samure_default_context_config();

struct samure_context {
  struct wl_display *display;
  struct wl_shm *shm;
  struct wl_compositor *compositor;
  struct zwlr_layer_shell_v1 *layer_shell;
  struct zxdg_output_manager_v1 *output_manager;

  struct samure_seat *seats;
  size_t num_seats;

  struct samure_output *outputs;
  size_t num_outputs;

  char *error_string;
};

extern struct samure_context *
samure_create_context(struct samure_context_config *config);
extern void samure_destroy_context(struct samure_context *ctx);
