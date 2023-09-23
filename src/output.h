#pragma once

#include "wayland/wlr-layer-shell-unstable-v1.h"
#include "wayland/xdg-output-unstable-v1.h"
#include <wayland-client.h>

struct samure_output {
  struct wl_output *output;
  struct zxdg_output_v1 *xdg_output;
  struct wl_surface *surface;
  struct zwlr_layer_surface_v1 *layer_surface;

  struct {
    int32_t x;
    int32_t y;
  } logical_position;

  struct {
    int32_t width;
    int32_t height;
  } logical_size;

  char *name;
};

extern struct samure_output samure_create_output(struct wl_output *output);
extern void samure_destroy_output(struct samure_output output);
