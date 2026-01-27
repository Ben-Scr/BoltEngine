#include "../pch.hpp"
#include "ImGuiRenderer.hpp"

#include "../Core/Time.hpp"
#include "../Graphics/Gizmos.hpp"
#include "../Core/Application.hpp"
#include "../Collections/Viewport.hpp"
#include "../Core/Window.hpp"
#include "../Scene/SceneManager.hpp"
#include "../Scene/Scene.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <GLFW/glfw3.h>

namespace Bolt {

	void ImGuiRenderer::Initialize(GLFWwindow* window) {
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 330 core");
		ImGui::StyleColorsDark();

		s_IsInitialized = true;
	}

	void ImGuiRenderer::Shutdown() {
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		s_IsInitialized = false;
	}

	void ImGuiRenderer::BeginFrame() {
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiRenderer::EndFrame() {
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
}