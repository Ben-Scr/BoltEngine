#include "pch.hpp"
#include "Scene/SceneManager.hpp"
#include "Scene/Scene.hpp"

#include <algorithm>
#include <exception>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <optional>
#include "SceneDefinition.hpp"
#include "Scene/BuiltInComponentRegistration.hpp"
#include "Systems/ParticleUpdateSystem.hpp"
#include "Core/Application.hpp"
#include "Components/Components.hpp"
#include "Events/SceneEvents.hpp"


namespace Bolt {
	namespace {
		struct EntitySnapshot {
			EntityHandle handle{ entt::null };
			std::optional<Transform2DComponent> transform2D;
			std::optional<RectTransformComponent> rectTransform;
			std::optional<NameComponent> name;
			std::optional<SpriteRendererComponent> spriteRenderer;
			std::optional<ImageComponent> image;
			std::optional<Camera2DComponent> camera2D;
			std::optional<ParticleSystem2DComponent> particleSystem2D;
			std::optional<BoxCollider2DComponent> boxCollider2D;
			std::optional<Rigidbody2DComponent> rigidbody2D;
			std::optional<AudioSourceComponent> audioSource;
			std::optional<BoltBody2DComponent> boltBody2D;
			std::optional<BoltBoxCollider2DComponent> boltBoxCollider2D;
			std::optional<BoltCircleCollider2DComponent> boltCircleCollider2D;
			bool hasIdTag = false;
			bool hasStaticTag = false;
			bool hasDisabledTag = false;
			bool hasDeadlyTag = false;
		};

		using SceneSnapshot = std::vector<EntitySnapshot>;

		template<typename T>
		static void CaptureComponentIfExists(const entt::registry& registry, EntityHandle entity, std::optional<T>& out) {
			if (registry.all_of<T>(entity)) {
				out = registry.get<T>(entity);
			}
		}

		template<typename T>
		static void RestoreComponentIfExists(entt::registry& registry, EntityHandle entity, const std::optional<T>& data) {
			if (data.has_value()) {
				registry.emplace<T>(entity, data.value());
			}
		}

		static SceneSnapshot CaptureSceneSnapshot(const Scene& scene) {
			SceneSnapshot snapshot;
			const entt::registry& registry = scene.GetRegistry();
			auto view = registry.view<entt::entity>();
			snapshot.reserve(view.size());

			for (const EntityHandle entity : view) {
				EntitySnapshot entitySnapshot;
				entitySnapshot.handle = entity;

				CaptureComponentIfExists(registry, entity, entitySnapshot.transform2D);
				CaptureComponentIfExists(registry, entity, entitySnapshot.rectTransform);
				CaptureComponentIfExists(registry, entity, entitySnapshot.name);
				CaptureComponentIfExists(registry, entity, entitySnapshot.spriteRenderer);
				CaptureComponentIfExists(registry, entity, entitySnapshot.image);
				CaptureComponentIfExists(registry, entity, entitySnapshot.camera2D);
				CaptureComponentIfExists(registry, entity, entitySnapshot.particleSystem2D);
				CaptureComponentIfExists(registry, entity, entitySnapshot.boxCollider2D);
				CaptureComponentIfExists(registry, entity, entitySnapshot.rigidbody2D);
				CaptureComponentIfExists(registry, entity, entitySnapshot.audioSource);
				CaptureComponentIfExists(registry, entity, entitySnapshot.boltBody2D);
				CaptureComponentIfExists(registry, entity, entitySnapshot.boltBoxCollider2D);
				CaptureComponentIfExists(registry, entity, entitySnapshot.boltCircleCollider2D);

				entitySnapshot.hasIdTag = registry.all_of<IdTag>(entity);
				entitySnapshot.hasStaticTag = registry.all_of<StaticTag>(entity);
				entitySnapshot.hasDisabledTag = registry.all_of<DisabledTag>(entity);
				entitySnapshot.hasDeadlyTag = registry.all_of<DeadlyTag>(entity);

				snapshot.push_back(std::move(entitySnapshot));
			}

			return snapshot;
		}

