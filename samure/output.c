#include "output.h"
#include "context.h"
#include <stdlib.h>

struct samure_output samure_create_output(struct wl_output *output) {
  struct samure_output o = {0};
  o.output = output;
  return o;
}

void samure_destroy_output(struct samure_output output) {
  free(output.name);
  zwlr_layer_surface_v1_destroy(output.layer_surface);
  wl_surface_destroy(output.surface);
  zxdg_output_v1_destroy(output.xdg_output);
  wl_output_destroy(output.output);
}

int samure_circle_in_output(struct samure_output *o, int32_t x, int32_t y,
                            int32_t r) {
  // Middle of the output
  const int32_t ox = o->geo.x + o->geo.w / 2;
  const int32_t oy = o->geo.y + o->geo.h / 2;

  // Distance between the middle of the circle and the middle of the output
  const int32_t dx = abs(x - ox);
  const int32_t dy = abs(y - oy);

  if ((dx > (o->geo.w / 2 + r)) || (dy > (o->geo.h / 2 + r)))
    return 0;

  if ((dx <= (o->geo.w / 2)) || (dy <= (o->geo.h / 2)))
    return 1;

  return (dx - o->geo.w / 2) * (dx - o->geo.w / 2) +
             (dy - o->geo.h / 2) * (dy - o->geo.h / 2) <=
         (r * r);
}

int samure_rect_in_output(struct samure_output *o, int32_t x, int32_t y,
                          int32_t w, int32_t h) {
  return (x < (o->geo.x + o->geo.w)) && ((x + w) > o->geo.x) &&
         (y < (o->geo.y + o->geo.h)) && ((y + h) > o->geo.y);
}

int samure_square_in_output(struct samure_output *output, int32_t square_x,
                            int32_t square_y, int32_t square_size) {
  return samure_rect_in_output(output, square_x, square_y, square_size,
                               square_size);
}

int samure_point_in_output(struct samure_output *o, int32_t x, int32_t y) {
  return (x > o->geo.x) && (x < (o->geo.x + o->geo.w)) && (y > o->geo.y) &&
         (y < o->geo.y + o->geo.h);
}

int samure_triangle_in_output(struct samure_output *o, int32_t x1, int32_t y1,
                              int32_t x2, int32_t y2, int32_t x3, int32_t y3) {
  return samure_point_in_output(o, x1, y1) &&
         samure_point_in_output(o, x2, y2) && samure_point_in_output(o, x3, y3);
}

void samure_output_set_pointer_interaction(struct samure_context *ctx,
                                           struct samure_output *output,
                                           int enable) {
  if (enable) {
    wl_surface_set_input_region(output->surface, NULL);
  } else {
    struct wl_region *reg = wl_compositor_create_region(ctx->compositor);
    if (!reg) {
      return;
    }

    wl_surface_set_input_region(output->surface, reg);
    wl_region_destroy(reg);
  }
  wl_surface_commit(output->surface);
}

void samure_output_set_input_regions(struct samure_context *ctx,
                                     struct samure_output *output,
                                     struct samure_rect *r, size_t num_rects) {
  struct wl_region *reg = wl_compositor_create_region(ctx->compositor);
  if (!reg) {
    return;
  }

  for (size_t i = 0; i < num_rects; i++) {
    wl_region_add(reg, r[i].x, r[i].y, r[i].w, r[i].h);
  }

  wl_surface_set_input_region(output->surface, reg);
  wl_region_destroy(reg);
  wl_surface_commit(output->surface);
}

void samure_output_set_keyboard_interaction(struct samure_output *output,
                                            int enable) {
  zwlr_layer_surface_v1_set_keyboard_interactivity(output->layer_surface,
                                                   (uint32_t)enable);
  wl_surface_commit(output->surface);
}
