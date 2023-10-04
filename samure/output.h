#pragma once

#include "error_handling.h"
#include "layer_surface.h"
#include "shared_memory.h"
#include "wayland/wlr-layer-shell-unstable-v1.h"
#include "wayland/xdg-output-unstable-v1.h"
#include <wayland-client.h>

#define OUT_X2(geo, val) (val - geo.x)
#define OUT_X(val) OUT_X2(output_geo, val)
#define OUT_Y2(geo, val) (val - geo.y)
#define OUT_Y(val) OUT_Y2(output_geo, val)

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
  struct samure_layer_surface **sfc;
  size_t num_sfc;
  struct samure_rect geo;
  char *name;
};

enum samure_screenshot_state {
  SAMURE_SCREENSHOT_PENDING,
  SAMURE_SCREENSHOT_READY,
  SAMURE_SCREENSHOT_FAILED,
  SAMURE_SCREENSHOT_DONE,
};

struct samure_screenshot_data {
  struct samure_context *ctx;
  struct samure_output *output;
  SAMURE_RESULT(shared_buffer) buffer_rs;
  enum samure_screenshot_state state;
};

SAMURE_DEFINE_RESULT(output);

extern SAMURE_RESULT(output)
    samure_create_output(struct samure_context *ctx, struct wl_output *output);
extern void samure_destroy_output(struct samure_context *ctx,
                                  struct samure_output *output);
extern int samure_circle_in_output(struct samure_rect output_geo,
                                   int32_t circle_x, int32_t circle_y,
                                   int32_t radius);
extern int samure_rect_in_output(struct samure_rect output_geo, int32_t rect_x,
                                 int32_t rect_y, int32_t rect_w,
                                 int32_t rect_h);
extern int samure_square_in_output(struct samure_rect output_geo,
                                   int32_t square_x, int32_t square_y,
                                   int32_t square_size);
extern int samure_point_in_output(struct samure_rect output_geo,
                                  int32_t point_x, int32_t point_y);
extern int samure_triangle_in_output(struct samure_rect output_geo,
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

extern void
samure_output_attach_layer_surface(struct samure_output *output,
                                   struct samure_layer_surface *layer_surface);

extern SAMURE_RESULT(shared_buffer)
    samure_output_screenshot(struct samure_context *ctx,
                             struct samure_output *output);
