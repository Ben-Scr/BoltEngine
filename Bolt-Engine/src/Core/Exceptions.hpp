#pragma once
#include "Core/BoltErrorCode.hpp"

#include <exception>
#include <source_location>
#include <stdexcept>
#include <string>
#include <string_view>

#include "Core/Log.hpp"

namespace Bolt {
	const char* ErrorCodeToString(BoltErrorCode code);

	class BoltException : public std::runtime_error {
	public:
		BoltException(BoltErrorCode code, std::string message, std::source_location location = std::source_location::current());

		BoltErrorCode Code() const noexcept { return m_Code; }
		const std::string& Message() const noexcept { return m_Message; }
		const std::source_location& Location() const noexcept { return m_Location; }

	private:
		static std::string BuildWhat(BoltErrorCode code, std::string_view message, const std::source_location& location);

		BoltErrorCode m_Code;
		std::string m_Message;
		std::source_location m_Location;
	};

	using BoltError = BoltException;

	[[noreturn]] void ThrowError(BoltErrorCode code, std::string_view message, std::source_location location = std::source_location::current());
	[[noreturn]] void RethrowWithContext(BoltErrorCode code, std::string message, std::source_location location = std::source_location::current());

} // namespace Bolt

#define BT_THROW(code, msg) ::Bolt::ThrowError((code), (msg), std::source_location::current())

#define BT_LOG_ERROR(code, msg) \
	do { \
		BT_CORE_ERROR("[{}] {}", ::Bolt::ErrorCodeToString(code), (msg)); \
	} while (0)

#define BOLT_LOG_ERROR_IF(cond, code, msg) \
	do { \
		if (cond) { \
			BT_LOG_ERROR((code), (msg)); \
		} \
	} while (0)

#define BOLT_TRY_CATCH_LOG(stmt) \
	do { \
		try { \
			stmt; \
		} catch (const std::exception& ex) { \
			BT_CORE_ERROR("Exception: {}", ex.what()); \
		} catch (...) { \
			BT_CORE_ERROR("Unknown exception"); \
		} \
	} while (0)
