#pragma once
#include <string>
#include <memory>
#include <source_location>

namespace Bolt {
	class Logger {
	public:
		static void Message(const std::string& message, const std::source_location& loc = std::source_location::current());
		static void Message(const std::string& topic, const std::string& message, const std::source_location& loc = std::source_location::current());

		static void Warning(const std::string& message, const std::source_location& loc = std::source_location::current());
		static void Warning(const std::string& topic, const std::string& message, const std::source_location& loc = std::source_location::current());

		static void Error(const std::string& message, const std::source_location& loc = std::source_location::current());
		static void Error(const std::string& topic, const std::string& message, const std::source_location& loc = std::source_location::current());
	};

	inline void Print(const std::string& message, const std::source_location& loc = std::source_location::current()) {
		Logger::Message(message, loc);
	}
	inline void PrintWarning(const std::string& message, const std::source_location& loc = std::source_location::current()) {
		Logger::Warning(message, loc);
	}
	inline void PrintError(const std::string& message, const std::source_location& loc = std::source_location::current()) {
		Logger::Error(message, loc);
	}
}