#include "pch.hpp"
#include "Project/ProjectManager.hpp"

namespace Bolt {

	std::unique_ptr<BoltProject> ProjectManager::s_CurrentProject = nullptr;

	void ProjectManager::SetCurrentProject(std::unique_ptr<BoltProject> project) {
		s_CurrentProject = std::move(project);
	}

	BoltProject* ProjectManager::GetCurrentProject() {
		return s_CurrentProject.get();
	}

	bool ProjectManager::HasProject() {
		return s_CurrentProject != nullptr;
	}

} // namespace Bolt
