#include "context.h"
#include "callbacks.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "backends/raw.h"

#define CTX_ERR(msg)                                                           \
  if (ctx->error_string)                                                       \
    free(ctx->error_string);                                                   \
  ctx->error_string = strdup(msg)
#define CTX_ERR_F(format, ...)                                                 \
  if (ctx->error_string)                                                       \
    free(ctx->error_string);                                                   \
  ctx->error_string = malloc(2048);                                            \
  snprintf(ctx->error_string, 2048, format, __VA_ARGS__)
#define CTX_ADD_ERR_F(format, ...)                                             \
  {                                                                            \
    const size_t error_string_len =                                            \
        ctx->error_string ? strlen(ctx->error_string) : 0;                     \
    ctx->error_string = realloc(ctx->error_string, error_string_len + 2048);   \
    snprintf(&ctx->error_string[error_string_len], 2048, "\n" format,          \
             __VA_ARGS__);                                                     \
  }

struct samure_context_config samure_default_context_config() {
  struct samure_context_config c = {0};
  return c;
}

struct samure_context_config
samure_create_context_config(samure_event_callback event_callback,
                             void *user_data) {
  struct samure_context_config c = samure_default_context_config();
  c.event_callback = event_callback;
  c.user_data = user_data;
  return c;
}

struct samure_context *
samure_create_context(struct samure_context_config *config) {
  struct samure_context *ctx = malloc(sizeof(struct samure_context));
  memset(ctx, 0, sizeof(*ctx));

  if (config) {
    ctx->config = *config;
  } else {
    ctx->config = samure_default_context_config();
  }

  ctx->display = wl_display_connect(NULL);
  if (ctx->display == NULL) {
    CTX_ERR("failed to connect to display");
    return ctx;
  }

  struct wl_registry *registry = wl_display_get_registry(ctx->display);
  wl_registry_add_listener(registry, &registry_listener, ctx);
  wl_display_roundtrip(ctx->display);

  // clang-format off
  if (ctx->num_outputs == 0)       CTX_ADD_ERR_F("%s", "no outputs");
  if (ctx->output_manager == NULL) CTX_ADD_ERR_F("%s", "no xdg output manager support");
  if (ctx->layer_shell == NULL)    CTX_ADD_ERR_F("%s", "no wlr layer shell support");
  if (ctx->shm == NULL)            CTX_ADD_ERR_F("%s", "no shm support");
  if (ctx->compositor == NULL)     CTX_ADD_ERR_F("%s", "no compositor support");
  // clang-format on

  if (ctx->error_string) {
    return ctx;
  }

  if (ctx->num_seats != 0) {
    for (size_t i = 0; i < ctx->num_seats; i++) {
      wl_seat_add_listener(ctx->seats[i].seat, &seat_listener, &ctx->seats[i]);
    }
    wl_display_roundtrip(ctx->display);
    for (size_t i = 0; i < ctx->num_seats; i++) {
      if (ctx->seats[i].pointer) {
        wl_pointer_add_listener(
            ctx->seats[i].pointer, &pointer_listener,
            samure_create_callback_data(ctx, &ctx->seats[i]));
      }
      // TODO: Add keyboard listener
      // if (ctx->seats[i].keyboard) {
      //    wl_keyboard_add_listener(ctx->seats[i].keyboard, NULL,
      //    ctx);
      // }
      // TODO: Add touch listener
    }
  }

  for (size_t i = 0; i < ctx->num_outputs; i++) {
    struct samure_output *o = &ctx->outputs[i];

    char namespace[1024];
    snprintf(namespace, 1024, "samurai-render-%zu", i);

    o->xdg_output =
        zxdg_output_manager_v1_get_xdg_output(ctx->output_manager, o->output);
    zxdg_output_v1_add_listener(o->xdg_output, &xdg_output_listener, o);

    o->surface = wl_compositor_create_surface(ctx->compositor);
    o->layer_surface = zwlr_layer_shell_v1_get_layer_surface(
        ctx->layer_shell, o->surface, o->output,
        ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY, namespace);
    zwlr_layer_surface_v1_add_listener(o->layer_surface,
                                       &layer_surface_listener,
                                       samure_create_callback_data(ctx, o));
    zwlr_layer_surface_v1_set_anchor(o->layer_surface,
                                     ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP |
                                         ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT |
                                         ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT |
                                         ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM);
    zwlr_layer_surface_v1_set_keyboard_interactivity(o->layer_surface, 0);
    zwlr_layer_surface_v1_set_exclusive_zone(o->layer_surface, -1);
    wl_surface_commit(o->surface);
  }
  wl_display_roundtrip(ctx->display);

  switch (ctx->config.backend) {
  default: // SAMURE_BACKEND_RAW
  {
    struct samure_backend_raw *r = samure_init_backend_raw(ctx);
    if (r->error_string) {
      CTX_ERR_F("failed to initialize raw backend: %s", r->error_string);
      free(r->error_string);
      free(r);
      return ctx;
    }
    ctx->backend = &r->base;
  } break;
  }

  return ctx;
}

void samure_destroy_context(struct samure_context *ctx) {
  if (ctx->backend && ctx->backend->destroy) {
    ctx->backend->destroy(ctx, ctx->backend);
  }

  wl_shm_destroy(ctx->shm);
  wl_compositor_destroy(ctx->compositor);
  zwlr_layer_shell_v1_destroy(ctx->layer_shell);
  zxdg_output_manager_v1_destroy(ctx->output_manager);

  for (size_t i = 0; i < ctx->num_seats; i++) {
    samure_destroy_seat(ctx->seats[i]);
  }
  free(ctx->seats);

  for (size_t i = 0; i < ctx->num_outputs; i++) {
    samure_destroy_output(ctx->outputs[i]);
  }
  free(ctx->outputs);

  wl_display_disconnect(ctx->display);

  free(ctx->events);

  free(ctx->error_string);
  free(ctx);
}

void samure_context_frame_start(struct samure_context *ctx) {
  wl_display_roundtrip(ctx->display);

  // Process events
  for (; ctx->event_index < ctx->num_events; ctx->event_index++) {
    struct samure_event *e = &ctx->events[ctx->event_index];

    switch (e->type) {
    case SAMURE_EVENT_LAYER_SURFACE_CONFIGURE:
      if (ctx->backend && ctx->backend->on_layer_surface_configure) {
        ctx->backend->on_layer_surface_configure(ctx->backend, ctx, e->output,
                                                 e->width, e->height);
      }
      break;
    default:
      if (ctx->config.event_callback) {
        ctx->config.event_callback(e, ctx, ctx->config.user_data);
      }
      break;
    }
  }
}

void samure_context_frame_end(struct samure_context *ctx) {
  if (ctx->backend && ctx->backend->frame_end) {
    ctx->backend->frame_end(ctx, ctx->backend);
  }
}