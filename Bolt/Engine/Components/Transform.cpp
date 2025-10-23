#include "../pch.hpp"
#include "../Components/Transform.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Bolt {
    void Transform::SetEulerAngles(const glm::vec3& eulerAnglesRadians) {
        Rotation = glm::quat(eulerAnglesRadians);
    }

    glm::vec3 Transform::GetEulerAngles() const {
        return glm::eulerAngles(Rotation);
    }

    glm::mat4 Transform::GetModelMatrix() const {
        glm::mat4 translation = glm::translate(glm::mat4(1.0f), Position);
        glm::mat4 rotationMat = glm::toMat4(Rotation);
        glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), Scale);
        return translation * rotationMat * scaleMat;
    }

    glm::mat3 Transform::GetNormalMatrix() const {
        glm::mat4 model = GetModelMatrix();
        return glm::transpose(glm::inverse(glm::mat3(model)));
    }
}