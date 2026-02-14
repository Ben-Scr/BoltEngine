#pragma once
#include "Scene/EntityHandle.hpp"
#include "Components/ComponentUtils.hpp"
#include "Core/Core.hpp"

namespace Bolt {

	class BOLT_API Entity {
		friend class Scene;
		friend class EntityHelper;

	public:
		static Entity Create();

		static void Destroy(Entity entity);
		void Destroy();
		static Entity Null;


		template<typename TComponent, typename... Args>
			requires (!std::is_empty_v<TComponent>)
		TComponent& AddComponent(Args&&... args) {
			return  ComponentUtils::AddComponent<TComponent>(*m_Registry, m_EntityHandle, std::forward<Args>(args)...);
		}

		template<typename TTag>
			requires std::is_empty_v<TTag>
		void AddComponent() {
			ComponentUtils::AddComponent<TTag>(*m_Registry, m_EntityHandle);
		}

		template<typename TComponent>
		bool HasComponent() const {
			return  ComponentUtils::HasComponent<TComponent>(*m_Registry, m_EntityHandle);
		}

		template<typename... TComponent>
		bool HasAnyComponent() const {
			return  ComponentUtils::HasAnyComponent<TComponent...>(*m_Registry, m_EntityHandle);
		}

		template<typename TComponent>
		TComponent& GetComponent() {
			return  ComponentUtils::GetComponent<TComponent>(*m_Registry, m_EntityHandle);
		}

		template<typename TComponent>
		const TComponent& GetComponent() const {
			return  ComponentUtils::GetComponent<TComponent>(*m_Registry, m_EntityHandle);
		}

		template<typename TComponent>
		bool TryGetComponent(TComponent*& out) {
			out = ComponentUtils::TryGetComponent<TComponent>(m_Registry, m_EntityHandle);
			return out != nullptr;
		}

		template<typename TComponent>
		void RemoveComponent() {
			ComponentUtils::RemoveComponent<TComponent>(*m_Registry, m_EntityHandle);
		}

		std::string GetName() const;

		EntityHandle GetHandle() const;

		void SetStatic(bool isStatic);
		void SetEnabled(bool enabled);

	private:
		explicit Entity(EntityHandle e, entt::registry& r);
		Entity(EntityHandle e, entt::registry* r) : m_EntityHandle{ e }, m_Registry{ r } {};
		EntityHandle    m_EntityHandle;
		entt::registry* m_Registry;
	};

	inline bool operator==(const Entity& a, const Entity& b) { return a.GetHandle() == b.GetHandle(); }
	inline bool operator!=(const Entity& a, const Entity& b) { return !(a == b); }
}