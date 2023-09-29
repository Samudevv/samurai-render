#include "opengl.h"
#include "../context.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GL_ERR(format)                                                         \
  gl->error_string = malloc(1024);                                             \
  snprintf(gl->error_string, 1024, "%s", format);                              \
  free(cfg);                                                                   \
  if (gl->context) {                                                           \
    eglDestroyContext(gl->display, gl->context);                               \
  }                                                                            \
  if (gl->display) {                                                           \
    eglTerminate(gl->display);                                                 \
  }
#define GL_ERR_F(format, ...)                                                  \
  char error_buffer[1024];                                                     \
  snprintf(error_buffer, 1024, format, __VA_ARGS__);                           \
  GL_ERR(error_buffer);

eglGetPlatformDisplayEXT_t eglGetPlatformDisplayEXT = NULL;
eglCreatePlatformWindowSurfaceEXT_t eglCreatePlatformWindowSurfaceEXT = NULL;

struct samure_opengl_config *samure_default_opengl_config() {
  struct samure_opengl_config *cfg =
      malloc(sizeof(struct samure_opengl_config));
  memset(cfg, 0, sizeof(struct samure_opengl_config));

  cfg->red_size = 8;
  cfg->green_size = 8;
  cfg->blue_size = 8;
  cfg->alpha_size = 8;
  cfg->major_version = 1;
  cfg->profile_mask = EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT;
  cfg->color_space = EGL_GL_COLORSPACE_LINEAR;
  cfg->render_buffer = EGL_BACK_BUFFER;

  return cfg;
}

struct samure_backend_opengl *
samure_init_backend_opengl(struct samure_context *ctx,
                           struct samure_opengl_config *cfg) {
  if (cfg == NULL) {
    cfg = samure_default_opengl_config();
  }

  struct samure_backend_opengl *gl =
      malloc(sizeof(struct samure_backend_opengl));
  memset(gl, 0, sizeof(struct samure_backend_opengl));

  eglGetPlatformDisplayEXT =
      (eglGetPlatformDisplayEXT_t)eglGetProcAddress("eglGetPlatformDisplayEXT");
  if (!eglGetPlatformDisplayEXT) {
    GL_ERR("failed to load eglGetPlatformDisplayEXT");
    return gl;
  }

  eglCreatePlatformWindowSurfaceEXT =
      (eglCreatePlatformWindowSurfaceEXT_t)eglGetProcAddress(
          "eglCreatePlatformWindowSurfaceEXT");
  if (!eglCreatePlatformWindowSurfaceEXT) {
    GL_ERR("failed to load eglCreatePlatformWindowSurfaceEXT");
    return gl;
  }

  gl->display =
      eglGetPlatformDisplayEXT(EGL_PLATFORM_WAYLAND_KHR, ctx->display, NULL);
  if (gl->display == EGL_NO_DISPLAY) {
    GL_ERR("failed to get egl display connection");
    return gl;
  }

  if (eglInitialize(gl->display, NULL, NULL) != EGL_TRUE) {
    GL_ERR("failed to initialize egl");
    return gl;
  }

  // clang-format off
  const EGLint config_attributes[] = {
      EGL_RED_SIZE,   cfg->red_size,
      EGL_BLUE_SIZE,  cfg->blue_size,
      EGL_GREEN_SIZE, cfg->green_size,
      EGL_ALPHA_SIZE, cfg->alpha_size,
      EGL_DEPTH_SIZE, cfg->depth_size,
      EGL_SAMPLES,    cfg->samples,
      EGL_CONFORMANT, EGL_OPENGL_BIT,
      EGL_NONE,       EGL_NONE,
  };

  const EGLint context_attributes[] = {
    EGL_CONTEXT_MAJOR_VERSION,       cfg->major_version,
    EGL_CONTEXT_MINOR_VERSION,       cfg->minor_version,
    EGL_CONTEXT_OPENGL_PROFILE_MASK, cfg->profile_mask,
    EGL_CONTEXT_OPENGL_DEBUG,        cfg->debug,
    EGL_NONE,                        EGL_NONE,
  };

  // clang-format on

  EGLint num_config;
  EGLConfig config;
  if (eglChooseConfig(gl->display, config_attributes, &config, 1,
                      &num_config) != EGL_TRUE) {
    GL_ERR("failed to choose egl config");
    return gl;
  }
  if (num_config == 0) {
    GL_ERR("did not find any egl configs");
    return gl;
  }

  if (eglBindAPI(EGL_OPENGL_API) != EGL_TRUE) {
    GL_ERR("failed to bind opengl api");
    return gl;
  }

  gl->context =
      eglCreateContext(gl->display, config, EGL_NO_CONTEXT, context_attributes);
  if (gl->context == EGL_NO_CONTEXT) {
    GL_ERR("failed to create egl context");
    return gl;
  }

  gl->config = config;
  gl->cfg = cfg;

  gl->base.destroy = samure_destroy_backend_opengl;
  gl->base.render_start = samure_backend_opengl_render_start;
  gl->base.render_end = samure_backend_opengl_render_end;
  gl->base.associate_layer_surface =
      samure_backend_opengl_associate_layer_surface;
  gl->base.unassociate_layer_surface =
      samure_backend_opengl_unassociate_layer_surface;

  return gl;
}

