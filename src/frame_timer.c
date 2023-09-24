#include "frame_timer.h"
#include <time.h>
#include <unistd.h>

#define CLOCKS2DT(start, end)                                                  \
  (((double)end - (double)start) / (double)CLOCKS_PER_SEC)
#define SWAP(type, a, b)                                                       \
  {                                                                            \
    const type temp = a;                                                       \
    a = b;                                                                     \
    b = temp;                                                                  \
  }

struct samure_frame_timer samure_init_frame_timer(uint32_t max_fps) {
  struct samure_frame_timer f = {0};
  f.max_fps = max_fps;
  f.fps = max_fps;
  f.delta_time = 1.0 / (double)max_fps;
  return f;
}

void samure_frame_timer_start_frame(struct samure_frame_timer *f) {
  f->start_time = clock();
}

void samure_frame_timer_end_frame(struct samure_frame_timer *f) {
  const clock_t end_time = clock();
  f->raw_delta_time = CLOCKS2DT(f->start_time, end_time);
  f->raw_delta_time += f->additional_time;

  // Limit FPS
  const double max_delta_time = 1.0 / (double)f->max_fps;
  if (f->raw_delta_time < max_delta_time) {
    const double max_sleep_time = max_delta_time - f->raw_delta_time;

    struct timespec sleep_start_time;
    const int sleep_start_time_result =
        clock_gettime(CLOCK_REALTIME, &sleep_start_time);

    const useconds_t sleep_time =
        (useconds_t)(max_sleep_time * 1000.0 * 1000.0);
    usleep(sleep_time);

    struct timespec sleep_end_time;
    const int sleep_end_time_result =
        clock_gettime(CLOCK_REALTIME, &sleep_end_time);

    if (sleep_start_time_result == 0 && sleep_end_time_result == 0) {
      const double slept_time =
          ((double)sleep_end_time.tv_nsec - (double)sleep_start_time.tv_nsec) /
          (1000.0 * 1000.0 * 1000.0);

      f->raw_delta_time += slept_time;
    }
  }

  const clock_t calc_time_start = clock();

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
  f->fps = (uint32_t)(1.0 / f->delta_time);

  const clock_t calc_time_end = clock();
  f->additional_time = CLOCKS2DT(calc_time_start, calc_time_end);
}
