#include <pch.hpp>
#include "Systems/ImGuiEditorLayer.hpp"

#include <imgui.h>
#include <glad/glad.h>

#include "Components/Components.hpp"
#include "Core/Application.hpp"
#include "Core/Window.hpp"
#include "Graphics/GizmoRenderer.hpp"
#include "Graphics/Gizmo.hpp"
#include "Graphics/Renderer2D.hpp"
#include "Graphics/TextureManager.hpp"
#include "Gui/EditorIcons.hpp"
#include "Scene/Scene.hpp"
#include "Scene/SceneManager.hpp"

namespace Bolt {

	void ImGuiEditorLayer::RenderSceneIntoFBO(ViewportFBO& fbo, Scene& scene,
		const glm::mat4& vp, const AABB& viewportAABB,
		bool withGizmos, const Color& clearColor)
	{
		auto* app = Application::GetInstance();
		if (!app) return;
		auto* renderer = app->GetRenderer2D();
		if (!renderer) return;

		int w = fbo.ViewportSize.GetWidth();
		int h = fbo.ViewportSize.GetHeight();

		glBindFramebuffer(GL_FRAMEBUFFER, fbo.FramebufferId);
		glViewport(0, 0, w, h);
		glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		SceneManager::Get().ForeachLoadedScene([&](const Scene& s) {
			renderer->RenderSceneWithVP(s, vp, viewportAABB);
			});

		if (withGizmos && Gizmo::IsEnabled()) {
			GizmoRenderer2D::RenderWithVP(vp);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		auto* window = Application::GetWindow();
		if (window) {
			glViewport(0, 0, window->GetWidth(), window->GetHeight());
		}
	}

	void ImGuiEditorLayer::RenderEditorView(Scene& scene) {
		m_IsEditorViewActive = ImGui::Begin("Editor View");

		if (!m_IsEditorViewActive) {
			m_IsEditorViewHovered = false;
			m_IsEditorViewFocused = false;
			ImGui::End();
			return;
		}

		const ImVec2 viewportSize = ImGui::GetContentRegionAvail();
		const int fbW = static_cast<int>(viewportSize.x);
		const int fbH = static_cast<int>(viewportSize.y);

		if (fbW > 0 && fbH > 0) {

			EnsureFBO(m_EditorViewFBO, fbW, fbH);
			m_EditorCamera.SetViewportSize(fbW, fbH);

			if (m_EditorViewFBO.FramebufferId != 0) {

				auto* app = Application::GetInstance();
				if (app) {
					auto& input = app->GetInput();
					float dt = app->GetTime().GetDeltaTimeUnscaled();

					Vec2 mouseDelta = { 0.0f, 0.0f };
					if (m_IsEditorViewHovered && input.GetMouse(MouseButton::Middle)) {
						mouseDelta = input.GetMouseDelta();
					}
					float scroll = m_IsEditorViewHovered ? input.ScrollValue() : 0.0f;

					m_EditorCamera.Update(dt, m_IsEditorViewHovered, mouseDelta, scroll);
				}

				glm::mat4 vp = m_EditorCamera.GetViewProjectionMatrix();
				AABB viewAABB = m_EditorCamera.GetViewportAABB();
				Gizmo::SetViewportAABBOverride(viewAABB);

				if (m_SelectedEntity != entt::null && scene.IsValid(m_SelectedEntity)
					&& scene.HasComponent<Transform2DComponent>(m_SelectedEntity)) {
					auto& transform = scene.GetComponent<Transform2DComponent>(m_SelectedEntity);
					Vec2 outlineScale = transform.Scale;

					if (scene.HasComponent<SpriteRendererComponent>(m_SelectedEntity)) {
						auto& sr = scene.GetComponent<SpriteRendererComponent>(m_SelectedEntity);
						Texture2D* tex = TextureManager::GetTexture(sr.TextureHandle);
						if (tex && tex->IsValid() && tex->GetHeight() > 0) {
							// Bounds are already encoded in the entity transform.
						}
					}

					Color prevColor = Gizmo::GetColor();
					float prevWidth = Gizmo::GetLineWidth();
					Gizmo::SetColor(Color(1.0f, 0.6f, 0.0f, 1.0f));
					Gizmo::SetLineWidth(2.0f);
					Gizmo::DrawSquare(transform.Position, outlineScale, transform.Rotation * (180.0f / 3.14159265f));
					Gizmo::SetColor(prevColor);
					Gizmo::SetLineWidth(prevWidth);
				}

				static const Color k_EditorClearColor(0.18f, 0.18f, 0.20f, 1.0f);
				RenderSceneIntoFBO(m_EditorViewFBO, scene, vp, viewAABB, true, k_EditorClearColor);
				Gizmo::ClearViewportAABBOverride();

				ImGui::Image(
					static_cast<ImTextureID>(static_cast<intptr_t>(m_EditorViewFBO.ColorTextureId)),
					viewportSize,
					ImVec2(0.0f, 1.0f),
					ImVec2(1.0f, 0.0f));

				ImVec2 imageTopLeft = ImGui::GetItemRectMin();

				const float iconSize = 24.0f;
				const float halfIcon = iconSize * 0.5f;
				auto camView = scene.GetRegistry().view<Camera2DComponent, Transform2DComponent>();
				for (auto [ent, cam, transform] : camView.each()) {
					glm::vec4 worldPos(transform.Position.x, transform.Position.y, 0.0f, 1.0f);
					glm::vec4 clipPos = vp * worldPos;
					if (clipPos.w == 0.0f) continue;

					float ndcX = clipPos.x / clipPos.w;
					float ndcY = clipPos.y / clipPos.w;
					float screenX = (ndcX * 0.5f + 0.5f) * viewportSize.x;
					float screenY = (1.0f - (ndcY * 0.5f + 0.5f)) * viewportSize.y;

					if (screenX < -halfIcon || screenX > viewportSize.x + halfIcon ||
						screenY < -halfIcon || screenY > viewportSize.y + halfIcon) {
						continue;
					}

					unsigned int camIcon = EditorIcons::Get("camera", 24);
					if (!camIcon) {
						continue;
					}

					ImVec2 iconPos(imageTopLeft.x + screenX - halfIcon, imageTopLeft.y + screenY - halfIcon);
					ImGui::GetWindowDrawList()->AddImage(
						static_cast<ImTextureID>(static_cast<intptr_t>(camIcon)),
						iconPos,
						ImVec2(iconPos.x + iconSize, iconPos.y + iconSize),
						ImVec2(0, 1), ImVec2(1, 0));
				}
			}
		}
		else {
			ImGui::TextDisabled("Editor View has no drawable area");
		}

		m_IsEditorViewHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows);
		m_IsEditorViewFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
		ImGui::End();
	}

