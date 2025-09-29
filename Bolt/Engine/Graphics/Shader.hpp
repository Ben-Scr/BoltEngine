#pragma once
#include <string>
#include <cstdint>
#include <glad/glad.h> 

namespace Bolt {
    class Shader {
    public:
        Shader(const std::string& vsPath, const std::string& fsPath);
        ~Shader();

        // In OpenGL ist "Submit" äquivalent zu "glUseProgram".
        // Der view-Parameter wird ignoriert, um die API beizubehalten.
        void Submit(uint16_t view) const;

        GLuint GetHandle() const { return m_Program; }
        bool IsValid() const { return m_IsValid && m_Program != 0; }

        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;
        Shader(Shader&&) noexcept;
        Shader& operator=(Shader&&) noexcept;

    private:
        static GLuint loadAndCompile(GLenum type, const std::string& path);
        GLuint m_Program{ 0 };
        bool m_IsValid{ false };
    };
}