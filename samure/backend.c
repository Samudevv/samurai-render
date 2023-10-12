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

#include "backend.h"

SAMURE_DEFINE_RESULT_UNWRAP(backend);

extern SAMURE_RESULT(backend) samure_create_backend(
    samure_on_layer_surface_configure_t on_layer_surface_configure,
    samure_render_start_t render_start, samure_render_end_t render_end,
    samure_destroy_t destroy,
    samure_associate_layer_surface_t associate_layer_surface,
    samure_unassociate_layer_surface_t unassociate_layer_surface) {
  SAMURE_RESULT_ALLOC(backend, b);

  b->on_layer_surface_configure = on_layer_surface_configure;
  b->render_start = render_start;
  b->render_end = render_end;
  b->destroy = destroy;
  b->associate_layer_surface = associate_layer_surface;
  b->unassociate_layer_surface = unassociate_layer_surface;

  SAMURE_RETURN_RESULT(backend, b);
}
