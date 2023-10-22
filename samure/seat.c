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

#include "seat.h"
#include "callbacks.h"
#include "context.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

SAMURE_DEFINE_RESULT_UNWRAP(seat);

SAMURE_RESULT(seat)
samure_create_seat(struct samure_context *ctx, struct wl_seat *seat) {
  SAMURE_RESULT_ALLOC(seat, s);

  s->seat = seat;

  wl_seat_add_listener(s->seat, &seat_listener,
                       samure_create_callback_data(ctx, s));
  wl_display_roundtrip(ctx->display);
  struct samure_callback_data *cbd = samure_create_callback_data(ctx, s);

  if (s->pointer) {
    wl_pointer_add_listener(s->pointer, &pointer_listener, cbd);
  }
  if (s->keyboard) {
    wl_keyboard_add_listener(s->keyboard, &keyboard_listener, cbd);
  }
  if (s->touch) {
    wl_touch_add_listener(s->touch, &touch_listener, cbd);
  }

  SAMURE_RETURN_RESULT(seat, s);
}

void samure_destroy_seat(struct samure_seat *s) {
  free(s->name);
  if (s->pointer) {
    wl_pointer_destroy(s->pointer);
  }
  if (s->keyboard)
    wl_keyboard_destroy(s->keyboard);
  if (s->touch)
    wl_touch_destroy(s->touch);
  if (s->seat)
    wl_seat_destroy(s->seat);
  free(s);
}
