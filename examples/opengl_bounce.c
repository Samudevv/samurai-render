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

  if (samure_circle_in_output(output, d->qx, d->qy, 100)) {
    glClearColor(0.7f, 0.0f, 0.4f, 0.1f);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0f, output->size.w, output->size.h, 0.0f, 0.0f, 1.0f);
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
  } else {
    glClearColor(0.5f, 0.0f, 0.1f, 0.1f);
    glClear(GL_COLOR_BUFFER_BIT);
  }
}

static void update_callback(struct samure_context *ctx, double delta_time,
                            void *data) {
  struct opengl_data *d = (struct opengl_data *)data;

  d->qx += d->dx * delta_time * 300.0;
  d->qy += d->dy * delta_time * 300.0;

  if (d->qx + 100 > ctx->outputs[0].size.w * 2) {
    d->dx *= -1.0;
  }
  if (d->qx - 100 < 0) {
    d->dx *= -1.0;
  }
  if (d->qy + 100 > ctx->outputs[0].size.h) {
    d->dy *= -1.0;
  }
  if (d->qy - 100 < 0) {
    d->dy *= -1.0;
  }
}

int main(void) {
  struct opengl_data d = {0};

  struct samure_context_config context_config = samure_create_context_config(
      event_callback, render_callback, update_callback, &d);
  context_config.backend = SAMURE_BACKEND_OPENGL;

  struct samure_context *ctx = samure_create_context(&context_config);
  if (ctx->error_string) {
    fprintf(stderr, "%s\n", ctx->error_string);
    return EXIT_FAILURE;
  }

  d.dx = 1.0;
  d.dy = 1.0;
  d.qx = ctx->outputs[0].size.w / 2.0;
  d.qy = ctx->outputs[0].size.h / 2.0;

  puts("Successfully initialized samurai-render context");

  samure_context_run(ctx);

  samure_destroy_context(ctx);

  puts("Successfully destroyed samurai-render context");

  return EXIT_SUCCESS;
}
