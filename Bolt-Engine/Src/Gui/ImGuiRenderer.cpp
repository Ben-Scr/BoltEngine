#include "pch.hpp"
#include "ImGuiRenderer.hpp"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>


namespace Bolt {

	void ImGuiRenderer::Initialize(GLFWwindow* window) {
		if (m_IsInitialized) {
			return;
		}

		BT_ASSERT(window != nullptr, "Window shouldn't be null!");

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		BT_VERIFY(ImGui_ImplGlfw_InitForOpenGL(window, true), "Failed to init glfw for imgui!" );
		BT_VERIFY(ImGui_ImplOpenGL3_Init("#version 330 core"), "Failed to init openGL3 for imgui!");
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