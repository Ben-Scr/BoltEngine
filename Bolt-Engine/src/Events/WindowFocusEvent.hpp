#pragma once

#include "Events/BoltEvent.hpp"

namespace Bolt {

	class WindowFocusEvent : public BoltEvent {
	public:
		WindowFocusEvent() = default;

		BT_EVENT_CLASS_TYPE(WindowFocus)
		BT_EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

} // namespace Bolt
