#pragma once
#include "Vec2.hpp"

namespace BoltPhys {
    struct AABB
    {
        Vec2 min;
        Vec2 max;

        constexpr bool Intersects(const AABB& other) const noexcept
        {
            return !(max.x < other.min.x || min.x > other.max.x || max.y < other.min.y || min.y > other.max.y);
        }

        constexpr Vec2 GetSize() const noexcept
        {
            return max - min;
        }
    };
}