#include "../pch.hpp"
#include "GameSystem.hpp"
#include "Core/Window.hpp"

namespace Bolt {
	void GameSystem::Awake(Scene& scene) {

	}


	void GameSystem::Start(Scene& scene) {
		Entity entity = scene.CreateEntity();
		entity.AddComponent<Transform2D>();
		SpriteRenderer& sp = entity.AddComponent<SpriteRenderer>();
		sp.TextureHandle = TextureManager::GetDefaultTexture(DefaultTexture::Square());

		Entity cameraEnt = scene.CreateEntity();
		cameraEnt.AddComponent<Transform2D>();
		Camera2D& camera2D = cameraEnt.AddComponent<Camera2D>();
	}

	void GameSystem::Update(Scene& scene) {

	}
}