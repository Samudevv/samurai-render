#include <backends/raw.h>
#include <context.h>
#include <linux/input-event-codes.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct blank_data {
  int running;
};

static void event_callback(struct samure_event *e, struct samure_context *ctx,
                           void *data) {
  struct samure_backend_raw *r = (struct samure_backend_raw *)ctx->backend;
  struct blank_data *d = (struct blank_data *)data;

  switch (e->type) {
  case SAMURE_EVENT_POINTER_BUTTON:
    if (e->button == BTN_LEFT && e->state == WL_POINTER_BUTTON_STATE_RELEASED) {
      d->running = 0;
    }
    break;
  case SAMURE_EVENT_POINTER_ENTER: {
    const uintptr_t i = OUTPUT_INDEX(e->output);
    uint8_t *pixels = (uint8_t *)r->surfaces[i].shared_buffer.data;
    for (size_t j = 0; j < ctx->outputs[i].logical_size.width *
                               ctx->outputs[i].logical_size.height * 4;
         j += 4) {
      pixels[j + 0] = 90;
      pixels[j + 1] = 0;
      pixels[j + 2] = 128;
      pixels[j + 3] = 10;
    }
  } break;
  case SAMURE_EVENT_POINTER_LEAVE: {
    const uintptr_t i = OUTPUT_INDEX(e->output);
    uint8_t *pixels = (uint8_t *)r->surfaces[i].shared_buffer.data;
    for (size_t j = 0; j < ctx->outputs[i].logical_size.width *
                               ctx->outputs[i].logical_size.height * 4;
         j += 4) {
      pixels[j + 0] = 128;
      pixels[j + 1] = 0;
      pixels[j + 2] = 90;
      pixels[j + 3] = 10;
    }
  } break;
  }
}

int main(void) {
  struct blank_data d = {.running = 1};

  struct samure_context_config context_config = samure_default_context_config();
  context_config.event_callback = event_callback;
  context_config.user_data = &d;

  struct samure_context *ctx = samure_create_context(&context_config);
  if (ctx->error_string) {
    fprintf(stderr, "%s\n", ctx->error_string);
    return EXIT_FAILURE;
  }

  puts("Successfully initialized samurai-render context");

  struct samure_backend_raw *raw = samure_get_backend_raw(ctx);

  for (size_t i = 0; i < raw->num_outputs; i++) {
    // Paint a pinkish overlay on the screen
    uint8_t *pixels = (uint8_t *)(raw->surfaces[i].shared_buffer.data);
    for (size_t j = 0; j < ctx->outputs[i].logical_size.width *
                               ctx->outputs[i].logical_size.height * 4;
         j += 4) {
      pixels[j + 0] = 128;
      pixels[j + 1] = 0;
      pixels[j + 2] = 90;
      pixels[j + 3] = 10;
    }
  }

  while (d.running) {
    samure_context_frame_start(ctx);

    samure_context_frame_end(ctx);

    usleep(16666);
  }

  samure_destroy_context(ctx);

  puts("Successfully destroyed samurai-render context");

  return EXIT_SUCCESS;
}
