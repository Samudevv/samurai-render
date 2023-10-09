#pragma once
#include "wayland/cursor-shape-v1-client-protocol.h"
#include "wayland/wlr-layer-shell-unstable-v1.h"
#include "wayland/wlr-screencopy-unstable-v1.h"
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

extern void output_geometry(void *data, struct wl_output *wl_output, int32_t x,
                            int32_t y, int32_t physical_width,
                            int32_t physical_height, int32_t subpixel,
                            const char *make, const char *model,
                            int32_t transform);

extern void output_done(void *data, struct wl_output *wl_output);

extern void output_scale(void *data, struct wl_output *wl_output,
                         int32_t factor);

extern void output_name(void *data, struct wl_output *wl_output,
                        const char *name);

extern void output_description(void *data, struct wl_output *wl_output,
                               const char *description);

extern void output_mode(void *data, struct wl_output *wl_output, uint32_t flags,
                        int32_t width, int32_t height, int32_t refresh);

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

extern void keyboard_keymap(void *data, struct wl_keyboard *wl_keyboard,
                            uint32_t format, int32_t fd, uint32_t size);

extern void keyboard_enter(void *data, struct wl_keyboard *wl_keyboard,
                           uint32_t serial, struct wl_surface *surface,
                           struct wl_array *keys);

extern void keyboard_leave(void *data, struct wl_keyboard *wl_keyboard,
                           uint32_t serial, struct wl_surface *surface);

extern void keyboard_key(void *data, struct wl_keyboard *wl_keyboard,
                         uint32_t serial, uint32_t time, uint32_t key,
                         uint32_t state);

extern void keyboard_modifiers(void *data, struct wl_keyboard *wl_keyboard,
                               uint32_t serial, uint32_t mods_depressed,
                               uint32_t mods_latched, uint32_t mods_locked,
                               uint32_t group);

extern void keyboard_repeat_info(void *data, struct wl_keyboard *wl_keyboard,
                                 int32_t rate, int32_t delay);

extern void screencopy_frame_buffer(
    void *data, struct zwlr_screencopy_frame_v1 *zwlr_screencopy_frame_v1,
    uint32_t format, uint32_t width, uint32_t height, uint32_t stride);

extern void screencopy_frame_flags(
    void *data, struct zwlr_screencopy_frame_v1 *zwlr_screencopy_frame_v1,
    uint32_t flags);

extern void screencopy_frame_ready(
    void *data, struct zwlr_screencopy_frame_v1 *zwlr_screencopy_frame_v1,
    uint32_t tv_sec_hi, uint32_t tv_sec_lo, uint32_t tv_nsec);

extern void screencopy_frame_failed(
    void *data, struct zwlr_screencopy_frame_v1 *zwlr_screencopy_frame_v1);

extern void screencopy_frame_damage(
    void *data, struct zwlr_screencopy_frame_v1 *zwlr_screencopy_frame_v1,
    uint32_t x, uint32_t y, uint32_t width, uint32_t height);

extern void screencopy_frame_linux_dmabuf(
    void *data, struct zwlr_screencopy_frame_v1 *zwlr_screencopy_frame_v1,
    uint32_t format, uint32_t width, uint32_t height);

extern void screencopy_frame_buffer_done(
    void *data, struct zwlr_screencopy_frame_v1 *zwlr_screencopy_frame_v1);

extern void touch_down(void *data, struct wl_touch *wl_touch, uint32_t serial,
                       uint32_t time, struct wl_surface *surface, int32_t id,
                       wl_fixed_t x, wl_fixed_t y);

extern void touch_up(void *data, struct wl_touch *wl_touch, uint32_t serial,
                     uint32_t time, int32_t id);

extern void touch_motion(void *data, struct wl_touch *wl_touch, uint32_t time,
                         int32_t id, wl_fixed_t x, wl_fixed_t y);

extern void touch_frame(void *data, struct wl_touch *wl_touch);

extern void touch_cancel(void *data, struct wl_touch *wl_touch);

extern void touch_shape(void *data, struct wl_touch *wl_touch, int32_t id,
                        wl_fixed_t major, wl_fixed_t minor);

extern void touch_orientation(void *data, struct wl_touch *wl_touch, int32_t id,
                              wl_fixed_t orientation);

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

static struct wl_keyboard_listener keyboard_listener = {
    .keymap = keyboard_keymap,
    .enter = keyboard_enter,
    .leave = keyboard_leave,
    .key = keyboard_key,
    .modifiers = keyboard_modifiers,
    .repeat_info = keyboard_repeat_info,
};

static struct zwlr_layer_surface_v1_listener layer_surface_listener = {
    .configure = layer_surface_configure,
    .closed = layer_surface_closed,
};

static struct wl_output_listener output_listener = {
    .description = output_description,
    .done = output_done,
    .geometry = output_geometry,
    .mode = output_mode,
    .name = output_name,
    .scale = output_scale,
};

static struct zxdg_output_v1_listener xdg_output_listener = {
    .logical_position = xdg_output_logical_position,
    .logical_size = xdg_output_logical_size,
    .done = xdg_output_done,
    .name = xdg_output_name,
    .description = xdg_output_description,
};

static struct zwlr_screencopy_frame_v1_listener screencopy_frame_listener = {
    .buffer = screencopy_frame_buffer,
    .buffer_done = screencopy_frame_buffer_done,
    .damage = screencopy_frame_damage,
    .failed = screencopy_frame_failed,
    .flags = screencopy_frame_flags,
    .linux_dmabuf = screencopy_frame_linux_dmabuf,
    .ready = screencopy_frame_ready,
};

static struct wl_touch_listener touch_listener = {
    .cancel = touch_cancel,
    .down = touch_down,
    .frame = touch_frame,
    .motion = touch_motion,
    .orientation = touch_orientation,
    .shape = touch_shape,
    .up = touch_up,
};

struct samure_callback_data {
  struct samure_context *ctx;
  void *data;
};

extern struct samure_callback_data *
samure_create_callback_data(struct samure_context *ctx, void *data);
