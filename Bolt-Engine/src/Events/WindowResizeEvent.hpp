#pragma once

#include "Events/BoltEvent.hpp"

namespace Bolt {

	class WindowResizeEvent : public BoltEvent {
	public:
		WindowResizeEvent(int width, int height)
			: m_Width(width), m_Height(height) {
		}

		int GetWidth() const { return m_Width; }
		int GetHeight() const { return m_Height; }

		std::string ToString() const override {
			return std::string("WindowResizeEvent: ") + std::to_string(m_Width) + ", " + std::to_string(m_Height);
		}

		BT_EVENT_CLASS_TYPE(WindowResize)
		BT_EVENT_CLASS_CATEGORY(EventCategoryApplication)

	private:
		int m_Width;
		int m_Height;
	};

} // namespace Bolt
