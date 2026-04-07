#include <pch.hpp>
#include "Gui/AssetBrowser.hpp"
#include "Serialization/Path.hpp"
#include "Project/ProjectManager.hpp"
#include "Serialization/SceneSerializer.hpp"
#include "Scene/SceneManager.hpp"
#include "Core/Log.hpp"
#include <imgui.h>
#include <algorithm>
#include <filesystem>
#include <fstream>

#include <thread>

#ifdef BT_PLATFORM_WINDOWS
#include <windows.h>
#include <shellapi.h>
#endif

namespace Bolt {
	void AssetBrowser::Initialize(const std::string& rootDirectory) {
		m_RootDirectory = rootDirectory;
		m_CurrentDirectory = rootDirectory;
		m_Thumbnails.Initialize();
		m_NeedsRefresh = true;
	}

	void AssetBrowser::Shutdown() {
		m_Thumbnails.Shutdown();
	}

	void AssetBrowser::NavigateTo(const std::string& directory) {
		m_CurrentDirectory = directory;
		m_SelectedPath.clear();
		CancelRename();
		m_NeedsRefresh = true;
	}

	void AssetBrowser::NavigateUp() {
		std::filesystem::path current(m_CurrentDirectory);
		std::filesystem::path root(m_RootDirectory);

		if (current != root && current.has_parent_path()) {
			std::filesystem::path parent = current.parent_path();
			if (parent.string().size() >= root.string().size()) {
				NavigateTo(parent.string());
			}
		}
	}

	void AssetBrowser::Refresh() {
		m_Entries = Directory::GetEntries(m_CurrentDirectory);
		m_NeedsRefresh = false;
	}

	void AssetBrowser::BeginRename(const std::string& path, const std::string& currentName) {
		m_IsRenaming = true;
		m_RenamePath = path;
		m_RenameFrameCounter = 0;
		std::snprintf(m_RenameBuffer, sizeof(m_RenameBuffer), "%s", currentName.c_str());
	}

	void AssetBrowser::CommitRename() {
		std::string newName(m_RenameBuffer);
		std::string oldName = std::filesystem::path(m_RenamePath).filename().string();

		if (!newName.empty() && newName != oldName) {
			RenameEntry(m_RenamePath, newName);
		}
		CancelRename();
	}

	void AssetBrowser::CancelRename() {
		m_IsRenaming = false;
		m_RenamePath.clear();
		m_RenameFrameCounter = 0;
	}

	bool AssetBrowser::IsRenamingEntry(const std::string& path) const {
		return m_IsRenaming && m_RenamePath == path;
	}

	void AssetBrowser::Render() {
		// Load pending scene on main thread (before ImGui frame)
		if (!m_PendingSceneLoad.empty())
		{
			std::string scenePath = m_PendingSceneLoad;
			m_PendingSceneLoad.clear();

			Scene* active = SceneManager::Get().GetActiveScene();
			if (active)
			{
				SceneSerializer::LoadFromFile(*active, scenePath);

				BoltProject* project = ProjectManager::GetCurrentProject();
				if (project)
				{
					std::string sceneName = std::filesystem::path(scenePath).stem().string();
					project->LastOpenedScene = sceneName;
					project->Save();
				}
			}
		}

		ImGui::Begin("Project");

		if (m_NeedsRefresh) {
			Refresh();
		}

		RenderBreadcrumb();
		ImGui::Separator();
		RenderGrid();

		ImGui::End();
	}

