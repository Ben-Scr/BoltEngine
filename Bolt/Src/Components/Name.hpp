#pragma once
#include <string>

namespace Bolt {
	struct NameTag {
		std::string Name;
		NameTag() = default;
		explicit NameTag(std::string name) : Name(std::move(name)) {}
	};
}