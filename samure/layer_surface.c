#include "layer_surface.h"
#include "callbacks.h"
#include "context.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SFC_ERR(msg)                                                           \
  if (s->error_string)                                                         \
    free(s->error_string);                                                     \
  s->error_string = strdup(msg)
#define SFC_ERR_F(format, ...)                                                 \
  if (s->error_string)                                                         \
    free(s->error_string);                                                     \
  s->error_string = malloc(2048);                                              \
  snprintf(s->error_string, 2048, format, __VA_ARGS__)

struct samure_layer_surface *
samure_create_layer_surface(struct samure_context *ctx, struct samure_output *o,
                            uint32_t layer, uint32_t anchor,
                            uint32_t keyboard_interaction,
                            int pointer_interaction, int backend_association) {
  struct samure_layer_surface *s = malloc(sizeof(struct samure_layer_surface));
  memset(s, 0, sizeof(struct samure_layer_surface));

  s->surface = wl_compositor_create_surface(ctx->compositor);
  if (!s->surface) {
    SFC_ERR("failed to create surface");
    return s;
  }

  s->layer_surface = zwlr_layer_shell_v1_get_layer_surface(
      ctx->layer_shell, s->surface, o->output, layer, "samurai-render");
  if (!s->layer_surface) {
    SFC_ERR("failed to create layer surface");
    wl_surface_destroy(s->surface);
    return s;
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
    wl_surface_set_input_region(s->surface, reg);
    wl_region_destroy(reg);
  }
  wl_surface_commit(s->surface);
  wl_display_roundtrip(ctx->display);

  if (backend_association && ctx->backend &&
      ctx->backend->associate_layer_surface) {
    ctx->backend->associate_layer_surface(ctx, ctx->backend, o, s);
  }

  return s;
}

void samure_destroy_layer_surface(struct samure_context *ctx,
                                  struct samure_output *o,
                                  struct samure_layer_surface *sfc) {
  if (ctx->backend && ctx->backend->unassociate_layer_surface) {
    ctx->backend->unassociate_layer_surface(ctx, ctx->backend, o, sfc);
  }

  zwlr_layer_surface_v1_destroy(sfc->layer_surface);
  wl_surface_destroy(sfc->surface);
  free(sfc->error_string);
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
  d->ctx = ctx;
  d->output = output;
  d->surface = surface;
  return d;
}
