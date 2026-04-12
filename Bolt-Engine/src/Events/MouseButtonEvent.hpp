#pragma once

#include "Events/BoltEvent.hpp"

namespace Bolt {

	class MouseButtonEvent : public BoltEvent {
	public:
		int GetButton() const { return m_Button; }

		BT_EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryMouse | EventCategoryMouseButton)

	protected:
		explicit MouseButtonEvent(int button)
			: m_Button(button) {
		}

		int m_Button;
	};

} // namespace Bolt
