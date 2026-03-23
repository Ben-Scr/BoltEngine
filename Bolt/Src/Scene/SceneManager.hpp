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
		SceneManager() = default;
		SceneManager(const SceneManager&) = delete;
		SceneManager& operator=(const SceneManager&) = delete;

		template<typename T>
		void RegisterComponentType(ComponentInfo componentInfo) {
			m_ComponentRegistry.Register<T>(componentInfo);
		}
		ComponentRegistry& GetComponentRegistry() {
			return m_ComponentRegistry;
		}
		const ComponentRegistry& GetComponentRegistry() const {
			return m_ComponentRegistry;
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

		bool IsInitialized() const { return m_IsInitialized; }

		std::vector<std::string> GetRegisteredSceneNames();
		std::vector<std::string> GetLoadedSceneNames();

		void ForeachLoadedScene(const std::function<void(const Scene&)>& func) const {
			for (const std::shared_ptr<Scene>& scenePointer : m_LoadedScenes) {
				if (scenePointer) {
					func(*scenePointer);
				}
			}
		}

		static SceneManager& Get();

	private:
		void Initialize();
		void RegisterCoreComponents();
		void Shutdown();

		void UpdateScenes();
		void OnGuiScenes();
		void FixedUpdateScenes();
		void InitializeStartupScenes();

		std::unordered_map<std::string, std::unique_ptr<SceneDefinition>> m_SceneDefinitions;
		std::vector<std::shared_ptr<Scene>> m_LoadedScenes;
		ComponentRegistry m_ComponentRegistry;
		Scene* m_ActiveScene = nullptr;
		bool m_IsInitialized = false;

		friend class Application;
		friend class PhysicsSystem2D;
	};

	//	// Note: Makro festlegen
	//#define REGISTER_COMPONENT(Type, componentInfo) \
	//    do { \
	//        SceneManager::RegisterComponentType<Type>(componentInfo); \
	//    } while (0)
}