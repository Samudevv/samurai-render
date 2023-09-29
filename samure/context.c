#include "context.h"
#include "callbacks.h"
#include "layer_surface.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "backends/cairo.h"
#include "backends/opengl.h"
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
  c.max_fps = SAMURE_MAX_FPS;
  return c;
}

struct samure_context_config
samure_create_context_config(samure_event_callback event_callback,
                             samure_render_callback render_callback,
                             samure_update_callback update_callback,
                             void *user_data) {
  struct samure_context_config c = samure_default_context_config();
  c.event_callback = event_callback;
  c.render_callback = render_callback;
  c.update_callback = update_callback;
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
  if (ctx->num_outputs == 0)             CTX_ADD_ERR_F("%s", "no outputs");
  if (ctx->output_manager == NULL)       CTX_ADD_ERR_F("%s", "no xdg output manager support");
  if (ctx->layer_shell == NULL)          CTX_ADD_ERR_F("%s", "no wlr layer shell support");
  if (ctx->shm == NULL)                  CTX_ADD_ERR_F("%s", "no shm support");
  if (ctx->compositor == NULL)           CTX_ADD_ERR_F("%s", "no compositor support");
  if (ctx->cursor_shape_manager == NULL) CTX_ADD_ERR_F("%s", "no cursor shape manager support");
  // clang-format on

  if (ctx->error_string) {
    return ctx;
  }

  if (ctx->num_seats != 0) {
    for (size_t i = 0; i < ctx->num_seats; i++) {
      wl_seat_add_listener(ctx->seats[i].seat, &seat_listener,
                           samure_create_callback_data(ctx, &ctx->seats[i]));
    }
    wl_display_roundtrip(ctx->display);
    for (size_t i = 0; i < ctx->num_seats; i++) {
      struct samure_callback_data *cbd =
          samure_create_callback_data(ctx, &ctx->seats[i]);

      if (ctx->seats[i].pointer) {
        wl_pointer_add_listener(ctx->seats[i].pointer, &pointer_listener, cbd);
      }
      if (ctx->seats[i].keyboard) {
        wl_keyboard_add_listener(ctx->seats[i].keyboard, &keyboard_listener,
                                 cbd);
      }
      if (ctx->seats[i].touch) {
        // TODO: Add touch listener
      }
    }
  }

  switch (ctx->config.backend) {
  case SAMURE_BACKEND_OPENGL: {
#ifdef BACKEND_OPENGL
    struct samure_backend_opengl *o =
        samure_init_backend_opengl(ctx, ctx->config.gl);
    ctx->config.gl = NULL;
    if (o->error_string) {
      CTX_ERR_F("failed to initialize opengl backend: %s", o->error_string);
      free(o->error_string);
      free(o);
      return ctx;
    }
    ctx->backend = &o->base;
#else
    CTX_ERR("samurai-render has not been build with opengl support (Build "
            "using --backend_opengl=y)");
#endif
  } break;
  case SAMURE_BACKEND_CAIRO: {
#ifdef BACKEND_CAIRO
    struct samure_backend_cairo *c = samure_init_backend_cairo(ctx);
    if (c->error_string) {
      CTX_ERR_F("failed to initialize cairo backend: %s", c->error_string);
      free(c->error_string);
      free(c);
      return ctx;
    }
    ctx->backend = &c->base;
#else
    CTX_ERR("samurai-render has not been build with cairo support (Build using "
            "--backend_cairo=y)");
#endif
  } break;
  case SAMURE_BACKEND_NONE:
    break;
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

  for (size_t i = 0; i < ctx->num_outputs; i++) {
    struct samure_output *o = &ctx->outputs[i];

    o->xdg_output =
        zxdg_output_manager_v1_get_xdg_output(ctx->output_manager, o->output);
    zxdg_output_v1_add_listener(o->xdg_output, &xdg_output_listener, o);
  }
  wl_display_roundtrip(ctx->display);

  ctx->frame_timer = samure_init_frame_timer(ctx->config.max_fps);

  if (!ctx->config.not_create_output_layer_surfaces) {
    samure_context_create_output_layer_surfaces(ctx);
  }

  return ctx;
}

void samure_destroy_context(struct samure_context *ctx) {
  wl_display_flush(ctx->display);

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
    samure_destroy_output(ctx, ctx->outputs[i]);
  }
  free(ctx->outputs);

  wl_display_disconnect(ctx->display);

  free(ctx->events);

  free(ctx->error_string);
  free(ctx);
}

void samure_context_run(struct samure_context *ctx) {
  ctx->running = 1;
  while (ctx->running) {
    samure_frame_timer_start_frame(&ctx->frame_timer);

    samure_context_process_events(ctx, ctx->config.event_callback);

    if (ctx->render_state != SAMURE_RENDER_STATE_NONE) {
      for (size_t i = 0; i < ctx->num_outputs; i++) {
        samure_context_render_output(ctx, &ctx->outputs[i],
                                     ctx->config.render_callback,
                                     ctx->frame_timer.delta_time);
      }

      if (ctx->render_state == SAMURE_RENDER_STATE_ONCE) {
        ctx->render_state = SAMURE_RENDER_STATE_NONE;
      }
    }

    samure_context_update(ctx, ctx->config.update_callback,
                          ctx->frame_timer.delta_time);

    samure_frame_timer_end_frame(&ctx->frame_timer);
  }
}

