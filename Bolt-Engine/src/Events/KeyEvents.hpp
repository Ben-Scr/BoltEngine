#pragma once
#include "Events/BoltEvent.hpp"

namespace Bolt {

	class KeyEvent : public BoltEvent {
	public:
		int GetKeyCode() const { return m_KeyCode; }

		BT_EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryKeyboard)

	protected:
		explicit KeyEvent(int keyCode)
			: m_KeyCode(keyCode) {}

		int m_KeyCode;
	};

	class KeyPressedEvent : public KeyEvent {
	public:
		KeyPressedEvent(int keyCode, bool isRepeat = false)
			: KeyEvent(keyCode), m_IsRepeat(isRepeat) {}

		bool IsRepeat() const { return m_IsRepeat; }

		std::string ToString() const override {
			return std::string("KeyPressedEvent: ") + std::to_string(m_KeyCode) + " (repeat=" + (m_IsRepeat ? "true" : "false") + ")";
		}

		BT_EVENT_CLASS_TYPE(KeyPressed)

	private:
		bool m_IsRepeat;
	};

	class KeyReleasedEvent : public KeyEvent {
	public:
		explicit KeyReleasedEvent(int keyCode)
			: KeyEvent(keyCode) {}

		std::string ToString() const override {
			return std::string("KeyReleasedEvent: ") + std::to_string(m_KeyCode);
		}

		BT_EVENT_CLASS_TYPE(KeyReleased)
	};

	class KeyTypedEvent : public KeyEvent {
	public:
		explicit KeyTypedEvent(int keyCode)
			: KeyEvent(keyCode) {}

		BT_EVENT_CLASS_TYPE(KeyTyped)
	};

} // namespace Bolt
