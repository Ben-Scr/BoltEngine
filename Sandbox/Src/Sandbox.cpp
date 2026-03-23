#include <Bolt.hpp>
#include "Scene/SceneDefinition.hpp"
#include "Scene/SceneManager.hpp"
#include "Scene/Scene.hpp"
#include "Components/Components.hpp"

#include "Systems/ImGuiDebugSystem.hpp"
#include "GameSystem.hpp"

#include <iostream>

using namespace Bolt;


class Sandbox : public Bolt::Application {
public:
	void ConfigureScenes() override {
		Bolt::SceneDefinition& def = GetSceneManager()->RegisterScene("Game");
		def.AddSystem<GameSystem>();
		def.AddSystem<ImGuiDebugSystem>();
		def.SetAsStartupScene();
	}

	~Sandbox() override = default;

	void Start() override {
		
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