#include "../pch.hpp"
#include "GameSystem.hpp"
#include "Scene/Scene.hpp"
#include "Components/SpriteRenderer.hpp"
#include "Components/Transform2D.hpp"
#include "Graphics/Texture2D.hpp"
#include "Graphics/TextureManager.hpp"

namespace Bolt {
	void GameSystem::Awake(Scene& scene) {
		TextureManager::Initialize();
	}

	void GameSystem::Start(Scene& scene) {
		Entity entity = scene.CreateRenderableEntity();

		auto handle = TextureManager::LoadTexture("Assets/Textures/Square.png",Filter::Trilinear, Wrap::Clamp, Wrap::Clamp);

		entity.GetComponent<SpriteRenderer>().TextureHandle = handle;
		entity.GetComponent<Transform2D>().Scale = {128, 128};
	}

	void GameSystem::Update(Scene& scene) {

	}
}