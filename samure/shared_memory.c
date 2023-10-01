#include "shared_memory.h"
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#define SHM_FILE_NAME "/samure-shared-memory"

SAMURE_DEFINE_RESULT_UNWRAP(shared_buffer);

SAMURE_RESULT(shared_buffer)
samure_create_shared_buffer(struct wl_shm *shm, uint32_t format, int32_t width,
                            int32_t height) {
  SAMURE_RESULT_ALLOC(shared_buffer, b);

  b->width = width;
  b->height = height;
  b->format = format;

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

    b->fd = shm_open(file_name, O_RDWR | O_CREAT | O_EXCL, 0600);
    if (b->fd >= 0) {
      shm_unlink(file_name);
      break;
    }
  } while (retries > 0 && errno == EEXIST);

  if (b->fd < 0) {
    SAMURE_DESTROY_ERROR(shared_buffer, b, SAMURE_ERROR_SHARED_BUFFER_FD_INIT);
  }

  if (ftruncate(b->fd, size) < 0) {
    SAMURE_DESTROY_ERROR(shared_buffer, b, SAMURE_ERROR_SHARED_BUFFER_TRUNCATE);
  }

  b->data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, b->fd, 0);
  if (b->data == MAP_FAILED) {
    b->data = NULL;
    SAMURE_DESTROY_ERROR(shared_buffer, b, SAMURE_ERROR_SHARED_BUFFER_MMAP);
  }

  struct wl_shm_pool *pool = wl_shm_create_pool(shm, b->fd, size);
  if (!pool) {
    SAMURE_DESTROY_ERROR(shared_buffer, b,
                         SAMURE_ERROR_SHARED_BUFFER_POOL_INIT);
  }
  b->buffer = wl_shm_pool_create_buffer(pool, 0, width, height, stride, format);
  wl_shm_pool_destroy(pool);
  if (!b->buffer) {
    SAMURE_DESTROY_ERROR(shared_buffer, b,
                         SAMURE_ERROR_SHARED_BUFFER_BUFFER_INIT);
  }

  SAMURE_RETURN_RESULT(shared_buffer, b);
}

void samure_destroy_shared_buffer(struct samure_shared_buffer *b) {
  if (b->data)
    munmap(b->data, b->width * b->height * 4);
  if (b->fd >= 0)
    close(b->fd);
  if (b->buffer)
    wl_buffer_destroy(b->buffer);
  free(b);
}

extern samure_error
samure_shared_buffer_copy(struct samure_shared_buffer *dst,
                          struct samure_shared_buffer *src) {
  if (src->format == dst->format && dst->width == src->width &&
      dst->height == src->height) {
    memcpy(dst->data, src->data, dst->width * dst->height * 4);
    return SAMURE_ERROR_NONE;
  }

  if (src->format != WL_SHM_FORMAT_XBGR8888 ||
      dst->format != SAMURE_BUFFER_FORMAT || dst->width != src->width ||
      dst->height != src->height) {
    return SAMURE_ERROR_FAILED;
  }

  uint8_t *s = (uint8_t *)src->data;
  uint8_t *d = (uint8_t *)dst->data;
  for (size_t i = 0; i < dst->width * dst->height * 4; i += 4) {
    d[i + 0] = s[i + 2]; // A
    d[i + 1] = s[i + 1]; // B
    d[i + 2] = s[i + 0]; // G
    d[i + 3] = s[i + 3]; // R
  }

  return SAMURE_ERROR_NONE;
}
