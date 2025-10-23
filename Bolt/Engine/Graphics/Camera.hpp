#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Bolt {
    class Camera {
    public:
        enum class ProjectionType {
            Perspective,
            Orthographic
        };

        Camera();

        static Camera* Main();
        static void SetMain(Camera* camera);
        void SetAsMain();

        void SetViewportSize(int width, int height);

        void SetPerspective(float verticalFovDegrees, float nearClip, float farClip);
        void SetOrthographic(float size, float nearClip, float farClip);
        void SetProjectionType(ProjectionType type);

        void SetPosition(const glm::vec3& position);
        void Move(const glm::vec3& delta);

        void SetRotation(const glm::quat& rotation);
        void SetEulerAngles(const glm::vec3& eulerAnglesRadians);
        void Rotate(const glm::quat& delta);
        void RotateEuler(const glm::vec3& deltaEulerRadians);

        void LookAt(const glm::vec3& target, const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f));

        const glm::vec3& GetPosition() const { return m_Position; }
        const glm::quat& GetRotation() const { return m_Rotation; }

        glm::vec3 GetForward() const;
        glm::vec3 GetUp() const;
        glm::vec3 GetRight() const;

        float GetAspectRatio() const { return m_AspectRatio; }
        float GetVerticalFov() const { return m_VerticalFov; }

        glm::mat4 GetViewMatrix() const { return m_ViewMatrix; }
        glm::mat4 GetProjectionMatrix() const { return m_ProjectionMatrix; }
        glm::mat4 GetViewProjectionMatrix() const { return m_ProjectionMatrix * m_ViewMatrix; }

        ProjectionType GetProjectionType() const { return m_ProjectionType; }

        float GetNearClip() const { return m_NearClip; }
        float GetFarClip() const { return m_FarClip; }

    private:
        void UpdateViewMatrix();
        void UpdateProjectionMatrix();

        static Camera* s_Main;

        ProjectionType m_ProjectionType{ ProjectionType::Perspective };
        glm::vec3 m_Position{ 0.0f, 0.0f, 5.0f };
        glm::quat m_Rotation{ 1.0f, 0.0f, 0.0f, 0.0f };
        float m_VerticalFov{ 45.0f };
        float m_OrthographicSize{ 10.0f };
        float m_NearClip{ 0.1f };
        float m_FarClip{ 1000.0f };
        float m_AspectRatio{ 16.0f / 9.0f };
        glm::mat4 m_ViewMatrix{ 1.0f };
        glm::mat4 m_ProjectionMatrix{ 1.0f };
    };
}
