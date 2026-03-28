#include "pch.hpp"
#include "File.hpp"

#include <algorithm>
#include <cstdlib>

namespace Bolt {
	bool File::Exists(const std::string& path) {
		return std::filesystem::exists(path);
	}

	std::vector<std::string> File::GetAllFiles(const std::string& dir) {
		std::vector<std::string> files;
		std::error_code ec;

		const std::filesystem::path base = std::filesystem::path(dir);
		if (!std::filesystem::exists(base, ec) || ec) return {};
		if (!std::filesystem::is_directory(base, ec) || ec) return {};

		for (std::filesystem::directory_iterator it(base, ec), end; it != end && !ec; it.increment(ec)) {
			const std::filesystem::directory_entry& entry = *it;
			std::error_code ec2;
			if (entry.is_regular_file(ec2) && !ec2) {
				files.push_back(entry.path().string());
			}
		}

		std::sort(files.begin(), files.end());
		return files;
	}

	std::filesystem::path File::GetHomeDirectory() {
#if defined(_WIN32)
		if (const char* userProfile = std::getenv("USERPROFILE")) {
			return std::filesystem::path(userProfile);
		}
		const char* homeDrive = std::getenv("HOMEDRIVE");
		const char* homePath = std::getenv("HOMEPATH");
		if (homeDrive && homePath) {
			return std::filesystem::path(std::string(homeDrive) + std::string(homePath));
		}
#else
		if (const char* home = std::getenv("HOME")) {
			return std::filesystem::path(home);
		}
#endif
		return std::filesystem::current_path();
	}

	std::filesystem::path File::GetBoltRootDirectory() {
		return GetHomeDirectory() / "Bolt";
	}

	std::filesystem::path File::GetEditorProjectsDirectory() {
		return std::filesystem::path("Bolt-Editor") / "Projects";
	}

	std::filesystem::path File::GetRuntimeProjectsDirectory() {
		return GetBoltRootDirectory() / "Projects";
	}

	std::filesystem::path File::GetRuntimeProjectDirectory(const std::string& projectName) {
		return GetRuntimeProjectsDirectory() / SanitizeProjectName(projectName);
	}

	std::string File::SanitizeProjectName(const std::string& name) {
		if (name.empty()) {
			return "BoltProject";
		}

		std::string sanitized = name;
		std::replace(sanitized.begin(), sanitized.end(), ' ', '_');
		return sanitized;
	}

	std::filesystem::path File::NormalizeProjectFilePath(const std::filesystem::path& path) {
		if (path.empty()) {
			return path;
		}

		if (path.extension() == ".boltproject.json") {
			return path;
		}

		if (!path.has_extension()) {
			std::filesystem::path normalized = path;
			normalized += ".boltproject.json";
			return normalized;
		}

		if (path.extension() == ".json") {
			return path;
		}

		std::filesystem::path normalized = path;
		normalized += ".boltproject.json";
		return normalized;
	}
}