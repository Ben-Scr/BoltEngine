#pragma once
#include "../Collections/Vec2.hpp"
#include <box2d/types.h>

namespace glm {
   struct mat3;
}

namespace Bolt {

    class Transform {
    public:
        Vec3 Position{ 0.0f, 0.0f, 0.0f };
        Vec3 Scale{ 1.0f, 1.0f, 1.0f };
        Vec3 Rotation{ 0.0f, 0.0f, 0.0f };


        Transform() = default;
        Transform(const Vec3& position) : Position{ position } {};
        Transform(const Vec3& position, const Vec3& rotation) : Position{ position }, Rotation{ rotation } {};
        Transform(const Vec3& position, const Vec3& scale, const Vec3& rotation) : Position{ position }, Scale{ scale }, Rotation{ rotation } {};

        static Transform FromPosition(const Vec3& pos);
        static Transform FromScale(const Vec3& scale);


        glm::mat3 GetModelMatrix() const;

        bool operator==(const Transform& other) const {
            return Position == other.Position
                && Rotation == other.Rotation
                && Scale == other.Scale;
        }
        bool operator!=(const Transform& other) const {
            return !(*this == other);
        }


        Transform operator+(const Transform& other) const {
            return Transform(Position + other.Position);
        }
        Transform operator-(const Transform& other) const {
            return Transform(Position - other.Position);
        }
        Transform& operator+=(const Transform& other) {
            Position += other.Position;
            Rotation += other.Rotation;
            Scale += other.Scale;
            return *this;
        }
        Transform& operator-=(const Transform& other) {
            Position -= other.Position;
            Rotation -= other.Rotation;
            Scale -= other.Scale;
            return *this;
        }


        Transform operator*(float scalar) const {
            return Transform(Position * scalar, Scale * scalar, Rotation * scalar);
        }
        Transform& operator*=(float scalar) {
            Position *= scalar;
            Rotation *= scalar;
            Scale *= scalar;
            return *this;
        }
    };
}