#include "pch.hpp"
#include "Scripting/ScriptSystem.hpp"
#include "Scripting/ScriptEngine.hpp"
#include "Scripting/ScriptComponent.hpp"
#include "Scene/Scene.hpp"
#include "Components/Tags.hpp"
#include "Core/Log.hpp"
#include "Serialization/Path.hpp"

#include <imgui.h>
#include <filesystem>

namespace Bolt {

	void ScriptSystem::Awake(Scene& scene)
	{
		m_LastScene = &scene;

		ScriptEngine::Init();

		if (m_CoreAssemblyPath.empty())
		{
			auto exeDir = std::filesystem::path(Path::ExecutableDir());
			auto siblingPath = exeDir / ".." / "Bolt-ScriptCore" / "Bolt-ScriptCore.dll";
			if (std::filesystem::exists(siblingPath))
				m_CoreAssemblyPath = siblingPath.string();
			else
				m_CoreAssemblyPath = "Bolt-ScriptCore.dll";
		}

		if (std::filesystem::exists(m_CoreAssemblyPath))
		{
			ScriptEngine::LoadCoreAssembly(m_CoreAssemblyPath);
		}
		else
		{
			BT_WARN_TAG("ScriptSystem", "Core assembly not found at: {} — scripting disabled", m_CoreAssemblyPath);
			return;
		}

		if (m_UserAssemblyPath.empty())
		{
			auto exeDir = std::filesystem::path(Path::ExecutableDir());
			auto sandboxPath = exeDir / ".." / "Bolt-Sandbox" / "Bolt-Sandbox.dll";
			if (std::filesystem::exists(sandboxPath))
				m_UserAssemblyPath = sandboxPath.string();
		}

		if (!m_UserAssemblyPath.empty() && std::filesystem::exists(m_UserAssemblyPath))
		{
			ScriptEngine::LoadUserAssembly(m_UserAssemblyPath);
		}

		// File watcher for script hot-reload
		{
			auto exeDir = std::filesystem::path(Path::ExecutableDir());
			auto sandboxSourceDir = exeDir / ".." / ".." / ".." / "Bolt-Sandbox" / "Source";
			auto sandboxCsproj = exeDir / ".." / ".." / ".." / "Bolt-Sandbox" / "Bolt-Sandbox.csproj";

			if (std::filesystem::exists(sandboxSourceDir) && std::filesystem::exists(sandboxCsproj))
			{
				m_SandboxProjectPath = std::filesystem::canonical(sandboxCsproj).string();
				auto canonSourceDir = std::filesystem::canonical(sandboxSourceDir).string();
				m_ScriptWatcher.Watch(canonSourceDir, ".cs", [this]() {
					RebuildAndReloadScripts();
				});
			}
		}

		BT_INFO_TAG("ScriptSystem", "Script system initialized");
	}

	void ScriptSystem::RebuildAndReloadScripts()
	{
		if (m_IsRebuilding) return;

		BT_INFO_TAG("ScriptSystem", "Rebuilding scripts...");
		m_IsRebuilding = true;
		m_RebuildStartTime = std::chrono::steady_clock::now();

		std::string buildCmd = "dotnet build \"" + m_SandboxProjectPath + "\" -c Release -p:Platform=x64 --no-dependencies --nologo -v q";
		m_RebuildFuture = std::async(std::launch::async, [buildCmd]() {
			return std::system(buildCmd.c_str());
		});
	}

	void ScriptSystem::OnGui(Scene& scene)
	{
		(void)scene;
		m_ScriptWatcher.Poll(1.0f);

		if (m_IsRebuilding && m_RebuildFuture.valid())
		{
			if (m_RebuildFuture.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready)
			{
				int result = m_RebuildFuture.get();
				m_IsRebuilding = false;

				if (result == 0)
				{
					ScriptEngine::ReloadAssemblies();

					if (m_LastScene)
					{
						auto view = m_LastScene->GetRegistry().view<ScriptComponent>();
						for (auto [ent, scriptComp] : view.each())
							for (auto& inst : scriptComp.Scripts)
								inst.Unbind();
					}

					BT_INFO_TAG("ScriptSystem", "Scripts rebuilt and reloaded");
				}
				else
				{
					BT_ERROR_TAG("ScriptSystem", "Build failed (exit code {})", result);
				}
			}
		}

		if (m_IsRebuilding)
		{
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImVec2 center = ImVec2(
				viewport->Pos.x + viewport->Size.x * 0.5f,
				viewport->Pos.y + viewport->Size.y * 0.5f);

			ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
			ImGui::SetNextWindowSize(ImVec2(320, 80));
			ImGui::SetNextWindowBgAlpha(0.92f);

			ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar
				| ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
				| ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse
				| ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoNav;

			ImGui::Begin("##ScriptBuildOverlay", nullptr, flags);
			ImGui::TextUnformatted("Compiling Scripts...");
			ImGui::Spacing();

			float elapsed = std::chrono::duration<float>(
				std::chrono::steady_clock::now() - m_RebuildStartTime).count();
			ImGui::ProgressBar(fmodf(elapsed * 0.4f, 1.0f), ImVec2(-1, 0), "");

			ImGui::End();
		}
	}

	void ScriptSystem::Update(Scene& scene)
	{
		if (!ScriptEngine::IsInitialized()) return;

		m_LastScene = &scene;
		ScriptEngine::SetScene(&scene);

		auto view = scene.GetRegistry().view<ScriptComponent>(entt::exclude<DisabledTag>);

		for (auto [entity, scriptComp] : view.each())
		{
			for (auto& instance : scriptComp.Scripts)
			{
				if (instance.GetClassName().empty()) continue;

				if (!instance.IsBound())
					instance.Bind(entity);

				if (!instance.HasManagedInstance())
				{
					if (!ScriptEngine::ClassExists(instance.GetClassName()))
						continue;

					uint32_t handle = ScriptEngine::CreateScriptInstance(
						instance.GetClassName(), entity);

					if (handle == 0)
						continue;

					instance.SetGCHandle(handle);
				}

				if (!instance.HasStarted())
				{
					instance.MarkStarted();
					ScriptEngine::InvokeStart(instance.GetGCHandle());
				}

				ScriptEngine::InvokeUpdate(instance.GetGCHandle());
			}
		}
	}

	void ScriptSystem::OnDestroy(Scene& scene)
	{
		if (!ScriptEngine::IsInitialized()) return;

		ScriptEngine::SetScene(&scene);

		auto view = scene.GetRegistry().view<ScriptComponent>();
		for (auto [entity, scriptComp] : view.each())
		{
			for (auto& instance : scriptComp.Scripts)
			{
				if (instance.HasManagedInstance())
				{
					ScriptEngine::InvokeOnDestroy(instance.GetGCHandle());
					ScriptEngine::DestroyScriptInstance(instance.GetGCHandle());
					instance.SetGCHandle(0);
				}
			}
		}

		ScriptEngine::Shutdown();
	}

} // namespace Bolt
