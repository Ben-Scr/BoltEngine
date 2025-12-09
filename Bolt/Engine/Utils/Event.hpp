#pragma once
#include <functional>
#include <vector>

namespace Bolt {
	template<typename... Args>
	class Event {
	public:
		using Callback = std::function<void(Args...)>;

		void Add(Callback callback)
		{
			m_Listeners.push_back(std::move(callback));
		}

		void Clear() {
			m_Listeners.clear();
		}

		void Invoke(Args... args) {
			for (auto& listener : m_Listeners) {
				listener(args...);
			}
		}

	private:
		std::vector<Callback> m_Listeners;
	};
}