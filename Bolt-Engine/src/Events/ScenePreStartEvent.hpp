#pragma once

#include "Events/SceneEvent.hpp"

namespace Bolt {

	class ScenePreStartEvent : public SceneEvent {
	public:
		explicit ScenePreStartEvent(const std::string& sceneName)
			: SceneEvent(sceneName) {
		}

		BT_EVENT_CLASS_TYPE(ScenePreStart)
	};

} // namespace Bolt
