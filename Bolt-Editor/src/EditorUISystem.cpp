#include "EditorUISystem.hpp"
#include <imgui.h>

namespace Bolt {
		void EditorUISystem::OnGui(Scene& scene) {
			ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_MenuBar);
			ImGui::End();
			//DrawDockspace();
			//DrawMenuBar(scene);
			//DrawHierarchy(scene);
			//DrawInspector(scene);
			//DrawStats();
		}

		void EditorUISystem::DrawDockspace() {
			ImGuiWindowFlags flags = /*ImGuiWindowFlags_NoDocking | */ ImGuiWindowFlags_NoTitleBar |
				ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
				ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
				ImGuiWindowFlags_MenuBar;

			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->Pos);
			ImGui::SetNextWindowSize(viewport->Size);
			//ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			ImGui::Begin("Bolt Editor Dockspace", nullptr, flags);
			ImGui::PopStyleVar(2);

			ImGuiID dockSpaceId = ImGui::GetID("BoltEditorMainDockspace");
			//ImGui::DockSpace(dockSpaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
			ImGui::End();
		}

		void EditorUISystem::DrawMenuBar(Scene& scene) {
			if (!ImGui::BeginMainMenuBar()) {
				return;
			}

			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("New Entity")) {
					CreateEntity(scene);
				}
				if (ImGui::MenuItem("Reload Scene")) {
					SceneManager::Get().ReloadScene(scene.GetName());
					m_SelectedEntity = entt::null;
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Quit")) {
					Application::Quit();
				}
				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}

		void EditorUISystem::DrawHierarchy(Scene& scene) {
			if (!ImGui::Begin("Hierarchy")) {
				ImGui::End();
				return;
			}

			if (ImGui::Button("Add Entity")) {
				CreateEntity(scene);
			}
			ImGui::Separator();

			if (m_SelectedEntity != entt::null && !scene.IsValid(m_SelectedEntity)) {
				m_SelectedEntity = entt::null;
			}

			auto view = scene.GetRegistry().view<entt::entity>();
			for (const EntityHandle handle : view) {
				Entity entity = scene.GetEntity(handle);
				const bool selected = m_SelectedEntity == handle;
				if (ImGui::Selectable(entity.GetName().c_str(), selected)) {
					m_SelectedEntity = handle;
				}
			}

			ImGui::End();
		}

		void  EditorUISystem::DrawInspector(Scene& scene) {
			if (!ImGui::Begin("Inspector")) {
				ImGui::End();
				return;
			}

			if (m_SelectedEntity == entt::null || !scene.IsValid(m_SelectedEntity)) {
				ImGui::TextDisabled("Select an entity to edit its components.");
				ImGui::End();
				return;
			}

			Entity entity = scene.GetEntity(m_SelectedEntity);
			/*if (entity.HasComponent<NameComponent>()) {
				auto& name = entity.GetComponent<NameComponent>().Name;
				char buffer[128]{};
				std::snprintf(buffer, sizeof(buffer), "%s", name.c_str());
				if (ImGui::InputText("Name", buffer, sizeof(buffer))) {
					name = buffer;
				}
			}*/

			if (entity.HasComponent<Transform2DComponent>()) {
				auto& transform = entity.GetComponent<Transform2DComponent>();
				ImGui::SeparatorText("Transform2D");
				ImGui::DragFloat2("Position", &transform.Position.x, 0.05f);
				ImGui::DragFloat2("Scale", &transform.Scale.x, 0.05f, 0.01f, 100.0f);
				ImGui::DragFloat("Rotation", &transform.Rotation, 0.01f, -6.28318f, 6.28318f);
			}

			if (entity.HasComponent<Camera2DComponent>()) {
				auto& camera = entity.GetComponent<Camera2DComponent>();
				ImGui::SeparatorText("Camera2D");
				float orthoSize = camera.GetOrthographicSize();
				if (ImGui::DragFloat("Orthographic Size", &orthoSize, 0.05f, 0.1f, 100.0f)) {
					camera.SetOrthographicSize(orthoSize);
				}
				float zoom = camera.GetZoom();
				if (ImGui::DragFloat("Zoom", &zoom, 0.05f, 0.1f, 8.0f)) {
					camera.SetZoom(zoom);
				}
			}

			ImGui::End();
		}

		void EditorUISystem::DrawStats() {
			if (!ImGui::Begin("Stats")) {
				ImGui::End();
				return;
			}

			auto* app = Application::GetInstance();
			auto& time = app->GetTime();
			const float fps = 1.0f / Max(0.0001f, time.GetDeltaTimeUnscaled());
			ImGui::Text("FPS: %.1f", fps);
			ImGui::Text("Frame: %d", time.GetFrameCount());
			ImGui::Text("Rendered Instances: %d", app->GetRenderer2D()->GetRenderedInstancesCount());
			ImGui::End();
		}

		void EditorUISystem::CreateEntity(Scene& scene) {
			const std::string name = "Entity " + std::to_string(++m_EntityCounter);
			Entity entity = scene.CreateEntity(name);
			m_SelectedEntity = entity.GetHandle();
		}
}