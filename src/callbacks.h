#pragma once
#include "wayland/wlr-layer-shell-unstable-v1.h"
#include "wayland/xdg-output-unstable-v1.h"
#include <wayland-client.h>

extern void registry_global(void *data, struct wl_registry *registry,
                            uint32_t name, const char *interface,
                            uint32_t version);

extern void registry_global_remove(void *data, struct wl_registry *registry,
                                   uint32_t name);

extern void seat_capabilities(void *data, struct wl_seat *seat,
                              uint32_t capabilities);

extern void seat_name(void *data, struct wl_seat *wl_seat, const char *name);

extern void pointer_enter(void *data, struct wl_pointer *pointer,
                          uint32_t serial, struct wl_surface *surface,
                          wl_fixed_t surface_x, wl_fixed_t surface_y);

extern void pointer_leave(void *data, struct wl_pointer *pointer,
                          uint32_t serial, struct wl_surface *surface);

extern void pointer_motion(void *data, struct wl_pointer *pointer,
                           uint32_t time, wl_fixed_t surface_x,
                           wl_fixed_t surface_y);

extern void pointer_button(void *data, struct wl_pointer *pointer,
                           uint32_t serial, uint32_t time, uint32_t button,
                           uint32_t state);

extern void pointer_axis(void *data, struct wl_pointer *wl_pointer,
                         uint32_t time, uint32_t axis, wl_fixed_t value);

extern void layer_surface_configure(void *data,
                                    struct zwlr_layer_surface_v1 *layer_surface,
                                    uint32_t serial, uint32_t width,
                                    uint32_t height);

extern void layer_surface_closed(void *data,
                                 struct zwlr_layer_surface_v1 *layer_surface);

extern void xdg_output_logical_position(void *data,
                                        struct zxdg_output_v1 *zxdg_output_v1,
                                        int32_t x, int32_t y);

extern void xdg_output_logical_size(void *data,
                                    struct zxdg_output_v1 *zxdg_output_v1,
                                    int32_t width, int32_t height);

extern void xdg_output_done(void *data, struct zxdg_output_v1 *zxdg_output_v1);

extern void xdg_output_name(void *data, struct zxdg_output_v1 *zxdg_output_v1,
                            const char *name);

extern void xdg_output_description(void *data,
                                   struct zxdg_output_v1 *zxdg_output_v1,
                                   const char *description);

extern void surface_frame(void *data, struct wl_callback *wl_callback,
                          uint32_t callback_data);

static struct wl_registry_listener registry_listener = {
    .global = registry_global,
    .global_remove = registry_global_remove,
};

static struct wl_seat_listener seat_listener = {
    .capabilities = seat_capabilities,
    .name = seat_name,
};

static struct wl_pointer_listener pointer_listener = {
    .enter = pointer_enter,
    .leave = pointer_leave,
    .motion = pointer_motion,
    .button = pointer_button,
    .axis = pointer_axis,
};

static struct zwlr_layer_surface_v1_listener layer_surface_listener = {
    .configure = layer_surface_configure,
    .closed = layer_surface_closed,
};

static struct zxdg_output_v1_listener xdg_output_listener = {
    .logical_position = xdg_output_logical_position,
    .logical_size = xdg_output_logical_size,
    .done = xdg_output_done,
    .name = xdg_output_name,
    .description = xdg_output_description,
};

static struct wl_callback_listener surface_frame_listener = {
    .done = surface_frame,
};

struct samure_callback_data {
  struct samure_context *ctx;
  void *data;
};

extern struct samure_callback_data *
samure_create_callback_data(struct samure_context *ctx, void *data);