		static void RestoreSceneSnapshot(const SceneSnapshot& snapshot, Scene& scene) {
			entt::registry& registry = scene.GetRegistry();
			registry.clear();

			for (const EntitySnapshot& entitySnapshot : snapshot) {
				registry.create(entitySnapshot.handle);
			}

			for (const EntitySnapshot& entitySnapshot : snapshot) {
				const EntityHandle entity = entitySnapshot.handle;
				RestoreComponentIfExists(registry, entity, entitySnapshot.transform2D);
				RestoreComponentIfExists(registry, entity, entitySnapshot.rectTransform);
				RestoreComponentIfExists(registry, entity, entitySnapshot.name);
				RestoreComponentIfExists(registry, entity, entitySnapshot.spriteRenderer);
				RestoreComponentIfExists(registry, entity, entitySnapshot.image);
				RestoreComponentIfExists(registry, entity, entitySnapshot.camera2D);
				RestoreComponentIfExists(registry, entity, entitySnapshot.particleSystem2D);
				RestoreComponentIfExists(registry, entity, entitySnapshot.boxCollider2D);
				RestoreComponentIfExists(registry, entity, entitySnapshot.rigidbody2D);
				RestoreComponentIfExists(registry, entity, entitySnapshot.audioSource);
				RestoreComponentIfExists(registry, entity, entitySnapshot.boltBody2D);
				RestoreComponentIfExists(registry, entity, entitySnapshot.boltBoxCollider2D);
				RestoreComponentIfExists(registry, entity, entitySnapshot.boltCircleCollider2D);

				if (entitySnapshot.hasIdTag) registry.emplace<IdTag>(entity);
				if (entitySnapshot.hasStaticTag) registry.emplace<StaticTag>(entity);
				if (entitySnapshot.hasDisabledTag) registry.emplace<DisabledTag>(entity);
				if (entitySnapshot.hasDeadlyTag) registry.emplace<DeadlyTag>(entity);
			}
		}
	}

	SceneManager& SceneManager::Get() {
		auto* app = Application::GetInstance();
		BT_CORE_ASSERT(app && app->GetSceneManager(), "SceneManager is not available before the Application instance exists");
		return *app->GetSceneManager();
	}

	void SceneManager::Initialize() {
		if (m_IsInitialized) {
			BT_CORE_WARN_TAG("SceneManager", "Initialize called more than once");
			return;
		}
		if (m_SceneDefinitions.empty()) {
			BT_CORE_ERROR_TAG("SceneManager", "No scenes were registered before SceneManager initialization");
			return;
		}

		m_IsInitialized = true;
		RegisterCoreComponents();
		InitializeStartupScenes();

		if (m_LoadedScenes.empty()) {
			auto& firstPair = *m_SceneDefinitions.begin();
			if (LoadScene(firstPair.first).expired()) {
				BT_CORE_ERROR_TAG("SceneManager", "Failed to load fallback scene '{}'", firstPair.first);
			}
			else {
				BT_CORE_WARN_TAG("SceneManager", "Loaded fallback scene '{}'", firstPair.first);
			}
		}
	}

	void SceneManager::RegisterCoreComponents() {
		RegisterBuiltInComponents(*this);
	}

	void SceneManager::Shutdown() {
		UnloadAllScenes(true);
		m_SceneDefinitions.clear();
		m_ActiveScene = nullptr;
		m_IsInitialized = false;
	}

