# SamuraiRender

Easy to use library to render into the Layer Shell of wayland compositors.

There are also [bindings to go](https://github.com/PucklaJ/samurai-render-go) available.

## SamuraiSelect

There is a proper application written using this library: [SamuraiSelect](https://github.com/PucklaJ/samurai-select). It is a tool for selecting a region on the screen(s) and (optionally) taking a screenshot.

## Compositor Support

All wayland compositors that implement the wlr layer shell protocol will be supported. To take screenshots the wlr screencopy protocol needs to be implemented. This library has been tested on the following compositors:

| Compositor   | wlr layer shell | wlr screencopy |
| ------------ | :-------------: | :------------: |
| Hyprland     |        ✅        |       ✅        |
| Sway         |        ✅        |       ✅        |
| KDE Plasma   |        ✅        |       ❌        |
| Gnome/Mutter |        ❌        |       ❌        |

## Build

You need to install the following software:

+ [xmake](https://xmake.io)
+ [Wayland Client Library](https://gitlab.freedesktop.org/wayland/wayland)
+ [Cairo](https://cairographics.org/) (optional)
+ [OpenGL Vendor Library](https://gitlab.freedesktop.org/glvnd/libglvnd) (optional)

If some dependencies are missing xmake will install them for you using its own package manager, but it is better to install them system-wide, because the packages might be outdated. On Arch Linux you can install these dependencies like so:

```
sudo pacman -S --needed xmake wayland cairo libglvnd
```

After that you can build the library either with or without examples:

```
xmake f --build_examples=y --backend_opengl=y --backend_cairo=y
xmake
xmake run cairo_bounce # left click to close it
```

The binaries will be under the specific platform folder in the build folder.

## Getting Started

```c
#include <samure/backends/raw.h>
#include <samure/context.h>

static void render(struct samure_context *ctx, struct samure_layer_surface *sfc,
                   struct samure_rect output_geo, void *user_data) {

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
```

This example just renders a transparent white overlay over all screens. To take a look at more complex examples that use different backends see the [examples folder](./examples/).
