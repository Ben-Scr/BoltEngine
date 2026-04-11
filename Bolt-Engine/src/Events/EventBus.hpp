#pragma once
#include "Collections/Ids.hpp"
#include "Core/Export.hpp"
#include "Events/BoltEvent.hpp"

#include <algorithm>
#include <functional>
#include <utility>
#include <vector>

namespace Bolt {
	class Application;

	class BOLT_API EventBus {
	public:
		using Callback = std::function<void(BoltEvent&)>;

		EventId Subscribe(Callback callback) {
			const EventId id(++m_NextId.value);
			m_Listeners.push_back({ id, std::move(callback) });
			return id;
		}

		template<typename TEvent, typename F>
		EventId Subscribe(F&& callback) {
			return Subscribe([fn = std::forward<F>(callback)](BoltEvent& event) mutable {
				if (event.GetEventType() == TEvent::GetStaticType()) {
					fn(static_cast<TEvent&>(event));
				}
			});
		}

		bool Unsubscribe(EventId id) {
			auto it = std::remove_if(m_Listeners.begin(), m_Listeners.end(), [id](const Entry& entry) {
				return entry.Id == id;
			});
			const bool removed = it != m_Listeners.end();
			m_Listeners.erase(it, m_Listeners.end());
			return removed;
		}

		void Clear() {
			m_Listeners.clear();
		}

	private:
		friend class Application;

		struct Entry {
			EventId Id;
			Callback Listener;
		};

		void Publish(BoltEvent& event) {
			const auto snapshot = m_Listeners;
			for (const Entry& entry : snapshot) {
				if (event.Handled) {
					break;
				}
				entry.Listener(event);
			}
		}

		std::vector<Entry> m_Listeners;
		EventId m_NextId{};
	};
}
