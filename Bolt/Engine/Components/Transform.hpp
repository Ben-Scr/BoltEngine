#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Bolt {
    class Transform {
    public:
        Transform() = default;
        explicit Transform(const Vec3& position) : Position(position) {}
        Transform(const Vec3& position, const glm::quat& rotation)
            : Position(position), Rotation(rotation) {
        }
        Transform(const Vec3& position, const Vec3& scale, const glm::quat& rotation)
            : Position(position), Scale(scale), Rotation(rotation) {
        }

        Vec3 Position{ 0.0f, 0.0f, 0.0f };
        Vec3 Scale{ 1.0f, 1.0f, 1.0f };
        glm::quat Rotation{ 1.0f, 0.0f, 0.0f, 0.0f };

        void SetEulerAngles(const Vec3& eulerAnglesRadians);
        Vec3 GetEulerAngles() const;

        glm::mat4 GetModelMatrix() const;
        glm::mat3 GetNormalMatrix() const;
    };
}