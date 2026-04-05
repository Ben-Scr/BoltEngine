#pragma once
#include "Gui/ThumbnailCache.hpp"
#include "Serialization/Directory.hpp"
#include <string>
#include <vector>

namespace Bolt {

	class AssetBrowser {
	public:
		void Initialize(const std::string& rootDirectory);
		void Shutdown();
		void Render();

	private:
		// Navigation
		void NavigateTo(const std::string& directory);
		void NavigateUp();
		void Refresh();

		// Rendering sub-sections
		void RenderBreadcrumb();
		void RenderGrid();
		void RenderAssetTile(const DirectoryEntry& entry, int index);

		// Context menus
		void RenderGridContextMenu();
		void RenderItemContextMenu(const DirectoryEntry& entry);

		// Drag and drop
		void HandleDragSource(const DirectoryEntry& entry);
		void HandleDropTarget(const DirectoryEntry& entry);

		// Actions
		void DeleteEntry(const std::string& path);
		void RenameEntry(const std::string& path, const std::string& newName);
		void CopyPathToClipboard(const std::string& path);
		void CreateFolder(const std::string& parentDir);

		// Rename helpers
		void BeginRename(const std::string& path, const std::string& currentName);
		void CommitRename();
		void CancelRename();
		bool IsRenamingEntry(const std::string& path) const;

		// State
		std::string m_RootDirectory;
		std::string m_CurrentDirectory;
		std::vector<DirectoryEntry> m_Entries;
		std::string m_SelectedPath;
		bool m_NeedsRefresh = true;

		// Rename state
		bool m_IsRenaming = false;
		std::string m_RenamePath;
		char m_RenameBuffer[256]{};
		int m_RenameFrameCounter = 0;

		// Layout
		float m_TileSize = 80.0f;
		float m_TilePadding = 8.0f;

		ThumbnailCache m_Thumbnails;
	};

} // namespace Bolt
