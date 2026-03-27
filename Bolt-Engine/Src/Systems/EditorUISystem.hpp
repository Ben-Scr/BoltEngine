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
		int m_EntityCounter = 0;
		int m_SelectedAddComponentIndex = 0;
		std::string m_LastSaveMessage;
		bool m_LastSaveSucceeded = false;

		void DrawDockspace();
		void DrawMenuBar(Scene& scene);
		void DrawHierarchy(Scene& scene);
		void DrawInspector(Scene& scene);
		void DrawStats();

		void DrawProjectLoader(Scene& scene);
		void LoadProject(const std::filesystem::path& projectPath, Scene& scene);
		void RefreshProjectEntries();

		void CreateEntity(Scene& scene);


		void DrawComponentManagement(Entity entity);
		void SaveCurrentScene(const Scene& scene);

		bool m_OpenProjectDialog = false;
		std::filesystem::path m_ProjectRootPath;
		std::filesystem::path m_SelectedProjectPath;
		std::filesystem::path m_LoadedProjectPath;
		std::vector<std::filesystem::path> m_ProjectEntries;
	};
}