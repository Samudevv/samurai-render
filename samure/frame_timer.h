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
#include <time.h>

#define SAMURE_NUM_MEASURES 11
#define SAMURE_NUM_TAKEAWAYS 2

struct samure_frame_timer {
  uint32_t max_fps;
  uint32_t fps;
  double delta_time;

  double start_time;
  double raw_delta_time;

  double raw_delta_times[SAMURE_NUM_MEASURES];
  size_t current_raw_delta_times_index;
  size_t num_raw_delta_times;
  double mean_delta_time;
  double smoothed_delta_times[SAMURE_NUM_MEASURES];
};

extern struct samure_frame_timer samure_init_frame_timer(uint32_t max_fps);
extern void samure_frame_timer_start_frame(struct samure_frame_timer *f);
extern void samure_frame_timer_end_frame(struct samure_frame_timer *f);
extern double samure_get_time();
