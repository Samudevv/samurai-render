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

#include "error_handling.h"
#include "layer_surface.h"
#include "rect.h"
#include "shared_memory.h"
#include "wayland/xdg-output.h"
#include <wayland-client.h>

#define GLOBAL_TO_LOCAL_SCALE(sfc, global) ((double)(global)*sfc->scale)
#define GLOBAL_TO_LOCAL(output_geo, sfc, member, var)                          \
  GLOBAL_TO_LOCAL_SCALE(sfc, ((double)(var) - (double)output_geo.member))
#define GLOBAL_TO_LOCAL_X(output_geo, sfc, global_x)                           \
  GLOBAL_TO_LOCAL(output_geo, sfc, x, global_x)
#define GLOBAL_TO_LOCAL_Y(output_geo, sfc, global_y)                           \
  GLOBAL_TO_LOCAL(output_geo, sfc, y, global_y)
#define RENDER_X(x) GLOBAL_TO_LOCAL_X(output_geo, sfc, x)
#define RENDER_Y(y) GLOBAL_TO_LOCAL_Y(output_geo, sfc, y)
#define RENDER_SCALE(var) GLOBAL_TO_LOCAL_SCALE(sfc, var)

struct samure_context;

struct samure_output {
  struct wl_output *output;
  struct zxdg_output_v1 *xdg_output;
  struct samure_layer_surface **sfc;
  size_t num_sfc;

  struct samure_rect geo;
  char *name;
};

enum samure_screenshot_state {
  SAMURE_SCREENSHOT_PENDING,
  SAMURE_SCREENSHOT_READY,
  SAMURE_SCREENSHOT_FAILED,
  SAMURE_SCREENSHOT_DONE,
};

struct samure_screenshot_data {
  struct samure_context *ctx;
  struct samure_output *output;
  SAMURE_RESULT(shared_buffer) buffer_rs;
  enum samure_screenshot_state state;
};

SAMURE_DEFINE_RESULT(output);

extern SAMURE_RESULT(output)
    samure_create_output(struct samure_context *ctx, struct wl_output *output);
extern void samure_destroy_output(struct samure_context *ctx,
                                  struct samure_output *output);

extern void samure_output_set_pointer_interaction(struct samure_context *ctx,
                                                  struct samure_output *output,
                                                  int enable);

extern void samure_output_set_input_regions(struct samure_context *ctx,
                                            struct samure_output *output,
                                            struct samure_rect *rects,
                                            size_t num_rects);

extern void samure_output_set_keyboard_interaction(struct samure_output *output,
                                                   int enable);

extern void
samure_output_attach_layer_surface(struct samure_output *output,
                                   struct samure_layer_surface *layer_surface);

extern SAMURE_RESULT(shared_buffer)
    samure_output_screenshot(struct samure_context *ctx,
                             struct samure_output *output, int capture_cursor);
