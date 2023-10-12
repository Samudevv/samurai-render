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

#include "../backend.h"
#include "../error_handling.h"
#include "../shared_memory.h"

struct samure_context;
struct samure_layer_surface;

struct samure_raw_surface {
  struct samure_shared_buffer *buffer;
};

struct samure_backend_raw {
  struct samure_backend base;
};

SAMURE_DEFINE_RESULT(backend_raw);

extern SAMURE_RESULT(backend_raw)
    samure_init_backend_raw(struct samure_context *ctx);
extern void samure_destroy_backend_raw(struct samure_context *ctx);
extern void
samure_backend_raw_render_end(struct samure_context *ctx,
                              struct samure_layer_surface *layer_surface);
extern samure_error
samure_backend_raw_associate_layer_surface(struct samure_context *ctx,
                                           struct samure_layer_surface *sfc);
extern void samure_backend_raw_on_layer_surface_configure(
    struct samure_context *ctx, struct samure_layer_surface *layer_surface,
    int32_t width, int32_t height);
extern void
samure_backend_raw_unassociate_layer_surface(struct samure_context *ctx,
                                             struct samure_layer_surface *sfc);
extern struct samure_backend_raw *
samure_get_backend_raw(struct samure_context *ctx);
extern struct samure_raw_surface *
samure_get_raw_surface(struct samure_layer_surface *layer_surface);
