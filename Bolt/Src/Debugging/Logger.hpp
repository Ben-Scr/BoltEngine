#pragma once
#include "Utils/Event.hpp"
#include <string>
#include <source_location>

namespace Bolt {
	enum LogLevel {
		None,
		Info,
		Warning,
		Error
	};

	class Logger {
	public:
		static void Message(const std::string& message, const std::source_location& loc = std::source_location::current());
		static void Message(const std::string& topic, const std::string& message, const std::source_location& loc = std::source_location::current());

		static void Warning(const std::string& message, const std::source_location& loc = std::source_location::current());
		static void Warning(const std::string& topic, const std::string& message, const std::source_location& loc = std::source_location::current());

		static void Error(const std::string& message, const std::source_location& loc = std::source_location::current());
		static void Error(const std::string& topic, const std::string& message, const std::source_location& loc = std::source_location::current());

	
		static Event<const std::string&, LogLevel> OnLog;
	private:
		static std::string ToString(const std::string& topic, const std::string& message, const std::source_location& loc = std::source_location::current());
		static std::string LogLevelToString(LogLevel logLevel) {
			switch (logLevel) {
			case LogLevel::None: return "None"; break;
			case LogLevel::Info: return "Info"; break;
			case LogLevel::Warning: return "Warning"; break;
			case LogLevel::Error: return "Error"; break;
			}
		}
		static void Log(const std::string& topic, const std::string& message, LogLevel logLevel, const std::source_location& loc = std::source_location::current());
		static void Log(const std::string& message, LogLevel logLevel, const std::source_location& loc = std::source_location::current());
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