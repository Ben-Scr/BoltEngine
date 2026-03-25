#pragma once
#include "Export.hpp"
#include "Collider2D.hpp"

namespace BoltPhys {
    class BOLT_PHYS_API CircleCollider final : public Collider2D
    {
    public:
        explicit CircleCollider(float radius);

        float GetRadius() const noexcept;
        void SetRadius(float radius);

        AABB ComputeAABB() const override;

    private:
        float m_radius = 0.5f;
    };
}