#include <pch.hpp>
#include "Exceptions.hpp"
#include <Windows.h>
#include <cstdlib>

namespace Bolt {
	[[noreturn]] void ThrowError(BoltErrorCode errorCode, const std::string& msg, const std::source_location loc) {
		throw Bolt::BoltError(errorCode, msg, loc);
	}

	[[noreturn]] void ReportContractViolation(const char* category, const char* expression, std::string_view msg, const std::source_location loc) {
		std::string fullMessage = std::string(category) + " failed: " + expression + " -> " + std::string(msg) +
			" @ " + loc.file_name() + ":" + std::to_string(loc.line());

		Logger::Error(fullMessage);
		MessageBoxA(nullptr, fullMessage.c_str(), "Bolt Contract Violation", MB_OK | MB_ICONERROR);

#if defined(BOLT_DEBUG)
		__debugbreak();
#endif
		std::terminate();
	}
}