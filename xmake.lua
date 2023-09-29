set_project("samurai-render")

option("build_examples")
    set_default(false)
    set_showmenu(true)
option("backend_cairo")
    set_default(true)
    set_showmenu(true)
    add_defines("BACKEND_CAIRO")
option("backend_opengl")
    set_default(false)
    set_showmenu(true)
    add_defines("BACKEND_OPENGL")
option_end()

add_requires("wayland")

if get_config("backend_cairo") then
    add_requires("cairo")
end

add_rules("mode.debug", "mode.release")
target("samurai-render")
    set_kind("$(kind)")
    add_packages("wayland")
    add_options(
        "backend_cairo",
        "backend_opengl"
    )
    if get_config("backend_cairo") then
        add_packages("cairo")
    end
    if get_config("backend_opengl") then
        add_links("EGL", "wayland-egl")
    end
    add_headerfiles(
        "samure/*.h",
        "samure/wayland/*.h",
        "samure/backends/*.h"
    )
    add_files(
        "samure/*.c",
        "samure/wayland/*.c",
        "samure/backends/*.c"
    )
    if not get_config("backend_cairo") then
        remove_files("samure/backends/cairo.c")
    end
    if not get_config("backend_opengl") then
        remove_files("samure/backends/opengl.c")
    end
target_end()

if get_config("build_examples") then
    add_requires("olive.c")
    target("olivec_bounce")
        set_kind("binary")
        add_packages("wayland", "olive.c")
        add_options(
            "backend_cairo",
            "backend_opengl"
        )
        add_includedirs(os.scriptdir())
        add_deps("samurai-render")
        add_files("examples/olivec_bounce.c")
    if get_config("backend_cairo") then
        target("cairo_bounce")
            set_kind("binary")
            add_packages("wayland", "cairo")
            add_options(
                "backend_cairo",
                "backend_opengl"
            )
            add_includedirs(os.scriptdir())
            add_deps("samurai-render")
            add_files("examples/cairo_bounce.c")

        target("slurpy")
            set_kind("binary")
            add_packages("wayland", "cairo")
            add_options(
                "backend_cairo",
                "backend_opengl"
            )
            add_includedirs(os.scriptdir())
            add_deps("samurai-render")
            add_files("examples/slurpy.c")
    end
    if get_config("backend_opengl") then
        target("opengl_bounce")
            set_kind("binary")
            add_packages("wayland")
            add_options(
                "backend_cairo",
                "backend_opengl"
            )
            add_includedirs(os.scriptdir())
            add_links("GL")
            add_deps("samurai-render")
            add_files("examples/opengl_bounce.c")
    end
end
