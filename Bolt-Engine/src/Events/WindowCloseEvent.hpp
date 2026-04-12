#pragma once

#include "Events/BoltEvent.hpp"

namespace Bolt {

	class WindowCloseEvent : public BoltEvent {
	public:
		WindowCloseEvent() = default;

		BT_EVENT_CLASS_TYPE(WindowClose)
		BT_EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

} // namespace Bolt
