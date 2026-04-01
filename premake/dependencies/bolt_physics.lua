project "Bolt-Physics"
    location (path.join(ROOT_DIR, "External/Bolt-Physics"))
    kind "StaticLib"
    language "C++"
    cppdialect "C++23"
    cdialect "C17"
    staticruntime "off"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        path.join(ROOT_DIR, "External/Bolt-Physics/include/**.h"),
        path.join(ROOT_DIR, "External/Bolt-Physics/include/**.hpp"),
        path.join(ROOT_DIR, "External/Bolt-Physics/src/**.h"),
        path.join(ROOT_DIR, "External/Bolt-Physics/src/**.hpp"),
        path.join(ROOT_DIR, "External/Bolt-Physics/src/**.c"),
        path.join(ROOT_DIR, "External/Bolt-Physics/src/**.cpp")
    }

    includedirs
    {
        path.join(ROOT_DIR, "External/Bolt-Physics/include"),
        path.join(ROOT_DIR, "External/Bolt-Physics/src")
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