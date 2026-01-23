#pragma once
#include <functional>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <utility>
#include "../Collections/Ids.hpp"

namespace Bolt {
    template<typename... Args>
    class Event {
    public:
        using Callback = std::function<void(Args...)>;

        EventId Add(Callback cb) {
            const EventId id = EventId(++m_NextId.value);
            m_Listeners.push_back({ id, std::move(cb) });
            return id;
        }
        bool Remove(EventId id) {
            auto it = std::remove_if(m_Listeners.begin(), m_Listeners.end(), [id](const Entry& e) { return e.id == id; });
            const bool removed = (it != m_Listeners.end());
            m_Listeners.erase(it, m_Listeners.end());
            return removed;
        }

        void Clear() {
            m_Listeners.clear();
        }

        void Invoke(Args... args) {
            for (auto& e : m_Listeners) {
                e.cb(args...);
            }
        }

    private:
        struct Entry {
            EventId id;
            Callback cb;
        };

        std::vector<Entry> m_Listeners;
        EventId m_NextId;
    };
}