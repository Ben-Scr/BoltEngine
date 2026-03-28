#include <pch.hpp>

#include "EditorUISystem.hpp"

#include <imgui.h>

#include <Utils/Path.hpp>

#include "Editor/EditorProjectSerializer.hpp"

namespace Bolt {
	void EditorUISystem::OnGui(Scene& scene) {
		DrawDockspace();
		DrawMenuBar(scene);
		DrawToolbar(scene);
		DrawProjectLoader(scene);
		DrawHierarchy(scene);
		DrawGameView(scene);
		DrawInspector(scene);
		if (m_ShowStats) {
			DrawStats();
		}
	}

	void EditorUISystem::DrawDockspace() {
		ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
			ImGuiWindowFlags_MenuBar;

		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::Begin("Bolt Editor Dockspace", nullptr, flags);
		ImGui::PopStyleVar(2);
		ImGui::End();
	}

	void EditorUISystem::DrawMenuBar(Scene& scene) {
		if (!ImGui::BeginMainMenuBar()) {
			return;
		}

		if (ImGui::BeginMenu("File")) {
			if (m_ProjectFilePath.empty()) {
				m_ProjectFilePath = GetDefaultProjectSavePath(scene);
			}

			char projectNameBuffer[128]{};
			std::snprintf(projectNameBuffer, sizeof(projectNameBuffer), "%s", m_ProjectName.c_str());

			if (ImGui::MenuItem("New Project")) {
				m_ProjectName = "UntitledProject";
				m_ProjectFilePath = GetDefaultProjectSavePath(scene);
				m_LastSaveStatus = "Created new project settings.";
				m_LastSaveSucceeded = true;
			}

			ImGui::TextDisabled("Project Name");
			if (ImGui::InputText("##ProjectName", projectNameBuffer, sizeof(projectNameBuffer))) {
				m_ProjectName = StringHelper::Trim(projectNameBuffer);
				if (m_ProjectName.empty()) {
					m_ProjectName = "UntitledProject";
				}
			}

			ImGui::Separator();
			const ImGuiIO& io = ImGui::GetIO();
			const bool saveShortcut = io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S, false);

			if (ImGui::MenuItem("Save Project", "Ctrl+S") || saveShortcut) {
				SaveProjectFromGui(scene, m_ProjectFilePath);
			}
			if (ImGui::MenuItem("Load Project...")) {
				m_ProjectRootPath = std::filesystem::path(Path::Combine("Bolt-Editor", "Projects"));
				RefreshProjectEntries();
				m_OpenProjectDialog = true;
			}

			ImGui::Separator();
			if (ImGui::MenuItem("Quit")) {
				Application::Quit();
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Stats")) {
			ImGui::MenuItem("Show Stats Overlay", nullptr, &m_ShowStats);
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	void EditorUISystem::DrawToolbar(Scene& scene) {
		if (m_ProjectFilePath.empty()) {
			m_ProjectFilePath = GetDefaultProjectSavePath(scene);
		}

		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImVec2 toolbarPos(viewport->Pos.x + 8.0f, viewport->Pos.y + 26.0f);
		ImVec2 toolbarSize(viewport->Size.x - 16.0f, 46.0f);
		ImGui::SetNextWindowPos(toolbarPos, ImGuiCond_Always);
		ImGui::SetNextWindowSize(toolbarSize, ImGuiCond_Always);

		ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar;

		if (!ImGui::Begin("##EditorToolbar", nullptr, flags)) {
			ImGui::End();
			return;
		}

		if (ImGui::Button("Add Entity")) {
			CreateEntity(scene);
		}

		ImGui::SameLine();
		const bool isPlaying = !Application::IsPaused();
		if (!isPlaying) {
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.55f, 0.35f, 1.0f));
		}
		if (ImGui::Button("Play")) {
			Application::Pause(false);
		}
		if (!isPlaying) {
			ImGui::PopStyleColor();
		}

		ImGui::SameLine();
		if (isPlaying) {
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.25f, 0.25f, 1.0f));
		}
		if (ImGui::Button("Stop")) {
			Application::Pause(true);
		}
		if (isPlaying) {
			ImGui::PopStyleColor();
		}

		ImGui::SameLine();
		char pathBuffer[512]{};
		std::snprintf(pathBuffer, sizeof(pathBuffer), "%s", m_ProjectFilePath.c_str());
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.52f);
		if (ImGui::InputText("Project Path", pathBuffer, sizeof(pathBuffer))) {
			m_ProjectFilePath = StringHelper::Trim(pathBuffer);
		}

		ImGui::SameLine();
		if (ImGui::Button("Save Project")) {
			SaveProjectFromGui(scene, m_ProjectFilePath);
		}

		ImGui::SameLine();
		if (ImGui::Button("Load Project")) {
			m_ProjectRootPath = std::filesystem::path(Path::Combine("Bolt-Editor", "Projects"));
			RefreshProjectEntries();
			m_OpenProjectDialog = true;
		}

		if (!m_LastSaveStatus.empty()) {
			if (m_LastSaveSucceeded) {
				ImGui::TextColored(ImVec4(0.4f, 0.85f, 0.4f, 1.0f), "%s", m_LastSaveStatus.c_str());
			}
			else {
				ImGui::TextColored(ImVec4(0.95f, 0.45f, 0.45f, 1.0f), "%s", m_LastSaveStatus.c_str());
			}
		}

		ImGui::End();
	}

	void EditorUISystem::DrawGameView(Scene& scene) {
		(void)scene;
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		const float menuBarHeight = 20.0f;
		const float toolbarHeight = 46.0f;
		const float panelTop = viewport->Pos.y + menuBarHeight + toolbarHeight + 12.0f;
		const float panelBottomPadding = 8.0f;
		const float panelHeight = viewport->Size.y - (panelTop - viewport->Pos.y) - panelBottomPadding;
		const float leftWidth = 300.0f;
		const float rightWidth = 360.0f;
		const float gutter = 8.0f;
		const float centerX = viewport->Pos.x + leftWidth + gutter;
		const float centerWidth = viewport->Size.x - leftWidth - rightWidth - (gutter * 2.0f);

		ImGui::SetNextWindowPos(ImVec2(centerX, panelTop), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(centerWidth, panelHeight), ImGuiCond_Always);

		if (!ImGui::Begin("Game View", nullptr,
			ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse)) {
			ImGui::End();
			return;
		}

		ImGui::TextDisabled("Live viewport");
		ImGui::Separator();
		ImGui::TextWrapped("The game is rendered live behind this panel. Use Play/Stop in the toolbar to control simulation.");

		ImVec2 min = ImGui::GetCursorScreenPos();
		ImVec2 avail = ImGui::GetContentRegionAvail();
		ImVec2 max(min.x + avail.x, min.y + avail.y);
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		drawList->AddRect(min, max, IM_COL32(180, 190, 210, 90), 8.0f, 0, 1.0f);
		drawList->AddText(ImVec2(min.x + 12.0f, min.y + 12.0f), IM_COL32(180, 190, 210, 255), "Game Output");
		ImGui::Dummy(avail);

		ImGui::End();
	}

	void EditorUISystem::DrawHierarchy(Scene& scene) {
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		const float menuBarHeight = 20.0f;
		const float toolbarHeight = 46.0f;
		const float panelTop = viewport->Pos.y + menuBarHeight + toolbarHeight + 12.0f;
		const float panelBottomPadding = 8.0f;
		const float panelHeight = viewport->Size.y - (panelTop - viewport->Pos.y) - panelBottomPadding;
		const float panelWidth = 300.0f;

		ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, panelTop), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight), ImGuiCond_Always);

		if (!ImGui::Begin("Hierarchy", nullptr,
			ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse)) {
			ImGui::End();
			return;
		}

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

	void EditorUISystem::DrawInspector(Scene& scene) {
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		const float menuBarHeight = 20.0f;
		const float toolbarHeight = 46.0f;
		const float panelTop = viewport->Pos.y + menuBarHeight + toolbarHeight + 12.0f;
		const float panelBottomPadding = 8.0f;
		const float panelHeight = viewport->Size.y - (panelTop - viewport->Pos.y) - panelBottomPadding;
		const float panelWidth = 360.0f;
		const float panelX = viewport->Pos.x + viewport->Size.x - panelWidth;

		ImGui::SetNextWindowPos(ImVec2(panelX, panelTop), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight), ImGuiCond_Always);

		if (!ImGui::Begin("Inspector", nullptr,
			ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse)) {
			ImGui::End();
			return;
		}

		if (m_SelectedEntity == entt::null || !scene.IsValid(m_SelectedEntity)) {
			ImGui::TextDisabled("Select an entity to edit its components.");
			ImGui::End();
			return;
		}

		Entity entity = scene.GetEntity(m_SelectedEntity);
		DrawComponentManagement(entity);

		if (entity.HasComponent<NameComponent>()) {
			auto& name = entity.GetComponent<NameComponent>().Name;
			char buffer[128]{};
			std::snprintf(buffer, sizeof(buffer), "%s", name.c_str());
			if (ImGui::InputText("Name", buffer, sizeof(buffer))) {
				name = buffer;
			}
		}

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
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x + viewport->Size.x - 320.0f, viewport->Pos.y + 74.0f), ImGuiCond_Always);
		ImGui::SetNextWindowBgAlpha(0.88f);
		if (!ImGui::Begin("Stats", nullptr,
			ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse)) {
			ImGui::End();
			return;
		}

		auto* app = Application::GetInstance();
		auto& time = app->GetTime();
		const float fps = 1.0f / Max(0.0001f, time.GetDeltaTimeUnscaled());
		ImGui::Text("FPS: %.1f", fps);
		ImGui::Text("Frame: %d", time.GetFrameCount());
		ImGui::Text("Rendered Instances: %d", app->GetRenderer2D()->GetRenderedInstancesCount());

		ImGui::Text("Registered Scenes");
		auto registeredScenes = app->GetSceneManager()->GetRegisteredSceneNames();

		for (const auto& sceneName : registeredScenes) {
			ImGui::Text(("- " + sceneName).c_str());
		}

		std::string activeScene = "Active Scene: " + app->GetSceneManager()->GetActiveScene()->GetName();
		ImGui::Text(activeScene.c_str());
		ImGui::End();
	}

	void EditorUISystem::DrawProjectLoader(Scene& scene) {
		if (!m_OpenProjectDialog) {
			return;
		}

		ImGui::OpenPopup("Load Existing Project");
		if (ImGui::BeginPopupModal("Load Existing Project", &m_OpenProjectDialog, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::TextWrapped("Select a project file from %s", m_ProjectRootPath.string().c_str());
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

		std::string error;
		if (LoadSceneFromFile(scene, projectPath, error)) {
			m_LoadedProjectPath = projectPath;
			m_ProjectFilePath = projectPath.string();
			m_LastSaveSucceeded = true;
			m_LastSaveStatus = "Loaded project from '" + projectPath.string() + "'";
			Logger::Message("Editor", m_LastSaveStatus);
			m_SelectedEntity = entt::null;
			m_OpenProjectDialog = false;
		}
		else {
			m_LastSaveSucceeded = false;
			m_LastSaveStatus = "Load failed: " + error;
			Logger::Error("Editor", m_LastSaveStatus);
		}
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

			if (entry.is_regular_file(errorCode) && !errorCode &&
				(entry.path().extension() == ".json" || entry.path().extension() == ".boltproject.json")) {
				m_ProjectEntries.push_back(entry.path());
			}
		}

		std::sort(m_ProjectEntries.begin(), m_ProjectEntries.end());
	}

	void EditorUISystem::CreateEntity(Scene& scene) {
		Entity entity = scene.CreateEntity("Entity (" + StringHelper::ToString(EntityHelper::EntitiesCount()) + ")");
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

	bool EditorUISystem::SaveProject(const Scene& scene, const std::string& path, std::string& outError) const {
		return SaveSceneToFile(scene, path, outError);
	}

	void EditorUISystem::SaveProjectFromGui(const Scene& scene, const std::string& path) {
		const std::string normalizedPath = StringHelper::Trim(path);
		if (normalizedPath.empty()) {
			m_LastSaveSucceeded = false;
			m_LastSaveStatus = "Save failed: path is empty.";
			return;
		}

		std::string error;
		if (SaveProject(scene, normalizedPath, error)) {
			m_ProjectFilePath = normalizedPath;
			if (!std::filesystem::path(m_ProjectFilePath).has_extension()) {
				m_ProjectFilePath += ".boltproject.json";
			}
			m_LastSaveSucceeded = true;
			m_LastSaveStatus = "Saved project to '" + m_ProjectFilePath + "'";
			Logger::Message("Editor", m_LastSaveStatus);
		}
		else {
			m_LastSaveSucceeded = false;
			m_LastSaveStatus = "Save failed: " + error;
			Logger::Error("Editor", m_LastSaveStatus);
		}
	}

	std::string EditorUISystem::GetDefaultProjectSavePath(const Scene& scene) const {
		const std::string safeSceneName = StringHelper::Replace(scene.GetName(), " ", "_");
		return Path::Combine("Bolt-Editor", "Projects", safeSceneName + ".boltproject.json");
	}
}