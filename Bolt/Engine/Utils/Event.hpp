#pragma once
#include <functional>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <utility>

namespace Bolt {
    template<typename... Args>
    class Event {
    public:
        using Callback = std::function<void(Args...)>;
        using ListenerId = std::uint64_t;

        ListenerId Add(Callback cb) {
            const ListenerId id = ++m_NextId;
            m_Listeners.push_back({ id, std::move(cb) });
            return id;
        }
        bool Remove(ListenerId id) {
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
            ListenerId id;
            Callback cb;
        };

        std::vector<Entry> m_Listeners;
        ListenerId m_NextId = 0;
    };
}