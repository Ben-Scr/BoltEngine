workspace "Bolt"
    architecture "x64"
    startproject "Bolt-Editor"
    toolset "v145"

    configurations
    {
        "Debug",
        "Release",
        "Dist"
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include "Dependencies.lua"

local function UseDependencySet(dep)
    if dep.IncludeDirs then
        includedirs(dep.IncludeDirs)
    end

    if dep.LibDirs then
        libdirs(dep.LibDirs)
    end

    if dep.Links then
        links(dep.Links)
    end
end

group "Dependencies"
project "ImGui"
    location "ImGui"
    kind "StaticLib"
    language "C++"
    cppdialect "C++23"
    staticruntime "off"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/include/**.h"
    }

    UseDependencySet(Dependency.ImGui)

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

group "Core"
project "Bolt-Engine"
    location "Bolt-Engine"
    kind "StaticLib"
    language "C++"
    cppdialect "C++23"
    cdialect "C17"
    staticruntime "off"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    pchheader "pch.hpp"
    pchsource "Bolt-Engine/Src/pch.cpp"

    files
    {
        "%{prj.name}/Src/**.h",
        "%{prj.name}/Src/**.hpp",
        "%{prj.name}/Src/**.c",
        "%{prj.name}/Src/**.cpp"
    }

    UseDependencySet(Dependency.EngineCore)

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

group "Runtime"
project "Bolt-Editor"
    location "Bolt-Editor"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++23"
    cdialect "C17"
    staticruntime "off"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.hpp"
    }

    UseDependencySet(Dependency.EditorRuntimeCommon)

    defines
    {
        "BT_PLATFORM_WINDOWS"
    }

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

project "Bolt-Runtime"
    location "Bolt-Runtime"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++23"
    cdialect "C17"
    staticruntime "off"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "%{prj.name}/Src/**.cpp",
        "%{prj.name}/Src/**.h",
        "%{prj.name}/Src/**.hpp"
    }

    UseDependencySet(Dependency.EditorRuntimeCommon)

    links
    {
        "%{Library.FreeType}",
        "%{Library.GDI32}"
    }

    defines
    {
        "BT_PLATFORM_WINDOWS"
    }

    postbuildcommands
    {
        "{COPYDIR} %{prj.location}/Assets ../bin/" .. outputdir .. "/Bolt-Runtime/Assets"
    }

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

group ""