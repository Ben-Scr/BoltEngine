#pragma once
#include "..\Collections\Vec2.hpp"
#include "..\Collections\Color.hpp"
#include "..\Collections\Viewport.hpp"


#include "Texture2D.hpp"
#include "Shader.hpp"
#include <cstdint>
#include <optional>

namespace Bolt {
    class Scene;

    enum GLCullingModes {
        GLNone = 0,
        GLFrontLeft = 0x0400,
        GLFrontRight = 0x0401,
        GLBackLeft = 0x0402,
        GLBackRight = 0x0403,
        GLFront = 0x0404,     
        GLBack = 0x0405,
        GLLeft = 0x0406,
        GLRight = 0x0407,
        GLFrontAndBack = 0x0408
    };

    struct GLInitProperties {
        Color BackgroundColor;
        bool EnableCulling;
        GLCullingModes CullingMode;

        GLInitProperties(Color backgroundColor, bool enableCulling, GLCullingModes cullMode)
            : BackgroundColor{ backgroundColor }, EnableCulling{ enableCulling }, CullingMode{ cullMode } {
        }
    };

    class Renderer2D {
    public:
        void Initialize(const GLInitProperties& glInitProps);
        void BeginFrame();
        void EndFrame();
        void RenderScenes();
        void RenderScene(Scene& scene);

    private:
        void Shutdown();

        unsigned m_VAO{ 0 }, m_VBO{ 0 }, m_EBO{ 0 };
        int u_MVP{ -1 }, u_spritePos{ -1 }, u_Scale{ -1 }, u_Rotation{ -1 };
        int u_UVOffset{ -1 }, u_UVScale{ -1 }, u_PremultipliedAlpha{ -1 }, u_AlphaCutoff{ -1 };

        std::optional<Shader> m_Sprite2DShader;
    };
}
