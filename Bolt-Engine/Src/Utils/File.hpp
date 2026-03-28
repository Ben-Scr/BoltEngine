#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace Bolt {
	class File {
	public:
		File() = delete;

		static bool Exists(const std::string& path);
		static std::vector<std::string> GetAllFiles(const std::string& dir);

		static std::filesystem::path GetHomeDirectory();
		static std::filesystem::path GetBoltRootDirectory();
		static std::filesystem::path GetEditorProjectsDirectory();
		static std::filesystem::path GetRuntimeProjectsDirectory();
		static std::filesystem::path GetRuntimeProjectDirectory(const std::string& projectName);

		static std::string SanitizeProjectName(const std::string& name);
		static std::filesystem::path NormalizeProjectFilePath(const std::filesystem::path& path);
	};
}
