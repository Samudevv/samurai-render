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

#include "cursors.h"
#include "context.h"
#include "seat.h"

struct samure_cursor samure_init_cursor(struct samure_seat *seat,
                                        struct wl_cursor_theme *theme,
                                        struct wl_compositor *compositor) {
  struct samure_cursor c = {0};
  c.seat = seat;
  c.surface = wl_compositor_create_surface(compositor);
  c.cursor = wl_cursor_theme_get_cursor(theme, "default");
  if (c.cursor) {
    c.current_cursor_image = c.cursor->images[0];
    // TODO: Handle output scale
    if (c.surface) {
      wl_surface_attach(
          c.surface, wl_cursor_image_get_buffer(c.current_cursor_image), 0, 0);
      if (seat->pointer) {
        wl_pointer_set_cursor(seat->pointer, seat->last_pointer_enter,
                              c.surface, c.current_cursor_image->hotspot_x,
                              c.current_cursor_image->hotspot_y);
      }
      wl_surface_commit(c.surface);
    }
  }

  return c;
}

void samure_destroy_cursor(struct samure_cursor cursor) {
  if (cursor.surface) {
    wl_surface_destroy(cursor.surface);
  }
}

void samure_cursor_set_shape(struct samure_cursor *c,
                             struct wl_cursor_theme *theme, const char *name) {
  c->cursor = wl_cursor_theme_get_cursor(theme, name);
  if (c->cursor) {
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
      wl_surface_damage(c->surface, 0, 0, c->current_cursor_image->width,
                        c->current_cursor_image->height);
      wl_surface_commit(c->surface);
    }
  }
}

SAMURE_DEFINE_RESULT_UNWRAP(cursor_engine);

SAMURE_RESULT(cursor_engine)
samure_create_cursor_engine(struct samure_context *ctx,
                            struct wp_cursor_shape_manager_v1 *manager) {
  SAMURE_RESULT_ALLOC(cursor_engine, c);

  if (manager) {
    c->manager = manager;
  } else {
    const char *cursor_theme = getenv("XCURSOR_THEME");
    if (!cursor_theme) {
      cursor_theme = getenv("GTK_THEME");
    }

    const char *cursor_size_str = getenv("XCURSOR_SIZE");
    int cursor_size = SAMURE_DEFAULT_CURSOR_SIZE;
    if (cursor_size_str) {
      // TODO: Error checking
      cursor_size = atoi(cursor_size_str);
    }

    // TODO: Respect output scale
    c->theme = wl_cursor_theme_load(cursor_theme, cursor_size, ctx->shm);
    if (!c->theme) {
      SAMURE_DESTROY_ERROR(cursor_engine, c, SAMURE_ERROR_CURSOR_THEME);
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

static const char *samure_cursor_shape_name(uint32_t shape) {
  // clang-format off
  switch (shape) {
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_CONTEXT_MENU:  return "context_menu";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_HELP:          return "help";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_POINTER:       return "pointer";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_PROGRESS:      return "progress";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_WAIT:          return "wait";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_CELL:          return "cell";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_CROSSHAIR:     return "crosshair";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_TEXT:          return "text";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_VERTICAL_TEXT: return "vertical_text";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_ALIAS:         return "alias";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_COPY:          return "copy";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_MOVE:          return "move";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_NO_DROP:       return "no_drop";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_NOT_ALLOWED:   return "not_allowed";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_GRAB:          return "grab";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_GRABBING:      return "grabbing";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_E_RESIZE:      return "e_resize";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_N_RESIZE:      return "n_resize";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_NE_RESIZE:     return "ne_resize";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_NW_RESIZE:     return "nw_resize";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_S_RESIZE:      return "s_resize";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_SE_RESIZE:     return "se_resize";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_SW_RESIZE:     return "sw_resize";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_W_RESIZE:      return "w_resize";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_EW_RESIZE:     return "ew_resize";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_NS_RESIZE:     return "ns_resize";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_NESW_RESIZE:   return "nesw_resize";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_NWSE_RESIZE:   return "nwse_resize";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_COL_RESIZE:    return "col_resize";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_ROW_RESIZE:    return "row_resize";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_ALL_SCROLL:    return "all_scroll";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_ZOOM_IN:       return "zoom_in";
	case WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_ZOOM_OUT:      return "zoom_out";
  default:                                            return "default";
  }
}

void samure_cursor_engine_set_shape(struct samure_cursor_engine *engine,
                                    struct samure_seat *seat, uint32_t shape) {
  if (seat->pointer) {
    if (engine->manager) {
      struct wp_cursor_shape_device_v1 *device =
          wp_cursor_shape_manager_v1_get_pointer(engine->manager,
                                                 seat->pointer);
      wp_cursor_shape_device_v1_set_shape(device, seat->last_pointer_enter,
                                          shape);
      wp_cursor_shape_device_v1_destroy(device);
    } else {
      for (size_t i = 0; i < engine->num_cursors; i++) {
        if (engine->cursors[i].seat == seat) {
          samure_cursor_set_shape(&engine->cursors[i], engine->theme,
                                  samure_cursor_shape_name(shape));
        }
      }
    }
  }
}

void samure_cursor_engine_pointer_enter(struct samure_cursor_engine *engine,
                                        struct samure_seat *seat) {
  if (!engine->manager) {
    for (size_t i = 0; i < engine->num_cursors; i++) {
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
