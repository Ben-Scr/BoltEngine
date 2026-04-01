IncludeDir = {}
IncludeDir["ExternalRoot"] = "External"
IncludeDir["ImGui"] = "External/imgui"
IncludeDir["Spdlog"] = "External/spdlog/include"
IncludeDir["GLFW"] = "External/glfw/include"
IncludeDir["Box2D"] = "External/box2d/include"
IncludeDir["GLM"] = "External/glm"
IncludeDir["EnTT"] = "External/entt/src"
IncludeDir["STB"] = "External/stb"
IncludeDir["MagicEnum"] = "External/magic_enum/include"
IncludeDir["MiniAudio"] = "External/miniaudio"
IncludeDir["Cereal"] = "External/cereal/include"
IncludeDir["Glad"] = "External/glad/include"
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
    IncludeDirs = { "%{IncludeDir.ImGui}", "External/imgui/include", "%{IncludeDir.GLFW}", "%{IncludeDir.Glad}" },
    BuildProject = true
}

Dependency["Spdlog"] =
{
    IncludeDirs = { "%{IncludeDir.Spdlog}" },
    HeaderOnly = true
}

Dependency["ExternalIncludes"] =
{
    IncludeDirs =
    {
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.Box2D}",
        "%{IncludeDir.GLM}",
        "%{IncludeDir.EnTT}",
        "%{IncludeDir.STB}",
        "%{IncludeDir.MagicEnum}",
        "%{IncludeDir.MiniAudio}",
        "%{IncludeDir.Cereal}",
        "%{IncludeDir.Glad}"
    }
}

Dependency["EngineCore"] =
{
    IncludeDirs =
    {
        "%{IncludeDir.BoltEngine}",
        "%{IncludeDir.Spdlog}",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.Glad}",
        "%{IncludeDir.Box2D}",
        "%{IncludeDir.GLM}",
        "%{IncludeDir.EnTT}",
        "%{IncludeDir.STB}",
        "%{IncludeDir.MagicEnum}",
        "%{IncludeDir.MiniAudio}",
        "%{IncludeDir.Cereal}"
    },

    Links =
    {
        "GLFW",
        "Box2D",
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
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.Glad}",
        "%{IncludeDir.Box2D}",
        "%{IncludeDir.GLM}",
        "%{IncludeDir.EnTT}",
        "%{IncludeDir.STB}",
        "%{IncludeDir.MagicEnum}",
        "%{IncludeDir.MiniAudio}",
        "%{IncludeDir.Cereal}"
    },
    
    Links =
    {
        "Bolt-Engine",
        "ImGui",
        "GLFW",
        "%{Library.OpenGL}",
        "Box2D",
        "%{Library.BoltPhys}"
    }
}