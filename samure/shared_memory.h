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
