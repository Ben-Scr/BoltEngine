#include "pch.hpp"
#include "Project/BoltProject.hpp"
#include "Serialization/Path.hpp"
#include "Serialization/Directory.hpp"
#include "Serialization/File.hpp"
#include "Serialization/Json.hpp"
#include "Core/Log.hpp"
#include "Core/Version.hpp"

#include <filesystem>
#include <fstream>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <random>

namespace Bolt {

	static std::string GenerateGUID() {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<uint32_t> dist(0, 0xFFFFFFFF);

		auto hex = [&](int bytes) {
			std::stringstream ss;
			ss << std::hex << std::setfill('0');
			for (int i = 0; i < bytes; i++)
				ss << std::setw(2) << (dist(gen) & 0xFF);
			return ss.str();
		};

		return hex(4) + "-" + hex(2) + "-" + hex(2) + "-" + hex(2) + "-" + hex(6);
	}

	static std::string NowISO8601() {
		auto t = std::time(nullptr);
		std::tm tm{};
		localtime_s(&tm, &t);
		std::stringstream ss;
		ss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S");
		return ss.str();
	}

	static void ResolvePaths(BoltProject& p) {
		p.AssetsDirectory = Path::Combine(p.RootDirectory, "Assets");
		p.ScriptsDirectory = Path::Combine(p.AssetsDirectory, "Scripts");
		p.ScenesDirectory = Path::Combine(p.AssetsDirectory, "Scenes");
		p.BoltAssetsDirectory = Path::Combine(p.RootDirectory, "BoltAssets");
		p.NativeScriptsDir = Path::Combine(p.RootDirectory, "NativeScripts");
		p.NativeSourceDir = Path::Combine(p.NativeScriptsDir, "Source");
		p.PackagesDirectory = Path::Combine(p.RootDirectory, "Packages");
		p.CsprojPath = Path::Combine(p.RootDirectory, p.Name + ".csproj");
		p.SlnPath = Path::Combine(p.RootDirectory, p.Name + ".sln");
		p.ProjectFilePath = Path::Combine(p.RootDirectory, "bolt-project.json");
	}

	static Json::Value BuildProjectJson(const BoltProject& project) {
		Json::Value root = Json::Value::MakeObject();
		root.AddMember("name", project.Name);
		root.AddMember("engineVersion", project.EngineVersion);
		root.AddMember("startupScene", project.StartupScene);
		root.AddMember("lastOpenedScene", project.LastOpenedScene);
		root.AddMember("buildWidth", project.BuildWidth);
		root.AddMember("buildHeight", project.BuildHeight);
		root.AddMember("buildFullscreen", project.BuildFullscreen);
		root.AddMember("buildResizable", project.BuildResizable);
		if (!project.AppIconPath.empty()) {
			root.AddMember("appIcon", project.AppIconPath);
		}

		Json::Value buildScenes = Json::Value::MakeArray();
		for (const std::string& sceneName : project.BuildSceneList) {
			buildScenes.Append(sceneName);
		}
		root.AddMember("buildScenes", std::move(buildScenes));
		return root;
	}

	std::string BoltProject::GetUserAssemblyOutputPath() const {
		return Path::Combine(RootDirectory, "bin", "Release", Name + ".dll");
	}

	std::string BoltProject::GetNativeDllPath() const {
		return Path::Combine(NativeScriptsDir, "build", "Release", Name + "-NativeScripts.dll");
	}

	std::string BoltProject::GetSceneFilePath(const std::string& sceneName) const {
		return Path::Combine(ScenesDirectory, sceneName + ".scene");
	}

	void BoltProject::Save() const {
		File::WriteAllText(ProjectFilePath, Json::Stringify(BuildProjectJson(*this), true));
	}

	std::string BoltProject::GetDefaultProjectsDir() {
		return Path::Combine(Path::GetSpecialFolderPath(SpecialFolder::User), "Bolt", "Projects");
	}

