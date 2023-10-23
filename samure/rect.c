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

#include "rect.h"
#include <stdlib.h>

int samure_circle_in_output(struct samure_rect o, int32_t x, int32_t y,
                            int32_t r) {
  // Middle of the output
  const int32_t ox = o.x + o.w / 2;
  const int32_t oy = o.y + o.h / 2;

  // Distance between the middle of the circle and the middle of the output
  const int32_t dx = abs(x - ox);
  const int32_t dy = abs(y - oy);

  if ((dx > (o.w / 2 + r)) || (dy > (o.h / 2 + r)))
    return 0;

  if ((dx <= (o.w / 2)) || (dy <= (o.h / 2)))
    return 1;

  return (dx - o.w / 2) * (dx - o.w / 2) + (dy - o.h / 2) * (dy - o.h / 2) <=
         (r * r);
}

int samure_rect_in_output(struct samure_rect o, int32_t x, int32_t y, int32_t w,
                          int32_t h) {
  return (x < (o.x + o.w)) && ((x + w) > o.x) && (y < (o.y + o.h)) &&
         ((y + h) > o.y);
}

int samure_square_in_output(struct samure_rect o, int32_t square_x,
                            int32_t square_y, int32_t square_size) {
  return samure_rect_in_output(o, square_x, square_y, square_size, square_size);
}

int samure_point_in_output(struct samure_rect o, int32_t x, int32_t y) {
  return (x > o.x) && (x < (o.x + o.w)) && (y > o.y) && (y < o.y + o.h);
}

int samure_triangle_in_output(struct samure_rect o, int32_t x1, int32_t y1,
                              int32_t x2, int32_t y2, int32_t x3, int32_t y3) {
  return samure_point_in_output(o, x1, y1) ||
         samure_point_in_output(o, x2, y2) || samure_point_in_output(o, x3, y3);
}
