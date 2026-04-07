#include <pch.hpp>
#include "Gui/AssetBrowser.hpp"
#include "Serialization/Path.hpp"
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
			if (ImGui::MenuItem("Create Script (C#)")) {
				CreateScript(m_CurrentDirectory);
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

		if (Directory::Rename(path, newName)) {
			std::filesystem::path p(path);
			std::string newPath = (p.parent_path() / newName).string();
			if (m_SelectedPath == path) {
				m_SelectedPath = newPath;
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
		// Start rename so the user can immediately change the script name
		std::string name = std::filesystem::path(scriptPath).filename().string();
		BeginRename(scriptPath, name);
	}

	void AssetBrowser::OpenAssetExternal(const DirectoryEntry& entry) {
		std::string entryPath = entry.Path;
		std::string exeDir = Path::ExecutableDir();

		std::thread([entryPath, exeDir]() {
			try
			{
				std::string ext = std::filesystem::path(entryPath).extension().string();
				std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

#ifdef BT_PLATFORM_WINDOWS
				if (ext == ".cs")
				{
					// Resolve the project source file
					auto sandboxSource = std::filesystem::path(exeDir) / ".." / ".." / ".." / "Bolt-Sandbox" / "Source"
						/ std::filesystem::path(entryPath).filename();
					auto sandboxSln = std::filesystem::path(exeDir) / ".." / ".." / ".." / "Bolt-Sandbox" / "Bolt-Sandbox.sln";

					std::filesystem::path fileToOpen;
					if (std::filesystem::exists(sandboxSource))
						fileToOpen = std::filesystem::canonical(sandboxSource);
					else
						fileToOpen = std::filesystem::absolute(entryPath);

					// Check if VS is already running with the Bolt-Sandbox solution
					// by searching for a window with "Bolt-Sandbox" in its title.
					struct FindData { bool found; };
					FindData data = { false };

					EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
						auto* d = reinterpret_cast<FindData*>(lParam);
						wchar_t title[512]{};
						GetWindowTextW(hwnd, title, 512);
						if (wcsstr(title, L"Bolt-Sandbox") != nullptr)
						{
							d->found = true;
							return FALSE;
						}
						return TRUE;
					}, reinterpret_cast<LPARAM>(&data));

					if (data.found)
					{
						// VS already has the solution open — just open the file.
						// VS picks it up in the existing instance.
						ShellExecuteW(nullptr, L"open", fileToOpen.wstring().c_str(),
							nullptr, nullptr, SW_SHOWNORMAL);
					}
					else
					{
						// VS not running with this solution — open the .sln.
						// This launches VS with full project context.
						if (std::filesystem::exists(sandboxSln))
						{
							auto slnPath = std::filesystem::canonical(sandboxSln);
							ShellExecuteW(nullptr, L"open", slnPath.wstring().c_str(),
								nullptr, nullptr, SW_SHOWNORMAL);
						}
						else
						{
							ShellExecuteW(nullptr, L"open", fileToOpen.wstring().c_str(),
								nullptr, nullptr, SW_SHOWNORMAL);
						}
					}
				}
				else
				{
					// Non-script files: open with default Windows handler
					auto filePath = std::filesystem::absolute(entryPath);
					ShellExecuteW(nullptr, L"open", filePath.wstring().c_str(),
						nullptr, nullptr, SW_SHOWNORMAL);
				}
#endif
			}
			catch (...) { /* silently ignore on background thread */ }
		}).detach();
	}

}
