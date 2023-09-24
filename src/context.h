#pragma once

#include "wayland/wlr-layer-shell-unstable-v1.h"
#include "wayland/xdg-output-unstable-v1.h"
#include <wayland-client.h>

#include "backend.h"
#include "events.h"
#include "output.h"
#include "seat.h"

#define SAMURE_NO_CONTEXT_CONFIG NULL

enum samure_backend_type {
  SAMURE_BACKEND_RAW,
};

struct samure_context_config {
  enum samure_backend_type backend;
};

extern struct samure_context_config samure_default_context_config();

struct samure_context;

typedef void (*samure_event_callback)(struct samure_event *event,
                                      struct samure_context *ctx,
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

  char *error_string;

  samure_event_callback event_callback;
  void *event_user_data;

  struct samure_backend *backend;

  struct samure_context_config config;
};

extern struct samure_context *
samure_create_context(struct samure_context_config *config);
extern void samure_destroy_context(struct samure_context *ctx);
extern void samure_context_frame_start(struct samure_context *ctx);
extern void samure_context_frame_end(struct samure_context *ctx);
extern struct samure_backend_raw *
samure_get_backend_raw(struct samure_context *ctx);
