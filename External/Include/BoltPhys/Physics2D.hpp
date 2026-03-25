#pragma once
#include "Export.hpp"
#include "Vec2.hpp"
#include "Contact.hpp"

namespace BoltPhys {
    class Collider2D;
    class PhysicsWorld;

    class BOLT_PHYS_API Physics2D
    {
    public:
        static void SetContext(PhysicsWorld& world) noexcept;
        static void ClearContext() noexcept;

        static Contact* OverlapsWith(Collider2D& collider);
        static Contact* OverlapsWith(Collider2D& colliderA, Collider2D& colliderB);

        static const Collider2D* ContainsPoint(const Vec2& point);
        static bool ContainsPoint(Collider2D& collider, const Vec2& point);

    private:
        static PhysicsWorld* s_context;
    };
}