#include "pch.hpp"
#include "QuadMesh.hpp"
#include <glad/glad.h>

namespace Bolt {
    namespace {
        struct V2UV {
            float x, y;
            float u, v;
        };

        constexpr V2UV QUAD_VERTICES[4] = {
            {-0.5f, -0.5f, 0.0f, 0.0f},
            { 0.5f, -0.5f, 1.0f, 0.0f},
            { 0.5f,  0.5f, 1.0f, 1.0f},
            {-0.5f,  0.5f, 0.0f, 1.0f},
        };

        constexpr unsigned short QUAD_INDICES[6] = { 0, 1, 2, 0, 2, 3 };
    }

    void QuadMesh::Initialize() {
        if (m_VAO != 0) {
            return;
        }

        glGenVertexArrays(1, &m_VAO);
        glGenBuffers(1, &m_VBO);
        glGenBuffers(1, &m_EBO);

        glBindVertexArray(m_VAO);

        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(QUAD_VERTICES), QUAD_VERTICES, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(QUAD_INDICES), QUAD_INDICES, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(V2UV), reinterpret_cast<void*>(offsetof(V2UV, x)));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(V2UV), reinterpret_cast<void*>(offsetof(V2UV, u)));

        glDisableVertexAttribArray(2);

        glBindVertexArray(0);
    }

    void QuadMesh::Bind() const {
        glBindVertexArray(m_VAO);
    }

    void QuadMesh::Unbind() const {
        glBindVertexArray(0);
    }

    void QuadMesh::Draw() const {
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
    }

    void QuadMesh::Shutdown() {
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
    }
}
