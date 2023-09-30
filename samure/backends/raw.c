#include "raw.h"
#include "../context.h"
#include "../layer_surface.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

SAMURE_RESULT(backend_raw) samure_init_backend_raw(struct samure_context *ctx) {
  SAMURE_RESULT_ALLOC(backend_raw, r);

  r->base.render_end = samure_backend_raw_render_end;
  r->base.destroy = samure_destroy_backend_raw;
  r->base.associate_layer_surface = samure_backend_raw_associate_layer_surface;
  r->base.unassociate_layer_surface =
      samure_backend_raw_unassociate_layer_surface;

  SAMURE_RETURN_RESULT(backend_raw, r);
}

void samure_destroy_backend_raw(struct samure_context *ctx,
                                struct samure_backend *b) {
  struct samure_backend_raw *r = (struct samure_backend_raw *)b;
  free(r);
}

void samure_backend_raw_render_end(struct samure_output *output,
                                   struct samure_layer_surface *layer_surface,
                                   struct samure_context *ctx,
                                   struct samure_backend *b) {
  struct samure_raw_surface *r =
      (struct samure_raw_surface *)layer_surface->backend_data;
  samure_layer_surface_draw_buffer(layer_surface, r->buffer);
}

samure_error samure_backend_raw_associate_layer_surface(
    struct samure_context *ctx, struct samure_backend *raw,
    struct samure_output *o, struct samure_layer_surface *sfc) {
  struct samure_backend_raw *r = (struct samure_backend_raw *)raw;
  struct samure_raw_surface *raw_sfc =
      malloc(sizeof(struct samure_raw_surface));
  if (!raw_sfc) {
    return SAMURE_ERROR_MEMORY;
  }
  memset(raw_sfc, 0, sizeof(struct samure_raw_surface));

  SAMURE_RESULT(shared_buffer)
  b_rs = samure_create_shared_buffer(ctx->shm, SAMURE_BUFFER_FORMAT, o->geo.w,
                                     o->geo.h);
  if (SAMURE_HAS_ERROR(b_rs)) {
    free(raw_sfc);
    return SAMURE_ERROR_SHARED_BUFFER_INIT | b_rs.error;
  }

  raw_sfc->buffer = SAMURE_GET_RESULT(shared_buffer, b_rs);

  sfc->backend_data = raw_sfc;
  samure_backend_raw_render_end(o, sfc, ctx, &r->base);

  return SAMURE_ERROR_NONE;
}

void samure_backend_raw_unassociate_layer_surface(
    struct samure_context *ctx, struct samure_backend *raw,
    struct samure_output *o, struct samure_layer_surface *sfc) {
  if (!sfc->backend_data) {
    return;
  }

  struct samure_raw_surface *r = (struct samure_raw_surface *)sfc->backend_data;

  samure_destroy_shared_buffer(r->buffer);
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
