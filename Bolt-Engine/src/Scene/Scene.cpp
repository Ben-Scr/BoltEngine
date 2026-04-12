#include "pch.hpp"
#include "Scene/Scene.hpp"
#include "Scene/Entity.hpp"

#include "Components/Audio/AudioSourceComponent.hpp"

#include "Components/Physics/Rigidbody2DComponent.hpp"
#include "Components/Physics/BoxCollider2DComponent.hpp"
#include "Components/Physics/BoltBody2DComponent.hpp"
#include "Components/Physics/BoltBoxCollider2DComponent.hpp"
#include "Components/Physics/BoltCircleCollider2DComponent.hpp"
#include "Physics/PhysicsSystem2D.hpp"
#include "Components/Graphics/ParticleSystem2DComponent.hpp"
#include "Components/Graphics/SpriteRendererComponent.hpp"
#include "Components/General/NameComponent.hpp"
#include <Components/Tags.hpp>

#include "Physics/Box2DWorld.hpp"

#include "Components/Graphics/Camera2DComponent.hpp"
#include "Components/General/UUIDComponent.hpp"
#include "Core/Application.hpp"

namespace Bolt {
	Entity Scene::CreateEntity() {
		auto entityHandle = CreateEntityHandle();
		AddComponent<UUIDComponent>(entityHandle);
		AddComponent<Transform2DComponent>(entityHandle);
		if (!Application::GetIsPlaying()) m_Dirty = true;
		return Entity(entityHandle, m_Registry);
	}
	Entity Scene::CreateEntity(const std::string& name) {
		auto entityHandle = CreateEntityHandle();
		AddComponent<UUIDComponent>(entityHandle);
		AddComponent<Transform2DComponent>(entityHandle);
		AddComponent<NameComponent>(entityHandle, name);
		if (!Application::GetIsPlaying()) m_Dirty = true;
		return Entity(entityHandle, m_Registry);
	}

	EntityHandle Scene::CreateEntityHandle() { return m_Registry.create(); }

	void Scene::DestroyEntity(Entity entity) { DestroyEntity(entity.GetHandle()); }
	void Scene::DestroyEntity(EntityHandle nativeEntity) { DestroyEntityInternal(nativeEntity, true); }

	void Scene::ClearEntities() {
		auto view = m_Registry.view<entt::entity>();
		std::vector<EntityHandle> entities;
		entities.reserve(view.size());

		for (EntityHandle entity : view) {
			entities.push_back(entity);
		}

		for (EntityHandle entity : entities) {
			if (m_Registry.valid(entity)) {
				DestroyEntityInternal(entity, false);
			}
		}
	}

	void Scene::MarkDirty() { if (!Application::GetIsPlaying()) m_Dirty = true; }

	Camera2DComponent* Scene::GetMainCamera() {
		const auto isUsableCamera = [this](EntityHandle entity) {
			return entity != entt::null
				&& m_Registry.valid(entity)
				&& m_Registry.all_of<Camera2DComponent>(entity)
				&& !m_Registry.all_of<DisabledTag>(entity);
		};

		if (isUsableCamera(m_MainCameraEntity)) {
			return &m_Registry.get<Camera2DComponent>(m_MainCameraEntity);
		}

		auto view = m_Registry.view<Camera2DComponent>(entt::exclude<DisabledTag>);
		for (EntityHandle entity : view) {
			m_MainCameraEntity = entity;
			return &view.get<Camera2DComponent>(entity);
		}

		m_MainCameraEntity = entt::null;
		return nullptr;
	}

	const Camera2DComponent* Scene::GetMainCamera() const {
		return const_cast<Scene*>(this)->GetMainCamera();
	}

	bool Scene::SetMainCamera(EntityHandle entity) {
		if (entity == entt::null
			|| !m_Registry.valid(entity)
			|| !m_Registry.all_of<Camera2DComponent>(entity)
			|| m_Registry.all_of<DisabledTag>(entity)) {
			return false;
		}

		m_MainCameraEntity = entity;
		return true;
	}

	void Scene::DestroyEntityInternal(EntityHandle nativeEntity, bool markDirty) {
		if (markDirty && !Application::GetIsPlaying()) {
			m_Dirty = true;
		}

		TrackEntityDestruction(nativeEntity);
		m_Registry.destroy(nativeEntity);
		UntrackEntityDestruction(nativeEntity);
	}

	void Scene::TrackEntityDestruction(EntityHandle entity) {
		m_EntitiesBeingDestroyed.insert(static_cast<uint32_t>(entity));
	}

	void Scene::UntrackEntityDestruction(EntityHandle entity) {
		m_EntitiesBeingDestroyed.erase(static_cast<uint32_t>(entity));
	}

	bool Scene::IsEntityBeingDestroyed(EntityHandle entity) const {
		return m_EntitiesBeingDestroyed.contains(static_cast<uint32_t>(entity));
	}

	void Scene::AwakeSystems() {
		ForeachEnabledSystem([this](ISystem& s) { s.Awake(*this); });
	}

	void Scene::StartSystems() {
		ForeachEnabledSystem([this](ISystem& s) { s.Start(*this); });
	}

