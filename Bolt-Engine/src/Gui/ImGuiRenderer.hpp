#pragma once

#include <GLFW/glfw3.h>

namespace Bolt {
	class Viewport;

	class ImGuiRenderer {
	public:
		ImGuiRenderer() = default;
		void Initialize(GLFWwindow* window);
		void Shutdown();

		void BeginFrame();
		void EndFrame();

	private:
		bool m_IsInitialized = false;
	};
}