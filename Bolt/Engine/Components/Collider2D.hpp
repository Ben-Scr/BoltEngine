#pragma once
#include "Rigidbody2D.hpp"
#include "../Debugging/Logger.hpp"
#include "../Physics/PhysicsSystem.hpp"
#include "../Physics/Box2DWorld.hpp"
#include "../Scene/EntityHandle.hpp"
#include <box2d/box2d.h>



namespace Bolt {
	struct Collision2D;

	class Collider2D {
	public:
		Collider2D() = default;

		bool IsValid();
		void SetFriction(float friction);
		void SetBounciness(float bounciness);
		void SetLayer(uint64_t layer);

		Vec2 GetBodyPosition();
		float GetRotationDegrees();

		void SetRegisterContacts(bool enabled);
		bool CanRegisterContacts();

		template<typename F>
		void OnCollisionEnter(F&& callback) {
			if (!CanRegisterContacts()) {
				Logger::Warning(
					"[BoxCollider] Failed to register OnCollisionEnter event: contact events are not enabled for this shape. "
					"Make sure to call Collider.RegisterContacts(true); before registering event callbacks."
				);
				return;
			}

			ContactBeginCallback cb(callback);
			PhysicsSystem2D::GetMainPhysicsWorld().GetDispatcher().RegisterBegin(m_ShapeId, std::move(cb));
		}
		template<typename F>
		void OnCollisionExit(F&& callback) {
			if (!CanRegisterContacts()) {
				Logger::Warning(
					"[BoxCollider] Failed to register OnCollisionExit event: contact events are not enabled for this shape. "
					"Make sure to call Collider.RegisterContacts(true); before registering event callbacks."
				);
			}

			PhysicsSystem2D::GetMainPhysicsWorld().GetDispatcher().RegisterEnd(m_ShapeId, std::forward<F>(callback));
		}
		template<typename F>
		void OnCollisionHit(F&& callback) {
			if (!CanRegisterContacts()) {
				Logger::Warning(
					"[BoxCollider] Failed to register OnCollisionHit event: contact events are not enabled for this shape. "
					"Make sure to call Collider.RegisterContacts(true); before registering event callbacks."
				);
			}

			PhysicsSystem2D::GetMainPhysicsWorld().GetDispatcher().RegisterHit(m_ShapeId, std::forward<F>(callback));
		}

		void EnableRotation(bool enabled);


		float GetRotationRadiant();

		void Destroy();

		b2BodyId m_BodyId;
		b2ShapeId m_ShapeId{ b2_nullShapeId };

		EntityHandle m_EntityHandle;
	private:
		void SetRotation(float radiant);
		void SetPositionRotation(const Vec2& position, float radiant);
		void SetPosition(const Vec2& position);
		void SetTransform(const Transform2D& tr);

		friend class Physics2D;
	};
}