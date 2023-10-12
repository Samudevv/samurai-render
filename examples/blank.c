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
