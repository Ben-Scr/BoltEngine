#include "ImGuiDebugSystem.hpp"
#include <imgui.h>

#include "Core/Application.hpp"
#include "Scene/EntityHelper.hpp"
#include "Core/Window.hpp"
#include "Core/Memory.hpp"
#include "Graphics/OpenGL.hpp"

namespace Bolt {
	void ImGuiDebugSystem::OnGui() {

		ImGui::Begin("Debug Settings");

		auto* window = Application::GetWindow();
		auto* renderer2D = Application::GetInstance()->GetRenderer2D();

		bool isVsync = window->IsVsync();
		if (ImGui::Checkbox("Vsync", &isVsync))
			window->SetVsync(isVsync);

		if (!isVsync) {
			float targetFrameRate = 144;
			targetFrameRate = Application::GetTargetFramerate();
			if (ImGui::SliderFloat("Target FPS", &targetFrameRate, 0.f, 244.f)) {
				Application::SetTargetFramerate(targetFrameRate);
			}
		}

		bool renderer2DEnabled = renderer2D->IsEnabled();
		if (ImGui::Checkbox("Renderer2D Enabled", &renderer2DEnabled)) {
			renderer2D->SetEnabled(renderer2DEnabled);
		}

		std::array<float, 4> gizmoCol = Gizmo::GetColor().ToArray();

		if (ImGui::ColorEdit4("Gizmo Color", gizmoCol.data())) {
			Gizmo::SetColor(Color::FromArray(gizmoCol));
		}

		std::array<float, 4> bgCol = OpenGL::GetBackgroundColor().ToArray();

		if (ImGui::ColorEdit4("Background Color", bgCol.data())) {
			OpenGL::SetBackgroundColor(Color::FromArray(bgCol));
		}

		bool runInBG = Application::GetRunInBackground();

		if (ImGui::Checkbox("Run in background", &runInBG)) {
			Application::SetRunInBackground(runInBG);
		}

		bool isFullscreen = window->IsFullScreen();

		if (ImGui::Checkbox("Fullscreen", &isFullscreen)) {
			window->SetFullScreen(isFullscreen);
		}

		float timeScale = Time::GetTimeScale();
		if (ImGui::SliderFloat("Timescale", &timeScale, 0.f, 10.f))
			Time::SetTimeScale(timeScale);

		float fixedFPS = 1.f / Time::GetFixedDeltaTime();
		if (ImGui::SliderFloat("Fixed FPS", &fixedFPS, 10.f, 244.f)) {
			Time::SetFixedDeltaTime(1.f/ fixedFPS);
		}

		bool enabledGizmo = Gizmo::IsEnabled();
		if (ImGui::Checkbox("Gizmo", &enabledGizmo)) {
			Gizmo::SetEnabled(enabledGizmo);
		}

		if (enabledGizmo) {
			static bool aabb = true;
			ImGui::Checkbox("AABB", &aabb);

			static bool collider = true;
			ImGui::Checkbox("Collider", &collider);

			float lineWidth = Gizmo::GetLineWidth();
			if (ImGui::SliderFloat("Gizmo Line Width", &lineWidth, 1.f, 10.f)) {
				Gizmo::SetLineWidth(lineWidth);
			}
		}

		if (ImGui::Button("Reload App")) {
			Application::Reload();
		}
		if (ImGui::Button("Reload Scene")) {
			SceneManager::ReloadScene(GetScene().GetName());
		}
		if (ImGui::Button("Quit")) {
			Application::Quit();
		}

		ImGui::End();
		ImGui::Begin("Debug Info");

		const std::string fps = "Current FPS: " + std::to_string(1.f / Time::GetDeltaTimeUnscaled());
		const std::string timescale = "Current TimeScale: " + std::to_string(Time::GetTimeScale());
		const std::string frameCount = "Frame Count: " + std::to_string(Time::GetFrameCount());

		auto vp = *window->GetMainViewport();

		const std::string windowSize = "Window Size: " + std::to_string(window->GetWidth()) + " x " + std::to_string(window->GetHeight());
		const std::string windowVP = "Viewport Size: " + StringHelper::ToString(vp);


		const std::string renderedInstances = "Rendered Instances: " + std::to_string(renderer2D->GetRenderedInstancesCount());
		const std::string renderLoopDuration = "Render Loop Duration: " + std::to_string(renderer2D->GetRRenderLoopDuration());

		auto stats = Memory::GetAllocationStats();

		ImGui::Text(("Total Allocated Memory: " + StringHelper::FromMemoryIEC(stats.TotalAllocated)).c_str());
		ImGui::Text(("Total Freed Memory: " + StringHelper::FromMemoryIEC(stats.TotalFreed)).c_str());
		ImGui::Text(("Active Memory: " + StringHelper::FromMemoryIEC(stats.TotalAllocated - stats.TotalFreed)).c_str());

		ImGui::Text(fps.c_str());
		ImGui::Text(timescale.c_str());
		ImGui::Text(frameCount.c_str());
		ImGui::Text(windowSize.c_str());
		ImGui::Text(windowVP.c_str());
		ImGui::Text(renderedInstances.c_str());
		ImGui::Text(renderLoopDuration.c_str());
		ImGui::End();
	}
}