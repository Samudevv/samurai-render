#pragma once

#include <wayland-client.h>

struct samure_seat {
  struct wl_seat *seat;
  struct wl_pointer *pointer;
  struct wl_keyboard *keyboard;
  struct wl_touch *touch;

  char *name;
};

extern struct samure_seat samure_create_seat(struct wl_seat *seat);
extern void samure_destroy_seat(struct samure_seat seat);