struct samure_rect samure_context_get_output_rect(struct samure_context *ctx) {
  struct samure_rect r = {
      .x = ctx->outputs[0].geo.x,
      .y = ctx->outputs[0].geo.y,
      .w = ctx->outputs[0].geo.x + ctx->outputs[0].geo.w,
      .h = ctx->outputs[0].geo.y + ctx->outputs[0].geo.h,
  };

  for (size_t i = 1; i < ctx->num_outputs; i++) {
    if (ctx->outputs[i].geo.x < r.x) {
      r.x = ctx->outputs[i].geo.x;
    }
    if (ctx->outputs[i].geo.y < r.y) {
      r.y = ctx->outputs[i].geo.y;
    }
    if (ctx->outputs[i].geo.x + ctx->outputs[i].geo.w > r.w) {
      r.w = ctx->outputs[i].geo.x + ctx->outputs[i].geo.w;
    }
    if (ctx->outputs[i].geo.y + ctx->outputs[i].geo.h > r.h) {
      r.h = ctx->outputs[i].geo.y + ctx->outputs[i].geo.h;
    }
  }

  r.w -= r.x;
  r.h -= r.y;

  return r;
}

void samure_context_set_pointer_interaction(struct samure_context *ctx,
                                            int enable) {
  for (size_t i = 0; i < ctx->num_outputs; i++) {
    samure_output_set_pointer_interaction(ctx, &ctx->outputs[i], enable);
  }
}

void samure_context_set_input_regions(struct samure_context *ctx,
                                      struct samure_rect *r, size_t num_rects) {
  for (size_t j = 0; j < ctx->num_outputs; j++) {
    struct samure_rect *output_rects = NULL;
    size_t num_output_rects = 0;

    for (size_t i = 0; i < num_rects; i++) {
      if (samure_rect_in_output(&ctx->outputs[j], r[i].x, r[i].y, r[i].w,
                                r[i].h)) {
        num_output_rects++;
        output_rects = realloc(output_rects,
                               num_output_rects * sizeof(struct samure_rect));

        output_rects[num_output_rects - 1].x =
            OUT_X2((&ctx->outputs[i]), r[i].x);
        output_rects[num_output_rects - 1].y =
            OUT_Y2((&ctx->outputs[i]), r[i].y);
        output_rects[num_output_rects - 1].w = r[i].w;
        output_rects[num_output_rects - 1].h = r[i].h;
      }
    }

    samure_output_set_input_regions(ctx, &ctx->outputs[j], output_rects,
                                    num_output_rects);
    free(output_rects);
  }
}

void samure_context_set_keyboard_interaction(struct samure_context *ctx,
                                             int enable) {
  for (size_t i = 0; i < ctx->num_outputs; i++) {
    samure_output_set_keyboard_interaction(&ctx->outputs[i], enable);
  }
}

void samure_context_process_events(struct samure_context *ctx,
                                   samure_event_callback event_callback) {
  wl_display_roundtrip(ctx->display);

  // Process events
  for (; ctx->event_index < ctx->num_events; ctx->event_index++) {
    struct samure_event *e = &ctx->events[ctx->event_index];

    switch (e->type) {
    case SAMURE_EVENT_LAYER_SURFACE_CONFIGURE:
      if (ctx->backend && ctx->backend->on_layer_surface_configure) {
        ctx->backend->on_layer_surface_configure(
            ctx->backend, ctx, e->output, e->surface, e->width, e->height);
      }
      break;
    default:
      if (event_callback) {
        event_callback(e, ctx, ctx->config.user_data);
      }
      break;
    }
  }
  ctx->event_index = 0;
  ctx->num_events = 0;
}

void samure_context_render_output(struct samure_context *ctx,
                                  struct samure_output *output,
                                  samure_render_callback render_callback,
                                  double delta_time) {
  for (size_t i = 0; i < output->num_sfc; i++) {
    if (ctx->backend && ctx->backend->render_start) {
      ctx->backend->render_start(output, output->sfc[i], ctx, ctx->backend);
    }

    if (render_callback) {
      render_callback(output, output->sfc[i], ctx, delta_time,
                      ctx->config.user_data);
    }

    if (ctx->backend && ctx->backend->render_end) {
      ctx->backend->render_end(output, output->sfc[i], ctx, ctx->backend);
    }
  }
}

void samure_context_update(struct samure_context *ctx,
                           samure_update_callback update_callback,
                           double delta_time) {
  if (update_callback) {
    update_callback(ctx, delta_time, ctx->config.user_data);
  }
}

void samure_context_create_output_layer_surfaces(struct samure_context *ctx) {
  for (size_t i = 0; i < ctx->num_outputs; i++) {
    struct samure_output *o = &ctx->outputs[i];

    struct samure_layer_surface *layer_surface = samure_create_layer_surface(
        ctx, o, SAMURE_LAYER_OVERLAY, SAMURE_LAYER_SURFACE_ANCHOR_FILL,
        (uint32_t)ctx->config.keyboard_interaction,
        ctx->config.pointer_interaction || ctx->config.touch_interaction, 1);
    if (layer_surface->error_string) {
      CTX_ADD_ERR_F("failed to create layer surface for output %zu: %s", i,
                    layer_surface->error_string);
      free(layer_surface->error_string);
      free(layer_surface);
      continue;
    }

    samure_output_attach_layer_surface(o, layer_surface);
  }
}
