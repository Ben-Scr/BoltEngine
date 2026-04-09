#include "pch.hpp"
#include "Physics/BoltPhysicsWorld2D.hpp"

#include <BoxCollider.hpp>
#include <CircleCollider.hpp>

namespace Bolt {

	BoltPhysicsWorld2D::BoltPhysicsWorld2D() : m_World() {}

	BoltPhysicsWorld2D::BoltPhysicsWorld2D(const BoltPhys::WorldSettings& settings)
		: m_World(settings) {
	}

	void BoltPhysicsWorld2D::Step(float dt) {
		m_World.Step(dt);
		DispatchContacts();
	}

	void BoltPhysicsWorld2D::Destroy() {
		m_ContactCallbacks.clear();
		m_BodyToEntity.clear();

		// Detach colliders and unregister everything before clearing ownership
		for (auto& [key, body] : m_Bodies) {
			m_World.DetachCollider(*body);
			m_World.UnregisterBody(*body);
		}
		for (auto& [key, collider] : m_Colliders) {
			m_World.UnregisterCollider(*collider);
		}

		m_Bodies.clear();
		m_Colliders.clear();
	}

	BoltPhys::Body* BoltPhysicsWorld2D::CreateBody(EntityHandle entity, BoltPhys::BodyType type) {
		uint32_t key = static_cast<uint32_t>(entity);
		if (m_Bodies.count(key)) {
			BT_CORE_WARN_TAG("BoltPhysics", "Entity already has a Bolt-Physics body");
			return m_Bodies[key].get();
		}

		auto body = std::make_unique<BoltPhys::Body>(type);
		BoltPhys::Body* ptr = body.get();
		m_World.RegisterBody(*ptr);
		m_BodyToEntity[ptr] = entity;
		m_Bodies[key] = std::move(body);
		return ptr;
	}

	void BoltPhysicsWorld2D::DestroyBody(EntityHandle entity) {
		uint32_t key = static_cast<uint32_t>(entity);

		// Destroy collider first if exists
		DestroyCollider(entity);

		auto it = m_Bodies.find(key);
		if (it == m_Bodies.end()) return;

		BoltPhys::Body* ptr = it->second.get();
		m_World.UnregisterBody(*ptr);
		m_BodyToEntity.erase(ptr);
		m_Bodies.erase(it);
		m_ContactCallbacks.erase(key);
	}

	BoltPhys::Body* BoltPhysicsWorld2D::GetBody(EntityHandle entity) {
		uint32_t key = static_cast<uint32_t>(entity);
		auto it = m_Bodies.find(key);
		return it != m_Bodies.end() ? it->second.get() : nullptr;
	}

	BoltPhys::BoxCollider* BoltPhysicsWorld2D::CreateBoxCollider(EntityHandle entity, const Vec2& halfExtents) {
		uint32_t key = static_cast<uint32_t>(entity);

		DestroyCollider(entity); // Replace existing

		auto collider = std::make_unique<BoltPhys::BoxCollider>(BoltPhys::Vec2(halfExtents.x, halfExtents.y));
		BoltPhys::BoxCollider* ptr = collider.get();
		m_World.RegisterCollider(*ptr);

		// Attach to body if one exists
		auto bodyIt = m_Bodies.find(key);
		if (bodyIt != m_Bodies.end()) {
			m_World.AttachCollider(*bodyIt->second, *ptr);
		}

		m_Colliders[key] = std::move(collider);
		return ptr;
	}

	BoltPhys::CircleCollider* BoltPhysicsWorld2D::CreateCircleCollider(EntityHandle entity, float radius) {
		uint32_t key = static_cast<uint32_t>(entity);

		DestroyCollider(entity);

		auto collider = std::make_unique<BoltPhys::CircleCollider>(radius);
		BoltPhys::CircleCollider* ptr = collider.get();
		m_World.RegisterCollider(*ptr);

		auto bodyIt = m_Bodies.find(key);
		if (bodyIt != m_Bodies.end()) {
			m_World.AttachCollider(*bodyIt->second, *ptr);
		}

		m_Colliders[key] = std::move(collider);
		return ptr;
	}

	void BoltPhysicsWorld2D::DestroyCollider(EntityHandle entity) {
		uint32_t key = static_cast<uint32_t>(entity);
		auto it = m_Colliders.find(key);
		if (it == m_Colliders.end()) return;

		BoltPhys::Collider* ptr = it->second.get();

		// Detach from body if attached
		auto bodyIt = m_Bodies.find(key);
		if (bodyIt != m_Bodies.end()) {
			m_World.DetachCollider(*bodyIt->second);
		}

		m_World.UnregisterCollider(*ptr);
		m_Colliders.erase(it);
	}

	void BoltPhysicsWorld2D::RegisterContactCallback(EntityHandle entity, BoltContactCallback callback) {
		m_ContactCallbacks[static_cast<uint32_t>(entity)] = std::move(callback);
	}

	void BoltPhysicsWorld2D::UnregisterContactCallback(EntityHandle entity) {
		m_ContactCallbacks.erase(static_cast<uint32_t>(entity));
	}

	void BoltPhysicsWorld2D::DispatchContacts() {
		const auto& contacts = m_World.GetContacts();
		for (const auto& contact : contacts) {
			EntityHandle entityA = entt::null;
			EntityHandle entityB = entt::null;

			auto itA = m_BodyToEntity.find(contact.bodyA);
			auto itB = m_BodyToEntity.find(contact.bodyB);
			if (itA != m_BodyToEntity.end()) entityA = itA->second;
			if (itB != m_BodyToEntity.end()) entityB = itB->second;

			BoltContact2D boltContact{
				entityA, entityB,
				{ contact.normal.x, contact.normal.y },
				contact.penetration
			};

			// Dispatch to both entities
			if (entityA != entt::null) {
				auto cbIt = m_ContactCallbacks.find(static_cast<uint32_t>(entityA));
				if (cbIt != m_ContactCallbacks.end()) cbIt->second(boltContact);
			}
			if (entityB != entt::null) {
				auto cbIt = m_ContactCallbacks.find(static_cast<uint32_t>(entityB));
				if (cbIt != m_ContactCallbacks.end()) cbIt->second(boltContact);
			}
		}
	}

}
