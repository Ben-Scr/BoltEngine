#pragma once

#include "Events/BoltEvent.hpp"

namespace Bolt {

	class KeyEvent : public BoltEvent {
	public:
		int GetKeyCode() const { return m_KeyCode; }

		BT_EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryKeyboard)

	protected:
		explicit KeyEvent(int keyCode)
			: m_KeyCode(keyCode) {
		}

		int m_KeyCode;
	};

} // namespace Bolt
