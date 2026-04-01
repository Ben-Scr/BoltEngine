project "Bolt-Physics"
    location "External/Bolt-Physics"
    kind "StaticLib"
    language "C++"
    cppdialect "C++23"
    cdialect "C17"
    staticruntime "off"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "External/Bolt-Physics/include/**.h",
        "External/Bolt-Physics/include/**.hpp",
        "External/Bolt-Physics/src/**.h",
        "External/Bolt-Physics/src/**.hpp",
        "External/Bolt-Physics/src/**.c",
        "External/Bolt-Physics/src/**.cpp"
    }

    includedirs
    {
        "External/Bolt-Physics/include",
        "External/Bolt-Physics/src"
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