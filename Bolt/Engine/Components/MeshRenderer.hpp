#pragma once

#include "../Collections/Color.hpp"
#include <memory>

namespace Bolt {
    class Mesh;

    class MeshRenderer {
    public:
        MeshRenderer() = default;
        explicit MeshRenderer(std::shared_ptr<Mesh> mesh) : Mesh(std::move(mesh)) {}

        std::shared_ptr<Mesh> Mesh;
        Color AlbedoColor{ 1.0f, 1.0f, 1.0f, 1.0f };
        bool Visible{ true };
    };
}