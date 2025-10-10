#pragma once
#include "..\Collections\Vec2.hpp"
#include "..\Collections\Color.hpp"
#include "..\Collections\Viewport.hpp"

#include "Texture2D.hpp"
#include "Shader.hpp"
#include <cstdint>
#include <optional>

namespace Bolt {

    enum GLCullingModes {
        GLNone = 0,
        GLFrontLeft = 0x0400,   // Achtung: diese Werte sind keine gültigen glCullFace Modi
        GLFrontRight = 0x0401,
        GLBackLeft = 0x0402,
        GLBackRight = 0x0403,
        GLFront = 0x0404,       // gültig
        GLBack = 0x0405,        // gültig
        GLLeft = 0x0406,
        GLRight = 0x0407,
        GLFrontAndBackT = 0x0408 // gültig (GL_FRONT_AND_BACK)
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
        void DrawSprite(Vec2 position, Vec2 scale, float rotZ, Color color,
            const Texture2D* texture = nullptr,
            Vec2 uvOffset = { 0.0f, 0.0f },
            Vec2 uvScale = { 1.0f, 1.0f });
        void BeginFrame();
        void EndFrame();

    private:
        void Shutdown();

        // GL-Objekte
        unsigned m_VAO{ 0 }, m_VBO{ 0 }, m_EBO{ 0 };
        unsigned m_WhiteTex{ 0 };
        std::shared_ptr<Viewport> viewport;

        int u_MVP{ -1 }, u_pritePos{ -1 }, u_Scale{ -1 }, u_Rotation{ -1 };
        int u_UVOffset{ -1 }, uUVScale{ -1 }, u_PremultipliedAlpha{ -1 }, u_AlphaCutoff{ -1 };

        std::optional<Shader> m_Sprite2DShader;
    };
}
