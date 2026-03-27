#include <pch.hpp>
#include "EditorUISystem.hpp"
#include "Editor/EditorSceneSerializer.hpp"
#include <imgui.h>
#include <algorithm>
#include <algorithm>
#include "Debugging/Logger.hpp"
#include "Utils/Path.hpp"


namespace Bolt {
	void EditorUISystem::OnGui(Scene& scene) {
		DrawDockspace();
		DrawMenuBar(scene);
		DrawHierarchy(scene);
		DrawInspector(scene);
		DrawStats();
		DrawProjectLoader(scene);
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
		if (m_ProjectRootPath.empty()) {
			m_ProjectRootPath = std::filesystem::path(Path::Combine(Path::Current(), "Bolt", "Projects"));
		}

		if (!ImGui::BeginMainMenuBar()) {
			return;
		}

		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("New Entity")) {
				CreateEntity(scene);
			}
			if (ImGui::MenuItem("Load Project...")) {
				RefreshProjectEntries();
				m_OpenProjectDialog = true;
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

		if (!m_LastSaveMessage.empty()) {
			const ImVec4 messageColor = m_LastSaveSucceeded
				? ImVec4(0.3f, 0.9f, 0.4f, 1.0f)
				: ImVec4(0.95f, 0.3f, 0.3f, 1.0f);
			ImGui::TextColored(messageColor, "%s", m_LastSaveMessage.c_str());
			ImGui::Separator();
		}

		if (m_SelectedEntity == entt::null || !scene.IsValid(m_SelectedEntity)) {
			ImGui::TextDisabled("Select an entity to edit its components.");
			ImGui::End();
			return;
		}

		Entity entity = scene.GetEntity(m_SelectedEntity);
		DrawComponentManagement(entity);
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

	void EditorUISystem::DrawProjectLoader(Scene& scene) {
		if (!m_OpenProjectDialog) {
			return;
		}

		ImGui::OpenPopup("Load Existing Project");
		if (ImGui::BeginPopupModal("Load Existing Project", &m_OpenProjectDialog, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::TextWrapped("Select a project folder from %s", m_ProjectRootPath.string().c_str());
			ImGui::Separator();

			if (m_ProjectEntries.empty()) {
				ImGui::TextDisabled("No projects were found in this directory.");
			}

			for (const auto& projectEntry : m_ProjectEntries) {
				const bool selected = (m_SelectedProjectPath == projectEntry);
				if (ImGui::Selectable(projectEntry.filename().string().c_str(), selected)) {
					m_SelectedProjectPath = projectEntry;
				}
			}

			ImGui::Separator();
			const bool canLoad = !m_SelectedProjectPath.empty();
			if (!canLoad) {
				ImGui::BeginDisabled();
			}

			if (ImGui::Button("Load Project")) {
				LoadProject(m_SelectedProjectPath, scene);
				m_OpenProjectDialog = false;
			}

			if (!canLoad) {
				ImGui::EndDisabled();
			}

			ImGui::SameLine();
			if (ImGui::Button("Cancel")) {
				m_OpenProjectDialog = false;
			}

			ImGui::EndPopup();
		}
	}

	void EditorUISystem::LoadProject(const std::filesystem::path& projectPath, Scene& scene) {
		if (projectPath.empty()) {
			return;
		}

		m_LoadedProjectPath = projectPath;
		Logger::Message("Editor", "Loaded project from '" + projectPath.string() + "'");
		SceneManager::Get().ReloadScene(scene.GetName());
		m_SelectedEntity = entt::null;
	}

	void EditorUISystem::RefreshProjectEntries() {
		m_ProjectEntries.clear();
		m_SelectedProjectPath.clear();

		std::error_code errorCode;
		if (!std::filesystem::exists(m_ProjectRootPath, errorCode) ||
			!std::filesystem::is_directory(m_ProjectRootPath, errorCode)) {
			return;
		}

		for (const auto& entry : std::filesystem::directory_iterator(m_ProjectRootPath, errorCode)) {
			if (errorCode) {
				break;
			}

			if (entry.is_directory(errorCode) && !errorCode) {
				m_ProjectEntries.push_back(entry.path());
			}
		}

		std::sort(m_ProjectEntries.begin(), m_ProjectEntries.end());
	}

	void EditorUISystem::CreateEntity(Scene& scene) {
		const std::string name = "Entity " + std::to_string(++m_EntityCounter);
		Entity entity = scene.CreateEntity(name);
		m_SelectedEntity = entity.GetHandle();
	}

	void EditorUISystem::DrawComponentManagement(Entity entity) {
		auto& componentRegistry = SceneManager::Get().GetComponentRegistry();
		std::vector<std::reference_wrapper<const ComponentInfo>> existingComponents;
		std::vector<std::reference_wrapper<const ComponentInfo>> missingComponents;

		componentRegistry.ForEachComponentInfo([&](std::type_index, const ComponentInfo& info) {
			if (!info.has || !info.add || !info.remove) {
				return;
			}

			if (info.has(entity)) {
				existingComponents.emplace_back(info);
			}
			else {
				missingComponents.emplace_back(info);
			}
			});

		auto byName = [](const auto& a, const auto& b) {
			return a.get().displayName < b.get().displayName;
			};
		std::sort(existingComponents.begin(), existingComponents.end(), byName);
		std::sort(missingComponents.begin(), missingComponents.end(), byName);

		ImGui::SeparatorText("Component Management");
		if (!missingComponents.empty()) {
			if (m_SelectedAddComponentIndex >= static_cast<int>(missingComponents.size())) {
				m_SelectedAddComponentIndex = 0;
			}

			if (ImGui::BeginCombo("Add Component", missingComponents[m_SelectedAddComponentIndex].get().displayName.c_str())) {
				for (int i = 0; i < static_cast<int>(missingComponents.size()); ++i) {
					const bool isSelected = (m_SelectedAddComponentIndex == i);
					if (ImGui::Selectable(missingComponents[i].get().displayName.c_str(), isSelected)) {
						m_SelectedAddComponentIndex = i;
					}
					if (isSelected) {
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}

			if (ImGui::Button("Add Selected Component")) {
				missingComponents[m_SelectedAddComponentIndex].get().add(entity);
			}
		}
		else {
			ImGui::TextDisabled("All registered components are already on this entity.");
		}

		const ComponentInfo* componentToRemove = nullptr;
		if (!existingComponents.empty()) {
			ImGui::SeparatorText("Attached Components");
			for (const ComponentInfo& info : existingComponents) {
				ImGui::PushID(info.displayName.c_str());
				ImGui::TextUnformatted(info.displayName.c_str());
				ImGui::SameLine();
				if (ImGui::Button("Remove")) {
					componentToRemove = &info;
				}
				ImGui::PopID();
			}
		}

		if (componentToRemove != nullptr) {
			componentToRemove->remove(entity);
		}
	}

	void EditorUISystem::SaveCurrentScene(const Scene& scene) {
		EditorSceneSaveResult result = EditorSceneSerializer::Save(scene);
		m_LastSaveSucceeded = result.Succeeded;
		m_LastSaveMessage = result.Message;
	}
}