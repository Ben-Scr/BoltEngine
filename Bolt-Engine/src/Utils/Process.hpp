#pragma once

#include "Core/Export.hpp"

#include <filesystem>
#include <string>
#include <vector>

namespace Bolt::Process {

	struct BOLT_API Result {
		int ExitCode = -1;
		std::string Output;

		bool Succeeded() const { return ExitCode == 0; }
	};

	BOLT_API Result Run(const std::vector<std::string>& command,
		const std::filesystem::path& workingDirectory = {});

	BOLT_API bool LaunchDetached(const std::vector<std::string>& command,
		const std::filesystem::path& workingDirectory = {});

} // namespace Bolt::Process
