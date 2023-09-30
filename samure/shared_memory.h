#pragma once

#include <wayland-client.h>

#include "error_handling.h"

#define SAMURE_BUFFER_FORMAT WL_SHM_FORMAT_ARGB8888

struct samure_shared_buffer {
  struct wl_buffer *buffer;
  void *data;
  int fd;
  int32_t width;
  int32_t height;
  uint32_t format;
};

SAMURE_DEFINE_RESULT(shared_buffer);

extern SAMURE_RESULT(shared_buffer)
    samure_create_shared_buffer(struct wl_shm *shm, uint32_t format,
                                int32_t width, int32_t height);
extern void samure_destroy_shared_buffer(struct samure_shared_buffer *b);
extern samure_error samure_shared_buffer_copy(struct samure_shared_buffer *dst,
                                              struct samure_shared_buffer *src);
