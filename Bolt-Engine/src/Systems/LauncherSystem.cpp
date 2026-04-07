#include "pch.hpp"
#include "Systems/LauncherSystem.hpp"
#include "Project/BoltProject.hpp"
#include "Project/ProjectManager.hpp"
#include "Scene/SceneManager.hpp"
#include "Core/Application.hpp"
#include "Core/Version.hpp"
#include "Core/Log.hpp"
#include "Serialization/Path.hpp"
#include "Serialization/Directory.hpp"
#include "Graphics/Renderer2D.hpp"

#include <imgui.h>
#include <filesystem>
#include <cstring>

#ifdef BT_PLATFORM_WINDOWS
#include <windows.h>
#include <shobjidl.h>
#endif

namespace Bolt {

	void LauncherSystem::Awake(Scene& scene) {
		(void)scene;

		auto* app = Application::GetInstance();
		if (app && app->GetRenderer2D())
			app->GetRenderer2D()->SetSkipBeginFrameRender(true);

		Application::SetIsPlaying(false);

		m_Registry.Load();
		m_Registry.ValidateAll();
		m_Registry.Save();

		std::string defaultDir = BoltProject::GetDefaultProjectsDir();
		std::strncpy(m_NewProjectLocation, defaultDir.c_str(), sizeof(m_NewProjectLocation) - 1);

		BT_CORE_INFO_TAG("Launcher", "Launcher opened with {} known projects", m_Registry.GetProjects().size());
	}

	void LauncherSystem::OnGui(Scene& scene) {
		(void)scene;
		RenderLauncherPanel();

		if (m_ShowCreatePopup)
			RenderCreateProjectPopup();
	}

	void LauncherSystem::OnDestroy(Scene& scene) {
		(void)scene;
	}

