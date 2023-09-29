#include "seat.h"
#include "callbacks.h"
#include <stdlib.h>

struct samure_seat samure_create_seat(struct wl_seat *seat) {
  struct samure_seat s = {0};
  s.seat = seat;
  return s;
}

void samure_destroy_seat(struct samure_seat seat) {
  free(seat.name);
  if (seat.pointer)
    wl_pointer_destroy(seat.pointer);
  if (seat.keyboard)
    wl_keyboard_destroy(seat.keyboard);
  if (seat.touch)
    wl_touch_destroy(seat.touch);
  wl_seat_destroy(seat.seat);
}
