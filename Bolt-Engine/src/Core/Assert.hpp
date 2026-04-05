#pragma once

#include "Core/Base.hpp"
#include "Core/Exceptions.hpp"
#include "Core/Log.hpp"

#include <source_location>
#include <string>
#include <string_view>
#include <utility>

#include <spdlog/fmt/fmt.h>

#ifdef BT_PLATFORM_WINDOWS
#define BT_DEBUG_BREAK __debugbreak()
#else
#include <csignal>
#define BT_DEBUG_BREAK raise(SIGTRAP)
#endif

namespace Bolt {

	void ReportAssertionFailure(
		const char* kind,
		const char* expression,
		std::string_view message,
		std::source_location location = std::source_location::current());

	inline std::string BuildAssertMessage(const std::string_view message) {
		return std::string(message);
	}

	inline std::string BuildAssertMessage(const BoltErrorCode code, const std::string_view message) {
		return fmt::format("[{}] {}", ErrorCodeToString(code), message);
	}

	template <typename T>
	inline std::string BuildAssertMessage(T&& message) {
		return std::string(std::forward<T>(message));
	}

} // namespace Bolt

#ifdef BT_DEBUG
#define BT_ENABLE_ASSERTS
#endif

#define BT_ENABLE_VERIFY

#define BT_ASSERT(cond, ...) \
	do { \
		if (!(cond)) { \
			::Bolt::ReportAssertionFailure("ASSERT", #cond, ::Bolt::BuildAssertMessage(__VA_ARGS__), std::source_location::current()); \
		} \
	} while (0)

#define BT_CORE_ASSERT(cond, ...) \
	do { \
		if (!(cond)) { \
			::Bolt::ReportAssertionFailure("CORE_ASSERT", #cond, ::Bolt::BuildAssertMessage(__VA_ARGS__), std::source_location::current()); \
		} \
	} while (0)

#ifdef BT_ENABLE_VERIFY
#define BT_VERIFY(cond, ...) \
	do { \
		if (!(cond)) { \
			::Bolt::ReportAssertionFailure("VERIFY", #cond, ::Bolt::BuildAssertMessage(__VA_ARGS__), std::source_location::current()); \
		} \
	} while (0)

#define BT_CORE_VERIFY(cond, ...) \
	do { \
		if (!(cond)) { \
			::Bolt::ReportAssertionFailure("CORE_VERIFY", #cond, ::Bolt::BuildAssertMessage(__VA_ARGS__), std::source_location::current()); \
		} \
	} while (0)
#else
#define BT_VERIFY(cond, ...) ((void)(cond))
#define BT_CORE_VERIFY(cond, ...) ((void)(cond))
#endif