#pragma once

#include <wayland-client.h>

#define EGL_PLATFORM_WAYLAND_KHR 0x31D8

struct wl_egl_window;

typedef struct wl_egl_window *(*wl_egl_window_create_t)(
    struct wl_surface *surface, int width, int height);
typedef void (*wl_egl_window_destroy_t)(struct wl_egl_window *);

extern wl_egl_window_create_t wl_egl_window_create;
extern wl_egl_window_destroy_t wl_egl_window_destroy;

extern void *libwayland_egl;

extern char *open_libwayland_egl();
extern void close_libwayland_egl();
