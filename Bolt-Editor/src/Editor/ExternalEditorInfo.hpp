#pragma once

#include "Editor/ExternalEditorType.hpp"

#include <string>

namespace Bolt {

	struct ExternalEditorInfo {
		ExternalEditorType Type = ExternalEditorType::Auto;
		std::string DisplayName;
		std::string ExecutablePath;
		bool Available = false;
	};

} // namespace Bolt
