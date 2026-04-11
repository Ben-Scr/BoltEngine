#pragma once
#include "Scene/ISystem.hpp"

namespace Bolt {
	class AudioUpdateSystem : public ISystem {
	public:
		void Start(Scene& scene) override;
	};
}
