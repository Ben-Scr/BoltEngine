group "Runtime"
project "Bolt-Editor"
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
        "src/**.cpp",
        "src/**.h",
        "src/**.hpp",
        "icon.rc"
    }

    UseDependencySet(Dependency.EditorRuntimeCommon)
    includedirs { "src" }

    filter "system:windows"
        buildoptions { "/utf-8" }
        systemversion "latest"
        defines { "BT_PLATFORM_WINDOWS" }
        postbuildcommands {
            CopyBoltAssets,
            '{COPYFILE} "%{wks.location}External/dotnet/lib/nethost.dll" "%{cfg.targetdir}/nethost.dll"'
        }

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
