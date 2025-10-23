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

 
    class Renderer2D {
    public:
        void Initialize();
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
