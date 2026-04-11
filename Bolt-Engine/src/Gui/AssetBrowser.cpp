#include <pch.hpp>
#include "Gui/AssetBrowser.hpp"
#include "Serialization/Path.hpp"
#include "Project/ProjectManager.hpp"
#include "Serialization/SceneSerializer.hpp"
#include "Scene/SceneManager.hpp"
#include "Scene/Scene.hpp"
#include "Components/General/NameComponent.hpp"
#include "Core/Log.hpp"
#include "Editor/ExternalEditor.hpp"
#include <imgui.h>
#include <algorithm>
#include "Gui/EditorIcons.hpp"
#include <filesystem>
#include <fstream>

#include <thread>

#ifdef BT_PLATFORM_WINDOWS
#include <windows.h>
#include <shellapi.h>
#endif

namespace Bolt {
	static const char* GetFileTypeIconName(const std::string& extension) {
		std::string ext = extension;
		std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

		if (ext == ".cs")                                                    return "asset_cs";
		if (ext == ".cpp" || ext == ".c" || ext == ".h" || ext == ".hpp")    return "asset_cpp";
		if (ext == ".scene" || ext == ".bolt")                               return "asset_scene";
		if (ext == ".prefab")                                                return "asset_scene";
		if (ext == ".txt" || ext == ".cfg" || ext == ".ini" ||
			ext == ".yaml" || ext == ".toml" || ext == ".xml" ||
			ext == ".json" || ext == ".lua")                                 return "asset_txt";
		if (ext == ".wav" || ext == ".mp3" || ext == ".ogg" || ext == ".flac") return "asset_audio";

		return nullptr;
	}

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

		// If this is a pending script creation, write the boilerplate now with the final name
		if (m_PendingScriptType != PendingScriptType::None) {
			std::string className = newName; // User typed the class name (no extension)
			std::string ext = (m_PendingScriptType == PendingScriptType::CSharp) ? ".cs" : ".cpp";

			// Ensure the name has the right extension
			if (!className.empty() && className.find('.') == std::string::npos) {
				// User typed bare name without extension — add it
			}
			else if (className.size() > ext.size()) {
				// User might have typed with extension — strip it for the class name
				std::string lowerName = className;
				std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
				if (lowerName.substr(lowerName.size() - ext.size()) == ext)
					className = className.substr(0, className.size() - ext.size());
			}

			if (className.empty()) className = "NewScript";

			std::string finalFileName = className + ext;
			std::string finalPath = (std::filesystem::path(m_PendingScriptDir) / finalFileName).string();

			// Delete the placeholder file
			if (std::filesystem::exists(m_RenamePath)) {
				std::error_code ec;
				std::filesystem::remove(m_RenamePath, ec);
			}

			// Write the real boilerplate with the correct class name
			if (m_PendingScriptType == PendingScriptType::CSharp) {
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

				std::ofstream file(finalPath);
				if (file.is_open()) { file << boilerplate; file.close(); }

				// Legacy sandbox copy
				BoltProject* project = ProjectManager::GetCurrentProject();
				if (!project) {
					auto sandboxDir = std::filesystem::path(Path::ExecutableDir()) / ".." / ".." / ".." / "Bolt-Sandbox" / "Source";
					if (std::filesystem::exists(sandboxDir)) {
						auto dst = sandboxDir / finalFileName;
						if (!std::filesystem::exists(dst)) {
							std::ofstream f(dst); if (f.is_open()) { f << boilerplate; f.close(); }
						}
					}
				}
			}
			else { // Native
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

				std::ofstream file(finalPath);
				if (file.is_open()) { file << boilerplate; file.close(); }

				// Copy to NativeScripts/Source/
				BoltProject* project = ProjectManager::GetCurrentProject();
				std::string nativeDir = project ? project->NativeSourceDir
					: (std::filesystem::path(Path::ExecutableDir()) / ".." / ".." / ".." / "Bolt-NativeScripts" / "Source").string();
				if (std::filesystem::exists(nativeDir)) {
					auto dst = std::filesystem::path(nativeDir) / finalFileName;
					if (!std::filesystem::exists(dst)) {
						std::ofstream f(dst); if (f.is_open()) { f << boilerplate; f.close(); }
					}
				}
			}

			m_SelectedPath = finalPath;
			m_PendingScriptType = PendingScriptType::None;
			m_PendingScriptDir.clear();
			m_NeedsRefresh = true;
			CancelRename();
			return;
		}

