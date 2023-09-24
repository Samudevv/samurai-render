#include <backends/raw.h>
#include <context.h>
#include <linux/input-event-codes.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

struct blank_data {
  clock_t start_time;
  double delta_time;

  struct samure_output *current_output;
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
                            struct samure_context *ctx, void *data) {
  struct blank_data *d = (struct blank_data *)data;

  struct samure_backend_raw *r = samure_get_backend_raw(ctx);
  const uintptr_t i = OUTPUT_INDEX(output);

  uint8_t *pixels = r->surfaces[i].shared_buffer.data;
  const int32_t width = r->surfaces[i].shared_buffer.width;
  const int32_t height = r->surfaces[i].shared_buffer.height;

  if (d->current_output == output) {
    for (size_t j = 0; j < width * height * 4; j += 4) {
      pixels[j + 0] = 90;
      pixels[j + 1] = 0;
      pixels[j + 2] = 128;
      pixels[j + 3] = 10;
    }
  } else {
    for (size_t j = 0; j < width * height * 4; j += 4) {
      pixels[j + 0] = 128;
      pixels[j + 1] = 0;
      pixels[j + 2] = 90;
      pixels[j + 3] = 10;
    }
  }
}

static void update_callback(struct samure_context *ctx, void *data) {
  struct blank_data *d = (struct blank_data *)data;

  const clock_t start_time = d->start_time;
  const clock_t end_time = clock();
  d->start_time = clock();
  d->delta_time =
      ((double)end_time - (double)start_time) / (double)CLOCKS_PER_SEC;
  printf("FPS: %.3f \n", 1.0 / d->delta_time);
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

  d.start_time = clock();

  samure_context_run(ctx);

  samure_destroy_context(ctx);

  puts("Successfully destroyed samurai-render context");

  return EXIT_SUCCESS;
}
