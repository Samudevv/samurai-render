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

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "callbacks.h"
#include "context.h"

#define NEW_EVENT()                                                            \
  ctx->num_events++;                                                           \
  if (ctx->num_events > ctx->cap_events) {                                     \
    ctx->cap_events = ctx->num_events;                                         \
    ctx->events =                                                              \
        realloc(ctx->events, ctx->cap_events * sizeof(struct samure_event));   \
    if (!ctx->events) {                                                        \
      ctx->cap_events = 0;                                                     \
      ctx->num_events = 0;                                                     \
      return;                                                                  \
    }                                                                          \
  }

#define LAST_EVENT ctx->events[ctx->num_events - 1]

#define OUTPUT_FOR_SURFACE()                                                   \
  struct samure_output *output = NULL;                                         \
  struct samure_layer_surface *layer_surface = NULL;                           \
  for (size_t i = 0; i < ctx->num_outputs; i++) {                              \
    for (size_t j = 0; j < ctx->outputs[i]->num_sfc; j++) {                    \
      if (ctx->outputs[i]->sfc[j]->surface == surface) {                       \
        output = ctx->outputs[i];                                              \
        layer_surface = ctx->outputs[i]->sfc[j];                               \
        break;                                                                 \
      }                                                                        \
    }                                                                          \
  }

void registry_global(void *data, struct wl_registry *registry, uint32_t name,
                     const char *interface, uint32_t version) {
  struct samure_callback_data *d = (struct samure_callback_data *)data;
  struct samure_context *ctx = d->ctx;
  struct samure_registry_data *reg_d = (struct samure_registry_data *)d->data;

  if (strcmp(interface, wl_shm_interface.name) == 0) {
    ctx->shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
  } else if (strcmp(interface, wl_seat_interface.name) == 0) {
    struct wl_seat *seat =
        wl_registry_bind(registry, name, &wl_seat_interface, 1);

    reg_d->num_seats++;
    reg_d->seats =
        realloc(reg_d->seats, reg_d->num_seats * sizeof(struct wl_seat *));
    if (!reg_d->seats) {
      reg_d->num_seats = 0;
      return;
    }
    reg_d->seats[reg_d->num_seats - 1] = seat;
  } else if (strcmp(interface, wl_compositor_interface.name) == 0) {
    ctx->compositor =
        wl_registry_bind(registry, name, &wl_compositor_interface, 1);
  } else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
    ctx->layer_shell =
        wl_registry_bind(registry, name, &zwlr_layer_shell_v1_interface, 1);
  } else if (strcmp(interface, wl_output_interface.name) == 0) {
    struct wl_output *output =
        wl_registry_bind(registry, name, &wl_output_interface, 3);

    reg_d->num_outputs++;
    reg_d->outputs = realloc(reg_d->outputs,
                             reg_d->num_outputs * sizeof(struct wl_output *));
    if (!reg_d->outputs) {
      reg_d->num_outputs = 0;
      return;
    }
    reg_d->outputs[reg_d->num_outputs - 1] = output;
  } else if (strcmp(interface, zxdg_output_manager_v1_interface.name) == 0) {
    ctx->output_manager =
        wl_registry_bind(registry, name, &zxdg_output_manager_v1_interface, 2);
  } else if (strcmp(interface, wp_cursor_shape_manager_v1_interface.name) ==
             0) {
    ctx->cursor_shape_manager = wl_registry_bind(
        registry, name, &wp_cursor_shape_manager_v1_interface, version);
  } else if (strcmp(interface, zwlr_screencopy_manager_v1_interface.name) ==
             0) {
    ctx->screencopy_manager = wl_registry_bind(
        registry, name, &zwlr_screencopy_manager_v1_interface, version);
  }
}

void registry_global_remove(void *data, struct wl_registry *registry,
                            uint32_t name) {}

void seat_capabilities(void *data, struct wl_seat *seat,
                       uint32_t capabilities) {
  struct samure_callback_data *d = (struct samure_callback_data *)data;
  struct samure_context *ctx = d->ctx;
  struct samure_seat *s = (struct samure_seat *)d->data;

  if (capabilities & WL_SEAT_CAPABILITY_POINTER) {
    s->pointer = wl_seat_get_pointer(seat);
    if (ctx->cursor_shape_manager) {
      s->cursor_shape_device = wp_cursor_shape_manager_v1_get_pointer(
          ctx->cursor_shape_manager, s->pointer);
    }
  }
  if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD) {
    s->keyboard = wl_seat_get_keyboard(seat);
  }
  if (capabilities & WL_SEAT_CAPABILITY_TOUCH) {
    s->touch = wl_seat_get_touch(seat);
  }
}

