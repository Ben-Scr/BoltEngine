#include "pch.hpp"
#include "Packages/PackageManager.hpp"
#include "Project/ProjectManager.hpp"
#include "Serialization/Path.hpp"
#include "Serialization/File.hpp"
#include "Utils/Process.hpp"

#include <fstream>
#include <regex>
#include <filesystem>

namespace Bolt {

	void PackageManager::Initialize(const std::string& toolExePath) {
		// Build candidate paths — always try canonical resolution
		auto exeDir = std::filesystem::path(Path::ExecutableDir());
		std::vector<std::filesystem::path> candidates = {
			std::filesystem::path(toolExePath),
			exeDir / ".." / ".." / ".." / "Bolt-PackageTool" / "bin" / "Release" / "net9.0" / "Bolt-PackageTool.exe",
			exeDir / "Bolt-PackageTool.exe"
		};

		for (const auto& candidate : candidates) {
			if (std::filesystem::exists(candidate)) {
				m_ToolExePath = std::filesystem::canonical(candidate).string();
				BT_CORE_INFO_TAG("PackageManager", "Found package tool at {}", m_ToolExePath);
				break;
			}
		}

		if (!m_ToolExePath.empty() && std::filesystem::exists(m_ToolExePath)) {
			m_IsReady = true;
			BT_CORE_INFO_TAG("PackageManager", "Package manager initialized");
		}
		else {
			// Try building the tool
			auto exeDir = std::filesystem::path(Path::ExecutableDir());
			auto projectDir = exeDir / ".." / ".." / ".." / "Bolt-PackageTool";
			if (std::filesystem::exists(projectDir / "Bolt-PackageTool.csproj")) {
				BT_CORE_INFO_TAG("PackageManager", "Building package tool...");
				const Process::Result buildResult = Process::Run({
					"dotnet",
					"build",
					projectDir.string(),
					"-c", "Release",
					"--nologo",
					"-v", "q"
				});
				if (buildResult.Succeeded()) {
					auto builtPath = projectDir / "bin" / "Release" / "net9.0" / "Bolt-PackageTool.exe";
					if (std::filesystem::exists(builtPath)) {
						m_ToolExePath = std::filesystem::canonical(builtPath).string();
						m_IsReady = true;
						BT_CORE_INFO_TAG("PackageManager", "Package tool built and ready");
					}
				}
				else {
					BT_CORE_ERROR_TAG("PackageManager", "Failed to build package tool (exit code {})", buildResult.ExitCode);
					if (!buildResult.Output.empty()) {
						BT_CORE_ERROR_TAG("PackageManager", "{}", buildResult.Output);
					}
				}
			}
			else {
				BT_CORE_WARN_TAG("PackageManager", "Package tool project not found, package manager disabled");
			}
		}
	}

	void PackageManager::Shutdown() {
		m_Sources.clear();
		m_IsReady = false;
	}

	void PackageManager::AddSource(std::unique_ptr<PackageSource> source) {
		m_Sources.push_back(std::move(source));
	}

	PackageSource* PackageManager::GetSource(int index) {
		if (index < 0 || index >= static_cast<int>(m_Sources.size()))
			return nullptr;
		return m_Sources[index].get();
	}

	std::string PackageManager::GetCsprojPath() const {
		BoltProject* project = ProjectManager::GetCurrentProject();
		return project ? project->CsprojPath : "";
	}

	std::future<std::vector<PackageInfo>> PackageManager::SearchAsync(int sourceIndex,
		const std::string& query, int take) {

		auto* source = GetSource(sourceIndex);
		if (!source) {
			return std::async(std::launch::deferred, []() -> std::vector<PackageInfo> { return {}; });
		}

		// Mark installed packages
		auto installed = GetInstalledPackages();

		return std::async(std::launch::async, [source, query, take, installed]() -> std::vector<PackageInfo> {
			auto results = source->Search(query, take);

			// Cross-reference with installed packages
			for (auto& pkg : results) {
				for (const auto& inst : installed) {
					if (pkg.Id == inst.Id) {
						pkg.IsInstalled = true;
						pkg.InstalledVersion = inst.Version;
						break;
					}
				}
			}
			return results;
		});
	}

