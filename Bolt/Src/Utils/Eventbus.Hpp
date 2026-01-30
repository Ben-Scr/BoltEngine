#pragma once
#include <functional>
#include <unordered_map>
#include <cstdint>
#include "Collections/Ids.hpp"

namespace Bolt {

    template <class... Args>
    class EventBus {
    public:
        using Callback = std::function<void(Args...)>;

        SubscriptionId subscribe(Callback cb) {
            const SubscriptionId id = ++m_nextId;
            m_callbacks.emplace(id, std::move(cb));
            return id;
        }

        void unsubscribe(SubscriptionId id) {
            m_callbacks.erase(id);
        }

        void publish(Args... args) {
            for (auto& [id, cb] : m_callbacks) {
                cb(args...);
            }
        }

    private:
        SubscriptionId m_nextId{ 0 };
        std::unordered_map<SubscriptionId, Callback> m_callbacks;
    };

}