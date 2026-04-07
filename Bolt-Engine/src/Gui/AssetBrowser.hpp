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
		void NavigateTo(const std::string& directory);
		void NavigateUp();
		void Refresh();

		void RenderBreadcrumb();
		void RenderGrid();
		void RenderAssetTile(const DirectoryEntry& entry, int index);

		void RenderGridContextMenu();
		void RenderItemContextMenu(const DirectoryEntry& entry);

		void HandleDragSource(const DirectoryEntry& entry);
		void HandleDropTarget(const DirectoryEntry& entry);
		void OpenAssetExternal(const DirectoryEntry& entry);

		void DeleteEntry(const std::string& path);
		void RenameEntry(const std::string& path, const std::string& newName);
		void CopyPathToClipboard(const std::string& path);
		void CreateFolder(const std::string& parentDir);
		void CreateScript(const std::string& parentDir);

		void BeginRename(const std::string& path, const std::string& currentName);
		void CommitRename();
		void CancelRename();
		bool IsRenamingEntry(const std::string& path) const;

		std::string m_RootDirectory;
		std::string m_CurrentDirectory;
		std::vector<DirectoryEntry> m_Entries;
		std::string m_SelectedPath;
		bool m_NeedsRefresh = true;

		bool m_IsRenaming = false;
		std::string m_RenamePath;
		char m_RenameBuffer[256]{};
		int m_RenameFrameCounter = 0;

		float m_TileSize = 80.0f;
		float m_TilePadding = 8.0f;

		bool m_ItemRightClicked = false;

		ThumbnailCache m_Thumbnails;
	};

}
