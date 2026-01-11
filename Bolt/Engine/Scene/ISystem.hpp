#pragma once

namespace Bolt {
	class Scene;

	class ISystem {
	public:
		virtual void Update() {}
		virtual void FixedUpdate() {}
		virtual void Awake() {}
		virtual void Start() {}
		virtual void OnDisable() {}
		virtual void OnDestroy() {}

		virtual void OnApplicationInitialize() {}
		virtual void OnApplicationExit() {}

		bool IsEnabled() { return m_Enabled; }
		virtual ~ISystem() = default;

		void SetScene(std::weak_ptr<Scene> scene) { m_Scene = scene; }
		Scene& GetScene() { return *m_Scene.lock(); }

	private:
		std::weak_ptr<Scene> m_Scene;
		bool m_Enabled = true;
		friend class Scene;
		friend class SceneDefinition;
	};
}