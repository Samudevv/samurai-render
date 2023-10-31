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
#include "wayland/cursor-shape.h"
#include <wayland-cursor.h>

#define SAMURE_DEFAULT_CURSOR_SIZE 24

struct samure_seat;
struct samure_context;

struct samure_cursor {
  struct samure_seat *seat;
  struct wl_cursor *cursor;
  struct wl_surface *surface;
  struct wl_cursor_image *current_cursor_image;
  unsigned int current_image_index;
  double current_time;
};

extern struct samure_cursor
samure_init_cursor(struct samure_seat *seat, struct wl_cursor_theme *theme,
                   struct wl_compositor *compositor);
extern void samure_destroy_cursor(struct samure_cursor cursor);
extern void samure_cursor_set_shape(struct samure_cursor *cursor,
                                    struct wl_cursor_theme *theme,
                                    const char *name);

struct samure_cursor_engine {
  struct wp_cursor_shape_manager_v1 *manager;

  struct wl_cursor_theme *theme;
  struct samure_cursor *cursors;
  size_t num_cursors;
};

SAMURE_DEFINE_RESULT(cursor_engine);

extern SAMURE_RESULT(cursor_engine)
    samure_create_cursor_engine(struct samure_context *ctx,
                                struct wp_cursor_shape_manager_v1 *manager);

extern void samure_destroy_cursor_engine(struct samure_cursor_engine *engine);

extern void samure_cursor_engine_set_shape(struct samure_cursor_engine *engine,
                                           struct samure_seat *seat,
                                           uint32_t shape);

extern void
samure_cursor_engine_pointer_enter(struct samure_cursor_engine *engine,
                                   struct samure_seat *seat);

extern void samure_cursor_engine_update(struct samure_cursor_engine *engine,
                                        double delta_time);
