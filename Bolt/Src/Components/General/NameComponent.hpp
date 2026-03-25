#pragma once
#include <string>

namespace Bolt {
	struct NameComponent {
		std::string Name;
		NameComponent() = default;
		explicit NameComponent(std::string name) : Name(std::move(name)) {}
	};
}