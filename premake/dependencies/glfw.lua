project "GLFW"
    location "External/glfw"
    kind "StaticLib"
    language "C"
    staticruntime "off"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "External/glfw/include/GLFW/glfw3.h",
        "External/glfw/include/GLFW/glfw3native.h",
        "External/glfw/src/**.h",
        "External/glfw/src/**.c"
    }

    removefiles
    {
        "External/glfw/src/cocoa_*.m",
        "External/glfw/src/cocoa_*.h",
        "External/glfw/src/nsgl_context.*",
        "External/glfw/src/posix_*.*",
        "External/glfw/src/wl_*.*",
        "External/glfw/src/x11_*.*",
        "External/glfw/src/glx_context.*",
        "External/glfw/src/linux_*.*",
        "External/glfw/src/null_*.*",
        "External/glfw/src/osmesa_context.*"
    }

    includedirs
    {
        "External/glfw/include",
        "External/glfw/src"
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