#pragma once
#include "Entity.hpp"
#include <string>
#include <unordered_map>
#include <typeindex>

namespace Bolt {
    enum ComponentCategory {
        Component,
        Tag,
        System,
	};

    struct ComponentInfo {
        std::string displayName;
        ComponentCategory category;

        ComponentInfo() = default;
		ComponentInfo(const std::string& displayName, ComponentCategory category)
            : displayName(std::move(displayName)), category(category) {}

        bool (*has)(Entity) = nullptr;
        void (*add)(Entity) = nullptr;
        void (*remove)(Entity) = nullptr;
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