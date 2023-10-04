#include <samure/backends/raw.h>
#include <samure/context.h>

static void render(struct samure_context *ctx, struct samure_layer_surface *sfc,
                   struct samure_rect output_geo, double delta_time,
                   void *user_data) {

  struct samure_raw_surface *r = samure_get_raw_surface(sfc);

  uint8_t *pixels = (uint8_t *)r->buffer->data;
  for (int32_t i = 0; i < r->buffer->width * r->buffer->height * 4; i += 4) {
    pixels[i + 0] = 30;
    pixels[i + 1] = 30;
    pixels[i + 2] = 30;
    pixels[i + 3] = 30;
  }
}

int main(void) {
  struct samure_context_config cfg =
      samure_create_context_config(NULL, render, NULL, NULL);

  struct samure_context *ctx =
      SAMURE_UNWRAP(context, samure_create_context(&cfg));

  samure_context_run(ctx);

  samure_destroy_context(ctx);

  return 0;
}
