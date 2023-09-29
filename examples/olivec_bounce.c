#include <linux/input-event-codes.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define OLIVEC_IMPLEMENTATION
#include <olive.c>

#include <samure/backends/raw.h>
#include <samure/context.h>

struct blank_data {
  Olivec_Canvas *canvas;
  double qx, qy;
  double dx, dy;
};

static void event_callback(struct samure_event *e, struct samure_context *ctx,
                           void *data) {
  struct blank_data *d = (struct blank_data *)data;

  switch (e->type) {
  case SAMURE_EVENT_POINTER_BUTTON:
    if (e->button == BTN_LEFT && e->state == WL_POINTER_BUTTON_STATE_RELEASED) {
      ctx->running = 0;
    }
    break;
  case SAMURE_EVENT_KEYBOARD_KEY:
    if (e->key == KEY_ESC && e->state == WL_KEYBOARD_KEY_STATE_RELEASED) {
      ctx->running = 0;
    }
    break;
  case SAMURE_EVENT_KEYBOARD_ENTER: {
    printf("keyboard_enter: output=%lu\n", OUT_IDX2(e->output));
  } break;
  case SAMURE_EVENT_KEYBOARD_LEAVE: {
    printf("keyboard_leave: output=%lu\n", OUT_IDX2(e->output));
  } break;
  }
}

static void render_callback(struct samure_output *output,
                            struct samure_context *ctx, double delta_time,
                            void *data) {
  struct blank_data *d = (struct blank_data *)data;
  const uintptr_t i = OUT_IDX();

  olivec_fill(d->canvas[i], 0x00000000);
  if (samure_circle_in_output(output, d->qx, d->qy, 100)) {
    olivec_circle(d->canvas[i], OUT_X(d->qx), OUT_Y(d->qy), 100, 0xFF00FF00);

    char buffer[1024];
    snprintf(buffer, 1024, "%d", ctx->frame_timer.fps);

    olivec_text(d->canvas[i], buffer, 5, 5, olivec_default_font, 5, 0xFFAAAAAA);
  }
}

static void render_callback_clear_outputs_on_exit(struct samure_output *output,
                                                  struct samure_context *ctx,
                                                  double delta_time,
                                                  void *data) {
  struct blank_data *d = (struct blank_data *)data;
  const uintptr_t i = OUT_IDX();
  olivec_fill(d->canvas[i], 0x00000000);
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
}

int main(void) {
  struct blank_data d = {0};

  struct samure_context_config context_config = samure_create_context_config(
      event_callback, render_callback, update_callback, &d);
  context_config.pointer_interaction = 1;

  struct samure_context *ctx = samure_create_context(&context_config);
  if (ctx->error_string) {
    fprintf(stderr, "%s\n", ctx->error_string);
    return EXIT_FAILURE;
  }

  puts("Successfully initialized samurai-render context");

  struct samure_backend_raw *r = samure_get_backend_raw(ctx);
  d.canvas = malloc(r->num_outputs * sizeof(Olivec_Canvas));
  for (size_t i = 0; i < r->num_outputs; i++) {
    d.canvas[i] =
        olivec_canvas(r->surfaces[i].shared_buffer.data, ctx->outputs[i].geo.w,
                      ctx->outputs[i].geo.h, ctx->outputs[i].geo.w);
  }

  const struct samure_rect rt = samure_context_get_output_rect(ctx);

  srand(time(NULL));
  d.dx = 1.0;
  d.dy = 1.0;
  d.qx = rand() % (rt.w - 200) + 100;
  d.qy = rand() % (rt.h - 200) + 100;

  samure_context_run(ctx);

  // Clear screen on exit (to avoid fading animation)
  for (size_t i = 0; i < ctx->num_outputs; i++) {
    samure_context_render_output(ctx, &ctx->outputs[i],
                                 render_callback_clear_outputs_on_exit, 0.0);
  }

  samure_destroy_context(ctx);

  puts("Successfully destroyed samurai-render context");

  return EXIT_SUCCESS;
}
