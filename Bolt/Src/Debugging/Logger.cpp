#include "pch.hpp"
#include "Logger.hpp"

namespace Bolt {
	Event<const std::string&, LogLevel> Logger::OnLog;

	void Logger::Message(const std::string& message, const std::source_location& loc) {
		Log(message, LogLevel::Info, loc);
	}
	void Logger::Message(const std::string& topic, const std::string& message, const std::source_location& loc) {
		Log(topic, message, LogLevel::Info, loc);
	}

	void Logger::Warning(const std::string& message, const std::source_location& loc) {
		Log(message, LogLevel::Warning, loc);
	}
	void Logger::Warning(const std::string& topic, const std::string& message, const std::source_location& loc) {
		Log(topic, message, LogLevel::Warning, loc);
	}

	void Logger::Error(const std::string& message, const std::source_location& loc) {
		Log(message, LogLevel::Error, loc);
	}
	void Logger::Error(const std::string& topic, const std::string& message, const std::source_location& loc) {
		Log(topic, message, LogLevel::Error, loc);
	}

	std::string Logger::ToString(const std::string& topic, const std::string& message, const std::source_location& loc) {
		std::ostringstream  oss;
		oss << "[Error][" << topic << "] " << message << '\n';
		return oss.str();
	}

	void Logger::Log(const std::string& topic, const std::string& message, LogLevel logLevel, const std::source_location& loc) {
		std::string s = "[" + LogLevelToString(logLevel) + "][" + topic + "] " + message + '\n';
		std::cout << s;
		OnLog.Invoke(s, logLevel);
	}
	void Logger::Log(const std::string& message, LogLevel logLevel, const std::source_location& loc) {
		std::string s = "[" + LogLevelToString(logLevel) + "] " + message + '\n';
		std::cout << s;
		OnLog.Invoke(s, logLevel);
	}
}