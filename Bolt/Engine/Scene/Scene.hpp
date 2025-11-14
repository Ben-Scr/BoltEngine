#pragma once
#include "../Scene/Entity.hpp"
#include "../Scene/ISystem.hpp"

namespace Bolt {
	class SceneDefinition;
	class Scene {
	public:
		Scene(const Scene&) = delete;
		//Scene& operator=(const Scene&) = delete;

		Entity CreateEntity();
		Entity CreateCamera();
		Entity CreateRenderableEntity();
		EntityHandle CreateEntityHandle();

		template<typename... TComponent>
		EntityHandle CreateEntityHandle() {
			auto entity = m_Registry.create();
			(m_Registry.emplace<TComponent>(entity), ...);
			return entity;
		}

		void DestroyEntity(Entity entity);
		void DestroyEntity(EntityHandle nativeEntity);

		bool IsValid(EntityHandle nativeEntity) const {
			return m_Registry.valid(nativeEntity);
		}
		template<typename TComponent, typename... Args>
			requires (!std::is_empty_v<TComponent>)
		TComponent& AddComponent(EntityHandle blockTexture, Args&&... args) {
			return ComponentUtils::AddComponent<TComponent>(m_Registry, blockTexture, std::forward<Args>(args)...);
		}

		template<typename TTag>
			requires std::is_empty_v<TTag>
		void AddComponent(EntityHandle blockTexture) {
			ComponentUtils::AddComponent<TTag>(m_Registry, blockTexture);
		}

		template<typename TComponent>
		bool HasComponent(EntityHandle blockTexture) const {
			return ComponentUtils::HasComponent<TComponent>(m_Registry, blockTexture);
		}

		template<typename... TComponent>
		bool HasAnyComponent(EntityHandle blockTexture) const {
			return ComponentUtils::HasAnyComponent<TComponent...>(m_Registry, blockTexture);
		}

		template<typename... TComponent, typename... TEntity>
		bool AnyHasComponent(TEntity... handles) const {
			return ComponentUtils::AnyHasComponent<TComponent...>(m_Registry, handles...);
		}

		template<typename TComponent>
		TComponent& GetComponent(EntityHandle blockTexture) {
			return ComponentUtils::GetComponent<TComponent>(m_Registry, blockTexture);
		}

		template<typename TComponent>
		const TComponent& GetComponent(EntityHandle blockTexture) const {
			return ComponentUtils::GetComponent<TComponent>(m_Registry, blockTexture);
		}

		template<typename TComponent>
		bool TryGetComponent(EntityHandle blockTexture, TComponent*& out) {
			out = ComponentUtils::TryGetComponent<TComponent>(m_Registry, blockTexture);
			return out != nullptr;
		}

		template<typename TComponent>
		void RemoveComponent(EntityHandle blockTexture) {
			ComponentUtils::RemoveComponent<TComponent>(m_Registry, blockTexture);
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
			throw std::runtime_error("System not found");
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
					if (ptr->m_enabled) {
						ptr->disable(*this);
						ptr->m_enabled = false;
					}
				}
			}
		}

		template<typename T>
		void EnableSystem() {
			static_assert(std::is_base_of<ISystem, T>::value, "T must derive from ISystem");

			for (auto& sysPtr : m_Systems) {
				if (auto ptr = dynamic_cast<T*>(sysPtr.get())) {
					ptr->m_enabled = true;
					ptr->enable(*this);
				}
			}
		}

		entt::registry& GetRegistry() { return m_Registry; }
		const entt::registry& GetRegistry() const { return m_Registry; }

		const std::string& GetName() const { return m_Name; }
		bool IsLoaded() const { return m_IsLoaded; }
		bool IsPersistent() const { return m_Persistent; }
		const SceneDefinition* GetDefinition() const { return m_Definition; }

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
		//void OnCircleCollider2DComponentConstruct(entt::registry& registry, entt::entity entity);

		void AwakeSystems();
		void StartSystems();
		void UpdateSystems();
		void FixedUpdateSystems();
		void DestroyScene();


		entt::registry m_Registry;
		std::vector<std::unique_ptr<ISystem>> m_Systems;


		std::string m_Name;
		const SceneDefinition* m_Definition;

		bool m_IsLoaded = false;
		bool m_Persistent = false;

		friend class SceneManager;
		friend class SceneDefinition;
		friend class Application;
	};
}