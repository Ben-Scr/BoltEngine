#pragma once
#include "Scene/ISystem.hpp"
#include "Scripting/NativeScriptHost.hpp"
#include "Serialization/FileWatcher.hpp"
#include "Core/Export.hpp"
#include "Utils/Process.hpp"
#include <string>
#include <future>
#include <chrono>

namespace Bolt {

	class BOLT_API ScriptSystem : public ISystem {
	public:
		void Awake(Scene& scene) override;
		void Update(Scene& scene) override;
		void OnGui(Scene& scene) override;
		void OnDestroy(Scene& scene) override;

		void SetCoreAssemblyPath(const std::string& path) { m_CoreAssemblyPath = path; }
		void SetUserAssemblyPath(const std::string& path) { m_UserAssemblyPath = path; }

		/// Suppress file watcher polling (e.g. while a script is being created/renamed)
		void SetRecompileSuppressed(bool suppressed) { m_SuppressRecompile = suppressed; }
		bool IsRecompileSuppressed() const { return m_SuppressRecompile; }

	private:
		void RebuildAndReloadScripts();
		void RebuildAndReloadNativeScripts();

		bool m_SuppressRecompile = false;

		std::string m_CoreAssemblyPath;
		std::string m_UserAssemblyPath;
		std::string m_SandboxProjectPath;
		std::string m_NativeProjectDirectory;
		Scene* m_LastScene = nullptr;

		// C# hot-reload
		FileWatcher m_ScriptWatcher;
		bool m_IsRebuilding = false;
		std::future<Process::Result> m_RebuildFuture;
		std::chrono::steady_clock::time_point m_RebuildStartTime;

		// C++ native scripts
		NativeScriptHost m_NativeHost;
		FileWatcher m_NativeWatcher;
		std::string m_NativeDLLPath;
		bool m_IsRebuildingNative = false;
		std::future<Process::Result> m_NativeRebuildFuture;
		std::chrono::steady_clock::time_point m_NativeRebuildStartTime;
	};

} // namespace Bolt
