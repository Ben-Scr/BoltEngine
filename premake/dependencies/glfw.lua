project "GLFW"
    location (path.join(ROOT_DIR, "External/glfw"))
    kind "StaticLib"
    language "C"
    staticruntime "off"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    local function glfwFile(relpath)
        return path.join(ROOT_DIR, "External/glfw", relpath)
    end

    local glfwFiles =
    {
        glfwFile("include/GLFW/glfw3.h"),
        glfwFile("include/GLFW/glfw3native.h"),

        glfwFile("src/internal.h"),
        glfwFile("src/platform.h"),
        glfwFile("src/mappings.h"),
        glfwFile("src/win32_platform.h"),
        glfwFile("src/win32_joystick.h"),
        glfwFile("src/wgl_context.h"),
        glfwFile("src/egl_context.h"),
        glfwFile("src/osmesa_context.h"),

        glfwFile("src/context.c"),
        glfwFile("src/init.c"),
        glfwFile("src/input.c"),
        glfwFile("src/monitor.c"),
        glfwFile("src/vulkan.c"),
        glfwFile("src/window.c"),
        glfwFile("src/win32_init.c"),
        glfwFile("src/win32_joystick.c"),
        glfwFile("src/win32_monitor.c"),
        glfwFile("src/win32_time.c"),
        glfwFile("src/win32_thread.c"),
        glfwFile("src/win32_window.c"),
        glfwFile("src/wgl_context.c"),
        glfwFile("src/egl_context.c"),
        glfwFile("src/osmesa_context.c")
    }

    -- GLFW 3.4+ split module loading into dedicated source units.
    -- Keep this compatible with both older and newer submodule snapshots.
    local optionalSources =
    {
        "src/win32_module.c",
        "src/null_init.c",
        "src/null_monitor.c",
        "src/null_window.c",
        "src/null_joystick.c"
    }

    for _, relpath in ipairs(optionalSources) do
        local fullpath = glfwFile(relpath)
        if os.isfile(fullpath) then
            table.insert(glfwFiles, fullpath)
        end
    end

    files(glfwFiles)

    includedirs
    {
        glfwFile("include"),
        glfwFile("src")
    }

    defines
    {
        "_GLFW_WIN32",
        "_CRT_SECURE_NO_WARNINGS"
    }

    filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
        runtime "Debug"
        symbols "On"
        defines { "_DEBUG" }

    filter "configurations:Release"
        runtime "Release"
        optimize "On"
        symbols "On"
        defines { "NDEBUG" }

    filter "configurations:Dist"
        runtime "Release"
        optimize "Full"
        symbols "Off"
        defines { "NDEBUG" }