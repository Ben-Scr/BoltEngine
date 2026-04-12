#pragma once

#include <string>

namespace Bolt {

	struct DirectoryEntry {
		std::string Path;
		std::string Name;
		bool IsDirectory = false;
	};

} // namespace Bolt
