#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Bolt {
    class Transform {
    public:
        Transform() = default;
        explicit Transform(const glm::vec3& position) : Position(position) {}
        Transform(const glm::vec3& position, const glm::quat& rotation)
            : Position(position), Rotation(rotation) {
        }
        Transform(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale)
            : Position(position), Rotation(rotation), Scale(scale) {
        }

        glm::vec3 Position{ 0.0f, 0.0f, 0.0f };
        glm::quat Rotation{ 1.0f, 0.0f, 0.0f, 0.0f };
        glm::vec3 Scale{ 1.0f, 1.0f, 1.0f };

        void SetEulerAngles(const glm::vec3& eulerAnglesRadians);
        glm::vec3 GetEulerAngles() const;

        glm::mat4 GetModelMatrix() const;
        glm::mat3 GetNormalMatrix() const;
    };
}