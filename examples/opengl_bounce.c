/***********************************************************************************
 *                         This file is part of samurai-render
 *                    https://github.com/Samudevv/samurai-render
 ***********************************************************************************
 * Copyright (c) 2023 Jonas Pucher
 *
 * This software is provided ‘as-is’, without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 ************************************************************************************/

#include <GL/gl.h>
#include <linux/input-event-codes.h>
#include <stdio.h>
#include <stdlib.h>

#include <samure/backends/opengl.h>
#include <samure/context.h>

struct opengl_data {
  double qx, qy;
  double dx, dy;
};

static void event_callback(struct samure_context *ctx, struct samure_event *e,
                           void *data) {
  switch (e->type) {
  case SAMURE_EVENT_POINTER_BUTTON:
    if (e->button == BTN_LEFT && e->state == WL_POINTER_BUTTON_STATE_RELEASED) {
      ctx->running = 0;
    }
    break;
  }
}

static void render_callback(struct samure_context *ctx,
                            struct samure_layer_surface *sfc,
                            struct samure_rect output_geo, void *data) {
  struct opengl_data *d = (struct opengl_data *)data;

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  if (samure_square_in_output(output_geo, d->qx - 100, d->qy - 100, 200)) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0f, RENDER_SCALE(sfc->w), RENDER_SCALE(sfc->h), 0.0f, 0.0f, 1.0f);
    glDisable(GL_DEPTH_TEST);

    const double qx = RENDER_X(d->qx);
    const double qy = RENDER_Y(d->qy);
    const double s = RENDER_SCALE(100.0);

    glBegin(GL_QUADS);
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex2f(qx - s, qy - s);
    glVertex2f(qx - s, qy + s);
    glVertex2f(qx + s, qy + s);
    glVertex2f(qx + s, qy - s);
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

  struct samure_context_config cfg = samure_create_context_config(
      event_callback, render_callback, update_callback, &d);
  cfg.backend = SAMURE_BACKEND_OPENGL;
  cfg.pointer_interaction = 1;
  cfg.gl = samure_default_opengl_config();
  cfg.gl->major_version = 1;
  cfg.gl->minor_version = 0;

  struct samure_context *ctx =
      SAMURE_UNWRAP(context, samure_create_context(&cfg));

  samure_backend_opengl_make_context_current(
      (struct samure_backend_opengl *)ctx->backend, ctx->outputs[0]->sfc[0]);

  printf("OpenGL: %s %s\n", glGetString(GL_VENDOR), glGetString(GL_VERSION));

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
