#include "../pch.hpp"
#include "GuiRenderer.hpp"

#include "../Scene/SceneManager.hpp"
#include "../Scene/Scene.hpp"
#include "../Graphics/Camera2D.hpp"
#include "../Graphics/TextureManager.hpp"

#include "../Components/Image.hpp"
#include "../Components/RectTransform.hpp"

#include "../Graphics/Shader.hpp"
#include "../Graphics/Instance44.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace Bolt {
	void GuiRenderer::Initialize() {
		m_SpriteShader.Initialize();
		m_QuadMesh.Initialize();

		m_IsInitialized = true;
	}
	void GuiRenderer::Shutdown() {
		m_IsInitialized = false;
	}

	void GuiRenderer::BeginFrame() {
		SceneManager::ForeachLoadedScene([&](const Scene& scene) { RenderScene(scene); });
	}
	void GuiRenderer::EndFrame() {

	}
	void GuiRenderer::RenderScene(const Scene& scene) {
		BOLT_RETURN_IF(!m_SpriteShader.IsValid(), BoltErrorCode::InvalidHandle, "Invalid Sprite 2D Shader");
		m_SpriteShader.Bind();

		// Camera 2D Region
		Camera2D* camera2D = Camera2D::Main();
		BOLT_RETURN_IF(!camera2D, BoltErrorCode::NullReference, "There is no main camera");

		camera2D->UpdateViewport();
		Viewport* vp = camera2D->GetViewport();


		int w = vp->Width;
		int h = vp->Height;

		const float aspect = vp->GetAspect();
		const float halfH = w;
		const float halfW = halfH * aspect;

		const float zNear = 0.0f;
		const float zFar = 100.0f;

		m_SpriteShader.SetMVP(glm::ortho(-halfW, +halfW, -halfH, +halfH, zNear, zFar));
		// Camera 2D Region
		std::vector<Instance44> instances;


		auto guiImageView = scene.GetRegistry().view<RectTransform, Image>(entt::exclude<DisabledTag>);
		instances.reserve(guiImageView.size_hint());

		for (const auto& [ent, rt, guiImage] : guiImageView.each()) {
			Vec2 bl = rt.GetBottomLeft();

			instances.emplace_back(
				bl,
				rt.GetSize(),
				rt.Rotation,
				guiImage.Color,
				guiImage.TextureHandle,
				100,
				100
			);
		}

		std::sort(instances.begin(), instances.end(),
			[](const Instance44& a, const Instance44& b) {
				if (a.SortingLayer != b.SortingLayer)
					return a.SortingLayer < b.SortingLayer;
				return a.SortingOrder < b.SortingOrder;
			});

		glDisable(GL_SCISSOR_TEST);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Final Rendering
		for (const Instance44& instance : instances) {
			m_SpriteShader.SetSpritePosition(instance.Position);
			m_SpriteShader.SetScale(instance.Scale);
			m_SpriteShader.SetRotation(instance.Rotation);
			m_SpriteShader.SetUV(glm::vec2(0.0f), glm::vec2(1.0f));

			glActiveTexture(GL_TEXTURE0);
			TextureHandle handle = instance.TextureHandle;
			if (!handle.IsValid()) {
				handle = TextureManager::GetDefaultTexture(DefaultTexture::Square);
			}
			Texture2D* texture = TextureManager::GetTexture(handle);
			if (texture->IsValid())
				texture->Submit(0);


			m_QuadMesh.Bind();
			m_SpriteShader.SetVertexColor(instance.Color);
			m_QuadMesh.Draw();
			m_QuadMesh.Unbind();
		}

		m_SpriteShader.Unbind();
	}
}