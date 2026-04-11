#pragma once
#include "Core/Layer.hpp"
#include "Core/Export.hpp"

namespace Bolt {
	class BOLT_API GizmosDebugSystem : public Layer {
	public:
		using Layer::Layer;

		void OnUpdate(Application& app, float dt) override;
	};
}
