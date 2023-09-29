#include <GL/gl.h>
#include <context.h>
#include <linux/input-event-codes.h>
#include <stdio.h>
#include <stdlib.h>

struct opengl_data {
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
  struct opengl_data *d = (struct opengl_data *)data;

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  if (samure_square_in_output(output, d->qx - 100, d->qy - 100, 200)) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0f, output->geo.w, output->geo.h, 0.0f, 0.0f, 1.0f);
    glDisable(GL_DEPTH_TEST);

    const double qx = OUT_X(d->qx);
    const double qy = OUT_Y(d->qy);

    glBegin(GL_QUADS);
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex2f(qx - 100.0f, qy - 100.0f);
    glVertex2f(qx - 100.0f, qy + 100.0f);
    glVertex2f(qx + 100.0f, qy + 100.0f);
    glVertex2f(qx + 100.0f, qy - 100.0f);
    glEnd();
  }
}

static void update_callback(struct samure_context *ctx, double delta_time,
                            void *data) {
  struct opengl_data *d = (struct opengl_data *)data;

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
  struct opengl_data d = {0};

  struct samure_context_config context_config = samure_create_context_config(
      event_callback, render_callback, update_callback, &d);
  context_config.backend = SAMURE_BACKEND_OPENGL;
  context_config.pointer_interaction = 1;

  struct samure_context *ctx = samure_create_context(&context_config);
  if (ctx->error_string) {
    fprintf(stderr, "%s\n", ctx->error_string);
    return EXIT_FAILURE;
  }

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
