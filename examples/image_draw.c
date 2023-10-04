#include <cairo/cairo.h>
#include <linux/input-event-codes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <samure/backends/cairo.h>
#include <samure/context.h>

struct image_draw_data {
  double x;
  double y;
  int pressed;
};

static void event_callback(struct samure_context *ctx, struct samure_event *e,
                           void *data) {
  struct image_draw_data *d = (struct image_draw_data *)data;

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

static void render_callback(struct samure_context *ctx,
                            struct samure_layer_surface *sfc,
                            struct samure_rect output_geo, double delta_time,
                            void *data) {
  struct samure_cairo_surface *c =
      (struct samure_cairo_surface *)sfc->backend_data;
  struct image_draw_data *d = (struct image_draw_data *)data;

  cairo_t *cairo = c->cairo;
  cairo_set_operator(cairo, CAIRO_OPERATOR_SOURCE);
  cairo_set_source_rgba(cairo, 1.0, 0.0, 0.0, 1.0);

  if (d->pressed) {
    cairo_arc(cairo, OUT_X(d->x), OUT_Y(d->y), 10.0, 0.0, M_PI * 2.0);
    cairo_fill(cairo);
  }
}

int main(int args, char *argv[]) {
  if (args != 2) {
    fprintf(stderr, "Invalid Arguments!\n Usage: %s [png file name]\n",
            argv[0]);
    return 1;
  }

  const char *background_file_name = argv[1];

  cairo_surface_t *bg_img =
      cairo_image_surface_create_from_png(background_file_name);
  if (!bg_img) {
    fprintf(stderr, "Failed to load background image\n");
    return 1;
  }

  const int bg_img_w = cairo_image_surface_get_width(bg_img);
  const int bg_img_h = cairo_image_surface_get_height(bg_img);
  const cairo_format_t bg_img_format = cairo_image_surface_get_format(bg_img);

  if (bg_img_format != CAIRO_FORMAT_ARGB32 &&
      bg_img_format != CAIRO_FORMAT_RGB24) {
    fprintf(stderr, "Background image is of wrong format: %d\n", bg_img_format);
    return 1;
  }

  struct image_draw_data d = {0};

  struct samure_context_config context_config =
      samure_create_context_config(event_callback, render_callback, NULL, &d);
  context_config.backend = SAMURE_BACKEND_CAIRO;
  context_config.pointer_interaction = 1;
  context_config.keyboard_interaction = 1;
  context_config.not_create_output_layer_surfaces = 1;
  context_config.max_fps = 60;

  struct samure_context *ctx =
      SAMURE_UNWRAP(context, samure_create_context(&context_config));

  puts("Successfully initialized samurai-render context");

  struct samure_layer_surface **bgs = NULL;

  if (bg_img) {
    bgs = malloc(ctx->num_outputs * sizeof(struct samure_layer_surface *));
    for (size_t i = 0; i < ctx->num_outputs; i++) {
      bgs[i] = SAMURE_UNWRAP(layer_surface,
                             samure_create_layer_surface(
                                 ctx, ctx->outputs[i], SAMURE_LAYER_OVERLAY,
                                 SAMURE_LAYER_SURFACE_ANCHOR_FILL, 0, 0, 0));
      bgs[i]->w = ctx->outputs[i]->geo.w;
      bgs[i]->h = ctx->outputs[i]->geo.h;
      SAMURE_ASSERT(samure_backend_cairo_associate_layer_surface(ctx, bgs[i]));

      if (bg_img_w == ctx->outputs[i]->geo.w &&
          bg_img_h == ctx->outputs[i]->geo.h) {
        struct samure_cairo_surface *c = samure_get_cairo_surface(bgs[i]);
        memcpy(c->buffer->data, cairo_image_surface_get_data(bg_img),
               ctx->outputs[i]->geo.w * ctx->outputs[i]->geo.h * 4);
        ctx->backend->render_end(ctx, bgs[i]);
      }
    }
    cairo_surface_destroy(bg_img);
  }

  samure_context_create_output_layer_surfaces(ctx);

  ctx->render_state = SAMURE_RENDER_STATE_ONCE;
  samure_context_run(ctx);

  if (bgs) {
    for (size_t i = 0; i < ctx->num_outputs; i++) {
      samure_destroy_layer_surface(ctx, bgs[i]);
    }
    free(bgs);
  }

  samure_destroy_context(ctx);

  puts("Successfully destroyed samurai-render context");

  return EXIT_SUCCESS;
}
