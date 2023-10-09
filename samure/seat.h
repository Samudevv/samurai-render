#pragma once

#include "error_handling.h"
#include "wayland/cursor-shape-v1-client-protocol.h"
#include <wayland-client.h>

struct samure_context;
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
  struct wp_cursor_shape_device_v1 *cursor_shape_device;
  struct samure_focus pointer_focus;
  struct samure_focus keyboard_focus;
  struct samure_focus touch_focus;
  char *name;
  uint32_t cursor_shape;
  uint32_t last_pointer_enter;
};

SAMURE_DEFINE_RESULT(seat);

extern SAMURE_RESULT(seat)
    samure_create_seat(struct samure_context *ctx, struct wl_seat *seat);
extern void samure_destroy_seat(struct samure_seat *seat);
extern void samure_seat_set_pointer_shape(struct samure_seat *seat,
                                          uint32_t shape);
