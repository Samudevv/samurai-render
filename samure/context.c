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

SAMURE_RESULT(context)
samure_create_context(struct samure_context_config *config) {
  SAMURE_RESULT_ALLOC(context, ctx);

  if (config) {
    ctx->config = *config;
  } else {
    ctx->config = samure_default_context_config();
  }

  ctx->display = wl_display_connect(NULL);
  if (ctx->display == NULL) {
    SAMURE_DESTROY_ERROR(context, ctx, SAMURE_ERROR_DISPLAY_CONNECT);
  }

  struct wl_registry *registry = wl_display_get_registry(ctx->display);
  wl_registry_add_listener(registry, &registry_listener, ctx);
  wl_display_roundtrip(ctx->display);

  uint64_t error_code = SAMURE_ERROR_NONE;

  // clang-format off
  if (ctx->num_outputs == 0)             { error_code |= SAMURE_ERROR_NO_OUTPUTS;              }
  if (ctx->output_manager == NULL)       { error_code |= SAMURE_ERROR_NO_XDG_OUTPUT_MANAGER;   }
  if (ctx->layer_shell == NULL)          { error_code |= SAMURE_ERROR_NO_LAYER_SHELL;          }
  if (ctx->shm == NULL)                  { error_code |= SAMURE_ERROR_NO_SHM;                  }
  if (ctx->compositor == NULL)           { error_code |= SAMURE_ERROR_NO_COMPOSITOR;           }
  if (ctx->cursor_shape_manager == NULL) { error_code |= SAMURE_ERROR_NO_CURSOR_SHAPE_MANAGER; }
  if (ctx->screencopy_manager == NULL)   { error_code |= SAMURE_ERROR_NO_SCREENCOPY_MANAGER;   }
  // clang-format on

  if (SAMURE_IS_ERROR(error_code)) {
    SAMURE_DESTROY_ERROR(context, ctx, error_code);
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
    SAMURE_RESULT(backend_opengl)
    o_rs = samure_init_backend_opengl(ctx, ctx->config.gl);
    ctx->config.gl = NULL;
    if (SAMURE_HAS_ERROR(o_rs)) {
      SAMURE_DESTROY_ERROR(context, ctx,
                           SAMURE_ERROR_BACKEND_INIT | o_rs.error);
    }
    ctx->backend = &SAMURE_GET_RESULT(backend_opengl, o_rs)->base;
#else
    SAMURE_DESTROY_ERROR(context, ctx, SAMURE_ERROR_NO_BACKEND_SUPPORT);
#endif
  } break;
  case SAMURE_BACKEND_CAIRO: {
#ifdef BACKEND_CAIRO
    SAMURE_RESULT(backend_cairo) c_rs = samure_init_backend_cairo(ctx);
    if (SAMURE_HAS_ERROR(c_rs)) {
      SAMURE_DESTROY_ERROR(context, ctx,
                           SAMURE_ERROR_BACKEND_INIT | c_rs.error);
    }
    ctx->backend = &SAMURE_GET_RESULT(backend_cairo, c_rs)->base;
#else
    SAMURE_DESTROY_ERROR(context, ctx, SAMURE_ERROR_NO_BACKEND_SUPPORT);
#endif
  } break;
  case SAMURE_BACKEND_NONE:
    break;
  default: // SAMURE_BACKEND_RAW
  {
    SAMURE_RESULT(backend_raw) r_rs = samure_init_backend_raw(ctx);
    if (SAMURE_HAS_ERROR(r_rs)) {
      SAMURE_DESTROY_ERROR(context, ctx,
                           SAMURE_ERROR_BACKEND_INIT | r_rs.error);
    }
    ctx->backend = &SAMURE_GET_RESULT(backend_raw, r_rs)->base;
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
    const uint64_t err = samure_context_create_output_layer_surfaces(ctx);
    if (SAMURE_IS_ERROR(err)) {
      SAMURE_DESTROY_ERROR(context, ctx, err);
    }
  }

  SAMURE_RETURN_RESULT(context, ctx);
}

void samure_destroy_context(struct samure_context *ctx) {
  if (ctx->display)
    wl_display_flush(ctx->display);

  if (ctx->backend && ctx->backend->destroy) {
    ctx->backend->destroy(ctx, ctx->backend);
  }

  for (size_t i = 0; i < ctx->num_seats; i++) {
    samure_destroy_seat(ctx->seats[i]);
  }
  free(ctx->seats);

  for (size_t i = 0; i < ctx->num_outputs; i++) {
    samure_destroy_output(ctx, ctx->outputs[i]);
  }
  free(ctx->outputs);

  if (ctx->shm)
    wl_shm_destroy(ctx->shm);
  if (ctx->compositor)
    wl_compositor_destroy(ctx->compositor);
  if (ctx->layer_shell)
    zwlr_layer_shell_v1_destroy(ctx->layer_shell);
  if (ctx->output_manager)
    zxdg_output_manager_v1_destroy(ctx->output_manager);
  if (ctx->cursor_shape_manager)
    wp_cursor_shape_manager_v1_destroy(ctx->cursor_shape_manager);
  if (ctx->screencopy_manager)
    zwlr_screencopy_manager_v1_destroy(ctx->screencopy_manager);

  if (ctx->display)
    wl_display_disconnect(ctx->display);

  free(ctx->events);
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

uint64_t
samure_context_create_output_layer_surfaces(struct samure_context *ctx) {
  uint64_t error_code = SAMURE_ERROR_NONE;

  for (size_t i = 0; i < ctx->num_outputs; i++) {
    struct samure_output *o = &ctx->outputs[i];

    struct samure_layer_surface *layer_surface = samure_create_layer_surface(
        ctx, o, SAMURE_LAYER_OVERLAY, SAMURE_LAYER_SURFACE_ANCHOR_FILL,
        (uint32_t)ctx->config.keyboard_interaction,
        ctx->config.pointer_interaction || ctx->config.touch_interaction, 1);
    if (layer_surface->error_string) {
      free(layer_surface->error_string);
      free(layer_surface);
      error_code |= SAMURE_ERROR_LAYER_SURFACE_INIT;
      continue;
    }

    samure_output_attach_layer_surface(o, layer_surface);
  }

  return error_code;
}
