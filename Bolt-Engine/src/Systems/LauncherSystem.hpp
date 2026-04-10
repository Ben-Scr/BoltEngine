#pragma once
#include "Scene/ISystem.hpp"
#include "Project/LauncherRegistry.hpp"
#include "Core/Export.hpp"
#include <chrono>
#include <string>
#include <unordered_map>

#ifdef BT_PLATFORM_WINDOWS
#include <windows.h>
#endif

namespace Bolt {

	class BOLT_API LauncherSystem : public ISystem {
	public:
		void Awake(Scene& scene) override;
		void OnGui(Scene& scene) override;
		void OnDestroy(Scene& scene) override;

	private:
		void RenderLauncherPanel();
		void RenderProjectList();
		void RenderCreateProjectPopup();
		void OpenProject(const LauncherProjectEntry& entry);
		void BrowseForExistingProject();

		LauncherRegistry m_Registry;

		char m_NewProjectName[256]{};
		char m_NewProjectLocation[512]{};
		std::string m_CreateError;
		std::string m_BrowseError;

		// L2: Prevent double-click on Create
		bool m_IsCreating = false;

		// L6: Signal to open the modal on next frame
		bool m_OpenCreatePopup = false;

		// L1: Deferred re-sort after loading screen
		std::string m_DeferredUpdatePath;

		bool m_IsOpening = false;
		std::chrono::steady_clock::time_point m_OpenStartTime;
		std::string m_OpeningProjectName;

		// L3: Track running projects (path -> PID)
#ifdef BT_PLATFORM_WINDOWS
		std::unordered_map<std::string, DWORD> m_RunningProjects;
#endif
		std::string m_OpenError;
	};

} // namespace Bolt
