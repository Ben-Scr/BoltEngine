#pragma once

#include "Events/BoltEvent.hpp"

#include <string>

namespace Bolt {

	class SceneEvent : public BoltEvent {
	public:
		const std::string& GetSceneName() const { return m_SceneName; }

		BT_EVENT_CLASS_CATEGORY(EventCategoryScene)

	protected:
		explicit SceneEvent(const std::string& sceneName)
			: m_SceneName(sceneName) {
		}

		std::string m_SceneName;
	};

} // namespace Bolt
