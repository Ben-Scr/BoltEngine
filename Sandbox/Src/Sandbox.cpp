#include <Bolt.hpp>
#include "GameSystem.hpp"
#include "Systems/ParticleUpdateSystem.hpp"
#include "ImGuiDebugSystem.hpp"
#include "Core/Memory.hpp"

#include <iostream>
#include <string>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdint>
#include <cstdio>
#include <vector>
#include <algorithm>

int main() {
	Bolt::SceneDefinition& def = Bolt::SceneManager::RegisterScene("Game");
	def.AddSystem<GameSystem>();
	def.AddSystem<Bolt::ParticleUpdateSystem>();
	def.AddSystem<Bolt::ImGuiDebugSystem>();

	Bolt::Application::SetRunInBackground(true);
	Bolt::Application::SetForceSingleInstance(true);
	Bolt::Application app;

	runapp:
	app.Run();

	std::cout << "Do you want to quit? (Y/N)\n";

	std::string input = "";
	size_t loopCount = 0;
	while (input != "y" && input != "n") {
		if (loopCount == 5) {
			std::cout << "Input entered wrong for the 5th time, last chance before terminating" << '\n';
		}
		if (loopCount > 5)
			break;

		std::cin >> input;
		std::transform(input.begin(), input.end(), input.begin(), [](unsigned char c) { return std::tolower(c); });
		++loopCount;
	}

	if (input == "n") {
		goto runapp;
	}
	std::cout << "Terminating...\n";
	return 0;
}