#pragma once

#include "Events/SceneEvent.hpp"

namespace Bolt {

	class ScenePreStopEvent : public SceneEvent {
	public:
		explicit ScenePreStopEvent(const std::string& sceneName)
			: SceneEvent(sceneName) {
		}

		BT_EVENT_CLASS_TYPE(ScenePreStop)
	};

} // namespace Bolt
