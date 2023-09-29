#pragma once

#include <wayland-client.h>

struct samure_output;

struct samure_seat {
  struct wl_seat *seat;
  struct wl_pointer *pointer;
  struct wl_keyboard *keyboard;
  struct wl_touch *touch;
  struct samure_output *pointer_focus;
  struct samure_output *keyboard_focus;

  char *name;
};

extern struct samure_seat samure_create_seat(struct wl_seat *seat);
extern void samure_destroy_seat(struct samure_seat seat);