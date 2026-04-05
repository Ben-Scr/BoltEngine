#pragma once
#include  "Scene/ISystem.hpp"
#include "Core/Export.hpp"
#include "Scene/Entity.hpp"

namespace Bolt {
	class BOLT_API GizmosDebugSystem : public ISystem {
	public:
		virtual void Update(Scene& scene);
	};
}