		// Normal rename (not a script creation)
		if (!newName.empty() && newName != oldName) {
			RenameEntry(m_RenamePath, newName);
		}
		CancelRename();
	}

	void AssetBrowser::CancelRename() {
		// If cancelling a pending script creation, delete the placeholder
		if (m_PendingScriptType != PendingScriptType::None) {
			if (!m_RenamePath.empty() && std::filesystem::exists(m_RenamePath)) {
				std::error_code ec;
				std::filesystem::remove(m_RenamePath, ec);
			}
			m_PendingScriptType = PendingScriptType::None;
			m_PendingScriptDir.clear();
			m_NeedsRefresh = true;
		}

		m_IsRenaming = false;
		m_RenamePath.clear();
		m_RenameFrameCounter = 0;
	}

	bool AssetBrowser::IsRenamingEntry(const std::string& path) const {
		return m_IsRenaming && m_RenamePath == path;
	}

	void AssetBrowser::Render() {
		// Process pending OS file drops
		if (!m_PendingExternalDrops.empty()) {
			OnExternalFileDrop(m_PendingExternalDrops);
			m_PendingExternalDrops.clear();
		}

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
		{
			unsigned int refreshIcon = EditorIcons::Get("redo", 16);
			bool clicked = false;
			if (refreshIcon) {
				clicked = ImGui::ImageButton("##Refresh",
					static_cast<ImTextureID>(static_cast<intptr_t>(refreshIcon)),
					ImVec2(14, 14), ImVec2(0, 1), ImVec2(1, 0));
			} else {
				clicked = ImGui::SmallButton("R");
			}
			if (clicked) m_NeedsRefresh = true;
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Refresh");
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

		// Accept entity drops from hierarchy panel to save as prefab
		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHY_ENTITY")) {
				struct HierarchyDragData { int Index; uint32_t EntityHandle; };
				auto* dragData = static_cast<const HierarchyDragData*>(payload->Data);
				entt::entity entityHandle = static_cast<entt::entity>(dragData->EntityHandle);

				Scene* activeScene = SceneManager::Get().GetActiveScene();
				if (activeScene && activeScene->IsValid(entityHandle)) {
					std::string entityName = "Entity";
					if (activeScene->HasComponent<NameComponent>(entityHandle))
						entityName = activeScene->GetComponent<NameComponent>(entityHandle).Name;
					std::string prefabPath = (std::filesystem::path(m_CurrentDirectory) / (entityName + ".prefab")).string();
					SceneSerializer::SaveEntityToFile(*activeScene, entityHandle, prefabPath);
					m_NeedsRefresh = true;
				}
			}
			ImGui::EndDragDropTarget();
		}

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

			bool drewTexture = false;
			const char* iconName = nullptr;

			if (type == AssetType::Folder) {
				iconName = "open_folder";
			}
			else if (!entry.IsDirectory) {
				iconName = GetFileTypeIconName(std::filesystem::path(entry.Path).extension().string());
				if (!iconName) iconName = "asset_bin";
			}

			if (iconName) {
				unsigned int icon = EditorIcons::Get(iconName, (int)m_TileSize);
				if (icon) {
					const float pad = m_TileSize * 0.1f;
					const float drawSize = m_TileSize - pad * 2.0f;
					ImGui::SetCursorScreenPos(ImVec2(iconPos.x + pad, iconPos.y + pad));
					ImGui::Image(
						static_cast<ImTextureID>(static_cast<intptr_t>(icon)),
						ImVec2(drawSize, drawSize),
						ImVec2(0, 1), ImVec2(1, 0)
					);
					ImGui::SetCursorScreenPos(ImVec2(iconPos.x, iconPos.y + m_TileSize));
					drewTexture = true;
				}
			}

			if (!drewTexture) {
				ThumbnailCache::DrawAssetIcon(type, iconPos, m_TileSize);
				ImGui::Dummy(ImVec2(m_TileSize, m_TileSize));
			}
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
			BoltProject* project = ProjectManager::GetCurrentProject();
			if (ext == ".cs")
			{
				if (project)
					projectSourceDir = project->ScriptsDirectory;
				else
					projectSourceDir = exeDir / ".." / ".." / ".." / "Bolt-Sandbox" / "Source";
			}
			else if (ext == ".cpp" || ext == ".h" || ext == ".hpp")
			{
				if (project)
					projectSourceDir = project->NativeSourceDir;
				else
					projectSourceDir = exeDir / ".." / ".." / ".." / "Bolt-NativeScripts" / "Source";
			}

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
		// Don't write the file yet — just create a placeholder entry for renaming.
		// The actual .cs file with boilerplate is written in CommitRename() after
		// the user confirms the name. This prevents the FileWatcher from triggering
		// a recompile with "NewScript" as the class name.

		std::string baseName = "NewScript";
		std::string ext = ".cs";
		std::string scriptPath = (std::filesystem::path(parentDir) / (baseName + ext)).string();
		int counter = 1;
		while (std::filesystem::exists(scriptPath)) {
			scriptPath = (std::filesystem::path(parentDir) / (baseName + std::to_string(counter) + ext)).string();
			counter++;
		}

		// Write a minimal empty file so it shows up in the browser
		{
			std::ofstream placeholder(scriptPath);
			placeholder << "// Rename this script to generate boilerplate\n";
		}

		m_PendingScriptType = PendingScriptType::CSharp;
		m_PendingScriptDir = parentDir;

		m_NeedsRefresh = true;
		Refresh();

		m_SelectedPath = scriptPath;
		std::string name = std::filesystem::path(scriptPath).stem().string();
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

		// Write a minimal placeholder so it shows up in the browser
		{
			std::ofstream placeholder(scriptPath);
			placeholder << "// Rename this script to generate boilerplate\n";
		}

		m_PendingScriptType = PendingScriptType::Native;
		m_PendingScriptDir = parentDir;

		m_NeedsRefresh = true;
		Refresh();

		m_SelectedPath = scriptPath;
		std::string name = std::filesystem::path(scriptPath).stem().string();
		BeginRename(scriptPath, name);
	}

	void AssetBrowser::CreateScene(const std::string& parentDir) {
		std::string baseName = "NewScene";
		std::string ext = ".scene";

		// Check for duplicate scene names across the entire assets tree
		auto sceneNameTaken = [&](const std::string& name) -> bool {
			try {
				for (auto& entry : std::filesystem::recursive_directory_iterator(
					m_RootDirectory, std::filesystem::directory_options::skip_permission_denied)) {
					if (entry.is_regular_file() && entry.path().extension() == ext
						&& entry.path().stem().string() == name)
						return true;
				}
			} catch (...) {}
			return false;
		};

		std::string sceneName = baseName;
		std::string scenePath = (std::filesystem::path(parentDir) / (sceneName + ext)).string();
		int counter = 1;
		while (sceneNameTaken(sceneName)) {
			sceneName = baseName + std::to_string(counter++);
			scenePath = (std::filesystem::path(parentDir) / (sceneName + ext)).string();
		}

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

			// Script/code files: always route through the configured external editor
			if (ExternalEditor::IsScriptExtension(ext))
			{
				ExternalEditor::OpenFile(entry.Path);
				return;
			}

			// Non-script files (images, audio, etc.): open with OS default
#ifdef BT_PLATFORM_WINDOWS
			std::wstring wpath = std::filesystem::absolute(entry.Path).wstring();
			std::thread([wpath]() {
				CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
				ShellExecuteW(nullptr, L"open", wpath.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
				CoUninitialize();
			}).detach();
#endif
		}
		catch (...) {}
	}

	void AssetBrowser::OnExternalFileDrop(const std::vector<std::string>& paths) {
		if (m_CurrentDirectory.empty()) return;

		int imported = 0;
		for (const auto& sourcePath : paths) {
			try {
				std::filesystem::path src(sourcePath);
				if (!std::filesystem::exists(src)) continue;

				std::filesystem::path destDir(m_CurrentDirectory);

				if (std::filesystem::is_directory(src)) {
					// Copy entire directory
					std::filesystem::path dest = destDir / src.filename();
					std::filesystem::copy(src, dest,
						std::filesystem::copy_options::recursive | std::filesystem::copy_options::skip_existing);
					imported++;
				}
				else {
					// Copy single file
					std::filesystem::path dest = destDir / src.filename();

					// Avoid overwriting — append counter if file exists
					if (std::filesystem::exists(dest)) {
						std::string stem = dest.stem().string();
						std::string ext = dest.extension().string();
						int counter = 1;
						while (std::filesystem::exists(dest)) {
							dest = destDir / (stem + " (" + std::to_string(counter) + ")" + ext);
							counter++;
						}
					}

					std::filesystem::copy_file(src, dest);
					imported++;
				}
			}
			catch (const std::exception& e) {
				BT_CORE_WARN_TAG("AssetBrowser", "Failed to import '{}': {}", sourcePath, e.what());
			}
		}

		if (imported > 0) {
			m_NeedsRefresh = true;
			BT_CORE_INFO_TAG("AssetBrowser", "Imported {} file(s) into {}", imported, m_CurrentDirectory);
		}
	}

}
