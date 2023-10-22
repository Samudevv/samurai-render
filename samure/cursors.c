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

#include "cursors.h"
#include "seat.h"

SAMURE_DEFINE_RESULT_UNWRAP(cursor_engine);

SAMURE_RESULT(cursor_engine)
samure_create_cursor_engine(struct wp_cursor_shape_manager_v1 *manager) {
  SAMURE_RESULT_ALLOC(cursor_engine, c);

  if (manager) {
    c->manager = manager;
  } else {
  }

  SAMURE_RETURN_RESULT(cursor_engine, c);
}

void samure_destroy_cursor_engine(struct samure_cursor_engine *engine) {
  if (engine->manager) {
    wp_cursor_shape_manager_v1_destroy(engine->manager);
  }
  free(engine);
}

void samure_cursor_engine_set_shape(struct samure_cursor_engine *engine,
                                    struct samure_seat *seat, uint32_t shape) {
  if (seat->pointer) {
    if (engine->manager) {
      struct wp_cursor_shape_device_v1 *device =
          wp_cursor_shape_manager_v1_get_pointer(engine->manager,
                                                 seat->pointer);
      wp_cursor_shape_device_v1_set_shape(device, seat->last_pointer_enter,
                                          shape);
      wp_cursor_shape_device_v1_destroy(device);
    } else {
    }
  }
}
