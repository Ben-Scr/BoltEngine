#include <pch.hpp>
#include "Exceptions.hpp"
#include <Windows.h>

namespace Bolt {
	void ThrowError(BoltErrorCode errorCode, const std::string& msg) {
		auto error = Bolt::BoltError((errorCode), (msg), std::source_location::current());
		MessageBoxA(nullptr, error.what(), "Bolt Assert", MB_OK | MB_ICONERROR);
		throw error;
	}
}