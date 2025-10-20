#include "../pch.hpp"
#include "GameSystem.hpp"

namespace Bolt {
	void GameSystem::Awake(Scene& scene) {
		TextureManager::Initialize();
	}

	void GameSystem::Start(Scene& scene) {
		Entity entity = scene.CreateRenderableEntity();
		auto handle = TextureManager::LoadTexture("Assets/Textures/block.png", Filter::Point, Wrap::Clamp, Wrap::Clamp);

		SpriteRenderer& sp = entity.GetComponent<SpriteRenderer>();
		sp.TextureHandle = handle;
		sp.Color = Color::White();
		entity.GetComponent<Transform2D>().Scale = { 1, 1 };
	
		Entity camEntity = scene.CreateEntity();
		Camera2D& camera2D = camEntity.AddComponent<Camera2D>();
	}

	void GameSystem::Update(Scene& scene) {

	}
}