#include <Bolt.hpp>
#include "Scene/SceneDefinition.hpp"
#include "Scene/SceneManager.hpp"
#include "Scene/Scene.hpp"
#include "Components/Components.hpp"

#include "Systems/ImGuiDebugSystem.hpp"
#include "GameSystem.hpp"
#include <Scene/EntityHelper.hpp>

#include <iostream>

#include <BoltPhys/Body.hpp>
#include <BoltPhys/Collider.hpp>
#include <BoltPhys/PhysicsWorld.hpp>
#include <BoltPhys/StaticBody.hpp>
#include <BoltPhys/DynamicBody.hpp>
#include <BoltPhys/KinematicBody.hpp>
#include <BoltPhys/BoxCollider.hpp>

#include <Graphics/Gizmos.hpp>

#include <Core/Time.hpp>
#include <Core/Input.hpp>

using namespace Bolt;
using namespace BoltPhys;


class Sandbox : public Bolt::Application {
public:
	PhysicsWorld world;
	BoltPhys::DynamicBody player;
	Entity playerEntity{ Entity::Null };
	BoltPhys::BoxCollider playerCollider = BoltPhys::BoxCollider({ 0.5f, 0.5f });
	BoltPhys::BoxCollider obstacleCollider = BoltPhys::BoxCollider({ 10.f, 0.5f });

	BoltPhys::StaticBody obstacle;

	void ConfigureScenes() override {
		Bolt::SceneDefinition& def = GetSceneManager()->RegisterScene("Game");
		def.AddSystem<GameSystem>();
		def.AddSystem<ImGuiDebugSystem>();
		def.SetAsStartupScene();
	}

	~Sandbox() override = default;

	void Start() override {
		Entity obstacleEnt = Bolt::EntityHelper::CreateWith<SpriteRendererComponent, Transform2DComponent>();

		playerEntity = Bolt::EntityHelper::CreateWith<SpriteRendererComponent, Transform2DComponent>();

		obstacle.SetPosition(BoltPhys::Vec2(0, -5.0f));
		obstacle.SetGravityEnabled(false);

		world.RegisterBody(obstacle);
		world.RegisterCollider(obstacleCollider);
		world.AttachCollider(obstacle, obstacleCollider);

		auto& obstacleTr  = obstacleEnt.GetComponent<Transform2DComponent>();

		obstacleTr.Position = obstacle.GetPosition();
		obstacleTr.Scale = obstacleCollider.GetHalfExtents() * 2.0f;

		world.RegisterBody(player);
		world.RegisterCollider(playerCollider);
		world.AttachCollider(player, playerCollider);
	}
	void Update() override {
		Bolt::Gizmo::DrawSquare(obstacle.GetPosition(), obstacleCollider.GetHalfExtents() * 2.0f, 0);
		playerEntity.GetComponent<Transform2DComponent>().Position = player.GetPosition();

		if(Input::GetAxis() != Zero())
		player.SetVelocity(Input::GetAxis() * 5.0f);
	}

	void FixedUpdate() override {
		world.Step(Bolt::Time::GetFixedDeltaTime());
	}

	void OnPaused() override {

	}
	void OnQuit() override {
		Logger::Message("Quit");
	}
};



Bolt::Application* Bolt::CreateApplication() {
	return new Sandbox();
}