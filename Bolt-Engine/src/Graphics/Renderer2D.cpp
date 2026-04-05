#include "pch.hpp"
#include "Renderer2D.hpp"

#include "Core/Window.hpp"
#include "Scene/SceneManager.hpp"
#include "Components/Graphics/SpriteRendererComponent.hpp"
#include "Components/Graphics/ParticleSystem2DComponent.hpp"
#include "Components/Graphics/Camera2DComponent.hpp"
#include "Components/Tags.hpp"
#include <Utils/Timer.hpp>

#include "Scene/Scene.hpp"
#include "Graphics/TextureManager.hpp"

#include <glad/glad.h>

namespace Bolt {
	void Renderer2D::Initialize() {
		m_QuadMesh.Initialize();
		m_SpriteShader.Initialize();
		m_Instances.reserve(512);

		BT_ASSERT(m_SpriteShader.IsValid(), BoltErrorCode::InvalidHandle, "Sprite shader is invalid.");
		m_IsInitialized = true;
	}

	void Renderer2D::BeginFrame() {
		Timer timer = Timer();

		if (m_OutputFboId != 0) {
			// Render into the editor viewport FBO.
			// Save the main viewport dimensions before mutating them so they can be restored after.
			const int savedW = Window::GetMainViewport()->GetWidth();
			const int savedH = Window::GetMainViewport()->GetHeight();

			glBindFramebuffer(GL_FRAMEBUFFER, m_OutputFboId);
			glViewport(0, 0, m_OutputWidth, m_OutputHeight);
			Window::GetMainViewport()->SetSize(m_OutputWidth, m_OutputHeight);

			glClear(GL_COLOR_BUFFER_BIT);
			if (m_IsInitialized && m_IsEnabled)
				RenderScenes();
			else
				m_RenderedInstancesCount = 0;

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glViewport(0, 0, savedW, savedH);
			Window::GetMainViewport()->SetSize(savedW, savedH);

			// Consumed — reset so that the next frame renders to the default framebuffer
			// unless the editor registers a new target via SetOutputTarget().
			m_OutputFboId = 0;
		}
		else {
			glClear(GL_COLOR_BUFFER_BIT);
			if (m_IsInitialized && m_IsEnabled)
				RenderScenes();
			else
				m_RenderedInstancesCount = 0;
		}

		m_RenderLoopDuration = timer.ElapsedMilliseconds();
	}

	void Renderer2D::EndFrame() {
		if (!m_IsEnabled) return;
	}

	void Renderer2D::RenderScenes() {
		SceneManager::Get().ForeachLoadedScene([&](const Scene& scene) {
			RenderScene(scene);
			});
	}

	void Renderer2D::RenderScene(const Scene& scene) {
		BT_ASSERT(m_SpriteShader.IsValid(), BoltErrorCode::InvalidHandle, "Invalid Sprite 2D Shader");
		m_SpriteShader.Bind();

		Camera2DComponent* camera2D = Camera2DComponent::Main();

		if (!camera2D) {
			BT_WARN_TAG("Renderer2D", "No main camera found in the scene. Nothing will be rendered.");
			return;
		}

		camera2D->UpdateViewport();
		AABB viewportAABB = camera2D->GetViewportAABB();

		const glm::mat4 vp = camera2D->GetViewProjectionMatrix();
		m_SpriteShader.SetMVP(vp);

		m_Instances.clear();

		auto ptsView = scene.GetRegistry().view<ParticleSystem2DComponent>(entt::exclude<DisabledTag>);
		auto srView = scene.GetRegistry().view<Transform2DComponent, SpriteRendererComponent>(entt::exclude<DisabledTag>);

		m_Instances.reserve(ptsView.size_hint() + srView.size_hint());

		for (const auto& [ent, particleSystem] : ptsView.each()) {
			glActiveTexture(GL_TEXTURE0);

			Texture2D* texture = TextureManager::GetTexture(particleSystem.GetTextureHandle());
			if (texture && texture->IsValid())
				texture->Submit(0);

			for (const auto& particle : particleSystem.GetParticles()) {
				if (!AABB::Intersects(viewportAABB, AABB::FromTransform(particle.Transform)))
					continue;

				m_Instances.emplace_back(
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

			m_Instances.emplace_back(
				tr.Position,
				tr.Scale,
				tr.Rotation,
				spriteRenderer.Color,
				spriteRenderer.TextureHandle,
				spriteRenderer.SortingOrder,
				spriteRenderer.SortingLayer
			);
		}

		std::sort(m_Instances.begin(), m_Instances.end(),
			[](const Instance44& a, const Instance44& b) {
				if (a.SortingLayer != b.SortingLayer)
					return a.SortingLayer < b.SortingLayer;
				return a.SortingOrder < b.SortingOrder;
			});

		for (const Instance44& instance : m_Instances) {
			m_SpriteShader.SetSpritePosition(instance.Position);
			m_SpriteShader.SetScale(instance.Scale);
			m_SpriteShader.SetRotation(instance.Rotation);
			m_SpriteShader.SetUV(glm::vec2(0.0f), glm::vec2(1.0f));

			glActiveTexture(GL_TEXTURE0);

			Texture2D* texture = TextureManager::GetTexture(instance.TextureHandle);
			if (texture && texture->IsValid())
				texture->Submit(0);

			m_QuadMesh.Bind();
			m_SpriteShader.SetVertexColor(instance.Color);
			m_QuadMesh.Draw();
			m_QuadMesh.Unbind();
		}

		m_SpriteShader.Unbind();
		m_RenderedInstancesCount = m_Instances.size();
	}

	void Renderer2D::Shutdown() {
		m_QuadMesh.Shutdown();
		m_SpriteShader.Shutdown();
		m_Instances.clear();
		m_Instances.shrink_to_fit();
	}
}
