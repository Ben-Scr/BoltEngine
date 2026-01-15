#pragma once
#include <stdexcept>

namespace Bolt {
	class BoltExeption : public std::runtime_error {
	public:
		BoltExeption(const std::string& from, const std::string msg) : std::runtime_error(from + ": " + msg) {};
		std::string from;
		std::string msg;
	};

	class SceneExeption : public BoltExeption {
	public:
		explicit SceneExeption(const std::string& msg) : BoltExeption("SceneExeption", msg) {}
	};
	class InvalidArgumentExeption : public BoltExeption {
	public:
		explicit InvalidArgumentExeption(const std::string& msg) : BoltExeption("SceneExeption", msg) {}
	};
	class NullReferenceExeption : public BoltExeption {
	public:
		explicit NullReferenceExeption(const std::string& msg) : BoltExeption("NullReferenceArgumentExeption", msg) {}
	};
	class OverflowExepection : public BoltExeption {
	public:
		explicit OverflowExepection(const std::string& msg) : BoltExeption("OverflowExepection", msg) {}
	};
}