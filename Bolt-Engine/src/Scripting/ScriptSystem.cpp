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
	namespace {
		std::string GetNativeLibraryFilename(const std::string& targetName)
		{
#if defined(BT_PLATFORM_WINDOWS)
			return targetName + ".dll";
#elif defined(BT_PLATFORM_LINUX)
			return "lib" + targetName + ".so";
#else
#error Unsupported platform for native scripts
#endif
		}

		std::filesystem::path NormalizeNativePath(std::filesystem::path path)
		{
			std::error_code ec;
			if (std::filesystem::exists(path)) {
				const std::filesystem::path canonicalPath = std::filesystem::weakly_canonical(path, ec);
				if (!ec) {
					return canonicalPath;
				}
			}

			return path.lexically_normal();
		}

		std::filesystem::path ResolveProjectNativeDLLPath(const BoltProject& project)
		{
			return NormalizeNativePath(std::filesystem::path(project.GetNativeDllPath()));
		}

		std::filesystem::path ResolveStandaloneNativeDLLPath(const std::filesystem::path& executableDirectory)
		{
			return NormalizeNativePath(
				executableDirectory / ".." / "Bolt-NativeScripts" / GetNativeLibraryFilename("Bolt-NativeScripts"));
		}

		std::string GetActiveNativeBuildConfig()
		{
			return BT_BUILD_CONFIG_NAME;
		}
	}

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
			if (std::filesystem::exists(project->NativeScriptsDir)) {
				m_NativeProjectDirectory = std::filesystem::canonical(project->NativeScriptsDir).string();
			}
			const std::filesystem::path nativeDll = ResolveProjectNativeDLLPath(*project);
			m_NativeDLLPath = nativeDll.string();
			if (std::filesystem::exists(nativeDll))
			{
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
			auto nativeProjectDir = exeDir / ".." / ".." / ".." / "Bolt-NativeScripts";
			if (std::filesystem::exists(nativeProjectDir)) {
				m_NativeProjectDirectory = std::filesystem::canonical(nativeProjectDir).string();
			}
			const std::filesystem::path nativeDll = ResolveStandaloneNativeDLLPath(exeDir);
			m_NativeDLLPath = nativeDll.string();
			if (std::filesystem::exists(nativeDll))
			{
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

		const std::string sandboxProjectPath = m_SandboxProjectPath;
		m_RebuildFuture = std::async(std::launch::async, [sandboxProjectPath]() {
			return Process::Run({
				"dotnet",
				"build",
				sandboxProjectPath,
				"-c", "Release",
				"--nologo",
				"-v", "q",
				"-p:DefineConstants=BOLT_EDITOR%3BBT_RELEASE"
			});
		});
	}

	// ── C++ native rebuild ─────────────────────────────────────────────

	void ScriptSystem::RebuildAndReloadNativeScripts()
	{
		if (m_IsRebuildingNative || m_NativeProjectDirectory.empty()) return;

		BT_INFO_TAG("ScriptSystem", "Rebuilding native scripts...");
		m_IsRebuildingNative = true;
		m_NativeRebuildStartTime = std::chrono::steady_clock::now();

		if (m_LastScene) {
			TeardownNativeScripts(*m_LastScene);
		}

		// Unload after scene instances are unbound so no stale native pointers survive the frame.
		m_NativeHost.UnloadDLL();

		const std::string nativeProjectDirectory = m_NativeProjectDirectory;
		const std::string buildConfig = GetActiveNativeBuildConfig();
		m_NativeRebuildFuture = std::async(std::launch::async, [nativeProjectDirectory, buildConfig]() {
			const std::filesystem::path buildDirectory = std::filesystem::path(nativeProjectDirectory) / "build";

			Process::Result configureResult = Process::Run({
				"cmake",
				"-B", buildDirectory.string(),
				"-S", nativeProjectDirectory,
				"-DCMAKE_BUILD_TYPE=" + buildConfig
			}, nativeProjectDirectory);
			if (!configureResult.Succeeded()) {
				return configureResult;
			}

			Process::Result buildResult = Process::Run({
				"cmake",
				"--build", buildDirectory.string(),
				"--config", buildConfig
			}, nativeProjectDirectory);
			if (!configureResult.Output.empty()) {
				buildResult.Output = configureResult.Output + buildResult.Output;
			}
			return buildResult;
		});
	}

	// ── OnGui: hot-reload polling + build overlay ──────────────────────

	void ScriptSystem::TeardownManagedScripts(Scene& scene)
	{
		if (!ScriptEngine::IsInitialized()) {
			return;
		}

		ScriptEngine::SetScene(&scene);

		auto view = scene.GetRegistry().view<ScriptComponent>();
		for (auto [entity, scriptComp] : view.each())
		{
			for (auto& instance : scriptComp.Scripts)
			{
				if (!instance.HasManagedInstance()) {
					continue;
				}

				ScriptEngine::InvokeOnDestroy(instance.GetGCHandle());
				ScriptEngine::DestroyScriptInstance(instance.GetGCHandle());
				instance.Unbind();
			}
		}
	}

	void ScriptSystem::TeardownNativeScripts(Scene& scene)
	{
		ScriptEngine::SetScene(&scene);
		const bool canDestroyNativeInstances = m_NativeHost.IsLoaded();

		auto view = scene.GetRegistry().view<ScriptComponent>();
		for (auto [entity, scriptComp] : view.each())
		{
			for (auto& instance : scriptComp.Scripts)
			{
				if (!instance.HasNativeInstance()) {
					continue;
				}

				if (canDestroyNativeInstances) {
					m_NativeHost.DestroyInstance(instance.GetNativePtr());
				}
				instance.Unbind();
			}
		}
	}

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
				Process::Result result = m_RebuildFuture.get();
				m_IsRebuilding = false;

				if (result.Succeeded())
				{
					if (m_LastScene) {
						TeardownManagedScripts(*m_LastScene);
					}
					ScriptEngine::ReloadAssemblies();
					BT_INFO_TAG("ScriptSystem", "C# scripts rebuilt and reloaded");
				}
				else
				{
					BT_ERROR_TAG("ScriptSystem", "C# build failed (exit code {})", result.ExitCode);
					if (!result.Output.empty()) {
						BT_ERROR_TAG("ScriptSystem", "{}", result.Output);
					}
				}
			}
		}

		// C++ build completion check
		if (m_IsRebuildingNative && m_NativeRebuildFuture.valid())
		{
			if (m_NativeRebuildFuture.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready)
			{
				Process::Result result = m_NativeRebuildFuture.get();
				m_IsRebuildingNative = false;

				if (result.Succeeded())
				{
					const std::filesystem::path exeDir = std::filesystem::path(Path::ExecutableDir());
					if (BoltProject* project = ProjectManager::GetCurrentProject()) {
						m_NativeDLLPath = ResolveProjectNativeDLLPath(*project).string();
					}
					else {
						m_NativeDLLPath = ResolveStandaloneNativeDLLPath(exeDir).string();
					}

					if (!std::filesystem::exists(m_NativeDLLPath) || !m_NativeHost.LoadDLL(m_NativeDLLPath)) {
						BT_ERROR_TAG("ScriptSystem", "Native scripts built, but failed to load '{}'", m_NativeDLLPath);
					}
					else {
						BT_INFO_TAG("ScriptSystem", "Native scripts rebuilt and reloaded");
					}
				}
				else
				{
					BT_ERROR_TAG("ScriptSystem", "Native build failed (exit code {})", result.ExitCode);
					if (!result.Output.empty()) {
						BT_ERROR_TAG("ScriptSystem", "{}", result.Output);
					}
				}
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
					else if (!m_IsRebuildingNative && m_NativeHost.IsLoaded())
					{
						NativeScript* native = m_NativeHost.CreateInstance(
							instance.GetClassName(), entity, &scene);
						if (native)
							instance.SetNativePtr(native);
					}

					if (!instance.HasAnyInstance())
						continue;

					// Apply pending [ShowInEditor] field values from deserialization
					if (instance.HasManagedInstance() && !scriptComp.PendingFieldValues.empty()) {
						std::string prefix = instance.GetClassName() + ".";
						auto& callbacks = ScriptEngine::GetCallbacks();
						if (callbacks.SetScriptField) {
							for (auto it = scriptComp.PendingFieldValues.begin(); it != scriptComp.PendingFieldValues.end(); ) {
								if (it->first.rfind(prefix, 0) == 0) {
									std::string fieldName = it->first.substr(prefix.size());
									callbacks.SetScriptField(
										static_cast<int32_t>(instance.GetGCHandle()),
										fieldName.c_str(), it->second.c_str());
									it = scriptComp.PendingFieldValues.erase(it);
								} else {
									++it;
								}
							}
						}
					}
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
					if (m_IsRebuildingNative || !instance.HasNativeInstance()) {
						continue;
					}

					NativeScript* nativeInstance = instance.GetNativePtr();
					if (!nativeInstance) {
						instance.Unbind();
						continue;
					}

					if (!instance.HasStarted())
					{
						instance.MarkStarted();
						nativeInstance->Start();
					}
					nativeInstance->Update(dt);
				}
			}
		}
	}

	void ScriptSystem::OnDestroy(Scene& scene)
	{
		m_ScriptWatcher.Stop();
		m_NativeWatcher.Stop();

		if (m_IsRebuilding && m_RebuildFuture.valid()) {
			(void)m_RebuildFuture.get();
			m_IsRebuilding = false;
		}
		if (m_IsRebuildingNative && m_NativeRebuildFuture.valid()) {
			(void)m_NativeRebuildFuture.get();
			m_IsRebuildingNative = false;
		}

		if (ScriptEngine::IsInitialized()) {
			TeardownManagedScripts(scene);
			TeardownNativeScripts(scene);
			ScriptEngine::Shutdown();
		}

		m_NativeHost.UnloadDLL();
		m_LastScene = nullptr;
		m_NativeProjectDirectory.clear();
	}

} // namespace Bolt