	SceneDefinition& SceneManager::RegisterScene(const std::string& name) {
		auto [it, inserted] = m_SceneDefinitions.emplace(name, std::make_unique<SceneDefinition>(name));
		if (!inserted) {
			BT_CORE_WARN_TAG("SceneManager", "Scene definition '{}' already exists", name);
			return *it->second;
		}

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
		if (!m_IsInitialized) {
			BT_CORE_ERROR_TAG("SceneManager", "LoadScene called before SceneManager initialization");
			return {};
		}
		SceneDefinition* definition = GetSceneDefinition(name);
		if (!definition) {
			return {};
		}
		if (!additive) {
			if (IsSceneLoaded(name)) {
				BT_CORE_WARN_TAG("SceneManager", "Scene '{}' is already loaded. Use LoadSceneAdditive() for multiple instances.", name);
				return GetLoadedScene(name).lock();
			}
			UnloadAllScenes(false);
		}

		std::shared_ptr<Scene> newScene = definition->Instantiate();
		for (const auto& callback : definition->m_LoadCallbacks) {
			callback(*newScene);
		}

		{
			ScenePreStartEvent e(name);
			Application* app = Application::GetInstance();
			if (app) app->OnEvent(e);
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

		{
			ScenePostStartEvent e(name);
			Application* app = Application::GetInstance();
			if (app) app->OnEvent(e);
		}

		BT_CORE_ASSERT(m_ActiveScene, "Active Scene is null after loading");
		return newScene;
	}

	std::weak_ptr<Scene> SceneManager::ReloadScene(const std::string& name) {
		std::shared_ptr<Scene> scene = GetLoadedScene(name).lock();
		if (!scene) {
			BT_CORE_WARN_TAG("SceneManager", "ReloadScene: scene '{}' is not loaded", name);
			return {};
		}
		const bool wasActive = m_ActiveScene == scene.get();
		const SceneSnapshot snapshot = CaptureSceneSnapshot(*scene);

		UnloadScene(name);
		std::weak_ptr<Scene> reloadedWeak = wasActive ? LoadScene(name) : LoadSceneAdditive(name);
		if (std::shared_ptr<Scene> reloaded = reloadedWeak.lock()) {
			RestoreSceneSnapshot(snapshot, *reloaded);
		}

		return reloadedWeak;
	}

	void SceneManager::UnloadScene(const std::string& name) {
		auto it = FindLoadedSceneIterator(name);
		if (it == m_LoadedScenes.end()) {
			BT_CORE_WARN_TAG("SceneManager", "Scene '{}' is not loaded", name);
			return;
		}

		Scene& scene = *(*it);
		if (scene.m_Persistent) {
			return;
		}

		ReleaseScene(it);
	}

	void SceneManager::ReleaseScene(LoadedSceneList::iterator it) {
		Scene& scene = *(*it);
		const std::string sceneName = scene.GetName();

		{
			ScenePreStopEvent e(sceneName);
			Application* app = Application::GetInstance();
			if (app) app->OnEvent(e);
		}

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

		{
			ScenePostStopEvent e(sceneName);
			Application* app = Application::GetInstance();
			if (app) app->OnEvent(e);
		}
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
		if (it == m_LoadedScenes.end()) {
			BT_CORE_WARN_TAG("SceneManager", "Scene '{}' is not loaded", name);
			return {};
		}
		return std::weak_ptr(*it);
	}

	Scene* SceneManager::GetActiveScene() {
		if (!m_ActiveScene) {
			BT_CORE_WARN_TAG("SceneManager", "There is no active scene");
		}
		return m_ActiveScene;
	}

	const Scene* SceneManager::GetActiveScene() const {
		if (!m_ActiveScene) {
			BT_CORE_WARN_TAG("SceneManager", "There is no active scene");
		}
		return m_ActiveScene;
	}

	bool SceneManager::SetActiveScene(const std::string& name) {
		auto it = FindLoadedSceneIterator(name);
		if (it == m_LoadedScenes.end()) {
			BT_CORE_WARN_TAG("SceneManager", "Scene '{}' is not loaded, load it first", name);
			return false;
		}
		m_ActiveScene = it->get();
		return true;
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
				BT_CORE_ERROR_TAG("SceneManager", "Failed to load startup scene '{}': {}", name, e.what());
			}
			catch (...) {
				BT_CORE_ERROR_TAG("SceneManager", "Failed to load startup scene '{}'", name);
			}
		}
	}

	SceneDefinition* SceneManager::GetSceneDefinition(const std::string& name) {
		auto it = m_SceneDefinitions.find(name);
		if (it == m_SceneDefinitions.end()) {
			BT_CORE_ERROR_TAG("SceneManager", "Scene definition '{}' not found, call RegisterScene() first", name);
			return nullptr;
		}
		return it->second.get();
	}

	const SceneDefinition* SceneManager::GetSceneDefinition(const std::string& name) const {
		auto it = m_SceneDefinitions.find(name);
		if (it == m_SceneDefinitions.end()) {
			BT_CORE_ERROR_TAG("SceneManager", "Scene definition '{}' not found, call RegisterScene() first", name);
			return nullptr;
		}
		return it->second.get();
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