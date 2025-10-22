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

		Color c = glInitProps.BackgroundColor;
		glClearColor(c.r, c.g, c.b, c.a);


		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


		if (glInitProps.EnableCulling) {
			glEnable(GL_CULL_FACE);
			glCullFace(glInitProps.CullingMode);
		}
		else {
			glDisable(GL_CULL_FACE);
		}

		m_QuadMesh.Initialize();
		m_SpriteShader.Initialize();
		if (!m_SpriteShader.IsValid()) {
			Logger::Error("[Renderer2D] Sprite shader invalid.");
			return;
		}
	}

	void Renderer2D::BeginFrame() {
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
		if (!m_SpriteShader.IsValid()) {
			Logger::Error("Shader", "Invalid Sprite 2D Shader");
			return;
		}

		m_SpriteShader.Bind();

		/* Camera Region */
		Camera2D* camera2D = Camera2D::Main();
		camera2D->UpdateViewport();
		AABB viewportAABB = camera2D->GetViewportAABB();

		if (camera2D == nullptr) {
			Logger::Error("Camera 2D", "There is no main camera");
			return;
		}

		const glm::mat4 vp = camera2D->GetViewProjectionMatrix();
		m_SpriteShader.SetMVP(vp);
		/* Camera Region End */

		int renderingSprites = 0;

		for (const auto& [ent, tr, spriteRenderer] : scene.GetRegistry().view<Transform2D, SpriteRenderer>().each()) {
			if (!AABB::Intersects(viewportAABB, AABB::FromTransform(tr)))
				continue;

			renderingSprites++;

			m_SpriteShader.SetSpritePosition(tr.Position);
			m_SpriteShader.SetScale(tr.Scale);
			m_SpriteShader.SetRotation(tr.Rotation);
			m_SpriteShader.SetUV(glm::vec2(0.0f), glm::vec2(1.0f));

			glActiveTexture(GL_TEXTURE0);
			Texture2D& texture = TextureManager::GetTexture(spriteRenderer.TextureHandle);



			if (texture.IsValid())
				texture.Submit(0);

			else
			{
				Logger::Warning("Invalid Texture");
			}


			m_QuadMesh.Bind();
			m_SpriteShader.SetVertexColor(spriteRenderer.Color);
			m_QuadMesh.Draw();
			m_QuadMesh.Unbind();
		}

		Logger::Message("Rendering " + std::to_string(renderingSprites) + " Sprites");

		m_SpriteShader.Unbind();
	}

	void Renderer2D::Shutdown() {
		m_QuadMesh.Shutdown();
		m_SpriteShader.Shutdown();
	}
}