void samure_destroy_backend_opengl(struct samure_context *ctx,
                                   struct samure_backend *backend) {
  struct samure_backend_opengl *gl = (struct samure_backend_opengl *)backend;

  eglDestroyContext(gl->display, gl->context);

  eglTerminate(gl->display);
  free(gl->error_string);
  free(gl->cfg);
  free(gl);
}

void samure_backend_opengl_render_start(
    struct samure_output *output, struct samure_layer_surface *layer_surface,
    struct samure_context *ctx, struct samure_backend *backend) {
  samure_backend_opengl_make_context_current(
      (struct samure_backend_opengl *)backend, layer_surface);
}

void samure_backend_opengl_render_end(
    struct samure_output *output, struct samure_layer_surface *layer_surface,
    struct samure_context *ctx, struct samure_backend *backend) {
  struct samure_backend_opengl *gl = (struct samure_backend_opengl *)backend;
  struct samure_opengl_surface *s =
      (struct samure_opengl_surface *)layer_surface->backend_data;
  eglSwapBuffers(gl->display, s->surface);
}

void samure_backend_opengl_associate_layer_surface(
    struct samure_context *ctx, struct samure_backend *backend,
    struct samure_output *output, struct samure_layer_surface *sfc) {
  struct samure_backend_opengl *gl = (struct samure_backend_opengl *)backend;
  struct samure_opengl_surface *s =
      malloc(sizeof(struct samure_opengl_surface));
  memset(s, 0, sizeof(struct samure_opengl_surface));

  sfc->backend_data = s;

  s->egl_window =
      wl_egl_window_create(sfc->surface, output->geo.w, output->geo.h);
  if (!s->egl_window) {
    /* TODO: handle error*/
    return;
  }

  // clang-format off
  const EGLint surface_attributes[] = {
    EGL_GL_COLORSPACE, gl->cfg->color_space,
    EGL_RENDER_BUFFER, gl->cfg->render_buffer,
    EGL_NONE,          EGL_NONE,
  };
  // clang-format on

  s->surface = eglCreatePlatformWindowSurfaceEXT(
      gl->display, gl->config, (EGLNativeWindowType)s->egl_window,
      surface_attributes);
  if (s->surface == EGL_NO_SURFACE) {
    /* TODO: handle error*/
  }
}

void samure_backend_opengl_unassociate_layer_surface(
    struct samure_context *ctx, struct samure_backend *backend,
    struct samure_output *output, struct samure_layer_surface *layer_surface) {
  struct samure_backend_opengl *gl = (struct samure_backend_opengl *)backend;
  struct samure_opengl_surface *s =
      (struct samure_opengl_surface *)layer_surface->backend_data;

  eglDestroySurface(gl->display, s->surface);
  wl_egl_window_destroy(s->egl_window);
  free(s);
  layer_surface->backend_data = NULL;
}

struct samure_backend_opengl *
samure_get_backend_opengl(struct samure_context *ctx) {
  return (struct samure_backend_opengl *)ctx->backend;
}

void samure_backend_opengl_make_context_current(
    struct samure_backend_opengl *gl,
    struct samure_layer_surface *layer_surface) {
  if (layer_surface) {
    struct samure_opengl_surface *s =
        (struct samure_opengl_surface *)layer_surface->backend_data;
    eglMakeCurrent(gl->display, s->surface, s->surface, gl->context);
  } else {
    eglMakeCurrent(gl->display, EGL_NO_SURFACE, EGL_NO_SURFACE, gl->context);
  }
}
