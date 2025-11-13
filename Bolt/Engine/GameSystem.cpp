#include "../pch.hpp"
#include "GameSystem.hpp"
#include "Core/Window.hpp"
#include "Components/Tags.hpp"

namespace Bolt {
	TextureHandle blockTex;

	void GameSystem::Awake(Scene& scene) {
		blockTex = TextureManager::LoadTexture("Assets/Textures/block.png");
	}

	template<typename Tag>
	void CreatePhysicsEntity(Scene& scene, Transform2D transform, BodyType bodyType, Color color = Color::White()) {
		Entity blockEntity = scene.CreateEntity();
	    blockEntity.AddComponent<Tag>();
		SpriteRenderer& sp = blockEntity.AddComponent<SpriteRenderer>();
		sp.TextureHandle = blockTex;
		sp.Color = color;
		auto& tr = blockEntity.GetComponent<Transform2D>();
		tr = transform;

		auto& collider = blockEntity.AddComponent<BoxCollider2D>();
		auto& rb2D = blockEntity.AddComponent<Rigidbody2D>();
		rb2D.SetBodyType(bodyType);
	}
	void CreateCamera(Scene& scene) {
		Entity cameraEnt = scene.CreateEntity();
		Camera2D& camera2D = cameraEnt.AddComponent<Camera2D>();
	}


	void GameSystem::OnCollisionEnter(const Collision2D& collision2D) {
		Scene& activeScene = SceneManager::GetActiveScene();

		bool isADeadly = activeScene.HasComponent<DeadlyTag>(collision2D.entityA);
		bool isBDeadly = activeScene.HasComponent<DeadlyTag>(collision2D.entityB);

		if (isADeadly || isBDeadly) {
			BoxCollider2D& collider2D = m_PlayerEntity.GetComponent<BoxCollider2D>();
			collider2D.SetLayer(0);
		}
	}

	void GameSystem::Start(Scene& scene) {

		AudioHandle handle = AudioManager::LoadAudio("Assets/Audio/Sfx/camera-flash.mp3");
		AudioSource source{ handle };
		AudioManager::PlayAudioSource(source);

		CreatePhysicsEntity<Tag>(scene, Transform2D(Vec2(0, -3), Vec2(100, 1)), BodyType::Static);
		CreatePhysicsEntity<Tag>(scene, Transform2D(Vec2(2, -2), Vec2(1, 1)), BodyType::Dynamic, Color::Cyan());

		CreatePhysicsEntity<DeadlyTag>(scene, Transform2D(Vec2(-2, -2), Vec2(1, 1)), BodyType::Dynamic, Color::Red());
		CreateCamera(scene);

		m_PlayerEntity = scene.CreateEntity();
		auto& spriteRenderer = m_PlayerEntity.AddComponent<SpriteRenderer>();
		m_PlayerEntity.AddComponent<Rigidbody2D>();
		auto& rb2D = m_PlayerEntity.AddComponent<BoxCollider2D>();
		rb2D.SetRegisterContacts(true); 
		rb2D.OnCollisionEnter([this](const Collision2D& collision) {
			this->OnCollisionEnter(collision);
			});
	}

	void GameSystem::Update(Scene& scene) {
		float speed = 5.0f;
		auto& rb2D = m_PlayerEntity.GetComponent<Rigidbody2D>();
		Vec2 input = Input::GetAxis() * speed;
		rb2D.SetVelocity(Vec2(input.x, rb2D.GetVelocity().y));

		if (Input::GetKeyDown(KeyCode::Space)) {
			rb2D.SetVelocity(Vec2(rb2D.GetVelocity().x, 5));
		}

		for (auto [ent, tr, box] : scene.GetRegistry().view<Transform2D, BoxCollider2D>().each()) {
			Gizmos::DrawSquare(box.GetBodyPosition(), box.GetScale(), box.GetRotationDegrees());
		}
	}
}