#include "raw.h"
#include "../context.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RAW_ADD_ERR_F(format, ...)                                             \
  {                                                                            \
    const size_t error_string_len =                                            \
        r->error_string ? strlen(r->error_string) : 0;                         \
    r->error_string = realloc(r->error_string, error_string_len + 2048);       \
    snprintf(&r->error_string[error_string_len], 2048, "\n" format,            \
             __VA_ARGS__);                                                     \
  }

struct samure_backend_raw *samure_init_backend_raw(struct samure_context *ctx) {
  struct samure_backend_raw *r = malloc(sizeof(struct samure_backend_raw));
  memset(r, 0, sizeof(*r));

  r->num_outputs = ctx->num_outputs;
  r->surfaces = malloc(r->num_outputs * sizeof(struct samure_raw_surface));

  for (size_t i = 0; i < r->num_outputs; i++) {
    r->surfaces[i].shared_buffer = samure_create_shared_buffer(
        ctx->shm, ctx->outputs[i].logical_size.width,
        ctx->outputs[i].logical_size.height);
    if (r->surfaces[i].shared_buffer.buffer == NULL) {
      RAW_ADD_ERR_F("failed to create shared memory buffer for surface %zu", i);
    } else {
      samure_backend_raw_render_end(&ctx->outputs[i], ctx, &r->base);
    }
  }

  r->base.render_end = samure_backend_raw_render_end;
  r->base.destroy = samure_destroy_backend_raw;

  return r;
}

void samure_destroy_backend_raw(struct samure_context *ctx,
                                struct samure_backend *b) {
  struct samure_backend_raw *r = (struct samure_backend_raw *)b;

  for (size_t i = 0; i < r->num_outputs; i++) {
    samure_destroy_shared_buffer(r->surfaces[i].shared_buffer);
  }
  free(r->surfaces);
  free(r->error_string);
  free(r);
}

void samure_backend_raw_render_end(struct samure_output *output,
                                   struct samure_context *ctx,
                                   struct samure_backend *b) {
  struct samure_backend_raw *r = (struct samure_backend_raw *)b;
  const uintptr_t i = OUTPUT_INDEX(output);
  wl_surface_attach(ctx->outputs[i].surface,
                    r->surfaces[i].shared_buffer.buffer, 0, 0);
  wl_surface_damage(ctx->outputs[i].surface, 0, 0,
                    ctx->outputs[i].logical_size.width,
                    ctx->outputs[i].logical_size.height);
  wl_surface_commit(ctx->outputs[i].surface);
}

struct samure_backend_raw *samure_get_backend_raw(struct samure_context *ctx) {
  return (struct samure_backend_raw *)ctx->backend;
}