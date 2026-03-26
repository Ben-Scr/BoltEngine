#include "pch.hpp"
#include "ImGuiRenderer.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <GLFW/glfw3.h>

namespace Bolt {

	void ImGuiRenderer::Initialize(GLFWwindow* window) {
		if (m_IsInitialized) {
			return;
		}

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		(void)ImGui::GetIO();
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 330 core");
		ImGui::StyleColorsDark();

		m_IsInitialized = true;
	}

	void ImGuiRenderer::Shutdown() {
		if (!m_IsInitialized) {
			return;
		}

		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		m_IsInitialized = false;
	}

	void ImGuiRenderer::BeginFrame() {
		if (!m_IsInitialized) {
			return;
		}
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiRenderer::EndFrame() {
		if (!m_IsInitialized) {
			return;
		}
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
}