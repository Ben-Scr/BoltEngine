#pragma once
#include "Export.hpp"
#include "Collider2D.hpp"

namespace BoltPhys {
    class BOLT_PHYS_API BoxCollider2D final : public Collider2D
    {
    public:
        BoxCollider2D() noexcept;
        explicit BoxCollider2D(const Vec2& halfExtents);

        const Vec2& GetHalfExtents() const noexcept;
        void SetHalfExtents(const Vec2& halfExtents) noexcept;

        AABB ComputeAABB() const override;

    private:
        Vec2 m_halfExtents;
    };
}