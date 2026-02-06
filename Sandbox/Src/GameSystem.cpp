#include "GameSystem.hpp"
#include "Components/Tags.hpp"
#include "Graphics/OpenGL.hpp"
#include "Scene/EntityHelper.hpp"
#include "Utils/Timer.hpp"
#include "Components/Name.hpp"

void GameSystem::Awake() {
	Gizmo::SetEnabled(false);
	m_AsteriodTexture = TextureManager::LoadTexture("../Assets/Textures/Meteor.png");
	m_LightTexture = TextureManager::LoadTexture("../Assets/Textures/Light.png");
	m_PlayerTexture = TextureManager::LoadTexture("../Assets/Textures/Player.png");
	m_LaserTexture = TextureManager::LoadTexture("../Assets/Textures/Laser.png");
}


void GameSystem::Start() {
	SceneManager::RegisterComponentType<NameTag>(ComponentInfo("Name", "Component"));
	SceneManager::RegisterComponentType<Transform2D>(ComponentInfo("Transform", "Component"));


	Scene& scene = GetScene();

	EntityHelper::CreateCamera2DEntity();
	OpenGL::SetClearColor(Color(0.1f, 0.1f, 0.1f));

	m_PlayerEmissionPts = scene.CreateEntity();
	ParticleSystem2D& pts2D = m_PlayerEmissionPts.AddComponent<ParticleSystem2D>();
	pts2D.EmissionSettings.EmitOverTime = 1000;
	pts2D.SetTexture(TextureManager::GetDefaultTexture(DefaultTexture::Circle));

	auto ptColor = Color(1.0f, 0.45f, 0.f, 0.1f);
	pts2D.RenderingSettings.Color = ptColor;
	pts2D.Shape = SquareParams(Vec2(0.2f, 0.2f));
	pts2D.ParticleSettings.LifeTime = 10;
	pts2D.ParticleSettings.Scale = 0.2f;
	pts2D.ParticleSettings.Gravity = Vec2(0, 1);
	pts2D.RenderingSettings.MaxParticles = 100'000;
	pts2D.RenderingSettings.SortingLayer = 0;
	pts2D.Play();

	m_PlayerEntity = scene.CreateEntity("Player");
	m_PlayerEntity.AddComponent<ImageComponent>();
	auto& spriteRenderer = m_PlayerEntity.AddComponent<SpriteRenderer>();
	spriteRenderer.TextureHandle = m_PlayerTexture;
}

struct LaserTag {};

struct AsteriodData {
	float Speed;
	float RotationSpeed;
	float LifeTime;
};

void GameSystem::Update() {
	Scene& scene = GetScene();
	auto mousePos = Camera2D::Main()->ScreenToWorld(Input::GetMousePosition());
	bool cntrlDown = Input::GetKey(KeyCode::LeftControl);

	if (Input::GetKeyDown(KeyCode::R) && cntrlDown) {
		SceneManager::ReloadScene(scene.GetName());
		return;
	}
	if (Input::GetKeyDown(KeyCode::O)) {
		for (int x = 0; x < 50; x++)
			for (int y = 0; y < 50; y++) {
				auto& tr = EntityHelper::CreateSpriteEntity().GetComponent<Transform2D>();
				tr.Position = Vec2(x, y) + mousePos;
			}
	}
	if (Input::GetKeyDown(KeyCode::P)) {
		Application::Pause(!Application::IsPaused());
	}

	if (Input::GetKeyDown(KeyCode::Q)) {
		Application::Quit();
	}

	if (Input::GetKeyDown(KeyCode::F2)) {
		Application::Reload();
	}

	if (Input::GetKey(KeyCode::E)) {
		Entity ent = EntityHelper::CreateWith<Transform2D, SpriteRenderer>();
		SpriteRenderer& sp = ent.GetComponent<SpriteRenderer>();
		sp.TextureHandle = m_AsteriodTexture;


		AsteriodData& asteriod = ent.AddComponent<AsteriodData>();
		asteriod.Speed = Random::NextFloat(2.f, 3.f);
		asteriod.LifeTime = 15.f;
		asteriod.RotationSpeed = 14.f;

		Transform2D& tr2D = ent.GetComponent<Transform2D>();
		auto playerTransform = m_PlayerEntity.GetComponent<Transform2D>();
		tr2D = playerTransform.Position + RandomOnCircle(10.f);

		float scale = Random::NextFloat(1.f, 5.f);
		tr2D.Scale = Vec2(scale, scale);
	}

	if (Input::GetKeyDown(KeyCode::Space)) {
		Entity ent = scene.CreateEntity();
		SpriteRenderer& sp = ent.AddComponent<SpriteRenderer>();
		sp.TextureHandle = m_LaserTexture;

		ent.AddComponent<LaserTag>();
		Transform2D& tr2D = ent.GetComponent<Transform2D>();
		tr2D = m_PlayerEntity.GetComponent<Transform2D>();
		tr2D.Scale = Vec2(0.26f, 1.08f) * 0.75f;
	}

	UpdatePlayerPts();
	PlayerMovement();
	MoveEntities();
	CameraMovement();
	DrawGizmos();
}

void GameSystem::UpdatePlayerPts() {
	auto& playerTr = m_PlayerEntity.GetComponent<Transform2D>();
	auto& pts2D = m_PlayerEmissionPts.GetComponent<ParticleSystem2D>();
	auto& tr = m_PlayerEmissionPts.GetComponent<Transform2D>();
	pts2D.ParticleSettings.MoveDirection = -playerTr.GetForwardDirection();
	tr.Position = playerTr.Position - (playerTr.GetForwardDirection() * 0.4f);
}