	void AssetBrowser::RenderBreadcrumb() {
		std::filesystem::path root(m_RootDirectory);
		std::filesystem::path current(m_CurrentDirectory);
		std::filesystem::path relative = std::filesystem::relative(current, root);

		if (m_CurrentDirectory != m_RootDirectory) {
			if (ImGui::SmallButton("<")) {
				NavigateUp();
				return;
			}
			ImGui::SameLine();
		}

		if (ImGui::SmallButton("Assets")) {
			NavigateTo(m_RootDirectory);
			return;
		}

		if (relative != "." && !relative.empty()) {
			std::filesystem::path accumulated = root;
			for (const auto& segment : relative) {
				accumulated /= segment;
				ImGui::SameLine();
				ImGui::TextUnformatted("/");
				ImGui::SameLine();

				std::string segStr = segment.string();
				std::string accStr = accumulated.string();

				if (accStr == m_CurrentDirectory) {
					ImGui::TextUnformatted(segStr.c_str());
				}
				else {
					if (ImGui::SmallButton(segStr.c_str())) {
						NavigateTo(accStr);
						return;
					}
				}
			}
		}

		ImGui::SameLine(ImGui::GetContentRegionAvail().x + ImGui::GetCursorPosX() - 30.0f);
		if (ImGui::SmallButton("R")) {
			m_NeedsRefresh = true;
		}
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("Refresh");
		}
	}

	void AssetBrowser::RenderGrid() {
		ImGui::BeginChild("AssetGrid", ImVec2(0, 0), ImGuiChildFlags_None, ImGuiWindowFlags_None);

		const float cellSize = m_TileSize + m_TilePadding;
		const float panelWidth = ImGui::GetContentRegionAvail().x;
		int columns = static_cast<int>(panelWidth / cellSize);
		if (columns < 1) columns = 1;

		m_ItemRightClicked = false;

		if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsAnyItemHovered()) {
			m_SelectedPath.clear();
			CancelRename();
		}

		for (int i = 0; i < static_cast<int>(m_Entries.size()); i++) {
			if (i % columns != 0) {
				ImGui::SameLine();
			}
			RenderAssetTile(m_Entries[i], i);
		}

		if (m_Entries.empty()) {
			ImGui::TextDisabled("Empty folder");
		}

		RenderGridContextMenu();

		ImGui::EndChild();
	}


	void AssetBrowser::RenderAssetTile(const DirectoryEntry& entry, int index) {
		ImGui::PushID(index);

		const bool isSelected = (m_SelectedPath == entry.Path);

		ImGui::BeginGroup();

		ImVec2 cursorPos = ImGui::GetCursorScreenPos();

		if (isSelected) {
			ImVec2 bgMin(cursorPos.x - 2, cursorPos.y - 2);
			ImVec2 bgMax(cursorPos.x + m_TileSize + 2, cursorPos.y + m_TileSize + ImGui::GetTextLineHeightWithSpacing() + 2);
			ImGui::GetWindowDrawList()->AddRectFilled(bgMin, bgMax, IM_COL32(60, 100, 160, 180), 4.0f);
		}

		ImVec2 iconPos = cursorPos;
		unsigned int thumbnail = 0;

		if (!entry.IsDirectory) {
			thumbnail = m_Thumbnails.GetThumbnail(entry.Path);
		}

		if (thumbnail != 0) {
			Texture2D* tex = nullptr;
			auto it = m_Thumbnails.GetCacheEntry(entry.Path);
			float texW = m_TileSize, texH = m_TileSize;
			if (it) {
				texW = it->GetWidth();
				texH = it->GetHeight();
			}

			float drawW = m_TileSize;
			float drawH = m_TileSize;

			if (texW > 0.0f && texH > 0.0f) {
				float aspect = texW / texH;
				if (aspect > 1.0f) {
					drawH = m_TileSize / aspect;
				}
				else {
					drawW = m_TileSize * aspect;
				}
			}

			float offsetX = (m_TileSize - drawW) * 0.5f;
			float offsetY = (m_TileSize - drawH) * 0.5f;

			ImGui::SetCursorScreenPos(ImVec2(iconPos.x + offsetX, iconPos.y + offsetY));
			ImGui::Image(
				static_cast<ImTextureID>(static_cast<intptr_t>(thumbnail)),
				ImVec2(drawW, drawH),
				ImVec2(0, 1), ImVec2(1, 0)
			);

			ImGui::SetCursorScreenPos(ImVec2(iconPos.x, iconPos.y + m_TileSize));
		}
		else {
			AssetType type = entry.IsDirectory
				? AssetType::Folder
				: ThumbnailCache::GetAssetType(std::filesystem::path(entry.Path).extension().string());

			ThumbnailCache::DrawAssetIcon(type, iconPos, m_TileSize);
			ImGui::Dummy(ImVec2(m_TileSize, m_TileSize));
		}

		if (IsRenamingEntry(entry.Path)) {
			m_RenameFrameCounter++;

			ImGui::PushItemWidth(m_TileSize);

			if (m_RenameFrameCounter == 1) {
				ImGui::SetKeyboardFocusHere();
			}

			bool committed = ImGui::InputText("##rename", m_RenameBuffer, sizeof(m_RenameBuffer),
				ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll);

			if (committed) {
				CommitRename();
			}
			else if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
				CancelRename();
			}
			else if (m_RenameFrameCounter > 2 && !ImGui::IsItemActive()) {
				CommitRename();
			}

			ImGui::PopItemWidth();
		}
		else {
			const float maxWidth = m_TileSize;
			const std::string& name = entry.Name;
			ImVec2 textSize = ImGui::CalcTextSize(name.c_str());

			if (textSize.x <= maxWidth) {
				float offsetX = (maxWidth - textSize.x) * 0.5f;
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX);
				ImGui::TextUnformatted(name.c_str());
			}
			else {
				const char* ellipsis = "...";
				float ellipsisWidth = ImGui::CalcTextSize(ellipsis).x;
				float availWidth = maxWidth - ellipsisWidth;

				int fitChars = 0;
				for (int i = 1; i <= static_cast<int>(name.size()); i++) {
					if (ImGui::CalcTextSize(name.c_str(), name.c_str() + i).x > availWidth)
						break;
					fitChars = i;
				}

				std::string display = name.substr(0, fitChars) + ellipsis;
				ImGui::TextUnformatted(display.c_str());

				if (ImGui::IsItemHovered()) {
					ImGui::SetTooltip("%s", name.c_str());
				}
			}
		}

		ImGui::EndGroup();

		if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
			m_SelectedPath = entry.Path;
			if (!IsRenamingEntry(entry.Path)) {
				CancelRename();
			}
		}

		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
			if (entry.IsDirectory) {
				NavigateTo(entry.Path);  // deferred via m_NeedsRefresh
			} else {
				OpenAssetExternal(entry);  // deferred via m_DeferredOpenPath
			}
		}

		RenderItemContextMenu(entry);

		HandleDragSource(entry);
		if (entry.IsDirectory) {
			HandleDropTarget(entry);
		}

		ImGui::PopID();

		ImGui::SameLine(0, 0);
		ImGui::Dummy(ImVec2(m_TilePadding, 0));
	}


	void AssetBrowser::RenderGridContextMenu() {
		if (!m_ItemRightClicked &&
			ImGui::IsMouseReleased(ImGuiMouseButton_Right) &&
			ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup))
		{
			ImGui::OpenPopup("##AssetGridCtx");
		}

		if (ImGui::BeginPopup("##AssetGridCtx")) {
			if (ImGui::MenuItem("Create Folder")) {
				CreateFolder(m_CurrentDirectory);
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Create Scene")) {
				CreateScene(m_CurrentDirectory);
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Create Script (C#)")) {
				CreateScript(m_CurrentDirectory);
			}
			if (ImGui::MenuItem("Create Script (C++)")) {
				CreateNativeScript(m_CurrentDirectory);
			}

			ImGui::EndPopup();
		}
	}

	void AssetBrowser::RenderItemContextMenu(const DirectoryEntry& entry) {
		if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
			ImGui::OpenPopup("##ItemCtx");
			m_ItemRightClicked = true;
		}

		if (ImGui::BeginPopup("##ItemCtx")) {
			m_SelectedPath = entry.Path;

			if (ImGui::MenuItem("Delete")) {
				DeleteEntry(entry.Path);
				ImGui::EndPopup();
				return;
			}
			if (ImGui::MenuItem("Rename")) {
				BeginRename(entry.Path, entry.Name);
			}
			if (ImGui::MenuItem("Copy Path")) {
				CopyPathToClipboard(entry.Path);
			}

			ImGui::EndPopup();
		}
	}

	void AssetBrowser::HandleDragSource(const DirectoryEntry& entry) {
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
			const char* pathStr = entry.Path.c_str();
			ImGui::SetDragDropPayload("ASSET_BROWSER_ITEM", pathStr, entry.Path.size() + 1);
			ImGui::Text("Move: %s", entry.Name.c_str());
			ImGui::EndDragDropSource();
		}
	}

	void AssetBrowser::HandleDropTarget(const DirectoryEntry& entry) {
		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_BROWSER_ITEM")) {
				std::string sourcePath(static_cast<const char*>(payload->Data));

				if (sourcePath != entry.Path) {
					if (Directory::Move(sourcePath, entry.Path)) {
						m_Thumbnails.Invalidate(sourcePath);
						if (m_SelectedPath == sourcePath) {
							m_SelectedPath.clear();
						}
						m_NeedsRefresh = true;
					}
				}
			}
			ImGui::EndDragDropTarget();
		}
	}


	void AssetBrowser::DeleteEntry(const std::string& path) {
		m_Thumbnails.Invalidate(path);

		if (Directory::Delete(path)) {
			if (m_SelectedPath == path) {
				m_SelectedPath.clear();
			}
			CancelRename();
			m_NeedsRefresh = true;
		}
	}

	void AssetBrowser::RenameEntry(const std::string& path, const std::string& newName) {
		m_Thumbnails.Invalidate(path);

		std::string oldExt = std::filesystem::path(path).extension().string();
		std::string oldStem = std::filesystem::path(path).stem().string();

		if (Directory::Rename(path, newName)) {
			std::filesystem::path p(path);
			std::string newPath = (p.parent_path() / newName).string();
			if (m_SelectedPath == path) {
				m_SelectedPath = newPath;
			}

			std::string ext = std::filesystem::path(newName).extension().string();
			std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
			std::string newStem = std::filesystem::path(newName).stem().string();
			auto exeDir = std::filesystem::path(Path::ExecutableDir());

			// Determine which project source dir this script belongs to
			std::filesystem::path projectSourceDir;
			if (ext == ".cs")
				projectSourceDir = exeDir / ".." / ".." / ".." / "Bolt-Sandbox" / "Source";
			else if (ext == ".cpp" || ext == ".h" || ext == ".hpp")
				projectSourceDir = exeDir / ".." / ".." / ".." / "Bolt-NativeScripts" / "Source";

			if (!projectSourceDir.empty() && std::filesystem::exists(projectSourceDir))
			{
				auto oldProjectFile = projectSourceDir / (oldStem + oldExt);
				auto newProjectFile = projectSourceDir / newName;

				if (std::filesystem::exists(oldProjectFile))
				{
					std::error_code ec;
					std::filesystem::rename(oldProjectFile, newProjectFile, ec);

					if (!ec)
					{
						// Update all occurrences of the old class name in the file
						std::ifstream in(newProjectFile);
						std::string content((std::istreambuf_iterator<char>(in)),
							std::istreambuf_iterator<char>());
						in.close();

						size_t pos = 0;
						while ((pos = content.find(oldStem, pos)) != std::string::npos)
						{
							content.replace(pos, oldStem.size(), newStem);
							pos += newStem.size();
						}

						std::ofstream out(newProjectFile);
						out << content;
					}
				}
			}

			m_NeedsRefresh = true;
		}
	}

	void AssetBrowser::CopyPathToClipboard(const std::string& path) {
		ImGui::SetClipboardText(path.c_str());
	}

	void AssetBrowser::CreateFolder(const std::string& parentDir) {
		std::string baseName = "New Folder";
		std::string folderPath = (std::filesystem::path(parentDir) / baseName).string();
		int counter = 1;
		while (Directory::Exists(folderPath)) {
			folderPath = (std::filesystem::path(parentDir) / (baseName + " " + std::to_string(counter))).string();
			counter++;
		}

		Directory::Create(folderPath, false);
		m_NeedsRefresh = true;

		Refresh();

		m_SelectedPath = folderPath;
		std::string name = std::filesystem::path(folderPath).filename().string();
		BeginRename(folderPath, name);
	}

	void AssetBrowser::CreateScript(const std::string& parentDir) {
		std::string baseName = "NewScript";
		std::string ext = ".cs";
		std::string scriptPath = (std::filesystem::path(parentDir) / (baseName + ext)).string();
		int counter = 1;
		while (std::filesystem::exists(scriptPath)) {
			scriptPath = (std::filesystem::path(parentDir) / (baseName + std::to_string(counter) + ext)).string();
			counter++;
		}

		// Derive class name from the filename (without extension)
		std::string className = std::filesystem::path(scriptPath).stem().string();

		// Write BoltScript boilerplate
		std::string boilerplate =
			"using Bolt;\n"
			"\n"
			"public class " + className + " : BoltScript\n"
			"{\n"
			"    public void Start()\n"
			"    {\n"
			"    }\n"
			"\n"
			"    public void Update()\n"
			"    {\n"
			"    }\n"
			"}\n";

		// Write to Assets folder (for Asset Browser display + drag-drop)
		std::ofstream file(scriptPath);
		if (file.is_open()) {
			file << boilerplate;
			file.close();
		}

		// Also write to Bolt-Sandbox/Source/ (for compilation)
		// The glob pattern in .csproj auto-includes it
		auto exeDir = std::filesystem::path(Path::ExecutableDir());
		auto sandboxSourceDir = exeDir / ".." / ".." / ".." / "Bolt-Sandbox" / "Source";
		if (std::filesystem::exists(sandboxSourceDir))
		{
			auto projectScriptPath = sandboxSourceDir / (className + ext);
			if (!std::filesystem::exists(projectScriptPath))
			{
				std::ofstream projectFile(projectScriptPath);
				if (projectFile.is_open()) {
					projectFile << boilerplate;
					projectFile.close();
					BT_INFO_TAG("AssetBrowser", "Script added to project: {}", projectScriptPath.string());
				}
			}
		}

		m_NeedsRefresh = true;
		Refresh();

		m_SelectedPath = scriptPath;
		std::string name = std::filesystem::path(scriptPath).filename().string();
		BeginRename(scriptPath, name);
	}

	void AssetBrowser::CreateNativeScript(const std::string& parentDir) {
		std::string baseName = "NewNativeScript";
		std::string ext = ".cpp";
		std::string scriptPath = (std::filesystem::path(parentDir) / (baseName + ext)).string();
		int counter = 1;
		while (std::filesystem::exists(scriptPath)) {
			scriptPath = (std::filesystem::path(parentDir) / (baseName + std::to_string(counter) + ext)).string();
			counter++;
		}

		std::string className = std::filesystem::path(scriptPath).stem().string();

		std::string boilerplate =
			"#include <Scripting/NativeScript.hpp>\n"
			"\n"
			"class " + className + " : public Bolt::NativeScript {\n"
			"public:\n"
			"    void Start() override\n"
			"    {\n"
			"    }\n"
			"\n"
			"    void Update(float dt) override\n"
			"    {\n"
			"    }\n"
			"\n"
			"    void OnDestroy() override\n"
			"    {\n"
			"    }\n"
			"};\n"
			"REGISTER_SCRIPT(" + className + ")\n";

		// Write to Assets folder
		std::ofstream file(scriptPath);
		if (file.is_open()) {
			file << boilerplate;
			file.close();
		}

		// Also write to Bolt-NativeScripts/Source/
		auto exeDir = std::filesystem::path(Path::ExecutableDir());
		auto nativeSourceDir = exeDir / ".." / ".." / ".." / "Bolt-NativeScripts" / "Source";
		if (std::filesystem::exists(nativeSourceDir))
		{
			auto projectScriptPath = nativeSourceDir / (className + ext);
			if (!std::filesystem::exists(projectScriptPath))
			{
				std::ofstream projectFile(projectScriptPath);
				if (projectFile.is_open()) {
					projectFile << boilerplate;
					projectFile.close();
				}
			}
		}

		m_NeedsRefresh = true;
		Refresh();

		m_SelectedPath = scriptPath;
		std::string name = std::filesystem::path(scriptPath).filename().string();
		BeginRename(scriptPath, name);
	}

	void AssetBrowser::CreateScene(const std::string& parentDir) {
		std::string baseName = "NewScene";
		std::string ext = ".scene";
		std::string scenePath = (std::filesystem::path(parentDir) / (baseName + ext)).string();
		int counter = 1;
		while (std::filesystem::exists(scenePath)) {
			scenePath = (std::filesystem::path(parentDir) / (baseName + std::to_string(counter) + ext)).string();
			counter++;
		}

		std::string sceneName = std::filesystem::path(scenePath).stem().string();

		// Default scene with a Camera entity
		std::string content =
			"{\n"
			"  \"name\": \"" + sceneName + "\",\n"
			"  \"entities\": [\n"
			"    {\n"
			"      \"name\": \"Camera\",\n"
			"      \"Transform2D\": { \"posX\": 0, \"posY\": 0, \"rotation\": 0, \"scaleX\": 1, \"scaleY\": 1 },\n"
			"      \"Camera2D\": { \"orthoSize\": 5, \"zoom\": 1 }\n"
			"    }\n"
			"  ]\n"
			"}\n";

		std::ofstream file(scenePath);
		if (file.is_open()) {
			file << content;
			file.close();
		}

		m_NeedsRefresh = true;
		Refresh();

		m_SelectedPath = scenePath;
		std::string name = std::filesystem::path(scenePath).filename().string();
		BeginRename(scenePath, name);
	}

	void AssetBrowser::OpenAssetExternal(const DirectoryEntry& entry) {
#ifdef BT_PLATFORM_WINDOWS
		try
		{
			std::string ext = std::filesystem::path(entry.Path).extension().string();
			std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

			// Scene files are loaded internally, not opened externally
			if (ext == ".scene")
			{
				m_PendingSceneLoad = entry.Path;
				return;
			}

			// Determine which project this file belongs to
			std::wstring slnPath, filePath, windowTitle;
			bool isScript = false;

			auto exeDir = std::filesystem::path(Path::ExecutableDir());

			if (ext == ".cs")
			{
				auto sln = exeDir / ".." / ".." / ".." / "Bolt-Sandbox" / "Bolt-Sandbox.sln";
				auto src = exeDir / ".." / ".." / ".." / "Bolt-Sandbox" / "Source"
					/ std::filesystem::path(entry.Path).filename();

				if (std::filesystem::exists(sln))
				{
					slnPath = std::filesystem::canonical(sln).wstring();
					filePath = std::filesystem::exists(src)
						? std::filesystem::canonical(src).wstring()
						: std::filesystem::absolute(entry.Path).wstring();
					windowTitle = L"Bolt-Sandbox";
					isScript = true;
				}
			}
			else if (ext == ".cpp" || ext == ".h" || ext == ".hpp")
			{
				auto sln = exeDir / ".." / ".." / ".." / "Bolt-NativeScripts" / "build" / "Bolt-NativeScripts.sln";
				auto src = exeDir / ".." / ".." / ".." / "Bolt-NativeScripts" / "Source"
					/ std::filesystem::path(entry.Path).filename();

				if (std::filesystem::exists(sln))
				{
					slnPath = std::filesystem::canonical(sln).wstring();
					filePath = std::filesystem::exists(src)
						? std::filesystem::canonical(src).wstring()
						: std::filesystem::absolute(entry.Path).wstring();
					windowTitle = L"Bolt-NativeScripts";
					isScript = true;
				}
			}

			if (isScript && !slnPath.empty())
			{
				// Check if VS already has this solution open
				bool vsRunning = false;
				std::wstring searchTitle = windowTitle;
				auto enumCtx = std::make_pair(&vsRunning, &searchTitle);
				EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
					auto* ctx = reinterpret_cast<std::pair<bool*, std::wstring*>*>(lParam);
					wchar_t title[512]{};
					GetWindowTextW(hwnd, title, 512);
					if (wcsstr(title, ctx->second->c_str()) != nullptr)
					{
						*ctx->first = true;
						return FALSE;
					}
					return TRUE;
				}, reinterpret_cast<LPARAM>(&enumCtx));

				std::wstring cmdLine;
				const wchar_t* devenv = L"C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\Common7\\IDE\\devenv.exe";

				if (vsRunning)
					cmdLine = L"\"" + std::wstring(devenv) + L"\" /Edit \"" + filePath + L"\"";
				else
					cmdLine = L"\"" + std::wstring(devenv) + L"\" \"" + slnPath
						+ L"\" /command \"File.OpenFile " + filePath + L"\"";

				std::thread([cmdLine]() {
					std::vector<wchar_t> buf(cmdLine.begin(), cmdLine.end());
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
				}).detach();
			}
			else
			{
				std::wstring wpath = std::filesystem::absolute(entry.Path).wstring();
				std::thread([wpath]() {
					CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
					ShellExecuteW(nullptr, L"open", wpath.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
					CoUninitialize();
				}).detach();
			}
		}
		catch (...) {}
#endif
	}

}
