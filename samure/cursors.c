/***********************************************************************************
 *                         This file is part of samurai-render
 *                    https://github.com/Samudevv/samurai-render
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

#include "cursors.h"
#include "context.h"
#include "seat.h"

static const char *samure_cursor_shape_name(uint32_t shape) {
  // clang-format off
  switch (shape) {
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_CONTEXT_MENU:  return "context-menu";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_HELP:          return "help";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_POINTER:       return "pointer";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_PROGRESS:      return "progress";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_WAIT:          return "wait";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_CELL:          return "cell";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_CROSSHAIR:     return "crosshair";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_TEXT:          return "text";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_VERTICAL_TEXT: return "vertical-text";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_ALIAS:         return "alias";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_COPY:          return "copy";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_MOVE:          return "move";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_NO_DROP:       return "no-drop";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_NOT_ALLOWED:   return "not-allowed";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_GRAB:          return "grab";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_GRABBING:      return "grabbing";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_E_RESIZE:      return "e-resize";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_N_RESIZE:      return "n-resize";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_NE_RESIZE:     return "ne-resize";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_NW_RESIZE:     return "nw-resize";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_S_RESIZE:      return "s-resize";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_SE_RESIZE:     return "se-resize";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_SW_RESIZE:     return "sw-resize";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_W_RESIZE:      return "w-resize";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_EW_RESIZE:     return "ew-resize";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_NS_RESIZE:     return "ns-resize";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_NESW_RESIZE:   return "nesw-resize";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_NWSE_RESIZE:   return "nwse-resize";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_COL_RESIZE:    return "col-resize";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_ROW_RESIZE:    return "row-resize";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_ALL_SCROLL:    return "all-scroll";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_ZOOM_IN:       return "zoom-in";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_ZOOM_OUT:      return "zoom-out";
  default:                                            return "default";
  }
  // clang-format on
}

struct samure_cursor samure_init_cursor(struct samure_seat *seat,
                                        struct wl_cursor_theme *theme,
                                        struct wl_compositor *compositor) {
  struct samure_cursor c = {0};
  c.seat = seat;
  c.current_shape = WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_DEFAULT;
  if (theme) {
    c.surface = wl_compositor_create_surface(compositor);
    c.cursor = wl_cursor_theme_get_cursor(
        theme, samure_cursor_shape_name(c.current_shape));
    if (c.cursor) {
      c.current_cursor_image = c.cursor->images[0];
      // TODO: Handle output scale
      if (c.surface) {
        wl_surface_attach(c.surface,
                          wl_cursor_image_get_buffer(c.current_cursor_image), 0,
                          0);
        if (seat->pointer) {
          wl_pointer_set_cursor(seat->pointer, seat->last_pointer_enter,
                                c.surface, c.current_cursor_image->hotspot_x,
                                c.current_cursor_image->hotspot_y);
        }
        wl_surface_commit(c.surface);
      }
    }
  }

  return c;
}

void samure_destroy_cursor(struct samure_cursor cursor) {
  if (cursor.surface) {
    wl_surface_destroy(cursor.surface);
  }
}

void samure_cursor_set_shape(struct samure_cursor_engine *engine,
                             struct samure_cursor *c,
                             struct wl_cursor_theme *theme, uint32_t shape) {
  c->current_shape = shape;

  if (engine->manager) {
    struct wp_cursor_shape_device_v1 *device =
        wp_cursor_shape_manager_v1_get_pointer(engine->manager,
                                               c->seat->pointer);
    wp_cursor_shape_device_v1_set_shape(device, c->seat->last_pointer_enter,
                                        shape);
    wp_cursor_shape_device_v1_destroy(device);
    return;
  }

  const char *name = samure_cursor_shape_name(shape);

  if (c->cursor && strcmp(c->cursor->name, name) == 0) {
    return;
  }

  c->cursor = wl_cursor_theme_get_cursor(theme, name);
  c->current_time = 0.0;
  c->current_image_index = 0;
  if (c->cursor) {
    const uint32_t old_width = c->current_cursor_image->width;
    const uint32_t old_height = c->current_cursor_image->height;
    c->current_cursor_image = c->cursor->images[0];
    if (c->surface) {
      // TODO: handle output scale
      wl_surface_attach(c->surface,
                        wl_cursor_image_get_buffer(c->current_cursor_image), 0,
                        0);
      if (c->seat->pointer) {
        wl_pointer_set_cursor(c->seat->pointer, c->seat->last_pointer_enter,
                              c->surface, c->current_cursor_image->hotspot_x,
                              c->current_cursor_image->hotspot_y);
      }
      wl_surface_damage(c->surface, 0, 0, old_width, old_height);
      wl_surface_commit(c->surface);
    }
  }
}

SAMURE_DEFINE_RESULT_UNWRAP(cursor_engine);

SAMURE_RESULT(cursor_engine)
samure_create_cursor_engine(struct samure_context *ctx,
                            struct wp_cursor_shape_manager_v1 *manager) {
  SAMURE_RESULT_ALLOC(cursor_engine, c);
  c->manager = manager;

  if (!c->manager) {
    const char *cursor_theme = getenv("XCURSOR_THEME");
    if (!cursor_theme) {
      cursor_theme = getenv("GTK_THEME");
    }

    const char *cursor_size_str = getenv("XCURSOR_SIZE");
    int cursor_size = 0;
    if (cursor_size_str) {
      cursor_size = atoi(cursor_size_str);
    }
    if (cursor_size == 0) {
      cursor_size = SAMURE_DEFAULT_CURSOR_SIZE;
    }

    // TODO: Respect output scale
    c->theme = wl_cursor_theme_load(cursor_theme, cursor_size, ctx->shm);
    if (!c->theme) {
      SAMURE_DESTROY_ERROR(cursor_engine, c, SAMURE_ERROR_CURSOR_THEME);
    }
  }

  c->num_cursors = ctx->num_seats;
  c->cursors = malloc(c->num_cursors * sizeof(struct samure_cursor));
  if (!c->cursors) {
    SAMURE_DESTROY_ERROR(cursor_engine, c, SAMURE_ERROR_MEMORY);
  }

  for (size_t i = 0; i < c->num_cursors; i++) {
    c->cursors[i] =
        samure_init_cursor(ctx->seats[i], c->theme, ctx->compositor);
  }

  SAMURE_RETURN_RESULT(cursor_engine, c);
}

void samure_destroy_cursor_engine(struct samure_cursor_engine *engine) {
  if (engine->manager) {
    wp_cursor_shape_manager_v1_destroy(engine->manager);
  }
  for (size_t i = 0; i < engine->num_cursors; i++) {
    samure_destroy_cursor(engine->cursors[i]);
  }
  if (engine->cursors) {
    free(engine->cursors);
  }
  if (engine->theme) {
    wl_cursor_theme_destroy(engine->theme);
  }
  free(engine);
}

void samure_cursor_engine_set_shape(struct samure_cursor_engine *engine,
                                    struct samure_seat *seat, uint32_t shape) {
  if (seat->pointer) {
    for (size_t i = 0; i < engine->num_cursors; i++) {
      if (engine->cursors[i].seat == seat) {
        samure_cursor_set_shape(engine, &engine->cursors[i], engine->theme,
                                shape);
      }
    }
  }
}

void samure_cursor_engine_pointer_enter(struct samure_cursor_engine *engine,
                                        struct samure_seat *seat) {
  if (!seat->pointer) {
    return;
  }

  for (size_t i = 0; i < engine->num_cursors; i++) {
    if (engine->manager) {
      struct wp_cursor_shape_device_v1 *device =
          wp_cursor_shape_manager_v1_get_pointer(engine->manager,
                                                 seat->pointer);
      wp_cursor_shape_device_v1_set_shape(device, seat->last_pointer_enter,
                                          engine->cursors[i].current_shape);
      wp_cursor_shape_device_v1_destroy(device);
    } else {
      if (engine->cursors[i].cursor) {
        struct samure_cursor *c = &engine->cursors[i];
        if (c->surface) {
          // TODO: handle output scale
          wl_surface_attach(c->surface,
                            wl_cursor_image_get_buffer(c->current_cursor_image),
                            0, 0);
          wl_pointer_set_cursor(c->seat->pointer, c->seat->last_pointer_enter,
                                c->surface, c->current_cursor_image->hotspot_x,
                                c->current_cursor_image->hotspot_y);
          wl_surface_commit(c->surface);
        }
      }
    }
  }
}

void samure_cursor_engine_update(struct samure_cursor_engine *engine,
                                 double delta_time) {
  for (size_t i = 0; i < engine->num_cursors; i++) {
    struct samure_cursor *c = &engine->cursors[i];
    if (c->cursor) {
      c->current_time += delta_time * 1000.0;
      if (c->current_time > (double)c->current_cursor_image->delay) {
        c->current_time = 0.0;
        c->current_image_index++;
        if (c->current_image_index == c->cursor->image_count) {
          c->current_image_index = 0;
        }
        const uint32_t old_width = c->current_cursor_image->width;
        const uint32_t old_height = c->current_cursor_image->height;
        c->current_cursor_image = c->cursor->images[c->current_image_index];
        if (c->surface) {
          wl_surface_attach(c->surface,
                            wl_cursor_image_get_buffer(c->current_cursor_image),
                            0, 0);
          if (c->seat->pointer) {
            // TODO: handle output scale
            wl_pointer_set_cursor(c->seat->pointer, c->seat->last_pointer_enter,
                                  c->surface,
                                  c->current_cursor_image->hotspot_x,
                                  c->current_cursor_image->hotspot_y);
          }
          wl_surface_damage(c->surface, 0, 0, old_width, old_height);
          wl_surface_commit(c->surface);
        }
      }
    }
  }
}
