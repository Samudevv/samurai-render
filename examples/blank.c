#include <backends/raw.h>
#include <context.h>
#include <linux/input-event-codes.h>
#include <stdio.h>
#include <stdlib.h>

#define OLIVEC_IMPLEMENTATION
#include <olive.c>

struct blank_data {
  struct samure_output *current_output;
  Olivec_Canvas *canvas;
  double cx, cy;
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
  case SAMURE_EVENT_POINTER_ENTER: {
    d->current_output = e->output;
  } break;
  case SAMURE_EVENT_POINTER_LEAVE: {
    d->current_output = NULL;
  } break;
  }
}

static void render_callback(struct samure_output *output,
                            struct samure_context *ctx, double delta_time,
                            void *data) {
  struct blank_data *d = (struct blank_data *)data;

  struct samure_backend_raw *r = samure_get_backend_raw(ctx);
  const uintptr_t i = OUTPUT_INDEX(output);

  uint8_t *pixels = r->surfaces[i].shared_buffer.data;
  const int32_t width = r->surfaces[i].shared_buffer.width;
  const int32_t height = r->surfaces[i].shared_buffer.height;

  const int circle_in_output =
      d->cx + 100 > output->logical_position.x &&
      d->cx - 100 < output->logical_position.x + output->logical_size.width;

  if (d->current_output == output) {
    olivec_fill(d->canvas[i], 0x0A5A0080);

    char buffer[1024];
    snprintf(buffer, 1024, "%d", ctx->frame_timer.fps);

    olivec_text(d->canvas[i], buffer, 5, 5, olivec_default_font, 5, 0xFFAAAAAA);
  } else {
    olivec_fill(d->canvas[i], 0x0A80005A);
  }

  if (circle_in_output) {
    const double cx = d->cx - (double)output->logical_position.x;
    const double cy = d->cy - (double)output->logical_position.y;

    olivec_circle(d->canvas[i], cx, cy, 100, 0xFF00FF00);
  }
}

static void update_callback(struct samure_context *ctx, double delta_time,
                            void *data) {
  struct blank_data *d = (struct blank_data *)data;

  d->cx += d->dx * delta_time * 400.0;
  d->cy += d->dy * delta_time * 400.0;

  if (d->cx + 100 > ctx->outputs[0].logical_size.width * 2) {
    d->dx *= -1.0;
  }
  if (d->cx - 100 < 0) {
    d->dx *= -1.0;
  }
  if (d->cy + 100 > ctx->outputs[0].logical_size.height) {
    d->dy *= -1.0;
  }
  if (d->cy - 100 < 0) {
    d->dy *= -1.0;
  }
}

int main(void) {
  struct blank_data d = {0};

  struct samure_context_config context_config = samure_create_context_config(
      event_callback, render_callback, update_callback, &d);

  struct samure_context *ctx = samure_create_context(&context_config);
  if (ctx->error_string) {
    fprintf(stderr, "%s\n", ctx->error_string);
    return EXIT_FAILURE;
  }

  puts("Successfully initialized samurai-render context");

  struct samure_backend_raw *r = samure_get_backend_raw(ctx);
  d.canvas = malloc(r->num_outputs * sizeof(Olivec_Canvas));
  for (size_t i = 0; i < r->num_outputs; i++) {
    d.canvas[i] = olivec_canvas(r->surfaces[i].shared_buffer.data,
                                ctx->outputs[i].logical_size.width,
                                ctx->outputs[i].logical_size.height,
                                ctx->outputs[i].logical_size.width);
  }

  d.dx = 1.0;
  d.dy = 1.0;
  d.cx = (double)ctx->outputs[0].logical_size.width / 2.0;
  d.cy = (double)ctx->outputs[0].logical_size.height / 2.0;

  samure_context_run(ctx);

  samure_destroy_context(ctx);

  puts("Successfully destroyed samurai-render context");

  return EXIT_SUCCESS;
}
