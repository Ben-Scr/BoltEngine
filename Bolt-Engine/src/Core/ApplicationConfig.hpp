#pragma once

#include "Core/WindowSpecification.hpp"

namespace Bolt {

	struct ApplicationConfig {
		WindowSpecification WindowSpecification{ 800, 800, "Bolt Runtime", true, true, false };
		bool EnableImGui = true;
		bool EnableGuiRenderer = true;
		bool EnableGizmoRenderer = true;
		bool EnablePhysics2D = true;
		bool EnableAudio = true;
		bool SetWindowIcon = true;
		bool Vsync = true;
	};

} // namespace Bolt
