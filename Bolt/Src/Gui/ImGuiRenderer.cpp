#include "../pch.hpp"
#include "ImGuiRenderer.hpp"

#include "../Core/Time.hpp"
#include "../Graphics/Gizmos.hpp"
#include "../Core/Application.hpp"
#include "../Collections/Viewport.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <GLFW/glfw3.h>

namespace Bolt {

	void ImGuiRenderer::Initialize() {
		m_Viewport = Window::GetMainViewport();

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		ImGui_ImplGlfw_InitForOpenGL(Window::GetActiveWindow()->GetGLFWWindow(), true);
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
		ImGui::SetNextWindowSize(ImVec2(m_Viewport->Width, m_Viewport->Height));

		ImGui::Begin("Debug Settings");

		static bool enabledVsync = true;
		ImGui::Checkbox("Vsync", &enabledVsync);
		Application::GetWindow().SetVsync(enabledVsync);

		if (!enabledVsync) {
			static float targetFrameRate = 144;
			ImGui::SliderFloat("Target FPS", &targetFrameRate, 0.f, 244.f);
			Application::SetTargetFramerate(targetFrameRate);
		}

		static float timeScale = 1.0;
		ImGui::SliderFloat("Timescale", &timeScale, 0.f, 10.f);
		Time::SetTimeScale(timeScale);

		static float fixedFPS = 50.f;
		ImGui::SliderFloat("Fixed FPS", &fixedFPS, 10.f, 244.f);

		static bool enabledGizmo = true;
		ImGui::Checkbox("Gizmo", &enabledGizmo);
		Gizmo::SetEnabled(enabledGizmo);

		if (enabledGizmo) {
			static bool aabb = true;
			ImGui::Checkbox("AABB", &aabb);

			static bool collider = true;
			ImGui::Checkbox("Collider", &collider);
		}

		ImGui::End();

		ImGui::Begin("Debug Info");

		const std::string fps = "Current FPS: " + std::to_string(1.f / Time::GetDeltaTime());
		const std::string timescale = "Current TimeScale: " + std::to_string(Time::GetTimeScale());
		//const std::string renderedInstances = "Rendered Instances: " + std::to_string(Application::GetInstance().m_Renderer2D->GetRenderedInstancesCount());
		//const std::string renderLoopDuration = "Render Loop Duration: " + std::to_string(m_Renderer2D->GetRRenderLoopDuration());

		ImGui::Text(fps.c_str());
		ImGui::Text(timescale.c_str());
		//ImGui::Text(renderedInstances.c_str());
		//ImGui::Text(renderLoopDuration.c_str());
		ImGui::End();
	}

	void ImGuiRenderer::EndFrame() {
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
}