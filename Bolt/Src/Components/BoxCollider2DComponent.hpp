#pragma once
#include "Collider2D.hpp"
#include "Rigidbody2DComponent.hpp"
#include "Physics/PhysicsSystem2D.hpp"

#include <box2d/box2d.h>

namespace Bolt {
	class BoxCollider2DComponent : public Collider2D {
		friend class PhysicsSystem2D;
	public:
		BoxCollider2DComponent() = default;

		void SetScale(const Vec2& scale, const Scene& scene);
		void SetEnabled(bool enabled);
		Vec2 GetScale();
		Vec2 GetLocalScale(const Scene& scene);

		// Note: has to be called when the transforms scale has been changed.
		void UpdateScale(const Scene& scene);

		void SetCenter(const Vec2& center, const Scene& scene);
		Vec2 GetCenter();
	private:
	};
}