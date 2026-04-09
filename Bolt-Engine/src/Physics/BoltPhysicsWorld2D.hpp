#pragma once
#include "Core/Export.hpp"
#include "Scene/EntityHandle.hpp"
#include "Collections/Vec2.hpp"

#include <PhysicsWorld.hpp>
#include <WorldSettings.hpp>
#include <Contact.hpp>
#include <BoxCollider.hpp>
#include <CircleCollider.hpp>

#include <unordered_map>
#include <functional>
#include <vector>

namespace Bolt {

	struct BoltContact2D {
		EntityHandle EntityA = entt::null;
		EntityHandle EntityB = entt::null;
		Vec2 Normal{};
		float Penetration = 0.0f;
	};

	using BoltContactCallback = std::function<void(const BoltContact2D&)>;

	/// Engine-level wrapper around the Bolt-Physics PhysicsWorld.
	/// Manages Body/Collider ownership and provides per-entity contact callbacks.
	class BOLT_API BoltPhysicsWorld2D {
	public:
		BoltPhysicsWorld2D();
		explicit BoltPhysicsWorld2D(const BoltPhys::WorldSettings& settings);

		void Step(float dt);
		void Destroy();

		BoltPhys::PhysicsWorld& GetWorld() { return m_World; }
		const BoltPhys::PhysicsWorld& GetWorld() const { return m_World; }

		void SetSettings(const BoltPhys::WorldSettings& settings) { m_World.SetSettings(settings); }
		const BoltPhys::WorldSettings& GetSettings() const { return m_World.GetSettings(); }

		// Body registration tied to an entity
		BoltPhys::Body* CreateBody(EntityHandle entity, BoltPhys::BodyType type);
		void DestroyBody(EntityHandle entity);
		BoltPhys::Body* GetBody(EntityHandle entity);

		// Collider attachment
		BoltPhys::BoxCollider* CreateBoxCollider(EntityHandle entity, const Vec2& halfExtents);
		BoltPhys::CircleCollider* CreateCircleCollider(EntityHandle entity, float radius);
		void DestroyCollider(EntityHandle entity);

		// Contact callbacks per entity
		void RegisterContactCallback(EntityHandle entity, BoltContactCallback callback);
		void UnregisterContactCallback(EntityHandle entity);

		size_t GetBodyCount() const { return m_World.GetBodyCount(); }

	private:
		void DispatchContacts();

		BoltPhys::PhysicsWorld m_World;

		// Ownership: bodies and colliders keyed by entity
		std::unordered_map<uint32_t, std::unique_ptr<BoltPhys::Body>> m_Bodies;
		std::unordered_map<uint32_t, std::unique_ptr<BoltPhys::Collider>> m_Colliders;

		// Entity lookup from Body pointer (reverse map)
		std::unordered_map<BoltPhys::Body*, EntityHandle> m_BodyToEntity;

		// Contact callbacks per entity
		std::unordered_map<uint32_t, BoltContactCallback> m_ContactCallbacks;
	};

}
