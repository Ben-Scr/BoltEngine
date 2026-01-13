#include <chrono>
#include <filesystem>
#include <iostream>
#include "Collections/AABB.hpp"

#include "Core/Application.hpp"
#include "GameSystem.hpp"
#include "Scene/Scene.hpp"
#include "Scene/SceneManager.hpp"
#include "Utils/CommandBuffer.hpp"
#include "Debugging/Logger.hpp"

int main() {
	Bolt::SceneDefinition& def = Bolt::SceneManager::RegisterScene("Game");
	def.AddSystem<Bolt::GameSystem>();
	Bolt::Application::SetForceSingleInstance(true);
	Bolt::Application app{ "Test" };
	app.Run();
	return 0;
}