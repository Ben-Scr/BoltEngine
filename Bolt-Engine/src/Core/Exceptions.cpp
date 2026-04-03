#include "pch.hpp"

#include "Core/Exceptions.hpp"

#include <utility>
#include <spdlog/fmt/fmt.h>

namespace Bolt {

	const char* ErrorCodeToString(const BoltErrorCode code) {
		switch (code) {
		case BoltErrorCode::InvalidArgument: return "InvalidArgument";
		case BoltErrorCode::NotInitialized: return "NotInitialized";
		case BoltErrorCode::AlreadyInitialized: return "AlreadyInitialized";
		case BoltErrorCode::FileNotFound: return "FileNotFound";
		case BoltErrorCode::InvalidHandle: return "InvalidHandle";
		case BoltErrorCode::OutOfRange: return "OutOfRange";
		case BoltErrorCode::OutOfBounds: return "OutOfBounds";
		case BoltErrorCode::Overflow: return "Overflow";
		case BoltErrorCode::NullReference: return "NullReference";
		case BoltErrorCode::LoadFailed: return "LoadFailed";
		case BoltErrorCode::InvalidValue: return "InvalidValue";
		case BoltErrorCode::Undefined: return "Undefined";
		default: return "Undefined";
		}
	}

	BoltException::BoltException(const BoltErrorCode code, std::string message, const std::source_location location)
		: std::runtime_error(BuildWhat(code, message, location)),
		m_Code(code),
		m_Message(std::move(message)),
		m_Location(location) {
	}

	std::string BoltException::BuildWhat(const BoltErrorCode code, const std::string_view message, const std::source_location& location) {
		return fmt::format("BoltException[{}] {} @ {}:{} ({})",
			ErrorCodeToString(code),
			message,
			location.file_name(),
			location.line(),
			location.function_name());
	}

	[[noreturn]] void ThrowError(const BoltErrorCode code, const std::string_view message, const std::source_location location) {
		throw BoltException(code, std::string(message), location);
	}

	[[noreturn]] void RethrowWithContext(const BoltErrorCode code, std::string message, const std::source_location location) {
		try {
			throw;
		}
		catch (const std::exception& ex) {
			message += std::string(" | caused by: ") + ex.what();
			throw BoltException(code, std::move(message), location);
		}
		catch (...) {
			message += " | caused by: <non-std exception>";
			throw BoltException(code, std::move(message), location);
		}
	}

} // namespace Bolt