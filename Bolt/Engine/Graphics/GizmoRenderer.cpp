#include "../pch.hpp"
#include "GizmoRenderer.hpp"
#include "Shader.hpp"
#include "Camera2D.hpp"
#include "Gizmos.hpp"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

namespace Bolt {
    namespace {
        struct UploadVertex {
            float x, y, z;
            float r, g, b, a;
        };

        constexpr GLsizei GizmoVertexStride = static_cast<GLsizei>(sizeof(UploadVertex));

        static void ConvertVertices(const std::vector<PosColorVertex>& src, std::vector<UploadVertex>& dst) {
            dst.clear();
            dst.reserve(src.size());
            for (const auto& v : src) {
                Color c = Color::FromABGR32(v.color);
                dst.push_back({ v.x, v.y, v.z, c.r, c.g, c.b, c.a });
            }
        }
    }

    bool GizmoRenderer::m_IsInitialized = false;
    std::unique_ptr<Shader> GizmoRenderer::m_GizmoShader;
    std::vector<PosColorVertex> GizmoRenderer::m_GizmoVertices;
    std::vector<uint16_t> GizmoRenderer::m_GizmoIndices;
    uint16_t GizmoRenderer::m_GizmoViewId = 1;
    unsigned int GizmoRenderer::m_VAO = 0;
    unsigned int GizmoRenderer::m_VBO = 0;
    unsigned int GizmoRenderer::m_EBO = 0;
    int GizmoRenderer::m_uMVP = -1;

    static std::vector<UploadVertex> s_UploadBuffer;

    bool GizmoRenderer::Initialize() {
        if (m_IsInitialized)
            return true;

        m_GizmoShader = std::make_unique<Shader>(
            "Assets/Shader/gizmo.vert.glsl",
            "Assets/Shader/gizmo.frag.glsl");

        if (!m_GizmoShader || !m_GizmoShader->IsValid()) {
            Logger::Error("GizmoRenderer", "Failed to load gizmo shader");
            m_GizmoShader.reset();
            return false;
        }

        GLuint program = m_GizmoShader->GetHandle();
        m_uMVP = glGetUniformLocation(program, "uMVP");

        glGenVertexArrays(1, &m_VAO);
        glGenBuffers(1, &m_VBO);
        glGenBuffers(1, &m_EBO);

        glBindVertexArray(m_VAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, GizmoVertexStride, reinterpret_cast<void*>(0));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, GizmoVertexStride, reinterpret_cast<void*>(sizeof(float) * 3));

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        m_GizmoVertices.reserve(4096);
        m_GizmoIndices.reserve(8192);

