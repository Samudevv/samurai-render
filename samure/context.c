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

#include "context.h"
#include "callbacks.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "backends/cairo.h"
#include "backends/opengl.h"
#include "backends/raw.h"

SAMURE_DEFINE_RESULT_UNWRAP(context);

struct samure_context_config
samure_create_context_config(samure_event_callback event_callback,
                             samure_render_callback render_callback,
                             samure_update_callback update_callback,
                             void *user_data) {
  struct samure_context_config c = {0};
  c.on_event = event_callback;
  c.on_render = render_callback;
  c.on_update = update_callback;
  c.user_data = user_data;
  return c;
}

SAMURE_RESULT(context)
samure_create_context(struct samure_context_config *config) {
  SAMURE_RESULT_ALLOC(context, ctx);

  if (config) {
    ctx->config = *config;
  } else {
    memset(&ctx->config, 0, sizeof(struct samure_context_config));
  }
  ctx->app.on_event = ctx->config.on_event;
  ctx->app.on_update = ctx->config.on_update;
  ctx->app.on_render = ctx->config.on_render;

  ctx->display = wl_display_connect(NULL);
  if (ctx->display == NULL) {
    SAMURE_DESTROY_ERROR(context, ctx, SAMURE_ERROR_DISPLAY_CONNECT);
  }

  struct samure_registry_data reg_d = {0};

  struct wl_registry *registry = wl_display_get_registry(ctx->display);
  wl_registry_add_listener(registry, &registry_listener,
                           samure_create_callback_data(ctx, &reg_d));
  wl_display_roundtrip(ctx->display);

  samure_error error_code = SAMURE_ERROR_NONE;

  // clang-format off
  if (reg_d.num_outputs == 0)            { error_code |= SAMURE_ERROR_NO_OUTPUTS;              }
  if (ctx->layer_shell == NULL)          { error_code |= SAMURE_ERROR_NO_LAYER_SHELL;          }
  if (ctx->compositor == NULL)           { error_code |= SAMURE_ERROR_NO_COMPOSITOR;           }
  // clang-format on

  if (SAMURE_IS_ERROR(error_code)) {
    SAMURE_DESTROY_ERROR(context, ctx, error_code);
  }

  if (reg_d.num_seats != 0) {
    ctx->seats = malloc(ctx->num_seats * sizeof(struct samure_seat *));
    for (size_t i = 0; i < reg_d.num_seats; i++) {
      SAMURE_RESULT(seat) s_rs = samure_create_seat(ctx, reg_d.seats[i]);
      if (!SAMURE_HAS_ERROR(s_rs)) {
        ctx->seats[ctx->num_seats] = SAMURE_UNWRAP(seat, s_rs);
        ctx->num_seats++;
      }
    }
  }
  free(reg_d.seats);

  SAMURE_RESULT(cursor_engine)
  ce_rs = samure_create_cursor_engine(ctx, reg_d.cursor_manager);
  if (!SAMURE_HAS_ERROR(ce_rs)) {
    ctx->cursor_engine = SAMURE_UNWRAP(cursor_engine, ce_rs);
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
    ctx->backend = &SAMURE_UNWRAP(backend_opengl, o_rs)->base;
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
    ctx->backend = &SAMURE_UNWRAP(backend_cairo, c_rs)->base;
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
    ctx->backend = &SAMURE_UNWRAP(backend_raw, r_rs)->base;
  } break;
  }

  ctx->outputs = malloc(reg_d.num_outputs * sizeof(struct samure_output *));
  if (ctx->outputs) {
    for (size_t i = 0; i < reg_d.num_outputs; i++) {
      SAMURE_RESULT(output) o_rs = samure_create_output(ctx, reg_d.outputs[i]);
      if (!SAMURE_HAS_ERROR(o_rs)) {
        ctx->outputs[ctx->num_outputs] = SAMURE_UNWRAP(output, o_rs);
        ctx->num_outputs++;
      }
    }
  }
  free(reg_d.outputs);

  if (ctx->config.max_update_frequency == 0) {
    // Use double the maximum output refresh rate as update frequency by default
    int32_t max_refresh_rate = ctx->outputs[0]->refresh_rate;
    for (size_t i = 1; i < ctx->num_outputs; i++) {
      if (ctx->outputs[i]->refresh_rate > max_refresh_rate) {
        max_refresh_rate = ctx->outputs[i]->refresh_rate;
      }
    }
    ctx->config.max_update_frequency = 2 * max_refresh_rate;
  }

  ctx->frame_timer = samure_init_frame_timer(ctx->config.max_update_frequency);

  if (!ctx->config.not_create_output_layer_surfaces) {
    const samure_error err = samure_context_create_output_layer_surfaces(ctx);
    if (SAMURE_IS_ERROR(err)) {
      SAMURE_DESTROY_ERROR(context, ctx, err);
    }
  }

  SAMURE_RETURN_RESULT(context, ctx);
}

