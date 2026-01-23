#pragma once
#include "QuadMesh.hpp"
#include "SpriteShaderProgram.hpp"
#include "TextureHandle.hpp"

namespace Bolt {
    class Scene;

    struct Instance44 {
        Vec2 Position{};
        Vec2 Scale{ 1.0f, 1.0f };
        float Rotation{ 0.0f };
        Color Color{};
        TextureHandle TextureHandle{};
        short SortingOrder{ 0 };
        std::uint8_t SortingLayer{ 0 };

        Instance44(Vec2 pos,
            Vec2 scale,
            float rotation,
            Bolt::Color color,
            Bolt::TextureHandle tex,
            short sortingOrder,
            std::uint8_t sortingLayer)
            : Position(pos)
            , Scale(scale)
            , Rotation(rotation)
            , Color(color)
            , TextureHandle(tex)
            , SortingOrder(sortingOrder)
            , SortingLayer(sortingLayer)
        {
        }

        Instance44() = default;

        Instance44(const Instance44&) = default;
        Instance44& operator=(const Instance44&) = default;
        Instance44(Instance44&&) noexcept = default;
        Instance44& operator=(Instance44&&) noexcept = default;
    };
    class Renderer2D {
    public:
        void Initialize();
        void BeginFrame();
        void EndFrame();
        void RenderScenes();
        void RenderScene(const Scene& scene);
        void Shutdown();

    private:
        bool m_Initialized;
        QuadMesh m_QuadMesh;
        SpriteShaderProgram m_SpriteShader;
    };
}
