#include "../pch.hpp"
#include "../Scene/SceneManager.hpp"
#include "../Scene/Scene.hpp"

#include <algorithm>
#include <exception>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include "../Debugging/Logger.hpp"
#include "SceneDefinition.hpp"

namespace Bolt {
	std::unordered_map<std::string, std::unique_ptr<SceneDefinition>> SceneManager::s_SceneDefinitions;
	std::vector<std::shared_ptr<Scene>> SceneManager::s_LoadedScenes;
	Scene* SceneManager::s_ActiveScene;

	void SceneManager::Initialize() {
		auto& firstPair = *s_SceneDefinitions.begin();
		std::string firstSceneName = firstPair.first;
		LoadScene(firstSceneName);

		Logger::Message("SceneManager", "Loaded Scene '" + firstSceneName + "'");
	}

	void SceneManager::Shutdown() {
		SceneManager::UnloadAllScenes();
	}

	SceneDefinition& SceneManager::RegisterScene(const std::string& name) {
		auto it = s_SceneDefinitions.find(name);

		BOLT_LOG_ERROR_IF(it != s_SceneDefinitions.end(), BoltErrorCode::InvalidArgument, "Scene definition with name '" + name +
			"' already exists. (Returning existing definition for modification)");


		auto definition = std::make_unique<SceneDefinition>(name);
		SceneDefinition& ref = *definition;

		s_SceneDefinitions[name] = std::move(definition);
		return ref;
	}

	std::weak_ptr<Scene> SceneManager::LoadScene(const std::string& name) {
		auto defIt = s_SceneDefinitions.find(name);

		BOLT_RETURN_VAL_IF(defIt == s_SceneDefinitions.end(), std::weak_ptr<Scene>{}, BoltErrorCode::InvalidArgument, "Scene definition '" + name +
			"' not found. Call SceneManager::RegisterScene() first.");

		auto loadedIt = std::find_if(s_LoadedScenes.begin(), s_LoadedScenes.end(),
			[&name](const std::shared_ptr<Scene>& scene) {
				return scene->GetName() == name;
			});

		BOLT_RETURN_VAL_IF(loadedIt != s_LoadedScenes.end(), std::weak_ptr<Scene>{}, BoltErrorCode::InvalidArgument, "Scene '" + name +
			"' is already loaded. Use LoadSceneAdditive() for multiple instances.");

		UnloadAllScenes(false);

		SceneDefinition* definition = defIt->second.get();
		std::shared_ptr newScene = definition->Instantiate();

		for (const auto& callback : definition->m_LoadCallbacks) {
			callback(*newScene);
		}

		newScene->m_IsLoaded = true;

		s_ActiveScene = &*newScene;
		s_LoadedScenes.push_back(std::move(newScene));

		if (s_ActiveScene == nullptr)
			throw "Scene is null";

		return std::weak_ptr<Scene>(s_LoadedScenes.back());
	}

	std::weak_ptr<Scene> SceneManager::LoadSceneAdditive(const std::string& name) {
		auto defIt = s_SceneDefinitions.find(name);

		BOLT_RETURN_VAL_IF(defIt == s_SceneDefinitions.end(), std::weak_ptr<Scene>{}, BoltErrorCode::InvalidArgument, "Scene definition '" + name +
			"' not found. Call SceneManager::RegisterScene() first.");

		SceneDefinition* definition = defIt->second.get();
		std::shared_ptr newScene = definition->Instantiate();

		for (const auto& callback : definition->m_LoadCallbacks) {
			callback(*newScene);
		}

		newScene->m_IsLoaded = true;

		if (s_ActiveScene == nullptr) {
			s_ActiveScene = newScene.get();
		}

		s_LoadedScenes.push_back(std::move(newScene));
		return std::weak_ptr(s_LoadedScenes.back());
	}

	std::weak_ptr<Scene> SceneManager::ReloadScene(const std::string name) {
		std::shared_ptr<Scene> scene = GetLoadedScene(name).lock();

		bool wasActive = s_ActiveScene == scene.get();

		UnloadScene(name);

		if (wasActive) {
			return LoadScene(name);
		}
		else {
			return LoadSceneAdditive(name);
		}
	}

