#pragma once
#include "Vec2.hpp"

namespace BoltPhys {
    class Body2D;

    struct Contact
    {
         Body2D* bodyA = nullptr;
         Body2D* bodyB = nullptr;
        Vec2 normal{};
        float penetration = 0.0f;
    };
}