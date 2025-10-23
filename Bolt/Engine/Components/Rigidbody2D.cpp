#include "../pch.hpp"
#include "../components/Rigidbody2D.hpp"
#include "../Components/Transform2D.hpp"
#include "../Physics/Box2DWorld.hpp"
#include "../Scene/Scene.hpp"
#include "../Scene/SceneManager.hpp"

namespace Bolt {
	void Rigidbody2D::SetBodyType(BodyType bodyType) {
		switch (bodyType) {
		case BodyType::Static:
			b2Body_SetType(m_BodyId, b2_staticBody);
			break;
		case BodyType::Kinematic:
			b2Body_SetType(m_BodyId, b2_kinematicBody);
			break;
		case BodyType::Dynamic:
			b2Body_SetType(m_BodyId, b2_dynamicBody);
			break;
		}
	}

	bool Rigidbody2D::IsAwake() const {
		return b2Body_IsAwake(m_BodyId);
	}

	void Rigidbody2D::SetVelocity(const Vec2& velocity) { b2Body_SetLinearVelocity(m_BodyId, b2Vec2(velocity.x, velocity.y)); }
	Vec2 Rigidbody2D::GetVelocity() const {
		b2Vec2 b2v = b2Body_GetLinearVelocity(m_BodyId);
		return Vec2{ b2v.x, b2v.y };
	}

	void Rigidbody2D::SetAngularVelocity(float velocity) { b2Body_SetAngularVelocity(m_BodyId, velocity); }
	float Rigidbody2D::GetAngularVelocity() const { return b2Body_GetAngularVelocity(m_BodyId); }

	void Rigidbody2D::SetGravityScale(float gravityScale) { b2Body_SetGravityScale(m_BodyId, gravityScale); }
	float Rigidbody2D::GetGravityScale() const { return b2Body_GetGravityScale(m_BodyId); }

	void Rigidbody2D::SetMass(float mass) {
		auto massData = b2Body_GetMassData(m_BodyId);
		massData.mass = mass;
		b2Body_SetMassData(m_BodyId, massData);
	}
	float Rigidbody2D::GetMass() const { return b2Body_GetMass(m_BodyId); }

	void Rigidbody2D::SetLinearDrag(float value) {
		b2Body_SetLinearDamping(m_BodyId, value);
	}
	void Rigidbody2D::SetAngularDrag(float value) {
		b2Body_SetAngularDamping(m_BodyId, value);
	}


	void Rigidbody2D::SetPosition(const Vec2& position) { b2Body_SetTransform(m_BodyId, b2Vec2(position.x, position.y), b2Body_GetRotation(m_BodyId)); }
	Vec2 Rigidbody2D::GetPosition() const { b2Vec2 b2Pos = IsValid() ? b2Body_GetPosition(m_BodyId) : b2Vec2_zero; return { b2Pos.x, b2Pos.y }; }

	void Rigidbody2D::SetTransform(const Transform2D& tr) { b2Body_SetTransform(m_BodyId, b2Vec2(tr.Position.x, tr.Position.y), tr.GetB2Rotation()); }

	float Rigidbody2D::GetRotation() const { return -b2Rot_GetAngle(b2Body_GetRotation(m_BodyId)); }

	void Rigidbody2D::SetEnabled(bool enabled) {
		if (enabled)
			b2Body_Enable(m_BodyId);
		else
			b2Body_Disable(m_BodyId);
	}

	b2BodyId Rigidbody2D::GetBodyHandle() const {
		if (IsValid())
		{
			return m_BodyId;
		}
	}
	bool Rigidbody2D::IsValid() const {
		if (!b2Body_IsValid(m_BodyId)) {
			return false;
		}

		return true;
	}

	void Rigidbody2D::Destroy() {
		if (IsValid()) {
			b2DestroyBody(m_BodyId);
		}
	}
}