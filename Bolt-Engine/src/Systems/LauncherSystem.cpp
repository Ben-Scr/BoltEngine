#include "pch.hpp"
#include "Systems/LauncherSystem.hpp"
#include "Project/BoltProject.hpp"
#include "Project/ProjectManager.hpp"
#include "Scene/SceneManager.hpp"
#include "Core/Application.hpp"
#include <Core/Version.hpp>
#include <Core/Log.hpp>
#include "Serialization/Path.hpp"
#include "Serialization/Directory.hpp"
#include "Graphics/Renderer2D.hpp"
#include <imgui.h>
#include <filesystem>
#include <cstring>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <chrono>

#ifdef BT_PLATFORM_WINDOWS
#include <windows.h>
#include <shobjidl.h>
#endif

namespace Bolt {

	// ── Relative Time Formatting ────────────────────────────────────

	static std::string FormatRelativeTime(const std::string& iso8601) {
		if (iso8601.empty()) return "";

		std::tm tm{};
		std::istringstream ss(iso8601);
		ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
		if (ss.fail()) return "";

		std::time_t then = std::mktime(&tm);
		std::time_t now = std::time(nullptr);
		double seconds = std::difftime(now, then);

		if (seconds < 0) return "Now";
		if (seconds < 60) return "Now";

		int minutes = static_cast<int>(seconds / 60);
		if (minutes < 60) return std::to_string(minutes) + "m ago";

		int hours = minutes / 60;
		if (hours < 24) return std::to_string(hours) + "h ago";

		int days = hours / 24;
		if (days < 30) return std::to_string(days) + "d ago";

		int months = days / 30;
		int remainingDays = days % 30;
		if (months < 12) {
			if (remainingDays > 0)
				return std::to_string(months) + "mo " + std::to_string(remainingDays) + "d ago";
			return std::to_string(months) + "mo ago";
		}

		int years = months / 12;
		int remainingMonths = months % 12;
		if (remainingMonths > 0)
			return std::to_string(years) + "y " + std::to_string(remainingMonths) + "mo ago";
		return std::to_string(years) + "y ago";
	}

	// ── Lifecycle ───────────────────────────────────────────────────

	void LauncherSystem::Awake(Scene& scene) {
		(void)scene;
		auto* app = Application::GetInstance();
		if (app && app->GetRenderer2D()) {
			app->GetRenderer2D()->SetSkipBeginFrameRender(true);
		}
		Application::SetIsPlaying(false);

		m_Registry.Load();
		m_Registry.ValidateAll();
		m_Registry.Save();

		std::snprintf(m_NewProjectLocation, sizeof(m_NewProjectLocation), "%s",
			BoltProject::GetDefaultProjectsDir().c_str());

		BT_INFO_TAG("Launcher", "Bolt Launcher opened ({} project(s))", m_Registry.GetProjects().size());
	}

	void LauncherSystem::OnGui(Scene& scene) {
		(void)scene;
		RenderLauncherPanel();
		if (m_ShowCreatePopup) RenderCreateProjectPopup();
	}

	void LauncherSystem::OnDestroy(Scene& scene) {
		(void)scene;
	}

	// ── Main Panel ──────────────────────────────────────────────────

	void LauncherSystem::RenderLauncherPanel() {
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);

		ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
			| ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking;

		ImGui::Begin("##Launcher", nullptr, flags);

		ImGui::TextUnformatted("Bolt Engine");
		ImGui::SameLine();
		ImGui::TextDisabled("%s", BT_VERSION);
		ImGui::Separator();
		ImGui::Spacing();

		// While opening a project, show a fullscreen loading overlay and block all interaction
		if (m_IsOpening) {
			float w = ImGui::GetContentRegionAvail().x;
			float h = ImGui::GetContentRegionAvail().y;
			ImVec2 center(ImGui::GetCursorScreenPos().x + w * 0.5f,
			              ImGui::GetCursorScreenPos().y + h * 0.4f);

			ImGui::SetCursorScreenPos(ImVec2(center.x - 120, center.y - 20));
			ImGui::TextUnformatted("Opening project...");

			ImGui::SetCursorScreenPos(ImVec2(center.x - 120, center.y + 5));
			float elapsed = std::chrono::duration<float>(
				std::chrono::steady_clock::now() - m_OpenStartTime).count();
			ImGui::ProgressBar(fmodf(elapsed * 0.3f, 1.0f), ImVec2(240, 0), "");

			ImGui::SetCursorScreenPos(ImVec2(center.x - 120, center.y + 30));
			ImGui::TextDisabled("%s", m_OpeningProjectName.c_str());

			ImGui::End();
			return; // Skip all other UI — blocks interaction completely
		}