	bool BoltProject::IsValidProjectName(const std::string& name) {
		if (name.empty() || name.size() > 64) return false;
		if (name[0] == '.') return false;
		for (char c : name)
			if (c == '<' || c == '>' || c == ':' || c == '"' || c == '/' || c == '\\' || c == '|' || c == '?' || c == '*')
				return false;
		return true;
	}

	bool BoltProject::Validate(const std::string& rootDir) {
		return std::filesystem::exists(Path::Combine(rootDir, "bolt-project.json"))
			&& std::filesystem::exists(Path::Combine(rootDir, "Assets"));
	}

	BoltProject BoltProject::Load(const std::string& rootDir) {
		BoltProject project;
		project.RootDirectory = rootDir;

		std::string configPath = Path::Combine(rootDir, "bolt-project.json");
		if (File::Exists(configPath)) {
			std::string jsonText = File::ReadAllText(configPath);
			Json::Value root;
			std::string parseError;
			if (!jsonText.empty() && Json::TryParse(jsonText, root, &parseError) && root.IsObject()) {
				if (const Json::Value* nameValue = root.FindMember("name")) {
					project.Name = nameValue->AsStringOr();
				}
				if (const Json::Value* versionValue = root.FindMember("engineVersion")) {
					project.EngineVersion = versionValue->AsStringOr();
				}
				if (const Json::Value* startupValue = root.FindMember("startupScene")) {
					const std::string startupScene = startupValue->AsStringOr();
					if (!startupScene.empty()) {
						project.StartupScene = startupScene;
					}
				}
				if (const Json::Value* lastSceneValue = root.FindMember("lastOpenedScene")) {
					const std::string lastScene = lastSceneValue->AsStringOr();
					if (!lastScene.empty()) {
						project.LastOpenedScene = lastScene;
					}
				}
				if (const Json::Value* buildWidthValue = root.FindMember("buildWidth")) {
					project.BuildWidth = buildWidthValue->AsIntOr(1280);
				}
				if (const Json::Value* buildHeightValue = root.FindMember("buildHeight")) {
					project.BuildHeight = buildHeightValue->AsIntOr(720);
				}
				if (const Json::Value* fullscreenValue = root.FindMember("buildFullscreen")) {
					project.BuildFullscreen = fullscreenValue->AsBoolOr(false);
				}
				if (const Json::Value* resizableValue = root.FindMember("buildResizable")) {
					project.BuildResizable = resizableValue->AsBoolOr(true);
				}
				if (const Json::Value* iconValue = root.FindMember("appIcon")) {
					project.AppIconPath = iconValue->AsStringOr();
				}
				if (const Json::Value* buildScenesValue = root.FindMember("buildScenes")) {
					project.BuildSceneList.clear();
					for (const Json::Value& sceneValue : buildScenesValue->GetArray()) {
						const std::string sceneName = sceneValue.AsStringOr();
						if (!sceneName.empty()) {
							project.BuildSceneList.push_back(sceneName);
						}
					}
				}
			}
			else if (!jsonText.empty()) {
				BT_CORE_WARN_TAG("BoltProject", "Failed to parse '{}': {}", configPath, parseError);
			}
		}

		if (project.Name.empty())
			project.Name = std::filesystem::path(rootDir).filename().string();
		if (project.EngineVersion.empty())
			project.EngineVersion = BT_VERSION;

		ResolvePaths(project);
		return project;
	}

