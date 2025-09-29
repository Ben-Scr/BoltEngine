#pragma once
#include <glm/glm.hpp>

#include <memory>
#include <vector>

namespace bgfx { struct VertexLayout; }

namespace Bolt {
    class Camera2D;
    struct PosColorVertex;
    class Shader;
    
    struct PosColorVertex { float x, y, z; uint32_t color; };

    class GizmoRenderer {
    public:
        static bool Initialize();
        static void Shutdown();
        static void OnResize(int w, int h);
        static void BeginFrame(uint16_t viewId = 1);
        static void Render();
        static void EndFrame();

    private:
        static void FlushGizmos();

        static bool m_IsInitialized;
        static std::unique_ptr<Shader> m_GizmoShader;
        static bgfx::VertexLayout m_GizmoLayout;


        static std::vector<PosColorVertex> m_GizmoVertices;
        static std::vector<uint16_t> m_GizmoIndices;

        static uint16_t m_GizmoViewId;
    };
}