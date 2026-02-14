#include <Bolt.hpp>
#include "Scene/SceneDefinition.hpp"
#include "Scene/SceneManager.hpp"
#include "Scene/Scene.hpp"
#include "Components/Components.hpp"

#include <iostream>

using namespace Bolt;

class Sandbox : public Bolt::Application {
public:
	Sandbox() {
		Bolt::SceneDefinition& def = Bolt::SceneManager::RegisterScene("Game");
	}

	void Start() override {
		Bolt::Scene* activeScene = Bolt::SceneManager::GetActiveScene();
		activeScene->CreateEntityHandle<Transform2DComponent, Camera2DComponent>();
	}
	void Update() override {

	}
	void BeforeQuit() override {

	}
};


Bolt::Application* Bolt::CreateApplication() {
	return new Sandbox();
}