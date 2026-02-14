#pragma once
#include "Core/Application.hpp"
#include  "Scene/ISystem.hpp"
#include "Core/Core.hpp"
#include "Scene/Entity.hpp"

namespace Bolt {
	class BOLT_API ImGuiDebugSystem : public ISystem {
	public:
		virtual void OnGui();
	private:
		EntityHandle m_SelectedEntity = entt::null;
	};
}