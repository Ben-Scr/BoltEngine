#pragma once

#include "Events/BoltEvent.hpp"

namespace Bolt {

	class MouseMovedEvent : public BoltEvent {
	public:
		MouseMovedEvent(float x, float y)
			: m_X(x), m_Y(y) {
		}

		float GetX() const { return m_X; }
		float GetY() const { return m_Y; }

		std::string ToString() const override {
			return std::string("MouseMovedEvent: ") + std::to_string(m_X) + ", " + std::to_string(m_Y);
		}

		BT_EVENT_CLASS_TYPE(MouseMoved)
		BT_EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryMouse)

	private:
		float m_X;
		float m_Y;
	};

} // namespace Bolt
