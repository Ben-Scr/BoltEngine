#include <pch.hpp>
#include "Directory.hpp"
#include <filesystem>

namespace Bolt {
	void Directory::Create(const std::string& dir, bool recursive){
		if(recursive)
			std::filesystem::create_directories(dir);
		else
			std::filesystem::create_directory(dir);
	}

	std::vector<std::string> Directory::GetAllFiles(const std::string& dir) {
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
}