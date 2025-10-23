#pragma once
#include <string>
#include <cstdint>
#include <glad/glad.h> 

namespace Bolt {
    class Shader {
    public:
        Shader(const std::string& vsPath, const std::string& fsPath);
        ~Shader();

        void Submit() const;

        GLuint GetHandle() const { return m_Program; }
        bool IsValid() const { return m_IsValid && m_Program != 0; }

        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;
        Shader(Shader&&) noexcept;
        Shader& operator=(Shader&&) noexcept;

    private:
        static GLuint LoadAndCompile(GLenum type, const std::string& path);
        GLuint m_Program{ 0 };
        bool m_IsValid{ false };
    };
}