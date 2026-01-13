#pragma once
#include "QuadMesh.hpp"
#include "SpriteShaderProgram.hpp"

namespace Bolt {
    class Scene;

    class Renderer2D {
    public:
        void Initialize();
        void BeginFrame();
        void EndFrame();
        void RenderScenes();
        void RenderScene(Scene& scene);
        void Shutdown();

    private:
        QuadMesh m_QuadMesh;
        SpriteShaderProgram m_SpriteShader;
    };
}
