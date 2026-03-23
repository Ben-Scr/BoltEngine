#pragma once
#include "Export.hpp"
#include "BodyType.hpp"
#include "Vec2.hpp"

namespace BoltPhys {
    class Collider;

    class BOLT_PHYS_API Body
    {
    public:
        virtual ~Body() = default;

        BodyType GetType() const noexcept;

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

        Collider* GetCollider() noexcept;
        const Collider* GetCollider() const noexcept;
        void AttachCollider(Collider* collider) noexcept;

    protected:
        explicit Body(BodyType type) noexcept;

    private:
        BodyType m_type;
        Vec2 m_position{};
        Vec2 m_velocity{};
        float m_mass = 1.0f;
        bool m_boundaryCheckEnabled = true;
        bool m_gravityEnabled = true;
        Collider* m_collider = nullptr;
    };
}