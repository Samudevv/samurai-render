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

#include <cairo/cairo.h>
#include <linux/input-event-codes.h>
#include <stdio.h>
#include <stdlib.h>

#include <samure/backends/cairo.h>
#include <samure/context.h>

struct cairo_data {
  double qx, qy;
  double dx, dy;
};

static void event_callback(struct samure_context *ctx, struct samure_event *e,
                           void *data) {
  switch (e->type) {
  case SAMURE_EVENT_POINTER_BUTTON:
    if (e->button == BTN_LEFT && e->state == WL_POINTER_BUTTON_STATE_RELEASED) {
      ctx->running = 0;
    }
    break;
  }
}

static void render_callback(struct samure_context *ctx,
                            struct samure_layer_surface *sfc,
                            struct samure_rect output_geo, void *data) {
  struct samure_cairo_surface *c =
      (struct samure_cairo_surface *)sfc->backend_data;
  struct cairo_data *d = (struct cairo_data *)data;

  const double qx = RENDER_X(d->qx);
  const double qy = RENDER_Y(d->qy);
  const double s = RENDER_SCALE(100);

  cairo_t *cairo = c->cairo;

  cairo_set_operator(cairo, CAIRO_OPERATOR_SOURCE);
  cairo_set_source_rgba(cairo, 0.0, 0.0, 0.0, 0.0);
  cairo_paint(cairo);

  if (samure_circle_in_output(output_geo, d->qx, d->qy, s)) {
    cairo_set_source_rgba(cairo, 0.0, 1.0, 0.0, 1.0);
    cairo_arc(cairo, qx, qy, s, 0, M_PI * 2.0);
    cairo_fill(cairo);
    cairo_set_source_rgba(cairo, 0.0, 0.0, 1.0, 1.0);
    cairo_arc(cairo, qx, qy, s, 0, M_PI * 2.0);
    cairo_stroke(cairo);
  }

  cairo_select_font_face(cairo, "sans-serif", CAIRO_FONT_SLANT_NORMAL,
                         CAIRO_FONT_WEIGHT_NORMAL);
  cairo_set_font_size(cairo, RENDER_SCALE(25.0));
  cairo_set_source_rgba(cairo, 1.0, 1.0, 0.0, 1.0);

  char buffer[1024];
  snprintf(buffer, 1024, "FPS: %d Hz", (int)(1.0 / sfc->frame_delta_time));

  cairo_text_extents_t text_size;
  cairo_text_extents(cairo, buffer, &text_size);

  cairo_move_to(cairo, 20.0, 20.0 + text_size.height);
  cairo_show_text(cairo, buffer);
}

static void update_callback(struct samure_context *ctx, double delta_time,
                            void *data) {
  struct cairo_data *d = (struct cairo_data *)data;

  d->qx += d->dx * delta_time * 300.0;
  d->qy += d->dy * delta_time * 300.0;

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
}

int main(void) {
  struct cairo_data d = {0};

  struct samure_context_config cfg = samure_create_context_config(
      event_callback, render_callback, update_callback, &d);
  cfg.backend = SAMURE_BACKEND_CAIRO;
  cfg.pointer_interaction = 1;

  SAMURE_RESULT(context) ctx_rs = samure_create_context(&cfg);
  SAMURE_RETURN_AND_PRINT_ON_ERROR(ctx_rs, "failed to create context", 1);

  struct samure_context *ctx = ctx_rs.result;

  const struct samure_rect r = samure_context_get_output_rect(ctx);

  srand(time(NULL));
  d.dx = 1.0;
  d.dy = 1.0;
  d.qx = rand() % (r.w - 200) + 100;
  d.qy = rand() % (r.h - 200) + 100;

  puts("Successfully initialized samurai-render context");

  samure_context_run(ctx);

  samure_destroy_context(ctx);

  puts("Successfully destroyed samurai-render context");

  return EXIT_SUCCESS;
}
