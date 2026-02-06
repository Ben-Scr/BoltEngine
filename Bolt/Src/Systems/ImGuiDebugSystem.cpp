#include "ImGuiDebugSystem.hpp"
#include <imgui.h>

#include "Core/Application.hpp"
#include "Scene/EntityHelper.hpp"
#include "Core/Window.hpp"
#include "Core/Memory.hpp"
#include "Graphics/OpenGL.hpp"

namespace Bolt {
	void ImGuiDebugSystem::OnGui() {
		auto* window = Application::GetInstance()->GetWindow();
		auto* renderer2D = Application::GetInstance()->GetRenderer2D();

        ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_MenuBar);

        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("Application")) {
                if (ImGui::MenuItem("Reload App")) { Application::Reload(); }
                if (ImGui::MenuItem("Quit")) { Application::Quit(); }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        if (ImGui::CollapsingHeader("Display & Graphics", ImGuiTreeNodeFlags_DefaultOpen)) {
            bool isFullscreen = window->IsFullScreen();
            if (ImGui::Checkbox("Fullscreen", &isFullscreen)) {
                window->SetFullScreen(isFullscreen);
            }

            bool isDecorated = window->IsDecorated();
            if (ImGui::Checkbox("Decorated", &isDecorated)) {
                window->SetDecoration(isDecorated);
            }

            bool isResizeable = window->IsResizeable();
            if (ImGui::Checkbox("Resizeable", &isResizeable)) {
                window->SetResizeable(isResizeable);
            }

            bool isVsync = window->IsVsync();
            if (ImGui::Checkbox("Vsync", &isVsync)) {
                window->SetVsync(isVsync);
            }

            if (!isVsync) {
                float targetFrameRate = Application::GetTargetFramerate();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
                if (ImGui::SliderFloat("Target FPS", &targetFrameRate, 30.f, 244.f, "%.0f FPS")) {
                    Application::SetTargetFramerate(targetFrameRate);
                }
            }

            ImGui::Separator();

            std::array<float, 4> bgCol = OpenGL::GetClearColor().ToArray();
            if (ImGui::ColorEdit4("Clear Color", bgCol.data(), ImGuiColorEditFlags_NoInputs)) {
                OpenGL::SetClearColor(Color::FromArray(bgCol));
            }
        }

        if (ImGui::CollapsingHeader("Time & Simulation")) {
            float timeScale = Time::GetTimeScale();
            if (ImGui::SliderFloat("Timescale", &timeScale, 0.f, 10.f, "%.2fx")) {
                Time::SetTimeScale(timeScale);
            }

            float fixedFPS = 1.f / Time::GetUnscaledFixedDeltaTime();
            if (ImGui::SliderFloat("Fixed Update (Hz)", &fixedFPS, 10.f, 244.f, "%.0f Hz")) {
                Time::SetFixedDeltaTime(1.f / fixedFPS);
            }

            bool runInBG = Application::GetRunInBackground();
            if (ImGui::Checkbox("Run in Background", &runInBG)) {
                Application::SetRunInBackground(runInBG);
            }
        }

        if (ImGui::CollapsingHeader("Gizmos & Debug Draw")) {
            bool renderer2DEnabled = renderer2D->IsEnabled();
            if (ImGui::Checkbox("Enable Renderer2D", &renderer2DEnabled)) {
                renderer2D->SetEnabled(renderer2DEnabled);
            }

            ImGui::Separator();

            bool enabledGizmo = Gizmo::IsEnabled();
            if (ImGui::Checkbox("Show Gizmos", &enabledGizmo)) {
                Gizmo::SetEnabled(enabledGizmo);
            }

            if (enabledGizmo) {
                ImGui::Indent();

                static bool aabb = true; ImGui::Checkbox("AABB Boxes", &aabb);
                ImGui::SameLine();
                static bool collider = true; ImGui::Checkbox("Colliders", &collider);

                float lineWidth = Gizmo::GetLineWidth();
                if (ImGui::SliderFloat("Line Width", &lineWidth, 1.f, 5.f)) {
                    Gizmo::SetLineWidth(lineWidth);
                }

                std::array<float, 4> gizmoCol = Gizmo::GetColor().ToArray();
                if (ImGui::ColorEdit4("Gizmo Tint", gizmoCol.data(), ImGuiColorEditFlags_NoInputs)) {
                    Gizmo::SetColor(Color::FromArray(gizmoCol));
                }

                ImGui::Unindent();
            }
        }

        ImGui::Spacing();
        ImGui::Separator();
        if (ImGui::Button("Reload Scene", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
            SceneManager::ReloadScene(GetScene().GetName());
        }

        ImGui::End();
		ImGui::Begin("Debug Info", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

#if defined(BT_RELEASE)
		ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Bolt Engine %s (Release)", Application::GetVersion().c_str());
#else
		ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Bolt Engine %s (Debug)", Application::GetVersion().c_str());
#endif
		ImGui::Separator();

		if (ImGui::CollapsingHeader("Performance", ImGuiTreeNodeFlags_DefaultOpen))
		{
			float fpsValue = 1.f / Time::GetDeltaTimeUnscaled();
			ImVec4 fpsColor = fpsValue < 30.0f ? ImVec4(1.0f, 0.3f, 0.3f, 1.0f) : ImVec4(0.3f, 1.0f, 0.3f, 1.0f);

			ImGui::Text("FPS: "); ImGui::SameLine();
			ImGui::TextColored(fpsColor, "%.2f", fpsValue);

			ImGui::Text("Frame Count: %d", Time::GetFrameCount());
			ImGui::Text("Time Scale:  %.2f", Time::GetTimeScale());
		}

		if (ImGui::CollapsingHeader("Memory"))
		{
			auto stats = Memory::GetAllocationStats();
			size_t activeMem = stats.TotalAllocated - stats.TotalFreed;

			if (ImGui::BeginTable("MemoryTable", 2))
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0); ImGui::Text("Total Allocated:");
				ImGui::TableSetColumnIndex(1); ImGui::TextUnformatted(StringHelper::FromMemoryIEC(stats.TotalAllocated).c_str());

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0); ImGui::Text("Active Usage:");
				ImGui::TableSetColumnIndex(1); ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "%s", StringHelper::FromMemoryIEC(activeMem).c_str());

				ImGui::EndTable();
			}
		}

        if (ImGui::CollapsingHeader("Scene Manager"))
        {
            ImGui::Text("Loaded Scenes");

            SceneManager::ForeachLoadedScene([](const Scene& scene) {
                ImGui::BulletText("%s", scene.GetName().c_str());
				});
        }

		if (ImGui::CollapsingHeader("Renderer"))
		{
			ImGui::BulletText("Instances: %d", renderer2D->GetRenderedInstancesCount());
			ImGui::BulletText("Loop Duration: %.3f ms", renderer2D->GetRRenderLoopDuration());

			ImGui::Spacing();
			ImGui::TextDisabled("Window & Viewport:");
			ImGui::Indent();
			ImGui::Text("Size: %d x %d", window->GetWidth(), window->GetHeight());
			ImGui::Text("VP:   %s", StringHelper::ToString(*window->GetMainViewport()).c_str());
			ImGui::Unindent();
		}

		ImGui::End();
	}
}