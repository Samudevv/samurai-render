#include <cairo/cairo.h>
#include <context.h>
#include <linux/input-event-codes.h>
#include <stdio.h>
#include <stdlib.h>

#include <backends/cairo.h>

struct cairo_data {
  double qx, qy;
  double dx, dy;
};

static void event_callback(struct samure_event *e, struct samure_context *ctx,
                           void *data) {
  switch (e->type) {
  case SAMURE_EVENT_POINTER_BUTTON:
    if (e->button == BTN_LEFT && e->state == WL_POINTER_BUTTON_STATE_RELEASED) {
      ctx->running = 0;
    }
    break;
  }
}

static void render_callback(struct samure_output *output,
                            struct samure_context *ctx, double delta_time,
                            void *data) {
  struct samure_backend_cairo *c = samure_get_backend_cairo(ctx);
  struct cairo_data *d = (struct cairo_data *)data;

  const double qx = OUT_X(d->qx);
  const double qy = OUT_Y(d->qy);

  const uintptr_t i = OUT_IDX();

  cairo_t *cairo = c->surfaces[i].cairo;

  cairo_set_operator(cairo, CAIRO_OPERATOR_SOURCE);
  cairo_set_source_rgba(cairo, 0.7, 0.0, 0.4, 0.3);
  cairo_paint(cairo);

  if (samure_circle_in_output(output, d->qx, d->qy, 100)) {
    cairo_set_source_rgba(cairo, 0.0, 1.0, 0.0, 1.0);
    cairo_arc(cairo, qx, qy, 100, 0, M_PI * 2.0);
    cairo_fill(cairo);
    cairo_set_source_rgba(cairo, 0.0, 0.0, 1.0, 1.0);
    cairo_arc(cairo, qx, qy, 100, 0, M_PI * 2.0);
    cairo_stroke(cairo);
  }

  cairo_select_font_face(cairo, "sans-serif", CAIRO_FONT_SLANT_NORMAL,
                         CAIRO_FONT_WEIGHT_NORMAL);
  cairo_set_font_size(cairo, 25.0);
  cairo_set_source_rgba(cairo, 1.0, 1.0, 0.0, 1.0);

  char buffer[1024];
  snprintf(buffer, 1024, "FPS: %d", ctx->frame_timer.fps);

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

  if (d->qx + 100 > ctx->outputs[0].size.w * 2) {
    d->dx *= -1.0;
  }
  if (d->qx - 100 < 0) {
    d->dx *= -1.0;
  }
  if (d->qy + 100 > ctx->outputs[0].size.h) {
    d->dy *= -1.0;
  }
  if (d->qy - 100 < 0) {
    d->dy *= -1.0;
  }
}

int main(void) {
  struct cairo_data d = {0};

  struct samure_context_config context_config = samure_create_context_config(
      event_callback, render_callback, update_callback, &d);
  context_config.backend = SAMURE_BACKEND_CAIRO;

  struct samure_context *ctx = samure_create_context(&context_config);
  if (ctx->error_string) {
    fprintf(stderr, "%s\n", ctx->error_string);
    return EXIT_FAILURE;
  }

  d.dx = 1.0;
  d.dy = 1.0;
  d.qx = ctx->outputs[0].size.w / 2.0;
  d.qy = ctx->outputs[0].size.h / 2.0;

  puts("Successfully initialized samurai-render context");

  samure_context_run(ctx);

  samure_destroy_context(ctx);

  puts("Successfully destroyed samurai-render context");

  return EXIT_SUCCESS;
}
