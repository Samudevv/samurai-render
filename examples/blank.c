#include <backends/raw.h>
#include <context.h>
#include <linux/input-event-codes.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void event_callback(struct samure_event *e, struct samure_context *ctx,
                           void *data) {
  int *running = (int *)data;
  switch (e->type) {
  case SAMURE_EVENT_POINTER_BUTTON:
    if (e->button == BTN_LEFT && e->state == WL_POINTER_BUTTON_STATE_RELEASED) {
      *running = 0;
    }
    break;
  }
}

int main(void) {
  struct samure_context *ctx = samure_create_context(SAMURE_NO_CONTEXT_CONFIG);
  if (ctx->error_string) {
    fprintf(stderr, "%s\n", ctx->error_string);
    return EXIT_FAILURE;
  }

  ctx->event_callback = event_callback;

  puts("Successfully initialized samurai-render context");

  struct samure_backend_raw raw = samure_init_backend_raw(ctx);
  if (raw.error_string) {
    fprintf(stderr, "Failed to initialize backend raw: %s\n", raw.error_string);
    samure_destroy_context(ctx);
    return EXIT_FAILURE;
  }

  ctx->backend = &raw.base;

  for (size_t i = 0; i < raw.num_outputs; i++) {
    // Paint a pinkish overlay on the screen
    uint8_t *pixels = (uint8_t *)(raw.surfaces[i].shared_buffer.data);
    for (size_t j = 0; j < ctx->outputs[i].logical_size.width *
                               ctx->outputs[i].logical_size.height * 4;
         j += 4) {
      pixels[j + 0] = 90;
      pixels[j + 1] = 0;
      pixels[j + 2] = 128;
      pixels[j + 3] = 10;
    }
  }

  int running = 1;
  ctx->event_user_data = &running;
  while (running) {
    samure_context_frame_start(ctx);

    samure_backend_raw_frame_end(ctx, raw);

    usleep(16666);
  }

  samure_destroy_backend_raw(raw);
  samure_destroy_context(ctx);

  puts("Successfully destroyed samurai-render context");

  return EXIT_SUCCESS;
}
