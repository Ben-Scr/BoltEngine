#pragma once

class GLFWwindow;

namespace Bolt {
	class Window;
	class Viewport;

	class ImGuiRenderer {
	public:
		ImGuiRenderer() = default;
		void Initialize(GLFWwindow* window);
		void Shutdown();

		void BeginFrame();
		void EndFrame();

	private:
		std::shared_ptr<Viewport> m_Viewport;
		bool s_IsInitialized;
	};
}