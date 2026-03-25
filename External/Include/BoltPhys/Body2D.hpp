#pragma once
#include "Export.hpp"
#include "BodyType.hpp"
#include "Vec2.hpp"

namespace BoltPhys {
    class Collider2D;

    class BOLT_PHYS_API Body2D
    {
    public:
        Body2D() noexcept;
        explicit Body2D(BodyType type) noexcept;
        ~Body2D() = default;
        void Destroy();

        BodyType GetBodyType() const noexcept;
        void SetBodyType(BodyType type) noexcept;

        const Vec2& GetPosition() const noexcept;
        void SetPosition(const Vec2& p) noexcept;

        const Vec2& GetVelocity() const noexcept;
        void SetVelocity(const Vec2& v) noexcept;

        float GetMass() const noexcept;
        void SetMass(float mass) noexcept;

        bool IsBoundaryCheckEnabled() const noexcept;
        void SetBoundaryCheckEnabled(bool enabled) noexcept;

        bool IsGravityEnabled() const noexcept;
        void SetGravityEnabled(bool enabled) noexcept;

        Collider2D* GetCollider() noexcept;
        const Collider2D* GetCollider() const noexcept;
        void AttachCollider(Collider2D* collider) noexcept;

    private:
        BodyType m_bodyType = BodyType::Dynamic;
        Vec2 m_position{};
        Vec2 m_velocity{};
        float m_mass = 1.0f;
        bool m_boundaryCheckEnabled = true;
        bool m_gravityEnabled = true;
        Collider2D* m_collider = nullptr;
    };
}