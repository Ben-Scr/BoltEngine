#include <Bolt.hpp>
#include "Scene/SceneDefinition.hpp"
#include "Scene/SceneManager.hpp"
#include "Scene/Scene.hpp"
#include "Components/Components.hpp"

#include "Systems/ImGuiDebugSystem.hpp"
#include "GameSystem.hpp"

#include <iostream>

#include <BoltPhys/Body.hpp>
#include <BoltPhys/Collider.hpp>
#include <BoltPhys/PhysicsWorld.hpp>
#include <BoltPhys/StaticBody.hpp>
#include <BoltPhys/BoxCollider.hpp>

using namespace Bolt;
using namespace BoltPhys;


class Sandbox : public Bolt::Application {
public:
	PhysicsWorld world;

	void ConfigureScenes() override {
		Bolt::SceneDefinition& def = GetSceneManager()->RegisterScene("Game");
		def.AddSystem<GameSystem>();
		def.AddSystem<ImGuiDebugSystem>();
		def.SetAsStartupScene();
	}

	~Sandbox() override = default;

	void Start() override {
		BoltPhys::DynamicBody player;
		BoltPhys::BoxCollider playerCollider({ 0.5f, 1.0f });

		world.RegisterBody(player);
		world.RegisterCollider(playerCollider);
		world.AttachCollider(player, playerCollider);

		world.Step(1.0f / 60.0f);
	}
	void Update() override {
		
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