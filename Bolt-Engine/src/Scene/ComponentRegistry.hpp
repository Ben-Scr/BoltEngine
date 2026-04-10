#pragma once
#include "Entity.hpp"
#include <string>
#include <unordered_map>
#include <typeindex>

namespace Bolt {
    enum class ComponentCategory {
        Component,
        Tag,
        System,
    };

    struct ComponentInfo {
        std::string displayName;
        ComponentCategory category;

        ComponentInfo() = default;
		ComponentInfo(const std::string& displayName, ComponentCategory category)
            : displayName(displayName), category(category) {}

        bool (*has)(Entity) = nullptr;
        void (*add)(Entity) = nullptr;
        void (*remove)(Entity) = nullptr;
        void (*copyTo)(Entity src, Entity dst) = nullptr;
        void (*drawInspector)(Entity) = nullptr;
    };

    class ComponentRegistry {
    public:
        template<typename T>
        void Register(ComponentInfo info) {
            const std::type_index id = typeid(T);

            info.has = [](Entity e) { return e.HasComponent<T>(); };
            info.add = [](Entity e) { e.AddComponent<T>(); };
            info.remove = [](Entity e) { e.RemoveComponent<T>(); };

            if constexpr (!std::is_empty_v<T>) {
                info.copyTo = [](Entity src, Entity dst) {
                    if (!src.HasComponent<T>()) return;
                    if (dst.HasComponent<T>())
                        dst.GetComponent<T>() = src.GetComponent<T>();
                    else
                        dst.AddComponent<T>(src.GetComponent<T>());
                };
            } else {
                info.copyTo = [](Entity src, Entity dst) {
                    if (src.HasComponent<T>() && !dst.HasComponent<T>())
                        dst.AddComponent<T>();
                };
            }

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

    private:
        std::unordered_map<std::type_index, ComponentInfo> m_map;
    };
}