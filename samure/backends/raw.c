#include "raw.h"
#include "../context.h"
#include "../layer_surface.h"
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

  r->base.render_end = samure_backend_raw_render_end;
  r->base.destroy = samure_destroy_backend_raw;
  r->base.associate_layer_surface = samure_backend_raw_associate_layer_surface;
  r->base.unassociate_layer_surface =
      samure_backend_raw_unassociate_layer_surface;

  return r;
}

void samure_destroy_backend_raw(struct samure_context *ctx,
                                struct samure_backend *b) {
  struct samure_backend_raw *r = (struct samure_backend_raw *)b;
  free(r->error_string);
  free(r);
}

void samure_backend_raw_render_end(struct samure_output *output,
                                   struct samure_layer_surface *layer_surface,
                                   struct samure_context *ctx,
                                   struct samure_backend *b) {
  struct samure_raw_surface *r =
      (struct samure_raw_surface *)layer_surface->backend_data;
  wl_surface_attach(layer_surface->surface, r->shared_buffer.buffer, 0, 0);
  wl_surface_damage(layer_surface->surface, 0, 0, output->geo.w, output->geo.h);
  wl_surface_commit(layer_surface->surface);
}

void samure_backend_raw_associate_layer_surface(
    struct samure_context *ctx, struct samure_backend *raw,
    struct samure_output *o, struct samure_layer_surface *sfc) {
  struct samure_backend_raw *r = (struct samure_backend_raw *)raw;
  sfc->backend_data = malloc(sizeof(struct samure_raw_surface));
  struct samure_raw_surface *raw_sfc =
      (struct samure_raw_surface *)sfc->backend_data;

  raw_sfc->shared_buffer =
      samure_create_shared_buffer(ctx->shm, BUFFER_FORMAT, o->geo.w, o->geo.h);
  if (raw_sfc->shared_buffer.buffer == NULL) {
    RAW_ADD_ERR_F("failed to create shared memory buffer for output %s",
                  o->name ? o->name : "");
  } else {
    samure_backend_raw_render_end(o, sfc, ctx, &r->base);
  }
}

void samure_backend_raw_unassociate_layer_surface(
    struct samure_context *ctx, struct samure_backend *raw,
    struct samure_output *o, struct samure_layer_surface *sfc) {
  if (!sfc->backend_data) {
    return;
  }

  struct samure_raw_surface *r = (struct samure_raw_surface *)sfc->backend_data;

  samure_destroy_shared_buffer(r->shared_buffer);
  free(r);
  sfc->backend_data = NULL;
}

struct samure_backend_raw *samure_get_backend_raw(struct samure_context *ctx) {
  return (struct samure_backend_raw *)ctx->backend;
}

struct samure_raw_surface *
samure_get_raw_surface(struct samure_layer_surface *layer_surface) {
  return (struct samure_raw_surface *)layer_surface->backend_data;
}
