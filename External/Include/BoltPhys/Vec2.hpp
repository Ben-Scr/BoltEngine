#pragma once

namespace BoltPhys {
    struct Vec2
    {
        float x = 0.0f;
        float y = 0.0f;

        constexpr Vec2 operator+(const Vec2& other) const noexcept
        {
            return { x + other.x, y + other.y };
        }

        constexpr Vec2 operator-(const Vec2& other) const noexcept
        {
            return { x - other.x, y - other.y };
        }

        constexpr Vec2 operator-() const noexcept
        {
            return { -x, -y };
        }

        constexpr Vec2 operator*(float scalar) const noexcept
        {
            return { x * scalar, y * scalar };
        }

        constexpr Vec2 operator/(float scalar) const noexcept
        {
            return { x / scalar, y / scalar };
        }

        constexpr Vec2& operator+=(const Vec2& other) noexcept
        {
            x += other.x;
            y += other.y;
            return *this;
        }

        constexpr Vec2& operator-=(const Vec2& other) noexcept
        {
            x -= other.x;
            y -= other.y;
            return *this;
        }

        constexpr Vec2& operator*=(float scalar) noexcept
        {
            x *= scalar;
            y *= scalar;
            return *this;
        }
    };

    constexpr Vec2 operator*(float scalar, const Vec2& value) noexcept
    {
        return value * scalar;
    }

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