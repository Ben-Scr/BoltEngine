#include "pch.hpp"
#include "Renderer2D.hpp"
#include "Camera2D.hpp"

#include "Scene/SceneManager.hpp"
#include "Components/SpriteRenderer.hpp"
#include "Components/ParticleSystem2D.hpp";
#include "Components/Tags.hpp"

#include "Scene/Scene.hpp"
#include "Graphics/TextureManager.hpp"

namespace Bolt {
	void Renderer2D::Initialize() {
		m_QuadMesh.Initialize();
		m_SpriteShader.Initialize();

		BOLT_ASSERT(m_SpriteShader.IsValid(), BoltErrorCode::InvalidHandle, "Sprite shader is invalid.");
		m_Initialized = true;
	}

	void Renderer2D::BeginFrame() {
		glClear(GL_COLOR_BUFFER_BIT);

		if (!m_Initialized || !m_IsEnabled) return;

		RenderScenes();
	}

	void Renderer2D::EndFrame() {
		if (!m_IsEnabled) return;
	}

	void Renderer2D::RenderScenes() {
		Timer timer = Timer::Start();
		SceneManager::ForeachLoadedScene([&](const Scene& scene) {
			RenderScene(scene);
			});
		m_RenderLoopDuration = timer.ElapsedMilliseconds();
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

		int renderingSprites = 0;

		std::vector<Instance44> instances;

		auto ptsView = scene.GetRegistry().view<ParticleSystem2D>(entt::exclude<DisabledTag>);
		auto srView = scene.GetRegistry().view<Transform2D, SpriteRenderer>(entt::exclude<DisabledTag>);

		instances.reserve(ptsView.size_hint() + srView.size_hint());

		for (const auto& [ent, particleSystem] : ptsView.each()) {
			glActiveTexture(GL_TEXTURE0);

			Texture2D* texture = TextureManager::GetTexture(particleSystem.GetTextureHandle());
			if (texture->IsValid())
				texture->Submit(0);

			for (const auto& particle : particleSystem.GetParticles()) {
				if (!AABB::Intersects(viewportAABB, AABB::FromTransform(particle.Transform)))
					continue;


				instances.emplace_back(
					particle.Transform.Position,
					particle.Transform.Scale,
					particle.Transform.Rotation,
					particleSystem.RenderingSettings.Color,
					particleSystem.m_TextureHandle,
					particleSystem.RenderingSettings.SortingOrder,
					particleSystem.RenderingSettings.SortingLayer
				);
			}
		}

		for (const auto& [ent, tr, spriteRenderer] : srView.each()) {
			if (!AABB::Intersects(viewportAABB, AABB::FromTransform(tr)))
				continue;

			instances.emplace_back(
				tr.Position,
				tr.Scale,
				tr.Rotation,
				spriteRenderer.Color,
				spriteRenderer.TextureHandle,
				spriteRenderer.SortingOrder,
				spriteRenderer.SortingLayer
			);
		}

		std::sort(instances.begin(), instances.end(),
			[](const Instance44& a, const Instance44& b) {
				if (a.SortingLayer != b.SortingLayer)
					return a.SortingLayer < b.SortingLayer;
				return a.SortingOrder < b.SortingOrder;
			});

		// Info: Final Rendering
		for (const Instance44& instance : instances) {
			m_SpriteShader.SetSpritePosition(instance.Position);
			m_SpriteShader.SetScale(instance.Scale);
			m_SpriteShader.SetRotation(instance.Rotation);
			m_SpriteShader.SetUV(glm::vec2(0.0f), glm::vec2(1.0f));

			glActiveTexture(GL_TEXTURE0);
			Texture2D* texture = TextureManager::GetTexture(instance.TextureHandle);

			if (texture->IsValid())
				texture->Submit(0);


			m_QuadMesh.Bind();
			m_SpriteShader.SetVertexColor(instance.Color);
			m_QuadMesh.Draw();
			m_QuadMesh.Unbind();
		}

		m_SpriteShader.Unbind();
		m_RenderedInstancesCount = instances.size();
	}

	void Renderer2D::Shutdown() {
		m_QuadMesh.Shutdown();
		m_SpriteShader.Shutdown();
	}
}
