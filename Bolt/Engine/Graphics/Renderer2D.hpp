#pragma once
#include "..\Collections\Vec2.hpp"
#include "..\Collections\Color.hpp"
#include "..\Collections\Viewport.hpp"

#include "Texture2D.hpp"
#include "QuadMesh.hpp"
#include "SpriteShaderProgram.hpp"
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

        QuadMesh m_QuadMesh;
        SpriteShaderProgram m_SpriteShader;
    };
}
