#include "cairo.h"
#include "../context.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CAI_ADD_ERR_F(format, ...)                                             \
  {                                                                            \
    const size_t error_string_len =                                            \
        c->error_string ? strlen(c->error_string) : 0;                         \
    c->error_string = realloc(c->error_string, error_string_len + 2048);       \
    snprintf(&c->error_string[error_string_len], 2048, "\n" format,            \
             __VA_ARGS__);                                                     \
  }

struct samure_backend_cairo *
samure_init_backend_cairo(struct samure_context *ctx) {
  struct samure_backend_cairo *c = malloc(sizeof(struct samure_backend_cairo));
  memset(c, 0, sizeof(struct samure_backend_cairo));

  c->num_outputs = ctx->num_outputs;
  c->surfaces = malloc(c->num_outputs * sizeof(struct samure_cairo_surface));
  for (size_t i = 0; i < c->num_outputs; i++) {
    c->surfaces[i].buffer = samure_create_shared_buffer(
        ctx->shm, ctx->outputs[i].size.w, ctx->outputs[i].size.h);
    if (c->surfaces[i].buffer.buffer == NULL) {
      CAI_ADD_ERR_F("failed to create shared memory buffer for output %zu", i);
    } else {
      samure_backend_cairo_render_end(&ctx->outputs[i], ctx, &c->base);

      c->surfaces[i].cairo_surface = cairo_image_surface_create_for_data(
          (unsigned char *)c->surfaces[i].buffer.data, CAIRO_FORMAT_ARGB32,
          c->surfaces[i].buffer.width, c->surfaces[i].buffer.height,
          cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32,
                                        c->surfaces[i].buffer.width));
      c->surfaces[i].cairo = cairo_create(c->surfaces[i].cairo_surface);
    }
  }

  c->base.destroy = samure_destroy_backend_cairo;
  c->base.render_end = samure_backend_cairo_render_end;

  return c;
}

void samure_destroy_backend_cairo(struct samure_context *ctx,
                                  struct samure_backend *backend) {
  struct samure_backend_cairo *c = (struct samure_backend_cairo *)backend;

  for (size_t i = 0; i < c->num_outputs; i++) {
    cairo_destroy(c->surfaces[i].cairo);
    cairo_surface_destroy(c->surfaces[i].cairo_surface);
    samure_destroy_shared_buffer(c->surfaces[i].buffer);
  }
  free(c->surfaces);
  free(c->error_string);
  free(c);
}

void samure_backend_cairo_render_end(struct samure_output *output,
                                     struct samure_context *ctx,
                                     struct samure_backend *backend) {
  struct samure_backend_cairo *c = (struct samure_backend_cairo *)backend;
  const uintptr_t i = OUT_IDX();
  wl_surface_attach(ctx->outputs[i].surface, c->surfaces[i].buffer.buffer, 0,
                    0);
  wl_surface_damage(ctx->outputs[i].surface, 0, 0, ctx->outputs[i].size.w,
                    ctx->outputs[i].size.h);
  wl_surface_commit(ctx->outputs[i].surface);
}

struct samure_backend_cairo *
samure_get_backend_cairo(struct samure_context *ctx) {
  return (struct samure_backend_cairo *)ctx->backend;
}