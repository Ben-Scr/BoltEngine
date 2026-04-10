#pragma once
#include "Core/UUID.hpp"

namespace Bolt {
	struct UUIDComponent {
		UUID Id;

		UUIDComponent() : Id(UUID()) {}
		explicit UUIDComponent(UUID id) : Id(id) {}
		explicit UUIDComponent(uint64_t id) : Id(UUID(id)) {}
	};
}
