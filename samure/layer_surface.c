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

#include "layer_surface.h"
#include "callbacks.h"
#include "context.h"
#include "wayland/fractional-scale.h"
#include "wayland/viewporter.h"
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
                            int keyboard_interaction, int pointer_interaction,
                            int backend_association) {
  SAMURE_RESULT_ALLOC(layer_surface, s);

  s->preferred_buffer_scale = 1;
  s->scale = 1.0;

  if (o) {
    s->w = o->geo.w;
    s->h = o->geo.h;
  }

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

  s->callback_data = samure_create_callback_data(ctx, s);

  zwlr_layer_surface_v1_add_listener(s->layer_surface, &layer_surface_listener,
                                     s->callback_data);
  zwlr_layer_surface_v1_set_anchor(s->layer_surface, anchor);
  zwlr_layer_surface_v1_set_keyboard_interactivity(
      s->layer_surface, (uint32_t)keyboard_interaction);
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

  if (ctx->fractional_scale_manager) {
    s->fractional_scale = wp_fractional_scale_manager_v1_get_fractional_scale(
        ctx->fractional_scale_manager, s->surface);
    if (!s->fractional_scale) {
      SAMURE_LAYER_SURFACE_DESTROY_ERROR(SAMURE_ERROR_FRACTIONAL_SCALE_INIT);
    }
    wp_fractional_scale_v1_add_listener(
        s->fractional_scale, &fractional_scale_listener, s->callback_data);
  }

  if (ctx->viewporter) {
    s->viewport = wp_viewporter_get_viewport(ctx->viewporter, s->surface);
    if (!s->viewport) {
      SAMURE_LAYER_SURFACE_DESTROY_ERROR(SAMURE_ERROR_VIEWPORT_INIT);
    }
  }

  wl_surface_add_listener(s->surface, &surface_listener, s->callback_data);
  wl_surface_commit(s->surface);
  wl_display_roundtrip(ctx->display);

  if (!s->fractional_scale) {
    s->scale = (double)s->preferred_buffer_scale;
    if (!s->viewport) {
      DEBUG_PRINTF("surface_set_buffer_scale scale=%d\n", (int32_t)s->scale);
      wl_surface_set_buffer_scale(s->surface, (int32_t)s->scale);
    }
  }

  if (backend_association && ctx->backend &&
      ctx->backend->associate_layer_surface) {
    const samure_error err = ctx->backend->associate_layer_surface(ctx, s);
    if (SAMURE_IS_ERROR(err)) {
      SAMURE_LAYER_SURFACE_DESTROY_ERROR(err);
    }
  }

  s->frame_start_time = samure_get_time();

  SAMURE_RETURN_RESULT(layer_surface, s);
}

void samure_destroy_layer_surface(struct samure_context *ctx,
                                  struct samure_layer_surface *sfc) {
  if (ctx->backend && ctx->backend->unassociate_layer_surface) {
    ctx->backend->unassociate_layer_surface(ctx, sfc);
  }

  if (sfc->layer_surface)
    zwlr_layer_surface_v1_destroy(sfc->layer_surface);
  if (sfc->fractional_scale)
    wp_fractional_scale_v1_destroy(sfc->fractional_scale);
  if (sfc->surface)
    wl_surface_destroy(sfc->surface);
  if (sfc->viewport)
    wp_viewport_destroy(sfc->viewport);
  if (sfc->callback_data)
    free(sfc->callback_data);
  free(sfc);
}

void samure_layer_surface_draw_buffer(struct samure_layer_surface *sfc,
                                      struct samure_shared_buffer *buf) {
  wl_surface_attach(sfc->surface, buf->buffer, 0, 0);
  wl_surface_damage_buffer(sfc->surface, 0, 0, buf->width, buf->height);
  if (sfc->viewport) {
    wp_viewport_set_destination(sfc->viewport, sfc->w, sfc->h);
    wp_viewport_set_source(sfc->viewport, 0, 0, wl_fixed_from_int(buf->width),
                           wl_fixed_from_int(buf->height));
  }
  wl_surface_commit(sfc->surface);
}

void samure_layer_surface_request_frame(struct samure_context *ctx,
                                        struct samure_layer_surface *sfc,
                                        struct samure_rect geo) {
  if (sfc->not_ready) {
    return;
  }

  struct wl_callback *cb = wl_surface_frame(sfc->surface);
  wl_callback_add_listener(cb, &frame_listener,
                           samure_create_frame_data(ctx, geo, sfc));
  sfc->not_ready = 1;
}

SAMURE_RESULT(shared_buffer)
samure_create_shared_buffer_for_layer_surface(
    struct samure_context *ctx, struct samure_layer_surface *sfc,
    struct samure_shared_buffer *old_buffer) {
  const uint32_t scaled_width = RENDER_SCALE(sfc->w);
  const uint32_t scaled_height = RENDER_SCALE(sfc->h);

  if (old_buffer) {
    if (old_buffer->width == scaled_width &&
        old_buffer->height == scaled_height) {
      SAMURE_RETURN_RESULT(shared_buffer, old_buffer);
    }

    samure_destroy_shared_buffer(old_buffer);
  }

  return samure_create_shared_buffer(ctx->shm, SAMURE_BUFFER_FORMAT,
                                     scaled_width == 0 ? 1 : scaled_width,
                                     scaled_height == 0 ? 1 : scaled_height);
}