void seat_name(void *data, struct wl_seat *wl_seat, const char *name) {
  struct samure_seat *s = (struct samure_seat *)data;

  s->name = strdup(name);
}

void pointer_enter(void *data, struct wl_pointer *pointer, uint32_t serial,
                   struct wl_surface *surface, wl_fixed_t surface_x,
                   wl_fixed_t surface_y) {
  struct samure_callback_data *d = (struct samure_callback_data *)data;
  struct samure_seat *seat = (struct samure_seat *)d->data;
  struct samure_context *ctx = d->ctx;

  if (seat->cursor_shape != 0 && seat->cursor_shape_device) {
    wp_cursor_shape_device_v1_set_shape(seat->cursor_shape_device, serial,
                                        seat->cursor_shape);
  }
  seat->last_pointer_enter = serial;

  OUTPUT_FOR_SURFACE();
  seat->pointer_focus.output = output;
  seat->pointer_focus.surface = layer_surface;

  NEW_EVENT();

  LAST_EVENT.type = SAMURE_EVENT_POINTER_ENTER;
  LAST_EVENT.seat = seat;
  LAST_EVENT.x = wl_fixed_to_double(surface_x);
  LAST_EVENT.y = wl_fixed_to_double(surface_y);
  LAST_EVENT.output = output;
  LAST_EVENT.surface = layer_surface;
}

void pointer_leave(void *data, struct wl_pointer *pointer, uint32_t serial,
                   struct wl_surface *surface) {
  struct samure_callback_data *d = (struct samure_callback_data *)data;
  struct samure_seat *seat = (struct samure_seat *)d->data;
  struct samure_context *ctx = d->ctx;

  OUTPUT_FOR_SURFACE();
  seat->pointer_focus.output = NULL;
  seat->pointer_focus.surface = NULL;

  NEW_EVENT();

  LAST_EVENT.type = SAMURE_EVENT_POINTER_LEAVE;
  LAST_EVENT.seat = seat;
  LAST_EVENT.output = output;
  LAST_EVENT.surface = layer_surface;
}

void pointer_motion(void *data, struct wl_pointer *pointer, uint32_t time,
                    wl_fixed_t surface_x, wl_fixed_t surface_y) {
  struct samure_callback_data *d = (struct samure_callback_data *)data;
  struct samure_context *ctx = d->ctx;

  NEW_EVENT();

  LAST_EVENT.type = SAMURE_EVENT_POINTER_MOTION;
  LAST_EVENT.seat = (struct samure_seat *)d->data;
  LAST_EVENT.x = wl_fixed_to_double(surface_x);
  LAST_EVENT.y = wl_fixed_to_double(surface_y);
}

void pointer_button(void *data, struct wl_pointer *pointer, uint32_t serial,
                    uint32_t time, uint32_t button, uint32_t state) {
  struct samure_callback_data *d = (struct samure_callback_data *)data;
  struct samure_context *ctx = d->ctx;
  struct samure_seat *seat = (struct samure_seat *)d->data;

  NEW_EVENT();

  LAST_EVENT.type = SAMURE_EVENT_POINTER_BUTTON;
  LAST_EVENT.seat = (struct samure_seat *)d->data;
  LAST_EVENT.button = button;
  LAST_EVENT.state = state;
}

void pointer_axis(void *data, struct wl_pointer *wl_pointer, uint32_t time,
                  uint32_t axis, wl_fixed_t value) {}

void layer_surface_configure(void *data,
                             struct zwlr_layer_surface_v1 *layer_surface,
                             uint32_t serial, uint32_t width, uint32_t height) {
  struct samure_callback_data *d = (struct samure_callback_data *)data;
  struct samure_context *ctx = d->ctx;
  zwlr_layer_surface_v1_ack_configure(layer_surface, serial);

  NEW_EVENT();

  LAST_EVENT.type = SAMURE_EVENT_LAYER_SURFACE_CONFIGURE;
  LAST_EVENT.surface = (struct samure_layer_surface *)d->data;
  LAST_EVENT.width = width;
  LAST_EVENT.height = height;
}

void layer_surface_closed(void *data,
                          struct zwlr_layer_surface_v1 *layer_surface) {}

void output_geometry(void *data, struct wl_output *wl_output, int32_t x,
                     int32_t y, int32_t physical_width, int32_t physical_height,
                     int32_t subpixel, const char *make, const char *model,
                     int32_t transform) {
  struct samure_output *o = (struct samure_output *)data;
  o->geo.x = x;
  o->geo.y = y;
}

