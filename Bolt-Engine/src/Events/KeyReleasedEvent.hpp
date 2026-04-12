#pragma once

#include "Events/KeyEvent.hpp"

namespace Bolt {

	class KeyReleasedEvent : public KeyEvent {
	public:
		explicit KeyReleasedEvent(int keyCode)
			: KeyEvent(keyCode) {
		}

		std::string ToString() const override {
			return std::string("KeyReleasedEvent: ") + std::to_string(m_KeyCode);
		}

		BT_EVENT_CLASS_TYPE(KeyReleased)
	};

} // namespace Bolt
