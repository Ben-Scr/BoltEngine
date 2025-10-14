#include "../pch.hpp"
#include "GameSystem.hpp"
#include "Scene/Scene.hpp"
#include "Components/SpriteRenderer.hpp"
#include "Components/Transform2D.hpp"
#include "Graphics/Camera2D.hpp"
#include "Graphics/Texture2D.hpp"
#include "Graphics/TextureManager.hpp"
#include "Collections/Viewport.hpp"
#include <memory>

namespace Bolt {
	void GameSystem::Awake(Scene& scene) {
		TextureManager::Initialize();
	}

	void GameSystem::Start(Scene& scene) {
		Entity entity = scene.CreateRenderableEntity();

		auto handle = TextureManager::LoadTexture("Assets/Textures/Square.png", Filter::Trilinear, Wrap::Clamp, Wrap::Clamp);

		SpriteRenderer& sp = entity.GetComponent<SpriteRenderer>();
		sp.TextureHandle = handle;
		sp.Color = Color::Red();
		entity.GetComponent<Transform2D>().Scale = { 0.5, 1 };


		Entity camEntity = scene.CreateEntity();
		std::shared_ptr<Viewport> viewport = Camera2D::GetSharedViewport();
		if (!viewport) {
			viewport = std::make_shared<Viewport>(Viewport(1, 1));
			Camera2D::SetSharedViewport(viewport);
		}
		Camera2D* camera2D = &camEntity.AddComponent<Camera2D>();

		if (viewport) {
			camera2D->UpdateViewport();
			camera2D->SetOrthographicSize(0.5f * static_cast<float>(viewport->GetHeight()));
		}
	}

	void GameSystem::Update(Scene& scene) {

	}
}