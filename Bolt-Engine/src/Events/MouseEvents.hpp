#pragma once
#include "Events/BoltEvent.hpp"

namespace Bolt {

	class MouseMovedEvent : public BoltEvent {
	public:
		MouseMovedEvent(float x, float y)
			: m_X(x), m_Y(y) {}

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

	class MouseScrolledEvent : public BoltEvent {
	public:
		MouseScrolledEvent(float xOffset, float yOffset)
			: m_XOffset(xOffset), m_YOffset(yOffset) {}

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

	class MouseButtonEvent : public BoltEvent {
	public:
		int GetButton() const { return m_Button; }

		BT_EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryMouse | EventCategoryMouseButton)

	protected:
		explicit MouseButtonEvent(int button)
			: m_Button(button) {}

		int m_Button;
	};

	class MouseButtonPressedEvent : public MouseButtonEvent {
	public:
		explicit MouseButtonPressedEvent(int button)
			: MouseButtonEvent(button) {}

		std::string ToString() const override {
			return std::string("MouseButtonPressedEvent: ") + std::to_string(m_Button);
		}

		BT_EVENT_CLASS_TYPE(MouseButtonPressed)
	};

	class MouseButtonReleasedEvent : public MouseButtonEvent {
	public:
		explicit MouseButtonReleasedEvent(int button)
			: MouseButtonEvent(button) {}

		std::string ToString() const override {
			return std::string("MouseButtonReleasedEvent: ") + std::to_string(m_Button);
		}

		BT_EVENT_CLASS_TYPE(MouseButtonReleased)
	};

} // namespace Bolt
