#pragma once
#include "..\Collections\Vec2.hpp"
#include "..\Collections\Color.hpp"

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
        unsigned mVAO{ 0 }, mVBO{ 0 }, mEBO{ 0 };
        unsigned mWhiteTex{ 0 };
        int mViewportW{ 0 }, mViewportH{ 0 };

        // Uniform-Locations
        int uMVP{ -1 }, uSpritePos{ -1 }, uScale{ -1 }, uRotation{ -1 };
        int uUVOffset{ -1 }, uUVScale{ -1 }, uPremultipliedAlpha{ -1 }, uAlphaCutoff{ -1 };

        // Einfache Ortho-Matrix (Pixel -> NDC)
        void makeOrtho(float l, float r, float b, float t, float* out16) const;

        // Dein Shader (Pfad ggf. anpassen – hier **GLSL**-Dateien, nicht .sc)
        std::optional<Shader> m_Sprite2DShader;
    };
}
