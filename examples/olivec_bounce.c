#include <linux/input-event-codes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define OLIVEC_IMPLEMENTATION
#include <olive.c>

#include <samure/backends/raw.h>
#include <samure/context.h>
#include <samure/layer_surface.h>

struct olivec_surface {
  struct samure_shared_buffer *buffer;
  Olivec_Canvas canvas;
};

static void olivec_destroy_backend(struct samure_context *ctx,
                                   struct samure_backend *raw) {
  free(raw);
}

static void olivec_backend_render_end(
    struct samure_output *output, struct samure_layer_surface *layer_surface,
    struct samure_context *ctx, struct samure_backend *raw) {
  struct olivec_surface *os =
      (struct olivec_surface *)layer_surface->backend_data;
  samure_layer_surface_draw_buffer(layer_surface, os->buffer);
}

static samure_error olivec_backend_associate_layer_surface(
    struct samure_context *ctx, struct samure_backend *raw,
    struct samure_output *output, struct samure_layer_surface *sfc) {
  struct olivec_surface *os = malloc(sizeof(struct olivec_surface));

  os->buffer = SAMURE_UNWRAP(
      shared_buffer, samure_create_shared_buffer(ctx->shm, SAMURE_BUFFER_FORMAT,
                                                 output->geo.w, output->geo.h));
  os->canvas = olivec_canvas(os->buffer->data, output->geo.w, output->geo.h,
                             output->geo.w);

  sfc->backend_data = os;
  return SAMURE_ERROR_NONE;
}

static void olivec_backend_unassociate_layer_surface(
    struct samure_context *ctx, struct samure_backend *raw,
    struct samure_output *output, struct samure_layer_surface *sfc) {
  if (!sfc->backend_data) {
    return;
  }

  struct olivec_surface *os = (struct olivec_surface *)sfc->backend_data;

  samure_destroy_shared_buffer(os->buffer);
  free(os);
  sfc->backend_data = NULL;
}

struct blank_data {
  Olivec_Canvas *canvas;
  double qx, qy;
  double dx, dy;
  uint32_t current_cursor;
  double elapsed_time;
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
    printf("keyboard_enter: output=%s\n", e->output->name);
  } break;
  case SAMURE_EVENT_KEYBOARD_LEAVE: {
    printf("keyboard_leave: output=%s\n", e->output->name);
  } break;
  }
}

static void render_callback(struct samure_output *output,
                            struct samure_layer_surface *sfc,
                            struct samure_context *ctx, double delta_time,
                            void *data) {
  struct olivec_surface *os = (struct olivec_surface *)sfc->backend_data;
  struct blank_data *d = (struct blank_data *)data;

  olivec_fill(os->canvas, 0x00000000);
  if (samure_circle_in_output(output, d->qx, d->qy, 100)) {
    olivec_circle(os->canvas, OUT_X(d->qx), OUT_Y(d->qy), 100, 0xFF00FF00);

    char buffer[1024];
    snprintf(buffer, 1024, "%d", ctx->frame_timer.fps);

    olivec_text(os->canvas, buffer, 5, 5, olivec_default_font, 5, 0xFFAAAAAA);
  }
}

static void render_callback_clear_outputs_on_exit(
    struct samure_output *output, struct samure_layer_surface *sfc,
    struct samure_context *ctx, double delta_time, void *data) {
  struct olivec_surface *os = (struct olivec_surface *)sfc->backend_data;
  struct blank_data *d = (struct blank_data *)data;
  olivec_fill(os->canvas, 0x00000000);
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

  d->elapsed_time += delta_time;
  if (d->elapsed_time > 0.5) {
    d->elapsed_time = 0.0;
    d->current_cursor++;
    if (d->current_cursor == WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_ZOOM_OUT + 1) {
      d->current_cursor = WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_DEFAULT;
    }

    for (size_t i = 0; i < ctx->num_seats; i++) {
      samure_seat_set_pointer_shape(ctx->seats[i], d->current_cursor);
    }
  }
}

int main(void) {
  struct blank_data d = {0};

  struct samure_context_config context_config = samure_create_context_config(
      event_callback, render_callback, update_callback, &d);
  context_config.backend = SAMURE_BACKEND_NONE;
  context_config.pointer_interaction = 1;
  context_config.not_create_output_layer_surfaces = 1;

  SAMURE_RESULT(context) ctx_rs = samure_create_context(&context_config);
  SAMURE_RETURN_AND_PRINT_ON_ERROR(ctx_rs, "Failed to create context",
                                   EXIT_FAILURE);
  struct samure_context *ctx = SAMURE_UNWRAP(context, ctx_rs);

  puts("Successfully initialized samurai-render context");

  struct samure_backend *bak = malloc(sizeof(struct samure_backend));
  assert(bak != NULL);
  memset(bak, 0, sizeof(struct samure_backend));
  bak->render_end = olivec_backend_render_end;
  bak->destroy = olivec_destroy_backend;
  bak->associate_layer_surface = olivec_backend_associate_layer_surface;
  bak->unassociate_layer_surface = olivec_backend_unassociate_layer_surface;

  ctx->backend = bak;

  samure_context_create_output_layer_surfaces(ctx);

  const struct samure_rect rt = samure_context_get_output_rect(ctx);

  srand(time(NULL));
  d.dx = 1.0;
  d.dy = 1.0;
  d.qx = rand() % (rt.w - 200) + 100;
  d.qy = rand() % (rt.h - 200) + 100;

  samure_context_run(ctx);

  // Clear screen on exit (to avoid fading animation)
  for (size_t i = 0; i < ctx->num_outputs; i++) {
    samure_context_render_output(ctx, ctx->outputs[i],
                                 render_callback_clear_outputs_on_exit, 0.0);
  }

  samure_destroy_context(ctx);

  puts("Successfully destroyed samurai-render context");

  return EXIT_SUCCESS;
}
