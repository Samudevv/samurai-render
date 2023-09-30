#include "shared_memory.h"
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#define SHM_FILE_NAME "/samure-shared-memory"

struct samure_shared_buffer samure_create_shared_buffer(struct wl_shm *shm,
                                                        uint32_t format,
                                                        int32_t width,
                                                        int32_t height) {
  struct samure_shared_buffer b = {0};

  const int32_t stride = width * 4;
  const int32_t size = stride * height;

  // Create shared memory file with random unique name
  char file_name[] = SHM_FILE_NAME "-XXXXXX";
  const size_t file_name_len = strlen(file_name);
  int retries = 100;

  do {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    long r = ts.tv_nsec;
    char *buf = &file_name[file_name_len - 6];
    for (int i = 0; i < 6; i++) {
      buf[i] = 'A' + (r & 15) + (r & 16) * 2;
      r >>= 5;
    }

    retries--;

    b.fd = shm_open(file_name, O_RDWR | O_CREAT | O_EXCL, 0600);
    if (b.fd >= 0) {
      shm_unlink(file_name);
      break;
    }
  } while (retries > 0 && errno == EEXIST);

  if (b.fd < 0) {
    return b;
  }

  if (ftruncate(b.fd, size) < 0) {
    close(b.fd);
    b.fd = -1;
    return b;
  }

  b.data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, b.fd, 0);
  if (b.data == MAP_FAILED) {
    close(b.fd);
    b.fd = -1;
    b.data = NULL;
    return b;
  }

  struct wl_shm_pool *pool = wl_shm_create_pool(shm, b.fd, size);
  b.buffer = wl_shm_pool_create_buffer(pool, 0, width, height, stride, format);
  wl_shm_pool_destroy(pool);

  b.width = width;
  b.height = height;
  b.format = format;

  return b;
}

void samure_destroy_shared_buffer(struct samure_shared_buffer b) {
  munmap(b.data, b.width * b.height * 4);
  close(b.fd);
  wl_buffer_destroy(b.buffer);
}

extern void samure_shared_buffer_copy(struct samure_shared_buffer dst,
                                      struct samure_shared_buffer src) {
  if (src.format == dst.format) {
    memcpy(dst.data, src.data, dst.width * dst.height * 4);
    return;
  }

  if (src.format != WL_SHM_FORMAT_XBGR8888 || dst.format != BUFFER_FORMAT ||
      dst.width != src.width || dst.height != src.height) {
    return;
  }

  uint8_t *s = (uint8_t *)src.data;
  uint8_t *d = (uint8_t *)dst.data;
  for (size_t i = 0; i < dst.width * dst.height * 4; i += 4) {
    d[i + 0] = s[i + 2]; // A
    d[i + 1] = s[i + 1]; // B
    d[i + 2] = s[i + 0]; // G
    d[i + 3] = s[i + 3]; // R
  }
}
