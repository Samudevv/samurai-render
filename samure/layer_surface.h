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

#pragma once

#include <wayland-client.h>

#include "rect.h"
#include "shared_memory.h"

// public
#define SAMURE_LAYER_SURFACE_ANCHOR_FILL                                       \
  (ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP | ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT |      \
   ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT | ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM)
#define SAMURE_LAYER_BACKGROUND ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND
#define SAMURE_LAYER_BOTTOM ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM
#define SAMURE_LAYER_TOP ZWLR_LAYER_SHELL_V1_LAYER_TOP
#define SAMURE_LAYER_OVERLAY ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY

struct samure_context;
struct samure_output;
struct zwlr_layer_surface_v1;
struct wp_fractional_scale_v1;
struct wp_viewport;

// public
struct samure_layer_surface {
  struct wl_surface *surface;
  struct zwlr_layer_surface_v1 *layer_surface;
  struct wp_fractional_scale_v1 *fractional_scale;
  struct wp_viewport *viewport;
  void *backend_data;
  uint32_t w;
  uint32_t h;
  int32_t preferred_buffer_scale;

  struct samure_callback_data *callback_data;
  int not_ready;
  int dirty;
  int configured;

  double frame_start_time; // Absolute time of the last frame (for internal use)
  double frame_delta_time; // The actual time that passes between each call to
                           // samure_context_render_layer_surface in seconds
  double scale;
};

SAMURE_DEFINE_RESULT(layer_surface);

// public
extern SAMURE_RESULT(layer_surface)
    samure_create_layer_surface(struct samure_context *ctx,
                                struct samure_output *output, uint32_t layer,
                                uint32_t anchor, int keyboard_interaction,
                                int pointer_interaction,
                                int backend_association);

// public
extern void samure_destroy_layer_surface(struct samure_context *ctx,
                                         struct samure_layer_surface *sfc);

// public
extern void samure_layer_surface_draw_buffer(struct samure_layer_surface *sfc,
                                             struct samure_shared_buffer *buf);

extern void samure_layer_surface_request_frame(struct samure_context *ctx,
                                               struct samure_layer_surface *sfc,
                                               struct samure_rect geo);

extern SAMURE_RESULT(shared_buffer)
    samure_create_shared_buffer_for_layer_surface(
        struct samure_context *ctx, struct samure_layer_surface *sfc,
        struct samure_shared_buffer *old_buffer);
