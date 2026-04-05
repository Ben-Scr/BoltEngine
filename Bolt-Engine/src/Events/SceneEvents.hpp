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
			: m_SceneName(sceneName) {}

		std::string m_SceneName;
	};

	class ScenePreStartEvent : public SceneEvent {
	public:
		explicit ScenePreStartEvent(const std::string& sceneName)
			: SceneEvent(sceneName) {}

		BT_EVENT_CLASS_TYPE(ScenePreStart)
	};

	class ScenePostStartEvent : public SceneEvent {
	public:
		explicit ScenePostStartEvent(const std::string& sceneName)
			: SceneEvent(sceneName) {}

		BT_EVENT_CLASS_TYPE(ScenePostStart)
	};

	class ScenePreStopEvent : public SceneEvent {
	public:
		explicit ScenePreStopEvent(const std::string& sceneName)
			: SceneEvent(sceneName) {}

		BT_EVENT_CLASS_TYPE(ScenePreStop)
	};

	class ScenePostStopEvent : public SceneEvent {
	public:
		explicit ScenePostStopEvent(const std::string& sceneName)
			: SceneEvent(sceneName) {}

		BT_EVENT_CLASS_TYPE(ScenePostStop)
	};

} // namespace Bolt
