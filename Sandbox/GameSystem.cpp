#include "GameSystem.hpp"
#include "Core/Window.hpp"
#include "Components/Tags.hpp"
#include "Graphics/OpenGL.hpp"
#include "Core/Application.hpp"


void GameSystem::Awake() {
	blockTex = TextureManager::LoadTexture("../Assets/Textures/block.png");
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
	Scene& scene = GetScene();

	//AudioHandle handle = AudioManager::LoadAudio("Assets/Audio/Sfx/camera-flash.mp3");
	//AudioSource source{ handle };
	//source.SetVolume(0.1f);
	//source.Play();

	//CreatePhysicsEntity<IdTag>(scene, Transform2D(Vec2(0, -3), Vec2(100, 1)), BodyType::Static);
	CreatePhysicsEntity(scene, Transform2D(Vec2(-49.5f, 47.5f), Vec2(1, 100)), BodyType::Static);
	CreatePhysicsEntity(scene, Transform2D(Vec2(49.5f, 47.5f), Vec2(1, 100)), BodyType::Static);

	CreatePhysicsEntity(scene, Transform2D(Vec2(2, -2), Vec2(1, 1)), BodyType::Dynamic, Color::Cyan());

	Entity deadlyBlock = CreatePhysicsEntity(scene, Transform2D(Vec2(-2, -2), Vec2(1, 1)), BodyType::Dynamic, Color::Red());
	deadlyBlock.AddComponent<DeadlyTag>();
	scene.CreateCamera();
	OpenGL::SetBackgroundColor(Color(0.1f, 0.1f, 0.1f));

	Entity ptsEntity = scene.CreateEntity();
	ParticleSystem2D& pts2D = ptsEntity.AddComponent<ParticleSystem2D>();
	pts2D.SetTexture(TextureManager::GetDefaultTexture(DefaultTexture::Circle));
	pts2D.RenderingSettings.Color = Color(1.f, 0.4f, 0.f, 0.1f);
	pts2D.Shape = SquareParams(Vec2(0.2f, 0.2f));
	pts2D.EmissionSettings.EmitOverTime = 0;
	pts2D.RenderingSettings.SortingLayer;
	pts2D.ParticleSettings.Scale = 0.2f;
	pts2D.RenderingSettings.MaxParticles = 100'000;
	pts2D.Play();

	m_PlayerEntity = scene.CreateEntity();
	auto& spriteRenderer = m_PlayerEntity.AddComponent<SpriteRenderer>();
	spriteRenderer.TextureHandle = TextureManager::LoadTexture("../Assets/Textures/Player.png");;
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

	auto mousePos = Camera2D::Main()->ScreenToWorld(Input::GetMousePosition());

	if (Input::GetKey(KeyCode::E)) {
		
		CreatePhysicsEntity(scene, Transform2D(mousePos), BodyType::Dynamic);
	}
	if (Input::GetKeyDown(KeyCode::C)) {
		auto mousePos = Camera2D::Main()->ScreenToWorld(Input::GetMousePosition());

		for (int x = 0; x < 50; x++)
			for (int y = 0; y < 50; y++)
				CreatePhysicsEntity(scene, Transform2D(mousePos + Vec2(x, y)), BodyType::Dynamic);
	}



	float speed = Input::GetKey(KeyCode::LeftShift) ? 20.f : 10.0f;
	auto& playerTr = m_PlayerEntity.GetComponent<Transform2D>();
	Vec2 input = speed * Time::GetDeltaTime() * playerTr.GetForwardDirection();
	playerTr.Position += input;

	auto entity = scene.GetSingletonEntity<ParticleSystem2D>();
	auto& pts2D = scene.GetComponent<ParticleSystem2D>(entity);
	auto& tr = scene.GetComponent<Transform2D>(entity);
	pts2D.ParticleSettings.MoveDirection = -playerTr.GetForwardDirection();
	pts2D.Emit(10);

	tr.Position = playerTr.Position - (playerTr.GetForwardDirection() * 0.4f);

	Vec2 lookDir = mousePos - playerTr.Position;
	float lookAtZ = atan2(lookDir.x, lookDir.y);
	float t = 0.1f; // [0..1]
	float delta = std::remainder(lookAtZ - playerTr.Rotation, 2.0f * 3.14159265358979323846f);
	playerTr.Rotation += delta * t;

	Camera2D* mainCam = Camera2D::Main();

	float lerpSpeed = 0.01f;
	Vec2 targetPos = playerTr.Position;
	Vec2 curPos = mainCam->GetPosition();
	Vec2 lerpPos = curPos + (targetPos - curPos) * lerpSpeed;

	mainCam->SetPosition(lerpPos);
	mainCam->AddOrthographicSize(-Input::ScrollValue() * Time::GetDeltaTime() * 100);

	bool isGrounded = Physics2D::Raycast(playerTr.Position, Down(), 0.6f).has_value();

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