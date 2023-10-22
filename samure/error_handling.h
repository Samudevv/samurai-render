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

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SAMURE_RESULT(typename) struct samure_##typename##_result

#define SAMURE_RESULT_TYPE(typename) struct samure_##typename

#define SAMURE_DEFINE_RESULT(typename)                                         \
  SAMURE_RESULT(typename) {                                                    \
    SAMURE_RESULT_TYPE(typename) * result;                                     \
    samure_error error;                                                        \
  };                                                                           \
  inline SAMURE_RESULT_TYPE(typename) * _samure_##                             \
      typename##_unwrap(SAMURE_RESULT(typename) rs) {                          \
    assert(rs.error == SAMURE_ERROR_NONE);                                     \
    return rs.result;                                                          \
  }

#define SAMURE_DEFINE_RESULT_UNWRAP(typename)                                  \
  extern inline SAMURE_RESULT_TYPE(typename) * _samure_##                      \
      typename##_unwrap(SAMURE_RESULT(typename) rs)

#define SAMURE_RETURN(typename, value, error_code)                             \
  SAMURE_RESULT(typename)                                                      \
  _##typename##_result = {.result = value, .error = error_code};               \
  return _##typename##_result

#define SAMURE_RETURN_RESULT(typename, value)                                  \
  SAMURE_RETURN(typename, value, SAMURE_ERROR_NONE)

#define SAMURE_RETURN_ERROR(typename, error_code)                              \
  SAMURE_RETURN(typename, NULL, error_code)

#define SAMURE_DESTROY_ERROR(typename, varname, error_code)                    \
  {                                                                            \
    samure_destroy_##typename(varname);                                        \
    SAMURE_RETURN_ERROR(typename, error_code);                                 \
  }

#define SAMURE_UNWRAP(typename, result) _samure_##typename##_unwrap(result)

#define SAMURE_HAS_ERROR(result) (result.error != SAMURE_ERROR_NONE)
#define SAMURE_IS_ERROR(error_code) (error_code != SAMURE_ERROR_NONE)

#define SAMURE_CALL_AND_RETURN_ON_ERROR(result, func, ...)                     \
  if (SAMURE_HAS_ERROR(result)) {                                              \
    func return __VA_ARGS__;                                                   \
  }

#define SAMURE_RETURN_AND_PRINT_ON_ERROR(result, msg, ...)                     \
  SAMURE_CALL_AND_RETURN_ON_ERROR(result, samure_perror(msg, result.error);    \
                                  , __VA_ARGS__)

#define SAMURE_RESULT_ALLOC(typename, varname)                                 \
  SAMURE_RESULT_TYPE(typename) *varname =                                      \
      malloc(sizeof(SAMURE_RESULT_TYPE(typename)));                            \
  if (!varname) {                                                              \
    SAMURE_RETURN_ERROR(typename, SAMURE_ERROR_MEMORY);                        \
  }                                                                            \
  memset(varname, 0, sizeof(*varname))

#define SAMURE_ASSERT(err) assert(err == SAMURE_ERROR_NONE)

#define SAMURE_ERROR_NONE 0
#define SAMURE_ERROR_FAILED (1 << 0)
#define SAMURE_ERROR_NOT_IMPLEMENTED (1 << 1)
#define SAMURE_ERROR_DISPLAY_CONNECT (1 << 2)
#define SAMURE_ERROR_NO_OUTPUTS (1 << 3)
#define SAMURE_ERROR_NO_XDG_OUTPUT_MANAGER (1 << 4)
#define SAMURE_ERROR_NO_LAYER_SHELL (1 << 5)
#define SAMURE_ERROR_NO_SHM (1 << 6)
#define SAMURE_ERROR_NO_COMPOSITOR (1 << 7)
#define SAMURE_ERROR_NO_CURSOR_SHAPE_MANAGER (1 << 8)
#define SAMURE_ERROR_NO_SCREENCOPY_MANAGER (1 << 9)
#define SAMURE_ERROR_BACKEND_INIT (1 << 10)
#define SAMURE_ERROR_NO_BACKEND_SUPPORT (1 << 11)
#define SAMURE_ERROR_LAYER_SURFACE_INIT (1 << 12)
#define SAMURE_ERROR_MEMORY (1 << 13)
#define SAMURE_ERROR_SHARED_BUFFER_INIT (1 << 14)
#define SAMURE_ERROR_OPENGL_LOAD_PROC (1 << 15)
#define SAMURE_ERROR_OPENGL_DISPLAY_CONNECT (1 << 16)
#define SAMURE_ERROR_OPENGL_INITIALIZE (1 << 17)
#define SAMURE_ERROR_OPENGL_CONFIG (1 << 18)
#define SAMURE_ERROR_OPENGL_BIND_API (1 << 19)
#define SAMURE_ERROR_OPENGL_CONTEXT_INIT (1 << 20)
#define SAMURE_ERROR_OPENGL_WL_EGL_WINDOW_INIT (1 << 21)
#define SAMURE_ERROR_OPENGL_SURFACE_INIT (1 << 22)
#define SAMURE_ERROR_SHARED_BUFFER_FD_INIT (1 << 23)
#define SAMURE_ERROR_SHARED_BUFFER_TRUNCATE (1 << 24)
#define SAMURE_ERROR_SHARED_BUFFER_MMAP (1 << 25)
#define SAMURE_ERROR_SHARED_BUFFER_POOL_INIT (1 << 26)
#define SAMURE_ERROR_SHARED_BUFFER_BUFFER_INIT (1 << 27)
#define SAMURE_ERROR_FRAME_INIT (1 << 28)
#define SAMURE_ERROR_CAIRO_SURFACE_INIT (1 << 29)
#define SAMURE_ERROR_CAIRO_INIT (1 << 30)
#define SAMURE_ERROR_SURFACE_INIT ((samure_error)1 << 31)
#define SAMURE_ERROR_OUTPUT_INIT ((samure_error)1 << 32)
#define SAMURE_ERROR_CURSOR_THEME ((samure_error)1 << 33)

