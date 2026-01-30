#pragma once
#include "Collections/Vec2.hpp"
#include <box2d/types.h>

namespace Bolt {
    static inline Vec2 Hadamard(const Vec2& a, const Vec2& b) {
        return { a.x * b.x, a.y * b.y };
    }

    static inline Vec2 Rotate(const Vec2& v, float radians) {
        float c = std::cos(radians);
        float s = std::sin(radians);
        return { c * v.x - s * v.y, s * v.x + c * v.y };
    }

	class Transform2D {
    public:
		Vec2 Position{ 0.0f, 0.0f };
		Vec2 Scale{ 1.0f, 1.0f };
        float Rotation{ 0.0f };   // Info: Z-Rotation angle in radians


		Transform2D() = default;
        Transform2D(const Vec2& position) : Position{ position } {};
        Transform2D(const Vec2& position, const Vec2& scale) : Position{ position }, Scale{ scale } {};
        Transform2D(const Vec2& position, const Vec2& scale, float rotation) : Position{ position }, Scale{scale}, Rotation{ rotation } {};

        static Transform2D FromPosition(const Vec2& pos);
        static Transform2D FromScale(const Vec2& scale);

        float GetRotationDegrees() const;
        glm::mat3 GetModelMatrix() const;
        Vec2 GetForwardDirection() const;

        Vec2 TransformPoint(const Vec2& localPoint) const {
            Vec2 p = Hadamard(localPoint, Scale); // S
            p = Rotate(p, Rotation);              // R
            p = { p.x + Position.x, p.y + Position.y }; // T
            return p;
        }
        Vec2 TransformVector(const Vec2& localVec) const {
            Vec2 v = Hadamard(localVec, Scale);
            return Rotate(v, Rotation);
        }

        // Info: Used internally for Box2D
        b2Rot GetB2Rotation() const;
        bool operator==(const Transform2D& other) const {
            return Position == other.Position
                && Rotation == other.Rotation
                && Scale == other.Scale;
        }
        bool operator!=(const Transform2D& other) const {
            return !(*this == other);
        }

        // Wrong code, will be fixed.
        Transform2D operator+(const Transform2D& other) const {
            return Transform2D(Position + other.Position);
        }
        Transform2D operator-(const Transform2D& other) const {
            return Transform2D(Position - other.Position);
        }
        Transform2D& operator+=(const Transform2D& other) {
            Position += other.Position;
            Rotation += other.Rotation;
            Scale += other.Scale;
            return *this;
        }
        Transform2D& operator-=(const Transform2D& other) {
            Position -= other.Position;
            Rotation -= other.Rotation;
            Scale -= other.Scale;
            return *this;
        }


        Transform2D operator*(float scalar) const {
            return Transform2D(Position * scalar, Scale * scalar, Rotation * scalar);
        }
        Transform2D& operator*=(float scalar) {
            Position *= scalar;
            Rotation *= scalar;
            Scale *= scalar;
            return *this;
        }
	};

    static inline float LookAt2D(const Transform2D& from, const Vec2& to) {
        Vec2 lookDir = to - from.Position;
        float lookAtZ = atan2(lookDir.x, lookDir.y);
        return std::remainder(lookAtZ - from.Rotation, TwoPi<float>());
    }
}