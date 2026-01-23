#include "../pch.hpp"
#include "Renderer2D.hpp"
#include "Camera2D.hpp"

#include "../Scene/SceneManager.hpp"
#include "../Components/SpriteRenderer.hpp"
#include "../Components/ParticleSystem2D.hpp";
#include "../Components/Tags.hpp"

#include "../Scene/Scene.hpp"
#include "../Graphics/TextureManager.hpp"

namespace Bolt {
	void Renderer2D::Initialize() {
		m_QuadMesh.Initialize();
		m_SpriteShader.Initialize();

		if (!m_SpriteShader.IsValid()) {
		//	throw BoltException("Renderer2DException", "Sprite shader is invalid.");
		}

		m_Initialized = true;
	}

	void Renderer2D::BeginFrame() {
		if (!m_Initialized) return;

		glClear(GL_COLOR_BUFFER_BIT);
		RenderScenes();
	}

	void Renderer2D::EndFrame() {

	}

	void Renderer2D::RenderScenes() {
		SceneManager::ForeachLoadedScene([&](const Scene& scene) {
			RenderScene(scene);
			});
	}

	void Renderer2D::RenderScene(const Scene& scene) {
		BOLT_RETURN_IF(!m_SpriteShader.IsValid(), BoltErrorCode::InvalidHandle, "Invalid Sprite 2D Shader");
		m_SpriteShader.Bind();

		// Camera 2D Region
		Camera2D* camera2D = Camera2D::Main();
		BOLT_RETURN_IF(!camera2D, BoltErrorCode::NullReference, "There is no main camera");

		camera2D->UpdateViewport();
		AABB viewportAABB = camera2D->GetViewportAABB();

		const glm::mat4 vp = camera2D->GetViewProjectionMatrix();
		m_SpriteShader.SetMVP(vp);
		// Camera 2D Region

		int renderingSprites = 0;

		for (const auto& [ent, particleSystem] : scene.GetRegistry().view<ParticleSystem2D>(entt::exclude<DisabledTag>).each()) {
			glActiveTexture(GL_TEXTURE0);

			Texture2D* texture = TextureManager::GetTexture(particleSystem.GetTextureHandle());
			if (texture->IsValid())
				texture->Submit(0);

			size_t count = 0;
			for (const auto& particle : particleSystem.GetParticles()) {
				if (!AABB::Intersects(viewportAABB, AABB::FromTransform(particle.Transform)))
					continue;

				m_SpriteShader.SetSpritePosition(particle.Transform.Position);
				m_SpriteShader.SetScale(particle.Transform.Scale);
				m_SpriteShader.SetRotation(particle.Transform.Rotation);
				m_SpriteShader.SetUV(glm::vec2(0.0f), glm::vec2(1.0f));

				m_QuadMesh.Bind();
				m_SpriteShader.SetVertexColor(particle.Color);
				m_QuadMesh.Draw();
				m_QuadMesh.Unbind();
				count++;
			}

			//Logger::Message("ParticleSystem2D", "Rendering" + std::to_string(count) + "/" + std::to_string(particleSystem.m_Particles.size()) + " Particles");
		}

		for (const auto& [ent, tr, spriteRenderer] : scene.GetRegistry().view<Transform2D, SpriteRenderer>(entt::exclude<DisabledTag>).each()) {
			if (!AABB::Intersects(viewportAABB, AABB::FromTransform(tr)))
				continue;

			renderingSprites++;

			m_SpriteShader.SetSpritePosition(tr.Position);
			m_SpriteShader.SetScale(tr.Scale);
			m_SpriteShader.SetRotation(tr.Rotation);
			m_SpriteShader.SetUV(glm::vec2(0.0f), glm::vec2(1.0f));

			glActiveTexture(GL_TEXTURE0);
			Texture2D* texture = TextureManager::GetTexture(spriteRenderer.TextureHandle);

			if (texture->IsValid())
				texture->Submit(0);


			m_QuadMesh.Bind();
			m_SpriteShader.SetVertexColor(spriteRenderer.Color);
			m_QuadMesh.Draw();
			m_QuadMesh.Unbind();
		}

#if 0
		Logger::Message("Rendering " + std::to_string(renderingSprites) + " Sprites");
#endif
		m_SpriteShader.Unbind();
	}

	void Renderer2D::Shutdown() {
		m_QuadMesh.Shutdown();
		m_SpriteShader.Shutdown();
	}
}