	void Scene::UpdateSystems() {
		ForeachEnabledSystem([this](ISystem& s) { s.Update(*this); });
	}

	void Scene::FixedUpdateSystems() {
		ForeachEnabledSystem([this](ISystem& s) { s.FixedUpdate(*this); });
	}

	void Scene::OnGuiSystems() {
		ForeachEnabledSystem([this](ISystem& s) { s.OnGui(*this); });
	}

	void Scene::DestroyScene() {
		ForeachEnabledSystem([this](ISystem& s) { s.OnDestroy(*this); });
	}

	Scene::Scene(const std::string& name, const SceneDefinition* definition, bool IsPersistent)
		: m_Name(name)
		, m_Definition(definition)
		, m_Persistent(IsPersistent)
		, m_IsLoaded(false) {

		m_Registry.on_construct<Rigidbody2DComponent>().connect<&Scene::OnRigidBody2DComponentConstruct>(this);
		m_Registry.on_construct<BoxCollider2DComponent>().connect<&Scene::OnBoxCollider2DComponentConstruct>(this);
		m_Registry.on_construct<Camera2DComponent>().connect<&Scene::OnCamera2DComponentConstruct>(this);
		m_Registry.on_construct<ParticleSystem2DComponent>().connect<&Scene::OnParticleSystem2DComponentConstruct>(this);

		m_Registry.on_destroy<Rigidbody2DComponent>().connect<&Scene::OnRigidBody2DComponentDestroy>(this);
		m_Registry.on_destroy<BoxCollider2DComponent>().connect<&Scene::OnBoxCollider2DComponentDestroy>(this);
		m_Registry.on_destroy<Camera2DComponent>().connect<&Scene::OnCamera2DComponentDestruct>(this);
		m_Registry.on_destroy<ParticleSystem2DComponent>().connect<&Scene::OnParticleSystem2DComponentDestruct>(this);

		// Bolt-Physics component hooks
		m_Registry.on_construct<BoltBody2DComponent>().connect<&Scene::OnBoltBody2DConstruct>(this);
		m_Registry.on_destroy<BoltBody2DComponent>().connect<&Scene::OnBoltBody2DDestroy>(this);
		m_Registry.on_construct<BoltBoxCollider2DComponent>().connect<&Scene::OnBoltBoxCollider2DConstruct>(this);
		m_Registry.on_destroy<BoltBoxCollider2DComponent>().connect<&Scene::OnBoltBoxCollider2DDestroy>(this);
		m_Registry.on_construct<BoltCircleCollider2DComponent>().connect<&Scene::OnBoltCircleCollider2DConstruct>(this);
		m_Registry.on_destroy<BoltCircleCollider2DComponent>().connect<&Scene::OnBoltCircleCollider2DDestroy>(this);
	}

	void Scene::OnRigidBody2DComponentConstruct(entt::registry& registry, EntityHandle entity)
	{
		if (!registry.all_of<Transform2DComponent>(entity)) {
			BT_CORE_WARN_TAG("Scene", "Adding missing Transform2DComponent before creating Rigidbody2D for entity {}", static_cast<uint32_t>(entity));
			registry.emplace<Transform2DComponent>(entity);
		}

		bool isEnabled = !registry.all_of<DisabledTag>(entity);
		Rigidbody2DComponent& rb2D = registry.get<Rigidbody2DComponent>(entity);

		if (HasAnyComponent<BoxCollider2DComponent>(entity)) {
			rb2D.m_BodyId = GetComponent<BoxCollider2DComponent>(entity).m_BodyId;
			rb2D.SetBodyType(BodyType::Dynamic);
		}
		else {
			rb2D.m_BodyId = PhysicsSystem2D::GetMainPhysicsWorld().CreateBody(entity, *this, BodyType::Dynamic);
		}

		rb2D.SetEnabled(isEnabled);
	}

	void Scene::OnRigidBody2DComponentDestroy(entt::registry& registry, EntityHandle entity) {
		auto& rb2D = GetComponent<Rigidbody2DComponent>(entity);

		if (!rb2D.IsValid()) return;

		if (IsEntityBeingDestroyed(entity) && registry.all_of<BoxCollider2DComponent>(entity)) {
			registry.get<BoxCollider2DComponent>(entity).Destroy();
		}
		else if (registry.all_of<BoxCollider2DComponent>(entity)) {
			rb2D.SetBodyType(BodyType::Static);
		}
		else {
			rb2D.Destroy();
		}
	}

