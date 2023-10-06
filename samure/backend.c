#include "backend.h"

SAMURE_DEFINE_RESULT_UNWRAP(backend);

extern SAMURE_RESULT(backend) samure_create_backend(
    samure_on_layer_surface_configure_t on_layer_surface_configure,
    samure_render_start_t render_start, samure_render_end_t render_end,
    samure_destroy_t destroy,
    samure_associate_layer_surface_t associate_layer_surface,
    samure_unassociate_layer_surface_t unassociate_layer_surface) {
  SAMURE_RESULT_ALLOC(backend, b);

  b->on_layer_surface_configure = on_layer_surface_configure;
  b->render_start = render_start;
  b->render_end = render_end;
  b->destroy = destroy;
  b->associate_layer_surface = associate_layer_surface;
  b->unassociate_layer_surface = unassociate_layer_surface;

  SAMURE_RETURN_RESULT(backend, b);
}