		float panelWidth = ImGui::GetContentRegionAvail().x;
		float rightColWidth = 200.0f;

		// Left column: project list
		ImGui::BeginChild("##ProjectList", ImVec2(panelWidth - rightColWidth - 10, 0), true);
		RenderProjectList();
		ImGui::EndChild();

		ImGui::SameLine();

		// Right column: actions
		ImGui::BeginChild("##Actions", ImVec2(rightColWidth, 0));

		if (ImGui::Button("Create New Project", ImVec2(-1, 0))) {
			m_ShowCreatePopup = true;
			m_CreateError.clear();
			m_NewProjectName[0] = '\0';
		}

		ImGui::Spacing();

		if (ImGui::Button("Add Existing Project", ImVec2(-1, 0))) {
			BrowseForExistingProject();
		}

		if (!m_BrowseError.empty()) {
			ImGui::TextColored(ImVec4(1, 0.3f, 0.3f, 1), "%s", m_BrowseError.c_str());
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		bool hasSelection = m_SelectedIndex >= 0
			&& m_SelectedIndex < static_cast<int>(m_Registry.GetProjects().size());

		if (hasSelection) {
			if (ImGui::Button("Open Selected", ImVec2(-1, 0))) {
				OpenProject(m_Registry.GetProjects()[m_SelectedIndex]);
			}
			ImGui::Spacing();
			if (ImGui::Button("Remove from List", ImVec2(-1, 0))) {
				m_Registry.RemoveProject(m_Registry.GetProjects()[m_SelectedIndex].path);
				m_Registry.Save();
				m_SelectedIndex = -1;
			}
		}

		ImGui::EndChild();

		ImGui::End();
	}

	// ── Project List ────────────────────────────────────────────────

	void LauncherSystem::RenderProjectList() {
		const auto& projects = m_Registry.GetProjects();

		if (projects.empty()) {
			ImGui::TextDisabled("No projects yet");
			ImGui::TextDisabled("Create a new project or add an existing one.");
			return;
		}

		for (int i = 0; i < static_cast<int>(projects.size()); i++) {
			const auto& entry = projects[i];
			bool selected = (m_SelectedIndex == i);

			ImGui::PushID(i);

			std::string timeStr = FormatRelativeTime(entry.lastOpened);

			ImVec2 cursorPos = ImGui::GetCursorScreenPos();
			ImVec2 availSize = ImVec2(ImGui::GetContentRegionAvail().x, 50.0f);

			if (ImGui::Selectable("##Entry", selected, 0, availSize)) {
				m_SelectedIndex = i;
			}

			if (!m_IsOpening && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
				OpenProject(entry);
			}

			// Draw name and details over the selectable
			ImGui::SetCursorScreenPos(ImVec2(cursorPos.x + 8, cursorPos.y + 4));
			ImGui::TextUnformatted(entry.name.c_str());

			ImGui::SetCursorScreenPos(ImVec2(cursorPos.x + 8, cursorPos.y + 24));
			ImGui::TextDisabled("%s", entry.path.c_str());

			if (!timeStr.empty()) {
				float timeWidth = ImGui::CalcTextSize(timeStr.c_str()).x;
				ImGui::SetCursorScreenPos(ImVec2(
					cursorPos.x + availSize.x - timeWidth - 12,
					cursorPos.y + 4));
				ImGui::TextDisabled("%s", timeStr.c_str());
			}

			ImGui::SetCursorScreenPos(ImVec2(cursorPos.x, cursorPos.y + availSize.y));

			ImGui::PopID();
		}
	}

	// ── Create Project Popup ────────────────────────────────────────

