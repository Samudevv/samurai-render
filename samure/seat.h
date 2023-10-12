/***********************************************************************************
 *                         This file is part of samurai-render
 *                    https://github.com/PucklaJ/samurai-render
 ***********************************************************************************
 * Copyright (c) 2023 Jonas Pucher
 *
 * This software is provided ‘as-is’, without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 ************************************************************************************/

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
