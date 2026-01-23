#include "../pch.hpp"
#include "../Components/Collider2D.hpp"


namespace Bolt {
	bool Collider2D::IsValid() { return b2Body_IsValid(m_BodyId) && b2Shape_IsValid(m_ShapeId); }
	void Collider2D::SetFriction(float friction) { b2Shape_SetFriction(m_ShapeId, friction); }
	void Collider2D::SetBounciness(float bounciness) { b2Shape_SetRestitution(m_ShapeId, bounciness); }
	void Collider2D::SetLayer(uint64_t layer) {
		b2Filter filter = b2Shape_GetFilter(m_ShapeId);
		filter.maskBits = layer;
		b2Shape_SetFilter(m_ShapeId, filter);
	}

	Vec2 Collider2D::GetBodyPosition() {
		b2Vec2 position = b2Body_GetPosition(m_BodyId);
		return { position.x, position.y };
	}
	float Collider2D::GetRotationDegrees() {
		return Degrees<float>(b2Rot_GetAngle(b2Body_GetRotation(m_BodyId)));
	}
	float Collider2D::GetRotationRadiant() {
		return  b2Rot_GetAngle(b2Body_GetRotation(m_BodyId));
	}

	void Collider2D::SetRegisterContacts(bool enabled) {
		b2Shape_EnableContactEvents(m_ShapeId, enabled);
	}
	bool Collider2D::CanRegisterContacts() { return b2Shape_AreContactEventsEnabled(m_ShapeId); }


	void Collider2D::EnableRotation(bool enabled) {
		b2Shape_SetDensity(m_ShapeId, enabled ? 1.f : 0.f, false);
	}


	void Collider2D::Destroy() {
		if (IsValid()) {
			b2DestroyBody(m_BodyId);
		}
	}

	void Collider2D::SetRotation(float radiant) {
		b2Body_SetTransform(m_BodyId, b2Body_GetPosition(m_BodyId), b2Rot(std::cos(radiant), std::sin(radiant)));
	}
	void Collider2D::SetPositionRotation(const Vec2& position, float radiant) {
		b2Body_SetTransform(m_BodyId, b2Vec2(position.x, position.y), b2Rot(std::cos(radiant), std::sin(radiant)));
	}
	void Collider2D::SetPosition(const Vec2& position) {
		b2Body_SetTransform(m_BodyId, b2Vec2(position.x, position.y), b2Body_GetRotation(m_BodyId));
	}
	void Collider2D::SetTransform(const Transform2D& tr) {
		b2Body_SetTransform(m_BodyId, b2Vec2(tr.Position.x, tr.Position.y), tr.GetB2Rotation());
	}
}