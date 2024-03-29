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

#pragma once

#include "wayland/cursor-shape.h"
#include "wayland/layer-shell.h"
#include "wayland/screencopy.h"
#include "wayland/viewporter.h"
#include "wayland/xdg-output.h"
#include <wayland-client.h>

#include "backend.h"
#include "cursors.h"
#include "error_handling.h"
#include "events.h"
#include "frame_timer.h"
#include "output.h"
#include "seat.h"

#define SAMURE_NO_CONTEXT_CONFIG NULL

struct samure_context;
struct samure_opengl_config;

// public
enum samure_backend_type {
  SAMURE_BACKEND_RAW,
  SAMURE_BACKEND_OPENGL,
  SAMURE_BACKEND_CAIRO,
  SAMURE_BACKEND_NONE,
};

// public
enum samure_render_state {
  SAMURE_RENDER_STATE_ALWAYS,
  SAMURE_RENDER_STATE_NONE,
  SAMURE_RENDER_STATE_ONCE,
};

// public
typedef void (*samure_event_callback)(struct samure_context *ctx,
                                      struct samure_event *event,
                                      void *user_data);
// public
typedef void (*samure_render_callback)(
    struct samure_context *ctx, struct samure_layer_surface *layer_surface,
    struct samure_rect output_geo, void *user_data);
// public
typedef void (*samure_update_callback)(struct samure_context *ctx,
                                       double delta_time, void *user_data);

// public
struct samure_app {
  samure_event_callback on_event;
  samure_render_callback on_render;
  samure_update_callback on_update;
};

// public
struct samure_context_config {
  enum samure_backend_type backend;
  int pointer_interaction;
  int keyboard_interaction;
  int touch_interaction;
  uint32_t max_update_frequency;
  struct samure_opengl_config *gl;
  int not_create_output_layer_surfaces;
  int not_request_frame;
  int force_client_cursors;

  samure_event_callback on_event;
  samure_render_callback on_render;
  samure_update_callback on_update;

  void *user_data;
};

// public
extern struct samure_context_config
samure_create_context_config(samure_event_callback event_callback,
                             samure_render_callback render_callback,
                             samure_update_callback update_callback,
                             void *user_data);

// public
struct samure_context {
  struct wl_display *display;
  struct wl_shm *shm;
  struct wl_compositor *compositor;
  struct zwlr_layer_shell_v1 *layer_shell;
  struct zxdg_output_manager_v1 *output_manager;
  struct zwlr_screencopy_manager_v1 *screencopy_manager;
  struct wp_fractional_scale_manager_v1 *fractional_scale_manager;
  struct wp_viewporter *viewporter;
  struct samure_cursor_engine *cursor_engine;

  struct samure_seat **seats;
  size_t num_seats;

  struct samure_output **outputs;
  size_t num_outputs;

  struct samure_event *events;
  size_t num_events;
  size_t cap_events;
  size_t event_index;
  int running;
  enum samure_render_state render_state;

  struct samure_backend *backend;

  struct samure_context_config config;
  struct samure_app app;

  struct samure_frame_timer frame_timer;
  void *backend_lib_handle;
};

struct samure_registry_data {
  struct wl_seat **seats;
  size_t num_seats;
  struct wl_output **outputs;
  size_t num_outputs;
  struct wp_cursor_shape_manager_v1 *cursor_manager;
  samure_error error;
};

SAMURE_DEFINE_RESULT(context);

// public
extern SAMURE_RESULT(context)
    samure_create_context(struct samure_context_config *config);
// public
extern SAMURE_RESULT(context)
    samure_create_context_with_backend(struct samure_context_config *config,
                                       struct samure_backend *backend);
// public
extern void samure_destroy_context(struct samure_context *ctx);
// public
extern void samure_context_run(struct samure_context *ctx);
// public
extern struct samure_rect
samure_context_get_output_rect(struct samure_context *ctx);
// public
extern void samure_context_set_pointer_interaction(struct samure_context *ctx,
                                                   int enable);
// public
extern void samure_context_set_input_regions(struct samure_context *ctx,
                                             struct samure_rect *rects,
                                             size_t num_rects);

// public
extern void samure_context_set_keyboard_interaction(struct samure_context *ctx,
                                                    int enable);

// public
extern void samure_context_process_events(struct samure_context *ctx);

// public
extern void
samure_context_render_layer_surface(struct samure_context *ctx,
                                    struct samure_layer_surface *sfc,
                                    struct samure_rect geo);

// public
extern void samure_context_render_output(struct samure_context *ctx,
                                         struct samure_output *output);

// public
extern void samure_context_update(struct samure_context *ctx,
                                  double delta_time);

// public
extern samure_error
samure_context_create_output_layer_surfaces(struct samure_context *ctx);

// public
extern void samure_context_set_pointer_shape(struct samure_context *ctx,
                                             uint32_t shape);

// public
extern void
samure_context_set_render_state(struct samure_context *ctx,
                                enum samure_render_state render_state);
