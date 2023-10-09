#include "seat.h"
#include "callbacks.h"
#include "context.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

SAMURE_DEFINE_RESULT_UNWRAP(seat);

SAMURE_RESULT(seat)
samure_create_seat(struct samure_context *ctx, struct wl_seat *seat) {
  SAMURE_RESULT_ALLOC(seat, s);

  s->seat = seat;
  s->cursor_shape = WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_DEFAULT;

  wl_seat_add_listener(s->seat, &seat_listener,
                       samure_create_callback_data(ctx, s));
  wl_display_roundtrip(ctx->display);
  struct samure_callback_data *cbd = samure_create_callback_data(ctx, s);

  if (s->pointer) {
    wl_pointer_add_listener(s->pointer, &pointer_listener, cbd);
  }
  if (s->keyboard) {
    wl_keyboard_add_listener(s->keyboard, &keyboard_listener, cbd);
  }
  if (s->touch) {
    wl_touch_add_listener(s->touch, &touch_listener, cbd);
  }

  SAMURE_RETURN_RESULT(seat, s);
}

void samure_destroy_seat(struct samure_seat *s) {
  free(s->name);
  if (s->pointer) {
    wl_pointer_destroy(s->pointer);
    if (s->cursor_shape_device)
      wp_cursor_shape_device_v1_destroy(s->cursor_shape_device);
  }
  if (s->keyboard)
    wl_keyboard_destroy(s->keyboard);
  if (s->touch)
    wl_touch_destroy(s->touch);
  if (s->seat)
    wl_seat_destroy(s->seat);
  free(s);
}

void samure_seat_set_pointer_shape(struct samure_seat *seat, uint32_t shape) {
  if (seat->cursor_shape_device) {
    seat->cursor_shape = shape;
    wp_cursor_shape_device_v1_set_shape(seat->cursor_shape_device,
                                        seat->last_pointer_enter,
                                        seat->cursor_shape);
  }
}