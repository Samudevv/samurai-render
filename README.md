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
| Plasma       |        ✅        |       ❌        |
| Gnome/Mutter |        ❌        |       ❌        |

## Build

You need to install the following software:

+ [xmake](https//xmake.io)
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
