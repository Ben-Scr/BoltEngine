#pragma once
#include <stdexcept>

namespace Bolt {
	class BoltException : public std::runtime_error {
	public:
		BoltException(const std::string& from, const std::string msg) : std::runtime_error(from + ": " + msg) {};
		std::string from;
		std::string msg;
	};

	class SceneException : public BoltException {
	public:
		explicit SceneException(const std::string& msg) : BoltException("SceneException", msg) {}
	};
	class InvalidArgumentException : public BoltException {
	public:
		explicit InvalidArgumentException(const std::string& msg) : BoltException("InvalidArgumentException", msg) {}
	};
	class NullReferenceException : public BoltException {
	public:
		explicit NullReferenceException(const std::string& msg) : BoltException("NullReferenceException", msg) {}
	};
	class OverflowException : public BoltException {
	public:
		explicit OverflowException(const std::string& msg) : BoltException("OverflowException", msg) {}
	};
	class NotInitializedException : public BoltException {
	public:
		explicit NotInitializedException(const std::string& msg) : BoltException("NotInitializedException", msg) {}
	};
}