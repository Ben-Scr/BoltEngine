#include <chrono>
#include <filesystem>
#include <iostream>
#include "Collections/AABB.hpp"

#include "Core/Application.hpp"
#include "GameSystem.hpp"
#include "Scene/Scene.hpp"
#include "Scene/SceneManager.hpp"
#include "Systems/ParticleUpdateSystem.hpp"
#include "Utils/CommandBuffer.hpp"
#include "Debugging/Logger.hpp"


int main() {
	Bolt::SceneDefinition& def = Bolt::SceneManager::RegisterScene("Game");
	def.AddSystem<Bolt::GameSystem>();
	def.AddSystem<Bolt::ParticleUpdateSystem>();
	Bolt::Scene scene = Bolt::SceneManager::LoadScene("Game");

	scene.IsLoaded();

	Bolt::Application::SetRunInBackground(false);
	Bolt::Application::SetForceSingleInstance(true);
	Bolt::Application app;
	app.Run();
	return 0;
}