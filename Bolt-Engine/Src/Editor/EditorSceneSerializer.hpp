#pragma once

#include <filesystem>
#include <string>

namespace Bolt {
	class Scene;

	struct EditorSceneSaveResult {
		bool Succeeded = false;
		std::filesystem::path FilePath;
		std::string Message;
	};

	class EditorSceneSerializer {
	public:
		static EditorSceneSaveResult Save(const Scene& scene);
	};
}