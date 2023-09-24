#include "frame_timer.h"
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

void samure_frame_time_end_frame(struct samure_frame_timer *f) {
  const clock_t end_time = clock();
  f->raw_delta_time = CLOCKS2DT(f->start_time, end_time);

  // Limit FPS
  const double max_delta_time = 1.0 / (double)f->max_fps;
  if (f->raw_delta_time < max_delta_time) {
    const clock_t sleep_start_time = clock();
    const useconds_t sleep_time =
        (useconds_t)((max_delta_time - f->raw_delta_time) * 1000000.0);
    usleep(sleep_time);
    const clock_t sleep_end_time = clock();

    f->raw_delta_time += CLOCKS2DT(sleep_start_time, sleep_end_time);
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
}
