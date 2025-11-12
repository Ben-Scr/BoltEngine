#pragma once
#include "Bolt.hpp"

namespace Bolt {

	class GameSystem : public ISystem {
	public:
		virtual void Awake(Scene& scene);
		virtual void Start(Scene& scene);
		virtual void Update(Scene& scene);
		void OnCollisionEnter(const Collision2D& collision);

	private:
		Entity m_CameraEntity{ Entity::Null };
		Entity m_PlayerEntity{ Entity::Null };

		float m_CameraYaw{ 90.0f };
		float m_CameraPitch{ 0.0f };
		bool m_ResetMouseRotation{ true };
	};
}