	void ImGuiEditorLayer::RenderGameView(Scene& scene) {
		(void)scene;

		m_IsGameViewActive = ImGui::Begin("Game View");

		if (!m_IsGameViewActive) {
			ImGui::End();
			m_IsGameViewFocused = false;
			m_IsGameViewHovered = false;
			return;
		}

		const ImVec2 viewportSize = ImGui::GetContentRegionAvail();
		const int fbW = static_cast<int>(viewportSize.x);
		const int fbH = static_cast<int>(viewportSize.y);

		if (fbW > 0 && fbH > 0) {
			EnsureFBO(m_GameViewFBO, fbW, fbH);

			Camera2DComponent* gameCam = Camera2DComponent::Main();
			if (m_GameViewFBO.FramebufferId != 0 && gameCam && gameCam->IsValid()) {
				Viewport* savedViewport = gameCam->GetViewport();
				int savedW = savedViewport->GetWidth();
				int savedH = savedViewport->GetHeight();
				savedViewport->SetSize(fbW, fbH);

				gameCam->UpdateViewport();
				glm::mat4 vp = gameCam->GetViewProjectionMatrix();
				AABB viewAABB = gameCam->GetViewportAABB();
				RenderSceneIntoFBO(m_GameViewFBO, scene, vp, viewAABB, true, gameCam->GetClearColor());

				savedViewport->SetSize(savedW, savedH);

				ImGui::Image(
					static_cast<ImTextureID>(static_cast<intptr_t>(m_GameViewFBO.ColorTextureId)),
					viewportSize,
					ImVec2(0.0f, 1.0f),
					ImVec2(1.0f, 0.0f));
			}
			else {
				ImGui::TextDisabled("No main camera in scene");
			}
		}
		else {
			ImGui::TextDisabled("Game View has no drawable area");
		}

		m_IsGameViewHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows);
		m_IsGameViewFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
		ImGui::End();
	}

} // namespace Bolt