void output_done(void *data, struct wl_output *wl_output) {
  struct samure_output *o = (struct samure_output *)data;
  if (o->scale != 0) {
    o->geo.w /= o->scale;
    o->geo.h /= o->scale;
  }
}

void output_scale(void *data, struct wl_output *wl_output, int32_t factor) {
  struct samure_output *o = (struct samure_output *)data;
  o->scale = factor;
}

void output_name(void *data, struct wl_output *wl_output, const char *name) {
  struct samure_output *o = (struct samure_output *)data;
  o->name = strdup(name);
}

void output_description(void *data, struct wl_output *wl_output,
                        const char *description) {}

void output_mode(void *data, struct wl_output *wl_output, uint32_t flags,
                 int32_t width, int32_t height, int32_t refresh) {
  struct samure_output *o = (struct samure_output *)data;
  if ((flags & WL_OUTPUT_MODE_CURRENT) == 0) {
    return;
  }

  o->geo.w = width;
  o->geo.h = height;
}

void xdg_output_logical_position(void *data,
                                 struct zxdg_output_v1 *zxdg_output_v1,
                                 int32_t x, int32_t y) {
  struct samure_output *o = data;
  o->geo.x = x;
  o->geo.y = y;
}

void xdg_output_logical_size(void *data, struct zxdg_output_v1 *zxdg_output_v1,
                             int32_t width, int32_t height) {
  struct samure_output *o = data;
  o->geo.w = width;
  o->geo.h = height;
}

void xdg_output_done(void *data, struct zxdg_output_v1 *zxdg_output_v1) {}

void xdg_output_name(void *data, struct zxdg_output_v1 *zxdg_output_v1,
                     const char *name) {
  struct samure_output *o = data;
  o->name = strdup(name);
}

void xdg_output_description(void *data, struct zxdg_output_v1 *zxdg_output_v1,
                            const char *description) {}

void keyboard_keymap(void *data, struct wl_keyboard *wl_keyboard,
                     uint32_t format, int32_t fd, uint32_t size) {}

void keyboard_enter(void *data, struct wl_keyboard *wl_keyboard,
                    uint32_t serial, struct wl_surface *surface,
                    struct wl_array *keys) {
  struct samure_callback_data *d = (struct samure_callback_data *)data;
  struct samure_context *ctx = d->ctx;
  struct samure_seat *seat = (struct samure_seat *)d->data;

  OUTPUT_FOR_SURFACE();
  seat->keyboard_focus.output = output;
  seat->keyboard_focus.surface = layer_surface;

  NEW_EVENT();

  LAST_EVENT.type = SAMURE_EVENT_KEYBOARD_ENTER;
  LAST_EVENT.seat = seat;
  LAST_EVENT.output = output;
  LAST_EVENT.surface = layer_surface;
}

void keyboard_leave(void *data, struct wl_keyboard *wl_keyboard,
                    uint32_t serial, struct wl_surface *surface) {
  struct samure_callback_data *d = (struct samure_callback_data *)data;
  struct samure_context *ctx = d->ctx;
  struct samure_seat *seat = (struct samure_seat *)d->data;

  seat->keyboard_focus.output = NULL;
  seat->keyboard_focus.surface = NULL;

  NEW_EVENT();

  LAST_EVENT.type = SAMURE_EVENT_KEYBOARD_LEAVE;
  LAST_EVENT.seat = seat;
  LAST_EVENT.output = NULL;
}

void keyboard_key(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial,
                  uint32_t time, uint32_t key, uint32_t state) {
  struct samure_callback_data *d = (struct samure_callback_data *)data;
  struct samure_context *ctx = d->ctx;
  struct samure_seat *seat = (struct samure_seat *)d->data;

  NEW_EVENT();

  LAST_EVENT.type = SAMURE_EVENT_KEYBOARD_KEY;
  LAST_EVENT.seat = seat;
  LAST_EVENT.button = key;
  LAST_EVENT.state = state;
}

void keyboard_modifiers(void *data, struct wl_keyboard *wl_keyboard,
                        uint32_t serial, uint32_t mods_depressed,
                        uint32_t mods_latched, uint32_t mods_locked,
                        uint32_t group) {}

void keyboard_repeat_info(void *data, struct wl_keyboard *wl_keyboard,
                          int32_t rate, int32_t delay) {}

void screencopy_frame_buffer(
    void *data, struct zwlr_screencopy_frame_v1 *zwlr_screencopy_frame_v1,
    uint32_t format, uint32_t width, uint32_t height, uint32_t stride) {
  struct samure_screenshot_data *d = (struct samure_screenshot_data *)data;

  d->buffer_rs =
      samure_create_shared_buffer(d->ctx->shm, format, width, height);
}

