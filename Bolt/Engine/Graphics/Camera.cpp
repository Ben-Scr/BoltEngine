#include "../pch.hpp"
#include "Camera.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Bolt {
    Camera* Camera::s_Main = nullptr;

    Camera::Camera() {
        UpdateProjectionMatrix();
        UpdateViewMatrix();
        s_Main = this;
    }

    Camera* Camera::Main() {
        return s_Main;
    }

    void Camera::SetMain(Camera* camera) {
        s_Main = camera;
    }

    void Camera::SetAsMain() {
        SetMain(this);
    }

    void Camera::SetViewportSize(int width, int height) {
        if (height <= 0) {
            height = 1;
        }
        if (width <= 0) {
            width = 1;
        }
        m_AspectRatio = static_cast<float>(width) / static_cast<float>(height);
        UpdateProjectionMatrix();
    }

    void Camera::SetPerspective(float verticalFovDegrees, float nearClip, float farClip) {
        m_ProjectionType = ProjectionType::Perspective;
        m_VerticalFov = verticalFovDegrees;
        m_NearClip = std::max(nearClip, 0.001f);
        m_FarClip = std::max(farClip, m_NearClip + 0.001f);
        UpdateProjectionMatrix();
    }

    void Camera::SetOrthographic(float size, float nearClip, float farClip) {
        m_ProjectionType = ProjectionType::Orthographic;
        m_OrthographicSize = std::max(size, 0.001f);
        m_NearClip = nearClip;
        m_FarClip = farClip;
        UpdateProjectionMatrix();
    }

    void Camera::SetProjectionType(ProjectionType type) {
        m_ProjectionType = type;
        UpdateProjectionMatrix();
    }

    void Camera::SetPosition(const glm::vec3& position) {
        m_Position = position;
        UpdateViewMatrix();
    }

    void Camera::Move(const glm::vec3& delta) {
        SetPosition(m_Position + delta);
    }

    void Camera::SetRotation(const glm::quat& rotation) {
        m_Rotation = glm::normalize(rotation);
        UpdateViewMatrix();
    }

    void Camera::SetEulerAngles(const glm::vec3& eulerAnglesRadians) {
        SetRotation(glm::quat(eulerAnglesRadians));
    }

    void Camera::Rotate(const glm::quat& delta) {
        SetRotation(delta * m_Rotation);
    }

    void Camera::RotateEuler(const glm::vec3& deltaEulerRadians) {
        Rotate(glm::quat(deltaEulerRadians));
    }

    void Camera::LookAt(const glm::vec3& target, const glm::vec3& up) {
        m_ViewMatrix = glm::lookAt(m_Position, target, up);
        glm::mat4 viewInv = glm::inverse(m_ViewMatrix);
        m_Rotation = glm::quat_cast(viewInv);
    }

    glm::vec3 Camera::GetForward() const {
        return glm::normalize(m_Rotation * glm::vec3(0.0f, 0.0f, -1.0f));
    }

    glm::vec3 Camera::GetUp() const {
        return glm::normalize(m_Rotation * glm::vec3(0.0f, 1.0f, 0.0f));
    }

    glm::vec3 Camera::GetRight() const {
        return glm::normalize(glm::cross(GetForward(), GetUp()));
    }

    void Camera::UpdateViewMatrix() {
        glm::vec3 forward = GetForward();
        glm::vec3 up = GetUp();
        m_ViewMatrix = glm::lookAt(m_Position, m_Position + forward, up);
    }

    void Camera::UpdateProjectionMatrix() {
        const float aspect = glm::max(m_AspectRatio, 0.001f);
        if (m_ProjectionType == ProjectionType::Perspective) {
            m_ProjectionMatrix = glm::perspective(glm::radians(m_VerticalFov), aspect, m_NearClip, m_FarClip);
        }
        else {
            float halfHeight = m_OrthographicSize * 0.5f;
            float halfWidth = halfHeight * aspect;
            m_ProjectionMatrix = glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, m_NearClip, m_FarClip);
        }
    }
}