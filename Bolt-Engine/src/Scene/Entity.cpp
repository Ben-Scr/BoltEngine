#include "pch.hpp"
#include "Scene/Entity.hpp"
#include "Scene/SceneManager.hpp"
#include "Scene/Scene.hpp"
#include "Components/General/NameComponent.hpp"
#include "Components/General/Transform2DComponent.hpp"
#include "Components/Tags.hpp"
#include "Debugging/Logger.hpp"


namespace Bolt {
	Entity Entity::Null{ entt::null, nullptr };

	EntityHandle Entity::GetHandle() const {
		return m_EntityHandle;
	}

	Entity::Entity(EntityHandle e, entt::registry& r)
		: m_EntityHandle(e), m_Registry(&r) {}

	Entity Entity::Create() {
		Scene* activeScene = SceneManager::Get().GetActiveScene();
		if (!activeScene || !activeScene->IsLoaded()) {
			Logger::Error("Entity", "Cannot create entity because there is no active scene loaded");
			return Entity::Null;
		}

		auto entity = Entity(activeScene->CreateEntityHandle(), activeScene->GetRegistry());
		entity.AddComponent<Transform2DComponent>();
		return entity;
	}

	void Entity::Destroy(Entity entity) {
		entity.Destroy();
	}

	void Entity::Destroy() {
		BT_ASSERT(m_Registry, BoltErrorCode::InvalidHandle, "Entity is not valid or has already been destroyed.");

		m_Registry->destroy(m_EntityHandle);
		m_EntityHandle = entt::null;
		m_Registry = nullptr;
	}

	std::string Entity::GetName() const {
		if (HasComponent<NameComponent>()) {
			return GetComponent<NameComponent>().Name;
		}
		else {
			return "Unnamed Entity (" + std::to_string(static_cast<std::uint32_t>(m_EntityHandle)) + ")";
		}
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