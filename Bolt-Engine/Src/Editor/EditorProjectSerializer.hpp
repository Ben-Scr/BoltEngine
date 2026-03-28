#pragma once

#include <filesystem>
#include <string>

#include "Scene/Scene.hpp"

namespace Bolt {
	bool SaveSceneToFile(const Scene& scene, const std::string& path, std::string& outError);
	bool LoadSceneFromFile(Scene& scene, const std::filesystem::path& projectPath, std::string& outError);
}