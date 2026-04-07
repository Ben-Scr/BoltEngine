#pragma once
#include "Scene/ISystem.hpp"
#include "Project/LauncherRegistry.hpp"
#include "Core/Export.hpp"

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

		bool m_ShowCreatePopup = false;
		char m_NewProjectName[256]{};
		char m_NewProjectLocation[512]{};
		std::string m_CreateError;
		std::string m_BrowseError;

		int m_SelectedIndex = -1;
	};

} // namespace Bolt
