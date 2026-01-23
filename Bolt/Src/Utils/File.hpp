#pragma once
#include <string>

namespace Bolt {
	class File {
	public:
		File() = delete;

		static bool Exists(const std::string& path);
		static std::vector<std::string> GetAllFiles(const std::string& dir);
	private:
	};
}