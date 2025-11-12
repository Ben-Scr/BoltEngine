#include "../pch.hpp"
#include "../Scene/Entity.hpp"
#include "../Scene/SceneManager.hpp"
#include "../Scene/Scene.hpp"



namespace Bolt {
	Entity Entity::Null{ entt::null, nullptr };

	EntityHandle Entity::GetHandle() const {
		return m_EntityHandle;
	}

	Entity::Entity(EntityHandle e, entt::registry& r)
		: m_EntityHandle(e), m_Registry(&r) {
	}

	Entity Entity::Create() {
		Scene& activeScene = SceneManager::GetActiveScene();

		if (!activeScene.IsLoaded()) {
			throw std::runtime_error("There is no active Scene Loaded");
		}

		auto entity = Entity(activeScene.CreateEntityHandle(), activeScene.GetRegistry());
		entity.AddComponent<Transform2D>();
		return entity;
	}

	void Entity::Destroy(Entity entity) {
		entity.Destroy();
	}
	void Entity::Destroy() {
		if (m_Registry == nullptr) {
			throw std::runtime_error("Entity is not valid or has already been destroyed.");
		}
		m_Registry->destroy(m_EntityHandle);
		m_EntityHandle = entt::null;
		m_Registry = nullptr;
	}

	void Entity::SetStatic(bool isStatic) {
		if (isStatic) { if (!HasComponent<StaticTag>()) AddComponent<StaticTag>(); }
		else { if (HasComponent<StaticTag>()) RemoveComponent<StaticTag>(); }
	}

	void Entity::SetEnabled(bool enabled) {
		if (enabled) { if (!HasComponent<DisabledTag>()) AddComponent<DisabledTag>(); }
		else { if (HasComponent<DisabledTag>()) RemoveComponent<DisabledTag>(); }
	}
}

