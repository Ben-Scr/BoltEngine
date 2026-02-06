#pragma once
#include <Bolt.hpp>

namespace Bolt {
	class ImGuiDebugSystem : public ISystem {
	public:
		virtual void OnGui();
	private:
		EntityHandle m_SelectedEntity = entt::null;
	};
}