        m_IsInitialized = true;
        return true;
    }

    void GizmoRenderer::Shutdown() {
        if (!m_IsInitialized)
            return;

        if (m_EBO) {
            glDeleteBuffers(1, &m_EBO);
            m_EBO = 0;
        }
        if (m_VBO) {
            glDeleteBuffers(1, &m_VBO);
            m_VBO = 0;
        }
        if (m_VAO) {
            glDeleteVertexArrays(1, &m_VAO);
            m_VAO = 0;
        }

        m_GizmoShader.reset();
        m_GizmoVertices.clear();
        m_GizmoIndices.clear();
        s_UploadBuffer.clear();

        m_IsInitialized = false;
    }

    void GizmoRenderer::OnResize(int /*w*/, int /*h*/) {
        
    }

    void GizmoRenderer::BeginFrame(uint16_t viewId) {
        if (!m_IsInitialized)
            return;

        m_GizmoViewId = viewId;
        m_GizmoVertices.clear();
        m_GizmoIndices.clear();
    }

    void GizmoRenderer::Render() {
        if (!m_IsInitialized || !Gizmos::s_IsEnabled)
            return;

        for (const auto& square : Gizmos::s_Squares) {
            uint32_t color = square.Color.ABGR32();
            uint16_t baseIndex = static_cast<uint16_t>(m_GizmoVertices.size());

            Vec2 corners[4] = {
                    Vec2(-square.HalfExtents.x, -square.HalfExtents.y),
                    Vec2(square.HalfExtents.x, -square.HalfExtents.y),
                    Vec2(square.HalfExtents.x,  square.HalfExtents.y),
                    Vec2(-square.HalfExtents.x,  square.HalfExtents.y)
            };

            for (int i = 0; i < 4; ++i) {
                Vec2 rotated = Rotated(corners[i], square.Radiant);
                Vec2 final = square.Center + rotated;
                m_GizmoVertices.push_back({ final.x, final.y, 0.0f, color });
            }

            m_GizmoIndices.push_back(baseIndex + 0);
            m_GizmoIndices.push_back(baseIndex + 1);
            m_GizmoIndices.push_back(baseIndex + 1);
            m_GizmoIndices.push_back(baseIndex + 2);
            m_GizmoIndices.push_back(baseIndex + 2);
            m_GizmoIndices.push_back(baseIndex + 3);
            m_GizmoIndices.push_back(baseIndex + 3);
            m_GizmoIndices.push_back(baseIndex + 0);
        }

        for (const auto& line : Gizmos::s_Lines) {
            uint32_t color = line.Color.ABGR32();
            uint16_t baseIndex = static_cast<uint16_t>(m_GizmoVertices.size());

            m_GizmoVertices.push_back({ line.Start.x, line.Start.y, 0.0f, color });
            m_GizmoVertices.push_back({ line.End.x, line.End.y, 0.0f, color });

            m_GizmoIndices.push_back(baseIndex);
            m_GizmoIndices.push_back(baseIndex + 1);
        }

        for (const auto& circle : Gizmos::s_Circles) {
            if (circle.Segments <= 0)
                continue;
            uint32_t color = circle.Color.ABGR32();
            uint16_t baseIndex = static_cast<uint16_t>(m_GizmoVertices.size());

            float angleStep = TwoPi<float>() / static_cast<float>(circle.Segments);
            for (int i = 0; i < circle.Segments; ++i) {
                float angle = i * angleStep;
                float x = circle.Center.x + circle.Radius * Cos(angle);
                float y = circle.Center.y + circle.Radius * Sin(angle);
                m_GizmoVertices.push_back({ x, y, 0.0f, color });
            }

            for (int i = 0; i < circle.Segments; ++i) {
                m_GizmoIndices.push_back(baseIndex + i);
                m_GizmoIndices.push_back(baseIndex + static_cast<uint16_t>((i + 1) % circle.Segments));
            }
        }

        FlushGizmos();
        Gizmos::Clear();
    }

    void GizmoRenderer::FlushGizmos() {
        if (!m_IsInitialized || m_GizmoVertices.empty() || !m_GizmoShader || !m_GizmoShader->IsValid())
            return;

        ConvertVertices(m_GizmoVertices, s_UploadBuffer);

        glBindVertexArray(m_VAO);

        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(s_UploadBuffer.size() * sizeof(UploadVertex)), s_UploadBuffer.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(m_GizmoIndices.size() * sizeof(uint16_t)), m_GizmoIndices.data(), GL_DYNAMIC_DRAW);

        m_GizmoShader->Submit();

        if (Camera2D::Main()) {
            glm::mat4 mvp = Camera2D::Main()->GetViewProjectionMatrix();
            if (m_uMVP >= 0) {
                glUniformMatrix4fv(m_uMVP, 1, GL_FALSE, glm::value_ptr(mvp));
            }
        }

        glLineWidth(1.0f);
        glDrawElements(GL_LINES, static_cast<GLsizei>(m_GizmoIndices.size()), GL_UNSIGNED_SHORT, nullptr);

        glBindVertexArray(0);
        glUseProgram(0);
    }

    void GizmoRenderer::EndFrame() {
        Render();
    }
}
