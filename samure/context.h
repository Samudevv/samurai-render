#pragma once

#include "wayland/cursor-shape-v1-client-protocol.h"
#include "wayland/wlr-layer-shell-unstable-v1.h"
#include "wayland/xdg-output-unstable-v1.h"
#include <wayland-client.h>

#include "backend.h"
#include "events.h"
#include "frame_timer.h"
#include "output.h"
#include "seat.h"

#define SAMURE_NO_CONTEXT_CONFIG NULL
#define SAMURE_MAX_FPS 60

struct samure_context;
struct samure_opengl_config;

enum samure_backend_type {
  SAMURE_BACKEND_RAW,
  SAMURE_BACKEND_OPENGL,
  SAMURE_BACKEND_CAIRO,
  SAMURE_BACKEND_NONE,
};

enum samure_render_state {
  SAMURE_RENDER_STATE_ALWAYS,
  SAMURE_RENDER_STATE_NONE,
  SAMURE_RENDER_STATE_ONCE,
};

typedef void (*samure_event_callback)(struct samure_event *event,
                                      struct samure_context *ctx,
                                      void *user_data);
typedef void (*samure_render_callback)(
    struct samure_output *output, struct samure_layer_surface *layer_surface,
    struct samure_context *ctx, double delta_time, void *user_data);
typedef void (*samure_update_callback)(struct samure_context *ctx,
                                       double delta_time, void *user_data);

struct samure_context_config {
  enum samure_backend_type backend;
  int pointer_interaction;
  int keyboard_interaction;
  int touch_interaction;
  uint32_t max_fps;
  struct samure_opengl_config *gl;
  int not_create_output_layer_surfaces;

  samure_event_callback event_callback;
  samure_render_callback render_callback;
  samure_update_callback update_callback;

  void *user_data;
};

extern struct samure_context_config samure_default_context_config();
extern struct samure_context_config
samure_create_context_config(samure_event_callback event_callback,
                             samure_render_callback render_callback,
                             samure_update_callback update_callback,
                             void *user_data);

struct samure_context {
  struct wl_display *display;
  struct wl_shm *shm;
  struct wl_compositor *compositor;
  struct zwlr_layer_shell_v1 *layer_shell;
  struct zxdg_output_manager_v1 *output_manager;
  struct wp_cursor_shape_manager_v1 *cursor_shape_manager;

  struct samure_seat *seats;
  size_t num_seats;

  struct samure_output *outputs;
  size_t num_outputs;

  struct samure_event *events;
  size_t num_events;
  size_t cap_events;
  size_t event_index;
  int running;
  enum samure_render_state render_state;

  char *error_string;

  struct samure_backend *backend;

  struct samure_context_config config;

  struct samure_frame_timer frame_timer;
};

extern struct samure_context *
samure_create_context(struct samure_context_config *config);
extern void samure_destroy_context(struct samure_context *ctx);
extern void samure_context_run(struct samure_context *ctx);
extern struct samure_rect
samure_context_get_output_rect(struct samure_context *ctx);
extern void samure_context_set_pointer_interaction(struct samure_context *ctx,
                                                   int enable);
extern void samure_context_set_input_regions(struct samure_context *ctx,
                                             struct samure_rect *rects,
                                             size_t num_rects);

extern void samure_context_set_keyboard_interaction(struct samure_context *ctx,
                                                    int enable);

extern void samure_context_process_events(struct samure_context *ctx,
                                          samure_event_callback event_callback);

extern void samure_context_render_output(struct samure_context *ctx,
                                         struct samure_output *output,
                                         samure_render_callback render_callback,
                                         double delta_time);

extern void samure_context_update(struct samure_context *ctx,
                                  samure_update_callback update_callback,
                                  double delta_time);

extern void
samure_context_create_output_layer_surfaces(struct samure_context *ctx);
