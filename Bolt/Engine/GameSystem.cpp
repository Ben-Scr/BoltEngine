#include "../pch.hpp"
#include "GameSystem.hpp"
#include "Core/Window.hpp"
#include "Components/Tags.hpp"

namespace Bolt {


	void GameSystem::Awake(Scene& scene) {
		blockTex = TextureManager::LoadTexture("Assets/Textures/block.png");
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
		source.SetVolume(1.f);
		source.Play();

		CreatePhysicsEntity<Tag>(scene, Transform2D(Vec2(0, -3), Vec2(100, 1)), BodyType::Static);
		CreatePhysicsEntity<Tag>(scene, Transform2D(Vec2(2, -2), Vec2(1, 1)), BodyType::Dynamic, Color::Cyan());

		CreatePhysicsEntity<DeadlyTag>(scene, Transform2D(Vec2(-2, -2), Vec2(1, 1)), BodyType::Dynamic, Color::Red());
		scene.CreateCamera();

		m_PlayerEntity = scene.CreateEntity();
		auto& spriteRenderer = m_PlayerEntity.AddComponent<SpriteRenderer>();
		m_PlayerEntity.AddComponent<Rigidbody2D>();

		auto& boxCollider2D = m_PlayerEntity.AddComponent<BoxCollider2D>();
		boxCollider2D.SetRegisterContacts(true); 
		boxCollider2D.OnCollisionEnter([this](const Collision2D& collision) {
			this->OnCollisionEnter(collision);
			});
	}

	void GameSystem::Update(Scene& scene) {
		float speed = 5.0f;
		auto& rb2D = m_PlayerEntity.GetComponent<Rigidbody2D>();
		Vec2 input = Input::GetAxis() * speed;
		rb2D.SetVelocity(Vec2(input.x, rb2D.GetVelocity().y));

		bool isGrounded = Physics2D::Raycast(rb2D.GetPosition(), Down(), 0.6f).has_value();

		if (Input::GetKey(KeyCode::Space) && isGrounded) {
			rb2D.SetVelocity(Vec2(rb2D.GetVelocity().x, 5));
		}

		for (auto [ent, tr, box] : scene.GetRegistry().view<Transform2D, BoxCollider2D>().each()) {
			Gizmos::DrawSquare(box.GetBodyPosition(), box.GetScale(), box.GetRotationDegrees());
		}
	}
}