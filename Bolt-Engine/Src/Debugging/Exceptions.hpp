#pragma once

#include <exception>
#include <ostream>
#include <source_location>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "Logger.hpp"
#include "Utils/StringHelper.hpp"

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
		InvalidValue,
		Undefined
	};

	enum class ErrorVerbosity {
		Normal,
		Detailed
	};

	enum class ContractViolationKind {
		Assert,
		Verify,
		CoreAssert,
		CoreVerify
	};

	struct ContractViolation {
		ContractViolationKind Kind;
		const char* Expression;
		std::string Message;
		std::source_location SourceLocation;
	};

	inline const char* ErrorCodeToString(const BoltErrorCode c) {
		switch (c) {
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

	inline const char* ContractViolationKindToString(const ContractViolationKind kind) {
		switch (kind) {
		case ContractViolationKind::Assert: return "ASSERT";
		case ContractViolationKind::Verify: return "VERIFY";
		case ContractViolationKind::CoreAssert: return "CORE_ASSERT";
		case ContractViolationKind::CoreVerify: return "CORE_VERIFY";
		default: return "CONTRACT_VIOLATION";
		}
	}

	inline std::ostream& operator<<(std::ostream& os, const BoltErrorCode c) {
		return os << ErrorCodeToString(c);
	}

	ErrorVerbosity GetErrorVerbosity();
	void SetErrorVerbosity(ErrorVerbosity verbosity);

	std::string FormatError(BoltErrorCode code, std::string_view msg, const std::source_location& loc);
	std::string FormatContractViolation(const ContractViolation& violation);

	class BoltError : public std::runtime_error {
	public:
		BoltError(
			BoltErrorCode code,
			std::string message,
			std::source_location loc = std::source_location::current(),
			std::exception_ptr cause = nullptr)
			: std::runtime_error(FormatError(code, message, loc)),
			m_ErrorCode(code),
			m_Message(std::move(message)),
			m_SourceLocation(loc),
			m_Cause(cause) {}

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
		std::source_location loc = std::source_location::current()) {
		throw BoltError(code, std::move(message), loc, std::current_exception());
	}

	void AppendCauseChain(std::string& out, std::exception_ptr ep, int depth = 0);
	std::string FormatForLog(const BoltError& e);

	[[noreturn]] void ThrowError(BoltErrorCode errorCode, std::string_view msg, std::source_location loc = std::source_location::current());
	[[noreturn]] void ReportContractViolation(ContractViolationKind kind, const char* expression, std::string_view msg, std::source_location loc = std::source_location::current());

	template<typename TMessage>
	inline std::string_view ContractMessage(TMessage&& msg) {
		return std::forward<TMessage>(msg);
	}

	template<typename TCode, typename TMessage>
	inline std::string_view ContractMessage(TCode&&, TMessage&& msg) {
		return std::forward<TMessage>(msg);
	}

#define BT_THROW(code, msg) \
	::Bolt::ThrowError((code), (msg), std::source_location::current())

#define BT_LOG_ERROR(code, msg) \
	do { \
		::Bolt::BoltError _e((code), (msg), std::source_location::current()); \
		Logger::Error(::Bolt::FormatForLog(_e)); \
	} while (0)

#define BOLT_LOG_ERROR_IF(cond, code, msg) \
	do { \
		if ((cond)) { \
			::Bolt::BoltError _e((code), (msg), std::source_location::current()); \
			Logger::Error(::Bolt::FormatForLog(_e)); \
		} \
	} while (0)
}

#define BOLT_TRY_CATCH_LOG(stmt) \
	do { \
		try { \
			stmt; \
		} catch (const std::exception& ex) { \
			Logger::Error(ex.what()); \
		} catch (...) { \
			Logger::Error("Unknown exception"); \
		} \
	} while (0)