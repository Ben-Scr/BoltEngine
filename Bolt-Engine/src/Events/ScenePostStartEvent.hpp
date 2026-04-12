#pragma once

#include "Events/SceneEvent.hpp"

namespace Bolt {

	class ScenePostStartEvent : public SceneEvent {
	public:
		explicit ScenePostStartEvent(const std::string& sceneName)
			: SceneEvent(sceneName) {
		}

		BT_EVENT_CLASS_TYPE(ScenePostStart)
	};

} // namespace Bolt