	void LauncherSystem::RenderLauncherPanel() {
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);

		ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
			| ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse
			| ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoDocking;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(30, 30));
		ImGui::Begin("##Launcher", nullptr, flags);
		ImGui::PopStyleVar();

		// Header
		ImGui::PushFont(nullptr);
		ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "Bolt Engine");
		ImGui::SameLine();
		ImGui::TextDisabled("%s", BT_VERSION);
		ImGui::Separator();
		ImGui::Spacing();

		// Two-column layout
		float panelWidth = ImGui::GetContentRegionAvail().x;
		float rightColWidth = 220.0f;
		float leftColWidth = panelWidth - rightColWidth - 20.0f;

		// Left column: project list (use remaining height)
		float availHeight = ImGui::GetContentRegionAvail().y;
		ImGui::BeginChild("##ProjectListCol", ImVec2(leftColWidth, availHeight), true);
		ImGui::Text("Projects");
		ImGui::Spacing();
		RenderProjectList();
		ImGui::EndChild();

		ImGui::SameLine(0, 20);

		// Right column: actions
		ImGui::BeginChild("##ActionsCol", ImVec2(rightColWidth, availHeight), ImGuiChildFlags_None);

		if (ImGui::Button("Create New Project", ImVec2(-1, 40))) {
			m_ShowCreatePopup = true;
			m_CreateError.clear();
			std::memset(m_NewProjectName, 0, sizeof(m_NewProjectName));
		}

		ImGui::Spacing();

		if (ImGui::Button("Add Existing Project", ImVec2(-1, 40))) {
			BrowseForExistingProject();
		}

		if (!m_BrowseError.empty()) {
			ImGui::Spacing();
			ImGui::TextColored(ImVec4(1, 0.4f, 0.4f, 1), "%s", m_BrowseError.c_str());
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		if (m_SelectedIndex >= 0 && m_SelectedIndex < (int)m_Registry.GetProjects().size()) {
			if (ImGui::Button("Open Selected", ImVec2(-1, 35))) {
				OpenProject(m_Registry.GetProjects()[m_SelectedIndex]);
			}
			ImGui::Spacing();
			if (ImGui::Button("Remove from List", ImVec2(-1, 30))) {
				m_Registry.RemoveProject(m_Registry.GetProjects()[m_SelectedIndex].path);
				m_Registry.Save();
				m_SelectedIndex = -1;
			}
		}

		ImGui::EndChild();
		ImGui::PopFont();
		ImGui::End();
	}

	void LauncherSystem::RenderProjectList() {
		auto& projects = m_Registry.GetProjects();

		if (projects.empty()) {
			ImGui::TextDisabled("No projects yet. Create a new project to get started.");
			return;
		}

		for (int i = 0; i < (int)projects.size(); i++) {
			auto& entry = projects[i];
			bool selected = (m_SelectedIndex == i);

			ImGui::PushID(i);

			// Simple selectable with embedded text — no custom drawing
			std::string label = entry.name + "\n" + entry.path;
			if (ImGui::Selectable(label.c_str(), selected, 0, ImVec2(0, 45)))
				m_SelectedIndex = i;

			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				OpenProject(entry);

			ImGui::PopID();
		}
	}

	void LauncherSystem::RenderCreateProjectPopup() {
		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		ImGui::SetNextWindowSize(ImVec2(500, 250), ImGuiCond_Appearing);

		if (ImGui::Begin("Create New Project", &m_ShowCreatePopup, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse)) {
			ImGui::Text("Project Name:");
			ImGui::SetNextItemWidth(-1);
			ImGui::InputText("##ProjName", m_NewProjectName, sizeof(m_NewProjectName));

			ImGui::Spacing();
			ImGui::Text("Location:");
			ImGui::SetNextItemWidth(-1);
			ImGui::InputText("##ProjLocation", m_NewProjectLocation, sizeof(m_NewProjectLocation));

			// Preview path
			if (strlen(m_NewProjectName) > 0) {
				ImGui::TextDisabled("Project will be created at: %s/%s",
					m_NewProjectLocation, m_NewProjectName);
			}

			ImGui::Spacing();

			if (!m_CreateError.empty())
				ImGui::TextColored(ImVec4(1, 0.3f, 0.3f, 1), "%s", m_CreateError.c_str());

			ImGui::Spacing();

			bool canCreate = strlen(m_NewProjectName) > 0
				&& BoltProject::IsValidProjectName(m_NewProjectName);

			if (!canCreate) ImGui::BeginDisabled();

			if (ImGui::Button("Create", ImVec2(120, 35))) {
				try {
					std::string name(m_NewProjectName);
					std::string location(m_NewProjectLocation);

					std::string fullPath = Path::Combine(location, name);
					if (std::filesystem::exists(fullPath)) {
						m_CreateError = "Directory already exists: " + fullPath;
					} else {
						auto project = BoltProject::Create(name, location);
						m_Registry.AddProject(project.Name, project.RootDirectory);
						m_Registry.Save();

						m_ShowCreatePopup = false;

						// Build the solution
						std::string buildCmd = "dotnet build \"" + project.CsprojPath + "\" -c Release --nologo -v q -nowarn:CS8632";
						std::system(buildCmd.c_str());

						// Open the project
						LauncherProjectEntry entry;
						entry.name = project.Name;
						entry.path = project.RootDirectory;
						OpenProject(entry);
					}
				} catch (const std::exception& e) {
					m_CreateError = e.what();
				}
			}

			if (!canCreate) ImGui::EndDisabled();

			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 35))) {
				m_ShowCreatePopup = false;
			}
		}
		ImGui::End();
	}

	void LauncherSystem::OpenProject(const LauncherProjectEntry& entry) {
		m_Registry.UpdateLastOpened(entry.path);
		m_Registry.Save();

#ifdef BT_PLATFORM_WINDOWS
		// Launch Bolt-Editor.exe with the project path as argument
		auto exeDir = std::filesystem::path(Path::ExecutableDir());
		auto editorExe = exeDir / "Bolt-Editor.exe";

		// If launcher is in Bolt-Launcher/, editor is in Bolt-Editor/ (sibling output dirs)
		if (!std::filesystem::exists(editorExe))
			editorExe = exeDir / ".." / "Bolt-Editor" / "Bolt-Editor.exe";

		if (std::filesystem::exists(editorExe))
		{
			std::wstring cmdLine = L"\"" + std::filesystem::canonical(editorExe).wstring()
				+ L"\" --project=\"" + std::wstring(entry.path.begin(), entry.path.end()) + L"\"";

			STARTUPINFOW si{};
			si.cb = sizeof(si);
			PROCESS_INFORMATION pi{};

			std::vector<wchar_t> buf(cmdLine.begin(), cmdLine.end());
			buf.push_back(L'\0');

			if (CreateProcessW(nullptr, buf.data(), nullptr, nullptr,
				FALSE, CREATE_NEW_PROCESS_GROUP, nullptr, nullptr, &si, &pi))
			{
				CloseHandle(pi.hProcess);
				CloseHandle(pi.hThread);
				BT_INFO_TAG("Launcher", "Launched editor for project: {}", entry.name);

				// Close the launcher — editor is now running
				Application::Quit();
			}
			else
			{
				BT_ERROR_TAG("Launcher", "Failed to launch editor");
			}
		}
		else
		{
			BT_ERROR_TAG("Launcher", "Bolt-Editor.exe not found");
		}
#endif
	}

	void LauncherSystem::BrowseForExistingProject() {
		m_BrowseError.clear();

#ifdef BT_PLATFORM_WINDOWS
		CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

		IFileOpenDialog* dialog = nullptr;
		HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_ALL,
			IID_IFileOpenDialog, reinterpret_cast<void**>(&dialog));

		if (SUCCEEDED(hr)) {
			DWORD options;
			dialog->GetOptions(&options);
			dialog->SetOptions(options | FOS_PICKFOLDERS);
			dialog->SetTitle(L"Select Bolt Project Folder");

			hr = dialog->Show(nullptr);
			if (SUCCEEDED(hr)) {
				IShellItem* item = nullptr;
				dialog->GetResult(&item);
				if (item) {
					PWSTR folderPath = nullptr;
					item->GetDisplayName(SIGDN_FILESYSPATH, &folderPath);
					if (folderPath) {
						int size = WideCharToMultiByte(CP_UTF8, 0, folderPath, -1, nullptr, 0, nullptr, nullptr);
						std::string path(size - 1, 0);
						WideCharToMultiByte(CP_UTF8, 0, folderPath, -1, &path[0], size, nullptr, nullptr);
						CoTaskMemFree(folderPath);

						if (BoltProject::Validate(path)) {
							auto project = BoltProject::Load(path);
							m_Registry.AddProject(project.Name, project.RootDirectory);
							m_Registry.Save();
						} else {
							m_BrowseError = "Not a valid Bolt project (missing bolt-project.json or Assets/)";
						}
					}
					item->Release();
				}
			}
			dialog->Release();
		}

		CoUninitialize();
#endif
	}

} // namespace Bolt