	void SceneManager::UnloadScene(const std::string& name) {
		auto it = std::find_if(s_LoadedScenes.begin(), s_LoadedScenes.end(),
			[&name](const std::shared_ptr<Scene>& scene) {
				return scene->GetName() == name;
			});


		BOLT_RETURN_IF(it == s_LoadedScenes.end(), BoltErrorCode::InvalidArgument, "Scene " + StringHelper::WrapWith(name, '\'') + " is not loaded.");

		Scene* scene = it->get();

		if (scene->m_Definition) {
			for (const auto& callback : scene->m_Definition->m_UnloadCallbacks) {
				callback(*scene);
			}
		}

		scene->m_IsLoaded = false;


		scene->DestroyScene();
		scene->m_Registry.clear();

		if (s_ActiveScene == scene) {
			s_ActiveScene = nullptr;

			for (const auto& loadedScene : s_LoadedScenes) {
				if (loadedScene.get() != scene && loadedScene->IsLoaded()) {
					s_ActiveScene = loadedScene.get();
					break;
				}
			}
		}

		s_LoadedScenes.erase(it);
	}

	void SceneManager::UnloadAllScenes(bool includePersistent) {
		ForeachLoadedScene([](const Scene& scene) {
			UnloadScene(scene.GetName());
			});
	}

	std::vector<std::weak_ptr<Scene>> SceneManager::GetLoadedScenes() {
		std::vector<std::weak_ptr<Scene>> loadedScenes;
		loadedScenes.reserve(s_LoadedScenes.size());

		for (const std::shared_ptr<Scene>& scene : s_LoadedScenes) {
			loadedScenes.emplace_back(scene);
		}

		return loadedScenes;
	}

	std::weak_ptr<Scene> SceneManager::GetLoadedScene(const std::string& name) {
		auto it = std::find_if(s_LoadedScenes.begin(), s_LoadedScenes.end(),
			[&name](const std::shared_ptr<Scene>& scene) {
				return scene->GetName() == name;
			});

		BOLT_RETURN_VAL_IF(it == s_LoadedScenes.end(), std::weak_ptr<Scene>{}, BoltErrorCode::NullReference, "Scene '" + name + "' is not loaded.");
		return std::weak_ptr(*it);
	}

	Scene* SceneManager::GetActiveScene() {
		BOLT_RETURN_VAL_IF(!s_ActiveScene, nullptr, BoltErrorCode::NullReference, "There is no active scene");
		return s_ActiveScene;
	}

	void SceneManager::SetActiveScene(const std::string& name) {
		auto it = std::find_if(s_LoadedScenes.begin(), s_LoadedScenes.end(),
			[&name](const std::shared_ptr<Scene>& scene) {
				return scene->GetName() == name;
			});

		BOLT_RETURN_IF(it == s_LoadedScenes.end(), BoltErrorCode::InvalidArgument, StringHelper::ToString(
			"Scene ", StringHelper::WrapWith(name, '\'')
			, "is not loaded. Load it first before setting as active."));

		s_ActiveScene = it._Ptr->get();
	}

	bool SceneManager::HasSceneDefinition(const std::string& name) {
		return s_SceneDefinitions.find(name) != s_SceneDefinitions.end();
	}

	bool SceneManager::IsSceneLoaded(const std::string& name) {
		return std::any_of(s_LoadedScenes.begin(), s_LoadedScenes.end(),
			[&name](const std::shared_ptr<Scene>& scene) {
				return scene->GetName() == name;
			});
	}

	std::vector<std::string> SceneManager::GetRegisteredSceneNames() {
		std::vector<std::string> names;
		names.reserve(s_SceneDefinitions.size());

		for (const auto& [name, def] : s_SceneDefinitions) {
			names.push_back(name);
		}

		return names;
	}

	std::vector<std::string> SceneManager::GetLoadedSceneNames() {
		std::vector<std::string> names;
		names.reserve(s_LoadedScenes.size());

		for (const auto& scene : s_LoadedScenes) {
			names.push_back(scene->GetName());
		}

		return names;
	}

	void SceneManager::UpdateScenes() {
		for (auto& scene : s_LoadedScenes) {
			if (scene->IsLoaded()) {
				scene->UpdateSystems();
			}
		}
	}

	void SceneManager::OnApplicationPaused() {
		for (auto& scene : s_LoadedScenes) {
			if (scene->IsLoaded()) {
				scene->OnApplicationPausedSystems();
			}
		}
	}

	void SceneManager::FixedUpdateScenes() {
		for (auto& scene : s_LoadedScenes) {
			if (scene->IsLoaded()) {
				scene->FixedUpdateSystems();
			}
		}
	}

	void SceneManager::InitializeStartupScenes() {
		for (const auto& [name, definition] : s_SceneDefinitions) {
			if (definition->IsStartupScene()) {
				try {
					if (s_ActiveScene == nullptr) {
						LoadScene(name);
					}
					else {
						LoadSceneAdditive(name);
					}
				}
				catch (const std::exception& e) {
					Logger::Error("Failed to load startup scene with name '" +
						name + "': " + e.what());
				}
				catch (...) {
					Logger::Error("Failed to load startup scene with name '" +
						name + "'");
				}
			}
		}
	}
}