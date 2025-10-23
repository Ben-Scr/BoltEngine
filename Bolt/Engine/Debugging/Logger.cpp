#include "../pch.hpp"
#include "Logger.hpp"

namespace Bolt {
	void Logger::Message(const std::string& message, const std::source_location& loc) {
		std::cout << "[Message] " << message << '\n';
	}
	void Logger::Message(const std::string& topic, const std::string& message, const std::source_location& loc) {
		std::cout << "[Message][" << topic << "] " << message << '\n';
	}

	void Logger::Warning(const std::string& message, const std::source_location& loc) {
		std::cout << "[Warning] " << message << '\n';
	}
	void Logger::Warning(const std::string& topic, const std::string& message, const std::source_location& loc) {
		std::cout << "[Warning][" << topic << "] " << message << '\n';
	}

	void Logger::Error(const std::string& message, const std::source_location& loc) {
		std::cout << "[Error] " << message << '\n';
	}
	void Logger::Error(const std::string& topic, const std::string& message, const std::source_location& loc) {
		std::cout << "[Error][" << topic << "] " << message << '\n';
	}
}