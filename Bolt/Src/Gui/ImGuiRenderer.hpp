#pragma once
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

class GLFWwindow;

namespace Bolt {

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