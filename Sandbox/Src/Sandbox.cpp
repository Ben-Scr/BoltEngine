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
#include <BoltPhys/BodyType.hpp>
#include <BoltPhys/Collider.hpp>
#include <BoltPhys/PhysicsWorld.hpp>
#include <BoltPhys/BoxCollider.hpp>

#include <Graphics/Gizmos.hpp>

#include <Core/Time.hpp>
#include <Core/Input.hpp>

using namespace Bolt;
using namespace BoltPhys;


class Sandbox : public Bolt::Application {
public:
	void ConfigureScenes() override {
		Bolt::SceneDefinition& def = GetSceneManager()->RegisterScene("Game");
		def.AddSystem<ImGuiDebugSystem>();
		def.SetAsStartupScene();
	}

	~Sandbox() override = default;

	void Start() override {
	
	}
	void Update() override {
		
	}
	void FixedUpdate() override {

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