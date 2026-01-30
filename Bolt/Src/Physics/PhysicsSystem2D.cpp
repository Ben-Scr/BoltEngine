#include "pch.hpp"
#include "Physics/PhysicsSystem2D.hpp"
#include "Components/Rigidbody2D.hpp"
#include "Physics/Box2DWorld.hpp"
#include "Scene/SceneManager.hpp"
#include "Scene/Scene.hpp"


namespace Bolt {
	bool PhysicsSystem2D::s_IsEnabled = true;
	Box2DWorld PhysicsSystem2D::s_MainWorld;

	void PhysicsSystem2D::Initialize() {
		s_MainWorld = {};
	}

	void PhysicsSystem2D::FixedUpdate(float dt) {
		if (!s_IsEnabled) return;
        
		s_MainWorld.Step(dt);
		s_MainWorld.GetDispatcher().Process(s_MainWorld.GetWorldID());

		for (auto& scene : SceneManager::s_LoadedScenes)
		{
			for (auto [ent, rb, tf] : scene->GetRegistry().view<Rigidbody2D, Transform2D>(entt::exclude<DisabledTag>).each()) {
				tf.Position = rb.GetPosition();
				tf.Rotation = rb.GetRotation();
			}
		}
	}

	void PhysicsSystem2D::Shutdown() {
		s_MainWorld.Destroy();
	}
}