/***********************************************************************************
 *                         This file is part of samurai-render
 *                    https://github.com/Samudevv/samurai-render
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
#include <EGL/egl.h>

struct wl_egl_window;
struct samure_layer_surface;
struct samure_backend;

// public
struct samure_opengl_config {
  int red_size;
  int green_size;
  int blue_size;
  int alpha_size;
  int samples;
  int depth_size;
  int major_version;
  int minor_version;
  int profile_mask;
  int debug;
  int color_space;
  int render_buffer;
};

// public
extern struct samure_opengl_config *samure_default_opengl_config();

// public
struct samure_opengl_surface {
  EGLSurface surface;
  struct wl_egl_window *egl_window;
};

// public
extern struct samure_opengl_surface *
samure_get_opengl_surface(struct samure_layer_surface *layer_surface);

// public
extern void samure_backend_opengl_make_context_current(
    struct samure_backend *gl, struct samure_layer_surface *layer_surface);
