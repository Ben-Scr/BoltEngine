#pragma once
#include "Scene/ISystem.hpp"

namespace Bolt {
	class ParticleUpdateSystem : public ISystem {
	public:
		virtual void Update();
	};
}