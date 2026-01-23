#include "../pch.hpp"

#include "../Physics/Physics2D.hpp"
#include "../Physics/PhysicsSystem.hpp"
#include "../Physics/Box2DWorld.hpp"

#include <box2d/box2d.h>
#include <box2d/types.h>
#include <box2d/collision.h>

namespace Bolt {
	std::optional<EntityHandle> Physics2D::OverlapCircle(const Vec2& center, float radius, OverlapMode mode) {
		auto& phys = PhysicsSystem2D::GetMainPhysicsWorld();
		b2WorldId world = phys.m_WorldId;

		b2ShapeProxy proxy{};
		proxy.count = 1;
		proxy.points[0] = { center.x, center.y };
		proxy.radius = radius;

		b2QueryFilter filter = b2DefaultQueryFilter();

		struct Qb {
			Vec2 ctr;
			OverlapMode mode;
			std::optional<entt::entity> first;
			std::optional<entt::entity> nearest;
			float  bestDist2 = std::numeric_limits<float>::max();

			static bool Report(b2ShapeId sId, void* ctx) {
				auto* self = static_cast<Qb*>(ctx);

				b2BodyId bId = b2Shape_GetBody(sId);
				void* ud = b2Body_GetUserData(bId);
				entt::entity h = static_cast<entt::entity>(reinterpret_cast<uintptr_t>(ud));

				if (self->mode == OverlapMode::First) {
					self->first = h;
					return false;
				}

				b2Transform xf = b2Body_GetTransform(bId);
				float dx = xf.p.x - self->ctr.x;
				float dy = xf.p.y - self->ctr.y;
				float d2 = dx * dx + dy * dy;
				if (d2 < self->bestDist2) {
					self->bestDist2 = d2;
					self->nearest = h;
				}
				return true;
			}
		} qb{ center, mode, std::nullopt, std::nullopt };

		b2World_OverlapShape(world, &proxy, filter, Qb::Report, &qb);

		return (mode == OverlapMode::First ? qb.first : qb.nearest);
	}
	std::optional<EntityHandle> Physics2D::OverlapBox(const Vec2& center, const Vec2& halfExtents, float degrees, OverlapMode mode) {
		auto& phys = PhysicsSystem2D::GetMainPhysicsWorld();
		b2WorldId world = phys.m_WorldId;
		float radians = Radians<float>(degrees);

		Vec2 corners[4] = {
			{ halfExtents.x,  halfExtents.y},
			{-halfExtents.x,  halfExtents.y},
			{-halfExtents.x, -halfExtents.y},
			{ halfExtents.x, -halfExtents.y}
		};
		Mat2 rot = {
			{ Cos(radians), -Sin(radians) },
			{ Sin(radians),  Cos(radians) }
		};

		b2ShapeProxy proxy{};
		proxy.count = 4;
		proxy.radius = 0.0f;
		for (int i = 0; i < 4; ++i) {
			Vec2 w = rot * corners[i] + Vec2{ center.x, center.y };
			proxy.points[i] = { w.x, w.y };
		}

		b2QueryFilter filter = b2DefaultQueryFilter();

		struct Qb {
			Vec2           ctr;
			OverlapMode         mode;
			std::optional<entt::entity> first;
			std::optional<entt::entity> nearest;
			float               bestDist2 = std::numeric_limits<float>::max();

			static bool Report(b2ShapeId sId, void* ctx) {
				auto* self = static_cast<Qb*>(ctx);


				b2BodyId bId = b2Shape_GetBody(sId);
				void* ud = b2Body_GetUserData(bId);
				entt::entity h = static_cast<entt::entity>(reinterpret_cast<uintptr_t>(ud));

				if (self->mode == OverlapMode::First) {
					self->first = h;
					return false;
				}

				b2Transform xf = b2Body_GetTransform(bId);
				float dx = xf.p.x - self->ctr.x;
				float dy = xf.p.y - self->ctr.y;
				float d2 = dx * dx + dy * dy;
				if (d2 < self->bestDist2) {
					self->bestDist2 = d2;
					self->nearest = h;
				}
				return true;
			}
		} qb{ center, mode, std::nullopt, std::nullopt };

		b2World_OverlapShape(world, &proxy, filter, Qb::Report, &qb);

		return (mode == OverlapMode::First ? qb.first : qb.nearest);
	}
	std::optional<RaycastHit2D> Physics2D::Raycast(const Vec2& origin,const Vec2& direction, float maxDistance) {
		auto& phys = PhysicsSystem2D::GetMainPhysicsWorld();
		b2WorldId world = phys.m_WorldId;

		b2Vec2 o{ origin.x, origin.y };
		Vec2 nd = Normalized(direction);
		b2Vec2 t{ nd.x * maxDistance, nd.y * maxDistance };

		b2QueryFilter filter = b2DefaultQueryFilter();

		b2RayResult r = b2World_CastRayClosest(world, o, t, filter);

		if (!b2Shape_IsValid(r.shapeId))
			return std::nullopt;

		b2BodyId   bId = b2Shape_GetBody(r.shapeId);
		void* ud = b2Body_GetUserData(bId);
		uintptr_t i = reinterpret_cast<uintptr_t>(ud);
		EntityHandle e = static_cast<EntityHandle>(i);

		RaycastHit2D hit;
		hit.entity = e;
		hit.point = { r.point.x, r.point.y };
		hit.normal = { r.normal.x, r.normal.y };
		hit.distance = r.fraction * maxDistance;
		return hit;
	}

