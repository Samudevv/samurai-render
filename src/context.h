#pragma once

#include "wayland/wlr-layer-shell-unstable-v1.h"
#include "wayland/xdg-output-unstable-v1.h"
#include <wayland-client.h>

#include "backend.h"
#include "events.h"
#include "output.h"
#include "seat.h"

#define SAMURE_NO_CONTEXT_CONFIG NULL

struct samure_context;

enum samure_backend_type {
  SAMURE_BACKEND_RAW,
  SAMURE_BACKEND_NONE,
};

typedef void (*samure_event_callback)(struct samure_event *event,
                                      struct samure_context *ctx,
                                      void *user_data);
typedef void (*samure_render_callback)(struct samure_output *output,
                                       struct samure_context *ctx,
                                       void *user_data);
typedef void (*samure_update_callback)(struct samure_context *ctx,
                                       void *user_data);

struct samure_context_config {
  enum samure_backend_type backend;

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

  struct samure_seat *seats;
  size_t num_seats;

  struct samure_output *outputs;
  size_t num_outputs;

  struct samure_event *events;
  size_t num_events;
  size_t event_index;
  int running;

  char *error_string;

  struct samure_backend *backend;

  struct samure_context_config config;
};

extern struct samure_context *
samure_create_context(struct samure_context_config *config);
extern void samure_destroy_context(struct samure_context *ctx);
extern void samure_context_frame_start(struct samure_context *ctx);
extern void samure_context_run(struct samure_context *ctx);
