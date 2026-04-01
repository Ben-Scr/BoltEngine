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
        path.join(ROOT_DIR, "External/glfw/src/**.h"),
        path.join(ROOT_DIR, "External/glfw/src/**.c")
    }

    removefiles
    {
        path.join(ROOT_DIR, "External/glfw/src/cocoa_*.m"),
        path.join(ROOT_DIR, "External/glfw/src/cocoa_*.h"),
        path.join(ROOT_DIR, "External/glfw/src/nsgl_context.*"),
        path.join(ROOT_DIR, "External/glfw/src/posix_*.*"),
        path.join(ROOT_DIR, "External/glfw/src/wl_*.*"),
        path.join(ROOT_DIR, "External/glfw/src/x11_*.*"),
        path.join(ROOT_DIR, "External/glfw/src/glx_context.*"),
        path.join(ROOT_DIR, "External/glfw/src/linux_*.*"),
        path.join(ROOT_DIR, "External/glfw/src/osmesa_context.*")
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