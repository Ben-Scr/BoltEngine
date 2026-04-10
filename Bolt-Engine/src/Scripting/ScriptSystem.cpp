#include "pch.hpp"
#include "Scripting/ScriptSystem.hpp"
#include "Scripting/ScriptEngine.hpp"
#include "Scripting/ScriptComponent.hpp"
#include "Scripting/NativeScript.hpp"
#include "Scene/Scene.hpp"
#include "Components/Tags.hpp"
#include "Core/Log.hpp"
#include "Core/Application.hpp"
#include "Serialization/Path.hpp"
#include "Project/ProjectManager.hpp"

#include <imgui.h>
#include <filesystem>

namespace Bolt {

	void ScriptSystem::Awake(Scene& scene)
	{
		m_LastScene = &scene;
		auto exeDir = std::filesystem::path(Path::ExecutableDir());
		BoltProject* project = ProjectManager::GetCurrentProject();

		ScriptEngine::Init();

		// Core assembly — check multiple locations
		if (m_CoreAssemblyPath.empty())
		{
			std::vector<std::filesystem::path> candidates = {
				exeDir / ".." / "Bolt-ScriptCore" / "Bolt-ScriptCore.dll",  // dev layout
				exeDir / "Bolt-ScriptCore.dll",                              // packaged build (beside exe)
			};
			for (const auto& candidate : candidates) {
				if (std::filesystem::exists(candidate)) {
					m_CoreAssemblyPath = std::filesystem::canonical(candidate).string();
					break;
				}
			}
		}

		if (std::filesystem::exists(m_CoreAssemblyPath))
			ScriptEngine::LoadCoreAssembly(m_CoreAssemblyPath);

		// User assembly — project-aware
		if (m_UserAssemblyPath.empty())
		{
			if (project)
			{
				auto userDll = std::filesystem::path(project->GetUserAssemblyOutputPath());
				if (std::filesystem::exists(userDll))
					m_UserAssemblyPath = userDll.string();
			}
			else
			{
				auto sandboxPath = exeDir / ".." / "Bolt-Sandbox" / "Bolt-Sandbox.dll";
				if (std::filesystem::exists(sandboxPath))
					m_UserAssemblyPath = sandboxPath.string();
			}
		}

		if (!m_UserAssemblyPath.empty() && std::filesystem::exists(m_UserAssemblyPath))
			ScriptEngine::LoadUserAssembly(m_UserAssemblyPath);

		// C# file watcher — project-aware
		if (project)
		{
			auto scriptsDir = std::filesystem::path(project->ScriptsDirectory);
			auto csproj = std::filesystem::path(project->CsprojPath);
			if (std::filesystem::exists(scriptsDir) && std::filesystem::exists(csproj))
			{
				m_SandboxProjectPath = std::filesystem::canonical(csproj).string();
				m_ScriptWatcher.Watch(
					std::filesystem::canonical(scriptsDir).string(), ".cs",
					[this]() { RebuildAndReloadScripts(); });
			}
		}
		else
		{
			auto sandboxSourceDir = exeDir / ".." / ".." / ".." / "Bolt-Sandbox" / "Source";
			auto sandboxCsproj = exeDir / ".." / ".." / ".." / "Bolt-Sandbox" / "Bolt-Sandbox.csproj";
			if (std::filesystem::exists(sandboxSourceDir) && std::filesystem::exists(sandboxCsproj))
			{
				m_SandboxProjectPath = std::filesystem::canonical(sandboxCsproj).string();
				m_ScriptWatcher.Watch(
					std::filesystem::canonical(sandboxSourceDir).string(), ".cs",
					[this]() { RebuildAndReloadScripts(); });
			}
		}

		// Native C++ scripting — project-aware
		if (project)
		{
			auto nativeDll = std::filesystem::path(project->GetNativeDllPath());
			if (std::filesystem::exists(nativeDll))
			{
				m_NativeDLLPath = std::filesystem::canonical(nativeDll).string();
				m_NativeHost.LoadDLL(m_NativeDLLPath);
			}
			auto nativeSourceDir = std::filesystem::path(project->NativeSourceDir);
			if (std::filesystem::exists(nativeSourceDir))
			{
				m_NativeWatcher.Watch(
					std::filesystem::canonical(nativeSourceDir).string(), ".cpp",
					[this]() { RebuildAndReloadNativeScripts(); });
			}
		}
		else
		{
			auto nativeDll = exeDir / ".." / "Bolt-NativeScripts" / "Bolt-NativeScripts.dll";
			if (std::filesystem::exists(nativeDll))
			{
				m_NativeDLLPath = std::filesystem::canonical(nativeDll).string();
				m_NativeHost.LoadDLL(m_NativeDLLPath);
			}
			auto nativeSourceDir = exeDir / ".." / ".." / ".." / "Bolt-NativeScripts" / "Source";
			if (std::filesystem::exists(nativeSourceDir))
			{
				m_NativeWatcher.Watch(
					std::filesystem::canonical(nativeSourceDir).string(), ".cpp",
					[this]() { RebuildAndReloadNativeScripts(); });
			}
		}

		BT_INFO_TAG("ScriptSystem", "Script system initialized");
	}