	void LauncherSystem::RenderCreateProjectPopup() {
		ImGui::OpenPopup("Create New Project");

		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		ImGui::SetNextWindowSize(ImVec2(450, 0), ImGuiCond_Appearing);

		if (ImGui::BeginPopupModal("Create New Project", &m_ShowCreatePopup, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::Text("Project Name:");
			ImGui::SetNextItemWidth(-1);
			ImGui::InputText("##ProjName", m_NewProjectName, sizeof(m_NewProjectName));

			ImGui::Spacing();
			ImGui::Text("Location:");
			ImGui::SetNextItemWidth(-1);
			ImGui::InputText("##ProjLocation", m_NewProjectLocation, sizeof(m_NewProjectLocation));

			ImGui::Spacing();
			std::string preview = Path::Combine(m_NewProjectLocation, m_NewProjectName);
			ImGui::TextDisabled("Path: %s", preview.c_str());

			if (!m_CreateError.empty()) {
				ImGui::Spacing();
				ImGui::TextColored(ImVec4(1, 0.3f, 0.3f, 1), "%s", m_CreateError.c_str());
			}

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			bool canCreate = BoltProject::IsValidProjectName(m_NewProjectName);
			if (!canCreate) ImGui::BeginDisabled();

			if (ImGui::Button("Create", ImVec2(120, 0))) {
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

						std::string buildCmd = "dotnet build \"" + project.CsprojPath + "\" -c Release --nologo -v q -nowarn:CS8632";
						std::system(buildCmd.c_str());

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
			if (ImGui::Button("Cancel", ImVec2(120, 0))) {
				m_ShowCreatePopup = false;
			}

			ImGui::EndPopup();
		}
	}

	// ── Open Project ────────────────────────────────────────────────
	// Launches the editor process but keeps the launcher open.

	void LauncherSystem::OpenProject(const LauncherProjectEntry& entry) {
		if (m_IsOpening) return; // Already opening a project — ignore

		// Set loading state FIRST — before anything that could re-sort the list
		m_IsOpening = true;
		m_OpenStartTime = std::chrono::steady_clock::now();
		m_OpeningProjectName = entry.name;

		// Now safe to update (even if it re-sorts, the loading screen blocks further clicks)
		m_Registry.UpdateLastOpened(entry.path);
		m_Registry.Save();

#ifdef BT_PLATFORM_WINDOWS
		auto exeDir = std::filesystem::path(Path::ExecutableDir());
		auto editorExe = exeDir / "Bolt-Editor.exe";
		if (!std::filesystem::exists(editorExe)) {
			editorExe = exeDir / ".." / "Bolt-Editor" / "Bolt-Editor.exe";
		}

		if (std::filesystem::exists(editorExe)) {
			std::string canonical = std::filesystem::canonical(editorExe).string();
			std::string cmdLine = "\"" + canonical + "\" --project=\"" + entry.path + "\"";

			std::thread([this, cmdLine]() {
				std::wstring wcmd(cmdLine.begin(), cmdLine.end());
				std::vector<wchar_t> buf(wcmd.begin(), wcmd.end());
				buf.push_back(L'\0');

				STARTUPINFOW si{};
				si.cb = sizeof(si);
				PROCESS_INFORMATION pi{};

				if (CreateProcessW(nullptr, buf.data(), nullptr, nullptr,
					FALSE, CREATE_NEW_PROCESS_GROUP, nullptr, nullptr, &si, &pi))
				{
					CloseHandle(pi.hProcess);
					CloseHandle(pi.hThread);
				}

				m_IsOpening = false;
			}).detach();

			BT_INFO_TAG("Launcher", "Opened project: {} at {}", entry.name, entry.path);
		}
		else {
			BT_ERROR_TAG("Launcher", "Bolt-Editor.exe not found");
			m_IsOpening = false;
		}
#endif
	}

	// ── Browse for Existing Project ─────────────────────────────────

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
				IShellItem* result = nullptr;
				dialog->GetResult(&result);
				if (result) {
					PWSTR widePath = nullptr;
					result->GetDisplayName(SIGDN_FILESYSPATH, &widePath);
					if (widePath) {
						int len = WideCharToMultiByte(CP_UTF8, 0, widePath, -1, nullptr, 0, nullptr, nullptr);
						std::string path(len - 1, '\0');
						WideCharToMultiByte(CP_UTF8, 0, widePath, -1, path.data(), len, nullptr, nullptr);
						CoTaskMemFree(widePath);

						if (BoltProject::Validate(path)) {
							auto project = BoltProject::Load(path);
							m_Registry.AddProject(project.Name, project.RootDirectory);
							m_Registry.Save();
						}
						else {
							m_BrowseError = "Not a valid Bolt project (missing bolt-project.json or Assets/)";
						}
					}
					result->Release();
				}
			}
			dialog->Release();
		}

		CoUninitialize();
#endif
	}

}
