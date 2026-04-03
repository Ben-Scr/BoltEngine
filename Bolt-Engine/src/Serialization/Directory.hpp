#pragma once
#include <vector>
#include <string>

namespace Bolt {
	class Directory {
		void Create(const std::string& dir, bool recursive = true);
		static std::vector<std::string> GetAllFiles(const std::string& dir);
	};
}