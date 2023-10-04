#include "layer_surface.h"
#include "callbacks.h"
#include "context.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SAMURE_LAYER_SURFACE_DESTROY_ERROR(error_code)                         \
  {                                                                            \
    samure_destroy_layer_surface(ctx, s);                                      \
    SAMURE_RETURN_ERROR(layer_surface, error_code);                            \
  }

SAMURE_DEFINE_RESULT_UNWRAP(layer_surface);

SAMURE_RESULT(layer_surface)
samure_create_layer_surface(struct samure_context *ctx, struct samure_output *o,
                            uint32_t layer, uint32_t anchor,
                            uint32_t keyboard_interaction,
                            int pointer_interaction, int backend_association) {
  SAMURE_RESULT_ALLOC(layer_surface, s);

  s->surface = wl_compositor_create_surface(ctx->compositor);
  if (!s->surface) {
    SAMURE_LAYER_SURFACE_DESTROY_ERROR(SAMURE_ERROR_SURFACE_INIT);
  }

  s->layer_surface = zwlr_layer_shell_v1_get_layer_surface(
      ctx->layer_shell, s->surface, o ? o->output : NULL, layer,
      "samurai-render");
  if (!s->layer_surface) {
    SAMURE_LAYER_SURFACE_DESTROY_ERROR(SAMURE_ERROR_LAYER_SURFACE_INIT);
  }

  s->callback_data = samure_create_layer_surface_callback_data(ctx, o, s);

  zwlr_layer_surface_v1_add_listener(s->layer_surface, &layer_surface_listener,
                                     s->callback_data);
  zwlr_layer_surface_v1_set_anchor(s->layer_surface, anchor);
  zwlr_layer_surface_v1_set_keyboard_interactivity(s->layer_surface,
                                                   keyboard_interaction);
  zwlr_layer_surface_v1_set_exclusive_zone(s->layer_surface, -1);
  if (pointer_interaction) {
    wl_surface_set_input_region(s->surface, NULL);
  } else {
    struct wl_region *reg = wl_compositor_create_region(ctx->compositor);
    if (reg) {
      wl_surface_set_input_region(s->surface, reg);
      wl_region_destroy(reg);
    }
  }
  wl_surface_commit(s->surface);
  wl_display_roundtrip(ctx->display);

  if (backend_association && ctx->backend &&
      ctx->backend->associate_layer_surface) {
    const samure_error err =
        ctx->backend->associate_layer_surface(ctx, ctx->backend, s);
    if (SAMURE_IS_ERROR(err)) {
      SAMURE_LAYER_SURFACE_DESTROY_ERROR(err);
    }
  }

  SAMURE_RETURN_RESULT(layer_surface, s);
}

void samure_destroy_layer_surface(struct samure_context *ctx,
                                  struct samure_layer_surface *sfc) {
  if (ctx->backend && ctx->backend->unassociate_layer_surface) {
    ctx->backend->unassociate_layer_surface(ctx, ctx->backend, sfc);
  }

  if (sfc->layer_surface)
    zwlr_layer_surface_v1_destroy(sfc->layer_surface);
  if (sfc->surface)
    wl_surface_destroy(sfc->surface);
  if (sfc->callback_data)
    free(sfc->callback_data);
  free(sfc);
}

void samure_layer_surface_draw_buffer(struct samure_layer_surface *sfc,
                                      struct samure_shared_buffer *buf) {
  wl_surface_attach(sfc->surface, buf->buffer, 0, 0);
  wl_surface_damage(sfc->surface, 0, 0, buf->width, buf->height);
  wl_surface_commit(sfc->surface);
}

struct samure_layer_surface_callback_data *
samure_create_layer_surface_callback_data(
    struct samure_context *ctx, struct samure_output *output,
    struct samure_layer_surface *surface) {
  struct samure_layer_surface_callback_data *d =
      malloc(sizeof(struct samure_layer_surface_callback_data));
  assert(d != NULL);
  d->ctx = ctx;
  d->output = output;
  d->surface = surface;
  return d;
}
