IncludeDir = {}
IncludeDir["External"] = "External/Include"
IncludeDir["ImGui"] = "ImGui/include"
IncludeDir["Spdlog"] = "Spdlog/include"
IncludeDir["BoltEngine"] = "Bolt-Engine/Src"

LibDir = {}
LibDir["External"] = "External/Lib"

Library = {}
Library["GLFW"] = "glfw3.lib"
Library["Box2D"] = "box2d.lib"
Library["BoltPhys"] = "BoltPhys.lib"
Library["FreeType"] = "freetype.lib"
Library["OpenGL"] = "opengl32.lib"
Library["GDI32"] = "gdi32.lib"

Dependency = {}
Dependency["ImGui"] =
{
    IncludeDirs = { "%{IncludeDir.ImGui}", "%{IncludeDir.External}" },
    BuildProject = true
}

Dependency["Spdlog"] =
{
    IncludeDirs = { "%{IncludeDir.Spdlog}" },
    HeaderOnly = true
}

Dependency["EngineCore"] =
{
    IncludeDirs =
    {
        "%{IncludeDir.BoltEngine}",
        "%{IncludeDir.Spdlog}",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.External}"
    },
    LibDirs = { "%{LibDir.External}" },
    Links =
    {
        "%{Library.GLFW}",
        "%{Library.Box2D}",
        "%{Library.OpenGL}"
    }
}

Dependency["EditorRuntimeCommon"] =
{
    IncludeDirs =
    {
        "%{IncludeDir.BoltEngine}",
        "%{IncludeDir.Spdlog}",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.External}"
    },
    LibDirs = { "%{LibDir.External}" },
    Links =
    {
        "Bolt-Engine",
        "ImGui",
        "%{Library.GLFW}",
        "%{Library.OpenGL}",
        "%{Library.Box2D}",
        "%{Library.BoltPhys}"
    }
}