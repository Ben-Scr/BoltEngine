#pragma once

namespace Bolt {
    class QuadMesh {
    public:
        void Initialize();
        void Bind() const;
        void Unbind() const;
        void Draw() const;
        void Shutdown();

    private:
        unsigned m_VAO{ 0 };
        unsigned m_VBO{ 0 };
        unsigned m_EBO{ 0 };
    };
}