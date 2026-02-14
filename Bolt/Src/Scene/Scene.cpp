#include "pch.hpp"
#include "Scene/Scene.hpp"
#include "Scene/Entity.hpp"

#include "Components/AudioSourceComponent.hpp"

#include "Components/Rigidbody2DComponent.hpp"
#include "Components/BoxCollider2DComponent.hpp"
#include "Components/ParticleSystem2DComponent.hpp"
#include "Components/SpriteRendererComponent.hpp"
#include "Components/NameComponent.hpp"

#include "Physics/Box2DWorld.hpp"

#include "Components/Camera2DComponent.hpp"

namespace Bolt {
	Entity Scene::CreateEntity() {
		auto entityHandle = CreateEntityHandle();
		AddComponent<Transform2DComponent>(entityHandle);
		return Entity(entityHandle, m_Registry);
	}
	Entity Scene::CreateEntity(const std::string& name) {
		auto entityHandle = CreateEntityHandle();
		AddComponent<Transform2DComponent>(entityHandle);
		AddComponent<NameComponent>(entityHandle, name);
		return Entity(entityHandle, m_Registry);
	}

	EntityHandle Scene::CreateEntityHandle() { return m_Registry.create(); }

	void Scene::DestroyEntity(Entity entity) { Entity::Destroy(entity); }
	void Scene::DestroyEntity(EntityHandle nativeEntity) { m_Registry.destroy(nativeEntity); }

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
	}

	void Scene::OnRigidBody2DComponentConstruct(entt::registry& registry, EntityHandle entity)
	{
		bool isEnabled = !registry.all_of<DisabledTag>(entity);
		Rigidbody2DComponent& rb2D = registry.get<Rigidbody2DComponent>(entity);

		// Info: Check if there is any 2d Collider attached to the entity
		// Get the body of the Collider and set it to Dynamic
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

		if (HasAnyComponent<BoxCollider2DComponent>(entity)) {
			rb2D.SetBodyType(BodyType::Static);
		}
		else {
			rb2D.Destroy();
		}
	}

	void Scene::OnBoxCollider2DComponentConstruct(entt::registry& registry, EntityHandle entity) {
		bool isEnabled = !registry.all_of<DisabledTag>(entity);
		BoxCollider2DComponent& boxCollider = GetComponent<BoxCollider2DComponent>(entity);

		// Info: Check if the collider already has the Rigidbody2D component
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
		boxCollider2D.Destroy();
	}

	void Scene::OnCamera2DComponentConstruct(entt::registry& registry, EntityHandle entity)
	{
		Camera2DComponent& camera2D = GetComponent<Camera2DComponent>(entity);
		camera2D.Initialize(GetComponent<Transform2DComponent>(entity));
		camera2D.UpdateViewport();
	}

	void Scene::OnCamera2DComponentDestruct(entt::registry& registry, EntityHandle entity)
	{
		Camera2DComponent& camera2D = GetComponent<Camera2DComponent>(entity);
		camera2D.Destroy();
	}

	void Scene::OnParticleSystem2DComponentConstruct(entt::registry& registry, EntityHandle entity) {
		auto& ps = registry.get<ParticleSystem2DComponent>(entity);
		ps.m_EmitterTransform = &GetComponent<Transform2DComponent>(entity);
	}
}