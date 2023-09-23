#include <stdlib.h>
#include <string.h>

#include "callbacks.h"
#include "context.h"

void registry_global(void *data, struct wl_registry *registry, uint32_t name,
                     const char *interface, uint32_t version) {
  struct samure_context *ctx = (struct samure_context *)data;

  if (strcmp(interface, wl_shm_interface.name) == 0) {
    ctx->shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
  } else if (strcmp(interface, wl_seat_interface.name) == 0) {
    struct wl_seat *seat =
        wl_registry_bind(registry, name, &wl_seat_interface, 1);

    ctx->num_seats++;
    ctx->seats =
        realloc(ctx->seats, ctx->num_seats * sizeof(struct samure_seat));
    ctx->seats[ctx->num_seats - 1] = samure_create_seat(seat);
  } else if (strcmp(interface, wl_compositor_interface.name) == 0) {
    ctx->compositor =
        wl_registry_bind(registry, name, &wl_compositor_interface, 1);
  } else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
    ctx->layer_shell =
        wl_registry_bind(registry, name, &zwlr_layer_shell_v1_interface, 1);
  } else if (strcmp(interface, wl_output_interface.name) == 0) {
    struct wl_output *output =
        wl_registry_bind(registry, name, &wl_output_interface, 3);

    ctx->num_outputs++;
    ctx->outputs =
        realloc(ctx->outputs, ctx->num_outputs * sizeof(struct samure_output));
    ctx->outputs[ctx->num_outputs - 1] = samure_create_output(output);

  } else if (strcmp(interface, zxdg_output_manager_v1_interface.name) == 0) {
    ctx->output_manager =
        wl_registry_bind(registry, name, &zxdg_output_manager_v1_interface, 2);
  }
}

void registry_global_remove(void *data, struct wl_registry *registry,
                            uint32_t name) {}

void seat_capabilities(void *data, struct wl_seat *seat,
                       uint32_t capabilities) {
  struct samure_seat *s = (struct samure_seat *)data;

  if (capabilities & WL_SEAT_CAPABILITY_POINTER) {
    s->pointer = wl_seat_get_pointer(seat);
  }
  if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD) {
    s->keyboard = wl_seat_get_keyboard(seat);
  }
  if (capabilities & WL_SEAT_CAPABILITY_TOUCH) {
    s->touch = wl_seat_get_touch(seat);
  }
}

void seat_name(void *data, struct wl_seat *wl_seat, const char *name) {
  struct samure_seat *s = (struct samure_seat *)data;

  s->name = strdup(name);
}

void pointer_enter(void *data, struct wl_pointer *pointer, uint32_t serial,
                   struct wl_surface *surface, wl_fixed_t surface_x,
                   wl_fixed_t surface_y) {}

void pointer_leave(void *data, struct wl_pointer *pointer, uint32_t serial,
                   struct wl_surface *surface) {}

void pointer_motion(void *data, struct wl_pointer *pointer, uint32_t time,
                    wl_fixed_t surface_x, wl_fixed_t surface_y) {}

void pointer_button(void *data, struct wl_pointer *pointer, uint32_t serial,
                    uint32_t time, uint32_t button, uint32_t state) {}

void pointer_axis(void *data, struct wl_pointer *wl_pointer, uint32_t time,
                  uint32_t axis, wl_fixed_t value) {}

void layer_surface_configure(void *data,
                             struct zwlr_layer_surface_v1 *layer_surface,
                             uint32_t serial, uint32_t width, uint32_t height) {
  zwlr_layer_surface_v1_ack_configure(layer_surface, serial);
}

void layer_surface_closed(void *data,
                          struct zwlr_layer_surface_v1 *layer_surface) {}

void xdg_output_logical_position(void *data,
                                 struct zxdg_output_v1 *zxdg_output_v1,
                                 int32_t x, int32_t y) {
  struct samure_output *o = data;
  o->logical_position.x = x;
  o->logical_position.y = y;
}

void xdg_output_logical_size(void *data, struct zxdg_output_v1 *zxdg_output_v1,
                             int32_t width, int32_t height) {
  struct samure_output *o = data;
  o->logical_size.width = width;
  o->logical_size.height = height;
}

void xdg_output_done(void *data, struct zxdg_output_v1 *zxdg_output_v1) {}

void xdg_output_name(void *data, struct zxdg_output_v1 *zxdg_output_v1,
                     const char *name) {
  struct samure_output *o = data;
  o->name = strdup(name);
}

void xdg_output_description(void *data, struct zxdg_output_v1 *zxdg_output_v1,
                            const char *description) {}
