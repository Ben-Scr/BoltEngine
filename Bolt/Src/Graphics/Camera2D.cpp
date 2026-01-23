#include "../pch.hpp"
#include "../Graphics/Camera2D.hpp"
#include "../Collections/Viewport.hpp"
#include "../Core/Window.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Bolt {
	Camera2D* Camera2D::s_Main = nullptr;

	Camera2D* Camera2D::Main() {
		return s_Main;
	}

	void Camera2D::UpdateViewport() {
		auto vp = Window::GetMainViewport();
		m_ViewportWidth = vp.Width;
		m_ViewportHeight = vp.Height;

		UpdateProj();
		UpdateViewportAABB();
	}

	glm::mat4 Camera2D::GetViewProjectionMatrix() const {
		return m_ProjMat * m_ViewMat;
	}

	void Camera2D::UpdateProj() {
		const float aspect = float(m_ViewportWidth) / float(m_ViewportHeight);
		const float halfH = m_OrthographicSize * m_Zoom;
		const float halfW = halfH * aspect;

		const float zNear = 0.0f;
		const float zFar = 100.0f;


		m_ProjMat = glm::ortho(-halfW, +halfW, -halfH, +halfH, zNear, zFar);
	}

	void Camera2D::UpdateView() {
		const float rotZ = m_Transform->Rotation;
		glm::mat4 camModel(1.0f);
		camModel = glm::translate(camModel, glm::vec3(m_Transform->Position.x, m_Transform->Position.y, 0.0f));
		camModel = glm::rotate(camModel, rotZ, glm::vec3(0.0f, 0.0f, 1.0f));

		m_ViewMat = glm::inverse(camModel);
		UpdateViewportAABB();
	}

	void Camera2D::UpdateViewportAABB() {
		Vec2 worldViewport = WorldViewPort();
		m_WorldViewportAABB = AABB::Create(m_Transform->Position, worldViewport / 2.f);
	}

	Vec2 Camera2D::WorldViewPort() const {
		float aspectRatio = static_cast<float>(m_ViewportWidth) / static_cast<float>(m_ViewportHeight);
		float worldHeight = 2.0f * (m_OrthographicSize / m_Zoom);
		float worldWidth = worldHeight * aspectRatio;
		return { worldWidth, worldHeight };
	}

	Vec2 Camera2D::ScreenToWorld(Vec2 pos) const
	{
		const float xNdc = (2.0f * pos.x / float(m_ViewportWidth)) - 1.0f;
		const float yNdc = 1.0f - (2.0f * pos.y / float(m_ViewportHeight));

		const float zNear = 0.0f, zFar = 100.0f;
		const float zNdc = -(zFar + zNear) / (zFar - zNear);

		const glm::vec4 clip(xNdc, yNdc, zNdc, 1.0f);

		const glm::mat4 vp = m_ProjMat * m_ViewMat;
		const glm::mat4 invVp = glm::inverse(vp);

		glm::vec4 world = invVp * clip;
		if (world.w != 0.0f) world /= world.w;

		return { world.x, world.y };
	}

	void Camera2D::Initialize(Transform2D& transform) {
		s_Main = this;
		m_Transform = &transform;
		m_ViewMat = glm::mat4(1.0f);
		m_ProjMat = glm::mat4(1.0f);
		UpdateProj();
		UpdateView();
	}

	void Camera2D::Destroy() {
		if (s_Main == this)
			s_Main = nullptr;

		m_Transform = nullptr;
	}
}