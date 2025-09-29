#include "../pch.hpp"
#include "Renderer2D.hpp"
#include "Camera2D.hpp"

#include "../Collections/Viewport.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>
#include <vector>
#include <cmath>

namespace Bolt {

    // Hilfsfunktionen
    static unsigned createWhiteTexture1x1() {
        unsigned tex = 0;
        unsigned char white[4] = { 255,255,255,255 };
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);
        return tex;
    }

    void Renderer2D::Initialize(const GLInitProperties& glInitProps) {
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            Logger::Error("Glad failed to load");
            return;
        }

        // Clear-Farbe (r, g, b, a)
        Color c = glInitProps.BackgroundColor;
        glClearColor(c.r, c.g, c.b, c.a);

        // Blending für Alpha
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Culling optional
        if (glInitProps.EnableCulling) {
            glEnable(GL_CULL_FACE);
            glCullFace(glInitProps.CullingMode);
        }
        else {
            glDisable(GL_CULL_FACE);
        }

        m_Sprite2DShader.emplace(Shader{ "Assets/Shader/sprite2d.vert.glsl", "Assets/Shader/sprite2d.frag.glsl" });

        // Vertex-Layout: location0 = vec2 position, location1 = vec2 uv
        struct V2UV { float x, y, u, v; };
        const V2UV verts[4] = {
            {-0.5f, -0.5f, 0.0f, 0.0f},
            { 0.5f, -0.5f, 1.0f, 0.0f},
            { 0.5f,  0.5f, 1.0f, 1.0f},
            {-0.5f,  0.5f, 0.0f, 1.0f},
        };
        const unsigned short idx[6] = { 0,1,2, 0,2,3 };

        glGenVertexArrays(1, &mVAO);
        glGenBuffers(1, &mVBO);
        glGenBuffers(1, &mEBO);

        glBindVertexArray(mVAO);
        glBindBuffer(GL_ARRAY_BUFFER, mVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0); // aPos
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(V2UV), (void*)offsetof(V2UV, x));

        glEnableVertexAttribArray(1); // aUV
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(V2UV), (void*)offsetof(V2UV, u));

        // aColor (location = 2) setzen wir pro-Draw als **konstantes** Attribut
        glDisableVertexAttribArray(2);

        glBindVertexArray(0);

        // 1x1 Weißtextur als Fallback
        mWhiteTex = createWhiteTexture1x1();

        // Shader vorbereiten + Uniform-Locations holen
        if (!m_Sprite2DShader.value().IsValid()) {
            Logger::Error("[Renderer2D] Sprite shader invalid.");
            return;
        }
        glUseProgram(m_Sprite2DShader.value().GetHandle());
        uMVP = glGetUniformLocation(m_Sprite2DShader.value().GetHandle(), "uMVP");
        uSpritePos = glGetUniformLocation(m_Sprite2DShader.value().GetHandle(), "uSpritePos");
        uScale = glGetUniformLocation(m_Sprite2DShader.value().GetHandle(), "uScale");
        uRotation = glGetUniformLocation(m_Sprite2DShader.value().GetHandle(), "uRotation");
        uUVOffset = glGetUniformLocation(m_Sprite2DShader.value().GetHandle(), "uUVOffset");
        uUVScale = glGetUniformLocation(m_Sprite2DShader.value().GetHandle(), "uUVScale");
        uPremultipliedAlpha = glGetUniformLocation(m_Sprite2DShader.value().GetHandle(), "uPremultipliedAlpha");
        uAlphaCutoff = glGetUniformLocation(m_Sprite2DShader.value().GetHandle(), "uAlphaCutoff");

        // Textur Sampler auf Einheit 0
        int locTex = glGetUniformLocation(m_Sprite2DShader.value().GetHandle(), "uTexture");
        if (locTex >= 0) glUniform1i(locTex, 0);
        // Standardwerte
        if (uUVOffset >= 0) glUniform2f(uUVOffset, 0.0f, 0.0f);
        if (uUVScale >= 0) glUniform2f(uUVScale, 1.0f, 1.0f);
        if (uPremultipliedAlpha >= 0) glUniform1i(uPremultipliedAlpha, 0);
        if (uAlphaCutoff >= 0) glUniform1f(uAlphaCutoff, 0.0f);

        glUseProgram(0);
    }

    void Renderer2D::BeginFrame() {
        // Viewport & Ortho (Pixel Koords: (0,0) links unten -> (W H) rechts oben)
        GLFWwindow* win = glfwGetCurrentContext();
        if (win) {
            glfwGetFramebufferSize(win, &mViewportW, &mViewportH);
            glViewport(0, 0, mViewportW, mViewportH);
        }
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void Renderer2D::DrawSprite(Vec2 position, Vec2 scale, float rotZ, Color color,
        const Texture2D* texture, Vec2 uvOffset, Vec2 uvScale)
    {
        if (!m_Sprite2DShader.value().IsValid()) return;

        // 1) Shader binden
        m_Sprite2DShader->Submit(0);

        static Camera2D camera2D = Camera2D();
        Camera2D::m_Viewport = std::make_shared<Viewport>(1,1);
        camera2D.SetOrthographicSize(5);


        camera2D.UpdateViewport();

        const glm::mat4 vp = camera2D.GetViewProjectionMatrix();
        if (uMVP >= 0) glUniformMatrix4fv(uMVP, 1, GL_FALSE, glm::value_ptr(vp));

        // 3) Sprite-Parameter
        if (uSpritePos >= 0) glUniform2f(uSpritePos, position.x, position.y);
        if (uScale >= 0) glUniform2f(uScale, scale.x, scale.y);
        if (uRotation >= 0) glUniform1f(uRotation, rotZ);

        // 4) Farbe als konstantes Attribut (location = 2)
        glVertexAttrib4f(2, color.r, color.g, color.b, color.a);

        // 5) UV-Ausschnitt (für Atlas/Spritesheet)
        if (uUVOffset >= 0) glUniform2f(uUVOffset, uvOffset.x, uvOffset.y);
        if (uUVScale >= 0) glUniform2f(uUVScale, uvScale.x, uvScale.y);

        // 6) Textur an Unit 0
        glActiveTexture(GL_TEXTURE0);
        if (texture && texture->IsValid())
            texture->Submit(0);           // bindet GL_TEXTURE_2D
        else
            glBindTexture(GL_TEXTURE_2D, mWhiteTex);

        // Hinweis: uTexture wurde bereits in Initialize auf 0 gesetzt.

        // 7) Draw
        glBindVertexArray(mVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
        glBindVertexArray(0);

        glUseProgram(0);
    }

    void Renderer2D::EndFrame() {

    }

    void Renderer2D::Shutdown() {
        if (mEBO) { glDeleteBuffers(1, &mEBO); mEBO = 0; }
        if (mVBO) { glDeleteBuffers(1, &mVBO); mVBO = 0; }
        if (mVAO) { glDeleteVertexArrays(1, &mVAO); mVAO = 0; }
        if (mWhiteTex) { glDeleteTextures(1, &mWhiteTex); mWhiteTex = 0; }
    }

}
