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

static void render_callback(struct samure_layer_surface *sfc,
                            struct samure_rect output_geo,
                            struct samure_context *ctx, double delta_time,
                            void *data) {
  struct samure_cairo_surface *c =
      (struct samure_cairo_surface *)sfc->backend_data;
  struct cairo_data *d = (struct cairo_data *)data;

  const double qx = OUT_X(d->qx);
  const double qy = OUT_Y(d->qy);

  cairo_t *cairo = c->cairo;

  cairo_set_operator(cairo, CAIRO_OPERATOR_SOURCE);
  cairo_set_source_rgba(cairo, 0.0, 0.0, 0.0, 0.0);
  cairo_paint(cairo);

  if (samure_circle_in_output(output_geo, d->qx, d->qy, 100)) {
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

  struct samure_context_config context_config = samure_create_context_config(
      event_callback, render_callback, update_callback, &d);
  context_config.backend = SAMURE_BACKEND_CAIRO;
  context_config.pointer_interaction = 1;

  struct samure_context *ctx =
      SAMURE_UNWRAP(context, samure_create_context(&context_config));

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
