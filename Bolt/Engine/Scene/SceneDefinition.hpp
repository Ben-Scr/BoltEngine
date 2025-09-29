#pragma once
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <typeindex>

namespace Bolt {
    class Scene;
    class ISystem;


    class SceneDefinition {
        friend class SceneManager;

    public:
        explicit SceneDefinition(const std::string& name)
            : m_name(name) {
        }


        template<typename TSystem, typename... Args>
        SceneDefinition& AddSystem(Args&&... args) {
            static_assert(std::is_base_of<ISystem, TSystem>::value,
                "TSystem must derive from ISystem");


            m_systemFactories.emplace_back(
                [capturedArgs = std::tuple<Args...>(std::forward<Args>(args)...)]() mutable {
                    return std::apply(
                        [](auto&&... a) {
                            return std::make_unique<TSystem>(std::forward<decltype(a)>(a)...);
                        },
                        capturedArgs
                    );
                }
            );


            m_systemTypes.push_back(std::type_index(typeid(TSystem)));

            return *this;
        }


        SceneDefinition& OnInitialize(std::function<void(Scene&)> callback) {
            m_initializeCallbacks.push_back(callback);
            return *this;
        }


        SceneDefinition& OnLoad(std::function<void(Scene&)> callback) {
            m_loadCallbacks.push_back(callback);
            return *this;
        }

        SceneDefinition& OnUnload(std::function<void(Scene&)> callback) {
            m_unloadCallbacks.push_back(callback);
            return *this;
        }


        SceneDefinition& SetAsStartupScene() {
            m_isStartupScene = true;
            return *this;
        }


        SceneDefinition& SetPersistent(bool persistent) {
            m_isPersistent = persistent;
            return *this;
        }

        const std::string& getName() const { return m_name; }
        bool isStartupScene() const { return m_isStartupScene; }
        bool isPersistent() const { return m_isPersistent; }

    private:
        friend class Scene;
        friend class SceneManager;

        std::string m_name;
        std::vector<std::function<std::unique_ptr<ISystem>()>> m_systemFactories;
        std::vector<std::type_index> m_systemTypes;
        std::vector<std::function<void(Scene&)>> m_initializeCallbacks;
        std::vector<std::function<void(Scene&)>> m_loadCallbacks;
        std::vector<std::function<void(Scene&)>> m_unloadCallbacks;

        bool m_isStartupScene = false;
        bool m_isPersistent = false;


        std::unique_ptr<Scene> Instantiate() const;
    };
}