#define SAMURE_NUM_ERRORS 34

typedef uint64_t samure_error;

static const char *samure_strerror(samure_error error_code) {
  // clang-format off
  switch (error_code) {
  case SAMURE_ERROR_NONE:                      return "no error";
  case SAMURE_ERROR_FAILED:                    return "failed";
  case SAMURE_ERROR_NOT_IMPLEMENTED:           return "not implemented";
  case SAMURE_ERROR_DISPLAY_CONNECT:           return "display connection failed";
  case SAMURE_ERROR_NO_OUTPUTS:                return "no outputs";
  case SAMURE_ERROR_NO_XDG_OUTPUT_MANAGER:     return "no xdg output manager";
  case SAMURE_ERROR_NO_LAYER_SHELL:            return "no layer shell";
  case SAMURE_ERROR_NO_SHM:                    return "no shm";
  case SAMURE_ERROR_NO_COMPOSITOR:             return "no compositor";
  case SAMURE_ERROR_NO_CURSOR_SHAPE_MANAGER:   return "no cursor shape manager";
  case SAMURE_ERROR_NO_SCREENCOPY_MANAGER:     return "no screencopy manager";
  case SAMURE_ERROR_BACKEND_INIT:              return "backend initialization failed";
  case SAMURE_ERROR_NO_BACKEND_SUPPORT:        return "backend is not supported";
  case SAMURE_ERROR_LAYER_SURFACE_INIT:        return "layer surface initialization failed";
  case SAMURE_ERROR_MEMORY:                    return "memory allocation failed";
  case SAMURE_ERROR_SHARED_BUFFER_INIT:        return "shared buffer initialization failed";
  case SAMURE_ERROR_OPENGL_LOAD_PROC:          return "loading of functions failed";
  case SAMURE_ERROR_OPENGL_DISPLAY_CONNECT:    return "egl display connection failed";
  case SAMURE_ERROR_OPENGL_INITIALIZE:         return "egl display initialization failed";
  case SAMURE_ERROR_OPENGL_CONFIG:             return "did not find a fitting config";
  case SAMURE_ERROR_OPENGL_BIND_API:           return "binding to opengl api failed";
  case SAMURE_ERROR_OPENGL_CONTEXT_INIT:       return "egl context creation failed";
  case SAMURE_ERROR_OPENGL_WL_EGL_WINDOW_INIT: return "wayland egl window creation failed";
  case SAMURE_ERROR_OPENGL_SURFACE_INIT:       return "egl surface creation failed";
  case SAMURE_ERROR_SHARED_BUFFER_FD_INIT:     return "failed to open file descriptor";
  case SAMURE_ERROR_SHARED_BUFFER_TRUNCATE:    return "failed to truncate file";
  case SAMURE_ERROR_SHARED_BUFFER_MMAP:        return "mmap failed";
  case SAMURE_ERROR_SHARED_BUFFER_POOL_INIT:   return "shm pool initialization failed";
  case SAMURE_ERROR_SHARED_BUFFER_BUFFER_INIT: return "shm buffer initialization failed";
  case SAMURE_ERROR_FRAME_INIT:                return "screencopy initialization failed";
  case SAMURE_ERROR_CAIRO_SURFACE_INIT:        return "cairo surface initialization failed";
  case SAMURE_ERROR_CAIRO_INIT:                return "cairo initialization failed";
  case SAMURE_ERROR_SURFACE_INIT:              return "surface initialization failed";
  case SAMURE_ERROR_OUTPUT_INIT:               return "output initialization failed";
  case SAMURE_ERROR_CURSOR_THEME:              return "failed to load cursor theme";
  default:                                     return "unknown error";
  }
  // clang-format on
}

static char *samure_build_error_string(samure_error error_code) {
  if (error_code == SAMURE_ERROR_NONE) {
    return strdup(samure_strerror(SAMURE_ERROR_NONE));
  }

  char *error_string = NULL;
  size_t index = 0;

  for (samure_error i = 0; i < SAMURE_NUM_ERRORS - 1; i++) {
    const samure_error code = error_code & (1 << i);
    if (code != SAMURE_ERROR_NONE) {
      const char *err_str = samure_strerror(code);
      const size_t err_str_len = strlen(err_str);
      error_string =
          realloc(error_string, index + err_str_len + 1 + 2 * (index != 0));
      if (!error_string) {
        index = 0;
        continue;
      }
      sprintf(&error_string[index], index == 0 ? "%s" : "; %s", err_str);
      index += err_str_len + 2 * (index != 0);
    }
  }

  if (!error_string) {
    return strdup("error message failed");
  }

  return error_string;
}

static int samure_perror(const char *msg, samure_error error_code) {
  char *error_string = samure_build_error_string(error_code);
  const int rv = fprintf(stderr, "%s: %s\n", msg, error_string);
  free(error_string);
  return rv;
}
