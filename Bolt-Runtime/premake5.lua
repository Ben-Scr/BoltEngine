project "Bolt-Runtime"
    location "."
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    cdialect "C17"
    staticruntime "off"

    targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
    objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "Src/**.cpp",
        "Src/**.h",
        "Src/**.hpp",
        "icon.rc"
    }

    UseDependencySet(Dependency.EditorRuntimeCommon)

    postbuildcommands
    {
        "{COPYDIR} %{prj.location}/Assets %{cfg.targetdir}/Assets"
    }

    filter "system:windows"
        buildoptions { "/utf-8" }
        systemversion "latest"
        links { "%{Library.GDI32}" }
        defines { "BT_PLATFORM_WINDOWS" }

    filter "system:linux"
        defines { "BT_PLATFORM_LINUX" }

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