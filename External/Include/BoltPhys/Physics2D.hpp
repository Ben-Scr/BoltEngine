#pragma once
#include "Export.hpp"
#include "Vec2.hpp"
#include "Contact.hpp"

namespace BoltPhys {
    class Collider;
    class PhysicsWorld;

    class BOLT_PHYS_API Physics2D
    {
    public:
        static void SetContext(PhysicsWorld& world) noexcept;
        static void ClearContext() noexcept;

        static Contact* OverlapsWith(Collider& collider);
        static Contact* OverlapsWith(Collider& colliderA, Collider& colliderB);

        static const Collider* ContainsPoint(const Vec2& point);
        static bool ContainsPoint(Collider& collider, const Vec2& point);

    private:
        static PhysicsWorld* s_context;
    };
}