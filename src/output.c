#include "output.h"
#include <stdlib.h>

struct samure_output samure_create_output(struct wl_output *output) {
  struct samure_output o = {0};
  o.output = output;
  return o;
}

void samure_destroy_output(struct samure_output output) {
  free(output.name);
  if (output.frame_callback) {
    wl_callback_destroy(output.frame_callback);
  }
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
