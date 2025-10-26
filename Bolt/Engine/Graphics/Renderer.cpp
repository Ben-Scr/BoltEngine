#include "../pch.hpp"
#include "Renderer.hpp"

#include "Camera.hpp"
#include "Mesh.hpp"
#include "Shader.hpp"

#include "../Components/MeshRenderer.hpp"
#include "../Components/Transform.hpp"
#include "../Scene/SceneManager.hpp"
#include "../Scene/Scene.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/type_ptr.hpp>

namespace Bolt {
    namespace {
        const glm::vec3 DEFAULT_LIGHT_DIRECTION = glm::normalize(glm::vec3(-0.5f, -0.5f, -0.3f));
        constexpr glm::vec3 DEFAULT_LIGHT_COLOR = glm::vec3(1.0f);
    }

    void Renderer::Initialize() {
        EnsureShader();
        m_Initialized = true;
    }

    void Renderer::Shutdown() {
        m_Shader.reset();
        m_uModel = m_uView = m_uProjection = m_uNormalMatrix = -1;
        m_uColor = m_uLightDir = m_uLightColor = m_uViewPos = -1;
        m_Initialized = false;
    }

    void Renderer::BeginFrame() {
        if (!m_Initialized) {
            return;
        }

        glClear(GL_DEPTH_BUFFER_BIT);
        RenderScenes();
    }

    void Renderer::EndFrame() {
    }

    void Renderer::RenderScenes() {
        for (const std::string& sceneName : SceneManager::GetLoadedSceneNames()) {
            RenderScene(SceneManager::GetLoadedScene(sceneName));
        }
    }

    void Renderer::RenderScene(Scene& scene) {
        if (!EnsureShader()) {
            Logger::Error("Renderer", "3D shader is invalid");
            return;
        }

        Camera* camera = Camera::Main();
        if (!camera) {
            Logger::Warning("Renderer", "No active 3D camera set");
            return;
        }

        const glm::mat4 view = camera->GetViewMatrix();
        const glm::mat4 projection = camera->GetProjectionMatrix();
        const glm::vec3 viewPos = camera->GetPosition();

        m_Shader->Submit();

        if (m_uView >= 0) {
            glUniformMatrix4fv(m_uView, 1, GL_FALSE, glm::value_ptr(view));
        }
        if (m_uProjection >= 0) {
            glUniformMatrix4fv(m_uProjection, 1, GL_FALSE, glm::value_ptr(projection));
        }
        if (m_uLightDir >= 0) {
            glUniform3fv(m_uLightDir, 1, glm::value_ptr(DEFAULT_LIGHT_DIRECTION));
        }
        if (m_uLightColor >= 0) {
            glUniform3fv(m_uLightColor, 1, glm::value_ptr(DEFAULT_LIGHT_COLOR));
        }
        if (m_uViewPos >= 0) {
            glUniform3fv(m_uViewPos, 1, glm::value_ptr(viewPos));
        }

        auto viewMesh = scene.GetRegistry().view<Transform, MeshRenderer>();
        for (auto [entity, transform, meshRenderer] : viewMesh.each()) {
            if (!meshRenderer.Visible || !meshRenderer.Mesh) {
                continue;
            }

            Mesh& mesh = *meshRenderer.Mesh;
            if (!mesh.IsValid()) {
                continue;
            }

            const glm::mat4 model = transform.GetModelMatrix();
            const glm::mat3 normalMatrix = transform.GetNormalMatrix();

            if (m_uModel >= 0) {
                glUniformMatrix4fv(m_uModel, 1, GL_FALSE, glm::value_ptr(model));
            }
            if (m_uNormalMatrix >= 0) {
                glUniformMatrix3fv(m_uNormalMatrix, 1, GL_FALSE, glm::value_ptr(normalMatrix));
            }
            if (m_uColor >= 0) {
                const Color& color = meshRenderer.AlbedoColor;
                glUniform4f(m_uColor, color.r, color.g, color.b, color.a);
            }

            mesh.Bind();
            mesh.Draw();
            mesh.Unbind();
        }

        glUseProgram(0);
    }

    bool Renderer::EnsureShader() {
        if (m_Shader.has_value()) {
            return m_Shader->IsValid();
        }

        m_Shader.emplace("Assets/Shader/3D/default.vert.glsl", "Assets/Shader/3D/default.frag.glsl");
        if (!m_Shader->IsValid()) {
            Logger::Error("Renderer", "Failed to load 3D shader");
            m_Shader.reset();
            return false;
        }

        GLuint handle = m_Shader->GetHandle();
        m_uModel = glGetUniformLocation(handle, "uModel");
        m_uView = glGetUniformLocation(handle, "uView");
        m_uProjection = glGetUniformLocation(handle, "uProjection");
        m_uNormalMatrix = glGetUniformLocation(handle, "uNormalMatrix");
        m_uColor = glGetUniformLocation(handle, "uColor");
        m_uLightDir = glGetUniformLocation(handle, "uDirectionalLight.direction");
        m_uLightColor = glGetUniformLocation(handle, "uDirectionalLight.color");
        m_uViewPos = glGetUniformLocation(handle, "uViewPos");

        return true;
    }
}