#include "pch.hpp"

#include "Core/Assert.hpp"

#include <cstdlib>

namespace Bolt {

	void ReportAssertionFailure(const char* kind, const char* expression, const std::string_view message, const std::source_location location) {
		const std::string formatted = fmt::format(
			"{} failed: ({}) | {} @ {}:{} ({})",
			kind,
			expression,
			message,
			location.file_name(),
			location.line(),
			location.function_name());

		BT_CORE_ERROR("{}", formatted);

#ifdef BT_DEBUG
		BT_DEBUG_BREAK;
#endif
		std::terminate();
	}

}