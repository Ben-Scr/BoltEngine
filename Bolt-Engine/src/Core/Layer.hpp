#pragma once
#include "Core/Export.hpp"
#include "Events/BoltEvent.hpp"

#include <string>

namespace Bolt {
	class Application;

	class BOLT_API Layer
	{
	public:
		explicit Layer(const std::string& name = "Layer");
		virtual ~Layer();

		virtual void OnAttach(Application& app) {}
		virtual void OnDetach(Application& app) {}
		virtual void OnUpdate(Application& app, float dt) {}
		virtual void OnFixedUpdate(Application& app, float fixedDt) {}
		virtual void OnImGuiRender(Application& app) {}
		virtual void OnEvent(Application& app, BoltEvent& event) {}

		inline const std::string& GetName() const { return m_DebugName; }
	protected:
		std::string m_DebugName;
	};
}

