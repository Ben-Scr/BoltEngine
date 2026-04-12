#pragma once

#include <string>

namespace Bolt {

	struct PackageOperationResult {
		bool Success = false;
		std::string Message;
	};

} // namespace Bolt
