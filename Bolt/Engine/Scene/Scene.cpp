#include "../pch.hpp"
#include "../Scene/Scene.hpp"
#include "../Scene/Entity.hpp"

#include "../Audio/AudioSource.hpp"

#include "../Components/Rigidbody2D.hpp"
#include "../Components/BoxCollider2D.hpp"
#include "../Components/ParticleSystem2D.hpp"
#include "../Components/SpriteRenderer.hpp"

#include "../Physics/Box2DWorld.hpp"

#include "../Graphics/Camera2D.hpp"

namespace Bolt {
	
	// Note: Creates an entity with a Transform2D component
	Entity Scene::CreateEntity()  {
		auto entityHandle = CreateEntityHandle();
		AddComponent<Transform2D>(entityHandle);
		return Entity(entityHandle, m_Registry);
	}

	// Note: Creates an entity with a Camera2D component
	Entity Scene::CreateCamera() {
		Entity entity = CreateEntity();
		Camera2D& camera2D = entity.AddComponent<Camera2D>();
		return entity;
	}

	// Note: Creates an entity with a Transform2D and SpriteRenderer component
	Entity Scene::CreateRenderableEntity() {
		Entity entity = CreateEntity();
		entity.AddComponent<SpriteRenderer>();
		return entity;
	}

	EntityHandle Scene::CreateEntityHandle() { return m_Registry.create(); }

	void Scene::DestroyEntity(Entity entity) { Entity::Destroy(entity); }
	void Scene::DestroyEntity(EntityHandle nativeEntity) { m_Registry.destroy(nativeEntity); }

	void Scene::AwakeSystems() {
		for (auto& s : m_Systems)
		{
			try {
				s->Awake();
			}
			catch (const std::runtime_error& e) {
				Logger::Error(e.what());
			}
		}
	}

	void Scene::StartSystems() {
		for (auto& s : m_Systems) {
			try {
				s->Start();
			}
			catch (const std::runtime_error& e) {
				Logger::Error(e.what());
			}
		}
	}

	void Scene::UpdateSystems() {
		for (auto& s : m_Systems)
		{
			if (s->m_Enabled)
			{
				try {
					s->Update();
				}
				catch (std::runtime_error e) {
					Logger::Error(e.what());
				}
			}
		}
	}

	void Scene::OnApplicationPausedSystems() {
		for (auto& s : m_Systems)
		{
			if (s->m_Enabled)
			{
				try {
					s->OnApplicationPaused();
				}
				catch (std::runtime_error e) {
					Logger::Error(e.what());
				}
			}
		}
	}

	void Scene::FixedUpdateSystems() {
		for (auto& s : m_Systems)
		{
			if (s->m_Enabled) {
				try {
					s->FixedUpdate();
				}
				catch (const std::runtime_error& e) {
					Logger::Error(e.what());
				}
			}
		}
	}

	void Scene::DestroyScene() {
		for (auto& s : m_Systems)
		{
			try
			{
				s->OnDestroy();
			}
			catch (const std::runtime_error& e) {
				Logger::Error(e.what());
			}
		}
	}

	Scene::Scene(const std::string& name, const SceneDefinition* definition, bool IsPersistent)
		: m_Name(name)
		, m_Definition(definition)
		, m_Persistent(IsPersistent)
		, m_IsLoaded(false) {

		m_Registry.on_construct<Rigidbody2D>().connect<&Scene::OnRigidBody2DComponentConstruct>(this);
		m_Registry.on_construct<BoxCollider2D>().connect<&Scene::OnBoxCollider2DComponentConstruct>(this);
		m_Registry.on_construct<Camera2D>().connect<&Scene::OnCamera2DComponentConstruct>(this);
		m_Registry.on_construct<ParticleSystem2D>().connect<&Scene::OnParticleSystem2DComponentConstruct>(this);

		m_Registry.on_destroy<Rigidbody2D>().connect<&Scene::OnRigidBody2DComponentDestroy>(this);
		m_Registry.on_destroy<BoxCollider2D>().connect<&Scene::OnBoxCollider2DComponentDestroy>(this);
		m_Registry.on_destroy<Camera2D>().connect<&Scene::OnCamera2DComponentDestruct>(this);
	}

	void Scene::OnRigidBody2DComponentConstruct(entt::registry& registry, EntityHandle entity)
	{
		bool isEnabled = !registry.all_of<DisabledTag>(entity);
		Rigidbody2D& rb2D = registry.get<Rigidbody2D>(entity);

		// Info: Check if there is any 2d Collider attached to the entity
		// Get the body of the Collider and set it to Dynamic
		if (HasAnyComponent<BoxCollider2D>(entity)) {
			rb2D.m_BodyId = GetComponent<BoxCollider2D>(entity).m_BodyId;
			rb2D.SetBodyType(BodyType::Dynamic);
		}
		else {
			rb2D.m_BodyId = PhysicsSystem2D::GetMainPhysicsWorld().CreateBody(entity, *this, BodyType::Dynamic);
		}

		rb2D.SetEnabled(isEnabled);
	}

	void Scene::OnRigidBody2DComponentDestroy(entt::registry& registry, EntityHandle entity) {
		auto& rb2D = GetComponent<Rigidbody2D>(entity);

		if (!rb2D.IsValid()) return;

		if (HasAnyComponent<BoxCollider2D>(entity)) {
			rb2D.SetBodyType(BodyType::Static);
		}
		else {
			rb2D.Destroy();
		}
	}

	void Scene::OnBoxCollider2DComponentConstruct(entt::registry& registry, EntityHandle entity) {
		bool isEnabled = !registry.all_of<DisabledTag>(entity);
		BoxCollider2D& boxCollider = GetComponent<BoxCollider2D>(entity);

		// Info: Check if the collider already has the Rigidbody2D component
		// Get the body and assign it to the boxCollider
		// Else Create a new Static body
		if (HasComponent<Rigidbody2D>(entity)) {
			auto& rb = GetComponent<Rigidbody2D>(entity);
			boxCollider.m_BodyId = rb.GetBodyHandle();
		}
		else {
			boxCollider.m_BodyId = PhysicsSystem2D::GetMainPhysicsWorld().CreateBody(entity, *this, BodyType::Static);
		}

		boxCollider.m_ShapeId = PhysicsSystem2D::GetMainPhysicsWorld().CreateShape(entity, *this, boxCollider.m_BodyId, ShapeType::Square);
		boxCollider.SetEnabled(isEnabled);
	}

	void Scene::OnBoxCollider2DComponentDestroy(entt::registry& registry, EntityHandle entity) {
		auto& boxCollider2D = GetComponent<BoxCollider2D>(entity);
		boxCollider2D.Destroy();
	}

	void Scene::OnCamera2DComponentConstruct(entt::registry& registry, EntityHandle entity)
	{
		Camera2D& camera2D = GetComponent<Camera2D>(entity);
		camera2D.Init(GetComponent<Transform2D>(entity));
		camera2D.UpdateViewport();
	}

	void Scene::OnCamera2DComponentDestruct(entt::registry& registry, EntityHandle entity)
	{
		Camera2D& camera2D = GetComponent<Camera2D>(entity);
		camera2D.Destroy();
	}

	void Scene::OnParticleSystem2DComponentConstruct(entt::registry& registry, EntityHandle entity) {
		auto& ps = registry.get<ParticleSystem2D>(entity);
		ps.m_EmitterTransform = &GetComponent<Transform2D>(entity);
	}
}