SAMURE_RESULT(context)
samure_create_context_with_backend(struct samure_context_config *config,
                                   struct samure_backend *backend) {
  struct samure_context_config cfg = {0};

  if (config) {
    cfg = *config;
  }

  const int not_create_output_layer_surfaces =
      cfg.not_create_output_layer_surfaces;
  cfg.not_create_output_layer_surfaces = 1;
  cfg.backend = SAMURE_BACKEND_NONE;

  SAMURE_RESULT(context) ctx_rs = samure_create_context(&cfg);
  if (SAMURE_HAS_ERROR(ctx_rs)) {
    return ctx_rs;
  }

  ctx_rs.result->backend = backend;

  if (!not_create_output_layer_surfaces) {
    cfg.not_create_output_layer_surfaces = 0;
    const samure_error err =
        samure_context_create_output_layer_surfaces(ctx_rs.result);
    if (SAMURE_IS_ERROR(err)) {
      samure_destroy_context(ctx_rs.result);
      ctx_rs.result = NULL;
      ctx_rs.error = err;
      return ctx_rs;
    }
  }

  return ctx_rs;
}

void samure_destroy_context(struct samure_context *ctx) {
  if (ctx->display)
    wl_display_flush(ctx->display);

  if (ctx->backend && ctx->backend->destroy) {
    ctx->backend->destroy(ctx);
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
  if (ctx->cursor_engine)
    samure_destroy_cursor_engine(ctx->cursor_engine);
  if (ctx->screencopy_manager)
    zwlr_screencopy_manager_v1_destroy(ctx->screencopy_manager);

  if (ctx->display)
    wl_display_disconnect(ctx->display);

  free(ctx->events);
  free(ctx);
}

void samure_context_run(struct samure_context *ctx) {
  samure_context_process_events(ctx);

  if (ctx->render_state != SAMURE_RENDER_STATE_NONE) {
    for (size_t i = 0; i < ctx->num_outputs; i++) {
      samure_context_render_output(ctx, ctx->outputs[i]);
    }
  }

  ctx->running = 1;
  while (ctx->running) {
    samure_frame_timer_start_frame(&ctx->frame_timer);

    samure_context_process_events(ctx);

    samure_context_update(ctx, ctx->frame_timer.delta_time);

    if (ctx->render_state != SAMURE_RENDER_STATE_NONE) {
      for (size_t i = 0; i < ctx->num_outputs; i++) {
        samure_context_render_output(ctx, ctx->outputs[i]);
      }
      if (ctx->render_state == SAMURE_RENDER_STATE_ONCE) {
        ctx->render_state = SAMURE_RENDER_STATE_NONE;
      }
    }

    samure_frame_timer_end_frame(&ctx->frame_timer);
  }
}

struct samure_rect samure_context_get_output_rect(struct samure_context *ctx) {
  if (ctx->num_outputs == 0) {
    struct samure_rect r = {.x = 0, .y = 0, .w = 0, .h = 0};
  }

  struct samure_rect r = {
      .x = ctx->outputs[0]->geo.x,
      .y = ctx->outputs[0]->geo.y,
      .w = ctx->outputs[0]->geo.x + ctx->outputs[0]->geo.w,
      .h = ctx->outputs[0]->geo.y + ctx->outputs[0]->geo.h,
  };

  for (size_t i = 1; i < ctx->num_outputs; i++) {
    if (ctx->outputs[i]->geo.x < r.x) {
      r.x = ctx->outputs[i]->geo.x;
    }
    if (ctx->outputs[i]->geo.y < r.y) {
      r.y = ctx->outputs[i]->geo.y;
    }
    if (ctx->outputs[i]->geo.x + ctx->outputs[i]->geo.w > r.w) {
      r.w = ctx->outputs[i]->geo.x + ctx->outputs[i]->geo.w;
    }
    if (ctx->outputs[i]->geo.y + ctx->outputs[i]->geo.h > r.h) {
      r.h = ctx->outputs[i]->geo.y + ctx->outputs[i]->geo.h;
    }
  }

  r.w -= r.x;
  r.h -= r.y;

  return r;
}

void samure_context_set_pointer_interaction(struct samure_context *ctx,
                                            int enable) {
  for (size_t i = 0; i < ctx->num_outputs; i++) {
    samure_output_set_pointer_interaction(ctx, ctx->outputs[i], enable);
  }
}

void samure_context_set_input_regions(struct samure_context *ctx,
                                      struct samure_rect *r, size_t num_rects) {
  for (size_t j = 0; j < ctx->num_outputs; j++) {
    struct samure_rect *output_rects = NULL;
    size_t num_output_rects = 0;

    for (size_t i = 0; i < num_rects; i++) {
      if (samure_rect_in_output(ctx->outputs[j]->geo, r[i].x, r[i].y, r[i].w,
                                r[i].h)) {
        num_output_rects++;
        output_rects = realloc(output_rects,
                               num_output_rects * sizeof(struct samure_rect));
        if (!output_rects) {
          num_output_rects = 0;
          continue;
        }

        output_rects[num_output_rects - 1].x =
            OUT_X2(ctx->outputs[j]->geo, r[i].x);
        output_rects[num_output_rects - 1].y =
            OUT_Y2(ctx->outputs[j]->geo, r[i].y);
        output_rects[num_output_rects - 1].w = r[i].w;
        output_rects[num_output_rects - 1].h = r[i].h;
      }
    }

    samure_output_set_input_regions(ctx, ctx->outputs[j], output_rects,
                                    num_output_rects);
    free(output_rects);
  }
}

void samure_context_set_keyboard_interaction(struct samure_context *ctx,
                                             int enable) {
  for (size_t i = 0; i < ctx->num_outputs; i++) {
    samure_output_set_keyboard_interaction(ctx->outputs[i], enable);
  }
}

void samure_context_process_events(struct samure_context *ctx) {
  wl_display_roundtrip(ctx->display);

  // Process events
  for (; ctx->event_index < ctx->num_events; ctx->event_index++) {
    struct samure_event *e = &ctx->events[ctx->event_index];

    switch (e->type) {
    case SAMURE_EVENT_LAYER_SURFACE_CONFIGURE:
      e->surface->w = e->width;
      e->surface->h = e->height;

      if (ctx->backend && ctx->backend->on_layer_surface_configure) {
        ctx->backend->on_layer_surface_configure(ctx, e->surface, e->width,
                                                 e->height);
      }

      e->surface->configured = 1;

      break;
    default:
      if (ctx->app.on_event) {
        ctx->app.on_event(ctx, e, ctx->config.user_data);
      }
      break;
    }
  }
  ctx->event_index = 0;
  ctx->num_events = 0;
}

void samure_context_render_layer_surface(struct samure_context *ctx,
                                         struct samure_layer_surface *sfc,
                                         struct samure_rect geo) {
  if (!sfc->configured) {
    return;
  }

  if (!ctx->config.not_request_frame) {
    if (sfc->not_ready) {
      sfc->dirty = 1;
      return;
    }

    samure_layer_surface_request_frame(ctx, sfc, geo);
  }

  const double end_time = samure_get_time();
  sfc->frame_delta_time = end_time - sfc->frame_start_time;
  sfc->frame_start_time = end_time;

  if (ctx->backend && ctx->backend->render_start) {
    ctx->backend->render_start(ctx, sfc);
  }

  if (ctx->app.on_render) {
    ctx->app.on_render(ctx, sfc, geo, ctx->config.user_data);
  }

  if (ctx->backend && ctx->backend->render_end) {
    ctx->backend->render_end(ctx, sfc);
  }

  sfc->dirty = 0;
}

void samure_context_render_output(struct samure_context *ctx,
                                  struct samure_output *output) {
  for (size_t i = 0; i < output->num_sfc; i++) {
    samure_context_render_layer_surface(ctx, output->sfc[i], output->geo);
  }
}

void samure_context_update(struct samure_context *ctx, double delta_time) {
  if (ctx->cursor_engine) {
    samure_cursor_engine_update(ctx->cursor_engine, delta_time);
  }

  if (ctx->app.on_update) {
    ctx->app.on_update(ctx, delta_time, ctx->config.user_data);
  }
}

samure_error
samure_context_create_output_layer_surfaces(struct samure_context *ctx) {
  samure_error error_code = SAMURE_ERROR_NONE;

  for (size_t i = 0; i < ctx->num_outputs; i++) {
    struct samure_output *o = ctx->outputs[i];

    SAMURE_RESULT(layer_surface)
    sfc_rs = samure_create_layer_surface(
        ctx, o, SAMURE_LAYER_OVERLAY, SAMURE_LAYER_SURFACE_ANCHOR_FILL,
        (uint32_t)ctx->config.keyboard_interaction,
        ctx->config.pointer_interaction || ctx->config.touch_interaction, 1);
    if (SAMURE_HAS_ERROR(sfc_rs)) {
      error_code |= SAMURE_ERROR_LAYER_SURFACE_INIT | sfc_rs.error;
      continue;
    }

    samure_output_attach_layer_surface(o, SAMURE_UNWRAP(layer_surface, sfc_rs));
  }

  return error_code;
}

void samure_context_set_pointer_shape(struct samure_context *ctx,
                                      uint32_t shape) {
  if (ctx->cursor_engine) {
    for (size_t i = 0; i < ctx->num_seats; i++) {
      samure_cursor_engine_set_shape(ctx->cursor_engine, ctx->seats[i], shape);
    }
  }
}

void samure_context_set_render_state(struct samure_context *ctx,
                                     enum samure_render_state render_state) {
  if (ctx->render_state == SAMURE_RENDER_STATE_NONE &&
      render_state != SAMURE_RENDER_STATE_NONE) {
    for (size_t i = 0; i < ctx->num_outputs; i++) {
      samure_context_render_output(ctx, ctx->outputs[i]);
    }
  }
  ctx->render_state = render_state;
}
