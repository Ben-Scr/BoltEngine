#include "../pch.hpp"
#include <iostream>
#include "Core/Application.hpp"
#include <chrono>
#include <filesystem>
#include "Scene/Scene.hpp"
#include "Scene/SceneManager.hpp"
#include "GameSystem.hpp"

std::filesystem::path getAssetsPath() {
	auto exePath = std::filesystem::current_path();
	return exePath / "Assets";
}


int main() {
	Bolt::SceneDefinition& def = Bolt::SceneManager::RegisterScene("Test");
	def.AddSystem<Bolt::GameSystem>();
	Bolt::Application app{};
	app.Run();
	return 0;
}