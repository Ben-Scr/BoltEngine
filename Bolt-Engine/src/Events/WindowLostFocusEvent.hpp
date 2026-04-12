#pragma once

#include "Events/BoltEvent.hpp"

namespace Bolt {

	class WindowLostFocusEvent : public BoltEvent {
	public:
		WindowLostFocusEvent() = default;

		BT_EVENT_CLASS_TYPE(WindowLostFocus)
		BT_EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

} // namespace Bolt