void GameSystem::PlayerMovement() {
	Scene& scene = GetScene();

	auto& playerTr = m_PlayerEntity.GetComponent<Transform2D>();
	auto mousePos = Camera2D::Main()->ScreenToWorld(Input::GetMousePosition());

	float speed = Input::GetKey(KeyCode::LeftShift) ? 15.f : 5.0f;

	static bool control = true;
	if (Input::GetKeyDown(KeyCode::F3))
	{
		control = !control;
		if (!control)
		{
			auto& pts = m_PlayerEmissionPts.GetComponent<ParticleSystem2D>();
			pts.Play();
		}
	}

	Vec2 controlInput = Input::GetAxis().y * playerTr.GetForwardDirection();
	Vec2 input = speed * (!control ? playerTr.GetForwardDirection() : controlInput) * Time::GetDeltaTime();

	if (control)
	{
		playerTr.Rotation += Input::GetAxis().x * 10 * Time::GetDeltaTime();

		if (LengthSquared(controlInput) == 0)
			m_PlayerEmissionPts.GetComponent<ParticleSystem2D>().Stop();
		else
			m_PlayerEmissionPts.GetComponent<ParticleSystem2D>().Play();
	}

	if (control || Distance(playerTr.Position, mousePos) > 0.25f)
		playerTr.Position += input;

	if (!control)
		playerTr.Rotation = LookAt2D(playerTr, mousePos) * 14;
}

void GameSystem::CameraMovement() {
	static bool followPlayer = false;

	Transform2D& playerTr = m_PlayerEntity.GetComponent<Transform2D>();
	Camera2D* mainCam = Camera2D::Main();
	auto mousePos = Camera2D::Main()->ScreenToWorld(Input::GetMousePosition());

	if (Input::GetKeyDown(KeyCode::F1))
		followPlayer = !followPlayer;

	if (followPlayer) {
		float lerpSpeed = Time::GetDeltaTimeUnscaled() * 10;
		Vec2 targetPos = playerTr.Position;
		Vec2 curPos = mainCam->GetPosition();
		Vec2 lerpPos = curPos + (targetPos - curPos) * lerpSpeed;

		mainCam->SetPosition(lerpPos);
		mainCam->SetOrthographicSize(7.0f);
	}
	else {
		static Vec2 mouseStartPos = mousePos;
		static bool mouseDown = false;

		if (Input::GetKey(KeyCode::LeftControl))
			mainCam->AddOrthographicSize(-Input::ScrollValue() * Time::GetDeltaTimeUnscaled() * 100);

		if (Input::GetMouseDown(MouseButton::Right)) {
			mouseStartPos = mousePos;
			mouseDown = true;
		}
		else if (Input::GetMouseUp(MouseButton::Right)) {
			mouseDown = false;
		}

		if (mouseDown) {
			mainCam->AddPosition(mouseStartPos - mousePos);
		}
	}
}

void GameSystem::MoveEntities() {
	Scene& scene = GetScene();
	Transform2D& playerTr = m_PlayerEntity.GetComponent<Transform2D>();

	// Info: Move laser enties forward
	for (auto [ent, tr] : scene.GetRegistry().view<Transform2D, LaserTag>().each()) {
		tr.Position += tr.GetForwardDirection() * Time::GetDeltaTime() * 10.f;
	}

	// Info: Move Asteriod entities forward and chase player
	for (auto [ent, tr, ast] : scene.GetRegistry().view<Transform2D, AsteriodData>().each()) {
		Vec2 lookPlayerDir = playerTr.Position - tr.Position;
		float lookAtZ = atan2(lookPlayerDir.x, lookPlayerDir.y);
		float delta = std::remainder(lookAtZ - tr.Rotation, 2.0f * 3.14159265358979323846f);
		tr.Rotation += delta * Time::GetDeltaTime();
		tr.Position += tr.GetForwardDirection() * ast.Speed * Time::GetDeltaTime();

		if (AABB::Intersects(AABB::FromTransform(tr), AABB::FromTransform(playerTr))) {

		}
	}
}

void GameSystem::DrawGizmos() {
	Scene& scene = GetScene();

	Gizmo::SetColor(Color::Green());

	for (auto [ent, tr, box] : scene.GetRegistry().view<Transform2D, BoxCollider2D>().each()) {
		Gizmo::DrawSquare(box.GetBodyPosition(), box.GetScale(), box.GetRotationDegrees());
	}

	Gizmo::SetColor(Color::Gray());

	for (auto [ent, tr] : scene.GetRegistry().view<Transform2D>().each()) {
		AABB aabb = AABB::FromTransform(tr);
		Gizmo::DrawSquare(tr.Position, aabb.Scale(), 0);
	}
	for (auto [ent, pts2D] : scene.GetRegistry().view<ParticleSystem2D>().each()) {
		for (const auto& particle : pts2D.GetParticles())
		{
			AABB aabb = AABB::FromTransform(particle.Transform);
			Gizmo::DrawSquare(particle.Transform.Position, aabb.Scale(), 0);
		}
	}
}

void GameSystem::OnApplicationPaused() {
	if (Input::GetKeyDown(KeyCode::P)) {
		Application::Pause(!Application::IsPaused());
	}
}