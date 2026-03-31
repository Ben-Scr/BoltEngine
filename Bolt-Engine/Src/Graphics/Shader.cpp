#include "pch.hpp"
#include "Shader.hpp"

#include <vector>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>

namespace Bolt {
    static bool ReadFileToString(const std::string& path, std::string& out) {
        std::ifstream input(path, std::ios::binary);
        if (!input.is_open()) {
            return false;
        }

        std::ostringstream buffer;
        buffer << input.rdbuf();
        out = buffer.str();
        return !out.empty();
    }

    GLuint Shader::LoadAndCompile(GLenum type, const std::string& path) {
        std::string src;

        BT_ASSERT(ReadFileToString(path, src), BoltErrorCode::LoadFailed, "Failed to read shader file: " + path);

        GLuint shader = glCreateShader(type);
        if (shader == 0) {
           BT_ERROR_TAG("Shader", "Failed to create shader object for file: " + path);
            return 0;
        }
        const char* csrc = src.c_str();
        glShaderSource(shader, 1, &csrc, nullptr);
        glCompileShader(shader);

        GLint ok = GL_FALSE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);

        if (ok != GL_TRUE) {
            GLint logLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
            std::vector<GLchar> log(std::max(1, logLen));
            glGetShaderInfoLog(shader, logLen, nullptr, log.data());

            const char* typeStr = (type == GL_VERTEX_SHADER) ? "vertex" :
                (type == GL_FRAGMENT_SHADER) ? "fragment" : "unknown";
            BT_ERROR_TAG("Shader", std::string("Compile failed (") + typeStr + "): " + path + "\n" + log.data());

            glDeleteShader(shader);
            return 0;
        }

        return shader;
    }

    Shader::Shader(const std::string& vsPath, const std::string& fsPath) {
        GLuint vs = LoadAndCompile(GL_VERTEX_SHADER, vsPath);
        if (vs == 0) return;

        GLuint fs = LoadAndCompile(GL_FRAGMENT_SHADER, fsPath);
        if (fs == 0) { glDeleteShader(vs); return; }

        m_Program = glCreateProgram();
        if (m_Program == 0) {
            BT_ERROR_TAG("Shader", "Failed to create shader program for files: " + vsPath + " + " + fsPath);
            glDeleteShader(vs);
            glDeleteShader(fs);
            return;
        }
        glAttachShader(m_Program, vs);
        glAttachShader(m_Program, fs);
        glLinkProgram(m_Program);

        glDeleteShader(vs);
        glDeleteShader(fs);

        GLint linked = GL_FALSE;
        glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
        if (linked != GL_TRUE) {
            GLint logLen = 0;
            glGetProgramiv(m_Program, GL_INFO_LOG_LENGTH, &logLen);
            std::vector<GLchar> log(std::max(1, logLen));
            glGetProgramInfoLog(m_Program, logLen, nullptr, log.data());

            BT_ERROR_TAG("Shader", std::string("Program link failed : ") + vsPath + " + " + fsPath + "\n" + log.data());

            glDeleteProgram(m_Program);
            m_Program = 0;
            return;
        }

        m_IsValid = true;
    }

    Shader::~Shader() {
        if (m_Program != 0) {
            glDeleteProgram(m_Program);
            m_Program = 0;
        }
    }

    Shader::Shader(Shader&& o) noexcept {
        m_Program = o.m_Program;
        m_IsValid = o.m_IsValid;
        o.m_Program = 0;
        o.m_IsValid = false;
    }

    Shader& Shader::operator=(Shader&& o) noexcept {
        if (this != &o) {
            if (m_Program != 0) {
                glDeleteProgram(m_Program);
            }
            m_Program = o.m_Program;
            m_IsValid = o.m_IsValid;
            o.m_Program = 0;
            o.m_IsValid = false;
        }
        return *this;
    }

    void Shader::Submit() const {
        if (IsValid()) {
            glUseProgram(m_Program);
        }
        else {
            glUseProgram(0);
        }
    }

}