#pragma once
#include "Events/BoltEvent.hpp"
#include <vector>
#include <string>

namespace Bolt {

	class WindowCloseEvent : public BoltEvent {
	public:
		WindowCloseEvent() = default;

		BT_EVENT_CLASS_TYPE(WindowClose)
		BT_EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class WindowResizeEvent : public BoltEvent {
	public:
		WindowResizeEvent(int width, int height)
			: m_Width(width), m_Height(height) {}

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

	class WindowFocusEvent : public BoltEvent {
	public:
		WindowFocusEvent() = default;

		BT_EVENT_CLASS_TYPE(WindowFocus)
		BT_EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class WindowLostFocusEvent : public BoltEvent {
	public:
		WindowLostFocusEvent() = default;

		BT_EVENT_CLASS_TYPE(WindowLostFocus)
		BT_EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class WindowMinimizeEvent : public BoltEvent {
	public:
		explicit WindowMinimizeEvent(bool minimized)
			: m_Minimized(minimized) {}

		bool IsMinimized() const { return m_Minimized; }

		BT_EVENT_CLASS_TYPE(WindowMinimize)
		BT_EVENT_CLASS_CATEGORY(EventCategoryApplication)

	private:
		bool m_Minimized;
	};

	class FileDropEvent : public BoltEvent {
	public:
		explicit FileDropEvent(std::vector<std::string> paths)
			: m_Paths(std::move(paths)) {}

		const std::vector<std::string>& GetPaths() const { return m_Paths; }
		int GetCount() const { return static_cast<int>(m_Paths.size()); }

		BT_EVENT_CLASS_TYPE(FileDrop)
		BT_EVENT_CLASS_CATEGORY(EventCategoryApplication)

	private:
		std::vector<std::string> m_Paths;
	};

} // namespace Bolt
