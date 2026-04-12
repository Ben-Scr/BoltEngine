#pragma once
#include "Core/Layer.hpp"
#include "Project/LauncherRegistry.hpp"
#include "Core/Export.hpp"
#include <chrono>
#include <string>
#include <unordered_map>

#ifdef BT_PLATFORM_WINDOWS
#include <windows.h>
#endif

namespace Bolt {

	class BOLT_API LauncherLayer : public Layer {
	public:
		using Layer::Layer;

		void OnAttach(Application& app) override;
		void OnImGuiRender(Application& app) override;
		void OnDetach(Application& app) override;

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

		bool m_IsCreating = false;
		bool m_OpenCreatePopup = false;
		std::string m_DeferredUpdatePath;

		bool m_IsOpening = false;
		std::chrono::steady_clock::time_point m_OpenStartTime;
		std::string m_OpeningProjectName;

#ifdef BT_PLATFORM_WINDOWS
		std::unordered_map<std::string, DWORD> m_RunningProjects;
#endif
		std::string m_OpenError;
	};

}
