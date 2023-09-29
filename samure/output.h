#pragma once

#include "wayland/wlr-layer-shell-unstable-v1.h"
#include "wayland/xdg-output-unstable-v1.h"
#include <wayland-client.h>

#define OUT_IDX2(output)                                                       \
  (((uintptr_t)output - (uintptr_t)ctx->outputs) / sizeof(struct samure_output))
#define OUT_IDX() OUT_IDX2(output)
#define OUT_X2(output, val) (val - output->geo.x)
#define OUT_X(val) OUT_X2(output, val)
#define OUT_Y2(output, val) (val - output->geo.y)
#define OUT_Y(val) OUT_Y2(output, val)

struct samure_context;

struct samure_rect {
  int32_t x;
  int32_t y;
  int32_t w;
  int32_t h;
};

struct samure_output {
  struct wl_output *output;
  struct zxdg_output_v1 *xdg_output;
  struct wl_surface *surface;
  struct zwlr_layer_surface_v1 *layer_surface;

  struct samure_rect geo;

  char *name;
};

extern struct samure_output samure_create_output(struct wl_output *output);
extern void samure_destroy_output(struct samure_output output);
extern int samure_circle_in_output(struct samure_output *output,
                                   int32_t circle_x, int32_t circle_y,
                                   int32_t radius);
extern int samure_rect_in_output(struct samure_output *output, int32_t rect_x,
                                 int32_t rect_y, int32_t rect_w,
                                 int32_t rect_h);
extern int samure_square_in_output(struct samure_output *output,
                                   int32_t square_x, int32_t square_y,
                                   int32_t square_size);
extern int samure_point_in_output(struct samure_output *output, int32_t point_x,
                                  int32_t point_y);
extern int samure_triangle_in_output(struct samure_output *output,
                                     int32_t tri_x1, int32_t tri_y1,
                                     int32_t tri_x2, int32_t tri_y2,
                                     int32_t tri_x3, int32_t tri_y3);

extern void samure_output_set_pointer_interaction(struct samure_context *ctx,
                                                  struct samure_output *output,
                                                  int enable);

extern void samure_output_set_input_regions(struct samure_context *ctx,
                                            struct samure_output *output,
                                            struct samure_rect *rects,
                                            size_t num_rects);

extern void samure_output_set_keyboard_interaction(struct samure_output *output,
                                                   int enable);