#pragma once

#include "Events/BoltEvent.hpp"

namespace Bolt {

	class MouseScrolledEvent : public BoltEvent {
	public:
		MouseScrolledEvent(float xOffset, float yOffset)
			: m_XOffset(xOffset), m_YOffset(yOffset) {
		}

		float GetXOffset() const { return m_XOffset; }
		float GetYOffset() const { return m_YOffset; }

		std::string ToString() const override {
			return std::string("MouseScrolledEvent: ") + std::to_string(m_XOffset) + ", " + std::to_string(m_YOffset);
		}

		BT_EVENT_CLASS_TYPE(MouseScrolled)
		BT_EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryMouse)

	private:
		float m_XOffset;
		float m_YOffset;
	};

} // namespace Bolt