	void Scene::OnBoxCollider2DComponentConstruct(entt::registry& registry, EntityHandle entity) {
		if (!registry.all_of<Transform2DComponent>(entity)) {
			BT_CORE_WARN_TAG("Scene", "Adding missing Transform2DComponent before creating BoxCollider2D for entity {}", static_cast<uint32_t>(entity));
			registry.emplace<Transform2DComponent>(entity);
		}

		bool isEnabled = !registry.all_of<DisabledTag>(entity);
		BoxCollider2DComponent& boxCollider = GetComponent<BoxCollider2DComponent>(entity);
		boxCollider.m_EntityHandle = entity;

		// Note: Check if the collider already has the Rigidbody2D component
		// Get the body and assign it to the boxCollider
		// Else Create a new Static body
		if (HasComponent<Rigidbody2DComponent>(entity)) {
			auto& rb = GetComponent<Rigidbody2DComponent>(entity);
			boxCollider.m_BodyId = rb.GetBodyHandle();
		}
		else {
			boxCollider.m_BodyId = PhysicsSystem2D::GetMainPhysicsWorld().CreateBody(entity, *this, BodyType::Static);
		}

		boxCollider.m_ShapeId = PhysicsSystem2D::GetMainPhysicsWorld().CreateShape(entity, *this, boxCollider.m_BodyId, ShapeType::Square);
		boxCollider.SetEnabled(isEnabled);
	}

	void Scene::OnBoxCollider2DComponentDestroy(entt::registry& registry, EntityHandle entity) {
		auto& boxCollider2D = GetComponent<BoxCollider2DComponent>(entity);
		if (IsEntityBeingDestroyed(entity) || !registry.all_of<Rigidbody2DComponent>(entity)) {
			boxCollider2D.Destroy();
		}
		else {
			boxCollider2D.DestroyShape(false);
		}
	}

	void Scene::OnCamera2DComponentConstruct(entt::registry& registry, EntityHandle entity)
	{
		Camera2DComponent& camera2D = GetComponent<Camera2DComponent>(entity);
		camera2D.Initialize(GetComponent<Transform2DComponent>(entity));
		camera2D.UpdateViewport();

		if (m_MainCameraEntity == entt::null
			|| !registry.valid(m_MainCameraEntity)
			|| registry.all_of<DisabledTag>(m_MainCameraEntity)) {
			m_MainCameraEntity = entity;
		}
	}

	void Scene::OnCamera2DComponentDestruct(entt::registry& registry, EntityHandle entity)
	{
		Camera2DComponent& camera2D = GetComponent<Camera2DComponent>(entity);
		camera2D.Destroy();

		if (m_MainCameraEntity == entity) {
			m_MainCameraEntity = entt::null;
			auto view = registry.view<Camera2DComponent>(entt::exclude<DisabledTag>);
			for (EntityHandle candidate : view) {
				if (candidate != entity) {
					m_MainCameraEntity = candidate;
					break;
				}
			}
		}
	}

	void Scene::OnParticleSystem2DComponentConstruct(entt::registry& registry, EntityHandle entity) {
		auto& ps = registry.get<ParticleSystem2DComponent>(entity);
		ps.m_EmitterTransform = &GetComponent<Transform2DComponent>(entity);
	}

	void Scene::OnParticleSystem2DComponentDestruct(entt::registry& registry, EntityHandle entity) {
		auto& ps = registry.get<ParticleSystem2DComponent>(entity);
		ps.m_EmitterTransform = nullptr;
	}

	// ── Bolt-Physics component hooks ────────────────────────────────

	void Scene::OnBoltBody2DConstruct(entt::registry& registry, EntityHandle entity) {
		auto& comp = registry.get<BoltBody2DComponent>(entity);
		auto& boltWorld = PhysicsSystem2D::GetBoltPhysicsWorld();

		comp.m_Body = boltWorld.CreateBody(entity, comp.Type);
		if (comp.m_Body) {
			comp.m_Body->SetMass(comp.Mass);
			comp.m_Body->SetGravityEnabled(comp.UseGravity);
			comp.m_Body->SetBoundaryCheckEnabled(comp.BoundaryCheck);

			// Sync initial position from Transform
			if (HasComponent<Transform2DComponent>(entity)) {
				auto& tf = GetComponent<Transform2DComponent>(entity);
				comp.m_Body->SetPosition({ tf.Position.x, tf.Position.y });
			}
		}
	}

	void Scene::OnBoltBody2DDestroy(entt::registry& registry, EntityHandle entity) {
		PhysicsSystem2D::GetBoltPhysicsWorld().DestroyBody(entity);
	}

	void Scene::OnBoltBoxCollider2DConstruct(entt::registry& registry, EntityHandle entity) {
		auto& comp = registry.get<BoltBoxCollider2DComponent>(entity);
		auto& boltWorld = PhysicsSystem2D::GetBoltPhysicsWorld();
		comp.m_Collider = boltWorld.CreateBoxCollider(entity, comp.HalfExtents);
	}

	void Scene::OnBoltBoxCollider2DDestroy(entt::registry& registry, EntityHandle entity) {
		PhysicsSystem2D::GetBoltPhysicsWorld().DestroyCollider(entity);
	}

	void Scene::OnBoltCircleCollider2DConstruct(entt::registry& registry, EntityHandle entity) {
		auto& comp = registry.get<BoltCircleCollider2DComponent>(entity);
		auto& boltWorld = PhysicsSystem2D::GetBoltPhysicsWorld();
		comp.m_Collider = boltWorld.CreateCircleCollider(entity, comp.Radius);
	}

	void Scene::OnBoltCircleCollider2DDestroy(entt::registry& registry, EntityHandle entity) {
		PhysicsSystem2D::GetBoltPhysicsWorld().DestroyCollider(entity);
	}
}
