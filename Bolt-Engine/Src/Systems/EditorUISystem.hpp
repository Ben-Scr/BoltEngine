#pragma once
#include "Components/Components.hpp"
#include "Core/Application.hpp"
#include "Core/Time.hpp"
#include "Graphics/Renderer2D.hpp"
#include "Scene/EntityHelper.hpp"
#include "Scene/ISystem.hpp"
#include "Scene/Scene.hpp"
#include "Scene/SceneDefinition.hpp"
#include "Scene/SceneManager.hpp"


namespace Bolt {
	class BOLT_API EditorUISystem : public ISystem {
	public:
		virtual void OnGui(Scene& scene);

	private:
		EntityHandle m_SelectedEntity = entt::null;
		std::string m_ProjectFilePath;
		std::string m_ProjectName = "UntitledProject";
		std::string m_LastSaveStatus;
		bool m_LastSaveSucceeded = true;
		bool m_ShowStats = true;
		int m_SelectedAddComponentIndex = 0;

		void DrawDockspace();
		void DrawMenuBar(Scene& scene);
		void DrawToolbar(Scene& scene);
		void DrawHierarchy(Scene& scene);
		void DrawInspector(Scene& scene);
		void DrawGameView(Scene& scene);
		void DrawStats();
		void DrawProjectLoader(Scene& scene);
		void LoadProject(const std::filesystem::path& projectPath, Scene& scene);
		void RefreshProjectEntries();

		void CreateEntity(Scene& scene);


		void DrawComponentManagement(Entity entity);

		bool SaveProject(const Scene& scene, const std::string& path, std::string& outError) const;
		void SaveProjectFromGui(const Scene& scene, const std::string& path);
		std::string GetDefaultProjectSavePath(const Scene& scene) const;

		bool m_OpenProjectDialog = false;
		std::filesystem::path m_ProjectRootPath;
		std::filesystem::path m_SelectedProjectPath;
		std::filesystem::path m_LoadedProjectPath;
		std::vector<std::filesystem::path> m_ProjectEntries;
	};
}
