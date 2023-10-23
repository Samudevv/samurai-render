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

#pragma once

#include "wayland/wlr-layer-shell-unstable-v1.h"
#include <wayland-client.h>

#include "rect.h"
#include "shared_memory.h"

#define SAMURE_LAYER_SURFACE_ANCHOR_FILL                                       \
  (ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP | ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT |      \
   ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT | ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM)
#define SAMURE_LAYER_OVERLAY ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY

struct samure_context;
struct samure_output;

struct samure_layer_surface {
  struct wl_surface *surface;
  struct zwlr_layer_surface_v1 *layer_surface;
  void *backend_data;
  uint32_t w;
  uint32_t h;

  struct samure_callback_data *callback_data;
  int not_ready;
  int dirty;
};

SAMURE_DEFINE_RESULT(layer_surface);

extern SAMURE_RESULT(layer_surface)
    samure_create_layer_surface(struct samure_context *ctx,
                                struct samure_output *output, uint32_t layer,
                                uint32_t anchor, int keyboard_interaction,
                                int pointer_interaction,
                                int backend_association);

extern void samure_destroy_layer_surface(struct samure_context *ctx,
                                         struct samure_layer_surface *sfc);

extern void samure_layer_surface_draw_buffer(struct samure_layer_surface *sfc,
                                             struct samure_shared_buffer *buf);

extern void samure_layer_surface_request_frame(struct samure_context *ctx,
                                               struct samure_layer_surface *sfc,
                                               struct samure_rect geo);
