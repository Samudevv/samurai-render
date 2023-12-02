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

enum samure_event_type {
  SAMURE_EVENT_LAYER_SURFACE_CONFIGURE,
  SAMURE_EVENT_POINTER_BUTTON,
  SAMURE_EVENT_POINTER_MOTION,
  SAMURE_EVENT_POINTER_ENTER,
  SAMURE_EVENT_POINTER_LEAVE,
  SAMURE_EVENT_KEYBOARD_KEY,
  SAMURE_EVENT_KEYBOARD_ENTER,
  SAMURE_EVENT_KEYBOARD_LEAVE,
  SAMURE_EVENT_TOUCH_DOWN,
  SAMURE_EVENT_TOUCH_UP,
  SAMURE_EVENT_TOUCH_MOTION,
};

struct samure_seat;
struct samure_output;
struct samure_layer_surface;

struct samure_event {
  int type;
  struct samure_seat *seat;
  struct samure_output *output;
  struct samure_layer_surface *surface;
  uint32_t button;
  uint32_t state;
  uint32_t width;
  uint32_t height;
  double x;
  double y;
  int32_t touch_id;
};
