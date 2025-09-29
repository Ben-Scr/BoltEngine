#include <iostream>
#include "Core/Application.hpp"
#include <chrono>
#include <filesystem>

std::filesystem::path getAssetsPath() {
	auto exePath = std::filesystem::current_path();
	return exePath / "Assets";
}


int main() {
	Bolt::Application app{};
	app.Run();
	return 0;
}