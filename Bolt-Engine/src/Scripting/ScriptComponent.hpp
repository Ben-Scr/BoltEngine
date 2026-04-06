#pragma once
#include "Scripting/ScriptInstance.hpp"
#include <string>
#include <vector>

namespace Bolt {

	struct ScriptComponent {
		std::vector<ScriptInstance> Scripts;

		ScriptComponent() = default;

		void AddScript(const std::string& className) {
			Scripts.emplace_back(className);
		}

		void RemoveScript(size_t index) {
			if (index < Scripts.size()) {
				Scripts.erase(Scripts.begin() + static_cast<ptrdiff_t>(index));
			}
		}

		bool HasScript(const std::string& className) const {
			for (const auto& s : Scripts) {
				if (s.GetClassName() == className) return true;
			}
			return false;
		}
	};

} 
