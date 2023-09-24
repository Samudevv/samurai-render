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
  const int32_t ox = o->pos.x + o->size.w / 2;
  const int32_t oy = o->pos.y + o->size.h / 2;

  // Distance between the middle of the circle and the middle of the output
  const int32_t dx = abs(x - ox);
  const int32_t dy = abs(y - oy);

  if ((dx > (o->size.w / 2 + r)) || (dy > (o->size.h / 2 + r)))
    return 0;

  if ((dx <= (o->size.w / 2)) || (dy <= (o->size.h / 2)))
    return 1;

  return (dx - o->size.w / 2) * (dx - o->size.w / 2) +
             (dy - o->size.h / 2) * (dy - o->size.h / 2) <=
         (r * r);
}