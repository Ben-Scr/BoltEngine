#pragma once
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

#include "Scene/SceneDefinition.hpp"
#include "Scene/ComponentRegistry.hpp"

namespace Bolt {
	class PhysicsSystem2D;
}

namespace Bolt {
	class Scene;

	class BOLT_API SceneManager {
		friend class Window;

	public:
		template<typename T>
		void RegisterComponentType(ComponentInfo componentInfo) {
			s_ComponentRegistry.Register<T>(componentInfo);
		}
		ComponentRegistry& GetComponentRegistry() {
			return s_ComponentRegistry;
		}
		SceneDefinition& RegisterScene(const std::string& name);
		std::weak_ptr<Scene> LoadScene(const std::string& name);

		std::weak_ptr<Scene> LoadSceneAdditive(const std::string& name);
		std::weak_ptr<Scene> ReloadScene(const std::string name);

		void UnloadScene(const std::string& name);
		void UnloadAllScenes(bool includePersistent = false);

		std::vector< std::weak_ptr<Scene>> GetLoadedScenes();
		std::weak_ptr<Scene> GetLoadedScene(const std::string& name);
		Scene* GetActiveScene();

		void SetActiveScene(const std::string& name);

		bool HasSceneDefinition(const std::string& name);
		bool IsSceneLoaded(const std::string& name);

		bool IsInitialized() { return s_IsInitialized; }

		std::vector<std::string> GetRegisteredSceneNames();
		std::vector<std::string> GetLoadedSceneNames();

		void ForeachLoadedScene(const std::function<void(const Scene&)>& func) {
			for (const std::weak_ptr<Scene>& scenePointer : s_LoadedScenes) {
				if (auto scene = scenePointer.lock())
					func(*scene);
			}
		}

	private:
		void Initialize();
		void RegisterCoreComponents();
		void Shutdown();

		void UpdateScenes();
		void OnGuiScenes();
		void FixedUpdateScenes();
		void InitializeStartupScenes();

		std::unordered_map<std::string, std::unique_ptr<SceneDefinition>> s_SceneDefinitions;
		std::vector<std::shared_ptr<Scene>> s_LoadedScenes;
	    ComponentRegistry s_ComponentRegistry;
		Scene* s_ActiveScene;
		bool s_IsInitialized;

		friend class Application;
		friend class PhysicsSystem2D;
	};

//	// Note: Makro festlegen
//#define REGISTER_COMPONENT(Type, componentInfo) \
//    do { \
//        SceneManager::RegisterComponentType<Type>(componentInfo); \
//    } while (0)
}