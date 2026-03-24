#pragma once
#include <glm/vec2.hpp>

namespace BoltPhys {
    using Vec2 = glm::vec2;

    constexpr float Dot(const Vec2& a, const Vec2& b) noexcept
    {
        return a.x * b.x + a.y * b.y;
    }

    constexpr float LengthSq(const Vec2& v) noexcept
    {
        return Dot(v, v);
    }

    float Length(const Vec2& v) noexcept;
    Vec2 Normalize(const Vec2& v) noexcept;
}