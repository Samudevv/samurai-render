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
if get_config("backend_opengl") then
    add_requires("libglvnd")
end

rule("wayland-protocol")
    set_extensions(".protocol")
    on_buildcmd_file(function (target, batchcmds, sourcefile, opt)
        function string.startswith(String, Start)
            return string.sub(String, 1, string.len(Start)) == Start
        end

        import("lib.detect.find_program")

        local scanner = find_program("wayland-scanner")
        if scanner == nil then
            print("error: wayland-scanner is required to generate c code for wayland-protocols")
            return
        end


        local lines = io.lines(sourcefile)
        local source = lines(0)
        local h_file = path.join("samure/wayland", lines(1))
        local c_file = path.join("samure/wayland", lines(2))

        if string.startswith(source, "https") then
            local curl = find_program("curl")
            if curl == nil then
                printf("error: curl is required to generate %s protocol\n", path.basename(sourcefile))
                return
            end
            local output = os.tmpfile()
            os.vrunv(curl, {"-o", output, source})
            source = output
        end

        batchcmds:show_progress(opt.progress, "${color.build.object}wayland-scanner.header %s", path.basename(sourcefile))
        batchcmds:vrunv("wayland-scanner", {"client-header", source, h_file})

        batchcmds:show_progress(opt.progress, "${color.build.object}wayland-scanner.source %s", path.basename(sourcefile))
        batchcmds:vrunv("wayland-scanner", {"private-code", source, c_file})

        batchcmds:add_depfiles(sourcefile)
        batchcmds:set_depmtime(os.mtime(c_file))
        batchcmds:set_depcache(target:dependfile(c_file))
    end)
rule_end()

target("wayland-protocols")
    set_kind("object")
    add_rules("wayland-protocol")
    add_files("samure/wayland/*.protocol")
target_end()

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
        add_packages("libglvnd")
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
    target("blank")
        set_kind("binary")
        add_packages("wayland")
        add_options(
            "backend_cairo",
            "backend_opengl"
        )
        add_includedirs(os.scriptdir())
        add_deps("samurai-render")
        add_files("examples/blank.c")
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

        target("image_draw")
            set_kind("binary")
            add_packages("wayland", "cairo")
            add_options(
                "backend_cairo",
                "backend_opengl"
            )
            add_includedirs(os.scriptdir())
            add_deps("samurai-render")
            add_files("examples/image_draw.c")

        target("screenshot_draw")
            set_kind("binary")
            add_packages("wayland", "cairo")
            add_options(
                "backend_cairo",
                "backend_opengl"
            )
            add_includedirs(os.scriptdir())
            add_deps("samurai-render")
            add_files("examples/screenshot_draw.c")
    end
    if get_config("backend_opengl") then
        target("opengl_bounce")
            set_kind("binary")
            add_packages("wayland", "libglvnd")
            add_options(
                "backend_cairo",
                "backend_opengl"
            )
            add_includedirs(os.scriptdir())
            add_deps("samurai-render")
            add_files("examples/opengl_bounce.c")
    end
end
