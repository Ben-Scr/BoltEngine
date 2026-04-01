project "GLFW"
    location (path.join(ROOT_DIR, "External/glfw"))
    kind "StaticLib"
    language "C"
    staticruntime "off"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        path.join(ROOT_DIR, "External/glfw/include/GLFW/glfw3.h"),
        path.join(ROOT_DIR, "External/glfw/include/GLFW/glfw3native.h"),

        path.join(ROOT_DIR, "External/glfw/src/internal.h"),
        path.join(ROOT_DIR, "External/glfw/src/platform.h"),
        path.join(ROOT_DIR, "External/glfw/src/mappings.h"),
        path.join(ROOT_DIR, "External/glfw/src/win32_platform.h"),
        path.join(ROOT_DIR, "External/glfw/src/win32_joystick.h"),
        path.join(ROOT_DIR, "External/glfw/src/wgl_context.h"),
        path.join(ROOT_DIR, "External/glfw/src/egl_context.h"),
        path.join(ROOT_DIR, "External/glfw/src/osmesa_context.h"),

        path.join(ROOT_DIR, "External/glfw/src/context.c"),
        path.join(ROOT_DIR, "External/glfw/src/init.c"),
        path.join(ROOT_DIR, "External/glfw/src/input.c"),
        path.join(ROOT_DIR, "External/glfw/src/monitor.c"),
        path.join(ROOT_DIR, "External/glfw/src/vulkan.c"),
        path.join(ROOT_DIR, "External/glfw/src/window.c"),
        path.join(ROOT_DIR, "External/glfw/src/win32_init.c"),
        path.join(ROOT_DIR, "External/glfw/src/win32_joystick.c"),
        path.join(ROOT_DIR, "External/glfw/src/win32_monitor.c"),
        path.join(ROOT_DIR, "External/glfw/src/win32_time.c"),
        path.join(ROOT_DIR, "External/glfw/src/win32_thread.c"),
        path.join(ROOT_DIR, "External/glfw/src/win32_window.c"),
        path.join(ROOT_DIR, "External/glfw/src/wgl_context.c"),
        path.join(ROOT_DIR, "External/glfw/src/egl_context.c"),
        path.join(ROOT_DIR, "External/glfw/src/osmesa_context.c")
    }

    includedirs
    {
        path.join(ROOT_DIR, "External/glfw/include"),
        path.join(ROOT_DIR, "External/glfw/src")
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