	BoltProject BoltProject::Create(const std::string& name, const std::string& parentDir) {
		BoltProject project;
		project.Name = name;
		project.RootDirectory = Path::Combine(parentDir, name);
		project.EngineVersion = BT_VERSION;
		ResolvePaths(project);

		// Create directory tree
		Directory::Create(project.RootDirectory);
		Directory::Create(project.AssetsDirectory);
		Directory::Create(project.ScriptsDirectory);
		Directory::Create(project.ScenesDirectory);
		Directory::Create(project.BoltAssetsDirectory);
		Directory::Create(project.NativeScriptsDir);
		Directory::Create(project.NativeSourceDir);
		Directory::Create(project.PackagesDirectory);
		Directory::Create(Path::Combine(project.RootDirectory, "bin"));
		Directory::Create(Path::Combine(project.RootDirectory, ".vscode"));

		// Copy engine default assets into the project's BoltAssets directory
		std::string engineAssets = Path::ResolveBoltAssets("");
		if (!engineAssets.empty() && Directory::Exists(engineAssets)) {
			try {
				std::filesystem::copy(engineAssets, project.BoltAssetsDirectory,
					std::filesystem::copy_options::recursive | std::filesystem::copy_options::skip_existing);
			} catch (...) {}
		}

		// Write bolt-project.json
		{
			Json::Value root = Json::Value::MakeObject();
			root.AddMember("name", name);
			root.AddMember("engineVersion", std::string(BT_VERSION));
			root.AddMember("createdAt", NowISO8601());
			File::WriteAllText(project.ProjectFilePath, Json::Stringify(root, true));
		}

		// Resolve path to Bolt-ScriptCore for ProjectReference
		auto exeDir = std::filesystem::path(Path::ExecutableDir());
		std::string scriptCorePath;
		auto coreRelative = exeDir / ".." / ".." / ".." / "Bolt-ScriptCore" / "Bolt-ScriptCore.csproj";
		if (std::filesystem::exists(coreRelative))
			scriptCorePath = std::filesystem::canonical(coreRelative).string();
		else
			scriptCorePath = "..\\Bolt-ScriptCore\\Bolt-ScriptCore.csproj";

		// Preserve existing NuGet PackageReference entries if .csproj already exists
		std::string existingPackageRefs;
		if (File::Exists(project.CsprojPath)) {
			std::ifstream existing(project.CsprojPath);
			std::string content((std::istreambuf_iterator<char>(existing)), std::istreambuf_iterator<char>());
			existing.close();

			// Extract all ItemGroup blocks that contain PackageReference
			size_t searchPos = 0;
			while (searchPos < content.size()) {
				size_t igStart = content.find("<ItemGroup>", searchPos);
				if (igStart == std::string::npos) break;
				size_t igEnd = content.find("</ItemGroup>", igStart);
				if (igEnd == std::string::npos) break;
				igEnd += 12; // length of "</ItemGroup>"

				std::string block = content.substr(igStart, igEnd - igStart);
				if (block.find("PackageReference") != std::string::npos
					|| block.find("<Reference ") != std::string::npos) {
					existingPackageRefs += "  " + block + "\n";
				}
				searchPos = igEnd;
			}
		}

		// Generate .csproj
		// Resolve the Bolt-ScriptCore DLL path relative to the project root.
		// We reference the compiled DLL instead of the .csproj so that
		// Bolt-ScriptCore does not appear in the user's Solution Explorer.
		std::string scriptCoreDllPath = Path::Combine(
			std::filesystem::path(scriptCorePath).parent_path().string(),
			"..", "bin", "Release-windows-x86_64", "Bolt-ScriptCore", "Bolt-ScriptCore.dll");
		// Normalize to absolute
		if (std::filesystem::exists(scriptCoreDllPath))
			scriptCoreDllPath = std::filesystem::canonical(scriptCoreDllPath).string();

		std::string csproj = R"(<Project Sdk="Microsoft.NET.Sdk">
  <PropertyGroup>
    <OutputType>Library</OutputType>
    <TargetFramework>net9.0</TargetFramework>
    <AllowUnsafeBlocks>true</AllowUnsafeBlocks>
    <EnableDynamicLoading>true</EnableDynamicLoading>
    <CopyLocalLockFileAssemblies>true</CopyLocalLockFileAssemblies>
    <EnableDefaultCompileItems>false</EnableDefaultCompileItems>
    <EnableDefaultNoneItems>false</EnableDefaultNoneItems>
    <Nullable>enable</Nullable>
    <AppendTargetFrameworkToOutputPath>false</AppendTargetFrameworkToOutputPath>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)' == 'Debug'">
    <OutputPath>bin\Debug\</OutputPath>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)' == 'Release'">
    <OutputPath>bin\Release\</OutputPath>
    <Optimize>true</Optimize>
  </PropertyGroup>
  <ItemGroup>
    <Compile Include="Assets\Scripts\**\*.cs" />
  </ItemGroup>
  <ItemGroup>
    <Reference Include="Bolt-ScriptCore">
      <HintPath>)" + scriptCoreDllPath + R"(</HintPath>
      <Private>false</Private>
    </Reference>
  </ItemGroup>
)" + existingPackageRefs + R"(</Project>
)";
		File::WriteAllText(project.CsprojPath, csproj);

		// Generate .sln — only the user project is visible.
		// Bolt-ScriptCore is linked via DLL reference in the .csproj,
		// so it does not need a project entry here.
		std::string projGuid = GenerateGUID();

		std::string sln = R"(
