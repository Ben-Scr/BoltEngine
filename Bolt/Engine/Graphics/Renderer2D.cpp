#include "../pch.hpp"
#include "Renderer2D.hpp"
#include "Camera2D.hpp"

#include "../Collections/Viewport.hpp"
#include "../Scene/SceneManager.hpp"
#include "../Components/SpriteRenderer.hpp"
#include "../Scene/Scene.hpp"
#include "../Graphics/TextureManager.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>
#include <vector>
#include <cmath>

namespace Bolt {
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


		struct V2UV { float x, y, u, v; };
		const V2UV verts[4] = {
			{-0.5f, -0.5f, 0.0f, 0.0f},
			{ 0.5f, -0.5f, 1.0f, 0.0f},
			{ 0.5f,  0.5f, 1.0f, 1.0f},
			{-0.5f,  0.5f, 0.0f, 1.0f},
		};
		const unsigned short idx[6] = { 0,1,2, 0,2,3 };

		glGenVertexArrays(1, &m_VAO);
		glGenBuffers(1, &m_VBO);
		glGenBuffers(1, &m_EBO);

		glBindVertexArray(m_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0); // aPos
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(V2UV), (void*)offsetof(V2UV, x));

		glEnableVertexAttribArray(1); // aUV
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(V2UV), (void*)offsetof(V2UV, u));

		// aColor (location = 2) setzen wir pro-Draw als **konstantes** Attribut
		glDisableVertexAttribArray(2);

		glBindVertexArray(0);

		if (!m_Sprite2DShader.value().IsValid()) {
			Logger::Error("[Renderer2D] Sprite shader invalid.");
			return;
		}

		glUseProgram(m_Sprite2DShader.value().GetHandle());
		u_MVP = glGetUniformLocation(m_Sprite2DShader.value().GetHandle(), "uMVP");
		u_spritePos = glGetUniformLocation(m_Sprite2DShader.value().GetHandle(), "uSpritePos");
		u_Scale = glGetUniformLocation(m_Sprite2DShader.value().GetHandle(), "uScale");
		u_Rotation = glGetUniformLocation(m_Sprite2DShader.value().GetHandle(), "uRotation");
		u_UVOffset = glGetUniformLocation(m_Sprite2DShader.value().GetHandle(), "uUVOffset");
		u_UVScale = glGetUniformLocation(m_Sprite2DShader.value().GetHandle(), "uUVScale");
		u_PremultipliedAlpha = glGetUniformLocation(m_Sprite2DShader.value().GetHandle(), "uPremultipliedAlpha");
		u_AlphaCutoff = glGetUniformLocation(m_Sprite2DShader.value().GetHandle(), "uAlphaCutoff");


		int locTex = glGetUniformLocation(m_Sprite2DShader.value().GetHandle(), "uTexture");
		if (locTex >= 0) glUniform1i(locTex, 0);

		// Default values
		if (u_UVOffset >= 0) glUniform2f(u_UVOffset, 0.0f, 0.0f);
		if (u_UVScale >= 0) glUniform2f(u_UVScale, 1.0f, 1.0f);
		if (u_PremultipliedAlpha >= 0) glUniform1i(u_PremultipliedAlpha, 0);
		if (u_AlphaCutoff >= 0) glUniform1f(u_AlphaCutoff, 0.0f);

		glUseProgram(0);
	}

	void Renderer2D::BeginFrame() {
		GLFWwindow* win = glfwGetCurrentContext();
		glClear(GL_COLOR_BUFFER_BIT);
		RenderScenes();
	}

	void Renderer2D::EndFrame() {

	}

	void Renderer2D::RenderScenes() {
		for (const std::string& sceneName : SceneManager::GetLoadedSceneNames()) {
			RenderScene(SceneManager::GetLoadedScene(sceneName));
		}
	}

	void Renderer2D::RenderScene(Scene& scene) {
		if (!m_Sprite2DShader.value().IsValid()) {
			Logger::Error("Shader", "Invalid Sprite 2D Shader");
			return;
		}

		m_Sprite2DShader.value().Submit(0);

		/* Camera Region */
		Camera2D* camera2D = Camera2D::Main();
		camera2D->UpdateViewport();

		if (camera2D == nullptr) {
			Logger::Error("Camera 2D", "There is no main camera");
			return;
		}

		const glm::mat4 vp = camera2D->GetViewProjectionMatrix();
		if (u_MVP >= 0) glUniformMatrix4fv(u_MVP, 1, GL_FALSE, glm::value_ptr(vp));
		/* Camera Region End */

		for (const auto& [ent, tr, spriteRenderer] : scene.GetRegistry().view<Transform2D, SpriteRenderer>().each()) {
			if (u_spritePos >= 0) glUniform2f(u_spritePos, tr.Position.x, tr.Position.y);
			if (u_Scale >= 0) glUniform2f(u_Scale, tr.Scale.x, tr.Scale.y);
			if (u_Rotation >= 0) glUniform1f(u_Rotation, tr.Rotation);

			// 4) Farbe als konstantes Attribut (location = 2)
			glVertexAttrib4f(2, spriteRenderer.Color.r, spriteRenderer.Color.g, spriteRenderer.Color.b, spriteRenderer.Color.a);

			// 5) UV-Ausschnitt (für Atlas/Spritesheet)
			if (u_UVOffset >= 0) glUniform2f(u_UVOffset, 0, 0);
			if (u_UVScale >= 0) glUniform2f(u_UVScale, 1, 1);

			// 6) Textur an Unit 0
			glActiveTexture(GL_TEXTURE0);
			Texture2D& texture = TextureManager::GetTexture(spriteRenderer.TextureHandle);



			if (texture.IsValid())
				texture.Submit(0);           // bindet GL_TEXTURE_2D

			else
			{
				Logger::Warning("Invalid Texture");
			}


			// 7) Draw
			glBindVertexArray(m_VAO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
			glBindVertexArray(0);
		}


		glUseProgram(0);
	}

	void Renderer2D::Shutdown() {
		if (m_EBO) { glDeleteBuffers(1, &m_EBO); m_EBO = 0; }
		if (m_VBO) { glDeleteBuffers(1, &m_VBO); m_VBO = 0; }
		if (m_VAO) { glDeleteVertexArrays(1, &m_VAO); m_VAO = 0; }
		if (m_VAO) { glDeleteVertexArrays(1, &m_VAO); m_VAO = 0; }
	}

}
