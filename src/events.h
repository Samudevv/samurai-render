#pragma once

#include <stdint.h>

enum samure_event_type {
  SAMURE_EVENT_LAYER_SURFACE_CONFIGURE,
  SAMURE_EVENT_POINTER_BUTTON,
  SAMURE_EVENT_POINTER_MOTION,
  SAMURE_EVENT_POINTER_ENTER,
  SAMURE_EVENT_POINTER_LEAVE,
};

struct samure_seat;
struct samure_output;

struct samure_event {
  int type;
  struct samure_seat *seat;
  struct samure_output *output;
  union {
    uint32_t button;
    uint32_t key;
  };
  uint32_t state;
  uint32_t width;
  uint32_t height;
  double x;
  double y;
};
