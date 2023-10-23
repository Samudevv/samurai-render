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

#include <stdint.h>

struct samure_rect {
  int32_t x;
  int32_t y;
  int32_t w;
  int32_t h;
};

extern int samure_circle_in_output(struct samure_rect output_geo,
                                   int32_t circle_x, int32_t circle_y,
                                   int32_t radius);
extern int samure_rect_in_output(struct samure_rect output_geo, int32_t rect_x,
                                 int32_t rect_y, int32_t rect_w,
                                 int32_t rect_h);
extern int samure_square_in_output(struct samure_rect output_geo,
                                   int32_t square_x, int32_t square_y,
                                   int32_t square_size);
extern int samure_point_in_output(struct samure_rect output_geo,
                                  int32_t point_x, int32_t point_y);
extern int samure_triangle_in_output(struct samure_rect output_geo,
                                     int32_t tri_x1, int32_t tri_y1,
                                     int32_t tri_x2, int32_t tri_y2,
                                     int32_t tri_x3, int32_t tri_y3);
