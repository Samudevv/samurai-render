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
#include <stdint.h>

#include "error_handling.h"

struct samure_context;
struct samure_output;
struct samure_layer_surface;

typedef void (*samure_on_layer_surface_configure_t)(
    struct samure_context *ctx, struct samure_layer_surface *layer_surface,
    int32_t width, int32_t height);
typedef void (*samure_render_start_t)(
    struct samure_context *ctx, struct samure_layer_surface *layer_surface);
typedef void (*samure_render_end_t)(struct samure_context *ctx,
                                    struct samure_layer_surface *layer_surface);
typedef void (*samure_destroy_t)(struct samure_context *ctx);
typedef samure_error (*samure_associate_layer_surface_t)(
    struct samure_context *ctx, struct samure_layer_surface *layer_surface);
typedef void (*samure_unassociate_layer_surface_t)(
    struct samure_context *ctx, struct samure_layer_surface *layer_surface);

struct samure_backend {
  samure_on_layer_surface_configure_t on_layer_surface_configure;
  samure_render_start_t render_start;
  samure_render_end_t render_end;
  samure_destroy_t destroy;
  samure_associate_layer_surface_t associate_layer_surface;
  samure_unassociate_layer_surface_t unassociate_layer_surface;
};

SAMURE_DEFINE_RESULT(backend);

extern SAMURE_RESULT(backend) samure_create_backend(
    samure_on_layer_surface_configure_t on_layer_surface_configure,
    samure_render_start_t render_start, samure_render_end_t render_end,
    samure_destroy_t destroy,
    samure_associate_layer_surface_t associate_layer_surface,
    samure_unassociate_layer_surface_t unassociate_layer_surface);
