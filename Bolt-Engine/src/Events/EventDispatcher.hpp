#pragma once

#include "Events/BoltEvent.hpp"

namespace Bolt {

	class EventDispatcher {
	public:
		explicit EventDispatcher(BoltEvent& event)
			: m_Event(event) {
		}

		template<typename T, typename F>
		bool Dispatch(F&& func) {
			if (m_Event.GetEventType() == T::GetStaticType() && !m_Event.Handled) {
				m_Event.Handled = func(static_cast<T&>(m_Event));
				return true;
			}
			return false;
		}

	private:
		BoltEvent& m_Event;
	};

} // namespace Bolt
