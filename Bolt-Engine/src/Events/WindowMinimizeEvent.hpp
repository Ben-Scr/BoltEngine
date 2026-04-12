#pragma once

#include "Events/BoltEvent.hpp"

namespace Bolt {

	class WindowMinimizeEvent : public BoltEvent {
	public:
		explicit WindowMinimizeEvent(bool minimized)
			: m_Minimized(minimized) {
		}

		bool IsMinimized() const { return m_Minimized; }

		BT_EVENT_CLASS_TYPE(WindowMinimize)
		BT_EVENT_CLASS_CATEGORY(EventCategoryApplication)

	private:
		bool m_Minimized;
	};

} // namespace Bolt
