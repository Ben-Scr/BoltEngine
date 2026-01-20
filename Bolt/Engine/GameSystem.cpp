#include "../pch.hpp"
#include "GameSystem.hpp"
#include "Core/Window.hpp"
#include "Components/Tags.hpp"
#include "Graphics/OpenGL.hpp"
#include "Core/Application.hpp"

namespace Bolt {

	void GameSystem::Awake() {
		blockTex = TextureManager::LoadTexture("Assets/Textures/block.png");
	}

	void GameSystem::OnCollisionEnter(const Collision2D& collision2D) {
		Scene& activeScene = *SceneManager::GetActiveScene();

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

		OpenGL::SetBackgroundColor(Random::NextColor());
	}

	void GameSystem::Start() {
		SceneManager::LoadScene("Test");

		Scene& scene = GetScene();

		//AudioHandle handle = AudioManager::LoadAudio("Assets/Audio/Sfx/camera-flash.mp3");
		//AudioSource source{ handle };
		//source.SetVolume(0.1f);
		//source.Play();

		CreatePhysicsEntity<IdTag>(scene, Transform2D(Vec2(0, -3), Vec2(100, 1)), BodyType::Static);
		CreatePhysicsEntity<IdTag>(scene, Transform2D(Vec2(-49.5f, 47.5f), Vec2(1, 100)), BodyType::Static);
		CreatePhysicsEntity<IdTag>(scene, Transform2D(Vec2(49.5f, 47.5f), Vec2(1, 100)), BodyType::Static);

		CreatePhysicsEntity<IdTag>(scene, Transform2D(Vec2(2, -2), Vec2(1, 1)), BodyType::Dynamic, Color::Cyan());

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

	void GameSystem::Update() {
		Scene& scene = GetScene();

		if (Input::GetKeyDown(KeyCode::R)) {
			SceneManager::ReloadScene(scene.GetName());
			return;
		}

		if (Input::GetKeyDown(KeyCode::P)) {
			Application::Pause(!Application::IsPaused());
		}
		if (Input::GetKeyDown(KeyCode::Q)) {
			Application::Quit();
		}

		if (Input::GetKey(KeyCode::E)) {
			auto mousePos = Camera2D::Main()->ScreenToWorld(Input::GetMousePosition());
			CreatePhysicsEntity<IdTag>(scene, Transform2D(mousePos), BodyType::Dynamic);
		}
		if (Input::GetKeyDown(KeyCode::C)) {
			auto mousePos = Camera2D::Main()->ScreenToWorld(Input::GetMousePosition());

			for (int x = 0; x < 50; x++)
				for (int y = 0; y < 50; y++)
					CreatePhysicsEntity<IdTag>(scene, Transform2D(mousePos + Vec2(x, y)), BodyType::Dynamic);
		}

		Camera2D* mainCam = Camera2D::Main();

		if (Input::GetMouse(MouseKeyCode::Right)) {
			Vec2 dir = Input::GetMouseDelta();
			dir.x *= -1;


			mainCam->AddPosition(dir * Time::GetDeltaTime() * mainCam->GetOrthographicSize());
		}

		mainCam->AddOrthographicSize(-Input::ScrollValue() * Time::GetDeltaTime() * 100);

		auto& pts2D = scene.GetSingletonComponent<ParticleSystem2D>();
		pts2D.Emit(1);

		float speed = 5.0f;
		auto& rb2D = m_PlayerEntity.GetComponent<Rigidbody2D>();
		Vec2 input = Input::GetAxis() * speed;
		rb2D.SetVelocity(Vec2(input.x, rb2D.GetVelocity().y));

		bool isGrounded = Physics2D::Raycast(rb2D.GetPosition(), Down(), 0.6f).has_value();

		if (Input::GetKey(KeyCode::Space) && isGrounded) {
			rb2D.SetVelocity(Vec2(rb2D.GetVelocity().x, 5));
		}

		DrawGizmos();
	}

	void GameSystem::OnApplicationPaused() {


		if (Input::GetKeyDown(KeyCode::P)) {
			Application::Pause(!Application::IsPaused());
		}
	}

	void GameSystem::DrawGizmos() {
		Scene& scene = GetScene();

		if (Input::GetKeyDown(KeyCode::F1))
			Gizmo::SetEnabled(!Gizmo::IsEnabled());

		Gizmo::SetColor(Color::Green());

		for (auto [ent, tr, box] : scene.GetRegistry().view<Transform2D, BoxCollider2D>().each()) {
			Gizmo::DrawSquare(box.GetBodyPosition(), box.GetScale(), box.GetRotationDegrees());
		}

		Gizmo::SetColor(Color::Gray());

		for (auto [ent, tr, box] : scene.GetRegistry().view<Transform2D, BoxCollider2D>().each()) {
			AABB aabb = AABB::FromTransform(tr);
			Gizmo::DrawSquare(tr.Position, aabb.Scale(), 0);
		}
	}
}