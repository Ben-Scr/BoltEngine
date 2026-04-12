#pragma once

#include "Events/SceneEvent.hpp"

namespace Bolt {

	class ScenePostStopEvent : public SceneEvent {
	public:
		explicit ScenePostStopEvent(const std::string& sceneName)
			: SceneEvent(sceneName) {
		}

		BT_EVENT_CLASS_TYPE(ScenePostStop)
	};

} // namespace Bolt
