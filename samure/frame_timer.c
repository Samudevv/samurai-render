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

#include "frame_timer.h"
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#define SWAP(type, a, b)                                                       \
  {                                                                            \
    const type temp = a;                                                       \
    a = b;                                                                     \
    b = temp;                                                                  \
  }

struct samure_frame_timer samure_init_frame_timer(uint32_t max_fps) {
  struct samure_frame_timer f = {0};
  f.max_update_frequency = max_fps;
  f.update_frequency = max_fps;
  f.delta_time = 1.0 / (double)max_fps;
  return f;
}

void samure_frame_timer_start_frame(struct samure_frame_timer *f) {
  f->start_time = samure_get_time();
}

void samure_frame_timer_end_frame(struct samure_frame_timer *f) {
  const double end_time = samure_get_time();
  f->raw_delta_time = end_time - f->start_time;

  // Limit FPS
  const double max_delta_time = 1.0 / (double)f->max_update_frequency;
  if (f->raw_delta_time < max_delta_time) {
    const double max_sleep_time = max_delta_time - f->raw_delta_time;

    const double sleep_start_time = samure_get_time();

    const useconds_t sleep_time =
        (useconds_t)(max_sleep_time * 1000.0 * 1000.0);
    usleep(sleep_time);

    const double sleep_end_time = samure_get_time();

    const double slept_time = sleep_end_time - sleep_start_time;

    f->raw_delta_time += slept_time;
  }

  // Store raw delta time
  f->raw_delta_times[f->current_raw_delta_times_index] = f->raw_delta_time;
  f->smoothed_delta_times[f->current_raw_delta_times_index] = f->raw_delta_time;
  if (f->num_raw_delta_times < SAMURE_NUM_MEASURES)
    f->num_raw_delta_times++;

  // Take away highest and lowest
  if (f->num_raw_delta_times > 2 * SAMURE_NUM_TAKEAWAYS) {
    for (size_t i = 0; i < SAMURE_NUM_TAKEAWAYS; i++) {
      size_t max_index = f->num_raw_delta_times - 1 - i;
      size_t min_index = i;
      double max_val = f->smoothed_delta_times[max_index];
      double min_val = f->smoothed_delta_times[min_index];

      for (size_t j = i; j < f->num_raw_delta_times - i; j++) {
        if (f->smoothed_delta_times[j] < min_val) {
          min_val = f->smoothed_delta_times[j];
          min_index = j;
        }
        if (f->smoothed_delta_times[j] > max_val) {
          max_val = f->smoothed_delta_times[j];
          max_index = j;
        }
      }

      SWAP(double, f->smoothed_delta_times[min_index],
           f->smoothed_delta_times[i]);
      SWAP(double, f->smoothed_delta_times[max_index],
           f->smoothed_delta_times[f->num_raw_delta_times - 1 - i]);
    }
  }

  // Calculate mean delta time
  f->mean_delta_time = 0.0;
  if (f->num_raw_delta_times > 2 * SAMURE_NUM_TAKEAWAYS) {
    for (size_t i = SAMURE_NUM_TAKEAWAYS;
         i < f->num_raw_delta_times - SAMURE_NUM_TAKEAWAYS; i++) {
      f->mean_delta_time += f->smoothed_delta_times[i];
    }

    f->mean_delta_time /=
        (double)(f->num_raw_delta_times - 2 * SAMURE_NUM_TAKEAWAYS);
  } else {
    for (size_t i = 0; i < f->num_raw_delta_times; i++) {
      f->mean_delta_time += f->smoothed_delta_times[i];
    }

    f->mean_delta_time /= (double)f->num_raw_delta_times;
  }

  f->current_raw_delta_times_index++;
  if (f->current_raw_delta_times_index == SAMURE_NUM_MEASURES) {
    f->current_raw_delta_times_index = 0;
  }

  f->delta_time = f->mean_delta_time;
  f->update_frequency = (uint32_t)(1.0 / f->delta_time);
}

double samure_get_time() {
  struct timespec tp;
  clock_gettime(CLOCK_REALTIME, &tp);
  return (double)(tp.tv_sec * (1000 * 1000 * 1000) + tp.tv_nsec) /
         (1000.0 * 1000.0 * 1000.0);
}
