#include "pch.hpp"
#include "Box2DWorld.hpp"
#include "Scene/Scene.hpp"

namespace Bolt {
	Box2DWorld::Box2DWorld() {
		b2WorldDef def = b2DefaultWorldDef();
		def.enableSleep = true;
		def.workerCount = 4;

		def.gravity = b2Vec2{ 0, -9.8f };
		m_WorldId = b2CreateWorld(&def);
	}
	Box2DWorld::~Box2DWorld() {
		return;
		if (b2World_IsValid(m_WorldId)) {
			b2DestroyWorld(m_WorldId);
		}
	}
	void Box2DWorld::Step(float dt) {
		b2World_Step(m_WorldId, dt, 5);
	}
	void Box2DWorld::Destroy() {
		b2DestroyWorld(m_WorldId);
	}

	b2BodyId Box2DWorld::CreateBody(EntityHandle nativeEntity, Scene& scene, BodyType bodyType) {
		Transform2DComponent& tr = scene.GetComponent<Transform2DComponent>(nativeEntity);

		b2Vec2 box2dPos(tr.Position.x, tr.Position.y);
		b2BodyDef bodyDef = b2DefaultBodyDef();
		bodyDef.type = bodyType == BodyType::Dynamic ? b2_dynamicBody : (bodyType == BodyType::Static ? b2_staticBody : b2_kinematicBody);
		bodyDef.gravityScale = 1.0f;
		bodyDef.position = box2dPos;
		bodyDef.rotation = tr.GetB2Rotation();
		bodyDef.isBullet = true;
		bodyDef.userData = reinterpret_cast<void*>(static_cast<uintptr_t>(nativeEntity));
		bodyDef.linearDamping = 0.1f;

		b2BodyId bodyId = b2CreateBody(m_WorldId, &bodyDef);
		return bodyId;
	}

	b2ShapeId Box2DWorld::CreateShape(EntityHandle nativeEntity, Scene& scene, b2BodyId bodyId, ShapeType shapeType) {
		Transform2DComponent transform = scene.GetComponent<Transform2DComponent>(nativeEntity);
		b2ShapeId shapeId = b2_nullShapeId;

		b2ShapeDef shapeDef = b2DefaultShapeDef();
		shapeDef.density = 1.f;
		shapeDef.material.friction = 0.3f;
		shapeDef.material.restitution = 0.f;

		if (shapeType == ShapeType::Square) {

			b2Polygon b2Polygon = b2MakeBox(0.5f * transform.Scale.x, 0.5f * transform.Scale.y);
			shapeId = b2CreatePolygonShape(bodyId, &shapeDef, &b2Polygon);
		}
		else if (shapeType == ShapeType::Circle)
		{
			float r = 0.25f * (transform.Scale.x + transform.Scale.y);
			b2Circle circle = { b2Vec2{0,0}, r };
			shapeId = b2CreateCircleShape(bodyId, &shapeDef, &circle);
		}


		b2Body_SetTransform(bodyId, b2Vec2(transform.Position.x, transform.Position.y), transform.GetB2Rotation());
		return shapeId;
	}

	CollisionDispatcher& Box2DWorld::GetDispatcher() { return m_Dispatcher; }
}