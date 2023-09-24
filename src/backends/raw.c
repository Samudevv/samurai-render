#include "raw.h"
#include "../context.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RAW_ADD_ERR_F(format, ...)                                             \
  {                                                                            \
    const size_t error_string_len =                                            \
        r.error_string ? strlen(r.error_string) : 0;                           \
    r.error_string = realloc(r.error_string, error_string_len + 2048);         \
    snprintf(&r.error_string[error_string_len], 2048, "\n" format,             \
             __VA_ARGS__);                                                     \
  }

struct samure_backend_raw samure_init_backend_raw(struct samure_context *ctx) {
  struct samure_backend_raw r = {0};

  r.num_outputs = ctx->num_outputs;
  r.surfaces = malloc(r.num_outputs * sizeof(struct samure_raw_surface));

  for (size_t i = 0; i < r.num_outputs; i++) {
    r.surfaces[i].shared_buffer = samure_create_shared_buffer(
        ctx->shm, ctx->outputs[i].logical_size.width,
        ctx->outputs[i].logical_size.height);
    if (r.surfaces[i].shared_buffer.buffer == NULL) {
      RAW_ADD_ERR_F("failed to create shared memory buffer for surface %zu", i);
    }
  }

  r.base.on_layer_surface_configure =
      samure_backend_raw_on_layer_surface_configure;

  return r;
}

void samure_destroy_backend_raw(struct samure_backend_raw raw) {
  for (size_t i = 0; i < raw.num_outputs; i++) {
    samure_destroy_shared_buffer(raw.surfaces[i].shared_buffer);
  }
  free(raw.surfaces);
  free(raw.error_string);
}

void samure_backend_raw_frame_end(struct samure_context *ctx,
                                  struct samure_backend_raw r) {
  for (size_t i = 0; i < r.num_outputs; i++) {
    wl_surface_attach(ctx->outputs[i].surface,
                      r.surfaces[i].shared_buffer.buffer, 0, 0);
    wl_surface_damage(ctx->outputs[i].surface, 0, 0,
                      ctx->outputs[i].logical_size.width,
                      ctx->outputs[i].logical_size.height);
    wl_surface_commit(ctx->outputs[i].surface);
  }
}

void samure_backend_raw_on_layer_surface_configure(void *backend,
                                                   struct samure_context *ctx,
                                                   struct samure_output *output,
                                                   int32_t width,
                                                   int32_t height) {
  const uintptr_t output_index = OUTPUT_INDEX(output);
}
