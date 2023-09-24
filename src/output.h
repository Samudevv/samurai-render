#pragma once

#include "wayland/wlr-layer-shell-unstable-v1.h"
#include "wayland/xdg-output-unstable-v1.h"
#include <wayland-client.h>

#define OUT_IDX2(output)                                                       \
  (((uintptr_t)output - (uintptr_t)ctx->outputs) / sizeof(struct samure_output))
#define OUT_IDX() OUT_IDX2(output)
#define OUT_X2(output, val) (val - output->pos.x)
#define OUT_X(val) OUT_X2(output, val)
#define OUT_Y2(output, val) (val - output->pos.y)
#define OUT_Y(val) OUT_Y2(output, val)

struct samure_output {
  struct wl_output *output;
  struct zxdg_output_v1 *xdg_output;
  struct wl_surface *surface;
  struct wl_callback *frame_callback;
  struct zwlr_layer_surface_v1 *layer_surface;

  struct {
    int32_t x;
    int32_t y;
  } pos;

  struct {
    int32_t w;
    int32_t h;
  } size;

  char *name;

  int surface_ready;
};

extern struct samure_output samure_create_output(struct wl_output *output);
extern void samure_destroy_output(struct samure_output output);
extern int samure_circle_in_output(struct samure_output *output,
                                   int32_t circle_x, int32_t circle_y,
                                   int32_t radius);
