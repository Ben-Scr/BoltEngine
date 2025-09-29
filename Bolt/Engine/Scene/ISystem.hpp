#pragma once

namespace Bolt {
    class Scene;

    class ISystem {
    public:
        virtual void Update(Scene& scene) {}
        virtual void FixedUpdate(Scene& scene) {}
        virtual void Awake(Scene& scene) {}
        virtual void Start(Scene& scene) {}
        virtual void OnDisable(Scene& scene) {}
        virtual void OnDestroy(Scene& scene) {}
        bool IsEnabled() { return m_Enabled; }
        virtual ~ISystem() = default;
    private:
        bool m_Enabled = true;
        friend class Scene;
    };
}