	// ── C# rebuild ─────────────────────────────────────────────────────

	void ScriptSystem::RebuildAndReloadScripts()
	{
		if (m_IsRebuilding) return;

		BT_INFO_TAG("ScriptSystem", "Rebuilding C# scripts...");
		m_IsRebuilding = true;
		m_RebuildStartTime = std::chrono::steady_clock::now();

		std::string buildCmd = "dotnet build \"" + m_SandboxProjectPath + "\" -c Release --nologo -v q -p:DefineConstants=BOLT_EDITOR";
		m_RebuildFuture = std::async(std::launch::async, [buildCmd]() {
			return std::system(buildCmd.c_str());
		});
	}

	// ── C++ native rebuild ─────────────────────────────────────────────

	void ScriptSystem::RebuildAndReloadNativeScripts()
	{
		if (m_IsRebuildingNative || m_NativeDLLPath.empty()) return;

		BT_INFO_TAG("ScriptSystem", "Rebuilding native scripts...");
		m_IsRebuildingNative = true;
		m_NativeRebuildStartTime = std::chrono::steady_clock::now();

		// Unload DLL before recompile so the file isn't locked
		m_NativeHost.UnloadDLL();

		auto exeDir = std::filesystem::path(Path::ExecutableDir());
		auto projectDir = std::filesystem::canonical(exeDir / ".." / ".." / ".." / "Bolt-NativeScripts").string();

		// Reconfigure first (picks up new/renamed files via GLOB_RECURSE), then build
		std::string configCmd = "cmake -B \"" + projectDir + "/build\" -S \"" + projectDir + "\" -G \"Visual Studio 17 2022\" -A x64";
		std::string buildCmd = "cmake --build \"" + projectDir + "/build\" --config Release";
		m_NativeRebuildFuture = std::async(std::launch::async, [configCmd, buildCmd]() {
			int rc = std::system(configCmd.c_str());
			if (rc != 0) return rc;
			return std::system(buildCmd.c_str());
		});
	}

	// ── OnGui: hot-reload polling + build overlay ──────────────────────