	std::vector<EntityHandle> Physics2D::OverlapCircleAll(const Vec2& center, float radius) {
		auto& phys = PhysicsSystem2D::GetMainPhysicsWorld();
		b2WorldId world = phys.m_WorldId;

		b2ShapeProxy proxy{};
		proxy.count = 1;
		proxy.points[0] = { center.x, center.y };
		proxy.radius = radius;

		b2QueryFilter filter = b2DefaultQueryFilter();

		std::vector<entt::entity> results;
		struct Cb {
			std::vector<entt::entity>* out;
			static bool Report(b2ShapeId sId, void* ctx) {
				auto* self = static_cast<Cb*>(ctx);
				b2BodyId bId = b2Shape_GetBody(sId);
				void* ud = b2Body_GetUserData(bId);
				entt::entity h = static_cast<entt::entity>(reinterpret_cast<uintptr_t>(ud));
				self->out->push_back(h);
				return true;
			}
		} cb{ &results };

		b2World_OverlapShape(world, &proxy, filter, Cb::Report, &cb);

		return results;
	}
	std::vector<EntityHandle> Physics2D::overlapBoxAll(const Vec2& center, const Vec2& halfExtents, float degrees) {
		auto& phys = PhysicsSystem2D::GetMainPhysicsWorld();
		b2WorldId world = phys.m_WorldId;
		float radians = Radians<float>(degrees);

		Vec2 corners[4] = {
			{ halfExtents.x,  halfExtents.y},
			{-halfExtents.x,  halfExtents.y},
			{-halfExtents.x, -halfExtents.y},
			{ halfExtents.x, -halfExtents.y}
		};
		Mat2 rot = {
			{ Cos(radians), -Sin(radians) },
			{ Sin(radians),  Cos(radians) }
		};

		b2ShapeProxy proxy{};
		proxy.count = 4;
		proxy.radius = 0.0f;
		for (int i = 0; i < 4; ++i) {
			Vec2 w = rot * corners[i] + Vec2{ center.x, center.y };
			proxy.points[i] = { w.x, w.y };
		}

		b2QueryFilter filter = b2DefaultQueryFilter();

		std::vector<entt::entity> results;
		struct Cb {
			std::vector<entt::entity>* out;
			static bool Report(b2ShapeId sId, void* ctx) {
				auto* self = static_cast<Cb*>(ctx);
				b2BodyId bId = b2Shape_GetBody(sId);
				void* ud = b2Body_GetUserData(bId);
				entt::entity h = static_cast<entt::entity>(reinterpret_cast<uintptr_t>(ud));
				self->out->push_back(h);
				return true;
			}
		} cb{ &results };

		b2World_OverlapShape(world, &proxy, filter, Cb::Report, &cb);

		return results;
	}
}