#pragma once
#include "Core/Layer.hpp"
#include "Core/Export.hpp"

namespace Bolt {
	class BOLT_API ImGuiDebugSystem : public Layer {
	public:
		using Layer::Layer;

		void OnImGuiRender(Application& app) override;
	};
}
