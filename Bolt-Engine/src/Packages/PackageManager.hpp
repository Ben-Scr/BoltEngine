#pragma once
#include "Packages/PackageSource.hpp"
#include "Core/Export.hpp"

#include <memory>
#include <future>
#include <string>
#include <vector>

namespace Bolt {

	class BOLT_API PackageManager {
	public:
		void Initialize(const std::string& toolExePath);
		void Shutdown();

		bool IsReady() const { return m_IsReady; }
		const std::string& GetToolPath() const { return m_ToolExePath; }

		void AddSource(std::unique_ptr<PackageSource> source);
		const std::vector<std::unique_ptr<PackageSource>>& GetSources() const { return m_Sources; }
		PackageSource* GetSource(int index);

		// Async operations — return futures polled by the UI each frame
		std::future<std::vector<PackageInfo>> SearchAsync(int sourceIndex,
			const std::string& query, int take = 20);

		std::future<PackageOperationResult> InstallAsync(int sourceIndex,
			const std::string& packageId, const std::string& version);

		std::future<PackageOperationResult> RemoveAsync(int sourceIndex,
			const std::string& packageId);

		// After install/remove: restore + rebuild + signal reload
		PackageOperationResult RestoreAndRebuild();

		// Read installed packages from the .csproj
		std::vector<PackageInfo> GetInstalledPackages() const;

		bool NeedsReload() const { return m_NeedsReload; }
		void ClearReloadFlag() { m_NeedsReload = false; }

	private:
		std::string GetCsprojPath() const;

		std::vector<std::unique_ptr<PackageSource>> m_Sources;
		std::string m_ToolExePath;
		bool m_IsReady = false;
		bool m_NeedsReload = false;
	};

}
