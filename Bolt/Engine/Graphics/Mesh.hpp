#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <cstdint>
#include <vector>

namespace Bolt {
    struct MeshVertex {
        glm::vec3 Position{};
        glm::vec3 Normal{ 0.0f, 1.0f, 0.0f };
        glm::vec2 TexCoord{ 0.0f, 0.0f };
    };

    class Mesh {
    public:
        Mesh() = default;
        Mesh(const Mesh&) = delete;
        Mesh& operator=(const Mesh&) = delete;
        Mesh(Mesh&& other) noexcept;
        Mesh& operator=(Mesh&& other) noexcept;
        ~Mesh();

        void SetData(const std::vector<MeshVertex>& vertices, const std::vector<uint32_t>& indices);
        void Bind() const;
        void Unbind() const;
        void Draw() const;
        void Shutdown();

        bool IsValid() const { return m_VAO != 0 && m_VertexCount > 0; }

    private:
        void Release();

        GLuint m_VAO{ 0 };
        GLuint m_VBO{ 0 };
        GLuint m_EBO{ 0 };
        GLsizei m_VertexCount{ 0 };
        GLsizei m_IndexCount{ 0 };
    };
}
