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

#include <linux/input-event-codes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define OLIVEC_IMPLEMENTATION
#include <olive.c>

#include <samure/backends/raw.h>
#include <samure/context.h>

struct olivec_surface {
  struct samure_shared_buffer *buffer;
  Olivec_Canvas canvas;
};

static void olivec_destroy_backend(struct samure_context *ctx) {
  free(ctx->backend);
}

static void
olivec_backend_render_end(struct samure_context *ctx,
                          struct samure_layer_surface *layer_surface) {
  struct olivec_surface *os =
      (struct olivec_surface *)layer_surface->backend_data;
  samure_layer_surface_draw_buffer(layer_surface, os->buffer);
}

static samure_error
olivec_backend_associate_layer_surface(struct samure_context *ctx,
                                       struct samure_layer_surface *sfc) {
  struct olivec_surface *os = malloc(sizeof(struct olivec_surface));

  os->buffer = SAMURE_UNWRAP(
      shared_buffer, samure_create_shared_buffer(ctx->shm, SAMURE_BUFFER_FORMAT,
                                                 sfc->w == 0 ? 1 : sfc->w,
                                                 sfc->h == 0 ? 1 : sfc->h));
  if (sfc->w != 0 && sfc->h != 0) {
    os->canvas = olivec_canvas(os->buffer->data, sfc->w, sfc->h, sfc->w);
  }

  sfc->backend_data = os;
  return SAMURE_ERROR_NONE;
}

static void olivec_backend_on_layer_surface_configure(
    struct samure_context *ctx, struct samure_layer_surface *layer_surface,
    int32_t width, int32_t height) {
  if (!layer_surface->backend_data) {
    return;
  }

  struct olivec_surface *os =
      (struct olivec_surface *)layer_surface->backend_data;

  if (os->buffer->width == width && os->buffer->height == height) {
    return;
  }

  if (os->buffer) {
    samure_destroy_shared_buffer(os->buffer);
  }

  SAMURE_RESULT(shared_buffer)
  b_rs = samure_create_shared_buffer(ctx->shm, SAMURE_BUFFER_FORMAT, width,
                                     height);
  os->buffer = SAMURE_UNWRAP(shared_buffer, b_rs);
  os->canvas = olivec_canvas(os->buffer->data, width, height, width);
}

static void
olivec_backend_unassociate_layer_surface(struct samure_context *ctx,
                                         struct samure_layer_surface *sfc) {
  if (!sfc->backend_data) {
    return;
  }

  struct olivec_surface *os = (struct olivec_surface *)sfc->backend_data;

  samure_destroy_shared_buffer(os->buffer);
  free(os);
  sfc->backend_data = NULL;
}

struct blank_data {
  double qx, qy;
  double dx, dy;
  uint32_t current_cursor;
  double elapsed_time;
};

static void event_callback(struct samure_context *ctx, struct samure_event *e,
                           void *data) {
  struct blank_data *d = (struct blank_data *)data;

  switch (e->type) {
  case SAMURE_EVENT_POINTER_BUTTON:
    if (e->button == BTN_LEFT && e->state == WL_POINTER_BUTTON_STATE_RELEASED) {
      ctx->running = 0;
    }
    break;
  case SAMURE_EVENT_KEYBOARD_KEY:
    if (e->button == KEY_ESC && e->state == WL_KEYBOARD_KEY_STATE_RELEASED) {
      ctx->running = 0;
    }
    break;
  case SAMURE_EVENT_KEYBOARD_ENTER: {
    printf("keyboard_enter: output=%s\n", e->output->name);
  } break;
  case SAMURE_EVENT_KEYBOARD_LEAVE: {
    printf("keyboard_leave: output=%s\n", e->output->name);
  } break;
  }
}

static void render_callback(struct samure_context *ctx,
                            struct samure_layer_surface *sfc,
                            struct samure_rect output_geo, void *data) {
  struct olivec_surface *os = (struct olivec_surface *)sfc->backend_data;
  struct blank_data *d = (struct blank_data *)data;

  olivec_fill(os->canvas, 0x00000000);
  if (samure_circle_in_output(output_geo, d->qx, d->qy, 100)) {
    olivec_circle(os->canvas, RENDER_X(d->qx), RENDER_Y(d->qy), 100,
                  0xFF00FF00);

    char buffer[1024];
    snprintf(buffer, 1024, "%.3f", 1.0 / sfc->frame_delta_time);

    olivec_text(os->canvas, buffer, 5, 5, olivec_default_font, 5, 0xFFAAAAAA);
  }
}

static void update_callback(struct samure_context *ctx, double delta_time,
                            void *data) {
  struct blank_data *d = (struct blank_data *)data;

  d->qx += d->dx * delta_time * 400.0;
  d->qy += d->dy * delta_time * 400.0;

  const struct samure_rect r = samure_context_get_output_rect(ctx);

  if (d->qx + 100 > r.w) {
    d->dx *= -1.0;
  }
  if (d->qx - 100 < r.x) {
    d->dx *= -1.0;
  }
  if (d->qy + 100 > r.h) {
    d->dy *= -1.0;
  }
  if (d->qy - 100 < r.y) {
    d->dy *= -1.0;
  }

  d->elapsed_time += delta_time;
  if (d->elapsed_time > 0.5) {
    d->elapsed_time = 0.0;
    d->current_cursor++;
    if (d->current_cursor == WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_ZOOM_OUT + 1) {
      d->current_cursor = WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_DEFAULT;
    }

    samure_context_set_pointer_shape(ctx, d->current_cursor);
  }
}

int main(void) {
  struct blank_data d = {0};

  struct samure_backend *olivec_backend = SAMURE_UNWRAP(
      backend,
      samure_create_backend(olivec_backend_on_layer_surface_configure, NULL,
                            olivec_backend_render_end, olivec_destroy_backend,
                            olivec_backend_associate_layer_surface,
                            olivec_backend_unassociate_layer_surface));

  struct samure_context_config cfg = samure_create_context_config(
      event_callback, render_callback, update_callback, &d);
  cfg.backend = SAMURE_BACKEND_NONE;
  cfg.pointer_interaction = 1;

  SAMURE_RESULT(context)
  ctx_rs = samure_create_context_with_backend(&cfg, olivec_backend);
  SAMURE_RETURN_AND_PRINT_ON_ERROR(ctx_rs, "Failed to create context",
                                   EXIT_FAILURE);
  struct samure_context *ctx = SAMURE_UNWRAP(context, ctx_rs);

  puts("Successfully initialized samurai-render context");

  const struct samure_rect rt = samure_context_get_output_rect(ctx);

  srand(time(NULL));
  d.dx = 1.0;
  d.dy = 1.0;
  d.qx = rand() % (rt.w - 200) + 100;
  d.qy = rand() % (rt.h - 200) + 100;

  samure_context_run(ctx);

  samure_destroy_context(ctx);

  puts("Successfully destroyed samurai-render context");

  return EXIT_SUCCESS;
}