	std::future<PackageOperationResult> PackageManager::InstallAsync(int sourceIndex,
		const std::string& packageId, const std::string& version) {

		auto* source = GetSource(sourceIndex);
		std::string csproj = GetCsprojPath();

		if (!source || csproj.empty()) {
			return std::async(std::launch::deferred, []() -> PackageOperationResult {
				return { false, "No source or project loaded" };
			});
		}

		return std::async(std::launch::async, [this, source, packageId, version, csproj]() -> PackageOperationResult {
			auto result = source->Install(packageId, version, csproj);
			if (result.Success) {
				auto rebuild = RestoreAndRebuild();
				if (!rebuild.Success)
					return { false, "Install succeeded but rebuild failed: " + rebuild.Message };
				m_NeedsReload = true;
			}
			return result;
		});
	}

	std::future<PackageOperationResult> PackageManager::RemoveAsync(int sourceIndex,
		const std::string& packageId) {

		auto* source = GetSource(sourceIndex);
		std::string csproj = GetCsprojPath();

		if (!source || csproj.empty()) {
			return std::async(std::launch::deferred, []() -> PackageOperationResult {
				return { false, "No source or project loaded" };
			});
		}

		return std::async(std::launch::async, [this, source, packageId, csproj]() -> PackageOperationResult {
			auto result = source->Remove(packageId, csproj);
			if (result.Success) {
				auto rebuild = RestoreAndRebuild();
				if (!rebuild.Success)
					return { false, "Remove succeeded but rebuild failed: " + rebuild.Message };
				m_NeedsReload = true;
			}
			return result;
		});
	}

	PackageOperationResult PackageManager::RestoreAndRebuild() {
		std::string csproj = GetCsprojPath();
		if (csproj.empty())
			return { false, "No project loaded" };

		// Step 1: dotnet restore
		Process::Result restoreResult = Process::Run({
			"dotnet",
			"restore",
			csproj,
			"--nologo",
			"-v", "q"
		});
		if (!restoreResult.Succeeded()) {
			BT_CORE_ERROR_TAG("PackageManager", "dotnet restore failed (exit code {})", restoreResult.ExitCode);
			if (!restoreResult.Output.empty()) {
				BT_CORE_ERROR_TAG("PackageManager", "{}", restoreResult.Output);
			}
			return { false, "dotnet restore failed" };
		}

		// Step 2: dotnet build
		Process::Result buildResult = Process::Run({
			"dotnet",
			"build",
			csproj,
			"-c", "Release",
			"--nologo",
			"-v", "q"
		});
		if (!buildResult.Succeeded()) {
			BT_CORE_ERROR_TAG("PackageManager", "dotnet build failed (exit code {})", buildResult.ExitCode);
			if (!buildResult.Output.empty()) {
				BT_CORE_ERROR_TAG("PackageManager", "{}", buildResult.Output);
			}
			return { false, "dotnet build failed" };
		}

		BT_CORE_INFO_TAG("PackageManager", "Restore and rebuild complete");
		return { true, "Build succeeded" };
	}

	std::vector<PackageInfo> PackageManager::GetInstalledPackages() const {
		std::vector<PackageInfo> installed;

		std::string csproj = GetCsprojPath();
		if (csproj.empty() || !File::Exists(csproj))
			return installed;

		std::ifstream in(csproj);
		if (!in.is_open()) return installed;
		std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
		in.close();

		// Find all PackageReference entries
		std::regex pkgRegex(R"rx(<PackageReference\s+Include="([^"]+)"\s+Version="([^"]+)")rx");
		std::sregex_iterator it(content.begin(), content.end(), pkgRegex);
		std::sregex_iterator end;

		for (; it != end; ++it) {
			PackageInfo info;
			info.Id = (*it)[1].str();
			info.Version = (*it)[2].str();
			info.IsInstalled = true;
			info.InstalledVersion = info.Version;
			info.SourceType = PackageSourceType::NuGet;
			info.SourceName = "NuGet";
			installed.push_back(std::move(info));
		}

		// Find all Reference entries (DLL engine packages)
		std::regex refRegex(R"rx(<Reference\s+Include="([^"]+)")rx");
		std::sregex_iterator rit(content.begin(), content.end(), refRegex);

		for (; rit != end; ++rit) {
			std::string name = (*rit)[1].str();
			// Skip system references
			if (name.find("System") == 0 || name.find("Microsoft") == 0)
				continue;

			PackageInfo info;
			info.Id = name;
			info.IsInstalled = true;
			info.SourceType = PackageSourceType::GitHub;
			info.SourceName = "Engine";
			installed.push_back(std::move(info));
		}

		return installed;
	}

}
