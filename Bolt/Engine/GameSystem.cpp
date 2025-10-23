#include "../pch.hpp"
#include "GameSystem.hpp"
#include "Core/Window.hpp"

namespace Bolt {
	void GameSystem::Awake(Scene& scene) {

	}

	void Create3DEntity(Scene& scene, const Transform& transform, std::shared_ptr<Mesh> mesh, Color color) {
		Entity cubeEntity = scene.CreateEntity();
		Transform& cubeTransform = cubeEntity.AddComponent<Transform>();
		cubeTransform = transform;

		MeshRenderer& meshRenderer = cubeEntity.AddComponent<MeshRenderer>();
		meshRenderer.Mesh = mesh;
		meshRenderer.AlbedoColor = color;
	}

	void GameSystem::Start(Scene& scene) {
		m_CameraEntity = scene.CreateEntity();
		Transform& cameraTransform = m_CameraEntity.AddComponent<Transform>();
		cameraTransform.Position = Vec3(0.0f, 0.0f, -10.0f);

		Camera& camera = m_CameraEntity.AddComponent<Camera>();
		camera.SetPerspective(60.0f, 0.01f, 100.0f);
		camera.SetPosition(cameraTransform.Position);
		camera.SetAsMain();

		const Vec3 lookTarget = Vec3(0.0f);
		camera.LookAt(lookTarget);

		const Vec3 forward = glm::normalize(lookTarget - cameraTransform.Position);
		const float radToDeg = 180.0f / glm::pi<float>();
		m_CameraYaw = std::atan2(forward.z, forward.x) * radToDeg;
		m_CameraPitch = std::asin(std::clamp(forward.y, -1.0f, 1.0f)) * radToDeg;
		cameraTransform.Rotation = camera.GetRotation();

		int length = 10;

		for (int x = 0; x < length; x++) {
			for (int y = 0; y < length; y++) {
				for (int z = 0; z < length; z++) {
					Create3DEntity(scene, Transform(Vec3(x, y, z + 10), Vec3(1, 1, 1), Vec3(0)), Mesh::Sphere(), Color::White());
				}
			}
		}

		Entity entity = scene.CreateEntity();
		entity.AddComponent<Transform2D>();
		SpriteRenderer& sp = entity.AddComponent<SpriteRenderer>();
		sp.TextureHandle = TextureManager::GetDefaultTexture(DefaultTexture::Square());

		Entity cameraEnt = scene.CreateEntity();
		cameraEnt.AddComponent<Transform2D>();
		Camera2D& camera2D = cameraEnt.AddComponent<Camera2D>();
	}

	void GameSystem::Update(Scene& scene) {
		if (m_CameraEntity == Entity::null) {
			return;
		}

		if (!m_CameraEntity.HasComponent<Transform>() || !m_CameraEntity.HasComponent<Camera>()) {
			return;
		}

		Transform& cameraTransform = m_CameraEntity.GetComponent<Transform>();
		Camera& camera = m_CameraEntity.GetComponent<Camera>();
		camera.SetViewportSize(Window::GetMainViewport().Width, Window::GetMainViewport().Height);

		const float deltaTime = Time::GetDeltaTime();
		const float baseSpeed = 5.0f;
		const float fastMultiplier = 4.0f;

		Vec3 movement(0.0f);
		const Vec3 forward = camera.GetForward();
		const Vec3 right = camera.GetRight();
		const Vec3 up = camera.GetUp();

		if (Input::GetKey(KeyCode::W)) movement += forward;
		if (Input::GetKey(KeyCode::S)) movement -= forward;
		if (Input::GetKey(KeyCode::D)) movement += right;
		if (Input::GetKey(KeyCode::A)) movement -= right;
		if (Input::GetKey(KeyCode::E) || Input::GetKey(KeyCode::Space)) movement += up;
		if (Input::GetKey(KeyCode::Q) || Input::GetKey(KeyCode::LeftControl)) movement -= up;

		if (glm::length(movement) > 0.0f) {
			movement = glm::normalize(movement);
			float speed = baseSpeed;
			if (Input::GetKey(KeyCode::LeftShift) || Input::GetKey(KeyCode::RightShift)) {
				speed *= fastMultiplier;
			}

			cameraTransform.Position += movement * speed * deltaTime;
		}

		camera.SetPosition(cameraTransform.Position);

		if (Input::GetMouse(MouseKeyCode::Right)) {
			Vec2 mouseDelta = Input::MouseDelta();
			if (!m_ResetMouseRotation) {
				constexpr float sensitivity = 0.1f;
				m_CameraYaw += mouseDelta.x * sensitivity;
				m_CameraPitch -= mouseDelta.y * sensitivity;
				m_CameraPitch = std::clamp(m_CameraPitch, -89.0f, 89.0f);
			}
			m_ResetMouseRotation = false;
		}
		else {
			m_ResetMouseRotation = true;
		}

		const float degToRad = glm::pi<float>() / 180.0f;
		const float yawRadians = m_CameraYaw * degToRad;
		const float pitchRadians = m_CameraPitch * degToRad;

		Vec3 forwardDirection;
		forwardDirection.x = std::cos(yawRadians) * std::cos(pitchRadians);
		forwardDirection.y = std::sin(pitchRadians);
		forwardDirection.z = std::sin(yawRadians) * std::cos(pitchRadians);
		forwardDirection = glm::normalize(forwardDirection);

		camera.LookAt(cameraTransform.Position + forwardDirection, Vec3(0.0f, 1.0f, 0.0f));
		cameraTransform.Rotation = camera.GetRotation();
	}
}