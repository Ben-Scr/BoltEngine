#include <pch.hpp>
#include "Exceptions.hpp"

#include <cstdlib>

#if defined(_WIN32)
#include <Windows.h>
#endif

namespace Bolt {
	ErrorVerbosity g_ErrorVerbosity = ErrorVerbosity::Detailed;

	ErrorVerbosity GetErrorVerbosity() {
		return g_ErrorVerbosity;
	}

	void SetErrorVerbosity(const ErrorVerbosity verbosity) {
		g_ErrorVerbosity = verbosity;
	}

	std::string FormatError(const BoltErrorCode code, const std::string_view msg, const std::source_location& loc) {
		std::string frontPart = std::string("BoltError[") + StringHelper::ToString(code) + "]: " + std::string(msg);

		if (GetErrorVerbosity() == ErrorVerbosity::Normal)
			return frontPart;

		return frontPart +
			" @ " + loc.file_name() + ":" + std::to_string(loc.line()) +
			" (" + loc.function_name() + ")";
	}

	std::string FormatContractViolation(const ContractViolation& violation) {
		std::string out = std::string(ContractViolationKindToString(violation.Kind)) + " failed: " + violation.Expression;
		if (!violation.Message.empty()) {
			out += " -> " + violation.Message;
		}

		out += " @ ";
		out += violation.SourceLocation.file_name();
		out += ":";
		out += std::to_string(violation.SourceLocation.line());
		return out;
	}

	void AppendCauseChain(std::string& out, const std::exception_ptr ep, int depth) {
		if (!ep || depth > 32) {
			return;
		}

		try {
			std::rethrow_exception(ep);
		}
		catch (const std::exception& e) {
			out += "\n  caused by: ";
			out += e.what();
			if (const auto* be = dynamic_cast<const BoltError*>(&e)) {
				AppendCauseChain(out, be->Cause(), depth + 1);
			}
		}
		catch (...) {
			out += "\n  caused by: <non-std exception>";
		}
	}

	std::string FormatForLog(const BoltError& e) {
		std::string out = e.what();
		if (GetErrorVerbosity() == ErrorVerbosity::Detailed) {
			AppendCauseChain(out, e.Cause());
		}
		return out;
	}

	[[noreturn]] void ThrowError(const BoltErrorCode errorCode, const std::string_view msg, const std::source_location loc) {
		throw BoltError(errorCode, std::string(msg), loc);
	}

	[[noreturn]] void ReportContractViolation(const ContractViolationKind kind, const char* expression, const std::string_view msg, const std::source_location loc) {
		const ContractViolation violation{ kind, expression, std::string(msg), loc };
		const std::string formatted = FormatContractViolation(violation);

		Logger::Error(formatted);

#ifdef _WIN32
		MessageBoxA(nullptr, formatted.c_str(), "Bolt Contract Violation", MB_OK | MB_ICONERROR);
#endif

#ifdef BT_DEBUG && _MSC_VER
		BT_DEBUG_BREAK;
#endif
		std::terminate();
	}
}