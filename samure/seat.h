#pragma once

#include <wayland-client.h>

struct samure_output;
struct samure_layer_surface;

struct samure_focus {
  struct samure_output *output;
  struct samure_layer_surface *surface;
};

struct samure_seat {
  struct wl_seat *seat;
  struct wl_pointer *pointer;
  struct wl_keyboard *keyboard;
  struct wl_touch *touch;
  struct samure_focus pointer_focus;
  struct samure_focus keyboard_focus;

  char *name;
};

extern struct samure_seat samure_create_seat(struct wl_seat *seat);
extern void samure_destroy_seat(struct samure_seat seat);