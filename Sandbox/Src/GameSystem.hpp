#pragma once
#include "Bolt.hpp"

using namespace Bolt;

class GameSystem : public ISystem {
public:
	virtual void Awake();
	virtual void Start();
	virtual void Update();
	virtual void OnApplicationPaused();

	Entity CreatePhysicsEntity(Scene& scene, Transform2D transform, BodyType bodyType, Color color = Color::White()) {
		Entity blockEntity = scene.CreateEntity();
		SpriteRenderer& sp = blockEntity.AddComponent<SpriteRenderer>();
		sp.TextureHandle = m_AsteriodTexture;
		sp.Color = color;
		auto& tr = blockEntity.GetComponent<Transform2D>();
		tr = transform;

		auto& boxCollider = blockEntity.AddComponent<BoxCollider2D>();
		auto& rb2D = blockEntity.AddComponent<Rigidbody2D>();
		rb2D.SetBodyType(bodyType);
		return blockEntity;
	}

	void UpdatePlayerPts();
	void PlayerMovement();
	void CameraMovement();
	void MoveEntities();
	void DrawGizmos();
private:
	Entity m_CameraEntity{ Entity::Null };
	Entity m_PlayerEntity{ Entity::Null };

	Entity m_PlayerEmissionPts{ Entity::Null };
	Entity m_PlayerExplosionPts{ Entity::Null };


	TextureHandle m_PlayerTexture;
	TextureHandle m_AsteriodTexture;
	TextureHandle m_LaserTexture;
	TextureHandle m_LightTexture;

	float m_CameraYaw{ 90.0f };
	float m_CameraPitch{ 0.0f };
	bool m_ResetMouseRotation{ true };
};