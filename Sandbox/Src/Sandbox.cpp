#include <Bolt.hpp>
#include "Scene/SceneDefinition.hpp"
#include "Scene/SceneManager.hpp"
#include "Scene/Scene.hpp"
#include "Components/Components.hpp"
#include "GameSystem.hpp"

#include <iostream>

using namespace Bolt;

class Sandbox : public Bolt::Application {
public:
	Sandbox() {
		Bolt::SceneDefinition& def = Bolt::SceneManager::RegisterScene("Game");
		//def.AddSystem<GameSystem>();
	}

	void Start() override {
		
	}
	void Update() override {
		
	}
	void OnPaused() override {

	}
	void BeforeQuit() override {

	}
};


Bolt::Application* Bolt::CreateApplication() {
	return new Sandbox();
}