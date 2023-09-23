#include <context.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
  struct samure_context *ctx = samure_create_context(SAMURE_NO_CONTEXT_CONFIG);
  if (ctx->error_string) {
    fprintf(stderr, "%s\n", ctx->error_string);
    return EXIT_FAILURE;
  }

  puts("Successfully initialized samurai-render context");

  samure_destroy_context(ctx);

  puts("Successfully destroyed samurai-render context");

  return EXIT_SUCCESS;
}
