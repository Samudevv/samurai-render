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
    add_headerfiles("src/*.h")
    add_files("src/*.c")

if get_config("build_examples") then
    target("blank")
        set_kind("binary")
        add_deps("samurai-render")
        add_files("examples/blank.c")
end
