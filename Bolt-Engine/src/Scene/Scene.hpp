#pragma once
#include "Scene/Entity.hpp"
#include "Scene/ISystem.hpp"
#include "Core/Export.hpp"

namespace Bolt {
	class SceneDefinition;
	class BOLT_API Scene {
		friend class SceneManager;
		friend class SceneDefinition;
		friend class Application;

	public:
		Scene(const Scene&) = delete;

		// Info: Creates an entity with a Transform2D component
		Entity CreateEntity();
		// Info: Creates an entity with a Transform2D and Name component
		Entity CreateEntity(const std::string& name);
		// Info: Creates an entity and returnes its handle
		EntityHandle CreateEntityHandle();

		Entity GetEntity(EntityHandle nativeEntity) { return Entity(nativeEntity, m_Registry); }
		Entity GetEntity(EntityHandle nativeEntity) const { return Entity(nativeEntity, const_cast<entt::registry&>(m_Registry)); }

		void DestroyEntity(Entity entity);
		void DestroyEntity(EntityHandle nativeEntity);

		bool IsValid(EntityHandle nativeEntity) const {
			return m_Registry.valid(nativeEntity);
		}
		template<typename TComponent, typename... Args>
			requires (!std::is_empty_v<TComponent>)
		TComponent& AddComponent(EntityHandle entity, Args&&... args) {
			return ComponentUtils::AddComponent<TComponent>(m_Registry, entity, std::forward<Args>(args)...);
		}

		template<typename TTag>
			requires std::is_empty_v<TTag>
		void AddComponent(EntityHandle entity) {
			ComponentUtils::AddComponent<TTag>(m_Registry, entity);
		}

		template<typename TComponent>
		bool HasComponent(EntityHandle entity) const {
			return ComponentUtils::HasComponent<TComponent>(m_Registry, entity);
		}

		template<typename... TComponent>
		bool HasAnyComponent(EntityHandle entity) const {
			return ComponentUtils::HasAnyComponent<TComponent...>(m_Registry, entity);
		}

		template<typename... TComponent, typename... TEntity>
		bool AnyHasComponent(TEntity... handles) const {
			return ComponentUtils::AnyHasComponent<TComponent...>(m_Registry, handles...);
		}

		template<typename TComponent>
		TComponent& GetComponent(EntityHandle entity) {
			return ComponentUtils::GetComponent<TComponent>(m_Registry, entity);
		}

		template<typename TComponent>
		const TComponent& GetComponent(EntityHandle entity) const {
			return ComponentUtils::GetComponent<TComponent>(m_Registry, entity);
		}

		template<typename TComponent>
		bool TryGetComponent(EntityHandle entity, TComponent*& out) {
			out = ComponentUtils::TryGetComponent<TComponent>(m_Registry, entity);
			return out != nullptr;
		}

		template<typename TComponent>
		void RemoveComponent(EntityHandle entity) {
			ComponentUtils::RemoveComponent<TComponent>(m_Registry, entity);
		}

		template<typename TComponent>
		TComponent& GetSingletonComponent() {
			auto view = m_Registry.view<TComponent>();
			const auto count = view.size();
			if (count == 0) {
				throw std::runtime_error(
					"Component of type \"" +
					std::string(typeid(TComponent).name()) +
					"\" not found"
				);
			}
			if (count > 1) {
				throw std::runtime_error(
					"More than one component of type \"" +
					std::string(typeid(TComponent).name()) +
					"\" exists in the registry, found " +
					std::to_string(count)
				);
			}

			auto entity = *view.begin();
			return view.get<TComponent>(entity);
		}

		template<typename TComponent>
		entt::entity GetSingletonEntity() {
			auto view = m_Registry.view<TComponent>();

			const auto count = view.size();
			if (count == 0) {
				throw std::runtime_error(
					"Component of type \"" +
					std::string(typeid(TComponent).name()) +
					"\" not found"
				);
			}
			if (count > 1) {
				throw std::runtime_error(
					"More than one component of type \"" +
					std::string(typeid(TComponent).name()) +
					"\" exists in the registry, found " +
					std::to_string(count)
				);
			}

			return *view.begin();
		}

