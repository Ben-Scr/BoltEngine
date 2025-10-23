#include "../pch.hpp"
#include "Mesh.hpp"

namespace Bolt {
    namespace {
        constexpr GLuint POSITION_LOCATION = 0;
        constexpr GLuint NORMAL_LOCATION = 1;
        constexpr GLuint TEXCOORD_LOCATION = 2;
    }

    Mesh::Mesh(Mesh&& other) noexcept {
        *this = std::move(other);
    }

    Mesh& Mesh::operator=(Mesh&& other) noexcept {
        if (this != &other) {
            Release();
            m_VAO = other.m_VAO;
            m_VBO = other.m_VBO;
            m_EBO = other.m_EBO;
            m_VertexCount = other.m_VertexCount;
            m_IndexCount = other.m_IndexCount;

            other.m_VAO = 0;
            other.m_VBO = 0;
            other.m_EBO = 0;
            other.m_VertexCount = 0;
            other.m_IndexCount = 0;
        }
        return *this;
    }

    Mesh::~Mesh() {
        Release();
    }

    void Mesh::SetData(const std::vector<MeshVertex>& vertices, const std::vector<uint32_t>& indices) {
        if (vertices.empty()) {
            Logger::Warning("[Mesh]", "Attempted to upload mesh with no vertices");
            return;
        }

        if (m_VAO == 0) {
            glGenVertexArrays(1, &m_VAO);
            glGenBuffers(1, &m_VBO);
            glGenBuffers(1, &m_EBO);
        }

        glBindVertexArray(m_VAO);

        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(MeshVertex)), vertices.data(), GL_STATIC_DRAW);

        if (!indices.empty()) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(indices.size() * sizeof(uint32_t)), indices.data(), GL_STATIC_DRAW);
            m_IndexCount = static_cast<GLsizei>(indices.size());
        }
        else {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            m_IndexCount = 0;
        }

        m_VertexCount = static_cast<GLsizei>(vertices.size());

        glEnableVertexAttribArray(POSITION_LOCATION);
        glVertexAttribPointer(POSITION_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), reinterpret_cast<void*>(offsetof(MeshVertex, Position)));

        glEnableVertexAttribArray(NORMAL_LOCATION);
        glVertexAttribPointer(NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), reinterpret_cast<void*>(offsetof(MeshVertex, Normal)));

        glEnableVertexAttribArray(TEXCOORD_LOCATION);
        glVertexAttribPointer(TEXCOORD_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), reinterpret_cast<void*>(offsetof(MeshVertex, TexCoord)));

        glBindVertexArray(0);
    }

    void Mesh::Bind() const {
        glBindVertexArray(m_VAO);
    }

    void Mesh::Unbind() const {
        glBindVertexArray(0);
    }

    void Mesh::Draw() const {
        if (!IsValid()) {
            return;
        }

        if (m_IndexCount > 0) {
            glDrawElements(GL_TRIANGLES, m_IndexCount, GL_UNSIGNED_INT, nullptr);
        }
        else {
            glDrawArrays(GL_TRIANGLES, 0, m_VertexCount);
        }
    }

    void Mesh::Shutdown() {
        Release();
    }

    void Mesh::Release() {
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
        m_VertexCount = 0;
        m_IndexCount = 0;
    }
}