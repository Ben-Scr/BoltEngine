#pragma once
#include <Bolt.hpp>

namespace Bolt {
	class ImGuiDebugSystem : public ISystem {
	public:
		virtual void OnGui();
	};
}