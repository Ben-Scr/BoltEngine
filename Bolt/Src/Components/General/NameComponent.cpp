#include <pch.hpp>
#include "NameComponent.hpp"
#include <Scene/EntityHelper.hpp>

namespace Bolt {
	NameComponent::NameComponent() : Name{ "Unnamed (" + std::to_string(EntityHelper::EntitiesCount()) + ")"} {

	}
}