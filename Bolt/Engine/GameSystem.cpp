#include "../pch.hpp"
#include "GameSystem.hpp"

namespace Bolt {
	TextureHandle handle;

	void GameSystem::Awake(Scene& scene) {
		TextureManager::Initialize();
	}

	void CreateEntity(Scene& scene,Transform2D transform, TextureHandle tex, BodyType bodyType = BodyType::Dynamic) {
		Entity entity = scene.CreateRenderableEntity();
		entity.GetComponent<Transform2D>() = transform;
		auto& collider = entity.AddComponent<BoxCollider2D>();
		auto& rb = entity.AddComponent<Rigidbody2D>();
		rb.SetBodyType(bodyType);

		SpriteRenderer& sp = entity.GetComponent<SpriteRenderer>();
		sp.TextureHandle = tex;
		sp.Color = Color::White();
	}

	void GameSystem::Start(Scene& scene) {

		handle = TextureManager::LoadTexture("Assets/Textures/block.png", Filter::Point, Wrap::Clamp, Wrap::Clamp);
		Entity camEntity = scene.CreateEntity();
		Camera2D& camera2D = camEntity.AddComponent<Camera2D>();
		CreateEntity(scene, Transform2D::FromPosition(Vec2(0, -1)), handle, BodyType::Static);
	}

	void GameSystem::Update(Scene& scene) {
		Camera2D& camera2D = *Camera2D::Main();
		Vec2 mousePos = camera2D.ScreenToWorld(Input::MousePosition());

		if (Input::GetKeyDown(KeyCode::E)) {
			CreateEntity(scene,Transform2D::FromPosition(mousePos), handle);
		}

		if (Input::GetMouse(1)) {
			Vec2 delta = Input::MouseDelta() * Time::GetDeltaTime() * camera2D.GetOrthographicSize() * 0.5f;
			delta.x *= -1.f;
			Vec2 targetPos = camera2D.GetPosition() + delta;
			camera2D.SetPosition(targetPos);
		}

		float delta = Input::ScrollValue() * Time::GetDeltaTime() * camera2D.GetOrthographicSize() * -2.f;
		float targetSize = camera2D.GetOrthographicSize() + delta;
		camera2D.SetOrthographicSize(targetSize);
	}
}