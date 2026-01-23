#pragma once

namespace Bolt {
	class Collider2D;
	class Rigidbody2D;
}

struct b2BodyId;
struct b2ShapeId;

namespace Bolt {
	class PhysicsUtility {
	public:
		static EntityHandle GetEntityHandleFromCollider(const Collider2D& collider);
		static EntityHandle GetEntityHandleFromRigidbody(const Rigidbody2D& rb);
		static EntityHandle GetEntityHandleFromBodyId(b2BodyId bodyId);
		static EntityHandle GetEntityHandleFromShapeID(b2ShapeId shapeId);
	};
}