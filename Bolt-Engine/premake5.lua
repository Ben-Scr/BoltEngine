group "Core"
project "Bolt-Engine"
    location "."
    kind "StaticLib"
    language "C++"
    cppdialect "C++23"
    cdialect "C17"
    staticruntime "off"

    targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
    objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")

    pchheader "pch.hpp"
    pchsource "Src/pch.cpp"

    files
    {
        "Src/**.h",
        "Src/**.hpp",
        "Src/**.c",
        "Src/**.cpp",
        "src/Core/**.h",
        "src/Core/**.hpp",
        "src/Core/**.cpp",
        "src/Systems/ImGuiEditorSystem.hpp",
        "src/Systems/ImGuiEditorSystem.cpp"
    }

    UseDependencySet(Dependency.EngineCore)

    filter "system:windows"
        buildoptions { "/utf-8" }
        systemversion "latest"
        defines { "BT_PLATFORM_WINDOWS", "BT_TRACK_MEMORY" }

    filter "system:linux"
        pic "On"
        defines { "BT_PLATFORM_LINUX", "BT_TRACK_MEMORY" }

    defines
    {
        "BT_PLATFORM_WINDOWS",
        "BT_TRACK_MEMORY"
    }

    filter "files:**/glad.c"
        flags { "NoPCH" }

    filter "files:**/ImGuiEditorSystem.cpp"
        flags { "NoPCH" }

    filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
        runtime "Debug"
        symbols "On"
        defines { "BT_DEBUG", "_DEBUG" }

    filter "configurations:Release"
        runtime "Release"
        optimize "On"
        symbols "On"
        defines { "BT_RELEASE", "NDEBUG" }

    filter "configurations:Dist"
        runtime "Release"
        optimize "Full"
        symbols "Off"
        defines { "BT_RELEASE", "NDEBUG" }