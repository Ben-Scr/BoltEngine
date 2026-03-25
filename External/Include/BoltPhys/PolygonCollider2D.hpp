#pragma once
#include "Export.hpp"
#include "Collider2D.hpp"

#include <cstddef>
#include <vector>

namespace BoltPhys {
    class BOLT_PHYS_API PolygonCollider2D final : public Collider2D
    {
    public:
        PolygonCollider2D();

        void SetVertices(const Vec2* vertices, std::size_t count);
        std::size_t GetVertexCount() const noexcept;
        const Vec2* GetVertices() const noexcept;

        AABB ComputeAABB() const override;

    private:
        std::vector<Vec2> m_vertices;
    };
}