Microsoft Visual Studio Solution File, Format Version 12.00
# Visual Studio Version 17
Project("{FAE04EC0-301F-11D3-BF4B-00C04F79EFBC}") = ")" + name + R"(", ")" + name + R"(.csproj", "{)" + projGuid + R"(}"
EndProject
Global
	GlobalSection(SolutionConfigurationPlatforms) = preSolution
		Debug|Any CPU = Debug|Any CPU
		Release|Any CPU = Release|Any CPU
	EndGlobalSection
	GlobalSection(ProjectConfigurationPlatforms) = postSolution
		{)" + projGuid + R"(}.Debug|Any CPU.ActiveCfg = Debug|AnyCPU
		{)" + projGuid + R"(}.Debug|Any CPU.Build.0 = Debug|AnyCPU
		{)" + projGuid + R"(}.Release|Any CPU.ActiveCfg = Release|AnyCPU
		{)" + projGuid + R"(}.Release|Any CPU.Build.0 = Release|AnyCPU
	EndGlobalSection
	GlobalSection(SolutionProperties) = preSolution
		HideSolutionNode = FALSE
	EndGlobalSection
EndGlobal
)";
		File::WriteAllText(project.SlnPath, sln);

		// Generate starter script
		std::string starterScript = R"(using Bolt;

public class GameScript : BoltScript
{
    public void Start()
    {
        Log.Info("Hello from )" + name + R"(!");
    }

    public void Update()
    {
    }
}
)";
		File::WriteAllText(Path::Combine(project.ScriptsDirectory, "GameScript.cs"), starterScript);

		// Generate .vscode/settings.json
		std::string vsCodeSettings = R"({
    "omnisharp.projectFilesExcludePattern": [],
    "dotnet.defaultSolution": ")" + name + R"(.sln"
})";
		File::WriteAllText(Path::Combine(project.RootDirectory, ".vscode", "settings.json"), vsCodeSettings);

		// Generate NativeScripts CMakeLists.txt
		std::string cmakeFile = R"(cmake_minimum_required(VERSION 3.20)
project()" + name + R"(-NativeScripts LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE NATIVE_SOURCES "Source/*.cpp" "Source/*.h" "Source/*.hpp")
add_library(${PROJECT_NAME} SHARED ${NATIVE_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    )" + std::filesystem::path(Path::ExecutableDir()).parent_path().parent_path().parent_path().string() + R"(/Bolt-Engine/src
)
target_compile_definitions(${PROJECT_NAME} PRIVATE BT_PLATFORM_WINDOWS)
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /utf-8)
endif()
)";
		File::WriteAllText(Path::Combine(project.NativeScriptsDir, "CMakeLists.txt"), cmakeFile);

		// Generate .gitignore
		File::WriteAllText(Path::Combine(project.RootDirectory, ".gitignore"),
			"bin/\nobj/\n.vs/\n*.user\nNativeScripts/build/\nPackages/\n");

		BT_INFO_TAG("BoltProject", "Created project: {} at {}", name, project.RootDirectory);
		return project;
	}

} // namespace Bolt
