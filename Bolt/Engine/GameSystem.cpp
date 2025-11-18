#include "../pch.hpp"
#include "GameSystem.hpp"
#include "Core/Window.hpp"
#include "Components/Tags.hpp"
#include "Graphics/OpenGL.hpp"

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
			collider2D.SetLayer(1);
		}

		entt::entity particleEntity = activeScene.GetSingletonEntity<ParticleSystem2D>();
		auto& pts2D = activeScene.GetComponent<ParticleSystem2D>(particleEntity);
		pts2D.ParticleSettings.LifeTime = 5;
		pts2D.ParticleSettings.UseGravity = true;
		pts2D.SetTexture(blockTex);
		Transform2D& transform2D = activeScene.GetComponent<Transform2D>(particleEntity);

		transform2D.Position = activeScene.GetComponent<Transform2D>(collision2D.entityB).Position + Down() / 2.f;
		pts2D.ParticleSettings.Scale = 0.1f;
		pts2D.ParticleSettings.UseRandomColors = true;
		pts2D.Emit(100);

		OpenGL::SetBackgroundColor(Color(Random::Range(0.f, 1.f), Random::Range(0.f, 1.f), Random::Range(0.f, 1.f)));
	}

	void GameSystem::Start(Scene& scene) {

		AudioHandle handle = AudioManager::LoadAudio("Assets/Audio/Sfx/camera-flash.mp3");
		AudioSource source{ handle };
		source.SetVolume(0.1f);
		source.Play(); 

		CreatePhysicsEntity<Tag>(scene, Transform2D(Vec2(0, -3), Vec2(100, 1)), BodyType::Static);
		CreatePhysicsEntity<Tag>(scene, Transform2D(Vec2(2, -2), Vec2(1, 1)), BodyType::Dynamic, Color::Cyan());

		CreatePhysicsEntity<DeadlyTag>(scene, Transform2D(Vec2(-2, -2), Vec2(1, 1)), BodyType::Dynamic, Color::Red());
		scene.CreateCamera();

		Entity ptsEntity = scene.CreateEntity();
		ParticleSystem2D& pts2D = ptsEntity.AddComponent<ParticleSystem2D>();
		pts2D.EmissionSettings.EmitOverTime = 0;
		pts2D.Play();

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
		auto& pts2D = scene.GetSingletonComponent<ParticleSystem2D>();
		pts2D.Update(Time::GetDeltaTime());

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