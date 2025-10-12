#pragma once
#include "Scene/ISystem.hpp"

namespace Bolt {
	class GameSystem : public ISystem {
	public:
		virtual void Awake(Scene& scene);
		virtual void Start(Scene& scene);
		virtual void Update(Scene& scene);
	};
}