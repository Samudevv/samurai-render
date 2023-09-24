set_project("samurai-render")

add_requires("wayland")

option("build_examples")
    set_default(false)
    set_showmenu(true)
option_end()

add_rules("mode.debug", "mode.release")
target("samurai-render")
    set_kind("static")
    add_packages("wayland")
    add_links("EGL")
    add_headerfiles(
        "src/*.h",
        "src/wayland/*.h",
        "src/backends/*.h"
    )
    add_files(
        "src/*.c",
        "src/wayland/*.c",
        "src/backends/*.c"
    )
target_end()

if get_config("build_examples") then
    add_requires("olive.c")
    target("olivec_bounce")
        set_kind("binary")
        add_packages("wayland", "olive.c")
        add_includedirs("src")
        add_deps("samurai-render")
        add_files("examples/olivec_bounce.c")
    target("opengl_bounce")
        set_kind("binary")
        add_packages("wayland")
        add_includedirs("src")
        add_links("GL")
        add_deps("samurai-render")
        add_files("examples/opengl_bounce.c")
end
