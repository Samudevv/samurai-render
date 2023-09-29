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