void screencopy_frame_flags(
    void *data, struct zwlr_screencopy_frame_v1 *zwlr_screencopy_frame_v1,
    uint32_t flags) {}

void screencopy_frame_ready(
    void *data, struct zwlr_screencopy_frame_v1 *zwlr_screencopy_frame_v1,
    uint32_t tv_sec_hi, uint32_t tv_sec_lo, uint32_t tv_nsec) {
  struct samure_screenshot_data *d = (struct samure_screenshot_data *)data;
  d->state = SAMURE_SCREENSHOT_READY;
}

void screencopy_frame_failed(
    void *data, struct zwlr_screencopy_frame_v1 *zwlr_screencopy_frame_v1) {
  struct samure_screenshot_data *d = (struct samure_screenshot_data *)data;
  d->state = SAMURE_SCREENSHOT_FAILED;
}

void screencopy_frame_damage(
    void *data, struct zwlr_screencopy_frame_v1 *zwlr_screencopy_frame_v1,
    uint32_t x, uint32_t y, uint32_t width, uint32_t height) {}

void screencopy_frame_linux_dmabuf(
    void *data, struct zwlr_screencopy_frame_v1 *zwlr_screencopy_frame_v1,
    uint32_t format, uint32_t width, uint32_t height) {}

void screencopy_frame_buffer_done(
    void *data, struct zwlr_screencopy_frame_v1 *zwlr_screencopy_frame_v1) {
  struct samure_screenshot_data *d = (struct samure_screenshot_data *)data;
  d->state = SAMURE_SCREENSHOT_DONE;
}

void touch_down(void *data, struct wl_touch *wl_touch, uint32_t serial,
                uint32_t time, struct wl_surface *surface, int32_t id,
                wl_fixed_t x, wl_fixed_t y) {
  struct samure_callback_data *d = (struct samure_callback_data *)data;
  struct samure_context *ctx = d->ctx;
  struct samure_seat *seat = (struct samure_seat *)d->data;

  OUTPUT_FOR_SURFACE();
  seat->touch_focus.output = output;
  seat->touch_focus.surface = layer_surface;

  NEW_EVENT();

  LAST_EVENT.type = SAMURE_EVENT_TOUCH_DOWN;
  LAST_EVENT.seat = seat;
  LAST_EVENT.output = output;
  LAST_EVENT.surface = layer_surface;
  LAST_EVENT.x = wl_fixed_to_double(x);
  LAST_EVENT.y = wl_fixed_to_double(y);
  LAST_EVENT.touch_id = id;
}

void touch_up(void *data, struct wl_touch *wl_touch, uint32_t serial,
              uint32_t time, int32_t id) {
  struct samure_callback_data *d = (struct samure_callback_data *)data;
  struct samure_context *ctx = d->ctx;
  struct samure_seat *seat = (struct samure_seat *)d->data;

  seat->touch_focus.output = NULL;
  seat->touch_focus.surface = NULL;

  NEW_EVENT();

  LAST_EVENT.type = SAMURE_EVENT_TOUCH_UP;
  LAST_EVENT.seat = seat;
  LAST_EVENT.touch_id = id;
}

void touch_motion(void *data, struct wl_touch *wl_touch, uint32_t time,
                  int32_t id, wl_fixed_t x, wl_fixed_t y) {
  struct samure_callback_data *d = (struct samure_callback_data *)data;
  struct samure_context *ctx = d->ctx;
  struct samure_seat *seat = (struct samure_seat *)d->data;

  NEW_EVENT();

  LAST_EVENT.type = SAMURE_EVENT_TOUCH_UP;
  LAST_EVENT.seat = seat;
  LAST_EVENT.x = wl_fixed_to_double(x);
  LAST_EVENT.y = wl_fixed_to_double(y);
  LAST_EVENT.touch_id = id;
}

void touch_frame(void *data, struct wl_touch *wl_touch) {}

void touch_cancel(void *data, struct wl_touch *wl_touch) {}

void touch_shape(void *data, struct wl_touch *wl_touch, int32_t id,
                 wl_fixed_t major, wl_fixed_t minor) {}

void touch_orientation(void *data, struct wl_touch *wl_touch, int32_t id,
                       wl_fixed_t orientation) {}

struct samure_callback_data *
samure_create_callback_data(struct samure_context *ctx, void *data) {
  struct samure_callback_data *d = (struct samure_callback_data *)malloc(
      sizeof(struct samure_callback_data));
  assert(d != NULL);

  d->ctx = ctx;
  d->data = data;

  return d;
}
