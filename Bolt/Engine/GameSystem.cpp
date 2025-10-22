#include "../pch.hpp"
#include "GameSystem.hpp"

namespace Bolt {
	TextureHandle blockTexture;
	TextureHandle squareTexture;

	void GameSystem::Awake(Scene& scene) {
		TextureManager::Initialize();
	}

	void CreateEntity(Scene& scene,Transform2D transform, TextureHandle tex, BodyType bodyType = BodyType::Dynamic, Color color = Color::White()) {
		Entity entity = scene.CreateRenderableEntity();
		entity.GetComponent<Transform2D>() = transform;
		auto& collider = entity.AddComponent<BoxCollider2D>();
		auto& rb = entity.AddComponent<Rigidbody2D>();
		rb.SetBodyType(bodyType);

		SpriteRenderer& sp = entity.GetComponent<SpriteRenderer>();
		sp.TextureHandle = tex;
		sp.Color = color;
	}

	void GameSystem::Start(Scene& scene) {

		blockTexture = TextureManager::LoadTexture("Assets/Textures/block.png", Filter::Point, Wrap::Clamp, Wrap::Clamp);
		squareTexture = TextureManager::GetDefaultTexture(DefaultTexture::Square);
		Entity camEntity = scene.CreateEntity();
		Camera2D& camera2D = camEntity.AddComponent<Camera2D>();
		CreateEntity(scene, Transform2D(Vec2(0, -1), Vec2(100, 1), 0), blockTexture, BodyType::Static, Color(1, 0, 0, 1));
	}

	void GameSystem::Update(Scene& scene) {
		Camera2D& camera2D = *Camera2D::Main();
		Vec2 mousePos = camera2D.ScreenToWorld(Input::MousePosition());

		if (Input::GetKeyDown(KeyCode::E)) {
			CreateEntity(scene,Transform2D(mousePos), blockTexture);
		}

		if (Input::GetMouse(MouseKeyCode::Left)) {
			Vec2 delta = Input::MouseDelta() * Time::GetDeltaTime() * camera2D.GetOrthographicSize() * 0.75f;
			delta.x *= -1.f;
			Vec2 targetPos = camera2D.GetPosition() + delta;
			camera2D.SetPosition(targetPos);
		}

		float delta = Input::ScrollValue() * Time::GetDeltaTime() * camera2D.GetOrthographicSize() * -2.f;
		float targetSize = camera2D.GetOrthographicSize() + delta;
		camera2D.SetOrthographicSize(targetSize);
	}
}