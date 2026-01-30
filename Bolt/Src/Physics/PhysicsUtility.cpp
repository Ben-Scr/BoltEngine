#include "pch.hpp"
#include "Physics/PhysicsUtility.hpp"
#include "Components/Rigidbody2D.hpp"
#include "Components/Collider2D.hpp"

namespace Bolt {
	EntityHandle PhysicsUtility::GetEntityHandleFromCollider(const Collider2D& collider) {
		void* ud = b2Body_GetUserData(collider.m_BodyId);
		return static_cast<EntityHandle>(reinterpret_cast<uintptr_t>(ud));
	}
	EntityHandle PhysicsUtility::GetEntityHandleFromRigidbody(const Rigidbody2D& rb) {
		void* ud = b2Body_GetUserData(rb.GetBodyHandle());
		return static_cast<EntityHandle>(reinterpret_cast<uintptr_t>(ud));
	}
	EntityHandle PhysicsUtility::GetEntityHandleFromBodyId(b2BodyId bodyId) {
		void* ud = b2Body_GetUserData(bodyId);
		return static_cast<EntityHandle>(reinterpret_cast<uintptr_t>(ud));
	}
	EntityHandle PhysicsUtility::GetEntityHandleFromShapeID(b2ShapeId shapeId) {
		void* ud = b2Body_GetUserData(b2Shape_GetBody(shapeId));
		return static_cast<EntityHandle>(reinterpret_cast<uintptr_t>(ud));
	}
} 