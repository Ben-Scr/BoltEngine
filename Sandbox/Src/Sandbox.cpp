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
#include <BoltPhys/BoxCollider.hpp>

#include <Core/Time.hpp>

using namespace Bolt;
using namespace BoltPhys;


class Sandbox : public Bolt::Application {
public:
	PhysicsWorld world;
	BoltPhys::DynamicBody player;
	Entity playerEntity{ Entity::Null };

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

		BoltPhys::BoxCollider playerCollider({ 0.5f, 1.0f });
		BoltPhys::BoxCollider obstacleCollider({ 10.f, 1.0f });

		obstacle.SetPosition(BoltPhys::Vec2(0, -5.0f));

		world.RegisterBody(obstacle);
		world.RegisterCollider(obstacleCollider);
		world.AttachCollider(obstacle, obstacleCollider);


		obstacleEnt.GetComponent<Transform2DComponent>().Position = Bolt::Vec2(0, -5.0f);

		world.RegisterBody(player);
		world.RegisterCollider(playerCollider);
		world.AttachCollider(player, playerCollider);
	}
	void Update() override {
		world.Step(Bolt::Time::GetFixedDeltaTime());
		auto position = player.GetPosition();
		playerEntity.GetComponent<Transform2DComponent>().Position = Bolt::Vec2(position.x, position.y);
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