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
ROOT_DIR = os.realpath(_MAIN_SCRIPT_DIR)

newoption
{
    trigger = "with-imgui-demo",
    description = "Include imgui_demo.cpp in the ImGui static library project."
}

include "Dependencies.lua"

local function UseDependencySet(dep)
    if dep.IncludeDirs then
        includedirs(dep.IncludeDirs)
    end

    if dep.LibDirs then
        libdirs(dep.LibDirs)
    end

    if dep.DependsOn then
        dependson(dep.DependsOn)
    end

    if dep.Links then
        links(dep.Links)
    end
end

group "Dependencies"
project "ImGui"
    location "External/imgui"
    kind "StaticLib"
    language "C++"
    cppdialect "C++23"
    staticruntime "off"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

files
    {
        -- Core
        "External/imgui/imconfig.h",
        "External/imgui/imgui.h",
        "External/imgui/imgui_internal.h",
        "External/imgui/imstb_rectpack.h",
        "External/imgui/imstb_textedit.h",
        "External/imgui/imstb_truetype.h",
        "External/imgui/imgui.cpp",
        "External/imgui/imgui_draw.cpp",
        "External/imgui/imgui_tables.cpp",
        "External/imgui/imgui_widgets.cpp",

        -- Backend (Bolt uses GLFW + OpenGL3)
        "External/imgui/backends/imgui_impl_glfw.h",
        "External/imgui/backends/imgui_impl_glfw.cpp",
        "External/imgui/backends/imgui_impl_opengl3.h",
        "External/imgui/backends/imgui_impl_opengl3.cpp",
        "External/imgui/backends/imgui_impl_opengl3_loader.h"
    }

    -- Optional demo source. Enable with --with-imgui-demo
    if _OPTIONS["with-imgui-demo"] then
        files { "External/imgui/imgui_demo.cpp" }
    end

    vpaths
    {
        ["Core/*"] =
        {
            "External/imgui/imconfig.h",
            "External/imgui/imgui.h",
            "External/imgui/imgui_internal.h",
            "External/imgui/imstb_rectpack.h",
            "External/imgui/imstb_textedit.h",
            "External/imgui/imstb_truetype.h",
            "External/imgui/imgui.cpp",
            "External/imgui/imgui_draw.cpp",
            "External/imgui/imgui_tables.cpp",
            "External/imgui/imgui_widgets.cpp"
        },
        ["Backends/*"] =
        {
            "External/imgui/backends/imgui_impl_glfw.h",
            "External/imgui/backends/imgui_impl_glfw.cpp",
            "External/imgui/backends/imgui_impl_opengl3.h",
            "External/imgui/backends/imgui_impl_opengl3.cpp",
            "External/imgui/backends/imgui_impl_opengl3_loader.h"
        },
        ["Optional/*"] = { "External/imgui/imgui_demo.cpp" }
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

    filter {}

include "premake/dependencies/glfw.lua"
include "premake/dependencies/box2d.lua"
include "premake/dependencies/glad.lua"
include "premake/dependencies/bolt_physics.lua"

group "Core"
project "Bolt-Engine"
    location "Bolt-Engine"
    kind "StaticLib"
    language "C++"
    cppdialect "C++23"
    cdialect "C17"
    staticruntime "off"

    buildoptions { "/utf-8" }

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

    buildoptions { "/utf-8" }

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

    buildoptions { "/utf-8" }

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