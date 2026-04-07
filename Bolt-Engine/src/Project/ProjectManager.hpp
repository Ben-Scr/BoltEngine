#pragma once
#include "Project/BoltProject.hpp"
#include "Core/Export.hpp"
#include <memory>

namespace Bolt {

	class BOLT_API ProjectManager {
	public:
		static void SetCurrentProject(std::unique_ptr<BoltProject> project);
		static BoltProject* GetCurrentProject();
		static bool HasProject();

	private:
		static std::unique_ptr<BoltProject> s_CurrentProject;
	};

} // namespace Bolt
