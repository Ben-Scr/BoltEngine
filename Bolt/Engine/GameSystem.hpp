#pragma once
#include "Bolt.hpp"

namespace Bolt {

	class GameSystem : public ISystem {
	public:
		virtual void Awake();
		virtual void Start();
		virtual void Update();

		void OnCollisionEnter(const Collision2D& collision);

		template<typename TTag>
		void CreatePhysicsEntity(Scene& scene, Transform2D transform, BodyType bodyType, Color color = Color::White()) {
			Entity blockEntity = scene.CreateEntity();
			blockEntity.AddComponent<TTag>();
			SpriteRenderer& sp = blockEntity.AddComponent<SpriteRenderer>();
			sp.TextureHandle = blockTex;
			sp.Color = color;
			auto& tr = blockEntity.GetComponent<Transform2D>();
			tr = transform;

			auto& boxCollider = blockEntity.AddComponent<BoxCollider2D>();
			auto& rb2D = blockEntity.AddComponent<Rigidbody2D>();
			rb2D.SetBodyType(bodyType);
		}

		void DrawGizmos();

	private:
		Entity m_CameraEntity{ Entity::Null };
		Entity m_PlayerEntity{ Entity::Null };
		TextureHandle blockTex;

		float m_CameraYaw{ 90.0f };
		float m_CameraPitch{ 0.0f };
		bool m_ResetMouseRotation{ true };
	};
}