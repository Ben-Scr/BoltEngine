#pragma once
#include <exception>
#include <source_location>
#include <stdexcept>
#include <string>
#include <utility>
#include "Logger.hpp"

namespace Bolt {
	enum class BoltErrorCode {
		InvalidArgument,
		NotInitialized,
		AlreadyInitialized,
		FileNotFound,
		InvalidHandle,
		OutOfRange,
		OutOfBounds,
		Overflow,
		NullReference,
		LoadFailed,
		Internal,
		Undefined
	};

	enum class ErrorVerbosity {
		Normal,
		Detailed
	};

	inline ErrorVerbosity g_ErrorVerbosity = ErrorVerbosity::Detailed;

	inline const char* ToString(BoltErrorCode c) {
		switch (c) {
		case BoltErrorCode::InvalidArgument:   return "InvalidArgument";
		case BoltErrorCode::NotInitialized:    return "NotInitialized";
		case BoltErrorCode::AlreadyInitialized:return "AlreadyInitialized";
		case BoltErrorCode::FileNotFound:      return "FileNotFound";
		case BoltErrorCode::InvalidHandle:     return "InvalidHandle";
		case BoltErrorCode::OutOfRange:        return "OutOfRange";
		case BoltErrorCode::Overflow:          return "Overflow";
		case BoltErrorCode::NullReference:     return "NullReference";
		case BoltErrorCode::LoadFailed:        return "LoadFailed";
		case BoltErrorCode::Internal:          return "Internal";
		default:                               return "Undefined";
		}
	}

	inline std::string FormatError(
		BoltErrorCode code,
		const std::string& msg,
		const std::source_location& loc
	) {
		if (g_ErrorVerbosity == ErrorVerbosity::Normal) {
			return std::string("BoltError[") + ToString(code) + "]: " + msg;
		}

		return std::string("BoltError[") + ToString(code) + "]: " + msg +
			" @ " + loc.file_name() + ":" + std::to_string(loc.line()) +
			" (" + loc.function_name() + ")";
	}

	class BoltError : public std::runtime_error {
	public:
		BoltError(
			BoltErrorCode code,
			std::string message,
			std::source_location loc = std::source_location::current(),
			std::exception_ptr cause = nullptr
		)
			: std::runtime_error(FormatError(code, message, loc)),
			m_ErrorCode(code),
			m_Message(std::move(message)),
			m_SourceLocation(loc),
			m_Cause(cause)
		{
		}

		BoltErrorCode ErrorCode() const { return m_ErrorCode; }
		const std::string& Message() const { return m_Message; }
		const std::source_location& SourceLocation() const { return m_SourceLocation; }
		const std::exception_ptr& Cause() const { return m_Cause; }

	private:
		BoltErrorCode m_ErrorCode;
		std::string m_Message;
		std::source_location m_SourceLocation;
		std::exception_ptr m_Cause;
	};

	[[noreturn]] inline void BoltRethrow(
		BoltErrorCode code,
		std::string message,
		std::source_location loc = std::source_location::current()
	) {
		throw BoltError(code, std::move(message), loc, std::current_exception());
	}

	inline void AppendCauseChain(std::string& out, std::exception_ptr ep, int depth = 0) {
		if (!ep) return;
		try {
			std::rethrow_exception(ep);
		}
		catch (const std::exception& e) {
			out += "\n  caused by: ";
			out += e.what();
			if (auto be = dynamic_cast<const BoltError*>(&e)) {
				AppendCauseChain(out, be->Cause(), depth + 1);
			}
		}
		catch (...) {
			out += "\n  caused by: <non-std exception>";
		}
	}

	inline std::string FormatForLog(const BoltError& e) {
		std::string out = e.what();
		if (g_ErrorVerbosity == ErrorVerbosity::Detailed) {
			AppendCauseChain(out, e.Cause());
		}
		return out;
	}


#define BOLT_THROW(code, msg) \
    throw ::Bolt::BoltError((code), (msg), std::source_location::current())

#if defined(BOLT_DEBUG)
#define BOLT_ASSERT(cond, code, msg) \
      do { if (!(cond)) BOLT_THROW((code), (msg)); } while (0)
#else
#define BOLT_ASSERT(cond, code, msg) do { if (!(cond)) BOLT_THROW((code), (msg)); } while (0)
#endif

#define BOLT_REQUIRE(cond, code, msg) \
    do { if (!(cond)) BOLT_THROW((code), (msg)); } while (0)


#define BOLT_LOG_ERROR(code, msg) \
    do { \
            ::Bolt::BoltError _e((code), (msg), std::source_location::current()); \
            Logger::Error(FormatForLog(_e)); \
    } while (0)

#define BOLT_LOG_ERROR_IF(cond, code, msg) \
    do { \
        if ((cond)) { \
            ::Bolt::BoltError _e((code), (msg), std::source_location::current()); \
            Logger::Error(FormatForLog(_e)); \
        } \
    } while (0)
}

#define BOLT_RETURN_VAL_IF(cond, retval, code, msg) \
    do { \
        if (cond) { \
            ::Bolt::BoltError _e((code), (msg), std::source_location::current()); \
            Logger::Error(::Bolt::FormatForLog(_e)); \
            return (retval); \
        } \
    } while (0)

#define BOLT_RETURN_IF(cond, code, msg) \
    do { \
        if (cond) { \
            ::Bolt::BoltError _e((code), (msg), std::source_location::current()); \
            Logger::Error(::Bolt::FormatForLog(_e)); \
           return; \
        } \
    } while (0)
