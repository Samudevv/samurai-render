#include "seat.h"
#include "callbacks.h"
#include <stdlib.h>

struct samure_seat samure_create_seat(struct wl_seat *seat) {
  struct samure_seat s = {0};
  s.seat = seat;
  s.cursor_shape = WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_DEFAULT;
  return s;
}

void samure_destroy_seat(struct samure_seat seat) {
  free(seat.name);
  if (seat.pointer) {
    wl_pointer_destroy(seat.pointer);
    wp_cursor_shape_device_v1_destroy(seat.cursor_shape_device);
  }
  if (seat.keyboard)
    wl_keyboard_destroy(seat.keyboard);
  if (seat.touch)
    wl_touch_destroy(seat.touch);
  wl_seat_destroy(seat.seat);
}

void samure_seat_set_pointer_shape(struct samure_seat *seat, uint32_t shape) {
  seat->cursor_shape = shape;
  wp_cursor_shape_device_v1_set_shape(
      seat->cursor_shape_device, seat->last_pointer_enter, seat->cursor_shape);
}