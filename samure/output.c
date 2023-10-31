/***********************************************************************************
 *                         This file is part of samurai-render
 *                    https://github.com/PucklaJ/samurai-render
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

#include "output.h"
#include "callbacks.h"
#include "context.h"
#include <assert.h>
#include <stdlib.h>

SAMURE_DEFINE_RESULT_UNWRAP(output);

SAMURE_RESULT(output)
samure_create_output(struct samure_context *ctx, struct wl_output *output) {
  SAMURE_RESULT_ALLOC(output, o);

  o->output = output;
  if (ctx->output_manager) {
    o->xdg_output =
        zxdg_output_manager_v1_get_xdg_output(ctx->output_manager, o->output);
    if (o->xdg_output) {
      zxdg_output_v1_add_listener(o->xdg_output, &xdg_output_listener, o);
    }
  }
  wl_output_add_listener(o->output, &output_listener, o);
  wl_display_roundtrip(ctx->display);

  if (o->refresh_rate == 0) {
    // If no refresh rate could be retrieved assume 60Hz
    o->refresh_rate = 60;
  }

  SAMURE_RETURN_RESULT(output, o);
}

void samure_destroy_output(struct samure_context *ctx,
                           struct samure_output *o) {
  free(o->name);
  for (size_t i = 0; i < o->num_sfc; i++) {
    samure_destroy_layer_surface(ctx, o->sfc[i]);
  }
  free(o->sfc);
  if (o->xdg_output)
    zxdg_output_v1_destroy(o->xdg_output);
  if (o->output)
    wl_output_destroy(o->output);
  free(o);
}

void samure_output_set_pointer_interaction(struct samure_context *ctx,
                                           struct samure_output *o,
                                           int enable) {
  for (size_t i = 0; i < o->num_sfc; i++) {
    if (enable) {
      wl_surface_set_input_region(o->sfc[i]->surface, NULL);
    } else {
      struct wl_region *reg = wl_compositor_create_region(ctx->compositor);
      if (!reg) {
        continue;
      }

      wl_surface_set_input_region(o->sfc[i]->surface, reg);
      wl_region_destroy(reg);
    }
    wl_surface_commit(o->sfc[i]->surface);
  }
}

void samure_output_set_input_regions(struct samure_context *ctx,
                                     struct samure_output *o,
                                     struct samure_rect *r, size_t num_rects) {
  struct wl_region *reg = wl_compositor_create_region(ctx->compositor);
  if (!reg) {
    return;
  }

  for (size_t i = 0; i < num_rects; i++) {
    wl_region_add(reg, r[i].x, r[i].y, r[i].w, r[i].h);
  }

  for (size_t i = 0; i < o->num_sfc; i++) {
    wl_surface_set_input_region(o->sfc[i]->surface, reg);
    wl_surface_commit(o->sfc[i]->surface);
  }
  wl_region_destroy(reg);
}

void samure_output_set_keyboard_interaction(struct samure_output *o,
                                            int enable) {
  for (size_t i = 0; i < o->num_sfc; i++) {
    zwlr_layer_surface_v1_set_keyboard_interactivity(o->sfc[i]->layer_surface,
                                                     (uint32_t)enable);
    wl_surface_commit(o->sfc[i]->surface);
  }
}

void samure_output_attach_layer_surface(struct samure_output *o,
                                        struct samure_layer_surface *sfc) {
  o->num_sfc++;
  o->sfc = realloc(o->sfc, o->num_sfc * sizeof(struct samure_layer_surface *));
  if (!o->sfc) {
    o->num_sfc = 0;
    return;
  }
  o->sfc[o->num_sfc - 1] = sfc;
}

extern SAMURE_RESULT(shared_buffer)
    samure_output_screenshot(struct samure_context *ctx,
                             struct samure_output *output, int capture_cursor) {
  uint64_t error_code = SAMURE_ERROR_NONE;
  if (!ctx->shm)
    error_code |= SAMURE_ERROR_NO_SHM;
  if (!ctx->screencopy_manager)
    error_code |= SAMURE_ERROR_NO_SCREENCOPY_MANAGER;
  if (error_code != SAMURE_ERROR_NONE) {
    SAMURE_RETURN_ERROR(shared_buffer, error_code);
  }

  struct samure_screenshot_data data = {0};
  data.ctx = ctx;
  data.output = output;
  data.buffer_rs.error = SAMURE_ERROR_NOT_IMPLEMENTED;

  struct zwlr_screencopy_frame_v1 *frame =
      zwlr_screencopy_manager_v1_capture_output(ctx->screencopy_manager,
                                                capture_cursor, output->output);
  if (!frame) {
    SAMURE_RETURN_ERROR(shared_buffer, SAMURE_ERROR_FRAME_INIT);
  };

  zwlr_screencopy_frame_v1_add_listener(frame, &screencopy_frame_listener,
                                        &data);

  while (data.state == SAMURE_SCREENSHOT_PENDING &&
         wl_display_dispatch(ctx->display) != -1)
    ;

  if (data.state == SAMURE_SCREENSHOT_FAILED) {
    if (data.buffer_rs.result) {
      samure_destroy_shared_buffer(data.buffer_rs.result);
    }
    SAMURE_RETURN_ERROR(shared_buffer,
                        SAMURE_ERROR_FAILED | data.buffer_rs.error);
  }

  if (SAMURE_HAS_ERROR(data.buffer_rs)) {
    SAMURE_RETURN_ERROR(shared_buffer, data.buffer_rs.error);
  }

  struct samure_shared_buffer *buffer =
      SAMURE_UNWRAP(shared_buffer, data.buffer_rs);

  data.state = SAMURE_SCREENSHOT_PENDING;
  zwlr_screencopy_frame_v1_copy(frame, buffer->buffer);

  while (data.state == SAMURE_SCREENSHOT_PENDING &&
         wl_display_dispatch(ctx->display) != -1)
    ;

  if (data.state == SAMURE_SCREENSHOT_FAILED) {
    SAMURE_DESTROY_ERROR(shared_buffer, buffer, SAMURE_ERROR_FAILED);
  }

  SAMURE_RETURN_RESULT(shared_buffer, buffer);
}
