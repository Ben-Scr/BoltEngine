#pragma once

#include "../Collections/Color.hpp"
#include "Shader.hpp"

#include <optional>

namespace Bolt {
    class Scene;
    class Mesh;
    class Camera;

    class Renderer {
    public:
        Renderer() = default;

        void Initialize();
        void Shutdown();

        void BeginFrame();
        void EndFrame();

        void RenderScenes();
        void RenderScene(Scene& scene);

    private:
        bool EnsureShader();

        std::optional<Shader> m_Shader;
        int m_uModel{ -1 };
        int m_uView{ -1 };
        int m_uProjection{ -1 };
        int m_uNormalMatrix{ -1 };
        int m_uColor{ -1 };
        int m_uLightDir{ -1 };
        int m_uLightColor{ -1 };
        int m_uViewPos{ -1 };
        bool m_Initialized{ false };
    };
}
