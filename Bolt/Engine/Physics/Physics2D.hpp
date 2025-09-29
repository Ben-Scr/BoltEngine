#pragma once
#include "../Collections/Vec2.hpp"
#include "../Scene/EntityHandle.hpp"

namespace Bolt {
	class Rigidbody2D;
	class Collider2D;
}

namespace Bolt {
	struct RaycastHit2D {
		EntityHandle entity;
		Vec2 point;
		Vec2 normal;
		float distance;
	};

	enum class OverlapMode {
		First,
		Nearest
	};

	class Physics2D {
	public:
		static std::optional<EntityHandle> OverlapCircle(const Vec2& center, float radius, OverlapMode mode);
		static std::optional<EntityHandle> OverlapBox(const Vec2& center, const Vec2& halfExtents, float degrees, OverlapMode mode);
		static std::optional<RaycastHit2D> Raycast(const Vec2& origin, Vec2& direction, float maxDistance);
		static std::vector<EntityHandle> OverlapCircleAll(const Vec2& center, float radius);
		static std::vector<EntityHandle> overlapBoxAll(const Vec2& center, const Vec2& halfExtents, float degrees);
	};
}