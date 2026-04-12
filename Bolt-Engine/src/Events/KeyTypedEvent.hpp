#pragma once

#include "Events/KeyEvent.hpp"

namespace Bolt {

	class KeyTypedEvent : public KeyEvent {
	public:
		explicit KeyTypedEvent(int keyCode)
			: KeyEvent(keyCode) {
		}

		BT_EVENT_CLASS_TYPE(KeyTyped)
	};

} // namespace Bolt
