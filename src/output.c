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
