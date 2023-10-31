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

#include <linux/input-event-codes.h>
#include <stdio.h>

#include <samure/backends/cairo.h>
#include <samure/context.h>

enum slurpy_state {
  STATE_NONE,
  STATE_CHANGE,
};

struct slurpy_point {
  double x;
  double y;
};

struct slurpy_data {
  struct slurpy_point start;
  struct slurpy_point end;
  enum slurpy_state state;
};

static void on_event(struct samure_context *ctx, struct samure_event *e,
                     void *user_data) {
  struct slurpy_data *d = (struct slurpy_data *)user_data;
  switch (e->type) {
  case SAMURE_EVENT_POINTER_BUTTON:
    if (e->button == BTN_LEFT) {
      if (e->state == WL_POINTER_BUTTON_STATE_PRESSED) {
        d->end.x = d->start.x;
        d->end.y = d->start.y;
        d->state = STATE_CHANGE;
      } else {
        ctx->running = 0;
      }
    }
    break;
  case SAMURE_EVENT_POINTER_MOTION:
    switch (d->state) {
    case STATE_NONE:
      d->start.x = e->x + e->seat->pointer_focus.output->geo.x;
      d->start.y = e->y + e->seat->pointer_focus.output->geo.y;
      break;
    case STATE_CHANGE:
      d->end.x = e->x + e->seat->pointer_focus.output->geo.x;
      d->end.y = e->y + e->seat->pointer_focus.output->geo.y;
      samure_context_set_render_state(ctx, SAMURE_RENDER_STATE_ONCE);
      break;
    }
    break;
  case SAMURE_EVENT_KEYBOARD_KEY:
    if ((e->button == KEY_ESC || e->button == KEY_ENTER) &&
        e->state == WL_KEYBOARD_KEY_STATE_RELEASED) {
      ctx->running = 0;
    }
    break;
  }
}

static void on_render(struct samure_context *ctx,
                      struct samure_layer_surface *sfc,
                      struct samure_rect output_geo, void *user_data) {
  struct slurpy_data *d = (struct slurpy_data *)user_data;
  struct samure_cairo_surface *c =
      (struct samure_cairo_surface *)sfc->backend_data;
  cairo_t *cr = c->cairo;
  cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
  cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, (double)(0x40) / 255.0);
  cairo_paint(cr);
  if (d->state == STATE_CHANGE) {
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.0);
    cairo_rectangle(cr, OUT_X(d->start.x), OUT_Y(d->start.y),
                    d->end.x - d->start.x, d->end.y - d->start.y);
    cairo_fill(cr);
    cairo_rectangle(cr, OUT_X(d->start.x), OUT_Y(d->start.y),
                    d->end.x - d->start.x, d->end.y - d->start.y);
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
    cairo_stroke(cr);
  }
}

int main(void) {
  struct slurpy_data d = {0};

  struct samure_context_config cfg =
      samure_create_context_config(on_event, on_render, NULL, &d);
  cfg.backend = SAMURE_BACKEND_CAIRO;
  cfg.pointer_interaction = 1;
  cfg.keyboard_interaction = 1;

  struct samure_context *ctx =
      SAMURE_UNWRAP(context, samure_create_context(&cfg));

  samure_context_set_pointer_shape(ctx,
                                   WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_CROSSHAIR);

  samure_context_set_render_state(ctx, SAMURE_RENDER_STATE_ONCE);
  samure_context_run(ctx);

  struct slurpy_point start;
  struct slurpy_point end;
  if (d.start.x > d.end.x) {
    start.x = d.end.x;
    end.x = d.start.x;
  } else {
    start.x = d.start.x;
    end.x = d.end.x;
  }
  if (d.start.y > d.end.y) {
    start.y = d.end.y;
    end.y = d.start.y;
  } else {
    start.y = d.start.y;
    end.y = d.end.y;
  }

  const struct slurpy_point dims = {
      .x = end.x - start.x,
      .y = end.y - start.y,
  };
  if (dims.x == 0.0 && dims.y == 0.0) {
    return 1;
  }

  printf("%.0f,%.0f %.0fx%.0f\n", start.x, start.y, dims.x, dims.y);
  return 0;
}