		template<typename T>
		T& GetSystem() {
			static_assert(std::is_base_of<ISystem, T>::value, "T must derive from ISystem");

			for (auto& sysPtr : m_Systems) {
				if (auto ptr = dynamic_cast<T*>(sysPtr.get())) {
					return *ptr;
				}
			}

			BT_LOG_ERROR(BoltErrorCode::Undefined, "System not found");
			std::terminate();
		}

		template<typename T>
		bool HasSystem() const {
			static_assert(std::is_base_of<ISystem, T>::value, "T must derive from ISystem");

			for (const auto& sysPtr : m_Systems) {
				if (dynamic_cast<T*>(sysPtr.get())) {
					return true;
				}
			}
			return false;
		}

		template<typename T>
		void DisableSystem() {
			static_assert(std::is_base_of<ISystem, T>::value, "T must derive from ISystem");

			for (auto& sysPtr : m_Systems) {
				if (auto ptr = dynamic_cast<T*>(sysPtr.get())) {
					if (ptr->IsEnabled()) {
						ptr->SetEnabled(false, *this);
					}
				}
			}
		}

		template<typename T>
		void EnableSystem() {
			static_assert(std::is_base_of<ISystem, T>::value, "T must derive from ISystem");

			for (auto& sysPtr : m_Systems) {
				if (auto ptr = dynamic_cast<T*>(sysPtr.get())) {
					ptr->SetEnabled(true, *this);
				}
			}
		}

		entt::registry& GetRegistry() { return m_Registry; }
		const entt::registry& GetRegistry() const { return m_Registry; }

		const std::string& GetName() const { return m_Name; }
		void SetName(const std::string& name) { m_Name = name; }
		bool IsLoaded() const { return m_IsLoaded; }
		bool IsPersistent() const { return m_Persistent; }
		const SceneDefinition* GetDefinition() const { return m_Definition; }

		bool IsDirty() const { return m_Dirty; }
		void MarkDirty() { m_Dirty = true; }
		void ClearDirty() { m_Dirty = false; }

	private:
		Scene(const std::string& name, const SceneDefinition* definition, bool IsPersistent);

		void OnTransform2DComponentConstruct(entt::registry& registry, EntityHandle entity);
		void OnRigidBody2DComponentConstruct(entt::registry& registry, EntityHandle entity);
		void OnRigidBody2DComponentDestroy(entt::registry& registry, EntityHandle entity);
		void OnBoxCollider2DComponentConstruct(entt::registry& registry, EntityHandle entity);
		void OnBoxCollider2DComponentDestroy(entt::registry& registry, EntityHandle entity);

		void OnCamera2DComponentConstruct(entt::registry& registry, EntityHandle entity);
		void OnCamera2DComponentDestruct(entt::registry& registry, EntityHandle entity);

		void OnParticleSystem2DComponentConstruct(entt::registry& registry, EntityHandle entity);
		void OnParticleSystem2DComponentDestruct(entt::registry& registry, EntityHandle entity);

		// Bolt-Physics component hooks
		void OnBoltBody2DConstruct(entt::registry& registry, EntityHandle entity);
		void OnBoltBody2DDestroy(entt::registry& registry, EntityHandle entity);
		void OnBoltBoxCollider2DConstruct(entt::registry& registry, EntityHandle entity);
		void OnBoltBoxCollider2DDestroy(entt::registry& registry, EntityHandle entity);
		void OnBoltCircleCollider2DConstruct(entt::registry& registry, EntityHandle entity);
		void OnBoltCircleCollider2DDestroy(entt::registry& registry, EntityHandle entity);

		void AwakeSystems();
		void StartSystems();
		void UpdateSystems();
		void FixedUpdateSystems();
		void OnGuiSystems();
		void DestroyScene();

		void ForeachEnabledSystem(const std::function<void(ISystem&)>& func) {
			for (size_t i = 0; i < m_Systems.size(); ++i) {
				ISystem& s = *m_Systems[i];
				if (s.IsEnabled())
				{
					try {
						func(s);
					}
					catch (const std::exception& e) {
						BT_ERROR(e.what());
					}
				}
			}
		}

		entt::registry m_Registry;
		std::vector<std::unique_ptr<ISystem>> m_Systems;


		std::string m_Name;
		const SceneDefinition* m_Definition;

		bool m_IsLoaded = false;
		bool m_Persistent = false;
		bool m_Dirty = false;
	};
}