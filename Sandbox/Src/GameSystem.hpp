#pragma once
#include <Bolt.hpp>
#include <functional>
#include "Components/Components.hpp"
#include "Scene/ISystem.hpp"
#include "Scene/Entity.hpp"
#include "Scene/Scene.hpp"

using namespace Bolt;

class GameSystem : public ISystem {
public:
	virtual void Awake();
	virtual void Start();
	virtual void Update();
	virtual void OnApplicationPaused();

	Entity CreatePhysicsEntity(Scene& scene, Transform2DComponent transform, BodyType bodyType, Color color = Color::White()) {
		Entity blockEntity = scene.CreateEntity();
		SpriteRendererComponent& sp = blockEntity.AddComponent<SpriteRendererComponent>();
		sp.TextureHandle = m_AsteriodTexture;
		sp.Color = color;
		auto& tr = blockEntity.GetComponent<Transform2DComponent>();
		tr = transform;

		auto& boxCollider = blockEntity.AddComponent<BoxCollider2DComponent>();
		auto& rb2D = blockEntity.AddComponent<Rigidbody2DComponent>();
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
	Entity m_RectEntity{ Entity::Null };

	TextureHandle m_PlayerTexture;
	TextureHandle m_AsteriodTexture;
	TextureHandle m_LaserTexture;
	TextureHandle m_LightTexture;

	float m_CameraYaw{ 90.0f };
	float m_CameraPitch{ 0.0f };
	bool m_ResetMouseRotation{ true };
};