	void ScriptSystem::OnGui(Scene& scene)
	{
		(void)scene;

		// Skip file watcher polling while a script is being created/renamed
		if (!m_SuppressRecompile) {
			m_ScriptWatcher.Poll(1.0f);
			m_NativeWatcher.Poll(1.0f);
		}

		bool anyRebuilding = false;

		// C# build completion check
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
						for (auto [ent, sc] : view.each())
							for (auto& inst : sc.Scripts)
								if (inst.GetType() == ScriptType::Managed)
									inst.Unbind();
					}
					BT_INFO_TAG("ScriptSystem", "C# scripts rebuilt and reloaded");
				}
				else
					BT_ERROR_TAG("ScriptSystem", "C# build failed (exit code {})", result);
			}
		}

		// C++ build completion check
		if (m_IsRebuildingNative && m_NativeRebuildFuture.valid())
		{
			if (m_NativeRebuildFuture.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready)
			{
				int result = m_NativeRebuildFuture.get();
				m_IsRebuildingNative = false;

				if (result == 0)
				{
					m_NativeHost.LoadDLL(m_NativeDLLPath);
					if (m_LastScene)
					{
						auto view = m_LastScene->GetRegistry().view<ScriptComponent>();
						for (auto [ent, sc] : view.each())
							for (auto& inst : sc.Scripts)
								if (inst.GetType() == ScriptType::Native)
									inst.Unbind();
					}
					BT_INFO_TAG("ScriptSystem", "Native scripts rebuilt and reloaded");
				}
				else
					BT_ERROR_TAG("ScriptSystem", "Native build failed (exit code {})", result);
			}
		}

		anyRebuilding = m_IsRebuilding || m_IsRebuildingNative;

		// Build overlay
		if (anyRebuilding)
		{
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImVec2 center(viewport->Pos.x + viewport->Size.x * 0.5f,
			              viewport->Pos.y + viewport->Size.y * 0.5f);

			ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
			ImGui::SetNextWindowSize(ImVec2(320, 80));
			ImGui::SetNextWindowBgAlpha(0.92f);

			ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
				| ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar
				| ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoNav;

			ImGui::Begin("##ScriptBuildOverlay", nullptr, flags);
			ImGui::TextUnformatted(m_IsRebuildingNative ? "Compiling Native Scripts..." : "Compiling Scripts...");
			ImGui::Spacing();

			auto startTime = m_IsRebuildingNative ? m_NativeRebuildStartTime : m_RebuildStartTime;
			float elapsed = std::chrono::duration<float>(std::chrono::steady_clock::now() - startTime).count();
			ImGui::ProgressBar(fmodf(elapsed * 0.4f, 1.0f), ImVec2(-1, 0), "");

			ImGui::End();
		}
	}

	// ── Update: dual dispatch (Managed + Native) ───────────────────────

	void ScriptSystem::Update(Scene& scene)
	{
		if (!ScriptEngine::IsInitialized()) return;

		m_LastScene = &scene;
		ScriptEngine::SetScene(&scene);

		float dt = Application::GetInstance() ? Application::GetInstance()->GetTime().GetDeltaTime() : 0.0f;

		auto view = scene.GetRegistry().view<ScriptComponent>(entt::exclude<DisabledTag>);

		for (auto [entity, scriptComp] : view.each())
		{
			for (auto& instance : scriptComp.Scripts)
			{
				if (instance.GetClassName().empty()) continue;

				if (!instance.IsBound())
					instance.Bind(entity);

				if (!instance.HasAnyInstance())
				{
					// Try C# (managed) first
					if (ScriptEngine::ClassExists(instance.GetClassName()))
					{
						uint32_t handle = ScriptEngine::CreateScriptInstance(
							instance.GetClassName(), entity);
						if (handle != 0)
							instance.SetGCHandle(handle);
					}
					// Then try native C++
					else if (m_NativeHost.IsLoaded())
					{
						NativeScript* native = m_NativeHost.CreateInstance(
							instance.GetClassName(), entity, &scene);
						if (native)
							instance.SetNativePtr(native);
					}

					if (!instance.HasAnyInstance())
						continue;
				}

				// Dispatch lifecycle based on script type
				if (instance.GetType() == ScriptType::Managed)
				{
					if (!instance.HasStarted())
					{
						instance.MarkStarted();
						ScriptEngine::InvokeStart(instance.GetGCHandle());
					}
					ScriptEngine::InvokeUpdate(instance.GetGCHandle());
				}
				else if (instance.GetType() == ScriptType::Native)
				{
					if (!instance.HasStarted())
					{
						instance.MarkStarted();
						instance.GetNativePtr()->Start();
					}
					instance.GetNativePtr()->Update(dt);
				}
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
				if (instance.HasNativeInstance())
				{
					m_NativeHost.DestroyInstance(instance.GetNativePtr());
					instance.SetNativePtr(nullptr);
				}
			}
		}

		m_NativeHost.UnloadDLL();
		ScriptEngine::Shutdown();
	}

} // namespace Bolt
