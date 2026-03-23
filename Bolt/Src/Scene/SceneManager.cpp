#include "pch.hpp"
#include "Scene/SceneManager.hpp"
#include "Scene/Scene.hpp"

#include <algorithm>
#include <exception>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include "Debugging/Logger.hpp"
#include "SceneDefinition.hpp"
#include "Components/Components.hpp"
#include "Systems/ParticleUpdateSystem.hpp"
#include "Core/Application.hpp"

namespace Bolt {
	SceneManager& SceneManager::Get() {
		auto* app = Application::GetInstance();
		BOLT_ASSERT(app && app->GetSceneManager(), BoltErrorCode::NotInitialized, "SceneManager is not available before the Application instance exists");
		return *app->GetSceneManager();
	}

	void SceneManager::Initialize() {
		BOLT_ASSERT(!m_IsInitialized, BoltErrorCode::AlreadyInitialized, "SceneManager already is initialized");
		BOLT_ASSERT(!m_SceneDefinitions.empty(), BoltErrorCode::InvalidArgument, "No scenes were registered before SceneManager initialization");

		m_IsInitialized = true;
		RegisterCoreComponents();
		InitializeStartupScenes();

		if (m_LoadedScenes.empty()) {
			auto& firstPair = *m_SceneDefinitions.begin();
			LoadScene(firstPair.first);
			Logger::Message("SceneManager", "Loaded fallback scene '" + firstPair.first + "'");
		}
	}

	void SceneManager::RegisterCoreComponents() {
		/*REGISTER_COMPONENT(NameComponent, ComponentInfo("Name", ComponentCategory::Component));
		REGISTER_COMPONENT(Transform2DComponent, ComponentInfo("Transform2D", ComponentCategory::Component));
		REGISTER_COMPONENT(ParticleSystem2DComponent, ComponentInfo("ParticleSystem2D", ComponentCategory::Component));
		REGISTER_COMPONENT(BoxCollider2DComponent, ComponentInfo("BoxCollider2D", ComponentCategory::Component));
		REGISTER_COMPONENT(Rigidbody2DComponent, ComponentInfo("Rigidbody2D", ComponentCategory::Component));
		REGISTER_COMPONENT(Camera2DComponent, ComponentInfo("Camera2D", ComponentCategory::Component));
		REGISTER_COMPONENT(SpriteRendererComponent, ComponentInfo("SpriteRenderer", ComponentCategory::Component));*/
	}

	void SceneManager::Shutdown() {
		UnloadAllScenes(true);
		m_SceneDefinitions.clear();
		m_ActiveScene = nullptr;
		m_IsInitialized = false;
	}

	SceneDefinition& SceneManager::RegisterScene(const std::string& name) {
		auto [it, inserted] = m_SceneDefinitions.emplace(name, std::make_unique<SceneDefinition>(name));
		BOLT_ASSERT(inserted, BoltErrorCode::InvalidArgument, "Scene definition with name '" + name + "' already exists.");

		SceneDefinition& definition = *it->second;
		definition.AddSystem<ParticleUpdateSystem>();
		return definition;
	}

	std::weak_ptr<Scene> SceneManager::LoadScene(const std::string& name) {
		return LoadSceneInternal(name, false);
	}

	std::weak_ptr<Scene> SceneManager::LoadSceneAdditive(const std::string& name) {
		return LoadSceneInternal(name, true);
	}

	std::shared_ptr<Scene> SceneManager::LoadSceneInternal(const std::string& name, bool additive) {
		BOLT_ASSERT(m_IsInitialized, BoltErrorCode::NotInitialized, "SceneManager isn't initialized");
		SceneDefinition& definition = GetSceneDefinitionOrThrow(name);
		if (!additive) {
			BOLT_ASSERT(!IsSceneLoaded(name), BoltErrorCode::InvalidArgument, "Scene '" + name + "' is already loaded. Use LoadSceneAdditive() for multiple instances.");
			UnloadAllScenes(false);
		}

		std::shared_ptr<Scene> newScene = definition.Instantiate();
		for (const auto& callback : definition.m_LoadCallbacks) {
			callback(*newScene);
		}

		newScene->m_IsLoaded = true;
		if (!m_ActiveScene) {
			m_ActiveScene = newScene.get();
		}
		newScene->AwakeSystems();
		newScene->StartSystems();
		m_LoadedScenes.push_back(newScene);
		if (!additive) {
			m_ActiveScene = newScene.get();
		}
		BOLT_ASSERT(m_ActiveScene, BoltErrorCode::NullReference, "Active Scene is null");
		return newScene;
	}

	std::weak_ptr<Scene> SceneManager::ReloadScene(const std::string& name) {
		std::shared_ptr<Scene> scene = GetLoadedScene(name).lock();
		const bool wasActive = m_ActiveScene == scene.get();
		UnloadScene(name);
		return wasActive ? LoadScene(name) : LoadSceneAdditive(name);
	}

	void SceneManager::UnloadScene(const std::string& name) {
		auto it = FindLoadedSceneIterator(name);
		BOLT_ASSERT(it != m_LoadedScenes.end(), BoltErrorCode::InvalidArgument, "Scene '" + name + "' is not loaded.");

		Scene& scene = *(*it);
		if (scene.m_Persistent) {
			return;
		}

		ReleaseScene(it);
	}

	void SceneManager::ReleaseScene(LoadedSceneList::iterator it) {
		Scene& scene = *(*it);
		if (scene.m_Definition) {
			for (const auto& callback : scene.m_Definition->m_UnloadCallbacks) {
				callback(scene);
			}
		}
		scene.m_IsLoaded = false;
		scene.DestroyScene();
		scene.m_Registry.clear();

		if (m_ActiveScene == &scene) {
			m_ActiveScene = nullptr;
		}
		m_LoadedScenes.erase(it);
		RefreshActiveScene();
	}

