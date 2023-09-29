#pragma once

#include <wayland-client.h>

struct samure_shared_buffer {
  struct wl_buffer *buffer;
  void *data;
  int fd;
  int32_t width;
  int32_t height;
};

extern struct samure_shared_buffer
samure_create_shared_buffer(struct wl_shm *shm, int32_t width, int32_t height);
extern void samure_destroy_shared_buffer(struct samure_shared_buffer b);
