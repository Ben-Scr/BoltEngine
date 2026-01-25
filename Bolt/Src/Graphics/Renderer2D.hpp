#pragma once
#include "QuadMesh.hpp"
#include "SpriteShaderProgram.hpp"
#include "TextureHandle.hpp"
#include "Instance44.hpp"

namespace Bolt {
    class Scene;

    class Renderer2D {
    public:
        void Initialize();
        void BeginFrame();
        void EndFrame();
        void RenderScenes();
        void RenderScene(const Scene& scene);
        void Shutdown();

       const size_t GetRenderedInstancesCount() const { return m_RenderedInstancesCount; }
       const float GetRRenderLoopDuration() const { return m_RenderLoopDuration; }

    private:
        size_t m_RenderedInstancesCount;

        // Info: Duration in ms
        float m_RenderLoopDuration;

        bool m_Initialized;
        QuadMesh m_QuadMesh;
        SpriteShaderProgram m_SpriteShader;
    };
}