	void SceneManager::UnloadAllScenes(bool includePersistent) {
		for (auto it = m_LoadedScenes.begin(); it != m_LoadedScenes.end();) {
			if (!includePersistent && (*it)->IsPersistent()) {
				++it;
				continue;
			}

			ReleaseScene(it);
			it = m_LoadedScenes.begin();
		}
	}

	std::vector<std::weak_ptr<Scene>> SceneManager::GetLoadedScenes() {
		std::vector<std::weak_ptr<Scene>> loadedScenes;
		loadedScenes.reserve(m_LoadedScenes.size());
		for (const std::shared_ptr<Scene>& scene : m_LoadedScenes) {
			loadedScenes.emplace_back(scene);
		}
		return loadedScenes;
	}

	std::weak_ptr<Scene> SceneManager::GetLoadedScene(const std::string& name) {
		auto it = FindLoadedSceneIterator(name);
		BOLT_ASSERT(it != m_LoadedScenes.end(), BoltErrorCode::NullReference, "Scene '" + name + "' is not loaded.");
		return std::weak_ptr(*it);
	}

	Scene* SceneManager::GetActiveScene() {
		BOLT_ASSERT(m_ActiveScene, BoltErrorCode::NullReference, "There is no active scene");
		return m_ActiveScene;
	}

	const Scene* SceneManager::GetActiveScene() const {
		BOLT_ASSERT(m_ActiveScene, BoltErrorCode::NullReference, "There is no active scene");
		return m_ActiveScene;
	}

	void SceneManager::SetActiveScene(const std::string& name) {
		auto it = FindLoadedSceneIterator(name);
		BOLT_ASSERT(it != m_LoadedScenes.end(), BoltErrorCode::InvalidArgument, "Scene '" + name + "' is not loaded. Load it first before setting as active.");
		m_ActiveScene = it->get();
	}

	bool SceneManager::HasSceneDefinition(const std::string& name) const {
		return m_SceneDefinitions.find(name) != m_SceneDefinitions.end();
	}

	bool SceneManager::IsSceneLoaded(const std::string& name) const {
		return FindLoadedSceneIterator(name) != m_LoadedScenes.end();
	}

	std::vector<std::string> SceneManager::GetRegisteredSceneNames() const {
		std::vector<std::string> names;
		names.reserve(m_SceneDefinitions.size());
		for (const auto& [name, definition] : m_SceneDefinitions) {
			names.push_back(name);
		}
		return names;
	}

	std::vector<std::string> SceneManager::GetLoadedSceneNames() const {
		std::vector<std::string> names;
		names.reserve(m_LoadedScenes.size());
		for (const auto& scene : m_LoadedScenes) {
			names.push_back(scene->GetName());
		}
		return names;
	}

	void SceneManager::UpdateScenes() {
		for (auto& scene : m_LoadedScenes) if (scene->IsLoaded()) scene->UpdateSystems();
	}

	void SceneManager::OnGuiScenes() {
		for (auto& scene : m_LoadedScenes) if (scene->IsLoaded()) scene->OnGuiSystems();
	}

	void SceneManager::FixedUpdateScenes() {
		for (auto& scene : m_LoadedScenes) if (scene->IsLoaded()) scene->FixedUpdateSystems();
	}

	void SceneManager::InitializeStartupScenes() {
		for (const auto& [name, definition] : m_SceneDefinitions) {
			if (!definition->IsStartupScene()) {
				continue;
			}
			try {
				if (m_ActiveScene == nullptr) {
					LoadScene(name);
				}
				else {
					LoadSceneAdditive(name);
				}
			}
			catch (const std::exception& e) {
				Logger::Error("Failed to load startup scene with name '" + name + "': " + e.what());
			}
			catch (...) {
				Logger::Error("Failed to load startup scene with name '" + name + "'");
			}
		}
	}

	SceneDefinition& SceneManager::GetSceneDefinitionOrThrow(const std::string& name) {
		auto it = m_SceneDefinitions.find(name);
		BOLT_ASSERT(it != m_SceneDefinitions.end(), BoltErrorCode::InvalidArgument, "Scene definition '" + name + "' not found. Call SceneManager::RegisterScene() first.");
		return *it->second;
	}

	const SceneDefinition& SceneManager::GetSceneDefinitionOrThrow(const std::string& name) const {
		auto it = m_SceneDefinitions.find(name);
		BOLT_ASSERT(it != m_SceneDefinitions.end(), BoltErrorCode::InvalidArgument, "Scene definition '" + name + "' not found. Call SceneManager::RegisterScene() first.");
		return *it->second;
	}

	SceneManager::LoadedSceneList::iterator SceneManager::FindLoadedSceneIterator(const std::string& name) {
		return std::find_if(m_LoadedScenes.begin(), m_LoadedScenes.end(), [&name](const std::shared_ptr<Scene>& scene) {
			return scene && scene->GetName() == name;
			});
	}

	SceneManager::LoadedSceneList::const_iterator SceneManager::FindLoadedSceneIterator(const std::string& name) const {
		return std::find_if(m_LoadedScenes.begin(), m_LoadedScenes.end(), [&name](const std::shared_ptr<Scene>& scene) {
			return scene && scene->GetName() == name;
			});
	}

	void SceneManager::RefreshActiveScene() {
		if (m_ActiveScene != nullptr) {
			return;
		}

		for (const auto& loadedScene : m_LoadedScenes) {
			if (loadedScene && loadedScene->IsLoaded()) {
				m_ActiveScene = loadedScene.get();
				break;
			}
		}
	}
}