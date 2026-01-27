#include "GameSystem.hpp"
#include "Components/Tags.hpp"
#include "Graphics/OpenGL.hpp"
#include "Core/Application.hpp"

#include <imgui.h>



void GameSystem::Awake() {
	m_AsteriodTexture = TextureManager::LoadTexture("../Assets/Textures/Meteor.png");
	m_LightTexture = TextureManager::LoadTexture("../Assets/Textures/Light.png");
	m_PlayerTexture = TextureManager::LoadTexture("../Assets/Textures/Player.png");
	m_LaserTexture = TextureManager::LoadTexture("../Assets/Textures/Laser.png");
}



void GameSystem::Start() {
	Scene& scene = GetScene();

	//m_RectEntity = scene.CreateEntity();
	//m_RectEntity.RemoveComponent<Transform2D>();
	//auto& guiImage = m_RectEntity.AddComponent<Image>();
	//m_RectEntity.AddComponent<RectTransform>();


	scene.CreateCamera();
	OpenGL::SetBackgroundColor(Color(0.1f, 0.1f, 0.1f));

	m_PlayerEmissionPts = scene.CreateEntity();
	ParticleSystem2D& pts2D = m_PlayerEmissionPts.AddComponent<ParticleSystem2D>();
	pts2D.SetTexture(TextureManager::GetDefaultTexture(DefaultTexture::Circle));

	auto ptColor = Color(1.0f, 0.45f, 0.f, 0.1f);
	pts2D.RenderingSettings.Color = ptColor;
	pts2D.Shape = SquareParams(Vec2(0.2f, 0.2f));
	pts2D.EmissionSettings.EmitOverTime = 0;
	pts2D.ParticleSettings.LifeTime = 10;
	pts2D.ParticleSettings.Scale = 0.2f;
	pts2D.ParticleSettings.Gravity = Vec2(0, 1);
	pts2D.RenderingSettings.MaxParticles = 100'000;
	pts2D.RenderingSettings.SortingLayer = 0;
	pts2D.Play();

	m_PlayerEntity = scene.CreateEntity();
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

	if (Input::GetKeyDown(KeyCode::R)) {
		SceneManager::ReloadScene(scene.GetName());
		return;
	}
	if (Input::GetKeyDown(KeyCode::O)) {
		for (int x = 0; x < 50; x++) 
			for (int y = 0; y < 50; y++) {
				auto& tr = scene.CreateRenderableEntity().GetComponent<Transform2D>();
				tr.Position = Vec2(x, y)+ mousePos;
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
		Entity ent = scene.CreateEntity();
		SpriteRenderer& sp = ent.AddComponent<SpriteRenderer>();
		sp.TextureHandle = m_AsteriodTexture;

		AsteriodData& asteriod = ent.AddComponent<AsteriodData>();
		asteriod.Speed = Random::NextFloat(3.f, 5.f);
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

void GameSystem::OnGui()
{
	ImGui::Begin("Debug Settings");

	static bool enabledVsync = true;
	ImGui::Checkbox("Vsync", &enabledVsync);
	Application::GetWindow().SetVsync(enabledVsync);

	if (!enabledVsync) {
		static float targetFrameRate = 144;
		ImGui::SliderFloat("Target FPS", &targetFrameRate, 0.f, 244.f);
		Application::SetTargetFramerate(targetFrameRate);
	}

	static float timeScale = 1.0;
	ImGui::SliderFloat("Timescale", &timeScale, 0.f, 10.f);
	Time::SetTimeScale(timeScale);

	static float fixedFPS = 50.f;
	ImGui::SliderFloat("Fixed FPS", &fixedFPS, 10.f, 244.f);

	static bool enabledGizmo = true;
	ImGui::Checkbox("Gizmo", &enabledGizmo);
	Gizmo::SetEnabled(enabledGizmo);

	if (enabledGizmo) {
		static bool aabb = true;
		ImGui::Checkbox("AABB", &aabb);

		static bool collider = true;
		ImGui::Checkbox("Collider", &collider);
	}

	ImGui::End();

	ImGui::Begin("Debug Info");

	const std::string fps = "Current FPS: " + std::to_string(1.f / Time::GetDeltaTimeUnscaled());
	const std::string timescale = "Current TimeScale: " + std::to_string(Time::GetTimeScale());

	auto renderer = Application::GetInstance().m_Renderer2D;

	const std::string renderedInstances = "Rendered Instances: " + std::to_string(renderer->GetRenderedInstancesCount());
	const std::string renderLoopDuration = "Render Loop Duration: " + std::to_string(renderer->GetRRenderLoopDuration());

	ImGui::Text(fps.c_str());
	ImGui::Text(timescale.c_str());
	ImGui::Text(renderedInstances.c_str());
	ImGui::Text(renderLoopDuration.c_str());
	ImGui::End();
}

void GameSystem::UpdatePlayerPts() {
	auto& playerTr = m_PlayerEntity.GetComponent<Transform2D>();
	auto& pts2D = m_PlayerEmissionPts.GetComponent<ParticleSystem2D>();
	auto& tr = m_PlayerEmissionPts.GetComponent<Transform2D>();
	pts2D.ParticleSettings.MoveDirection = -playerTr.GetForwardDirection();
	pts2D.Emit(15);

	tr.Position = playerTr.Position - (playerTr.GetForwardDirection() * 0.4f);
}

void GameSystem::PlayerMovement() {
	auto mousePos = Camera2D::Main()->ScreenToWorld(Input::GetMousePosition());

	float speed = Input::GetKey(KeyCode::LeftShift) ? 15.f : 5.0f;
	auto& playerTr = m_PlayerEntity.GetComponent<Transform2D>();
	Vec2 input = speed * Time::GetDeltaTime() * playerTr.GetForwardDirection();
	playerTr.Position += input;

	Vec2 lookDir = mousePos - playerTr.Position;
	float lookAtZ = atan2(lookDir.x, lookDir.y);
	float t = 0.1f;
	float delta = std::remainder(lookAtZ - playerTr.Rotation, 2.0f * 3.14159265358979323846f);
	playerTr.Rotation += delta * t;
}

void GameSystem::CameraMovement() {
	Transform2D& playerTr = m_PlayerEntity.GetComponent<Transform2D>();
	Camera2D* mainCam = Camera2D::Main();

	float lerpSpeed = 0.1f;
	Vec2 targetPos = playerTr.Position;
	Vec2 curPos = mainCam->GetPosition();
	Vec2 lerpPos = curPos + (targetPos - curPos) * lerpSpeed;

	mainCam->SetPosition(lerpPos);

	if (Input::GetKey(KeyCode::LeftControl))
		mainCam->AddOrthographicSize(-Input::ScrollValue() * Time::GetDeltaTime() * 100);
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
		float t = 0.1f;
		float delta = std::remainder(lookAtZ - tr.Rotation, 2.0f * 3.14159265358979323846f);
		tr.Rotation += delta * t;
		tr.Position += tr.GetForwardDirection() * Time::GetDeltaTime() * ast.Speed;

		if (AABB::Intersects(AABB::FromTransform(tr), AABB::FromTransform(playerTr))) {

		}
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

	Gizmo::SetColor(Color::Red());

	for (auto [ent, tr] : scene.GetRegistry().view<Transform2D>().each()) {
		AABB aabb = AABB::FromTransform(tr);
		Gizmo::DrawSquare(tr.Position, aabb.Scale(), 0);
	}
}

void GameSystem::OnApplicationPaused() {
	if (Input::GetKeyDown(KeyCode::P)) {
		Application::Pause(!Application::IsPaused());
	}
}