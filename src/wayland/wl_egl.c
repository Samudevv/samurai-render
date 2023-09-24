#include "wl_egl.h"
#include <dlfcn.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

wl_egl_window_create_t wl_egl_window_create = NULL;
wl_egl_window_destroy_t wl_egl_window_destroy = NULL;

void *libwayland_egl = NULL;

char *open_libwayland_egl() {
  libwayland_egl = dlopen("libwayland-egl.so.1", RTLD_LAZY);
  if (!libwayland_egl) {
    char *err = malloc(1024);
    snprintf(err, 1024, "failed to open libwayland-egl.so.1: %s",
             strerror(errno));
    return err;
  }

  wl_egl_window_create = dlsym(libwayland_egl, "wl_egl_window_create");
  if (!wl_egl_window_create) {
    char *err = malloc(1024);
    snprintf(err, 1024, "failed to load wl_egl_window_create: %s",
             strerror(errno));
    return err;
  }

  wl_egl_window_destroy = dlsym(libwayland_egl, "wl_egl_window_destroy");
  if (!wl_egl_window_destroy) {
    char *err = malloc(1024);
    snprintf(err, 1024, "failed to load wl_egl_window_destroy: %s",
             strerror(errno));
    return err;
  }

  return NULL;
}

void close_libwayland_egl() { dlclose(libwayland_egl); }
