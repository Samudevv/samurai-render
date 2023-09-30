#include <cairo/cairo.h>
#include <linux/input-event-codes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <samure/backends/cairo.h>
#include <samure/context.h>

struct screenshot_draw_data {
  double x;
  double y;
  int pressed;
};

static void event_callback(struct samure_event *e, struct samure_context *ctx,
                           void *data) {
  struct screenshot_draw_data *d = (struct screenshot_draw_data *)data;

  switch (e->type) {
  case SAMURE_EVENT_POINTER_BUTTON:
    if (e->button == BTN_LEFT) {
      d->pressed = e->state == WL_POINTER_BUTTON_STATE_PRESSED;
    }
    break;
  case SAMURE_EVENT_POINTER_MOTION:
    d->x = e->x + e->seat->pointer_focus.output->geo.x;
    d->y = e->y + e->seat->pointer_focus.output->geo.y;
    if (d->pressed) {
      ctx->render_state = SAMURE_RENDER_STATE_ONCE;
    }
    break;
  case SAMURE_EVENT_KEYBOARD_KEY:
    if (e->key == KEY_ESC && e->state == WL_KEYBOARD_KEY_STATE_RELEASED) {
      ctx->running = 0;
    }
    break;
  }
}

static void render_callback(struct samure_output *output,
                            struct samure_layer_surface *sfc,
                            struct samure_context *ctx, double delta_time,
                            void *data) {
  struct samure_cairo_surface *c =
      (struct samure_cairo_surface *)sfc->backend_data;
  struct screenshot_draw_data *d = (struct screenshot_draw_data *)data;

  cairo_t *cairo = c->cairo;
  cairo_set_operator(cairo, CAIRO_OPERATOR_SOURCE);
  cairo_set_source_rgba(cairo, 1.0, 0.0, 0.0, 1.0);

  if (d->pressed) {
    cairo_arc(cairo, OUT_X(d->x), OUT_Y(d->y), 10.0, 0.0, M_PI * 2.0);
    cairo_fill(cairo);
  }
}

int main(int args, char *argv[]) {
  struct screenshot_draw_data d = {0};

  struct samure_context_config context_config =
      samure_create_context_config(event_callback, render_callback, NULL, &d);
  context_config.backend = SAMURE_BACKEND_CAIRO;
  context_config.pointer_interaction = 1;
  context_config.keyboard_interaction = 1;
  context_config.not_create_output_layer_surfaces = 1;
  context_config.max_fps = 60;

  struct samure_context *ctx = samure_create_context(&context_config);
  if (ctx->error_string) {
    fprintf(stderr, "%s\n", ctx->error_string);
    return EXIT_FAILURE;
  }

  puts("Successfully initialized samurai-render context");

  struct samure_layer_surface **bgs = NULL;

  bgs = malloc(ctx->num_outputs * sizeof(struct samure_layer_surface *));
  for (size_t i = 0; i < ctx->num_outputs; i++) {
    bgs[i] =
        samure_create_layer_surface(ctx, &ctx->outputs[i], SAMURE_LAYER_OVERLAY,
                                    SAMURE_LAYER_SURFACE_ANCHOR_FILL, 0, 0, 0);

    struct samure_shared_buffer screenshot =
        samure_output_screenshot(ctx, &ctx->outputs[i]);
    if (screenshot.buffer) {
      samure_layer_surface_draw_buffer(bgs[i], screenshot);
      samure_destroy_shared_buffer(screenshot);
    }
  }

  samure_context_create_output_layer_surfaces(ctx);

  ctx->render_state = SAMURE_RENDER_STATE_ONCE;
  samure_context_run(ctx);

  if (bgs) {
    for (size_t i = 0; i < ctx->num_outputs; i++) {
      samure_destroy_layer_surface(ctx, &ctx->outputs[i], bgs[i]);
    }
    free(bgs);
  }

  samure_destroy_context(ctx);

  puts("Successfully destroyed samurai-render context");

  return EXIT_SUCCESS;
}
