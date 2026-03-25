#pragma once

class GLFWwindow;

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
		bool s_IsInitialized;
	};
}