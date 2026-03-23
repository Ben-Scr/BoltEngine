#pragma once
#include "Export.hpp"
#include "Collider.hpp"

namespace BoltPhys {
    class BOLT_PHYS_API BoxCollider final : public Collider
    {
    public:
        explicit BoxCollider(const Vec2& halfExtents);

        const Vec2& GetHalfExtents() const noexcept;
        void SetHalfExtents(const Vec2& halfExtents) noexcept;

        AABB ComputeAABB() const override;

    private:
        Vec2 m_halfExtents;
    };
}