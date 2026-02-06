#pragma once
#include "Entity.hpp"
#include <string>
#include <unordered_map>
#include <typeindex>

namespace Bolt {
    struct ComponentInfo {
        std::string displayName;
        std::string category;
        int order = 0;

        // Kern-Operationen für den Editor (type-erased)
        bool (*has)(Entity) = nullptr;
        void (*add)(Entity) = nullptr;
        void (*remove)(Entity) = nullptr;
        void (*drawInspector)(Entity) = nullptr; // optional
    };

    class ComponentRegistry {
    public:
        template<typename T>
        void Register(ComponentInfo info) {
            const std::type_index id = typeid(T);

            info.has = [](Entity e) { return e.HasComponent<T>(); };
            info.add = [](Entity e) { e.AddComponent<T>(); };
            info.remove = [](Entity e) { e.RemoveComponent<T>(); };

            m_map[id] = std::move(info);
        }

        const auto& All() const { return m_map; }

        template <typename F>
        void ForEachComponentInfo(F&& fn) {
            for (auto& [id, info] : m_map)
                fn(id, info);
        }

        template <typename F>
        void ForEachComponentInfo(F&& fn) const {
            for (const auto& [id, info] : m_map)
                fn(id, info);
        }

        template <typename F>
        void ForEachComponentInfoSorted(F&& fn) const {
            std::vector<std::pair<std::type_index, const ComponentInfo*>> items;
            items.reserve(m_map.size());

            for (const auto& [id, info] : m_map)
                items.emplace_back(id, &info);

            std::sort(items.begin(), items.end(),
                [](auto& a, auto& b) {
                    const auto& A = *a.second;
                    const auto& B = *b.second;
                    if (A.order != B.order) return A.order < B.order;
                    return A.displayName < B.displayName;
                });

            for (auto& [id, infoPtr] : items)
                fn(id, *infoPtr);
        }

    private:
        std::unordered_map<std::type_index, ComponentInfo> m_map;
    };
}