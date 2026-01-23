#pragma once
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include "../Scene/SceneDefinition.hpp"

namespace Bolt {
	class PhysicsSystem2D;
}

namespace Bolt {
	class Scene;

	class SceneManager {
	public:
		static SceneDefinition& RegisterScene(const std::string& name);
		static std::weak_ptr<Scene> LoadScene(const std::string& name);

		static std::weak_ptr<Scene> LoadSceneAdditive(const std::string& name);
		static std::weak_ptr<Scene> ReloadScene(const std::string name);

		static void UnloadScene(const std::string& name);
		static void UnloadAllScenes(bool includePersistent = false);

		static std::vector< std::weak_ptr<Scene>> GetLoadedScenes();
		static std::weak_ptr<Scene> GetLoadedScene(const std::string& name);
		static Scene* GetActiveScene();

		static void SetActiveScene(const std::string& name);

		static bool HasSceneDefinition(const std::string& name);
		static bool IsSceneLoaded(const std::string& name);


		static std::vector<std::string> GetRegisteredSceneNames();
		static std::vector<std::string> GetLoadedSceneNames();

		static void ForeachLoadedScene(const std::function<void(const Scene&)>& func) {
			for (const std::weak_ptr<Scene>& scenePointer : GetLoadedScenes()) {
				if (auto scene = scenePointer.lock())
					func(*scene);
			}
		}

	private:
		static void Initialize();
		static void Shutdown();

		static void UpdateScenes();
		static void OnApplicationPaused();
		static void FixedUpdateScenes();
		static void InitializeStartupScenes();

		static std::unordered_map<std::string, std::unique_ptr<SceneDefinition>> s_SceneDefinitions;
		static std::vector<std::shared_ptr<Scene>> s_LoadedScenes;
		static Scene* s_ActiveScene;
		static bool s_IsInitialized;

		friend class Application;
		friend class PhysicsSystem2D;
	};
}