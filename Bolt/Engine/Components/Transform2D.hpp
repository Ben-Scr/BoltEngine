#pragma once
#include "../Collections/Vec2.hpp"
#include <box2d/types.h>

namespace Bolt {

	class Transform2D {
    public:
		Vec2 Position{ 0.0f, 0.0f };
		float Rotation{ 0.0f };   // Note: Rotation angle in radians
		Vec2 Scale{ 1.0f, 1.0f };


		Transform2D() = default;
        Transform2D(const Vec2& position) : Position{ position } {};
        Transform2D(const Vec2& position, float rotation) : Position{ position }, Rotation{ rotation } {};
        Transform2D(const Vec2& position, float rotation, const Vec2& scale) : Position{ position }, Rotation{ rotation }, Scale{scale} {};

        static Transform2D FromPosition(const Vec2& pos);
        static Transform2D FromScale(const Vec2& scale);

        float GetRotationDegrees() const;
        glm::mat3 GetModelMatrix() const;

        // Note: Only used internally by Box2D
        b2Rot GetB2Rotation() const;
        bool operator==(const Transform2D& other) const {
            return Position == other.Position
                && Rotation == other.Rotation
                && Scale == other.Scale;
        }
        bool operator!=(const Transform2D& other) const {
            return !(*this == other);
        }

        // Error: Wrong code, will be fixed.
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
            return Transform2D(Position * scalar, Rotation * scalar, Scale * scalar);
        }
        Transform2D& operator*=(float scalar) {
            Position *= scalar;
            Rotation *= scalar;
            Scale *= scalar;
            return *this;
        }
	};
}